"""fig_correlation_tiers.py — figure for the three ceilings (paper SS7.5).

CLAIM: the potential is not "whatever is not forbidden".  It is a specific,
tightly constrained subset of the consistent, and the constraint has no
accepted derivation.

THREE PIECES, ALL EXACT:
  (a) the three ceilings 2, 2 sqrt2, 4, and which structure each admits
  (b) certified randomness H_min(S), zero at 2 and exactly one bit at 2 sqrt2
  (c) the commutator price curve, identity vs direct operator norm

CROSS-CHECK: the Landau identity B^2 = 4I - [A0,A1](x)[B0,B1] is verified
numerically before it is used to draw anything.

SCOPE: these are established bounds (Tsirelson; Popescu-Rohrlich) restated.
The figure introduces no new result; its contribution is to locate the
paper's open question inside them.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks

I2 = np.eye(2, dtype=complex)
sx = np.array([[0, 1], [1, 0]], dtype=complex)
sz = np.array([[1, 0], [0, -1]], dtype=complex)
TS = 2.0 * np.sqrt(2.0)




def comm(X, Y):
    return X @ Y - Y @ X


def obs(t):
    return np.cos(t) * sz + np.sin(t) * sx


def chsh(A0, A1, B0, B1):
    return np.kron(A0, B0 + B1) + np.kron(A1, B0 - B1)


def check_landau(n=500, seed=11):
    """Cross-check the identity before using it to draw anything."""
    rng = np.random.default_rng(seed)
    worst = 0.0
    for _ in range(n):
        a, b, c, d = rng.uniform(0, 2 * np.pi, 4)
        A0, A1, B0, B1 = obs(a), obs(b), obs(c), obs(d)
        B = chsh(A0, A1, B0, B1)
        rhs = 4 * np.kron(I2, I2) - np.kron(comm(A0, A1), comm(B0, B1))
        worst = max(worst, np.abs(B @ B - rhs).max())
    return worst


def hmin(S):
    """Device-independent min-entropy from the CHSH value."""
    pg = 0.5 + 0.5 * np.sqrt(np.clip(2 - S * S / 4, 0.0, None))
    return -np.log2(pg)


# =====================================================================
def panel_tiers(ax):
    rows = [("no-signalling", 4.0, CG, "any consistent\nprobability assignment"),
            ("quantum", TS, CO, "a noncommutative\nweight structure"),
            ("local", 2.0, C1, "pre-assigned values")]
    for i, (nm, v, col, _) in enumerate(rows):
        ax.barh(i, v, height=0.52, color=col, alpha=0.30,
                edgecolor=col, lw=1.2, zorder=2)
        ax.text(v + 0.07, i, f"{v:.4f}", fontsize=FS_ANN, color=CK,
                ha="left", va="center", zorder=5)
    # the unexplained gap, hatched on the outer bar
    ax.barh(0, 4.0 - TS, left=TS, height=0.52, color="none",
            edgecolor=CK, lw=0.9, hatch="////", zorder=3)
    ax.axvline(2.0, color=C1, lw=0.9, ls="--", zorder=1)
    ax.axvline(TS, color=CO, lw=0.9, ls="--", zorder=1)
    ax.set_yticks(range(len(rows)))
    ax.set_yticklabels([r[0] for r in rows])
    ax.set_xlim(0, 4.75)
    ax.set_ylim(-0.6, 2.6)
    ax.set_xticks([0, 1, 2, 3, 4])
    ax.set_xlabel("CHSH value  $S$")
    # the inter-row gap: placing this beside the bar collides with the
    # "2.8284" value label
    ax.annotate("no accepted\nderivation", xy=(3.41, 0.28),
                xytext=(3.41, 0.50), fontsize=FS_ANN, color=CK,
                ha="center", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_title("(a)  three ceilings, and an\nunexplained gap")


def panel_randomness(ax):
    S = np.linspace(2.0, TS, 400)
    ax.plot(S, hmin(S), color=CO, lw=1.8)
    ax.plot([2.0, TS], [hmin(2.0), hmin(TS)], "o", color=CK, ms=4.5,
            zorder=4)
    ax.set_xlim(1.96, 2.90)
    ax.set_ylim(-0.06, 1.13)
    ax.set_xticks([2.0, 2.2, 2.4, 2.6, 2.8])
    ax.set_yticks([0.0, 0.5, 1.0])
    ax.set_xlabel("CHSH value  $S$")
    ax.set_ylabel("certified bits per trial")
    ax.annotate("$S=2$: nothing certified;\nthe outcomes could have\n"
                "been written down first",
                xy=(2.0, 0.0), xytext=(2.10, 0.42), fontsize=FS_ANN,
                color=CK, ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.annotate("$2\\sqrt{2}$: exactly one bit",
                xy=(TS, 1.0), xytext=(2.79, 0.70), fontsize=FS_ANN,
                color=CK, ha="right", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_title("(b)  what it buys: certified\nrandomness, per trial")


def panel_price(ax):
    degs = np.linspace(0, 90, 46)
    ident, direct = [], []
    for d in degs:
        th = np.deg2rad(d)
        A0, A1 = obs(0.0), obs(th)
        cA = np.linalg.norm(comm(A0, A1), 2)
        ident.append(np.sqrt(4 + cA * cA))
        best = 0.0
        for phi in np.linspace(0, np.pi, 220):
            B0, B1 = obs(phi), obs(phi - th)
            best = max(best, np.linalg.norm(chsh(A0, A1, B0, B1), 2))
        direct.append(best)
    ident, direct = np.array(ident), np.array(direct)

    ax.plot(degs, ident, color=CK, ls=":", lw=1.4,
            label="$\\sqrt{4+c_Ac_B}$")
    ax.plot(degs[::3], direct[::3], "o", color=CO, ms=3.8,
            label="direct")
    ax.axhline(2.0, color=C1, lw=0.9, ls="--", zorder=1)
    ax.axhline(TS, color=CO, lw=0.9, ls="--", zorder=1)
    ax.set_xlim(-3, 96)
    ax.set_ylim(1.93, 2.95)
    ax.set_xticks([0, 30, 60, 90])
    ax.set_yticks([2.0, 2.2, 2.4, 2.6, 2.8])
    # a 38-char label centred on a 2.5in panel overflows the figure edge
    ax.set_xlabel("incompatibility  $\\theta$  (deg)")
    ax.set_ylabel("$S_{\\max}$")
    ax.legend(loc="lower right", handlelength=1.5, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.annotate("commuting:\nno excess", xy=(1.5, 2.0), xytext=(14, 2.60),
                fontsize=FS_ANN, color=C1, ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_title("(c)  $2\\sqrt{2}$ needs BOTH sides\n"
                 "maximally incompatible")
    return np.abs(ident - direct).max()


def main():
    print("Correlation tiers figure (paper SS7.5)")
    worst = check_landau()
    print(f"  [verify] Landau identity, max |LHS-RHS| = {worst:.3e}")
    assert worst < 1e-12, "Landau identity failed"

    print(f"  [verify] ceilings: local 2, quantum {TS:.6f}, no-signalling 4")
    print(f"  [verify] quantum / algebraic = {TS/4:.4f}  "
          f"(gap left unused: {100*(1-TS/4):.1f}%)")
    h2, hts = hmin(2.0), hmin(TS)
    print(f"  [verify] H_min(2) = {h2:.9f}  (exact 0)")
    print(f"  [verify] H_min(2sqrt2) = {hts:.9f}  (exact 1)")
    assert abs(h2) < 1e-12 and abs(hts - 1) < 1e-12, "randomness endpoints"

    fig, axes = plt.subplots(1, 3, figsize=(fs.TEXTWIDTH_IN, 2.4587),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.12, h_pad=0.12,
                                hspace=0.07, wspace=0.09)
    panel_tiers(axes[0])
    panel_randomness(axes[1])
    dev = panel_price(axes[2])
    print(f"  [verify] price curve: identity vs direct, max dev = {dev:.3e}")
    assert dev < 1e-9, "price curve disagrees with the operator norm"
    fs.save(fig, "tiers")
    print(f"  wrote {fs.FIGDIR / 'tiers.pdf'}")


if __name__ == "__main__":
    main()
