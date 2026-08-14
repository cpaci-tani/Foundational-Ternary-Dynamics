#!/usr/bin/env python3
"""FTD-0945 one-marker repair for the execution-invalid FTD-0944 parent."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = (
    ROOT
    / "scripts/proofs/"
    "proof_existing_event_mediated_relative_history_carrier_audit.py"
)
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_AUDIT_v1.md"
)
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_CERTIFICATE_REPAIR_v2.md"
)
PARENT_SHA256 = "2B7E9AE5427B5EAA680E50433AB343EE4D3315C28081C7832364597FDFAA34B7"
PARENT_PROTOCOL_SHA256 = "9E2EF3C707A798AD73F7DF1280273F2924B9C7D3B337393000C6175E55811B1D"
REPAIR_PROTOCOL_SHA256 = "2E170186B8D5CBCE61005CCDF9A90715A31A894D576A5FFBCA3F91744B3F612D"
OLD = '"No new primitive storage type",'
NEW = '"not force a new primitive storage type",'


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    source = PARENT.read_text(encoding="utf-8")
    check("frozen parent protocol hash", digest(PARENT_PROTOCOL) == PARENT_PROTOCOL_SHA256)
    check("frozen parent certificate hash", digest(PARENT) == PARENT_SHA256)
    check("frozen repair protocol hash", digest(REPAIR_PROTOCOL) == REPAIR_PROTOCOL_SHA256)
    check("old prose marker occurs exactly once", source.count(OLD) == 1)
    check("new protocol marker absent before repair", source.count(NEW) == 0)
    repaired = source.replace(OLD, NEW, 1)
    check(
        "exactly one in-memory marker substitution",
        repaired.count(OLD) == 0 and repaired.count(NEW) == 1,
    )

    for index, (label, condition) in enumerate(checks, start=1):
        print(f"R{index:02d} {'PASS' if condition else 'FAIL'} {label}")
    if not all(condition for _, condition in checks):
        print("FTD-0945 repair integrity failed")
        return 1

    namespace = {
        "__name__": "__main__",
        "__file__": str(PARENT),
        "__package__": None,
    }
    try:
        exec(compile(repaired, str(PARENT), "exec"), namespace, namespace)
    except SystemExit as exc:
        inherited_exit = int(exc.code or 0)
    else:
        inherited_exit = 0

    inherited_ok = inherited_exit == 0
    print(f"R07 {'PASS' if inherited_ok else 'FAIL'} inherited certificate exits zero")
    if not inherited_ok:
        print("FTD-0945 inherited certificate failed")
        return inherited_exit or 1

    print("FTD-0945 repair integrity: 7/7 checks passed")
    print("PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED")
    print("REPAIR_COUNT=EXACTLY_ONE")
    print("REPAIR_SCOPE=ONE_PROTOCOL_PROSE_MARKER")
    print("MATHEMATICS_SOURCES_OUTCOMES=UNCHANGED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
