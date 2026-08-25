#!/usr/bin/env python3
"""Exact clocked-remainder recoil and translation-charge boundary.

This certificate replaces the provisional interpretation of a one-tick
material relocation as a physical velocity.  For a fixed integer cadence
L >= 6, a localized body carries an integer subcell remainder a in Z_L^3 and
a persistent directed impulse token d.  The lifted coordinate Y=L x+a obeys
Y'=Y+d.  The visible lattice coordinate therefore moves by at most one Moore
hop per tick and, for an SC token, by exactly one node per L ticks.

The same construction gives a reversible standing/outgoing port collision
that changes d rather than translating x.  A first-order translation-sector
action then supplies the exact discrete Noether/Legendre map at quadratic
order.  Cubic symmetry fixes the quadratic form up to one positive inertia
scale; it does not fix that scale or the field translation charge.

No experimental value, master root, fitted coefficient, Born target, or
continuum gravity formula enters.
"""

from __future__ import annotations

from dataclasses import dataclass
from itertools import product

import sympy as sp

from proof_moore_bond_capacity_type_census import signed_permutation_matrices


Vector = tuple[int, int, int]
ZERO: Vector = (0, 0, 0)
SC_DIRECTIONS: tuple[Vector, ...] = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)


def add(left: Vector, right: Vector) -> Vector:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def scale(factor: int, value: Vector) -> Vector:
    return tuple(factor * entry for entry in value)  # type: ignore[return-value]


def matrix_vector(matrix, value: Vector) -> Vector:
    result = sp.Matrix(matrix) * sp.Matrix(value)
    return tuple(int(entry) for entry in result)  # type: ignore[return-value]


@dataclass(frozen=True)
class WorldlineState:
    position: Vector
    remainder: Vector
    impulse: Vector


def divide_chart(value: Vector, cadence: int) -> tuple[Vector, Vector]:
    quotients = []
    residues = []
    for entry in value:
        quotient, residue = divmod(entry, cadence)
        quotients.append(quotient)
        residues.append(residue)
    return tuple(quotients), tuple(residues)  # type: ignore[return-value]


def lifted(state: WorldlineState, cadence: int) -> Vector:
    return add(scale(cadence, state.position), state.remainder)


def stream(state: WorldlineState, cadence: int) -> WorldlineState:
    raw_remainder = add(state.remainder, state.impulse)
    carry, remainder = divide_chart(raw_remainder, cadence)
    return WorldlineState(
        add(state.position, carry),
        remainder,
        state.impulse,
    )


def inverse_stream(state: WorldlineState, cadence: int) -> WorldlineState:
    prior_remainder_raw = add(state.remainder, scale(-1, state.impulse))
    chart_shift, prior_remainder = divide_chart(prior_remainder_raw, cadence)
    # prior_remainder_raw = cadence*chart_shift + prior_remainder and the
    # forward carry is therefore -chart_shift.
    prior_position = add(state.position, chart_shift)
    return WorldlineState(prior_position, prior_remainder, state.impulse)


def transform_chart(state: WorldlineState, cadence: int, matrix) -> WorldlineState:
    transformed_lift = matrix_vector(matrix, lifted(state, cadence))
    position, remainder = divide_chart(transformed_lift, cadence)
    return WorldlineState(
        position,
        remainder,
        matrix_vector(matrix, state.impulse),
    )


@dataclass(frozen=True)
class PortState:
    impulse: Vector
    outgoing: int
    capacity: int


def port_collision(state: PortState, direction: Vector) -> PortState:
    emitted = PortState(scale(-1, direction), 1, 0)
    standing = PortState(ZERO, 0, 1)
    if state == standing:
        return emitted
    if state == emitted:
        return standing
    raise ValueError("state is outside the registered reversible port pair")


def verify_worldline() -> int:
    checks = 0
    group = tuple(signed_permutation_matrices())
    assert len(group) == 48
    checks += 1

    for cadence in range(6, 13):
        residues = product(range(cadence), repeat=3)
        for remainder in residues:
            for impulse in (ZERO,) + SC_DIRECTIONS:
                state = WorldlineState((2, -1, 3), remainder, impulse)
                endpoint = stream(state, cadence)
                assert inverse_stream(endpoint, cadence) == state
                assert lifted(endpoint, cadence) == add(lifted(state, cadence), impulse)
                displacement = add(endpoint.position, scale(-1, state.position))
                assert all(entry in (-1, 0, 1) for entry in displacement)
                assert sum(abs(entry) for entry in displacement) <= 1
                checks += 4

    # A rational SC impulse is an exact one-node-per-L clocked worldline.
    for cadence in range(6, 25):
        for remainder in product(range(cadence), repeat=3):
            for impulse in SC_DIRECTIONS:
                state = WorldlineState((0, 0, 0), remainder, impulse)
                endpoint = state
                for _ in range(cadence):
                    endpoint = stream(endpoint, cadence)
                assert endpoint.position == add(state.position, impulse)
                assert endpoint.remainder == state.remainder
                assert endpoint.impulse == impulse
                assert sp.Rational(1, cadence) <= sp.Rational(1, 6)
                checks += 4

    # Full signed-cubic covariance uses the affine residue chart induced from
    # the lifted integer coordinate, including reflections.
    cadence = 6
    for matrix in group:
        for remainder in product(range(cadence), repeat=3):
            for impulse in (ZERO,) + SC_DIRECTIONS:
                state = WorldlineState((1, -2, 0), remainder, impulse)
                transformed_after = transform_chart(stream(state, cadence), cadence, matrix)
                after_transformed = stream(transform_chart(state, cadence, matrix), cadence)
                assert transformed_after == after_transformed
                checks += 1

    return checks


def verify_port_collision() -> int:
    checks = 0
    group = tuple(signed_permutation_matrices())
    for direction in SC_DIRECTIONS:
        standing = PortState(ZERO, 0, 1)
        emitted = port_collision(standing, direction)
        assert emitted == PortState(scale(-1, direction), 1, 0)
        assert port_collision(emitted, direction) == standing
        assert add(emitted.impulse, scale(emitted.outgoing, direction)) == ZERO
        assert add(standing.impulse, scale(standing.outgoing, direction)) == ZERO
        checks += 4

        for matrix in group:
            transformed_direction = matrix_vector(matrix, direction)
            transformed_emitted = PortState(
                matrix_vector(matrix, emitted.impulse),
                emitted.outgoing,
                emitted.capacity,
            )
            assert port_collision(standing, transformed_direction) == transformed_emitted
            checks += 1
    return checks


def verify_action_and_legendre_boundary() -> int:
    checks = 0
    mass = sp.symbols("m", positive=True)
    cadence = sp.symbols("L", integer=True, positive=True)
    kappa = sp.symbols("kappa", positive=True)
    gamma = sp.symbols("Gamma", positive=True)

    p = sp.Matrix(sp.symbols("p0:3"))
    delta_y = sp.Matrix(sp.symbols("dy0:3"))
    local_action = p.dot(delta_y) - p.dot(p) / (2 * mass)
    legendre_equations = sp.Matrix([sp.diff(local_action, entry) for entry in p])
    assert sp.simplify(legendre_equations - (delta_y - p / mass)) == sp.zeros(3, 1)
    checks += 1

    p_previous = sp.Matrix(sp.symbols("pp0:3"))
    p_next = sp.Matrix(sp.symbols("pn0:3"))
    y_previous = sp.Matrix(sp.symbols("yp0:3"))
    y_current = sp.Matrix(sp.symbols("yc0:3"))
    y_next = sp.Matrix(sp.symbols("yn0:3"))
    adjacent_action = (
        p_previous.dot(y_current - y_previous)
        + p_next.dot(y_next - y_current)
    )
    noether_equations = sp.Matrix(
        [sp.diff(adjacent_action, entry) for entry in y_current]
    )
    assert noether_equations == p_previous - p_next
    checks += 1

    # Cubic symmetry leaves one positive quadratic coefficient and no linear
    # polar term.  This is the low-momentum Legendre map, not a mass value.
    a00, a01, a02, a11, a12, a22 = sp.symbols("a00 a01 a02 a11 a12 a22")
    metric = sp.Matrix(
        ((a00, a01, a02), (a01, a11, a12), (a02, a12, a22))
    )
    ell = sp.Matrix(sp.symbols("ell0:3"))
    metric_equations = []
    linear_equations = []
    for matrix_raw in signed_permutation_matrices():
        matrix = sp.Matrix(matrix_raw)
        metric_equations.extend(matrix.T * metric * matrix - metric)
        linear_equations.extend(matrix.T * ell - ell)
    metric_solution = sp.solve(metric_equations, (a01, a02, a11, a12, a22), dict=True)
    assert metric_solution == [{a01: 0, a02: 0, a11: a00, a12: 0, a22: a00}]
    assert sp.solve(linear_equations, tuple(ell), dict=True) == [
        {ell[0]: 0, ell[1]: 0, ell[2]: 0}
    ]
    checks += 2

    # The minimum canonical kick exchanges one common coefficient between
    # matter and field.  It fixes equality of charges, not their scale.
    x_m = sp.Matrix(sp.symbols("xm0:3"))
    x_f = sp.Matrix(sp.symbols("xf0:3"))
    p_m_new = sp.Matrix(sp.symbols("pm0:3"))
    p_f_new = sp.Matrix(sp.symbols("pf0:3"))
    route = sp.Matrix(sp.symbols("r0:3"))
    generator = (
        x_m.dot(p_m_new + kappa * route)
        + x_f.dot(p_f_new - kappa * route)
    )
    p_m_old = sp.Matrix([sp.diff(generator, entry) for entry in x_m])
    p_f_old = sp.Matrix([sp.diff(generator, entry) for entry in x_f])
    x_m_new = sp.Matrix([sp.diff(generator, entry) for entry in p_m_new])
    x_f_new = sp.Matrix([sp.diff(generator, entry) for entry in p_f_new])
    assert p_m_new == p_m_old - kappa * route
    assert p_f_new == p_f_old + kappa * route
    assert sp.simplify(p_m_new + p_f_new - p_m_old - p_f_old) == sp.zeros(3, 1)
    assert x_m_new == x_m and x_f_new == x_f
    checks += 4

    # For a rest body kicked onto the unit-per-L lattice orbit, the common
    # exchange coefficient is m/L and the recoil energy is kappa/(2L).
    required_kappa = mass / cadence
    recoil_energy = sp.simplify(required_kappa**2 / (2 * mass))
    assert recoil_energy == mass / (2 * cadence**2)
    assert sp.simplify(recoil_energy - required_kappa / (2 * cadence)) == 0
    checks += 2

    # Conditional diagnostic only: if a future symmetric field stress assigns
    # p_field=6 Gamma, canonical matching gives m=6 L Gamma.  Combining that
    # with the selected Gamma/2 field handoff gives an L-dependent work ratio,
    # not a native alpha measurement.
    matched_mass = 6 * cadence * gamma
    matched_recoil = sp.simplify(matched_mass / (2 * cadence**2))
    source_work = sp.simplify(gamma / 2 + matched_recoil)
    field_work_fraction = sp.simplify((gamma / 2) / source_work)
    blocked_response = sp.simplify(gamma / source_work)
    assert matched_recoil == 3 * gamma / cadence
    assert source_work == gamma * (cadence + 6) / (2 * cadence)
    assert field_work_fraction == cadence / (cadence + 6)
    assert blocked_response == 2 * cadence / (cadence + 6)
    checks += 4

    return checks


def main() -> None:
    checks = verify_worldline()
    checks += verify_port_collision()
    checks += verify_action_and_legendre_boundary()

    print("finite remainder clock: Y=Lx+a and Y'=Y+d")
    print("SC impulse token: one visible node hop per L ticks")
    print("L>=6 respects the selected field speed c_eff=1/6; L>6 is subluminal")
    print("standing/outgoing collision changes impulse, not position")
    print("translation-sector action gives conserved canonical p and Delta y=p/m")
    print("cubic quadratic kinetic metric is unique up to one positive inertia scale")
    print("unit-per-L recoil requires kappa=m/L and costs m/(2L^2)")
    print("field Noether charge, inertia scale, and full common action remain open")
    print(
        "PASS: clocked-remainder recoil and Noether boundary "
        f"({checks} exact checks)"
    )


if __name__ == "__main__":
    main()
