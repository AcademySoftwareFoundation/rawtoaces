#!/usr/bin/env python3
"""
Script to restructure the rawtoaces codebase:
1. Move C++ code from src/* (except bindings) to src/cpp/*
2. Rename src/bindings to src/python
3. Update references in CMakeLists.txt and other files
"""

import os
import re
import shutil
from pathlib import Path

SRC_DIR = Path("F:/Projects/coding/rawtoaces/src")
CPP_DIR = SRC_DIR / "cpp"
BINDINGS_DIR = SRC_DIR / "bindings"
PYTHON_DIR = SRC_DIR / "python"

# Directories to move to cpp (excluding bindings)
DIRS_TO_MOVE = ["rawtoaces", "rawtoaces_core", "rawtoaces_util", "misc"]

def move_dirs_to_cpp():
    """Move C++ source directories to src/cpp"""
    print("=== Moving directories to src/cpp ===")
    
    # Create cpp directory if it doesn't exist
    CPP_DIR.mkdir(exist_ok=True)
    
    for dir_name in DIRS_TO_MOVE:
        src_path = SRC_DIR / dir_name
        dest_path = CPP_DIR / dir_name
        
        if src_path.exists():
            print(f"Moving {src_path} -> {dest_path}")
            if dest_path.exists():
                print(f"  Warning: {dest_path} already exists, removing")
                shutil.rmtree(dest_path)
            shutil.move(str(src_path), str(dest_path))
        else:
            print(f"  Skipping {src_path} - does not exist")
    
    print()

def rename_bindings_to_python():
    """Rename bindings directory to python"""
    print("=== Renaming bindings to python ===")
    
    if BINDINGS_DIR.exists():
        if PYTHON_DIR.exists():
            print(f"  Warning: {PYTHON_DIR} already exists, removing")
            shutil.rmtree(PYTHON_DIR)
        print(f"Renaming {BINDINGS_DIR} -> {PYTHON_DIR}")
        shutil.move(str(BINDINGS_DIR), str(PYTHON_DIR))
    else:
        print(f"  Skipping - {BINDINGS_DIR} does not exist")
    print()

def update_cmake_references():
    """Update references in CMakeLists.txt files"""
    print("=== Updating CMakeLists.txt references ===")
    
    # Update main CMakeLists.txt
    main_cmake = Path("F:/Projects/coding/rawtoaces/CMakeLists.txt")
    if main_cmake.exists():
        content = main_cmake.read_text()
        
        # Replace src/binding with src/python
        if "src/bindings" in content:
            print(f"Updating src/bindings -> src/python in {main_cmake}")
            content = content.replace("src/bindings", "src/python")
            main_cmake.write_text(content)
        
        # Also check for bindings without src/
        if "bindings" in content and "src/" not in content:
            # Check if this is a reference we need to update
            lines = content.split('\n')
            for i, line in enumerate(lines):
                if 'add_subdirectory("src/binding' in line:
                    lines[i] = line.replace('add_subdirectory("src/binding', 'add_subdirectory("src/python')
                    print(f"  Updated CMakeLists.txt line {i+1}")
                    content = '\n'.join(lines)
        
        if "bindings" in content.lower():
            print(f"  Remaining 'bindings' references in CMakeLists.txt:")
            for i, line in enumerate(content.split('\n'), 1):
                if 'bindings' in line.lower():
                    print(f"    Line {i}: {line.strip()}")
    print()

def update_cpp_source_references():
    """Update include paths in C++ source files"""
    print("=== Scanning C++ source files for path references ===")
    
    # Find all .cpp, .h files in src/cpp
    for root, dirs, files in os.walk(CPP_DIR):
        for file in files:
            if file.endswith(('.cpp', '.h', '.hpp')):
                file_path = Path(root) / file
                try:
                    content = file_path.read_text()
                    if 'bindings' in content.lower() or 'py_' in content.lower():
                        print(f"  Found {'bindings' if 'bindings' in content.lower() else ''} in {file_path}")
                except Exception as e:
                    pass
    
    # Check bindings (now python) directory
    if PYTHON_DIR.exists():
        print("\n  Python directory contents:")
        for item in PYTHON_DIR.iterdir():
            print(f"    {item.name}")
    print()

def update_python_bindings_references():
    """Update references in Python binding files"""
    print("=== Checking Python binding files ===")
    
    if PYTHON_DIR.exists():
        for root, dirs, files in os.walk(PYTHON_DIR):
            for file in files:
                if file.endswith(('.cpp', '.h', '.hpp', '.py')):
                    file_path = Path(root) / file
                    try:
                        content = file_path.read_text()
                        if 'bindings' in content.lower():
                            print(f"  Found 'bindings' in {file_path}")
                            for i, line in enumerate(content.split('\n')[:20], 1):
                                if 'bindings' in line.lower():
                                    print(f"    Line {i}: {line.strip()}")
                    except Exception as e:
                        pass
    print()

def main():
    print("Starting code restructure...")
    print()
    
    # Step 1: Move directories to cpp
    move_dirs_to_cpp()
    
    # Step 2: Rename bindings to python
    rename_bindings_to_python()
    
    # Step 3: Update CMake references
    update_cmake_references()
    
    # Step 4: Check for remaining references
    update_cpp_source_references()
    update_python_bindings_references()
    
    print("=== Summary ===")
    print("Directories moved to src/cpp:", DIRS_TO_MOVE)
    print("Directory renamed: bindings -> python")
    print()
    print("Next steps:")
    print("1. Review the changes above")
    print("2. Build the project to verify")
    print("3. Fix any remaining path issues")

if __name__ == "__main__":
    main()
