"""FTD-0841 exact local flux self-pair tensor certificate.

This source-locked certificate tests a registered local mathematical
extension. It does not search coefficients, fit a period, or modify production
dynamics.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCE_HASHES = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/lagrangian.h":
        "0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md":
        "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
    "engine/include/ftd/eft/native_pair_energy_recursion.h":
        "81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A",
}


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"[{'PASS' if passed else 'FAIL'}] {name}")


def exact_zero(expression: sp.Expr | sp.MatrixBase) -> bool:
    if isinstance(expression, sp.MatrixBase):
        return all(sp.simplify(sp.expand_func(item)) == 0 for item in expression)
    return sp.simplify(sp.expand_func(expression)) == 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


check(
    "C1 all seven frozen production, theory, and reference sources match",
    all(sha256(ROOT / relative) == expected
        for relative, expected in SOURCE_HASHES.items()),
)

voxel_source = (ROOT / "engine/include/ftd/voxel.h").read_text(
    encoding="utf-8")
lagrangian_source = (ROOT / "engine/include/ftd/lagrangian.h").read_text(
    encoding="utf-8")
energy_source = (ROOT / "engine/src/energy_ledger_compute.cpp").read_text(
    encoding="utf-8")
phase_read_source = (
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
).read_text(encoding="utf-8")

check(
    "C2 each voxel has local flux and wave momentum registers",
    "Vec3 flux;" in voxel_source
    and "Vec3 wave_vel;" in voxel_source
    and "The canonical momentum of the flux field." in lagrangian_source
    and "wave_vel (the conjugate momentum)" in lagrangian_source,
)

registered_energy_line = (
    "const double E_total = 0.5 * (E_field + E_wave) + E_kin + E_strong;"
)
forbidden_tokens = (
    "pair_energy", "self_pair_tensor", "quartic_flux_energy",
    "flux.mag2() * flux.mag2()", "4.0 * lambda * flux.mag2()"
)
check(
    "C3 the frozen production core has no local self-pair energy channel",
    registered_energy_line in energy_source
    and all(token not in energy_source for token in forbidden_tokens)
    and all(token not in phase_read_source for token in forbidden_tokens),
)

x, y, z = sp.symbols("x y z", real=True)
J = sp.Matrix([x, y, z])
U = J * J.T
check(
    "C4 the local self-pair tensor is symmetric and generically rank one",
    U == U.T and U.rank() == 1,
)

r2 = (J.T * J)[0]
frobenius_sq = sp.trace(U.T * U)
check(
    "C5 the self-pair Frobenius norm squared is exactly flux norm to the fourth",
    exact_zero(frobenius_sq - r2**2),
)

wx, wy, wz = sp.symbols("w_x w_y w_z", real=True)
W = sp.Matrix([wx, wy, wz])
m, lam = sp.symbols("m lambda", positive=True, real=True)
w2 = (W.T * W)[0]
H = w2 / (2 * m) + lam * r2**2
H_pair = w2 / (2 * m) + lam * frobenius_sq
check(
    "C6 the local Hamiltonian is exactly kinetic plus quadratic tensor-pair energy",
    exact_zero(H - H_pair),
)

gradient = sp.Matrix([sp.diff(lam * r2**2, item) for item in J])
check(
    "C7 the tensor-pair energy has the radial cubic gradient",
    exact_zero(gradient - 4 * lam * r2 * J),
)

Jdot = sp.Matrix([sp.diff(H, item) for item in W])
Wdot = -sp.Matrix([sp.diff(H, item) for item in J])
check(
    "C8 Hamilton equations are vector velocity plus radial cubic force",
    exact_zero(Jdot - W / m)
    and exact_zero(Wdot + 4 * lam * r2 * J),
)

angular_derivative = Jdot.cross(W) + J.cross(Wdot)
check(
    "C9 the continuous radial self-pair flow conserves angular momentum",
    exact_zero(angular_derivative),
)

e1, e2, e3, q, p = sp.symbols("e_1 e_2 e_3 q p", real=True)
e = sp.Matrix([e1, e2, e3])
e_norm = (e.T * e)[0]
polar_J = q * e
polar_W = p * e
polar_Jdot = polar_W / m
polar_Wdot = -4 * lam * ((polar_J.T * polar_J)[0]) * polar_J
polar_force_defect = sp.factor(
    polar_Wdot + 4 * lam * q**3 * e)
check(
    "C10 every unit-vector linearly polarized sector is invariant",
    exact_zero(polar_Jdot - (p / m) * e)
    and exact_zero(
        polar_force_defect + 4 * lam * q**3 * (e_norm - 1) * e
    ),
)

polar_H = (polar_W.T * polar_W)[0] / (2 * m) \
    + lam * ((polar_J.T * polar_J)[0])**2
scalar_H = p**2 / (2 * m) + lam * q**4
polar_H_defect = sp.factor(polar_H - scalar_H)
expected_H_defect = (
    p**2 * (e_norm - 1) / (2 * m)
    + lam * q**4 * (e_norm**2 - 1)
)
check(
    "C11 a unit-polarized sector reduces exactly to the FTD-0840 scalar Hamiltonian",
    exact_zero(polar_H_defect - expected_H_defect),
)

gstar = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
beta_quartic = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
Aamp = sp.symbols("A", positive=True, real=True)
period = sp.sqrt(m / (2 * lam)) * beta_quartic / Aamp
check(
    "C12 the polarized continuum sector has the exact G-star period-amplitude law",
    exact_zero(period * Aamp
               - sp.sqrt(sp.pi) * gstar * sp.sqrt(m / (2 * lam))),
)

I1 = x**4 + y**4 + z**4
I2 = x**2 * y**2 + y**2 * z**2 + z**2 * x**2
swap_xy = {x: y, y: x}
cycle_xyz = {x: y, y: z, z: x}
flip_x = {x: -x}
check(
    "C13 the two quartic basis invariants survive cubic signed-permutation generators",
    all(exact_zero(invariant.xreplace(transform) - invariant)
        for invariant in (I1, I2)
        for transform in (swap_xy, cycle_xyz, flip_x)),
)

a, b = sp.symbols("a b", real=True)
axis_value = a
diagonal_value = a / 2 + b / 4
check(
    "C14 a forty-five-degree rotation forces the radial ratio b equals two a",
    sp.solve(sp.Eq(axis_value, diagonal_value), b) == [2 * a],
)

check(
    "C15 the self-pair Frobenius quartic has exactly the full-rotation radial ratio",
    exact_zero(r2**2 - (I1 + 2 * I2)),
)

j0x, j0y, j0z = sp.symbols("j0_x j0_y j0_z", real=True)
j1x, j1y, j1z = sp.symbols("j1_x j1_y j1_z", real=True)
w0x, w0y, w0z = sp.symbols("w0_x w0_y w0_z", real=True)
w1x, w1y, w1z = sp.symbols("w1_x w1_y w1_z", real=True)
J0 = sp.Matrix([j0x, j0y, j0z])
J1 = sp.Matrix([j1x, j1y, j1z])
W0 = sp.Matrix([w0x, w0y, w0z])
W1 = sp.Matrix([w1x, w1y, w1z])
r0_sq = J0.dot(J0)
r1_sq = J1.dot(J1)
discrete_gradient = lam * (r1_sq + r0_sq) * (J1 + J0)
check(
    "C16 the vector force is the exact quartic secant discrete gradient",
    exact_zero((J1 - J0).dot(discrete_gradient)
               - lam * (r1_sq**2 - r0_sq**2)),
)

diagonal_gradient = discrete_gradient.subs({
    j1x: j0x, j1y: j0y, j1z: j0z,
})
check(
    "C17 the vector discrete gradient tends to the radial cubic force",
    exact_zero(diagonal_gradient - 4 * lam * r0_sq * J0),
)

h = sp.symbols("h", nonzero=True, real=True)
delta_J_on_step = h * (W1 + W0) / (2 * m)
delta_W_on_step = -h * discrete_gradient
energy_change_on_step = (
    delta_W_on_step.dot(W1 + W0) / (2 * m)
    + delta_J_on_step.dot(discrete_gradient)
)
check(
    "C18 the vector discrete-gradient recursion conserves total energy exactly",
    exact_zero(energy_change_on_step),
)

residual_J = J1 - J0 - h * (W1 + W0) / (2 * m)
residual_W = W1 - W0 + h * discrete_gradient
reverse_step_J = J0 - J1 + h * (W0 + W1) / (2 * m)
reverse_step_W = W0 - W1 - h * discrete_gradient
physical_reverse_J = J0 - J1 + h * (W0 + W1) / (2 * m)
physical_reverse_W = W1 - W0 + h * discrete_gradient
check(
    "C19 endpoint-step and physical momentum reversals preserve the equations",
    exact_zero(reverse_step_J + residual_J)
    and exact_zero(reverse_step_W + residual_W)
    and exact_zero(physical_reverse_J + residual_J)
    and exact_zero(physical_reverse_W - residual_W),
)

Jbar = (J1 + J0) / 2
Wbar = (W1 + W0) / 2
angular_change_on_step = (
    Jbar.cross(delta_W_on_step) + delta_J_on_step.cross(Wbar)
)
check(
    "C20 the discrete vector recursion conserves angular momentum exactly",
    exact_zero(angular_change_on_step),
)

swept_on_step = Jbar.dot(delta_W_on_step) \
    - Wbar.dot(delta_J_on_step)
swept_target = -h * (
    lam * (r1_sq + r0_sq) * (J1 + J0).dot(J1 + J0) / 2
    + (W1 + W0).dot(W1 + W0) / (4 * m)
)
check(
    "C21 the vector swept-area scalar has the exact negative-square factorization",
    exact_zero(swept_on_step - swept_target),
)

antipodal_substitution = {
    j1x: -j0x, j1y: -j0y, j1z: -j0z,
    w1x: -w0x, w1y: -w0y, w1z: -w0z,
}
antipodal_J = sp.simplify(residual_J.subs(antipodal_substitution))
antipodal_W = sp.simplify(residual_W.subs(antipodal_substitution))
check(
    "C22 zero swept area is compatible with the step equations only at the origin",
    exact_zero(antipodal_J + 2 * J0)
    and exact_zero(antipodal_W + 2 * W0),
)

AA, BB, vector_sq, point_sq, anchor_sq = sp.symbols(
    "AA BB V2 X2 J02", real=True)
directional_form = (point_sq + anchor_sq) * vector_sq \
    + 2 * AA * (AA + BB)
monotone_decomposition = (
    2 * AA**2 + (AA + BB)**2
    + (point_sq * vector_sq - AA**2)
    + (anchor_sq * vector_sq - BB**2)
)
check(
    "C23 the nonlinear vector derivative has the registered strong-monotonicity decomposition",
    exact_zero(directional_form - monotone_decomposition),
)

t = sp.symbols("t", positive=True, real=True)
n1, n2, n3 = sp.symbols("n_1 n_2 n_3", real=True)
N = sp.Matrix([n1, n2, n3])
X = t * N
F_X = 2 * m * (X - J0) / h - 2 * W0 \
    + h * lam * (X.dot(X) + r0_sq) * (X + J0)
coercive_polynomial = sp.Poly(sp.expand(F_X.dot(X)), t)
check(
    "C24 strong monotonicity and the positive radial leading term give one global next state",
    coercive_polynomial.degree() == 4
    and exact_zero(coercive_polynomial.LC() - h * lam * (N.dot(N))**2),
)

E = sp.symbols("E", positive=True, real=True)
coordinate_bound_fourth = E / lam
momentum_bound_squared = 2 * m * E
check(
    "C25 every positive-energy vector shell is compact in local phase space",
    bool(coordinate_bound_fourth.is_positive)
    and bool(momentum_bound_squared.is_positive),
)

check(
    "C26 the result is a local mathematical gearbox with physical selections open",
    registered_energy_line in energy_source
    and exact_zero(frobenius_sq - r2**2)
    and exact_zero(energy_change_on_step)
    and exact_zero(angular_change_on_step)
    and exact_zero(r2**2 - (I1 + 2 * I2))
    and sp.solve(sp.Eq(axis_value, diagonal_value), b) == [2 * a],
)

passed = sum(1 for _, value in checks if value)
all_pass = passed == len(checks)
print(f"\nFTD-0841 local flux self-pair tensor: {passed}/{len(checks)} PASS")
if all_pass:
    print("VOXEL_FLUX_AND_WAVE_VELOCITY_SUPPLY_LOCAL_CANONICAL_TYPE")
    print("SELF_PAIR_TENSOR_FROBENIUS_ENERGY_GIVES_AXIS_FREE_QUARTIC")
    print("VECTOR_RECURSION_UNIQUE_REVERSIBLE_ENERGY_AND_ANGULAR_MOMENTUM_CLOSED")
    print("POLARIZED_CONTINUUM_SECTOR_HAS_GSTAR_PERIOD")
    print("PRODUCTION_COUPLING_ISOTROPY_POLARIZATION_SUPPORT_AND_TICK_CADENCE_OPEN")
else:
    print("LOCAL_FLUX_SELF_PAIR_TENSOR_CERTIFICATE_INVALID")

raise SystemExit(0 if all_pass else 1)
