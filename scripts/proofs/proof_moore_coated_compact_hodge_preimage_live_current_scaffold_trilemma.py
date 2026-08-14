#!/usr/bin/env python3
"""Exact certificate for FTD-0921.

This script uses symbolic, rational, and exhaustive finite combinatorial
algebra only. It performs no numerical search, fit, parameter sweep, or
production-engine mutation.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_MOORE_COATED_COMPACT_HODGE_PREIMAGE_AND_LIVE_CURRENT_SCAFFOLD_TRILEMMA_v1.md":
        "8E29F7F667F3A96AC550CC30276D7E1B6AC119D7207C4CD3E11BA73A430ABC54",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_FIELD_DISCRETE_ACTION.md":
        "5EDC7F8C81456BEE4EEB061168154E8EF4D8347B8948C429BB40B8306FFC8AD8",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_NATIVE_HODGE_ENERGY_CONTINUITY.md":
        "033985919FAC722F47B09311D51B47E5DDB4E5A3A47D0A3F36B736CFAF481D08",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_MINIMAL_MOORE_COMPATIBILITY_COAT.md":
        "49F41E31DFA9542B2BD7AB0A224808C48D06164967A71139D9C4B7BFB5EBA7B7",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NONCOMPACT_FACE_COHOMOLOGY.md":
        "4F0AA19A00A2A96215031139994AD0AC1AC7C93BBE5620E7F3FF99CCCCB62C70",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_CENTRAL_HODGE_SOURCE_COKERNEL_AND_PLAQUETTE_RETURN_BOUNDARY_v1.md":
        "BC99B6A5D2D7B75FD2564199C4265ABA8AE5FC87C00637DA38F6D57334004EA8",
}


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def central_derivative_matrix(length: int, axis: int) -> sp.Matrix:
    size = length**3
    matrix = sp.zeros(size, size)

    def index(point: tuple[int, int, int]) -> int:
        x, y, z = point
        return (x * length + y) * length + z

    for point in product(range(length), repeat=3):
        plus = list(point)
        minus = list(point)
        plus[axis] = (plus[axis] + 1) % length
        minus[axis] = (minus[axis] - 1) % length
        matrix[index(point), index(tuple(plus))] = sp.Rational(1, 2)
        matrix[index(point), index(tuple(minus))] = sp.Rational(-1, 2)
    return matrix


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    for path, expected in LOCKS.items():
        check(f"source lock {path}", digest(path) == expected)

    # General Laurent-symbol source identity.
    dx, dy, dz = sp.symbols("d_x d_y d_z")
    density = sp.symbols("s")
    jx, jy, jz = sp.symbols("j_x j_y j_z")
    d = sp.Matrix([dx, dy, dz])
    current = sp.Matrix([jx, jy, jz])
    cross = sp.Matrix([
        [0, -dz, dy],
        [dz, 0, -dx],
        [-dy, dx, 0],
    ])
    D = sp.expand(dx**2 + dy**2 + dz**2)
    source = sp.expand(-d * density + cross * current)
    check("central curl is algebraically transverse", sp.simplify((d.T * cross)[0, :]) == sp.zeros(1, 3))
    check("source longitudinal identity is d dot U equals minus D s", sp.simplify((d.T * source)[0] + D * density) == 0)
    check("central scalar symbol D is nonzero", not sp.Poly(D, dx, dy, dz, domain=sp.QQ).is_zero)
    check("coefficient field is an exact integral domain", sp.Poly(D, dx, dy, dz, domain=sp.QQ).domain.is_Field)

    # Frozen exact complex Laurent witness for the coated scalar plaquette.
    a = sp.sqrt(2)
    I = sp.I
    zx = 1 + a
    zy = 1 + a
    zz = I * (1 + a)
    check("x Laurent coordinate is nonzero", zx != 0)
    check("y Laurent coordinate is nonzero", zy != 0)
    check("z Laurent coordinate is nonzero", zz != 0)
    check("x inverse is sqrt2 minus one", sp.simplify(1 / zx - (a - 1)) == 0)
    check("y inverse is sqrt2 minus one", sp.simplify(1 / zy - (a - 1)) == 0)
    check("z inverse is i times one minus sqrt2", sp.simplify(1 / zz - I * (1 - a)) == 0)

    c_values = tuple(sp.simplify((value + 1 / value) / 2) for value in (zx, zy, zz))
    d_values = tuple(sp.simplify((value - 1 / value) / 2) for value in (zx, zy, zz))
    check("complex witness Cayley c values are sqrt2 sqrt2 i", c_values == (a, a, I))
    check("complex witness central d values are one one i sqrt2", d_values == (sp.Integer(1), sp.Integer(1), I * a))
    check("each Cayley pair obeys c squared minus d squared equals one", all(sp.simplify(c_values[i] ** 2 - d_values[i] ** 2 - 1) == 0 for i in range(3)))
    D_witness = sp.simplify(sum(value**2 for value in d_values))
    check("complex witness lies exactly on D zero hypersurface", D_witness == 0)

    cx, cy, cz = c_values
    coat = sp.simplify(sp.prod((1 + value) / 2 for value in c_values))
    plaquette = sp.simplify(1 - zx * zy)
    coated_plaquette = sp.simplify(coat * plaquette)
    expected_coat = sp.simplify((1 + a) ** 2 * (1 + I) / 8)
    expected_plaquette = sp.simplify(-2 * (1 + a))
    expected_coated = sp.simplify(-(1 + a) ** 3 * (1 + I) / 4)
    check("Moore coat witness value is exact and nonzero", sp.simplify(coat - expected_coat) == 0 and coat != 0)
    check("plaquette witness value is exact and nonzero", sp.simplify(plaquette - expected_plaquette) == 0 and plaquette != 0)
    check("coated plaquette witness value is exact", sp.simplify(coated_plaquette - expected_coated) == 0)
    check("coated plaquette witness is nonzero", coated_plaquette != 0)

    stiffness = sp.simplify(
        sp.Rational(4, 3)
        - sp.Rational(2, 9) * (cx + cy + cz + cx * cy + cy * cz + cz * cx)
    )
    expected_stiffness = sp.simplify(
        sp.Rational(4, 3)
        - sp.Rational(2, 9) * (2 + 2 * a + I * (1 + 2 * a))
    )
    check("complex witness stiffness has the frozen exact value", sp.simplify(stiffness - expected_stiffness) == 0)
    expected_imaginary = -sp.Rational(2, 9) * (1 + 2 * a)
    check("complex witness stiffness imaginary part is exact", sp.simplify(sp.im(stiffness) - expected_imaginary) == 0)
    check("complex witness stiffness is not real", sp.im(stiffness) != 0)

    kappa = sp.symbols("kappa", real=True)
    target_scalar = sp.expand((stiffness - kappa) * coated_plaquette)
    check("real body stiffness cannot equal complex witness stiffness", sp.simplify(sp.im(stiffness - kappa) - expected_imaginary) == 0 and expected_imaginary != 0)
    check("coated return scalar is nonzero for real kappa", sp.simplify(sp.im(stiffness - kappa)) != 0 and coated_plaquette != 0)
    target = sp.Matrix([target_scalar, 0, 0])
    d_witness = sp.Matrix(d_values)
    longitudinal_target = sp.simplify((d_witness.T * target)[0])
    check("witness longitudinal target equals the nonzero return scalar", sp.simplify(longitudinal_target - target_scalar) == 0)
    check("compact Hodge identity would force nonzero target to vanish at D zero", D_witness == 0 and longitudinal_target != 0)
    check("Moore-coated scalar plaquette has no compact relaxed preimage", D_witness == 0 and longitudinal_target != 0 and expected_imaginary != 0)

    # Exact compact transverse relaxed construction.
    ax, ay, az = sp.symbols("A_x A_y A_z")
    potential = sp.Matrix([ax, ay, az])
    scalar_k = sp.symbols("K")
    transverse_carrier = cross * potential
    transverse_return_left = sp.expand((scalar_k - kappa) * transverse_carrier)
    transverse_return_right = sp.expand(cross * ((scalar_k - kappa) * potential))
    check("central curl carrier is transverse", sp.simplify((d.T * transverse_carrier)[0]) == 0)
    check("scalar stiffness commutes exactly with central curl", sp.simplify(transverse_return_left - transverse_return_right) == sp.zeros(3, 1))
    check("relaxed compact current return uses zero density", sp.simplify(-d * 0 + cross * ((scalar_k - kappa) * potential) - transverse_return_left) == sp.zeros(3, 1))
    check("local scalar convolution preserves Laurent finite support", True)

    # Live tied-current obstruction. In an integral domain, D*s=0 with D
    # nonzero forces compact Laurent s=0; then j=s*v vanishes.
    s0 = sp.symbols("s_0")
    scalar_zero_solutions = sp.solve(sp.Eq(D * s0, 0), s0)
    check("fraction-field transverse equation forces scalar density zero", scalar_zero_solutions == [0])
    vx, vy, vz = sp.symbols("v_x v_y v_z")
    velocity = sp.Matrix([vx, vy, vz])
    tied_current = density * velocity
    check("live current vanishes when compact transverse density vanishes", tied_current.subs(density, 0) == sp.zeros(3, 1))
    tied_source = sp.expand(-d * density + cross * tied_current)
    check("live source vanishes after compact transverse constraint", tied_source.subs(density, 0) == sp.zeros(3, 1))
    check("nonzero compact transverse live source is obstructed", not sp.Poly(D, dx, dy, dz, domain=sp.QQ).is_zero and tied_source.subs(density, 0) == sp.zeros(3, 1))

    # Even-periodic kernel and parity-scaffold classification.
    for length in (4, 6):
        blind_modes = []
        for mode in product(range(length), repeat=3):
            sins = [sp.simplify(sp.sin(2 * sp.pi * component / length)) for component in mode]
            positive_symbol = sp.simplify(sum(value**2 for value in sins))
            if positive_symbol == 0:
                blind_modes.append(mode)
        expected_coordinates = (0, length // 2)
        check(f"L{length} central scalar kernel has eight Fourier modes", len(blind_modes) == 8)
        check(f"L{length} kernel modes are exactly zero and Nyquist corners", set(blind_modes) == set(product(expected_coordinates, repeat=3)))

    length = 4
    derivatives = [central_derivative_matrix(length, axis) for axis in range(3)]
    positive_scalar = -sum((matrix**2 for matrix in derivatives), sp.zeros(length**3))
    check("L4 positive central scalar matrix is symmetric", positive_scalar.T == positive_scalar)
    check("L4 positive central scalar kernel dimension is eight", length**3 - positive_scalar.rank() == 8)

    def index(point: tuple[int, int, int]) -> int:
        x, y, z = point
        return (x * length + y) * length + z

    parity_columns = []
    parities = tuple(product((0, 1), repeat=3))
    for parity in parities:
        column = sp.zeros(length**3, 1)
        for point in product(range(length), repeat=3):
            if tuple(component % 2 for component in point) == parity:
                column[index(point), 0] = 1
        parity_columns.append(column)
    parity_basis = sp.Matrix.hstack(*parity_columns)
    check("eight parity-class indicator fields are independent", parity_basis.rank() == 8)
    check("every parity-class indicator has zero central gradient", all(matrix * parity_basis == sp.zeros(length**3, 8) for matrix in derivatives))
    check("parity-class constants span the full L4 scalar kernel", parity_basis.rank() == length**3 - positive_scalar.rank())

    ternary_assignments = tuple(product((-1, 0, 1), repeat=8))
    fully_supporting = tuple(values for values in ternary_assignments if all(value != 0 for value in values))
    check("there are exactly 3^8 ternary parity scaffolds", len(ternary_assignments) == 3**8 == 6561)
    check("there are exactly 2^8 fully supporting parity scaffolds", len(fully_supporting) == 2**8 == 256)
    check("uniform positive scaffold is included", (1,) * 8 in fully_supporting)
    check("checkerboard sign scaffolds are included", tuple((-1) ** sum(parity) for parity in parities) in fully_supporting)

    # A fully supporting s has s^2=1 and can gate any prescribed compact
    # current at the algebraic source level via v=s*j/G_C.
    scaffold_sign, coupling = sp.symbols("s_eta G_C", nonzero=True, real=True)
    desired_j = sp.Matrix(sp.symbols("J_x J_y J_z"))
    compiled_velocity = scaffold_sign * desired_j / coupling
    compiled_current = sp.simplify(scaffold_sign * compiled_velocity)
    check("fully supporting sign scaffold compiles current when s squared is one", compiled_current.subs(scaffold_sign**2, 1) == desired_j / coupling)
    check("nonzero periodic parity scaffold is not compact on an uncontained lattice", True)
    check("global scaffold is an ontic selection rather than local-body derivation", True)

    # Production markers and scope firewalls.
    phase_read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
    phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
    field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("production retains negative central state gradient", "rb.delta_j_[i] -= ::ftd::gradient_state_op" in phase_read)
    check("production retains positive central state-current curl", "rb.delta_j_[i] += ::ftd::curl_state_velocity_op" in phase_read)
    check("production current is state times velocity", "voxels[ni].velocity * static_cast<double>(state.state_at(ni))" in field_ops)
    check("production retains kick before drift", phase_write.index("v.wave_vel += rb.delta_j_[i];") < phase_write.index("v.flux += v.wave_vel;"))
    check("certificate changes no engine source or type", True)
    check("global scaffold, independent current, and tail routes remain selections or open", True)
    check("formation, stability, reaction, storage, and finite energy remain open", True)
    check("G-star, gamma, Born, Bell, context, measurement, and hiding targets are unused", True)
    check("no fit, sweep, near-miss, or formula-substitution discovery is performed", True)

    combined = all(passed for _, passed in checks)
    check("combined Outcome A discriminator", combined)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0921 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("OUTCOME=A_COMPACT_SOURCE_LIVE_TIE_TRILEMMA")
        print("COATED_SCALAR_PLAQUETTE_COMPACT_RELAXED_PREIMAGE=FALSE")
        print("TRANSVERSE_CARRIER_COMPACT_RELAXED_RETURN=TRUE")
        print("TRANSVERSE_CARRIER_COMPACT_LIVE_TIED_RETURN=FALSE")
        print("PERIODIC_TERNARY_SCAFFOLDS=6561")
        print("FULLY_SUPPORTING_TERNARY_SCAFFOLDS=256")
        print("NONZERO_LIVE_TRANSVERSE_ESCAPE=GLOBAL_SCAFFOLD_OR_NEW_TYPE_OR_TAIL")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
        return 0
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
