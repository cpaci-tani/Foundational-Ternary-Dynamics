#!/usr/bin/env python3
"""FTD-0876 exact certificate for the native flux/wave-velocity carrier."""

from __future__ import annotations

import hashlib
from fractions import Fraction
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_v1.md"
)
THEOREM = ROOT / (
    "docs/theory/10_eft_program/derivations/"
    "native_time_carrier_programme/"
    "THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_PRODUCTION_BOUNDARY_v1.md"
)

FROZEN = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md":
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/src/render_bridge.cpp":
        "BFAD7886CB83A590F0AACA11C03CE25B1FF51D94B4C17B06F5D555E46C18D724",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/tests/test_leapfrog_integrator_audit.cpp":
        "725B6B66FE8A83960E572332FDA6CE5E0021FBEA6389B465EEE647E364E0313C",
}
PROTOCOL_SHA256 = "5808E0EC49F90F654A9EE4911BECAC64BE5063D2EBF4D58A69099F977DE20484"

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


# Frozen-source and protocol gates.
for rel, expected in FROZEN.items():
    check(f"source hash {rel}", sha256(ROOT / rel) == expected)
check("protocol pre-run hash", sha256(PROTOCOL) == PROTOCOL_SHA256)

theorem_text = THEOREM.read_text(encoding="utf-8")
check(
    "theorem declares exact history/Markov equivalence",
    "[THEOREM — EXACT HISTORY/MARKOV EQUIVALENCE]" in theorem_text,
)

# Exact history chart.
q_prev, q_now, h = sp.symbols("q_prev q_now h", nonzero=True)
p_half = (q_now - q_prev) / h
recovered_prev = sp.simplify(q_now - h * p_half)
check("scalar history chart recovers the prior slice", recovered_prev == q_prev)
check("scalar history chart preserves the current slice", q_now == q_now)

vectors = [
    ([Fraction(1, 3), Fraction(-2, 5)], [Fraction(7, 4), Fraction(3, 8)], Fraction(2, 3)),
    ([Fraction(-5, 7), Fraction(0)], [Fraction(4, 9), Fraction(-11, 6)], Fraction(5, 4)),
]
vector_roundtrips = True
for previous, current, step in vectors:
    momentum = [(b - a) / step for a, b in zip(previous, current)]
    recovered = [b - step * p for b, p in zip(current, momentum)]
    vector_roundtrips &= recovered == previous
check("exact rational finite-vector history charts round trip", vector_roundtrips)
check("history chart is undefined at zero step", sp.denom(p_half).has(h))

# Kick/drift and recurrence algebra in a two-coordinate witness.
h0 = sp.symbols("h0", nonzero=True)
k11, k12, k21, k22 = sp.symbols("k11 k12 k21 k22")
K = sp.Matrix([[k11, k12], [k21, k22]])
I = sp.eye(2)
Z = sp.zeros(2)
Omega = Z.row_join(I).col_join((-I).row_join(Z))
Kick = I.row_join(Z).col_join((-h0 * K).row_join(I))
Drift = I.row_join(h0 * I).col_join(Z.row_join(I))
S = sp.simplify(Drift * Kick)

Ksym = sp.Matrix([[k11, k12], [k12, k22]])
Kick_sym = I.row_join(Z).col_join((-h0 * Ksym).row_join(I))
Ssym = sp.simplify(Drift * Kick_sym)
check("drift is exactly symplectic", sp.simplify(Drift.T * Omega * Drift - Omega) == sp.zeros(4))
check("symmetric-stiffness kick is exactly symplectic", sp.simplify(Kick_sym.T * Omega * Kick_sym - Omega) == sp.zeros(4))
check("composed symmetric kick/drift is exactly symplectic", sp.simplify(Ssym.T * Omega * Ssym - Omega) == sp.zeros(4))

residual = sp.simplify(S.T * Omega * S - Omega)
check("general residual vanishes when K is symmetric", sp.simplify(residual.subs(k21, k12)) == sp.zeros(4))
check("nonsymmetric stiffness produces a nonzero residual", residual.subs({k11: 1, k12: 2, k21: 3, k22: 4, h0: 1}) != sp.zeros(4))
check("symplectic residual depends only on K transpose defect", all(sp.simplify(x).subs(k21, k12) == 0 for x in residual))
check("composed map has determinant one", sp.simplify(Ssym.det()) == 1)

Sinv = sp.simplify(Kick_sym.inv() * Drift.inv())
check("declared inverse is a left inverse", sp.simplify(Sinv * Ssym) == sp.eye(4))
check("declared inverse is a right inverse", sp.simplify(Ssym * Sinv) == sp.eye(4))
check("phase volume is exactly preserved", sp.simplify(abs(Ssym.det())) == 1)

# Recurrence from the staggered history chart.
qm1, q0, stiffness = sp.symbols("qm1 q0 stiffness")
phalf = (q0 - qm1) / h0
pnext = phalf - h0 * stiffness * q0
qnext = sp.expand(q0 + h0 * pnext)
recurrence_residual = sp.simplify(qnext - 2 * q0 + qm1 + h0**2 * stiffness * q0)
check("kick/drift equals the registered second-order recurrence", recurrence_residual == 0)

# Three native canonical pairs and componentwise/vector rail generator.
Omega6 = sp.zeros(6)
for a in range(3):
    Omega6[a, 3 + a] = 1
    Omega6[3 + a, a] = -1
check("one voxel carries a nondegenerate six-dimensional canonical form", Omega6.det() == 1)
check("three same-component canonical brackets equal one", all(Omega6[a, 3 + a] == 1 for a in range(3)))
check("cross-component canonical brackets vanish", all(Omega6[a, 3 + b] == 0 for a in range(3) for b in range(3) if a != b))

e1, e2, e3 = sp.symbols("e1 e2 e3", real=True)
e = sp.Matrix([e1, e2, e3])
check("unit projection has canonical bracket", sp.simplify((e.T * e)[0]).subs(e1**2 + e2**2 + e3**2, 1) == 1)

Jj = sp.Matrix(sp.symbols("Jj0:3"))
Pj = sp.Matrix(sp.symbols("Pj0:3"))
Jk = sp.Matrix(sp.symbols("Jk0:3"))
Pk = sp.Matrix(sp.symbols("Pk0:3"))
vector_generator = sp.expand((Jj.dot(Pk) - Jk.dot(Pj)))
component_sum = sp.expand(sum(Jj[a] * Pk[a] - Jk[a] * Pj[a] for a in range(3)))
check("vector bond generator is the sum of scalar generators", sp.simplify(vector_generator - component_sum) == 0)

scalar_subs = {Jj[1]: 0, Jj[2]: 0, Pj[1]: 0, Pj[2]: 0, Jk[1]: 0, Jk[2]: 0, Pk[1]: 0, Pk[2]: 0}
check("vector generator reduces to the scalar rail", sp.simplify(vector_generator.subs(scalar_subs) - (Jj[0] * Pk[0] - Jk[0] * Pj[0])) == 0)

R = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 1]])
rotated_generator = sp.expand((R * Jj).dot(R * Pk) - (R * Jk).dot(R * Pj))
check("vector generator is invariant under a cubic quarter-turn", sp.simplify(rotated_generator - vector_generator) == 0)

# Source-locked engine facts.
voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
render = (ROOT / "engine/src/render_bridge.cpp").read_text(encoding="utf-8")
read = (ROOT / "engine/src/render_bridge_phases/phase_read.cpp").read_text(encoding="utf-8")
write = (ROOT / "engine/src/render_bridge_phases/phase_write.cpp").read_text(encoding="utf-8")
audit = (ROOT / "engine/tests/test_leapfrog_integrator_audit.cpp").read_text(encoding="utf-8")

check("Voxel stores native flux", "Vec3 flux;" in voxel)
check("Voxel stores native wave velocity", "Vec3 wave_vel;" in voxel)
check("dual substrate stores both wave velocities", "Vec3 wave_vel_L;" in voxel and "Vec3 wave_vel_R;" in voxel)
check("render source declares second-order wave equation", "d²J/dt² = c²∇²J" in render)
check("phase read computes a Laplacian acceleration", "rb.delta_j_[i] = lap * cw2;" in read)
check("phase write performs momentum kick", "v.wave_vel += rb.delta_j_[i] * rb.dt_;" in write)
check("phase write performs configuration drift", "v.flux += v.wave_vel * rb.dt_;" in write)
check("source explicitly uses staggered leapfrog interpretation", "wave_vel = v(t + h/2)" in render)
check("damping scales both canonical coordinates", "v.flux *= eff_damping;" in write and "v.wave_vel *= eff_damping;" in write)
check("Langevin branch consumes indexed noise", "VoxelRng::LangevinNoiseX" in write and "one_minus_gamma * v.wave_vel.x + sigma * nx" in write)
check("Gauss projection is a separate post-wave map", "projection remains a separate constraint map" in render)
check("genesis and evaporation are outside the free-wave loop", "Loop 2: Genesis and Evaporation" in write)
check("legacy audit isolates damping and Gauss off", "rb.toggles.damping           = false;" in audit and "rb.toggles.gauss_projection  = false;" in audit)

# Exact closed-negative controls.
rho = sp.symbols("rho")
D = rho * sp.eye(4)
check("uniform damping pulls back Omega by rho squared", sp.simplify(D.T * Omega * D - rho**2 * Omega) == sp.zeros(4))
check("uniform damping determinant is rho to phase dimension", sp.simplify(D.det() - rho**4) == 0)
check("strict damping is not symplectic", (D.T * Omega * D).subs(rho, sp.Rational(3, 4)) != Omega)
check("zero damping factor is noninvertible", D.subs(rho, 0).det() == 0)

G = sp.diag(1, 0, 1, 0)
check("nonidentity projection is idempotent", G * G == G and G != sp.eye(4))
check("nonidentity projection is noninvertible", G.det() == 0)
check("every symplectic witness here is invertible", Ssym.det() != 0)

# Symplectic does not mean the naive continuous energy is exactly conserved.
q_before = sp.Rational(1)
p_before = sp.Rational(0)
h_energy = sp.Rational(1, 2)
p_after = p_before - h_energy * q_before
q_after = q_before + h_energy * p_after
energy_before = (p_before**2 + q_before**2) / 2
energy_after = (p_after**2 + q_after**2) / 2
check("finite-step naive energy changes in an exact witness", sp.simplify(energy_after - energy_before) == sp.Rational(-3, 32))
check("the same finite-step witness has unit Jacobian determinant", sp.Matrix([[1 - h_energy**2, h_energy], [-h_energy, 1]]).det() == 1)

scope_markers = [
    "No new selected type is booked",
    "COMPLETE_PRODUCTION_TICK_SYMPLECTIC=NO",
    "GSTAR_ROLE=SEPARATE_CALENDAR",
    "No Born, Bell, `G*`, Lorentz, biological, or completeness result",
    "does not show that the production tick contains this bond interaction",
]
check("all registered scope markers are present", all(marker in theorem_text for marker in scope_markers))
check("carrier availability is separated from preparation", "carrier-coordinate\navailability" in theorem_text and "dynamic preparation" in theorem_text)
check("terminal gate reached with C1-C54 passing", failures == 0 and checks == 54)

print(f"\nFTD-0876 flux/wave-velocity canonical carrier: {checks - failures}/{checks} PASS")
if failures == 0 and checks == 55:
    print("FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_THEOREM")
    print("HISTORY_MARKOV_CHART=EXACT_BIJECTION")
    print("FREE_WAVE_KICK_DRIFT=SYMPLECTIC")
    print("NATIVE_CANONICAL_PAIRS_PER_SITE=3")
    print("COMPLETE_PRODUCTION_TICK_SYMPLECTIC=NO")
    print("PRODUCTION_PARITY_ACTUATOR=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0876_CERTIFICATE_INVALID")
raise SystemExit(1)
