import os

index_file = r'c:\Users\cpaci\Desktop\ftd\META_DOCUMENTATION_MAP.md'
with open(index_file, 'r', encoding='utf-8') as f:
    index_content = f.read()

missing = []
for root, dirs, files in os.walk(r'c:\Users\cpaci\Desktop\ftd\docs\theory'):
    if 'archive' in root:
        continue
    for file in files:
        if file.endswith('.md'):
            rel_path = os.path.relpath(os.path.join(root, file), r'c:\Users\cpaci\Desktop\ftd\docs\theory').replace('\\', '/')
            if file not in index_content:
                missing.append(rel_path)

print("Missing from META_DOCUMENTATION_MAP.md:")
for m in missing:
    print(m)
