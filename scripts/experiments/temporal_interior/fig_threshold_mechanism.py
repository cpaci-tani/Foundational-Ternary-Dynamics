"""fig_threshold_mechanism.py -- what a threshold counts, drawn.

CLAIM (paper section "What a threshold counts"): a detection rate is
presentations per unit time times success per presentation; presentations
go as frequency, success goes as amplitude, and their product is
occupation n = E/Omega -- so the 1/sqrt(2 Omega) of field theory is the
conversion from how big a wave is to how often it knocks.

The section's RESULT figure (bornregime) shows the measured interpolation.
This is the MECHANISM figure, and it is deliberately noise-free: adding a
noise trace would silently place the cartoon in one regime or the other,
which is the measured result's job, not the mechanism's.  Two modes at
EQUAL OCCUPATION -- one tall and slow, one short and fast -- against two
thresholds:

  a LOW threshold both modes clear: crossings count PRESENTATIONS, and
  the fast mode logs exactly Omega2/Omega1 = 4x as many;

  a HIGH threshold only the tall mode reaches: SUCCESS gates on
  amplitude, and the fast mode logs zero.

A noisy threshold samples between these two faces, and the product the
two faces multiply to is occupation.  Every crossing shown is computed
from the plotted trace; the [verify] block asserts the 4:1 count ratio
and the amplitude gate exactly.
"""
from __future__ import annotations

import numpy as np
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG

OM1, OM2 = 0.40, 1.60             # slow, fast: ratio exactly 4
A1 = 1.00
A2 = A1 * np.sqrt(OM1 / OM2)      # equal occupation n = Omega A^2 / 2
TH_LO, TH_HI = 0.30, 0.80         # A2 = 0.5 sits between them
T_END = 47.124                    # exactly 3 slow periods = 12 fast ones
DT = 0.002


def upcross(sig, theta):
    return np.where((sig[:-1] < theta) & (sig[1:] >= theta))[0]


def main():
    print("Threshold-mechanism figure")
    t = np.arange(0.0, T_END, DT)
    s1 = A1 * np.sin(OM1 * t)
    s2 = A2 * np.sin(OM2 * t)

    n1, n2 = 0.5 * OM1 * A1 ** 2, 0.5 * OM2 * A2 ** 2
    print(f"  [verify] occupations equal: n1 = {n1:.6f}, n2 = {n2:.6f}")
    assert abs(n1 - n2) < 1e-12

    lo1, lo2 = upcross(s1, TH_LO), upcross(s2, TH_LO)
    hi1, hi2 = upcross(s1, TH_HI), upcross(s2, TH_HI)
    print(f"  [verify] LOW threshold  ({TH_LO}): slow {len(lo1)}, "
          f"fast {len(lo2)}  -> ratio {len(lo2)/len(lo1):.2f} "
          f"(presentations, = Omega2/Omega1 = {OM2/OM1:.0f})")
    print(f"  [verify] HIGH threshold ({TH_HI}): slow {len(hi1)}, "
          f"fast {len(hi2)}  (success gates on amplitude)")
    assert len(lo2) == 4 * len(lo1), "presentation ratio must be exactly 4"
    assert len(hi2) == 0 and len(hi1) > 0, "amplitude gate failed"

    fig, axes = plt.subplots(2, 1, figsize=(fs.TEXTWIDTH_IN, 3.6),
                             sharex=True)

    # -- (a) equal occupation, seen
    ax = axes[0]
    ax.plot(t, s1, color=C1, lw=1.5,
            label=r"slow: $\Omega=0.4$, $A=1.0$")
    ax.plot(t, s2, color=CO, lw=1.1,
            label=r"fast: $\Omega=1.6$, $A=0.5$")
    ax.axhline(0, color=CG, lw=0.5)
    ax.set_ylabel("signal")
    ax.set_ylim(-1.35, 1.75)
    ax.legend(loc="upper right", ncols=2, frameon=True, framealpha=1.0,
              edgecolor="none", facecolor="white").set_zorder(9)
    ax.text(0.8, 1.38,
            r"equal occupation $n=\tfrac12\Omega A^{2}$: half the height,"
            r" four times the knocks",
            fontsize=FS_ANN, color=CK, ha="left", va="center")
    ax.set_title("(a)  two modes a threshold cannot tell apart by occupation")

    # -- (b) the two faces of the count
    ax = axes[1]
    ax.plot(t, s1, color=C1, lw=1.5)
    ax.plot(t, s2, color=CO, lw=1.1)
    for th, lab in ((TH_LO, "low threshold: counts presentations"),
                    (TH_HI, "high threshold: gates on amplitude")):
        ax.axhline(th, color=CK, lw=1.0, ls="--")
        ax.text(T_END - 0.6, th + 0.05, lab, fontsize=FS_ANN, color=CK,
                ha="right", va="bottom")
    ax.plot(t[lo1], np.full(len(lo1), TH_LO), "v", color=C1, ms=6.0,
            zorder=5, mec="white", mew=0.6)
    ax.plot(t[lo2], np.full(len(lo2), TH_LO), "^", color=CO, ms=5.2,
            zorder=5, mec="white", mew=0.6)
    ax.plot(t[hi1], np.full(len(hi1), TH_HI), "v", color=C1, ms=6.0,
            zorder=5, mec="white", mew=0.6)
    ax.text(0.8, -1.15,
            f"low: slow {len(lo1)}, fast {len(lo2)} (exactly "
            r"$\Omega_2/\Omega_1=4$); "
            f"high: slow {len(hi1)}, fast {len(hi2)} --- "
            "their product is what occupation weighs",
            fontsize=FS_ANN, color=CK, ha="left", va="center")
    ax.set_xlabel("time")
    ax.set_ylabel("signal")
    ax.set_ylim(-1.4, 1.45)
    ax.set_xlim(0, T_END)
    ax.set_title("(b)  presentations $\\times$ success: "
                 "the two faces a noisy threshold multiplies")

    fs.save(fig, "mechanism")
    print(f"  wrote {fs.FIGDIR / 'mechanism.pdf'}")


if __name__ == "__main__":
    main()
