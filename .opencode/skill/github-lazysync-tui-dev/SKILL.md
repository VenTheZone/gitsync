---
name: github-lazysync-tui-dev
description: Develop terminal user interface for github-lazysync including ANSI colors, keyboard input handling, and TUI layout design. Use when modifying the TUI, adding new views, or fixing display issues.
---

# GitHub-LazySync TUI Development

This Skill provides guidance for developing and maintaining the terminal user interface (TUI) in github-lazysync, including ANSI color codes, keyboard input handling, and layout design.

## Quick Start

```bash
# Test TUI
./gitsync --interface tui

# Test with specific directory
./gitsync --interface tui /path/to/repos

# Simple mode (no TUI)
./gitsync --interface simple
```

## TUI Architecture

### Interface Modes

| Mode | Flag | Description |
|------|------|-------------|
| TUI | `--interface tui` | Full terminal interface with colors and navigation |
| Simple | `--interface simple` | Basic text-based selection |
| Auto | `--interface auto` | TUI when available, simple otherwise |

### TUI Components

```
┌─────────────────────────────────────────────────────────────┐
│  GitSync v2.0                                               │
├─────────────────────────────────────────────────────────────┤
│  Filter: [type to filter...]                                │
├──────────────────┬──────────────────────────────────────────┤
│  Repositories:   │  Selected Repository:                    │
│  ▶ repo1         │  Name: repo1                             │
│    repo2         │  Path: /path/to/repo1                    │
│    repo3         │  Branch: main                            │
│                  │  Status: Has remote changes              │
├──────────────────┴──────────────────────────────────────────┤
│  ↑↓/jk: Navigate | Enter: Select | q: Quit | n: Rescan     │
└─────────────────────────────────────────────────────────────┘
```

## ANSI Color Codes

### Color Definitions

Located at the top of `src/github_scanner.c`:

```c
#define COLOR_GREEN  "\033[1;32m"   // Success, titles
#define COLOR_YELLOW "\033[1;33m"   // Warnings, indicators
#define COLOR_RED    "\033[1;31m"   // Errors
#define COLOR_BLUE   "\033[1;34m"   // Info, headers
#define COLOR_CYAN   "\033[1;36m"   // Lists, highlights
#define COLOR_WHITE  "\033[1;37m"   // Primary text
#define COLOR_RESET  "\033[0m"      // Reset to default
#define COLOR_DIM    "\033[2;37m"   // Dimmed text
#define COLOR_BOLD   "\033[1m"      // Bold text
```

### Usage Examples

```c
// Colored output
printf("%s[%s]%s Success message\n", COLOR_GREEN, "OK", COLOR_RESET);
printf("%s[%s]%s Error message\n", COLOR_RED, "ERROR", COLOR_RESET);
printf("%s[%s]%s Info message\n", COLOR_BLUE, "INFO", COLOR_RESET);

// Highlighted text
printf("%sHighlighted%s\n", COLOR_WHITE, COLOR_RESET);

// Colored indicators
printf("%s[✓]%s Clean\n", COLOR_GREEN, COLOR_RESET);      // Green check
printf("%s[+]%s Local changes\n", COLOR_YELLOW, COLOR_RESET);  // Yellow plus
printf("%s[↓]%s Remote changes\n", COLOR_CYAN, COLOR_RESET);   // Cyan down arrow
```

### Terminal Control Sequences

```c
// Clear screen
printf("\033[2J\033[H");  // Full clear and home
// OR use:
clear_screen();  // Custom function

// Move cursor
printf("\033[row;colH");  // Position cursor (1-indexed)
printf("\033[%d;1H", row);  // Move to start of row

// Save/restore cursor position
printf("\033[s");  // Save
printf("\033[u");  // Restore

// Erase to end of line
printf("\033[K");
```

## Keyboard Input Handling

### Raw Mode

The TUI uses raw terminal mode for immediate key detection:

```c
#include <termios.h>

static struct termios old_termios, new_termios;

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &old_termios);
    new_termios = old_termios;
    new_termios.c_lflag &= ~(ICANON | ECHO);  // Disable canonical mode and echo
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
}

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_termios);  // Restore original settings
}
```

**Important:** Always restore raw mode on exit:

```c
void cleanup_and_exit(void) {
    disable_raw_mode();
    clear_screen();
    exit(0);
}
```

### Key Handling

```c
int ch = getchar();

if (ch == '\033') {  // Escape sequence (arrow keys)
    getchar();  // Skip '['
    int arrow_key = getchar();
    switch(arrow_key) {
        case 'A':  // Up arrow
            if (cursor_pos > 0) cursor_pos--;
            break;
        case 'B':  // Down arrow
            if (cursor_pos < count - 1) cursor_pos++;
            break;
        case 'C':  // Right arrow
            break;
        case 'D':  // Left arrow
            break;
    }
} else if (ch == '\n' || ch == '\r') {  // Enter
    selected = current_item;
    running = 0;
} else if (ch >= 32 && ch <= 126) {  // Printable characters
    // Add to filter
    filter_text[strlen(filter_text) + 1] = '\0';
    filter_text[strlen(filter_text)] = (char)ch;
} else if (ch == 127 || ch == 8) {  // Backspace
    size_t len = strlen(filter_text);
    if (len > 0) {
        filter_text[len - 1] = '\0';
    }
} else if (ch == 'q' || ch == 'Q') {  // Quit
    running = 0;
} else if (ch == 'n' || ch == 'N') {  // Rescan
    rescan_repositories();
}
```

### Key Mappings

| Key | Action |
|-----|--------|
| ↑ / k | Move cursor up |
| ↓ / j | Move cursor down |
| → / Enter | Select item |
| ← | (Reserved) |
| Type letters | Filter list |
| Backspace / Del | Remove filter character |
| q | Quit |
| n | Rescan repositories |

## TUI Functions

### Drawing Functions

```c
// Clear screen
void clear_screen(void);

// Draw separator line
void draw_separator_line(int y, int length);

// Loading indicator
void start_loading(const char* message);
void stop_loading(void);

// Display functions
void show_success(const char* message);
void show_error(const char* message);
void show_info(const char* message);

// TUI panels
void draw_header(void);
void draw_repo_list(int cursor_pos);
void draw_selected_details(int cursor_pos);
void draw_help_bar(void);
void draw_filter_info(void);
```

### Filter System

The TUI includes real-time filtering:

```c
static char filter_text[256] = "";
static int filtered_count = 0;

void apply_filter(void) {
    filtered_count = 0;
    for (int i = 0; i < repo_count; i++) {
        if (strstr(repos[i].name, filter_text) != NULL) {
            filtered_count++;
        }
    }
}
```

## Layout Design

### Panel Positions

```c
// Header (rows 1-4)
#define HEADER_START_Y 1
#define HEADER_HEIGHT 4

// Repository list (rows 5+)
#define LIST_START_Y 5

// Selected details (rows 8+)
#define DETAILS_START_Y 8

// Help bar (bottom)
#define HELP_START_Y 24
```

### Drawing Pattern

```c
void draw_ui(int cursor_pos) {
    clear_screen();
    
    draw_header();
    draw_filter_info();
    draw_repo_list(cursor_pos);
    draw_selected_details(cursor_pos);
    draw_help_bar();
    
    fflush(stdout);  // Ensure display updates
}
```

## Testing TUI

### Manual Testing

```bash
# Test on different terminal sizes
./gitsync --interface tui

# Test navigation
# Type letters to filter
# Use arrow keys to navigate
# Press Enter to select
# Press 'q' to quit
# Press 'n' to rescan
```

### Automated Testing

```bash
# Test simple interface (non-interactive)
echo -e "1\nq" | ./gitsync --interface simple

# Test help
./gitsync --help | grep -i tui
```

## Common TUI Issues

### Garbled Display

**Cause:** Incorrect ANSI codes or cursor positioning  
**Fix:** Ensure proper use of COLOR_RESET after colored text

```c
// BEFORE: Missing reset
printf("%sError:%s Something failed\n", COLOR_RED, "");  // Missing COLOR_RESET

// AFTER: Proper reset
printf("%sError:%s Something failed%s\n", COLOR_RED, "ERROR", COLOR_RESET);
```

### Keyboard Input Not Responding

**Cause:** Raw mode not properly enabled/disabled  
**Fix:** Always restore terminal state:

```c
// Use atexit for safety
atexit(disable_raw_mode);

// Or use try-finally pattern
enable_raw_mode();
if (some_error) {
    disable_raw_mode();
    return error;
}
disable_raw_mode();
```

### Filter Not Working

**Cause:** Filter applied before data loaded  
**Fix:** Apply filter after repository scan completes

```c
// Correct order:
scan_repositories();      // Load data first
apply_filter();           // Then apply filter
draw_ui(cursor_pos);      // Finally draw
```

## TUI Best Practices

1. **Always restore terminal**: Use disable_raw_mode() on exit
2. **Use COLOR_RESET**: Reset colors after each colored output
3. **Test on multiple terminals**: Try xterm, gnome-terminal, tmux
4. **Handle resize**: Clear and redraw on screen changes
5. **Minimize flickering**: Use cursor positioning instead of full redraws
6. **Provide feedback**: Show loading states and progress
7. **Help text**: Display keyboard shortcuts prominently

## References

- [Lessons Learned](/home/ubuntu/Projects/github-lazysync/.opencode/memory/lessons_learned.md)
- Activity Log: [activity_log.md](/home/ubuntu/Projects/github-lazysync/.opencode/memory/activity_log.md)
- ANSI escape codes: See source code color definitions
