"""probe_causal_isotropy.py — how isotropic is the causal structure, as a
function of the amplitude you can detect?

THE TENSION.  A discrete substrate has two causal boundaries and they have
different symmetry:

  STRICT SUPPORT   set by the stencil.  For M18 it is the cuboctahedron,
                   whose radius runs from t (face) to sqrt(2) t (edge) --
                   manifestly ANISOTROPIC, by 41% between axes.
  EFFECTIVE CONE   characterized by the dispersion.  Omega = C|k| + O(k^3)
                   has an isotropic leading term, so long-wave packet peaks
                   concentrate near a SPHERE of radius C t.

Both are real.  So "is the causal structure isotropic?" has no unqualified
answer -- it depends on the amplitude at which you can still see.  This
script measures the surface

    R_u(eps) = max{ r : |phi| >= eps along direction u }

over a Fibonacci sampling of the sphere, and reports the anisotropy

    A(eps) = (R_max - R_min) / R_mean

as a function of eps.  A(eps) -> 0 is the isotropic (relativistic) regime;
A(eps) -> 0.34 is the bare cuboctahedron.  The crossover is the sensitivity
at which the lattice's preferred directions become visible at all.

This is the quantity that a preferred-frame experiment actually bounds, so
it is the honest form of the free-sector Lorentz statement.
"""
from __future__ import annotations

import numpy as np

C = 1.0 / np.sqrt(3.0)


def greens(T, R):
    g = np.arange(-R, R + 1)
    N = len(g)
    phi = np.zeros((N, N, N))
    phi[R, R, R] = 1.0
    prev = phi.copy()

    def lap(f):
        out = -4.0 * f
        for ax in (0, 1, 2):
            out += (np.roll(f, 1, ax) + np.roll(f, -1, ax)) / 3.0
        for a, b in ((0, 1), (1, 2), (2, 0)):
            for sa in (1, -1):
                for sb in (1, -1):
                    out += np.roll(np.roll(f, sa, a), sb, b) / 6.0
        return out

    for _ in range(T):
        phi, prev = 2 * phi - prev + C * C * lap(phi), phi
    return np.abs(phi)


def fib_directions(n):
    i = np.arange(n) + 0.5
    z = 1 - 2 * i / n
    r = np.sqrt(np.maximum(0.0, 1 - z * z))
    th = np.pi * (1 + 5 ** 0.5) * i
    return np.stack([r * np.cos(th), r * np.sin(th), z], -1)


def trilinear(a, pts, R):
    """Sample |phi| at continuous positions (lattice coords, origin centre)."""
    q = pts + R
    f = np.floor(q).astype(int)
    d = q - f
    f = np.clip(f, 0, a.shape[0] - 2)
    out = np.zeros(len(pts))
    for dx in (0, 1):
        for dy in (0, 1):
            for dz in (0, 1):
                w = ((1 - d[:, 0]) if dx == 0 else d[:, 0]) * \
                    ((1 - d[:, 1]) if dy == 0 else d[:, 1]) * \
                    ((1 - d[:, 2]) if dz == 0 else d[:, 2])
                out += w * a[f[:, 0] + dx, f[:, 1] + dy, f[:, 2] + dz]
    return out


def radius_at(a, dirs, eps_list, R, rmax, nr=1400):
    """Outermost radius at which |phi| still reaches eps, per direction."""
    rs = np.linspace(0.0, rmax, nr)
    prof = np.empty((len(dirs), nr))
    for i, u in enumerate(dirs):
        prof[i] = trilinear(a, np.outer(rs, u), R)
    # envelope: running max from the outside in, so the crossing is unique
    env = np.maximum.accumulate(prof[:, ::-1], axis=1)[:, ::-1]
    out = {}
    for eps in eps_list:
        idx = (env >= eps).sum(axis=1) - 1
        out[eps] = np.where(idx >= 0, rs[np.clip(idx, 0, nr - 1)], np.nan)
    return out


def scan(T, R, ndir=600, nr=4000):
    """A(eps) at one t.  Rows whose mean radius falls inside the effective
    cone are NEAR FIELD, not a strict causal boundary, and are dropped: there the
    envelope is tracking the packet interior, not its edge."""
    a = greens(T, R)
    dirs = fib_directions(ndir)
    key = np.array([[1, 0, 0], [1, 1, 0], [1, 1, 1]], float)
    key /= np.linalg.norm(key, axis=1, keepdims=True)
    eps_list = [10.0 ** -e for e in range(2, 16)]
    rmax = np.sqrt(2) * T + 1.0
    Rv = radius_at(a, dirs, eps_list, R, rmax, nr)
    Rk = radius_at(a, key, eps_list, R, rmax, nr)
    rows = []
    for eps in eps_list:
        r = Rv[eps]
        r = r[np.isfinite(r) & (r > 1)]
        if len(r) < ndir // 2 or r.mean() < C * T:
            continue                        # near field: not a boundary
        rows.append(dict(eps=eps, Rm=r.mean(),
                         A=(r.max() - r.min()) / r.mean(),
                         rms=r.std() / r.mean(), key=Rk[eps]))
    return a, rows


def main():
    cub = (np.sqrt(2) - 1) / ((np.sqrt(2) + 1) / 2)
    print("Causal isotropy versus detection amplitude")
    print(f"  M18 leapfrog, C = 1/sqrt(3), point source")
    print(f"  bare cuboctahedron anisotropy (max-min)/mean = {cub:.4f}"
          f"   (the stencil's own shape)")

    T, R = 24, 36
    a, rows = scan(T, R)
    print(f"\n  DETAIL AT t = {T}   (effective cone {C*T:.3f}, support "
          f"{1.0*T:.1f}-{np.sqrt(2)*T:.1f}, peak |phi| {a.max():.3e})")
    print("     eps      R_mean   (max-min)/mean   rms/mean"
          "      [100]     [110]     [111]")
    for r in rows:
        k = r["key"]
        print(f"    {r['eps']:.0e}  {r['Rm']:8.3f}   {r['A']:9.4f}"
              f"   {r['rms']:9.4f}   {k[0]:8.3f}  {k[1]:8.3f}  {k[2]:8.3f}")

    print("\n  SCALING WITH t, AT A SELF-NORMALIZING THRESHOLD")
    print("    The effective-cone amplitude itself decays with t, so a fixed eps")
    print("    ladder drifts relative to that contour.  The threshold is set")
    print("    instead to eta x (envelope at r = C t), which tracks it.")
    print("\n      t    E_front      A(eta=1)   A(eta=.01)  A(eta=1e-4)")
    dat = {1.0: [], 1e-2: [], 1e-4: []}
    for T2, R2 in ((10, 17), (14, 22), (18, 28), (24, 36), (30, 45),
                   (36, 54)):
        a2 = greens(T2, R2)
        dirs = fib_directions(600)
        rmax = np.sqrt(2) * T2 + 1.0
        nr = 5000
        rs = np.linspace(0.0, rmax, nr)
        prof = np.empty((len(dirs), nr))
        for i, u in enumerate(dirs):
            prof[i] = trilinear(a2, np.outer(rs, u), R2)
        env = np.maximum.accumulate(prof[:, ::-1], axis=1)[:, ::-1]
        jf = int(np.searchsorted(rs, C * T2))
        Ef = float(env[:, jf].mean())        # envelope AT the front
        line = f"    {T2:3d}   {Ef:.3e}"
        for eta in (1.0, 1e-2, 1e-4):
            idx = (env >= eta * Ef).sum(axis=1) - 1
            r = rs[np.clip(idx, 0, nr - 1)][idx >= 0]
            A = (r.max() - r.min()) / r.mean()
            dat[eta].append((T2, A))
            line += f"    {A:8.5f}"
        print(line)

    print()
    for eta in (1.0, 1e-2, 1e-4):
        t = np.array([d[0] for d in dat[eta]], float)
        A = np.array([d[1] for d in dat[eta]], float)
        p, c = np.polyfit(np.log(t), np.log(A), 1)
        print(f"    eta = {eta:6.0e}:  A(t) ~ t^({p:+.2f})"
              f"   [Airy prediction: -4/3 = -1.33]")
    print(f"\n  READ-OFF")
    print(f"    At the effective cone the detected contour is spherical to"
          f" {rows[0]['A']:.2%} at t={T}.")
    print(f"    Fourteen decades down it has only reached"
          f" {rows[-1]['A']:.2%}, against the stencil's own {cub:.1%}.")


if __name__ == "__main__":
    main()
