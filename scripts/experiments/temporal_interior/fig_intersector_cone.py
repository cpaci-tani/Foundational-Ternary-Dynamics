"""fig_intersector_cone.py — why the sectors miss, and what a hit costs.

CLAIM: the production flux symbol has both an exact 18-square integer-hop
decomposition and a more economical exact seven-square half-angle
decomposition.  Half offsets lower the known termwise-Clifford construction
price; they are not
forced by sum-of-squares existence.  The rigorous half-angle square bounds
are 3--7 (spinor dimension 2--8), with a numerical four-square candidate
at dimension 4.

FOUR PANELS: (a)--(c) are exact; (d) combines exact constructions and
rigorous bounds with one explicitly numerical candidate:
  (a) THE MISMATCH.  Flux and Wilson normalised to the same slope still
      part company, and by a DIFFERENT amount in each symmetry direction --
      so no rescaling closes the gap.
  (b) THE PLACEMENTS.  The economical basis uses half-offset face/edge
      placements.  A longer exact construction uses integer face/edge hops.
  (c) THE IDENTITY.  The nine squares stacked, summing exactly to -L18
      along a cut through the zone.
  (d) THE PRICE, corrected.  n squares need n mutually anticommuting
      structures and dimension 2^k carries 2k+1.  The exact integer-hop
      construction has eighteen squares (dim 512); an exact spatially
      cubic-covariant half-angle SEVEN exists (dim 8), and
      the best found is FOUR (dim 4, ordinary Dirac, one structure spare
      for a mass) at the cost of cubic covariance; the rigorous floor is
      three.  The exact rigorous interval is [3,7]; four is a numerical
      candidate without an exact or interval existence certificate. An earlier five came from
      an under-dispersed search.  See derive_sos_rank_minimal.py.

PRIOR ART: FTD-0412 (scalar-r no-go at q^4 vs the BCC-time pole) and
FTD-0413 (face-diagonal weight buying q^4).  The exact all-orders no-go,
the rank obstruction and the SOS resolution are
ANALYSIS_INTERSECTOR_CONE_RANK_OBSTRUCTION_v1.
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


def dticks(axis, t, fmt="{:g}"):
    fs.decade_ticks(axis, t, fmt)


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


def integer_sos_terms(q):
    """Exact integer-frequency SOS: two squares for each M18 hop pair."""
    dirs_weights = [((1, 0, 0), 1 / 3), ((0, 1, 0), 1 / 3),
                    ((0, 0, 1), 1 / 3),
                    ((1, 1, 0), 1 / 6), ((1, -1, 0), 1 / 6),
                    ((0, 1, 1), 1 / 6), ((0, 1, -1), 1 / 6),
                    ((1, 0, 1), 1 / 6), ((1, 0, -1), 1 / 6)]
    terms = []
    for d, w in dirs_weights:
        phase = np.sum(q * np.asarray(d), axis=-1)
        terms.extend((w * np.sin(phase) ** 2,
                      w * (1 - np.cos(phase)) ** 2))
    return np.asarray(terms)


DIRS = [("[100]", (1, 0, 0), C1, "o", "-"),
        ("[110]", (1, 1, 0), CO, "s", "--"),
        ("[111]", (1, 1, 1), C4, "^", "-.")]


# =====================================================================
def panel_mismatch(ax):
    for nm, d, col, mk, ls in DIRS:
        u = np.array(d, float) / np.linalg.norm(d)
        s = np.linspace(1e-3, 2.6, 300)
        q = s[:, None] * u
        ax.plot(s, wilson(q) / flux(q), color=col, ls=ls, lw=1.7,
                label=nm)
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
    for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1),
                   (1, 1), (1, -1), (-1, 1), (-1, -1)):
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
    ax.text(-2.25, 3.25, "orange: economical half-offset basis",
            fontsize=FS_ANN, color=CO, ha="left", va="top")
    ax.text(-2.25, 2.88, "blue dashed: exact integer-hop alternative",
            fontsize=FS_ANN, color=C1, ha="left", va="top")
    ax.set_title("(b)  two exact placement classes:\n"
                 "half-offset economy, integer-hop existence")


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
    # Two short lines: the long single-clause version overran the panel.
    ax.set_title("(c)  one decomposition, not the fewest:\n"
                 "nine squares stack onto the symbol")
    return np.abs(face.sum(0) + edge.sum(0) - flux(q)).max()


def spinor_dim(n):
    """Smallest 2^k carrying n mutually anticommuting Hermitian structures."""
    k = 0
    while 2 * k + 1 < n:
        k += 1
    return 2 ** k


def panel_price(ax):
    """Show exact spatial-symbol constructions and a search-scoped result.

    The integer-hop eighteen and half-angle seven are exact.  The four is
    numerical (residual 5.3e-15 on 4000 fresh momenta), not cubic-covariant,
    and has no existence or minimality certificate.
    """
    k = np.arange(1, 6)
    dim, nstruct = 2 ** k, 2 * k + 1
    ax.step(dim, nstruct, where="post", color=CG, lw=1.6, zorder=2)
    ax.plot(dim, nstruct, "o", color=CG, ms=4.2, zorder=3)

    marks = ((18, CK, "o", "eighteen\nexact, integer-hop"),
             (7, C1, "s", "seven\nexact, cubic-covariant"),
             (4, CO, "*", "four: best found\n(not cubic-covariant)"))
    for n, col, mk, lab in marks:
        d = spinor_dim(n)
        ax.plot([d], [n], mk, color=col, ms=13 if mk == "*" else 7.0,
                zorder=6, mec="white", mew=0.8)
        ax.plot([1.7, d], [n, n], color=col, lw=1.0, ls=":", zorder=4)
        ax.plot([d, d], [2.2, n], color=col, lw=1.0, ls=":", zorder=4)

    ax.set_xscale("log", base=2)
    ax.set_xlim(1.7, 800)
    ax.set_ylim(2.2, 20.5)
    dticks(ax.xaxis, [2, 4, 8, 32, 128, 512], "{:d}")
    ax.set_yticks([3, 7, 11, 15, 18])
    ax.set_xlabel("spinor dimension")
    ax.set_ylabel("squares in termwise Clifford ansatz")
    # Kept short: the long form ran through the dim-8 guide line.
    ax.text(2.05, 4.45, "four: numerical candidate\n(rigorous bounds: 3--7)",
            fontsize=FS_ANN, color=CO, ha="left", va="bottom")
    ax.text(9.0, 7.35, "seven: exact half-offset", fontsize=FS_ANN,
            color=C1, ha="left", va="bottom")
    ax.text(470, 18.35, "eighteen: exact integer-hop", fontsize=FS_ANN,
            color=CK, ha="right", va="bottom")
    ax.set_title("(d)  termwise-Clifford price: integer 18 $\\to$ 512\n"
                 "half-offset 7 $\\to$ dim 8; four is numerical")


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
    integer_terms = integer_sos_terms(qs)
    e18 = np.abs(integer_terms.sum(0) - flux(qs)).max()
    print(f"  [verify] exact integer-hop EIGHTEEN-square identity: "
          f"max |residual| = {e18:.3e}")
    assert integer_terms.shape[0] == 18 and e18 < 1e-12
    # The exact spatially cubic-covariant seven, checked here so panel (d)
    # draws nothing
    # it has not verified.  x = q/2.
    x = qs / 2.0
    s2, c2 = np.sin(x) ** 2, np.cos(x) ** 2
    cyc = ((0, 1, 2), (1, 2, 0), (2, 0, 1))
    seven = (4.0 * sum(s2[:, i] * c2[:, j] * c2[:, k] for i, j, k in cyc)
             + (16.0 / 3.0) * sum(s2[:, j] * s2[:, k] * c2[:, i]
                                  for i, j, k in cyc)
             + 4.0 * s2[:, 0] * s2[:, 1] * s2[:, 2])
    e7 = np.abs(seven - flux(qs)).max()
    print(f"  [verify] exact cubic-covariant SEVEN-square identity: "
          f"max |residual| = {e7:.3e}")
    assert e7 < 1e-12, "the seven-square identity failed"
    for n in (18, 9, 7, 4):
        print(f"  [verify] {n} squares -> spinor dimension {spinor_dim(n)}")
    for nm, d, _, _, _ in DIRS:
        u = np.array(d, float) / np.linalg.norm(d)
        q = 2.0 * u
        print(f"  [verify] at |q|=2 along {nm}: "
              f"E_W^2/(-L18) = {wilson(q[None])[0]/flux(q[None])[0]:.4f}")

    fig, axes = plt.subplots(2, 2, figsize=(fs.TEXTWIDTH_IN, 5.4073),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)
    panel_mismatch(axes[0, 0])
    panel_geometry(axes[0, 1])
    e = panel_identity(axes[1, 0])
    print(f"  [verify] identity along [111]: max |residual| = {e:.3e}")
    panel_price(axes[1, 1])
    fs.save(fig, "intersector")
    print(f"  wrote {fs.FIGDIR / 'intersector.pdf'}")


if __name__ == "__main__":
    main()
