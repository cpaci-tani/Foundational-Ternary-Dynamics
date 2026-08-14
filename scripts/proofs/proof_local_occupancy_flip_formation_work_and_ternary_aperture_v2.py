#!/usr/bin/env python3
"""FTD-0992 verifier-only repair wrapper for the FTD-0991 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
import re
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_WORK_AND_TERNARY_APERTURE_v1.md"
PARENT_PROOF = ROOT / "scripts/proofs/proof_local_occupancy_flip_formation_work_and_ternary_aperture.py"
REPAIR_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_LOCAL_OCCUPANCY_FLIP_FORMATION_CERTIFICATE_REPAIR_v2.md"

PARENT_PROTOCOL_HASH = "34A71B6E77DBB23FA0D256F0032A5A708405F67CDA63D59AC756A15CA49062E7"
PARENT_PROOF_HASH = "A8F0D61500C5878036E25B4CBEA4148FDD72BC64BDDF94D130EA08BFB38BBA16"

OLD_ACTION = '"H_u=\\\\omega I_u" in clutch'
NEW_ACTION = '"H_u=omega I" in clutch'
OLD_FIREWALL = "firewall in protocol_text"
NEW_FIREWALL = "firewall in protocol_norm"

# Reporting-only classification, not a re-evaluation: these are the two
# FTD-0991 checks (parent script, F3 block) whose asserted condition reduces
# to a literal self-cancellation (X-X==0 / X+(-X)==0) with no independent
# fact from any theorem/doc/source file entering the comparison -- they
# cannot fail for any input. Every other inherited check is a genuine
# computed/compared assertion (hash compare, string count/containment, or
# sympy/Python evaluation against an external fact) and is reported as
# computational. This constant changes no check's logic, tolerance, or
# pass/fail outcome -- it only labels which already-printed PASS/FAIL line
# belongs in which reporting bucket.
VACUOUS_LABELS = frozenset(
    {
        "F3 growth work join minus cut",
        "F3 reverse work exact",
    }
)

CHECK_LINE_RE = re.compile(r"^  (PASS|FAIL)  (.+)$")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def classify_inherited(inherited: str) -> tuple[int, int, int, int]:
    """Split the inherited FTD-0991 PASS/FAIL lines into computational vs
    disclosure/scope (structurally-cannot-fail) buckets by exact label match
    against VACUOUS_LABELS. Pure re-count of already-printed outcomes --
    does not touch, re-run, or reinterpret any check."""
    comp_passed = comp_total = 0
    vacuous_passed = vacuous_total = 0
    for line in inherited.splitlines():
        match = CHECK_LINE_RE.match(line)
        if not match:
            continue
        status, rest = match.groups()
        label = rest.split(": ", 1)[0]
        is_pass = status == "PASS"
        if label in VACUOUS_LABELS:
            vacuous_total += 1
            vacuous_passed += int(is_pass)
        else:
            comp_total += 1
            comp_passed += int(is_pass)
    return comp_passed, comp_total, vacuous_passed, vacuous_total


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-0992 verifier-only repair integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("repair protocol exists", REPAIR_PROTOCOL.exists()),
        check("old action predicate occurs once", source.count(OLD_ACTION) == 1),
        check("new action predicate absent", source.count(NEW_ACTION) == 0),
        check("old firewall predicate occurs once", source.count(OLD_FIREWALL) == 1),
        check("new firewall predicate absent", source.count(NEW_FIREWALL) == 0),
    ]
    if not all(gates):
        return 1

    repaired = source.replace(OLD_ACTION, NEW_ACTION, 1)
    repaired = repaired.replace(OLD_FIREWALL, NEW_FIREWALL, 1)
    gates.append(
        check(
            "exactly two in-memory substitutions",
            repaired.count(NEW_ACTION) == 1
            and repaired.count(NEW_FIREWALL) == 1
            and repaired.count(OLD_ACTION) == 0
            and repaired.count(OLD_FIREWALL) == 0,
        )
    )

    namespace = {"__name__": "ftd_0991_repaired", "__file__": str(PARENT_PROOF)}
    output = io.StringIO()
    exit_code = 1
    with contextlib.redirect_stdout(output):
        exec(compile(repaired, str(PARENT_PROOF), "exec"), namespace)
        exit_code = int(namespace["main"]())
    inherited = output.getvalue()
    print(inherited, end="")

    gates.extend(
        [
            check("inherited repaired certificate exits zero", exit_code == 0),
            check("all inherited checks pass", "83/83 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )
    passed = sum(gates)
    print(f"repair_checks={len(gates)} passed={passed} failed={len(gates)-passed}")

    # v2's own 13 wrapper gates (hash/text/exit-code comparisons above) are
    # all genuine computed checks in the same sense as the inherited
    # non-vacuous ones -- none of them reduce to a literal self-cancellation
    # -- so they fold into the computational bucket alongside the inherited
    # F1/F2/F4/F5/F6 checks and the 81 of 83 F3-block checks that compare an
    # independent fact. Only the 2 inherited F3 tautologies are disclosure/
    # scope assertions that cannot fail. This is a re-presentation of the
    # same 96 already-computed PASS/FAIL outcomes above, not a new tally.
    comp_passed, comp_total, vacuous_passed, vacuous_total = classify_inherited(inherited)
    comp_passed += passed
    comp_total += len(gates)
    print(
        f"{comp_passed}/{comp_total} computational checks passed; "
        f"{vacuous_total} disclosure/scope assertions logged (cannot fail)"
    )

    if not all(gates):
        return 1
    print("FTD-0992 OUTCOME B - repaired FTD-0991 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
