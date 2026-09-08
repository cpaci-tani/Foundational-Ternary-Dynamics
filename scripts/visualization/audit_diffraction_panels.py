"""audit_diffraction_panels.py — audit and recreation of a six-panel
"Fraunhofer diffraction" figure (double slit, 4-pinhole cross, circular
aperture, 3 rings, many rings, square pinhole grid).

Finding the figures carry
-------------------------
The original figure mixed two different computations under one caption:

  * panels 3-5 (Airy, ring sums) are radial *far-field* formulas;
  * panels 1, 2, 6 are the *near field of point sources lying in the
    observation plane* -- the give-aways are bright spots at the source
    positions (impossible in a far-field pattern), hyperbolic fringes
    pinching between two foci, and a collimated cross of the array's own
    width.  None of that is Fraunhofer diffraction.

Two figure families are produced:

  fraunhofer_maps / fraunhofer_cuts
      what the six captions *describe*: the far-field intensity of each
      aperture, computed by a direct Fourier sum (or FFT of the aperture
      mask for the disk) and checked against the closed form.  Predicted
      zero loci / principal orders are overlaid, and every panel's residual
      against the exact answer is printed in the [verify] block.
  inplane_sources
      what panels 1, 2, 6 of the original *were*:
      |sum_j exp(i k r_j)/sqrt(r_j)|^2 for sources in the plotted plane.
      The two-source fringes are asserted to lie on the hyperbolae
      r1 - r2 = m*lambda.

Units: lambda = 1.  Far-field axes are direction cosines
u = sin(theta_x), v = sin(theta_y), so the patterns are exact (no paraxial
screen distance enters).  Nothing here touches the FTD lattice: this is
continuum Fourier optics, and it establishes only what the panels should
look like and what the originals computed.

Run:  python audit_diffraction_panels.py
"""
from __future__ import annotations

from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator, FixedFormatter, NullLocator
from scipy.optimize import brentq
from scipy.special import j0, j1, jn_zeros

OUT = Path(__file__).resolve().parent / "results" / "diffraction_audit"
OUT.mkdir(parents=True, exist_ok=True)

# ---------------------------------------------------------------------
# one typographic scheme
# ---------------------------------------------------------------------
FS_TICK, FS_LAB, FS_TITLE, FS_LEG, FS_ANN = 7.0, 8.0, 7.4, 6.8, 6.6
plt.rcParams.update({
    "font.family": "serif", "mathtext.fontset": "cm",
    "font.size": FS_TICK, "axes.labelsize": FS_LAB,
    "axes.titlesize": FS_TITLE, "legend.fontsize": FS_LEG,
    "xtick.labelsize": FS_TICK, "ytick.labelsize": FS_TICK,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.titlepad": 6.0, "axes.labelpad": 3.0,
    "figure.dpi": 160, "savefig.dpi": 300, "lines.linewidth": 1.3,
})
C1, C2, CK, CG = "#2a78d6", "#eb6834", "#2b2b2b", "#9a9a9a"
OVER = "#d8f3ff"          # overlay colour on the magma maps


def decade_ticks(axis, ticks, labels):
    axis.set_major_locator(FixedLocator(ticks))
    axis.set_major_formatter(FixedFormatter(labels))
    axis.set_minor_locator(NullLocator())


# =====================================================================
# far-field machinery
# =====================================================================
U = np.linspace(-1.0, 1.0, 401)          # direction cosines, du = 0.005
UU, VV = np.meshgrid(U, U)               # index [v, u]


def direct_sum(x, y, w, u=U, v=U):
    """A(u,v) = sum_j w_j exp(-2 pi i (u x_j + v y_j)), evaluated directly.
    No FFT and no closed form: this is the independent route."""
    Ex = np.exp(-2j * np.pi * np.outer(u, x))          # (Nu, M)
    Ey = np.exp(-2j * np.pi * np.outer(v, y))          # (Nv, M)
    return (Ey * w) @ Ex.T                             # (Nv, Nu)


def norm_I(A):
    I = np.abs(A) ** 2
    return I / I.max()


def local_minima(x, y, n, xmin=0.0):
    """First n interior local minima of y(x) with x > xmin, refined by a
    parabola through the three points around each."""
    out = []
    for i in range(1, len(y) - 1):
        if x[i] <= xmin:
            continue
        if y[i] < y[i - 1] and y[i] <= y[i + 1]:
            d = (y[i - 1] - 2 * y[i] + y[i + 1])
            off = 0.5 * (y[i - 1] - y[i + 1]) / d if d != 0 else 0.0
            out.append(x[i] + off * (x[1] - x[0]))
            if len(out) == n:
                break
    return np.array(out)


def local_maxima(x, y, n, xmin=0.0):
    return local_minima(x, -y, n, xmin)


# ---------- (a) double slit: width a, separation d, height h ----------
A_SL, D_SL, H_SL, DX = 2.0, 8.0, 0.8, 0.01


def slit_numeric():
    """Riemann sum of the Fourier integral over the two rectangles."""
    nx, ny = int(round(A_SL / DX)), int(round(H_SL / DX))
    xs1 = -A_SL / 2 + DX * (np.arange(nx) + 0.5)
    xs = np.concatenate([xs1 - D_SL / 2, xs1 + D_SL / 2])
    ys = -H_SL / 2 + DX * (np.arange(ny) + 0.5)
    Fx = np.exp(-2j * np.pi * np.outer(U, xs)).sum(1) * DX
    Fy = np.exp(-2j * np.pi * np.outer(U, ys)).sum(1) * DX
    return np.outer(Fy, Fx)


def slit_exact(u, v):
    return (A_SL * H_SL * np.sinc(A_SL * u) * np.sinc(H_SL * v)
            * 2.0 * np.cos(np.pi * D_SL * u))


# ---------- (b) four pinholes at (+-d,0),(0,+-d) ----------
D_PIN = 5.0


def pin_numeric():
    return direct_sum(np.array([D_PIN, -D_PIN, 0.0, 0.0]),
                      np.array([0.0, 0.0, D_PIN, -D_PIN]), np.ones(4))


def pin_exact(u, v):
    return 2 * np.cos(2 * np.pi * D_PIN * u) + 2 * np.cos(2 * np.pi * D_PIN * v)


# ---------- (c) circular aperture of radius a: FFT of an area mask ----------
A_DISK, N_FFT, DPIX, SS = 2.0, 4096, 0.05, 4


def disk_numeric():
    n_half = int(np.ceil(A_DISK / DPIX)) + 2
    idx = np.arange(-n_half, n_half + 1)
    sub = (np.arange(SS) + 0.5) / SS - 0.5
    xs = (idx[:, None] + sub[None, :]).ravel() * DPIX
    XX, YY = np.meshgrid(xs, xs)
    inside = (XX ** 2 + YY ** 2 <= A_DISK ** 2).astype(float)
    m = len(idx)
    small = inside.reshape(m, SS, m, SS).mean(axis=(1, 3))   # area fraction
    mask = np.zeros((N_FFT, N_FFT))
    c, k = N_FFT // 2, m // 2
    mask[c - k:c + k + 1, c - k:c + k + 1] = small
    F = np.fft.fftshift(np.fft.fft2(mask))
    f = np.fft.fftshift(np.fft.fftfreq(N_FFT, DPIX))
    sel = np.abs(f) <= 1.0 + 1e-9
    return F[np.ix_(sel, sel)], f[sel]


def airy_exact(rho):
    x = 2 * np.pi * A_DISK * rho
    with np.errstate(invalid="ignore", divide="ignore"):
        amp = np.where(x == 0, 1.0, 2 * j1(x) / x)
    return amp ** 2


# ---------- (d),(e) thin concentric rings ----------
RINGS3 = [2.0, 4.0, 6.0]
RINGS8 = [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]
M_RING = 1440


def rings_numeric(radii):
    al = 2 * np.pi * (np.arange(M_RING) + 0.5) / M_RING
    x = np.concatenate([a * np.cos(al) for a in radii])
    y = np.concatenate([a * np.sin(al) for a in radii])
    w = np.concatenate([np.full(M_RING, 2 * np.pi * a / M_RING) for a in radii])
    return direct_sum(x, y, w)


def rings_exact_amp(rho, radii):
    return sum(2 * np.pi * a * j0(2 * np.pi * a * rho) for a in radii)


def rings_zeros(radii, n):
    rho = np.linspace(1e-3, 1.0, 4000)
    f = rings_exact_amp(rho, radii)
    s = np.where(np.sign(f[:-1]) != np.sign(f[1:]))[0]
    z = [brentq(lambda r: rings_exact_amp(r, radii), rho[i], rho[i + 1])
         for i in s[:n]]
    return np.array(z)


# ---------- (f) N x N pinhole grid, pitch d ----------
N_G, D_G = 8, 3.0


def grid_numeric():
    c = D_G * (np.arange(N_G) - (N_G - 1) / 2)
    X, Y = np.meshgrid(c, c)
    return direct_sum(X.ravel(), Y.ravel(), np.ones(N_G * N_G))


def grating_I(u):
    """[sin(N pi d u)/sin(pi d u)]^2 with the removable singularity filled."""
    num, den = np.sin(N_G * np.pi * D_G * u), np.sin(np.pi * D_G * u)
    with np.errstate(invalid="ignore", divide="ignore"):
        r = np.where(np.abs(den) < 1e-9, N_G, num / den)
    return r ** 2


def grid_exact_I(u, v):
    return grating_I(u) * grating_I(v)


# =====================================================================
# in-plane sources (what the original panels 1, 2, 6 were)
# =====================================================================
XY = np.linspace(-50.0, 50.0, 601)
XX, YY = np.meshgrid(XY, XY)


def inplane(sources, lam):
    """|sum_j exp(i k r_j) / sqrt(r_j)|^2 -- 2D cylindrical waves from point
    sources in the plotted plane, r floored at lambda/4 at the sources."""
    psi = np.zeros_like(XX, dtype=complex)
    for sx, sy in sources:
        r = np.maximum(np.hypot(XX - sx, YY - sy), lam / 4)
        psi += np.exp(2j * np.pi * r / lam) / np.sqrt(r)
    return np.abs(psi) ** 2


# =====================================================================
# compute + verify
# =====================================================================
def compute():
    R = {}
    print("Fraunhofer audit -- lambda = 1, axes u = sin(theta_x), v = sin(theta_y)")
    print(f"  slit a={A_SL} d={D_SL} h={H_SL} | pinholes d={D_PIN} | disk a={A_DISK}"
          f" | rings {RINGS3} and {RINGS8} | grid {N_G}x{N_G} d={D_G}")

    # (a) double slit
    In, Ie = norm_I(slit_numeric()), norm_I(slit_exact(UU, VV))
    dev = np.abs(In - Ie).max()
    assert dev < 2e-3, f"double slit numeric vs closed form: {dev}"
    zn = local_minima(U, In[200], 6)
    zp = np.sort(np.concatenate([(np.arange(4) + 0.5) / D_SL, [1 / A_SL, 4.5 / D_SL]]))
    print(f"  [verify] (a) slit    max|I_num - I_exact| = {dev:.2e}  (Riemann, dx={DX})")
    print(f"  [verify] (a) zeros measured {np.array2string(zn, precision=4)}")
    print(f"  [verify] (a) zeros exact    {np.array2string(zp, precision=4)}"
          f"  max diff {np.abs(zn - zp).max():.1e}")
    R["slit"] = (In, Ie, zp)

    # (b) four pinholes
    In, Ie = norm_I(pin_numeric()), norm_I(pin_exact(UU, VV))
    dev = np.abs(In - Ie).max()
    assert dev < 1e-9, f"pinholes: {dev}"
    print(f"  [verify] (b) pinholes max|I_num - I_exact| = {dev:.2e}")
    R["pin"] = (In, Ie)

    # (c) disk via FFT
    F, f = disk_numeric()
    In = norm_I(F)
    i0 = int(np.argmin(np.abs(f)))
    assert f[i0] == 0.0
    FU, FV = np.meshgrid(f, f)
    Ie = airy_exact(np.hypot(FU, FV))
    dev = np.abs(In - Ie).max()
    lobe = Ie > 1e-2
    rel = (np.abs(In - Ie) / Ie)[lobe].max()
    assert dev < 2e-3, f"disk: {dev}"
    zn = local_minima(f, In[i0], 3)
    zp = jn_zeros(1, 3) / (2 * np.pi * A_DISK)
    print(f"  [verify] (c) disk     max|I_num - I_exact| = {dev:.2e} (peak-norm.), "
          f"max rel. in main lobe = {rel:.2e}  (FFT {N_FFT}^2, pitch {DPIX}, {SS}x supersample)")
    print(f"  [verify] (c) Airy zeros measured {np.array2string(zn, precision=4)}")
    print(f"  [verify] (c) Airy zeros exact    {np.array2string(zp, precision=4)}"
          f"  = j1,n/(2 pi a), max diff {np.abs(zn - zp).max():.1e}")
    R["disk"] = (In, Ie, f, zp)

    # (d),(e) rings
    for key, radii in (("rings3", RINGS3), ("rings8", RINGS8)):
        In = norm_I(rings_numeric(radii))
        amp = rings_exact_amp(np.hypot(UU, VV), radii)
        Ie = amp ** 2 / (amp ** 2).max()
        dev = np.abs(In - Ie).max()
        assert dev < 1e-8, f"{key}: {dev}"
        zp = rings_zeros(radii, 4)
        zn = local_minima(U, In[200], 4)
        print(f"  [verify] ({'d' if key == 'rings3' else 'e'}) {key:7s} "
              f"max|I_num - I_exact| = {dev:.2e}  (M={M_RING} pts/ring)")
        print(f"  [verify]     zeros measured {np.array2string(zn, precision=4)}"
              f"  exact {np.array2string(zp, precision=4)}"
              f"  max diff {np.abs(zn - zp).max():.1e}")
        R[key] = (In, Ie, zp)

    # (f) grid
    In, Ie = norm_I(grid_numeric()), grid_exact_I(UU, VV) / N_G ** 4
    dev = np.abs(In - Ie).max()
    assert dev < 1e-9, f"grid: {dev}"
    # principal orders only: an unthresholded maximum search returned the
    # first three SECONDARY maxima (0.060, 0.103, 0.146, heights ~0.05)
    # instead of the orders at m/d.  Height-gate at 0.5 (secondaries of an
    # N=8 grating peak at (N sin(3pi/2N))^-2 = 0.045).  u = 1 is the array
    # edge and is not an interior extremum, so only m = 1, 2 are measured.
    mn = local_maxima(U, np.where(In[200] > 0.5, In[200], 0.0), 2)
    mp = np.arange(1, 3) / D_G
    assert np.abs(mn - mp).max() < 1e-3, f"grid orders: {mn} vs {mp}"
    print(f"  [verify] (f) grid     max|I_num - I_exact| = {dev:.2e}")
    print(f"  [verify] (f) orders measured {np.array2string(mn, precision=4)}"
          f"  exact m/d {np.array2string(mp, precision=4)}"
          f"  max diff {np.abs(mn - mp).max():.1e}")
    R["grid"] = (In, Ie)

    # in-plane replicas
    S2 = [(-5.0, 0.0), (5.0, 0.0)]
    S4 = S2 + [(0.0, -5.0), (0.0, 5.0)]
    c = 2.0 * (np.arange(15) - 7)
    SG = [(x, y) for x in c for y in c]
    I2, I4 = inplane(S2, 1.0), inplane(S4, 1.0)
    IG = inplane(SG, 2.0)                       # d = lambda: end-fire rows
    # hyperbola check along the top edge y = +50
    x = XY
    row = I2[-1]
    pk = local_maxima(x, row, 40, xmin=-60.0)
    r1 = np.hypot(pk - S2[0][0], 50.0)
    r2 = np.hypot(pk - S2[1][0], 50.0)
    dd = r1 - r2
    hdev = np.abs(dd - np.round(dd)).max()
    assert hdev < 0.05, f"two-source maxima not on hyperbolae: {hdev}"
    print(f"  [verify] in-plane two sources: {len(pk)} maxima on the frame edge, "
          f"max |(r1 - r2)/lambda - m| = {hdev:.3f}  (grid dx = {x[1]-x[0]:.3f})")
    R["inplane"] = (I2, I4, IG, S2, S4)
    return R


# =====================================================================
# figures
# =====================================================================
def _map(ax, I, extent=(-1, 1, -1, 1), vmin=-4):
    with np.errstate(divide="ignore"):
        L = np.log10(np.maximum(I, 10.0 ** vmin))
    im = ax.imshow(L, extent=extent, origin="lower", cmap="magma",
                   vmin=vmin, vmax=0, interpolation="nearest")
    ax.set_xticks([-1, -0.5, 0, 0.5, 1])
    ax.set_yticks([-1, -0.5, 0, 0.5, 1])
    ax.set_xticklabels(["$-1$", "", "$0$", "", "$1$"])
    ax.set_yticklabels(["$-1$", "", "$0$", "", "$1$"])
    ax.set_xlim(-1, 1); ax.set_ylim(-1, 1)     # the FFT grid stops at 0.9985
    for s in ("top", "right"):
        ax.spines[s].set_visible(True)
    ax.tick_params(length=2.0)
    return im


def _circle(ax, r, **kw):
    t = np.linspace(0, 2 * np.pi, 361)
    ax.plot(r * np.cos(t), r * np.sin(t), **kw)


def fig_maps(R):
    fig, axes = plt.subplots(2, 3, figsize=(7.4, 5.25), constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.06, h_pad=0.08, hspace=0.05, wspace=0.05)
    ov = dict(color=OVER, lw=0.55, alpha=0.85)

    ax = axes[0, 0]
    In, _, zp = R["slit"]
    im = _map(ax, In)
    for z in zp[zp != 1 / A_SL]:
        ax.plot([z, z], [0.82, 0.95], **ov); ax.plot([-z, -z], [0.82, 0.95], **ov)
    for z in (1 / A_SL, -1 / A_SL):
        ax.plot([z, z], [-1, 1], ls="--", **ov)
    ax.set_title("(a)  double slit, $a=2\\lambda$, $d=8\\lambda$\n"
                 "zeros $(2m{+}1)/2d$ ticked, $\\pm1/a$ dashed")

    ax = axes[0, 1]
    _map(ax, R["pin"][0])
    m = np.arange(-5, 6) / D_PIN
    MX, MY = np.meshgrid(m, m)
    ax.plot(MX.ravel(), MY.ravel(), "+", color=OVER, ms=2.6, mew=0.5, alpha=0.85)
    ax.set_title("(b)  four pinholes at $\\pm d$, $d=5\\lambda$\n"
                 "maxima at $(m,n)/d$ crossed; no hot spots")

    ax = axes[0, 2]
    In, _, f, zp = R["disk"]
    _map(ax, In, extent=(f[0], f[-1], f[0], f[-1]))
    for z in zp:
        _circle(ax, z, ls=":", **ov)
    ax.set_title("(c)  circular aperture, $a=2\\lambda$\n"
                 "Airy zeros $2\\pi a\\rho=3.83,\\,7.02,\\,10.2$ dotted")

    for ax, key, lab, txt in ((axes[1, 0], "rings3", "d", "three rings, $a=2,4,6\\lambda$"),
                              (axes[1, 1], "rings8", "e", "eight rings, $a=1\\ldots8\\lambda$")):
        In, _, zp = R[key]
        _map(ax, In)
        for z in zp:
            _circle(ax, z, ls=":", **ov)
        line2 = ("$\\Sigma_i a_iJ_0(2\\pi a_i\\rho)$, four zeros dotted" if lab == "d"
                 else "same law; first four zeros dotted")
        ax.set_title(f"({lab})  {txt}\n{line2}")

    ax = axes[1, 2]
    _map(ax, R["grid"][0])
    m = np.arange(-3, 4) / D_G
    MX, MY = np.meshgrid(m, m)
    ax.plot(MX.ravel(), MY.ravel(), "+", color=OVER, ms=2.6, mew=0.5, alpha=0.85)
    ax.set_title("(f)  $8{\\times}8$ pinholes, $d=3\\lambda$\n"
                 "orders $(m,n)/d$ crossed; 6 side peaks")

    for ax in axes[1]:
        ax.set_xlabel("$u=\\sin\\theta_x$")
    for ax in axes[:, 0]:
        ax.set_ylabel("$v=\\sin\\theta_y$")
    cb = fig.colorbar(im, ax=axes.ravel().tolist(), shrink=0.55, pad=0.012, aspect=28)
    cb.set_label("$\\log_{10} I/I_{\\max}$", fontsize=FS_LAB)
    cb.ax.tick_params(labelsize=FS_TICK)
    fig.savefig(OUT / "fraunhofer_maps.pdf")
    fig.savefig(OUT / "fraunhofer_maps.png", dpi=220)
    plt.close(fig)


def fig_cuts(R):
    fig, axes = plt.subplots(2, 3, figsize=(7.4, 4.3), constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.06, h_pad=0.10, hspace=0.08, wspace=0.06)
    floor = 1e-6

    def cut(ax, x, yn, ye, title, xlab, every=6, xlim=(0, 1)):
        ax.semilogy(x, np.maximum(ye, floor), color=CK, ls=":", lw=1.1,
                    label="closed form")
        ax.semilogy(x[::every], np.maximum(yn[::every], floor), "o", color=C2,
                    ms=2.6, mew=0, label="numeric")
        ax.set_xlim(*xlim); ax.set_ylim(1e-5, 2.0)
        decade_ticks(ax.yaxis, [1e-4, 1e-2, 1], ["$10^{-4}$", "$10^{-2}$", "$1$"])
        ax.set_xlabel(xlab); ax.set_title(title)

    In, Ie, _ = R["slit"]
    cut(axes[0, 0], U, In[200], Ie[200],
        "(a)  double slit, $v=0$\nnumeric sum lands on $\\mathrm{sinc}^2(au)\\cos^2(\\pi du)$",
        "$u$")
    In, Ie = R["pin"]
    cut(axes[0, 1], U, In[200], Ie[200],
        "(b)  four pinholes, $v=0$\n$|1+\\cos2\\pi du|^2$ recovered to $4{\\times}10^{-15}$", "$u$")
    In, Ie, f, _ = R["disk"]
    i0 = int(np.argmin(np.abs(f)))
    cut(axes[0, 2], f, In[i0], Ie[i0],
        "(c)  circular aperture, radial\nFFT of the mask follows $[2J_1(x)/x]^2$",
        "$\\rho$", every=4)
    In, Ie, _ = R["rings3"]
    cut(axes[1, 0], U, In[200], Ie[200],
        "(d)  three rings, radial\nring-sample sum matches $[\\Sigma_i a_iJ_0]^2$", "$\\rho$")
    In, Ie, _ = R["rings8"]
    cut(axes[1, 1], U, In[200], Ie[200],
        "(e)  eight rings, radial\nsame, with the beat of adjacent radii", "$\\rho$")
    In, Ie = R["grid"]
    cut(axes[1, 2], U, In[200], Ie[200],
        "(f)  pinhole grid, $v=0$\norders at $m/3$, six side peaks between",
        "$u$", every=4)
    for ax in axes[:, 0]:
        ax.set_ylabel("$I/I_{\\max}$")
    axes[0, 0].legend(loc="upper right", frameon=True, framealpha=1.0,
                      edgecolor="none", facecolor="white", handlelength=1.6,
                      borderpad=0.3, labelspacing=0.25).set_zorder(9)
    fig.savefig(OUT / "fraunhofer_cuts.pdf")
    fig.savefig(OUT / "fraunhofer_cuts.png", dpi=220)
    plt.close(fig)


def fig_inplane(R):
    I2, I4, IG, S2, S4 = R["inplane"]
    fig, axes = plt.subplots(1, 3, figsize=(7.4, 2.7), constrained_layout=True)
    fig.get_layout_engine().set(w_pad=0.06, h_pad=0.08, wspace=0.05)
    ext = (-50, 50, -50, 50)
    for ax, I in zip(axes, (I2, I4, IG)):
        with np.errstate(divide="ignore"):
            L = np.log10(I / I.max())
        im = ax.imshow(np.maximum(L, -3), extent=ext, origin="lower", cmap="magma",
                       vmin=-3, vmax=0, interpolation="nearest")
        ax.set_xticks([-50, 0, 50]); ax.set_yticks([-50, 0, 50])
        for s in ("top", "right"):
            ax.spines[s].set_visible(True)
        ax.tick_params(length=2.0)
        ax.set_xlabel("$x/\\lambda$")
    axes[0].set_ylabel("$y/\\lambda$")
    # hyperbolae r1 - r2 = m lambda, m = +-1..+-4, drawn from the closed form
    # foci at (+-c, 0); branch m has semi-axis a = m*lambda/2.  Clip t so the
    # curves stop at the frame instead of autoscaling the axes.
    for m in range(1, 5):
        aa = m / 2.0; cc = 5.0; bb = np.sqrt(cc ** 2 - aa ** 2)
        tmax = np.arcsinh(50.0 / bb)
        t = np.linspace(-tmax, tmax, 400)
        xh, yh = aa * np.cosh(t), bb * np.sinh(t)
        for sgn in (1, -1):
            axes[0].plot(sgn * xh, yh, color=OVER, lw=0.5, alpha=0.8)
    for sx, sy in S2:
        axes[0].plot(sx, sy, "o", ms=2.5, mfc="none", mec=OVER, mew=0.6)
    for sx, sy in S4:
        axes[1].plot(sx, sy, "o", ms=2.5, mfc="none", mec=OVER, mew=0.6)
    for ax in axes:
        ax.set_xlim(-50, 50); ax.set_ylim(-50, 50)
    axes[0].set_title("(a)  two in-plane sources at $\\pm5\\lambda$\n"
                      "fringes lie on $r_1{-}r_2=m\\lambda$ (drawn)")
    axes[1].set_title("(b)  four in-plane sources\n"
                      "hot spots at the sources themselves")
    axes[2].set_title("(c)  $15{\\times}15$ in-plane sources, $d=\\lambda$\n"
                      "end-fire rows: cross of array width")
    cb = fig.colorbar(im, ax=axes.ravel().tolist(), shrink=0.8, pad=0.012, aspect=22)
    cb.set_label("$\\log_{10} I/I_{\\max}$", fontsize=FS_LAB)
    cb.ax.tick_params(labelsize=FS_TICK)
    fig.savefig(OUT / "inplane_sources.pdf")
    fig.savefig(OUT / "inplane_sources.png", dpi=220)
    plt.close(fig)


def main():
    R = compute()
    fig_maps(R); fig_cuts(R); fig_inplane(R)
    print(f"  wrote {OUT}/fraunhofer_maps.{{pdf,png}}, fraunhofer_cuts.*, inplane_sources.*")


if __name__ == "__main__":
    main()
