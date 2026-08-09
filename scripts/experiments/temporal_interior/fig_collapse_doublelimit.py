"""fig_collapse_doublelimit.py -- the universal collapse, and the double limit.

CLAIM (paper section "Off the threshold, exactly"): every oscillation of
V = mu x^2/2 + lambda x^4 at every detuning lies on ONE curve,
T*A*sqrt(2 lambda/m) = 4 kappa K(kappa), with the quartic clock at the
self-dual modulus kappa^2 = 1/2 where 4 kappa K = sqrt(pi) G*; and with a
sextic admixture the constant drifts a SECOND, independent way, so G* is
the double limit (mu -> 0, A -> 0) and neither limit alone suffices.

TWO PANELS, all exact quadrature:
  (a) the collapse curve over kappa^2: harmonic side, quartic point,
      double-well side -- one axis carrying every detuning.
  (b) the double-limit surface F(s, r) = T*A*sqrt(2 lambda/m) over the
      two drift directions s = mu/(2 lambda A^2) and r = nu A^2/lambda.
      G* lives at the corner (0,0) only; the two axes are the two exact
      drift laws, eq:collapse and eq:sexticdrift.

Everything drawn is computed here; the [verify] block asserts the corner,
both limits, and agreement with derive_period_decomposition.py's values.
"""
from __future__ import annotations

import numpy as np
from scipy.integrate import quad
from scipy.special import ellipk, gamma as Gamma
import _figstyle as fs          # sets backend + rcParams; import first
import matplotlib.pyplot as plt


FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = (
    fs.FS_TICK, fs.FS_LAB, fs.FS_TITLE, fs.FS_LEG, fs.FS_ANN)
C1, CO, CG_, C4 = fs.C1, fs.C2, fs.C3, fs.C4
CK, CG = fs.CK, fs.CG

GS = Gamma(0.25) / Gamma(0.75)
TARGET = np.sqrt(np.pi) * GS                    # 5.24411510858424


def collapse(k2):
    """4 kappa K(kappa); scipy's ellipk takes the PARAMETER m = kappa^2."""
    k2 = np.asarray(k2, float)
    return 4.0 * np.sqrt(k2) * ellipk(k2)


def F(s, r):
    """T*A*sqrt(2 lambda/m) for V = mu x^2/2 + lambda x^4 + nu x^6.

    s = mu/(2 lambda A^2), r = nu A^2/lambda; exact quadrature with the
    turning-point singularity at u = 1 declared to quad.
    """
    val, _ = quad(lambda u: 1.0 / np.sqrt(s * (1 - u * u)
                                          + (1 - u ** 4)
                                          + r * (1 - u ** 6)),
                  0.0, 1.0, points=[1.0], limit=400)
    return 4.0 * val


def panel_collapse(ax):
    k2 = np.linspace(1e-6, 0.985, 600)
    ax.plot(k2, collapse(k2), color=C1, lw=1.8, zorder=3)

    # regimes, shaded lightly and labelled where there is room
    ax.axvspan(0.0, 0.5, color=C1, alpha=0.06, zorder=0)
    ax.axvspan(0.5, 1.0, color=CO, alpha=0.06, zorder=0)
    ax.axvline(0.5, color=CG, lw=0.8, ls=":", zorder=1)

    ax.plot([0.5], [TARGET], "*", color=CO, ms=13, zorder=5,
            mec="white", mew=0.8)
    ax.annotate(r"$\kappa^2=\tfrac12$:  $4\kappa K=\sqrt{\pi}\,G^{*}$"
                "\n" r"the self-dual modulus, $K'=K$",
                xy=(0.5, TARGET), xytext=(0.10, 6.6),
                fontsize=FS_ANN, color=CO, ha="left", va="center",
                arrowprops=dict(arrowstyle="-", color=CG, lw=0.7))
    ax.text(0.03, 1.15, "harmonic side\n($\\mu>0$)", fontsize=FS_ANN,
            color=C1, ha="left", va="bottom")
    ax.text(0.97, 1.15, "double-well side\n($\\mu<0$, over the barrier)",
            fontsize=FS_ANN, color=CO, ha="right", va="bottom")

    ax.set_xlim(0, 1)
    ax.set_ylim(0, 9.0)
    ax.set_xticks([0, 0.25, 0.5, 0.75, 1.0])
    ax.set_xlabel(r"modulus $\kappa^{2} = 2\lambda A^{2}/(\mu+4\lambda A^{2})$")
    ax.set_ylabel(r"$T\,A\,\sqrt{2\lambda/m}$")
    ax.set_title("(a)  one curve carries every detuning:\n"
                 "the universal collapse $4\\kappa K(\\kappa)$")


def panel_doublelimit(ax):
    s = np.linspace(0.0, 2.0, 120)
    r = np.linspace(0.0, 1.0, 120)
    SS, RR = np.meshgrid(s, r)
    Z = np.vectorize(F)(SS, RR)

    lv = np.array([3.6, 4.0, 4.4, 4.8, 5.0, 5.15])
    cs = ax.contour(SS, RR, Z, levels=lv, colors=[CG], linewidths=0.8)
    ax.clabel(cs, fmt="%.2f", fontsize=6.5)

    BOX = dict(facecolor="white", edgecolor="none", pad=1.2)
    ax.plot([0], [0], "*", color=CO, ms=14, zorder=6,
            mec="white", mew=0.8, clip_on=False)
    ax.annotate(r"$G^{*}$ lives HERE only:"
                "\n" r"$F(0,0)=\sqrt{\pi}\,G^{*}$",
                xy=(0.02, 0.02), xytext=(1.05, 0.62),
                fontsize=FS_ANN, color=CO, ha="left", va="center",
                bbox=BOX, zorder=7,
                arrowprops=dict(arrowstyle="->", color=CO, lw=1.0,
                                shrinkB=4))

    # the two independent drift directions, along the axes
    ax.annotate("", xy=(0.85, 0.0), xytext=(0.06, 0.0),
                arrowprops=dict(arrowstyle="->", color=C1, lw=1.6))
    ax.text(0.62, 0.055, "detuning drift  (eq. collapse)", fontsize=FS_ANN,
            color=C1, ha="center", va="bottom", bbox=BOX, zorder=7)
    ax.annotate("", xy=(0.0, 0.62), xytext=(0.0, 0.05),
                arrowprops=dict(arrowstyle="->", color=C4, lw=1.6))
    ax.text(0.06, 0.86, "sextic drift\n(eq. sexticdrift)", fontsize=FS_ANN,
            color=C4, ha="left", va="center", bbox=BOX, zorder=7)

    ax.set_xlim(0, 2)
    ax.set_ylim(0, 1)
    ax.set_xlabel(r"detuning $s=\mu/(2\lambda A^{2})$")
    ax.set_ylabel(r"sextic admixture $r=\nu A^{2}/\lambda$")
    ax.set_title("(b)  the double limit: two independent drifts,\n"
                 "and the constant only at their corner")


def main():
    print("Collapse + double-limit figure")

    # -- verify the corner and both limits before drawing anything
    corner = F(0.0, 0.0)
    print(f"  [verify] F(0,0)            = {corner:.12f}")
    print(f"  [verify] sqrt(pi)*G*       = {TARGET:.12f}")
    assert abs(corner - TARGET) < 1e-9, "corner is not sqrt(pi) G*"

    sd = collapse(0.5)
    assert abs(sd - TARGET) < 1e-9, "self-dual point off"
    print(f"  [verify] 4 kappa K at 1/2  = {sd:.12f}   (same constant)")

    # harmonic limit: F(s,0) -> 2 pi/sqrt(s) from BELOW, at rate O(1/s)
    # (the quartic term only adds to the denominator).  Assert the sign,
    # the rate, and monotone improvement.
    devs = []
    for s in (50.0, 200.0):
        got, want = F(s, 0.0), 2 * np.pi / np.sqrt(s)
        dev = got / want - 1
        assert -2.0 / s < dev < 0, "harmonic limit failed at s=%g" % s
        devs.append(abs(dev))
    assert devs[1] < devs[0], "harmonic limit not improving with s"
    print("  [verify] harmonic limit F(s,0) -> 2 pi/sqrt(s) from below, "
          "O(1/s)   OK")

    # r-drift agrees with derive_period_decomposition.py
    for r_val, want in ((0.01, 5.21331264942), (0.10, 4.95893836664),
                        (1.00, 3.55629244139)):
        got = F(0.0, r_val)
        assert abs(got - want) < 1e-9, f"r-drift mismatch at r={r_val}"
    print("  [verify] r-drift matches derive_period_decomposition to 1e-9")

    fig, axes = plt.subplots(1, 2, figsize=(fs.TEXTWIDTH_IN, 2.95))
    panel_collapse(axes[0])
    panel_doublelimit(axes[1])
    fs.save(fig, "collapse")
    print(f"  wrote {fs.FIGDIR / 'collapse.pdf'}")


if __name__ == "__main__":
    main()
