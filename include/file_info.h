#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <time.h>
#include <stddef.h>

typedef struct {
    char* filepath;
    char* normalized_name;
    time_t mtime;
    size_t file_size;
    char file_hash[65];  // SHA-256 hash (64 chars + null terminator)
    int is_tracked;      // Whether file is tracked by git
} FileInfo;

typedef struct {
    FileInfo* files;
    int count;
    int capacity;
} DuplicateGroup;

typedef enum {
    KEEP_NEWEST,
    KEEP_LARGEST,
    KEEP_MOST_RECENT_COMMIT,
    KEEP_MANUAL_SELECT
} ResolutionStrategy;

typedef struct {
    ResolutionStrategy strategy;
    double similarity_threshold;
    int backup_before_delete;
    int interactive_mode;
    int dry_run;
    char* backup_location;
    char** exclude_patterns;
    int exclude_count;
} Config;

// File information functions
FileInfo* file_info_create(const char* filepath);
void file_info_free(FileInfo* info);
int file_info_compare(const void* a, const void* b);
char* normalize_filename(const char* filename);
int files_are_identical(const char* file1, const char* file2);
void calculate_file_hash(const char* filepath, char* hash_output);

// Duplicate group functions
DuplicateGroup* duplicate_group_create(int initial_capacity);
void duplicate_group_add(DuplicateGroup* group, FileInfo* file);
void duplicate_group_free(DuplicateGroup* group);
void duplicate_group_sort(DuplicateGroup* group);

#endif // FILE_INFO_H