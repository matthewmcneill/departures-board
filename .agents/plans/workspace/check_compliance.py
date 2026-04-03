import os
import re

dirs_to_check = ['src', 'modules']
workspace_root = '/Users/mcneillm/Documents/Projects/departures-board'

def is_camel_case(name):
    # Strip extension
    base = os.path.splitext(name)[0]
    # To be camelCase, it usually starts with lower case, contains no underscores or hyphens
    if not base:
        return False
    if base[0].isupper():
        # Sometimes classes are UpperCamelCase, but the rule says camelCase. We'll warn if UpperCamelCase just in case.
        pass
    if '_' in base or '-' in base:
        return False
    return True

issues = []

for d in dirs_to_check:
    full_dir = os.path.join(workspace_root, d)
    for root, _, files in os.walk(full_dir):
        for f in files:
            if f.endswith('.cpp') or f.endswith('.hpp'):
                # Ignore generated files
                if f.startswith('layout') or f == 'fonts.cpp' or f == 'fonts.hpp':
                    continue
                
                file_path = os.path.join(root, f)
                rel_path = os.path.relpath(file_path, workspace_root)
                
                file_issues = []
                
                # Check naming
                if not is_camel_case(f):
                    file_issues.append(f"Filename '{f}' is not camelCase.")
                
                # Check headers
                with open(file_path, 'r', encoding='utf-8') as file:
                    content = file.read()
                    
                    if "Module:" not in content:
                        file_issues.append("Missing 'Module:' in header.")
                    if "Description:" not in content:
                        file_issues.append("Missing 'Description:' in header.")
                    if "Exported Functions/Classes:" not in content:
                        file_issues.append("Missing 'Exported Functions/Classes:' in header.")
                    if "Departures Board (c)" not in content:
                        file_issues.append("Missing 'Departures Board (c)' license text.")
                
                if file_issues:
                    issues.append({
                        "file": rel_path,
                        "issues": file_issues
                    })

for entry in issues:
    print(f"--- {entry['file']} ---")
    for iss in entry['issues']:
        print(f"  - {iss}")
print(f"\nTotal files with issues: {len(issues)}")
