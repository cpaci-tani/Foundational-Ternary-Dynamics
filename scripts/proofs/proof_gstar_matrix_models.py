"""proof_gstar_matrix_models.py — FTD-0366 / G* in strongly-coupled matrix models.

Claim (FTD-0366, [SYNTHESIS] + [STRUCTURAL OBSERVATION]):
    The strongly-coupled (monomial) quartic one-matrix model — the 0-dimensional
    QFT with action S = Tr X^4, exactly solved by Cordova-Heidenreich-Popolitov-
    Shakirov (CHPS), Commun. Math. Phys. 361 (2018) 1235-1274, arXiv:1611.03142
    (section/equation numbers per arXiv v1 = OSTI 1537659 copy) — has observables
    valued in Q(G*), G* = Gamma(1/4)/Gamma(3/4): its Z_4 contour eigen-sectors
    carry exactly the three Gamma-classes {Gamma(1/4), pi, Gamma(3/4)}, the
    conjugate-sector amplitude RATIO is G*-graded (Euler-reflection ratio) while
    the amplitude PRODUCT is pi-graded (Euler-reflection product), and normalized
    correlators are rational functions of G*.

What this script does:
    (C1)  Verifies the CHPS moment formula on Z_r ray-eigencontours
          (their eq 3.14) by 100-digit quadrature on real-parameterized rays.
    (C2)  Verifies CHPS Theorem 1 (eq 3.20) against direct quadrature (N<=2)
          and the real-line determinant formula (eq 3.5/3.7) symbolically.
    (C3)  Verifies the sector grading for r=4, N<=12: a=1 -> Q*Gamma(1/4)^N,
          a=2 -> Q*pi^(N/2), a=3 -> Q*Gamma(3/4)^N (symbolic, exact).
    (C4)  Verifies ratio/product dichotomy: amplitude ratio (4,1)/(4,3) = G* at
          N=1 and Q*G*^N at N in {4,8} (full Z ratio incl. delta signs);
          amplitude product at N=1 = Gamma(1/4)*Gamma(3/4) = sqrt(2)*pi
          [reflection at z=1/4], with explicit sqrt(2)*pi vs sqrt(2*pi)
          disambiguation.
    (C5)  Verifies <Tr X^2> in Q(G*): N=1: 1/G*; N=2: (G*^2+4)/(4G*) (both by
          quadrature); N=3: G*(G*^2+4)/(4(G*^2-4)) (symbolic determinant route,
          CHPS eq 3.6 — never 3D quadrature).
    (C6)  Verifies single-trace rationality (CHPS eq 3.50): r<Tr X^r> = N^2 and
          r^2<Tr X^(2r)> = 2N^3 + a*(r-a)*N on pure phases (symbolic moment-
          determinant route + one numeric spot check).
    (C7)  Verifies the r=3 grading: sectors carry Gamma(1/3)/Gamma(2/3) classes
          — the equianharmonic (d=-3) sibling, NOT G* (d=-4): the two CM worlds
          stay separate.
    (C8)  Verifies the FTD spine tie-ins: G* = Gamma(1/4)^2/(pi*sqrt(2)) and the
          Watson identity G*^2/(2*pi) = Gamma(1/4)^4/(4*pi^3).
    (C9)  Verifies CHPS Theorem 6 (Z_r Vandermonde projection/refactorization)
          symbolically for r in {3,4}, small N, including a delta=0 case.
    (C10) Verifies the pure-phase support rule (Z != 0 iff N = 0 or a mod r) and
          the FULL Theorem 1 identity det[moments] = delta_{r,a}(N) * amplitude
          (Andreief/Heine route, includes the sign) for r in {3,4}, all a, N<=6;
          plus floor-hardening unit checks (floor((i-a)/r) < 0 for i < a).

What this script is NOT:
    - NOT a derivation of any FTD physics claim. It verifies external theorems
      (CHPS 2018) and their intersection with already-theorem-grade FTD spine
      identities. It imports no physics from the matrix-model literature.
    - NOT a numerical search. Every check verifies a pre-stated exact identity
      (anti-target discipline; no PSLQ, no near-miss scanning anywhere).
    - NOT a promotion instrument. Standing invariants are untouched: no alpha
      is derived anywhere; x+ = 1/alpha stays [STRONGLY MOTIVATED CONJECTURE]
      (FTD-0013); MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION]; the master
      quadratic is NOT produced by the matrix model (sector ratios could
      manufacture 16G*^2 / 16G*^3 only as substitution identities, which are
      prohibited); the golden gate is untouched (docs + scripts only).

Usage:
    python scripts/proofs/proof_gstar_matrix_models.py
"""

from __future__ import annotations

import os
import sys
import time

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import mpmath as mpm
import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite, G_STAR as G_STAR_FLOAT64  # noqa: E402

# High-precision context for every numeric check (booleans only are registered
# with the ProofSuite — mpf residuals never cross the float64 boundary).
mpm.mp.dps = 100

TOL_1D = mpm.mpf(10) ** (-60)   # 1D ray quadrature at dps=100
TOL_2D = mpm.mpf(10) ** (-25)   # 2D eigenvalue quadrature, run at dps=30
TOL_ID = mpm.mpf(10) ** (-80)   # closed-form vs closed-form at dps=100

R14 = sp.Rational(1, 4)
G14 = sp.gamma(sp.Rational(1, 4))
G34 = sp.gamma(sp.Rational(3, 4))
G13 = sp.gamma(sp.Rational(1, 3))
G23 = sp.gamma(sp.Rational(2, 3))

suite = ProofSuite("G* in strongly-coupled matrix models (CHPS 2018 x FTD spine)")


# =============================================================================
# CHPS closed forms (all index arithmetic in exact sympy Rationals — the floor
# in Theorem 1 is genuine floor, NOT int() truncation: floor((i-a)/r) = -1 for
# 0 <= i < a, where int((i-a)/r) would give 0).
# =============================================================================

def amplitude(r: int, a: int, N: int) -> sp.Expr:
    """CHPS Theorem 1 Gamma-amplitude: prod_{i=0}^{N-1} Gamma(floor(i/r)+1) *
    Gamma(floor((i-a)/r) + a/r + 1).  (Z^{(r,a)}_N = delta_{r,a}(N) (2pi)^-N * this.)"""
    p = sp.Integer(1)
    for i in range(N):
        p *= sp.gamma(sp.floor(sp.Rational(i, r)) + 1)
        p *= sp.gamma(sp.floor(sp.Rational(i - a, r)) + sp.Rational(a, r) + 1)
    return sp.simplify(p)


def delta_sign(r: int, a: int, N: int) -> int:
    """CHPS eq 3.21: delta_{r,a}(N) in {0, +1, -1}."""
    if N % r not in (0, a % r):
        return 0
    at = r - a
    exponent = (-(-N // r)) * (a * (a - 1) // 2) + (N // r) * (at * (at - 1) // 2)
    return -1 if exponent % 2 else 1


def moment(r: int, a: int, q: int) -> sp.Expr:
    """CHPS eq 3.14: int_{C_{r,a}} x^q e^{-x^r} dx = delta_{r | q+1-a} Gamma((q+1)/r)."""
    if (q + 1 - a) % r != 0:
        return sp.Integer(0)
    return sp.gamma(sp.Rational(q + 1, r))


def Z_det(r: int, a: int, N: int, insert_power: int = 0) -> sp.Expr:
    """Pure-phase partition function (times (2pi)^N N-independent prefactor
    stripped) via the Andreief/Heine moment determinant:
        det_{NxN}[ m_{i+j-2} ]  (= delta_{r,a}(N) * amplitude(r,a,N)).
    With insert_power = p > 0, returns Z[Tr X^p] * (2pi)^N via
        sum_k det[ m_{i+j-2+p*delta_{ik}} ]   (CHPS eq 3.6 generalized to C_{r,a})."""
    if insert_power == 0:
        M = sp.Matrix(N, N, lambda i, j: moment(r, a, i + j))
        return sp.factor(sp.simplify(M.det()))
    total = sp.Integer(0)
    for k in range(N):
        M = sp.Matrix(N, N, lambda i, j: moment(r, a, i + j + (insert_power if i == k else 0)))
        total += M.det()
    return sp.factor(sp.simplify(total))


def gamma_class_ratio(expr: sp.Expr, base: sp.Expr, power: int) -> sp.Expr:
    """Return simplify(expr / base**power) — used to assert Q-membership."""
    return sp.cancel(sp.simplify(expr / base**power))


def is_rational_expr(q: sp.Expr) -> bool:
    return bool(sp.simplify(q).is_Rational)


# =============================================================================
# C1 — moment formula on Z_r ray eigencontours, by 100-digit quadrature.
#
# Contour parameterization (first complex-contour precedent in this repo):
# each ray B_j runs 0 -> omega^j * inf with omega = exp(2*pi*I/r).  Substituting
# z = omega^j * t (t real >= 0) gives z^r = t^r exactly, so
#     int_{B_j} z^q e^{-z^r} dz = omega^{j(q+1)} * int_0^inf t^q e^{-t^r} dt,
# and the integrand on every ray decays like exp(-t^r) — no oscillatory tails.
# The eigencontour C_{r,a} = sum_j omega^{-ja} B_j then has moments
#     sum_j omega^{j(q+1-a)} * I(q,r) = [r if r | q+1-a else 0] * I(q,r),
# with I(q,r) = Gamma((q+1)/r)/r computed by mpm.quad on [0, inf].
# =============================================================================

def check_c1() -> None:
    for r in (3, 4):
        for q in range(0, 2 * r):
            I_num = mpm.quad(lambda t, q=q, r=r: t**q * mpm.e**(-(t**r)), [0, mpm.inf])
            for a in range(1, r):
                acc = mpm.mpc(0)
                for j in range(r):
                    acc += mpm.exp(2j * mpm.pi * j * (q + 1 - a) / r)
                got = acc * I_num
                want = mpm.mpf(0) if (q + 1 - a) % r else mpm.gamma(mpm.mpf(q + 1) / r)
                ok = mpm.fabs(got - want) < TOL_1D
                suite.assert_true(
                    f"C1 moment r={r} a={a} q={q}: quad = closed Gamma-form", ok,
                    tag="[EXTERNAL]")


# =============================================================================
# C2 — CHPS Theorem 1 vs quadrature (N<=2) and vs the real-line determinant
# formula (eq 3.5) / listed values (eq 3.7), symbolically for N<=4.
# =============================================================================

def realline_Z_det(N: int) -> sp.Expr:
    """CHPS eq 3.5: Z_N = (4pi)^-N det[ (1+(-1)^(i+j))/2 * Gamma((i+j-1)/4) ],
    1-indexed i,j.  Returned WITHOUT the (4pi)^-N prefactor."""
    def entry(i, j):  # 0-indexed -> 1-indexed
        I, J = i + 1, j + 1
        if (I + J) % 2:
            return sp.Integer(0)
        return sp.gamma(sp.Rational(I + J - 1, 4))
    M = sp.Matrix(N, N, entry)
    return sp.factor(sp.simplify(M.det()))


def check_c2() -> None:
    # (a) Theorem 1 vs 1D quadrature at N=1, r=4: Z^(4,1)_1 = Gamma(1/4)/(2pi).
    I0 = mpm.quad(lambda t: mpm.e**(-(t**4)), [0, mpm.inf])
    z11 = mpm.mpc(0)
    for j in range(4):
        z11 += mpm.exp(2j * mpm.pi * j * (0 + 1 - 1) / 4)
    z11 = z11 * I0 / (2 * mpm.pi)  # C_{4,1} moment q=0, over 2pi
    amp_num = mpm.mpf(str(sp.N(amplitude(4, 1, 1), 110)))
    thm = delta_sign(4, 1, 1) * amp_num / (2 * mpm.pi)
    suite.assert_true("C2a Theorem 1 (4,1) N=1 = ray quadrature",
                      mpm.fabs(z11 - thm) < TOL_1D, tag="[EXTERNAL]")

    # (b) real-line determinant (3.5) vs listed closed forms (3.7), symbolic N<=4.
    listed = {
        1: G14,
        2: G14 * G34,
        3: G14**2 * G34 - 4 * G34**3,
        4: -G14**4 + 16 * G14**2 * G34**2 - 48 * G34**4,
    }
    # eq 3.7 denominators are 4pi, 16pi^2, 256pi^3, 2^14 pi^4
    # = (4pi)^N * 2^{(N-1)(N-2)}; our det strips (4pi)^-N, so
    # det / listed = 2^{-(N-1)(N-2)} (derived by direct expansion).
    want_ratio = {N: sp.Rational(1, 2**((N - 1) * (N - 2))) for N in (1, 2, 3, 4)}
    for N in (1, 2, 3, 4):
        q = sp.cancel(sp.simplify(realline_Z_det(N) / listed[N]))
        ok = is_rational_expr(q) and sp.simplify(q - want_ratio[N]) == 0
        suite.assert_true(
            f"C2b real-line det (3.5) N={N} = eq-3.7 form (x 2^-(N-1)(N-2))",
            bool(ok), tag="[EXTERNAL]")

    # (c) real-line Z_1, Z_2 vs direct quadrature (2D at dps=30, declared tol 1e-25;
    #     truncation to [-6,6] contributes < 1e-500 since e^(-x^4) at |x|=6).
    z1_num = mpm.quad(lambda x: mpm.e**(-(x**4)), [-mpm.inf, mpm.inf]) / (2 * mpm.pi)
    z1_ref = mpm.gamma(mpm.mpf(1) / 4) / (4 * mpm.pi)
    suite.assert_true("C2c real-line Z_1 quadrature = Gamma(1/4)/(4pi)",
                      mpm.fabs(z1_num - z1_ref) < TOL_1D, tag="[EXTERNAL]")

    with mpm.workdps(30):
        w = lambda x, y: (x - y)**2 * mpm.e**(-(x**4) - (y**4))
        z2_num = mpm.quad(lambda x: mpm.quad(lambda y: w(x, y), [-6, 6]), [-6, 6])
        z2_num = z2_num / (2 * (2 * mpm.pi)**2)
        z2_ref = mpm.gamma(mpm.mpf(1) / 4) * mpm.gamma(mpm.mpf(3) / 4) / (16 * mpm.pi**2)
        ok = mpm.fabs(z2_num - z2_ref) < TOL_2D
    suite.assert_true("C2c real-line Z_2 quadrature = Gamma(1/4)Gamma(3/4)/(16pi^2)",
                      bool(ok), tag="[EXTERNAL]")


# =============================================================================
# C3 — sector grading, r=4, N<=12 (symbolic, exact): the three Gamma-classes.
# =============================================================================

def check_c3() -> None:
    for N in range(1, 13):
        for a in (1, 2, 3):
            if N % 4 not in (0, a):
                continue
            amp = amplitude(4, a, N)
            if a == 1:
                q = gamma_class_ratio(amp, G14, N)
                label = "Q*Gamma(1/4)^N"
            elif a == 3:
                q = gamma_class_ratio(amp, G34, N)
                label = "Q*Gamma(3/4)^N"
            else:
                q = gamma_class_ratio(amp, sp.pi, N // 2)
                label = "Q*pi^(N/2)"
            suite.assert_true(f"C3 grading (4,{a}) N={N}: amplitude in {label}",
                              is_rational_expr(q), tag="[EXTERNAL]")


# =============================================================================
# C4 — ratio/product dichotomy + sqrt(2)*pi disambiguation.
# =============================================================================

def check_c4() -> None:
    Gs = G14 / G34  # G* symbolically

    # Amplitude ratio at N=1 (a=3 partition function itself vanishes at N=1 by
    # the support rule — this is the Gamma-AMPLITUDE ratio, N-qualified).
    r1 = sp.cancel(sp.simplify(amplitude(4, 1, 1) / amplitude(4, 3, 1)))
    suite.assert_true("C4 amplitude ratio (4,1)/(4,3) at N=1 = G*",
                      sp.simplify(r1 - Gs) == 0, tag="[EXTERNAL]")

    # Full partition-function ratios (delta signs included) where both phases
    # are supported: N=4 -> G*^4/48; N=8 -> 125*G*^8/435456.
    for N, want in ((4, Gs**4 / 48), (8, sp.Rational(125, 435456) * Gs**8)):
        num = delta_sign(4, 1, N) * amplitude(4, 1, N)
        den = delta_sign(4, 3, N) * amplitude(4, 3, N)
        ratio = sp.cancel(sp.simplify(num / den))
        suite.assert_true(f"C4 full Z ratio (4,1)/(4,3) at N={N} = Q*G*^{N}",
                          sp.simplify(ratio - want) == 0, tag="[EXTERNAL]")

    # General-r N=1 conjugate-sector amplitude ratio realizes the race-constant
    # family R_r = Gamma(1/r)/Gamma(1-1/r) of MATH_FAMILY_OF_RACES.md (R_4 = G*):
    for r in range(3, 9):
        ratio_r = sp.cancel(sp.simplify(amplitude(r, 1, 1) / amplitude(r, r - 1, 1)))
        want_r = sp.gamma(sp.Rational(1, r)) / sp.gamma(1 - sp.Rational(1, r))
        suite.assert_true(
            f"C4 N=1 sector ratio of Tr X^{r} model = race constant R_{r}",
            sp.simplify(ratio_r - want_r) == 0, tag="[EXTERNAL]")

    # Amplitude product at N=1 = Gamma(1/4)*Gamma(3/4) = sqrt(2)*pi
    # (Euler reflection at z=1/4). Disambiguation: sqrt(2)*pi ~ 4.443, NOT
    # sqrt(2*pi) ~ 2.507.
    prod = sp.simplify(amplitude(4, 1, 1) * amplitude(4, 3, 1))
    suite.assert_true("C4 amplitude product at N=1 = sqrt(2)*pi (reflection)",
                      sp.simplify(prod - sp.sqrt(2) * sp.pi) == 0, tag="[EXTERNAL]")
    p_num = mpm.gamma(mpm.mpf(1) / 4) * mpm.gamma(mpm.mpf(3) / 4)
    ok_dis = (mpm.fabs(p_num - mpm.sqrt(2) * mpm.pi) < TOL_ID
              and mpm.fabs(p_num - mpm.sqrt(2 * mpm.pi)) > mpm.mpf(1))
    suite.assert_true(
        "C4 disambiguation: product = sqrt(2)*pi (~4.443), != sqrt(2*pi) (~2.507)",
        bool(ok_dis), tag="[THEOREM]")


# =============================================================================
# C5 — normalized correlators in Q(G*).
# =============================================================================

def check_c5() -> None:
    Gs_num = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)

    # N=1 by quadrature: <x^2> = Gamma(3/4)/Gamma(1/4) = 1/G*.
    num = mpm.quad(lambda x: x**2 * mpm.e**(-(x**4)), [-mpm.inf, mpm.inf])
    den = mpm.quad(lambda x: mpm.e**(-(x**4)), [-mpm.inf, mpm.inf])
    suite.assert_true("C5 <Tr X^2> N=1 = 1/G* (quadrature, dps=100)",
                      mpm.fabs(num / den - 1 / Gs_num) < TOL_1D, tag="[EXTERNAL]")

    # N=2 by 2D quadrature at dps=30: <Tr X^2> = (G*^2+4)/(4G*).
    with mpm.workdps(30):
        w = lambda x, y: (x - y)**2 * mpm.e**(-(x**4) - (y**4))
        Z2 = mpm.quad(lambda x: mpm.quad(lambda y: w(x, y), [-6, 6]), [-6, 6])
        T2 = mpm.quad(lambda x: mpm.quad(lambda y: (x**2 + y**2) * w(x, y), [-6, 6]), [-6, 6])
        gs = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
        ok = mpm.fabs(T2 / Z2 - (gs**2 + 4) / (4 * gs)) < TOL_2D
    suite.assert_true("C5 <Tr X^2> N=2 = (G*^2+4)/(4G*) (quadrature, dps=30)",
                      bool(ok), tag="[EXTERNAL]")

    # N=3 by the symbolic determinant route (real line = weight e^{-x^4} on R):
    # real-line moments m_q = Gamma((q+1)/4) for q even, 0 for q odd.
    def m_rl(q: int) -> sp.Expr:
        return sp.gamma(sp.Rational(q + 1, 4)) if q % 2 == 0 else sp.Integer(0)

    def Z_rl(N: int, p: int = 0) -> sp.Expr:
        if p == 0:
            return sp.Matrix(N, N, lambda i, j: m_rl(i + j)).det()
        tot = sp.Integer(0)
        for k in range(N):
            tot += sp.Matrix(N, N, lambda i, j: m_rl(i + j + (p if i == k else 0))).det()
        return tot

    corr3 = sp.cancel(sp.simplify(Z_rl(3, 2) / Z_rl(3)))
    Gs = G14 / G34
    want3 = Gs * (Gs**2 + 4) / (4 * (Gs**2 - 4))
    suite.assert_true("C5 <Tr X^2> N=3 = G*(G*^2+4)/(4(G*^2-4)) (symbolic det route)",
                      sp.simplify(corr3 - want3) == 0, tag="[EXTERNAL]")
    # Membership in Q(G*) is a real symbolic check: reduce every shifted Gamma
    # (e.g. Gamma(5/4) from the moment determinant) to its fractional base via
    # the recurrence Gamma(f+n) = poch(f,n)*Gamma(f), rewrite in a fresh symbol
    # g via Gamma(3/4) -> Gamma(1/4)/g, and confirm the result is a rational
    # function of g alone (no Gamma, no pi survive).
    def reduce_gammas(e: sp.Expr) -> sp.Expr:
        for gm in e.atoms(sp.gamma):
            arg = gm.args[0]
            if arg.is_Rational and arg > 1:
                n = int(sp.floor(arg))
                frac = arg - n
                if frac != 0:
                    e = e.subs(gm, sp.rf(frac, n) * sp.gamma(frac))
        return sp.cancel(sp.simplify(e))

    # Two proven substitutions map any element of the ring generated by
    # {Gamma(1/4), Gamma(3/4), pi} into g = G*:
    #   (i)  reflection:  Gamma(3/4) = sqrt(2)*pi/Gamma(1/4)     [C8 step 1]
    #   (ii) definition:  Gamma(1/4)^2 = sqrt(2)*pi*G*           [C8 step 2]
    # A normalized correlator lies in Q(G*) iff pi cancels after both.
    g = sp.Symbol("g", positive=True)
    for N in (1, 2, 3):
        corr = reduce_gammas(Z_rl(N, 2) / Z_rl(N))
        in_g = corr.subs(G34, sp.sqrt(2) * sp.pi / G14)
        in_g = in_g.subs(G14, sp.sqrt(sp.sqrt(2) * sp.pi * g))
        in_g = sp.cancel(sp.simplify(in_g))
        ok = (in_g.free_symbols == {g}) and not in_g.atoms(sp.gamma) \
            and not in_g.has(sp.pi) and in_g.is_rational_function(g)
        suite.assert_true(f"C5 membership: <Tr X^2> N={N} is a rational function of G*",
                          bool(ok), tag="[EXTERNAL]")


# =============================================================================
# C6 — single-trace rationality on pure phases (CHPS eq 3.50).
# =============================================================================

def check_c6() -> None:
    # r<Tr X^r> = N^2 and r^2<Tr X^{2r}> = 2N^3 + a*(r-a)*N, via the moment-
    # determinant route on C_{r,a} (supported N only; normalized correlators).
    cases = [(4, 1, 1), (4, 1, 4), (4, 1, 5), (4, 3, 3), (4, 3, 4), (3, 1, 3), (3, 2, 2)]
    for r, a, N in cases:
        Z0 = Z_det(r, a, N)
        Z1 = Z_det(r, a, N, insert_power=r)
        got = sp.cancel(sp.simplify(Z1 / Z0)) * r
        suite.assert_true(f"C6 r<TrX^r> = N^2 at (r,a,N)=({r},{a},{N})",
                          sp.simplify(got - N**2) == 0, tag="[EXTERNAL]")
    for r, a, N in [(4, 1, 4), (4, 3, 4), (3, 1, 3)]:
        Z0 = Z_det(r, a, N)
        Z2 = Z_det(r, a, N, insert_power=2 * r)
        got = sp.cancel(sp.simplify(Z2 / Z0)) * r**2
        want = 2 * N**3 + a * (r - a) * N
        suite.assert_true(f"C6 r^2<TrX^2r> = 2N^3+a(r-a)N at (r,a,N)=({r},{a},{N})",
                          sp.simplify(got - want) == 0, tag="[EXTERNAL]")
    # Numeric spot check: N=1 real line, <x^4> = Gamma(5/4)/Gamma(1/4) = 1/4.
    num = mpm.quad(lambda x: x**4 * mpm.e**(-(x**4)), [-mpm.inf, mpm.inf])
    den = mpm.quad(lambda x: mpm.e**(-(x**4)), [-mpm.inf, mpm.inf])
    suite.assert_true("C6 numeric spot: <x^4> N=1 = 1/4",
                      mpm.fabs(num / den - mpm.mpf(1) / 4) < TOL_1D, tag="[EXTERNAL]")


# =============================================================================
# C7 — r=3 grading: the equianharmonic world (d=-3), disjoint from G* (d=-4).
# =============================================================================

def check_c7() -> None:
    for N in range(1, 10):
        for a in (1, 2):
            if N % 3 not in (0, a):
                continue
            amp = amplitude(3, a, N)
            base = G13 if a == 1 else G23
            q = gamma_class_ratio(amp, base, N)
            suite.assert_true(
                f"C7 grading (3,{a}) N={N}: amplitude in Q*Gamma({a}/3)^N",
                is_rational_expr(q), tag="[EXTERNAL]")
    # The r=3 transcendental is Gamma(1/3)/Gamma(2/3) — NOT G* (they differ by
    # ~0.98: 1.9781... vs 2.9587...); the two CM worlds stay separate.  Its
    # reflection product is 2*pi/sqrt(3) (z=1/3), not sqrt(2)*pi (z=1/4).
    g3 = mpm.gamma(mpm.mpf(1) / 3) / mpm.gamma(mpm.mpf(2) / 3)
    gs = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
    suite.assert_true("C7 equianharmonic ratio != G* (d=-3 vs d=-4 stay separate)",
                      mpm.fabs(g3 - gs) > mpm.mpf("0.5"), tag="[THEOREM]")
    prod3 = sp.simplify(sp.gammasimp(G13 * G23) - 2 * sp.pi / sp.sqrt(3))
    suite.assert_true("C7 reflection product at z=1/3 = 2*pi/sqrt(3)",
                      prod3 == 0, tag="[THEOREM]")


# =============================================================================
# C8 — FTD spine tie-ins (already theorem-grade in the spine; pinned here
# because the doc cites them next to the ensemble statements).
# =============================================================================

def check_c8() -> None:
    # Step 1: the Euler reflection at z=1/4 itself (gammasimp proves it):
    refl = sp.simplify(sp.gammasimp(G14 * G34) - sp.sqrt(2) * sp.pi)
    suite.assert_true("C8 reflection: Gamma(1/4)*Gamma(3/4) = sqrt(2)*pi",
                      refl == 0, tag="[THEOREM]")
    # Step 2: given reflection, substitute Gamma(3/4) = sqrt(2)*pi/Gamma(1/4).
    refl_sub = {G34: sp.sqrt(2) * sp.pi / G14}
    ok1 = sp.simplify((G14 / G34 - G14**2 / (sp.pi * sp.sqrt(2))).subs(refl_sub)) == 0
    suite.assert_true("C8 G* = Gamma(1/4)/Gamma(3/4) = Gamma(1/4)^2/(pi*sqrt(2))",
                      bool(ok1), tag="[THEOREM]")
    Gs = G14 / G34
    ok2 = sp.simplify(
        (Gs**2 / (2 * sp.pi) - G14**4 / (4 * sp.pi**3)).subs(refl_sub)) == 0
    suite.assert_true("C8 Watson: G*^2/(2pi) = Gamma(1/4)^4/(4pi^3)",
                      bool(ok2), tag="[THEOREM]")
    # Convention pin vs common.py's float64 G_STAR (sanity, 1e-12 only):
    gs_num = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
    suite.assert_true("C8 numeric G* agrees with common.py G_STAR (float64 sanity)",
                      abs(float(gs_num) - G_STAR_FLOAT64) < 1e-12, tag="[THEOREM]")


# =============================================================================
# C9 — CHPS Theorem 6: Z_r projection refactorizes the Vandermonde.
#
#   P_{a_1..a_N} prod_{I>J}(x_I - x_J)
#     = delta_{a_1..a_N} prod_I x_I^{a_I mod r} prod_{mu} prod_{i>j in block mu}
#       (x^r_{I_{mu,i}} - x^r_{I_{mu,j}}),
# where P_a f(x) = (1/r) sum_j omega^{-ja} f(omega^j x) per variable, blocks
# collect the I with a_I = mu (mod r), and delta is the sign of the sigma
# permutation (0 if sigma is not a permutation).
# =============================================================================

def theorem6_lhs(r: int, avec: tuple[int, ...], xs: list[sp.Symbol]) -> sp.Expr:
    N = len(xs)
    # Radical form of the primitive r-th root of unity (auto-evaluates for
    # r in {3,4}); powers indexed mod r to avoid negative-power fractions.
    omega = sp.expand(sp.cos(2 * sp.pi / r) + sp.I * sp.sin(2 * sp.pi / r))
    om = [sp.expand(omega**k) for k in range(r)]
    vdm = sp.prod([xs[i] - xs[j] for i in range(N) for j in range(i)])
    expr = sp.expand(vdm)
    for idx, a in enumerate(avec):
        acc = sp.Integer(0)
        for j in range(r):
            acc += om[(-j * a) % r] * expr.subs(xs[idx], om[j % r] * xs[idx])
        expr = sp.expand(acc / r)
    return expr


def theorem6_rhs(r: int, avec: tuple[int, ...], xs: list[sp.Symbol]) -> sp.Expr:
    N = len(xs)
    blocks: dict[int, list[int]] = {}
    for idx, a in enumerate(avec):
        blocks.setdefault(a % r, []).append(idx)
    # sigma(I_{mu,i}) = 1 + r*(i-1) + mu  (1-indexed slots)
    sigma = {}
    for mu, members in blocks.items():
        for i, idx in enumerate(members, start=1):
            sigma[idx + 1] = 1 + r * (i - 1) + mu
    if sorted(sigma.values()) != list(range(1, N + 1)):
        return sp.Integer(0)
    perm = [sigma[i] for i in range(1, N + 1)]
    # parity of the permutation (count inversions)
    inv = sum(1 for i in range(N) for j in range(i + 1, N) if perm[i] > perm[j])
    sign = -1 if inv % 2 else 1
    out = sp.Integer(sign)
    for idx, a in enumerate(avec):
        out *= xs[idx]**(a % r)
    for mu, members in blocks.items():
        for i in range(len(members)):
            for j in range(i):
                out *= xs[members[i]]**r - xs[members[j]]**r
    return sp.expand(out)


def check_c9() -> None:
    cases = [
        (3, (0, 1, 2)),          # balanced -> delta = +/-1
        (3, (2, 1, 0)),          # permuted labels
        (3, (0, 1, 1)),          # unbalanced -> delta = 0 (projection vanishes)
        (4, (0, 1)),             # N < r, distinct residues
        (4, (0, 1, 2, 3)),       # balanced full block
        (4, (1, 1, 2, 3)),       # unbalanced -> 0
    ]
    for r, avec in cases:
        xs = sp.symbols(f"x0:{len(avec)}")
        lhs = theorem6_lhs(r, avec, list(xs))
        rhs = theorem6_rhs(r, avec, list(xs))
        diff = sp.expand(lhs - rhs)
        if diff == 0:
            ok = True
        else:
            # check every polynomial coefficient (a sum of roots of unity)
            # simplifies to zero
            poly = sp.Poly(diff, *xs)
            ok = all(sp.simplify(c) == 0 for c in poly.coeffs())
        suite.assert_true(f"C9 Theorem 6 projection r={r} a={avec}",
                          bool(ok), tag="[EXTERNAL]")


# =============================================================================
# C10 — support rule + FULL Theorem 1 (det = delta * amplitude, sign included)
# + floor-hardening unit checks.
# =============================================================================

def check_c10() -> None:
    # Floor hardening: sympy floor is genuine floor on negative rationals
    # (int() truncates toward zero and would silently corrupt Theorem 1 at i < a).
    floor_ok = (sp.floor(sp.Rational(-3, 4)) == -1
                and sp.floor(sp.Rational(-1, 4)) == -1
                and int(-3 / 4) == 0)  # the trap this check guards against
    suite.assert_true("C10 floor hardening: floor(-3/4) = -1 (int() would give 0)",
                      bool(floor_ok), tag="[THEOREM]")

    for r in (3, 4):
        for a in range(1, r):
            for N in range(1, 7):
                det_val = Z_det(r, a, N)
                thm_val = delta_sign(r, a, N) * amplitude(r, a, N)
                supported = N % r in (0, a)
                if not supported:
                    ok = sp.simplify(det_val) == 0
                    suite.assert_true(
                        f"C10 support: det vanishes off-support (r,a,N)=({r},{a},{N})",
                        bool(ok), tag="[EXTERNAL]")
                else:
                    diff = sp.simplify(det_val - thm_val)
                    suite.assert_true(
                        f"C10 Theorem 1 FULL (det=delta*amp) (r,a,N)=({r},{a},{N})",
                        diff == 0, tag="[EXTERNAL]")


# =============================================================================
# C11 — RQ-MM-1: reflection positivity as a parity discriminator (CHPS §2.3).
#
# (a) Odd r: the (r-1)-th moment vanishes in EVERY pure phase — from the CHPS
#     moment formula, delta_{r | (r-1)+1-a} = delta_{r | r-a} = 0 for all
#     a in {1..r-1} — equivalently x^{r-1} e^{-x^r} = -(1/r) d/dx e^{-x^r} is
#     an exact form, so it integrates to zero on every closed contour.  Since
#     r-1 is EVEN for odd r, O = x^{(r-1)/2} is a nonzero operator with
#     <O^dag O> = 0: reflection positivity fails on every admissible contour
#     at N=1.  (CHPS prove the lambda-deformed version; N>1 remains their
#     "expected".)
# (b) The lambda-deformed zero-norm operator (CHPS eq 2.23) for the cubic:
#     O = x e^{x^2/4}  =>  <O^dag O> integrand = x^2 e^{-lambda x^3}, exact.
# (c) The cubic N=1 Gram bound: with moments determined by the loop equations
#     up to the single unknown m1, the Gram matrix of {1, x, x^2} admits a
#     PSD completion iff |lambda| < 2^(-1/2) 3^(-7/4) (CHPS §2.3).  Verified
#     by bisection on the semidefinite feasibility boundary.
# (d) Even r: the real-line measure e^{-x^r} dx is positive, so Hankel moment
#     matrices are positive-definite (manifest RP) — checked for r = 4, 6
#     at sizes up to 7x7.  Minimality, not uniqueness: EVERY even r is RP on
#     the real line; r=4 is only the smallest interacting case.
# (e) Even r on non-real contours: RP still fails (e.g. x has zero norm on
#     C_{4,1} since the second moment vanishes there).
# =============================================================================

def check_c11() -> None:
    lam, x = sp.symbols("lambda_ x", positive=False)

    # (a) odd-r zero-norm operator on every pure phase, symbolic.
    for r in (3, 5, 7):
        all_vanish = all(moment(r, a, r - 1) == 0 for a in range(1, r))
        suite.assert_true(
            f"C11a odd r={r}: (r-1)-moment = 0 in every pure phase "
            f"=> x^{(r - 1) // 2} has zero norm", all_vanish, tag="[EXTERNAL]")
        exact = sp.simplify(x**(r - 1) * sp.exp(-x**r)
                            + sp.diff(sp.exp(-x**r), x) / r)
        suite.assert_true(
            f"C11a odd r={r}: x^(r-1) e^(-x^r) is an exact form", exact == 0,
            tag="[THEOREM]")

    # (b) CHPS eq 2.23 integrand is exact for the lambda-cubic.
    integrand = x**2 * sp.exp(x**2 / 2) * sp.exp(-x**2 / 2 - lam * x**3)
    exact_b = sp.simplify(integrand + sp.diff(sp.exp(-lam * x**3), x) / (3 * lam))
    suite.assert_true("C11b lambda-cubic zero-norm integrand (2.23) is exact",
                      exact_b == 0, tag="[EXTERNAL]")

    # (c) Gram bound |lambda| < 2^(-1/2) 3^(-7/4) for the cubic at N=1.
    # Loop equations for W = x^2/2 + lambda x^3 (m0 = 1):
    #   m_{k+1} + 3 lambda m_{k+2} = k m_{k-1}
    # => m2 = -m1/(3L); m3 = (1 - m2)/(3L); m4 = (2 m1 - m3)/(3L).
    import numpy as np

    def min_eig_best(L: float) -> float:
        def gram_eigmin(m1: float) -> float:
            m2 = -m1 / (3 * L)
            m3 = (1 - m2) / (3 * L)
            m4 = (2 * m1 - m3) / (3 * L)
            G = np.array([[1, m1, m2], [m1, m2, m3], [m2, m3, m4]])
            return float(np.linalg.eigvalsh(G)[0])
        # coarse-to-fine 1D maximization over the single free parameter m1
        best, grid = -np.inf, np.linspace(-3, 3, 601)
        for _ in range(4):
            vals = [gram_eigmin(m) for m in grid]
            i = int(np.argmax(vals))
            best = vals[i]
            lo = grid[max(0, i - 1)]
            hi = grid[min(len(grid) - 1, i + 1)]
            grid = np.linspace(lo, hi, 601)
        return best

    lo, hi = 0.05, 0.2  # bracket: feasible at 0.05, infeasible at 0.2
    ok_bracket = min_eig_best(lo) > 0 > min_eig_best(hi)
    for _ in range(40):
        mid = 0.5 * (lo + hi)
        if min_eig_best(mid) > 0:
            lo = mid
        else:
            hi = mid
    lam_crit = 0.5 * (lo + hi)
    lam_chps = float(2 ** (-0.5) * 3 ** (-1.75))
    suite.assert_true(
        "C11c cubic N=1 Gram bound = 2^(-1/2) 3^(-7/4) (bisection, rel 1e-6)",
        ok_bracket and abs(lam_crit - lam_chps) / lam_chps < 1e-6,
        tag="[EXTERNAL]")

    # (d) even r: real-line Hankel positivity (manifest RP), r = 4 and 6.
    for r in (4, 6):
        def m_even(q: int, r: int = r) -> mpm.mpf:
            return mpm.gamma(mpm.mpf(q + 1) / r) if q % 2 == 0 else mpm.mpf(0)
        n = 7
        H = mpm.matrix(n, n)
        for i in range(n):
            for j in range(n):
                H[i, j] = m_even(i + j)
        eigs = mpm.eigsy(H, eigvals_only=True)
        suite.assert_true(
            f"C11d real-line r={r} Hankel 7x7 positive-definite (manifest RP)",
            all(e > 0 for e in eigs), tag="[EXTERNAL]")

    # (e) even r, non-real contour: x has zero norm on C_{4,1}.
    suite.assert_true("C11e second moment vanishes on C_{4,1} => x zero-norm "
                      "(pure phases are not RP)", moment(4, 1, 2) == 0,
                      tag="[EXTERNAL]")


# =============================================================================
# Main
# =============================================================================

def main() -> int:
    t0 = time.time()
    print("=" * 70)
    print("  FTD-0366 - G* in strongly-coupled matrix models (CHPS 2018)")
    print("  CHPS = Cordova-Heidenreich-Popolitov-Shakirov,")
    print("  Commun. Math. Phys. 361 (2018) 1235-1274, arXiv:1611.03142v1")
    print("=" * 70)

    check_c1()
    check_c2()
    check_c3()
    check_c4()
    check_c5()
    check_c6()
    check_c7()
    check_c8()
    check_c9()
    check_c10()
    check_c11()

    suite.print_summary()

    print(f"\n  Wall time: {time.time() - t0:.1f}s")
    print("\n  STANDING INVARIANTS (unchanged by every outcome above):")
    print("  - No alpha derived; x+ = 1/alpha stays [SMC] (FTD-0013).")
    print("  - MC-T4.3 stays [FOUNDATIONAL OBSTRUCTION].")
    print("  - The master quadratic is NOT produced by the matrix model;")
    print("    sector-ratio 'constructions' of 16G*^2/16G*^3 would be")
    print("    substitution identities and are prohibited.")
    print("  - External theorems verified here are CHPS 2018 (attribution),")
    print("    not FTD results; no FTD tag is promoted by this script.")
    return 0 if suite.all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
