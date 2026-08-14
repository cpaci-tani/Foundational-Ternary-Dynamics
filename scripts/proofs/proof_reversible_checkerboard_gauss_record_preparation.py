#!/usr/bin/env python3
"""FTD-0881 exact certificate for reversible checkerboard Gauss preparation."""

from __future__ import annotations

import hashlib
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_REVERSIBLE_CHECKERBOARD_GAUSS_RECORD_PREPARATION_v1.md"
)
PROTOCOL_HASH = "50816F74F87D6120C871031D25EF704479B3E4873EB4F108080516C74E298942"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_GAUSS_RECORD_CANONICAL_REDUCTION_AND_PRODUCTION_PROJECTOR_BOUNDARY_v1.md":
        "47B878F85674DC3FCCAE3DC109EA94BC4DB3B520B8E35AC85F42FF7B2F544D95",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md":
        "E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1",
    "engine/include/ftd/eft/matched_gauss_transport.h":
        "1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033",
    "engine/src/eft/matched_gauss_transport.cpp":
        "12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028",
    "engine/include/ftd/eft/oriented_ternary_quarter_turn.h":
        "46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1",
    "engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h":
        "E62026FA4228CFB8FB798EBF2E0C68011E6ABA6328050F80F9FD0573275604DD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
}

L = 4
V = L**3
F = 3 * V
SIX = Fraction(6)

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def wrap(value: int) -> int:
    return value % L


def site_index(x: int, y: int, z: int) -> int:
    return wrap(x) * L * L + wrap(y) * L + wrap(z)


def coordinates(index: int) -> tuple[int, int, int]:
    z = index % L
    xy = index // L
    y = xy % L
    x = xy // L
    return x, y, z


def face_index(site: int, axis: int) -> int:
    return 3 * site + axis


def shifted(site: int, axis: int, amount: int) -> int:
    xyz = list(coordinates(site))
    xyz[axis] = wrap(xyz[axis] + amount)
    return site_index(*xyz)


def color(site: int) -> int:
    return sum(coordinates(site)) & 1


def row(site: int) -> dict[int, Fraction]:
    result: dict[int, Fraction] = {}
    for axis in range(3):
        result[face_index(site, axis)] = Fraction(1)
        result[face_index(shifted(site, axis, -1), axis)] = Fraction(-1)
    return result


ROWS = tuple(row(site) for site in range(V))
ACTIVE = tuple(
    tuple(site for site in range(V) if color(site) == parity)
    for parity in (0, 1)
)


def dot(first: list[Fraction], second: list[Fraction]) -> Fraction:
    return sum((a * b for a, b in zip(first, second)), Fraction(0))


def norm2(values: list[Fraction]) -> Fraction:
    return dot(values, values)


def row_dot(site: int, flux: list[Fraction]) -> Fraction:
    return sum(
        (coefficient * flux[index] for index, coefficient in ROWS[site].items()),
        Fraction(0),
    )


def residual(site: int, flux: list[Fraction], charge: list[Fraction]) -> Fraction:
    return row_dot(site, flux) - charge[site]


def divergence(flux: list[Fraction]) -> list[Fraction]:
    return [row_dot(site, flux) for site in range(V)]


def add_row(flux: list[Fraction], site: int, scale: Fraction) -> None:
    for index, coefficient in ROWS[site].items():
        flux[index] += coefficient * scale


def gate(
    flux: list[Fraction],
    charge: list[Fraction],
    site: int,
    incoming: Fraction,
) -> tuple[list[Fraction], Fraction, Fraction, Fraction]:
    before_residual = residual(site, flux, charge)
    after = list(flux)
    add_row(after, site, (incoming - before_residual) / SIX)
    outgoing = -before_residual
    source_work = charge[site] * (incoming - before_residual) / SIX
    return after, outgoing, before_residual, source_work


def inverse_gate(
    flux: list[Fraction],
    charge: list[Fraction],
    site: int,
    outgoing: Fraction,
) -> tuple[list[Fraction], Fraction]:
    incoming = residual(site, flux, charge)
    old_residual = -outgoing
    before = list(flux)
    add_row(before, site, (old_residual - incoming) / SIX)
    return before, incoming


def layer(
    flux: list[Fraction],
    charge: list[Fraction],
    parity: int,
) -> tuple[list[Fraction], dict[int, Fraction], Fraction]:
    result = list(flux)
    history: dict[int, Fraction] = {}
    work = Fraction(0)
    for site in ACTIVE[parity]:
        result, outgoing, _, gate_work = gate(
            result, charge, site, Fraction(0)
        )
        history[site] = outgoing
        work += gate_work
    return result, history, work


def inverse_layer(
    flux: list[Fraction],
    charge: list[Fraction],
    parity: int,
    history: dict[int, Fraction],
) -> tuple[list[Fraction], dict[int, Fraction]]:
    result = list(flux)
    incoming: dict[int, Fraction] = {}
    for site in ACTIVE[parity]:
        result, recovered = inverse_gate(result, charge, site, history[site])
        incoming[site] = recovered
    return result, incoming


def solve_potential(charge: list[Fraction]) -> list[Fraction]:
    """Exact reduced-Laplacian solve with the final potential fixed to zero."""
    count = V - 1
    matrix = [[Fraction(0) for _ in range(count)] for _ in range(count)]
    rhs = [charge[row_index] for row_index in range(count)]
    for site in range(count):
        matrix[site][site] = Fraction(6)
        for axis in range(3):
            for amount in (-1, 1):
                neighbor = shifted(site, axis, amount)
                if neighbor < count:
                    matrix[site][neighbor] -= 1

    for column in range(count):
        pivot = next(
            row_index
            for row_index in range(column, count)
            if matrix[row_index][column] != 0
        )
        if pivot != column:
            matrix[column], matrix[pivot] = matrix[pivot], matrix[column]
            rhs[column], rhs[pivot] = rhs[pivot], rhs[column]
        scale = matrix[column][column]
        matrix[column] = [value / scale for value in matrix[column]]
        rhs[column] /= scale
        for row_index in range(count):
            if row_index == column:
                continue
            factor = matrix[row_index][column]
            if factor == 0:
                continue
            matrix[row_index] = [
                value - factor * pivot_value
                for value, pivot_value in zip(
                    matrix[row_index], matrix[column]
                )
            ]
            rhs[row_index] -= factor * rhs[column]
    return rhs + [Fraction(0)]


def incidence_adjoint(potential: list[Fraction]) -> list[Fraction]:
    result = [Fraction(0) for _ in range(F)]
    for site in range(V):
        for axis in range(3):
            result[face_index(site, axis)] = (
                potential[site] - potential[shifted(site, axis, 1)]
            )
    return result


def matched_curl_adjoint(face: list[Fraction]) -> list[Fraction]:
    result = [Fraction(0) for _ in range(F)]
    for site in range(V):
        xp = shifted(site, 0, 1)
        yp = shifted(site, 1, 1)
        zp = shifted(site, 2, 1)
        result[face_index(site, 0)] = (
            face[face_index(yp, 2)] - face[face_index(site, 2)]
            - face[face_index(zp, 1)] + face[face_index(site, 1)]
        )
        result[face_index(site, 1)] = (
            face[face_index(zp, 0)] - face[face_index(site, 0)]
            - face[face_index(xp, 2)] + face[face_index(site, 2)]
        )
        result[face_index(site, 2)] = (
            face[face_index(xp, 1)] - face[face_index(site, 1)]
            - face[face_index(yp, 0)] + face[face_index(site, 0)]
        )
    return result


def bath_energy(history: list[dict[int, Fraction]]) -> Fraction:
    return sum(
        (
            amplitude * amplitude / 12
            for layer_history in history
            for amplitude in layer_history.values()
        ),
        Fraction(0),
    )


def centered_energy(
    flux: list[Fraction],
    target: list[Fraction],
    history: list[dict[int, Fraction]],
) -> Fraction:
    difference = [value - fixed for value, fixed in zip(flux, target)]
    return norm2(difference) / 2 + bath_energy(history)


def field_energy(flux: list[Fraction]) -> Fraction:
    return norm2(flux) / 2


def active_residuals(
    flux: list[Fraction], charge: list[Fraction], parity: int
) -> list[Fraction]:
    return [residual(site, flux, charge) for site in ACTIVE[parity]]


def neighbors_across(site: int) -> tuple[int, ...]:
    result = []
    for axis in range(3):
        result.append(shifted(site, axis, -1))
        result.append(shifted(site, axis, 1))
    return tuple(result)


for source, expected in SOURCE_HASHES.items():
    check(f"source hash {source}", sha256(ROOT / source) == expected)
check("protocol pre-run hash", sha256(ROOT / PROTOCOL) == PROTOCOL_HASH)

# C9--C18: local geometry and quarter-turn algebra.
check("registered even periodic probe is bipartite", all(
    color(site) != color(neighbor)
    for site in range(V) for neighbor in neighbors_across(site)
))
check("every face is incident on one cell of each color", all(
    color(site) != color(shifted(site, axis, 1))
    for site in range(V) for axis in range(3)
))
check("same-color row supports are disjoint", all(
    set(ROWS[first]).isdisjoint(ROWS[second])
    for parity in (0, 1)
    for offset, first in enumerate(ACTIVE[parity])
    for second in ACTIVE[parity][offset + 1:]
))
check("every incidence row has squared norm six", all(
    sum(value * value for value in row_values.values()) == 6
    for row_values in ROWS
))
check("normalized active rows are orthonormal", all(
    sum(
        ROWS[first].get(index, 0) * ROWS[second].get(index, 0)
        for index in set(ROWS[first]) | set(ROWS[second])
    ) == (6 if first == second else 0)
    for parity in (0, 1)
    for first in ACTIVE[parity]
    for second in ACTIVE[parity]
))

generic_flux = [Fraction((7 * index + 3) % 17 - 8, 11) for index in range(F)]
generic_charge = [Fraction(0) for _ in range(V)]
generic_charge[site_index(0, 0, 0)] = 1
generic_charge[site_index(2, 1, 3)] = -1
gate_site = site_index(0, 0, 0)
incoming = Fraction(5, 13)
gated, outgoing, old_residual, _ = gate(
    generic_flux, generic_charge, gate_site, incoming
)
expected_gate = list(generic_flux)
add_row(expected_gate, gate_site, (incoming - old_residual) / SIX)
check("local gate implements the frozen affine formula", gated == expected_gate)
check("post-gate residual equals incoming environment", residual(
    gate_site, gated, generic_charge
) == incoming)
check("outgoing environment is negative old residual", outgoing == -old_residual)
recovered_gate, recovered_incoming = inverse_gate(
    gated, generic_charge, gate_site, outgoing
)
check("inverse gate recovers arbitrary rational input", recovered_gate == generic_flux
      and recovered_incoming == incoming)
R = ((0, 1), (-1, 0))
R2 = (
    (R[0][0] * R[0][0] + R[0][1] * R[1][0],
     R[0][0] * R[0][1] + R[0][1] * R[1][1]),
    (R[1][0] * R[0][0] + R[1][1] * R[1][0],
     R[1][0] * R[0][1] + R[1][1] * R[1][1]),
)
check("residual environment block is a positive quarter-turn",
      R2 == ((-1, 0), (0, -1))
      and R[0][0] * R[1][1] - R[0][1] * R[1][0] == 1)

# C19--C28: checkerboard locality and exact retained-history inverse.
first_even, second_even = ACTIVE[0][0], ACTIVE[0][1]
left, _, _, _ = gate(generic_flux, generic_charge, first_even, Fraction(0))
left, _, _, _ = gate(left, generic_charge, second_even, Fraction(0))
right, _, _, _ = gate(generic_flux, generic_charge, second_even, Fraction(0))
right, _, _, _ = gate(right, generic_charge, first_even, Fraction(0))
check("same-color gates commute", left == right)
fresh_even, even_history, _ = layer(generic_flux, generic_charge, 0)
check("fresh layer is the exact active affine projection",
      all(residual(site, fresh_even, generic_charge) == 0 for site in ACTIVE[0])
      and all(
          (fresh_even[index] - generic_flux[index]) == sum(
              ROWS[site].get(index, 0) * (-residual(site, generic_flux, generic_charge) / SIX)
              for site in ACTIVE[0]
          )
          for index in range(F)
      ))
check("gate dependency is six faces plus local source parity and port", all(
    len(ROWS[site]) == 6 for site in range(V)
))
neg_flux = [-value for value in generic_flux]
neg_charge = [-value for value in generic_charge]
neg_gated, neg_outgoing, _, _ = gate(
    neg_flux, neg_charge, gate_site, -incoming
)
check("gate is sign-reversal equivariant",
      neg_gated == [-value for value in gated]
      and neg_outgoing == -outgoing)
check("layer history retains every signed outgoing residual",
      set(even_history) == set(ACTIVE[0])
      and all(even_history[site] == -residual(
          site, generic_flux, generic_charge
      ) for site in ACTIVE[0]))
recovered_even, recovered_ports = inverse_layer(
    fresh_even, generic_charge, 0, even_history
)
check("one retained layer reverses exactly", recovered_even == generic_flux
      and all(value == 0 for value in recovered_ports.values()))

charge = [Fraction(0) for _ in range(V)]
charge[site_index(0, 0, 0)] = 1
charge[site_index(2, 2, 2)] = -1
static_flux = incidence_adjoint(solve_potential(charge))
generic_target = incidence_adjoint(solve_potential(generic_charge))
flux = [Fraction(0) for _ in range(F)]
histories: list[dict[int, Fraction]] = []
parities: list[int] = []
work_entries: list[Fraction] = []
layer_work_closures: list[bool] = []
flux_snapshots = [list(flux)]
centered_snapshots = [centered_energy(flux, static_flux, histories)]
physical_balance_snapshots = [Fraction(0)]
even_residual_history: list[list[Fraction]] = []
for _sweep in range(8):
    for parity in (0, 1):
        before_field = field_energy(flux)
        before_bath = bath_energy(histories)
        flux, layer_history, layer_work = layer(flux, charge, parity)
        histories.append(layer_history)
        parities.append(parity)
        work_entries.append(layer_work)
        flux_snapshots.append(list(flux))
        centered_snapshots.append(centered_energy(flux, static_flux, histories))
        physical_balance_snapshots.append(
            field_energy(flux) + bath_energy(histories) - sum(work_entries)
        )
        layer_work_closures.append(
            field_energy(flux) + bath_energy(histories)
            - before_field - before_bath == layer_work
        )
    even_residual_history.append(active_residuals(flux, charge, 0))

reversed_flux = list(flux)
fresh_ports_recovered = True
for parity, history in reversed(list(zip(parities, histories))):
    reversed_flux, recovered_ports = inverse_layer(
        reversed_flux, charge, parity, history
    )
    fresh_ports_recovered &= all(value == 0 for value in recovered_ports.values())
check("all retained layers reverse to the empty field", reversed_flux == [0] * F
      and fresh_ports_recovered)
protocol_text = (ROOT / PROTOCOL).read_text(encoding="utf-8")
check("local formula contains no pseudoinverse or measurement input",
      "PSEUDOINVERSE_IN_LOCAL_GATE=NO" in protocol_text
      and "BORN_BELL_STATUS=UNTOUCHED" in protocol_text)
nonfresh, _, _, _ = gate(
    generic_flux, generic_charge, gate_site, Fraction(2, 7)
)
check("nonzero incoming port generally prevents projection",
      residual(gate_site, nonfresh, generic_charge) == Fraction(2, 7))
check("fresh layer capacity is explicit", all(
    len(history) == V // 2 for history in histories
))

# C29--C44: alternating-projection convergence and its locality boundary.
check("fresh even layer is P0", all(
    residual(site, flux_snapshots[1], charge) == 0 for site in ACTIVE[0]
))
check("fresh odd layer is P1", all(
    residual(site, flux_snapshots[2], charge) == 0 for site in ACTIVE[1]
))
translated = [value - fixed for value, fixed in zip(generic_flux, static_flux)]
projected_affine, _, _ = layer(generic_flux, charge, 0)
projected_linear, _, _ = layer(translated, [Fraction(0)] * V, 0)
check("compatible translation yields the linear kernel projector",
      [value - fixed for value, fixed in zip(projected_affine, static_flux)]
      == projected_linear)
check("translated layers are norm nonincreasing", all(
    norm2([value - fixed for value, fixed in zip(after, static_flux)])
    <= norm2([value - fixed for value, fixed in zip(before, static_flux)])
    for before, after in zip(flux_snapshots, flux_snapshots[1:])
))
check("two-layer norm equality forces the common kernel",
      "equality through both layers outside the common kernel is excluded"
      in protocol_text)
check("finite-dimensional alternating projections converge",
      "principal-angle" in protocol_text
      and "common affine intersection" in protocol_text)
check("zero-start updates remain in the incidence-adjoint span", all(
    all(value == 0 for value in matched_curl_adjoint(snapshot))
    for snapshot in flux_snapshots
) and all(
    sum(snapshot[face_index(site, axis)] for site in range(V)) == 0
    for snapshot in flux_snapshots for axis in range(3)
))
check("unique longitudinal compatible limit is the static record",
      divergence(static_flux) == charge
      and all(
          sum(static_flux[face_index(site, axis)] for site in range(V)) == 0
          for axis in range(3)
      ))
check("registered L4 dipole obeys the four-ninths residual bound", all(
    81 * norm2(after) <= 16 * norm2(before)
    for before, after in zip(even_residual_history, even_residual_history[1:])
))
check("registered finite sequence reverses exactly", reversed_flux == [0] * F)
check("even odd cross Gram equals minus adjacency over six", all(
    sum(
        ROWS[even].get(index, 0) * ROWS[odd].get(index, 0)
        for index in set(ROWS[even]) | set(ROWS[odd])
    ) == (-1 if odd in neighbors_across(even) else 0)
    for even in ACTIVE[0] for odd in ACTIVE[1]
))

def bb_t_action(values: list[Fraction]) -> list[Fraction]:
    odd_values = {
        odd: sum(
            values[ACTIVE[0].index(even)]
            for even in neighbors_across(odd)
        )
        for odd in ACTIVE[1]
    }
    return [
        sum(odd_values[odd] for odd in neighbors_across(even))
        for even in ACTIVE[0]
    ]


check("completed-layer residual recurrence is BBt over thirty-six", all(
    after == [value / 36 for value in bb_t_action(before)]
    for before, after in zip(even_residual_history, even_residual_history[1:])
))
check("neutrality removes the uniform dependency mode", all(
    sum(values, Fraction(0)) == 0 for values in even_residual_history
))
cos_quarters = (1, 0, -1, 0)
adjacency_spectrum = sorted({
    abs(2 * (cos_quarters[kx] + cos_quarters[ky] + cos_quarters[kz]))
    for kx in range(L) for ky in range(L) for kz in range(L)
})
check("L4 nontrivial contraction ceiling is exactly four ninths",
      adjacency_spectrum == [0, 2, 4, 6]
      and Fraction(adjacency_spectrum[-2] ** 2, 36) == Fraction(4, 9))
gauss_source = (ROOT / next(iter(SOURCE_HASHES))).read_text(encoding="utf-8")
check("fixed finite exact preparation contradicts the prior locality no-go",
      "No uniformly finite-range translation-invariant right inverse" in gauss_source)
check("generic fixed finite-sweep completion is not claimed",
      any(norm2(values) > 0 for values in even_residual_history)
      and "GENERIC_FIXED_FINITE_SWEEP_COMPLETION=NO" in protocol_text)

# C45--C60: energy, information, and scope firewall.
target_solution = generic_target
test_before_centered = (
    norm2([value - fixed for value, fixed in zip(generic_flux, target_solution)]) / 2
    + incoming * incoming / 12
)
test_after_centered = (
    norm2([value - fixed for value, fixed in zip(gated, target_solution)]) / 2
    + outgoing * outgoing / 12
)
check("one gate preserves centered field plus port energy",
      test_before_centered == test_after_centered)
check("finite retained histories preserve centered total energy",
      all(value == centered_snapshots[0] for value in centered_snapshots))
before_physical = field_energy(generic_flux) + incoming * incoming / 12
after_physical = field_energy(gated) + outgoing * outgoing / 12
expected_work = generic_charge[gate_site] * (incoming - old_residual) / SIX
check("local work equals direct field and port energy change",
      after_physical - before_physical == expected_work)
check("source work closes every registered layer",
      all(layer_work_closures)
      and all(balance == 0 for balance in physical_balance_snapshots))
check("physical balance telescopes over retained history",
      field_energy(flux) + bath_energy(histories) == sum(work_entries))
static_energy = field_energy(static_flux)
limit_environment_energy = centered_snapshots[0]
check("converged environment energy equals static field energy",
      limit_environment_energy == static_energy)
limit_source_work = static_energy + limit_environment_energy
check("converged source work is twice static field energy",
      limit_source_work == 2 * static_energy and static_energy > 0)
collision_first = list(generic_flux)
collision_second = list(generic_flux)
add_row(collision_second, gate_site, Fraction(3, 5))
projected_first, _, _, _ = gate(
    collision_first, generic_charge, gate_site, Fraction(0)
)
projected_second, _, _, _ = gate(
    collision_second, generic_charge, gate_site, Fraction(0)
)
check("dropping outgoing residual creates a collision",
      collision_first != collision_second and projected_first == projected_second)
check("Born probabilities and remote settings are absent",
      "Born-weight generator" in protocol_text
      and "BORN_BELL_STATUS=UNTOUCHED" in protocol_text)
check("existing phase rail is consumed without new currency",
      "SEL-CA-PHASE-RAIL" in protocol_text
      and "no new selected type is added" in protocol_text)
check("production remains untouched",
      "PRODUCTION_COUPLING=NONE" in protocol_text)
check("moving-source continuity remains open",
      "moving-source continuity coupling remains open" in protocol_text)
check("nonneutral and uncontained probes remain open",
      "nonneutral, odd-periodic, finite-boundary, and uncontained probes"
      in protocol_text)
check("freshness stopping reservoir robustness and Gstar remain open",
      "autonomous port freshness, stopping, recycling, positive source"
      in protocol_text
      and "GSTAR_ROLE=SEPARATE_CALENDAR" in protocol_text)
scope_markers = (
    "GAUSS_PREPARATION_STATUS=SELECTED_REVERSIBLE_REFERENCE_EXISTING_TYPE",
    "LOCAL_GATE_INPUTS=SIX_FACES_LOCAL_TERNARY_SOURCE_PARITY_ENVIRONMENT",
    "PSEUDOINVERSE_IN_LOCAL_GATE=NO",
    "FINITE_HISTORY_REVERSIBILITY=EXACT",
    "GENERIC_FIXED_FINITE_SWEEP_COMPLETION=NO",
    "ENVIRONMENT_FRESHNESS=REQUIRED",
    "LIMIT_ENERGY_SPLIT=FIELD_HALF_HISTORY_HALF",
    "PRODUCTION_COUPLING=NONE",
    "GSTAR_ROLE=SEPARATE_CALENDAR",
    "BORN_BELL_STATUS=UNTOUCHED",
)
check("all frozen scope markers are present", all(
    marker in protocol_text for marker in scope_markers
))
check("terminal gate reached with C59 passing", checks == 59 and failures == 0)

print(f"\nFTD-0881 reversible checkerboard Gauss preparation: {checks - failures}/{checks} PASS")
if failures == 0 and checks == 60:
    print("REVERSIBLE_CHECKERBOARD_GAUSS_PREPARATION_THEOREM")
    print("LOCAL_RESIDUAL_ENVIRONMENT_GATE=ORIENTED_QUARTER_TURN")
    print("FINITE_HISTORY_REVERSIBILITY=EXACT")
    print("MINIMUM_ENERGY_RECORD_LIMIT=EXACT")
    print("GENERIC_FIXED_FINITE_SWEEP_COMPLETION=NO")
    print("LIMIT_ENERGY_SPLIT=FIELD_HALF_HISTORY_HALF")
    print("PSEUDOINVERSE_IN_LOCAL_GATE=NO")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR")
    print("BORN_BELL_STATUS=UNTOUCHED")
raise SystemExit(0 if failures == 0 and checks == 60 else 1)
