#!/usr/bin/env python3
"""FTD-0989 verifier-only repair wrapper for the FTD-0988 certificate."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PARENT_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_C18_BOND_CLUTCH_CURRENT_AND_RECIPROCAL_SWITCHING_DISCRIMINATOR_v1.md"
PARENT_PROOF = ROOT / "scripts/proofs/proof_c18_bond_clutch_current_and_reciprocal_switching.py"
REPAIR_PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_C18_BOND_CLUTCH_CERTIFICATE_REPAIR_v2.md"

PARENT_PROTOCOL_HASH = "B85BAAA418F0BFF2AE67678BDB1FBD25532EB1CEC9FF596F2325F8D00AE169DD"
PARENT_PROOF_HASH = "FA0A0A5885612959D5AC782F8AF396A73A275840F08AC3047F1DF6A69859FAD1"

OLD = '"H+2I identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit"'
NEW = '"H+2I` identity remains exact only for the explicitly non-Hamiltonian observable-amplitude audit"'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}")
    return bool(condition)


def main() -> int:
    print("FTD-0989 verifier-only repair integrity")
    before_protocol = PARENT_PROTOCOL.read_bytes()
    before_proof = PARENT_PROOF.read_bytes()
    source = before_proof.decode("utf-8")

    gates = [
        check("hash parent protocol", sha256(PARENT_PROTOCOL) == PARENT_PROTOCOL_HASH),
        check("hash parent proof", sha256(PARENT_PROOF) == PARENT_PROOF_HASH),
        check("repair protocol exists", REPAIR_PROTOCOL.exists()),
        check("old predicate occurs once", source.count(OLD) == 1),
        check("new predicate absent", source.count(NEW) == 0),
    ]
    if not all(gates):
        return 1

    repaired = source.replace(OLD, NEW, 1)
    gates.append(check("exactly one in-memory substitution", repaired.count(NEW) == 1 and repaired.count(OLD) == 0))

    namespace = {"__name__": "ftd_0988_repaired", "__file__": str(PARENT_PROOF)}
    output = io.StringIO()
    exit_code = 1
    with contextlib.redirect_stdout(output):
        exec(compile(repaired, str(PARENT_PROOF), "exec"), namespace)
        exit_code = int(namespace["main"]())
    inherited = output.getvalue()
    print(inherited, end="")

    gates.extend([
        check("inherited repaired certificate exits zero", exit_code == 0),
        check("all inherited checks pass", "73/73 checks passed" in inherited),
        check("inherited Outcome B unchanged", "OUTCOME B" in inherited),
        check("parent protocol preserved", PARENT_PROTOCOL.read_bytes() == before_protocol),
        check("parent proof preserved", PARENT_PROOF.read_bytes() == before_proof),
    ])
    passed = sum(gates)
    print(f"repair_checks={len(gates)} passed={passed} failed={len(gates)-passed}")
    if not all(gates):
        return 1
    print("FTD-0989 OUTCOME B - repaired FTD-0988 certificate valid")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

