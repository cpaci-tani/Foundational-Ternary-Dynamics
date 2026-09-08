# scripts/phi_v2_lattice/hydro/boltzmann.py
"""H1-prime: exact linearized Boltzmann operator of the successor at physical density.

Everything acceptance-grade is fmpq/Fraction; the float64 helpers serve campaign
predictions and laboratory curves only.
"""
from __future__ import annotations
from fractions import Fraction
import flint
import numpy as np
import sympy as sp
from ..recovery_hydro_dispersion import Exact, Dispersion
from ..recovery_hydro_verdict import exact_verdict
from . import channels as H

N = 24
DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))
_BITS = np.arange(N, dtype=np.uint32)
_COUNTS = None


def _bits(states, table_slice=None):
    src = states if table_slice is None else table_slice
    return ((src[:, None] >> _BITS[None, :]) & 1).astype(np.int64)


def collision_counts():
    """C[n,c,e] = sum_{|A|=n} (1_c(F(A)) - 1_c(A)) 1[e in A]; D likewise with 1[e not in A]."""
    global _COUNTS
    if _COUNTS is not None:
        return _COUNTS
    table = H.load_table()
    C = np.zeros((N + 1, N, N), dtype=np.int64)
    D = np.zeros((N + 1, N, N), dtype=np.int64)
    chunk = 1 << 18
    for start in range(0, 1 << N, chunk):
        states = np.arange(start, start + chunk, dtype=np.uint32)
        A = _bits(states)
        F = _bits(states, table[start:start + chunk])
        delta = F - A
        n = A.sum(axis=1)
        for m in np.unique(n):
            sel = n == m
            C[m] += delta[sel].T @ A[sel]
            D[m] += delta[sel].T @ (1 - A[sel])
    _COUNTS = (C, D)
    return _COUNTS


def K_exact(d):
    d = Fraction(d)
    C, D = collision_counts()
    K = [[Fraction(0)] * N for _ in range(N)]
    for n in range(N + 1):
        wC = d ** (n - 1) * (1 - d) ** (N - n) if n >= 1 else Fraction(0)
        wD = d ** n * (1 - d) ** (N - 1 - n) if n <= N - 1 else Fraction(0)
        for c in range(N):
            for e in range(N):
                if C[n, c, e]:
                    K[c][e] += wC * int(C[n, c, e])
                if D[n, c, e]:
                    K[c][e] -= wD * int(D[n, c, e])
    return Exact.from_rows(K)


def direction_matrices(direction):
    D1, D2 = Exact.zeros(N, N), Exact.zeros(N, N)
    half = Exact.scalar(Fraction(1, 2))
    for c, v in enumerate(H.VELOCITIES):
        value = sum(n * x for n, x in zip(direction, v))
        D1[c, c] = value
        D2[c, c] = half * (value * value)
    return D1, D2


def period_series(direction, d):
    J = Exact.identity(N) + K_exact(d)
    D1, D2 = direction_matrices(direction)
    return J, D1 * J, D2 * J


def weights_rows():
    return tuple([tuple([1] * N)] + [tuple(v[a] for v in H.VELOCITIES) for a in range(3)])


def bases(P0, d):
    W = Exact.from_rows([list(r) for r in weights_rows()])
    A = Exact.identity(N) - P0
    if W * A != Exact.zeros(4, N):
        raise ValueError("number/momentum weights are not the left null space of I - P0")
    scale = Exact.scalar(Fraction(d).denominator ** N)
    B = flint.fmpz_mat(N, N)
    for i in range(N):
        for j in range(N):
            q = A[i, j] * scale
            if q.q != 1:
                raise ValueError("entry denominator does not divide den(d)^24")
            B[i, j] = int(q.p)
    basis, nullity = B.nullspace()
    if nullity != 4:
        raise ValueError(f"right nullity {nullity} != 4: undiscovered invariant or defect")
    V = Exact.zeros(N, 4)
    for j in range(4):
        for i in range(N):
            V[i, j] = int(basis[i, j])
    return W, V * (W * V).inv()


def reduced_resolvent(P0, W, V):
    I = Exact.identity(N)
    VW = V * W
    return ((I - P0) + VW).inv() * (I - VW)


def effective_operators(series, W, V, R):
    P0, A1, A2 = series
    return W * A1 * V, W * (A2 + A1 * R * A1) * V


def dispersion(direction, d) -> Dispersion:
    series = period_series(direction, d)
    W, V = bases(series[0], d)
    R = reduced_resolvent(series[0], W, V)
    M1, M2 = effective_operators(series, W, V, R)
    return Dispersion("exact_fmpq", tuple(direction), None, Fraction(d), M1, M2, W, V, N - 4, series[0])


def verdict(d) -> dict:
    out = exact_verdict({n: dispersion(n, d) for n in DIRECTIONS})
    out["density"] = str(Fraction(d))
    trans = out.get("transverse_polynomial_normalized", {})
    if trans:
        c1, c0 = (Fraction(x) for x in next(iter(trans.values())))
        nu = -c1 / 2
        out["shear_viscosity"] = str(nu)
        out["transverse_double_root"] = bool(c1 * c1 == 4 * c0)
        out["stable"] = bool(nu > 0)
    else:
        out["shear_viscosity"], out["transverse_double_root"], out["stable"] = None, None, None
    speeds = out.get("sound_speed_squared_normalized", {})
    out["sound_speed_squared"] = next(iter(speeds.values())) if speeds else None
    return out


def nonlinear_coefficients(d):
    d = Fraction(d)
    dq = sp.Rational(d.numerator, d.denominator)
    E = (1 - dq) / dq                       # exp(h0) with f(h0) = d
    u, h2, q1 = sp.symbols("u h2 q1")
    def occupancy(vx):
        delta = h2 * u ** 2 + q1 * u * vx
        return sp.series(1 / (1 + E * sp.exp(delta)), u, 0, 3).removeO()
    occ = [sp.expand(occupancy(sp.Integer(v[0]))) for v in H.VELOCITIES]
    rho = 24 * dq
    mass = sp.expand(sum(occ) - rho)
    mom = sp.expand(sum(n * v[0] for n, v in zip(occ, H.VELOCITIES)) - rho * u)
    q1_sol = sp.solve(sp.Eq(mom.coeff(u, 1), 0), q1)[0]
    h2_sol = sp.solve(sp.Eq(sp.expand(mass.coeff(u, 2).subs(q1, q1_sol)), 0), h2)[0]
    subs = {q1: q1_sol, h2: h2_sol}
    Pxx = sp.expand(sum(n * v[0] ** 2 for n, v in zip(occ, H.VELOCITIES)).subs(subs))
    Pyy = sp.expand(sum(n * v[1] ** 2 for n, v in zip(occ, H.VELOCITIES)).subs(subs))
    to_fraction = lambda x: Fraction(str(sp.nsimplify(sp.simplify(x))))
    return {"g": to_fraction((Pxx.coeff(u, 2) - Pyy.coeff(u, 2)) / rho),
            "pressure_0": to_fraction(Pyy.coeff(u, 0)), "pressure_u2": to_fraction(Pyy.coeff(u, 2)),
            "q1": to_fraction(q1_sol), "h2": to_fraction(h2_sol)}


def numeric_jacobian(occupancy: np.ndarray) -> np.ndarray:
    """float64 K at a general per-velocity occupancy vector (prediction use only)."""
    occ = np.asarray(occupancy, dtype=float)
    table = H.load_table()
    log_on, log_off = np.log(occ), np.log1p(-occ)
    K = np.zeros((N, N))
    chunk = 1 << 18
    for start in range(0, 1 << N, chunk):
        states = np.arange(start, start + chunk, dtype=np.uint32)
        A = _bits(states).astype(float)
        F = _bits(states, table[start:start + chunk]).astype(float)
        w = np.exp(A @ log_on + (1 - A) @ log_off)
        delta = F - A
        factor = A / occ[None, :] - (1 - A) / (1 - occ)[None, :]
        K += (delta * w[:, None]).T @ factor
    return K


def numeric_stage_map(k, occupancy) -> np.ndarray:
    V = np.array(H.VELOCITIES, dtype=float)
    phase = np.exp(-1j * V @ np.asarray(k, dtype=float))
    return phase[:, None] * (np.eye(N) + numeric_jacobian(occupancy))


def equilibrium(d, u0: float) -> np.ndarray:
    """Fermi-Dirac occupancy N_i = 1/(1+exp(h + q v_ix)) with sum N = 24 d, sum N v_x = 24 d u0."""
    V = np.array(H.VELOCITIES, dtype=float)
    rho = 24 * float(Fraction(d))
    h, q = float(np.log((1 - float(Fraction(d))) / float(Fraction(d)))), 0.0
    for _ in range(100):
        z = h + q * V[:, 0]
        f = 1 / (1 + np.exp(z)); fp = -f * (1 - f)
        residual = np.array([f.sum() - rho, f @ V[:, 0] - rho * u0])
        if np.abs(residual).max() < 1e-15:
            break
        jac = np.array([[fp.sum(), fp @ V[:, 0]], [fp @ V[:, 0], fp @ V[:, 0] ** 2]])
        h, q = np.array([h, q]) - np.linalg.solve(jac, residual)
    return 1 / (1 + np.exp(h + q * V[:, 0]))
