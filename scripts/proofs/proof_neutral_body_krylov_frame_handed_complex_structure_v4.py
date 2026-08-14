#!/usr/bin/env python3
"""FTD-0969 verifier-marker-only repair wrapper for FTD-0968."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme"
PROOFS = ROOT / "scripts/proofs"

PARENT_PROTOCOL = BASE / "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
PARENT_SCRIPT = PROOFS / "proof_neutral_body_krylov_frame_handed_complex_structure.py"
REPAIR_PROTOCOL_V2 = BASE / "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_CERTIFICATE_IMPLEMENTATION_REPAIR_v2.md"
REPAIR_WRAPPER_V2 = PROOFS / "proof_neutral_body_krylov_frame_handed_complex_structure_v2.py"
REPAIR_PROTOCOL_V3 = BASE / "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_TWO_SITE_SYMBOL_REPAIR_v3.md"
REPAIR_WRAPPER_V3 = PROOFS / "proof_neutral_body_krylov_frame_handed_complex_structure_v3.py"
REPAIR_PROTOCOL_V4 = BASE / "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_PARENT_OUTCOME_MARKER_REPAIR_v4.md"

EXPECTED = {
    PARENT_PROTOCOL:
        "F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C",
    PARENT_SCRIPT:
        "794B92828417A2264CAA3B75A6CF678E777D5D65D931DDAC8ECB24E8589F7C58",
    REPAIR_PROTOCOL_V2:
        "8D754A510DDF2399DF9243E0F9B8FAEDDF3EEAE8646D0EF5BC2A617DCBB7DA9F",
    REPAIR_WRAPPER_V2:
        "BF416D09B3A89A6C93863D40DE5D2F8E364443673FC363EEDAA6284EF266734F",
    REPAIR_PROTOCOL_V3:
        "55DB0E19370B743199E40ADF863DC4E9B90DB93A5FDC5196DB6BDCCC5B061122",
    REPAIR_WRAPPER_V3:
        "555FB4C627D585E01D3F7BB9E5E4F4F5A13E4FA95E4EB309217746F4BF08D4CF",
    REPAIR_PROTOCOL_V4:
        "A44ADE36E7778BD1599895F86F08FE220321B7A5449EF73FC70BCCBA24BD077E",
}

OLD_MARKER = '        "FTD-0966 OUTCOME B" in inherited,\n'
NEW_MARKER = '        "OUTCOME B - exact conditional regional frame" in inherited,\n'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = []
    for path, expected in EXPECTED.items():
        integrity.append((f"hash {path.name}", before[path] == expected))

    source = REPAIR_WRAPPER_V3.read_text(encoding="utf-8")
    old_count = source.count(OLD_MARKER)
    new_count = source.count(NEW_MARKER)
    integrity.append(("old parent outcome predicate occurs exactly once", old_count == 1))
    integrity.append(("actual parent outcome predicate absent before repair", new_count == 0))
    substitutions = int(old_count == 1 and new_count == 0)
    patched = source.replace(OLD_MARKER, NEW_MARKER, 1) if substitutions else source
    integrity.append(("exactly one in-memory substitution", substitutions == 1))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 1:
        namespace = {
            "__file__": str(REPAIR_WRAPPER_V3),
            "__name__": "__main__",
        }
        try:
            with contextlib.redirect_stdout(output):
                exec(
                    compile(patched, str(REPAIR_WRAPPER_V3), "exec"),
                    namespace,
                )
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("repaired FTD-0968 wrapper exit zero", inherited_exit == 0))
    integrity.append((
        "nested FTD-0966 certificate exactly 75/75",
        "checks=75 passed=75 failed=0" in inherited,
    ))
    integrity.append((
        "nested FTD-0967 repair integrity exactly 19/19",
        "repair_checks=19 passed=19 failed=0" in inherited,
    ))
    integrity.append((
        "repaired FTD-0968 integrity exactly 20/20",
        "repair_checks=20 passed=20 failed=0" in inherited,
    ))
    integrity.append((
        "repaired FTD-0968 Outcome B",
        "FTD-0968 OUTCOME B" in inherited,
    ))

    after = {path: sha256(path) for path in EXPECTED}
    for path in EXPECTED:
        integrity.append((f"preserved {path.name}", after[path] == before[path]))

    print("FTD-0969 parent outcome-marker repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0969 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0969 OUTCOME B - complete FTD-0966 certificate chain valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
