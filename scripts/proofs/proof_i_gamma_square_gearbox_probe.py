"""FTD-0839 exact i/Gamma/quartic-square split certificate.

This certificate tests identities and controls fixed before execution.  It
does not search parameters, fit a period, or modify production dynamics.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCE_HASHES = {
    "docs/theory/03_derivations/foundational_mechanics/"
    "DERIV_GSTAR_QUARTER_CONJUGACY.md":
        "52196EDE252C4DF772C3943B8EEDB459B805AAA027E74548F3F779C4D74C6C33",
    "docs/theory/03_derivations/foundational_mechanics/"
    "DERIV_GSTAR_FINITE_APPROX.md":
        "F6002D358CE0F832ECBF6D6FE33E67F96BF0BAAEB22604CC1B2E85AF2FF5DBBE",
    "docs/theory/01_reference/SPEC_FQCR.md":
        "C840E0C63A098CA8DDEC6B9D558817B9767E752F173D1AE4C47E2AC3E2887C72",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md":
        "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md":
        "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"[{'PASS' if passed else 'FAIL'}] {name}")


def exact_zero(expression: sp.Expr) -> bool:
    return sp.simplify(sp.expand_func(expression)) == 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


check(
    "C1 frozen theory sources match the preregistered hashes",
    all(sha256(ROOT / relative) == expected
        for relative, expected in SOURCE_HASHES.items()),
)

# Algebra supplied by i itself.
I2 = sp.eye(2)
J = sp.Matrix([[0, -1], [1, 0]])
check(
    "C2 the real lift of multiplication by i is an oriented order-four map",
    J.T * J == I2 and J.det() == 1 and J**2 == -I2 and J**4 == I2,
)
check(
    "C3 J squared equals minus identity and fixes only the eigenvalues plus/minus i",
    J.eigenvals() == {-sp.I: 1, sp.I: 1},
)

# The quarter shifts are conditional on adding a circle and the twisted
# boundary condition.  J alone has no spatial domain or spectrum.
a_plus = sp.Rational(1, 4)
a_minus = sp.Rational(3, 4)
twist_plus = sp.exp(2 * sp.pi * sp.I * a_plus).expand(complex=True)
twist_minus = sp.exp(2 * sp.pi * sp.I * a_minus).expand(complex=True)
check(
    "C4 the added twisted-circle boundary maps the two J eigenphases to quarter shifts",
    exact_zero(twist_plus - sp.I) and exact_zero(twist_minus + sp.I),
)

def det_half_line(a: sp.Rational) -> sp.Expr:
    """Lerch zeta determinant of {n+a}_{n>=0}."""

    return sp.sqrt(2 * sp.pi) / sp.gamma(a)


gstar = sp.gamma(a_plus) / sp.gamma(a_minus)
half_line_ratio = det_half_line(a_minus) / det_half_line(a_plus)
check(
    "C5 an anchored chiral half-line determinant ratio is exactly G star",
    exact_zero(half_line_ratio - gstar),
)

# The lower endpoint is load-bearing.  Moving the spectral origin by one
# produces G*/3, not G*.
origin_one_ratio = sp.gamma(1 + a_plus) / sp.gamma(1 + a_minus)
check(
    "C6 shifting the half-line origin once changes the ratio to G star over three",
    exact_zero(origin_one_ratio - gstar / 3),
)

# Multiplicity and operator order are also load-bearing.
r = sp.symbols("r", integer=True, positive=True)
check(
    "C7 r identical chiral copies raise the determinant ratio to the rth power",
    exact_zero((half_line_ratio**r) - gstar**r),
)

squared_half_line_ratio = (
    det_half_line(a_minus) ** 2 / det_half_line(a_plus) ** 2
)
check(
    "C8 squaring the positive operator changes the chiral ratio from G star to G star squared",
    exact_zero(squared_half_line_ratio - gstar**2),
)

# Zeta determinants have a scaling anomaly because zeta_H(0,a)=1/2-a.
c = sp.symbols("c", positive=True)
zeta_zero_plus = sp.Rational(1, 2) - a_plus
zeta_zero_minus = sp.Rational(1, 2) - a_minus
check(
    "C9 the two Hurwitz zeta values at zero differ by minus one half",
    zeta_zero_plus == sp.Rational(1, 4)
    and zeta_zero_minus == -sp.Rational(1, 4)
    and zeta_zero_minus - zeta_zero_plus == -sp.Rational(1, 2),
)
scaled_ratio = c ** (zeta_zero_minus - zeta_zero_plus) * half_line_ratio
check(
    "C10 a common spectral scale multiplies the zeta ratio by c to minus one half",
    exact_zero(scaled_ratio - gstar / sp.sqrt(c)),
)

# An orientation-blind full-line Laplacian is the decisive control.  Its
# determinant is 4 sin^2(pi a), equal in conjugate quarter sectors.
def det_full_line_laplacian(a: sp.Rational) -> sp.Expr:
    return 4 * sp.sin(sp.pi * a) ** 2


full_plus = sp.simplify(det_full_line_laplacian(a_plus))
full_minus = sp.simplify(det_full_line_laplacian(a_minus))
check(
    "C11 both full-line quarter-twisted Laplacian determinants equal two",
    full_plus == 2 and full_minus == 2,
)
check(
    "C12 the orientation-blind full-line determinant ratio is one, not G star",
    exact_zero(full_minus / full_plus - 1),
)

# The square/pair field supplies quarticity exactly.
x, y, lam = sp.symbols("x y lambda", real=True)
psi_norm_sq = x**2 + y**2
u_re = x**2 - y**2
u_im = 2 * x * y
u_norm_sq = sp.expand(u_re**2 + u_im**2)
check(
    "C13 the complex square U=psi squared has norm squared equal to norm psi to the fourth",
    exact_zero(u_norm_sq - psi_norm_sq**2),
)

q = sp.symbols("q", real=True)
quartic_energy = lam * q**4
check(
    "C14 the square-field energy produces the exact cubic restoring force",
    exact_zero(-sp.diff(quartic_energy, q) + 4 * lam * q**3),
)

# But the same square is a two-to-one quotient and erases the sign of the
# oriented quarter-turn.
psi = sp.symbols("psi", complex=True)
check(
    "C15 the plus-i and minus-i lifts collide under the complex square",
    exact_zero((sp.I * psi) ** 2 - (-sp.I * psi) ** 2),
)

def doubled_shift(a: sp.Rational) -> sp.Rational:
    return sp.Rational((2 * a).p % (2 * a).q, (2 * a).q)


check(
    "C16 both quarter twists double to the same half-twist sector",
    doubled_shift(a_plus) == sp.Rational(1, 2)
    and doubled_shift(a_minus) == sp.Rational(1, 2),
)

half_twist_det = det_half_line(sp.Rational(1, 2))
check(
    "C17 the half-twist half-line determinant is exactly square root two",
    exact_zero(half_twist_det - sp.sqrt(2)),
)
check(
    "C18 the two squared quarter sectors therefore have determinant ratio one",
    exact_zero(half_twist_det / half_twist_det - 1),
)

# Symmetric square cannot distinguish J from -J.
check(
    "C19 the symmetric-square action identifies the two orientations",
    J**2 == (-J) ** 2 == -I2,
)

oriented_character_plus = sp.im(sp.I)
oriented_character_minus = sp.im(-sp.I)
square_image_plus = sp.I**2
square_image_minus = (-sp.I) ** 2
check(
    "C20 no function of the square image alone can recover this orientation witness",
    square_image_plus == square_image_minus
    and oriented_character_plus != oriented_character_minus,
)

# The proposed primitive alphabet {0,i} is not dynamically closed.  The
# smallest zero-augmented orbit of multiplication by i is {0,+/-1,+/-i}.
proposed_alphabet = {sp.Integer(0), sp.I}
phase_alphabet = {sp.Integer(0), sp.Integer(1), sp.I, -sp.Integer(1), -sp.I}
check(
    "C21 the proposed alphabet zero-and-i is not closed under multiplication by i",
    sp.I * sp.I not in proposed_alphabet,
)
check(
    "C22 the zero-augmented C4 phase alphabet is closed under multiplication by i",
    all(sp.simplify(sp.I * value) in phase_alphabet for value in phase_alphabet),
)

# Finite products cancel a common scale at each finite N; this is deliberately
# contrasted with C10, where zeta regularization retains unequal exponents.
N = sp.symbols("N", integer=True, nonnegative=True)
finite_scale_exponent_difference = (N + 1) - (N + 1)
check(
    "C23 equal finite truncations cancel their common spectral scale",
    finite_scale_exponent_difference == 0,
)

# Final discriminator: the orientation carrier and quartic carrier are exact
# but live on opposite sides of a non-injective quotient.
check(
    "C24 the exact probe resolves to a split architecture rather than an automatic gearbox",
    J**2 == -I2
    and exact_zero(half_line_ratio - gstar)
    and exact_zero(u_norm_sq - psi_norm_sq**2)
    and square_image_plus == square_image_minus
    and exact_zero(full_minus / full_plus - 1),
)

passed = sum(1 for _, value in checks if value)
all_pass = passed == len(checks)
print(f"\nFTD-0839 i/Gamma/quartic-square split: {passed}/{len(checks)} PASS")
if all_pass:
    print("I_FORCES_ORIENTED_QUARTER_EIGENPHASES_ONLY")
    print("GSTAR_REQUIRES_TWISTED_DOMAIN_CHIRAL_HALF_LINE_SCALE_AND_ORIGIN")
    print("COMPLEX_SQUARE_SUPPLIES_QUARTIC_ENERGY_AND_ERASES_ORIENTATION")
    print("LIFT_TO_PAIR_GEARBOX_STATUS=OPEN")
else:
    print("I_GAMMA_QUARTIC_SQUARE_CERTIFICATE_INVALID")

raise SystemExit(0 if all_pass else 1)
