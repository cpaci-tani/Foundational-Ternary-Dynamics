import os
import re

base_dir = r"c:\Users\cpaci\Desktop\ftd\docs\theory"

moves = {
    "03_derivations/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md": "03_derivations/archive/AUDIT_CLOCK_HYPOTHESIS_v3_CLOSED_NEGATIVE.md",
    "03_derivations/DERIV_QUANTUM_MECHANICS_RESOLVED.md": "03_derivations/archive/DERIV_QUANTUM_MECHANICS_RESOLVED.md",
    "10_eft_program/AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_DETERMINANT_GRADING_CLOSED_NEGATIVE.md",
    "10_eft_program/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE.md",
    "10_eft_program/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md": "10_eft_program/archive/closed_negative/AUDIT_ALPHA_READOUT_OBSERVABLE_SELECTION_CLOSED_NEGATIVE_SYNTHESIS.md",
    "10_eft_program/AUDIT_MANIFESTATION_NONCOMMUTATIVITY_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_MANIFESTATION_NONCOMMUTATIVITY_CLOSED_NEGATIVE.md",
    "10_eft_program/AUDIT_MODULAR_TIME_ALGEBRA_TYPE_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_MODULAR_TIME_ALGEBRA_TYPE_CLOSED_NEGATIVE.md",
    "10_eft_program/AUDIT_SYMPLECTIC_BUDGET_SYMMETRY_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_SYMPLECTIC_BUDGET_SYMMETRY_CLOSED_NEGATIVE.md",
    "10_eft_program/AUDIT_X_MINUS_CLOSED_NEGATIVE.md": "10_eft_program/archive/closed_negative/AUDIT_X_MINUS_CLOSED_NEGATIVE.md"
}

# Also need to map basenames to their new paths relative to the file being updated, but let's stick to standard markdown link replacements where possible.
# Most links are like (03_derivations/...) or (AUDIT_...).

def replace_in_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()
    
    orig_content = content
    
    for old, new in moves.items():
        # Replace absolute-ish paths
        content = content.replace("(" + old + ")", "(" + new + ")")
        content = content.replace("]({old})".format(old=old), "]({new})".format(new=new))
        
        # Replace just the basename if it's referenced directly
        basename = os.path.basename(old)
        
        # determine new relative path from current file's dir
        current_dir = os.path.dirname(filepath)
        new_abs = os.path.join(base_dir, new)
        rel_new = os.path.relpath(new_abs, current_dir).replace('\\', '/')
        
        # Regex to find links that are exactly (basename) or (./basename)
        content = re.sub(r'\]\((?:\./)?' + re.escape(basename) + r'\)', ']' + f'({rel_new})', content)

    if content != orig_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Updated {filepath}")

for root, dirs, files in os.walk(base_dir):
    for f in files:
        if f.endswith('.md'):
            replace_in_file(os.path.join(root, f))

print("Link update complete.")
