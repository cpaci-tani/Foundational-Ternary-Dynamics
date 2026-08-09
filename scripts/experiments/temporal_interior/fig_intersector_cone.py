"""fig_intersector_cone.py — why the sectors miss, and what a hit costs.

CLAIM: the production flux symbol is a sum of squares whose arguments sit
at HALF-lattice offsets; integer-displacement Wilson fermions therefore
cannot match it at any shell count, and the price of a match is spinor
dimension 16.

FOUR PANELS, all exact:
  (a) THE MISMATCH.  Flux and Wilson normalised to the same slope still
      part company, and by a DIFFERENT amount in each symmetry direction --
      so no rescaling closes the gap.
  (b) THE REASON.  The nine squares carry arguments q_i/2 and
      (q_i +- q_j)/2, i.e. hops of half a lattice unit.  Wilson hops land
      on integer sites.  The sectors were being matched on the wrong
      lattice.
  (c) THE IDENTITY.  The nine squares stacked, summing exactly to -L18
      along a cut through the zone.
  (d) THE PRICE.  n squares need n mutually anticommuting structures;
      dimension 2^k carries 2k+1.  Nine crosses at dim 16.

PRIOR ART: FTD-0412 (scalar-r no-go at q^4 vs the BCC-time pole) and
FTD-0413 (face-diagonal weight buying q^4).  The exact all-orders no-go,
the rank obstruction and the SOS resolution are
ANALYSIS_INTERSECTOR_CONE_RANK_OBSTRUCTION_v1.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator, FixedFormatter, NullLocator

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


def dticks(axis, t, fmt="{:g}"):
    lab = list(fmt) if not isinstance(fmt, str) else [fmt.format(x) for x in t]
    axis.set_major_locator(FixedLocator(t))
    axis.set_major_formatter(FixedFormatter(lab))
    axis.set_minor_locator(NullLocator())


def flux(q):
    """-L18(q): production M18 spatial symbol."""
    c = np.cos(q)
    return (4 - (2 / 3) * c.sum(-1)
            - (2 / 3) * (c[..., 0] * c[..., 1] + c[..., 1] * c[..., 2]
                         + c[..., 2] * c[..., 0]))


def wilson(q, r=1.0, cs2=1.0):
    """E_W^2 = c_s^2 [ sum sin^2 q_i + r^2 (sum (1 - cos q_i))^2 ]."""
    return cs2 * ((np.sin(q) ** 2).sum(-1)
                  + r ** 2 * ((1 - np.cos(q)).sum(-1)) ** 2)


def sos_terms(q):
    """The nine squares, as (3 face, 6 edge)."""
    face = [np.sin(q[..., i] / 2) ** 2 for i in range(3)]
    edge = []
    for i, j in ((0, 1), (1, 2), (2, 0)):
        edge.append(np.sin((q[..., i] - q[..., j]) / 2) ** 2)
        edge.append(np.sin((q[..., i] + q[..., j]) / 2) ** 2)
    return (4 / 3) * np.array(face), (2 / 3) * np.array(edge)


DIRS = [("[100]", (1, 0, 0), C1, "o"),
        ("[110]", (1, 1, 0), CO, "s"),
        ("[111]", (1, 1, 1), C4, "^")]


# =====================================================================
def panel_mismatch(ax):
    for nm, d, col, mk in DIRS:
        u = np.array(d, float) / np.linalg.norm(d)
        s = np.linspace(1e-3, 2.6, 300)
        q = s[:, None] * u
        ax.plot(s, wilson(q) / flux(q), color=col, lw=1.7, label=nm)
    ax.axhline(1.0, color=CK, lw=1.1, ls="--", zorder=1)
    ax.set_xlim(0, 2.6)
    ax.set_ylim(0.93, 2.32)
    ax.set_xlabel("$|q|$   along the symmetry direction")
    ax.set_ylabel("$E_W^2 \\,/\\, (-L_{18})$")
    ax.legend(loc="upper left", handlelength=1.6, borderpad=0.3,
              labelspacing=0.26, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    # they are IDENTICALLY equal along [100]: both are 2 - 2 cos q
    ax.text(2.52, 1.03, "$[100]$: identically $2-2\\cos q$", fontsize=FS_ANN,
            color=C1, ha="right", va="bottom")
    # (no second annotation: the only free band collides with the legend,
    # and the title already carries the point)
    ax.set_title("(a)  exact along the axes, wrong\n"
                 "off them: the mismatch is angular")


def panel_geometry(ax):
    """z = 0 slice: which sites each sector's hops can reach."""
    g = np.arange(-2, 3)
    X, Y = np.meshgrid(g, g)
    ax.plot(X.ravel(), Y.ravel(), "o", color=CG, ms=5.0, mfc="none",
            mew=1.1, zorder=2)
    h = np.arange(-2, 2.5, 0.5)
    HX, HY = np.meshgrid(h, h)
    half = (np.abs(HX % 1 - 0.5) < 1e-9) | (np.abs(HY % 1 - 0.5) < 1e-9)
    ax.plot(HX[half], HY[half], ".", color=CO, ms=3.4, zorder=2)
    hops = [(0.5, 0), (-0.5, 0), (0, 0.5), (0, -0.5),
            (0.5, 0.5), (0.5, -0.5), (-0.5, 0.5), (-0.5, -0.5)]
    for dx, dy in hops:
        ax.annotate("", xy=(dx, dy), xytext=(0, 0),
                    arrowprops=dict(arrowstyle="->", color=CO, lw=1.3))
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
        ax.annotate("", xy=(dx, dy), xytext=(0, 0),
                    arrowprops=dict(arrowstyle="->", color=C1, lw=1.1,
                                    linestyle="dashed"))
    ax.plot([0], [0], "o", color=CK, ms=5.0, zorder=5)
    ax.set_aspect("equal")
    ax.set_xlim(-2.35, 2.35)
    # the lattice fills the frame, so the captions need headroom above the
    # y = 2 row rather than a corner inside it
    ax.set_ylim(-2.35, 3.35)
    ax.set_xticks([-2, -1, 0, 1, 2])
    ax.set_yticks([-2, -1, 0, 1, 2])
    ax.set_xlabel("lattice $x$   ($z=0$ slice)")
    ax.set_ylabel("lattice $y$")
    ax.text(-2.25, 3.25, "orange: the SOS hops, all half-integer",
            fontsize=FS_ANN, color=CO, ha="left", va="top")
    ax.text(-2.25, 2.88, "blue dashed: Wilson, integer only",
            fontsize=FS_ANN, color=C1, ha="left", va="top")
    ax.set_title("(b)  the reason: the flux's squares\n"
                 "sit on the half-offset lattice")


def panel_identity(ax):
    u = np.array([1, 1, 1], float) / np.sqrt(3)
    s = np.linspace(0, np.pi * np.sqrt(3), 300)
    q = s[:, None] * u
    face, edge = sos_terms(q)
    fsum, esum = face.sum(0), edge.sum(0)
    ax.fill_between(s, 0, fsum, color=C1, alpha=0.35, lw=0,
                    label="3 face squares")
    ax.fill_between(s, fsum, fsum + esum, color=CO, alpha=0.35, lw=0,
                    label="6 edge squares")
    ax.plot(s, flux(q), color=CK, lw=1.8, ls="--", label="$-L_{18}$")
    ax.set_xlim(0, s[-1])
    ax.set_ylim(0, 8.4)
    # 2 pi lies OUTSIDE this axis (the cut ends at sqrt(3) pi)
    ax.set_xticks([0, np.pi, np.pi * np.sqrt(3)])
    ax.set_xticklabels(["0", "$\\pi$", "$\\sqrt{3}\\,\\pi$"])
    ax.set_xlabel("$|q|$   along $[111]$")
    ax.set_ylabel("symbol")
    ax.legend(loc="upper left", handlelength=1.5, borderpad=0.3,
              labelspacing=0.26, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.set_title("(c)  the identity: nine squares stack\n"
                 "exactly onto the flux symbol")
    return np.abs(face.sum(0) + edge.sum(0) - flux(q)).max()


def panel_price(ax):
    k = np.arange(1, 6)
    dim, nstruct = 2 ** k, 2 * k + 1
    ax.step(dim, nstruct, where="post", color=C1, lw=1.8, zorder=3)
    ax.plot(dim, nstruct, "o", color=C1, ms=5.0, zorder=4)
    ax.axhline(9, color=CO, lw=1.5, ls="--", zorder=2)
    ax.plot([16], [9], "*", color=CO, ms=13, zorder=5)
    ax.set_xscale("log", base=2)
    ax.set_xlim(1.7, 40)
    ax.set_ylim(2.2, 12.5)
    dticks(ax.xaxis, [2, 4, 8, 16, 32], "{:d}")
    ax.set_yticks([3, 5, 7, 9, 11])
    ax.set_xlabel("spinor dimension")
    ax.set_ylabel("anticommuting structures")
    ax.text(2.0, 9.35, "nine squares needed", fontsize=FS_ANN, color=CO,
            ha="left", va="bottom")
    ax.annotate("Dirac ($4$) carries $5$:\nnot enough", xy=(4, 5),
                xytext=(5.2, 3.1), fontsize=FS_ANN, color=CK,
                ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.text(17.5, 9.0, "first sufficient:\ndim $16$", fontsize=FS_ANN,
            color=CO, ha="left", va="center")
    ax.set_title("(d)  the price: nine structures,\n"
                 "hence 16-component spinors")


def main():
    print("Inter-sector cone figure")
    rng = np.random.default_rng(3)
    qs = rng.uniform(-np.pi, np.pi, (4000, 3))
    face, edge = sos_terms(qs)
    err = np.abs(face.sum(0) + edge.sum(0) - flux(qs)).max()
    print(f"  [verify] SOS identity over 4000 random q: max |residual| "
          f"= {err:.3e}")
    assert err < 1e-12, "SOS identity failed"
    print(f"  [verify] squares: {face.shape[0]} face + {edge.shape[0]} edge "
          f"= {face.shape[0] + edge.shape[0]}")
    for k in range(1, 6):
        if 2 * k + 1 >= 9:
            print(f"  [verify] first sufficient spinor dimension = {2**k} "
                  f"({2*k+1} structures)")
            break
    for nm, d, _, _ in DIRS:
        u = np.array(d, float) / np.linalg.norm(d)
        q = 2.0 * u
        print(f"  [verify] at |q|=2 along {nm}: "
              f"E_W^2/(-L18) = {wilson(q[None])[0]/flux(q[None])[0]:.4f}")

    fig, axes = plt.subplots(2, 2, figsize=(7.3, 6.4),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)
    panel_mismatch(axes[0, 0])
    panel_geometry(axes[0, 1])
    e = panel_identity(axes[1, 0])
    print(f"  [verify] identity along [111]: max |residual| = {e:.3e}")
    panel_price(axes[1, 1])
    fig.savefig(FIGDIR / "fig16_intersector.pdf")
    fig.savefig(OUT / "fig16_intersector.png", dpi=200)
    print(f"  wrote {FIGDIR / 'fig16_intersector.pdf'}")


if __name__ == "__main__":
    main()
