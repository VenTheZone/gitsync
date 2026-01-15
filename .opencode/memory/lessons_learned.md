# Lessons Learned

*Project: github-lazysync*
*Last Updated: 2025-01-15*
*Knowledge Base Version: 1.0*

---

## 📚 Do's (Best Practices)

*Use this section to document successful patterns and approaches.*

### General
- **Skill Loading**: Use `/continual-learning` or `skill continual-learning` to load continual learning capability
  - Skills make workflows discoverable and shareable via GitHub
  - Skills provide on-demand guidance without explicit command invocation
  - Related: `.opencode/skill/<name>/SKILL.md`

- **Read First**: Always read a file before editing it
  - Prevents overwriting existing content
  - Required by Edit tool for safety
  - Related: `read` tool usage

### Code Quality
- **Always compile with strict warnings**: Use `clang -Werror -Wall -Wextra` to catch format string errors and implicit declarations
  - Prevents runtime issues from printf format mismatches
  - Related: `.opencode/skill/<name>/SKILL.md`

- **Function declaration order matters in C99**: Either use forward declarations or define functions before their first call
  - ISO C99 doesn't support implicit function declarations
  - Fix: Add static forward declaration OR move implementation before first use

- **Static Analysis Integration**: Run clang-tidy before every commit
  ```bash
  make clang-tidy  # Fix all warnings
  make cppcheck    # Check for undefined behavior
  make format      # Ensure consistent style
  ```
  - Clang-tidy catches: unused variables, memory leaks, null pointer dereference, buffer overflows
  - Cppcheck catches: uninitialized variables, array bounds, resource leaks, signed/unsigned mismatches

- **Memory Safety in C**: Always use safe string operations
  ```c
  // BAD: Buffer overflow
  char buffer[10];
  strcpy(buffer, user_input);
  
  // GOOD: Safe copy
  char buffer[256];
  strncpy(buffer, user_input, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';
  ```

- **Error Handling**: Never ignore system call return values
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

- **Magic Numbers**: Replace hardcoded values with named constants
  ```c
  // BAD: Magic number
  char buffer[256];
  
  // GOOD: Named constant
  #define MAX_BUFFER_SIZE 256
  char buffer[MAX_BUFFER_SIZE];
  ```

### Architecture
- **Simple sync philosophy**: Keep tools focused on core purpose (like Obsidian-GitHub-Sync)
  - Sync workflow: Pull → Stage → Commit → Push
  - No branching, no auto-timers, manual sync triggers
  - User resolves conflicts in their editor
  - Related: `.opencode/skill/<name>/SKILL.md`

- **Remove unused code**: Unused functions cause compilation errors with `-Wunused-function`
  - Run `make` frequently to catch unused code
  - Either use the function or remove it
  - Keeps codebase clean and focused

---

## ❌ Don'ts (Pitfalls to Avoid)

*Use this section to document mistakes to prevent repeating them.*

### Common Mistakes
- **Forgetting printf format parameters**: Missing or extra arguments fail compilation with strict warnings
  - Always count %s placeholders and match with parameters exactly
  - Use empty string "" when no dynamic content needed

- **Skipping forward declarations**: Calling functions before they're defined causes implicit declaration errors in C99
  - Fix: Add static void function_name(parameters); before first use

- **Ignoring fflush() return values**: Can lead to undetected I/O errors
  - Fix: Cast to void: (void)fflush(stdout);

- **Uninitialized FILE pointers**: Always initialize before use
  ```c
  // BAD
  FILE *fp;
  fp = popen(cmd, "r");
  
  // GOOD
  FILE *fp = NULL;
  if ((fp = popen(cmd, "r")) != NULL) {
      // use fp
  }
  ```

- **Missing braces around single statements**: Causes subtle bugs
  ```c
  // BAD
  if (!fp) return 0;
  
  // GOOD
  if (!fp) {
      return 0;
  }
  ```

### Anti-patterns
<!-- Add entries like: "Don't nest callbacks - use promises or async/await" -->

### Configuration
<!-- Add entries like: "Don't hardcode API keys - use environment variables" -->

---

## ⚠️ Edge Cases & Gotchas

*Use this section for specific edge cases, platform-specific issues, and subtle bugs.*

### Platform-Specific
<!-- Add entries like: "Windows file paths need double backslashes" -->

### Race Conditions
<!-- Add entries like: "File read can fail if write hasn't flushed yet" -->

### Performance
- **Recursive directory scanning needs depth limits**: Unbounded recursion can cause stack overflow on deep directory structures
  - Solution: Add depth parameter to limit recursion (e.g., depth > 5 return)

---

## 🔧 Project-Specific Patterns

*Use this section for project-specific conventions and decisions.*

### File Structure
- Main source: `src/github_scanner.c`
- Build system: `Makefile` with clang compilation
- Strict compiler flags: `-Werror -Wall -Wextra -std=c99`

### Code Patterns
- Color codes defined as macros: `COLOR_GREEN`, `COLOR_RESET`, etc.
- Repository struct with name, path, branch, and change status fields
- Recursive directory scanning with depth limiting for safety

### Naming Conventions
<!-- Add entries like: "Use camelCase for JavaScript, snake_case for Python" -->

### API Patterns
<!-- Add entries like: "REST endpoints use /api/v1/ prefix" -->

---

## 🎯 Recent Insights

*Entries auto-generated by /smart-task command*

### 2025-01-15

**Issue**: Repository scanning failed for specific directory paths, showing "No repositories found"

**Lesson**: The `scan_directory()` function was declared but never implemented, causing implicit declaration errors. Fixed by:
1. Implementing the recursive directory scanner with git repository detection
2. Adding proper depth limiting to prevent infinite recursion  
3. Fixing printf format string errors that caused compilation failures

**Example**: Key fix was checking `is_git_repo(path)` before recursing, ensuring each git directory is detected exactly once

**Impact**: Directory-specific scanning now works: `./gitsync /home/ubuntu/Projects/Obsidian-Notes/` correctly finds and displays the repository

**Category**: Code Quality (Bug Fix)

### 2025-01-15

**Issue**: Compiling with -Wunused-function flags unused functions as errors

**Lesson**: Removed two unused duplicate detection functions:
1. `report_and_remove_duplicates()`
2. `check_for_duplicate_filenames()`

These were implemented but never called, causing build failures with strict warnings. Following the Obsidian-GitHub-Sync philosophy of simplicity, these advanced features were removed rather than integrated.

**Example**: Functions defined but not called anywhere in the codebase should be removed immediately

**Impact**: Build now succeeds with zero errors, code is more focused on core sync functionality

**Category**: Code Quality (Code Cleanup)

### 2025-01-15

**Issue**: README documentation listed features that were not implemented (duplicate detection)

**Lesson**: Documentation should match actual implementation:
1. Ran `make clang-tidy` to verify build status
2. Reviewed README against actual functionality
3. Updated features list to remove unimplemented features:
   - Removed "Duplicate Detection" feature
   - Removed "Filename Conflict Reporting" feature
   - Updated TUI design description to match actual layout

**Example**: README listed "3-Panel Layout" but actual layout is Header → Filter → List → Details → Help

**Impact**: Documentation now accurately reflects implementation, preventing user confusion

**Category**: Documentation (Accuracy)

### 2025-01-15

**Issue**: Need to verify sync functionality works end-to-end

**Lesson**: Successfully tested live sync on Obsidian vault:
1. Detected remote changes ([↓] indicator)
2. Pulled 11 files successfully  
3. Committed with auto-timestamp: "🤖 Smart sync: 2026-01-15 10:38"
4. Completed sync without errors

**Example**: Test command:
```bash
echo "1" | ./gitsync --interface simple --commit-mode date /path/to/Obsidian-Notes/
```

**Impact**: Confirmed github-lazysync works correctly for real-world Obsidian vault synchronization

**Category**: Validation (Functional Test)

### 2025-01-15

**Issue**: Release assets not downloadable via wget/curl (404 Not Found)

**Lesson**: Repository was private, preventing unauthenticated downloads:
1. gh release view showed assets as "uploaded"
2. wget/curl returned 404 for download URLs
3. gh repo view revealed `isPrivate: true`
4. Fixed with: `gh repo edit VenTheZone/gitsync --visibility public`
5. After making public: wget successfully downloaded 31K binary

**Example**: Always make repository public BEFORE creating releases with downloadable assets:
```bash
gh repo edit OWNER/REPO --visibility public
gh release create v1.0.0 ...
gh release upload v1.0.0 ./binary
```

**Impact**: Release assets now publicly accessible for all users

**Category**: DevOps (Release Management)

---

*Tip: Use the `/smart-task` command to automatically add entries to this file after completing tasks.*
