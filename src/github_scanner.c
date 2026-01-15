#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>
#include <libgen.h>

#define MAX_REPOS 100
#define MAX_PATH_LEN 1024

#define COLOR_GREEN  "\033[1;32m"
#define COLOR_YELLOW "\033[1;33m"
#define COLOR_RED    "\033[1;31m"
#define COLOR_BLUE   "\033[1;34m"
#define COLOR_CYAN   "\033[1;36m"
#define COLOR_WHITE  "\033[1;37m"
#define COLOR_RESET  "\033[0m"
#define COLOR_DIM    "\033[2;37m"
#define COLOR_BOLD   "\033[1m"

typedef struct {
    char name[256];
    char path[MAX_PATH_LEN];
    char remote[512];
    char branch[64];
    int is_git;
    int has_local_changes;
    int has_remote_changes;
    int sync_status;
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

typedef enum {
    SYNC_IDLE,
    SYNC_SCANNING,
    SYNC_FETCHING,
    SYNC_PULLING,
    SYNC_COMMITTING,
    SYNC_PUSHING,
    SYNC_DONE,
    SYNC_ERROR
} SyncState;

typedef struct {
    InterfaceMode mode;
    const char* scan_dir;
    int use_date_commit;
    CommitMode commit_mode;
    int show_help;
    int show_version;
} ProgramConfig;

Repository repos[MAX_REPOS];
static int repo_count = 0;
static int loading_active = 0;

static char filter_text[256] = "";
static int filtered_count = 0;

void show_error(const char* message);
void show_warning(const char* message);
void show_success(const char* message);
void show_info(const char* message);
void start_loading(const char* message);
void stop_loading(void);

static void clear_screen(void) {
    printf("\033[2J\033[1;1H");
    fflush(stdout);
}

static void draw_separator_line(int y, int length) {
    printf("\033[%d;1H", y);
    for (int i = 0; i < length; i++) printf("─");
    printf("\n");
}

void show_error(const char* message) {
    printf("%s[%s]%s %s\n", COLOR_RED, "ERROR", COLOR_RESET, message);
    fflush(stdout);
}

void show_warning(const char* message) {
    printf("%s[%s]%s %s\n", COLOR_YELLOW, "WARN", COLOR_RESET, message);
    fflush(stdout);
}

void show_success(const char* message) {
    printf("%s[%s]%s %s\n", COLOR_GREEN, "OK", COLOR_RESET, message);
    fflush(stdout);
}

void show_info(const char* message) {
    printf("%s[%s]%s %s\n", COLOR_BLUE, "INFO", COLOR_RESET, message);
    fflush(stdout);
}

void start_loading(const char* message) {
    loading_active = 1;
    printf("%s[%s]%s %s ", COLOR_YELLOW, "LOADING", COLOR_RESET, message);
    fflush(stdout);
}

void stop_loading(void) {
    if (loading_active) {
        printf("\b \b");
        fflush(stdout);
        loading_active = 0;
    }
}

static int is_git_repo(const char* path) {
    char git_path[1024];
    struct stat st;
    snprintf(git_path, sizeof(git_path), "%s/.git", path);
    return stat(git_path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int has_local_changes(const char *path) {
    char cmd[MAX_PATH_LEN * 2];
    char buffer[256];
    int has_changes = 0;
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git status --porcelain 2>/dev/null | head -1", path);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    if (fgets(buffer, sizeof(buffer), fp)) {
        has_changes = 1;
    }
    pclose(fp);
    return has_changes;
}

static int has_remote_changes(const char *path) {
    char cmd[MAX_PATH_LEN * 2];
    char buffer[256];
    int has_changes = 0;
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git fetch origin main 2>/dev/null && git log HEAD..origin/main --oneline 2>/dev/null | head -1", path);
    FILE *fp = popen(cmd, "r");
    if (!fp) return 0;
    
    if (fgets(buffer, sizeof(buffer), fp)) {
        has_changes = 1;
    }
    pclose(fp);
    return has_changes;
}

static void get_branch_name(const char *path, char *branch, size_t branch_size) {
    char cmd[MAX_PATH_LEN * 2];
    FILE *fp;
    
    strcpy(branch, "unknown");
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git branch --show-current 2>/dev/null || echo 'unknown'", path);
    fp = popen(cmd, "r");
    if (fp) {
        if (fgets(branch, branch_size, fp)) {
            branch[strcspn(branch, "\n")] = '\0';
        }
        pclose(fp);
    }
}

static void get_remote_url(const char *path, char *remote, size_t remote_size) {
    char cmd[MAX_PATH_LEN * 2];
    FILE *fp;
    
    strcpy(remote, "No remote");
    
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git remote get-url origin 2>/dev/null || echo 'No remote'", path);
    fp = popen(cmd, "r");
    if (fp) {
        if (fgets(remote, remote_size, fp)) {
            remote[strcspn(remote, "\n")] = '\0';
        }
        pclose(fp);
    }
}

static void get_repo_info(Repository* repo) {
    get_branch_name(repo->path, repo->branch, sizeof(repo->branch));
    get_remote_url(repo->path, repo->remote, sizeof(repo->remote));
    repo->has_local_changes = has_local_changes(repo->path);
    repo->has_remote_changes = has_remote_changes(repo->path);
    repo->sync_status = SYNC_IDLE;
}

static int is_excluded_path(const char* path) {
    const char* exclude_patterns[] = {
        "/.git/", "/.oh-my-zsh/", "/node_modules/", "/.cache/",
        "/.local/share/", "/.config/", "/.mozilla/", "/.thumbnails/",
        NULL
    };
    
    for (int i = 0; exclude_patterns[i] != NULL; i++) {
        if (strstr(path, exclude_patterns[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

static void add_repo_from_path(const char* full_path) {
    if (repo_count >= MAX_REPOS) return;
    
    if (is_excluded_path(full_path)) return;
    
    struct stat st;
    if (stat(full_path, &st) != 0 || !S_ISDIR(st.st_mode)) return;
    
    char git_path[MAX_PATH_LEN];
    snprintf(git_path, sizeof(git_path), "%s/.git", full_path);
    
    if (stat(git_path, &st) != 0 || !S_ISDIR(st.st_mode)) return;
    
    Repository *repo = &repos[repo_count];
    repo->is_git = 1;
    
    const char* dir_name = strrchr(full_path, '/');
    if (dir_name) dir_name++;
    else dir_name = full_path;
    
    strncpy(repo->name, dir_name, sizeof(repo->name) - 1);
    repo->name[sizeof(repo->name) - 1] = '\0';
    strncpy(repo->path, full_path, sizeof(repo->path) - 1);
    repo->path[sizeof(repo->path) - 1] = '\0';
    
    get_repo_info(repo);
    repo_count++;
}

static void scan_system_for_repos(void) {
    repo_count = 0;
    
    char cmd[MAX_PATH_LEN * 4];
    snprintf(cmd, sizeof(cmd),
        "find /home /opt /usr/local -type d -name '.git' 2>/dev/null | "
        "while read -r gitpath; do dirname \"$gitpath\"; done | sort -u");
    
    FILE* fp = popen(cmd, "r");
    if (!fp) return;
    
    char line[MAX_PATH_LEN];
    while (fgets(line, sizeof(line), fp) && repo_count < MAX_REPOS) {
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) > 1) {
            add_repo_from_path(line);
        }
    }
    pclose(fp);
}

static void scan_directory(const char *path, int depth) {
    if (depth > 5) return; // Prevent infinite recursion
    
    // Check if current path is a git repository
    if (is_git_repo(path)) {
        if (repo_count < MAX_REPOS) {
            // Extract repo name from path
            const char *base = strrchr(path, '/');
            if (base) {
                base++; // Skip the '/'
            } else {
                base = path;
            }
            
            strncpy(repos[repo_count].name, base, sizeof(repos[repo_count].name) - 1);
            strncpy(repos[repo_count].path, path, sizeof(repos[repo_count].path) - 1);
            repos[repo_count].name[sizeof(repos[repo_count].name) - 1] = '\0';
            repos[repo_count].path[sizeof(repos[repo_count].path) - 1] = '\0';
            
            repos[repo_count].has_local_changes = has_local_changes(path);
            repos[repo_count].has_remote_changes = has_remote_changes(path);
            get_branch_name(path, repos[repo_count].branch, sizeof(repos[repo_count].branch));
            
            repo_count++;
        }
        return; // Don't scan inside a git repo
    }
    
    DIR *dir = opendir(path);
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue; // Skip hidden files/dirs
        
        char full_path[2048];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        if (entry->d_type == DT_DIR) {
            scan_directory(full_path, depth + 1);
        }
    }
    closedir(dir);
}

Repository* scan_github_repos(const char* root_dir, int* count) {
    repo_count = 0;
    
    printf("\n");
    start_loading("Scanning for Git repositories");
    
    if (strcmp(root_dir, ".") == 0 || root_dir[0] == '\0') {
        scan_system_for_repos();
    } else {
        // Scan specific directory 
        scan_directory(root_dir, 0);
    }
    
    stop_loading();
    printf(" done\n");
    
    if (repo_count > 0) {
        printf("%s[%s]%s Found %d repository %s\n", COLOR_GREEN, "OK", COLOR_RESET, repo_count, repo_count == 1 ? "" : "s");
    } else {
        printf("%s[%s]%s No repositories found\n", COLOR_YELLOW, "WARN", COLOR_RESET);
    }
    
    *count = repo_count;
    return repos;
}

static struct termios old_termios, new_termios;

static void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

static void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);
}

static void draw_header(void) {
    clear_screen();
    
    // Title
    printf("%s  GitSync v2.0  %s\n\n", COLOR_GREEN, COLOR_RESET);
    
    // Repository count
    printf("%sFound %d repositories%s\n\n", COLOR_BLUE, repo_count, repo_count == 1 ? "" : "ies");
    
    // Separator
    draw_separator_line(4, 80);
}

static void draw_repo_list(int cursor_pos) {
    int start_y = 5;
    
    printf("\033[%d;1H", start_y);
    printf("%sRepositories:%s\n", COLOR_CYAN, COLOR_RESET);
    
    for (int i = 0; i < repo_count; i++) {
        printf("\033[%d;3H", start_y + 1 + i);
        
        if (i == cursor_pos) {
            printf("%s▶ %s%s", COLOR_GREEN, COLOR_WHITE, repos[i].name);
        } else {
            printf("  %s", repos[i].name);
        }
        
        // Status indicators
        if (repos[i].has_local_changes) {
            printf("%s [+]%s", COLOR_YELLOW, COLOR_RESET);
        }
        if (repos[i].has_remote_changes) {
            printf("%s [↓]%s", COLOR_CYAN, COLOR_RESET);
        }
        if (!repos[i].has_local_changes && !repos[i].has_remote_changes) {
            printf("%s [✓]%s", COLOR_GREEN, COLOR_RESET);
        }
    }
}

static void draw_selected_details(int cursor_pos) {
    if (cursor_pos < 0 || cursor_pos >= repo_count) return;
    
    Repository* repo = &repos[cursor_pos];
    int details_start = 5 + repo_count + 2;
    
    printf("\033[%d;1H", details_start);
    printf("\n%sSelected Repository:%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  Name: %s%s%s\n", COLOR_WHITE, repo->name, COLOR_RESET);
    printf("  Path: %s%s%s\n", COLOR_WHITE, repo->path, COLOR_RESET);
    printf("  Branch: %s%s%s\n", COLOR_WHITE, repo->branch, COLOR_RESET);
    printf("  Remote: %s%s%s\n", COLOR_WHITE, repo->remote, COLOR_RESET);
    
    // Status
    printf("  Status: ");
    if (repo->has_local_changes) {
        printf("%sHas local changes%s", COLOR_YELLOW, COLOR_RESET);
    }
    if (repo->has_remote_changes) {
        printf("%sHas remote changes%s", COLOR_CYAN, COLOR_RESET);
    }
    if (!repo->has_local_changes && !repo->has_remote_changes) {
        printf("%sClean%s", COLOR_GREEN, COLOR_RESET);
    }
    printf("\n");
}

static void draw_filter_info(void) {
    printf("\033[1;1H");
    printf("Filter: %s%s%s", COLOR_GREEN, filter_text, COLOR_RESET);
    if (strlen(filter_text) > 0) {
        printf(" (%d matches)", filtered_count);
    }
    printf("\n");
}

static void draw_help_bar(void) {
    printf("\033[24;1H");
    printf("%sNavigation: ↑↓/jk  | Select: Enter | Filter: type letters | Clear: Backspace | Quit: q | Rescan: n%s", COLOR_DIM, COLOR_RESET);
    printf("\033[K"); // Clear rest of line
}

static int get_filtered_count(void) {
    if (strlen(filter_text) == 0) return repo_count;
    
    int count = 0;
    for (int i = 0; i < repo_count; i++) {
        if (strstr(repos[i].name, filter_text) != NULL) {
            count++;
        }
    }
    return count;
}

static int get_filtered_index(int visible_index) {
    if (strlen(filter_text) == 0) return visible_index;
    
    int count = 0;
    for (int i = 0; i < repo_count; i++) {
        if (strstr(repos[i].name, filter_text) != NULL) {
            if (count == visible_index) return i;
            count++;
        }
    }
    return 0;
}



static char* tui_select_repo(const char* scan_dir, CommitMode commit_mode) {
    (void)commit_mode;
    if (repo_count == 0) {
        return NULL;
    }
    
    printf("\nPress any key to start...");
    getchar();
    
    int running = 1;
    int cursor_pos = 0;
    char* selected = NULL;
    
    enable_raw_mode();
    
    while (running) {
        filtered_count = get_filtered_count();
        
        draw_header();
        draw_filter_info();
        draw_repo_list(cursor_pos);
        draw_selected_details(cursor_pos);
        draw_help_bar();
        
        int ch = getchar();
        
        if (ch == '\033') {
            getchar(); // Skip [
            int arrow_key = getchar();
            switch(arrow_key) {
                case 'A':
                    if (cursor_pos > 0) cursor_pos--;
                    break;
                case 'B':
                    if (cursor_pos < filtered_count - 1) cursor_pos++;
                    break;
            }
        } else if (ch >= 32 && ch <= 126) { // Printable characters
            if (ch == 'q' || ch == 'Q') {
                running = 0;
            } else if (ch == '\n' || ch == '\r') {
                if (filtered_count > 0) {
                    int actual_idx = get_filtered_index(cursor_pos);
                    selected = strdup(repos[actual_idx].name);
                }
                running = 0;
            } else if (ch == 127 || ch == 8) { // Backspace - need to check for both
                size_t len = strlen(filter_text);
                if (len > 0) {
                    filter_text[len - 1] = '\0';
                    cursor_pos = 0;
                }
            } else if (ch == 'n' || ch == 'N') {
                disable_raw_mode();
                printf("\n");
                printf("Rescanning repositories...\n");
                scan_github_repos(scan_dir, &repo_count);
                enable_raw_mode();
                cursor_pos = 0;
            } else {
                // Add to filter
                size_t len = strlen(filter_text);
                if (len < 255) {
                    filter_text[len] = (char)ch;
                    filter_text[len + 1] = '\0';
                    cursor_pos = 0;
                }
            }
        } else if (ch == 127 || ch == 8) { // Backspace - also handle here for safety
            size_t len = strlen(filter_text);
            if (len > 0) {
                filter_text[len - 1] = '\0';
                cursor_pos = 0;
            }
        }
    }
    
    disable_raw_mode();
    clear_screen();
    
    return selected;
}

static char* simple_select_repo(void) {
    if (repo_count == 0) {
        return NULL;
    }
    
    printf("\n%s[%s]%s Available repositories:\n", COLOR_BLUE, "LIST", COLOR_RESET);
    for (int i = 0; i < repo_count; i++) {
        char status[32] = "";
        if (repos[i].has_local_changes) strcat(status, "[+] ");
        if (repos[i].has_remote_changes) strcat(status, "[↓] ");
        if (!repos[i].has_local_changes && !repos[i].has_remote_changes) strcat(status, "[✓] ");
        
        printf("  %s%d.%s %s%s %s(%s)%s\n", COLOR_YELLOW, i + 1, COLOR_RESET, 
               COLOR_WHITE, repos[i].name, COLOR_BLUE, status, COLOR_RESET);
        printf("      %s%s%s\n", COLOR_DIM, repos[i].path, COLOR_RESET);
    }
    
    int selection = 0;
    printf("\n%s[%s]%s Select repository (1-%d): ", COLOR_BLUE, "INPUT", COLOR_RESET, repo_count);
    if (scanf("%d", &selection) == 1 && selection >= 1 && selection <= repo_count) {
        while (getchar() != '\n');
        return strdup(repos[selection - 1].path);
    }
    
    while (getchar() != '\n');
    return NULL;
}

static InterfaceMode detect_best_interface(void) {
    return MODE_TUI;
}



static void scan_system_for_repos(void);
static void sync_repository(const char* path, CommitMode commit_mode);
static char* select_repository_interface(InterfaceMode mode, const char* scan_dir, CommitMode commit_mode);
static InterfaceMode detect_best_interface(void);

static char* select_repository_interface(InterfaceMode mode, const char* scan_dir, CommitMode commit_mode) {
    char* selected = NULL;
    
    switch (mode) {
        case MODE_TUI:
            selected = tui_select_repo(scan_dir, commit_mode);
            break;
        case MODE_SIMPLE:
            selected = simple_select_repo();
            break;
        case MODE_AUTO:
        default:
            selected = select_repository_interface(detect_best_interface(), scan_dir, commit_mode);
            break;
    }
    
    // If a repository was selected, sync it
    if (selected) {
        // Find the actual repository path from the name
        for (int i = 0; i < repo_count; i++) {
            if (strcmp(repos[i].name, selected) == 0) {
                sync_repository(repos[i].path, commit_mode);
                free(selected);
                selected = NULL;
                break;
            }
        }
    }
    
    return selected;
}

static void sync_repository(const char* path, CommitMode commit_mode) {
    (void)commit_mode;
    
    /*
     * Simple sync workflow inspired by Obsidian-GitHub-Sync plugin:
     * 1. Pull remote changes first (handles conflicts upfront)
     * 2. Stage all local changes
     * 3. Commit with auto-timestamp
     * 4. Push to remote
     * 
     * Philosophy: Keep it simple, no branching, user resolves conflicts manually
     */
    
    printf("\n");
    printf("%s[%s]%s Syncing vault: %s%s%s\n", COLOR_BLUE, "SYNC", COLOR_RESET, COLOR_WHITE, path, COLOR_RESET);
    
    if (!is_git_repo(path)) {
        printf("%s[%s]%s Not a git repository%s\n", COLOR_RED, "ERROR", COLOR_RESET, "");
        return;
    }
    
    char cmd[2048];
    char final_commit_msg[512];
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    strftime(final_commit_msg, sizeof(final_commit_msg), "GitSync: %Y-%m-%d %H:%M", t);
    
    // Check for changes first
    printf("\n");
    start_loading("Checking for changes...");
    int local_changes = has_local_changes(path);
    int remote_changes = has_remote_changes(path);
    stop_loading();
    
    if (!local_changes && !remote_changes) {
        printf("%s[%s]%s Vault is up to date\n", COLOR_GREEN, "OK", COLOR_RESET);
        return;
    }
    
    // Step 1: Pull remote changes first (like Obsidian-GitHub-Sync)
    printf("\n");
    start_loading("Pulling remote changes...");
    snprintf(cmd, sizeof(cmd), "cd \"%s\" && git pull origin main 2>&1", path);
    int pull_result = system(cmd);
    stop_loading();
    
    if (pull_result != 0) {
        printf("\n%s[%s]%s Pull failed - merge conflicts detected\n", COLOR_RED, "CONFLICT", COLOR_RESET);
        printf("%s[%s]%s Resolve conflicts in your editor, then run sync again\n", COLOR_YELLOW, "HINT", COLOR_RESET);
        return;
    }
    printf("%s[%s]%s Pulled remote changes\n", COLOR_GREEN, "OK", COLOR_RESET);
    
    // Step 2: Stage and commit local changes
    if (local_changes) {
        printf("\n");
        start_loading("Staging and committing local changes...");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git add -A && git commit -m \"%s\" 2>&1", path, final_commit_msg);
        int commit_result = system(cmd);
        stop_loading();
        
        if (commit_result == 0) {
            printf(" done\n");
            show_success("Changes committed");
        } else {
            printf(" failed\n");
            show_error("Nothing to commit or commit failed");
        }
    }
    
    // Step 3: Push changes
    if (local_changes || remote_changes) {
        printf("\n");
        start_loading("Pushing to remote...");
        snprintf(cmd, sizeof(cmd), "cd \"%s\" && git push origin main 2>&1", path);
        int push_result = system(cmd);
        stop_loading();
        
        if (push_result == 0) {
            printf(" done\n");
            show_success("Sync complete!");
        } else {
            printf(" failed\n");
            show_error("Push failed - may need to pull again");
        }
    } else {
        printf("\n%s[%s]%s Sync complete\n", COLOR_GREEN, "OK", COLOR_RESET);
    }
}

static void parse_arguments(int argc, char* argv[], ProgramConfig* config) {
    config->mode = MODE_AUTO;
    config->scan_dir = NULL;
    config->use_date_commit = 0;
    config->commit_mode = COMMIT_MANUAL;
    config->show_help = 0;
    config->show_version = 0;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--date-commit") == 0) {
            config->use_date_commit = 1;
            config->commit_mode = COMMIT_DATE;
        } else if (strcmp(argv[i], "--commit-mode") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "date") == 0) {
                    config->commit_mode = COMMIT_DATE;
                } else if (strcmp(argv[i + 1], "manual") == 0) {
                    config->commit_mode = COMMIT_MANUAL;
                } else if (strcmp(argv[i + 1], "prompt") == 0) {
                    config->commit_mode = COMMIT_PROMPT;
                }
                i++;
            }
        } else if (strcmp(argv[i], "--interface") == 0) {
            if (i + 1 < argc) {
                if (strcmp(argv[i + 1], "auto") == 0) {
                    config->mode = MODE_TUI;
                } else if (strcmp(argv[i + 1], "simple") == 0) {
                    config->mode = MODE_SIMPLE;
                } else if (strcmp(argv[i + 1], "tui") == 0) {
                    config->mode = MODE_TUI;
                }
                i++;
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
    
    if (!config->scan_dir) {
        config->scan_dir = "";
    }
}

static void print_help(const char* program_name) {
    printf("\n");
    printf("%s  GitSync v2.0  %s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n");
    
    printf("%sUsage:%s %s [OPTIONS] [DIRECTORY]%s\n", COLOR_WHITE, COLOR_RESET, program_name, COLOR_RESET);
    printf("\n");
    
    printf("%sOptions:%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s--help, -h%s         Show this help information\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s--version, -v%s      Show version\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s--interface MODE%s    Interface mode: auto, simple, tui (default: auto)\n", COLOR_CYAN, COLOR_RESET);
    printf("  %s--commit-mode MODE%s  Commit mode: date, manual, prompt (default: manual)\n", COLOR_CYAN, COLOR_RESET);
    printf("\n");
    
    printf("%sTUI Controls:%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s↑↓/jk%s              Navigate repository list\n", COLOR_CYAN, COLOR_RESET);
    printf("  %senter%s              Select repository\n", COLOR_CYAN, COLOR_RESET);
    printf("  %sType letters%s        Filter repositories\n", COLOR_CYAN, COLOR_RESET);
    printf("  %sbackspace%s          Clear filter character\n", COLOR_CYAN, COLOR_RESET);
    printf("  %sq%s                   Quit\n", COLOR_CYAN, COLOR_RESET);
    printf("  %sn%s                   Rescan repositories\n", COLOR_CYAN, COLOR_RESET);
    printf("\n");
    
    printf("%sRepository Indicators:%s\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s[✓]%s Clean - no changes\n", COLOR_GREEN, COLOR_RESET);
    printf("  %s[+]%s  Has local changes\n", COLOR_YELLOW, COLOR_RESET);
    printf("  %s[↓]%s  Has remote changes\n", COLOR_CYAN, COLOR_RESET);
    printf("\n");
}

static void print_version(void) {
    printf("%s  GitSync v2.0  %s\n", COLOR_GREEN, COLOR_RESET);
}

int main(int argc, char* argv[]) {
    ProgramConfig config;
    
    parse_arguments(argc, argv, &config);
    
    if (config.show_version) {
        print_version();
        return 0;
    }
    
    if (config.show_help) {
        print_help(argv[0]);
        return 0;
    }
    
    printf("\n");
    printf("%s  GitSync v2.0  %s\n", COLOR_GREEN, COLOR_RESET);
    printf("\n");
    
    const char* dir_display = config.scan_dir[0] ? config.scan_dir : "(system-wide)";
    printf("%s[%s]%s Directory: %s%s%s\n", COLOR_BLUE, "INFO", COLOR_RESET, COLOR_WHITE, dir_display, COLOR_RESET);
    
    int repo_count_temp = 0;
    scan_github_repos(config.scan_dir, &repo_count_temp);
    repo_count = repo_count_temp;
    
    if (repo_count == 0) {
        printf("\n");
        show_warning("No repositories found.");
        show_info("Scanned /home, /opt, /usr/local for .git directories.");
        return 1;
    }
    
    char* selected = select_repository_interface(config.mode, config.scan_dir, config.commit_mode);
    
    if (selected) {
        printf("\n");
        show_info("Selected repository:");
        printf("  %s%s%s\n", COLOR_WHITE, selected, COLOR_RESET);
        sync_repository(selected, config.commit_mode);
        free(selected);
    } else {
        printf("\n");
        show_info("No repository selected.");
    }
    
    return 0;
}