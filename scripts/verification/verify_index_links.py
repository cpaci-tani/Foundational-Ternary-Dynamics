import os
import re

base_dir = r"c:\Users\cpaci\Desktop\ftd"
meta_index_path = os.path.join(base_dir, "docs", "theory", "META_INDEX.md")
native_index_path = os.path.join(base_dir, "docs", "theory", "10_eft_program", "INDEX_FTD_NATIVE_EFT.md")

# Find all links of the form [Link Name](path/to/file.md) or [Link Name](file.md)
link_pattern = re.compile(r"\[([^\]]+)\]\(([^)]+)\)")

def verify_file_links(file_path, relative_to_dir):
    print(f"\nChecking links in {os.path.basename(file_path)}...")
    if not os.path.exists(file_path):
        print(f"Error: Index file {file_path} does not exist.")
        return
        
    with open(file_path, "r", encoding="utf-8") as f:
        content = f.read()
        
    links = link_pattern.findall(content)
    total = 0
    broken = 0
    
    for name, path in links:
        if path.startswith("http") or path.startswith("mailto:") or path.startswith("#"):
            continue
            
        # Clean query parameters or anchors
        clean_path = path.split("#")[0].split("?")[0]
        if not clean_path:
            continue
            
        total += 1
        target_path = os.path.normpath(os.path.join(relative_to_dir, clean_path))
        if not os.path.exists(target_path):
            print(f"  [BROKEN] {name} -> {path} (Resolved path: {target_path})")
            broken += 1
            
    print(f"Verified {total} file links. Broken: {broken}")

verify_file_links(meta_index_path, os.path.join(base_dir, "docs", "theory"))
verify_file_links(native_index_path, os.path.join(base_dir, "docs", "theory", "10_eft_program"))
