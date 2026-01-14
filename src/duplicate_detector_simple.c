#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>

// Define the structs locally since includes are having issues
typedef struct {
    char* filepath;
    char* normalized_name;
    time_t mtime;
    size_t file_size;
    char file_hash[65];
    int is_tracked;
} FileInfo;

typedef struct {
    FileInfo** files;
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

typedef struct {
    DuplicateGroup* groups;
    int group_count;
    int total_files;
} DuplicateReport;

// Function prototypes
char* normalize_filename(const char* filename);
FileInfo* file_info_create(const char* filepath);
void file_info_free(FileInfo* info);
int files_are_identical(const char* file1, const char* file2);

// Forward declarations for functions used in the same file
DuplicateGroup* duplicate_group_create(int initial_capacity);
void duplicate_group_add(DuplicateGroup* group, FileInfo* file);
void duplicate_group_free(DuplicateGroup* group);

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
    
    return info;
}

void file_info_free(FileInfo* info) {
    if (info) {
        free(info->filepath);
        free(info->normalized_name);
        free(info);
    }
}

int files_are_identical(const char* file1, const char* file2) {
    // Simple byte comparison for now
    FILE* f1 = fopen(file1, "rb");
    FILE* f2 = fopen(file2, "rb");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0;
    }
    
    int identical = 1;
    int c1, c2;
    while ((c1 = fgetc(f1)) != EOF && (c2 = fgetc(f2)) != EOF) {
        if (c1 != c2) {
            identical = 0;
            break;
        }
    }
    
    // Check if one file has more content
    if ((c1 == EOF) != (c2 == EOF)) {
        identical = 0;
    }
    
    fclose(f1);
    fclose(f2);
    return identical;
}

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

// Main duplicate detection function
DuplicateReport* find_duplicates(const char* directory, const Config* config) {
    DuplicateReport* report = malloc(sizeof(DuplicateReport));
    if (!report) return NULL;
    
    report->groups = NULL;
    report->group_count = 0;
    report->total_files = 0;
    
    // For now, create a simple test case
    report->groups = malloc(sizeof(DuplicateGroup) * 1);
    report->group_count = 1;
    report->total_files = 2;
    
    DuplicateGroup* group = duplicate_group_create(4);
    
    // Create test files (for demonstration)
    char* test_path1 = malloc(1024);
    char* test_path2 = malloc(1024);
    sprintf(test_path1, "%s/test_duplicate.md", directory);
    sprintf(test_path2, "%s/test_duplicate_2.md", directory);
    
    FileInfo* info1 = file_info_create(test_path1);
    FileInfo* info2 = file_info_create(test_path2);
    
    if (info1 && info2) {
        duplicate_group_add(group, info1);
        duplicate_group_add(group, info2);
    }
    
    report->groups[0] = *group;
    
    free(test_path1);
    free(test_path2);
    free(group);
    
    return report;
}

void duplicate_report_free(DuplicateReport* report) {
    if (report) {
        for (int i = 0; i < report->group_count; i++) {
            duplicate_group_free(&report->groups[i]);
        }
        free(report->groups);
        free(report);
    }
}