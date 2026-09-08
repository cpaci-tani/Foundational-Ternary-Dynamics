# scripts/phi_v2_lattice/recovery_hydro_dispersion.py
"""Small-wavevector dispersion of the 12-stage linearized Boltzmann propagator (gate H1).

With the formal variable eps = -i*kappa and wavevector k = kappa*direction, the
streaming phase e^{-i k.d} = e^{eps (direction.d)} expands with real rational
coefficients, so P(eps) = P0 + eps*A1 + eps^2*A2 has real entries. Eigenvalues
read lambda = 1 - i*kappa*mu1 - kappa^2*mu2 and the per-period damping is
mu2 - mu1^2/2. Two tracks: exact fmpq at the declared proxy coupling PROXY_R,
certified arb enclosures at the physical coupling r(p). The contract is
engine/docs/DERIV_STRICT_HYDRODYNAMIC_DISPERSION.md.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction

import flint
import numpy as np

from . import channels as C
from . import recovery_hydro_invariants as H
from . import recovery_kinetic_response as R

N = 192
PROXY_R = Fraction(1, 695)
DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))
REFERENCE_PS = (Fraction(1, 96), Fraction(1, 192), Fraction(1, 48))
STAGES = 12  # one period = 12 stages = 48 physical microticks


def physical_r(p) -> Fraction:
    p = Fraction(p)
    if not 0 < p < 1:
        raise ValueError("interior reference density required")
    return p * (1 - p) ** 189


class Exact:
    """python-flint fmpq_mat arithmetic (proxy coupling)."""
    name = "exact_fmpq"

    @staticmethod
    def scalar(x):
        x = Fraction(x)
        return flint.fmpq(x.numerator, x.denominator)

    @staticmethod
    def zeros(n, m):
        return flint.fmpq_mat(n, m)

    @staticmethod
    def identity(n):
        I = flint.fmpq_mat(n, n)
        for i in range(n):
            I[i, i] = 1
        return I

    @staticmethod
    def from_rows(rows):
        return flint.fmpq_mat([[Exact.scalar(v) for v in row] for row in rows])


class Certified:
    """python-flint arb_mat ball arithmetic (physical coupling); precision is ctx.prec."""
    name = "certified_arb"

    @staticmethod
    def scalar(x):
        x = Fraction(x)
        return flint.arb(x.numerator) / flint.arb(x.denominator)

    @staticmethod
    def zeros(n, m):
        return flint.arb_mat(n, m)

    @staticmethod
    def identity(n):
        I = flint.arb_mat(n, n)
        for i in range(n):
            I[i, i] = 1
        return I

    @staticmethod
    def from_rows(rows):
        return flint.arb_mat([[Certified.scalar(v) for v in row] for row in rows])


def encloses(ball, exact) -> bool:
    """True when the arb ball contains the exact rational."""
    delta = ball - Certified.scalar(Fraction(int(exact.p), int(exact.q)))
    return abs(delta.mid()) <= delta.rad()


def streaming_permutation(A):
    P = A.zeros(N, N)
    for c in range(N):
        P[C.U(c), c] = 1
    return P


def collision_jacobian(A, layer, r):
    """I + r*K_layer: the first-degree marginal Jacobian of the exactly-two collision."""
    K = R.collision_operator(layer).integer_correction
    rs = A.scalar(r)
    J = A.zeros(N, N)
    for i in range(N):
        row = K[i]
        for j in range(N):
            if row[j]:
                J[i, j] = rs * row[j]
        J[i, i] += 1
    return J


def direction_matrices(A, direction):
    """diag(direction . d(c)) and diag((direction . d(c))^2 / 2)."""
    D1 = A.zeros(N, N)
    D2 = A.zeros(N, N)
    half = A.scalar(Fraction(1, 2))
    for c in range(N):
        value = sum(n * d for n, d in zip(direction, C.tangent(c)))
        D1[c, c] = value
        D2[c, c] = half * (value * value)
    return D1, D2


def _series_mul(X, Y):
    return (X[0] * Y[0], X[0] * Y[1] + X[1] * Y[0], X[0] * Y[2] + X[1] * Y[1] + X[2] * Y[0])


def period_series(A, direction, r, l0=0):
    """(P0, A1, A2): P(eps) = P0 + eps*A1 + eps^2*A2 over one 12-stage period.

    Stage t applies collision layer (l0 - t) mod 3 then streams: M_t = S(eps) J_t,
    S(eps) = P_U (I + eps D1 + eps^2 D2). The period map is M_11 ... M_0.
    """
    PU = streaming_permutation(A)
    D1, D2 = direction_matrices(A, direction)
    S = (PU, PU * D1, PU * D2)
    J = [collision_jacobian(A, q, r) for q in range(3)]
    acc = (A.identity(N), A.zeros(N, N), A.zeros(N, N))
    for t in range(STAGES):
        Jt = J[(l0 - t) % 3]
        acc = _series_mul((S[0] * Jt, S[1] * Jt, S[2] * Jt), acc)
    return acc


@dataclass(frozen=True)
class Bases:
    W: object  # 7 x N conserved weights (rows), stage 0
    V: object  # N x 7 equilibrium modes with W V = I


def exact_bases(P0, r, l0=0) -> Bases:
    """W from the locked kernel; V as the exact right null space normalized to W V = I."""
    W = Exact.from_rows(H.locked_kernel(l0))
    A = Exact.identity(N) - P0
    if W * A != Exact.zeros(7, N):
        raise ValueError("locked kernel is not the left null space of I - P0")
    scale = Exact.scalar(Fraction(r).denominator ** STAGES)
    B = flint.fmpz_mat(N, N)
    for i in range(N):
        for j in range(N):
            q = A[i, j] * scale
            if q.q != 1:
                raise ValueError("entry denominator does not divide den(r)^12")
            B[i, j] = int(q.p)
    basis, nullity = B.nullspace()
    if nullity != 7:
        raise ValueError(f"right nullity {nullity} != 7: undiscovered invariant or defect")
    V = Exact.zeros(N, 7)
    for j in range(7):
        for i in range(N):
            V[i, j] = int(basis[i, j])
    return Bases(W, V * (W * V).inv())


def certified_bases(P0, proxy: Bases) -> Bases:
    """V(r) = T^{-1} V0 with T = (I - P0) + V0 W; solve certifies invertibility."""
    W = Certified.from_rows([[int(proxy.W[i, j].p) for j in range(N)] for i in range(7)])
    V0 = flint.arb_mat([[Certified.scalar(Fraction(int(proxy.V[i, j].p), int(proxy.V[i, j].q)))
                         for j in range(7)] for i in range(N)])
    T = (Certified.identity(N) - P0) + V0 * W
    return Bases(W, T.solve(V0))


def reduced_resolvent(A, P0, W, V):
    """R with (I - P0) R = I - V W, W R = 0, R V = 0."""
    I = A.identity(N)
    VW = V * W
    return ((I - P0) + VW).inv() * (I - VW)


def effective_operators(series, W, V, resolvent):
    P0, A1, A2 = series
    M1 = W * A1 * V
    M2 = W * (A2 + A1 * resolvent * A1) * V
    return M1, M2


@dataclass(frozen=True)
class Dispersion:
    track: str
    direction: tuple
    r: Fraction | None
    p: Fraction | None
    M1: object
    M2: object
    W: object
    V: object
    rank_complement: int | None
    P0: object


def exact_dispersion(direction, r=PROXY_R, l0=0) -> Dispersion:
    series = period_series(Exact, direction, r, l0)
    bases = exact_bases(series[0], r, l0)
    rank = (Exact.identity(N) - series[0]).rank()
    resolvent = reduced_resolvent(Exact, series[0], bases.W, bases.V)
    M1, M2 = effective_operators(series, bases.W, bases.V, resolvent)
    return Dispersion(Exact.name, tuple(direction), Fraction(r), None, M1, M2,
                      bases.W, bases.V, rank, series[0])


def certified_dispersion(direction, p, proxy: Dispersion, r=None, l0=0, prec=256) -> Dispersion:
    """Certified enclosures at physical r(p) (or an explicit r for cross-checks)."""
    saved = flint.ctx.prec
    flint.ctx.prec = prec
    try:
        r = physical_r(p) if r is None else Fraction(r)
        series = period_series(Certified, direction, r, l0)
        bases = certified_bases(series[0], Bases(proxy.W, proxy.V))
        resolvent = reduced_resolvent(Certified, series[0], bases.W, bases.V)
        M1, M2 = effective_operators(series, bases.W, bases.V, resolvent)
        return Dispersion(Certified.name, tuple(direction), r, None if p is None else Fraction(p),
                          M1, M2, bases.W, bases.V, None, series[0])
    finally:
        flint.ctx.prec = saved


_RESIDUAL_PREC = 4096  # ctx.prec floor for this function's own arithmetic


def certified_residuals(d: Dispersion) -> dict:
    """Largest ball radii of the defining identities; a rigorous consistency report.

    d's arb entries already carry whatever radius their construction precision
    gave them; this function's own subtractions/products must not themselves
    become the bottleneck by running at flint's ambient ctx.prec (module
    default 53 bits -- double precision -- since certified_dispersion restores
    the caller's saved precision before returning). Elevate locally so the
    reported radius reflects d's actual precision, not the caller's ambient one.
    """
    saved = flint.ctx.prec
    flint.ctx.prec = max(saved, _RESIDUAL_PREC)
    try:
        I = Certified.identity(N)
        left = d.W * (I - d.P0)
        bio = d.W * d.V - Certified.identity(7)
        right = (I - d.P0) * d.V
        def worst(M, n, m):
            return max(float(abs(M[i, j].mid()) + M[i, j].rad()) for i in range(n) for j in range(m))
        return {"max_left_null_radius": worst(left, 7, N),
                "max_biorthogonality_error": worst(bio, 7, 7),
                "max_right_null_error": worst(right, N, 7)}
    finally:
        flint.ctx.prec = saved


def numeric_period_map(k, p, r=None, l0=0) -> np.ndarray:
    """Full complex period map at a finite wavevector k (radians per site), float64.

    Prediction-curve use only. S_k = P_U diag(exp(-i k.d)); J_q = I + r K_q.
    """
    r = float(physical_r(p)) if r is None else float(r)
    d = np.array([C.tangent(c) for c in range(N)], dtype=float)
    phase = np.exp(-1j * d @ np.asarray(k, dtype=float))
    PU = np.zeros((N, N))
    PU[[C.U(c) for c in range(N)], range(N)] = 1.0
    S = PU * phase[None, :]
    J = [np.eye(N) + r * np.array(R.collision_operator(q).integer_correction, dtype=float)
         for q in range(3)]
    P = np.eye(N, dtype=complex)
    for t in range(STAGES):
        P = (S @ J[(l0 - t) % 3]) @ P
    return P
