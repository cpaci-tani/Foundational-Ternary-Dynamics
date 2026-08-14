"""FTD-0951 verifier-only repair for the frozen FTD-0950 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_v1.md"
)
PARENT_PROOF = (
    ROOT / "scripts/proofs/"
    "proof_causal_work_booked_c18_finite_radius_relaxation.py"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_CAUSAL_WORK_BOOKED_C18_FINITE_RADIUS_RELAXATION_CERTIFICATE_REPAIR_v2.md"
)

LOCKED_HASHES = {
    PARENT_PROTOCOL:
        "12C21B138BCFFB0F8613194620F8D75A287E6DDD9E25EC40DF50E14B78220988",
    PARENT_PROOF:
        "A2690CAEAEA7363C5E14D492844B250874545EABC8AF029415B3671E69D45071",
    REPAIR_PROTOCOL:
        "776AA1FCA1126D4CA728C9A1FDC11C90CF3E9ED337742AB0608F1ED9C85A33E4",
}

for path, expected in LOCKED_HASHES.items():
    actual = sha256(path.read_bytes()).hexdigest().upper()
    if actual != expected:
        raise SystemExit(
            f"FTD-0951 fail closed: hash drift {path.name}: {actual}"
        )

source = PARENT_PROOF.read_text(encoding="utf-8")
old = "next_increment_bound == b*c**(n+1), next_increment_bound"
new = (
    "sp.simplify(next_increment_bound - b*c**(n+1)) == 0, "
    "next_increment_bound"
)
if source.count(old) != 1 or source.count(new) != 0:
    raise SystemExit(
        "FTD-0951 fail closed: normalization-repair anchors are not unique"
    )

repaired = source.replace(old, new, 1)
if repaired.count(new) != 1 or repaired.count(old) != 0:
    raise SystemExit("FTD-0951 fail closed: in-memory substitution failed")

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
        f"FTD-0951 inherited certificate failed: exit={inherited_exit}"
    )

checks = [
    ("parent protocol hash", True),
    ("parent certificate hash", True),
    ("repair protocol hash", True),
    ("old fragment occurred exactly once", source.count(old) == 1),
    ("new fragment absent from parent", source.count(new) == 0),
    ("one in-memory normalization substitution", repaired.count(new) == 1),
    ("parent files preserved", True),
    ("equations/constants/outcome unchanged", True),
    ("inherited repaired certificate exit zero", inherited_exit == 0),
]

print("FTD-0951 verifier-only repair integrity")
for name, passed in checks:
    print(f"  {'PASS' if passed else 'FAIL'}  {name}")
passed_count = sum(passed for _, passed in checks)
print(f"repair_checks={len(checks)} passed={passed_count} "
      f"failed={len(checks)-passed_count}")

if passed_count != len(checks):
    raise SystemExit(1)

print("FTD-0951 OUTCOME A — repaired FTD-0950 certificate valid")
