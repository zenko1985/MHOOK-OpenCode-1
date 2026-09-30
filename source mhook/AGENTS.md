# Agent Communication Rules

## Language Requirements

**ALWAYS respond to the user in Russian language.** All communication, explanations, questions, and code comments must be in Russian.

## Task Clarification Rules

**ALWAYS ask clarifying questions** if the task is unclear, ambiguous, or missing important details. Do not proceed with implementation until you fully understand the requirements. Ask about:
- Specific expected behavior
- Edge cases and error handling
- UI/UX preferences
- File locations or naming conventions
- Any constraints or limitations

---

# MHook2 Project

MHook2 - Windows-приложение для эмуляции клавиатурных нажатий через мышь и eye-trackers (Tobii REX, TheEyeTribe).

## Project Overview

- **Platform**: Windows (Win32 API)
- **Language**: C++
- **Architecture**: Pattern "Strategy" with central abstract class
- **IDE**: Visual Studio

## Project Structure

```
├── MHook2/                 # Main source code
│   ├── *.h                 # Header files
│   ├── *.cpp               # Source files
│   ├── *.hpp               # C++ headers
│   ├── resource.h          # Resource definitions
│   └── resource.rc         # Resource file (UTF-16 LE)
├── MHook2.sln             # Visual Studio solution
├── MHook2.vcxproj         # Visual Studio project
└── README.txt             # Project documentation
```

## Code Standards

### File Encoding (CRITICAL)

**All source files (.cpp, .h, .hpp) must be saved with UTF-8 with BOM encoding (Codepage 65001).**

This is mandatory for proper handling of Russian characters and comments in the source code. The project uses Russian language extensively in UI strings and comments.

**DO NOT** save files as:
- UTF-8 without BOM
- ANSI/Windows-1251
- UTF-16

**Always use**: UTF-8 with BOM (65001)

### Line Endings (REQUIRED)

**All source files MUST use CRLF line endings (\r\n) for Windows/Visual Studio compatibility.**

Mixed line endings (LF and CRLF in same file) will cause compilation issues and must be avoided.

**Always use**: CRLF (\r\n)

**Encoding, Syntax and Line Endings Fix Script**:
After ANY edit to .cpp or .h files, ALWAYS run:
```bash
python fix_encoding.py
```

This script (`fix_encoding.py` in project root) automatically:
- Detects current encoding of each file
- Converts to UTF-8 with BOM if needed
- Handles UTF-16, Windows-1251, and other encodings
- **FIXES SYNTAX ERRORS**: `class Name:public` → `class Name : public`
- **NORMALIZES LINE ENDINGS**: Converts all LF (\n) to CRLF (\r\n)
- Shows progress for each file processed
- Skips temporary directories (temp, x64, Debug, Release)

### Naming Conventions

- **Classes**: PascalCase (e.g., `MHookHandler`, `MHSettings`)
- **Static Classes**: Prefix with `MH` (e.g., `MHKeypad`, `MHVector`)
- **Methods**: PascalCase (e.g., `OnMouseMove`, `OnLDown`)
- **Variables**: 
  - Member variables: Use descriptive names (e.g., `rbutton_pressed`, `position_mem`)
  - Static variables: snake_case (e.g., `flag_autoclick_lmb`)
  - Global variables: prefix with `flag_` for booleans (e.g., `flag_left_button_waits`)
- **Constants**: UPPER_CASE with underscores (e.g., `IDC_CHECK_AUTOCLICK`, `MH_NUM_SCANCODES`)
- **Resource IDs**: Prefix with `IDC_` for controls, `IDD_` for dialogs, `IDB_` for bitmaps

### Code Style

- Use tabs for indentation
- Opening brace on the same line for functions and control structures
- Always use braces even for single-line blocks
- Comment in Russian for project-specific logic
- Use `//` for single-line comments, `/* */` for multi-line

### Include Order

1. Windows headers (`<Windows.h>`)
2. Standard library headers
3. Project headers (in quotes)
4. Resource header (`resource.h`)

Example:
```cpp
#include <Windows.h>
#include "HookHandler.h"
#include "Settings.h"
```

### Class Structure

For handler classes inheriting from `MHookHandler`:
- Implement virtual methods from base class
- Add mode-specific fields at the end
- Use `// Название режима` comment before class definition

### Resource Management

- Dialog controls: Use `BS_AUTOCHECKBOX | WS_TABSTOP` for checkboxes
- ComboBoxes: Use `CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP`
- Control IDs: Must be unique, defined in `resource.h`
- Dialog layouts: Keep consistent spacing (10-15 pixels between controls)

### Settings Pattern

When adding new settings:
1. Add static variable in `Settings.h`
2. Initialize in `Settings.cpp`
3. Add control in `FillDialogue()`
4. Add load logic in `AfterLoad()`
5. Add save logic in `SettingsDialogue()` (WM_COMMAND section)

### Hook Handler Pattern

When implementing new modes:
1. Inherit from `MHookHandler`
2. Override virtual methods as needed
3. Implement `OnLDown()` and `OnLUp()` for click handling
4. Use timers (ID 1-6) for delayed actions

## Critical Rules

1. **Encoding**: ALL .cpp and .h files MUST be UTF-8 with BOM
2. **Russian Text**: All UI strings that appear to user must be in Russian
3. **Timer IDs**: 
   - 1: Key press timeout
   - 2: Screen corner timer
   - 3: Left button release
   - 4: Right button release  
   - 5: Magic windows timer
   - 6: Autoclicker timer
4. **Key Simulation**: Use `MHKeypad::Press4()` and `MHKeypad::Press8()` for key presses
5. **Error Handling**: Use `MHRepErr` for error reporting

## Development Guidelines

- Test all modes after changes to `HookHandler` base class
- When modifying resource files, ensure IDs are unique
- Magic windows: Always check `active` flag before processing
- Eye-trackers: Handle connection failures gracefully
- Settings: Provide sensible defaults for new options

## External Dependencies

- Windows SDK (Win32 API)
- tobii_stream_engine.dll (for Tobii eye-trackers)
- TCP connection on port 6555 (for TheEyeTribe)

## Build Requirements

- Visual Studio 2019 or later
- Windows SDK 10.0 or later
- Character Set: Use Multi-Byte (not Unicode) for Win32 compatibility

## Post-Edit Verification Checklist

### 1. Encoding Verification (MANDATORY)

**ALWAYS run the encoding fix script after editing any .cpp or .h files:**

```bash
python fix_encoding.py
```

This script automatically converts all source files to UTF-8 with BOM encoding.

**Location**: `fix_encoding.py` in project root
**Purpose**: Ensures all .cpp, .h, .hpp files have proper UTF-8 with BOM encoding
**When to run**: 
- After ANY file edit
- Before committing changes
- After any automated tool modifies files

### 2. Variable Dependencies Check (CRITICAL)

**When modifying arrays or constants, ALWAYS verify related definitions:**

Example - Adding items to dlg_scancodes array:
1. ✅ Check `MH_NUM_SCANCODES` in Settings.h (increase by number of new items)
2. ✅ Check `MH_NUM_SCANCODES_EXTRA` in Settings.h (increase by number of new items)
3. ✅ Verify array bounds match declaration: `dlg_scancodes[MH_NUM_SCANCODES_EXTRA]`
4. ✅ Check all usages in Settings.cpp (loops, initialization, etc.)
5. ✅ Verify MagicWindow.cpp uses correct index bounds
6. ✅ Check Settings2.cpp for any related references

**Common pattern for arrays**:
```cpp
// In Settings.h
#define MH_NUM_SCANCODES 105        // Update this!
#define MH_NUM_SCANCODES_EXTRA 110  // Update this!

// In Settings.cpp
MHWORDChar dlg_scancodes[MH_NUM_SCANCODES_EXTRA] = { ... };
```

**Search pattern**:
```bash
# Always search for all related constants
grep -n "MH_NUM_SCANCODES\|dlg_scancodes" *.cpp *.h
grep -n "ARRAY_SIZE\|array_name" *.cpp *.h
```

### 3. Build Verification (REQUIRED)

**After completing changes, ALWAYS verify the build:**

1. **Open in Visual Studio**: Load MHook2.sln
2. **Clean solution**: Build → Clean Solution
3. **Rebuild**: Build → Rebuild Solution
4. **Check for ERRORS** (ignore warnings):
   - View → Error List
   - Filter by "Errors only" (ignore warnings)
   - All errors must be resolved before proceeding

**Common build errors to check for**:
- C2078: "too many initializers" - array size mismatch
- LNK errors: Missing implementations
- C3861: Identifier not found
- C2065: Undeclared identifier

**DO NOT** consider the task complete if there are ANY errors.

### 4. Testing Checklist

After successful build:
1. Test in Debug mode
2. Test all affected modes (1-7)
3. Test new functionality thoroughly
4. Verify Russian text displays correctly
5. Check no memory leaks or crashes

## Notes

- The project extensively uses Win32 API - avoid modern C++ features that conflict
- Resource file (.rc) is UTF-16 LE - handle with care when editing
- Static variables are used extensively for global state
- Timer-based logic is critical for proper key press timing
- **Always re-run fix_encoding.py after any batch operations**
