"""Is a periodic triangulated sheet an n = 4 mechanism?

Protocol: PREREG_TRIANGULATED_SHEET_N4_v1.md, locked at 0abf097b BEFORE this
file existed.

Sheet: triangular lattice, spacing 1, all sites s = 0 (mask = 1/2 for every
pair, forced -- triangles mean it cannot be +/- 2-coloured). NN at q = 1 (the
compact-law minimum), 2nd neighbours at sqrt3 = 1.732 > support 1.2247.

Two protocols, per prereg sec 3:
  P1 FIXED box  - relax site coordinates at fixed lattice vectors
  P2 FREE box   - additionally relax the cell (variable-cell strain)
A carrier is a free body, so P2 is the physically relevant one, and it is the
one the corrugation argument of prereg sec 2 says might kill this.
"""
import numpy as np
from scipy.optimize import minimize

EPS, CUT = 0.01, 1.5
MASK = 0.5                                   # all sites neutral


def V(q):
    return 0.0 if q >= CUT else -16 * EPS * (q - 1.5) ** 2 * (q - 0.75)


def dV(q):
    return 0.0 if q >= CUT else -48 * EPS * (q - 1.5) * (q - 1.0)


def sheet(m, n):
    """Triangular lattice, m x n cells, spacing 1, in the z = 0 plane."""
    a1 = np.array([1.0, 0.0, 0.0])
    a2 = np.array([0.5, np.sqrt(3) / 2, 0.0])
    p0 = np.array([i * a1 + j * a2 for i in range(m) for j in range(n)])
    A0 = np.array([m * a1, n * a2])
    # bonds by minimum image over the 6 NN directions
    dirs = [(1, 0), (0, 1), (-1, 1), (-1, 0), (0, -1), (1, -1)]
    idx = {(i, j): k for k, (i, j) in enumerate(
        [(i, j) for i in range(m) for j in range(n)])}
    bonds = []
    for i in range(m):
        for j in range(n):
            for di, dj in dirs:
                ii, jj = i + di, j + dj
                w1 = (ii // m) if ii not in range(m) else 0
                w2 = (jj // n) if jj not in range(n) else 0
                k, l = ii % m, jj % n
                a, b = idx[(i, j)], idx[(k, l)]
                if a < b or (a == b and (w1, w2) > (0, 0)):
                    bonds.append((a, b, w1, w2))
    # de-duplicate
    seen, out = set(), []
    for a, b, w1, w2 in bonds:
        key = (a, b, w1, w2) if a < b else (b, a, -w1, -w2)
        if key not in seen:
            seen.add(key); out.append(key)
    return p0, A0, out


def energy(u, strain, p0, A0, bonds):
    E3 = np.eye(3) + strain
    p = p0 @ E3.T + u.reshape(-1, 3)
    A = A0 @ E3.T
    e = 0.0
    for a, b, w1, w2 in bonds:
        d = p[b] + w1 * A[0] + w2 * A[1] - p[a]
        e += MASK * V(float(d @ d))
    return e


def grad(u, strain, p0, A0, bonds, want_strain=False):
    E3 = np.eye(3) + strain
    p = p0 @ E3.T + u.reshape(-1, 3)
    A = A0 @ E3.T
    gu = np.zeros_like(p); gs = np.zeros((3, 3))
    for a, b, w1, w2 in bonds:
        d0 = p0[b] + w1 * A0[0] + w2 * A0[1] - p0[a]
        d = p[b] + w1 * A[0] + w2 * A[1] - p[a]
        q = float(d @ d)
        if q >= CUT:
            continue
        f = MASK * dV(q) * 2.0
        gu[b] += f * d; gu[a] -= f * d
        if want_strain:
            gs += f * np.outer(d, d0)
    return (gu.reshape(-1), gs) if want_strain else gu.reshape(-1)


def relax(zmode, amp, p0, A0, bonds, free_box, restarts=6):
    """Hold the out-of-plane mode at `amp`, relax everything else.
    free_box=True additionally relaxes the cell (prereg P2)."""
    N = len(p0)
    z = np.zeros((N, 3)); z[:, 2] = zmode
    z = z.reshape(-1); z /= np.linalg.norm(z)
    fixed = amp * z
    # free directions: all u orthogonal to the held mode
    def unpack(v):
        u = fixed + v[:3 * N] - z * (z @ v[:3 * N])
        s = np.zeros((3, 3))
        if free_box:
            s[0, 0], s[1, 1] = v[3 * N], v[3 * N + 1]
            s[0, 1] = s[1, 0] = v[3 * N + 2]
        return u, s

    def f(v):
        u, s = unpack(v)
        return energy(u, s, p0, A0, bonds)

    def fp(v):
        u, s = unpack(v)
        gu, gs = grad(u, s, p0, A0, bonds, want_strain=True)
        gu = gu - z * (z @ gu)
        out = np.zeros_like(v); out[:3 * N] = gu
        if free_box:
            out[3 * N] = gs[0, 0]; out[3 * N + 1] = gs[1, 1]
            out[3 * N + 2] = gs[0, 1] + gs[1, 0]
        return out

    # Relaxation must find the TRUE minimum at fixed mode amplitude. The
    # variable-cell landscape is stiff, and a single start gave
    # non-monotonic energies (a four-order drop at one amplitude only),
    # i.e. optimiser failure rather than physics. Restart and take the best.
    n = 3 * N + (3 if free_box else 0)
    rng = np.random.default_rng(11)
    best = np.inf
    for k in range(restarts):
        v0 = np.zeros(n) if k == 0 else rng.normal(scale=0.02 * amp, size=n)
        r = minimize(f, v0, jac=fp, method="L-BFGS-B",
                     options=dict(maxiter=200000, ftol=1e-20, gtol=1e-18,
                                  maxfun=200000))
        # polish
        r = minimize(f, r.x, jac=fp, method="L-BFGS-B",
                     options=dict(maxiter=200000, ftol=1e-20, gtol=1e-18,
                                  maxfun=200000))
        best = min(best, float(r.fun))
    return best


def run(m, n, label):
    p0, A0, bonds = sheet(m, n)
    N = len(p0)
    E0 = energy(np.zeros(3 * N), np.zeros((3, 3)), p0, A0, bonds)
    print(f"\n=== {label}: {m}x{n}, N={N}, B={len(bonds)} "
          f"(3N={3*N})  E0={E0:.8f} ===")
    # C-stress: verify omega = 1 is a self-stress
    resid = np.zeros((N, 3))
    for a, b, w1, w2 in bonds:
        d = p0[b] + w1 * A0[0] + w2 * A0[1] - p0[a]
        resid[a] += d; resid[b] -= d
    print(f"  C-stress: max |sum_j (p_i - p_j)| = "
          f"{np.abs(resid).max():.3e}  (omega=1 is a self-stress)")

    # out-of-plane test modes, including the corrugation of prereg sec 2
    x, y = p0[:, 0], p0[:, 1]
    qx = 2 * np.pi / (m * 1.0)
    modes = {
        "corrugation cos(q.x)": np.cos(qx * x),
        "corrugation cos(2q.x)": np.cos(2 * qx * x),
        "egg-carton": np.cos(qx * x) * np.cos(2 * np.pi * y / (n * np.sqrt(3) / 2)),
        "random": np.random.default_rng(7).normal(size=N),
    }
    amps = (0.02, 0.05, 0.1, 0.2)
    print(f"  {'mode':>24} {'protocol':>10} " +
          " ".join(f"{a:>11.3f}" for a in amps) + f" {'exponent':>9}")
    for name, zm in modes.items():
        zm = zm - zm.mean()
        for tag, fb in (("P1 fixed", False), ("P2 FREE", True)):
            dE = [relax(zm, a, p0, A0, bonds, fb) - E0 for a in amps]
            pos = [d for d in dE if d > 1e-15]
            ex = (np.polyfit(np.log(amps[-len(pos):]), np.log(pos), 1)[0]
                  if len(pos) >= 2 else float("nan"))
            print(f"  {name:>24} {tag:>10} " +
                  " ".join(f"{d:>11.3e}" for d in dE) + f" {ex:>9.3f}")


if __name__ == "__main__":
    for m, n in ((4, 4), (6, 6)):
        run(m, n, "triangulated sheet")
