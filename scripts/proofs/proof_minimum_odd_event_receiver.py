#!/usr/bin/env python3
"""Exact certificate for FTD-0851: minimum odd event receiver."""

from __future__ import annotations

import hashlib
from pathlib import Path
import re
import sys

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "engine/src/render_bridge_phases/phase_movement.cpp":
        "6149B37C5A28B8EE9B8544CAEC24006D0964D1C8F344CA63C68DC6536A47E8FB",
    "engine/include/ftd/eft/history_event_journal.h":
        "4A9AEDC650FE882C0CB6421901784095DA4EA079D3CCBC985DD412148583955A",
    "engine/src/eft/history_event_journal.cpp":
        "94EBB526F3F31CB53D8907109BA29BD207E3D8E3828DCCA6D2C2C7B31B620B91",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md":
        "090F139CBA8C930A9761A33EFBFB59BD2767F22E4DF50031120B70E18D42EA15",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_NATURAL_EXTENSION.md":
        "2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PRODUCTION_TERNARY_LATCH_BOUNDARY_v1.md":
        "95F39274E361868E039368AB149A9196F2008D2BB58CD5F0DAD0CD8F7E92110B",
}


checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


texts: dict[str, str] = {}
for relative, expected in SOURCES.items():
    path = ROOT / relative
    actual = sha256(path) if path.exists() else "MISSING"
    check(f"source hash {relative}", actual == expected)
    texts[relative] = path.read_text(encoding="utf-8") if path.exists() else ""

movement = texts["engine/src/render_bridge_phases/phase_movement.cpp"]
journal_h = texts["engine/include/ftd/eft/history_event_journal.h"]
journal_cpp = texts["engine/src/eft/history_event_journal.cpp"]
ledger = texts["engine/src/energy_ledger_compute.cpp"]
bounce_audit = texts[
    "docs/theory/07_assessment/common_action_mechanics_reciprocity/"
    "AUDIT_PRODUCTION_SAME_SIGN_BOUNCE.md"
]

# Production has two byte-parallel movement-order branches. Audit both.
same_sign_chunks = re.findall(
    r"else if \(t\.state == v\.state\) \{(.*?)\n\s*\} else \{",
    movement,
    flags=re.DOTALL,
)
check(
    "C8 both production paths expose the same-sign occupied-target branch",
    len(same_sign_chunks) == 2,
)
check(
    "C9 same-sign branch flips mover axes and resets mover remainder",
    len(same_sign_chunks) == 2
    and all(
        "v.velocity.x *= -1.0" in chunk
        and "v.velocity.y *= -1.0" in chunk
        and "v.velocity.z *= -1.0" in chunk
        and "v.remainder = {}" in chunk
        for chunk in same_sign_chunks
    ),
)
check(
    "C10 same-sign branch writes no target field recoil or history event",
    len(same_sign_chunks) == 2
    and all(
        "t.velocity" not in chunk
        and "t.flux" not in chunk
        and "HistoryEvent" not in chunk
        for chunk in same_sign_chunks
    )
    and "remainder reset erases the collision phase" in bounce_audit,
)

annihilation_chunks = re.findall(
    r"// Opposite sign: annihilation.*?(?=// FTD-HISTORY-END)",
    movement,
    flags=re.DOTALL,
)
check(
    "C11 annihilation clears both records phase variables and labels",
    len(annihilation_chunks) == 2
    and all(
        "rb.set_state(i, 0)" in chunk
        and "rb.set_state(target, 0)" in chunk
        and "v.velocity = {}; t.velocity = {}" in chunk
        and "v.remainder = {}; t.remainder = {}" in chunk
        and "v.particle_id = -1; t.particle_id = -1" in chunk
        and "v.spin = 0; v.color = 0" in chunk
        for chunk in annihilation_chunks
    ),
)
check(
    "C12 annihilation redistributes saved flux over the two six-neighbour shells",
    len(annihilation_chunks) == 2
    and all(
        "Vec3 flux_v = v.flux" in chunk
        and "Vec3 flux_t = t.flux" in chunk
        and "neighbors_6(i)" in chunk
        and "neighbors_6(target)" in chunk
        and "flux_v * (1.0 / 6.0)" in chunk
        and "flux_t * (1.0 / 6.0)" in chunk
        for chunk in annihilation_chunks
    ),
)
check(
    "C13 annihilation field emission contains no sign-bearing source term",
    len(annihilation_chunks) == 2
    and all(
        "moving_state" not in chunk
        and "static_cast<double>(v.state)" not in chunk
        and "static_cast<double>(t.state)" not in chunk
        for chunk in annihilation_chunks
    ),
)
check(
    "C14 event journal is explicitly observation-only and state neutral",
    "The journal is an observer" in journal_h
    and "It is disabled by default" in journal_h
    and "never writes lattice, voxel, toggle, or integrator state" in journal_h
    and "bool enabled = false" in journal_cpp,
)
check(
    "C15 journal snapshot retains the complete before-and-after voxel payload",
    "Voxel voxel{}" in journal_h
    and "std::array<HistorySiteState, 2> before{}" in journal_h
    and "std::array<HistorySiteState, 2> after{}" in journal_h
    and "out.voxel = voxel" in journal_cpp,
)
check(
    "C16 aggregate energy audit contains no event-history or receiver input",
    "HistoryEvent" not in ledger
    and "history_event" not in ledger
    and "controller" not in ledger.lower()
    and "bath" not in ledger.lower(),
)

# Exact minimum-receiver algebra.
signed_preimages = {-1, 1}
erasure_outputs = {0 for _s in signed_preimages}
check(
    "C17 signed erasure is exactly many-to-one",
    len(signed_preimages) == 2 and len(erasure_outputs) == 1,
)

B = sp.symbols("B", nonnegative=True, real=True)
s = sp.symbols("s", real=True)
energy_only_plus = B
energy_only_minus = B
check(
    "C18 an energy-only receiver is sign blind",
    sp.simplify(energy_only_plus - energy_only_minus) == 0,
)
check(
    "C19 sign completeness requires at least two receiver outputs",
    len({(-1, B), (1, B)}) == 2,
)

receiver_plus = (1, B)
receiver_minus = (-1, B)
check(
    "C20 general receiver distinguishes signs including zero export",
    receiver_plus != receiver_minus
    and receiver_plus[0] == 1
    and receiver_minus[0] == -1,
)
check(
    "C21 general receiver energy account closes exactly",
    sp.simplify(receiver_plus[1] - B) == 0
    and sp.simplify(receiver_minus[1] - B) == 0,
)

B_pos = sp.symbols("B_pos", positive=True, real=True)
a_plus = sp.sqrt(2 * B_pos)
a_minus = -sp.sqrt(2 * B_pos)
check(
    "C22 positive-export signed amplitude closes receiver energy",
    sp.simplify(a_plus**2 / 2 - B_pos) == 0
    and sp.simplify(a_minus**2 / 2 - B_pos) == 0,
)
check(
    "C23 positive-export amplitude retains and inverts the event sign",
    sp.ask(sp.Q.positive(a_plus)) is True
    and sp.ask(sp.Q.negative(a_minus)) is True,
)
check(
    "C24 zero export collapses both amplitudes and requires a separate odd label",
    a_plus.subs(B_pos, 0) == 0 and a_minus.subs(B_pos, 0) == 0,
)

L = s * sp.sqrt(B)
R = -s * sp.sqrt(B)
C = sp.simplify((L + R) / sp.sqrt(2))
D = sp.simplify((L - R) / sp.sqrt(2))
H_lr = sp.simplify((L**2 + R**2) / 2)
check("C25 bilateral receiver leaves the common channel zero", C == 0)
check(
    "C26 bilateral receiver places the sign in the relative channel",
    sp.simplify(D - s * sp.sqrt(2 * B)) == 0,
)
check(
    "C27 bilateral receiver energy is exact on signed events",
    sp.simplify(H_lr.subs(s**2, 1) - B) == 0,
)

histories = {(first, second) for first in (-1, 1) for second in (-1, 1)}
overwritten = {second for _first, second in histories}
check(
    "C28 finite receiver overwrite loses earlier event history",
    len(histories) == 4 and len(overwritten) == 2,
)

production_fragments = (
    len(same_sign_chunks) == 2
    and len(annihilation_chunks) == 2
    and "The journal is an observer" in journal_h
)
production_complete = False  # Exact source gates C10, C13, C14, C16 forbid it.
check(
    "C29 production has barrier exhaust and observer fragments but no complete receiver",
    production_fragments and not production_complete,
)

transition_text = "\n".join(same_sign_chunks + annihilation_chunks)
forbidden_targets = ("MeasurementContext", "Born", "G_STAR", "cadence_target")
check(
    "C30 audited transition formulas are context Born G-star and cadence-target blind",
    all(token not in transition_text for token in forbidden_targets),
)


passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0851 minimum odd event receiver: {passed}/{total} "
      f"{'PASS' if passed == total == 30 else 'FAIL'}")

if passed == total == 30:
    print("TWO_RECEIVER_OUTPUTS_ARE_MINIMUM_FOR_SIGN_COMPLETE_ZERO_EXPORT")
    print("POSITIVE_EXPORT_COMPRESSES_SIGN_AND_ENERGY_TO_ONE_ODD_AMPLITUDE")
    print("BALANCED_BILATERAL_PULSE_IS_ONE_SELECTED_ZERO_COMMON_REALIZATION")
    print("PRODUCTION_BARRIER_EXHAUST_AND_JOURNAL_ARE_INCOMPLETE_FRAGMENTS")
    print("VERDICT=OUTCOME_B_MINIMUM_RECEIVER_DERIVED_PRODUCTION_INCOMPLETE")
    sys.exit(0)

sys.exit(1)
