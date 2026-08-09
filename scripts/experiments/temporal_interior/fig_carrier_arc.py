"""fig_carrier_arc.py — the carrier arc, shown as BEHAVIOUR not statistics.

An earlier version of this figure plotted fitted dilation exponents as
scatter points: five numbers here, nine there, and one panel that simply
replotted two others side by side.  Those are tables wearing a plot's
clothes.  The claim of this arc is that a clock ticks slower when it
moves, and that is something you can watch happen -- so this version shows
the raw traces and the raw curves, and lets the fitted numbers stay in the
text where they belong.

FOUR PANELS:
  (a) the carrier itself: the kink profile at rest and at u = 0.5 C, so
      the Lorentz contraction of the object is visible.
  (b) THE CLAIM, DIRECTLY: the internal clock's trace at rest and boosted,
      same window, starting in phase.  The moving clock visibly falls
      behind.  That is time dilation, watched rather than inferred.
  (c) THE COMPARISON: Omega/Omega_0 against gamma.  Two-category carriers
      FAN OUT away from 1/gamma -- different materials, different curves.
      One-energy carriers COLLAPSE onto it.  Universality as convergence
      of curves, not as a spread of dots.
  (d) why it is still not the G* clock: the shape-mode frequency is flat
      in amplitude, where the quartic law would fall as 1/A.

SCOPE: illustrates results established elsewhere; introduces no new claim.
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
from scipy.linalg import toeplitz, eigh
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


C = 1.0 / np.sqrt(3.0)
C2 = C * C

FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG
decade_ticks = fs.decade_ticks

NK, DK = 4096, 900.0
XK = (np.arange(NK) - NK // 2).astype(float)


def dticks(axis, t, fmt="{:g}"):
    fs.decade_ticks(axis, t, fmt)


# ---------------- one-energy carrier: phi^4 kink ---------------------
def kink(lam, v, u, sq=1.0):
    w = C * np.sqrt(2.0 / lam) / v
    g = 1.0 / np.sqrt(1.0 - (u / C) ** 2)
    a, b = g * (XK + DK) / (w * sq), g * (XK - DK) / (w * sq)
    phi = v * (np.tanh(a) - np.tanh(b) - 1.0)
    dot = -(u * g * v / (w * sq)) * (np.cosh(a) ** -2 - np.cosh(b) ** -2)
    return phi, dot, np.sqrt(2 * lam) * v, w, g


def kink_trace(lam, v, u, T=24000, sq=0.93):
    phi, dot, m, w, g = kink(lam, v, u, sq)
    prev = phi - dot
    tr = np.empty(T)
    for t in range(T):
        acc = C2 * (np.roll(phi, 1) - 2 * phi + np.roll(phi, -1)) \
            - lam * phi * (phi * phi - v * v)
        phi, prev = 2 * phi - prev + acc, phi
        gx = 0.5 * (np.roll(phi, -1) - np.roll(phi, 1))
        tr[t] = np.abs(gx).max()
    s = tr[int(T * 0.2):]
    s = s - s.mean()
    P = np.abs(np.fft.rfft(s * np.hanning(len(s)))) ** 2
    f = np.fft.rfftfreq(len(s), 1.0)
    return tr, 2 * np.pi * f[np.argmax(P[3:]) + 3], m, w, g


# ---------------- two-category carrier -------------------------------
LB = 512


def w_lat(q, M):
    W = 4.0 * C2 * np.sin(np.asarray(q) / 2.0) ** 2 + M * M
    return 2.0 * np.arcsin(np.sqrt(W) / 2.0)


def two_cat(K, G, R, M):
    q = 2.0 * np.pi * np.arange(LB) / LB
    t = np.fft.ifft(w_lat(q, M) + w_lat(K - q, M))
    r = np.arange(LB)
    d = np.minimum(r, LB - r).astype(float)
    ev = eigh(toeplitz(t, np.conj(t)) + np.diag(-G / np.cosh(d / R) ** 2),
              eigvals_only=True)
    return ev[0], ev[1]


# ---------------- panels ---------------------------------------------
def panel_profile(ax, lam=0.03, v=1.0):
    # gamma = 1.155 is a 13% narrowing and invisible at this zoom; use a
    # harder boost so the contraction is actually seen
    for u, col, ls, lab in ((0.0, C1, "-", "at rest"),
                            (0.82 * C, CO, "--", "at $u=0.82\\,C$")):
        phi, _, _, w, g = kink(lam, v, u)
        sel = np.abs(XK + DK) < 13
        ax.plot(XK[sel] + DK, phi[sel], ls, color=col, lw=1.8,
                label=f"{lab}   ($\\gamma={g:.3f}$)")
    ax.axhline(0, color=CG, lw=0.6, zorder=0)
    ax.set_xlim(-13, 13)
    ax.set_ylim(-1.35, 1.35)
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("position  (lattice sites)")
    ax.set_ylabel("field  $\\varphi$")
    ax.legend(loc="upper left", handlelength=1.6, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    # point BETWEEN the two curves, where the gap is widest
    ax.annotate("narrower by $1/\\gamma$", xy=(-2.2, -0.52),
                xytext=(1.2, -1.07), fontsize=FS_ANN, color=CO,
                ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.set_title("(a)  the carrier: one field, and it\n"
                 "contracts when it moves")


def panel_ticking(ax, tr0, trU, om0, omU):
    """The full run on one axis.

    Fourteen cycles of each trace: viable only because the boosted trace
    is thin and dense-dashed, so the two read as interleaved curves and
    the slow beat between them -- in phase at release, drifting apart --
    IS the visible content of the full span.
    """
    n = 430
    a = tr0[:n] - tr0[:n].mean()
    b = trU[:n] - trU[:n].mean()
    a, b = a / np.abs(a).max(), b / np.abs(b).max()
    t = np.arange(n)
    ax.plot(t, a, color=C1, lw=1.1,
            label=f"at rest,  $T={2*np.pi/om0:.1f}$")
    ax.plot(t, b, color=CO, lw=0.8, ls=(0, (2.2, 1.0)),
            label=f"boosted,  $T={2*np.pi/omU:.1f}$")
    ax.axhline(0, color=CG, lw=0.6, zorder=0)
    ax.set_xlim(0, n)
    ax.set_ylim(-1.25, 1.95)
    ax.set_yticks([-1, 0, 1])
    ax.set_xlabel("ticks")
    ax.set_ylabel("clock coordinate")
    ax.legend(loc="upper right", handlelength=1.6, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0, ncol=1,
              edgecolor="none", facecolor="white").set_zorder(9)
    lag = n * (om0 - omU) / om0 / (2 * np.pi / om0)
    # top-left band (above the traces); three short lines, because the
    # legend owns the right half of the same band and any line past
    # ~0.5 axes width runs under its opaque face
    ax.text(0.02, 0.975, f"released in phase;\nby tick {n} they are\n"
            f"{lag:.1f} cycles apart", fontsize=FS_ANN, color=CK,
            ha="left", va="top", transform=ax.transAxes, linespacing=1.05)
    ax.set_title("(b)  the same clock, at rest and moving:\n"
                 "the moving one ticks slower")


def panel_collapse(ax, two, one):
    g = np.linspace(1.0, 1.30, 100)
    ax.plot(g, 1.0 / g, color=CK, lw=1.8, ls="--", zorder=3,
            label="relativity, $1/\\gamma$")
    dash_cycle = ("-", "--", "-.", ":", (0, (3, 1, 1, 1)))
    for (gs, rs), col, ls in zip(
            two, (CO, C4, CG_, "#8a1c4a", "#70572d"), dash_cycle):
        ax.plot(gs, rs, color=col, ls=ls, lw=1.0, alpha=0.9, zorder=2)
        ax.plot(gs[-1:], rs[-1:], "o", color=col, ms=3.6, zorder=2)
    for (gs, rs), mk in zip(one, ("o", "s", "^")):
        ax.plot(gs, rs, mk + "-", color=C1, ms=4.2, lw=1.4, zorder=4)
    ax.set_xlim(0.995, 1.235)
    ax.set_ylim(0.70, 1.30)
    ax.set_xlabel("$\\gamma$")
    ax.set_ylabel("$\\Omega(\\gamma)\\,/\\,\\Omega(0)$")
    ax.legend(loc="lower left", handlelength=1.8, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(1.228, 1.245, "two energies:\ndifferent materials,\n"
                          "different curves", fontsize=FS_ANN, color=CO,
            ha="right", va="top")
    # top-left is the only region no curve enters; a thin leader ties the
    # label to the blue bundle it names (every lower placement collided
    # with either the descending curves or the legend)
    ax.annotate("one energy: every carrier\non the same curve",
                xy=(1.185, 0.82), xytext=(1.005, 1.285),
                fontsize=FS_ANN, color=C1, ha="left", va="top", zorder=8,
                arrowprops=dict(arrowstyle="->", color=CG, lw=0.8,
                                shrinkB=3))
    ax.set_title("(c)  universality is curves collapsing,\n"
                 "not numbers agreeing")


def panel_isochrony(ax, amps, oms, om0):
    A = np.array(amps)
    ax.plot(A, np.array(oms) / om0, "D-", color=C4, ms=5.0,
            label="measured shape mode")
    ax.plot(A, A[0] / A, color=CO, lw=1.6, ls=":",
            label="a $G^*$ clock would go as $1/A$")
    ax.axhline(1.0, color=CG, lw=0.8, ls="-", zorder=0)
    ax.set_xlim(A.min() * 0.88, A.max() * 1.12)
    ax.set_ylim(0.12, 1.32)
    ax.set_xlabel("perturbation amplitude")
    ax.set_ylabel("frequency, normalised")
    # lower-left would sit on the 1/A curve and hide it
    ax.legend(loc="center right", handlelength=1.8, borderpad=0.3,
              labelspacing=0.28, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(A.mean(), 1.08, "flat: a $\\pi$-clock, not a $G^*$ clock",
            fontsize=FS_ANN, color=C4, ha="center", va="bottom")
    ax.set_title("(d)  the carrier that works is\n"
                 "isochronous, so $G^*$ is still unbought")


def main():
    print("Carrier arc figure — behaviour, not summary statistics")
    v, lam = 1.0, 0.03

    tr0, om0, m, w, _ = kink_trace(lam, v, 0.0)
    trU, omU, _, _, gU = kink_trace(lam, v, 0.5 * C)
    print(f"  [verify] rest  Omega={om0:.6f}  predicted {np.sqrt(3)/2*m:.6f}"
          f"  ratio {om0/(np.sqrt(3)/2*m):.4f}")
    print(f"  [verify] boost Omega={omU:.6f}  gamma={gU:.5f}  "
          f"Omega/Omega0={omU/om0:.5f}  vs 1/gamma={1/gU:.5f}")

    # (c) both families sampled over the SAME gamma range.  Different
    # materials reach a given gamma at very different K, so scan K and
    # keep the portion with gamma <= GMAX rather than fixing K.
    GMAX = 1.30
    two = []
    for M, G, R in ((0.40, 0.10, 6), (0.40, 0.30, 3), (0.25, 0.06, 7),
                    (0.60, 0.20, 5), (0.80, 0.40, 4)):
        a0, b0 = two_cat(0.0, G, R, M)
        gap0 = b0 - a0
        gs, rs = [1.0], [1.0]
        for K in np.linspace(0.15, 2.4, 15):
            a, b = two_cat(K, G, R, M)
            g = a / a0
            if g > GMAX:
                break
            gs.append(g)
            rs.append((b - a) / gap0)
        two.append((np.array(gs), np.array(rs)))
    ends = np.array([np.interp(1.22, g, r) for g, r in two])
    print(f"  [verify] two-category, at gamma=1.22: ratios "
          f"{np.round(ends,3)}  spread {ends.max()-ends.min():.3f}"
          f"   (1/gamma = {1/1.22:.3f})")
    assert ends.max() - ends.min() > 0.15, "two-category fan unexpectedly tight"

    one = []
    for lm in (0.02, 0.03, 0.04):
        _, o0, _, _, _ = kink_trace(lm, v, 0.0)
        gs, rs = [1.0], [1.0]
        for uc in (0.25, 0.40, 0.50, 0.58):
            _, o, _, _, g = kink_trace(lm, v, uc * C)
            gs.append(g)
            rs.append(o / o0)
        one.append((np.array(gs), np.array(rs)))
    oe = np.array([np.interp(1.22, g, r) for g, r in one])
    print(f"  [verify] one-energy,   at gamma=1.22: ratios "
          f"{np.round(oe,3)}  spread {oe.max()-oe.min():.3f}")
    print(f"  [verify] fan-out ratio two/one = "
          f"{(ends.max()-ends.min())/(oe.max()-oe.min()):.1f}x")
    dev = max(np.abs(rs - 1.0 / gs).max() for gs, rs in one)
    print(f"  [verify] one-energy max |Omega/Omega0 - 1/gamma| = {dev:.4f}")
    assert dev < 0.05, "one-energy curves are not tracking 1/gamma"

    amps, oms = [], []
    for sq in (0.97, 0.93, 0.88, 0.82):
        _, o, _, _, _ = kink_trace(lam, v, 0.0, T=16000, sq=sq)
        amps.append(1.0 - sq)
        oms.append(o)
    binw = 2 * np.pi / (0.8 * 16000)
    print(f"  [verify] isochrony: freqs {np.round(oms,6)}; spread "
          f"{np.ptp(oms):.2e} vs FFT bin {binw:.2e} "
          f"=> flat to <= {binw/oms[0]:.2%}")

    fig = plt.figure(figsize=(fs.TEXTWIDTH_IN, 5.3228),
                     constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.10, h_pad=0.12,
                                hspace=0.07, wspace=0.07)
    gs = fig.add_gridspec(2, 2)
    axA = fig.add_subplot(gs[0, 0])
    axB = fig.add_subplot(gs[0, 1])
    axC = fig.add_subplot(gs[1, 0])
    axD = fig.add_subplot(gs[1, 1])
    panel_profile(axA)
    panel_ticking(axB, tr0, trU, om0, omU)
    panel_collapse(axC, two, one)
    panel_isochrony(axD, amps, oms, oms[0])
    fs.save(fig, "carrierarc")
    print(f"  wrote {fs.FIGDIR / 'carrierarc.pdf'}")


if __name__ == "__main__":
    main()
