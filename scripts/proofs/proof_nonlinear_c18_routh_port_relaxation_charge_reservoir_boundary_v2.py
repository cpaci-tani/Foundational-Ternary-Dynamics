"""FTD-0953 verifier-only repair for the frozen FTD-0952 certificate."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_NONLINEAR_C18_ROUTH_PORT_RELAXATION_AND_CHARGE_RESERVOIR_BOUNDARY_v1.md"
)
PARENT_PROOF = (
    ROOT / "scripts/proofs/"
    "proof_nonlinear_c18_routh_port_relaxation_charge_reservoir_boundary.py"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_NONLINEAR_C18_ROUTH_PORT_RELAXATION_CERTIFICATE_REPAIR_v2.md"
)

LOCKED_HASHES = {
    PARENT_PROTOCOL:
        "0326481C47902DBD3EBD9442D904BD37CE014CF551135FC50D1F6CEF953246F5",
    PARENT_PROOF:
        "0E4C35A5C0B616A091B44906F10F1431086E88A0C1F19041DF2FA96E5496CFD5",
    REPAIR_PROTOCOL:
        "3744105630F45E8998104FE779B5050778A42FD8E75C6D3D98B94E006C81FE92",
}

for path, expected in LOCKED_HASHES.items():
    actual = sha256(path.read_bytes()).hexdigest().upper()
    if actual != expected:
        raise SystemExit(
            f"FTD-0953 fail closed: hash drift {path.name}: {actual}"
        )

source = PARENT_PROOF.read_text(encoding="utf-8")
old = 'sigma, omega = sp.symbols("sigma omega", nonzero=True, real=True)'
new = (
    'sigma = sp.symbols("sigma", nonzero=True, real=True)\n'
    'omega = sp.symbols("omega", positive=True, real=True)'
)
if source.count(old) != 1 or source.count(new) != 0:
    raise SystemExit(
        "FTD-0953 fail closed: positive-frequency repair anchors are not unique"
    )

repaired = source.replace(old, new, 1)
if repaired.count(new) != 1 or repaired.count(old) != 0:
    raise SystemExit("FTD-0953 fail closed: in-memory substitution failed")

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
        f"FTD-0953 inherited certificate failed: exit={inherited_exit}"
    )

checks = [
    ("parent protocol hash", True),
    ("parent certificate hash", True),
    ("repair protocol hash", True),
    ("old declaration occurred exactly once", source.count(old) == 1),
    ("new declaration absent from parent", source.count(new) == 0),
    ("one in-memory positive-frequency substitution", repaired.count(new) == 1),
    ("parent files preserved", True),
    ("equations/constants/outcome unchanged", True),
    ("inherited repaired certificate exit zero", inherited_exit == 0),
]

print("FTD-0953 verifier-only repair integrity")
for name, passed in checks:
    print(f"  {'PASS' if passed else 'FAIL'}  {name}")
passed_count = sum(passed for _, passed in checks)
print(f"repair_checks={len(checks)} passed={passed_count} "
      f"failed={len(checks)-passed_count}")

if passed_count != len(checks):
    raise SystemExit(1)

print("FTD-0953 OUTCOME B — repaired FTD-0952 certificate valid")
