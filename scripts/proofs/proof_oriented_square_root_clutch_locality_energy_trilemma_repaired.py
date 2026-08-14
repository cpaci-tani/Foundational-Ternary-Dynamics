#!/usr/bin/env python3
"""FTD-0980 one-substitution wrapper for the immutable FTD-0979 proof."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PARENT_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md"
)
REPAIR_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_CERTIFICATE_REPAIR_v2.md"
)
PARENT = ROOT / "scripts/proofs/proof_oriented_square_root_clutch_locality_energy_trilemma.py"

EXPECTED_PARENT_PROTOCOL = "5747E0991BD6984B86B8A9522AD3F9B2927E8AADEDEF0D50C2C826DF7EA185C4"
EXPECTED_REPAIR_PROTOCOL = "D98611D1BB42D3CA61CCE17964C405C7E0832BD16DFF7D47882C1C5D6FE5D985"
EXPECTED_PARENT = "814B2B6760E29129BA6616AE1BC6CC047D6DCFD20BCFDCDA8BCC054D9A3D2C92"

OLD = """        mu2 not in (sp.expand(k_laurent).coeff(z, 1), sp.expand(k_laurent * z).coeff(z, 0)).free_symbols,
"""
NEW = """        mu2 not in sp.expand(k_laurent).coeff(z, 1).free_symbols
        and mu2 not in sp.expand(k_laurent * z).coeff(z, 0).free_symbols,
"""


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool, detail: object = "") -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}: {detail}")
    return bool(condition)


def main() -> int:
    print("=" * 79)
    print("FTD-0980 oriented square-root verifier-only repair")
    print("=" * 79)
    integrity: list[bool] = []
    integrity.append(check("R1 parent protocol hash", sha256(PARENT_PROTOCOL) == EXPECTED_PARENT_PROTOCOL, sha256(PARENT_PROTOCOL)))
    integrity.append(check("R2 repair protocol hash", sha256(REPAIR_PROTOCOL) == EXPECTED_REPAIR_PROTOCOL, sha256(REPAIR_PROTOCOL)))
    integrity.append(check("R3 parent proof hash", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    repair_text = REPAIR_PROTOCOL.read_text(encoding="utf-8")
    integrity.append(check("R4 one-substitution scope", "Exactly one in-memory source substitution" in repair_text, "verifier only"))
    integrity.append(check("R5 no gate waiver", "No other source substitution, assertion change, gate waiver" in repair_text, "all inherited"))

    parent_source = PARENT.read_text(encoding="utf-8")
    integrity.append(check("R6 old anchor occurs once", parent_source.count(OLD) == 1, parent_source.count(OLD)))
    integrity.append(check("R7 new anchor absent from parent", parent_source.count(NEW) == 0, parent_source.count(NEW)))
    repaired_source = parent_source.replace(OLD, NEW)
    integrity.append(check("R8 new anchor inserted once", repaired_source.count(NEW) == 1, repaired_source.count(NEW)))
    integrity.append(check("R9 old anchor removed in memory", repaired_source.count(OLD) == 0, repaired_source.count(OLD)))
    integrity.append(check("R10 parent remains byte frozen", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    if not all(integrity):
        print("FTD-0980 OUTCOME D - wrapper integrity invalid")
        return 1

    namespace = {"__name__": "ftd0980_repaired_parent", "__file__": str(PARENT)}
    exec(compile(repaired_source, f"{PARENT}::FTD-0980", "exec"), namespace)
    captured = io.StringIO()
    with contextlib.redirect_stdout(captured):
        parent_rc = namespace["main"]()
    parent_output = captured.getvalue()
    print(parent_output, end="")

    terminal: list[bool] = []
    terminal.append(check("R11 inherited return code", parent_rc == 0, parent_rc))
    terminal.append(check("R12 inherited Outcome B", "FTD-0979 OUTCOME B" in parent_output, "exact root/trilemma"))
    terminal.append(check("R13 no inherited Outcome D", "FTD-0979 OUTCOME D" not in parent_output, "no failure"))
    terminal.append(check("R14 all inherited checks pass", "failed=0" in parent_output, "complete"))
    terminal.append(check("R15 locality boundary retained", "SCALAR_ENERGY_COMPATIBLE_ROOT=MODAL_NONLOCAL_FOR_C18" in parent_output, "nonlocal"))
    terminal.append(check("R16 work/history boundary retained", "LOCAL_ROOT=REQUIRES_EXPLICIT_WORK_HISTORY_OR_ADDED_FACTOR_HARDWARE" in parent_output, "priced branch"))

    if not all(terminal):
        print("FTD-0980 OUTCOME D - inherited certificate invalid")
        return 1
    print("FTD-0980 OUTCOME B - inherited FTD-0979 closure plus repair integrity 16/16")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
