"""FTD-0948 wrapper-integrity repair for the invalid FTD-0947 wrapper."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
V2_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_CERTIFICATE_REPAIR_v2.md"
)
V2_WRAPPER = (
    ROOT
    / "scripts/proofs/"
      "proof_minimum_nonlinear_relative_field_recursive_charge_and_source_frame_v2.py"
)
V3_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_REPAIR_INTEGRITY_v3.md"
)

EXPECTED = {
    V2_PROTOCOL:
        "62BEA97459DD1EE7F6455F0C081AC3035126DDD494B1E8F7C35D394218ADDDC2",
    V2_WRAPPER:
        "63DBE4FD4D701A8A2C3150B393848E3EC3D698415C0D83A7A89784C01DB468D8",
    V3_PROTOCOL:
        "62C76F2E8D181CA2FE3DD43BAB4A3B1AC1F66E68BB6713CAFEFE0B4725D897F7",
}

for path, expected in EXPECTED.items():
    actual = sha256(path.read_bytes()).hexdigest().upper()
    if actual != expected:
        raise SystemExit(f"FTD-0948 fail closed: hash drift {path.name}: {actual}")

source = V2_WRAPPER.read_text(encoding="utf-8")
old = "if old in source or source.count(new) != 1:"
new = "if source.count(new) != 1 or (old not in new and old in source):"

if source.count(old) != 1 or source.count(new) != 0:
    raise SystemExit(
        "FTD-0948 fail closed: wrapper-integrity repair anchors are not unique"
    )
source = source.replace(old, new, 1)
if source.count(old) != 0 or source.count(new) != 1:
    raise SystemExit("FTD-0948 fail closed: wrapper-integrity substitution failed")

namespace = {
    "__name__": "__main__",
    "__file__": str(V2_WRAPPER),
}
try:
    exec(compile(source, str(V2_WRAPPER), "exec"), namespace)
except SystemExit as exc:
    inherited_exit = 0 if exc.code is None else int(exc.code)
else:
    inherited_exit = 0

if inherited_exit != 0:
    raise SystemExit(f"FTD-0948 inherited wrapper failed: exit={inherited_exit}")

print()
print("FTD-0948 wrapper-integrity repair")
print("  PASS  FTD-0947 protocol hash")
print("  PASS  FTD-0947 wrapper hash")
print("  PASS  FTD-0948 protocol hash")
print("  PASS  old meta-condition occurred exactly once")
print("  PASS  one in-memory substitution")
print("  PASS  inherited FTD-0947 wrapper exit zero")
print("repair_checks=6 passed=6 failed=0")
print("FTD-0948 OUTCOME B — repaired certificate and wrapper valid")
