#!/usr/bin/env python3
"""
Update include paths in test files after restructuring.
"""

import os
import re
from pathlib import Path

TESTS_DIR = Path("F:/Projects/coding/rawtoaces/tests")

# Pattern to match old paths with ../src/ prefix and replace with ../src/cpp/
def update_paths(content):
    # First do specific replacements
    replacements = [
        ("../src/rawtoaces_core/", "../src/cpp/rawtoaces_core/"),
        ("../src/rawtoaces_util/", "../src/cpp/rawtoaces_util/"),
        ("../src/misc/", "../src/cpp/misc/"),
    ]
    
    for old, new in replacements:
        content = content.replace(old, new)
    
    # Also replace ../src/ that's used in test_data_helpers files
    # Check if it's a path reference (not a comment)
    lines = content.split('\n')
    result = []
    for line in lines:
        # Only replace if it's not a comment
        comment_pos = line.find('#')
        src_pos = line.find('../src/')
        if comment_pos == -1 or src_pos < comment_pos:
            line = line.replace("../src/", "../src/cpp/")
        result.append(line)
    
    return '\n'.join(result)

def main():
    print("Updating test files...")
    
    for root, dirs, files in os.walk(TESTS_DIR):
        for file in files:
            if file.endswith('.cpp'):
                file_path = Path(root) / file
                try:
                    content = file_path.read_text()
                    original_content = content
                    content = update_paths(content)
                    
                    if content != original_content:
                        print(f"  Updated: {file_path.relative_to(TESTS_DIR)}")
                        file_path.write_text(content)
                except Exception as e:
                    print(f"  Error processing {file_path}: {e}")
    
    print("\nDone!")

if __name__ == "__main__":
    main()
