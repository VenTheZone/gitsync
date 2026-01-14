#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <openssl/sha.h>
#include "file_info.h"

// Create FileInfo structure
FileInfo* file_info_create(const char* filepath) {
    FileInfo* info = malloc(sizeof(FileInfo));
    if (!info) return NULL;
    
    info->filepath = strdup(filepath);
    info->normalized_name = normalize_filename(filepath);
    info->mtime = 0;
    info->file_size = 0;
    info->is_tracked = 0;
    memset(info->file_hash, 0, sizeof(info->file_hash));
    
    // Get file stats
    struct stat st;
    if (stat(filepath, &st) == 0) {
        info->mtime = st.st_mtime;
        info->file_size = st.st_size;
    }
    
    // Calculate hash
    calculate_file_hash(filepath, info->file_hash);
    
    return info;
}

void file_info_free(FileInfo* info) {
    if (info) {
        free(info->filepath);
        free(info->normalized_name);
        free(info);
    }
}

// Compare function for sorting
int file_info_compare(const void* a, const void* b) {
    const FileInfo* fa = (const FileInfo*)a;
    const FileInfo* fb = (const FileInfo*)b;
    return strcmp(fa->normalized_name, fb->normalized_name);
}

// Normalize filename for comparison
char* normalize_filename(const char* filename) {
    if (!filename) return NULL;
    
    char* name = strdup(filename);
    if (!name) return NULL;
    
    // Remove .md extension
    char* ext = strrchr(name, '.');
    if (ext && strcmp(ext, ".md") == 0) {
        *ext = '\0';
    }
    
    // Convert to lowercase and remove special chars
    char* p = name;
    char* write_pos = name;
    while (*p) {
        if (isalnum(*p) || *p == ' ' || *p == '-' || *p == '_') {
            *write_pos = tolower(*p);
            write_pos++;
        }
        p++;
    }
    *write_pos = '\0';
    
    // Remove trailing numbers and spaces
    write_pos = name + strlen(name) - 1;
    while (write_pos >= name && (*write_pos == ' ' || isdigit(*write_pos))) {
        *write_pos = '\0';
        write_pos--;
    }
    
    return name;
}

// Check if files are identical using hash
int files_are_identical(const char* file1, const char* file2) {
    char hash1[65], hash2[65];
    calculate_file_hash(file1, hash1);
    calculate_file_hash(file2, hash2);
    return strcmp(hash1, hash2) == 0;
}

// Calculate SHA-256 hash of file
void calculate_file_hash(const char* filepath, char* hash_output) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        strcpy(hash_output, "0000000000000000000000000000000000000000000000000000000000000000000000");
        return;
    }
    
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    unsigned char buffer[4096];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        SHA256_Update(&sha256, buffer, bytes_read);
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(hash, &sha256);
    
    fclose(file);
    
    // Convert to hex string
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }
    hash_output[64] = '\0';
}

// Duplicate group functions
DuplicateGroup* duplicate_group_create(int initial_capacity) {
    DuplicateGroup* group = malloc(sizeof(DuplicateGroup));
    if (!group) return NULL;
    
    group->files = malloc(sizeof(FileInfo*) * initial_capacity);
    group->count = 0;
    group->capacity = initial_capacity;
    
    return group;
}

void duplicate_group_add(DuplicateGroup* group, FileInfo* file) {
    if (group->count >= group->capacity) {
        group->capacity *= 2;
        group->files = realloc(group->files, sizeof(FileInfo*) * group->capacity);
    }
    
    group->files[group->count++] = file;
}

void duplicate_group_free(DuplicateGroup* group) {
    if (group) {
        for (int i = 0; i < group->count; i++) {
            file_info_free(group->files[i]);
        }
        free(group->files);
        free(group);
    }
}

void duplicate_group_sort(DuplicateGroup* group) {
    qsort(group->files, group->count, sizeof(FileInfo*), file_info_compare);
}