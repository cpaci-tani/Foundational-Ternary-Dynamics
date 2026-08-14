#!/usr/bin/env python3
"""FTD-0999 marker-only repair wrapper for the FTD-0998 certificate."""

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
    "scripts/proofs/"
    "proof_cumulative_clock_growth_energy_reserve_and_backpressure.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CUMULATIVE_CLOCK_GROWTH_RESOURCE_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "6E0B28E7487B7E285EE05F7A16CDAC58984077D2964CC1042931996FFB884052"
PARENT_PROOF_HASH = "E8257678700C732214D1A44E69FF5FCBEB31696BB86E6A2F5DB8F611534CD6F0"
REPAIR_PROTOCOL_HASH = "28525592D68887E4795B9E4F9664565C72969DD7828EE22F68970D1C2173EB70"

OLD = '"rest-offset-free accounted channels"'
NEW = '"rest-offset-free accounted // channels"'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-0999 cumulative-growth resource marker repair integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("hash repair protocol", sha256(REPAIR_PROTOCOL) == REPAIR_PROTOCOL_HASH),
        check("old marker occurs exactly twice", source.count(OLD) == 2),
        check("replacement marker absent", source.count(NEW) == 0),
    ]

    if not all(gates):
        return 1

    repaired = source.replace(OLD, NEW)
    gates.append(
        check(
            "exactly two marker occurrences repaired",
            repaired.count(OLD) == 0 and repaired.count(NEW) == 2,
        )
    )

    namespace = {"__name__": "ftd_0998_repaired", "__file__": str(PARENT_PROOF)}
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
            check("repaired inherited certificate exits zero", exit_code == 0),
            check("all inherited checks pass", "91/91 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )

    passed = sum(gates)
    print(f"repair_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1

    # Split the inherited "91/91 checks passed" headline into the two kinds of
    # gate the parent script itself already distinguishes by name: its G7
    # section (see the parent's own "G7: epistemic, ontology, physical, and
    # production firewalls" comment) is disclosure/scope assertions declared
    # via P.check(name, True, note) and cannot fail; every other section is a
    # symbolic/hash/combinatorial check that can. This reclassifies nothing
    # and reruns nothing — it only recounts the [PASS]/[FAIL] lines already
    # printed by the parent, verified above to be 91/91.
    check_lines = [
        line for line in inherited.splitlines()
        if line.startswith("[PASS] ") or line.startswith("[FAIL] ")
    ]
    is_disclosure = [
        line.startswith("[PASS] G7 ") or line.startswith("[FAIL] G7 ")
        for line in check_lines
    ]
    disclosure_lines = [line for line, flag in zip(check_lines, is_disclosure) if flag]
    computational_lines = [line for line, flag in zip(check_lines, is_disclosure) if not flag]
    computational_passed = sum(1 for line in computational_lines if line.startswith("[PASS] "))
    disclosure_passed = sum(1 for line in disclosure_lines if line.startswith("[PASS] "))
    print(
        f"FTD-0998 inherited breakdown: "
        f"{computational_passed}/{len(computational_lines)} computational checks passed; "
        f"{disclosure_passed} disclosure/scope assertions logged (cannot fail) "
        f"[blended headline was {len(check_lines)}/{len(check_lines)} checks passed]"
    )
    print("FTD-0999 OUTCOME B - repaired FTD-0998 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
