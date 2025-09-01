#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>
#include <openssl/evp.h>
#include <iomanip>
#include <sstream>
#include <mpi.h>
#include <chrono>

using namespace std;
namespace fs = filesystem;

// Function to compute the SHA-256 hash of a file
string get_file_hash(const fs::path& file_path) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_length;
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();

    if (mdctx == nullptr) {
        cerr << "Could not create EVP_MD_CTX" << endl;
        return "";
    }

    const EVP_MD* md = EVP_sha256();
    EVP_DigestInit_ex(mdctx, md, nullptr);

    try {
        ifstream file(file_path, ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Could not open file");
        }

        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            EVP_DigestUpdate(mdctx, buffer, file.gcount());
        }
        EVP_DigestUpdate(mdctx, buffer, file.gcount());

        EVP_DigestFinal_ex(mdctx, hash, &hash_length);

        ostringstream hex_stream;
        for (unsigned int i = 0; i < hash_length; ++i) {
            hex_stream << hex << setw(2) << setfill('0') << (int)hash[i];
        }

        EVP_MD_CTX_free(mdctx);
        return hex_stream.str();
    } catch (const exception& e) {
        cerr << "Could not read file " << file_path << ": " << e.what() << endl;
        EVP_MD_CTX_free(mdctx);
        return "";
    }
}

// Function to process a subset of files in a directory
unordered_map<string, vector<fs::path>> process_files_subset(const fs::path& root_directory, int rank, int size) {
    unordered_map<string, vector<fs::path>> local_files_hash;
    int file_count = 0;
    
    // Iterate through all files and process only those assigned to this process
    for (const auto& entry : fs::recursive_directory_iterator(root_directory)) {
        if (fs::is_regular_file(entry)) {
            // Simple distribution strategy: process files where (file_count % size) == rank
            if (file_count % size == rank) {
                string file_hash = get_file_hash(entry.path());
                if (!file_hash.empty()) {
                    local_files_hash[file_hash].push_back(entry.path());
                }
            }
            file_count++;
        }
    }
    
    return local_files_hash;
}

// Function to gather all file hashes from all processes
unordered_map<string, vector<fs::path>> gather_all_hashes(const unordered_map<string, vector<fs::path>>& local_files_hash, int rank, int size) {
    unordered_map<string, vector<fs::path>> global_files_hash;
    
    if (size == 1) {
        return local_files_hash;
    }
    
    // First, gather all hash counts from each process
    vector<int> hash_counts(size);
    int local_count = local_files_hash.size();
    MPI_Gather(&local_count, 1, MPI_INT, hash_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Process 0 will collect all data
    if (rank == 0) {
        // First, add local data
        global_files_hash = local_files_hash;
        
        // Receive data from other processes
        for (int src = 1; src < size; src++) {
            int num_entries = hash_counts[src];
            for (int i = 0; i < num_entries; i++) {
                // Receive hash string
                int hash_length;
                MPI_Recv(&hash_length, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                char* hash_str = new char[hash_length + 1];
                MPI_Recv(hash_str, hash_length, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                hash_str[hash_length] = '\0';
                string hash_key(hash_str);
                delete[] hash_str;
                
                // Receive number of paths for this hash
                int num_paths;
                MPI_Recv(&num_paths, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                
                // Receive each path
                for (int j = 0; j < num_paths; j++) {
                    int path_length;
                    MPI_Recv(&path_length, 1, MPI_INT, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    char* path_str = new char[path_length + 1];
                    MPI_Recv(path_str, path_length, MPI_CHAR, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    path_str[path_length] = '\0';
                    global_files_hash[hash_key].push_back(fs::path(path_str));
                    delete[] path_str;
                }
            }
        }
    } else {
        // Send data to process 0
        int num_entries = local_files_hash.size();
        for (const auto& [hash, paths] : local_files_hash) {
            // Send hash string
            int hash_length = hash.size();
            MPI_Send(&hash_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(hash.c_str(), hash_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            
            // Send number of paths
            int num_paths = paths.size();
            MPI_Send(&num_paths, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            
            // Send each path
            for (const auto& path : paths) {
                string path_str = path.string();
                int path_length = path_str.size();
                MPI_Send(&path_length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
                MPI_Send(path_str.c_str(), path_length, MPI_CHAR, 0, 0, MPI_COMM_WORLD);
            }
        }
    }
    
    return global_files_hash;
}

// Function to find duplicate files using MPI
vector<vector<fs::path>> find_duplicate_files_mpi(const fs::path& root_directory, int rank, int size) {
    vector<vector<fs::path>> duplicates;
    
    // Each process processes its subset of files
    auto local_files_hash = process_files_subset(root_directory, rank, size);
    
    // Gather all results to process 0
    auto global_files_hash = gather_all_hashes(local_files_hash, rank, size);
    
    // Only process 0 handles the duplicates
    if (rank == 0) {
        for (const auto& [_, file_paths] : global_files_hash) {
            if (file_paths.size() > 1) {
                duplicates.push_back(file_paths);
            }
        }
    }
    
    return duplicates;
}

// Function to move duplicate files (unchanged from original)
void move_duplicates(const vector<vector<fs::path>>& duplicates, const fs::path& destination) {
    fs::create_directories(destination);

    for (const auto& file_group : duplicates) {
        cout << "\nFound the following duplicate files:" << endl;
        for (size_t i = 0; i < file_group.size(); ++i) {
            cout << i + 1 << ": " << file_group[i] << endl;
        }
        cout << "0: To Skip the current file group" << endl;

        int choice;
        cout << "\nEnter the number of the file you want to keep (1-" << file_group.size() << "): ";
        cin >> choice;
        cout<<endl;
        
        if(choice == 0) continue;

        while (choice < 1 || choice > file_group.size()) {
            cout << "Invalid choice. Please enter a number between 1 and " << file_group.size()+1 << ": ";
            cin >> choice;
        }

        for (size_t i = 0; i < file_group.size(); ++i) {
            if (i + 1 != choice) {
                try {
                    fs::path destination_path = destination / file_group[i].filename();
                    fs::rename(file_group[i], destination_path);
                    cout << "Moved " << file_group[i] << " to " << destination_path << endl;
                } catch (const exception& e) {
                    cerr << "Could not move file " << file_group[i] << ": " << e.what() << endl;
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    string root_directory;
    string destination_directory;

    if (rank == 0) {
        cout << "\n\n-----------------------------------\n";
        cout << "Welcome to Duplicate File Remover";
        cout << "\n-----------------------------------\n\n";
        cout.flush(); // Ensure the welcome message is printed

        cout << "Enter the root directory to search for duplicate files: ";
        cout.flush(); // Force prompt display before getline()
        getline(cin, root_directory);

        cout << "Enter the destination directory for duplicate files: ";
        cout.flush(); // Force prompt display before getline()
        getline(cin, destination_directory);

        cout << "\nScanning for duplicate files with " << size << " processes.....\n" << endl;
    }

    // Broadcast root directory
    int root_dir_len = (rank == 0) ? root_directory.size() : 0;
    MPI_Bcast(&root_dir_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    char* root_dir_buffer = new char[root_dir_len + 1];
    if (rank == 0) {
        strcpy(root_dir_buffer, root_directory.c_str());
    }
    MPI_Bcast(root_dir_buffer, root_dir_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    root_directory = string(root_dir_buffer);
    delete[] root_dir_buffer;

    // Broadcast destination directory
    int dest_dir_len = (rank == 0) ? destination_directory.size() : 0;
    MPI_Bcast(&dest_dir_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    char* dest_dir_buffer = new char[dest_dir_len + 1];
    if (rank == 0) {
        strcpy(dest_dir_buffer, destination_directory.c_str());
    }
    MPI_Bcast(dest_dir_buffer, dest_dir_len + 1, MPI_CHAR, 0, MPI_COMM_WORLD);
    destination_directory = string(dest_dir_buffer);
    delete[] dest_dir_buffer;
    
    auto start = std::chrono::high_resolution_clock::now();
    // Now all processes proceed to scan files
    auto duplicates = find_duplicate_files_mpi(root_directory, rank, size);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Only rank 0 handles duplicates
    if (rank == 0) {
    	std::cout << "Time taken: " << elapsed.count() << " seconds\n\n" << std::endl;
    	double sequentialTime = 99.1866;
        double parallelTime = elapsed.count();
        double speedup = sequentialTime / parallelTime;
        double efficiency = speedup / size;

        std::cout << "Speedup: " << speedup << std::endl;
        std::cout << "Efficiency: " << efficiency << std::endl;

        if (!duplicates.empty()) {
            cout << "Found " << duplicates.size() << " groups of duplicate files." << endl;
            move_duplicates(duplicates, destination_directory);
            cout << "\nDuplicate files have been processed." << endl;
        } else {
            cout << "No duplicate files found." << endl;
        }
    }

    MPI_Finalize();
    return 0;
}
