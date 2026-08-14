#!/usr/bin/env python3
"""FTD-0963 verifier-only repair wrapper for the FTD-0962 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_REVERSIBLE_GEARBOX_v1.md"
)
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_oriented_phase_connection_token_loading_reversible_gearbox.py"
REPAIR_PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ORIENTED_PHASE_CONNECTION_GEARBOX_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL: "535E14ADE46A886542165A815C4B807DFE35B0ACBD7B6131342C4DB9126C96B0",
    PARENT_SCRIPT: "8AD520B137323AFD02961109C84CD8B09B49D8FF7FD393803B950D08155E8F98",
    REPAIR_PROTOCOL: "A01C5F9ECCA3E62E6F59052636073DBB71EB6E59A6CD9AE0C5B717D71F3586E3",
}

REPAIRS = (
    (
        '"merely linear term",',
        '"A merely\\nlinear term",',
    ),
    (
        '"Reference success cannot count as substrate evidence",',
        '"Reference success cannot count as\\nsubstrate evidence",',
    ),
    (
        '"backreaction changes the pulse area",',
        '"backreaction changes\\nthe clock rate and makes the pulse area",',
    ),
    (
        '"signed phase current",',
        '"signed\\nphase current",',
    ),
    (
        '"No relation between `Omega/kappa` and `G*`",',
        '"not derive that gearbox, select its integer, or identify either frequency with\\nthe critical-quartic `G*` calendar",',
    ),
    (
        '"no new public memory type",',
        '"does not need another public memory type",',
    ),
    (
        '"costs one token",',
        '"one token and therefore requires",',
    ),
    (
        '"K^2/(2M)"',
        '"{K^2\\\\over2M}"',
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
    integrity.append(("exactly eight in-memory substitutions", substitutions == 8))

    output = io.StringIO()
    inherited_exit = 1
    if substitutions == 8:
        namespace = {"__file__": str(PARENT_SCRIPT), "__name__": "__main__"}
        try:
            with contextlib.redirect_stdout(output):
                exec(compile(patched, str(PARENT_SCRIPT), "exec"), namespace)
        except SystemExit as exc:
            inherited_exit = int(exc.code or 0)

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited 115/115 pass marker", "checks=115 passed=115 failed=0" in inherited))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0963 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(
        f"repair_checks={len(integrity)} passed={passed_count} "
        f"failed={failed_count}"
    )
    if failed_count:
        print("FTD-0963 OUTCOME D — repair integrity failure")
        return 1
    print("FTD-0963 OUTCOME B — repaired FTD-0962 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
