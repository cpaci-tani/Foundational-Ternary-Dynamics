"""fig_cone_arc.py — figure for the cone arc (paper SS8.6-8.8).

CLAIM: the limiting speed depends on the rest mass; a bound composite
inherits its CONSTITUENTS' excess rather than its own total mass's; and
the mechanical clock's Galilean character is a modelling artifact, not a
categorical fact about the carrier.

FOUR PIECES, ALL EXACT (each checked against a closed form):
  (a) C_eff(M)/C - 1  extracted from the exact dispersion, against M^2/12
  (b) dilation residual against the analytic k^4/(36 M^2)
  (c) composite delta vs N, flat, against the naive (N M)^2/6 growing as N^2
  (d) the two-limit validity plane, with real systems placed on it

SCOPE: illustrates results established elsewhere; introduces no new claim
and moves no tag.  Scalar sector only -- says nothing about FTD's own mass
mechanism.  The composite result is first order in the LV parameter with
non-relativistic internal motion.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


C = 1.0 / np.sqrt(3.0)
C2 = C * C

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks




# =====================================================================
# exact dispersion along an axis:  4 sin^2(w/2) = C^2 (-L) + M^2
# =====================================================================
def omega(k, M):
    W = 4.0 * C2 * np.sin(np.asarray(k) / 2.0) ** 2 + M * M
    return 2.0 * np.arcsin(np.sqrt(W) / 2.0)


def vgroup(k, M):
    """dw/dk.  W = 4 sin^2(w/2) gives dW/dw = 2 sin w, so
    dw/dk = (dW/dk)/(2 sin w) with dW/dk = 4 C^2 sin(k/2) cos(k/2)
          = 2 C^2 sin k.   Net:  C^2 sin k / sin w.
    (The factor 2 in dW/dw is easy to drop; doing so doubles v_g and turns
    the genuine O(k^4) residual in panel (b) into a spurious O(k^2) one.)"""
    k = np.asarray(k, dtype=float)
    return C2 * np.sin(k) / np.sin(omega(k, M))


def check_vgroup():
    """Cross-check against an independent route: finite differences."""
    k0, M = 0.317, 0.41
    h = 1e-6
    fd = (omega(k0 + h, M) - omega(k0 - h, M)) / (2 * h)
    an = float(vgroup(k0, M))
    return an, float(fd)


def C_eff(M):
    """k^2 coefficient of the exact dispersion, by fit at small k."""
    ks = np.linspace(1e-3, 6e-3, 12)
    O2 = omega(ks, M) ** 2
    O0 = omega(0.0, M) ** 2
    A = np.stack([ks ** 2, ks ** 4], -1)
    a2, _ = np.linalg.lstsq(A, O2 - O0, rcond=None)[0]
    return np.sqrt(a2)


# =====================================================================
def panel_ceff(ax):
    Ms = np.logspace(np.log10(0.02), np.log10(0.6), 22)
    dev = np.array([C_eff(M) / C - 1.0 for M in Ms])
    pred = Ms ** 2 / 12.0
    small = Ms < 0.15
    slope = np.polyfit(np.log(Ms[small]), np.log(dev[small]), 1)[0]

    ax.loglog(Ms, pred, color=CK, ls=":", lw=1.2, label="$M^2/12$")
    ax.loglog(Ms, dev, "o", color=CO, ms=4.0, label="from the dispersion")
    ax.set_xlabel("rest frequency  $M$")
    ax.set_ylabel("$C_{\\rm eff}/C - 1$")
    ax.set_xlim(0.017, 0.72)
    decade_ticks(ax.xaxis, [0.02, 0.05, 0.1, 0.2, 0.5])
    decade_ticks(ax.yaxis, [1e-5, 1e-4, 1e-3, 1e-2],
                 ["$10^{-5}$", "$10^{-4}$", "$10^{-3}$", "$10^{-2}$"])
    ax.legend(loc="upper left", handlelength=1.8, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(0.62, 2.2e-5, f"slope $= {slope:.4f}$", fontsize=FS_ANN,
            color=CK, ha="right", va="bottom")
    ax.set_title("(a)  the limiting speed depends on the\n"
                 "rest mass, as $M^2/12$")
    return slope, np.abs(dev[small] / pred[small] - 1).max()


def panel_dilation(ax):
    styles = [(0.5, CO, "o"), (0.2, C1, "s"), (0.05, C4, "^")]
    worst = 0.0
    for M, col, mk in styles:
        Ce = C_eff(M)
        ks = np.logspace(np.log10(0.004), np.log10(0.12), 16)
        Om = omega(ks, M)
        vg = vgroup(ks, M)
        O0 = omega(0.0, M)
        res = Om * np.sqrt(np.clip(1 - (vg / Ce) ** 2, 0, None)) / O0 - 1
        pred = ks ** 4 / (36 * M * M)
        beta = vg / C
        ax.loglog(beta, pred, color=CK, ls=":", lw=1.0, zorder=1)
        ax.loglog(beta, res, mk, color=col, ms=3.6,
                  label=f"$M={M}$", zorder=3)
        worst = max(worst, np.abs(res / pred - 1).max())
    ax.set_xlabel("$\\beta = v_g/C$")
    ax.set_ylabel("dilation residual")
    decade_ticks(ax.xaxis, [1e-3, 1e-2, 1e-1],
                 ["$10^{-3}$", "$10^{-2}$", "$10^{-1}$"])
    decade_ticks(ax.yaxis, [1e-12, 1e-9, 1e-6],
                 ["$10^{-12}$", "$10^{-9}$", "$10^{-6}$"])
    ax.legend(loc="upper left", handlelength=1.4, borderpad=0.3,
              labelspacing=0.25, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(0.97, 0.04, "dotted: $k^4/36M^2$", fontsize=FS_ANN,
            color=CK, ha="right", va="bottom", transform=ax.transAxes)
    ax.set_title("(b)  within a species dilation is exact\n"
                 "to $O(\\beta^4)$, and the residual is known")
    return worst


def composite_delta(masses):
    """delta from E(K) = min over splits of sum_a omega_a(k_a).
    For N identical constituents the split is k_a = K/N by symmetry."""
    Ks = np.linspace(0.0, 2e-3, 9)
    n = len(masses)
    assert len(set(masses)) == 1, "identical-constituent branch only"
    E = np.array([n * omega(K / n, masses[0]) for K in Ks])
    A = np.stack([Ks ** 2, Ks ** 4], -1)
    a2, _ = np.linalg.lstsq(A[1:], (E - E[0])[1:], rcond=None)[0]
    return (2.0 * E[0] * a2) / C2 - 1.0


def panel_composite(ax):
    m0 = 0.3
    Ns = np.array([1, 2, 3, 5, 8, 12, 20, 30])
    dc = np.array([composite_delta([m0] * int(n)) for n in Ns])
    naive = (Ns * m0) ** 2 / 6.0
    ax.loglog(Ns, naive, "s--", color=C1, ms=4.0,
              label="if it were one particle\nof the total mass")
    ax.loglog(Ns, dc, "o-", color=CO, ms=4.5,
              label="composite (computed)")
    ax.axhline(m0 * m0 / 6, color=CK, ls=":", lw=1.0, zorder=1)
    ax.set_xlabel("number of constituents  $N$")
    ax.set_ylabel("$\\delta = c^2/C^2 - 1$")
    ax.set_xlim(0.8, 42)
    ax.set_ylim(3e-3, 4e1)
    decade_ticks(ax.xaxis, [1, 2, 5, 10, 30], "{:d}")
    decade_ticks(ax.yaxis, [1e-2, 1e-1, 1e0, 1e1],
                 ["$10^{-2}$", "$10^{-1}$", "$1$", "$10$"])
    ax.legend(loc="upper left", handlelength=1.6, borderpad=0.3,
              labelspacing=0.3, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    # empty band below both curves; sitting it near the axis collides
    # with the "30" tick label
    ax.text(1.15, 5.5e-3, "constituent $M^2/6$", fontsize=FS_ANN,
            color=CK, ha="left", va="center")
    ax.set_title("(c)  the composite inherits its constituents',\n"
                 "not its own total mass's")
    return dc


def panel_validity(ax):
    c = 2.99792458e8
    ref = [("Earth-Sun", 1.9910e-7, 1.4960e11, 9.93e-5, C1, "o"),
           ("hydrogen", 2.0670e16, 5.2918e-11, 7.297e-3, CG_, "s"),
           ("heavy nucleus", 1.5e22, 6.0e-15, 0.25, C4, "^")]
    # reference labels go DOWN-RIGHT, except the nucleus, which sits just
    # up-left of the clock chain and would collide there
    off = {"heavy nucleus": ((-7, 7), "right", "bottom")}
    for nm, w, r, voc, col, mk in ref:
        ax.loglog([w * r / c], [voc], mk, color=col, ms=6.0, zorder=4)
        d, ha, va = off.get(nm, ((6, -9), "left", "top"))
        ax.annotate(nm, xy=(w * r / c, voc), xytext=d,
                    textcoords="offset points", fontsize=FS_ANN,
                    color=col, ha=ha, va=va)
    TA = np.sqrt(np.pi) * 2.958675119188639          # T*A for the MVC mode
    As = np.array([0.12, 0.20, 0.30, 0.50])
    T = TA / As
    wr = (2 * np.pi / T) * 1.0 / C
    voc = As ** 2 / C
    ax.loglog(wr, voc, "D-", color=CO, ms=4.5, zorder=4,
              label="mechanical clock")
    for A, x, y in zip(As, wr, voc):
        ax.annotate(f"$A={A:.2f}$", xy=(x, y), xytext=(6, -6),
                    textcoords="offset points", fontsize=FS_ANN,
                    color=CO, ha="left", va="top")
    ax.axvspan(1e-5, 0.1, color=CG_, alpha=0.07, lw=0)
    ax.axhspan(1e-5, 0.1, color=CG_, alpha=0.07, lw=0)
    ax.plot([1e-5, 3], [1e-5, 3], color=CG, lw=0.6, ls="-", zorder=0)
    ax.set_xlim(3e-5, 3.0)
    ax.set_ylim(3e-5, 1.2)
    decade_ticks(ax.xaxis, [1e-4, 1e-2, 1e0],
                 ["$10^{-4}$", "$10^{-2}$", "$1$"])
    decade_ticks(ax.yaxis, [1e-4, 1e-2, 1e0],
                 ["$10^{-4}$", "$10^{-2}$", "$1$"])
    ax.set_xlabel("retardation  $\\omega r/c$")
    ax.set_ylabel("constituent speed  $v/c$")
    ax.text(0.022, 3.0e-4, "potential\nlicensed", fontsize=FS_ANN,
            color=CG_, ha="center", va="center")
    ax.set_title("(d)  the clock sits outside both limits\n"
                 "its own model assumes")
    return wr, voc


def main():
    print("Cone arc figure (paper SS8.6-8.8)")
    an, fd = check_vgroup()
    print(f"  [verify] v_g analytic {an:.12f}  finite-diff {fd:.12f}"
          f"   agree: {abs(an - fd) < 1e-8}")
    assert abs(an - fd) < 1e-8, "group-velocity formula disagrees with FD"

    fig, axes = plt.subplots(2, 2, figsize=(fs.TEXTWIDTH_IN, 5.4073),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)

    slope, ceff_err = panel_ceff(axes[0, 0])
    print(f"  [verify] C_eff slope = {slope:.6f}   (exact 2)")
    print(f"  [verify] max |C_eff/(M^2/12) - 1| at M<0.15 = {ceff_err:.3e}")

    worst = panel_dilation(axes[0, 1])
    print(f"  [verify] residual vs k^4/36M^2: max rel dev = {worst:.3e}")

    dc = panel_composite(axes[1, 0])
    spread = dc.max() / dc.min() - 1
    print(f"  [verify] composite delta over N=1..30: spread = {spread:.3e}"
          f"   (naive would grow 900x)")
    assert spread < 1e-5, "composite delta is not N-independent"

    wr, voc = panel_validity(axes[1, 1])
    print(f"  [verify] clock at A=0.30: wr/C = {wr[2]:.4f}, "
          f"v/C = {voc[2]:.4f}   (both >> 0.1)")
    fs.save(fig, "conearc")
    print(f"  wrote {fs.FIGDIR / 'conearc.pdf'}")


if __name__ == "__main__":
    main()
