# Duplicate File Remover

## Description

**Duplicate File Remover** is a command-line utility designed to identify and manage duplicate files within a directory. By leveraging cryptographic hashing (SHA-256), the program calculates unique file hashes to detect duplicates. Users can then decide which file to keep, while the remaining duplicates are moved to a specified destination.

---

## Features

- **Recursive Search:** Scans the given directory and its subdirectories for files.
- **Hash-Based Detection:** Uses SHA-256 hashing to accurately identify duplicates.
- **Interactive User Prompt:** Allows users to select which file to retain among detected duplicates.
- **File Management:** Moves unselected duplicate files to a destination directory.
- **Error Handling:** Provides user-friendly error messages for file access or processing issues.

---

## Prerequisites

Before running the program, ensure the following:

1. A C++ compiler with support for **C++17** or later.
2. [OpenSSL](https://www.openssl.org/) library for SHA-256 hashing.
3. Filesystem support (standard with **C++17**).
4. Permissions to access, read, and move files in the directories you specify.

---

## Installation

Follow these steps to set up and run the program:

1. **Install OpenSSL:**  
   Ensure OpenSSL is installed. Refer to the [OpenSSL Installation Guide](https://www.openssl.org/docs/) for your platform.

2. **Clone or Download the Repository:**  
   Clone the repository or download the source code to your local machine.

3. **Compile the Program:**  
   Use a C++ compiler to compile the program. For example:
   ```bash
   g++ -std=c++17 -o DuplicateFileRemover Duplicate_File_Remover.cpp -lssl -lcrypto

4. **Run the Program**
   ```bash
   ./DuplicateFileRemover

---

## Usage

1. **Launch the Program:**  
   Run the compiled executable to start the Duplicate File Remover.

2. **Enter Directories:**  
   - Specify the root directory to search for duplicate files.  
   - Specify the destination directory where duplicate files should be moved.

3. **Handle Duplicates:**  
   - The program identifies groups of duplicate files and displays their paths.  
   - You will be prompted to select which file to keep from each group.  
   - The unselected files are moved to the destination directory.

4. **Completion:**  
   The program will notify you once all duplicates have been processed.

---

##Example

```plaintext
-----------------------------------
Welcome to Duplicate File Remover
-----------------------------------

Enter the root directory to search for duplicate files: /home/user/Documents
Enter the destination directory for duplicate files: /home/user/RemovedDuplicates

Scanning for duplicate files.....

Found 1 group of duplicate files.

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

## Limitations

- Requires sufficient storage space in the destination directory to move duplicates.
- Currently, only regular files are checked for duplicates; symbolic links and directories are ignored.
- The program does not delete files; duplicates are only moved to the specified directory.

---

## Future Enhancements

- Add support for symbolic links and directories.
- Provide a graphical user interface (GUI) for non-technical users.
- Implement multi-threading for faster duplicate detection in large directories.

---
