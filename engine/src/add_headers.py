import os
import glob

src_dir = r"C:\Users\cpaci\Desktop\ftd\engine\src"
cpp_files = glob.glob(os.path.join(src_dir, "*.cpp"))

for filepath in cpp_files:
    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()
    
    if "@file" in content and "@purpose" in content:
        continue
        
    filename = os.path.basename(filepath)
    
    header = f"""/**
 * @file engine/src/{filename}
 * @purpose Core engine implementation file for {filename.replace('.cpp', '')}.
 * @consumers Internal engine components.
 * @contract CONTRACTS.md (Engine Core)
 */
"""
    
    # Check if file already starts with a comment block
    if content.strip().startswith("/*"):
        # We might want to replace the existing header or just prepend
        pass
        
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(header + content)
    print(f"Added header to {filename}")
