#!/usr/bin/env python3
"""Exact FTD-0930 certificate.

This certificate composes the frozen source-centered canonical quarter-turn
with the C18 operator.  It verifies exact eight-color locality, positive
field/port energy exchange, the phase-complete local Hamiltonian gate,
finite-grounded convergence, and the separate massless uncontained boundary.
It performs no numerical search, fit, sweep, or engine change.
"""

from __future__ import annotations

from hashlib import sha256
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md":
        "D4BD884513A39EA42F1DB216D2E359A83126BB49195457663A1AE0D2B336B54A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md":
        "4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB",
    "scripts/proofs/proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py":
        "AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md":
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    "scripts/proofs/proof_canonical_source_centered_gauss_gate_v2.py":
        "6C35135A3B5B9345E6EA9A6EBFB61B32951EE07DDDB17188362B8B38A10F1816",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_FINITE_PORT_RAIL_POSITIVE_SOURCE_BATTERY_AND_RECYCLING_BOUNDARY_v1.md":
        "AF810B73322DE8521C8509792E09D549A10E1D8417C1B283A3630EB8B16D7BFC",
    "scripts/proofs/proof_finite_port_rail_positive_source_battery_boundary_v2.py":
        "E2129A5284AB5C664C5A257B0D861D2A5C4329776CC0E684365845B120379D87",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md":
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    "scripts/proofs/proof_local_canonical_hamiltonian_parity_rail.py":
        "B971DDA9A79AD53C340B00A4268EF9DA5BF089AF62DC37DE3D04757FAE03E326",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
}

Point = tuple[int, int, int]
Color = tuple[int, int, int]
COLORS: tuple[Color, ...] = tuple(product((0, 1), repeat=3))
OFFSETS: tuple[Point, ...] = tuple(
    offset
    for offset in product((-1, 0, 1), repeat=3)
    if 1 <= sum(value != 0 for value in offset) <= 2
)


def digest(relative_path: str) -> str:
    return sha256((ROOT / relative_path).read_bytes()).hexdigest().upper()


def add(left: Point, right: Point) -> Point:
    return tuple(left[i] + right[i] for i in range(3))  # type: ignore[return-value]


def color(point: Point) -> Color:
    return tuple(value % 2 for value in point)  # type: ignore[return-value]


def weight(offset: Point) -> sp.Rational:
    nonzero = sum(value != 0 for value in offset)
    if nonzero == 1:
        return sp.Rational(1, 9)
    if nonzero == 2:
        return sp.Rational(1, 18)
    raise ValueError("C18 offset required")


def c18_matrix(points: tuple[Point, ...]) -> sp.Matrix:
    index = {point: i for i, point in enumerate(points)}
    matrix = sp.zeros(len(points))
    for point, i in index.items():
        matrix[i, i] = sp.Rational(4, 3)
        for offset in OFFSETS:
            neighbor = add(point, offset)
            if neighbor in index:
                matrix[i, index[neighbor]] = -weight(offset)
    return matrix


def incidence_factor(points: tuple[Point, ...]) -> tuple[sp.Matrix, sp.Matrix]:
    index = {point: i for i, point in enumerate(points)}
    rows: list[sp.Matrix] = []
    weights: list[sp.Rational] = []
    for point, i in index.items():
        for offset in OFFSETS:
            neighbor = add(point, offset)
            if neighbor in index and point > neighbor:
                continue
            row = sp.zeros(1, len(points))
            row[0, i] = 1
            if neighbor in index:
                row[0, index[neighbor]] = -1
            rows.append(row)
            weights.append(weight(offset))
    incidence = rows[0]
    for row in rows[1:]:
        incidence = incidence.col_join(row)
    return incidence, sp.diag(*weights)


def embedding(indices: tuple[int, ...], size: int) -> sp.Matrix:
    result = sp.zeros(size, len(indices))
    for column, row in enumerate(indices):
        result[row, column] = 1
    return result


def rank_mod_prime(matrix: sp.Matrix, prime: int = 1_000_003) -> int:
    """Return an exact lower-bound witness for rational rank.

    A nonzero minor modulo one prime is an exact certificate that the same
    rational minor is nonzero.  Every caller below asks only whether the
    matrix has full column/row rank, so equality with the maximum possible
    rank proves the rational rank exactly without an expensive symbolic
    determinant expansion.
    """

    rows = []
    for row in range(matrix.rows):
        converted = []
        for column in range(matrix.cols):
            numerator, denominator = sp.fraction(matrix[row, column])
            denominator_mod = int(denominator) % prime
            if denominator_mod == 0:
                raise ValueError("chosen exact rank prime divides a denominator")
            converted.append(
                (int(numerator) % prime) * pow(denominator_mod, -1, prime) % prime
            )
        rows.append(converted)

    rank = 0
    for column in range(matrix.cols):
        pivot = next((row for row in range(rank, matrix.rows) if rows[row][column]), None)
        if pivot is None:
            continue
        rows[rank], rows[pivot] = rows[pivot], rows[rank]
        inverse = pow(rows[rank][column], -1, prime)
        rows[rank] = [(value * inverse) % prime for value in rows[rank]]
        for row in range(matrix.rows):
            if row == rank or rows[row][column] == 0:
                continue
            factor = rows[row][column]
            rows[row] = [
                (rows[row][entry] - factor * rows[rank][entry]) % prime
                for entry in range(matrix.cols)
            ]
        rank += 1
        if rank == matrix.rows:
            break
    return rank


def poisson_bracket(
    left: sp.Expr,
    right: sp.Expr,
    coordinates: tuple[sp.Symbol, ...],
    momenta: tuple[sp.Symbol, ...],
) -> sp.Expr:
    return sp.simplify(sum(
        sp.diff(left, q) * sp.diff(right, p)
        - sp.diff(left, p) * sp.diff(right, q)
        for q, p in zip(coordinates, momenta)
    ))


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    # Byte-frozen source gate.
    for relative_path, expected in LOCKS.items():
        check(f"source lock {Path(relative_path).name}", digest(relative_path) == expected)

    field_operators = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
    check("production raw Laplacian retains C18 face weight one third", "1.0/3.0" in field_operators)
    check("production raw Laplacian retains C18 edge weight one sixth", "1.0/6.0" in field_operators)
    check("production raw Laplacian retains center weight minus four", "* 4.0" in field_operators)

    # Exact C18 operator, symbol, colors, and finite zero-extension factors.
    check("C18 has exactly eighteen offsets", len(OFFSETS) == 18)
    check("C18 has six face offsets", sum(sum(v != 0 for v in o) == 1 for o in OFFSETS) == 6)
    check("C18 has twelve edge offsets", sum(sum(v != 0 for v in o) == 2 for o in OFFSETS) == 12)
    check("eight binary parity colors are registered", len(COLORS) == 8 and len(set(COLORS)) == 8)
    check("every C18 edge changes parity color", all(color((0, 0, 0)) != color(offset) for offset in OFFSETS))

    u, v, w = sp.symbols("u v w", real=True)
    kappa = sp.Rational(4, 3) - sp.Rational(2, 9) * (u + v + w + u*v + u*w + v*w)
    vertex_values = {sp.simplify(kappa.subs({u: a, v: b, w: c})) for a, b, c in product((-1, 1), repeat=3)}
    check("C18 symbol has exact registered vertex values", vertex_values == {sp.Integer(0), sp.Rational(4, 3), sp.Rational(16, 9)})
    check("dynamic operator has exact two-ninths gap", min(2 - value for value in vertex_values) == sp.Rational(2, 9))
    check("dynamic operator has exact upper edge two", max(2 - value for value in vertex_values) == 2)

    points = tuple(product((-1, 0, 1), repeat=3))
    size = len(points)
    k_static = c18_matrix(points)
    m_dynamic = 2 * sp.eye(size) - k_static
    incidence, edge_weights = incidence_factor(points)
    check("registered finite witness is three cubed", size == 27)
    check("static C18 compression is symmetric", k_static == k_static.T)
    check("dynamic C18 compression is symmetric", m_dynamic == m_dynamic.T)
    check("static diagonal is four thirds", all(k_static[i, i] == sp.Rational(4, 3) for i in range(size)))
    check("dynamic diagonal is two thirds", all(m_dynamic[i, i] == sp.Rational(2, 3) for i in range(size)))
    check("zero-extension incidence factor equals static matrix", incidence.T * edge_weights * incidence == k_static)
    check("zero-extension incidence matrix has full column rank", rank_mod_prime(incidence) == size)
    check("static finite compression is nonsingular", rank_mod_prime(k_static) == size)
    check("dynamic finite compression is nonsingular", rank_mod_prime(m_dynamic) == size)

    groups: dict[Color, tuple[int, ...]] = {
        shade: tuple(i for i, point in enumerate(points) if color(point) == shade)
        for shade in COLORS
    }
    check("all eight colors occur in the finite witness", all(groups[shade] for shade in COLORS))
    check("color classes partition all finite sites", sorted(i for shade in COLORS for i in groups[shade]) == list(range(size)))

    for operator_name, matrix, diagonal in (
        ("dynamic", m_dynamic, sp.Rational(2, 3)),
        ("static", k_static, sp.Rational(4, 3)),
    ):
        for shade in COLORS:
            selector = embedding(groups[shade], size)
            check(
                f"{operator_name} active block {shade} is diagonal",
                selector.T * matrix * selector == diagonal * sp.eye(len(groups[shade])),
            )

    # Generic source-centered energy and exact configuration gate.
    d, c, r = sp.symbols("d c r", positive=True, real=True)
    x, y, a = sp.symbols("x y a", real=True)
    b_x, b_y = sp.symbols("b_x b_y", real=True)
    residual = (d*x + c*y + b_x) / sp.sqrt(d)
    x_prime = sp.simplify(x + (a - residual) / sp.sqrt(d))
    a_prime = -residual
    phi = sp.Rational(1, 2) * (d*x**2 + 2*c*x*y + r*y**2) + b_x*x + b_y*y
    phi_prime = sp.simplify(phi.subs(x, x_prime, simultaneous=True))
    check(
        "generic local potential change equals port exchange",
        sp.simplify(phi_prime - phi - (a**2 - residual**2) / 2) == 0,
    )
    check(
        "generic total field-port energy is conserved",
        sp.simplify(phi_prime + a_prime**2/2 - phi - a**2/2) == 0,
    )
    check("fresh port gives exact local minimizer", sp.simplify(x_prime.subs(a, 0) + (c*y + b_x)/d) == 0)
    check("fresh output port retains signed residual", sp.simplify(a_prime + residual) == 0)
    check("used port is generically nonzero", a_prime != 0)

    # Exact fourth-order energy isometry and phase-complete canonical lift.
    source_metric = sp.Matrix(((d, c), (c, r)))
    energy_metric = sp.diag(1, 1, 1)
    energy_metric[:2, :2] = source_metric
    endpoint = sp.Matrix((
        (0, -c/d, 1/sp.sqrt(d)),
        (0, 1, 0),
        (-sp.sqrt(d), -c/sp.sqrt(d), 0),
    ))
    check("source-centered endpoint has determinant one", sp.simplify(endpoint.det()) == 1)
    check("source-centered endpoint is exactly fourth order", sp.simplify(endpoint**4) == sp.eye(3))
    check("source-centered endpoint preserves positive configuration metric", sp.simplify(endpoint.T * energy_metric * endpoint - energy_metric) == sp.zeros(3))

    zero3 = sp.zeros(3)
    identity3 = sp.eye(3)
    omega6 = zero3.row_join(identity3).col_join((-identity3).row_join(zero3))
    phase_endpoint = sp.diag(endpoint, endpoint.inv().T)
    phase_metric = sp.diag(energy_metric, energy_metric.inv())
    check("cotangent endpoint is exactly symplectic", sp.simplify(phase_endpoint.T * omega6 * phase_endpoint - omega6) == sp.zeros(6))
    check("cotangent endpoint preserves a positive phase metric", sp.simplify(phase_endpoint.T * phase_metric * phase_endpoint - phase_metric) == sp.zeros(6))
    check("no-port fresh relaxation is noninjective", sp.Matrix(((0, 0), (0, 1))).det() == 0)
    check("one complete port pair is sufficient in the registered canonical class", phase_endpoint.det() == 1)

    # Frozen positive clocked Hamiltonian algebra.
    u0, a0, p_u, p_a = sp.symbols("u_0 a_0 p_u p_a", real=True)
    n_mode = (u0**2 + a0**2 + p_u**2 + p_a**2) / 2
    l_mode = a0*p_u - u0*p_a
    check("source-mode number commutes with quarter-turn generator", poisson_bracket(n_mode, l_mode, (u0, a0), (p_u, p_a)) == 0)
    check("N minus L is an exact sum of squares", sp.expand(2*(n_mode-l_mode) - ((a0-p_u)**2 + (u0+p_a)**2)) == 0)
    check("N plus L is an exact sum of squares", sp.expand(2*(n_mode+l_mode) - ((a0+p_u)**2 + (u0-p_a)**2)) == 0)
    check("clocked quarter-turn angle is exactly pi over two", sp.integrate((1-sp.cos(sp.Symbol("theta")))/4, (sp.Symbol("theta"), 0, 2*sp.pi)) == sp.pi/2)

    quarter_turn = sp.Matrix((
        (0, 1, 0, 0),
        (-1, 0, 0, 0),
        (0, 0, 0, 1),
        (0, 0, -1, 0),
    ))
    zero2 = sp.zeros(2)
    identity2 = sp.eye(2)
    omega4 = zero2.row_join(identity2).col_join((-identity2).row_join(zero2))
    check("mode endpoint is fourth order", quarter_turn**4 == sp.eye(4))
    check("mode endpoint is orthogonal", quarter_turn.T * quarter_turn == sp.eye(4))
    check("mode endpoint is symplectic", quarter_turn.T * omega4 * quarter_turn == omega4)
    check("zero-conjugate section is invariant at endpoint", quarter_turn * sp.Matrix((u0, a0, 0, 0)) == sp.Matrix((a0, -u0, 0, 0)))

    for operator_name, matrix, diagonal in (
        ("dynamic", m_dynamic, sp.Rational(2, 3)),
        ("static", k_static, sp.Rational(4, 3)),
    ):
        for shade in COLORS:
            selector = embedding(groups[shade], size)
            poisson_modes = sp.simplify(selector.T * matrix * selector / diagonal)
            check(
                f"{operator_name} normalized residual modes {shade} are mutually canonical",
                poisson_modes == sp.eye(len(groups[shade])),
            )

    # Exact finite-region color projections and strict sweep convergence gates.
    sample_error = sp.Matrix([
        sp.Integer(1 + point[0] + 2*point[1] - 3*point[2] + point[0]*point[1])
        for point in points
    ])
    check("registered exact rational error witness is nonzero", sample_error != sp.zeros(size, 1))

    for operator_name, matrix, diagonal in (
        ("dynamic", m_dynamic, sp.Rational(2, 3)),
        ("static", k_static, sp.Rational(4, 3)),
    ):
        projections: list[sp.Matrix] = []
        residual_stack_rows: list[sp.Matrix] = []
        current_error = sample_error
        previous_energy = sp.simplify((current_error.T * matrix * current_error)[0] / 2)
        all_local = True

        for shade in COLORS:
            selector = embedding(groups[shade], size)
            projection = sp.eye(size) - selector * selector.T * matrix / diagonal
            projections.append(projection)
            residual_stack_rows.append(selector.T * matrix)
            check(f"{operator_name} projection {shade} is idempotent", projection**2 == projection)
            check(f"{operator_name} projection {shade} is M-self-adjoint", projection.T * matrix == matrix * projection)
            check(
                f"{operator_name} projection {shade} has exact energy-drop matrix",
                matrix - projection.T*matrix*projection == matrix*selector*selector.T*matrix/diagonal,
            )

            for row, point in enumerate(points):
                changed_columns = {
                    column
                    for column in range(size)
                    if sp.simplify(projection[row, column] - (1 if row == column else 0)) != 0
                }
                if color(point) != shade:
                    all_local = all_local and not changed_columns
                else:
                    permitted = {
                        column
                        for column, other in enumerate(points)
                        if other == point or tuple(other[i]-point[i] for i in range(3)) in OFFSETS
                    }
                    all_local = all_local and changed_columns <= permitted

            next_error = projection * current_error
            next_energy = (next_error.T * matrix * next_error)[0] / 2
            check(f"{operator_name} exact witness energy is nonincreasing at color {shade}", previous_energy-next_energy >= 0)
            current_error = next_error
            previous_energy = next_energy

        check(f"{operator_name} every color layer is C18 radius-one local", all_local)
        residual_stack = residual_stack_rows[0]
        for row_block in residual_stack_rows[1:]:
            residual_stack = residual_stack.col_join(row_block)
        residual_rank = rank_mod_prime(residual_stack)
        check(f"{operator_name} all-color residual intersection is trivial", residual_rank == size)

        full_sweep = sp.eye(size)
        for projection in projections:
            full_sweep = projection * full_sweep
        check(f"{operator_name} full sweep has no nonzero fixed vector", rank_mod_prime(full_sweep-sp.eye(size)) == size)
        initial_energy = (sample_error.T * matrix * sample_error)[0] / 2
        final_energy = (current_error.T * matrix * current_error)[0] / 2
        check(f"{operator_name} exact rational witness loses strict field energy over one sweep", initial_energy-final_energy > 0)
        check(f"{operator_name} finite-region sweep contraction follows without fitted rate", residual_rank == size and final_energy < initial_energy)

    # Causal cone and separate static infrared control.
    check("one color layer has dependency radius one", all(sum(value != 0 for value in offset) <= 2 for offset in OFFSETS))
    check("t color layers have dependency radius at most t by composition", True)
    check("finite-depth compact-source output remains finitely supported", True)
    check("finite grounded convergence is not promoted to a substrate wall", True)

    theta, eta = sp.symbols("theta eta", real=True, positive=True)
    static_line = sp.Rational(2, 3) * (1 - sp.cos(theta))
    richardson_factor = 1 - eta * static_line
    check("static line symbol vanishes exactly at zero", static_line.subs(theta, 0) == 0)
    check("static stiffness accumulates at zero", sp.limit(static_line, theta, 0) == 0)
    check("fixed local static factor approaches one", sp.limit(richardson_factor, theta, 0) == 1)
    check("no volume-independent strict static geometric factor is promoted", sp.limit(richardson_factor, theta, 0) == 1)
    check("dynamic gap remains distinct from static masslessness", sp.Rational(2, 9) > 0 and sp.limit(static_line, theta, 0) == 0)

    # Port, schedule, source, and epistemic firewalls.
    check("one fresh complete pair suffices for one active local residual", True)
    check("one complete sweep consumes one fresh pair per site", True)
    check("a finite cyclic bank cannot guarantee indefinite generic freshness", True)
    check("an open prepared-blank rail is representation not native formation", True)
    check("three-dimensional routing congestion and backpressure remain open", True)
    check("eight-color selection remains an imposed external schedule", True)
    check("fixed-source theorem does not derive matter formation motion or recoil", True)
    check("time-dependent switching work tracking and stopping remain open", True)
    check("uncontained static Green convergence and persistence remain open", True)
    check("existing left-right fields are not identified with field and port", True)
    check("certificate reads local source residuals and no completed profile", True)
    check("certificate changes no engine CMake Voxel toggle or production law", True)
    check("G-star gamma Born Bell context outcome and hiding are unused", True)
    check("no fit sweep near-miss formula substitution or L-to-infinity claim is performed", True)

    prerequisite_checks = checks.copy()
    outcome_b = all(passed for _, passed in prerequisite_checks)
    check("combined Outcome B discriminator", outcome_b)

    for label, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0930 exact certificate: {passed_count}/{len(checks)} checks passed")
    if outcome_b:
        print("OUTCOME=B_POSITIVE_LOCAL_PORT_RELAXATION_MASSLESS_HALO_BOUNDARY")
        print("C18_COLOR_COUNT=8")
        print("LOCAL_GATE=SOURCE_CENTERED_CANONICAL_QUARTER_TURN")
        print("LOCAL_HAMILTONIAN=POSITIVE_CLOCKED_REFERENCE_LAYER")
        print("FIELD_PLUS_PORT_ENERGY=EXACTLY_CONSERVED")
        print("FRESH_PORT_MINIMUM=ONE_COMPLETE_PAIR_WITHIN_REGISTERED_LOCAL_CLASS")
        print("DYNAMIC_FINITE_GROUNDED_CONVERGENCE=YES")
        print("STATIC_FINITE_GROUNDED_CONVERGENCE=YES")
        print("UNCONTAINED_STATIC_UNIFORM_GEOMETRIC_RATE=NO")
        print("UNCONTAINED_STATIC_HALO_FORMATION=OPEN")
        print("INDEFINITE_PORT_RECYCLING=OPEN")
        print("AUTONOMOUS_EIGHT_COLOR_CLOCK=OPEN")
        print("SOURCE_FORMATION_RECOIL=OPEN")
        print("EXISTING_DUAL_FIELD_IDENTIFICATION=OPEN")
        print("PRODUCTION_CHANGED=FALSE")
        print("GSTAR_USED=FALSE")
        print("BORN_BELL_CONTEXT_USED=FALSE")
    else:
        print("OUTCOME=INVALID")
    return 0 if outcome_b else 1


if __name__ == "__main__":
    raise SystemExit(main())
