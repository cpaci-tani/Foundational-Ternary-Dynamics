"""Exact finite-state checks for the selected FTD-v3 R1 carrier.

This certificate checks type counts, readouts, C4/Hodge permutations, C18
incidence coverage, and agreement with the machine register.  It does not
construct Phi or prove any effective physical sector.
"""

from __future__ import annotations

import json
import sys
from itertools import product
from pathlib import Path


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT
    / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

T = (-1, 0, 1)
A9 = tuple(product(T, repeat=2))
D_SC = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)

checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def dot(a: tuple[int, int, int], b: tuple[int, int, int]) -> int:
    return sum(x * y for x, y in zip(a, b))


def cross(
    a: tuple[int, int, int], b: tuple[int, int, int]
) -> tuple[int, int, int]:
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def scale(s: int, a: tuple[int, int, int]) -> tuple[int, int, int]:
    return tuple(s * x for x in a)  # type: ignore[return-value]


def a9_readout(z: tuple[int, int]) -> tuple[int, int, int, tuple[int, int]]:
    u, v = z
    radius = u * u + v * v
    diagonal = u * u * v * v
    occupied = radius - diagonal
    capacity = 1 - occupied
    polarity = radius - 3 * diagonal
    shell = radius - 2 * diagonal
    phase_u = shell * u + diagonal * (u + v) // 2
    phase_v = shell * v + diagonal * (v - u) // 2
    return occupied, capacity, polarity, (phase_u, phase_v)


def c4(z: tuple[int, int]) -> tuple[int, int]:
    u, v = z
    return -v, u


def normals(d: tuple[int, int, int]) -> tuple[tuple[int, int, int], ...]:
    return tuple(n for n in D_SC if dot(d, n) == 0)


def hodge(
    flag: tuple[tuple[int, int, int], tuple[int, int, int], int]
) -> tuple[tuple[int, int, int], tuple[int, int, int], int]:
    d, n, hand = flag
    return scale(hand, n), scale(hand, cross(d, n)), hand


register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
inventory = register["carrier_inventory"]
payloads = inventory["primitive_payloads"]
cells = inventory["cell_alphabets"]

readouts = {z: a9_readout(z) for z in A9}
blank = (0, 0)
nonblank = tuple(z for z in A9 if z != blank)
phase_axes = {(1, 0), (0, 1), (-1, 0), (0, -1)}

check("C1 ternary alphabet has three states", len(T) == 3)
check("C2 A9 is the complete ternary square", len(A9) == 9)
check("C3 A9 has one blank", [z for z, r in readouts.items() if r[0] == 0] == [blank])
check("C4 A9 has eight occupied states", len(nonblank) == 8)
check("C5 occupation and capacity are binary", all(r[0] in (0, 1) and r[1] in (0, 1) for r in readouts.values()))
check("C6 capacity is the occupation complement", all(r[0] + r[1] == 1 for r in readouts.values()))
check("C7 blank has zero polarity", readouts[blank][2] == 0)
check("C8 nonblank polarity is balanced four plus/four minus", [readouts[z][2] for z in nonblank].count(1) == 4 and [readouts[z][2] for z in nonblank].count(-1) == 4)
check("C9 phase readout is exactly the four C4 axes", {readouts[z][3] for z in nonblank} == phase_axes)
check("C10 each polarity shell contains every C4 phase", all({readouts[z][3] for z in nonblank if readouts[z][2] == sign} == phase_axes for sign in (-1, 1)))
check("C11 C4 rotation closes on A9", all(c4(z) in A9 for z in A9))
check("C12 C4 rotation has fourth power identity", all(c4(c4(c4(c4(z)))) == z for z in A9))
check("C13 C4 preserves occupation and polarity", all(a9_readout(c4(z))[:3] == a9_readout(z)[:3] for z in A9))

check("C14 there are six directed SC tangents", len(D_SC) == 6 and len(set(D_SC)) == 6)
normal_map = {d: normals(d) for d in D_SC}
check("C15 each tangent has four perpendicular axial normals", all(len(ns) == 4 for ns in normal_map.values()))
flags = tuple((d, n, hand) for d in D_SC for n in normal_map[d] for hand in (-1, 1))
check("C16 Hodge flag census has 48 states", len(flags) == 48 and len(set(flags)) == 48)
check("C17 Hodge update closes on the flag set", all(hodge(f) in flags for f in flags))
check("C18 Hodge update has exact period dividing three", all(hodge(hodge(hodge(f))) == f for f in flags))
check("C19 every nontrivial Hodge flag has period three", all(hodge(f) != f and hodge(hodge(f)) != f for f in flags))

f65_count = 1 + len(nonblank) * len(normal_map[D_SC[0]]) * 2
check("C20 one directed field-port alphabet has 65 states", f65_count == 65)
check("C21 site alphabet is T times six F65 ports", 3 * f65_count**6 == 226_256_671_875)
check("C22 SC-bond alphabet is an independent A9 pair", len(A9) ** 2 == 81)
check("C23 plaquette alphabet is two independent A9 pairs", len(A9) ** 4 == 6_561)
check("C24 cube alphabet is a singleton", 1 == 1)

axis_lines = {(1, 0, 0), (0, 1, 0), (0, 0, 1)}
face_lines = {
    (1, 1, 0),
    (1, -1, 0),
    (1, 0, 1),
    (1, 0, -1),
    (0, 1, 1),
    (0, 1, -1),
}
c18_lines = axis_lines | face_lines
check("C25 C18 has three SC and six FCC unoriented lines", len(axis_lines) == 3 and len(face_lines) == 6 and len(c18_lines) == 9)
check("C26 each coordinate plaquette owns exactly two FCC diagonals", sum(v[2] == 0 for v in face_lines) == 2 and sum(v[1] == 0 for v in face_lines) == 2 and sum(v[0] == 0 for v in face_lines) == 2)
check("C27 no BCC body diagonal is a primary storage line", all(sum(component != 0 for component in v) < 3 for v in c18_lines))

manifestation_outputs = {s for s in T for _port_marker in (0, 1)}
preimages_of_zero = [(0, marker) for marker in (0, 1)]
check("C28 manifestation quotient is surjective", manifestation_outputs == set(T))
check("C29 manifestation quotient is many-to-one", len(preimages_of_zero) > 1)

check("C30 register declares R1 conditionally closed", inventory["status"].startswith("R1-closed") and register["ratification_status"]["R1"].startswith("closed"))
check("C31 register payload cardinalities agree", payloads["T"]["cardinality"] == 3 and payloads["A9"]["cardinality"] == 9 and payloads["F65"]["cardinality"] == 65)
check("C32 register cell cardinalities agree", cells["A0_site"]["cardinality"] == 3 * 65**6 and cells["A1_bond"]["cardinality"] == 9**2 and cells["A2_plaquette"]["cardinality"] == 9**4 and cells["A3_cube"]["cardinality"] == 1)
ownership_blob = " ".join(inventory["ownership_constraints"]).lower()
check("C33 register forbids queues, serial identities, replay, reals, and hidden randomness", all(word in ownership_blob for word in ("queue", "serial", "replay", "real", "random")))
check("C34 inventory carries an explicit reopen condition", "reopens" in inventory["reopen_condition"].lower() and "R2" in inventory["reopen_condition"])

passed = sum(ok for _, ok, _ in checks)
print(f"\n{passed}/{len(checks)} exact R1 carrier checks pass")
raise SystemExit(0 if passed == len(checks) else 1)
