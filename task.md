# Enhanced CLI Interface and Commit Mode Implementation Task

## OBJECTIVE
Enhance github_scanner.c with multiple interface modes and flexible commit message options while preserving all existing functionality.

## REQUIREMENTS - Interface Modes (COMPLETED)
1. Add InterfaceMode enum: {MODE_AUTO, MODE_SIMPLE, MODE_TUI}
2. Add ProgramConfig struct to hold mode and settings
3. Enhanced argument parsing:
   - --interface (auto|simple|tui) - default auto
   - Preserve existing flags: --date-commit, --help, --version, --scan-dir
4. Create function stubs:
   - detect_best_interface() - checks for fzf availability
   - run_simple_interface() - basic text selection
   - run_tui_interface() - current fzf functionality
5. Update help text to show new interface options
6. Maintain ALL existing functionality - no breaking changes

## REQUIREMENTS - Commit Modes (NEW)
1. Add CommitMode enum: {COMMIT_DATE, COMMIT_MANUAL, COMMIT_PROMPT}
2. Add --commit-mode flag to argument parsing with options:
   - date: Use timestamped commit (current --date-commit behavior)
   - manual: Prompt for manual commit message (current default behavior)  
   - prompt: Ask user to choose between date/manual each time
3. Add commit_mode field to ProgramConfig struct
4. Modify sync_repository() to handle all three modes
5. Update help text with examples for all commit modes
6. Maintain backward compatibility -- --date-commit flag should still work

## IMPLEMENTATION DETAILS - Commit Modes
- For COMMIT_DATE: Use "gitsync: YYYY-MM-DD HH:MM:SS" format
- For COMMIT_MANUAL: Prompt user for message with fgets()
- For COMMIT_PROMPT: Ask "Use timestamped commit? (y/n): " each sync
- Keep existing --date-commit flag working (set mode to COMMIT_DATE)
- Default should be COMMIT_MANUAL for backward compatibility

## CURRENT STATE ANALYSIS
- File exists: src/github_scanner.c (436 lines)
- InterfaceMode enum implemented with MODE_* prefix ✓
- ProgramConfig struct exists with interface modes ✓
- Function stubs implemented and working ✓
- FZF functionality is working ✓
- --interface flag parsing implemented ✓
- Help text updated for interface modes ✓
- Commit functionality currently only supports date/manual toggle
- Need to add CommitMode enum and flexible commit options
- sync_repository() function needs enhancement for three commit modes

## IMPLEMENTATION PLAN - Commit Modes
1. Add CommitMode enum with COMMIT_DATE, COMMIT_MANUAL, COMMIT_PROMPT
2. Add commit_mode field to ProgramConfig struct
3. Update parse_arguments() to handle --commit-mode flag
4. Maintain backward compatibility with existing --date-commit flag
5. Enhance sync_repository() to handle all three commit modes
6. Update help text with commit mode examples
7. Update main() to display commit mode info
8. Test all commit modes and backward compatibility

## VERIFICATION CHECKLIST - Interface Modes (COMPLETED)
- [x] Enum uses MODE_* prefix (MODE_AUTO, MODE_SIMPLE, MODE_TUI)
- [x] ProgramConfig struct is properly defined
- [x] --interface flag parsing works (auto|simple|tui)
- [x] All existing flags preserved (--date-commit, --help, --version)
- [x] detect_best_interface() function exists and works
- [x] run_simple_interface() function exists
- [x] run_tui_interface() function exists with current fzf code
- [x] Help text shows new interface options
- [x] Code compiles without warnings
- [x] No breaking changes to existing functionality

## VERIFICATION CHECKLIST - Commit Modes (NEW)
- [x] CommitMode enum exists with COMMIT_DATE, COMMIT_MANUAL, COMMIT_PROMPT
- [x] commit_mode field added to ProgramConfig struct
- [x] --commit-mode flag parsing works (date|manual|prompt)
- [x] --date-commit flag still works and sets COMMIT_DATE mode
- [x] Default commit mode is COMMIT_MANUAL
- [x] sync_repository() handles all three commit modes correctly
- [x] COMMIT_DATE uses "gitsync: YYYY-MM-DD HH:MM:SS" format
- [x] COMMIT_MANUAL prompts user for commit message
- [x] COMMIT_PROMPT asks "Use timestamped commit? (y/n): " each time
- [x] Help text includes commit mode options and examples
- [x] Backward compatibility maintained - no breaking changes
- [x] Code compiles without warnings
- [x] All commit modes tested and working