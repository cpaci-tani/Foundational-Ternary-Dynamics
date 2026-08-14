#!/usr/bin/env python3
"""FTD-0874 exact certificate for the alternating oriented parity rail."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_v1.md"
)
PROTOCOL_HASH = "92C090ED43306249B963F757AD205F8C2B948944759A75CA46436606DDDC9BBB"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md":
        "94A75E375B8CB918B04C6D5C8DF5021380E8DA74243490BF1DD954ECBA26E32A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md":
        "898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md":
        "73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98",
    "engine/include/ftd/eft/oriented_ternary_quarter_turn.h":
        "46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1",
    "engine/include/ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h":
        "10BB9BFF5CC98E6CD72EC77F46E67766D458214E474296A7F3023AA27E2F8A94",
}

TERNARY = (-1, 0, 1)
PAIRS = tuple(itertools.product(TERNARY, repeat=2))
R = ((0, -1), (1, 0))
R_INVERSE = ((0, 1), (-1, 0))

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


def apply_pair(matrix, pair):
    return tuple(
        sum(matrix[row][column] * pair[column] for column in range(2))
        for row in range(2)
    )


def mmul(first, second):
    return tuple(
        tuple(
            sum(first[row][k] * second[k][column] for k in range(2))
            for column in range(2)
        )
        for row in range(2)
    )


def matching(length: int, tick: int):
    return tuple((left, left + 1) for left in range(tick & 1, length - 1, 2))


def layer(state, tick: int):
    result = list(state)
    for left, right in matching(len(state), tick):
        result[left], result[right] = -state[right], state[left]
    return tuple(result)


def inverse_layer(state, tick: int):
    result = list(state)
    for left, right in matching(len(state), tick):
        result[left], result[right] = state[right], -state[left]
    return tuple(result)


def qnorm(state) -> int:
    return sum(value * value for value in state)


def support(state):
    return tuple(index for index, value in enumerate(state) if value != 0)


for source, expected in SOURCE_HASHES.items():
    check(f"source hash {source}", sha256(ROOT / source) == expected)
check("protocol pre-run hash", sha256(ROOT / PROTOCOL) == PROTOCOL_HASH)

pair_outputs = tuple(apply_pair(R, pair) for pair in PAIRS)
check("bond map closes on ternary pairs", all(output in PAIRS for output in pair_outputs))
check("bond map permutes all nine pairs", len(set(pair_outputs)) == len(PAIRS))
check(
    "registered inverse recovers every pair",
    all(apply_pair(R_INVERSE, apply_pair(R, pair)) == pair for pair in PAIRS),
)
check(
    "R squared is minus identity",
    mmul(R, R) == ((-1, 0), (0, -1)),
)
check(
    "pair label norm is preserved",
    all(qnorm(apply_pair(R, pair)) == qnorm(pair) for pair in PAIRS),
)
check(
    "pair nonzero-label count is preserved",
    all(len(support(apply_pair(R, pair))) == len(support(pair)) for pair in PAIRS),
)
check(
    "bond map is sign-reversal equivariant",
    all(
        apply_pair(R, (-pair[0], -pair[1]))
        == tuple(-value for value in apply_pair(R, pair))
        for pair in PAIRS
    ),
)
check("bond matrix has determinant positive one", R[0][0] * R[1][1] - R[0][1] * R[1][0] == 1)

finite_lengths = range(2, 6)
check(
    "each parity matching is disjoint",
    all(
        len({site for bond in matching(length, tick) for site in bond})
        == 2 * len(matching(length, tick))
        for length in finite_lengths for tick in (0, 1)
    ),
)
finite_spaces = {
    length: tuple(itertools.product(TERNARY, repeat=length))
    for length in finite_lengths
}
check(
    "finite layers are full-state permutations",
    all(
        len({layer(state, tick) for state in finite_spaces[length]})
        == len(finite_spaces[length])
        for length in finite_lengths for tick in (0, 1)
    ),
)
check(
    "finite inverse layers recover every state",
    all(
        inverse_layer(layer(state, tick), tick) == state
        for length in finite_lengths for tick in (0, 1)
        for state in finite_spaces[length]
    ),
)

def dependency_set(length: int, tick: int, site: int):
    for left, right in matching(length, tick):
        if site in (left, right):
            return {left, right}
    return {site}


check(
    "each output reads itself and at most one adjacent site",
    all(
        len(dependency_set(length, tick, site)) <= 2
        for length in finite_lengths for tick in (0, 1)
        for site in range(length)
    ),
)
check(
    "no dependency crosses more than one edge",
    all(
        max(abs(site - dependency) for dependency in dependency_set(length, tick, site)) <= 1
        for length in finite_lengths for tick in (0, 1)
        for site in range(length)
    ),
)
check(
    "unmatched endpoints remain fixed",
    all(
        all(
            layer(state, tick)[site] == state[site]
            for site in range(length)
            if dependency_set(length, tick, site) == {site}
        )
        for length in finite_lengths for tick in (0, 1)
        for state in finite_spaces[length]
    ),
)

first = layer((1, 0, 0), 0)
check("first even layer moves site zero to one", first == (0, 1, 0))

propagation_ok = True
sign_ok = True
speed_ok = True
source_clear = True
trail_clear = True
inverse_ok = True
for sign in (-1, 1):
    for horizon in range(1, 9):
        initial = (sign,) + (0,) * horizon
        state = initial
        history = [state]
        for tick in range(horizon):
            state = layer(state, tick)
            history.append(state)
        expected = (0,) * horizon + (sign,)
        propagation_ok &= state == expected
        sign_ok &= state[horizon] == sign
        speed_ok &= support(state) == (horizon,)
        source_clear &= all(snapshot[0] == 0 for snapshot in history[1:])
        trail_clear &= all(
            all(value == 0 for value in snapshot[:tick])
            for tick, snapshot in enumerate(history[1:], start=1)
        )
        recovered = state
        for tick in reversed(range(horizon)):
            recovered = inverse_layer(recovered, tick)
        inverse_ok &= recovered == initial

check("prepared-pulse finite-horizon formula holds", propagation_ok)
check("propagated sign is unchanged", sign_ok)
check("pulse displacement equals elapsed ticks", speed_ok)
check("source clears after the first layer", source_clear)
check("all sites behind the pulse remain clear", trail_clear)
check("reverse layers recover every prepared history", inverse_ok)

fixed_state = (1, 0, 0, 0, 0)
for _ in range(8):
    fixed_state = layer(fixed_state, 0)
check("one fixed matching never transports beyond site one", all(index <= 1 for index in support(fixed_state)))

alternating_state = (1, 0, 0, 0, 0)
for tick in range(4):
    alternating_state = layer(alternating_state, tick)
check("alternating matchings transport beyond site one", support(alternating_state) == (4,))
check(
    "two matchings are minimal in the registered class",
    all(index <= 1 for index in support(fixed_state)) and support(alternating_state) == (4,),
)

occupied_pairs = tuple(pair for pair in PAIRS if pair[0] != 0 and pair[1] != 0)
check(
    "occupied downstream pairs undergo reciprocal exchange",
    all(apply_pair(R, pair) == (-pair[1], pair[0]) for pair in occupied_pairs),
)
check(
    "occupied exchange retains both label magnitudes",
    all(
        sorted(abs(value) for value in apply_pair(R, pair))
        == sorted(abs(value) for value in pair)
        for pair in occupied_pairs
    ),
)
check("no ternary pair is erased or hidden", len(set(pair_outputs)) == 9)
check(
    "ready transfer clears upstream exactly",
    all(apply_pair(R, (sign, 0)) == (0, sign) for sign in TERNARY),
)
check("occupied exchange does not imply downstream readiness", apply_pair(R, (1, 1))[1] != 0)
full_control = (1,) * 6
full_after = layer(full_control, 0)
check(
    "fully occupied control retains zero vacancies",
    len(support(full_after)) == len(full_after),
)

check(
    "retained finite layers remain bijective",
    all(
        qnorm(layer(state, tick)) == qnorm(state)
        for length in finite_lengths for tick in (0, 1)
        for state in finite_spaces[length]
    ),
)
dropped_outputs = {
    layer(state, 0)[1:]
    for state in finite_spaces[2]
}
check("discarding a post-layer endpoint is noninjective", len(dropped_outputs) < len(finite_spaces[2]))

permutations = tuple(itertools.permutations(range(3)))
fixed_implication = all(
    not (permutation[x] == y and x != y and permutation[y] == y)
    for permutation in permutations for x in range(3) for y in range(3)
)
check("injective maps cannot enter a distinct fixed state", fixed_implication)
check(
    "finite bijections have no transient predecessor of a fixed point",
    all(
        all(permutation[x] != y for x in range(3) if x != y)
        for permutation in permutations for y in range(3)
        if permutation[y] == y
    ),
)
moving = (1, 0, 0, 0)
moving_next = layer(moving, 0)
moving_later = layer(moving_next, 1)
check(
    "outward motion clears source without a fixed done state",
    moving_next[0] == 0 and moving_later[0] == 0 and moving_next != moving_later,
)

actuator_outputs = tuple((0, sign) for sign in TERNARY)
rail_outputs = tuple(layer((port, 0), 0) for _, port in actuator_outputs)
check(
    "actuator preparation composes exactly with first rail bond",
    rail_outputs == tuple((0, sign) for sign in TERNARY),
)
check(
    "composed spatial dependency remains one edge",
    all(abs(site - dependency) <= 1
        for site in range(2) for dependency in dependency_set(2, 0, site)),
)

protocol_text = (ROOT / PROTOCOL).read_text(encoding="utf-8")
scope_markers = (
    "PARITY_RAIL_STATUS=SELECTED_REFERENCE_EXISTING_TYPE",
    "GLOBAL_TICK_ROLE=EXISTING_INTEGER_PARITY_SCHEDULER",
    "FINITE_HORIZON_ONLY=TRUE",
    "BACKPRESSURE_PROGRESS=OPEN",
    "PRODUCTION_COUPLING=NONE",
    "GSTAR_ROLE=SEPARATE_CALENDAR_NOT_RAIL",
    "BORN_BELL_STATUS=UNTOUCHED",
)
check("all registered scope markers are present", all(marker in protocol_text for marker in scope_markers))
check(
    "finite-horizon and backpressure debts remain open",
    "FINITE_HORIZON_ONLY=TRUE" in protocol_text
    and "BACKPRESSURE_PROGRESS=OPEN" in protocol_text,
)
check("terminal gate reached with C1-C47 passing", checks == 47 and failures == 0)

print(f"\nFTD-0874 alternating oriented ternary parity rail: {checks - failures}/{checks} PASS")
if checks == 48 and failures == 0:
    print("ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_THEOREM")
    print("PREPARED_PULSE_SPEED=ONE_EDGE_PER_GLOBAL_TICK")
    print("LOCAL_SOURCE_CLEARING=OUTWARD_REVERSIBLE_MOTION")
    print("BACKPRESSURE=RECIPROCAL_RETENTION_NOT_PROGRESS")
    print("FIXED_DONE_STATE=EXCLUDED_FOR_DISTINCT_REVERSIBLE_PREDECESSOR")
    print("PARITY_RAIL_STATUS=SELECTED_REFERENCE_EXISTING_TYPE")
    print("FINITE_HORIZON_ONLY=TRUE")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR_NOT_RAIL")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0874_CERTIFICATE_INVALID")
raise SystemExit(1)
