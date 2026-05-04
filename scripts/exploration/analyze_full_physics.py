"""Compare full-physics vs +color+triad-only string lengths.

Hypothesis: under complete physics (all force toggles ON + Langevin baseline),
the engine's emergent behavior may differ qualitatively from the stripped-down
+color+triad results. Specifically:
  - Strings may have different lengths (true emergent particle-like content)
  - Pair production toggle may produce richer matter+antimatter structures
  - Strong + color + exchange forces together may produce confinement-like
    binding rather than runaway flooding
"""
from __future__ import annotations
import json
from pathlib import Path
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

FULL = Path(r"C:/Users/cpaci/Desktop/ftd/full_physics.json")
TRIAD = Path(r"C:/Users/cpaci/Desktop/ftd/string_verify.json")
OUTDIR = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/quark_pngs")

COLOR_HEX = {"x": "#e53935", "y": "#43a047", "z": "#1e88e5"}
COLOR_BY_AXIS = {"x": "R", "y": "G", "z": "B"}
FTD_LABELS = {1: "1", 2: "mult(E_g)=2", 3: "N_c=3", 4: "N_base=4",
              7: "b_3=7", 13: "N_eff=13"}

def main():
    if not FULL.exists():
        print(f"Full physics data not yet available at {FULL}")
        return
    with FULL.open() as f:
        full_runs = json.load(f).get("runs", [])
    with TRIAD.open() as f:
        triad_runs = json.load(f).get("runs", [])
    print(f"Full physics: {len(full_runs)} runs")
    print(f"+color+triad only: {len(triad_runs)} runs")

    if not full_runs:
        print("No full-physics runs completed (likely toggle validation failed). Check progress log.")
        return

    print("\n" + "=" * 80)
    print("  FULL PHYSICS vs +color+triad-ONLY: string-length comparison")
    print("=" * 80)
    print(f"{'L':<5}{'axis':<8}{'+color+triad':<22}{'FULL PHYSICS':<22}{'change':<15}")
    print("-" * 80)
    for L in sorted(set(r["L"] for r in full_runs + triad_runs)):
        for axis in ["x", "y", "z"]:
            triad_ns = sorted([r["n_total"] for r in triad_runs if r["L"] == L and r["axis"] == axis])
            full_ns = sorted([r["n_total"] for r in full_runs if r["L"] == L and r["axis"] == axis])
            if not triad_ns and not full_ns: continue
            triad_str = str(triad_ns) if triad_ns else "—"
            full_str = str(full_ns) if full_ns else "—"
            change = ""
            if triad_ns and full_ns:
                t_mean = np.mean(triad_ns); f_mean = np.mean(full_ns)
                if abs(t_mean - f_mean) < 0.5:
                    change = "same"
                else:
                    change = f"{f_mean - t_mean:+.1f}"
            print(f"{L:<5}{axis} ({COLOR_BY_AXIS[axis]}){'':<3}{triad_str:<22}{full_str:<22}{change:<15}")

    # Color content summary
    print("\n--- Color content under full physics ---")
    print(f"{'L':<5}{'axis':<6}{'seed':<6}{'n':<6}{'R':<5}{'G':<5}{'B':<5}{'colorless':<10}{'matter':<8}{'antimatter':<10}")
    for r in full_runs:
        print(f"{r['L']:<5}{r['axis']:<6}{r['seed']:<6}{r['n_total']:<6}"
              f"{r['color_R']:<5}{r['color_G']:<5}{r['color_B']:<5}"
              f"{r['color_none']:<10}{r['n_matter']:<8}{r['n_antimatter']:<10}")

    # Render comparison
    render_comparison(full_runs, triad_runs)

def render_comparison(full, triad):
    fig, axes_arr = plt.subplots(1, 3, figsize=(15, 5))
    Ls = sorted(set(r["L"] for r in full + triad))
    for i, axis in enumerate(["x", "y", "z"]):
        ax = axes_arr[i]
        col = COLOR_HEX[axis]
        for L in Ls:
            triad_ns = [r["n_total"] for r in triad if r["L"] == L and r["axis"] == axis]
            full_ns = [r["n_total"] for r in full if r["L"] == L and r["axis"] == axis]
            x_t = L - 1; x_f = L + 1
            if triad_ns:
                ax.scatter([x_t]*len(triad_ns), triad_ns, color=col, marker="o", s=80,
                           alpha=0.6, edgecolor="black", label="+color+triad" if L == Ls[0] else None)
            if full_ns:
                ax.scatter([x_f]*len(full_ns), full_ns, color=col, marker="s", s=80,
                           alpha=0.9, edgecolor="black",
                           label="full physics" if L == Ls[0] else None)
        for n_fti in [1, 2, 3, 4, 7, 13]:
            ax.axhline(n_fti, linestyle=":", alpha=0.3, color="gray")
        ax.set_xlabel("L")
        ax.set_ylabel("n_total")
        ax.set_title(f"+{axis} flux -> {COLOR_BY_AXIS[axis]}")
        ax.legend(loc="upper right")
        ax.grid(alpha=0.2)
    plt.suptitle("FULL PHYSICS vs +color+triad ONLY\n"
                 "Circles = +color+triad only; Squares = full physics (all toggles)")
    plt.tight_layout()
    out = OUTDIR / "full_vs_triad_strings.png"
    plt.savefig(out, dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"\nWrote: {out}")

if __name__ == "__main__":
    main()
