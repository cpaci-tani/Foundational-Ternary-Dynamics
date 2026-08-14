#!/usr/bin/env python3
"""FTD-0872 exact certificate for the oriented ternary quarter-turn gearbox."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md"
)
PROTOCOL_HASH = "63462EDAA5970A1EC934F34A1ABF1EB95FC22D1969E6817EE8BA1912FC96E295"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md":
        "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_TERNARY_ELIGIBILITY_CLUTCH_AND_ONE_SHOT_HANDSHAKE_v1.md":
        "6BD280A51DEF9A1B5E373D0084A9C19597772CD31D2B5D278B2323315AC2153D",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_REVERSIBLE_TERNARY_SIGNAL_UNCOMPUTATION_AND_RESET_BOUNDARY_v1.md":
        "F52BE0CD97FAE06CF6A39C6E0784EC75746F7B8ABF9843C4EF78B37181C8D2CC",
    "engine/include/ftd/eft/reciprocal_record_port.h":
        "5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08",
    "engine/include/ftd/eft/ternary_eligibility_clutch.h":
        "C53ED1A7FCFF54E4236D2353CA319BCE61EC459C1A7A90F2069C01145256FE43",
    "engine/include/ftd/eft/reversible_ternary_signal_uncomputation.h":
        "2596A4873D957E43FFFA25DEDF984F7EF3D1146307DEF765164C72FCB22A65AD",
}

TERNARY = (-1, 0, 1)
ENCODE = {-1: 2, 0: 0, 1: 1}
DECODE = {0: 0, 1: 1, 2: -1}
I2 = ((1, 0), (0, 1))
NEG_I2 = ((2, 0), (0, 2))
R = ((0, 2), (1, 0))
R_INVERSE = ((0, 1), (2, 0))

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


def madd(first, second):
    return tuple(
        tuple((first[row][column] + second[row][column]) % 3 for column in range(2))
        for row in range(2)
    )


def mscale(scale: int, matrix):
    return tuple(
        tuple((scale * matrix[row][column]) % 3 for column in range(2))
        for row in range(2)
    )


def mmul(first, second):
    return tuple(
        tuple(
            sum(first[row][k] * second[k][column] for k in range(2)) % 3
            for column in range(2)
        )
        for row in range(2)
    )


def transpose(matrix):
    return tuple(tuple(matrix[column][row] for column in range(2)) for row in range(2))


def determinant(matrix) -> int:
    return (matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]) % 3


def apply(matrix, pair: tuple[int, int]) -> tuple[int, int]:
    residues = (ENCODE[pair[0]], ENCODE[pair[1]])
    output = tuple(
        sum(matrix[row][column] * residues[column] for column in range(2)) % 3
        for row in range(2)
    )
    return DECODE[output[0]], DECODE[output[1]]


def negative(pair: tuple[int, int]) -> tuple[int, int]:
    return -pair[0], -pair[1]


def energy(pair: tuple[int, int]) -> int:
    return pair[0] * pair[0] + pair[1] * pair[1]


def area(first: tuple[int, int], second: tuple[int, int]) -> int:
    return first[0] * second[1] - first[1] * second[0]


def controlled(eligibility: int):
    return madd(mscale(1 - eligibility, I2), mscale(eligibility, R))


def sym2(matrix):
    a, b = matrix[0]
    c, d = matrix[1]
    return (
        (a * a, 2 * a * b, b * b),
        (a * c, a * d + b * c, b * d),
        (c * c, 2 * c * d, d * d),
    )


def ready_conditional(pair: tuple[int, int]) -> tuple[int, int]:
    return apply(R, pair) if pair[1] == 0 else pair


for source, expected in SOURCE_HASHES.items():
    check(f"source hash {source}", sha256(ROOT / source) == expected)
check("protocol pre-run hash", sha256(ROOT / PROTOCOL) == PROTOCOL_HASH)

check(
    "ternary encoding is bijective",
    set(ENCODE) == set(TERNARY)
    and set(ENCODE.values()) == {0, 1, 2}
    and all(DECODE[ENCODE[value]] == value for value in TERNARY),
)
check(
    "R modular evaluation matches (-o,s)",
    all(apply(R, (s, o)) == (-o, s) for s in TERNARY for o in TERNARY),
)
check("R squared is minus identity", mmul(R, R) == NEG_I2)
check("R fourth power is identity", mmul(mmul(R, R), mmul(R, R)) == I2)
check("R has positive orientation", determinant(R) == 1)
check("R is orthogonal over F3", mmul(transpose(R), R) == I2)
check(
    "R preserves representative energy on all states",
    all(energy(apply(R, pair)) == energy(pair) for pair in itertools.product(TERNARY, repeat=2)),
)
check("zero eligibility is identity", controlled(0) == I2)
check("unit eligibility is R", controlled(1) == R)
check("zero-eligibility inverse is identity", mmul(controlled(0), I2) == I2)
check("unit-eligibility inverse is minus R", mmul(controlled(1), R_INVERSE) == I2)
states = tuple(itertools.product(TERNARY, repeat=2))
check("G0 permutes all nine states", len({apply(controlled(0), pair) for pair in states}) == 9)
check("G1 permutes all nine states", len({apply(controlled(1), pair) for pair in states}) == 9)
check("ready emission transfers every ternary value", all(apply(R, (s, 0)) == (0, s) for s in TERNARY))
check("inverse absorption restores every ternary value", all(apply(R_INVERSE, (0, s)) == (s, 0) for s in TERNARY))
check("no-event state is fixed", apply(R, (0, 0)) == (0, 0))
check(
    "sign reversal commutes with both orientations",
    all(
        apply(matrix, negative(pair)) == negative(apply(matrix, pair))
        for matrix in (R, R_INVERSE)
        for pair in states
    ),
)
check(
    "forward oriented area equals positive energy",
    all(pair == (0, 0) or area(pair, apply(R, pair)) == energy(pair) > 0 for pair in states),
)
check(
    "inverse oriented area equals negative energy",
    all(pair == (0, 0) or area(pair, apply(R_INVERSE, pair)) == -energy(pair) < 0 for pair in states),
)
R_INTEGER = ((0, -1), (1, 0))
MINUS_R_INTEGER = ((0, 1), (-1, 0))
check("symmetric square loses central orientation", sym2(R_INTEGER) == sym2(MINUS_R_INTEGER))

qualifying = []
for entries in itertools.product(range(3), repeat=4):
    matrix = ((entries[0], entries[1]), (entries[2], entries[3]))
    maps_source_to_output = (matrix[0][0], matrix[1][0]) == (0, 1)
    if maps_source_to_output and determinant(matrix) == 1 and mmul(transpose(matrix), matrix) == I2:
        qualifying.append(matrix)
check("R is the unique registered F3 isometry", qualifying == [R])

UNSIGNED_SWAP = ((0, 1), (1, 0))
check(
    "unsigned swap transfers but reverses orientation",
    apply(UNSIGNED_SWAP, (1, 0)) == (0, 1)
    and determinant(UNSIGNED_SWAP) == 2
    and mmul(transpose(UNSIGNED_SWAP), UNSIGNED_SWAP) == I2,
)

first_collision = ready_conditional((1, 0))
second_collision = ready_conditional((0, 1))
check("naive ready-port wrapper has explicit collision", first_collision == second_collision == (0, 1))
check("naive ready-port wrapper is noninjective", len({ready_conditional(pair) for pair in states}) < 9)
check(
    "all-domain controlled maps remain bijective",
    all(len({apply(controlled(a), pair) for pair in states}) == 9 for a in (0, 1)),
)
check(
    "joint quarter-turn performs no logical erasure",
    len(states) == 9 and len({apply(R, pair) for pair in states}) == 9,
)
check(
    "ready-event endpoint energy is unchanged",
    all(energy((s, 0)) == energy(apply(R, (s, 0))) for s in TERNARY),
)

protocol_text = (ROOT / PROTOCOL).read_text(encoding="utf-8")
check("controller-work scope marker", "CONTROLLER_WORK_STATUS=OPEN" in protocol_text)
check("production-coupling scope marker", "PRODUCTION_COUPLING=NONE" in protocol_text)
check("Gstar scope marker", "GSTAR_ROLE=NOT_DERIVED" in protocol_text)
check("Born/Bell scope marker", "BORN_BELL_STATUS=UNTOUCHED" in protocol_text)
check("terminal gate reached with C1-C39 passing", checks == 39 and failures == 0)

print(f"\nFTD-0872 oriented ternary quarter-turn gearbox: {checks - failures}/{checks} PASS")
if checks == 40 and failures == 0:
    print("ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_THEOREM")
    print("READY_EMISSION=(s,0)->(0,s)")
    print("RECIPROCAL_ABSORPTION=R_INVERSE")
    print("NAIVE_EMPTY_PORT_FAIL_CLOSED=NONINJECTIVE")
    print("CONTROLLER_WORK_STATUS=OPEN")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=NOT_DERIVED")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0872_CERTIFICATE_INVALID")
raise SystemExit(1)
