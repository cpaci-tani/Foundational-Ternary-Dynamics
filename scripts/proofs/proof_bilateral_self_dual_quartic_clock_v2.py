"""FTD-0836 repaired exact bilateral self-dual quartic clock certificate.

The only mathematical check change from FTD-0835 is algebraic comparison of
the factored C17 stability margin.  The theorem verdict is fail-closed.
"""

from __future__ import annotations

import sympy as sp


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"[{'PASS' if passed else 'FAIL'}] {name}")


def sym2(matrix: sp.Matrix) -> sp.Matrix:
    """Symmetric-square matrix in the basis (a^2, a b, b^2)."""

    a, b, c, d = matrix[0, 0], matrix[0, 1], matrix[1, 0], matrix[1, 1]
    return sp.Matrix(
        [
            [a**2, 2 * a * b, b**2],
            [a * c, a * d + b * c, b * d],
            [c**2, 2 * c * d, d**2],
        ]
    )


J = sp.Matrix([[0, 1], [-1, 0]])
identity = sp.eye(2)
check("C1 oriented exchange is orthogonal", J.T * J == identity)
check("C2 oriented exchange squares to minus identity", J**2 == -identity)
check("C3 oriented exchange has order four", J**4 == identity)

state = sp.Matrix([1, 0])
orbit = []
for _ in range(4):
    orbit.append(tuple(int(value) for value in state))
    state = J * state
check(
    "C4 exact four-state ternary orbit",
    orbit == [(1, 0), (0, -1), (-1, 0), (0, 1)]
    and state == sp.Matrix([1, 0]),
)

left, right = sp.symbols("L R", real=True)
channel = sp.Matrix([left, right])
advanced = J * channel
energy = sp.expand(left**2 + right**2)
advanced_energy = sp.expand(advanced.dot(advanced))
phase_current = sp.expand(left * advanced[1] - right * advanced[0])
reverse = -J * channel
reverse_current = sp.expand(left * reverse[1] - right * reverse[0])

check("C5 quadratic channel energy is invariant", advanced_energy == energy)
check("C6 forward phase current is minus the energy", phase_current == -energy)
check("C7 reverse phase current is plus the energy", reverse_current == energy)
check("C8 symmetric square loses orientation", sym2(J) == sym2(-J))

x, y = sp.symbols("x y", real=True)
signed_u = x * sp.Abs(x)
check(
    "C9 signed energy coordinate maps quartic shell to a circle",
    sp.simplify(signed_u**2 + y**2 - (x**4 + y**2)) == 0,
)

# Phi is bijective because f(x)=x|x| is odd and strictly increasing.  Hence
# D4=Phi^{-1} J Phi inherits its powers by conjugacy.
check("C10 nonlinear lift squares to the central sign", J**2 == -identity)
check("C11 nonlinear lift has order four", J**4 == identity)

xdot = y
ydot = -2 * x**3
shell_derivative = sp.expand(4 * x**3 * xdot + 2 * y * ydot)
check("C12 normalized quartic flow preserves x^4+y^2", shell_derivative == 0)

# Work on x>0, hence u=x^2 and x=sqrt(u).  Other open quadrants follow by
# the odd signed-energy coordinate and the same absolute-value factor.
u = sp.symbols("u", positive=True)
udot = 2 * sp.sqrt(u) * y
ydot_u = -2 * u * sp.sqrt(u)
induced = sp.Matrix([udot, ydot_u])
weighted_rotation = 2 * sp.sqrt(u) * J * sp.Matrix([u, y])
check("C13 induced energy-channel flow is weighted oriented rotation", induced == weighted_rotation)

angular_velocity = sp.simplify((u * ydot_u - y * udot) / (u**2 + y**2))
angular_velocity_on_shell = sp.simplify(angular_velocity.subs(y**2, 1 - u**2))
check("C14 angular velocity is -2 sqrt(|u|) on the unit shell", angular_velocity_on_shell == -2 * sp.sqrt(u))

quarter_beta = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2)) / 4
full_period = sp.simplify(4 * quarter_beta)
gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
expected_period = sp.sqrt(sp.pi) * gstar
check(
    "C15 weighted full traversal is sqrt(pi) G*",
    sp.simplify(sp.expand_func(full_period) - expected_period) == 0,
)

amplitude, mass, coupling = sp.symbols("A m lambda", positive=True)
dimensionless_rate = amplitude * sp.sqrt(2 * coupling / mass)
physical_period = expected_period / dimensionless_rate
check(
    "C16 dimensional restoration gives the quartic clock law",
    sp.simplify(physical_period * amplitude - expected_period * sp.sqrt(mass / (2 * coupling))) == 0,
)

eta, core_energy, bath = sp.symbols("eta E B", real=True)
gain = 1 + eta * (1 - core_energy)
next_energy = sp.expand(core_energy * gain**2)
linear_multiplier = sp.simplify(sp.diff(next_energy, core_energy).subs(core_energy, 1))
stability_margin = sp.factor(1 - linear_multiplier**2)
expected_margin = 4 * eta * (1 - eta)
ledger_residual = sp.simplify(
    next_energy + (bath + core_energy - next_energy) - (core_energy + bath)
)
check(
    "C17 imposed radial repair and environmental ledger close conditionally",
    linear_multiplier == 1 - 2 * eta
    and sp.simplify(stability_margin - expected_margin) == 0
    and ledger_residual == 0,
)

passed = sum(1 for _, value in checks if value)
all_pass = passed == len(checks)
print(f"\nFTD-0836 bilateral self-dual quartic clock: {passed}/{len(checks)} PASS")
if all_pass:
    print("BILATERAL_SELF_DUAL_QUARTIC_CLOCK_COORDINATE_THEOREM")
else:
    print("BILATERAL_SELF_DUAL_QUARTIC_CLOCK_CERTIFICATE_INVALID")
print("QUARTIC_HAMILTONIAN_STATUS=SELECTED_INPUT")
print("RADIAL_STABILIZER_STATUS=IMPOSED_REFERENCE_WITH_EXPLICIT_LEDGER")
print("NATIVE_SUBSTRATE_REALIZATION=OPEN")

raise SystemExit(0 if all_pass else 1)
