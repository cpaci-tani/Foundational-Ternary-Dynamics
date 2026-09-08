"""lattice_disk_test.py -- circular aperture on the engine's 3D lattice wave
law vs the Airy pattern [2 J1(x)/x]^2, x = 2 pi a sin(theta)/lambda.

Full 3D: the 18-point stencil (lap3d) and leapfrog from
lattice_aperture_test.py, c^2 = 1/3.  Plane wave from a soft source plane,
Dirichlet screen with a disk hole of radius a = lambda (area-weighted edge),
cubic sponge on all six faces.  Intensity is read on TWO meridian arcs of
radius R = 10 lambda (>= 2 D^2/lambda = 8 lambda): one in a lattice-axis
plane (x-y) and one in the diagonal plane (x, (y+z)/sqrt2).  Their
difference is the lattice-anisotropy measurement; their agreement with the
Airy form (x cos^2 theta for the Dirichlet screen, RS-I) is the optics test.

Run:  python lattice_disk_test.py        (~10 min, single core)
"""
from __future__ import annotations

import time
from pathlib import Path

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from scipy.ndimage import map_coordinates
from scipy.special import j1, jn_zeros

from lattice_aperture_test import (C1, C2, C3, CG, CK, C2SQ, OUT, decade_ticks,
                                   lap3d, omega_exact)

LAM, A_LAM, R_LAM = 6, 1.0, 10.0


def airy(s):
    x = 2 * np.pi * A_LAM * s
    with np.errstate(invalid="ignore", divide="ignore"):
        amp = np.where(x == 0, 1.0, 2 * j1(x) / x)
    return amp ** 2


def run(lam=LAM, n_per=20):
    f = OUT / f"lattice_disk_lam{lam}.npz"
    if f.exists():
        return dict(np.load(f))
    k = 2 * np.pi / lam
    w = float(omega_exact(k, 0.0, 0.0)); T = 2 * np.pi / w; cph = w / k
    sp, xs, xa = 6 * lam, 7 * lam, 10 * lam
    R = R_LAM * lam
    Nx = int(xa + (R_LAM + 4) * lam + sp)
    half = int((R_LAM + 4) * lam + sp); N = 2 * half; c0 = half
    u = np.zeros((Nx, N, N), np.float32); v = np.zeros_like(u)

    ix = np.arange(Nx, dtype=np.float32); iy = np.arange(N, dtype=np.float32)
    def ramp(i, n):
        d = np.minimum(i, n - 1 - i)
        return np.clip(1 - d / sp, 0, 1) ** 3
    sig = 0.1 * (ramp(ix, Nx)[:, None, None] + ramp(iy, N)[None, :, None]
                 + ramp(iy, N)[None, None, :])
    damp = (1.0 - np.minimum(sig, 0.2)).astype(np.float32)

    # disk hole, area-weighted: transmission t in [0,1] per cell (4x4 sub-samples)
    sub = (np.arange(4) + 0.5) / 4 - 0.5
    yy = (np.arange(N)[:, None] + sub[None, :]).ravel() - c0
    YY, ZZ = np.meshgrid(yy, yy, indexing="ij")
    inside = (YY ** 2 + ZZ ** 2 <= (A_LAM * lam) ** 2).astype(np.float32)
    tr = inside.reshape(N, 4, N, 4).mean(axis=(1, 3))          # (N, N)
    screen_keep = np.ones((Nx, N, N), np.float32)
    screen_keep[xa] = tr; screen_keep[xa + 1] = tr

    th = np.deg2rad(np.arange(-85.0, 85.01, 1.0))
    cx = xa + 1.0
    arc_axis = np.array([cx + R * np.cos(th), c0 + R * np.sin(th), np.full_like(th, c0)])
    arc_diag = np.array([cx + R * np.cos(th), c0 + R * np.sin(th) / np.sqrt(2),
                         c0 + R * np.sin(th) / np.sqrt(2)])

    t_arrive = (xa - xs + R + 2 * lam) / cph
    t0 = int(t_arrive + 8 * T); n_avg = int(round(n_per * T)); n_tot = t0 + n_avg
    win = np.hanning(n_avg + 2)[1:-1]
    # demodulate on the grid (see lattice_aperture_test.py: bilinear sampling
    # of the raw wave at 6 cells/lambda under-reads by up to 13 %)
    accg = np.zeros((Nx - xa, N, N), np.complex64); wsum = 0.0
    arc_axis[0] -= xa; arc_diag[0] -= xa
    ramp_n = int(4 * T)
    print(f"  disk lam={lam}: grid {Nx}x{N}x{N} ({Nx*N*N/1e6:.1f} M cells), R={R:.0f}, "
          f"{n_tot} ticks, averaging from t={t0}")
    tw = time.time()
    for t in range(n_tot):
        v += C2SQ * lap3d(u)
        v[xs] += np.float32(0.05 * min(1.0, t / ramp_n) * np.sin(w * t))
        u += v
        u *= damp; v *= damp
        u *= screen_keep; v *= screen_keep
        if t >= t0:
            j = t - t0
            accg += np.complex64(win[j] * np.exp(1j * w * t)) * u[xa:]; wsum += win[j]
        if t % 50 == 0:
            print(f"    t={t:4d}/{n_tot}  {time.time() - tw:6.1f}s  max|J|={np.abs(u).max():.3f}", flush=True)
    Ig = np.abs(2 * accg / wsum) ** 2
    Ia = map_coordinates(Ig, arc_axis, order=1, mode="nearest")
    Id = map_coordinates(Ig, arc_diag, order=1, mode="nearest")
    norm = max(Ia.max(), Id.max())
    snap = np.sqrt(Ig[:int((R_LAM + 4) * lam), :, c0])       # |A| in the x-y plane
    out = dict(theta=th, I_axis=Ia / norm, I_diag=Id / norm, snap=snap, lam=lam, R=R, T=T)
    np.savez_compressed(f, **out)
    return out


def figure(r):
    th, Ia, Id, lam, R = r["theta"], r["I_axis"], r["I_diag"], int(r["lam"]), float(r["R"])
    s = np.sin(th)
    Ib = airy(s); Irs = Ib * np.cos(th) ** 2; Irs /= Irs.max()
    m60 = np.abs(th) <= np.deg2rad(60)
    da, dd = np.abs(Ia - Irs)[m60].max(), np.abs(Id - Irs)[m60].max()
    aniso = np.abs(Ia - Id)[m60].max()
    z_pred = jn_zeros(1, 1)[0] / (2 * np.pi * A_LAM)
    def first_min(I):
        i = np.where((I[1:-1] < I[:-2]) & (I[1:-1] <= I[2:]))[0] + 1
        i = i[(s[i] > 0) & (I[i] < 0.02)]
        return s[i[0]] if len(i) else np.nan
    print(f"  [measure] disk lam={lam}: max|I_axis - Airy*cos^2| = {da:.3f}, "
          f"max|I_diag - Airy*cos^2| = {dd:.3f}, max|I_axis - I_diag| = {aniso:.3f} (|theta|<=60)")
    print(f"  [measure] first Airy zero sin(theta): axis {first_min(Ia):.4f}  diag {first_min(Id):.4f}"
          f"  exact j_1,1/(2 pi a) = {z_pred:.4f}")

    fig, axes = plt.subplots(1, 3, figsize=(7.4, 2.75), constrained_layout=True,
                             gridspec_kw=dict(width_ratios=[1.0, 1.15, 1.0]))
    fig.get_layout_engine().set(w_pad=0.06, h_pad=0.08, wspace=0.05)
    ax = axes[0]
    snap = r["snap"]                                   # |A| from the screen onward
    ext = (0.0, snap.shape[0] / lam, -snap.shape[1] / (2 * lam), snap.shape[1] / (2 * lam))
    xg = (np.arange(snap.shape[0]) + 0.5)[:, None]
    yg = (np.arange(snap.shape[1]) - snap.shape[1] / 2)[None, :]
    rr = np.hypot(xg, yg) / lam
    S = snap * np.maximum(rr, 1.0)                     # 3D far field falls as 1/r
    with np.errstate(divide="ignore"):
        L = np.log10(np.maximum(S.T / S[xg[:, 0] > 3 * lam].max(), 1e-3))
    ax.imshow(L, extent=ext, origin="lower", cmap="magma", vmin=-3, vmax=0, aspect="equal",
              interpolation="nearest")
    tt = np.linspace(-np.pi / 2, np.pi / 2, 200)
    ax.plot(R / lam * np.cos(tt), R / lam * np.sin(tt), color="#d8f3ff", lw=0.5, ls="--")
    ax.set_xlabel("$x/\\lambda$ (screen at 0)"); ax.set_ylabel("$y/\\lambda$")
    ax.set_title(f"(a)  $r\\,|A|$ in the $x$-$y$ plane behind a\ndisk hole $a=\\lambda$; arc $R=10\\lambda$ dashed")
    for sd in ("top", "right"):
        ax.spines[sd].set_visible(True)

    ax = axes[1]
    sf = np.linspace(-1, 1, 1001); floor = 1e-4
    Ibf = airy(sf); Irsf = Ibf * (1 - sf ** 2); Irsf /= Irsf.max()
    ax.semilogy(sf, np.maximum(Ibf, floor), color=CG, ls=":", lw=1.0, label="Airy $[2J_1(x)/x]^2$")
    ax.semilogy(sf, np.maximum(Irsf, floor), color=CK, lw=1.0, label="Airy $\\times\\cos^2\\theta$ (RS-I)")
    ax.semilogy(s, np.maximum(Ia, floor), "o", color=C2, ms=2.4, mew=0, label="lattice, axis plane")
    ax.semilogy(s, np.maximum(Id, floor), "s", color=C1, ms=2.2, mew=0, label="lattice, diagonal plane")
    ax.set_xlim(-1, 1); ax.set_ylim(1e-3, 2)
    decade_ticks(ax.yaxis, [1e-3, 1e-2, 1e-1, 1], ["$10^{-3}$", "$10^{-2}$", "$10^{-1}$", "$1$"])
    ax.set_xlabel("$\\sin\\theta$"); ax.set_ylabel("$I/I_{\\max}$")
    ax.set_title(f"(b)  Airy profile on the lattice, $\\lambda={lam}$ cells\n"
                 f"max$|\\Delta I|$ to RS-I {max(da, dd):.3f} ($|\\theta|\\leq60^\\circ$)")
    ax.legend(loc="lower center", frameon=True, framealpha=1.0, edgecolor="none",
              facecolor="white", handlelength=1.5, borderpad=0.3, labelspacing=0.25).set_zorder(9)

    ax = axes[2]
    ax.plot(s, Ia - Irs, color=C2, lw=0.9, label="axis plane $-$ RS-I")
    ax.plot(s, Id - Irs, color=C1, lw=0.9, label="diagonal plane $-$ RS-I")
    ax.plot(s, Ia - Id, color=C3, lw=0.9, ls="--", label="axis $-$ diagonal")
    ax.axhline(0, color=CG, lw=0.6, zorder=0)
    ax.set_xlim(-1, 1); ax.set_ylim(-0.1, 0.1); ax.set_yticks([-0.1, 0, 0.1])
    ax.set_xlabel("$\\sin\\theta$"); ax.set_ylabel("residual")
    ax.set_title(f"(c)  residuals; axis$-$diagonal is the\nlattice anisotropy: max {aniso:.3f}")
    ax.legend(loc="upper right", frameon=True, framealpha=1.0, edgecolor="none",
              facecolor="white", handlelength=1.5, borderpad=0.3, labelspacing=0.25).set_zorder(9)
    fig.savefig(OUT / "lattice_disk_airy.pdf")
    fig.savefig(OUT / "lattice_disk_airy.png", dpi=220)
    plt.close(fig)
    print(f"  wrote {OUT / 'lattice_disk_airy.png'}")


if __name__ == "__main__":
    figure(run())
