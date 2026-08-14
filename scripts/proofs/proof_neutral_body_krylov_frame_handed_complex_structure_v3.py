#!/usr/bin/env python3
"""FTD-0968 two-site symbol-construction repair wrapper for FTD-0967."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
)
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/"
    "proof_neutral_body_krylov_frame_handed_complex_structure.py"
)
REPAIR_PROTOCOL_V2 = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_CERTIFICATE_IMPLEMENTATION_REPAIR_v2.md"
)
REPAIR_WRAPPER_V2 = (
    ROOT / "scripts/proofs/"
    "proof_neutral_body_krylov_frame_handed_complex_structure_v2.py"
)
REPAIR_PROTOCOL_V3 = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_TWO_SITE_SYMBOL_REPAIR_v3.md"
)

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
}

OLD_R1 = '''NEW_G4 = ''' + "'''" + '''    r1 = sp.Matrix(sp.symbols("r10:3", real=True))
'''
NEW_R1 = '''NEW_G4 = ''' + "'''" + '''    r1 = sp.Matrix(sp.symbols("r1_0:3", real=True))
'''

OLD_R2 = '''    r2 = sp.Matrix(sp.symbols("r20:3", real=True))
    centroid2 = (r1 + r2) / 2
'''
NEW_R2 = '''    r2 = sp.Matrix(sp.symbols("r2_0:3", real=True))
    centroid2 = (r1 + r2) / 2
'''

REPAIRS = (
    (OLD_R1, NEW_R1),
    (OLD_R2, NEW_R2),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = []
    for path, expected in EXPECTED.items():
        integrity.append((f"hash {path.name}", before[path] == expected))

    source = REPAIR_WRAPPER_V2.read_text(encoding="utf-8")
    patched = source
    substitutions = 0
    for index, (old, new) in enumerate(REPAIRS, start=1):
        old_count = patched.count(old)
        new_count = patched.count(new)
        integrity.append((f"old NEW_G4 anchor {index} occurs exactly once", old_count == 1))
        integrity.append((f"new NEW_G4 anchor {index} absent before repair", new_count == 0))
        if old_count == 1 and new_count == 0:
            patched = patched.replace(old, new, 1)
            substitutions += 1
    integrity.append(("exactly two in-memory substitutions", substitutions == 2))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 2:
        namespace = {
            "__file__": str(REPAIR_WRAPPER_V2),
            "__name__": "__main__",
        }
        try:
            with contextlib.redirect_stdout(output):
                exec(
                    compile(patched, str(REPAIR_WRAPPER_V2), "exec"),
                    namespace,
                )
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("repaired FTD-0967 wrapper exit zero", inherited_exit == 0))
    integrity.append((
        "inherited FTD-0966 certificate exactly 75/75",
        "checks=75 passed=75 failed=0" in inherited,
    ))
    integrity.append((
        "inherited FTD-0966 Outcome B",
        "FTD-0966 OUTCOME B" in inherited,
    ))
    integrity.append((
        "FTD-0967 repair integrity exactly 19/19",
        "repair_checks=19 passed=19 failed=0" in inherited,
    ))
    integrity.append((
        "FTD-0967 repaired certificate Outcome B",
        "FTD-0967 OUTCOME B" in inherited,
    ))

    after = {path: sha256(path) for path in EXPECTED}
    for path in EXPECTED:
        integrity.append((f"preserved {path.name}", after[path] == before[path]))

    print("FTD-0968 two-site symbol repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0968 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0968 OUTCOME B - FTD-0966 certificate valid after scoped repairs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
