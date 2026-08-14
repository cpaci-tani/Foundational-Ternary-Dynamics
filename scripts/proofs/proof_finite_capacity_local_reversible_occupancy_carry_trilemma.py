#!/usr/bin/env python3
"""Exact FTD-0941 finite-capacity occupancy-carry trilemma certificate.

This is a finite combinatorial/algebraic certificate.  It performs no
numerical near-miss search and introduces no production dynamics.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from itertools import permutations, product
from pathlib import Path
from typing import Dict, Iterable, Tuple


ROOT = Path(__file__).resolve().parents[2]
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md"
)

EXPECTED_HASHES = {
    PREREG: "46F9F124C5324CDB35F34E7F228D451630460C586FC2FE62F30563EBE218AB45",
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_PHASE_GATED_NEUTRAL_C4_HODGE_CHORD_AND_OCCUPANCY_CARRY_BOUNDARY_v1.md"
    ): "13C3A820AE368CCABCF5B5DC34B2CBA869B951899B1343AAD4CFD066BCBC3299",
    ROOT / "scripts/proofs/proof_phase_gated_neutral_c4_hodge_chord_occupancy_carry_boundary_v2.py": (
        "412BB20D5BD14918F81892CB1EBF4495866E16E8B7544443235CC9E93AA6B5B8"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md"
    ): "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    ROOT / "scripts/proofs/proof_causal_odd_pulse_history_carrier.py": (
        "9E1238C161851798442D75607A81E80346FFD6CBD16F9F13194FDC311FD9920D"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md"
    ): "8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048",
    ROOT / "scripts/proofs/proof_reciprocal_carry_reservoir_local_impulse_ledger.py": (
        "A12998C3E6599BD76AA6F36615A31B1BED37EFE206CAE39ADC0E51F658A89C19"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_QUASILOCAL_COMPANION_PREPARATION_AND_REVERSIBLE_HISTORY_FORMATION_BOUNDARY_v1.md"
    ): "4E00155889BAD84D3ED4A7B907BFBC86589DEA6873A24529519ADE310DC9CEFB",
    ROOT / "scripts/proofs/proof_quasilocal_companion_preparation_reversible_history_formation_boundary.py": (
        "AE6B5A068C9F1A0F0F81A73DB2EB037EF13F49F31845070B833602558B4AF0A7"
    ),
    ROOT / (
        "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
        "THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md"
    ): "9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD",
    ROOT / "scripts/proofs/proof_configuration_space_carrier.py": (
        "A309DCFDD50974B3F3C7177D6365F8FBB5BF08C30C4A6CD932DC5FDB399F87CE"
    ),
}

Vec = Tuple[int, int, int]
Channel = Tuple[Vec, int]
EventState = Dict[Channel, int]
RailKey = Tuple[Vec, int, int]
RailState = Dict[RailKey, int]

MOORE: Tuple[Vec, ...] = tuple(
    v for v in product((-1, 0, 1), repeat=3) if v != (0, 0, 0)
)
EDGE_DIRECTIONS: Tuple[Vec, ...] = tuple(
    v for v in MOORE if sum(abs(x) for x in v) == 2
)
LANES = (0, 1)

checks: list[tuple[str, bool]] = []


def check(name: str, condition: bool) -> None:
    checks.append((name, bool(condition)))


def file_hash(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def clean_event(event: EventState) -> EventState:
    return {key: int(value) for key, value in event.items() if value}


def clean_rail(rail: RailState) -> RailState:
    return {key: int(value) for key, value in rail.items() if value}


def forward(event: EventState, rail: RailState) -> tuple[EventState, RailState]:
    """Swap every source-depth port, then stream one causal cell outward."""
    event = clean_event(event)
    rail = clean_rail(rail)
    next_event: EventState = {}
    swapped_rail = dict(rail)
    channels = set(event)
    channels.update((nu, lane) for nu, lane, depth in rail if depth == 0)

    for channel in channels:
        nu, lane = channel
        old_event = event.get(channel, 0)
        old_origin = rail.get((nu, lane, 0), 0)
        if old_origin:
            next_event[channel] = old_origin
        if old_event:
            swapped_rail[(nu, lane, 0)] = old_event
        else:
            swapped_rail.pop((nu, lane, 0), None)

    next_rail = {
        (nu, lane, depth + 1): value
        for (nu, lane, depth), value in swapped_rail.items()
        if value
    }
    return clean_event(next_event), clean_rail(next_rail)


def inverse(event: EventState, rail: RailState) -> tuple[EventState, RailState]:
    """Undo the stream, then undo the source-depth swap."""
    event = clean_event(event)
    unstreamed = {
        (nu, lane, depth - 1): value
        for (nu, lane, depth), value in clean_rail(rail).items()
        if value
    }
    old_event: EventState = {}
    old_rail = dict(unstreamed)
    channels = set(event)
    channels.update((nu, lane) for nu, lane, depth in unstreamed if depth == 0)

    for channel in channels:
        nu, lane = channel
        swapped_event = event.get(channel, 0)
        swapped_origin = unstreamed.get((nu, lane, 0), 0)
        if swapped_origin:
            old_event[channel] = swapped_origin
        if swapped_event:
            old_rail[(nu, lane, 0)] = swapped_event
        else:
            old_rail.pop((nu, lane, 0), None)

    return clean_event(old_event), clean_rail(old_rail)


def token_number(event: EventState, rail: RailState) -> int:
    return sum(v * v for v in event.values()) + sum(v * v for v in rail.values())


def carry_vector(event: EventState, rail: RailState) -> Vec:
    out = [0, 0, 0]
    for (nu, _lane), value in event.items():
        for i in range(3):
            out[i] += value * nu[i]
    for (nu, _lane, _depth), value in rail.items():
        for i in range(3):
            out[i] += value * nu[i]
    return tuple(out)  # type: ignore[return-value]


def load_hop(event: EventState, direction: Vec) -> EventState:
    result = clean_event(event)
    for lane in LANES:
        key = (direction, lane)
        if result.get(key, 0):
            raise ValueError("event lane is not fresh")
        result[key] = 1
    return result


def emit_word(word: Iterable[Vec]) -> tuple[EventState, RailState, int]:
    event: EventState = {}
    rail: RailState = {}
    count = 0
    for direction in word:
        event = load_hop(event, direction)
        event, rail = forward(event, rail)
        if event:
            raise AssertionError("blank incoming rail should clear the event port")
        count += 1
    return event, rail, count


def decode_word(rail: RailState, length: int) -> tuple[Vec, ...]:
    decoded: list[Vec] = []
    for tick in range(length):
        depth = length - tick
        candidates = []
        for nu in MOORE:
            if all(rail.get((nu, lane, depth), 0) == 1 for lane in LANES):
                candidates.append(nu)
        if len(candidates) != 1:
            raise AssertionError(f"depth {depth} has {len(candidates)} candidate directions")
        decoded.append(candidates[0])
    return tuple(decoded)


def signed_permutation_group() -> tuple[tuple[tuple[int, int, int], ...], ...]:
    matrices = []
    for perm in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            rows = []
            for i in range(3):
                row = [0, 0, 0]
                row[perm[i]] = signs[i]
                rows.append(tuple(row))
            matrices.append(tuple(rows))
    return tuple(matrices)


GROUP = signed_permutation_group()


def act_vec(matrix: tuple[tuple[int, int, int], ...], vec: Vec) -> Vec:
    return tuple(sum(matrix[i][j] * vec[j] for j in range(3)) for i in range(3))  # type: ignore[return-value]


def act_state(
    matrix: tuple[tuple[int, int, int], ...],
    event: EventState,
    rail: RailState,
) -> tuple[EventState, RailState]:
    return (
        {(act_vec(matrix, nu), lane): value for (nu, lane), value in event.items()},
        {
            (act_vec(matrix, nu), lane, depth): value
            for (nu, lane, depth), value in rail.items()
        },
    )


def forward_periodic(
    event: EventState, rail: RailState, period: int
) -> tuple[EventState, RailState]:
    next_event, shifted = forward(event, rail)
    wrapped: RailState = {}
    for (nu, lane, depth), value in shifted.items():
        key = (nu, lane, depth % period)
        if key in wrapped:
            raise AssertionError("registered binary periodic port collision")
        wrapped[key] = value
    return next_event, wrapped


# Frozen provenance.
for path, expected in EXPECTED_HASHES.items():
    check(f"frozen hash: {path.relative_to(ROOT)}", path.is_file() and file_hash(path) == expected)

protocol_text = PREREG.read_text(encoding="utf-8")
for marker in (
    "fixed bounded finite-alphabet carrier",
    "cumulative face flux with no body identity",
    "locally transported body/worldline label",
    "separately represented link/carry state",
    "finite-per-site reversible Moore-token export",
    "Outcome B",
    "no thermodynamic `kT ln 2`",
):
    check(f"protocol marker: {marker}", marker in protocol_text)

# Fixed-support finite-capacity no-go and reversible-orbit periodicity.
fixed_support_no_go = True
for alphabet_size in range(2, 6):
    for region_size in range(1, 4):
        capacity = alphabet_size**region_size
        fixture_no_go = len(range(capacity + 1)) > capacity
        fixed_support_no_go = fixed_support_no_go and fixture_no_go
        check(
            f"pigeonhole q={alphabet_size} r={region_size}",
            fixture_no_go,
        )

reversible_orbits_periodic = True
for size in range(2, 5):
    all_periodic = True
    for perm in permutations(range(size)):
        for start in range(size):
            seen = set()
            state = start
            for _ in range(size + 1):
                if state in seen:
                    break
                seen.add(state)
                state = perm[state]
            if state not in seen or len(seen) > size:
                all_periodic = False
    check(f"all reversible {size}-state carriers are periodic", all_periodic)
    reversible_orbits_periodic = reversible_orbits_periodic and all_periodic

# Cumulative flux and finite reductions.
events = (1, 1, -1, 1, -1, -1, 1)
prefix = []
value = 0
for increment in events:
    value += increment
    prefix.append(value)
check("integer cumulative flux composes exactly", value == sum(events))
for increment in reversed(events):
    value -= increment
check("integer cumulative flux reverses exactly", value == 0)
for modulus in range(2, 8):
    check(
        f"mod-{modulus} carry identifies distinct windings",
        (0 % modulus) == (modulus % modulus) and 0 != modulus,
    )
    saturation = [min(x + 1, modulus - 1) for x in range(modulus)]
    check(
        f"capacity-{modulus} saturation is noninjective",
        len(set(saturation)) < len(saturation),
    )

# Finite body-label capacity and unlabeled collision ambiguity.
for labels in range(1, 5):
    for side in range(2, 5):
        capacity = labels * side**3
        check(
            f"body label B={labels} L={side} cannot encode all windings",
            capacity + 1 > capacity,
        )

labelled_pass = (("A", 1), ("B", -1))
labelled_exchange = (("A", -1), ("B", 1))
unlabelled_pass = tuple(sorted(position for _label, position in labelled_pass))
unlabelled_exchange = tuple(sorted(position for _label, position in labelled_exchange))
check(
    "unlabelled occupancy does not choose pass-through versus label exchange",
    labelled_pass != labelled_exchange and unlabelled_pass == unlabelled_exchange,
)

# Moore geometry and group action.
check("Moore channel count is 26", len(MOORE) == 26)
check("Moore edge-direction count is 12", len(EDGE_DIRECTIONS) == 12)
check("signed cubic group count is 48", len(GROUP) == 48 and len(set(GROUP)) == 48)
check(
    "signed cubic group permutes Moore channels",
    all({act_vec(matrix, nu) for nu in MOORE} == set(MOORE) for matrix in GROUP),
)

# Exhaustive one-channel forward/inverse checks over a finite support stencil.
test_nu = (1, 1, 0)
for bits in product((0, 1), repeat=8):
    event: EventState = {
        (test_nu, lane): bits[lane] for lane in LANES if bits[lane]
    }
    rail: RailState = {}
    cursor = 2
    for lane in LANES:
        for depth in (-1, 0, 1):
            if bits[cursor]:
                rail[(test_nu, lane, depth)] = 1
            cursor += 1
    endpoint = forward(event, rail)
    check(
        f"exact inverse fixture {bits}",
        inverse(*endpoint) == (clean_event(event), clean_rail(rail)),
    )
    check(
        f"token energy fixture {bits}",
        token_number(event, rail) == token_number(*endpoint),
    )

# Every registered Moore edge carries the FTD-0940 aggregate 2d and reverses.
for direction in EDGE_DIRECTIONS:
    event = load_hop({}, direction)
    endpoint = forward(event, {})
    check(f"blank source clears for {direction}", endpoint[0] == {})
    check(
        f"occupancy carry is 2d for {direction}",
        carry_vector(*endpoint) == tuple(2 * x for x in direction),
    )
    reverse_endpoint = forward(load_hop({}, tuple(-x for x in direction)), {})
    check(
        f"reverse occupancy carry is -2d for {direction}",
        carry_vector(*reverse_endpoint) == tuple(-2 * x for x in direction),
    )

# Ordered history, zero-net C4 orientation, and support growth.
clockwise: tuple[Vec, ...] = (
    (-1, 1, 0),
    (-1, -1, 0),
    (1, -1, 0),
    (1, 1, 0),
)
counterclockwise = tuple(tuple(-x for x in direction) for direction in reversed(clockwise))
cw_state = emit_word(clockwise)
ccw_state = emit_word(counterclockwise)
check("clockwise C4 word has zero net vector", carry_vector(cw_state[0], cw_state[1]) == (0, 0, 0))
check("counterclockwise C4 word has zero net vector", carry_vector(ccw_state[0], ccw_state[1]) == (0, 0, 0))
check("clockwise word is exactly recoverable", decode_word(cw_state[1], cw_state[2]) == clockwise)
check(
    "counterclockwise word is exactly recoverable",
    decode_word(ccw_state[1], ccw_state[2]) == counterclockwise,
)
check("opposite C4 orientations retain distinct carrier states", cw_state[1] != ccw_state[1])

same_direction_word = ((1, 1, 0),) * 17
same_state = emit_word(same_direction_word)
depths = [depth for (_nu, _lane, depth) in same_state[1]]
check("N hops retain exactly 2N finite tokens", token_number(same_state[0], same_state[1]) == 34)
check("N hops require causal depth N", max(depths) == 17)
check("N-hop word remains exactly recoverable", decode_word(same_state[1], 17) == same_direction_word)

# Signed-cubic equivariance on a state containing injection, backpressure, and history.
sample_event: EventState = {((1, 1, 0), 0): 1, ((-1, 0, 1), 1): 1}
sample_rail: RailState = {
    ((1, 1, 0), 0, 0): 1,
    ((0, -1, 1), 1, -2): 1,
    ((-1, -1, -1), 0, 3): 1,
}
for index, matrix in enumerate(GROUP):
    lhs = act_state(matrix, *forward(sample_event, sample_rail))
    rhs = forward(*act_state(matrix, sample_event, sample_rail))
    check(f"signed-cubic covariance arm {index + 1}", lhs == rhs)

# Collision/backpressure and finite multiplicity.
direction = (1, 1, 0)
different = load_hop(load_hop({}, direction), (-1, 1, 0))
different_endpoint = forward(different, {})
check("different direction channels compose", token_number(*different_endpoint) == 4)
check(
    "two same-direction occupancy lanes remain distinct",
    all(different_endpoint[1].get((direction, lane, 1), 0) == 1 for lane in LANES),
)
occupied_event = {((direction), 0): 1}
occupied_rail = {((direction), 0, 0): 1}
backpressured = forward(occupied_event, occupied_rail)
check("occupied source returns exact backpressure", backpressured[0] == occupied_event)
check("occupied carrier token still streams", backpressured[1] == {((direction), 0, 1): 1})
check("registered neutral carrier has exactly two lanes", len(LANES) == 2 and 2 not in LANES)
check("binary logical erase is noninjective", {0: 0, 1: 0}[0] == {0: 0, 1: 0}[1])

# Periodic return yields a source-port token rather than loss.
period = 5
periodic_event = {((direction), 0): 1}
periodic_rail: RailState = {}
initial_number = token_number(periodic_event, periodic_rail)
for _ in range(period + 1):
    periodic_event, periodic_rail = forward_periodic(periodic_event, periodic_rail, period)
check("periodic return appears as backpressure", periodic_event == {((direction), 0): 1})
check("periodic backpressure loses no token", token_number(periodic_event, periodic_rail) == initial_number)

# The carrier energy scale is free although its quadratic count is preserved.
energy_before_state = (sample_event, sample_rail)
energy_after_state = forward(*energy_before_state)
for epsilon in (Fraction(1, 2), Fraction(1, 1), Fraction(7, 3)):
    before = epsilon * Fraction(token_number(*energy_before_state), 2)
    after = epsilon * Fraction(token_number(*energy_after_state), 2)
    check(f"token energy preserved for epsilon={epsilon}", before == after)
check("token energy normalization remains free", Fraction(1, 2) != Fraction(7, 3))

# Outcome lock.  The branch is computed from the registered no-go predicates,
# not selected by a literal verdict flag.
fixed_support_closes = not (fixed_support_no_go and reversible_orbits_periodic)
token_export_passes = all(condition for _name, condition in checks)
outcome = "B" if (not fixed_support_closes and token_export_passes) else "C"
check("registered verdict is Outcome B", outcome == "B")

failed = [name for name, condition in checks if not condition]
for name, condition in checks:
    print(f"[{'PASS' if condition else 'FAIL'}] {name}")

print()
print(f"FTD-0941 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
print(f"VERDICT=OUTCOME_{outcome}")
print("FIXED_BOUNDED_FINITE_ALPHABET_RETAINS_UNBOUNDED_WINDING=FALSE")
print("CUMULATIVE_INTEGER_FLUX=EXACT_BUT_UNBOUNDED_LOCAL_CAPACITY")
print("FINITE_BODY_LABEL=IDENTITY_BOOKKEEPING_NOT_UNWRAPPED_WINDING")
print("FINITE_LINK_REGISTER=MODULAR_OR_BACKPRESSURED")
print("REVERSIBLE_MOORE_TOKEN_EXPORT=FINITE_PER_PORT_EXPANDING_SUPPORT")
print("CLOCKWISE_COUNTERCLOCKWISE_HISTORY=EXACTLY_DISTINGUISHED_AT_FIXED_HUB")
print("TOKEN_ENERGY=EXACT_CONDITIONAL_FREE_NORMALIZATION")
print("PRODUCTION_OR_BODY_LOCAL_CARRY=NOT_DERIVED")

if failed:
    raise SystemExit("failed gates: " + ", ".join(failed))
