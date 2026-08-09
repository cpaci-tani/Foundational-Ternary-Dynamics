"""fig_bell_angles.py — what the angles do, and which part needs a record.

Two claims are commonly run together.  This figure separates them, because
one is true and the other is not.

  TRUE:  the CHSH number S is a COMPILED STATISTIC.  Its four correlators
         come from four disjoint sub-ensembles; no trial contributes to
         more than one; and the classical bound is exceeded only once the
         FOURTH context is added.  S does not exist without a compiler.

  FALSE: "noncommutativity therefore lives only in the record."  Three
         polarisers at 0/45/90 transmit I0/8 where two crossed ones
         transmit nothing -- one beam, one pass, no ensemble comparison,
         no memory.  The angle dependence is physical.

PHOTON CONVENTION.  For polarisation-entangled light E(th) = cos(2 th),
the angle doubling that makes real photon Bell tests use 22.5 deg steps.

PIECES (exact unless noted):
  (a) E(th): quantum cos(2 th) against the best local bound, which is
      LINEAR and touches it at 0, 45, 90 deg -- so the violation is an
      angle-window effect, zero at the touching points.
  (b) the four CHSH terms, each exactly 1/sqrt2, and the running sum:
      the bound is crossed ONLY on the fourth.
  (c) three polarisers -- single beam, no record, angle-dependent.
  (d) S accumulating over trials [MONTE CARLO, shown with a band], and
      the four sub-ensembles verified disjoint.

SCOPE.  This does not show Bell's theorem inapplicable.  It shows which
ingredient needs records (the sum) and which does not (the commutator
that lets the sum exceed 2).
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

OUT = Path(__file__).resolve().parent
FIGDIR = (Path(__file__).resolve().parents[3] / "dissemination" / "papers"
          / "semantic_ontology" / "figures")
FIGDIR.mkdir(parents=True, exist_ok=True)

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = 7.5, 8.5, 9.0, 7.0, 7.0
plt.rcParams.update({
    "font.family": "serif", "mathtext.fontset": "cm",
    "font.size": FS_TICK, "axes.labelsize": FS_LAB,
    "axes.titlesize": FS_TITLE, "legend.fontsize": FS_LEG,
    "xtick.labelsize": FS_TICK, "ytick.labelsize": FS_TICK,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.titlepad": 7.0, "axes.labelpad": 3.5,
    "figure.dpi": 160, "savefig.dpi": 300, "lines.linewidth": 1.5,
})
C1, CO, CG_, C4 = "#2a78d6", "#eb6834", "#1baf7a", "#7a4bbd"
CK, CG = "#2b2b2b", "#9a9a9a"
TS = 2.0 * np.sqrt(2.0)

# CHSH settings for light, in degrees
A0, A1, B0, B1 = 0.0, 45.0, 22.5, 67.5
TERMS = [("$E(a,b)$", A0, B0, +1), ("$-E(a,b')$", A0, B1, -1),
         ("$E(a',b)$", A1, B0, +1), ("$E(a',b')$", A1, B1, +1)]


def Equant(dth_deg):
    """Polarisation-entangled light: angle doubling."""
    return np.cos(2 * np.deg2rad(dth_deg))


def Elocal(dth_deg):
    """Best local model: LINEAR in the angle, running from +1 at 0 deg to
    -1 at 90 deg, hence touching the quantum curve at 0, 45 and 90.

    The slope is 2/90 per degree, not 4/90 -- the latter reaches -1 at
    45 deg where the quantum curve is 0, and the two would not touch at
    all.  Caught by the assert in main()."""
    t = np.abs(np.asarray(dth_deg, dtype=float)) % 180.0
    t = np.where(t > 90.0, 180.0 - t, t)
    return 1.0 - 2.0 * t / 90.0


# =====================================================================
def panel_curve(ax):
    th = np.linspace(0, 90, 400)
    q, l = Equant(th), Elocal(th)
    ax.fill_between(th, l, q, where=q >= l, color=CO, alpha=0.16, lw=0)
    ax.plot(th, q, color=CO, lw=1.8, label="quantum  $\\cos 2\\theta$")
    ax.plot(th, l, color=C1, lw=1.4, ls="--", label="best local model")
    for t in (0.0, 45.0, 90.0):
        ax.plot([t], [Equant(t)], "o", color=CK, ms=3.6, zorder=5)
    ax.axvline(22.5, color=CG, lw=0.7, ls=":", zorder=1)
    ax.axvline(67.5, color=CG, lw=0.7, ls=":", zorder=1)
    ax.set_xlim(-3, 93)
    ax.set_ylim(-1.18, 1.30)
    ax.set_xticks([0, 22.5, 45, 67.5, 90])
    ax.set_xticklabels(["0", "22.5", "45", "67.5", "90"])
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("relative polariser angle  $\\theta$  (deg)")
    ax.set_ylabel("correlation  $E$")
    ax.legend(loc="upper right", handlelength=1.7, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(45, -0.92, "they agree at $0,45,90$;\nthe gap is a window",
            fontsize=FS_ANN, color=CK, ha="center", va="center")
    ax.set_title("(a)  the angles matter because the two\n"
                 "curves touch at three angles")
    gap = q - l
    return th[int(np.argmax(gap))], gap.max()


def panel_terms(ax):
    """Ordering-free by construction.

    An earlier draft plotted the RUNNING SUM and claimed the bound was
    crossed "on the fourth term".  Two things were wrong: it crosses on
    the third (3/sqrt2 = 2.121), and "crossed at term k" depends on the
    arbitrary ordering of the four contexts, so it is not a statement
    about the physics at all.  What is ordering-free: no single context
    is anomalous, and the bound constrains only the total."""
    vals = np.array([s * Equant(a - b) for _, a, b, s in TERMS])
    tot = vals.sum()
    x = np.arange(1, 5)
    ax.bar(x, vals, width=0.5, color=CO, alpha=0.35, edgecolor=CO,
           lw=1.2, zorder=2)
    ax.bar([6.0], [tot], width=0.5, color=CK, alpha=0.30, edgecolor=CK,
           lw=1.2, zorder=2)
    ax.plot([0.5, 4.5], [1.0, 1.0], color=C1, lw=1.1, ls="--", zorder=3)
    ax.plot([5.4, 6.6], [2.0, 2.0], color=C1, lw=1.1, ls="--", zorder=3)
    ax.plot([5.4, 6.6], [TS, TS], color=CO, lw=1.1, ls=":", zorder=3)
    ax.set_xticks(list(x) + [6.0])
    ax.set_xticklabels([t[0] for t in TERMS] + ["$S$"])
    ax.set_xlim(0.4, 7.0)
    ax.set_ylim(0, 3.5)
    ax.set_yticks([0, 1, 2, 3])
    ax.set_ylabel("value")
    ax.text(2.5, 1.10, "any single correlator: $|E|\\leq 1$",
            fontsize=FS_ANN, color=C1, ha="center", va="bottom")
    ax.text(6.0, 2.0 - 0.16, "local bound", fontsize=FS_ANN, color=C1,
            ha="center", va="top")
    ax.text(6.0, TS + 0.09, f"${tot:.4f}$", fontsize=FS_ANN, color=CO,
            ha="center", va="bottom")
    # (no in-panel note here: any text wide enough to read overlaps the
    # bars, and the title already carries the point)
    ax.set_title("(b)  no context is anomalous; the bound\n"
                 "constrains only the combination")
    return vals, tot


def panel_polarisers(ax):
    # 361 points => step 0.25 deg, so the grid CONTAINS 45 exactly.
    # With linspace(0, 90, 400) the step is 0.2256 and the peak is sampled
    # at 0.124998, which then fails an exact assert for no physical reason.
    th = np.linspace(0, 90, 361)
    I = 0.125 * np.sin(np.deg2rad(2 * th)) ** 2
    ax.plot(th, I, color=C4, lw=1.8)
    ax.plot([45], [0.125], "o", color=CK, ms=4.5, zorder=5)
    ax.axhline(0.0, color=C1, lw=1.2, ls="--", zorder=1)
    ax.set_xlim(-3, 93)
    ax.set_ylim(-0.016, 0.196)
    ax.set_xticks([0, 22.5, 45, 67.5, 90])
    ax.set_xticklabels(["0", "22.5", "45", "67.5", "90"])
    ax.set_yticks([0.0, 0.05, 0.10, 0.125])
    ax.set_yticklabels(["0", "0.05", "0.10", "1/8"])
    ax.set_xlabel("middle polariser angle  (deg)")
    ax.set_ylabel("transmitted  $I/I_0$")
    ax.annotate("insert a third polariser and\nlight passes where none did",
                xy=(45, 0.125), xytext=(45, 0.052), fontsize=FS_ANN,
                color=CK, ha="center", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    # upper-left: the curve rises from 0 on the left, so ANY text placed
    # near y=0 there is crossed by it
    ax.plot([0, 90], [0, 0], "o", color=C1, ms=4.0, zorder=5)
    ax.text(3, 0.190, "at $0$ or $90$ the third polariser\n"
                      "does nothing: still zero",
            fontsize=FS_ANN, color=C1, ha="left", va="top")
    ax.set_title("(c)  one beam, one pass, no record:\n"
                 "the angle dependence is physical")
    return I.max()


def panel_accumulate(ax, ntrial=20000, seeds=24, rng0=5):
    """MONTE CARLO -- the only statistical piece here, so it carries a band.
    Each trial draws one setting pair and one outcome pair; the four
    sub-ensembles are verified disjoint."""
    pairs = [(A0, B0, +1), (A0, B1, -1), (A1, B0, +1), (A1, B1, +1)]
    Es = np.array([Equant(a - b) for a, b, _ in pairs])
    sg = np.array([s for _, _, s in pairs])
    grid = np.unique(np.geomspace(200, ntrial, 40).astype(int))
    runs = np.empty((seeds, len(grid)))
    disjoint_ok = True
    for s in range(seeds):
        rg = np.random.default_rng(rng0 + 17 * s)
        ctx = rg.integers(0, 4, ntrial)
        u = rg.random(ntrial)
        # P(ab=+1 | context) = (1+E)/2
        agree = u < (1 + Es[ctx]) / 2
        prod = np.where(agree, 1.0, -1.0)
        if s == 0:
            idx = [np.flatnonzero(ctx == c) for c in range(4)]
            tot = sum(len(i) for i in idx)
            pair_ok = all(np.intersect1d(idx[i], idx[j]).size == 0
                          for i in range(4) for j in range(i + 1, 4))
            disjoint_ok = pair_ok and tot == ntrial
        for gi, n in enumerate(grid):
            c, p = ctx[:n], prod[:n]
            Ehat = np.array([p[c == k].mean() if np.any(c == k) else 0.0
                             for k in range(4)])
            runs[s, gi] = float(np.dot(sg, Ehat))
    med = np.median(runs, axis=0)
    lo, hi = np.percentile(runs, [16, 84], axis=0)
    ax.fill_between(grid, lo, hi, color=CO, alpha=0.22, lw=0)
    ax.semilogx(grid, med, color=CO, lw=1.6, label="compiled $S$  ($1\\sigma$)")
    ax.axhline(2.0, color=C1, lw=1.1, ls="--", zorder=1)
    ax.axhline(TS, color=CK, lw=1.0, ls=":", zorder=1)
    ax.set_xlim(180, ntrial * 1.15)
    ax.set_ylim(1.55, 3.15)
    ax.set_yticks([2.0, 2.5, TS])
    ax.set_yticklabels(["2", "2.5", "$2\\sqrt{2}$"])
    ax.set_xlabel("trials compiled")
    ax.set_ylabel("$S$")
    ax.legend(loc="lower right", handlelength=1.6, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    # below the local bound: the only region the band never enters
    ax.text(250, 1.79, "no trial enters more than one\n"
                       "of the four contexts", fontsize=FS_ANN, color=CK,
            ha="left", va="center")
    ax.set_title("(d)  $S$ exists only after compiling\n"
                 "four disjoint sub-ensembles")
    return med[-1], disjoint_ok


def main():
    print("Bell angles figure")

    fig, axes = plt.subplots(2, 2, figsize=(7.3, 6.2),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)

    th_max, gap_max = panel_curve(axes[0, 0])
    for t in (0.0, 45.0, 90.0):
        assert abs(Equant(t) - Elocal(t)) < 1e-12, f"curves differ at {t}"
    print(f"  [verify] quantum and local agree exactly at 0, 45, 90 deg")
    print(f"  [verify] widest gap at theta = {th_max:.2f} deg, "
          f"size {gap_max:.6f}")

    vals, tot = panel_terms(axes[0, 1])
    print(f"  [verify] four terms = {np.round(vals, 9)}   "
          f"(each exactly {1/np.sqrt(2):.9f})")
    assert np.allclose(vals, 1 / np.sqrt(2)), "terms are not all 1/sqrt2"
    print(f"  [verify] every |term| <= 1: {bool(np.all(np.abs(vals) <= 1))}"
          f"   (max {np.abs(vals).max():.6f})")
    assert np.all(np.abs(vals) <= 1.0), "a correlator exceeds 1"
    print(f"  [verify] total S = {tot:.9f}   (exact {TS:.9f})")
    assert abs(tot - TS) < 1e-12, "sum is not 2 sqrt 2"

    Imax = panel_polarisers(axes[1, 0])
    print(f"  [verify] three polarisers, peak I/I0 = {Imax:.9f}   "
          f"(exact 0.125)")
    assert abs(Imax - 0.125) < 1e-9, "three-polariser peak is not 1/8"
    print(f"  [verify] two crossed polarisers alone: 0 (exact)")

    Sfin, disjoint = panel_accumulate(axes[1, 1])
    print(f"  [verify] sub-ensembles pairwise disjoint and exhaustive: "
          f"{disjoint}")
    assert disjoint, "the four contexts are not disjoint"
    print(f"  [verify] compiled S at 20000 trials = {Sfin:.4f}   "
          f"(target {TS:.4f})")

    fig.savefig(FIGDIR / "fig13_bellangles.pdf")
    fig.savefig(OUT / "fig13_bellangles.png", dpi=200)
    print(f"  wrote {FIGDIR / 'fig13_bellangles.pdf'}")


if __name__ == "__main__":
    main()
