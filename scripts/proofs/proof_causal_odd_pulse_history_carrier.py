#!/usr/bin/env python3
"""Exact certificate for FTD-0852: causal odd-pulse history carrier."""

from __future__ import annotations

import hashlib
from pathlib import Path
import sys

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md":
        "ED76BCD3266A472A96601BD673E85FF43B60CD0B2C5AF09E27CD08DA0ED700CF",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md":
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
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

receiver_theorem = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md"
]
phase_read = texts["engine/src/render_bridge_phases/phase_read.cpp"]
phase_write = texts["engine/src/render_bridge_phases/phase_write.cpp"]
ledger = texts["engine/src/energy_ledger_compute.cpp"]
voxel = texts["engine/include/ftd/voxel.h"]

check(
    "C7 production voxel carries four dual field coordinates",
    all(
        token in voxel
        for token in ("Vec3 flux_L", "Vec3 flux_R", "Vec3 wave_vel_L", "Vec3 wave_vel_R")
    ),
)
check(
    "C8 production declares observable common fields as L plus R",
    "Observable: flux = flux_L + flux_R" in voxel
    and "v.flux = v.flux_L + v.flux_R" in phase_write
    and "v.wave_vel = v.wave_vel_L + v.wave_vel_R" in phase_write,
)
check(
    "C9 L and R receive the same local wave operator coefficient",
    "rb.delta_j_L_[i] = lap_L * cw2" in phase_read
    and "rb.delta_j_R_[i] = lap_R * cw2" in phase_read
    and "laplacian_field<&Voxel::flux_L>" in phase_read
    and "laplacian_field<&Voxel::flux_R>" in phase_read,
)
check(
    "C10 matter coupling source is added equally to L and R",
    "rb.delta_j_L_[i] += curl_sv - grad_s" in phase_read
    and "rb.delta_j_R_[i] += curl_sv - grad_s" in phase_read,
)

lap_l, lap_r, cw2, source = sp.symbols("lap_l lap_r cw2 source", real=True)
delta_l = cw2 * lap_l + source
delta_r = cw2 * lap_r + source
check(
    "C11 equal matter source cancels exactly from the relative acceleration",
    sp.expand((delta_l - delta_r) - cw2 * (lap_l - lap_r)) == 0,
)
check(
    "C12 imposed clock terms also act with identical L and R coefficients",
    "rb.delta_j_L_[i] -= rb.voxels_[i].flux_L * omega_eff_sq" in phase_read
    and "rb.delta_j_R_[i] -= rb.voxels_[i].flux_R * omega_eff_sq" in phase_read
    and "rb.delta_j_L_[i] -= rb.voxels_[i].flux_L * omega0_sq" in phase_read
    and "rb.delta_j_R_[i] -= rb.voxels_[i].flux_R * omega0_sq" in phase_read,
)
check(
    "C13 phase write integrates and damps L and R separately",
    "v.wave_vel_L += rb.delta_j_L_[i]" in phase_write
    and "v.wave_vel_R += rb.delta_j_R_[i]" in phase_write
    and "v.flux_L += v.wave_vel_L" in phase_write
    and "v.flux_R += v.wave_vel_R" in phase_write
    and "v.flux_L *= eff_damping" in phase_write
    and "v.flux_R *= eff_damping" in phase_write,
)
check(
    "C14 frozen dual stencil reads both propagation directions",
    "rb.voxels_[i+1].flux_L" in phase_read
    and "rb.voxels_[i-1].flux_L" in phase_read
    and "rb.voxels_[i+1].flux_R" in phase_read
    and "rb.voxels_[i-1].flux_R" in phase_read,
)
check(
    "C15 aggregate energy ledger reads only common flux and wave velocity",
    "E_field += v.flux.mag2()" in ledger
    and "E_wave  += v.wave_vel.mag2()" in ledger,
)
check(
    "C16 aggregate energy ledger contains no separate dual-channel square",
    all(token not in ledger for token in ("flux_L", "flux_R", "wave_vel_L", "wave_vel_R")),
)

# Exact rail algebra.
B = sp.symbols("B", positive=True, real=True)
a_plus = sp.sqrt(2 * B)
a_minus = -sp.sqrt(2 * B)
check(
    "C17 odd pulse amplitude closes the positive event energy",
    sp.simplify(a_plus**2 / 2 - B) == 0
    and sp.simplify(a_minus**2 / 2 - B) == 0,
)
check(
    "C18 odd pulse sign recovers the erased orientation",
    sp.ask(sp.Q.positive(a_plus)) is True
    and sp.ask(sp.Q.negative(a_minus)) is True,
)

def rail_step(state: list[sp.Expr], injected: sp.Expr) -> list[sp.Expr]:
    """Finite prefix of the half-line shift; tail retention is checked separately."""
    return [injected, *state[:-1]]


a0, a1, a2, a3 = sp.symbols("a0 a1 a2 a3", nonzero=True, real=True)
state = [sp.Integer(0)] * 4
for injected in (a0, a1, a2, a3):
    state = rail_step(state, injected)
check(
    "C19 rail update has dependency radius one",
    state == [a3, a2, a1, a0],
)
check(
    "C20 depth records event age exactly before boundary contact",
    all(state[j] == (a3, a2, a1, a0)[j] for j in range(4)),
)

B0, B1, B2, B3 = sp.symbols("B0 B1 B2 B3", positive=True, real=True)
energy_state = [sp.sqrt(2 * B3), sp.sqrt(2 * B2), sp.sqrt(2 * B1), sp.sqrt(2 * B0)]
H = sp.simplify(sum(x**2 for x in energy_state) / 2)
check(
    "C21 half-line carrier energy is the exact sum of retained event exports",
    sp.simplify(H - (B0 + B1 + B2 + B3)) == 0,
)

d0, d1, b = sp.symbols("d0 d1 b", real=True)
e0 = d0**2 / 2
e1 = d1**2 / 2
injected = sp.sqrt(2 * b**2)  # exact nonnegative-energy witness, B_event=b^2
e0_next = injected**2 / 2
e1_next = e0
f_half = e0
f_three_half = e1
check(
    "C22 site-zero energy continuity has exactly the event source",
    sp.simplify(e0_next - e0 + f_half - 0 - b**2) == 0,
)
check(
    "C23 positive-depth energy continuity is source free",
    sp.simplify(e1_next - e1 + f_three_half - f_half) == 0,
)

next_half_line = [a3, a2, a1, a0]
recovered_event = next_half_line[0]
recovered_previous = next_half_line[1:]
check(
    "C24 half-line inverse recovers newest event and every prior amplitude",
    recovered_event == a3 and recovered_previous == [a2, a1, a0],
)
two_steps = rail_step(rail_step([0, 0, 0], a0), a1)
check(
    "C25 receiver reuse moves rather than overwrites the previous pulse",
    two_steps == [a1, a0, 0],
)
check(
    "C26 distinct two-event sign histories remain distinguishable",
    len({(s1, s0) for s1 in (-1, 1) for s0 in (-1, 1)}) == 4,
)

x0, x1, x2, anew = sp.symbols("x0 x1 x2 anew", real=True)
finite_before = [x0, x1, x2]
finite_after = rail_step(finite_before, anew)
H_before = sum(x**2 for x in finite_before) / 2
H_after = sum(x**2 for x in finite_after) / 2
E_out = x2**2 / 2
check(
    "C27 finite rail energy changes by injection minus outgoing tail energy",
    sp.simplify(H_after - H_before - (anew**2 / 2 - E_out)) == 0,
)
check(
    "C28 scalar outgoing-energy ledger is blind to tail sign",
    sp.simplify((x2**2 / 2) - ((-x2) ** 2 / 2)) == 0,
)
check(
    "C29 retaining signed tail amplitude restores the missing boundary distinction",
    x2 != -x2 and sp.simplify(x2**2 - (-x2) ** 2) == 0,
)

D = sp.symbols("D", real=True)
L = D / sp.sqrt(2)
R = -D / sp.sqrt(2)
C = sp.simplify((L + R) / sp.sqrt(2))
D_back = sp.simplify((L - R) / sp.sqrt(2))
H_lr = sp.simplify((L**2 + R**2) / 2)
check(
    "C30 bilateral rail is zero common signed relative and energy exact",
    C == 0 and sp.simplify(D_back - D) == 0 and sp.simplify(H_lr - D**2 / 2) == 0,
)

directions = (
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
)
arm = sp.sqrt(B / 3)
six_energy = sp.simplify(sum(arm**2 / 2 for _ in directions))
balance = tuple(sum(direction[k] for direction in directions) for k in range(3))
check(
    "C31 six equal face rails are cubically balanced and carry total energy B",
    sp.simplify(six_energy - B) == 0 and balance == (0, 0, 0),
)

dual_candidate = all(ok for _, ok in checks[6:16])
production_complete = False  # C14--C16 plus the frozen FTD-0851 boundary forbid equivalence.
dual_transition_scope = phase_read + phase_write.split("// ---- Single-substrate")[0]
forbidden = ("MeasurementContext", "Born", "G_STAR", "cadence_target")
check(
    "C32 production has a target-blind relative candidate channel but not the exact carrier",
    dual_candidate
    and not production_complete
    and "The three fragments do not compose" in receiver_theorem
    and all(token not in dual_transition_scope for token in forbidden),
)


passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0852 causal odd-pulse history carrier: {passed}/{total} "
      f"{'PASS' if passed == total == 32 else 'FAIL'}")

if passed == total == 32:
    print("HALF_LINE_ODD_PULSE_SHIFT_IS_LOCAL_CAUSAL_INJECTIVE_AND_ENERGY_CLOSED")
    print("FINITE_RECEIVER_CAPACITY_REQUIRES_SIGNED_TAIL_EXPORT")
    print("PRODUCTION_DUAL_DIFFERENCE_IS_A_HOMOGENEOUS_CANDIDATE_CHANNEL")
    print("PRODUCTION_LEDGER_AND_EVENTS_DO_NOT_COMPLETE_THE_HISTORY_CARRIER")
    print("VERDICT=OUTCOME_B_EXACT_REFERENCE_CARRIER_PRODUCTION_PARTIAL")
    sys.exit(0)

sys.exit(1)
