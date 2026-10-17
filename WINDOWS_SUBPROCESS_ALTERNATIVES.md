# Windows Subprocess Execution Alternatives

This document lists alternative methods to `_popen()` for running subprocesses on Windows, particularly when dealing with DLL loading issues.

## Current Issue

The `_popen()` function on Windows is causing access violations (`0xC0000005`) when running `rawtoaces.exe` as a subprocess from tests. The executable works fine when run directly but crashes when spawned via `_popen()`.

## Why `_popen()` Might Fail

Based on research and common Windows subprocess issues:

1. **Environment Variable Inheritance**: `_popen()` may not properly inherit all environment variables, particularly those needed for DLL resolution (e.g., `PATH`, `OPENIMAGEIO_DISABLE_PLUGINS`).

2. **Working Directory**: While `_popen()` inherits the working directory, some DLL loading mechanisms may rely on relative paths that resolve differently in subprocess contexts.

3. **Console Handle Inheritance**: `_popen()` creates pipes for stdout/stderr, which may interfere with DLL initialization code that expects a full console environment.

4. **Security Context**: `_popen()` runs the subprocess with potentially different security/permission contexts that might affect DLL loading.

5. **Process Creation Flags**: `_popen()` uses default process creation flags which may not include necessary flags for proper DLL loading in subprocesses.

## Alternative Subprocess Methods

### 1. `CreateProcess()` (Windows API)

**Pros:**
- Full control over process creation
- Can explicitly set environment variables
- Can control process inheritance and security attributes
- Can redirect stdout/stderr to pipes
- Allows setting process creation flags (e.g., `CREATE_UNICODE_ENVIRONMENT`)

**Cons:**
- Windows-specific (not cross-platform)
- More complex to implement (requires manual pipe creation)
- More boilerplate code

**Example:**
```cpp
#include <windows.h>
#include <stdio.h>

bool run_command_createprocess(const std::string& command, std::string& output) {
    HANDLE hChildStd_OUT_Rd = NULL;
    HANDLE hChildStd_OUT_Wr = NULL;
    
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;
    
    // Create pipe for stdout
    if (!CreatePipe(&hChildStd_OUT_Rd, &hChildStd_OUT_Wr, &saAttr, 0)) {
        return false;
    }
    
    // Ensure read handle is not inherited
    SetHandleInformation(hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0);
    
    PROCESS_INFORMATION piProcInfo;
    STARTUPINFOA siStartInfo;
    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&siStartInfo, sizeof(STARTUPINFOA));
    
    siStartInfo.cb = sizeof(STARTUPINFOA);
    siStartInfo.hStdError = hChildStd_OUT_Wr;
    siStartInfo.hStdOutput = hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;
    
    // Get current environment and modify if needed
    LPSTR env = GetEnvironmentStrings();
    // Can modify env here before process creation
    
    BOOL success = CreateProcessA(
        NULL,                    // Application name
        (LPSTR)command.c_str(),  // Command line
        NULL,                    // Process security attributes
        NULL,                    // Thread security attributes
        TRUE,                    // Inherit handles
        CREATE_UNICODE_ENVIRONMENT,  // Creation flags
        env,                     // Environment block (NULL = inherit)
        NULL,                    // Working directory (NULL = inherit)
        &siStartInfo,            // Startup info
        &piProcInfo              // Process information
    );
    
    if (!success) {
        CloseHandle(hChildStd_OUT_Wr);
        CloseHandle(hChildStd_OUT_Rd);
        FreeEnvironmentStrings(env);
        return false;
    }
    
    CloseHandle(hChildStd_OUT_Wr);
    CloseHandle(piProcInfo.hThread);
    
    // Read output from pipe
    char buffer[4096];
    DWORD dwRead;
    output.clear();
    while (ReadFile(hChildStd_OUT_Rd, buffer, sizeof(buffer) - 1, &dwRead, NULL) && dwRead > 0) {
        buffer[dwRead] = '\0';
        output += buffer;
    }
    
    // Wait for process to complete
    WaitForSingleObject(piProcInfo.hProcess, INFINITE);
    
    DWORD exitCode;
    GetExitCodeProcess(piProcInfo.hProcess, &exitCode);
    
    CloseHandle(hChildStd_OUT_Rd);
    CloseHandle(piProcInfo.hProcess);
    FreeEnvironmentStrings(env);
    
    return exitCode == 0;
}
```

### 2. `CreateProcess()` with Explicit PATH Setup

**Key Improvement**: Explicitly modify `PATH` in the environment block before process creation.

```cpp
std::string build_env_with_path(const std::string& additional_path) {
    // Get current PATH
    DWORD size = GetEnvironmentVariableA("PATH", NULL, 0);
    std::string current_path(size, '\0');
    GetEnvironmentVariableA("PATH", &current_path[0], size);
    
    // Prepend additional path
    std::string new_path = additional_path + ";" + current_path;
    
    // Build environment block
    // Format: KEY1=VALUE1\0KEY2=VALUE2\0\0
    std::string env_block = "PATH=" + new_path + "\0";
    
    // Copy other important environment variables
    // ... (can selectively copy needed vars)
    
    env_block += "\0"; // Double null terminator
    return env_block;
}
```

### 3. `std::system()` (C++ Standard Library)

**Pros:**
- Simple to use
- Cross-platform
- Inherits full environment

**Cons:**
- Less control over process
- Cannot easily capture output
- Security concerns (shell injection)
- May not fix DLL issues if they're environment-related

**Example:**
```cpp
int exit_code = std::system("rawtoaces.exe --help");
```

### 4. Boost.Process (Third-party Library)

**Pros:**
- Modern C++ interface
- Cross-platform
- Good control over process creation
- Can set environment variables easily
- Actively maintained

**Cons:**
- Requires Boost dependency
- May need to add to project

**Example:**
```cpp
#include <boost/process.hpp>

namespace bp = boost::process;

bp::ipstream pipe_stream;
bp::child c(bp::search_path("rawtoaces.exe"), 
            bp::args({"arg1", "arg2"}),
            bp::std_out > pipe_stream,
            bp::env["PATH"] = "custom_path;" + bp::native_environment()["PATH"].to_string());

std::string line;
std::string output;
while (std::getline(pipe_stream, line)) {
    output += line + "\n";
}

c.wait();
int exit_code = c.exit_code();
```

### 5. `ShellExecuteEx()` (Windows API)

**Pros:**
- Can run with elevated privileges if needed
- Handles file associations
- Can wait for completion

**Cons:**
- Less control over I/O redirection
- More complex for capturing output
- May spawn separate windows

### 6. Temporary Batch File Approach

**Workaround**: Write a batch file that sets up the environment and runs the command, then execute the batch file.

**Pros:**
- Can fully control environment setup
- Simple to debug
- Can set multiple environment variables easily

**Cons:**
- Creates temporary files
- Slower (spawns cmd.exe)
- Platform-specific

**Example:**
```cpp
std::string create_batch_file(const std::string& command, const std::string& working_dir) {
    std::filesystem::path temp_file = std::filesystem::temp_directory_path() / "rawtoaces_test.bat";
    
    std::ofstream bat(temp_file);
    bat << "@echo off\n";
    bat << "cd /d \"" << working_dir << "\"\n";
    bat << "set PATH=" << additional_path << ";%PATH%\n";
    bat << command << "\n";
    bat.close();
    
    return temp_file.string();
}
```

## Recommended Approach

### For Immediate Testing:
1. **Try `CreateProcess()` with explicit environment variable setup**
   - Build a custom environment block that includes the vcpkg DLL directory in PATH
   - Use `CREATE_UNICODE_ENVIRONMENT` flag
   - This gives the most control and is most likely to fix DLL loading issues

### For Long-term Solution:
1. **Use Boost.Process** (if acceptable to add dependency)
   - Clean, modern API
   - Cross-platform
   - Easy environment variable management

2. **Or implement a wrapper using `CreateProcess()`**
   - More control
   - No external dependencies
   - Can be made cross-platform with `#ifdef` blocks

## Debugging DLL Loading Issues

### Tools to Use:
1. **Process Monitor (ProcMon)** - Monitor DLL loads in real-time
2. **Dependency Walker** - Analyze DLL dependencies
3. **WinDbg** - Debug crashes and view call stacks
4. **Event Viewer** - Check Windows error logs for faulting modules

### What to Check:
1. Are all required DLLs in the executable's directory?
2. Are DLLs being found via PATH? (Check Process Monitor)
3. Are there DLL version mismatches?
4. Are there missing dependencies (e.g., Visual C++ runtime)?
5. Are environment variables properly inherited?

## Testing Strategy

1. Add debug output to `rawtoaces.exe` to see exactly where it crashes
2. Run `rawtoaces.exe` directly (works) vs via `_popen()` (crashes) and compare:
   - Environment variables (`GetEnvironmentStrings()`)
   - Working directory
   - DLL load order (Process Monitor)
3. Try `CreateProcess()` with explicit environment setup
4. Compare successful direct execution vs failed subprocess execution

## References

- [CreateProcess Windows API](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessa)
- [Process Creation Flags](https://learn.microsoft.com/en-us/windows/win32/procthread/process-creation-flags)
- [DLL Loading Order](https://learn.microsoft.com/en-us/windows/win32/dlls/dynamic-link-library-search-order)
- [Boost.Process Documentation](https://www.boost.org/doc/libs/1_84_0/doc/html/process.html)

