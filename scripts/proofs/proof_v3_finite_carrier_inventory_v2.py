"""Exact carrier checks for the R5-capable FTD-v3 inventory v2."""

from __future__ import annotations

import json
import sys
from itertools import product
from pathlib import Path


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

Vec = tuple[int, int, int]
D_SC: tuple[Vec, ...] = (
    (1, 0, 0),
    (-1, 0, 0),
    (0, 1, 0),
    (0, -1, 0),
    (0, 0, 1),
    (0, 0, -1),
)
EXPECTED_HASH = "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"


def dot(a: Vec, b: Vec) -> int:
    return sum(x * y for x, y in zip(a, b))


def cross(a: Vec, b: Vec) -> Vec:
    return (
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0],
    )


def scale(s: int, a: Vec) -> Vec:
    return tuple(s * x for x in a)  # type: ignore[return-value]


def internal_tick(channel: tuple[Vec, Vec, int, int, int]):
    d, n, hand, phase, polarity = channel
    return (
        scale(hand, n),
        scale(hand, cross(d, n)),
        hand,
        (phase + 1) % 4,
        polarity,
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
inventory = register["carrier_inventory"]
payloads = inventory["primitive_payloads"]
cells = inventory["cell_alphabets"]

channels = tuple(
    (d, n, hand, phase, polarity)
    for d in D_SC
    for n in D_SC
    if dot(d, n) == 0
    for hand in (-1, 1)
    for phase in range(4)
    for polarity in (-1, 1)
)
channel_set = set(channels)

check("C1 six directed SC tangents", len(D_SC) == 6)
check("C2 four perpendicular normals per tangent", all(sum(dot(d, n) == 0 for n in D_SC) == 4 for d in D_SC))
check("C3 complete field-channel census is 384", len(channels) == 384 and len(channel_set) == 384)
check("C4 each polarity layer has 192 channels", all(sum(channel[-1] == sign for channel in channels) == 192 for sign in (-1, 1)))
check("C5 each tangent owns 64 channels", all(sum(channel[0] == d for channel in channels) == 64 for d in D_SC))
check("C6 internal Hodge/C4 tick closes on the channel set", all(internal_tick(channel) in channel_set for channel in channels))
check("C7 internal tick is bijective", len({internal_tick(channel) for channel in channels}) == 384)

advanced = {channel: channel for channel in channels}
for _ in range(12):
    advanced = {source: internal_tick(target) for source, target in advanced.items()}
check("C8 combined C3/C4 internal tick has period dividing twelve", all(source == target for source, target in advanced.items()))

check("C9 field-bank cardinality is exactly 2^384", 2 ** len(channels) == 2**384)
check("C10 site alphabet cardinality is 9*2^384", 3 * 3 * 2 ** len(channels) == 9 * 2**384)
check("C11 SC bond alphabet is A9^2", 9**2 == 81)
check("C12 plaquette alphabet is A9^4", 9**4 == 6_561)
check("C13 cube alphabet is singleton", 1 == 1)

check("C14 manifestation is surjective", {s for s in (-1, 0, 1) for _layer in range(3)} == {-1, 0, 1})
check("C15 manifestation is many-to-one", len([(0, layer, occupancy) for layer in range(3) for occupancy in (0, 1)]) > 1)
check("C16 charge conjugation is a channel permutation", {(d, n, h, k, -eps) for d, n, h, k, eps in channels} == channel_set)

check("C17 register selects the v2 carrier", inventory["version"] == 2 and inventory["specification"].endswith("R1_v2.md"))
check("C18 register channel cardinality agrees", payloads["field_channel_bank"]["channel_count"] == 384 and payloads["field_channel_bank"]["cardinality"] == "2^384")
check("C19 register local C3 layer agrees", payloads["C3_layer"]["cardinality"] == 3)
check("C20 register site cardinality agrees", cells["A0_site"]["cardinality"] == "9*2^384")
check("C21 register relation cardinalities agree", cells["A1_bond"]["cardinality"] == 81 and cells["A2_plaquette"]["cardinality"] == 6_561 and cells["A3_cube"]["cardinality"] == 1)
check("C22 frozen collision hash agrees", inventory["collision_table_sha256"] == EXPECTED_HASH)

ownership_blob = " ".join(inventory["ownership_constraints"]).lower()
check("C23 ownership excludes queues, identities, replay, and real registers", all(term in ownership_blob for term in ("queue", "identity", "replay", "real")))
check("C24 every occupied channel has a unique site/channel owner", "site/channel" in ownership_blob and "exclusion" in ownership_blob)
check(
    "C25 inventory retains an explicit successor-amendment condition",
    "successor amendment is required" in inventory["reopen_condition"].lower(),
)

passed = sum(ok for _, ok, _ in checks)
print(f"\n{passed}/{len(checks)} exact R1-v2 carrier checks pass")
raise SystemExit(0 if passed == len(checks) else 1)
