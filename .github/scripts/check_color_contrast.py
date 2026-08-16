"""Checks color-related source files for colorblind-problematic color pairs.

Extracted from .github/workflows/accessibility.yml (2026-08-16): the script
was previously embedded as a `python3 - <<'PYEOF'` heredoc inside a YAML
`run: |` block, flush-left with no indentation. That broke the YAML block
scalar (every line of a `|` block must be indented at least as much as the
block's first content line), making the entire workflow file fail to parse.
Living in its own file sidesteps the YAML heredoc-indentation trap entirely.
"""
import os

PROBLEMATIC = [
    ('red', 'green'),
    ('#FF0000', '#00FF00'),
    ('#FF0000', '#008000'),
]

issues = []
style_files = []
for root, dirs, files in os.walk('.'):
    dirs[:] = [d for d in dirs if d not in ('node_modules', 'archive', '.git')]
    for f in files:
        if f.endswith(('.py', '.css', '.scss')) and ('style' in f.lower() or 'color' in f.lower()):
            style_files.append(os.path.join(root, f))

print(f"Found {len(style_files)} style-related files to check")

for sf in style_files:
    try:
        with open(sf, 'r', encoding='utf-8', errors='ignore') as fh:
            content = fh.read().lower()
            for c1, c2 in PROBLEMATIC:
                if c1.lower() in content and c2.lower() in content:
                    issues.append(f"{sf}: potentially problematic color pair ({c1}, {c2})")
    except Exception:
        pass

if issues:
    print("WARNINGS:")
    for issue in issues:
        print(f"  - {issue}")
else:
    print("No obvious colorblind-problematic color combinations found.")
