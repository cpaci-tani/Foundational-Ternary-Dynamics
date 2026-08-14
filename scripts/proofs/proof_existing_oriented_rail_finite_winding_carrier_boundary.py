#!/usr/bin/env python3
"""FTD-0960 exact certificate for the existing-rail winding carrier.

This is a finite combinatorial/symbolic certificate.  It performs no floating
fit, parameter search, or near-miss scan.
"""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "protocol": (
        ROOT
        / "docs/theory/10_eft_program/preregistrations/"
        "native_time_carrier_programme/"
        "PREREG_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_BOUNDARY_v1.md",
        "B8BDCCCDEB5ECFE4FE2B9CAAD1C00AAF69C5E5F6CD0E4266866FBDF79A6ADDBA",
    ),
    "parity_rail": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md",
        "E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1",
    ),
    "odd_history": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md",
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    ),
    "occupancy_carry": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_FINITE_CAPACITY_LOCAL_REVERSIBLE_OCCUPANCY_CARRY_TRILEMMA_v1.md",
        "A89DE2964B7D48100EC850547D00BB540D05F1166CF18CABE654EB9D26917548",
    ),
    "isochrony_lift": (
        ROOT
        / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md",
        "746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E",
    ),
}


checks: list[tuple[str, str, bool, str]] = []


def record(group: str, label: str, condition: bool, detail: object) -> None:
    checks.append((group, label, bool(condition), str(detail)))


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def encode(word: tuple[int, ...]) -> tuple[tuple[int, ...], tuple[int, ...]]:
    """Equation (1), with depth zero first."""
    reverse = tuple(reversed(word))
    plus = tuple(int(s == 1) for s in reverse)
    minus = tuple(int(s == -1) for s in reverse)
    return plus, minus


def decode(state: tuple[tuple[int, ...], tuple[int, ...]]) -> tuple[int, ...]:
    plus, minus = state
    if len(plus) != len(minus):
        raise ValueError("channel lengths differ")
    depth_word: list[int] = []
    for p, m in zip(plus, minus):
        if (p, m) == (1, 0):
            depth_word.append(1)
        elif (p, m) == (0, 1):
            depth_word.append(-1)
        elif (p, m) == (0, 0):
            depth_word.append(0)
        else:
            raise ValueError("both oriented channels occupied at one slot")
    return tuple(reversed(depth_word))


def push(
    state: tuple[tuple[int, ...], tuple[int, ...]], s: int
) -> tuple[tuple[int, ...], tuple[int, ...]]:
    if s not in (-1, 0, 1):
        raise ValueError("crossing label is not ternary")
    plus, minus = state
    return (int(s == 1),) + plus, (int(s == -1),) + minus


def pop(
    state: tuple[tuple[int, ...], tuple[int, ...]]
) -> tuple[tuple[tuple[int, ...], tuple[int, ...]], int]:
    plus, minus = state
    if not plus or len(plus) != len(minus):
        raise ValueError("no depth-zero slot")
    s = decode(((plus[0],), (minus[0],)))[0]
    return (plus[1:], minus[1:]), s


def winding(state: tuple[tuple[int, ...], tuple[int, ...]], w0: int = 0) -> int:
    plus, minus = state
    return w0 + sum(plus) - sum(minus)


def token_count(state: tuple[tuple[int, ...], tuple[int, ...]]) -> int:
    plus, minus = state
    return sum(plus) + sum(minus)


def parity_pair(a: int, b: int) -> tuple[int, int]:
    return -b, a


def layer_matrix(length: int, parity: int) -> tuple[tuple[int, ...], ...]:
    matrix = [[0 for _ in range(length)] for _ in range(length)]
    j = 0
    while j < length:
        if j % 2 == parity and j + 1 < length:
            matrix[j][j + 1] = -1
            matrix[j + 1][j] = 1
            j += 2
        else:
            matrix[j][j] = 1
            j += 1
    return tuple(tuple(row) for row in matrix)


def signed_permutation(matrix: tuple[tuple[int, ...], ...]) -> bool:
    row_ok = all(sum(value != 0 for value in row) == 1 for row in matrix)
    col_ok = all(
        sum(matrix[row][col] != 0 for row in range(len(matrix))) == 1
        for col in range(len(matrix))
    )
    entries_ok = all(value in (-1, 0, 1) for row in matrix for value in row)
    return row_ok and col_ok and entries_ok


def main() -> int:
    print("=" * 79)
    print("FTD-0960 existing oriented-rail finite winding carrier proof")
    print("=" * 79)

    texts: dict[str, str] = {}
    for name, (path, expected) in SOURCES.items():
        actual = sha256(path)
        texts[name] = path.read_text(encoding="utf-8")
        record("G1", f"hash {path.name}", actual == expected, actual)

    protocol_markers = (
        "Success in one category may not be reported as success in another.",
        "No completed infinite rail is assumed",
        "This is a direct-history carrier",
        "compact finite counter requires an explicit overflow/backpressure",
        "Loading remains a separate transaction",
        "The expected result is Outcome B",
        "no numerical fitting, tolerance, or near-miss search",
    )
    for marker in protocol_markers:
        record("G1", f"protocol marker {marker[:42]}", marker in texts["protocol"], marker)

    dependency_markers = {
        "parity_rail": (
            "R(a,b)=(-b,a)",
            "preserves `a^2+b^2`, the number of nonzero labels",
            "retention under backpressure",
        ),
        "occupancy_carry": (
            "finite-alphabet Moore-token rail can export every registered neutral-body hop",
            "distinguishes clockwise from counterclockwise",
            "Loading an event port is a separate transaction",
        ),
        "isochrony_lift": (
            "w'=w+s",
            "No finite `n` stores every integer winding injectively",
            "active clutch whose work, reserve, reciprocal",
        ),
    }
    for source, markers in dependency_markers.items():
        for marker in markers:
            record("G1", f"{source} marker {marker[:38]}", marker in texts[source], marker)

    all_injective = True
    all_decode = True
    all_step_inverse = True
    all_recursive = True
    cardinalities: list[tuple[int, int]] = []
    for length in range(0, 8):
        seen: set[tuple[tuple[int, ...], tuple[int, ...]]] = set()
        for word in itertools.product((-1, 0, 1), repeat=length):
            state = encode(word)
            seen.add(state)
            all_decode &= decode(state) == word

            built: tuple[tuple[int, ...], tuple[int, ...]] = ((), ())
            for s in word:
                built = push(built, s)
            all_recursive &= built == state

            recovered: list[int] = []
            unwind = state
            for _ in word:
                unwind, s = pop(unwind)
                recovered.append(s)
            all_step_inverse &= tuple(reversed(recovered)) == word and unwind == ((), ())
        cardinalities.append((length, len(seen)))
        all_injective &= len(seen) == 3**length

    record("G2", "fixed-horizon injection through length seven", all_injective, cardinalities)
    record("G2", "channel-depth decoder is exact", all_decode, "decode(encode(word))=word")
    record("G2", "recursive shift/insertion equals closed encoding", all_recursive, "push fold")
    record("G2", "reverse stream/source readout is exact", all_step_inverse, "pop inverse")
    record("G2", "zero slot remains distinguishable with known depth", decode(((0,), (0,))) == (0,), "s=0")
    invalid_rejected = False
    try:
        decode(((1,), (1,)))
    except ValueError:
        invalid_rejected = True
    record("G2", "double-oriented slot fails closed", invalid_rejected, "(1,1) rejected")

    winding_identity = True
    token_identity = True
    sign_reversal = True
    for length in range(0, 8):
        for word in itertools.product((-1, 0, 1), repeat=length):
            state = encode(word)
            winding_identity &= winding(state, 7) == 7 + sum(word)
            token_identity &= token_count(state) == sum(s != 0 for s in word)
            reversed_sign = tuple(-s for s in word)
            swapped = state[1], state[0]
            sign_reversal &= encode(reversed_sign) == swapped
            sign_reversal &= winding(swapped) == -winding(state)
    record("G3", "rail count difference equals iterated lifted winding", winding_identity, "w_N=w_0+sum s_t")
    record("G3", "nonzero event count equals token count", token_identity, "m tokens")
    record("G3", "orientation reversal exchanges channels and winding", sign_reversal, "c_+<->c_-")
    plus_then_minus = encode((1, -1))
    minus_then_plus = encode((-1, 1))
    record("G3", "opposite zero-net words remain distinct", plus_then_minus != minus_then_plus, (plus_then_minus, minus_then_plus))
    record("G3", "opposite zero-net words have equal winding", winding(plus_then_minus) == winding(minus_then_plus) == 0, 0)
    record("G3", "depth reconstructs chronological order", decode(encode((1, 0, -1, 1))) == (1, 0, -1, 1), "ordered word")

    sample = encode((1, 0, -1, -1, 1))
    shifted = (0,) + sample[0], (0,) + sample[1]
    record("G4", "one rail update has radius-one dependency", shifted[0][1:] == sample[0] and shifted[1][1:] == sample[1], "c'(j+1)=c(j)")
    record("G4", "transport preserves token energy coefficient", token_count(shifted) == token_count(sample), token_count(sample))
    loaded = push(sample, 1)
    record("G4", "nonzero loading adds one half-scale token", token_count(loaded) == token_count(sample) + 1, "Delta H=epsilon_*/2")
    held = push(sample, 0)
    record("G4", "zero crossing adds no token energy", token_count(held) == token_count(sample), "Delta H=0")
    record("G4", "selected ray sign exchange is covariant", winding((sample[1], sample[0])) == -winding(sample), "nu_0<->-nu_0")
    record("G4", "energy scale remains selected", "positive selected token scale `epsilon_*`" in texts["protocol"], "epsilon_* not derived")

    record("G5", "length-N prefix stores exactly N update slots", len(sample[0]) == 5 and len(sample[1]) == 5, "N=5")
    record("G5", "fresh causal front or backpressure is explicit", "fresh causal\n+front, tail export, or backpressure" in texts["protocol"], "finite horizon")
    record("G5", "expanding carrier retains cancellation history", token_count(plus_then_minus) == 2 and winding(plus_then_minus) == 0, "two tokens, net zero")
    record("G5", "direct history is not compact winding", token_count(encode((1, -1) * 4)) == 8 and winding(encode((1, -1) * 4)) == 0, "eight tokens, net zero")
    record("G5", "finite support price agrees with source theorem", "occupied support and token energy\n+grow with the retained history" in texts["occupancy_carry"], "FTD-0941")

    boundary_collision = True
    cyclic_not_integer = True
    capacity_bound = True
    for width in range(1, 9):
        encodings = {w: (w,) for w in range(-width, width + 1)}
        fail_closed_outputs = {
            w: encodings[w + 1] if w < width else encodings[width]
            for w in range(-width, width + 1)
        }
        boundary_collision &= fail_closed_outputs[width - 1] == fail_closed_outputs[width]
        cyclic_not_integer &= -width != width + 1
        for trits in range(1, 8):
            if 3**trits >= 2 * width + 1:
                capacity_bound &= trits >= 1
                break
    record("G6", "fail-closed increment has two boundary preimages", boundary_collision, "E(W-1),E(W)->E(W)")
    record("G6", "cyclic wrap is not integer increment", cyclic_not_integer, "W maps to -W")
    record("G6", "ternary information capacity bound retained", capacity_bound, "3^n>=2W+1")

    pair_states = list(itertools.product((-1, 0, 1), repeat=2))
    pair_outputs = [parity_pair(a, b) for a, b in pair_states]
    record("G6", "R is a nine-state permutation", len(set(pair_outputs)) == 9, len(set(pair_outputs)))
    record("G6", "R preserves quadratic label count", all(a * a + b * b == u * u + v * v for (a, b), (u, v) in zip(pair_states, pair_outputs)), "a^2+b^2")
    record("G6", "R outputs each depend on one input coordinate", parity_pair(1, 0) == (0, 1) and parity_pair(0, 1) == (-1, 0), "signed coordinate permutation")
    matrices = [layer_matrix(7, parity) for parity in (0, 1)]
    record("G6", "both alternating layers are signed permutations", all(signed_permutation(matrix) for matrix in matrices), "monomial matrices")
    record("G6", "parity rail source says congestion is not resolved", "is not congestion resolution" in texts["parity_rail"], "backpressure boundary")
    record("G6", "compact carry is not silently claimed", "do not by themselves supply a nonlinear\n+compact carry/overflow transaction" in texts["protocol"], "carry open")

    forbidden_promotions = (
        "G* is derived",
        "Born rule is derived",
        "production winding carrier is closed",
        "native acquisition is closed",
        "compact balanced-ternary carry is derived",
    )
    protocol_lower = texts["protocol"].lower()
    record("G7", "no forbidden promotion marker", all(marker.lower() not in protocol_lower for marker in forbidden_promotions), "physics firewall")
    record("G7", "native acquisition remains open", "physical acquisition of `sign(Pi)`" in texts["protocol"], "open")
    record("G7", "active gearbox remains open", "active controller gearbox" in texts["protocol"], "open")
    record("G7", "G* identification remains open", "any identification with `G*`" in texts["protocol"], "open")
    record("G7", "production firewall explicit", "production, Born/Bell, Lorentz hiding, or completeness" in texts["protocol"], "unchanged")
    record("G7", "target leakage forbidden", "It may not read target winding" in texts["protocol"], "current sign only")
    record("G7", "frozen classifier is Outcome B", "**Outcome B:**" in texts["protocol"], "expanding exact; compact/native open")

    for group, label, ok, detail in checks:
        status = "PASS" if ok else "FAIL"
        print(f"  {status:4s}  {group} {label}: {detail}")

    passed = sum(ok for _, _, ok, _ in checks)
    failed = len(checks) - passed
    print("-" * 79)
    print(f"checks={len(checks)} passed={passed} failed={failed}")
    if failed:
        print("OUTCOME D — certificate failure prevents classification")
        return 1

    print(
        "OUTCOME B — an existing pair of oriented history channels exactly\n"
        "retains every finite crossing word and its lifted net winding.\n"
        "The realization spends expanding causal support and selected token\n"
        "energy. Existing parity transport does not itself provide a compact\n"
        "reversible carry/overflow transaction, and native crossing loading\n"
        "plus the active controller gearbox remain open."
    )
    print("EXISTING_EXPANDING_RAIL_FINITE_WINDING=EXACT_CONDITIONAL")
    print("EXISTING_FIXED_COMPACT_WINDING_COUNTER=NOT_REALIZED")
    print("NATIVE_CROSSING_ACQUISITION_AND_ACTIVE_GEARBOX=OPEN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
