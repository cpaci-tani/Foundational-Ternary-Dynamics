"""Analyze multi-seed × multi-L string verification.

Tests two predictions:
  A. String lengths {R=4, G=2, B=3} are seed-independent at L=32
  B. String lengths are L-invariant across L in {32, 48, 64}

If both hold, the irrep-multiplicity interpretation gets [STRONGLY MOTIVATED CONJECTURE].
"""
from __future__ import annotations
import json
from pathlib import Path
from collections import defaultdict
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

JSON = Path(r"C:/Users/cpaci/Desktop/ftd/string_verify.json")
OUTDIR = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/quark_pngs")

COLOR_BY_AXIS = {"x": "R", "y": "G", "z": "B"}
COLOR_HEX = {"R": "#e53935", "G": "#43a047", "B": "#1e88e5"}
PREDICTED = {"R": 4, "G": 2, "B": 3}
IRREP_LABEL = {"R": "mult(A_{1g})=4", "G": "mult(E_g)=2", "B": "mult(T_{1u})=3"}

def main():
    with JSON.open() as f:
        data = json.load(f)
    runs = data["runs"]
    print(f"Loaded {len(runs)} runs")

    # Index by (L, axis)
    by_LA = defaultdict(list)
    for r in runs:
        by_LA[(r["L"], r["axis"])].append(r)

    # Print verification table
    print("\n" + "=" * 70)
    print("  STRING-LENGTH VERIFICATION (n_total per axis × L × seed)")
    print("=" * 70)
    print(f"{'axis':<6}{'predicted':<12}", end="")
    for L in [32, 48, 64]:
        print(f"L={L} (3 seeds)".ljust(22), end="")
    print()
    print("-" * 70)
    for axis in ["x", "y", "z"]:
        col = COLOR_BY_AXIS[axis]
        pred = PREDICTED[col]
        print(f"{axis} ({col}){'':<6}n={pred:<8}", end="")
        for L in [32, 48, 64]:
            items = by_LA[(L, axis)]
            ns = sorted(r["n_total"] for r in items)
            print(f"  {ns}".ljust(22), end="")
        print()
    print()

    # Color counts per run (where do colors actually go?)
    print("\n" + "=" * 70)
    print("  COLOR CONTENT (n_R, n_G, n_B per run)")
    print("=" * 70)
    print(f"{'L':<5}{'axis':<6}{'seed':<6}{'n_total':<10}{'R':<5}{'G':<5}{'B':<5}{'matter':<8}{'antimatter':<10}")
    print("-" * 70)
    for r in runs:
        print(f"{r['L']:<5}{r['axis']:<6}{r['seed']:<6}{r['n_total']:<10}"
              f"{r['color_R']:<5}{r['color_G']:<5}{r['color_B']:<5}"
              f"{r['n_matter']:<8}{r['n_antimatter']:<10}")
    print()

    # Verdict on Prediction A (seed-independence at L=32)
    print("\n--- Prediction A: seed-independence at L=32 ---")
    pred_A_pass = True
    for axis in ["x", "y", "z"]:
        col = COLOR_BY_AXIS[axis]
        items = by_LA[(32, axis)]
        ns = [r["n_total"] for r in items]
        seed_consistent = (max(ns) - min(ns) <= 1)  # tolerate ±1
        matches_predicted = (PREDICTED[col] in ns and abs(np.mean(ns) - PREDICTED[col]) <= 1)
        status = "PASS" if (seed_consistent and matches_predicted) else "FAIL"
        if status == "FAIL": pred_A_pass = False
        print(f"  {axis} ({col}): seeds give n in {sorted(ns)}, "
              f"predicted={PREDICTED[col]}  [{status}]")

    # Verdict on Prediction B (L-invariance)
    print("\n--- Prediction B: L-invariance across {32, 48, 64} ---")
    pred_B_pass = True
    for axis in ["x", "y", "z"]:
        col = COLOR_BY_AXIS[axis]
        means = []
        for L in [32, 48, 64]:
            items = by_LA[(L, axis)]
            ns = [r["n_total"] for r in items]
            means.append(np.mean(ns))
        L_consistent = (max(means) - min(means) <= 2)  # tolerate ±2 across L
        status = "PASS" if L_consistent else "FAIL"
        if status == "FAIL": pred_B_pass = False
        print(f"  {axis} ({col}): mean n at L=32/48/64 = {means[0]:.1f}/{means[1]:.1f}/{means[2]:.1f}  [{status}]")

    print("\n--- Combined verdict ---")
    if pred_A_pass and pred_B_pass:
        print("  Both predictions PASS. Irrep-multiplicity interpretation is structurally")
        print("  supported. Tag candidate: [STRONGLY MOTIVATED CONJECTURE].")
    elif pred_A_pass:
        print("  Prediction A PASSES (seed-independent) but B FAILS (L-dependent).")
        print("  String lengths are deterministic at L=32 but not L-invariant — finite-size effect.")
    elif pred_B_pass:
        print("  Prediction A FAILS (seed-dependent) but B PASSES (L-invariant on average).")
        print("  Stochastic at fixed L but bulk pattern is L-invariant. Need M-seed averaging.")
    else:
        print("  BOTH PREDICTIONS FAIL. The irrep-multiplicity reading is falsified.")
        print("  String lengths depend on both seed and L — finite-size + stochastic, not structural.")

    # Render comparison plot
    render_comparison(runs)

def render_comparison(runs):
    """Bar chart: n_total per (axis, L) with seed spread."""
    fig, ax = plt.subplots(figsize=(11, 6))
    width = 0.25
    x_positions = np.arange(3)  # x, y, z

    for i, L in enumerate([32, 48, 64]):
        for j, axis in enumerate(["x", "y", "z"]):
            col = COLOR_BY_AXIS[axis]
            items = [r for r in runs if r["L"] == L and r["axis"] == axis]
            ns = [r["n_total"] for r in items]
            x_pos = x_positions[j] + (i - 1) * width
            mean_n = np.mean(ns)
            ax.bar(x_pos, mean_n, width=width, color=COLOR_HEX[col],
                   edgecolor="black", alpha=0.6 + 0.2*i,
                   label=f"L={L}" if j == 0 else None)
            for n in ns:
                ax.scatter([x_pos], [n], color="black", s=30, zorder=5)
            ax.text(x_pos, max(ns) + 0.4, f"{ns}", ha="center", fontsize=7)

    # Predicted lines
    for j, axis in enumerate(["x", "y", "z"]):
        col = COLOR_BY_AXIS[axis]
        pred = PREDICTED[col]
        ax.plot([x_positions[j] - 0.4, x_positions[j] + 0.4], [pred, pred],
                color=COLOR_HEX[col], linewidth=2, linestyle="--",
                alpha=0.8)

    ax.set_xticks(x_positions)
    ax.set_xticklabels([f"+x → R\nmult(A₁g)=4", f"+y → G\nmult(Eg)=2", f"+z → B\nmult(T₁u)=3"])
    ax.set_ylabel("String length (n_total at t=300)")
    ax.set_title("String-length verification: 3 seeds × 3 L values × 3 axes\n"
                 "Dashed lines = O_h irrep multiplicity prediction; bars = mean per L; dots = individual seeds")
    ax.legend(loc="upper right")
    ax.set_ylim(0, max(8, max(r["n_total"] for r in runs) + 2))
    plt.tight_layout()
    out = OUTDIR / "string_verification.png"
    plt.savefig(out, dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"\nWrote: {out}")

if __name__ == "__main__":
    main()
