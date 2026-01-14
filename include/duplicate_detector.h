#ifndef DUPLICATE_DETECTOR_H
#define DUPLICATE_DETECTOR_H

#include "file_info.h"

// Duplicate detection functions
typedef struct {
    DuplicateGroup* groups;
    int group_count;
    int total_files;
} DuplicateReport;

// Main detection function
DuplicateReport* find_duplicates(const char* directory, const Config* config);
void duplicate_report_free(DuplicateReport* report);

// FZF integration for interactive selection
char* fzf_select_duplicate(const DuplicateGroup* group, int interactive);

// Similarity analysis
double calculate_content_similarity(const char* file1, const char* file2);
int is_numbered_duplicate(const char* name1, const char* name2);
FileInfo* select_file_to_keep(const DuplicateGroup* group, ResolutionStrategy strategy);

#endif // DUPLICATE_DETECTOR_H