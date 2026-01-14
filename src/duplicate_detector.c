#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fnmatch.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "file_info.h"

// Forward declarations since we need normalize_filename
char* normalize_filename(const char* filename);
int files_are_identical(const char* file1, const char* file2);

// Line diff-based similarity calculation
double calculate_content_similarity(const char* file1, const char* file2) {
    FILE* f1 = fopen(file1, "r");
    FILE* f2 = fopen(file2, "r");
    if (!f1 || !f2) {
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return 0.0;
    }
    
    // Count lines in both files
    int lines1 = 0, lines2 = 0;
    char buffer[1024];
    
    while (fgets(buffer, sizeof(buffer), f1)) lines1++;
    rewind(f1);
    
    while (fgets(buffer, sizeof(buffer), f2)) lines2++;
    rewind(f2);
    
    if (lines1 == 0 && lines2 == 0) return 1.0;
    if (lines1 == 0 || lines2 == 0) return 0.0;
    
    // Calculate line-by-line similarity
    int same_lines = 0;
    int total_lines = (lines1 > lines2) ? lines1 : lines2;
    
    char line1[1024], line2[1024];
    while (fgets(line1, sizeof(line1), f1) && fgets(line2, sizeof(line2), f2)) {
        if (strcmp(line1, line2) == 0) {
            same_lines++;
        }
    }
    
    fclose(f1);
    fclose(f2);
    
    return (double)same_lines / (double)total_lines;
}

// Check if one file is a numbered duplicate of another
int is_numbered_duplicate(const char* name1, const char* name2) {
    // Remove .md extension
    char* base1 = strdup(name1);
    char* base2 = strdup(name2);
    
    char* ext1 = strrchr(base1, '.');
    char* ext2 = strrchr(base2, '.');
    if (ext1) *ext1 = '\0';
    if (ext2) *ext2 = '\0';
    
    // Normalize both names
    char* norm1 = normalize_filename(base1);
    char* norm2 = normalize_filename(base2);
    
    int result = 0;
    
    // Check if one looks like "filename 2" and the other is "filename"
    if (strstr(norm1, " ") && strstr(norm2, " ")) {
        char* space1 = strrchr(norm1, ' ');
        char* space2 = strrchr(norm2, ' ');
        
        if (space1) *space1 = '\0';
        if (space2) *space2 = '\0';
        
        // Check base names are the same and one has a number
        if (strcmp(norm1, norm2) == 0) {
            if ((space1 && isdigit(space1[1])) || (space2 && isdigit(space2[1]))) {
                result = 1;
            }
        }
    }
    
    free(base1);
    free(base2);
    free(norm1);
    free(norm2);
    
    return result;
}

// Select which file to keep based on strategy
FileInfo* select_file_to_keep(const DuplicateGroup* group, ResolutionStrategy strategy) {
    if (group->count <= 1) return group->files[0];
    
    FileInfo* best = group->files[0];
    
    for (int i = 1; i < group->count; i++) {
        FileInfo* current = group->files[i];
        
        switch (strategy) {
            case KEEP_NEWEST:
                if (current->mtime > best->mtime) {
                    best = current;
                }
                break;
                
            case KEEP_LARGEST:
                if (current->file_size > best->file_size) {
                    best = current;
                }
                break;
                
            case KEEP_MOST_RECENT_COMMIT:
                // For now, use modification time as proxy
                if (current->mtime > best->mtime) {
                    best = current;
                }
                break;
                
            case KEEP_MANUAL_SELECT:
                // This will be handled by FZF integration
                break;
        }
    }
    
    return best;
}

// FZF integration for interactive file selection
char* fzf_select_duplicate(const DuplicateGroup* group, int interactive) {
    if (!interactive || group->count <= 1) {
        return strdup(group->files[0]->filepath);
    }
    
    // Create temporary file with options
    char temp_file[] = "/tmp/lazysync_fzf_XXXXXX";
    int fd = mkstemp(temp_file);
    if (fd == -1) return NULL;
    
    FILE* f = fdopen(fd, "w");
    if (!f) {
        close(fd);
        return NULL;
    }
    
    // Write options to temporary file
    for (int i = 0; i < group->count; i++) {
        FileInfo* info = group->files[i];
        fprintf(f, "%s|%ld|%zu|%.2f|", 
                info->filepath, 
                info->mtime, 
                info->file_size,
                0.0); // similarity to best file
        
        // Read first few lines for preview
        FILE* preview = fopen(info->filepath, "r");
        if (preview) {
            char line[200];
            int line_count = 0;
            while (fgets(line, sizeof(line), preview) && line_count < 3) {
                fprintf(f, "|%s", line);
                line_count++;
            }
            fclose(preview);
        }
        fprintf(f, "\n");
    }
    fclose(f);
    
    // Build FZF command
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
            "cat %s | fzf --delimiter='|' "
            "--header='Choose file to keep (arrow keys, enter to select)' "
            "--preview-window=up:3 "
            "--preview='echo {4}' "
            "--bind='ctrl-a:select-all' "
            "--bind='esc:abort' "
            "--height=40%%", 
            temp_file);
    
    // Execute FZF
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {
        unlink(temp_file);
        return NULL;
    }
    
    char selection[1024];
    if (fgets(selection, sizeof(selection), pipe)) {
        // Remove everything after first | (just the filepath)
        char* pipe_pos = strchr(selection, '|');
        if (pipe_pos) {
            *pipe_pos = '\0';
        }
        
        char* result = strdup(selection);
        pclose(pipe);
        unlink(temp_file);
        return result;
    }
    
    pclose(pipe);
    unlink(temp_file);
    return NULL;
}

// Main duplicate detection function
DuplicateReport* find_duplicates(const char* directory, const Config* config) {
    DuplicateReport* report = malloc(sizeof(DuplicateReport));
    if (!report) return NULL;
    
    report->groups = NULL;
    report->group_count = 0;
    report->total_files = 0;
    
    // Collect all files
    FileInfo** all_files = NULL;
    int file_count = 0;
    
    DIR* dir = opendir(directory);
    if (!dir) {
        free(report);
        return NULL;
    }
    
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Skip .git, .obsidian, and hidden files
        if (entry->d_name[0] == '.') continue;
        if (strcmp(entry->d_name, ".git") == 0) continue;
        if (strcmp(entry->d_name, ".obsidian") == 0) continue;
        
        // Check exclude patterns
        int excluded = 0;
        for (int i = 0; i < config->exclude_count; i++) {
            if (fnmatch(config->exclude_patterns[i], entry->d_name, 0) == 0) {
                excluded = 1;
                break;
            }
        }
        if (excluded) continue;
        
        // Only process .md files
        char* ext = strrchr(entry->d_name, '.');
        if (!ext || strcmp(ext, ".md") != 0) continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
        
        FileInfo* info = file_info_create(full_path);
        if (info) {
            all_files = realloc(all_files, sizeof(FileInfo*) * (file_count + 1));
            all_files[file_count++] = info;
        }
    }
    closedir(dir);
    
    report->total_files = file_count;
    
    // Group by normalized names
    for (int i = 0; i < file_count; i++) {
        int found_group = 0;
        
        // Check if this file matches an existing group
        for (int g = 0; g < report->group_count; g++) {
            DuplicateGroup* group = &report->groups[g];
            if (strcmp(group->files[0]->normalized_name, 
                      all_files[i]->normalized_name) == 0 ||
                is_numbered_duplicate(group->files[0]->normalized_name,
                                   all_files[i]->normalized_name)) {
                
                duplicate_group_add(group, all_files[i]);
                found_group = 1;
                break;
            }
        }
        
        // Create new group if no match found
        if (!found_group) {
            report->groups = realloc(report->groups, 
                                sizeof(DuplicateGroup) * (report->group_count + 1));
            DuplicateGroup* new_group = duplicate_group_create(4);
            duplicate_group_add(new_group, all_files[i]);
            report->groups[report->group_count++] = *new_group;
            free(new_group);
        }
    }
    
    // Filter groups with actual duplicates (count > 1)
    int actual_groups = 0;
    for (int i = 0; i < report->group_count; i++) {
        if (report->groups[i].count > 1) {
            if (actual_groups != i) {
                report->groups[actual_groups] = report->groups[i];
            }
            actual_groups++;
        } else {
            duplicate_group_free(&report->groups[i]);
        }
    }
    report->group_count = actual_groups;
    
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