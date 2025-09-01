# Duplicate File Remover


## Description


**Duplicate File Remover** is a high-performance command-line utility designed to identify and manage duplicate files within a directory using MPI (Message Passing Interface) for parallel processing. By leveraging cryptographic hashing (SHA-256) and distributed computing, the program efficiently calculates file hashes across multiple processes to detect duplicates. Users can then decide which file to keep, while the remaining duplicates are moved to a specified destination.


---


## Key Features


- **Parallel Processing:** Utilizes MPI to distribute file scanning and hashing across multiple processes
- **Recursive Search:** Scans the given directory and its subdirectories for files
- **Hash-Based Detection:** Uses SHA-256 hashing to accurately identify duplicates
- **Interactive User Prompt:** Allows users to select which file to retain among detected duplicates
- **File Management:** Moves unselected duplicate files to a destination directory
- **Performance Metrics:** Reports execution time, speedup, and efficiency
- **Error Handling:** Provides user-friendly error messages for file access or processing issues


---


## Prerequisites


Before running the program, ensure the following:


1. A C++ compiler with support for **C++17** or later
2. [OpenSSL](https://www.openssl.org/) library for SHA-256 hashing
3. [MPI Implementation](https://www.open-mpi.org/) (OpenMPI or MPICH)
4. Filesystem support (standard with **C++17**)
5. Permissions to access, read, and move files in the directories you specify


---


## Installation


Follow these steps to set up and run the program:


1. **Install Dependencies:**  
   Ensure OpenSSL and MPI are installed:
   ```bash
   # For Ubuntu/Debian
   sudo apt update
   sudo apt install libssl-dev openmpi-bin openmpi-common libopenmpi-dev
   
   # For CentOS/RHEL
   sudo yum install openssl-devel openmpi-devel
   ```


2. **Clone or Download the Source Code:**  
   Get the program source code to your local machine.


3. **Compile the Program:**  
   Use the MPI compiler wrapper to compile the program:
   ```bash
   mpic++ -std=c++17 -o ParallelDuplicateFileRemover Parallel_Duplicate_File_Remover.cpp -lssl -lcrypto
   ```


4. **Run the Program with MPI:**  
   Execute the program using MPI:
   ```bash
   mpirun -np 4 ./ParallelDuplicateFileRemover
   ```
   Replace `4` with the number of processes you want to use.


---


## Usage


1. **Launch the Program:**  
   Run the compiled executable with MPI to start the Parallel Duplicate File Remover.


2. **Enter Directories:**  
   - Specify the root directory to search for duplicate files  
   - Specify the destination directory where duplicate files should be moved


3. **Monitor Parallel Processing:**  
   - The program will distribute file processing across all available MPI processes
   - Process 0 will collect and consolidate results from all processes


4. **Handle Duplicates:**  
   - The program identifies groups of duplicate files and displays their paths
   - You will be prompted to select which file to keep from each group
   - The unselected files are moved to the destination directory


5. **Review Performance Metrics:**  
   - The program reports execution time, speedup, and efficiency after completion


---


## Example


```plaintext
$ mpirun -np 4 ./ParallelDuplicateFileRemover


-----------------------------------
Welcome to Duplicate File Remover
-----------------------------------


Enter the root directory to search for duplicate files: /home/user/Documents
Enter the destination directory for duplicate files: /home/user/RemovedDuplicates


Scanning for duplicate files with 4 processes.....


Time taken: 24.7966 seconds


Speedup: 4.0
Efficiency: 1.0


Found 3 groups of duplicate files.


Found the following duplicate files:
1: /home/user/Documents/file1.txt
2: /home/user/Documents/copy_of_file1.txt
3: /home/user/Documents/backup/file1.txt
0: To Skip the current file group


Enter the number of the file you want to keep (1-3): 1


Moved /home/user/Documents/copy_of_file1.txt to /home/user/RemovedDuplicates/copy_of_file1.txt
Moved /home/user/Documents/backup/file1.txt to /home/user/RemovedDuplicates/file1.txt


Duplicate files have been processed.
```


---


## Performance Metrics


The program reports three key performance metrics:


1. **Execution Time:** Total time taken to scan and identify duplicates
2. **Speedup:** Ratio of sequential execution time to parallel execution time
3. **Efficiency:** Speedup divided by the number of processes (ideal value is 1.0)


These metrics help evaluate the effectiveness of the parallel implementation and identify potential bottlenecks.


---


## Implementation Details


### Parallel Strategy
- Files are distributed across processes using a simple modulo-based approach
- Each process computes hashes for its assigned files independently
- Process 0 acts as the master, collecting and consolidating results from all workers
- Custom MPI communication protocol for sending hash and file path data


### Hash Computation
- Uses OpenSSL's EVP interface for SHA-256 hashing
- Processes files in chunks to handle large files efficiently
- Converts binary hashes to hexadecimal strings for comparison


### Data Distribution
- Simple round-robin distribution: file_count % process_count == rank
- Balanced workload assuming similar file sizes across the directory


---


## Limitations


- Requires sufficient storage space in the destination directory to move duplicates
- The program does not delete files; duplicates are only moved to the specified directory
- Load balancing assumes relatively uniform file sizes; large files may cause imbalance
- MPI communication overhead may limit scalability for very large numbers of processes


---


## Future Enhancements


- Dynamic load balancing for better distribution of large files
- Support for symbolic links and directories
- Additional hash algorithms for user selection
- Graphical user interface (GUI) for non-technical users
- Hybrid MPI+OpenMP implementation for multi-level parallelism
- Network file system optimization for distributed environments


---


## Contributing


Contributions to improve the Parallel Duplicate File Remover are welcome. Please feel free to submit pull requests or open issues for bugs and feature requests.


---


## License


This project is open source and available under the [MIT License](LICENSE).
