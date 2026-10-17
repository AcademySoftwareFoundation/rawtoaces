# Windows Test Debugging Insights and Pitfalls

This document captures key insights, assumptions, and pitfalls discovered while debugging Windows test failures. **Read this before attempting to fix Windows test issues.**

## Table of Contents
1. [Project Structure](#project-structure)
2. [CMake and Build System](#cmake-and-build-system)
3. [Working Directory Behavior](#working-directory-behavior)
4. [DLL Loading on Windows](#dll-loading-on-windows)
5. [Path Resolution](#path-resolution)
6. [CTest Configuration](#ctest-configuration)
7. [Subprocess Execution (`_popen`)](#subprocess-execution-_popen)
8. [CI vs Local Differences](#ci-vs-local-differences)
9. [Common Pitfalls](#common-pitfalls)
10. [Current Known Issues](#current-known-issues)

---

## Project Structure

### Build Directory Layout
```
build/
├── src/
│   └── rawtoaces/
│       └── Release/
│           ├── rawtoaces.exe
│           └── [DLLs from vcpkg]
├── tests/
│   └── Release/
│       └── Test_*.exe
└── _deps/
    └── rawtoaces_data-src/
        └── data/
            ├── camera/
            ├── illuminant/
            └── ...
```

**Key Points:**
- Executables are in `Release/` subdirectories (Visual Studio multi-config generator)
- All required DLLs are colocated with `rawtoaces.exe` in `build/src/rawtoaces/Release/`
- Test data files are in `build/_deps/rawtoaces_data-src/data/`
- Test materials are in `tests/materials/` (relative to project root)

---

## CMake and Build System

### Multi-Config Generator (Visual Studio)
- **Generator**: Visual Studio generator is a **multi-config generator**
- **Executable Location**: Executables are in `build/tests/Release/` not `build/tests/`
- **Solution**: Call `add_test` with the target name. CMake knows the executable’s location for each configuration and substitutes it when generating the test command.
  ```cmake
  add_test(NAME Test_ImageConverter COMMAND Test_ImageConverter)
  ```
- **Why**: When the command refers to an executable target, CMake expands it to the configuration-specific path (`Release/`, `Debug/`, etc.), so no manual path handling is necessary.

### Test Registration
- Tests register themselves with plain `add_test` calls that name the executable target. CMake resolves the correct configuration-specific path automatically.
  ```cmake
  add_test(NAME Test_ImageConverter COMMAND Test_ImageConverter)
  ```

---

## Working Directory Behavior

### CTest Default Working Directory
- **Default**: CTest sets the working directory to where the test executable is located
- **Verified**: For `build/tests/Release/Test_ImageConverter.exe`, working directory is `build/tests/` (NOT `build/tests/Release/`)
- **Impact**: All relative paths in tests resolve from this directory

### Test Code Relative Paths
Tests use relative paths that assume a specific working directory:

1. **Data Files**: `../_deps/rawtoaces_data-src/data/`
   - Resolves to `build/_deps/rawtoaces_data-src/data/` when working directory is `build/tests/`
   - **DO NOT** set `WORKING_DIRECTORY` to `build/tests/Release/` - this breaks the path resolution

2. **Test Materials**: `../../tests/materials/blackmagic_cinema_camera_cinemadng.dng`
   - Resolves to `tests/materials/...` when working directory is `build/tests/`

3. **Executable Path**: `../src/rawtoaces/Release/rawtoaces.exe`
   - Resolves to `build/src/rawtoaces/Release/rawtoaces.exe` when working directory is `build/tests/`

### CI Working Directory
- **CI runs from**: `build` directory (see `.github/workflows/build-steps.yml` line 97)
- **Command**: `working-directory: build` then `ctest`
- **Why it works**: CTest still uses the default working directory (`build/tests/`), so relative paths resolve correctly

### ⚠️ Critical Assumption
**DO NOT change `WORKING_DIRECTORY` in `add_test()` commands.** The default behavior (working directory = executable location) is correct and matches CI behavior.

---

## DLL Loading on Windows

### Windows DLL Search Order
Windows searches for DLLs in this order:
1. **Executable's directory** (highest priority)
2. System directories
3. Directories in `PATH` environment variable

### Key Insight
- **All required DLLs are colocated** with `rawtoaces.exe` in `build/src/rawtoaces/Release/`
- **Direct execution works** because Windows finds DLLs in the executable's directory
- **No PATH manipulation needed** for direct execution

### PATH Environment Variable
- No additional PATH manipulation is performed in the current CMake setup. CTest launches tests directly and the Visual Studio/applocal logic ensures the required DLLs are beside each executable. Keeping `PATH` untouched mirrors CI behaviour.

---

## Path Resolution

### `std::filesystem::absolute()` Behavior
- **Resolves relative to current working directory**
- **Example**: `std::filesystem::absolute("../_deps/...")` from `build/tests/` resolves to `build/_deps/...`
- **Used in tests**: Tests use `std::filesystem::absolute(DATA_PATH "...")` where `DATA_PATH` is `../_deps/rawtoaces_data-src/data/`

### Test Code Path Construction
In `test_image_converter.cpp`, the `run_rawtoaces_command()` function uses the relative path to the Release executable on Windows and the native binary on other platforms. `_popen` inherits the working directory, so this resolves correctly; again, the crash is not caused by path resolution.

---

## CTest Configuration

### Running Tests Locally
```bash
# From project root
cd build
ctest -C Release --output-on-failure

# Or from build directory
ctest -C Release --output-on-failure
```

**Why `-C Release`?**
- Visual Studio generator is multi-config
- Must specify configuration (Release, Debug, etc.)
- Without `-C`, CTest fails with: "Test not available without configuration. (Missing "-C <config>")"

### Test Execution Flow
1. CTest runs from `build` directory (or project root)
2. CTest sets working directory to `build/tests/` (where executable is located)
3. Test executable runs with that working directory
4. Relative paths in test code resolve from that working directory

---

## Subprocess Execution (`_popen`)

### The Problem
- **Direct execution works**: Running `rawtoaces.exe` directly from command line works fine
- **`_popen` crashes**: Running `rawtoaces.exe` via `_popen()` in tests causes access violations (exit code `-1073741819` = `0xC0000005`)

### Current Implementation
Tests use `platform_popen()` which wraps `_popen()` on Windows:
```cpp
std::string command_with_stderr = command + " 2>&1";
FILE *pipe = platform_popen( command_with_stderr.c_str(), "r" );
```

### Verified Findings
**✅ Working Directory Inheritance**: `_popen` DOES inherit the working directory correctly
- Test working directory: `build/tests/`
- Relative paths resolve correctly from this directory
- Both relative and absolute paths work for finding the executable

**✅ Path Resolution**: Path resolution is NOT the issue
- Relative paths work fine: `../src/rawtoaces/Release/rawtoaces.exe` resolves correctly
- Absolute paths also work
- **Access violations occur with BOTH relative and absolute paths**

**❌ The Real Issue**: The problem is NOT about working directory or path resolution
- Access violations occur during image processing, not at startup
- The executable is found and starts correctly
- The crash happens later, suggesting DLL loading or environment differences

### Why It Fails
**Verified**: The issue is NOT about:
- ❌ Working directory inheritance (works correctly)
- ❌ Path resolution (works correctly)
- ❌ Finding the executable (works correctly)
- ❌ PATH manipulation (we removed it and the crash persists)

**Likely Causes** (still needs investigation):
- DLL loading behavior differences in `_popen` subprocess vs direct execution
- Environment variable differences in the subprocess beyond PATH
- Memory access issues specific to subprocess execution context
- Windows security/permission differences for subprocess execution

### ⚠️ Unresolved Issue
This is a **known issue** that needs further investigation. The access violations suggest:
- DLL loading problems when spawned via `_popen` (even though DLLs are colocated)
- Environment differences between direct execution and subprocess execution
- Possible need for explicit PATH or DLL path setup in subprocess
- Windows-specific behavior of `_popen` that differs from direct execution

---

## CI vs Local Differences

### CI Configuration (`.github/workflows/build-steps.yml`)
```yaml
- name: Test
  working-directory: build
  run: ctest
```

**Key Points:**
- CI runs `ctest` from `build` directory
- CI does **not** specify `-C Release` (but it works - why?)
- Tests pass on CI and Mac, fail on Windows locally

### Why CI Works
- CI uses a Docker image or GitHub Actions runner
- Environment might be different (cleaner, more consistent)
- DLL loading might work differently in CI environment

### ⚠️ Important
**Tests work on CI and Mac, but fail on Windows locally.** This suggests:
- Windows-specific issue (DLL loading, `_popen` behavior)
- Not a fundamental test design problem
- Solution should be Windows-specific, not change cross-platform behavior

---

## Common Pitfalls

### ❌ Don't Do This

1. **Don't set `WORKING_DIRECTORY` in `add_test()`**
   ```cmake
   # WRONG - breaks relative path resolution
   add_test(NAME test COMMAND exe)
   set_tests_properties(test PROPERTIES WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
   ```
   - Breaks `../_deps/...` path resolution
   - Doesn't match CI behavior

2. **Don't assume working directory is build root**
   - CTest sets working directory to executable location
   - Relative paths must account for this

3. **Don't add unnecessary PATH manipulation**
   - If DLLs are colocated, PATH manipulation might not be needed
   - But keep it for safety (vcpkg DLLs might be in bin directory)

4. **Don't change cross-platform behavior for Windows-only issues**
   - Tests work on CI and Mac
   - Fix should be Windows-specific

### ✅ Do This

1. **Let CTest use default working directory**
   - Don't set `WORKING_DIRECTORY` property
   - Default behavior matches CI

2. **Test locally before changing code**
   - Run `ctest -C Release --output-on-failure` to see actual failures
   - Don't assume what the problem is

---

## Current Known Issues

### 1. Access Violations in `Test_ImageConverter`
**Status**: Unresolved

**Symptoms:**
- Exit code `-1073741819` (`0xC0000005`) when `rawtoaces.exe` runs via `_popen`
- Occurs during image processing, not at startup
- Direct execution works fine

**Affected Tests:**
- `test_spectral_conversion_success()`
- `test_rawtoaces_spectral_mode_complete_success_with_custom_camera_info()`
- `test_rawtoaces_spectral_mode_complete_success_with_default_illuminant_warning()`
- `test_illuminant_ignored_with_metadata_wb()`

**Verified (NOT the cause):**
- ✅ Working directory inheritance works correctly
- ✅ Path resolution works correctly (both relative and absolute paths work)
- ✅ Executable is found and starts correctly
- ❌ Access violations occur with BOTH relative and absolute paths

**Likely Causes** (needs investigation):
- DLL loading behavior differences in `_popen` subprocess vs direct execution
- Environment variable differences in the subprocess environment
- Memory access issues specific to subprocess execution context
- Windows security/permission differences for subprocess execution

**Next Steps:**
- Investigate DLL loading in `_popen` subprocess (even though DLLs are colocated)
- Check environment variable inheritance in `_popen`
- Consider alternative subprocess execution methods (CreateProcess, etc.)
- Compare environment between direct execution and `_popen` subprocess

### 2. Assertion Failures (Expected Output Not Found)
**Status**: Partially resolved (some tests pass, some fail)

**Symptoms:**
- Tests expect specific error/success messages in output
- Output doesn't contain expected strings
- May be related to access violations (process crashes before producing expected output)

**Affected Tests:**
- Various tests checking for specific error messages
- Tests checking for success messages (fail because process crashes)

---

## Debugging Tips

### 1. Check Working Directory
```cpp
// Add to test code temporarily
std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
```

### 2. Check Executable Path
```cpp
// In run_rawtoaces_command()
std::cout << "Executable path: " << exe_path_str << std::endl;
```

### 3. Run Executable Directly
```bash
# From build/tests/Release
F:\Projects\coding\rawtoaces\build\src\rawtoaces\Release\rawtoaces.exe --list-cameras
```

### 4. Check DLL Loading
- Use Dependency Walker or similar tool
- Check if all DLLs are found
- Verify DLLs are in executable directory

### 5. Check Test Output
```bash
ctest -C Release -V -R Test_ImageConverter
```
- `-V` for verbose output
- `-R` to run specific test

---

## Summary

### Key Takeaways

1. **Working Directory**: CTest sets working directory to `build/tests/` (where executable is located). Don't change this.

2. **Path Resolution**: Relative paths resolve from the working directory (`build/tests/`). `_popen` inherits this directory, so relative paths work fine. Absolute paths are optional and do not address the crash.

3. **Multi-Config Generators**: Register tests with the executable target name; CMake expands it to the correct configuration-specific path.

4. **DLL Loading**: Windows searches executable directory first. If DLLs are colocated, direct execution works without PATH manipulation.

5. **`_popen` Issues**: 
   - ✅ `_popen` DOES inherit working directory correctly
   - ✅ Path resolution works correctly (relative paths are sufficient)
   - ✅ No PATH tweaking is required (removing it does not change behaviour)
   - ❌ Access violations occur regardless of path handling
   - ❌ The issue is NOT about working directory or path resolution
   - ❌ Likely DLL loading or other environment differences in `_popen` subprocess (unresolved)

6. **CI vs Local**: Tests work on CI and Mac, fail on Windows locally. Fix should be Windows-specific.

7. **Don't Over-Engineer**: Avoid unnecessary changes. The default CTest behavior is correct. Don't assume path resolution is the issue - verify first.

---

## References

- **CMake Multi-Config Generators**: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html#multi-config-generators
- **CTest Working Directory**: https://cmake.org/cmake/help/latest/manual/ctest.1.html
- **Windows DLL Search Order**: https://docs.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order
- **`_popen` Documentation**: https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/popen-wpopen

---

**Last Updated**: Based on debugging session where tests were failing on Windows but working on CI and Mac.

---

## Building and Running Tests on Windows

### Prerequisites
- Visual Studio 2022 with C++ workloads
- vcpkg installed at `C:/vcpkg` (the project relies on the shared install used in CI)
- Git Bash or PowerShell for running the commands below

### Configure (from the project root)
```powershell
# Run once, or whenever the build directory needs to be regenerated.
cmake -B build -S . ^
      -D CMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
      -D CMAKE_CXX_STANDARD=17 ^
      -D ENABLE_SHARED=OFF
```

### Build
```powershell
cmake --build build --config Release
```

### Run the unit tests
```powershell
# Default working directory must be the repository root or the build directory.
cd build
ctest -C Release --output-on-failure
```

### Run the install/config tests (optional, mirrors CI `test_install`)
```powershell
cd ..  # back to repo root if needed
cmake -B build_config_test -S ./tests/config_tests ^
      -D RAWTOACES_DIR=%CD%/install/lib/cmake/RAWTOACES ^
      -D CMAKE_TOOLCHAIN_FILE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake" ^
      -D CMAKE_CXX_STANDARD=17

cmake --build build_config_test --config Release

cd build_config_test
ctest -C Release --output-on-failure
```

### Notes
- All commands assume you’ve already run the dependency script or that the shared `C:/vcpkg/installed` tree is present (same convention as CI).
- The Visual Studio generator is multi-config; always pass `--config Release` (or Debug) to `cmake --build` and `ctest -C`.
- Tests run from `build/tests` by default; relative paths in the test code rely on that working directory.

