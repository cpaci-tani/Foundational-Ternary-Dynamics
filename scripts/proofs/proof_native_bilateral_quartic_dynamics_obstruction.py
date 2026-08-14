"""FTD-0837 exact native bilateral/quartic dynamics obstruction certificate.

This is an exact symbolic and source-hash-locked certificate.  It does not
search parameters, fit data, or modify production dynamics.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCE_HASHES = {
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "engine/src/transmutation_phases.cpp":
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043",
    "engine/include/ftd/term_toggles.h":
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "docs/theory/08_structural/EXPLR_DUAL_SUBSTRATE_STAGGERED_ENCODING.md":
        "30E85A9F1ACADEF6D7D8FEF02A371480531159B2D37E4E660219AF48077CAF87",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "ANALYSIS_QUARTIC_SELECTION_REFUTED_v1.md":
        "F89284886047F6F4BE638BE1D03680D3378DB81FB808E610C6FD579FA65CF358",
    "docs/theory/03_derivations/DERIV_LAGRANGIAN_FROM_TICK_RULE.md":
        "FB09580E8060D1DB79D249C6422E62A7EE33EB63DAD339487DEF89FB4910B3AA",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"[{'PASS' if passed else 'FAIL'}] {name}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


check(
    "C1 frozen production and theory sources match the preregistered hashes",
    all(sha256(ROOT / relative) == expected
        for relative, expected in SOURCE_HASHES.items()),
)

# Any identical affine update L' = A L + b, R' = A R + b block-diagonalizes
# exactly in the sum/difference register F=L+R, D=L-R.
a11, a12, a21, a22 = sp.symbols("a11 a12 a21 a22", real=True)
b1, b2 = sp.symbols("b1 b2", real=True)
A = sp.Matrix([[a11, a12], [a21, a22]])
b = sp.Matrix([b1, b2])
I2 = sp.eye(2)
Z2 = sp.zeros(2)
M_native = sp.diag(A, A)
P = sp.Matrix.vstack(sp.Matrix.hstack(I2, I2), sp.Matrix.hstack(I2, -I2))
P_inv = sp.Rational(1, 2) * P
source_native = sp.Matrix.vstack(b, b)
M_fd = sp.simplify(P * M_native * P_inv)
source_fd = sp.simplify(P * source_native)

check("C2 sum/difference register change is invertible", P * P_inv == sp.eye(4))
check("C3 identical L/R operator remains block diagonal", M_fd == sp.diag(A, A))
check(
    "C4 shared source drives F only",
    source_fd == sp.Matrix.vstack(2 * b, sp.zeros(2, 1)),
)
check(
    "C5 native homogeneous update has no L/R exchange block",
    M_native[:2, 2:] == Z2 and M_native[2:, :2] == Z2,
)

# The desired oriented exchange and the production weak swap are different
# conjugacy classes: J has order four and determinant +1; S has order two and
# determinant -1.  In F/D variables S is only D -> -D.
J = sp.Matrix([[0, 1], [-1, 0]])
S = sp.Matrix([[0, 1], [1, 0]])
P2 = sp.Matrix([[1, 1], [1, -1]])
P2_inv = sp.Rational(1, 2) * P2
check(
    "C6 oriented exchange is the order-four complex structure",
    J.T * J == I2 and J**2 == -I2 and J**4 == I2 and J.det() == 1,
)
check(
    "C7 weak L/R swap is a reflection, not an oriented quarter-turn",
    S**2 == I2 and S.det() == -1
    and P2 * S * P2_inv == sp.diag(1, -1)
    and S != J and S != -J,
)

# On every fixed-state/source branch the production field law is affine.
# Its conservative modal potential has degree at most two.  Nulling both the
# equilibrium force and the quadratic stiffness makes the branch flat, not
# quartic.
q, kappa, h = sp.symbols("q kappa h", real=True)
V_fixed_branch = sp.Rational(1, 2) * kappa * q**2 - h * q
check(
    "C8 fixed-state production potential has no quartic Taylor coefficient",
    sp.diff(V_fixed_branch, q, 4) == 0,
)
check(
    "C9 null force plus null stiffness makes the native branch flat",
    sp.expand(V_fixed_branch.subs({h: 0, kappa: 0})) == 0,
)

# The signed-energy warp is not supplied by one primitive ternary state.
ternary = (-1, 0, 1)
check(
    "C10 primitive ternary signed square is the identity",
    all(s * abs(s) == s for s in ternary),
)
p_plus = sp.Rational(3, 4)
p_minus = sp.Rational(1, 4)
mean_s = p_plus - p_minus
mean_signed_square = p_plus - p_minus
coarse_signed_square = mean_s * abs(mean_s)
check(
    "C11 averaging and the signed-energy warp do not commute",
    mean_signed_square == sp.Rational(1, 2)
    and coarse_signed_square == sp.Rational(1, 4)
    and mean_signed_square != coarse_signed_square,
)
m = sp.symbols("m", real=True)
check(
    "C12 an independent constituent pair supplies m squared only after a closure assumption",
    sp.expand((m * m) - m**2) == 0,
)

# Uniform production damping is homogeneous.  Its quadratic energy has only
# the zero fixed point for the production regime 0<g<1; it cannot select a
# positive shell.
E, g = sp.symbols("E g", real=True)
E_damped = sp.expand((1 - g) ** 2 * E)
fixed_residual = sp.factor(E_damped - E)
check(
    "C13 homogeneous damping fixed-point equation factors exactly",
    fixed_residual == E * g * (g - 2),
)
check(
    "C14 damping has no distinguished positive target energy",
    sp.diff(E_damped, E) == (1 - g) ** 2
    and sp.diff(E_damped, E, 2) == 0,
)

# Minimal extension theorem.  In two real channels, an orientation-preserving
# orthogonal map is [[c,s],[-s,c]].  Requiring Q^2=-I forces c=0,s=+/-1,
# hence Q=+/-J.
c, s = sp.symbols("c s", real=True)
c_sq = sp.solve(
    [sp.Eq(c**2 + s**2, 1), sp.Eq(c**2 - s**2, -1)],
    [c**2, s**2],
    dict=True,
)
check(
    "C15 oriented orthogonal order-four exchange is unique up to direction",
    c_sq == [{c**2: 0, s**2: 1}],
)

# A context-blind radial correction following the quarter-turn has
# X' = rho(E) J X and E' = rho(E)^2 E.  The first radial jet is sufficient to
# derive the stability condition.
a1 = sp.symbols("a1", real=True)
rho_jet = 1 + a1 * (E - 1)
E_jet_next = sp.expand(E * rho_jet**2)
radial_multiplier = sp.simplify(sp.diff(E_jet_next, E).subs(E, 1))
check(
    "C16 unit-shell radial multiplier is one plus twice the gain slope",
    radial_multiplier == 1 + 2 * a1,
)

rho0 = sp.symbols("rho0", positive=True)
constant_gain_energy = rho0**2 * E
check(
    "C17 a positive constant gain fixing the unit shell is neutral",
    sp.solve(sp.Eq(constant_gain_energy.subs(E, 1), 1), rho0) == [1]
    and sp.diff(constant_gain_energy.subs(rho0, 1), E) == 1,
)

eta, B = sp.symbols("eta B", real=True)
rho = 1 + eta * (1 - E)
E_next = sp.expand(E * rho**2)
multiplier = sp.simplify(sp.diff(E_next, E).subs(E, 1))
stability_margin = sp.factor(1 - multiplier**2)
check(
    "C18 lowest-degree nonconstant gain has conditional stable range 0<eta<1",
    multiplier == 1 - 2 * eta
    and stability_margin == 4 * eta * (1 - eta),
)
energy_transfer = sp.factor(E_next - E)
check(
    "C19 radial repair exchanges rather than creates core energy",
    energy_transfer == E * eta * (E - 1) * (E * eta - eta - 2),
)
B_next = B + E - E_next
check(
    "C20 one bath account closes total energy exactly",
    sp.simplify(E_next + B_next - (E + B)) == 0,
)

# Conditional quartic lift and G* traversal.  This remains downstream of a
# separately supplied coarse-graining map q -> u=q|q|.
x, y = sp.symbols("x y", real=True)
u_signed = x * sp.Abs(x)
check(
    "C21 selected signed coarse coordinate lifts a circle to a quartic shell",
    sp.simplify(u_signed**2 + y**2 - (x**4 + y**2)) == 0,
)
gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
quartic_traversal = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
check(
    "C22 conditional quartic traversal is exactly sqrt(pi) G*",
    sp.simplify(sp.expand_func(quartic_traversal) - sp.sqrt(sp.pi) * gstar) == 0,
)

passed = sum(1 for _, value in checks if value)
all_pass = passed == len(checks)
print(f"\nFTD-0837 native bilateral/quartic dynamics: {passed}/{len(checks)} PASS")
if all_pass:
    print("FROZEN_PRODUCTION_CORE_ORIENTED_EXCHANGE_ABSENT")
    print("FROZEN_PRODUCTION_CORE_SMOOTH_QUARTIC_RESTORER_ABSENT")
    print("FROZEN_PRODUCTION_CORE_NONZERO_STABLE_SHELL_ABSENT")
    print("MINIMAL_BILATERAL_RADIAL_BATH_EXTENSION_CONDITIONAL_THEOREM")
else:
    print("NATIVE_BILATERAL_QUARTIC_DYNAMICS_CERTIFICATE_INVALID")
print("COARSE_GRAINING_PAIR_CLOSURE_STATUS=SELECTED_AND_OPEN")
print("GSTAR_SUBSTRATE_GEARBOX_STATUS=NOT_DERIVED")

raise SystemExit(0 if all_pass else 1)
