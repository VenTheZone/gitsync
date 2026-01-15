---
name: github-lazysync-c-dev
description: Build, compile, and analyze the github-lazysync C project with clang, static analysis, and memory safety. Use when building the project, fixing compilation errors, or running code quality checks.
---

# GitHub-LazySync C Development

This Skill provides comprehensive guidance for developing and maintaining the github-lazysync C project, including build processes, static analysis, and memory safety patterns.

## Quick Start

```bash
# Build the project
make

# Clean and rebuild
make clean && make

# Run with debug symbols
make debug

# Static build
make static-build
```

## Build System

### Makefile Targets

| Target | Purpose | Command |
|--------|---------|---------|
| `make` | Default build with clang/gcc | Compiles `src/github_scanner.c` |
| `make clean` | Remove build artifacts | Deletes `gitsync` binary |
| `make debug` | Build with address sanitizer | `clang -g -fsanitize=address` |
| `make static-build` | Static binary | Links statically |
| `make install` | Install to system | `sudo make install` |
| `make clang-tidy` | Static analysis | Runs clang-tidy checks |
| `make cppcheck` | Advanced analysis | Runs cppcheck |
| `make format` | Code formatting | Runs clang-format |
| `make analyze` | Run all checks | clang-tidy + cppcheck |

### Compiler Requirements

**Required:**
- GCC or Clang
- POSIX-compatible system (Linux, macOS)
- Git installed

**Optional:**
- clang-tidy (for `make clang-tidy`)
- cppcheck (for `make cppcheck`)
- clang-format (for `make format`)

### Installation

```bash
# Standard install to /usr/local/bin
sudo make install

# Custom prefix
make install PREFIX=$HOME/.local
```

## Static Analysis

### Clang-Tidy

```bash
make clang-tidy
```

**What it catches:**
- Unused variables and functions
- Memory leaks
- Null pointer dereference
- Buffer overflows
- Code style issues

**Common fixes:**

```c
// BEFORE: Unused function (causes -Wunused-function error)
static void unused_function(void) {
    // ...
}

// AFTER: Remove unused code or use it
// (Unused functions cause build failure with -Werror)
```

### Cppcheck

```bash
make cppcheck
```

**What it catches:**
- Uninitialized variables
- Array bounds violations
- Resource leaks
- Signed/unsigned mismatches

### Code Formatting

```bash
make format
```

Uses clang-format for consistent style.

## Compilation with Strict Warnings

The project uses strict compiler flags:

```makefile
CFLAGS = -Werror -Wall -Wextra -std=c99 -D_GNU_SOURCE
```

**Common errors and fixes:**

### Implicit Function Declaration

```
error: call to undeclared function 'function_name'
```

**Cause:** Function called before declaration in C99  
**Fix:** Add forward declaration or move function definition before first use

```c
// Forward declaration
static void function_name(const char* path);

// Or define before use
static void function_name(const char* path) {
    // implementation
}
```

### Format String Mismatches

```
error: more '%' conversions than data arguments
```

**Cause:** printf format specifiers don't match parameters  
**Fix:** Count %s placeholders and ensure all have corresponding arguments

```c
// BEFORE: Missing parameter
printf("%s[%s]%s Not a git repository%s\n", COLOR_RED, "ERROR", COLOR_RESET);
//                                              ^^^ Missing final %s argument

// AFTER: Add empty string parameter
printf("%s[%s]%s Not a git repository%s\n", COLOR_RED, "ERROR", COLOR_RESET, "");
```

### Unused Functions

```
error: unused function 'function_name' [-Werror,-Wunused-function]
```

**Cause:** Function defined but never called  
**Fix:** Either use the function or remove it

```c
// BEFORE: Unused function
static void old_feature(const char* path) {
    // ...
}  // Never called

// AFTER: Remove unused code
// (Keeps codebase clean and build successful)
```

## Memory Safety

### Safe String Operations

```c
// BAD: Buffer overflow
char buffer[10];
strcpy(buffer, user_input);  // May overflow

// GOOD: Safe copy
char buffer[256];
strncpy(buffer, user_input, sizeof(buffer) - 1);
buffer[sizeof(buffer) - 1] = '\0';
```

### Error Handling

```c
// BAD: Ignore errors
system("git pull");

// GOOD: Check return value
int result = system("git pull");
if (result != 0) {
    printf("Error: git pull failed\n");
    return;
}
```

### File Pointer Initialization

```c
// BAD
FILE *fp;
fp = popen(cmd, "r");

// GOOD
FILE *fp = NULL;
if ((fp = popen(cmd, "r")) != NULL) {
    // use fp
    pclose(fp);
} else {
    printf("Error: popen failed\n");
}
```

## Project Structure

```
github-lazysync/
├── src/
│   └── github_scanner.c    # Main source (~900 lines)
├── Makefile                # Build system
├── README.md               # Documentation
└── .opencode/
    └── skill/
        └── github-lazysync-c-dev/  # This skill
```

## Common Development Tasks

### Adding a New Feature

1. Edit `src/github_scanner.c`
2. Test compilation: `make`
3. Run static analysis: `make clang-tidy`
4. Test functionality: `./gitsync --help`

### Fixing Compilation Errors

1. Read the error message carefully
2. Identify error type (implicit declaration, format string, unused function)
3. Apply the appropriate fix from this skill
4. Rebuild: `make clean && make`

### Running All Checks

```bash
# Complete validation
make clean && make && make clang-tidy && make cppcheck
```

## Debugging

### Address Sanitizer

```bash
make debug
./gitsync [options]
```

Detects memory errors:
- Use-after-free
- Buffer overflows
- Memory leaks

### Print Debugging

Add debug output:

```c
printf("DEBUG: Reached here with path=%s\n", path);
fflush(stdout);
```

### GDB Debugging

```bash
# Compile with debug symbols
make debug

# Run with GDB
gdb ./gitsync
(gdb) run [arguments]
(gdb) backtrace
(gdb) break function_name
(gdb) continue
```

## Best Practices

1. **Compile frequently**: Run `make` after every change
2. **Fix warnings immediately**: Don't let them accumulate
3. **Run static analysis**: Use `make clang-tidy` before committing
4. **Remove dead code**: Unused functions break the build
5. **Check return values**: Always validate system calls
6. **Use safe string functions**: strncpy, snprintf instead of strcpy, sprintf
7. **Initialize pointers**: Set to NULL before assignment

## Troubleshooting

### Build Fails with Unused Function Warning

```bash
# Find unused functions
make clang-tidy 2>&1 | grep "unused function"

# Remove unused code or integrate it
```

### Clang-Tidy Shows Many Warnings

```bash
# Run with specific checks only
clang-tidy src/github_scanner.c --checks='*,-clang-analyzer-*,-llvm-*'
```

### Static Build Fails

```bash
# Install static libraries
sudo apt install -y libc6-dev

# Retry
make static-build
```

## References

- [C Development Tools - Static Analysis](/home/ubuntu/Projects/Obsidian-Notes/C%20Development%20Tools%20-%20Static%20Analysis.md)
- [Lessons Learned](/home/ubuntu/Projects/github-lazysync/.opencode/memory/lessons_learned.md)
- Activity Log: [activity_log.md](/home/ubuntu/Projects/github-lazysync/.opencode/memory/activity_log.md)
