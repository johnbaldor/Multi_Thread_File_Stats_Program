File Statistics Server Program
This program simulates a server that handles requests to gather statistics about files. The program processes a list of file names, determines their types (regular files, directories, special files), and counts text files. It supports two architectures: a serial version and a multi-threaded version that allows multiple files to be processed in parallel.

To compile the program:
g++ -o proj3 proj3.cpp -lpthread

To run the serial version of the program:
./proj3

To run the multi-threaded version with a thread limit, use the following:
./proj3 thread [max_threads]

Example:
./proj3 thread 10

Features
File Type Identification: Identifies file types such as regular files, directories, and special files using the stat() system call.
Text File Detection: Reads file contents to determine whether a file contains only text.
Statistics Collection: Counts the total number of files, file sizes, and the number of text files.
Multi-threaded Architecture: Processes file requests concurrently using threads, with a specified limit on the number of threads running at once.
