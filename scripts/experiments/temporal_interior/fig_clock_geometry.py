"""fig_clock_geometry.py — the two clocks, seen.

CLAIM: pi and G* are not abstract normalisations.  They are the arc
lengths of two curves; the degenerate clock lives at a pitchfork threshold
where the energy valley goes flat in one direction; and time dilation is
the geometry of tick events on a worldline.

SIX PIECES, all exact:
  (a) the circle and the lemniscate drawn to the same scale, with their
      arc lengths 2 pi and sqrt(pi) G* -- the constants ARE the curves.
  (b) phase portraits at equal energy spacing: harmonic orbits are
      ellipses traversed uniformly; quartic orbits are flat-flanked ovals
      and get FASTER as they grow.
  (c) the pitchfork V = mu x^2/2 + lam x^4: the degenerate clock lives on
      the single surface mu = 0 between one well and two.
  (d) the degeneracy as geometry: energy contours around a nondegenerate
      minimum are ellipses; around a quartic one they open into a flat
      valley -- and flatness IS cheapness to steer.
  (e) dilation as geometry: tick events on two worldlines, one at rest and
      one moving, drawn from the MEASURED kink periods.
  (f) the period laws, the behaviour that distinguishes the two clocks.

PRIOR ART.  The marginal-stability reading of G*, the period law and the
lemniscate arc-length identity are established in
dissemination/papers/edge_clock/ ("The Clock at the Edge of Stability").
This figure draws them; it does not add to them.  Panel (d) and the
steerability reading are from FOUND_GSTAR_DEGENERACY_INTERPRETATION_v1.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
from scipy.integrate import quad
from scipy.special import gamma as Gam, beta as Bfn
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


GS = Gam(0.25) / Gam(0.75)
C = 1.0 / np.sqrt(3.0)

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks


def dticks(axis, t, fmt="{:g}"):
    fs.decade_ticks(axis, t, fmt)


# =====================================================================
def lemniscate(n=1200):
    """r^2 = cos 2 theta, both lobes."""
    th = np.linspace(-np.pi / 4 + 1e-9, np.pi / 4 - 1e-9, n)
    r = np.sqrt(np.cos(2 * th))
    x, y = r * np.cos(th), r * np.sin(th)
    return np.concatenate([x, -x[::-1]]), np.concatenate([y, -y[::-1]])


def lemniscate_arclength():
    """ds = d theta / sqrt(cos 2 theta); total = 4 * int_0^{pi/4}."""
    v, _ = quad(lambda t: 1.0 / np.sqrt(np.cos(2 * t)), 0, np.pi / 4,
                points=[np.pi / 4], limit=200)
    return 4 * v


# =====================================================================
def panel_curves(ax):
    th = np.linspace(0, 2 * np.pi, 400)
    ax.plot(np.cos(th), np.sin(th), color=C1, lw=1.8, label="circle")
    lx, ly = lemniscate()
    ax.plot(lx, ly, color=CO, lw=1.8, label="lemniscate")
    ax.plot([0], [0], "o", color=CK, ms=3.0)
    ax.set_aspect("equal")
    ax.set_xlim(-1.35, 1.35)
    # both labels must sit BELOW y = -1, outside the circle, or the circle
    # runs straight through them
    ax.set_ylim(-1.62, 1.18)
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([-1, 0, 1])
    ax.legend(loc="upper left", handlelength=1.5, borderpad=0.3,
              labelspacing=0.26, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(0.0, -1.20, "arc length $2\\pi = 6.2832$", fontsize=FS_ANN,
            color=C1, ha="center", va="top")
    ax.text(0.0, -1.42, "arc length $\\sqrt{\\pi}G^{*} = 5.2441$",
            fontsize=FS_ANN, color=CO, ha="center", va="top")
    ax.set_title("(a)  the constants are the curves:\n"
                 "$\\pi$ for one, $G^{*}$ for the other")


def orbit(E, V, xmax, n=500):
    """Closed level set p^2/2 + V(x) = E, built as one path.

    Plotting the two branches on a shared x-grid leaves visible GAPS at
    the turning points, where p -> 0 and the mask drops the endpoint."""
    xs = np.linspace(-xmax, xmax, n)
    p = np.sqrt(np.maximum(2.0 * (E - V(xs)), 0.0))
    return (np.concatenate([xs, xs[::-1], xs[:1]]),
            np.concatenate([p, -p[::-1], p[:1]]))


def panel_phase(ax):
    for E, al in zip((0.12, 0.28, 0.50, 0.78), (0.45, 0.6, 0.78, 1.0)):
        x, p = orbit(E, lambda t: 0.5 * t ** 2, np.sqrt(2 * E))
        ax.plot(x, p, color=C1, lw=1.2, alpha=al)
        x, p = orbit(E, lambda t: t ** 4, E ** 0.25)
        ax.plot(x, p, color=CO, lw=1.2, alpha=al)
    ax.set_aspect("equal")
    ax.set_xlim(-1.45, 1.45)
    ax.set_ylim(-1.45, 1.45)
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("$x$")
    ax.set_ylabel("$p$")
    # only the extreme corners are free of orbits; the title carries the
    # explanation, so the labels here name the families and nothing more
    ax.text(-1.41, 1.41, "harmonic", fontsize=FS_ANN, color=C1,
            ha="left", va="top")
    ax.text(1.41, -1.41, "quartic", fontsize=FS_ANN, color=CO,
            ha="right", va="bottom")
    ax.set_title("(b)  phase orbits: an ellipse keeps\n"
                 "time, an oval counts its own size")


def panel_pitchfork(ax):
    x = np.linspace(-1.5, 1.5, 500)
    for mu, col, lab in ((0.9, C1, "$\\mu>0$: one well, harmonic"),
                         (0.0, CO, "$\\mu=0$: the $G^{*}$ clock"),
                         (-0.9, C4, "$\\mu<0$: two wells")):
        ax.plot(x, 0.5 * mu * x ** 2 + x ** 4, color=col, lw=1.8, label=lab)
    ax.axhline(0, color=CG, lw=0.6, zorder=0)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-0.35, 1.55)
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([0, 1])
    ax.set_xlabel("$x$")
    ax.set_ylabel("$V$")
    ax.legend(loc="upper center", handlelength=1.5, borderpad=0.3,
              labelspacing=0.26, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.set_title("(c)  the clock lives on one surface:\n"
                 "the threshold between one well and two")


def panel_valley(ax):
    g = np.linspace(-1.5, 1.5, 320)
    X, Y = np.meshgrid(g, g)
    lv = np.array([0.02, 0.06, 0.15, 0.32, 0.6])
    ax.contour(X, Y, 0.5 * X ** 2 + 0.5 * Y ** 2, levels=lv,
               colors=C1, linewidths=1.0, linestyles="dashed")
    ax.contour(X, Y, 0.5 * X ** 2 + Y ** 4, levels=lv,
               colors=CO, linewidths=1.4)
    ax.plot([0], [0], "o", color=CK, ms=3.5)
    ax.annotate("", xy=(0.0, 0.90), xytext=(0.0, -0.90),
                arrowprops=dict(arrowstyle="<->", color=CO, lw=1.1))
    ax.set_aspect("equal")
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("stiff direction")
    ax.set_ylabel("clock direction")
    ax.text(-1.42, 1.36, "dashed: nondegenerate\nsolid: quartic valley",
            fontsize=FS_ANN, color=CK, ha="left", va="top")
    ax.text(0.12, -1.30, "flat $\\Rightarrow$ cheap to steer",
            fontsize=FS_ANN, color=CO, ha="left", va="bottom")
    ax.set_title("(d)  degeneracy, seen: the valley\n"
                 "opens along the clock direction")


def panel_worldlines(ax, T0, Tu, u):
    tmax = 190.0
    ax.plot([0, 0], [0, tmax], color=C1, lw=1.6)
    ax.plot([0, u * tmax], [0, tmax], color=CO, lw=1.6)
    for n in range(int(tmax // T0) + 1):
        ax.plot([-0.055 * tmax * C, 0.055 * tmax * C], [n * T0] * 2,
                color=C1, lw=1.6, solid_capstyle="butt")
    for n in range(int(tmax // Tu) + 1):
        t = n * Tu
        ax.plot([u * t - 0.055 * tmax * C, u * t + 0.055 * tmax * C],
                [t] * 2, color=CO, lw=1.6, solid_capstyle="butt")
    for s in (+1, -1):
        ax.plot([0, s * C * tmax], [0, tmax], color=CG, lw=0.8, ls=":")
    ax.set_xlim(-40, 118)
    ax.set_ylim(-8, tmax)
    ax.set_xticks([0, 50, 100])
    ax.set_yticks([0, 60, 120, 180])
    ax.set_xlabel("position")
    ax.set_ylabel("time  (ticks)")
    ax.text(-36, 178, f"at rest\n$T={T0:.1f}$", fontsize=FS_ANN, color=C1,
            ha="left", va="top")
    ax.text(112, 96, f"moving\n$T={Tu:.1f}$", fontsize=FS_ANN, color=CO,
            ha="right", va="center")
    # the left light line passes through (-36, 30); park this clear of both
    ax.text(114, 22, "dotted: light", fontsize=FS_ANN, color=CG,
            ha="right", va="center")
    ax.set_title("(e)  dilation, seen: the moving clock's\n"
                 "ticks are further apart in time")


def panel_laws(ax):
    A = np.logspace(np.log10(0.12), np.log10(1.0), 60)
    ax.loglog(A, np.ones_like(A), color=C1, lw=1.8,
              label="harmonic:  $T$ fixed")
    ax.loglog(A, A[0] / A, color=CO, lw=1.8,
              label="quartic:  $T \\propto 1/A$")
    ax.set_xlim(0.11, 1.1)
    ax.set_ylim(0.09, 1.7)
    dticks(ax.xaxis, [0.2, 0.5, 1.0])
    dticks(ax.yaxis, [0.1, 0.3, 1.0])
    ax.set_xlabel("amplitude  $A$")
    ax.set_ylabel("period, normalised")
    ax.legend(loc="lower left", handlelength=1.6, borderpad=0.3,
              labelspacing=0.26, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(1.05, 1.12, "$T\\!\\cdot\\!A=\\sqrt{\\pi}G^{*}$", fontsize=FS_ANN,
            color=CO, ha="right", va="bottom")
    ax.set_title("(f)  and the behaviour that follows:\n"
                 "one ignores amplitude, one counts it")


def main():
    print("Clock geometry figure")
    L = lemniscate_arclength()
    print(f"  [verify] lemniscate arc length = {L:.9f}")
    print(f"  [verify] sqrt(pi) G*           = {np.sqrt(np.pi)*GS:.9f}")
    print(f"  [verify] B(1/4,1/2)            = {Bfn(0.25,0.5):.9f}")
    assert abs(L - np.sqrt(np.pi) * GS) < 1e-7, "lemniscate arc length"
    assert abs(Bfn(0.25, 0.5) - np.sqrt(np.pi) * GS) < 1e-12, "Beta identity"
    print(f"  [verify] circle circumference  = {2*np.pi:.9f} = 2 pi")
    print(f"  [verify] ratio 2pi / sqrt(pi)G* = {2*np.pi/(np.sqrt(np.pi)*GS):.9f}")

    # measured kink periods (lam = 0.03), from fig_carrier_arc
    T0, Tu, u = 29.6, 35.0, 0.5 * C
    print(f"  [verify] measured kink periods: rest {T0}, at u=0.5C {Tu}"
          f"   ratio {Tu/T0:.4f}  (ideal gamma {1/np.sqrt(1-0.25):.4f})")

    # 7.55, not the aspect-preserving 7.94: this figure carries the
    # longest caption in the paper and overflowed its page by 24 pt.
    fig, axes = plt.subplots(3, 2, figsize=(fs.TEXTWIDTH_IN, 7.5500),
                             constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)
    panel_curves(axes[0, 0])
    panel_phase(axes[0, 1])
    panel_pitchfork(axes[1, 0])
    panel_valley(axes[1, 1])
    panel_worldlines(axes[2, 0], T0, Tu, u)
    panel_laws(axes[2, 1])
    fs.save(fig, "clockgeometry")
    print(f"  wrote {fs.FIGDIR / 'clockgeometry.pdf'}")


if __name__ == "__main__":
    main()
