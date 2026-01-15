# GITSYNC v2.0 - GitHub Repository Sync Tool

A C-based terminal tool for smart Git repository synchronization with modern TUI interface.

## Quick Install

```bash
# Linux/macOS (one-liner)
curl -LO https://github.com/VenTheZone/gitsync/releases/download/v2.0.0/install.sh && chmod +x install.sh && ./install.sh
```

Or compile from source:

```bash
# Clone and build
git clone https://github.com/VenTheZone/gitsync.git
cd gitsync
make

# Install
sudo make install
```

Or download manually: https://github.com/VenTheZone/gitsync/releases/tag/v2.0.0

## Requirements

- GCC or Clang compiler
- Git
- POSIX-compatible system (Linux, macOS)

## Quick Start

```bash
# Sync your Obsidian vault
gitsync /path/to/Obsidian-Notes/

# With TUI (default)
gitsync --interface tui /path/to/vault/

# With auto-timestamped commits (recommended)
gitsync --commit-mode date /path/to/vault/
```

## Features

### System-Wide Repository Discovery
- Automatically scans `/home`, `/opt`, `/usr/local` for Git repositories
- Excludes common non-project directories (.git, .oh-my-zsh, node_modules, etc.)
- Displays repository count and status indicators

### Interface Modes
- **TUI Mode** (default): Native terminal interface with real-time filtering and 3-panel layout
- **Simple Mode**: Text-based interactive selection
- **Auto Mode**: Automatically selects TUI when available

### Smart Sync Operations
- **Change Detection**: Automatically detects local and remote changes
- **Status Indicators**: [✓] clean, [+] modified, [↓] remote changes
- **Simple Workflow**: Pull → Stage → Commit → Push
- **Conflict Handling**: Detects conflicts and guides user to resolve manually

### TUI Design
- **Clean Layout**: Header → Filter bar → Repository list → Selected details → Help bar
- **Color Coding**: Semantic colors for status, headers, and indicators
- **Help Bar**: Contextual keyboard shortcuts displayed at bottom
- **Real-time Filtering**: Type to filter repository list

### Commit Modes
- **Date Mode** (`--commit-mode date`): Automatic timestamped commits
- **Manual Mode** (`--commit-mode manual`): Prompt for commit message
- **Prompt Mode** (`--commit-mode prompt`): Ask each time

## Build

```bash
# Simple compilation
gcc -Wall -Wextra -std=c99 -D_GNU_SOURCE -o gitsync src/github_scanner.c

# Using Make (auto-detects clang or gcc)
make

# With optimizations
make optimized

# Static build
make static-build

# Debug build with address sanitizer
make debug
```

## Installation

```bash
# Install to /usr/local/bin
sudo make install

# Custom install prefix
make install PREFIX=$HOME/.local
```

## Usage

```bash
# Scan all repositories system-wide (default behavior)
./gitsync

# Scan specific directory
./gitsync /path/to/repos

# TUI mode with date commits
./gitsync --commit-mode date

# Simple text interface
./gitsync --interface simple

# Show help
./gitsync --help

# Show version
./gitsync --version
```

## TUI Controls

| Key | Action |
|-----|--------|
| Type any character | Filter repositories in real-time |
| ↑/↓ or j/k | Navigate through list |
| → or Enter | Select repository |
| / | Start filtering |
| Backspace | Remove filter character |
| Ctrl+U | Clear filter completely |
| n | Rescan repositories |
| q | Quit |

## Repository Indicators

| Indicator | Meaning |
|-----------|---------|
| `[✓]` | Clean - no changes |
| `[+]` | Has local uncommitted changes |
| `[↓]` | Has remote changes to pull |

## Static Analysis & Testing

```bash
# Run clang-tidy
make clang-tidy

# Run cppcheck
make cppcheck

# Run both
make analyze

# Format code
make format
```

## Troubleshooting

**"Permission denied" when running gitsync:**
```bash
chmod +x gitsync
```

**"git: command not found":**
Install git first: https://git-scm.com/downloads

**TUI显示乱码:**
Ensure terminal supports ANSI colors. Try simple mode:
```bash
gitsync --interface simple /path/to/vault/
```

**Sync显示"No repositories found":**
Ensure the directory is a git repository:
```bash
cd /path/to/vault/
git status
```

## Project Structure

```
github-lazysync/
├── src/
│   └── github_scanner.c    # Main source file (single-file implementation)
├── gitsync                 # Compiled binary
├── Makefile                # Build system with targets
├── README.md               # This file
└── AGENTS.md               # Evolve orchestrator configuration
```

## Requirements

- GCC or Clang compiler
- POSIX-compatible system (Linux, macOS)
- Git installed

## For Developers

```bash
# Clone and build from source
git clone https://github.com/VenTheZone/gitsync.git
cd gitsync
make

# Run tests
./gitsync --help
```

## Version

Current version: 2.0

## License

Open source project for Git repository management.
