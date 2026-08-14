#!/usr/bin/env python3
"""FTD-0987 two-line verifier repair for the FTD-0986 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NATIVE_COMMON_MODE_WORK_PORT_OWNERSHIP_DISCRIMINATOR_v1.md"
)
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_native_common_mode_work_port_ownership.py"
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NATIVE_COMMON_MODE_WORK_PORT_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL: "7E5E00C9262D3E6AF5D2BBD41D7F2845D4744D902157C32BADA7F6787D86AECF",
    PARENT_SCRIPT: "88B3296231CAFA4F98E7778B82BE00538D9E66BD3EC54BE1E628F0E1EFBD5DD3",
    REPAIR_PROTOCOL: "FBEA4B287636CFEBF0D29C4B8A14B1FFB0E880F11A6EF03D8803426D9EAB7D7A",
}

REPAIRS = (
    (
        '    cert.check("W1 local ownership debt inherited", "prepositioned complete local work pairs" in ownership_norm)',
        '    cert.check("W1 local ownership debt inherited", "one complete local pair" in ownership_norm)',
    ),
    (
        '    cert.check("W5 upper extremal coefficient", sp.expand(extremal_term).coeff(z, m + 1) == -b * u_max)',
        '    cert.check("W5 upper extremal coefficient", sp.expand(sp.cancel(extremal_term / z**m)).coeff(z, 1) == -b * u_max)',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = [
        (f"hash {path.name}", before[path] == expected)
        for path, expected in EXPECTED.items()
    ]

    source = PARENT_SCRIPT.read_text(encoding="utf-8")
    repaired = source
    substitutions = 0
    for index, (old, new) in enumerate(REPAIRS, start=1):
        integrity.append((f"old full-line anchor {index} occurs once", repaired.count(old) == 1))
        integrity.append((f"new full-line anchor {index} absent", new not in repaired))
        if repaired.count(old) == 1 and new not in repaired:
            repaired = repaired.replace(old, new, 1)
            substitutions += 1
    integrity.append(("exactly two in-memory substitutions", substitutions == 2))

    output = io.StringIO()
    inherited_exit = 1
    namespace = {"__name__": "ftd0986_repaired_in_memory", "__file__": str(PARENT_SCRIPT)}
    if all(passed for _, passed in integrity):
        with contextlib.redirect_stdout(output):
            exec(compile(repaired, str(PARENT_SCRIPT), "exec"), namespace)
            inherited_exit = int(namespace["main"]())

    inherited = output.getvalue()
    print(inherited, end="")
    full_pass = re.search(r"exact certificate: (\d+)/\1 checks passed", inherited)
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("all inherited checks pass", full_pass is not None))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0987 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(f"repair_checks={len(integrity)} passed={passed_count} failed={failed_count}")
    if failed_count:
        print("FTD-0987 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0987 OUTCOME B - repaired FTD-0986 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
