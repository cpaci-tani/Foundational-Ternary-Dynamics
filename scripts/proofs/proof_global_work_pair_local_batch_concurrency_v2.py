#!/usr/bin/env python3
"""FTD-0984 two-call verifier repair for the FTD-0983 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GLOBAL_WORK_PAIR_VERSUS_LOCAL_BATCH_CONCURRENCY_v1.md"
)
PARENT_SCRIPT = ROOT / "scripts/proofs/proof_global_work_pair_local_batch_concurrency.py"
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v2.md"
)

EXPECTED = {
    PARENT_PROTOCOL: "4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C",
    PARENT_SCRIPT: "E985B8EE6952AC494963F0B7DD1A4BD81FEBBD8D72881BCDA8E4C6D8DC733F0D",
    REPAIR_PROTOCOL: "4557C4DDAF9D9A987F84C3779A5659AEAE83A3960DC3119C2A46572FB116FD18",
}

OLD_TOKEN = "sp.simpl("
NEW_TOKEN = "sp.simplify("
OLD_LINES = (
    '    cert.check("G4 local energy one", sp.simpl(h1_out + i1 + local_w1_zero - h1 - i1) == 0, "H1+I1")',
    '    cert.check("G4 local energy two", sp.simpl(h2_out + i2 + local_w2_zero - h2 - i2) == 0, "H2+I2")',
)
NEW_LINES = tuple(line.replace(OLD_TOKEN, NEW_TOKEN) for line in OLD_LINES)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = [
        (f"hash {path.name}", before[path] == expected)
        for path, expected in EXPECTED.items()
    ]

    source = PARENT_SCRIPT.read_text(encoding="utf-8")
    integrity.append(("old token occurs exactly twice", source.count(OLD_TOKEN) == 2))
    for index, line in enumerate(OLD_LINES, start=1):
        integrity.append((f"old full-line anchor {index} occurs exactly once", source.count(line) == 1))
    for index, line in enumerate(NEW_LINES, start=1):
        integrity.append((f"new full-line anchor {index} absent", line not in source))

    repaired = source.replace(OLD_TOKEN, NEW_TOKEN)
    integrity.append(("exactly two in-memory substitutions", repaired.count(NEW_TOKEN) == source.count(NEW_TOKEN) + 2))

    output = io.StringIO()
    inherited_exit = 1
    namespace = {"__name__": "ftd0983_repaired_in_memory", "__file__": str(PARENT_SCRIPT)}
    if all(passed for _, passed in integrity):
        with contextlib.redirect_stdout(output):
            exec(compile(repaired, str(PARENT_SCRIPT), "exec"), namespace)
            inherited_exit = int(namespace["main"]())

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired certificate exit zero", inherited_exit == 0))
    integrity.append(("inherited 59/59 pass marker", "59/59 checks passed" in inherited))
    integrity.append(("inherited Outcome B unchanged", "OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    integrity.append(("parent protocol preserved", after[PARENT_PROTOCOL] == before[PARENT_PROTOCOL]))
    integrity.append(("parent certificate preserved", after[PARENT_SCRIPT] == before[PARENT_SCRIPT]))

    print("FTD-0984 verifier-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(f"repair_checks={len(integrity)} passed={passed_count} failed={failed_count}")
    if failed_count:
        print("FTD-0984 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0984 OUTCOME B - repaired FTD-0983 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
