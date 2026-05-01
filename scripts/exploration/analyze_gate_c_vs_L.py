"""
Cross-L analysis: map Gate C (RG semigroup ratio) and diagonal M_aa values
across L ∈ {24, 32, 48, 64} to characterize the L-dependence of FTD-0112's
nonlinear-regime EFT.

Inputs:
  engine/results/s_eff_nonlinear_2026-04-29/L{24,32,48,64}_prod_T0.100_LARGE/
    M_ab.csv, M_ab_stderr.csv, rg_semigroup.txt, run.log

Outputs:
  - tabular Gate C, JJ exactness, J4 exactness, stateSq deviation, etc.
    per lattice size
  - markdown summary table (stdout)
"""

from __future__ import annotations
import csv
import math
import os
from pathlib import Path

ROOT = Path("engine/results/s_eff_nonlinear_2026-04-29")
LATTICE_SIZES = [24, 32, 48, 64, 128]
ACTIVE_OPS = [
    "JJ", "divJ2", "curlJ2", "JdotDivJ", "J4",
    "stateSq", "reactionDensity",
    "genesisFlux", "JdotDeltaS",
]
SPATIAL = ["JJ", "divJ2", "curlJ2", "JdotDivJ", "J4"]
DENSITY = ["stateSq", "reactionDensity"]
REACTION_FLUX = ["genesisFlux", "JdotDeltaS"]


def parse_csv(path):
    rows = list(csv.DictReader(open(path)))
    out = {}
    for r in rows:
        try:
            out[(r["op_a"], r["op_b"])] = float(r["value"])
        except (ValueError, KeyError):
            pass
    return out


def parse_rg_ratio(path):
    if not path.exists():
        return None
    for line in path.read_text().splitlines():
        if line.startswith("rg_ratio"):
            return float(line.split()[1])
    return None


def gate_a_diag_pass(m, e):
    """Count diagonals with bootstrap-stderr/|M| < 30%."""
    p = 0
    for op in ACTIVE_OPS:
        v = m.get((op, op))
        s = e.get((op, op))
        if v is None or s is None or math.isnan(v) or math.isnan(s):
            continue
        if abs(v) < 1e-30:
            continue
        if abs(s / v) < 0.30:
            p += 1
    return p


def gate_a_offdiag_count(m, e):
    """Off-diagonal: count entries with bootstrap-stderr/|M| < 30%."""
    p = 0
    total = 0
    for a in ACTIVE_OPS:
        for b in ACTIVE_OPS:
            if a == b:
                continue
            v = m.get((a, b))
            s = e.get((a, b))
            if v is None or s is None or math.isnan(v) or math.isnan(s):
                continue
            if abs(v) < 1e-30:
                continue
            total += 1
            if abs(s / v) < 0.30:
                p += 1
    return p, total


def cross_sector_5sigma(m, e):
    """Count cross-sector entries above 5σ for SPATIAL ↔ REACTION-FLUX."""
    counts = {}
    for direction, As, Bs in [
        ("SP↔SP", SPATIAL, SPATIAL),
        ("SP→RF", SPATIAL, REACTION_FLUX),
        ("RF→SP", REACTION_FLUX, SPATIAL),
        ("SP→D", SPATIAL, DENSITY),
        ("D→SP", DENSITY, SPATIAL),
    ]:
        nz = 0
        tot = 0
        for a in As:
            for b in Bs:
                if a == b:
                    continue
                v = m.get((a, b))
                s = e.get((a, b))
                if v is None or s is None:
                    continue
                if math.isnan(v) or math.isnan(s):
                    continue
                if abs(v) < 1e-30:
                    continue
                tot += 1
                if abs(v) > 5 * s:
                    nz += 1
        counts[direction] = (nz, tot)
    return counts


def main():
    print("=" * 80)
    print("Cross-L analysis: M_ab(b=2) at L ∈ {24, 32, 48, 64}, T=0.100, pair-rich")
    print("=" * 80)
    print()

    print("Diagonal M_aa values across L (theorem-grade entries should be invariant):")
    print()
    header = f"{'op':>17} | " + " | ".join(f"{f'L={L}':>16}" for L in LATTICE_SIZES)
    print(header)
    print("-" * len(header))

    data_by_L = {}
    for L in LATTICE_SIZES:
        path = ROOT / f"L{L}_prod_T0.100_LARGE"
        if not (path / "M_ab.csv").exists():
            data_by_L[L] = None
            continue
        m = parse_csv(path / "M_ab.csv")
        e = parse_csv(path / "M_ab_stderr.csv")
        rg = parse_rg_ratio(path / "rg_semigroup.txt")
        data_by_L[L] = (m, e, rg)

    for op in ACTIVE_OPS:
        cells = []
        for L in LATTICE_SIZES:
            d = data_by_L[L]
            if d is None:
                cells.append("(no data)")
            else:
                m, e, _ = d
                v = m.get((op, op))
                s = e.get((op, op))
                if v is None or math.isnan(v):
                    cells.append("dropped")
                else:
                    cells.append(f"{v:>9.3f}±{s:.3f}")
        print(f"{op:>17} | " + " | ".join(f"{c:>16}" for c in cells))

    print()
    print("Gate C (RG semigroup ratio, threshold 0.30):")
    for L in LATTICE_SIZES:
        d = data_by_L[L]
        if d is None:
            print(f"  L={L}: (no data)")
            continue
        _, _, rg = d
        if rg is None:
            print(f"  L={L}: no rg_semigroup.txt found")
        else:
            verdict = "PASS" if rg < 0.30 else "FAIL"
            print(f"  L={L}: ratio = {rg:.4f}  [{verdict}]")

    print()
    print("Gate A diagonal (≥30% per-entry stderr):")
    for L in LATTICE_SIZES:
        d = data_by_L[L]
        if d is None:
            continue
        m, e, _ = d
        diag = gate_a_diag_pass(m, e)
        off, tot = gate_a_offdiag_count(m, e)
        print(f"  L={L}: diag {diag}/9; off-diag {off}/{tot}")

    print()
    print("Sector decoupling (5σ threshold):")
    print(f"  {'L':>4} | {'SP↔SP':>10} | {'SP→RF':>10} | {'RF→SP':>10} | {'SP→D':>10} | {'D→SP':>10}")
    for L in LATTICE_SIZES:
        d = data_by_L[L]
        if d is None:
            continue
        m, e, _ = d
        cs = cross_sector_5sigma(m, e)
        cells = [
            f"{cs['SP↔SP'][0]}/{cs['SP↔SP'][1]}",
            f"{cs['SP→RF'][0]}/{cs['SP→RF'][1]}",
            f"{cs['RF→SP'][0]}/{cs['RF→SP'][1]}",
            f"{cs['SP→D'][0]}/{cs['SP→D'][1]}",
            f"{cs['D→SP'][0]}/{cs['D→SP'][1]}",
        ]
        print(f"  {L:>4} | " + " | ".join(f"{c:>10}" for c in cells))

    print()
    print("Theorem 1 (JJ = b⁴ = 16) check across L:")
    for L in LATTICE_SIZES:
        d = data_by_L[L]
        if d is None:
            continue
        m, e, _ = d
        v = m.get(("JJ", "JJ"))
        s = e.get(("JJ", "JJ"))
        if v is None:
            continue
        deviation = abs(v - 16.0)
        within = deviation < 5 * s if s and s > 0 else (deviation < 1e-3)
        marker = "✓" if within else "✗"
        print(f"  L={L}: M_JJ = {v:.6f} ± {s:.6f}  (deviation from 16: {deviation:.6f}) {marker}")

    print()
    print("Theorem 2 (J4 = b⁸ = 256) check across L:")
    for L in LATTICE_SIZES:
        d = data_by_L[L]
        if d is None:
            continue
        m, e, _ = d
        v = m.get(("J4", "J4"))
        s = e.get(("J4", "J4"))
        if v is None:
            continue
        deviation = abs(v - 256.0)
        within = deviation < 5 * s if s and s > 0 else (deviation < 1e-3)
        marker = "✓" if within else "✗"
        print(f"  L={L}: M_J4 = {v:.4f} ± {s:.4f}  (deviation from 256: {deviation:.4f}) {marker}")


if __name__ == "__main__":
    main()
