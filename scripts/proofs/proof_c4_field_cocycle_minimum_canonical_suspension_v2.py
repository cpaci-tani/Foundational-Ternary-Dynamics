#!/usr/bin/env python3
"""FTD-0974 source-marker-only repair wrapper for FTD-0973."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_C4_FIELD_COCYCLE_AND_MINIMUM_CANONICAL_SUSPENSION_v1.md"
)
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/"
    "proof_c4_field_cocycle_minimum_canonical_suspension.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_C4_FIELD_COCYCLE_CERTIFICATE_SOURCE_MARKER_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL:
        "6328CD0FCA455BB135F1642D9A85C4BADFB63C3A9DA070B3BC8765434E4F1E87",
    PARENT_SCRIPT:
        "B83F616681E1E27D2F9AE6F2F935403032E5FB536E8B6942D7157DB909C2A3B8",
    REPAIR_PROTOCOL:
        "F32E722B1C684A01C9A523282D8F178C5D9240D2BADCA4A90535FC7ABF5B7EE4",
}

OLD = '        list(SOURCES)[0]: "positive complete-square phase connection",\n'
NEW = '        list(SOURCES)[0]: "exact positive autonomous connection Hamiltonian",\n'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = []
    for path, expected in EXPECTED.items():
        integrity.append((f"hash {path.name}", before[path] == expected))

    source = PARENT_SCRIPT.read_text(encoding="utf-8")
    old_count = source.count(OLD)
    new_count = source.count(NEW)
    integrity.append(("old source marker occurs exactly once", old_count == 1))
    integrity.append(("actual source marker absent before repair", new_count == 0))
    substitutions = int(old_count == 1 and new_count == 0)
    patched = source.replace(OLD, NEW, 1) if substitutions else source
    integrity.append(("exactly one in-memory substitution", substitutions == 1))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 1:
        namespace = {"__file__": str(PARENT_SCRIPT), "__name__": "__main__"}
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(patched, str(PARENT_SCRIPT), "exec"), namespace)
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited certificate exactly 64/64", "checks=64 passed=64 failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "FTD-0973 OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    for path in EXPECTED:
        integrity.append((f"preserved {path.name}", after[path] == before[path]))

    print("FTD-0974 source-marker repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0974 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0974 OUTCOME B - repaired FTD-0973 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
