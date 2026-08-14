#!/usr/bin/env python3
"""FTD-1001c verifier-only relock wrapper for the FTD-0998/0999 certificate.

Inherits the FTD-0999 marker substitution unchanged and additionally refreshes
the parent's pinned hash of THEOREM_COMMON_RELATIVE_CATALYTIC..., whose bytes
moved under the 2026-08-13 documentation-only transparency amendment (see
PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_RELOCK_v3.md). Carries
forward the FTD-0999 computational/disclosure breakdown report.
"""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CUMULATIVE_CLOCK_GROWTH_ENERGY_RESERVE_AND_BACKPRESSURE_v1.md"
)
PARENT_PROOF = ROOT / (
    "scripts/proofs/proof_cumulative_clock_growth_energy_reserve_and_backpressure.py"
)
PRIOR_REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_REPAIR_v2.md"
)
PRIOR_REPAIR_WRAPPER = ROOT / (
    "scripts/proofs/proof_cumulative_clock_growth_energy_reserve_and_backpressure_v2.py"
)
RELOCK_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_RELOCK_v3.md"
)

PARENT_PROTOCOL_HASH = "6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052"
PARENT_PROOF_HASH = "E8257678700C732214D1A44E69FF5FCBEB31696BB86E6A2F5DB8F611534CD6F0"
PRIOR_REPAIR_PROTOCOL_HASH = "28525592D68887E4795B9E4F9664565C72969DD7828EE22F68970D1C2173EB70"
PRIOR_REPAIR_WRAPPER_HASH = "3FC2DA55EF1DAC8C48CE65AF5B76870981B75F15C191C45D6B62BCD6306961E3"

MARKER_OLD = '"rest-offset-free accounted channels"'
MARKER_NEW = '"rest-offset-free accounted // channels"'
HASH_OLD = "9418AA0841B3122A65B3276525A7B9DEDE89C31FEA563AC4055B8F50EF262110"
HASH_NEW = "95357F142A94FBA2B4A3441429C6C4B81818D19342268092E51622AB34ED2B00"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-1001c cumulative-growth certificate relock integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("hash prior repair protocol", sha256(PRIOR_REPAIR_PROTOCOL) == PRIOR_REPAIR_PROTOCOL_HASH),
        check("hash prior repair wrapper", sha256(PRIOR_REPAIR_WRAPPER) == PRIOR_REPAIR_WRAPPER_HASH),
        check("relock protocol exists", RELOCK_PROTOCOL.exists()),
        check("old marker occurs exactly twice", source.count(MARKER_OLD) == 2),
        check("replacement marker absent", source.count(MARKER_NEW) == 0),
        check("stale hash occurs exactly once", source.count(HASH_OLD) == 1),
        check("current hash absent", source.count(HASH_NEW) == 0),
    ]
    if not all(gates):
        return 1

    repaired = source.replace(MARKER_OLD, MARKER_NEW).replace(HASH_OLD, HASH_NEW, 1)
    gates.append(
        check(
            "authorized substitutions applied exactly",
            repaired.count(MARKER_OLD) == 0
            and repaired.count(MARKER_NEW) == 2
            and repaired.count(HASH_OLD) == 0
            and repaired.count(HASH_NEW) == 1,
        )
    )

    namespace = {"__name__": "ftd_0998_relocked", "__file__": str(PARENT_PROOF)}
    output = io.StringIO()
    exit_code = 1
    with contextlib.redirect_stdout(output):
        try:
            exec(compile(repaired, str(PARENT_PROOF), "exec"), namespace)
            exit_code = 0
        except SystemExit as exc:
            exit_code = int(exc.code or 0)
    inherited = output.getvalue()
    print(inherited, end="")

    gates.extend(
        [
            check("relocked inherited certificate exits zero", exit_code == 0),
            check("all inherited checks pass", "91/91 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )
    passed = sum(gates)
    print(f"relock_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1

    # Carried forward from FTD-0999: recount the parent's own printed
    # [PASS]/[FAIL] lines into computational vs disclosure (G7) buckets.
    check_lines = [
        line for line in inherited.splitlines()
        if line.startswith("[PASS] ") or line.startswith("[FAIL] ")
    ]
    disclosure = [ln for ln in check_lines if ln.startswith("[PASS] G7 ") or ln.startswith("[FAIL] G7 ")]
    computational = [ln for ln in check_lines if ln not in disclosure]
    comp_passed = sum(1 for ln in computational if ln.startswith("[PASS] "))
    disc_passed = sum(1 for ln in disclosure if ln.startswith("[PASS] "))
    print(
        f"FTD-0998 inherited breakdown: "
        f"{comp_passed}/{len(computational)} computational checks passed; "
        f"{disc_passed} disclosure/scope assertions logged (cannot fail) "
        f"[blended headline was {len(check_lines)}/{len(check_lines)} checks passed]"
    )
    print("FTD-1001c OUTCOME B - relocked FTD-0998/0999 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
