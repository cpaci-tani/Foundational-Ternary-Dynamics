import os
import shutil

base_dir = r"c:\Users\cpaci\Desktop\ftd\docs\theory"

# Move dictionary: old_path relative to base_dir -> new_path relative to base_dir
moves = {}

def get_files(subdir):
    dir_path = os.path.join(base_dir, subdir)
    if not os.path.exists(dir_path): return []
    return [f for f in os.listdir(dir_path) if f.endswith('.md') and not f.startswith('INDEX_')]

# 03_derivations mapping
deriv_files = get_files("03_derivations")
for f in deriv_files:
    if f.startswith("AUDIT_"):
        moves[f"03_derivations/{f}"] = f"07_assessment/audits/{f}"
    elif any(k in f for k in ["SU2", "SU3", "HIGGS", "CHIRAL", "PION", "FERMI", "THREE_GENERATIONS", "YUKAWA", "NC_", "GAUGE", "QCD", "Z3"]):
        moves[f"03_derivations/{f}"] = f"03_derivations/standard_model/{f}"
    elif any(k in f for k in ["EINSTEIN", "BLACK_HOLE", "DARK", "GRAVITY", "COSMIC", "NEWTON", "DIFFEROMORPHISM", "STELLAR", "SCALE", "RELATIVITY"]):
        moves[f"03_derivations/{f}"] = f"03_derivations/gravity_and_cosmology/{f}"
    elif any(k in f for k in ["COULOMB", "QED", "LIENARD", "HODGE", "LW", "EM_", "MAGNETIC", "STATE_FLUX"]):
        moves[f"03_derivations/{f}"] = f"03_derivations/electromagnetism/{f}"
    elif any(k in f for k in ["BELL", "BORN", "DIRAC", "QM", "QUANTUM", "SINGLET", "PATH_INTEGRAL", "SPIN", "MEASUREMENT", "NONCOMMUTATIVE", "OBSERVER"]):
        moves[f"03_derivations/{f}"] = f"03_derivations/quantum_mechanics/{f}"
    else:
        moves[f"03_derivations/{f}"] = f"03_derivations/foundational_mechanics/{f}"

# 09_mathematical mapping
math_files = get_files("09_mathematical")
for f in math_files:
    if f.startswith("AUDIT_"):
        moves[f"09_mathematical/{f}"] = f"07_assessment/audits/{f}"
    elif any(k in f for k in ["LFUNCTION", "RIEMANN", "CM_", "EULER", "INTEGER", "CONSTANT", "CHOWLA", "HEEGNER", "TOWER", "LVALUE", "PERIOD", "GSTAR", "PI", "ZETA", "ALGEBRAIC", "PRIME"]):
        moves[f"09_mathematical/{f}"] = f"09_mathematical/number_theory/{f}"
    elif any(k in f for k in ["BIVECTOR", "CAYLEY", "FOURCIER", "CLIFFORD", "WALSH", "HADAMARD", "DIRAC_KAHLER", "TERNARY_MATRIX", "WH_"]):
        moves[f"09_mathematical/{f}"] = f"09_mathematical/algebra/{f}"
    elif any(k in f for k in ["FQCR", "QUOTIENT", "OBSERVER_TESTS", "QUARTER"]):
        moves[f"09_mathematical/{f}"] = f"09_mathematical/fqcr_program/{f}"
    else:
        moves[f"09_mathematical/{f}"] = f"09_mathematical/general_math/{f}"

# 10_eft_program mapping
eft_files = get_files("10_eft_program")
for f in eft_files:
    if f.startswith("REF_"):
        continue # Leave in root
    elif f.startswith("AUDIT_"):
        moves[f"10_eft_program/{f}"] = f"07_assessment/audits/{f}"
    elif f.startswith("PREREG_"):
        moves[f"10_eft_program/{f}"] = f"10_eft_program/preregistrations/{f}"
    elif f.startswith("DERIV_") or f.startswith("FOUND_"):
        moves[f"10_eft_program/{f}"] = f"10_eft_program/derivations/{f}"
    elif f.startswith("SCOPE_") or f.startswith("SPEC_") or f.startswith("OPEN_"):
        moves[f"10_eft_program/{f}"] = f"10_eft_program/scopes_and_specs/{f}"
    elif f.startswith("REPORT_") or f.startswith("STATUS_") or f.startswith("RETROSPECTIVE_") or f.startswith("ANALYSIS_"):
        moves[f"10_eft_program/{f}"] = f"10_eft_program/reports_and_audits/{f}"
    elif f.startswith("THEOREM_"):
        moves[f"10_eft_program/{f}"] = f"10_eft_program/derivations/{f}"
    else:
        moves[f"10_eft_program/{f}"] = f"10_eft_program/general/{f}"

# Ensure directories exist
for old, new in moves.items():
    new_abs = os.path.join(base_dir, new)
    os.makedirs(os.path.dirname(new_abs), exist_ok=True)

# Actually move the files via git mv or regular move (if not in git)
import subprocess

for old, new in moves.items():
    old_abs = os.path.join(base_dir, old)
    new_abs = os.path.join(base_dir, new)
    if os.path.exists(old_abs):
        # We use git mv to preserve history
        subprocess.run(["git", "mv", old_abs, new_abs], cwd=base_dir, check=False)

print("Files moved.")

# Now rename 06_reference frame context/ to 06_reference_frames_and_measurement/
# We need to use git mv for the directory
subprocess.run(["git", "mv", "06_reference frame context", "06_reference_frames_and_measurement"], cwd=base_dir, check=False)

# Rename internal files
# Also update 07_assessment to have core_ledgers and campaigns
os.makedirs(os.path.join(base_dir, "07_assessment/core_ledgers"), exist_ok=True)
os.makedirs(os.path.join(base_dir, "07_assessment/campaigns"), exist_ok=True)
subprocess.run(["git", "mv", "07_assessment/LEDGER.md", "07_assessment/core_ledgers/LEDGER.md"], cwd=base_dir, check=False)
subprocess.run(["git", "mv", "07_assessment/TRACKER_ONTIC_TRUTH.md", "07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md"], cwd=base_dir, check=False)
subprocess.run(["git", "mv", "07_assessment/TRACKER_OPEN_ITEMS.md", "07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md"], cwd=base_dir, check=False)

if os.path.exists(os.path.join(base_dir, "07_assessment/reframe_deployment")):
    subprocess.run(["git", "mv", "07_assessment/reframe_deployment", "07_assessment/campaigns/reframe_deployment"], cwd=base_dir, check=False)
if os.path.exists(os.path.join(base_dir, "07_assessment/archive_session_outputs")):
    subprocess.run(["git", "mv", "07_assessment/archive_session_outputs", "07_assessment/campaigns/archive_session_outputs"], cwd=base_dir, check=False)

# Generate a unified list of moves for the link updater
moves["07_assessment/LEDGER.md"] = "07_assessment/core_ledgers/LEDGER.md"
moves["07_assessment/TRACKER_ONTIC_TRUTH.md"] = "07_assessment/core_ledgers/TRACKER_ONTIC_TRUTH.md"
moves["07_assessment/TRACKER_OPEN_ITEMS.md"] = "07_assessment/core_ledgers/TRACKER_OPEN_ITEMS.md"

# Also account for the directory rename in our link updates
# Any link to 06_reference frame context/ needs to become 06_reference_frames_and_measurement/
import re

def update_links():
    all_md_files = []
    for root, dirs, files in os.walk(base_dir):
        for f in files:
            if f.endswith('.md'):
                all_md_files.append(os.path.join(root, f))
    
    for filepath in all_md_files:
        try:
            with open(filepath, 'r', encoding='utf-8') as f:
                content = f.read()
            
            orig_content = content
            
            # Directory rename 06_reference frame context -> 06_reference_frames_and_measurement
            content = content.replace("06_reference frame context", "06_reference_frames_and_measurement")
            
            # For each file move, update links
            for old, new in moves.items():
                # simple string replacement for absolute-ish paths
                content = content.replace("(" + old + ")", "(" + new + ")")
                
                # Also replace the basename if it's referenced directly in same dir
                basename = os.path.basename(old)
                current_dir = os.path.dirname(filepath)
                new_abs = os.path.join(base_dir, new)
                
                # Use relpath
                # if current_dir == base_dir, relpath is just 'new'
                try:
                    rel_new = os.path.relpath(new_abs, current_dir).replace('\\', '/')
                    # Replace (basename) or (./basename)
                    content = re.sub(r'\]\((?:\./)?' + re.escape(basename) + r'\)', ']' + f'({rel_new})', content)
                except ValueError:
                    pass

            if content != orig_content:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(content)
                print(f"Updated links in {filepath}")
        except Exception as e:
            print(f"Failed to process {filepath}: {e}")

update_links()
print("Link update complete.")

