#!/usr/bin/env python3
"""FTD-0961 verifier-only repair wrapper for the FTD-0960 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_BOUNDARY_v1.md"
)
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_existing_oriented_rail_finite_winding_carrier_boundary.py"
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_EXISTING_ORIENTED_RAIL_FINITE_WINDING_CARRIER_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL: "B8BDCCCDEB5ECFE4FE2B9CAAD1C00AAF69C5E5F6CD0E4266866FBDF79A6ADDBA",
    PARENT_SCRIPT: "EAF1890622606B584EB3473FB6D5444C52CAB79B38AA8E70A808AE28CA6A28C8",
    REPAIR_PROTOCOL: "2A5D0CE0857C5EB218D979071A313A02C66EFD3681F0679F57EA25BBBF9CE336",
}

REPAIRS = (
    (
        '"No completed infinite rail is assumed",',
        '"No completed\\ninfinite rail is assumed",',
    ),
    (
        '"Loading an event port is a separate transaction",',
        '"Loading an event port is a\\nseparate transaction",',
    ),
    (
        '"fresh causal\\n+front, tail export, or backpressure"',
        '"fresh causal\\nfront, tail export, or backpressure"',
    ),
    (
        '"occupied support and token energy\\n+grow with the retained history"',
        '"occupied support and token energy\\ngrow with the retained history"',
    ),
    (
        '"do not by themselves supply a nonlinear\\n+compact carry/overflow transaction"',
        '"do not by themselves supply a nonlinear\\ncompact carry/overflow transaction"',
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

    integrity.append(("exactly five in-memory substitutions", substitutions == 5))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 5:
        namespace = {"__file__": str(PARENT_SCRIPT), "__name__": "__main__"}
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(patched, str(PARENT_SCRIPT), "exec"), namespace)
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited 60/60 pass marker", "checks=60 passed=60 failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0961 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0961 OUTCOME D — repair integrity failure")
        return 1
    print("FTD-0961 OUTCOME B — repaired FTD-0960 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
