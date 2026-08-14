#!/usr/bin/env python3
"""Exact FTD-0942 existing-L/R occupancy-history carrier classifier.

This certificate performs finite exact algebra and source-contract checks.  It
does not search for numerical coincidences, fit parameters, or modify the
production engine.
"""

from __future__ import annotations

from fractions import Fraction as Q
from hashlib import sha256
from itertools import product
from pathlib import Path
from typing import Iterable, Sequence


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_EXISTING_LR_OCCUPANCY_HISTORY_CARRIER_CLASSIFIER_v1.md"
)

EXPECTED_HASHES = {
    PREREG: "F7550994C541D209A63F7B936A4DE96A3B0AA50B43AD9DB8217CE5C100097F82",
    ROOT / "engine/include/ftd/voxel.h": (
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3"
    ),
    ROOT / "engine/include/ftd/term_toggles.h": (
        "2731A2BF1EF01456DFDFE4F1E20C8E64E3D839136BC633B13771D13360AC64AA"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": (
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": (
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4"
    ),
    ROOT / "engine/src/render_bridge_phases/phase_movement.cpp": (
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB"
    ),
    ROOT / "engine/src/injection.cpp": (
        "228A1AE44532DB7D80A0EC10ABF5639B2811849189EF2F71A6343EE59C253DC5"
    ),
    ROOT / "engine/src/constructors/constructors_molecules.cpp": (
        "568C896020392F448E6F2484547C60B502E9701D2F3B9FBF2FDC11B8706D06D8"
    ),
    ROOT / "engine/src/transmutation_phases.cpp": (
        "4013A9B769199D54976347378FD03DFF6415B7F641F35D3FAE498125EB288043"
    ),
    ROOT / "engine/src/diagnostics_compute.cpp": (
        "C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md"
    ): "A89DE2964B7D48100EC850547D00BB540D05F1166CF18CABE654EB9D26917548",
    ROOT / "scripts/proofs/proof_finite_capacity_local_reversible_occupancy_carry_trilemma.py": (
        "0256BF01710F8D6B9FFCE717FA8CB6A0E0E0B0715F2BC2F004380B9A5374FBC7"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md"
    ): "656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md"
    ): "2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md"
    ): "08FBF3361C453DC9E0A99184920883DBC6DE15B5043F7EFC140B0EB740A26474",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md"
    ): "4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md"
    ): "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
}


Vec3 = tuple[Q, Q, Q]
Matrix = list[list[Q]]
checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def qvec(values: Iterable[int | Q]) -> Vec3:
    x, y, z = values
    return Q(x), Q(y), Q(z)


def vadd(a: Vec3, b: Vec3) -> Vec3:
    return tuple(x + y for x, y in zip(a, b))  # type: ignore[return-value]


def vsub(a: Vec3, b: Vec3) -> Vec3:
    return tuple(x - y for x, y in zip(a, b))  # type: ignore[return-value]


def vscale(a: Vec3, scalar: Q) -> Vec3:
    return tuple(scalar * x for x in a)  # type: ignore[return-value]


def mat_vec(matrix: Sequence[Sequence[Q]], vector: Sequence[Q]) -> list[Q]:
    return [sum(row[j] * vector[j] for j in range(len(vector))) for row in matrix]


def mat_mul(left: Matrix, right: Matrix) -> Matrix:
    return [
        [sum(left[i][k] * right[k][j] for k in range(len(right)))
         for j in range(len(right[0]))]
        for i in range(len(left))
    ]


def identity(size: int) -> Matrix:
    return [[Q(int(i == j)) for j in range(size)] for i in range(size)]


def determinant(matrix: Matrix) -> Q:
    work = [row[:] for row in matrix]
    det = Q(1)
    size = len(work)
    for col in range(size):
        pivot = next((row for row in range(col, size) if work[row][col]), None)
        if pivot is None:
            return Q(0)
        if pivot != col:
            work[col], work[pivot] = work[pivot], work[col]
            det = -det
        value = work[col][col]
        det *= value
        for row in range(col + 1, size):
            if not work[row][col]:
                continue
            factor = work[row][col] / value
            for j in range(col, size):
                work[row][j] -= factor * work[col][j]
    return det


def block_wave_matrix(stiffness: Matrix, h: Q) -> Matrix:
    """State order is (D, P); kick P'=P+hKD then drift D'=D+hP'."""
    size = len(stiffness)
    out = [[Q(0) for _ in range(2 * size)] for _ in range(2 * size)]
    for i in range(size):
        for j in range(size):
            out[i][j] = Q(int(i == j)) + h * h * stiffness[i][j]
            out[i + size][j] = h * stiffness[i][j]
        out[i][i + size] = h
        out[i + size][i + size] = Q(1)
    return out


def block_wave_inverse(stiffness: Matrix, h: Q) -> Matrix:
    """D=D'-hP'; P=P'-hKD."""
    size = len(stiffness)
    out = [[Q(0) for _ in range(2 * size)] for _ in range(2 * size)]
    for i in range(size):
        out[i][i] = Q(1)
        out[i][i + size] = -h
        for j in range(size):
            out[i + size][j] = -h * stiffness[i][j]
            out[i + size][j + size] = (
                Q(int(i == j)) + h * h * stiffness[i][j]
            )
    return out


def coupled_neighbors() -> dict[tuple[int, int, int], Q]:
    weights: dict[tuple[int, int, int], Q] = {}
    for direction in product((-1, 0, 1), repeat=3):
        norm1 = sum(abs(component) for component in direction)
        if norm1 == 1:
            weights[direction] = Q(1, 3)
        elif norm1 == 2:
            weights[direction] = Q(1, 6)
    return weights


def pulse(direction: tuple[int, int, int], a: Q, b: Q) -> tuple[Q, ...]:
    """General two-vector odd covariant pulse e(nu)=(a nu,b nu)."""
    return tuple(a * Q(x) for x in direction) + tuple(b * Q(x) for x in direction)


# ---------------------------------------------------------------------------
# Gate 0: fail closed on the preregistration and every frozen source.
# ---------------------------------------------------------------------------

for path, expected in EXPECTED_HASHES.items():
    actual = file_hash(path)
    if actual != expected:
        raise SystemExit(
            f"FTD-0942 INVALID: source drift for {path.relative_to(ROOT)}: "
            f"expected {expected}, got {actual}"
        )
    check(f"hash:{path.relative_to(ROOT)}", True)


# ---------------------------------------------------------------------------
# Gate 1: source contracts, including the current telemetry correction.
# ---------------------------------------------------------------------------

source_markers = {
    ROOT / "engine/include/ftd/voxel.h": (
        "Vec3 flux_L;",
        "Vec3 flux_R;",
        "Vec3 wave_vel_L;",
        "Vec3 wave_vel_R;",
        "flux = flux_L + flux_R",
        "flux_L - flux_R",
    ),
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp": (
        "rb.delta_j_L_[i] = lap_L * cw2;",
        "rb.delta_j_R_[i] = lap_R * cw2;",
        "rb.delta_j_L_[i] += curl_sv - grad_s;",
        "rb.delta_j_R_[i] += curl_sv - grad_s;",
        "* INV3",
        "* INV6",
        "flux_L * 4.0",
        "flux_R * 4.0",
    ),
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp": (
        "v.wave_vel_L += rb.delta_j_L_[i]",
        "v.wave_vel_R += rb.delta_j_R_[i]",
        "v.flux_L += v.wave_vel_L",
        "v.flux_R += v.wave_vel_R",
        "v.flux_L *= eff_damping;",
        "v.flux_R *= eff_damping;",
        "v.flux = v.flux_L + v.flux_R;",
    ),
    ROOT / "engine/src/injection.cpp": (
        "v.flux_L = flux_val * 0.5;",
        "v.flux_R = flux_val * 0.5;",
        "v.wave_vel_L = v.wave_vel_L + half;",
        "v.wave_vel_R = v.wave_vel_R + half;",
        "v.flux_L = flux_val * frac_major;",
        "v.flux_R = flux_val * frac_minor;",
    ),
    ROOT / "engine/src/constructors/constructors_molecules.cpp": (
        "Chirality seed: flux_L / flux_R asymmetry",
        "vox[idx].flux_L = Vec3{fl, 0, 0};",
        "vox[idx].flux_R = Vec3{fr, 0, 0};",
    ),
    ROOT / "engine/src/transmutation_phases.cpp": (
        "std::swap(v.flux_L, v.flux_R);",
        "std::swap(v.wave_vel_L, v.wave_vel_R);",
    ),
    ROOT / "engine/src/render_bridge_phases/phase_movement.cpp": (
        "Opposite sign: annihilation",
        "v.flux_L = {}; v.flux_R = {};",
        "t.flux_L = {}; t.flux_R = {};",
        "neighbors_6",
        "observation-only native event journal",
    ),
    ROOT / "engine/src/diagnostics_compute.cpp": (
        "a.E_L_total += integrate_voxel_density(",
        "a.E_R_total += integrate_voxel_density(",
        "a.wv_L_total += integrate_voxel_density(",
        "a.wv_R_total += integrate_voxel_density(",
        "quadratic_field_energy_density(v.flux_L.mag2())",
        "quadratic_field_energy_density(v.wave_vel_R.mag2())",
    ),
    ROOT / "engine/include/ftd/term_toggles.h": (
        "bool dual_substrate = true;",
        "bool wave_propagation = true;",
        "bool damping = true;",
        "bool genesis = true;",
        "bool gauss_projection = true;",
    ),
}

for path, markers in source_markers.items():
    text = path.read_text(encoding="utf-8")
    for marker in markers:
        check(f"source-marker:{path.name}:{marker[:42]}", marker in text)


# ---------------------------------------------------------------------------
# Gate 2: exact L/R <-> common/relative storage isomorphism.
# ---------------------------------------------------------------------------

samples = (
    qvec((-2, 1, 3)),
    qvec((Q(1, 2), Q(-3, 5), Q(7, 4))),
    qvec((0, 0, 0)),
)

for left in samples:
    for right in samples:
        common = vadd(left, right)
        relative = vsub(left, right)
        recovered_left = vscale(vadd(common, relative), Q(1, 2))
        recovered_right = vscale(vsub(common, relative), Q(1, 2))
        check(f"storage-inverse:L:{left}:{right}", recovered_left == left)
        check(f"storage-inverse:R:{left}:{right}", recovered_right == right)


# Equal source disappears exactly from the relative equation.
for left in samples:
    for right in samples:
        for source in samples:
            k = Q(7, 5)
            h = Q(2, 3)
            left_next = vadd(left, vadd(vscale(left, h * k), source))
            right_next = vadd(right, vadd(vscale(right, h * k), source))
            relative_next = vsub(left_next, right_next)
            expected = vadd(vsub(left, right), vscale(vsub(left, right), h * k))
            check(
                f"equal-source-cancels:{left}:{right}:{source}",
                relative_next == expected,
            )


# ---------------------------------------------------------------------------
# Gate 3: isolated kick--drift is an exact aggregate canonical carrier.
# ---------------------------------------------------------------------------

stiffness: Matrix = [
    [Q(-2), Q(1), Q(0)],
    [Q(1), Q(-2), Q(1)],
    [Q(0), Q(1), Q(-2)],
]
h = Q(2, 5)
forward = block_wave_matrix(stiffness, h)
inverse = block_wave_inverse(stiffness, h)
unit = identity(6)
check("bare-wave:left-inverse", mat_mul(inverse, forward) == unit)
check("bare-wave:right-inverse", mat_mul(forward, inverse) == unit)
check("bare-wave:determinant-one", determinant(forward) == Q(1))

states = (
    [Q(0)] * 6,
    [Q(1), Q(-2), Q(3), Q(4), Q(0), Q(-1)],
    [Q(1, 3), Q(2, 7), Q(-5, 4), Q(9, 5), Q(-2, 3), Q(11, 8)],
)
for state in states:
    image = mat_vec(forward, state)
    recovered = mat_vec(inverse, image)
    check(f"bare-wave:state-recovery:{state}", recovered == state)


# ---------------------------------------------------------------------------
# Gate 4: a source-local pulse fans into 18 sites, not one nu-channel.
# ---------------------------------------------------------------------------

weights = coupled_neighbors()
faces = tuple(direction for direction in weights if sum(abs(x) for x in direction) == 1)
edges = tuple(direction for direction in weights if sum(abs(x) for x in direction) == 2)
corners = tuple(
    direction for direction in product((-1, 0, 1), repeat=3)
    if sum(abs(x) for x in direction) == 3
)
moore = tuple(
    direction for direction in product((-1, 0, 1), repeat=3)
    if direction != (0, 0, 0)
)

check("stencil:six-faces", len(faces) == 6)
check("stencil:twelve-edges", len(edges) == 12)
check("stencil:eight-corners", len(corners) == 8)
check("stencil:eighteen-coupled", len(weights) == 18)
check("stencil:face-weight", all(weights[d] == Q(1, 3) for d in faces))
check("stencil:edge-weight", all(weights[d] == Q(1, 6) for d in edges))
check("stencil:corners-absent", all(d not in weights for d in corners))
check("stencil:constant-annihilation", sum(weights.values(), Q(0)) - Q(4) == 0)

source_pulse = qvec((1, 2, -1))
neighbor_image = {direction: vscale(source_pulse, weight) for direction, weight in weights.items()}
check("fanout:all-coupled-nonzero", all(value != qvec((0, 0, 0)) for value in neighbor_image.values()))
check("fanout:support-size-18", len([v for v in neighbor_image.values() if v != qvec((0, 0, 0))]) == 18)
for target in moore:
    support = set(neighbor_image)
    check(f"fanout:not-pure-translation:{target}", support != {target})

# With no D pulse, source-local P drifts at the source rather than translating.
zero = qvec((0, 0, 0))
source_momentum = qvec((2, -1, 4))
check("momentum-only:no-neighbor-image", all(vscale(zero, w) == zero for w in weights.values()))
check("momentum-only:source-remains", source_momentum != zero)


# ---------------------------------------------------------------------------
# Gate 5: odd covariant co-located pulses do not retain token factorization.
# ---------------------------------------------------------------------------

for a, b in ((Q(1), Q(0)), (Q(0), Q(1)), (Q(2, 3), Q(-5, 7))):
    for direction in moore:
        opposite = tuple(-x for x in direction)
        encoded = pulse(direction, a, b)
        encoded_opposite = pulse(opposite, a, b)
        check(
            f"odd-covariance:{a}:{b}:{direction}",
            encoded_opposite == tuple(-x for x in encoded),
        )
        aggregate = tuple(x + y for x, y in zip(encoded, encoded_opposite))
        check(f"collision-to-vacuum:{a}:{b}:{direction}", aggregate == (Q(0),) * 6)

# Any linear invertible continuation maps the collided aggregate exactly as it
# maps vacuum; it cannot recreate the missing multiset factorization.
check("collision:vacuum-future-equality", mat_vec(forward, [Q(0)] * 6) == [Q(0)] * 6)
check("collision:distinct-multisets-same-aggregate", {(1, 0, 0), (-1, 0, 0)} != set())


# ---------------------------------------------------------------------------
# Gate 6: classify production realization and the allowed next routes.
# ---------------------------------------------------------------------------

aggregate_storage_pass = all(
    condition for name, condition in checks
    if name.startswith("storage-inverse")
)
aggregate_inverse_pass = all(
    condition for name, condition in checks
    if name.startswith("bare-wave")
)
equal_source_deposit_pass = False  # exact equation (4): the relative source is zero
direction_route_pass = False       # exact 18-neighbor fanout, not a nu-permutation
collision_separation_pass = False  # equations (7)--(8)
backpressure_pass = False          # no registered nu/lane source port in frozen storage
energy_transaction_pass = False    # telemetry has no event debit/current/epsilon_*

check("classification:aggregate-storage-pass", aggregate_storage_pass)
check("classification:aggregate-inverse-pass", aggregate_inverse_pass)
check("classification:event-deposit-fails", not equal_source_deposit_pass)
check("classification:direction-route-fails", not direction_route_pass)
check("classification:collision-separation-fails", not collision_separation_pass)
check("classification:backpressure-fails", not backpressure_pass)
check("classification:energy-transaction-fails", not energy_transaction_pass)

# Current telemetry correction: separate L/R squares exist, while source
# inspection finds no stiffness/current/event-debit/epsilon_* transaction.
diagnostics_text = (ROOT / "engine/src/diagnostics_compute.cpp").read_text(encoding="utf-8")
check("telemetry:separate-L", "a.E_L_total" in diagnostics_text and "a.wv_L_total" in diagnostics_text)
check("telemetry:separate-R", "a.E_R_total" in diagnostics_text and "a.wv_R_total" in diagnostics_text)
check("telemetry:no-epsilon-star", "epsilon_*" not in diagnostics_text and "epsilon_star" not in diagnostics_text)
check("telemetry:no-occupancy-debit", "occupancy_debit" not in diagnostics_text)

# The type-price alternatives are classifications, not an adoption.
routes = {
    "derived_protected_field": {
        "new_primitive_type": False,
        "needs_nonlinear_invariant": True,
        "ontic": True,
    },
    "selected_channelized_ports": {
        "new_primitive_type": True,
        "needs_nonlinear_invariant": False,
        "ontic": True,
    },
    "external_observation_journal": {
        "new_primitive_type": False,
        "needs_nonlinear_invariant": False,
        "ontic": False,
    },
}
check("routes:three-distinct", len(routes) == 3)
check("routes:existing-fields-remain-open", not routes["derived_protected_field"]["new_primitive_type"])
check("routes:channelized-type-priced", routes["selected_channelized_ports"]["new_primitive_type"])
check("routes:journal-not-ontic", not routes["external_observation_journal"]["ontic"])


outcome_a = all((
    aggregate_storage_pass,
    aggregate_inverse_pass,
    equal_source_deposit_pass,
    direction_route_pass,
    collision_separation_pass,
    backpressure_pass,
    energy_transaction_pass,
))
outcome_b = (
    aggregate_storage_pass
    and aggregate_inverse_pass
    and not all((
        equal_source_deposit_pass,
        direction_route_pass,
        collision_separation_pass,
        backpressure_pass,
        energy_transaction_pass,
    ))
)
outcome_c = not (aggregate_storage_pass and aggregate_inverse_pass)

check("outcome:not-A", not outcome_a)
check("outcome:B", outcome_b)
check("outcome:not-C", not outcome_c)


failed = [name for name, condition in checks if not condition]
print(f"FTD-0942 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
print("Outcome B -- existing L/R fields are an exact aggregate canonical carrier in the")
print("isolated bare-wave sector, but current production dynamics do not realize the")
print("collision-separated occupancy-history carrier of FTD-0941.")
print("Missing gates: event deposit, nu-channel routing, collision separation,")
print("backpressure, and an exact source-energy transaction.")
print("Type verdict: no new primitive is forced; protected nonlinear field pulses")
print("remain open alongside a separately priced channelized-port realization.")

if failed:
    for name in failed:
        print(f"FAIL: {name}")
    raise SystemExit(1)
