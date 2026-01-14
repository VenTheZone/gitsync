#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

#define MAX_REPOS 100
#define MAX_PATH_LEN 1024

typedef struct {
    char name[256];
    char path[MAX_PATH_LEN];
    char remote[512];
    char branch[64];
    int is_git;
} Repository;

typedef enum {
    MODE_AUTO,
    MODE_SIMPLE,
    MODE_TUI
} InterfaceMode;

typedef enum {
    COMMIT_DATE,
    COMMIT_MANUAL,
    COMMIT_PROMPT
} CommitMode;

typedef struct {
    InterfaceMode mode;
    const char* scan_dir;
    int use_date_commit;
    CommitMode commit_mode;
    int show_help;
    int show_version;
} ProgramConfig;

Repository repos[MAX_REPOS];
int repo_count = 0;

// Check if directory is a git repository
int is_git_repo(const char* path) {
    char git_path[1024];
    snprintf(git_path, sizeof(git_path), "%s/.git", path);
    
    struct stat st;
    return stat(git_path, &st) == 0 && S_ISDIR(st.st_mode);
}

// Check if repository has uncommitted changes
int has_local_changes(const char *path) {
    char cmd[MAX_PATH_LEN * 2];
    char buffer[256];
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git status --porcelain 2>/dev/null | head -1", path);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    int has_changes = 0;
    if (fgets(buffer, sizeof(buffer), fp)) {
        has_changes = 1;
    }
    pclose(fp);
    return has_changes;
}

// Check if remote has changes
int has_remote_changes(const char *path) {
    char cmd[MAX_PATH_LEN * 2];
    char buffer[256];
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git fetch origin main 2>/dev/null && git log HEAD..origin/main --oneline 2>/dev/null | head -1", path);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    int has_changes = 0;
    if (fgets(buffer, sizeof(buffer), fp)) {
        has_changes = 1;
    }
    pclose(fp);
    return has_changes;
}

// Check if fzf is available
int has_fzf() {
    return system("which fzf > /dev/null 2>&1") == 0;
}

void show_loading(const char *message) {
    printf("[~] %s", message);
    fflush(stdout);
    
    char spinner[] = "|/-\\";
    for (int i = 0; i < 10; i++) {
        printf("\b%c", spinner[i % 4]);
        fflush(stdout);
        usleep(100000);
    }
    printf("\b \n");
}

// Get repository info (remote and branch)
void get_repo_info(Repository* repo) {
    char cmd[MAX_PATH_LEN * 2];
    FILE* fp;
    
    // Initialize defaults
    strcpy(repo->remote, "No remote");
    strcpy(repo->branch, "unknown");
    
    if (!repo->is_git) return;
    
    // Get remote URL
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git remote get-url origin 2>/dev/null || echo 'No remote'", repo->path);
    fp = popen(cmd, "r");
    if (fp) {
        if (fgets(repo->remote, sizeof(repo->remote), fp)) {
            repo->remote[strcspn(repo->remote, "\n")] = '\0';
        }
        pclose(fp);
    }
    
    // Get current branch
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git branch --show-current 2>/dev/null || echo 'unknown'", repo->path);
    fp = popen(cmd, "r");
    if (fp) {
        if (fgets(repo->branch, sizeof(repo->branch), fp)) {
            repo->branch[strcspn(repo->branch, "\n")] = '\0';
        }
        pclose(fp);
    }
}

void scan_directory(const char *path, int depth) {
    if (depth > 3 || repo_count >= MAX_REPOS) return;
    
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL && repo_count < MAX_REPOS) {
        if (entry->d_name[0] == '.') continue;
        
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            if (is_git_repo(full_path)) {
                Repository *repo = &repos[repo_count];
                strncpy(repo->name, entry->d_name, sizeof(repo->name) - 1);
                strncpy(repo->path, full_path, sizeof(repo->path) - 1);
                repo->is_git = 1;
                get_repo_info(repo);
                repo_count++;
            } else {
                scan_directory(full_path, depth + 1);
            }
        }
    }
    
    closedir(dir);
}

Repository* scan_github_repos(const char* root_dir, int* count) {
    repo_count = 0;
    scan_directory(root_dir, 0);
    *count = repo_count;
    return repos;
}

// Terminal control functions for native TUI
void enable_raw_mode();
void disable_raw_mode();
void clear_screen();
void print_tui_header();
void display_repos_with_highlight(int cursor_pos);

// Function declarations for interface modes
char* simple_select_repo();
char* tui_select_repo();
InterfaceMode detect_best_interface(void);
char* select_repository_interface(InterfaceMode mode);

// Terminal control functions
static struct termios old_termios, new_termios;

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

void clear_screen() {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}

void print_tui_header() {
    printf("\n");
    printf("+------------------------------------------------------+\n");
    printf("|              GITHUB SCANNER v1.2                    |\n");
    printf("|    ↑/↓ or j/k: Navigate | Enter: Sync | q: Quit    |\n");
    printf("|                    n: Rescan repositories           |\n");
    printf("+------------------------------------------------------+\n");
}

void display_repos_with_highlight(int cursor_pos) {
    clear_screen();
    print_tui_header();
    
    printf("\nFound %d Git repositories:\n\n", repo_count);
    
    for (int i = 0; i < repo_count; i++) {
        Repository *repo = &repos[i];
        
        // Show highlight for cursor position
        if (i == cursor_pos) {
            printf("\033[1;32m▶ \033[0m"); // Green pointer for highlight
        } else {
            printf("  ");
        }
        
        printf("%-30s \033[2;37m[%s]\033[0m\n", repo->name, repo->branch);
        printf("   \033[2;36m%s\033[0m\n", repo->path);
        
        if (strcmp(repo->remote, "No remote") != 0) {
            printf("   \033[2;33m%s\033[0m\n", repo->remote);
        }
        printf("\n");
    }
    
    printf("\nNavigate with ↑/↓ arrow keys\n");
    printf("Press Enter to sync highlighted repository\n");
    printf("Press 'q' to quit\n");
}

// Required function stubs
char* run_simple_interface(GitHubRepo* repos, int count);
char* run_tui_interface(GitHubRepo* repos, int count);

// Simple Interface - Basic text selection
char* simple_select_repo() {
    if (repo_count == 0) {
        return NULL;
    }
    
    printf("\nAvailable repositories:\n");
    for (int i = 0; i < repo_count; i++) {
        printf("  %d. %s (%s)\n", i + 1, repos[i].name, repos[i].is_git ? "Git" : "No Git");
    }
    
    int selection = 0;
    printf("\nSelect repository (1-%d): ", repo_count);
    if (scanf("%d", &selection) == 1 && selection >= 1 && selection <= repo_count) {
        while (getchar() != '\n');
        return strdup(repos[selection - 1].path);
    }
    
    while (getchar() != '\n');
    return NULL;
}

// TUI Interface - Native terminal implementation
char* tui_select_repo() {
    if (repo_count == 0) {
        return NULL;
    }
    
    int running = 1;
    int cursor_pos = 0;
    char* selected = NULL;
    
    // Set terminal to raw mode for immediate input
    enable_raw_mode();
    
    while (running) {
        display_repos_with_highlight(cursor_pos);
        
        int ch = getchar();
        
        // Handle arrow keys (they come as escape sequences)
        if (ch == '\033') {
            // Skip the next two characters for arrow keys
            getchar(); // Skip '['
            int arrow_key = getchar();
            
            switch(arrow_key) {
                case 'A': // Up arrow
                    if (cursor_pos > 0) cursor_pos--;
                    break;
                case 'B': // Down arrow
                    if (cursor_pos < repo_count - 1) cursor_pos++;
                    break;
            }
        } else {
            switch(ch) {
                case 'q':
                case 'Q':
                    running = 0;
                    break;
                    
                case '\n': // Enter key
                case '\r': // Carriage return
                    selected = strdup(repos[cursor_pos].path);
                    running = 0;
                    break;
                    
                case 'k': // vim-style up
                case 'K':
                    if (cursor_pos > 0) cursor_pos--;
                    break;
                    
                case 'j': // vim-style down
                case 'J':
                    if (cursor_pos < repo_count - 1) cursor_pos++;
                    break;
                    
                case 'n':
                case 'N':
                    disable_raw_mode();
                    repo_count = 0;
                    cursor_pos = 0;
                    show_loading("Rescanning for Git repositories");
                    scan_directory(".", 0);
                    enable_raw_mode();
                    break;
            }
        }
    }
    
    // Restore terminal mode
    disable_raw_mode();
    clear_screen();
    
    return selected;
}

// Required stub functions
char* run_simple_interface() {
    return simple_select_repo();
}

char* run_tui_interface() {
    return tui_select_repo();
}

// Auto-detect best interface available
InterfaceMode detect_best_interface(void) {
    if (has_fzf()) {
        return MODE_TUI;
    }
    return MODE_SIMPLE;
}

// Mode-specific repository selection
char* select_repository_interface(InterfaceMode mode) {
    switch (mode) {
        case MODE_TUI:
            return tui_select_repo();
        case MODE_SIMPLE:
            return simple_select_repo();
        case MODE_AUTO:
        default:
            return select_repository_interface(detect_best_interface());
    }
}

// Sync selected repository
void sync_repository(const char* path, CommitMode commit_mode) {
    printf("Syncing repository: %s\n", path);
    
    if (is_git_repo(path)) {
        char cmd[1024];
        int use_timestamp = 0;
        char final_commit_msg[512] = "gitsync: Manual sync";
        
        // Determine commit message based on mode
        switch (commit_mode) {
            case COMMIT_DATE:
                use_timestamp = 1;
                break;
            case COMMIT_MANUAL:
                printf("Enter commit message: ");
                if (fgets(final_commit_msg, sizeof(final_commit_msg), stdin)) {
                    final_commit_msg[strcspn(final_commit_msg, "\n")] = '\0';
                }
                break;
            case COMMIT_PROMPT:
                printf("Use timestamped commit? (y/n): ");
                char response[10];
                if (fgets(response, sizeof(response), stdin)) {
                    if (response[0] == 'y' || response[0] == 'Y') {
                        use_timestamp = 1;
                    } else {
                        printf("Enter commit message: ");
                        if (fgets(final_commit_msg, sizeof(final_commit_msg), stdin)) {
                            final_commit_msg[strcspn(final_commit_msg, "\n")] = '\0';
                        }
                    }
                }
                break;
        }
        
        // Generate timestamp if needed
        if (use_timestamp) {
            char timestamp[64];
            time_t now = time(NULL);
            struct tm* t = localtime(&now);
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);
            snprintf(final_commit_msg, sizeof(final_commit_msg), "gitsync: %s", timestamp);
        }
        
        // Fetch latest changes
        printf("Fetching changes...\n");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git fetch origin", path);
        system(cmd);
        
        // Pull changes
        printf("Pulling changes...\n");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git pull origin main", path);
        system(cmd);
        
        // Commit with appropriate message
        printf("Committing changes...\n");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git add -A && git commit -m \"%s\"", path, final_commit_msg);
        system(cmd);
        
        // Push changes
        printf("Pushing changes...\n");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git push origin main", path);
        system(cmd);
        
        printf("Repository synced successfully!\n");
    } else {
        printf("Not a git repository\n");
    }
}

// Parse command line arguments into configuration
void parse_arguments(int argc, char* argv[], ProgramConfig* config) {
    config->mode = MODE_AUTO;
    config->scan_dir = NULL;
    config->use_date_commit = 0;
    config->commit_mode = COMMIT_MANUAL;  // Default to manual for backward compatibility
    config->show_help = 0;
    config->show_version = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--date-commit") == 0) {
            config->use_date_commit = 1;
            config->commit_mode = COMMIT_DATE;  // Maintain backward compatibility
        } else if (strcmp(argv[i], "--commit-mode") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "date") == 0) {
                    config->commit_mode = COMMIT_DATE;
                } else if (strcmp(argv[i + 1], "manual") == 0) {
                    config->commit_mode = COMMIT_MANUAL;
                } else if (strcmp(argv[i + 1], "prompt") == 0) {
                    config->commit_mode = COMMIT_PROMPT;
                }
                i++; // Skip the commit mode value
            }
        } else if (strcmp(argv[i], "--interface") == 0) {
            // New --interface flag
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "auto") == 0) {
                    config->mode = MODE_AUTO;
                } else if (strcmp(argv[i + 1], "simple") == 0) {
                    config->mode = MODE_SIMPLE;
                } else if (strcmp(argv[i + 1], "tui") == 0) {
                    config->mode = MODE_TUI;
                }
                i++; // Skip the interface value
            }
        } else if (strcmp(argv[i], "--tui") == 0) {
            config->mode = MODE_TUI;
        } else if (strcmp(argv[i], "--simple") == 0) {
            config->mode = MODE_SIMPLE;
        } else if (strcmp(argv[i], "--auto") == 0) {
            config->mode = MODE_AUTO;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            config->show_help = 1;
        } else if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0) {
            config->show_version = 1;
        } else if (argv[i][0] != '-') {
            if (!config->scan_dir) {
                config->scan_dir = argv[i];
            }
        }
    }
    
    // Default to current directory if no directory specified
    if (!config->scan_dir) {
        config->scan_dir = ".";
    }
}

// Print help information
void print_help(const char* program_name) {
    printf("Usage: %s [OPTIONS] [DIRECTORY]\n", program_name);
    printf("\nInterface Modes:\n");
    printf("  --interface MODE   Set interface mode (auto|simple|tui, default: auto)\n");
    printf("  --auto             Auto-detect best interface (default)\n");
    printf("  --tui              Use TUI interface (fzf-based)\n");
    printf("  --simple           Use simple text interface\n");
    printf("\nCommit Modes:\n");
    printf("  --commit-mode MODE Set commit mode (date|manual|prompt, default: manual)\n");
    printf("  --date-commit      Use timestamped commit message (legacy, sets mode to date)\n");
    printf("\nOptions:\n");
    printf("  --help, -h         Show this help message\n");
    printf("  --version, -v      Show version\n");
    printf("\nCommit Mode Details:\n");
    printf("  date               Use \"gitsync: YYYY-MM-DD HH:MM:SS\" format\n");
    printf("  manual             Prompt for manual commit message each time\n");
    printf("  prompt             Ask to choose between date/manual each sync\n");
    printf("\nExamples:\n");
    printf("  %s                              # Auto mode, manual commits, scan current directory\n", program_name);
    printf("  %s --commit-mode date          # Use timestamped commits\n", program_name);
    printf("  %s --commit-mode manual         # Prompt for manual commit messages\n", program_name);
    printf("  %s --commit-mode prompt         # Ask to choose each time\n", program_name);
    printf("  %s --interface tui --commit-mode date # TUI mode with date commits\n", program_name);
    printf("  %s --interface simple --commit-mode manual # Simple mode with manual commits\n", program_name);
    printf("  %s --date-commit                # Legacy flag - same as --commit-mode date\n", program_name);
    printf("  %s /path/to/repos --commit-mode prompt # Specific directory, prompt mode\n", program_name);
}

// Print version information
void print_version(void) {
    printf("gitsync v1.2\n");
}

// Print interface mode information
void print_interface_info(InterfaceMode mode) {
    const char* mode_str;
    switch (mode) {
        case MODE_TUI:
            mode_str = "TUI (fzf-based)";
            break;
        case MODE_SIMPLE:
            mode_str = "Simple text";
            break;
        case MODE_AUTO:
        default:
            mode_str = "Auto-detect";
            break;
    }
    printf("Interface mode: %s\n", mode_str);
}

// Print commit mode information
void print_commit_mode_info(CommitMode mode) {
    const char* mode_str;
    switch (mode) {
        case COMMIT_DATE:
            mode_str = "Date-based (timestamped)";
            break;
        case COMMIT_MANUAL:
            mode_str = "Manual (prompt each time)";
            break;
        case COMMIT_PROMPT:
            mode_str = "Interactive (ask each time)";
            break;
        default:
            mode_str = "Unknown";
            break;
    }
    printf("Commit mode: %s\n", mode_str);
}

int main(int argc, char* argv[]) {
    ProgramConfig config;
    
    // Parse command line arguments
    parse_arguments(argc, argv, &config);
    
    // Handle version and help flags
    if (config.show_version) {
        print_version();
        return 0;
    }
    
    if (config.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    // Print startup information
    print_version();
    printf("Scanning directory: %s\n", config.scan_dir);
    print_interface_info(config.mode);
    
    print_commit_mode_info(config.commit_mode);
    
    // Scan for repositories
    scan_directory(config.scan_dir, 0);
    
    if (repo_count == 0) {
        printf("No repositories found.\n");
        return 1;
    }
    
    printf("Found %d repositories:\n", repo_count);
    for (int i = 0; i < repo_count; i++) {
        printf("  %s (%s)\n", 
               repos[i].name, 
               repos[i].is_git ? "Git" : "No Git");
    }
    
    // Repository selection using specified interface mode
    printf("\nStarting repository selection...\n");
    char* selected = select_repository_interface(config.mode);
    
    if (selected) {
        printf("\nSelected: %s\n", selected);
        sync_repository(selected, config.commit_mode);
        free(selected);
    } else {
        printf("\nNo repository selected.\n");
    }
    
    // Cleanup (not needed with static repos array)
    
    return 0;
}