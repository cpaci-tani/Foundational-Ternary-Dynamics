import os
import subprocess
import re

base_dir = r"c:\Users\cpaci\Desktop\ftd"

# 1. Directory and file renames
subprocess.run(["git", "mv", "docs/theory/06_reference_frames_and_measurement", "docs/theory/06_reference_frames_and_measurement"], cwd=base_dir, check=False)
subprocess.run(["git", "mv", "docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md", "docs/theory/01_reference/REF_REFERENCE_FRAME_VOCABULARY.md"], cwd=base_dir, check=False)

# 2. Text Replacements
# Order matters: longest/most specific first
replacements = [
    ("06_reference_frames_and_measurement", "06_reference_frames_and_measurement"),
    ("REF_REFERENCE_FRAME_VOCABULARY", "REF_REFERENCE_FRAME_VOCABULARY"),
    ("reference frame projection", "reference frame projection"),
    ("Reference frame projection", "Reference frame projection"),
    ("frame-relative readout", "frame-relative readout"),
    ("Frame-relative readout", "Frame-relative readout"),
    ("reference frame", "reference frame"),
    ("Reference frame", "Reference frame"),
    ("reference frame structure", "reference frame structure"),
    ("Reference frame structure", "Reference frame structure"),
    ("frame-internal causation", "frame-internal causation"),
    ("Frame-internal causation", "Frame-internal causation"),
    ("active frame dynamics", "active frame dynamics"),
    ("Active frame dynamics", "Active frame dynamics"),
    ("local reference frame", "local reference frame"),
    ("Local reference frame", "Local reference frame"),
    ("reference frames", "reference frames"),
    ("Reference frames", "Reference frames"),
    ("reference frame", "reference frame"),
    ("Reference frame", "Reference frame"),
    ("reference frame context", "reference frame context"),
    ("Reference frame context", "Reference frame context"),
    ("reference frame structure", "reference frame structure"),
    ("Reference frame structure", "Reference frame structure"),
    ("frame-relative", "frame-relative"),
    ("Frame-relative", "Frame-relative"),
    ("frame dynamics", "frame dynamics"),
    ("Frame dynamics", "Frame dynamics"),
    ("active-frame", "active-frame"),
    ("Active-frame", "Active-frame"),
    ("reference-frame", "reference-frame"),
    ("Reference-frame", "Reference-frame")
]

def apply_replacements(text):
    for old, new in replacements:
        text = text.replace(old, new)
    return text

# Walk and replace
for root, dirs, files in os.walk(base_dir):
    # Skip .git and node_modules
    if '.git' in root or 'node_modules' in root or '__pycache__' in root or '.venv' in root:
        continue
    for f in files:
        if f.endswith('.md') or f.endswith('.cpp') or f.endswith('.h') or f.endswith('.py') or f.endswith('.js'):
            path = os.path.join(root, f)
            try:
                with open(path, 'r', encoding='utf-8') as file:
                    content = file.read()
                
                new_content = apply_replacements(content)
                
                if new_content != content:
                    with open(path, 'w', encoding='utf-8') as file:
                        file.write(new_content)
                    print(f"Updated: {path}")
            except Exception as e:
                pass

print("Reframing complete.")
