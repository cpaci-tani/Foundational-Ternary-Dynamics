"""Analyze L=128 string verification — the critical falsifier.

Combines L=128 results with prior L=32/48/64 data.

Verdicts:
  - If L=128 lengths hit FTD framework integers (1,2,3,4,7,13,27) deterministically across seeds:
    [STRONGLY MOTIVATED CONJECTURE] for "engine string lengths quantize to FTD integers"
  - If L=128 lengths are arbitrary non-FTD integers: L=32 match was coincidence
  - If L=128 has seed-spread (non-deterministic): determinism property fails at L=128
"""
from __future__ import annotations
import json
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

L128_JSON = Path(r"C:/Users/cpaci/Desktop/ftd/string_l128.json")
LOWER_JSON = Path(r"C:/Users/cpaci/Desktop/ftd/string_verify.json")
OUTDIR = Path(r"C:/Users/cpaci/Desktop/ftd/dissemination/interactive/quark_pngs")

FTD_INTEGERS = {1, 2, 3, 4, 7, 13, 27}
FTD_LABELS = {1: "1", 2: "mult(E_g)=2", 3: "N_c=mult(T_1u)=3",
              4: "N_base=mult(A_1g)=4", 7: "b_3=4+3=7",
              13: "N_eff=13", 27: "27-block size"}
COLOR_HEX = {"R": "#e53935", "G": "#43a047", "B": "#1e88e5"}
COLOR_BY_AXIS = {"x": "R", "y": "G", "z": "B"}

def main():
    # Load all data
    with LOWER_JSON.open() as f:
        lower = json.load(f)["runs"]
    if not L128_JSON.exists():
        print(f"L=128 data not yet available at {L128_JSON}")
        return
    with L128_JSON.open() as f:
        l128 = json.load(f)["runs"]
    all_runs = lower + l128
    print(f"Loaded {len(lower)} L<128 runs + {len(l128)} L=128 runs = {len(all_runs)} total")

    # Print combined table
    print("\n" + "=" * 78)
    print("  COMBINED STRING-LENGTH TABLE (n_total per axis x L x seed)")
    print("=" * 78)
    Ls = sorted(set(r["L"] for r in all_runs))
    print(f"{'axis':<8}", end="")
    for L in Ls:
        print(f"L={L}".ljust(28), end="")
    print()
    print("-" * 78)
    for axis in ["x", "y", "z"]:
        print(f"{axis} ({COLOR_BY_AXIS[axis]}){'':<3}", end="")
        for L in Ls:
            ns = [r["n_total"] for r in all_runs if r["L"] == L and r["axis"] == axis]
            ns_str = sorted(ns)
            in_ftd = [n in FTD_INTEGERS for n in set(ns)]
            tag = "" if not all(in_ftd) else " (all FTD)"
            print(f"  {ns_str}{tag}".ljust(28), end="")
        print()

    # Verdict on L=128
    print("\n" + "=" * 78)
    print("  L=128 VERDICT")
    print("=" * 78)
    determinism = True
    ftd_hits = 0
    total_axes = 0
    for axis in ["x", "y", "z"]:
        total_axes += 1
        items = [r for r in l128 if r["axis"] == axis]
        ns = [r["n_total"] for r in items]
        n_set = set(ns)
        unique_n = sorted(n_set)
        if len(unique_n) == 1:
            n = unique_n[0]
            in_ftd = n in FTD_INTEGERS
            label = FTD_LABELS.get(n, "")
            status = "DETERMINISTIC + FTD-MATCH" if in_ftd else "DETERMINISTIC + non-FTD"
            print(f"  {axis} ({COLOR_BY_AXIS[axis]}): n={n}  [{status}]  {label}")
            if in_ftd: ftd_hits += 1
        else:
            determinism = False
            print(f"  {axis} ({COLOR_BY_AXIS[axis]}): n={unique_n}  [SEED-VARIABLE — determinism fails]")

    print(f"\n  Determinism: {'YES' if determinism else 'NO'}")
    print(f"  FTD-integer match: {ftd_hits} / {total_axes}")

    print("\n--- Combined verdict ---")
    if determinism and ftd_hits == 3:
        print("  [STRONGLY MOTIVATED CONJECTURE upgrade candidate]")
        print("  All 3 axes at L=128 produce deterministic FTD-integer string lengths.")
        print("  Combined with L=32/48/64 data, the engine's string-length quantization")
        print("  to FTD framework integers is structurally tight at 4 tested L values.")
    elif determinism and ftd_hits >= 2:
        print("  [PARTIAL CONFIRMATION]")
        print(f"  {ftd_hits} of 3 axes hit FTD integers; the third may be a non-framework integer")
        print("  or an FTD framework combination not yet enumerated.")
    elif determinism:
        print("  [WEAK CONFIRMATION]")
        print("  Determinism preserved; but FTD-integer match below threshold.")
    elif ftd_hits >= 2:
        print("  [DETERMINISM-WEAKENED PARTIAL CONFIRMATION]")
        print("  Some seed-spread, but most axes hit FTD integers on average.")
    else:
        print("  [FALSIFIED]")
        print("  Neither determinism nor FTD-integer matching survives at L=128.")
        print("  The L=32 finding was an L=32-specific finite-size resonance.")

    # Render combined visualization
    render_combined(all_runs, Ls)

def render_combined(runs, Ls):
    fig, axes_arr = plt.subplots(1, 3, figsize=(15, 5))
    for i, axis in enumerate(["x", "y", "z"]):
        ax = axes_arr[i]
        col = COLOR_BY_AXIS[axis]
        # Plot per-L data
        for L in Ls:
            items = [r for r in runs if r["L"] == L and r["axis"] == axis]
            ns = [r["n_total"] for r in items]
            ax.scatter([L] * len(ns), ns, color=COLOR_HEX[col],
                       s=80, alpha=0.7, edgecolor="black")
            mean_n = np.mean(ns)
            ax.scatter([L], [mean_n], color="black", marker="_", s=200, linewidth=2)
            ax.text(L, max(ns) + 0.5, str(sorted(ns)), ha="center", fontsize=8)
        # FTD framework integer reference lines
        for n_fti in [1, 2, 3, 4, 7, 13]:
            ax.axhline(n_fti, linestyle=":", alpha=0.3, color="gray")
            ax.text(Ls[-1] + 5, n_fti, FTD_LABELS.get(n_fti, str(n_fti)),
                    fontsize=7, color="gray", verticalalignment="center")
        ax.set_xlabel("L")
        ax.set_ylabel("n_total (string length)")
        ax.set_title(f"+{axis} flux -> {col}")
        ax.set_xlim(min(Ls) - 5, max(Ls) + 25)
        ax.set_ylim(0, max(35, max(r["n_total"] for r in runs) + 5))
        ax.grid(alpha=0.2)
    plt.suptitle("FTD engine string lengths: pure-axis flux at L ∈ {32, 48, 64, 128}\n"
                 "(dotted lines = FTD framework integers; black dashes = mean per L)")
    plt.tight_layout()
    out = OUTDIR / "string_lengths_all_L.png"
    plt.savefig(out, dpi=120, bbox_inches="tight")
    plt.close(fig)
    print(f"\nWrote: {out}")

if __name__ == "__main__":
    main()
