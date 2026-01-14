# Enhanced CLI Interface Implementation Plan

## TECHNICAL DESIGN

### Current Issues to Fix
1. **Enum Naming**: Current code uses `INTERFACE_*` but requirements specify `MODE_*`
2. **Missing Flag**: No `--interface` flag parsing (current uses separate `--tui`, `--simple`, `--auto`)
3. **Function Names**: Requirements want `run_simple_interface()` and `run_tui_interface()` instead of current `*_select_repo()`
4. **Compilation Warning**: Buffer truncation warning in sync_repository()

### Implementation Strategy

#### Phase 1: Enum and Structure Updates
- Rename InterfaceMode values from `INTERFACE_*` to `MODE_*`
- Keep ProgramConfig struct unchanged (already correct)
- Update all references to use new enum values

#### Phase 2: Argument Parsing Enhancement
- Add `--interface` flag parsing accepting (auto|simple|tui)
- Maintain backward compatibility with existing flags
- Update parse_arguments() function logic

#### Phase 3: Function Mapping
- Create wrapper functions `run_simple_interface()` and `run_tui_interface()`
- Keep existing `*_select_repo()` functions for internal use
- Ensure `detect_best_interface()` remains unchanged

#### Phase 4: Help Text Updates
- Add `--interface` option to help text
- Maintain existing examples and add new ones

#### Phase 5: Code Quality
- Fix compilation warning in sync_repository()
- Ensure all code follows existing patterns
- Verify ASCII-only characters

## COMPATIBILITY MATRIX
| Feature | Current | Target | Status |
|---------|---------|--------|---------|
| InterfaceMode enum | INTERFACE_* | MODE_* | Needs Update |
| --interface flag | No | Yes | Needs Addition |
| Function names | *_select_repo | run_*_interface | Needs Wrappers |
| Help text | Basic | Enhanced | Needs Update |
| FZF functionality | Working | Preserved | Maintain |

## TESTING STRATEGY
1. Compile with `gcc -Wall -Wextra -std=c99 -D_GNU_SOURCE`
2. Test each interface mode:
   - `./github_scanner --interface tui`
   - `./github_scanner --interface simple`
   - `./github_scanner --interface auto`
3. Verify backward compatibility:
   - `./github_scanner --tui`
   - `./github_scanner --simple`
   - `./github_scanner --auto`
4. Test all existing flags work with new interface flag