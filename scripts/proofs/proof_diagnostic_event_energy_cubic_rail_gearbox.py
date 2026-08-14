#!/usr/bin/env python3
"""FTD-0854 exact diagnostic-event-energy/cubic-rail certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CUBIC_ODD_EVENT_DEPOSIT_v1.md":
        "08FBF3361C453DC9E0A99184920883DBC6DE15B5043F7EFC140B0EB740A26474",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "engine/include/ftd/causal_kinematics.h":
        "705501451985333D64128A0896216A137A2D836673AEB02E9ACE6DE4F2E53AA2",
    "engine/include/ftd/ontic/particle_masses.h":
        "EFE9D68C9ECF6520510519B972D5CDD5925FD86026270AB0E4CAA5BFD6F1B0B1",
    "engine/src/diagnostics_compute.cpp":
        "C3703292F8474EBC119F70024B0F3E4A23921C26EA58F8F6AB5E7581FB654AA6",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for relative, expected in SOURCES.items():
    path = ROOT / relative
    check(f"source hash {relative}", path.is_file() and sha256(path) == expected)

causal = (ROOT / "engine/include/ftd/causal_kinematics.h").read_text(
    encoding="utf-8"
)
masses = (ROOT / "engine/include/ftd/ontic/particle_masses.h").read_text(
    encoding="utf-8"
)
diagnostics = (ROOT / "engine/src/diagnostics_compute.cpp").read_text(
    encoding="utf-8"
)
phase_write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(
    encoding="utf-8"
)
ledger = (ROOT / "engine/src/energy_ledger_compute.cpp").read_text(
    encoding="utf-8"
)
protocol = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md"
).read_text(encoding="utf-8")

check(
    "C8 causal contract defines gamma total energy and kinetic energy",
    "double flat_gamma" in causal
    and "gamma * E_REST" in causal
    and "(gamma - 1.0) * E_REST" in causal,
)
check(
    "C9 rest energy is the positive adopted inertial-speed role",
    "E_REST          = M_INERTIAL * C_SPEED * C_SPEED" in masses
    and "M_INERTIAL      = K_B" in masses
    and "inline constexpr double K_B = 0.511" in masses,
)
check(
    "C10 diagnostics count matter rest and kinetic energy only for nonzero state",
    "if (s != 0)" in diagnostics
    and "a.particle_ke += kinetic" in diagnostics
    and "a.particle_rest_energy += E_REST" in diagnostics,
)
check(
    "C11 diagnostic particle total is rest plus kinetic",
    "a.particle_energy = a.particle_rest_energy + a.particle_ke" in diagnostics
    and "a.total_energy = a.field_energy + a.wave_energy + a.particle_energy"
    in diagnostics,
)

evaporation = phase_write.split(
    "// Evaporation (shared single + dual):", maxsplit=1
)[1]
check(
    "C12 evaporation clears the record and three labels",
    "rb.set_state(i, 0);" in evaporation
    and "v.particle_id = -1;" in evaporation
    and "v.spin = 0;" in evaporation
    and "v.color = 0;" in evaporation,
)
check(
    "C13 evaporation makes no compensating continuous-field assignment",
    "v.flux *=" not in evaporation
    and "v.wave_vel *=" not in evaporation
    and "v.velocity =" not in evaporation,
)
check(
    "C14 aggregate drift ledger omits rest and separate dual squares",
    "E_REST" not in ledger
    and "flux_L" not in ledger
    and "flux_R" not in ledger
    and "wave_vel_L" not in ledger
    and "wave_vel_R" not in ledger,
)

E, gamma, s = sp.symbols("E gamma s", positive=True)
kinetic = (gamma - 1) * E
B = sp.simplify(E + kinetic)
check("C15 diagnostic event energy is exactly gamma times rest energy", B == gamma * E)
check("C16 diagnostic event energy is strictly positive", B.is_positive)
check(
    "C17 diagnostic event energy is independent of erased sign",
    s not in B.free_symbols,
)
check(
    "C18 manifested-to-void diagnostic matter decrement is exactly B",
    sp.simplify((E + kinetic) - 0 - B) == 0,
)

a = sp.symbols("a", real=True)
Q = sp.simplify(6 * (2 * a))
D = sp.simplify(Q / sp.sqrt(12))
K = sp.simplify(sp.Rational(1, 2) * 6 * (a**2 + a**2))
check("C19 six-face radial coordinate is Q equals twelve a", Q == 12 * a)
check("C20 normalized rail amplitude is D equals square-root twelve a", D == sp.sqrt(12) * a)
check(
    "C21 cubic radial energy equals D squared over two equals Q squared over 24",
    sp.simplify(K - D**2 / 2) == 0 and sp.simplify(K - Q**2 / 24) == 0,
)

Bp = sp.symbols("B", positive=True)
sigma = sp.symbols("sigma", nonzero=True, real=True)
p = sp.sqrt(Bp / 6)
event_a = sigma * p
event_D = sp.simplify(sp.sqrt(12) * event_a)
event_Q = sp.simplify(12 * event_a)
check(
    "C22 cubic deposit is exactly the odd rail amplitude",
    sp.simplify(event_D - sigma * sp.sqrt(2 * Bp)) == 0,
)
check(
    "C23 cubic post-event coordinate is signed square-root 24B",
    sp.simplify(event_Q - sigma * sp.sqrt(24 * Bp)) == 0,
)

d0, d1, d2 = sp.symbols("d0 d1 d2", real=True)
new = sigma * sp.sqrt(2 * Bp)
before = [d0, d1, d2]
after = [new, d0, d1, d2]
check(
    "C24 combined rail write has dependency radius one",
    after[1:] == before,
)
check(
    "C25 half-line inverse recovers the event and every prior rail amplitude",
    sp.simplify(after[0] ** 2 / 2 - Bp) == 0 and after[1:] == before,
)
before_energy = sum(value**2 / 2 for value in before)
after_energy = sum(value**2 / 2 for value in after)
check(
    "C26 combined shift-write increases receiver energy by exactly B",
    sp.simplify(after_energy - before_energy - Bp) == 0,
)
check(
    "C27 outward shift vacates the old port without overwriting it",
    after[1] == d0 and after[0] == new,
)
check(
    "C28 rail depth increases every retained event age by one",
    all(after[j + 1] == before[j] for j in range(len(before))),
)

faces = [
    sp.Matrix((1, 0, 0)), sp.Matrix((-1, 0, 0)),
    sp.Matrix((0, 1, 0)), sp.Matrix((0, -1, 0)),
    sp.Matrix((0, 0, 1)), sp.Matrix((0, 0, -1)),
]
check(
    "C29 six axial rail impulses are cubically balanced",
    sum(faces, sp.zeros(3, 1)) == sp.zeros(3, 1),
)
left = sigma * p
right = -sigma * p
check(
    "C30 sign reversal is covariant with L-R exchange",
    sp.simplify(left.subs(sigma, -sigma) - right) == 0
    and sp.simplify(right.subs(sigma, -sigma) - left) == 0,
)
check(
    "C31 registered construction is context outcome-target Born G-star and cadence blind",
    all(
        firewall in protocol
        for firewall in (
            "No future context or outcome target is",
            "Born, Bell, `G*`, cadence",
            "target blind",
        )
    ),
)
check(
    "C32 production ledger stencil reserved-channel barrier and full-state lift remain open",
    "E_REST" not in ledger
    and "flux_L" not in ledger
    and "bidirectional dual wave stencil" in protocol
    and "full-state reversal" in protocol
    and "reciprocal protected-record barrier" in protocol,
)

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0854 diagnostic event energy and cubic rail gearbox: {passed}/{total} PASS")
if passed == total == 32:
    print("DIAGNOSTIC_MATTER_DECREMENT_SUPPLIES_POSITIVE_CONTEXT_BLIND_EVENT_B")
    print("CUBIC_RADIAL_COORDINATE_IS_EXACTLY_THE_NORMALIZED_ODD_HISTORY_RAIL")
    print("OUTWARD_SHIFT_FORMS_THE_NEXT_READY_PORT_AND_CLOSES_RECEIVER_ENERGY")
    print("PRODUCTION_DUAL_LEDGER_RESERVED_RAIL_BARRIER_AND_FULL_STATE_LIFT_REMAIN_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_REFERENCE_GEARBOX_PRODUCTION_INCOMPLETE")
    raise SystemExit(0)

print("VERDICT=OUTCOME_C_NO_THEOREM")
raise SystemExit(1)
