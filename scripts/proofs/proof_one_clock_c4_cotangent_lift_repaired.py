#!/usr/bin/env python3
"""FTD-0977 verifier-only wrapper for the immutable FTD-0976 proof."""

from __future__ import annotations

import contextlib
import hashlib
import io
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PARENT_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md"
)
REPAIR_PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ONE_CLOCK_C4_COTANGENT_LIFT_CERTIFICATE_REPAIR_v2.md"
)
PARENT = ROOT / "scripts/proofs/proof_one_clock_c4_cotangent_lift.py"

EXPECTED_PARENT_PROTOCOL = "FD80A0524A8BB437210FC213B0DB071F8FCBB11E03D67594A23BCF4443B084F2"
EXPECTED_REPAIR_PROTOCOL = "E08115800DEACDC8D9059D815BF87D408AED927BF33B3E74E368BE8DCDCC296F"
EXPECTED_PARENT = "436E54D2CF9A117CA17F53D054D7C51F670A156A81EA0F4F658F63C85BC6065A"


OLD_PROTOCOL_READ = '    protocol_text = PROTOCOL.read_text(encoding="utf-8")\n'
NEW_PROTOCOL_READ = (
    '    protocol_text = PROTOCOL.read_text(encoding="utf-8")\n'
    '    protocol_text = " ".join(protocol_text.split())\n'
)

OLD_MOMENTUM_CHECK = '''    cert.check(
        "G2 unique canonical momentum shift",
        sp.solve(
            sp.Eq(
                sp.Symbol("Pi_trial"),
                sp.Symbol("K_trial") - r_g * conn_g * generator_g + q * r_i * conn_i * action_i,
            ),
            sp.Symbol("K_trial"),
        )
        == [sp.Symbol("Pi_trial") + r_g * conn_g * generator_g - q * r_i * conn_i * action_i],
        k_mech,
    )
'''

NEW_MOMENTUM_CHECK = '''    pi_trial = sp.Symbol("Pi_trial")
    k_trial = sp.Symbol("K_trial")
    momentum_solutions = sp.solve(
        sp.Eq(
            pi_trial,
            k_trial - r_g * conn_g * generator_g + q * r_i * conn_i * action_i,
        ),
        k_trial,
    )
    expected_momentum = pi_trial + r_g * conn_g * generator_g - q * r_i * conn_i * action_i
    cert.check(
        "G2 unique canonical momentum shift",
        len(momentum_solutions) == 1
        and sp.simplify(momentum_solutions[0] - expected_momentum) == 0,
        momentum_solutions,
    )
'''


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def report(label: str, passed: bool, detail: object = "") -> bool:
    print(f"  {'PASS' if passed else 'FAIL'}  {label}: {detail}")
    return bool(passed)


def main() -> int:
    print("=" * 79)
    print("FTD-0977 one-clock C4 cotangent-lift verifier-only repair")
    print("=" * 79)
    integrity: list[bool] = []
    integrity.append(report("R1 parent protocol hash", sha256(PARENT_PROTOCOL) == EXPECTED_PARENT_PROTOCOL, sha256(PARENT_PROTOCOL)))
    integrity.append(report("R2 repair protocol hash", sha256(REPAIR_PROTOCOL) == EXPECTED_REPAIR_PROTOCOL, sha256(REPAIR_PROTOCOL)))
    integrity.append(report("R3 parent proof hash", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    repair_text = REPAIR_PROTOCOL.read_text(encoding="utf-8")
    integrity.append(report("R4 repair scope marker", "representation-only substitutions" in repair_text, "verifier only"))
    integrity.append(report("R5 no gate waiver marker", "No failed physical or\nmathematical gate may be waived" in repair_text, "all 52 inherited"))

    parent_source = PARENT.read_text(encoding="utf-8")
    integrity.append(report("R6 unique protocol-read anchor", parent_source.count(OLD_PROTOCOL_READ) == 1, parent_source.count(OLD_PROTOCOL_READ)))
    integrity.append(report("R7 unique momentum-check anchor", parent_source.count(OLD_MOMENTUM_CHECK) == 1, parent_source.count(OLD_MOMENTUM_CHECK)))

    repaired_source = parent_source.replace(OLD_PROTOCOL_READ, NEW_PROTOCOL_READ)
    repaired_source = repaired_source.replace(OLD_MOMENTUM_CHECK, NEW_MOMENTUM_CHECK)
    integrity.append(report("R8 normalized-read inserted once", repaired_source.count(NEW_PROTOCOL_READ) == 1, repaired_source.count(NEW_PROTOCOL_READ)))
    integrity.append(report("R9 residual-check inserted once", repaired_source.count(NEW_MOMENTUM_CHECK) == 1, repaired_source.count(NEW_MOMENTUM_CHECK)))
    integrity.append(report("R10 parent remains byte-frozen", sha256(PARENT) == EXPECTED_PARENT, sha256(PARENT)))

    if not all(integrity):
        print("FTD-0977 OUTCOME D - wrapper integrity invalid")
        return 1

    namespace = {"__name__": "ftd0977_repaired_parent", "__file__": str(PARENT)}
    exec(compile(repaired_source, f"{PARENT}::FTD-0977", "exec"), namespace)
    captured = io.StringIO()
    with contextlib.redirect_stdout(captured):
        parent_rc = namespace["main"]()
    parent_output = captured.getvalue()
    print(parent_output, end="")

    terminal: list[bool] = []
    terminal.append(report("R11 inherited return code", parent_rc == 0, parent_rc))
    terminal.append(report("R12 inherited 52-gate closure", "checks=52 passed=52 failed=0" in parent_output, "52/52"))
    terminal.append(report("R13 inherited Outcome B", "FTD-0976 OUTCOME B" in parent_output, "conditional cotangent theorem"))
    terminal.append(report("R14 no inherited Outcome D", "FTD-0976 OUTCOME D" not in parent_output, "no failure"))
    terminal.append(report("R15 representation debt retained", "COMMON_DIAGONAL_G_MINUS_QI_CONNECTION=SELECTION" in parent_output, "selection"))
    terminal.append(report("R16 passive boundary retained", "REGULAR_LOCAL_CONNECTION=PASSIVE_PURE_GAUGE" in parent_output, "pure gauge"))

    if not all(terminal):
        print("FTD-0977 OUTCOME D - inherited certificate invalid")
        return 1
    print("FTD-0977 OUTCOME B - inherited 52/52 plus repair integrity 16/16")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
