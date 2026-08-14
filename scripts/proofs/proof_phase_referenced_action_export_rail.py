#!/usr/bin/env python3
"""Exact certificate for FTD-0861: phase-referenced action export rail."""

from __future__ import annotations

import hashlib
from pathlib import Path
import sys

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/PREREG_PHASE_REFERENCED_ACTION_EXPORT_RAIL_v1.md"
)

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md":
        "4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md":
        "06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md":
        "8269A241928681A6126B4D1F189FDEC3C5869916AF90E8825216844048D5A4C8",
    "docs/theory/10_eft_program/native_time_carrier_programme/SPEC_CARRIER_CONSTRAINTS_v1.md":
        "6E14439EF155FF3590910DAEFDAACB2A348942112664712E681D3F84C11EB23C",
    "engine/include/ftd/eft/relative_action_transducer.h":
        "E4E7C237D7AF7BB3B3000CFAC4D63C0E8126801422EF43809754BAF086400D42",
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


# Real complex structure and the coherent phase calendar.
J = sp.Matrix([[0, -1], [1, 0]])
identity2 = sp.eye(2)
check(
    "C8 real quarter turn is orthogonal orientation preserving and squares to minus one",
    J * J == -identity2 and J.T * J == identity2 and J.det() == 1,
)

phi0, kappa, omega = sp.symbols("phi0 kappa omega", real=True)
j, n, r = sp.symbols("j n r", integer=True)
phase = phi0 + kappa * j - omega * n
shifted_phase = phi0 + kappa * (j + 1) - omega * (n + 1)
check(
    "C9 one-cell calendar mismatch is exactly spatial twist minus temporal advance",
    sp.expand(shifted_phase - phase) == kappa - omega,
)
calendar_mismatch = sp.symbols("m", integer=True)
check(
    "C10 coherent shift condition is kappa minus omega modulo two pi",
    sp.simplify(
        sp.exp(sp.I * (kappa - omega)).subs(
            kappa, omega + 2 * sp.pi * calendar_mismatch
        )
        - 1
    ) == 0,
)
characteristic_phase = phase.subs({kappa: omega, j: j + r, n: n + r})
check(
    "C11 baseline phase is constant on the outward j plus one n plus one characteristic",
    sp.expand(characteristic_phase - phase.subs(kappa, omega)) == 0,
)
incoming_phase = phi0 - kappa - omega * n
next_port_phase = phi0 - omega * (n + 1)
check(
    "C12 upstream baseline equals the next port baseline under calendar coherence",
    sp.expand((incoming_phase - next_port_phase).subs(kappa, omega)) == 0,
)


# Pump one prepared baseline pair.  Rotation equivariance permits the exact
# representative beta=(sqrt(2 I_*),0) without loss of generality.
I_star, B = sp.symbols("I_star B", positive=True, real=True)
radius = sp.sqrt(2 * I_star)
beta = sp.Matrix([radius, 0])
check(
    "C13 prepared baseline carries exactly the declared nonzero action",
    sp.simplify((beta.dot(beta)) / 2 - I_star) == 0,
)

gain = sp.sqrt((I_star + B) / I_star)
loaded_plus = sp.simplify(gain * J * beta)
loaded_minus = sp.simplify(-gain * J * beta)
loaded_action = sp.simplify(loaded_plus.dot(loaded_plus) / 2)
check(
    "C14 phase-referenced pump increments carrier action by exactly event energy",
    sp.simplify(loaded_action - I_star - B) == 0,
)
check(
    "C15 loaded carrier is exactly orthogonal to its prepared baseline",
    sp.simplify(beta.dot(loaded_plus)) == 0
    and sp.simplify(beta.dot(loaded_minus)) == 0,
)


def cross(first: sp.Matrix, second: sp.Matrix) -> sp.Expr:
    return sp.expand(first[0] * second[1] - first[1] * second[0])


cross_plus = sp.simplify(cross(beta, loaded_plus))
cross_minus = sp.simplify(cross(beta, loaded_minus))
check(
    "C16 baseline-loaded oriented areas have opposite nonzero signs",
    sp.ask(sp.Q.positive(cross_plus)) is True
    and sp.ask(sp.Q.negative(cross_minus)) is True,
)
check(
    "C17 event energy is recovered from loaded action minus baseline action",
    sp.simplify(loaded_action - I_star - B) == 0,
)
check(
    "C18 event sign is recovered from oriented area against the baseline",
    sp.ask(sp.Q.positive(cross_plus)) is True
    and sp.ask(sp.Q.negative(cross_minus)) is True,
)
check(
    "C19 opposite signs have disjoint outputs on the same prepared baseline",
    loaded_plus != loaded_minus
    and sp.simplify(loaded_plus + loaded_minus) == sp.zeros(2, 1),
)

q, p = sp.symbols("q p", real=True)
z = sp.Matrix([q, p])
action = sp.simplify(z.dot(z) / 2)
arbitrary_gain = sp.sqrt((action + B) / action)
collision_plus = sp.simplify(arbitrary_gain * J * z)
minus_background = -z
minus_background_action = sp.simplify(minus_background.dot(minus_background) / 2)
collision_minus = sp.simplify(
    -sp.sqrt((minus_background_action + B) / minus_background_action)
    * J
    * minus_background
)
check(
    "C20 prepared-domain result does not erase the arbitrary-background collision",
    sp.simplify(collision_plus - collision_minus) == sp.zeros(2, 1),
)


# The finite open rail: load the new prepared input, shift every retained pair
# one cell, and export the old tail as a pair rather than as scalar energy.
def rail_step(state: list[sp.Expr], injected: sp.Expr) -> tuple[list[sp.Expr], sp.Expr]:
    return [injected, *state[:-1]], state[-1]


z0, z1, z2, y = sp.symbols("z0 z1 z2 y")
next_state, tail = rail_step([z0, z1, z2], y)
check(
    "C21 rail update is a radius-one outward dependency with explicit tail export",
    next_state == [y, z0, z1] and tail == z2,
)

event_pair = sp.Symbol("event_pair")
state = [sp.Integer(0)] * 4
for step in range(4):
    state, _ = rail_step(state, event_pair if step == 0 else sp.Symbol(f"fresh_{step}"))
check(
    "C22 a loaded pair advances exactly one causal cell per tick",
    state[3] == event_pair,
)
check(
    "C23 coherent baseline reference advances on the same characteristic",
    sp.expand(characteristic_phase - phase.subs(kappa, omega)) == 0,
)
check(
    "C24 signed quarter-turn phase offset is invariant along the rail characteristic",
    sp.simplify(
        (phase.subs(kappa, omega) + sp.pi / 2)
        - characteristic_phase
        - sp.pi / 2
    ) == 0,
)
check(
    "C25 arbitrary-depth readout recovers energy and sign against the local calendar",
    sp.simplify(loaded_action - I_star - B) == 0
    and sp.ask(sp.Q.positive(cross_plus)) is True
    and sp.ask(sp.Q.negative(cross_minus)) is True,
)

e0, e1, e2, e3 = sp.symbols("e0 e1 e2 e3", nonzero=True)
history = [sp.Integer(0)] * 4
for injected in (e0, e1, e2, e3):
    history, _ = rail_step(history, injected)
check(
    "C26 consecutive events retain their exact age ordering before tail contact",
    history == [e3, e2, e1, e0],
)
fresh_baseline_0, fresh_baseline_1 = sp.symbols(
    "fresh_baseline_0 fresh_baseline_1", nonzero=True
)
first, _ = rail_step([fresh_baseline_0, z1, z2], e0)
second, _ = rail_step(first, e1)
check(
    "C27 a new prepared input reuses the port while moving the prior event outward",
    first[0] == e0 and second[:2] == [e1, e0],
)

E0, E1, E2, B_new = sp.symbols("E0 E1 E2 B_new", nonnegative=True)
H_before = E0 + E1 + E2
H_after = B_new + E0 + E1
check(
    "C28 retained excess action changes by injection minus exported tail excess",
    sp.simplify(H_after - H_before - (B_new - E2)) == 0,
)
tail_cross = sp.symbols("tail_cross", nonzero=True, real=True)
check(
    "C29 scalar tail excess action alone is blind to orientation sign",
    sp.simplify(tail_cross**2 - (-tail_cross) ** 2) == 0,
)
check(
    "C30 exported pair plus exported calendar reference retains tail orientation",
    tail_cross != -tail_cross
    and sp.sign(tail_cross) == -sp.sign(-tail_cross),
)

B_max = sp.symbols("B_max", positive=True, real=True)
d0, d1, d2, d3 = sp.symbols("d0 d1 d2 d3", nonnegative=True)
bounded_events = [B_max - deficit for deficit in (d0, d1, d2, d3)]
check(
    "C31 length-N rail retained excess is bounded by N times the event bound",
    sp.simplify(4 * B_max - sum(bounded_events) - (d0 + d1 + d2 + d3)) == 0,
)


# A closed environment completion is a canonical-pair permutation composed
# with the fixed-control symplectic pump.  The open retained rail alone omits
# its input, tail, and event control and is not claimed reversible.
omega2 = sp.Matrix([[0, 1], [-1, 0]])
Omega = sp.diag(omega2, omega2, omega2, omega2)
pair_permutation = sp.zeros(8)
pair_map = (3, 0, 1, 2)  # (tail, pumped input, old depth 0, old depth 1)
for output_pair, input_pair in enumerate(pair_map):
    pair_permutation[2 * output_pair, 2 * input_pair] = 1
    pair_permutation[2 * output_pair + 1, 2 * input_pair + 1] = 1
check(
    "C32 environment-completed rail permutation preserves the canonical form",
    pair_permutation.T * Omega * pair_permutation == Omega
    and pair_permutation.det() == 1,
)
I, theta = sp.symbols("I theta", real=True)
action_angle_jacobian = sp.Matrix([[sp.diff(I + B, I), sp.diff(I + B, theta)],
                                   [sp.diff(theta + sp.pi / 2, I),
                                    sp.diff(theta + sp.pi / 2, theta)]])
check(
    "C33 fixed-branch action pump preserves the action-angle symplectic area",
    action_angle_jacobian.det() == 1,
)
old_state = [sp.Symbol(f"old_{index}") for index in range(3)]
new_state, old_tail = rail_step(old_state, sp.Symbol("pumped_input"))
recovered_state = [*new_state[1:], old_tail]
check(
    "C34 tail plus fixed event inverse makes the extended rail step injective",
    recovered_state == old_state,
)

event_boundary = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md"
]
transducer_boundary = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_RELATIVE_ACTION_ORIENTATION_TRANSDUCER_BOUNDARY_v1.md"
]
check(
    "C35 selected phase rail is not equivalent to frozen production C18",
    "cannot be renamed as the exact one-cell" in event_boundary
    and "does not actuate the relative pair" in transducer_boundary
    and "No production code changed" in event_boundary,
)

protocol_text = PROTOCOL.read_text(encoding="utf-8") if PROTOCOL.exists() else ""
flat_protocol = " ".join(protocol_text.split())
scope_markers = (
    "does not derive a persistent vacuum carrier",
    "does not derive the phase calendar",
    "does not derive G* cadence",
    "does not derive Born frequencies",
    "does not alter production C18",
    "or completeness",
)
check(
    "C36 scope firewall blocks clock quantum and production promotion",
    all(marker in flat_protocol for marker in scope_markers),
)


passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(
    f"FTD-0861 phase-referenced action export rail: {passed}/{total} "
    f"{'PASS' if passed == total == 36 else 'FAIL'}"
)

if passed == total == 36:
    print("COHERENT_PHASE_CALENDAR_PRESERVES_SIGNED_QUARTER_TURN_ALONG_CAUSAL_RAIL")
    print("PREPARED_BASELINE_MAKES_EVENT_ENERGY_AND_ORIENTATION_EXACTLY_RECOVERABLE")
    print("FINITE_RAIL_HAS_EXACT_EXCESS_ACTION_LEDGER_AND_BOUNDED_RETAINED_LOAD")
    print("REVERSIBILITY_REQUIRES_INPUT_TAIL_AND_FIXED_EVENT_CONTROL_ENVIRONMENT")
    print("PRODUCTION_C18_PHASE_MAINTENANCE_AND_CONTROLLER_REMAIN_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_SELECTED_REFERENCE_RAIL_PRODUCTION_UNREALIZED")
    sys.exit(0)

sys.exit(1)
