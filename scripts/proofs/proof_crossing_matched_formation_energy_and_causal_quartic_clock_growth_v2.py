#!/usr/bin/env python3
"""FTD-0996 verifier-only repair wrapper for the FTD-0995 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CROSSING_MATCHED_FORMATION_ENERGY_AND_CAUSAL_QUARTIC_CLOCK_GROWTH_v1.md"
)
PARENT_PROOF = ROOT / (
    "scripts/proofs/"
    "proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth.py"
)
REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md"
)

PARENT_PROTOCOL_HASH = "B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F"
PARENT_PROOF_HASH = "17DE90F5BBEFD1BDEFC22AACB236C024FBE8446BD5DE765AA7F95B79EDD87574"
REPAIR_PROTOCOL_HASH = "854C1EDA934DA8CDFA1B0C2649EF9CE2A20C4D6A30731D28A2C285BDB7379554"

REPAIRS = (
    (
        '"E_join-E_cut" in formation_text',
        '"E_{\\\\rm join}-E_{\\\\rm cut}" in formation_text',
    ),
    (
        "sp.simplify(forward_energy_change - (U + sigma_symbol * momentum * root)) == 0,",
        "sp.simplify((forward_energy_change - (U + sigma_symbol * momentum * root)).subs(sigma_symbol**2, 1)) == 0,",
    ),
    (
        'sp.simplify(forward_energy_change.subs(momentum, 0) - U) == 0)',
        'sp.simplify((forward_energy_change.subs(momentum, 0) - U).subs(sigma_symbol**2, 1)) == 0)',
    ),
    (
        'K * uniform_q == sp.zeros(3, 1)',
        '(K * uniform_q).applyfunc(sp.simplify) == sp.zeros(3, 1)',
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-0996 verifier-only repair integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("hash repair protocol", sha256(REPAIR_PROTOCOL) == REPAIR_PROTOCOL_HASH),
    ]

    for index, (old, new) in enumerate(REPAIRS, start=1):
        gates.append(check(f"repair {index} old fragment occurs once", source.count(old) == 1))
        gates.append(check(f"repair {index} new fragment absent", source.count(new) == 0))

    if not all(gates):
        return 1

    repaired = source
    for old, new in REPAIRS:
        repaired = repaired.replace(old, new, 1)

    gates.append(
        check(
            "exactly four authorized substitutions",
            all(repaired.count(old) == 0 and repaired.count(new) == 1 for old, new in REPAIRS),
        )
    )

    namespace = {"__name__": "ftd_0995_repaired", "__file__": str(PARENT_PROOF)}
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
            check("all inherited checks pass", "88/88 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )

    passed = sum(gates)
    print(f"repair_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1
    print("FTD-0996 OUTCOME B - repaired FTD-0995 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
