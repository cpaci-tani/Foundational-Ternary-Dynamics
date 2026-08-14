#!/usr/bin/env python3
"""FTD-0994 verifier-only repair wrapper for the FTD-0993 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_ZERO_ACTION_CANONICAL_SHEAR_SEED_AND_CAUSAL_BODY_GROWTH_BOUNDARY_v1.md"
PARENT_PROOF = ROOT / "scripts/proofs/proof_zero_action_canonical_shear_seed_and_causal_body_growth_boundary.py"
REPAIR_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_ZERO_ACTION_CANONICAL_SHEAR_CERTIFICATE_REPAIR_v2.md"

PARENT_PROTOCOL_HASH = "9A25D55B35BC32787E8FCBC513B6225B31ADA2E84249AB8F273992F489662753"
PARENT_PROOF_HASH = "4F158B7A8847852D1DEF98E29E30999634FF769B27C56C04E1E39C2048029831"

OLD = "2A93D9CFF23DFFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8"
NEW = "2A93D9CFF23DFFDFEEC5E1F07CB7C023D95FBACC9B05BEA4E3F77775124D87C8"


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-0994 verifier-only repair integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("repair protocol exists", REPAIR_PROTOCOL.exists()),
        check("old hash occurs once", source.count(OLD) == 1),
        check("correct hash absent", source.count(NEW) == 0),
    ]
    if not all(gates):
        return 1

    repaired = source.replace(OLD, NEW, 1)
    gates.append(check("exactly one in-memory substitution", repaired.count(NEW) == 1 and repaired.count(OLD) == 0))

    namespace = {"__name__": "ftd_0993_repaired", "__file__": str(PARENT_PROOF)}
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
            check("all inherited checks pass", "96/96 checks passed" in inherited),
            check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
            check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
            check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
        ]
    )
    passed = sum(gates)
    print(f"repair_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1
    print("FTD-0994 OUTCOME B - repaired FTD-0993 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
