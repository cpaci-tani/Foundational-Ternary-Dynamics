#!/usr/bin/env python3
"""FTD-1001a verifier-only relock wrapper for the FTD-0993/0994 certificate.

Inherits the FTD-0994 hash-literal repair unchanged and additionally refreshes
the parent's pinned hash of THEOREM_LOCAL_OCCUPANCY_FLIP..., whose bytes moved
under the 2026-08-13 documentation-only transparency amendment (see
PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_RELOCK_v3.md).
"""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md"
)
PARENT_PROOF = ROOT / (
    "scripts/proofs/proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary.py"
)
PRIOR_REPAIR_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_REPAIR_v2.md"
)
RELOCK_PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_RELOCK_v3.md"
)

PARENT_PROTOCOL_HASH = "9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753"
PARENT_PROOF_HASH = "4F158B7A8847852D1DEF98E29E30999634FF769B27C56C04E1E39C2048029831"
PRIOR_REPAIR_PROTOCOL_HASH = "0504086A3D106D3A04B90B20467394D6E2F0F3206E126525F29A89B1345851D9"

REPAIRS = (
    # inherited from FTD-0994: complete the truncated C18 hash literal
    (
        "2A93D9CFF23DFFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8",
        "2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8",
    ),
    # new (relock): refresh the aperture-theorem source pin
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
    print("FTD-1001a zero-action certificate relock integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("hash prior repair protocol", sha256(PRIOR_REPAIR_PROTOCOL) == PRIOR_REPAIR_PROTOCOL_HASH),
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
            "exactly two authorized substitutions",
            all(repaired.count(old) == 0 and repaired.count(new) == 1 for old, new in REPAIRS),
        )
    )

    namespace = {"__name__": "ftd_0993_relocked", "__file__": str(PARENT_PROOF)}
    output = io.StringIO()
    exit_code = 1
    with contextlib.redirect_stdout(output):
        exec(compile(repaired, str(PARENT_PROOF), "exec"), namespace)
        exit_code = int(namespace["main"]())
    inherited = output.getvalue()
    print(inherited, end="")

    gates.extend(
        [
            check("inherited relocked certificate exits zero", exit_code == 0),
            check("all inherited checks pass", "96/96 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )
    passed = sum(gates)
    print(f"relock_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1
    print("FTD-1001a OUTCOME B - relocked FTD-0993/0994 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
