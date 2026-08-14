#!/usr/bin/env python3
"""FTD-0985 one-marker repair wrapper for the FTD-0984 wrapper."""

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
V2_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v2.md"
)
V2_SCRIPT = ROOT / "scripts/proofs/proof_global_work_pair_local_batch_concurrency_v2.py"
V3_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GLOBAL_WORK_PAIR_LOCAL_BATCH_CONCURRENCY_CERTIFICATE_REPAIR_v3.md"
)

EXPECTED = {
    PARENT_PROTOCOL: "4D47C48793A591A54168B4A24EFFBB537EA8F11F6F226C0B52049A3E7CBD8C6C",
    PARENT_SCRIPT: "E985B8EE6952AC494963F0B7DD1A4BD81FEBBD8D72881BCDA8E4C6D8DC733F0D",
    V2_PROTOCOL: "4557C4DDAF9D9A987F84C3779A5659AEAE83A3960DC3119C2A46572FB116FD18",
    V2_SCRIPT: "16E950F3F92C588864E41981F3FA29B19D98FB220BD499394F476DC1344120B0",
    V3_PROTOCOL: "216FDA8F40D22FAE27D9B8388B00F9ECE7500125B47DAB7A5CCB3EE2F2460DD8",
}

OLD = '    integrity.append(("inherited 59/59 pass marker", "59/59 checks passed" in inherited))'
NEW = '    integrity.append(("inherited 62/62 pass marker", "62/62 checks passed" in inherited))'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def main() -> int:
    before = {path: sha256(path) for path in EXPECTED}
    integrity: list[tuple[str, bool]] = [
        (f"hash {path.name}", before[path] == expected)
        for path, expected in EXPECTED.items()
    ]

    source = V2_SCRIPT.read_text(encoding="utf-8")
    integrity.append(("old full-line anchor occurs exactly once", source.count(OLD) == 1))
    integrity.append(("new full-line anchor absent", NEW not in source))
    repaired = source.replace(OLD, NEW, 1)
    integrity.append(("exactly one in-memory substitution", repaired.count(NEW) == 1 and OLD not in repaired))

    output = io.StringIO()
    inherited_exit = 1
    namespace = {"__name__": "ftd0984_repaired_in_memory", "__file__": str(V2_SCRIPT)}
    if all(passed for _, passed in integrity):
        with contextlib.redirect_stdout(output):
            exec(compile(repaired, str(V2_SCRIPT), "exec"), namespace)
            inherited_exit = int(namespace["main"]())

    inherited = output.getvalue()
    print(inherited, end="")
    integrity.append(("inherited repaired wrapper exit zero", inherited_exit == 0))
    integrity.append(("physical certificate 62/62", "62/62 checks passed" in inherited))
    integrity.append(("physical certificate Outcome B", "global clock/action pair is an exact aggregate" in inherited))
    integrity.append(("v2 integrity repaired to 14/14", "repair_checks=14 passed=14 failed=0" in inherited))
    integrity.append(("v2 wrapper Outcome B", "FTD-0984 OUTCOME B" in inherited))

    after = {path: sha256(path) for path in EXPECTED}
    for path in EXPECTED:
        integrity.append((f"preserved {path.name}", after[path] == before[path]))

    print("FTD-0985 marker-only repair integrity")
    for label, passed in integrity:
        print(f"  {'PASS' if passed else 'FAIL'}  {label}")
    passed_count = sum(passed for _, passed in integrity)
    failed_count = len(integrity) - passed_count
    print(f"repair_checks={len(integrity)} passed={passed_count} failed={failed_count}")
    if failed_count:
        print("FTD-0985 OUTCOME D - repair integrity failure")
        return 1
    print("FTD-0985 OUTCOME B - repaired FTD-0983 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
