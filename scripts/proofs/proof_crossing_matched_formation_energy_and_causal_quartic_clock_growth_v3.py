#!/usr/bin/env python3
"""FTD-1001b verifier-only relock wrapper for the FTD-0995/0996 certificate.

Inherits the four FTD-0996 substitutions unchanged, pins the 2026-08-13
scope-corrected amendment of that repair protocol at its current hash, and
additionally refreshes the parent's pinned hash of
THEOREM_LOCAL_OCCUPANCY_FLIP..., whose bytes moved under the same day's
documentation-only transparency amendment (see
PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_RELOCK_v3.md).
"""

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
    "scripts/proofs/proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth.py"
)
AMENDED_REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_REPAIR_v2.md"
)
PRIOR_REPAIR_WRAPPER = ROOT / (
    "scripts/proofs/proof_crossing_matched_formation_energy_and_causal_quartic_clock_growth_v2.py"
)
RELOCK_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_CROSSING_MATCHED_CLOCK_GROWTH_CERTIFICATE_RELOCK_v3.md"
)

PARENT_PROTOCOL_HASH = "B1113C02CFF82C0BD2F14D77FA5C661AC290243C2CC4C94AF9C552E9D665957F"
PARENT_PROOF_HASH = "17DE90F5BBEFD1BDEFC22AACB236C024FBE8446BD5DE765AA7F95B79EDD87574"
AMENDED_REPAIR_PROTOCOL_HASH = "36B7E3F0C3645E28605633BD2A46E0F8EB64C9D852BEDAF513F883C3DDE8B12D"
PRIOR_REPAIR_WRAPPER_HASH = "9104D6F3FD842C8BF09C7F35BC080BCCCA96EFCC0F4022CAAAF9DF3846B130E2"

REPAIRS = (
    # 1-4 inherited verbatim from FTD-0996 (PREREG ...REPAIR_v2.md section 3)
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
    # 5 new (relock): refresh the aperture-theorem source pin
    (
        "E4D4BBCF2A0E09953EA2107FD80954E50BB2ED9BE45A9C9C6D2381DA018D7B9F",
        "C1AFBB93596DC60AC9C5EDB600843EA0650D1A78ECCC39339A7EAC3ABF75B142",
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-1001b crossing-matched certificate relock integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("hash amended repair protocol", sha256(AMENDED_REPAIR_PROTOCOL) == AMENDED_REPAIR_PROTOCOL_HASH),
        check("hash prior repair wrapper", sha256(PRIOR_REPAIR_WRAPPER) == PRIOR_REPAIR_WRAPPER_HASH),
        check("relock protocol exists", RELOCK_PROTOCOL.exists()),
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
            "exactly five authorized substitutions",
            all(repaired.count(old) == 0 and repaired.count(new) == 1 for old, new in REPAIRS),
        )
    )

    namespace = {"__name__": "ftd_0995_relocked", "__file__": str(PARENT_PROOF)}
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
            check("inherited relocked certificate exits zero", exit_code == 0),
            check("all inherited checks pass", "88/88 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )
    passed = sum(gates)
    print(f"relock_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1
    print("FTD-1001b OUTCOME B - relocked FTD-0995/0996 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
