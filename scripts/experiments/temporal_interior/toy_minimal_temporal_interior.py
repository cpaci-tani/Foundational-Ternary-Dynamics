"""toy_minimal_temporal_interior.py — a complete, computable toy model of
the semantic-ontology architecture.

THE MODEL.  Five COEXISTING pieces -- not coupled: each panel is an
independent computation and no module consumes another's state.  They
share module-level constants, and that is the whole of the composition
claim.  Each is minimal and has an exact or independently-known answer,
so every number below is checkable:

  1. SUBSTRATE   a discrete field on a lattice with a causal polytope and
                 an emergent light cone (the containment result).
  2. CLOCK       the minimum viable carrier's mirror-even mode, an exact
                 quartic oscillator:  Q'' = -(4 lam/m) Q^3,
                 period law  T*A = sqrt(pi) G* sqrt(m/2 lam).
  3. REGISTER    a bistable coordinate in a double well of barrier eps,
                 retention tau_flip ~ exp(eps/T)  (Kramers/Arrhenius).
  4. GATE        a threshold on |J| that converts continuous potentiality
                 into discrete actual events.
  5. NOISE       two channels, fast (tau_f) and slow (tau_s), whose ratio
                 to the mode frequency sets the weighting regime.

WHAT IT DEMONSTRATES, end to end:
  * succession -> duration      (the clock keeps time at G*)
  * passage -> retention        (the register holds a bit for exp(eps/T))
  * potentiality -> actuality   (the gate produces singular events)
  * the weighting of those events interpolates amplitude -> Born as the
    mode outruns the noise bandwidth
  * eps prices binding AND memory (panels d, e) -- NOT the clock rate:
    the clock integrates its own hard-coded lam, so the third role of
    eps is asserted by the architecture and not exhibited here

Outputs a six-panel figure with strict typographic discipline: one font
size scheme, no rotated text except axis labels, no overlapping artists,
generous panel spacing.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt
from matplotlib.patches import Polygon, Circle
from scipy.special import gamma as Gamma
from scipy.integrate import solve_ivp


G_STAR = Gamma(0.25) / Gamma(0.75)
C_CONE = 1.0 / np.sqrt(3.0)
EPS = 1.0                     # bond depth = barrier; drives (d) and (e)
                              # ONLY -- the clock's lam is independent
K_THRESH = 0.5054620197

# ----- one typographic scheme, applied everywhere ---------------------
FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, C2, C3, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks


# =====================================================================
# 1. SUBSTRATE — causal polytopes and the contained light cone
# =====================================================================
def panel_substrate(ax):
    sq = Polygon([(1, 1), (-1, 1), (-1, -1), (1, -1)], closed=True,
                 fill=False, ec=CG, lw=1.4, ls="--", zorder=2)
    di = Polygon([(1, 0), (0, 1), (-1, 0), (0, -1)], closed=True,
                 fill=False, ec=C1, lw=1.8, zorder=3)
    ax.add_patch(sq)
    ax.add_patch(di)
    ax.add_patch(Circle((0, 0), C_CONE, fill=True, fc=C2, alpha=0.16,
                        ec=C2, lw=1.8, zorder=4))
    ax.plot([0], [0], marker="o", ms=3.5, color=CK, zorder=6)

    ax.annotate("Moore reach\n(inradius 1)", xy=(-1.0, 1.0), xytext=(-1.98, 1.52),
                fontsize=FS_ANN, color=CG, ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.annotate("octahedral reach\n(inradius $1/\\sqrt{3}$)", xy=(0.5, 0.5),
                xytext=(0.72, 1.52), fontsize=FS_ANN, color=C1,
                ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=C1, lw=0.7))
    ax.annotate("light cone\n$C=1/\\sqrt{3}$", xy=(-0.34, -0.34),
                xytext=(-1.98, -1.62), fontsize=FS_ANN, color=C2,
                ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=C2, lw=0.7))

    ax.set_xlim(-2.15, 2.15)
    ax.set_ylim(-2.05, 2.05)
    ax.set_aspect("equal")
    ax.set_xticks([-1, 0, 1])
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("lattice cells per tick")
    ax.set_title("(a)  substrate: the light cone sits inside every\n"
                 "candidate causal polytope")


# =====================================================================
# 2. CLOCK — the exact quartic oscillator
# =====================================================================
def clock_period(A0, lam=2.0, m=4.0, span=2.6):
    """Exact period of Q'' = -(4 lam/m) Q^3 by event detection.
    The first upward zero of Qdot is the far turning point = half period."""
    def rhs(t, y):
        return [y[1], -(4 * lam / m) * y[0] ** 3]

    def turn(t, y):
        return y[1]
    turn.direction = 1.0
    Tg = np.sqrt(np.pi) * G_STAR * np.sqrt(m / (2 * lam)) / A0
    sol = solve_ivp(rhs, [0, span * Tg], [A0, 0.0], events=turn,
                    rtol=1e-12, atol=1e-14, dense_output=True, max_step=Tg/60)
    return 2.0 * sol.t_events[0][0], sol


def panel_clock(ax):
    """Unscaled Q(t): the periods visibly differ, which is the whole point."""
    lam, m = 2.0, 4.0
    tmax = 46.0
    for A0, col, ls in [(0.30, C1, "-"), (0.20, C4, "--"),
                        (0.12, C2, "-.")]:
        T, sol = clock_period(A0, lam, m, span=2.6)
        tend = min(tmax, sol.t[-1])
        ts = np.linspace(0, tend, 1400)
        ax.plot(ts, sol.sol(ts)[0], color=col, ls=ls, lw=1.5,
                label=f"$A={A0:.2f}$,   $T={T:.1f}$")
        ax.plot([T], [A0], marker="|", ms=9, color=col, mew=1.6)
    ax.axhline(0, color=CG, lw=0.6, zorder=0)
    ax.set_xlabel("time  $t$")
    ax.set_ylabel("clock coordinate  $Q$")
    ax.set_xlim(0, tmax)
    ax.set_ylim(-0.34, 0.52)
    ax.set_yticks([-0.3, 0.0, 0.3])
    ax.legend(frameon=False, loc="upper right", handlelength=1.5,
              borderpad=0.2, labelspacing=0.28)
    ax.set_title("(b)  clock: a bigger swing is a $faster$ clock\n"
                 "(ticks mark one period)")




def panel_clock_recovery(ax):
    """log-log T vs A: slope exactly -1, intercept exactly sqrt(pi) G*."""
    lam, m = 2.0, 4.0
    amps = np.array([0.08, 0.12, 0.18, 0.25, 0.35, 0.50])
    Ts = np.array([clock_period(a, lam, m)[0] for a in amps])
    Grec = Ts * amps * np.sqrt(2 * lam / m) / np.sqrt(np.pi)
    slope = np.polyfit(np.log(amps), np.log(Ts), 1)[0]
    dev = np.abs(Grec - G_STAR).max() / G_STAR

    af = np.logspace(np.log10(0.07), np.log10(0.58), 100)
    ax.loglog(af, np.sqrt(np.pi) * G_STAR * np.sqrt(m / (2 * lam)) / af,
              color=CK, ls=":", lw=1.2, label="$T=\\sqrt{\\pi}\\,G^*\\!/A$")
    ax.loglog(amps, Ts, "o", color=C2, ms=5.0, label="integrated")
    ax.set_xlabel("amplitude  $A$")
    ax.set_ylabel("period  $T$")
    ax.set_xlim(0.065, 0.62)
    ax.set_ylim(9.0, 78.0)
    # plain decimal ticks: the default log minor labels collide (3e-1 / 4e-1)
    decade_ticks(ax.xaxis, [0.08, 0.12, 0.2, 0.3, 0.5])
    decade_ticks(ax.yaxis, [10, 20, 40, 60], "{:.0f}")
    ax.legend(frameon=False, loc="upper right", handlelength=1.6,
              borderpad=0.2, labelspacing=0.3)
    ax.text(0.075, 12.0, f"slope $= {slope:.6f}$\n"
                         f"$G^*$ recovered to ${dev:.0e}$",
            fontsize=FS_ANN, color=CK, ha="left", va="center")
    ax.set_title("(c)  the clock law, with no fitted scale:\n"
                 "slope $-1$ and the constant is $G^*$")
    print(f"  [verify] clock: slope = {slope:.9f}  (exact -1)")
    print(f"  [verify] clock: max |G*_rec/G*-1| = {dev:.3e}")


# =====================================================================
# 3. REGISTER — double well, Arrhenius retention
# =====================================================================
def panel_register(ax):
    R = np.linspace(-1.65, 1.65, 700)
    V = EPS * (R**2 - 1.0)**2
    ax.plot(R, V, color=C3, lw=1.8, zorder=3)
    ax.axhline(EPS, color=CG, ls=":", lw=1.0, zorder=1)
    ax.plot([-1, 1], [0, 0], "o", color=CK, ms=5.5, zorder=4)
    ax.annotate("", xy=(0, EPS), xytext=(0, 0.0),
                arrowprops=dict(arrowstyle="<->", color=C2, lw=1.2))
    # above the peak: the only region wide enough to clear both well arms
    ax.text(0.0, 1.09, "barrier $=\\varepsilon$", fontsize=FS_ANN,
            color=C2, ha="center", va="bottom")
    ax.text(-1.0, -0.24, "state $0$", fontsize=FS_ANN, ha="center", color=CK)
    ax.text(1.0, -0.24, "state $1$", fontsize=FS_ANN, ha="center", color=CK)
    ax.set_xlabel("register coordinate  $R$")
    ax.set_ylabel("$V(R)/\\varepsilon$")
    ax.set_xlim(-1.75, 1.75)
    ax.set_ylim(-0.42, 1.62)
    ax.set_yticks([0, 1])
    ax.set_title("(d)  register: two states, one barrier,\n"
                 "and it is the same $\\varepsilon$")


def panel_retention(ax):
    T = np.logspace(-1.35, -0.25, 260)
    tau = np.exp(EPS / T)
    ax.loglog(T, tau, color=C3, lw=1.8, zorder=3)
    for Tm, dx, dy in ((0.06, 9, 6), (0.12, 9, 6), (0.25, 9, 6)):
        ax.plot([Tm], [np.exp(1 / Tm)], "o", color=C3, ms=4.5, zorder=4)
        ax.annotate(f"$T={Tm}$", xy=(Tm, np.exp(1 / Tm)),
                    xytext=(dx, dy), textcoords="offset points",
                    fontsize=FS_ANN, color=CK, ha="left", va="bottom")
    ax.set_xlabel("noise temperature  $T/\\varepsilon$")
    ax.set_ylabel("retention  $\\tau_{\\rm flip}\\,\\nu_0$")
    ax.set_xlim(0.043, 0.58)
    decade_ticks(ax.xaxis, [0.05, 0.1, 0.2, 0.4])
    ax.set_title("(e)  retention is Arrhenius:\n"
                 "$\\tau_{\\rm flip}\\sim\\nu_0^{-1}e^{\\varepsilon/T}$")


# =====================================================================
# 4-5. GATE + NOISE — the weighting regime
# =====================================================================
def born_fraction(lam1, lam2, tau, L=1536, T=6000, seeds=10, sigma=0.17,
                  A1=0.10, rng0=7, boot=800):
    """Returns (BF, 1-sigma CI half-widths (lo,hi), Om_bar*tau).
    Per-seed excess is retained so the estimate carries a bootstrap CI --
    this instrument is deliberately small, so the uncertainty is real and
    is shown rather than hidden."""
    k1, k2 = 2 * np.pi / lam1, 2 * np.pi / lam2
    Om = lambda k: 2 * np.arcsin(np.clip(C_CONE * np.sin(k / 2), -1, 1))
    O1, O2 = Om(k1), Om(k2)
    A2 = A1 * np.sqrt(O1 / O2)
    x = np.arange(L)
    p1, p2 = np.cos(k1 * x), np.cos(k2 * x)
    a = np.exp(-1.0 / tau)
    s = sigma * np.sqrt(1 - a * a)
    th = np.random.default_rng(rng0).uniform(0, 2 * np.pi, 2)
    X = np.column_stack([np.ones(L), np.cos(2 * k1 * x), np.cos(2 * k2 * x)])
    Ramp = O1 / O2
    per_seed = np.zeros((seeds, L))
    for sd in range(seeds):
        rg = np.random.default_rng(1000 + 31 * sd)
        d = rg.standard_normal((T, L))
        sig_c = None
        for mode, acc in (("s", 1.0), ("c", 0.0)):
            xi = np.zeros(L)
            prev = acc * (A1 * p1 * np.cos(th[0]) + A2 * p2 * np.cos(th[1]))
            c = np.zeros(L)
            for t in range(1, T):
                xi = a * xi + s * d[t]
                F = xi + acc * (A1 * p1 * np.cos(O1 * t + th[0])
                                + A2 * p2 * np.cos(O2 * t + th[1]))
                c += (prev < K_THRESH) & (F >= K_THRESH)
                prev = F
            if mode == "s":
                sig_c = c
            else:
                per_seed[sd] = sig_c - c

    def bf_of(ex):
        co, *_ = np.linalg.lstsq(X, ex, rcond=None)
        return ((co[2] / co[1]) - Ramp) / (1 - Ramp)

    bf = bf_of(per_seed.mean(axis=0))
    rg = np.random.default_rng(99)
    bs = [bf_of(per_seed[rg.integers(0, seeds, seeds)].mean(axis=0))
          for _ in range(boot)]
    lo, hi = np.percentile(bs, [16, 84])
    return bf, (bf - lo, hi - bf), np.sqrt(O1 * O2) * tau


def panel_regime(ax):
    cells = [(64, 32, 4), (32, 16, 8), (16, 8, 16), (16, 8, 48), (8, 4, 96)]
    xs, ys, el, eh = [], [], [], []
    for l1, l2, tau in cells:
        bf, (dlo, dhi), xt = born_fraction(l1, l2, tau)
        xs.append(xt); ys.append(bf); el.append(dlo); eh.append(dhi)
        print(f"  [verify] gate: Om*tau={xt:7.2f}  BF={bf:6.3f} "
              f"(-{dlo:.3f}/+{dhi:.3f})")
    xs, ys = np.array(xs), np.array(ys)
    xf = np.logspace(-0.7, 2.7, 300)
    ax.semilogx(xf, 0.860 * xf**2 / (16.6**2 + xf**2), color=CK, ls=":",
                lw=1.2, label="locked-run reference")
    ax.errorbar(xs, ys, yerr=[el, eh], fmt="o", color=C2, ms=5.0, lw=1.2,
                capsize=2.5, label="toy model  ($1\\sigma$)")
    ax.axhline(0.0, color=CG, lw=0.6, zorder=0)
    ax.set_xlabel("$\\bar\\Omega\\,\\tau$   (mode frequency $\\times$ noise time)")
    ax.set_ylabel("Born-fraction")
    ax.set_ylim(-0.14, 1.16)
    ax.set_xlim(0.2, 500)
    ax.set_yticks([0.0, 0.5, 1.0])
    # lifted clear of the leftmost point and its error bar (top at BF~0.06)
    ax.text(0.23, 0.20, "amplitude\nweighting", fontsize=FS_ANN, color=CK,
            ha="left", va="bottom")
    ax.text(420, 0.66, "Born\nweighting", fontsize=FS_ANN, color=CK,
            ha="right", va="top")
    ax.legend(frameon=False, loc="upper left", handlelength=1.6,
              borderpad=0.2, labelspacing=0.3)
    ax.set_title("(f)  gate: the weighting of actual events\n"
                 "interpolates with the timescale ratio")


# =====================================================================
def main():
    print("Minimal Temporal Interior — toy model")
    print(f"  G* = {G_STAR:.12f},  C = 1/sqrt(3) = {C_CONE:.6f},  "
          f"eps = {EPS}, K = {K_THRESH}")
    fig, axes = plt.subplots(3, 2, figsize=(fs.TEXTWIDTH_IN, 7.9013),
                             constrained_layout=True)
    panel_substrate(axes[0, 0])
    panel_clock(axes[0, 1])
    panel_clock_recovery(axes[1, 0])
    panel_register(axes[1, 1])
    panel_retention(axes[2, 0])
    panel_regime(axes[2, 1])
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12, hspace=0.07,
                                wspace=0.07)
    fs.save(fig, "toymodel")
    print(f"  wrote {fs.FIGDIR/'toymodel.pdf'}")


if __name__ == "__main__":
    main()
