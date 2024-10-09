#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <linux/limits.h>

#define BUFSIZE 1024

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int num_text_files = 0;
long total_regular_bytes = 0;
long total_text_bytes = 0;
int num_special_files = 0;
int Directories = 0;
int bad_files = 0;

void *process_file_or_directory(void *arg);
int is_all_printable(const char *filename);
long count_bytes_in_regular_files(const char *path);
long count_bytes_in_text_files(const char *path);
int count_special_files(const char *path);



int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s thread <max_threads>\n", argv[0]);
        return 1;
    }

    int max_threads = atoi(argv[2]);
    if (max_threads < 1 || max_threads > 15) {
        fprintf(stderr, "Error: max_threads must be between 1 and 15.\n");
        return 1;
    }

    char input[PATH_MAX];
    pthread_t threads[max_threads];
    int thread_count = 0;

    while (scanf("%s", input) != EOF) {
        char *filename = strdup(input);
        if (pthread_create(&threads[thread_count], NULL, process_file_or_directory, filename) != 0) {
            perror("pthread_create");
            free(filename);
            continue;
        }

        thread_count++;

        if (thread_count >= max_threads) {
            pthread_join(threads[thread_count - max_threads], NULL);
            thread_count--;
        }
    }

    for (int i = 0; i < thread_count; ++i) {
        pthread_join(threads[i], NULL);
    }

    printf("Bad Files: %d\n", bad_files);
    printf("Directories: %d\n", Directories);
    printf("Regular Files: %d\n", num_special_files + num_text_files);
    printf("Special Files: %d\n", num_special_files);
    printf("Regular File Bytes: %ld\n", total_regular_bytes);
    printf("Text Files: %d\n", num_text_files);
    printf("Text File Bytes: %ld\n", total_text_bytes);

    return 0;
}








void *process_file_or_directory(void *arg) {
    char *path = (char *)arg;
    struct stat sb;

    if (stat(path, &sb) == 0 && S_ISDIR(sb.st_mode)) {
        num_special_files += count_special_files(path);
        total_regular_bytes += count_bytes_in_regular_files(path);
        total_text_bytes += count_bytes_in_text_files(path);
        Directories += 1;
    } else {
        if (stat(path, &sb) == -1) {
            perror("stat");
            fprintf(stderr, "Error stating file %s\n", path);
            free(path);
            pthread_exit(NULL);
        }

        if (!S_ISREG(sb.st_mode)) {
            free(path);
            pthread_exit(NULL);
        }

        long file_size = sb.st_size;
        int is_text_file = is_all_printable(path);

        pthread_mutex_lock(&mutex);
        num_special_files++;
        total_regular_bytes += file_size;
        if (is_text_file) {
            num_text_files++;
            total_text_bytes += file_size;
        }
        pthread_mutex_unlock(&mutex);
    }

    free(path);
    pthread_exit(NULL);
}





int is_all_printable(const char *filename) {
    int fd, nread;
    char buf[BUFSIZE];

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open");
        bad_files += 1;
        return 0;
    }

    while ((nread = read(fd, buf, BUFSIZE)) > 0) {
        for (int i = 0; i < nread; i++) {
            if (!isprint(buf[i]) && !isspace(buf[i])) {
                close(fd);
                return 0;
            }
        }
    }

    close(fd);
    return 1;
}





long count_bytes_in_regular_files(const char *path) {
    long total_bytes = 0;
    struct stat sb;
    struct dirent *entry;
    DIR *dir;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (stat(filepath, &sb) == -1) {
            perror("stat");
            closedir(dir);
            return -1;
        }

        if (S_ISREG(sb.st_mode)) {
            total_bytes += sb.st_size;
        }
    }

    closedir(dir);
    return total_bytes;
}






long count_bytes_in_text_files(const char *path) {
    long total_bytes = 0;
    struct stat sb;
    struct dirent *entry;
    DIR *dir;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (stat(filepath, &sb) == -1) {
            perror("stat");
            closedir(dir);
            return -1;
        }

        if (S_ISREG(sb.st_mode) && is_all_printable(filepath)) {
            total_bytes += sb.st_size;
            pthread_mutex_lock(&mutex);
            num_text_files++;
            pthread_mutex_unlock(&mutex);
        }
    }

    closedir(dir);
    return total_bytes;
}





int count_special_files(const char *path) {
    int count = 0;
    struct stat sb;
    struct dirent *entry;
    DIR *dir;

    dir = opendir(path);
    if (dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(dir)) != NULL) {
        char filepath[PATH_MAX];
        snprintf(filepath, sizeof(filepath), "%s/%s", path, entry->d_name);

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (lstat(filepath, &sb) == -1) {
            perror("lstat");
            closedir(dir);
            return -1;
        }

        if (!S_ISDIR(sb.st_mode) && !S_ISREG(sb.st_mode)) {
            count++;
        }
    }

    closedir(dir);
    return count;
}