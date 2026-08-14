#!/usr/bin/env python3
"""FTD-0972 verifier-marker-only repair wrapper for FTD-0971."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md"
)
PARENT_SCRIPT = (
    ROOT / "scripts/proofs/"
    "proof_krylov_degeneracy_ternary_latch_oriented_c4.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_KRYLOV_DEGENERACY_C4_CERTIFICATE_MARKER_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL:
        "85E6BA5B4CEFC7CDBF70A5CB903C19D3E6230632889DE70927A4C1E5FF28C8E5",
    PARENT_SCRIPT:
        "F8C44B012A3FDF60B974327B95D8534A5EA1E48351F59DA10CE4C331C11169D0",
    REPAIR_PROTOCOL:
        "4F69BFBB20BDB277BF76244F1FAAA3752090421CFBC8A22A63B2688F4647D574",
}

REPAIRS = (
    (
        '        "minimum self-delimiting reversible transition",\n',
        '        "minimum\\nself-delimiting reversible transition",\n',
    ),
    (
        '        "This is not a universal one-bit claim",\n',
        '        "This is not a universal one-bit\\nclaim",\n',
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
        integrity.append((f"old marker {index} occurs exactly once", old_count == 1))
        integrity.append((f"new marker {index} absent before repair", new_count == 0))
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
    integrity.append(("inherited certificate exactly 62/62", "checks=62 passed=62 failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "FTD-0971 OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    for path in EXPECTED:
        integrity.append((f"preserved {path.name}", after[path] == before[path]))

    print("FTD-0972 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0972 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0972 OUTCOME B - repaired FTD-0971 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
