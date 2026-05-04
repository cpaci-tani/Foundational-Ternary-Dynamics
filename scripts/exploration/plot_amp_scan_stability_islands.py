"""Plot full-physics amp scan: log(n_total) vs A, marking stability islands.

Output: dissemination/interactive/full_physics_amp_scan.png
"""
from __future__ import annotations
import json
import math
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(r"C:/Users/cpaci/Desktop/ftd")
JSON = ROOT / "full_amp_scan.json"
OUT = ROOT / "dissemination" / "interactive" / "full_physics_amp_scan.png"


def main() -> None:
    runs = json.loads(JSON.read_text())["runs"]
    A = [r["A"] for r in runs]
    n = [r["n_total"] for r in runs]
    nR = [r["n_R"] for r in runs]
    nG = [r["n_G"] for r in runs]
    nB = [r["n_B"] for r in runs]
    matter = [r["n_matter"] for r in runs]
    anti = [r["n_antimatter"] for r in runs]

    fig, axes = plt.subplots(3, 1, figsize=(11, 11), sharex=True)

    ax = axes[0]
    log_n = [math.log10(max(v, 0.5)) for v in n]
    ax.plot(A, log_n, "o-", color="#222")
    ax.axhline(math.log10(0.9 * 64**3), ls=":", color="#aa0000",
               label=f"flooding ~90% of L={64}^3")
    for A_isl, label in [(9.0, "Island 1"), (9.5, ""), (13.0, "Island 2")]:
        ax.axvline(A_isl, color="#0066aa", alpha=0.35, lw=1.2)
        if label:
            ax.text(A_isl, log_n[A.index(A_isl)] + 0.3, label,
                    color="#0066aa", fontsize=9, ha="center")
    ax.set_ylabel("log10(n_total)")
    ax.set_title("Full-physics amplitude scan at L=64 (single seed, 200 ticks, +x flux)")
    ax.legend(loc="upper left", fontsize=9)
    ax.grid(True, alpha=0.3)

    ax = axes[1]
    ax.plot(A, nR, "o-", color="#cc2222", label="R")
    ax.plot(A, nG, "s-", color="#22aa44", label="G")
    ax.plot(A, nB, "^-", color="#2266cc", label="B")
    for A_isl in (9.0, 9.5, 13.0):
        ax.axvline(A_isl, color="#0066aa", alpha=0.35, lw=1.2)
    ax.set_ylabel("color count")
    ax.set_yscale("symlog", linthresh=10)
    ax.legend(loc="upper left", fontsize=9)
    ax.grid(True, alpha=0.3)

    ax = axes[2]
    ratio = []
    for r in runs:
        m = r["n_matter"]
        a = r["n_antimatter"]
        tot = m + a
        ratio.append(m / tot if tot else 0.5)
    ax.plot(A, ratio, "o-", color="#552288")
    ax.axhline(0.5, ls=":", color="#888")
    ax.axhline(0.7, ls=":", color="#0066aa", label="islands ~7:3")
    for A_isl in (9.0, 9.5, 13.0):
        ax.axvline(A_isl, color="#0066aa", alpha=0.35, lw=1.2)
    ax.set_xlabel("A / K_GENESIS")
    ax.set_ylabel("matter / (matter + anti)")
    ax.set_ylim(0.3, 1.05)
    ax.legend(loc="upper right", fontsize=9)
    ax.grid(True, alpha=0.3)

    fig.tight_layout()
    OUT.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(OUT, dpi=140)
    print(f"wrote {OUT}")


if __name__ == "__main__":
    main()
