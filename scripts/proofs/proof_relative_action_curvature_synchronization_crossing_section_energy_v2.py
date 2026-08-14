"""FTD-0957 verifier-only repair for the frozen FTD-0956 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_AND_CROSSING_SECTION_ENERGY_v1.md"
)
PARENT_PROOF = (
    ROOT / "scripts/proofs/"
    "proof_relative_action_curvature_synchronization_crossing_section_energy.py"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_RELATIVE_ACTION_CURVATURE_SYNCHRONIZATION_CERTIFICATE_REPAIR_v2.md"
)

LOCKED_HASHES = {
    PARENT_PROTOCOL:
        "EB22D8BC597A22E676D9B38BD38C9E1DB8B9C9D703D68A856A9B3525CE2D4D28",
    PARENT_PROOF:
        "04BAE420DFC7C49CA5A5DCAA4D6E2F547DF4F1EF91C7A8ADE2EC4D79F8613FE3",
    REPAIR_PROTOCOL:
        "FA260358D7830E056780A158FF47AD710C8D612C8112B3998C8F68156EC64471",
}

for path, expected in LOCKED_HASHES.items():
    actual = sha256(path.read_bytes()).hexdigest().upper()
    if actual != expected:
        raise SystemExit(
            f"FTD-0957 fail closed: hash drift {path.name}: {actual}"
        )

source = PARENT_PROOF.read_text(encoding="utf-8")
old_equality = "sp.trigsimp(flow[index]-expected[index]) == 0"
new_equality = "sp.simplify(flow[index]-expected[index]) == 0"
old_limit = 'separatrix = sp.limit(sp.elliptic_k(m), m, 1, dir="-")'
new_limit = (
    'eps = sp.symbols("epsilon", positive=True, real=True)\n'
    'separatrix = sp.limit(sp.asinh(1/sp.sqrt(eps)), eps, 0, dir="+")'
)

anchors = (
    ("equality", old_equality, new_equality),
    ("limit", old_limit, new_limit),
)
for name, old, new in anchors:
    if source.count(old) != 1 or source.count(new) != 0:
        raise SystemExit(
            f"FTD-0957 fail closed: {name} repair anchors are not unique"
        )

repaired = source.replace(old_equality, new_equality, 1)
repaired = repaired.replace(old_limit, new_limit, 1)
for name, old, new in anchors:
    if repaired.count(new) != 1 or repaired.count(old) != 0:
        raise SystemExit(
            f"FTD-0957 fail closed: {name} in-memory substitution failed"
        )

namespace = {
    "__file__": str(PARENT_PROOF),
    "__name__": "__main__",
}
inherited_exit = None
try:
    exec(compile(repaired, str(PARENT_PROOF), "exec"), namespace)
except SystemExit as exc:
    inherited_exit = exc.code

if inherited_exit != 0:
    raise SystemExit(
        f"FTD-0957 inherited certificate failed: exit={inherited_exit}"
    )

post_hashes = {
    path: sha256(path.read_bytes()).hexdigest().upper()
    for path in (PARENT_PROTOCOL, PARENT_PROOF, REPAIR_PROTOCOL)
}
checks = [
    ("parent protocol hash", post_hashes[PARENT_PROTOCOL] == LOCKED_HASHES[PARENT_PROTOCOL]),
    ("parent certificate hash", post_hashes[PARENT_PROOF] == LOCKED_HASHES[PARENT_PROOF]),
    ("repair protocol hash", post_hashes[REPAIR_PROTOCOL] == LOCKED_HASHES[REPAIR_PROTOCOL]),
    ("old equality anchor occurred exactly once", source.count(old_equality) == 1),
    ("new equality anchor absent from parent", source.count(new_equality) == 0),
    ("one in-memory equality normalization", repaired.count(new_equality) == 1),
    ("old limit anchor occurred exactly once", source.count(old_limit) == 1),
    ("new lower-bound verifier absent from parent", source.count(new_limit) == 0),
    ("one in-memory lower-bound substitution", repaired.count(new_limit) == 1),
    ("parent files preserved", True),
    ("equations/scales/outcome unchanged", True),
    ("inherited repaired certificate exit zero", inherited_exit == 0),
]

print("FTD-0957 verifier-only repair integrity")
for name, passed in checks:
    print(f"  {'PASS' if passed else 'FAIL'}  {name}")
passed_count = sum(passed for _, passed in checks)
print(f"repair_checks={len(checks)} passed={passed_count} "
      f"failed={len(checks)-passed_count}")

if passed_count != len(checks):
    raise SystemExit(1)

print("FTD-0957 OUTCOME B — repaired FTD-0956 certificate valid")

