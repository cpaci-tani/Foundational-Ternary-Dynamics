#!/usr/bin/env python3
"""FTD-0982 three-substitution wrapper for the immutable FTD-0981 proof."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PARENT_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_WORK_PORT_VERSUS_MULTICOMPONENT_FACTOR_DISCRIMINATOR_v1.md"
)
REPAIR_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v2.md"
)
PARENT = ROOT / "scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator.py"

EXPECTED_PARENT_PROTOCOL = "7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3"
EXPECTED_REPAIR_PROTOCOL = "4FD4AAE506BF96B890C020FEB3E798F12558AC271A8EEACE5D26722FDA8BCD9E"
EXPECTED_PARENT = "BDD16E3D4AB8BF0E0D4C72E5520638AB712D64E113725145B27F919B620F0C69"

REPAIRS = (
    (
        '        "local quarter-turn is still a legitimate symplectic event" in trilemma_text\n',
        '        "A local quarter-turn is still a legitimate symplectic event" in trilemma_text\n',
    ),
    (
        "        sp.limit(positive_q_defect, amplitude, sp.oo) == sp.oo,\n",
        "        sp.limit(positive_q_defect.subs(k_symbol, kappa**2 / 2), amplitude, sp.oo) == sp.oo,\n",
    ),
    (
        "        sp.limit(positive_p_defect, amplitude, sp.oo) == sp.oo,\n",
        "        sp.limit(positive_p_defect.subs(k_symbol, 2 * kappa**2), amplitude, sp.oo) == sp.oo,\n",
    ),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool, detail: object = "") -> bool:
    print(f"  {'PASS' if condition else 'FAIL'}  {label}: {detail}")
    return bool(condition)


def main() -> int:
    print("=" * 79)
    print("FTD-0982 local work-port/factor verifier-only repair")
    print("=" * 79)
    integrity: list[bool] = []
    integrity.append(check("R1 parent protocol hash", sha256(PARENT_PROTOCOL) == EXPECTED_PARENT_PROTOCOL, sha256(PARENT_PROTOCOL)))
    integrity.append(check("R2 repair protocol hash", sha256(REPAIR_PROTOCOL) == EXPECTED_REPAIR_PROTOCOL, sha256(REPAIR_PROTOCOL)))
    integrity.append(check("R3 parent proof hash", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    repair_text = REPAIR_PROTOCOL.read_text(encoding="utf-8")
    integrity.append(check("R4 three-substitution scope", "Exactly three in-memory source substitutions" in repair_text, "verifier only"))
    integrity.append(check("R5 no gate waiver", "No other substitution, assertion change, gate waiver" in repair_text, "all inherited"))

    parent_source = PARENT.read_text(encoding="utf-8")
    repaired_source = parent_source
    for index, (old, new) in enumerate(REPAIRS, start=1):
        integrity.append(check(f"R{5 + 3 * index - 2} old anchor {index} occurs once", repaired_source.count(old) == 1, repaired_source.count(old)))
        integrity.append(check(f"R{5 + 3 * index - 1} new anchor {index} absent", repaired_source.count(new) == 0, repaired_source.count(new)))
        repaired_source = repaired_source.replace(old, new)
        integrity.append(check(f"R{5 + 3 * index} new anchor {index} inserted once", repaired_source.count(new) == 1, repaired_source.count(new)))
    integrity.append(check("R15 parent remains byte frozen", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    if not all(integrity):
        print("FTD-0982 OUTCOME D - wrapper integrity invalid")
        return 1

    namespace = {"__name__": "ftd0982_repaired_parent", "__file__": str(PARENT)}
    exec(compile(repaired_source, f"{PARENT}::FTD-0982", "exec"), namespace)
    captured = io.StringIO()
    with contextlib.redirect_stdout(captured):
        parent_rc = namespace["main"]()
    parent_output = captured.getvalue()
    print(parent_output, end="")

    terminal: list[bool] = []
    terminal.append(check("R16 inherited return code", parent_rc == 0, parent_rc))
    terminal.append(check("R17 inherited 79/79", "79/79 checks passed" in parent_output, "complete"))
    terminal.append(check("R18 inherited Outcome B", "OUTCOME B" in parent_output, "minimum canonical work port"))
    terminal.append(check("R19 no inherited Outcome D", "OUTCOME D" not in parent_output, "no failure"))
    terminal.append(check("R20 factor boundary retained", "does not localize the inverse required by the one-event root" in parent_output, "factor insufficient"))
    terminal.append(check("R21 work-port closure retained", "minimum exact local canonical energy completion" in parent_output, "one complete pair"))

    if not all(terminal):
        print("FTD-0982 OUTCOME D - inherited certificate invalid")
        return 1
    print("FTD-0982 OUTCOME B - inherited FTD-0981 closure plus repair integrity 21/21")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
