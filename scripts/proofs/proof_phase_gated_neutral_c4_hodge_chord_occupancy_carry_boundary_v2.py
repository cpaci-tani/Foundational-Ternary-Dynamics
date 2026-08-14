#!/usr/bin/env python3
"""FTD-0940 verifier-only repair for the execution-invalid FTD-0939 parent."""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT = ROOT / "scripts/proofs/proof_phase_gated_neutral_c4_hodge_chord_occupancy_carry_boundary.py"
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_PHASE_GATED_NEUTRAL_C4_HODGE_CHORD_OCCUPANCY_CARRY_CERTIFICATE_REPAIR_v2.md"
)
PARENT_SHA256 = "F5266CC6219A7C1D81729A977FD9727045A7E4610E1364D2629D1FB6CC89463C"
REPAIR_PROTOCOL_SHA256 = "16831EAA6DB7FD4C9DF801D70AF6CC645653FAB9A31D5867E5FE57C61C000524"
OLD = "if any(abs(value) != 1 for value in displacement):"
NEW = "if any(value not in (-1, 0, 1) for value in displacement):"


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    source = PARENT.read_text(encoding="utf-8")
    check("frozen parent certificate hash", digest(PARENT) == PARENT_SHA256)
    check("frozen repair protocol hash", digest(REPAIR_PROTOCOL) == REPAIR_PROTOCOL_SHA256)
    check("old Moore guard occurs exactly once", source.count(OLD) == 1)
    check("new Moore guard is absent before repair", source.count(NEW) == 0)
    repaired = source.replace(OLD, NEW, 1)
    check("one in-memory guard substitution is applied", repaired.count(OLD) == 0 and repaired.count(NEW) == 1)

    for index, (label, condition) in enumerate(checks, start=1):
        print(f"R{index:02d} {'PASS' if condition else 'FAIL'} {label}")
    if not all(condition for _, condition in checks):
        print("FTD-0940 repair integrity failed")
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
    print(f"R06 {'PASS' if inherited_ok else 'FAIL'} inherited certificate exits zero")
    if not inherited_ok:
        print("FTD-0940 inherited certificate failed")
        return inherited_exit or 1

    print("FTD-0940 repair integrity: 6/6 checks passed")
    print("PARENT_PROTOCOL_AND_CERTIFICATE=PRESERVED")
    print("REPAIR_COUNT=EXACTLY_ONE")
    print("REPAIR_SCOPE=MOORE_COMPONENT_ALPHABET_GUARD")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
