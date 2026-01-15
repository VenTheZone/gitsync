---
name: github-lazysync-usage
description: Use github-lazysync to sync Obsidian vaults and Git repositories across devices. Use when user wants to sync notes, pull changes, push updates, or manage vault synchronization via terminal.
---

# GitHub-LazySync Usage Guide

This Skill provides comprehensive guidance for using github-lazysync to synchronize Obsidian vaults and Git repositories across multiple devices via terminal.

## What is GitHub-LazySync?

GitHub-LazySync is a terminal-based tool that:
- **Scans** your system for Git repositories
- **Detects** local and remote changes
- **Syncs** your Obsidian vault across devices
- **Provides** a simple TUI for repository selection

Inspired by the Obsidian-GitHub-Sync plugin, but designed for terminal users who prefer command-line workflows.

## Quick Start

### Basic Sync

```bash
# Sync your Obsidian vault
./gitsync /path/to/Obsidian-Notes/

# Or scan all repositories
./gitsync
```

### First Time Setup

```bash
# 1. Navigate to your Obsidian vault
cd /path/to/Obsidian-Notes/

# 2. Initialize git (if not already done)
git init
git add .
git commit -m "Initial commit"

# 3. Add remote (GitHub)
git remote add origin https://github.com/yourusername/your-repo.git

# 4. Push to GitHub
git push -u origin main

# 5. Now sync with github-lazysync
./gitsync /path/to/Obsidian-Notes/
```

## Installation

```bash
# Build from source
cd github-lazysync
make

# Install to system
sudo make install

# Or use directly
./gitsync [options]
```

## Command Line Options

### Basic Options

| Option | Description |
|--------|-------------|
| `--help, -h` | Show help information |
| `--version, -v` | Show version |
| `[DIRECTORY]` | Directory to scan (default: current directory) |

### Interface Options

| Option | Description |
|--------|-------------|
| `--interface auto` | Auto-detect best interface (default) |
| `--interface tui` | Full terminal interface with colors and navigation |
| `--interface simple` | Simple text-based selection |

### Commit Options

| Option | Description |
|--------|-------------|
| `--commit-mode date` | Automatic timestamped commits (recommended) |
| `--commit-mode manual` | Prompt for commit message each time |
| `--commit-mode prompt` | Ask whether to use timestamp or custom message |

## Usage Examples

### Sync Your Obsidian Vault

```bash
# With TUI (recommended for first time)
./gitsync /path/to/Obsidian-Notes/

# With simple interface
./gitsync --interface simple /path/to/Obsidian-Notes/

# With automatic date commits
./gitsync --commit-mode date /path/to/Obsidian-Notes/
```

### Scan All Repositories

```bash
# Scan home directory
./gitsync /home/yourusername/

# Scan specific directory
./gitsync /opt/projects/
```

### Batch Sync

```bash
# Sync multiple vaults
for vault in ~/Obsidian-Notes ~/Work-Notes; do
    ./gitsync --commit-mode date "$vault"
done
```

## TUI Controls

When using the TUI interface:

| Key | Action |
|-----|--------|
| ↑ / ↓ or j / k | Navigate repository list |
| → / Enter | Select repository to sync |
| Type letters | Filter repositories by name |
| Backspace | Remove filter character |
| q | Quit |
| n | Rescan repositories |

### Repository Indicators

| Indicator | Meaning |
|-----------|---------|
| `[✓]` | Clean - no changes to sync |
| `[+]` | Has local uncommitted changes |
| `[↓]` | Has remote changes to pull |

## Sync Workflow

### What Happens During Sync

1. **Pull**: Fetches and merges remote changes
2. **Stage**: Adds all local changes
3. **Commit**: Creates commit with timestamp
4. **Push**: Sends changes to remote

```
┌─────────────────────────────────────┐
│  Step 1: Pull remote changes        │
│  Step 2: Stage local changes        │
│  Step 3: Commit with timestamp      │
│  Step 4: Push to remote             │
└─────────────────────────────────────┘
```

### Conflict Resolution

If you have merge conflicts:

1. **Pull fails**: Sync shows conflict message
2. **Resolve in editor**: Open vault in Obsidian or VS Code
3. **Fix conflicts**: Accept changes or resolve manually
4. **Run sync again**: `./gitsync /path/to/Obsidian-Notes/`

Example conflict resolution:

```bash
# After conflict message, edit files in your vault
# Then stage and commit
cd /path/to/Obsidian-Notes/
git add .
git commit -m "Resolve merge conflicts"
./gitsync /path/to/Obsidian-Notes/  # Sync the resolution
```

## Best Practices

### Recommended Setup

```bash
# Use TUI with date commits for automation
./gitsync --interface tui --commit-mode date /path/to/Obsidian-Notes/
```

### Sync Frequency

- **Daily sync**: Morning routine to pull overnight changes
- **Before writing**: Sync before starting new notes
- **After writing**: Sync when done taking notes
- **End of day**: Final sync before bed

### Automation Script

Create a sync script:

```bash
#!/bin/bash
# save as: ~/bin/sync-obsidian

VAULT_PATH="$HOME/Obsidian-Notes"
echo "Syncing Obsidian vault..."
./gitsync --commit-mode date "$VAULT_PATH"
echo "Done!"
```

Make it executable and run:

```bash
chmod +x ~/bin/sync-obsidian
~/bin/sync-obsidian
```

## Multi-Device Sync

### Device 1 (Desktop)

```bash
# Work on notes
./gitsync --commit-mode date /path/to/Obsidian-Notes/
```

### Device 2 (Laptop)

```bash
# Pull changes
./gitsync /path/to/Obsidian-Notes/

# Make changes...

# Push back
./gitsync --commit-mode date /path/to/Obsidian-Notes/
```

### Conflict Prevention

1. **Sync before starting**: Always pull before editing
2. **Sync when done**: Always push when finished
3. **Use same commit mode**: Consistent across devices
4. **Communicate**: Know when others are editing

## Troubleshooting

### "No repositories found"

**Cause:** Git repository not detected  
**Fix:** Ensure directory has `.git` folder

```bash
ls -la /path/to/Obsidian-Notes/ | grep .git
# If no output, run:
cd /path/to/Obsidian-Notes/
git init
```

### "Not a git repository"

**Cause:** Path is not a git repo  
**Fix:** Initialize git or use correct path

```bash
# Check if git repo
git status

# If not initialized:
git init
git remote add origin https://github.com/user/repo.git
```

### "Push failed"

**Cause:** Remote has newer changes  
**Fix:** Pull first, then push

```bash
cd /path/to/Obsidian-Notes/
git pull origin main
git push origin main

# Or use github-lazysync which handles this:
./gitsync /path/to/Obsidian-Notes/
```

### Merge Conflicts

**Cause:** Multiple devices edited same note  
**Fix:** Resolve conflicts manually

```bash
cd /path/to/Obsidian-Notes/
# Open conflicting file in editor
# Resolve conflicts (look for <<<<<<< ======= >>>>>>>)
git add .
git commit -m "Resolve conflicts"
./gitsync /path/to/Obsidian-Notes/
```

## Common Errors and Solutions

| Error | Solution |
|-------|----------|
| "Permission denied" | Check git remote URL, use SSH if needed |
| "Authentication failed" | Set up git credentials or SSH keys |
| "Remote does not exist" | Create GitHub repo and add remote |
| "Nothing to commit" | No local changes to sync |

## Advanced Usage

### Filter Repositories

In TUI mode, type to filter the repository list:

```
Type: "obsidian"  → Shows only Obsidian-related repos
Type: "work"      → Shows only work-related repos
Backspace         → Remove characters
```

### Rescan Repositories

Press `n` in TUI to refresh the repository list after adding new repos.

### Exit TUI

Press `q` to exit the TUI at any time.

## Git Configuration

Ensure git is configured for your vault:

```bash
cd /path/to/Obsidian-Notes/
git config user.name "Your Name"
git config user.email "your.email@example.com"
```

For automatic authentication, set up SSH keys or use a credential helper:

```bash
git config --global credential.helper store
```

## Performance Tips

- **Exclude large files**: Add to `.gitignore`
- **Exclude temporary files**: Don't sync Obsidian cache
- **Use SSD**: Faster file operations
- **Regular maintenance**: Occasionally run `git gc`

## Comparison with Obsidian-GitHub-Sync Plugin

| Feature | GitHub-LazySync | Obsidian-GitHub-Sync |
|---------|----------------|---------------------|
| Platform | Terminal | Obsidian Plugin |
| Interface | TUI | GUI |
| Automation | Manual | Auto-sync option |
| Complexity | Simple | Simple |
| Git knowledge | Basic | Minimal |

Both follow the same philosophy: keep sync simple, no branching, user resolves conflicts manually.

## References

- [README.md](/home/ubuntu/Projects/github-lazysync/README.md)
- Git documentation: https://git-scm.com/doc
- Obsidian-GitHub-Sync: https://github.com/kevinmkchin/Obsidian-GitHub-Sync
