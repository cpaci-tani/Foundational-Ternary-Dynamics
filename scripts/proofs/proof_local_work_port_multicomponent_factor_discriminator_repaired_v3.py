#!/usr/bin/env python3
"""FTD-0982 final verifier wrapper for the immutable FTD-0981 proof."""

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
REPAIR_V2 = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v2.md"
)
REPAIR_V3 = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_WORK_PORT_FACTOR_CERTIFICATE_REPAIR_v3.md"
)
PARENT = ROOT / "scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator.py"
WRAPPER_V2 = ROOT / "scripts/proofs/proof_local_work_port_multicomponent_factor_discriminator_repaired.py"

EXPECTED_PARENT_PROTOCOL = "7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3"
EXPECTED_PARENT = "BDD16E3D4AB8BF0E0D4C72E5520638AB712D64E113725145B27F919B620F0C69"
EXPECTED_REPAIR_V2 = "4FD4AAE506BF96B890C020FEB3E798F12558AC271A8EEACE5D26722FDA8BCD9E"
EXPECTED_WRAPPER_V2 = "39F0287B56EB4FC62BF04CEB0A40FFFCD8B3B06455229068321812C7CA984B09"
EXPECTED_REPAIR_V3 = "6BE59B135CEA66F04A2F659E5A177AF8A4BD53AD0DDF592A2C8A173ACE946FB2"

REPAIRS = (
    (
        '        "local quarter-turn is still a legitimate symplectic event" in trilemma_text\n',
        '        "local quarter-turn is still a legitimate symplectic event" in " ".join(trilemma_text.split())\n',
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
    print("FTD-0982 local work-port/factor final verifier repair")
    print("=" * 79)
    integrity: list[bool] = []
    frozen = (
        ("parent protocol", PARENT_PROTOCOL, EXPECTED_PARENT_PROTOCOL),
        ("parent proof", PARENT, EXPECTED_PARENT),
        ("v2 repair protocol", REPAIR_V2, EXPECTED_REPAIR_V2),
        ("v2 wrapper", WRAPPER_V2, EXPECTED_WRAPPER_V2),
        ("v3 repair protocol", REPAIR_V3, EXPECTED_REPAIR_V3),
    )
    for label, path, expected in frozen:
        actual = sha256(path)
        integrity.append(check(f"I hash {label}", actual == expected, actual))

    v3_text = REPAIR_V3.read_text(encoding="utf-8")
    integrity.append(check("I three-substitution scope", "Exactly three in-memory substitutions" in v3_text, "verifier only"))
    integrity.append(check("I no gate waiver", "No other source substitution, assertion change, gate waiver" in v3_text, "all inherited"))

    parent_source = PARENT.read_text(encoding="utf-8")
    repaired_source = parent_source
    for index, (old, new) in enumerate(REPAIRS, start=1):
        integrity.append(check(f"I old anchor {index} once", repaired_source.count(old) == 1, repaired_source.count(old)))
        integrity.append(check(f"I new anchor {index} absent", repaired_source.count(new) == 0, repaired_source.count(new)))
        repaired_source = repaired_source.replace(old, new)
        integrity.append(check(f"I new anchor {index} once", repaired_source.count(new) == 1, repaired_source.count(new)))

    for label, path, expected in frozen[:-1]:
        integrity.append(check(f"I {label} remains frozen", sha256(path) == expected, sha256(path)))

    if not all(integrity):
        print("FTD-0982 OUTCOME D - wrapper integrity invalid")
        return 1

    namespace = {"__name__": "ftd0982_final_repaired_parent", "__file__": str(PARENT)}
    exec(compile(repaired_source, f"{PARENT}::FTD-0982-v3", "exec"), namespace)
    captured = io.StringIO()
    with contextlib.redirect_stdout(captured):
        parent_rc = namespace["main"]()
    parent_output = captured.getvalue()
    print(parent_output, end="")

    terminal = [
        check("T inherited return code", parent_rc == 0, parent_rc),
        check("T inherited 79/79", "79/79 checks passed" in parent_output, "complete"),
        check("T inherited Outcome B", "OUTCOME B" in parent_output, "minimum canonical work port"),
        check("T no inherited Outcome D", "OUTCOME D" not in parent_output, "no failure"),
        check("T factor boundary retained", "does not localize the inverse required by the one-event root" in parent_output, "factor insufficient"),
        check("T work-port closure retained", "minimum exact local canonical energy completion" in parent_output, "one complete pair"),
    ]
    if not all(terminal):
        print("FTD-0982 OUTCOME D - inherited certificate invalid")
        return 1

    print("FTD-0982 OUTCOME B - inherited FTD-0981 closure plus final repair integrity")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
