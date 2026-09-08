"""lattice_aperture_test.py -- does the engine's lattice wave law reproduce
Fraunhofer diffraction?  The figure that can fail.

The wave law is the engine's, re-implemented in numpy so a run takes
seconds: leapfrog  v += c^2 L J ; J += v  with c^2 = 1/3 and the 18-point
Patra-Karttunen Laplacian (face 1/3, edge 1/6, centre -4;
engine/include/ftd/constants.h).  A slit aperture is z-invariant, so the 3D
stencil applied to a z-invariant field reduces EXACTLY to the 2D stencil
    L2 u = (2/3) sum_axial + (1/6) sum_diagonal - (10/3) u,
which is asserted against the 3D stencil below.

Instrument
----------
  * plane wave from a soft line source, normal incidence on a Dirichlet
    screen (J = 0) with slits cut in it; quadratic sponge on all four sides
  * intensity read on an arc of radius R = 100 lambda behind the screen
    (R >= 2 D^2/lambda for the widest aperture used), by Hann-windowed
    quadrature demodulation at the drive frequency over 24 periods
  * closed forms: the bare Fourier pattern the six-panel captions describe,
    and Rayleigh-Sommerfeld I = bare x cos^2(theta), the correct far field
    for a Dirichlet screen under the Kirchhoff aperture assumption
  * control: rerun with lambda doubled at fixed aperture-in-wavelengths.
    A residual that does not move is screen-edge physics; one that shrinks
    is lattice discretisation.

Exact pieces (asserted): 2D/3D stencil reduction; leapfrog dispersion
cos(omega) = 1 + c^2 L(k)/2 recovered from the running solver.

Run:  python lattice_aperture_test.py            (lambda = 6 cells, both slits)
      python lattice_aperture_test.py --lam 12   (control, ~5 min, then replots)
"""
from __future__ import annotations

import argparse
import time
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.ticker import FixedLocator, FixedFormatter, NullLocator
from scipy.ndimage import map_coordinates

OUT = Path(__file__).resolve().parent / "results" / "diffraction_audit"
OUT.mkdir(parents=True, exist_ok=True)

FS_TICK, FS_LAB, FS_TITLE, FS_LEG = 7.0, 8.0, 7.4, 6.8
plt.rcParams.update({
    "font.family": "serif", "mathtext.fontset": "cm",
    "font.size": FS_TICK, "axes.labelsize": FS_LAB,
    "axes.titlesize": FS_TITLE, "legend.fontsize": FS_LEG,
    "xtick.labelsize": FS_TICK, "ytick.labelsize": FS_TICK,
    "axes.spines.top": False, "axes.spines.right": False,
    "axes.titlepad": 6.0, "axes.labelpad": 3.0,
    "figure.dpi": 160, "savefig.dpi": 300, "lines.linewidth": 1.3,
})
C1, C2, C3, CK, CG = "#2a78d6", "#eb6834", "#1baf7a", "#2b2b2b", "#9a9a9a"

C2SQ = 1.0 / 3.0                     # engine c^2 (ontic C_WAVE^2)
W_FACE, W_EDGE = 1.0 / 3.0, 1.0 / 6.0


def decade_ticks(axis, ticks, labels):
    axis.set_major_locator(FixedLocator(ticks))
    axis.set_major_formatter(FixedFormatter(labels))
    axis.set_minor_locator(NullLocator())


# =====================================================================
# stencils
# =====================================================================
def lap3d(u):
    r = np.roll
    faces = (r(u, 1, 0) + r(u, -1, 0) + r(u, 1, 1) + r(u, -1, 1)
             + r(u, 1, 2) + r(u, -1, 2))
    edges = 0.0
    for a, b in ((0, 1), (0, 2), (1, 2)):
        for sa in (1, -1):
            for sb in (1, -1):
                edges = edges + r(r(u, sa, a), sb, b)
    return W_FACE * faces + W_EDGE * edges - 4.0 * u


def lap2d(u):
    """z-invariant reduction of lap3d: axial 2/3, diagonal 1/6, centre -10/3."""
    r = np.roll
    ax = r(u, 1, 0) + r(u, -1, 0) + r(u, 1, 1) + r(u, -1, 1)
    dg = (r(r(u, 1, 0), 1, 1) + r(r(u, 1, 0), -1, 1)
          + r(r(u, -1, 0), 1, 1) + r(r(u, -1, 0), -1, 1))
    return (2.0 / 3.0) * ax + (1.0 / 6.0) * dg - (10.0 / 3.0) * u


def omega_exact(kx, ky, kz=0.0):
    """leapfrog dispersion of the 18-point stencil: cos w = 1 + c^2 L(k)/2."""
    cx, cy, cz = np.cos(kx), np.cos(ky), np.cos(kz)
    L = (2.0 / 3.0) * (cx + cy + cz) + (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz) - 4.0
    return np.arccos(1.0 + C2SQ * L / 2.0)


def verify_stencils():
    rng = np.random.default_rng(3)
    u2 = rng.standard_normal((12, 10))
    u3 = np.repeat(u2[:, :, None], 5, axis=2)          # z-invariant field
    red = lap3d(u3)[:, :, 2]
    dev = np.abs(red - lap2d(u2)).max()
    assert dev < 1e-12, f"2D reduction of the 18-pt stencil is wrong: {dev}"
    assert abs(6 * W_FACE + 12 * W_EDGE - 4.0) < 1e-15, "3D sum rule"
    assert abs(4 * (2 / 3) + 4 * (1 / 6) - 10 / 3) < 1e-15, "2D sum rule"
    # running-solver dispersion: single Fourier mode obeys u+ + u- = 2 cos(w) u
    N, lam = 48, 8
    x = np.arange(N)
    for name, (mx, my) in (("[10]", (1, 0)), ("[11]", (1, 1))):
        kx, ky = 2 * np.pi * mx * (N // lam) / N, 2 * np.pi * my * (N // lam) / N
        u = np.cos(kx * x[:, None] + ky * x[None, :])
        v = np.zeros_like(u)
        v += C2SQ * lap2d(u); um, u = u, u + v          # one step
        v += C2SQ * lap2d(u); up = u + v
        cosw = ((up + um) / (2 * u))[3, 5]
        w_meas, w_ex = np.arccos(cosw), omega_exact(kx, ky)
        print(f"  [verify] dispersion {name}: omega solver {w_meas:.12f}  "
              f"exact {w_ex:.12f}  diff {abs(w_meas - w_ex):.1e}")
        assert abs(w_meas - w_ex) < 1e-10
    print(f"  [verify] 3D->2D stencil reduction max dev {dev:.1e}; sum rules hold")
    # read-out self-test: on-grid demodulation of a travelling plane wave must
    # return a spatially flat |A| (the arc sampler then interpolates a
    # smooth field, not an oscillating one)
    kx, w = 2 * np.pi / 6, float(omega_exact(2 * np.pi / 6, 0.0))
    n = int(round(24 * 2 * np.pi / w)); win = np.hanning(n + 2)[1:-1]
    xg = np.arange(48)[:, None] + 0 * np.arange(48)[None, :]
    acc = sum(win[t] * np.exp(1j * w * t) * np.cos(kx * xg - w * t) for t in range(n))
    amp = np.abs(2 * acc / win.sum())
    flat = amp.max() / amp.min() - 1
    # the only x-dependence is the image term (1/2) e^{ikx} sum_t w e^{2iwt},
    # i.e. the Hann window's leakage at 2*omega: flat <= 2*leak exactly
    leak = abs(sum(win[t] * np.exp(2j * w * t) for t in range(n))) / win.sum()
    assert flat <= 2 * leak * (1 + 1e-6) + 1e-12, f"|A| ripple {flat} exceeds image leakage {2*leak}"
    print(f"  [verify] on-grid demodulation of a plane wave: |A| ripple {flat:.2e} "
          f"<= 2 x Hann leakage at 2w = {2*leak:.2e}; mean {amp.mean():.6f} (exact 1)")


# =====================================================================
# apertures (in wavelengths) and their closed forms
# =====================================================================
APERTURES = {
    "double": dict(a=2.0, centres=(-2.5, 2.5), label="double slit $a=2\\lambda$, $d=5\\lambda$"),
    "grating": dict(a=1.0, centres=(-2.5, 0.0, 2.5), label="3 slits $a=\\lambda$, $d=2.5\\lambda$"),
}


def bare_pattern(s, ap):
    """|FT of the aperture|^2 in direction cosine s = sin(theta), lambda = 1."""
    a, cs = ap["a"], np.asarray(ap["centres"])
    amp = np.sinc(a * s)[..., None] * np.exp(-2j * np.pi * np.outer(s, cs))
    I = np.abs(amp.sum(-1)) ** 2
    return I / I.max()


def predicted_zeros(ap, smax=0.95):
    s = np.linspace(0, smax, 20001)
    I = bare_pattern(s, ap)
    i = np.where((I[1:-1] < I[:-2]) & (I[1:-1] <= I[2:]))[0] + 1
    return s[i][I[i] < 1e-3]


# =====================================================================
# lattice run
# =====================================================================
def run_case(lam, key, R_lam=100.0, n_per=24, verbose=True):
    ap = APERTURES[key]
    f = OUT / f"lattice_{key}_lam{lam}.npz"
    if f.exists():
        return dict(np.load(f))
    k = 2 * np.pi / lam
    w = float(omega_exact(k, 0.0)); T = 2 * np.pi / w
    cph = w / k
    # A 5-lambda quadratic sponge (sigma_max 0.2) reflected ~5 % in amplitude
    # and put a standing-wave ripple of +-0.1 on the on-axis intensity; the
    # arc runs tangent to x-normal wavefronts there, so the ripple period in
    # sin(theta) was ~sqrt(lambda/R).  Cured by a 10-lambda cubic ramp with
    # the boundary 10 lambda beyond the read-out arc.
    sp, xs, xa = 10 * lam, 11 * lam, 15 * lam
    R = R_lam * lam
    Nx = int(xa + (R_lam + 10) * lam + sp)
    half = int((R_lam + 10) * lam + sp)
    Ny = 2 * half; y0 = half
    u = np.zeros((Nx, Ny)); v = np.zeros_like(u)

    ix = np.arange(Nx)[:, None].astype(float); iy = np.arange(Ny)[None, :].astype(float)
    dx = np.minimum(ix, Nx - 1 - ix); dy = np.minimum(iy, Ny - 1 - iy)
    sig = 0.1 * (np.clip(1 - dx / sp, 0, 1) ** 3 + np.clip(1 - dy / sp, 0, 1) ** 3)
    damp = 1.0 - np.minimum(sig, 0.2)

    # Dirichlet screen with slits, two cells thick
    yy = (np.arange(Ny) - y0) / lam
    open_ = np.zeros(Ny, bool)
    for c in ap["centres"]:
        open_ |= np.abs(yy - c) < ap["a"] / 2
    screen = np.zeros((Nx, Ny), bool); screen[xa:xa + 2, :] = ~open_

    # arc behind the screen
    th = np.deg2rad(np.arange(-85.0, 85.01, 0.5))
    cx, cy = xa + 1.0, float(y0)
    coords = np.array([cx + R * np.cos(th), cy + R * np.sin(th)])

    t_arrive = (xa - xs + R + 2 * lam) / cph
    t0 = int(t_arrive + 8 * T)
    n_avg = int(round(n_per * T))
    n_tot = t0 + n_avg
    win = np.hanning(n_avg + 2)[1:-1]
    # Demodulate ON THE GRID, then sample the smooth envelope.  Sampling the
    # raw oscillating field by bilinear interpolation at 6 cells/lambda
    # under-reads the amplitude by up to 13 % (|1/2 + e^{ik}/2| = 0.866 at a
    # half-cell offset); along the arc, tangent to the axis, the sub-cell
    # offset cycles slowly and produced -12 % dips at sin(theta) ~ sqrt(n/300)
    # that looked exactly like a boundary reflection.
    x_keep = xa + int((R_lam + 10) * lam)
    accg = np.zeros((x_keep - xa, Ny), complex); wsum = 0.0
    coords = coords - np.array([[xa], [0.0]])
    ramp_n = int(4 * T)
    if verbose:
        print(f"  lattice {key:7s} lam={lam:2d}: grid {Nx}x{Ny}, R={R:.0f}, T={T:.3f} ticks, "
              f"{n_tot} ticks, averaging {n_avg} ticks from t={t0}")
    t_wall = time.time()
    for t in range(n_tot):
        v += C2SQ * lap2d(u)
        ramp = min(1.0, t / ramp_n)
        v[xs, :] += 0.05 * ramp * np.sin(w * t)
        u += v
        u *= damp; v *= damp
        u[screen] = 0.0; v[screen] = 0.0
        if t >= t0:
            j = t - t0
            accg += (win[j] * np.exp(1j * w * t)) * u[xa:x_keep]; wsum += win[j]
        if verbose and t % 400 == 0 and t:
            print(f"    t={t:5d}/{n_tot}  {time.time() - t_wall:5.1f}s  max|J|={np.abs(u).max():.3f}")
    Ig = np.abs(2 * accg / wsum) ** 2                       # smooth envelope
    I = map_coordinates(Ig, coords, order=1, mode="nearest")
    snap = np.sqrt(Ig)                                       # |A| behind the screen
    out = dict(theta=th, I=I / I.max(), snap=snap[::max(1, lam // 3), ::max(1, lam // 3)],
               lam=lam, R=R, xa=xa, y0=y0, Nx=Nx, Ny=Ny, T=T)
    np.savez_compressed(f, **out)
    return out


# =====================================================================
# analysis + figure
# =====================================================================
def measured_zeros(s, I, n=6, depth=0.02):
    """Interior minima with I < depth.  Without the gate a ripple dip at
    sin(theta)=0.62 was reported as the double-slit zero at 0.70."""
    i = np.where((I[1:-1] < I[:-2]) & (I[1:-1] <= I[2:]))[0] + 1
    i = i[(s[i] > 0) & (I[i] < depth)]
    out = []
    for j in i[:n]:
        d = I[j - 1] - 2 * I[j] + I[j + 1]
        off = 0.5 * (I[j - 1] - I[j + 1]) / d if d != 0 else 0.0
        out.append(s[j] + off * (s[j] - s[j - 1]))
    return np.array(out)


def analyse(res):
    rows = {}
    for key, ap in APERTURES.items():
        for lam, r in sorted(res[key].items()):
            th, I = r["theta"], r["I"]
            s = np.sin(th)
            Ib = bare_pattern(s, ap)
            Irs = Ib * np.cos(th) ** 2; Irs /= Irs.max()
            m60 = np.abs(th) <= np.deg2rad(60)
            db, drs = np.abs(I - Ib)[m60].max(), np.abs(I - Irs)[m60].max()
            zm = measured_zeros(s[m60], I[m60], 4)
            zp = predicted_zeros(ap, np.sin(np.deg2rad(60)))[:len(zm)]
            print(f"  [measure] {key:7s} lam={lam:2d}: max|I_lat - I_bare| = {db:.3f}, "
                  f"max|I_lat - I_RS1| = {drs:.3f}  (|theta|<=60 deg)")
            print(f"  [measure]     zeros (sin theta) lattice {np.array2string(zm, precision=4)}"
                  f"  closed form {np.array2string(zp, precision=4)}"
                  f"  max diff {np.abs(zm - zp).max() if len(zp) else float('nan'):.4f}")
            rows[(key, lam)] = (db, drs, zm, zp)
    return rows


def figure(res, rows):
    keys = list(APERTURES)
    fig, axes = plt.subplots(3, 2, figsize=(7.4, 7.0), constrained_layout=True,
                             gridspec_kw=dict(height_ratios=[1.25, 1.0, 0.55]))
    fig.get_layout_engine().set(w_pad=0.06, h_pad=0.08, hspace=0.05, wspace=0.05)
    for col, key in enumerate(keys):
        ap = APERTURES[key]
        lams = sorted(res[key])
        r0 = res[key][lams[0]]
        # --- field snapshot (the object)
        ax = axes[0, col]
        snap, lam, R = r0["snap"], int(r0["lam"]), float(r0["R"])
        st = max(1, lam // 3)
        ext = (-2.0, (snap.shape[0] * st) / lam - 2.0,
               -(snap.shape[1] * st) / (2 * lam), (snap.shape[1] * st) / (2 * lam))
        # snapshot is the demodulated |A| from the screen onward; the far-field
        # falls as 1/sqrt(r) in 2D, so scale by sqrt(r) to show the lobes
        ext = (0.0, (snap.shape[0] * st) / lam,
               -(snap.shape[1] * st) / (2 * lam), (snap.shape[1] * st) / (2 * lam))
        xg = (np.arange(snap.shape[0]) * st + 0.5)[:, None]
        yg = (np.arange(snap.shape[1]) * st - snap.shape[1] * st / 2)[None, :]
        rr = np.hypot(xg, yg) / lam
        S = snap * np.sqrt(np.maximum(rr, 1.0))
        with np.errstate(divide="ignore"):
            L = np.log10(np.maximum(S.T / S[xg[:, 0] > 5 * lam].max(), 1e-3))
        ax.imshow(L, extent=ext, origin="lower", cmap="magma", vmin=-3, vmax=0,
                  aspect="equal", interpolation="nearest")
        tt = np.linspace(-np.pi / 2, np.pi / 2, 200)
        ax.plot(R / lam * np.cos(tt), R / lam * np.sin(tt), color="#d8f3ff", lw=0.5, ls="--")
        ax.set_xlabel("$x/\\lambda$ (screen at 0)"); ax.set_ylabel("$y/\\lambda$" if col == 0 else "")
        ax.set_title(f"({'ab'[col]})  lattice $\\sqrt{{r}}\\,|A|$ behind the {ap['label']}\n"
                     f"$\\lambda={lam}$ cells; read-out arc $R=100\\lambda$ dashed")
        ax.set_xlim(0, ext[1]); ax.set_ylim(ext[2], ext[3])
        for sd in ("top", "right"):
            ax.spines[sd].set_visible(True)
        # --- pattern
        ax = axes[1, col]
        th = r0["theta"]; s = np.sin(th)
        sf = np.linspace(-1, 1, 2001)
        Ib = bare_pattern(sf, ap)
        Irs = Ib * (1 - sf ** 2); Irs /= Irs.max()
        floor = 1e-5
        ax.semilogy(sf, np.maximum(Ib, floor), color=CG, ls=":", lw=1.0,
                    label="bare Fourier pattern (the captions)")
        ax.semilogy(sf, np.maximum(Irs, floor), color=CK, lw=1.0,
                    label="Rayleigh-Sommerfeld I: $\\times\\cos^2\\theta$")
        mk = [("o", C2), ("s", C1), ("^", C3)]
        for (m, c), lam_ in zip(mk, lams):
            r = res[key][lam_]
            ax.semilogy(np.sin(r["theta"])[::3], np.maximum(r["I"], floor)[::3], m, color=c,
                        ms=2.4, mew=0, label=f"lattice, $\\lambda={int(lam_)}$ cells")
        ax.set_xlim(-1, 1); ax.set_ylim(1e-4, 2)
        decade_ticks(ax.yaxis, [1e-4, 1e-2, 1], ["$10^{-4}$", "$10^{-2}$", "$1$"])
        ax.set_ylabel("$I/I_{\\max}$" if col == 0 else "")
        ax.set_xticklabels([])
        db, drs = rows[(key, lams[0])][:2]
        ax.set_title(f"({'cd'[col]})  arc intensity: lattice vs closed forms\n"
                     f"max$|\\Delta I|$ to RS-I {drs:.3f}, to bare {db:.3f} ($|\\theta|\\leq60^\\circ$)")
        if col == 0:
            ax.legend(loc="lower center", frameon=True, framealpha=1.0, edgecolor="none",
                      facecolor="white", handlelength=1.5, borderpad=0.3,
                      labelspacing=0.25, ncol=1).set_zorder(9)
        # --- residual to RS-I
        ax = axes[2, col]
        for (m, c), lam_ in zip(mk, lams):
            r = res[key][lam_]
            th_ = r["theta"]; s_ = np.sin(th_)
            Ib_ = bare_pattern(s_, ap)
            Irs_ = Ib_ * np.cos(th_) ** 2; Irs_ /= Irs_.max()
            ax.plot(s_, r["I"] - Ib_, color=c, lw=0.9, label=f"to bare, $\\lambda={int(lam_)}$")
            ax.plot(s_, r["I"] - Irs_, color=c, lw=0.8, ls="--", label=f"to RS-I, $\\lambda={int(lam_)}$")
        ax.axhline(0, color=CG, lw=0.6, zorder=0)
        ax.set_xlim(-1, 1); ax.set_ylim(-0.2, 0.2)
        ax.set_yticks([-0.2, -0.1, 0, 0.1, 0.2])
        ax.set_xlabel("$\\sin\\theta$"); ax.set_ylabel("$I_{\\rm lat}-I_{\\rm pred}$" if col == 0 else "")
        if len(lams) > 1:
            d6, d12 = rows[(key, lams[0])][0], rows[(key, lams[1])][0]
            ax.set_title(f"({'ef'[col]})  residual {d6:.3f}$\\to${d12:.3f} under "
                         f"$\\lambda\\to2\\lambda$\n($O(h^2)$ would give $4\\times$): "
                         f"mostly screen-edge physics")
        else:
            ax.set_title(f"({'ef'[col]})  residuals")
        if col == 1:
            ax.legend(loc="lower center", ncol=2, frameon=True, framealpha=1.0, edgecolor="none",
                      facecolor="white", handlelength=1.6, borderpad=0.25,
                      labelspacing=0.2, columnspacing=0.8).set_zorder(9)
    fig.savefig(OUT / "lattice_vs_fraunhofer.pdf")
    fig.savefig(OUT / "lattice_vs_fraunhofer.png", dpi=220)
    plt.close(fig)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--lam", type=int, nargs="*", default=[6])
    args = p.parse_args()
    print("Lattice aperture test -- engine 18-pt stencil, c^2 = 1/3, leapfrog")
    verify_stencils()
    res = {k: {} for k in APERTURES}
    for lam in args.lam:
        for key in APERTURES:
            res[key][lam] = run_case(lam, key)
    # pull in any other cached runs so the control overlays when present
    for f in OUT.glob("lattice_*_lam*.npz"):
        key, lam = f.stem.split("_")[1], int(f.stem.split("lam")[1])
        if key in res and lam not in res[key]:
            res[key][lam] = dict(np.load(f))
    rows = analyse(res)
    figure(res, rows)
    print(f"  wrote {OUT / 'lattice_vs_fraunhofer.png'}")


if __name__ == "__main__":
    main()
