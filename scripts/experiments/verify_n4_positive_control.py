"""POSITIVE CONTROL the prereg omitted: can the screen DETECT n = 4 when it
is genuinely present?

PREREG_MAXWELL_C3_SCREEN_v1 Tier A carries only NEGATIVE controls (a known
n = infinity and a known n = 2). A screen that returned "no n = 4" because it
is structurally blind to n = 4 would pass both. This closes that hole.

The textbook example of a framework that is first-order FLEXIBLE but
second-order RIGID: three COLLINEAR points with all three bars present,
natural lengths 1, 1, 2. Moving the middle point perpendicular preserves all
bond lengths to first order, but at second order the two short bars must
lengthen by ~d^2/2 each while the long bar is fixed -- the triangle inequality
blocks the motion. Energy ~ d^4, positive. That is n = 4 by construction.

Harmonic bonds are used deliberately: this is a control on the ALGORITHM
(null space, relaxation guard, exponent fit), not on the FTD compact law.
"""
import numpy as np
from scipy.linalg import null_space, orth
from scipy.optimize import minimize

K = 1.0
BONDS = [(0, 1, 1.0), (1, 2, 1.0), (0, 2, 2.0)]   # (i, j, natural length)


def energy(x):
    p = x.reshape(-1, 3); e = 0.0
    for i, j, L in BONDS:
        e += 0.5 * K * (np.linalg.norm(p[i] - p[j]) - L) ** 2
    return float(e)


def trivial_modes(x):
    p = x.reshape(-1, 3); n = len(p); r = p - p.mean(0)
    cols = [np.tile(a, n) for a in np.eye(3)]
    cols += [np.cross(a, r).reshape(-1) for a in np.eye(3)]
    return orth(np.array(cols).T, rcond=1e-10)


x0 = np.array([-1., 0, 0, 0., 0, 0, 1., 0, 0])     # collinear, zero tension
print(f"E0 = {energy(x0):.3e}  (zero tension: every bond at natural length)")

h = 1e-5; n = 9
H = np.zeros((n, n))
for i in range(n):
    for j in range(n):
        a = x0.copy(); a[i] += h; a[j] += h
        b = x0.copy(); b[i] += h; b[j] -= h
        c = x0.copy(); c[i] -= h; c[j] += h
        d = x0.copy(); d[i] -= h; d[j] -= h
        H[i, j] = (energy(a) - energy(b) - energy(c) + energy(d)) / (4 * h * h)
H = (H + H.T) / 2
ev, evec = np.linalg.eigh(H)
print("Hessian eigenvalues:", np.round(ev, 8))

T = trivial_modes(x0)
N = evec[:, np.abs(ev) < 1e-7]
P = N - T @ (T.T @ N)
U, S, _ = np.linalg.svd(P, full_matrices=False)
N0 = U[:, S > 1e-8]
print(f"trivial modes = {T.shape[1]} (collinear)   nontrivial null dim = {N0.shape[1]}")

print(f"\n{'t':>8} {'dE straight':>15} {'dE RELAXED':>15} {'exponent':>10}")
prev = None
for t in (0.01, 0.02, 0.05, 0.1, 0.2):
    u = N0[:, 0]
    start = x0 + t * u
    C = null_space(np.hstack([T, u.reshape(-1, 1)]).T)
    r = minimize(lambda z: energy(start + C @ z), np.zeros(C.shape[1]),
                 method="L-BFGS-B", options=dict(maxiter=20000, ftol=1e-18))
    dE = float(r.fun) - energy(x0)
    ex = (np.log(dE / prev[1]) / np.log(t / prev[0])) if prev and dE > 0 else np.nan
    print(f"{t:>8.3f} {energy(start)-energy(x0):>15.6e} {dE:>15.6e} {ex:>10.4f}")
    prev = (t, dE)

print("\nIf dE RELAXED scales as t^4 with a POSITIVE coefficient, the screen")
print("detects n = 4 when it is present, and NO_NATIVE_N4 is a real negative")
print("rather than an artifact of a blind instrument.")
