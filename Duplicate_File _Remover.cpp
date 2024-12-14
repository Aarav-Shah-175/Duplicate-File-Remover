#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <openssl/evp.h> //sha256 (Converts long binary strings to a 256 byte unique hex code)
#include <iomanip>
#include <sstream>

using namespace std;
namespace fs = filesystem;

// Function to compute the MD5 hash of a file using EVP
string get_file_hash(const fs::path& file_path) {
    unsigned char hash[EVP_MAX_MD_SIZE]; // Buffer for the hash
    unsigned int hash_length;            // Length of the hash
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new(); // Create a new context

    if (mdctx == nullptr) {
        cerr << "Could not create EVP_MD_CTX" << endl;  //standardized error checking
        return "";
    }

    const EVP_MD* md = EVP_sha256(); // Use SHA-256 algorithm
    EVP_DigestInit_ex(mdctx, md, nullptr); // Initialize the context

    try {
        ifstream file(file_path, ios::binary);
        if (!file.is_open()) {
            throw runtime_error("Could not open file");
        }

        char buffer[4096];
        while (file.read(buffer, sizeof(buffer))) {
            EVP_DigestUpdate(mdctx, buffer, file.gcount()); // Update hash with the file's data
        }
        EVP_DigestUpdate(mdctx, buffer, file.gcount()); // Handle the last chunk

        EVP_DigestFinal_ex(mdctx, hash, &hash_length); // Finalize the hash

        ostringstream hex_stream;
        for (unsigned int i = 0; i < hash_length; ++i) {
            hex_stream << hex << setw(2) << setfill('0') << (int)hash[i];
        }

        EVP_MD_CTX_free(mdctx); // Clean up

        return hex_stream.str();
    } catch (const exception& e) {
        cerr << "Could not read file " << file_path << ": " << e.what() << endl;
        EVP_MD_CTX_free(mdctx); // Clean up
        return "";
    }
}


// Function to find duplicate files (Uses inbuilt functions like recursive_directory_iterator to traverse the entire directory and store duplicates in a 2D Vector)
vector<vector<fs::path>> find_duplicate_files(const fs::path& root_directory) {
    unordered_map<string, vector<fs::path>> files_hash;
    vector<vector<fs::path>> duplicates;

    for (const auto& entry : fs::recursive_directory_iterator(root_directory)) {
        if (fs::is_regular_file(entry)) {
            string file_hash = get_file_hash(entry.path());
            if (!file_hash.empty()) {
                files_hash[file_hash].push_back(entry.path());
            }
        }
    }

    for (const auto& [_, file_paths] : files_hash) {
        if (file_paths.size() > 1) {
            duplicates.push_back(file_paths);
        }
    }

    return duplicates;
}

// Function to move duplicate files, asking the user which one to keep
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

        // Validate user input
        while (choice < 1 || choice > file_group.size()) {
            cout << "Invalid choice. Please enter a number between 1 and " << file_group.size()+1 << ": ";
            cin >> choice;
        }

        // Move the files not selected by the user
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

int main() {
    cout << "\n\n-----------------------------------\n";
    cout << "Welcome to Duplicate File Remover";
    cout << "\n-----------------------------------\n\n";

    string root_directory;
    string destination_directory;

    cout << "Enter the root directory to search for duplicate files: ";
    getline(cin, root_directory);

    cout << "Enter the destination directory for duplicate files: ";
    getline(cin, destination_directory);

    cout << "\nScanning for duplicate files.....\n" << endl;
    auto duplicates = find_duplicate_files(root_directory);

    if (!duplicates.empty()) {
        cout << "Found " << duplicates.size() << " groups of duplicate files." << endl;
        move_duplicates(duplicates, destination_directory);
        cout << "\nDuplicate files have been processed." << endl;
    } else {
        cout << "No duplicate files found." << endl;
    }
}
