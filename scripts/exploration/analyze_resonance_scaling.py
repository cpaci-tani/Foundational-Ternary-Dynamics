"""Analyze resonance scaling across L=32, L=48, L=64 maps.

Reads the three resonance map output files and produces:
- Unified resonance window table per L
- L-invariance scoring per cluster size n
- Correlation between A_resonance(L) and L (linear regression)

Usage: python analyze_resonance_scaling.py
"""
from __future__ import annotations
import re
import os
from pathlib import Path

OUTDIR = Path(r"C:/Users/cpaci/AppData/Local/Temp/claude/C--Users-cpaci-Desktop-ftd-engine/0e8da311-3a0a-4cbc-8baf-b24658340879/tasks")

L_FILES = {
    32: "b8h2wmyi8.output",
    48: "boz87i0le.output",
    64: "b0a16gmtp.output",
}

def parse_map(path: Path):
    """Parse a resonance map output. Return list of (A, n_init, n_final, regime)."""
    if not path.exists():
        return []
    rows = []
    with path.open() as f:
        for line in f:
            # Match pattern like: "   3.00         1       1         1       none      none    0.00    BOUND-trivial"
            m = re.match(
                r'\s+(\d+\.\d+)\s+(\d+)\s+(\S+)\s+(\d+)\s+(\S+)\s+(\S+)\s+(\d+\.\d+)\s+(\S+)',
                line,
            )
            if m:
                A = float(m.group(1))
                n_init = int(m.group(2))
                n_final = int(m.group(4))
                regime = m.group(8)
                rows.append((A, n_init, n_final, regime))
    return rows

def main():
    all_data = {}
    for L, fname in L_FILES.items():
        path = OUTDIR / fname
        all_data[L] = parse_map(path)
        print(f"L={L}: {len(all_data[L])} amplitudes parsed from {fname}")
    print()

    # Find all stable (A, n) pairs per L
    stable_per_L = {}
    for L, rows in all_data.items():
        stable_per_L[L] = [(A, n) for (A, _, n, regime) in rows
                           if regime in ("STABLE", "BOUND-trivial") and n > 0]

    # Print tables
    for L, stable in stable_per_L.items():
        print(f"--- L={L} stable (A, n) pairs ---")
        for A, n in stable:
            print(f"  A={A:.2f}  ->  n={n}")
        print()

    # FTD framework integer matches
    fti = {3: "N_c", 4: "N_base", 7: "b_3", 13: "N_eff"}
    print("=== FTD framework integer L-invariance ===")
    for n_target, label in fti.items():
        windows_per_L = {}
        for L, stable in stable_per_L.items():
            wins = [A for (A, n) in stable if n == n_target]
            windows_per_L[L] = wins
        appears = sum(1 for L in L_FILES if windows_per_L.get(L))
        status = "YES L-INVARIANT" if appears == len(L_FILES) else (
            "weakly L-invariant" if appears == 2 else "L-specific" if appears == 1 else "absent"
        )
        print(f"\n  {label} = {n_target}: {status} (appears at {appears}/{len(L_FILES)} L values)")
        for L in sorted(L_FILES.keys()):
            w = windows_per_L.get(L, [])
            if w:
                print(f"    L={L}: A in {w}")
            else:
                print(f"    L={L}: NOT present")

    # All n-values that appear at >=2 L values
    print("\n\n=== All cluster sizes with multi-L appearance ===")
    all_n = set()
    for stable in stable_per_L.values():
        for _, n in stable:
            all_n.add(n)
    for n in sorted(all_n):
        Ls_with_n = []
        for L, stable in stable_per_L.items():
            if any(nn == n for (_, nn) in stable):
                Ls_with_n.append(L)
        if len(Ls_with_n) >= 2:
            label = fti.get(n, "")
            print(f"  n={n:3d}  appears at L in {Ls_with_n}  {label}")

if __name__ == "__main__":
    main()
