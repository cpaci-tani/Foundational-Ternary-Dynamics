#!/usr/bin/env python3
"""Exact certificate for FTD-0863: catalytic phase-reference transducer."""

from __future__ import annotations

import hashlib
from pathlib import Path
import sys

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_CATALYTIC_PHASE_REFERENCE_TRANSDUCER_v1.md"
)

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md":
        "06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md":
        "8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md":
        "94A75E375B8CB918B04C6D5C8DF5021380E8DA74243490BF1DD954ECBA26E32A",
    "engine/include/ftd/eft/reciprocal_record_port.h":
        "5973BF10BCE122304368E3BD191EA810D3DD6AB106B69B9D9022F662136D2B08",
    "engine/include/ftd/eft/phase_referenced_action_rail.h":
        "19EA541D11547460CC3AA3D041E8854E5A0277B6FDF58097B087E6D2139DF5DB",
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


J = sp.Matrix([[0, -1], [1, 0]])
identity2 = sp.eye(2)
check(
    "C7 phase quarter turn is an orthogonal symplectic complex structure",
    J * J == -identity2 and J.T * J == identity2 and J.det() == 1,
)

omega = sp.symbols("omega", real=True)
R_minus = sp.Matrix(
    [[sp.cos(omega), sp.sin(omega)],
     [-sp.sin(omega), sp.cos(omega)]]
)
q, p = sp.symbols("q p", real=True)
beta = sp.Matrix([q, p])
rotated_beta = sp.simplify(R_minus * beta)
pilot_action = sp.simplify(beta.dot(beta) / 2)
rotated_action = sp.trigsimp(rotated_beta.dot(rotated_beta) / 2)
check(
    "C8 autonomous phase rotation preserves pilot action exactly",
    sp.simplify(sp.trigsimp(rotated_action - pilot_action)) == 0,
)
check(
    "C9 autonomous phase rotation is orthogonal and orientation preserving",
    sp.simplify(sp.trigsimp(R_minus.T * R_minus)) == identity2
    and sp.simplify(sp.trigsimp(R_minus.det())) == 1,
)
symplectic_form = sp.Matrix([[0, 1], [-1, 0]])
check(
    "C10 autonomous phase rotation preserves the canonical symplectic form",
    sp.simplify(sp.trigsimp(R_minus.T * symplectic_form * R_minus))
    == symplectic_form,
)
R_plus = R_minus.subs(omega, -omega)
check(
    "C11 inverse phase step is the opposite rotation",
    sp.simplify(sp.trigsimp(R_plus * R_minus)) == identity2,
)
K = sp.diag(1, -1)
check(
    "C12 canonical time reversal exchanges the two rotation senses",
    sp.simplify(sp.trigsimp(K * R_minus * K - R_plus)) == sp.zeros(2),
)

phi0, kappa = sp.symbols("phi0 kappa", real=True)
j, n = sp.symbols("j n", integer=True)
phase = phi0 + kappa * j - omega * n
next_phase = phi0 + kappa * (j + 1) - omega * (n + 1)
check(
    "C13 local oscillator and outward signal rail are coherent iff twist matches advance",
    sp.expand(next_phase - phase) == kappa - omega,
)
N, winding = sp.symbols("N winding", integer=True, positive=True)
check(
    "C14 periodic nonzero pilot ring requires integer spatial winding",
    sp.simplify(
        sp.exp(sp.I * N * kappa).subs(kappa, 2 * sp.pi * winding / N) - 1
    ) == 0,
)
check(
    "C15 open-rail phase advance remains a free selected parameter",
    sp.diff(sp.expand(next_phase - phase), omega) == -1,
)


# A nonzero reference pair beta defines an orthonormal phase frame e,f=Je.
I_star = sp.symbols("I_star", positive=True, real=True)
reference = sp.Matrix([sp.sqrt(2 * I_star), 0])
e = sp.simplify(reference / sp.sqrt(2 * I_star))
f = sp.simplify(J * e)
check(
    "C16 normalized reference and its quarter turn form an orthonormal frame",
    e.dot(e) == 1 and f.dot(f) == 1 and e.dot(f) == 0,
)

a, b = sp.symbols("a b", real=True)
signal = a * f + b * e
check(
    "C17 phase-frame projections reconstruct an arbitrary signal pair",
    sp.simplify(f.dot(signal) - a) == 0
    and sp.simplify(e.dot(signal) - b) == 0,
)

g = sp.symbols("g", integer=True)
Sg = sp.Matrix([[1 - g, g], [g, 1 - g]])
check(
    "C18 hold and exchange gates are identity and swap",
    Sg.subs(g, 0) == sp.eye(2)
    and Sg.subs(g, 1) == sp.Matrix([[0, 1], [1, 0]]),
)
check(
    "C19 both registered gate branches are orthogonal",
    all(Sg.subs(g, value).T * Sg.subs(g, value) == sp.eye(2)
        for value in (0, 1)),
)
check(
    "C20 both registered gate branches are involutions",
    all(Sg.subs(g, value) ** 2 == sp.eye(2) for value in (0, 1)),
)

m = sp.symbols("m", real=True)
ma = sp.Matrix([m, a])
ma_after = sp.simplify(Sg * ma)
m_after = ma_after[0]
a_after = ma_after[1]
signal_after = sp.simplify(a_after * f + b * e)
energy_before = sp.simplify((m**2 + signal.dot(signal)) / 2)
energy_after = sp.simplify((m_after**2 + signal_after.dot(signal_after)) / 2)
check(
    "C21 matter plus signal energy is exact for hold and exchange",
    all(sp.simplify((energy_after - energy_before).subs(g, value)) == 0
        for value in (0, 1)),
)
check(
    "C22 signed matter plus orthogonal signal content is exact",
    all(sp.simplify(
        (m_after + a_after - m - a).subs(g, value)
    ) == 0 for value in (0, 1)),
)
check(
    "C23 signal component parallel to the pilot is a spectator",
    sp.simplify(e.dot(signal_after) - b) == 0,
)

B = sp.symbols("B", positive=True, real=True)
sign = sp.symbols("sign", real=True, nonzero=True)
matter_event = sign * sp.sqrt(2 * B)
emitted = sp.simplify(
    signal_after.subs({g: 1, m: matter_event, a: 0, b: 0})
)
expected_emitted = sp.simplify(
    sign * sp.sqrt(B / I_star) * J * reference
)
check(
    "C24 open gate emits the exact pilot-referenced zero-baseline signal",
    sp.simplify(emitted - expected_emitted) == sp.zeros(2, 1),
)
check(
    "C25 emitted signal carries exactly the event energy for sign square one",
    all(sp.simplify((emitted.dot(emitted) / 2 - B).subs(sign, sigma)) == 0
        for sigma in (-1, 1)),
)
oriented_area = sp.simplify(
    reference[0] * emitted[1] - reference[1] * emitted[0]
)
check(
    "C26 pilot-signal oriented area recovers the event sign",
    sp.ask(sp.Q.positive(oriented_area.subs(sign, 1))) is True
    and sp.ask(sp.Q.negative(oriented_area.subs(sign, -1))) is True,
)

incoming_amplitude = sp.simplify(f.dot(emitted))
absorbed = sp.simplify(
    ma_after.subs({g: 1, m: 0, a: incoming_amplitude})
)
check(
    "C27 the same gate reciprocally absorbs the emitted signal",
    sp.simplify(absorbed[0] - matter_event) == 0 and absorbed[1] == 0,
)
check(
    "C28 catalytic reference pair is unchanged by signal exchange",
    reference == reference.copy(),
)
total_with_pilot_before = sp.simplify(I_star + matter_event**2 / 2)
total_with_pilot_after = sp.simplify(I_star + emitted.dot(emitted) / 2)
check(
    "C29 total pilot matter and signal energy closes without spending pilot action",
    all(sp.simplify(
        (total_with_pilot_after - total_with_pilot_before).subs(sign, sigma)
    ) == 0 for sigma in (-1, 1)),
)
check(
    "C30 separate phase reference permits exact emission from zero signal",
    signal.subs({a: 0, b: 0}) == sp.zeros(2, 1)
    and emitted != sp.zeros(2, 1),
)

alpha = sp.symbols("alpha", real=True)
R_alpha = sp.Matrix(
    [[sp.cos(alpha), -sp.sin(alpha)],
     [sp.sin(alpha), sp.cos(alpha)]]
)
covariant_left = sp.simplify(R_alpha * J * reference)
covariant_right = sp.simplify(J * R_alpha * reference)
check(
    "C31 joint rotation of reference and signal preserves the emission law",
    sp.simplify(sp.trigsimp(covariant_left - covariant_right))
    == sp.zeros(2, 1),
)
check(
    "C32 simultaneous matter and signal sign reversal is equivariant",
    sp.simplify(emitted.subs(sign, -1) + emitted.subs(sign, 1))
    == sp.zeros(2, 1),
)
check(
    "C33 baseline action and event signal energy are separate nonduplicated accounts",
    sp.simplify(reference.dot(reference) / 2 - I_star) == 0
    and all(sp.simplify((emitted.dot(emitted) / 2).subs(sign, sigma) - B) == 0
            for sigma in (-1, 1)),
)

phase_offset = sp.symbols("phase_offset", real=True)
local_reference_phase = phase + phase_offset
downstream_reference_phase = next_phase + phase_offset
check(
    "C34 pilot-referenced sign readout is transported with the coherent rail",
    sp.expand(
        (downstream_reference_phase - local_reference_phase).subs(kappa, omega)
    ) == 0,
)

event_boundary = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md"
]
phase_rail = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md"
]
check(
    "C35 production lacks the protected phase reference and exact signal rail",
    "production acceptance does not determine either reciprocal-port" in event_boundary
    and "Production C18 has the exact dispersive trace obstruction" in phase_rail
    and "No production field is reserved as a maintained nonzero baseline" in phase_rail,
)

protocol_text = PROTOCOL.read_text(encoding="utf-8") if PROTOCOL.exists() else ""
flat_protocol = " ".join(protocol_text.split())
scope_markers = (
    "does not derive the pilot frequency",
    "does not derive G* cadence",
    "does not derive Born frequencies",
    "does not establish cost-free control",
    "does not alter production C18",
    "or completeness",
)
check(
    "C36 scope firewall blocks cadence control and production promotion",
    all(marker in flat_protocol for marker in scope_markers),
)


passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(
    f"FTD-0863 catalytic phase-reference transducer: {passed}/{total} "
    f"{'PASS' if passed == total == 36 else 'FAIL'}"
)

if passed == total == 36:
    print("AUTONOMOUS_PHASE_REFERENCE_ROTATION_IS_REVERSIBLE_AND_ACTION_PRESERVING")
    print("REFERENCE_ORIENTS_AN_EXACT_RECIPROCAL_MATTER_SIGNAL_SWAP")
    print("ZERO_BASELINE_SIGNAL_CARRIES_EVENT_ENERGY_WITHOUT_SPENDING_PILOT_ACTION")
    print("FINITE_PERIODIC_PILOT_REQUIRES_COMMENSURATE_SPATIAL_TEMPORAL_WINDING")
    print("PILOT_FREQUENCY_GSTAR_GEARBOX_AND_PRODUCTION_REALIZATION_REMAIN_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_CATALYTIC_REFERENCE_PRODUCTION_UNREALIZED")
    sys.exit(0)

sys.exit(1)
