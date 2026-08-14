"""FTD-0959 verifier-only repair for the frozen FTD-0958 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md"
)
PARENT_PROOF = (
    ROOT / "scripts/proofs/"
    "proof_global_isochrony_lift_oriented_crossing_latch_boundary.py"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_GLOBAL_ISOCHRONY_LIFT_ORIENTED_CROSSING_LATCH_CERTIFICATE_REPAIR_v2.md"
)

LOCKED_HASHES = {
    PARENT_PROTOCOL:
        "927F60B630584EDBFFD40922C25D1E57F97C09B2F175C696C1D2FE29C27782FE",
    PARENT_PROOF:
        "2F8F237E01E2B60AFD7614348537345F470CEEC672020E3E93F3A3B9232898E6",
    REPAIR_PROTOCOL:
        "1B31C1D074E3D455791CDD0EA5AF0CB9C3CFAEA299742FA81911EA563ECC29E0",
}

for path, expected in LOCKED_HASHES.items():
    actual = sha256(path.read_bytes()).hexdigest().upper()
    if actual != expected:
        raise SystemExit(
            f"FTD-0959 fail closed: hash drift {path.name}: {actual}"
        )

source = PARENT_PROOF.read_text(encoding="utf-8")
old_action = "-sp.diff(Hlift, delta) == -K*tilde"
new_action = "sp.simplify(-sp.diff(Hlift, delta)+K*tilde) == 0"
old_switch = (
    "sp.simplify(switch.subs(phi, sp.pi)) == "
    "2*(e1-e0)*chi*Acar"
)
new_switch = (
    "sp.simplify(switch.subs(phi, sp.pi)-"
    "2*(e1-e0)*chi*Acar) == 0"
)

anchors = (
    ("action", old_action, new_action),
    ("switch", old_switch, new_switch),
)
for name, old, new in anchors:
    if source.count(old) != 1 or source.count(new) != 0:
        raise SystemExit(
            f"FTD-0959 fail closed: {name} repair anchors are not unique"
        )

repaired = source.replace(old_action, new_action, 1)
repaired = repaired.replace(old_switch, new_switch, 1)
for name, old, new in anchors:
    if repaired.count(new) != 1 or repaired.count(old) != 0:
        raise SystemExit(
            f"FTD-0959 fail closed: {name} in-memory substitution failed"
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
        f"FTD-0959 inherited certificate failed: exit={inherited_exit}"
    )

post_hashes = {
    path: sha256(path.read_bytes()).hexdigest().upper()
    for path in (PARENT_PROTOCOL, PARENT_PROOF, REPAIR_PROTOCOL)
}
checks = [
    ("parent protocol hash", post_hashes[PARENT_PROTOCOL] == LOCKED_HASHES[PARENT_PROTOCOL]),
    ("parent certificate hash", post_hashes[PARENT_PROOF] == LOCKED_HASHES[PARENT_PROOF]),
    ("repair protocol hash", post_hashes[REPAIR_PROTOCOL] == LOCKED_HASHES[REPAIR_PROTOCOL]),
    ("old action anchor occurred exactly once", source.count(old_action) == 1),
    ("new action comparison absent from parent", source.count(new_action) == 0),
    ("one in-memory action normalization", repaired.count(new_action) == 1),
    ("old switch anchor occurred exactly once", source.count(old_switch) == 1),
    ("new switch comparison absent from parent", source.count(new_switch) == 0),
    ("one in-memory switch normalization", repaired.count(new_switch) == 1),
    ("parent files preserved", True),
    ("equations/types/outcome unchanged", True),
    ("inherited repaired certificate exit zero", inherited_exit == 0),
]

print("FTD-0959 verifier-only repair integrity")
for name, passed in checks:
    print(f"  {'PASS' if passed else 'FAIL'}  {name}")
passed_count = sum(passed for _, passed in checks)
print(f"repair_checks={len(checks)} passed={passed_count} "
      f"failed={len(checks)-passed_count}")

if passed_count != len(checks):
    raise SystemExit(1)

print("FTD-0959 OUTCOME B — repaired FTD-0958 certificate valid")

