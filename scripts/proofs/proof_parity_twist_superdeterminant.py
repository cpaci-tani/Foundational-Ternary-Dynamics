"""proof_parity_twist_superdeterminant.py — FTD-0381 / the parity twist as a
superdeterminant structure.

Question (locked in EXPLR_PARITY_TWIST_SUPERDETERMINANT.md BEFORE evaluation):
    Does the CHPS r=4 monomial matrix model carry a NATIVE Z/2-graded (super)
    structure under which the chi_{-4}-even and chi_{-4}-odd combinations of
    the conjugate sectors (a=1, a=3) arise as the determinant and Berezinian
    (superdeterminant) of a single model-native object — upgrading the
    FTD-0127/0366 "parity twist" (product -> sqrt(2)*pi, ratio -> G*) from
    bookkeeping to structure?

Naturality criteria (N1-N3, locked; the verdict lives ONLY here):
    N1  The grading character is model-native: epsilon(x^q) = chi_{-4}(q+1),
        the unique nontrivial Dirichlet character of (Z/4)^x, already the
        sector-label character of CHPS/FTD-0366; conjugate sectors {1,3} are
        its +/- eigen-labels.
    N2  The graded object is model-native: the SAME Andreief moment matrix
        (same weight e^{-x^4}, same construction) evaluated at the two points
        of the (Z/4)^x orbit, direct-summed. Residual freedom must be exactly
        one orientation bit (which sector is even), declared, nothing more.
    N3  An odd operator exists natively: some observable already in the model
        maps the sectors to each other, anticommutes with epsilon, and has a
        meaningful square. Candidate: Q = multiplication by x^2 (the model's
        quadratic observable), with Q^2 = x^4-insertion = the action density,
        acting on moments as the Euler/Gamma-recurrence operator.

Anti-targets (locked):
    - Manufacturability: ANY pair (A, B) is det/Ber of diag(A, B); the content
      is naturality (N1-N3) ONLY. Demonstrated explicitly in P12.
    - No fermion claim: FTD-0379/0380 closed native fermion emergence at the
      tested protocols; nothing here reopens that. A superdeterminant is
      Grassmann-free bookkeeping.
    - No alpha, no x_+, no master-quadratic content; the orientation bit is
      NOT identified with FC-W's delta bit (shape-consonance only).
    - No numerical search; every check is a pre-stated exact identity.

External input: CHPS 2018 Theorem 1 / eq 3.14 / eq 3.21 closed forms, as
already machine-verified at 100 dps by proof_gstar_matrix_models.py (FTD-0366,
155/155) — imported here as formulas, not re-derived.

Usage:
    python scripts/proofs/proof_parity_twist_superdeterminant.py
"""

from __future__ import annotations

import os
import sys

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")

import mpmath as mpm
import sympy as sp

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import ProofSuite  # noqa: E402

mpm.mp.dps = 60

G14 = sp.gamma(sp.Rational(1, 4))
G34 = sp.gamma(sp.Rational(3, 4))
GSTAR = G14 / G34
SQRT2PI = sp.sqrt(2) * sp.pi   # Gamma(1/4)*Gamma(3/4) = pi/sin(pi/4)

suite = ProofSuite("Parity twist as superdeterminant (FTD-0381)")


# ---------------------------------------------------------------- CHPS input
def moment(r: int, a: int, q: int) -> sp.Expr:
    """CHPS eq 3.14 (verified by FTD-0366 C1): sector-a moment of x^q."""
    if (q + 1 - a) % r != 0:
        return sp.Integer(0)
    return sp.gamma(sp.Rational(q + 1, r))


def amplitude(r: int, a: int, N: int) -> sp.Expr:
    """CHPS Theorem 1 Gamma-amplitude (verified by FTD-0366 C2/C10)."""
    p = sp.Integer(1)
    for i in range(N):
        p *= sp.gamma(sp.floor(sp.Rational(i, r)) + 1)
        p *= sp.gamma(sp.floor(sp.Rational(i - a, r)) + sp.Rational(a, r) + 1)
    return sp.gammasimp(p)


def delta_sign(r: int, a: int, N: int) -> int:
    """CHPS eq 3.21 support sign (verified by FTD-0366 C10)."""
    if N % r not in (0, a % r):
        return 0
    at = r - a
    exponent = (-(-N // r)) * (a * (a - 1) // 2) + (N // r) * (at * (at - 1) // 2)
    return -1 if exponent % 2 else 1


def is_rational(expr: sp.Expr) -> bool:
    s = sp.gammasimp(sp.simplify(expr))
    return s.is_rational is True


def chi_m4(n: int) -> int:
    """Dirichlet character mod 4: chi(1)=+1, chi(3)=-1, chi(even)=0."""
    n %= 4
    return {0: 0, 1: 1, 2: 0, 3: -1}[n]


# ============================================================ P1: selection
ok = True
for a in (1, 3):
    for q in range(0, 16):
        nz = moment(4, a, q) != 0
        ok &= (nz == ((q % 4) == (a - 1) % 4))
suite.assert_true("P1  sector selection: moment(4,a,q) != 0 iff q = a-1 (mod 4)", ok)

# ================================================== P2: sector Gamma-classes
ok = True
for k in range(0, 7):
    ok &= is_rational(moment(4, 1, 4 * k) / G14)        # q = 0 mod 4
    ok &= is_rational(moment(4, 3, 4 * k + 2) / G34)    # q = 2 mod 4
suite.assert_true("P2  sector classes: a=1 moments in Q*Gamma(1/4); a=3 in Q*Gamma(3/4)", ok)

# ======================================= P3: reflection pairs the two classes
ok = True
for k in range(0, 7):
    for kp in range(0, 7):
        prod = moment(4, 1, 4 * k) * moment(4, 3, 4 * kp + 2)
        ok &= is_rational(prod / SQRT2PI)
suite.assert_true("P3  cross-sector products in Q*sqrt(2)*pi (reflection at z=1/4)", ok)

# ============================================= P4 (N1): the grading character
# epsilon(x^q) = chi_{-4}(q+1) reproduces the sector pairing:
#   q = 0 mod 4  ->  +1  = chi_{-4}(a=1)   (sector 1 = even)
#   q = 2 mod 4  ->  -1  = chi_{-4}(a=3)   (sector 3 = odd)
#   q odd        ->   0  (non-graded: these pair with the a=2 / a=4 sectors)
ok = True
for q in range(0, 16):
    eps = chi_m4(q + 1)
    if q % 4 == 0:
        ok &= (eps == +1 == chi_m4(1))
    elif q % 4 == 2:
        ok &= (eps == -1 == chi_m4(3))
    else:
        ok &= (eps == 0)
suite.assert_true("P4  N1: grading operator = chi_{-4}(q+1); sectors {1,3} are its +/- labels", ok)

# =========================== P5/P6: amplitude-level det and Berezinian classes
ok_det, ok_ber = True, True
for N in range(1, 9):
    A1 = amplitude(4, 1, N)
    A3 = amplitude(4, 3, N)
    ok_det &= is_rational(A1 * A3 / SQRT2PI**N)
    ok_ber &= is_rational((A1 / A3) / GSTAR**N)
# N=1 exact anchor values
A1_, A3_ = amplitude(4, 1, 1), amplitude(4, 3, 1)
ok_det &= sp.gammasimp(A1_ * A3_ - SQRT2PI) == 0
ok_ber &= sp.gammasimp(A1_ / A3_ - GSTAR) == 0
suite.assert_true("P5  AMPLITUDE product A1*A3 in Q*(sqrt(2)*pi)^N, N<=8 (identity level, NOT a matrix det)", ok_det)
suite.assert_true("P6  AMPLITUDE ratio  A1/A3 in Q*G*^N,           N<=8 (identity level, NOT a matrix Ber)", ok_ber)

# ================== P6b: the ACTUAL Hankel determinants (redteam finding 1)
# The native graded object's det/Ber exist non-degenerately ONLY on the
# delta-support N = 0 mod 4; off support the sector-3 Hankel determinant
# vanishes and Ber is undefined.
def hankel(a: int, N: int) -> sp.Matrix:
    return sp.Matrix(N, N, lambda i, j: moment(4, a, i + j))

D1_4 = sp.gammasimp(hankel(1, 4).det())
D3_4 = sp.gammasimp(hankel(3, 4).det())
ok = (D1_4 != 0) and (D3_4 != 0)
ok &= is_rational(D1_4 * D3_4 / SQRT2PI**4)
ok &= sp.gammasimp((D1_4 / D3_4) / GSTAR**4 - sp.Rational(1, 48)) == 0
ok &= sp.gammasimp(hankel(3, 1).det()) == 0            # N=1: sector-3 block singular
ok &= sp.gammasimp(hankel(1, 2).det()) == 0            # N=2: off support for a=1 too
suite.assert_true("P6b MATRIX level: det/Ber of the Hankel pair exist at N=4 "
                  "(Ber = G*^4/48 exactly); singular off-support (N=1,2)", ok)

# ================================ P7: Z-level (delta signs) on common support
ok = True
for N in (4, 8):
    Z1 = delta_sign(4, 1, N) * amplitude(4, 1, N)
    Z3 = delta_sign(4, 3, N) * amplitude(4, 3, N)
    ok &= (Z1 != 0) and (Z3 != 0)
    ok &= is_rational(Z1 * Z3 / SQRT2PI**N)
    ok &= is_rational((Z1 / Z3) / GSTAR**N)
suite.assert_true("P7  Z-level det/Ber classes hold with delta signs at N in {4,8}", ok)

# ====================================== P8: common support = N in 4Z exactly
ok = True
for N in range(1, 17):
    both = (delta_sign(4, 1, N) != 0) and (delta_sign(4, 3, N) != 0)
    ok &= (both == (N % 4 == 0))
suite.assert_true("P8  Z-level superstructure supported exactly on N = 0 (mod 4)", ok)

# ============================== P9 (N3): Q = x^2 anticommutes with epsilon
# Matrices on the even-monomial space W = span{x^q : q even, q <= K}.
K = 20
qs = list(range(0, K + 1, 2))
idx = {q: i for i, q in enumerate(qs)}
n = len(qs)
E = sp.zeros(n, n)
Q = sp.zeros(n, n)
for q in qs:
    E[idx[q], idx[q]] = chi_m4(q + 1)
    if q + 2 <= K:
        Q[idx[q + 2], idx[q]] = 1          # multiplication by x^2
AC = E * Q + Q * E
ok = True
for q in qs:
    if q + 2 <= K:                          # interior columns (truncation edge excluded)
        for i in range(n):
            ok &= (AC[i, idx[q]] == 0)
suite.assert_true("P9  N3a: {epsilon, Q} = 0 — x^2-multiplication is an ODD operator", ok)

# ==================== P10 (N3): Q^2 = action insertion = Euler operator on moments
# x^4 * x^q = x^{q+4}; on sector moments: Gamma recurrence
#   moment(4, a, q+4) = ((q+1)/4) * moment(4, a, q).
ok = True
for a in (1, 3):
    for q in range(0, 21):
        m0 = moment(4, a, q)
        if m0 == 0:
            continue
        ok &= sp.gammasimp(moment(4, a, q + 4) - sp.Rational(q + 1, 4) * m0) == 0
suite.assert_true("P10 N3b: Q^2 = x^4-insertion acts on moments as the Euler/Gamma recurrence (q+1)/4", ok)

# =============================== P11: orientation bit — Ber inverts on swap
ok = True
for N in (1, 4):
    A1 = amplitude(4, 1, N)
    A3 = amplitude(4, 3, N)
    ok &= sp.gammasimp((A1 / A3) * (A3 / A1) - 1) == 0
    ok &= is_rational((A3 / A1) / GSTAR**(-N))
suite.assert_true("P11 orientation bit: block swap sends Ber -> Ber^{-1} (G*^N -> G*^{-N})", ok)

# ======================= P12: manufacturability guard (content = naturality)
A, B = sp.symbols("A B", positive=True)
M = sp.diag(A, B)
ok = (sp.simplify(M.det() - A * B) == 0) and (sp.simplify((A / B) - M[0, 0] / M[1, 1]) == 0)
suite.assert_true("P12 guard: ANY pair is det/Ber of a diagonal object — content is N1-N3 only", ok)

# ============================================= P13: numeric G* cross-check
g_num = mpm.gamma(mpm.mpf(1) / 4) / mpm.gamma(mpm.mpf(3) / 4)
g_sym = mpm.mpf(sp.N(GSTAR, 50).__str__())
suite.assert_true("P13 numeric cross-check: G* = Gamma(1/4)/Gamma(3/4) at 50 dps",
            abs(g_num - g_sym) < mpm.mpf(10) ** (-45))

suite.print_summary()
sys.exit(0 if suite.all_pass else 1)

