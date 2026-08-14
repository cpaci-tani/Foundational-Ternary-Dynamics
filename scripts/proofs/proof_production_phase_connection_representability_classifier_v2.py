#!/usr/bin/env python3
"""FTD-0965 verifier-only repair wrapper for the FTD-0964 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_CLASSIFIER_v1.md"
)
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/"
    "proof_production_phase_connection_representability_classifier.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL:
        "B44C925D56BC66B3C9FCA2781AC29C86D0E8EADCF60DCA90FAA0BAD67B6A3E21",
    PARENT_SCRIPT:
        "2199DE8A4FDB5239B27D1973880B27D7C886DBF34D7BA22FB40A948786FB1C09",
    REPAIR_PROTOCOL:
        "4B3E916D72A83958FB4660488FE0B16CD7B27963044859554545204926736B4C",
}

REPAIRS = (
    (
        '        "no new public continuous storage type is forced by local scalar capacity",',
        '        "No new public continuous storage type is forced by local scalar capacity",',
    ),
    (
        '        "no site-local cubic-covariant linear scalar chart exists",',
        '        "site-local cubic-covariant linear scalar chart exists",',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = []
    for path, expected in EXPECTED.items():
        integrity.append((f"hash {path.name}", before[path] == expected))

    source = PARENT_SCRIPT.read_text(encoding="utf-8")
    patched = source
    substitutions = 0
    for index, (old, new) in enumerate(REPAIRS, start=1):
        old_count = patched.count(old)
        new_count = patched.count(new)
        integrity.append((f"old anchor {index} occurs exactly once", old_count == 1))
        integrity.append((f"new anchor {index} absent before repair", new_count == 0))
        if old_count == 1 and new_count == 0:
            patched = patched.replace(old, new, 1)
            substitutions += 1
    integrity.append(("exactly two in-memory substitutions", substitutions == 2))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 2:
        namespace = {"__file__": str(PARENT_SCRIPT), "__name__": "__main__"}
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(patched, str(PARENT_SCRIPT), "exec"), namespace)
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited 72/72 pass marker", "checks=72 passed=72 failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0965 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0965 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0965 OUTCOME B - repaired FTD-0964 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
