#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "file_info.h"
#include "duplicate_detector.h"

void print_usage(const char* program_name) {
    printf("GitHub LazySync v2.0 - Smart File Management\n\n");
    printf("Usage: %s [OPTIONS] [DIRECTORY]\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -s, --scan              Scan directory for duplicates\n");
    printf("  -c, --cleanup             Clean up found duplicates\n");
    printf("  -i, --interactive        Interactive mode with fzf\n");
    printf("  -d, --dry-run            Show what would be done\n");
    printf("  -t, --threshold NUM      Similarity threshold (0.0-1.0)\n");
    printf("  -b, --backup DIR         Backup directory\n");
    printf("  -v, --verbose            Verbose output\n\n");
    printf("Examples:\n");
    printf("  %s --scan /path/to/vault\n", program_name);
    printf("  %s --cleanup --interactive /path/to/vault\n", program_name);
    printf("  %s --cleanup --dry-run --threshold 0.9 /path/to/vault\n", program_name);
}

int main(int argc, char* argv[]) {
    Config config = {
        .strategy = KEEP_NEWEST,
        .similarity_threshold = 0.85,
        .backup_before_delete = 1,
        .interactive_mode = 0,
        .dry_run = 0,
        .backup_location = NULL,
        .exclude_patterns = NULL,
        .exclude_count = 0
    };
    
    int scan_mode = 0;
    int cleanup_mode = 0;
    const char* directory = ".";
    
    // Parse command line arguments
    static struct option long_options[] = {
        {"help",         no_argument,       0, 'h'},
        {"scan",         no_argument,       0, 's'},
        {"cleanup",      no_argument,       0, 'c'},
        {"interactive",   no_argument,       0, 'i'},
        {"dry-run",      no_argument,       0, 'd'},
        {"threshold",     required_argument, 0, 't'},
        {"backup",       required_argument, 0, 'b'},
        {"verbose",      no_argument,       0, 'v'},
        {0, 0, 0, 0}
    };
    
    int c;
    while ((c = getopt_long(argc, argv, "hscidt:b:v", long_options, NULL)) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv[0]);
                return 0;
            case 's':
                scan_mode = 1;
                break;
            case 'c':
                cleanup_mode = 1;
                break;
            case 'i':
                config.interactive_mode = 1;
                break;
            case 'd':
                config.dry_run = 1;
                break;
            case 't':
                config.similarity_threshold = atof(optarg);
                if (config.similarity_threshold < 0.0 || config.similarity_threshold > 1.0) {
                    fprintf(stderr, "Error: Threshold must be between 0.0 and 1.0\n");
                    return 1;
                }
                break;
            case 'b':
                config.backup_location = strdup(optarg);
                break;
            case 'v':
                // Set verbose flag (for now, we'll ignore)
                break;
            case '?':
                print_usage(argv[0]);
                return 1;
            default:
                abort();
        }
    }
    
    // Get directory from remaining arguments
    if (optind < argc) {
        directory = argv[optind];
    }
    
    // Default to scan mode if no mode specified
    if (!scan_mode && !cleanup_mode) {
        scan_mode = 1;
    }
    
    printf("🔍 GitHub LazySync v2.0\n");
    printf("Directory: %s\n", directory);
    printf("Mode: %s\n", scan_mode ? "Scan" : "Cleanup");
    if (config.interactive_mode) printf("Interactive: Yes\n");
    if (config.dry_run) printf("Dry Run: Yes\n");
    printf("Threshold: %.2f\n\n", config.similarity_threshold);
    
    // Perform action
    DuplicateReport* report = find_duplicates(directory, &config);
    if (!report) {
        fprintf(stderr, "Error: Failed to scan directory\n");
        return 1;
    }
    
    printf("📊 Scan Results:\n");
    printf("Total files: %d\n", report->total_files);
    printf("Duplicate groups: %d\n", report->group_count);
    
    if (report->group_count > 0) {
        printf("\n🔄 Found Duplicate Groups:\n");
        for (int i = 0; i < report->group_count; i++) {
            DuplicateGroup* group = &report->groups[i];
            printf("Group %d: %d files\n", i + 1, group->count);
            
            for (int j = 0; j < group->count; j++) {
                FileInfo* info = group->files[j];
                printf("  - %s (%ld bytes, %s)\n", 
                       info->filepath,
                       info->file_size,
                       ctime(&info->mtime));
            }
            
            if (config.interactive_mode) {
                char* selected = fzf_select_duplicate(group, 1);
                if (selected) {
                    printf("User selected: %s\n", selected);
                    free(selected);
                }
            }
            printf("\n");
        }
        
        if (cleanup_mode && !config.dry_run) {
            printf("🧹 Starting cleanup...\n");
            // TODO: Implement actual cleanup logic
        } else if (config.dry_run) {
            printf("🔍 DRY RUN - No files were modified\n");
        }
    } else {
        printf("✅ No duplicates found!\n");
    }
    
    duplicate_report_free(report);
    
    if (config.backup_location) {
        free(config.backup_location);
    }
    
    return 0;
}