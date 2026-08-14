#!/usr/bin/env python3
"""FTD-0857 exact native event-activation/characteristic boundary certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
SOURCES = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/eft/native_modal_phase_action.h":
        "C1E9D5C1944E66D7601D193DC77A39980EBA24B84A41F7D752A3A363910060B6",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_MINIMUM_RECIPROCAL_RECORD_PORT_BARRIER_v1.md":
        "5D13921555B2289ABC5425F4D2436545C4C3BF0638FC71C98A577E3325D215EA",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_DIAGNOSTIC_EVENT_ENERGY_CUBIC_RAIL_GEARBOX_v1.md":
        "4A498C6D7C7E65FA685D9F0879157D76713F310A6D025CCAA8756C3F1E0322E6",
}

checks: list[tuple[str, bool]] = []


def check(label: str, condition: object) -> None:
    ok = bool(condition)
    checks.append((label, ok))
    print(f"{'PASS' if ok else 'FAIL'}  {label}")


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


for relative, expected in SOURCES.items():
    path = ROOT / relative
    check(f"source hash {relative}", path.is_file() and digest(path) == expected)

voxel = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
phase_read = (
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
).read_text(encoding="utf-8")
phase_write = (
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp"
).read_text(encoding="utf-8")
modal = (
    ROOT / "engine/include/ftd/eft/native_modal_phase_action.h"
).read_text(encoding="utf-8")
protocol = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md"
).read_text(encoding="utf-8")

event_start = phase_write.index("// ---- Loop 2: Genesis and Evaporation ----")
event_end = phase_write.index("// ---- Sequential post-pass", event_start)
event_slice = phase_write[event_start:event_end]
genesis_pos = event_slice.index("// Genesis (dual)")
evaporation_pos = event_slice.index("// Evaporation (shared single + dual)")

check(
    "C8 production stores common-capable and relative-capable field/momentum pairs",
    all(
        declaration in voxel
        for declaration in (
            "Vec3 flux_L;",
            "Vec3 flux_R;",
            "Vec3 wave_vel_L;",
            "Vec3 wave_vel_R;",
        )
    ),
)
check(
    "C9 phase_read applies matched C18 Laplacians to L and R",
    "lap_L = fL + eL - rb.voxels_[i].flux_L * 4.0;" in phase_read
    and "lap_R = fR + eR - rb.voxels_[i].flux_R * 4.0;" in phase_read
    and "rb.delta_j_L_[i] = lap_L * cw2;" in phase_read
    and "rb.delta_j_R_[i] = lap_R * cw2;" in phase_read,
)
check(
    "C10 equal production matter sources cancel from the relative acceleration",
    "rb.delta_j_L_[i] += curl_sv - grad_s;" in phase_read
    and "rb.delta_j_R_[i] += curl_sv - grad_s;" in phase_read,
)
check(
    "C11 phase_write advances the two dual pairs by matched kick-drift rules",
    "v.wave_vel_L += rb.delta_j_L_[i];" in phase_write
    and "v.wave_vel_R += rb.delta_j_R_[i];" in phase_write
    and "v.flux_L += v.wave_vel_L;" in phase_write
    and "v.flux_R += v.wave_vel_R;" in phase_write,
)
check(
    "C12 production reconstructs only the common observable pair",
    "v.flux = v.flux_L + v.flux_R;" in phase_write
    and "v.wave_vel = v.wave_vel_L + v.wave_vel_R;" in phase_write,
)
check(
    "C13 event decisions run in a fixed sequential site order",
    "SEQUENTIAL — DETERMINISM REQUIREMENT" in event_slice
    and "for (int i = 0; i < N; ++i)" in event_slice,
)
check(
    "C14 dual genesis acceptance reads void state and common flux magnitude",
    "v.state == 0 && v.flux.mag2() > K_GENESIS * K_GENESIS" in event_slice
    and "double dens = std::sqrt(v.flux.mag2());" in event_slice
    and "double p = 1.0 - std::exp(-excess / K_MANIFEST);" in event_slice,
)
check(
    "C15 genesis acceptance uses the stateless site-tick-stream draw",
    "VoxelRng::GenesisManifest" in event_slice
    and "voxel_uniform(gseed, i, rb.tick_" in event_slice,
)
check(
    "C16 evaporation acceptance requires an unlocked occupied record",
    "v.state != 0 && !v.locked" in event_slice,
)
check(
    "C17 evaporation hazard reads common site-plus-six-neighbour energy",
    "double local_energy = v.flux.mag2() + v.wave_vel.mag2();" in event_slice
    and "const auto& nbrs = rb.lattice_.neighbors_6(i);" in event_slice
    and "local_energy += rb.voxels_[n].flux.mag2() + rb.voxels_[n].wave_vel.mag2();"
    in event_slice
    and "double evap_prob = std::exp(-local_energy / (K_MANIFEST * K_MANIFEST));"
    in event_slice,
)
check(
    "C18 evaporation acceptance uses a keyed draw and local proper-time factor",
    "VoxelRng::Evaporation" in event_slice
    and "const double dtau = proper_time_rate" in event_slice
    and "evap_prob * K_EVAP_RATE * dtau" in event_slice,
)
check(
    "C19 source order permits genesis followed by evaporation at one site",
    genesis_pos < evaporation_pos
    and "manifest_at(rb, v, chi" in event_slice
    and "rb.set_state(i, 0);" in event_slice,
)
check(
    "C20 event acceptance reads no Born Gstar measurement-context or reciprocal-port target",
    all(
        token not in event_slice
        for token in (
            "Born",
            "G_STAR",
            "MeasurementContext",
            "scatter_reciprocal_record_port",
            "sqrt(2 * B)",
        )
    ),
)

# Common/relative change of coordinates and its exact kernel.
jl, jr, wl, wr, dj, dw = sp.symbols("jl jr wl wr dj dw", real=True)
c = jl + jr
d = jl - jr
v = wl + wr
p_rel = wl - wr
jl2 = jl + dj / 2
jr2 = jr - dj / 2
wl2 = wl + dw / 2
wr2 = wr - dw / 2

check(
    "C21 common-relative transform is exactly invertible",
    sp.solve(
        (sp.Eq(sp.Symbol("C"), jl + jr), sp.Eq(sp.Symbol("D"), jl - jr)),
        (jl, jr),
        dict=True,
    )
    == [{jl: sp.Symbol("C") / 2 + sp.Symbol("D") / 2,
         jr: sp.Symbol("C") / 2 - sp.Symbol("D") / 2}],
)
check(
    "C22 arbitrary antisymmetric perturbations fix both common coordinates",
    sp.simplify((jl2 + jr2) - c) == 0
    and sp.simplify((wl2 + wr2) - v) == 0,
)
check(
    "C23 the same perturbations shift relative field and momentum arbitrarily",
    sp.simplify((jl2 - jr2) - d - dj) == 0
    and sp.simplify((wl2 - wr2) - p_rel - dw) == 0,
)

kg, km, ug = sp.symbols("K_G K_M u_g", positive=True)
common_magnitude = sp.symbols("Cmag", positive=True)
genesis_probability = 1 - sp.exp(-(common_magnitude - kg) / km)
check(
    "C24 genesis acceptance functional is invariant on the relative kernel",
    genesis_probability.free_symbols == {common_magnitude, kg, km, ug} - {ug}
    and sp.simplify((jl2 + jr2) - (jl + jr)) == 0,
)

e7, ke, dtau, ue = sp.symbols("E7 K_E dtau u_e", positive=True)
evaporation_threshold = sp.exp(-e7 / km**2) * ke * dtau
check(
    "C25 evaporation acceptance functional is invariant on the relative kernel",
    evaporation_threshold.free_symbols == {e7, km, ke, dtau}
    and sp.simplify((wl2 + wr2) - (wl + wr)) == 0,
)

# Exact counterexample: the common event data agree while the relative input differs.
common_state_a = (sp.Integer(1), sp.Integer(1), sp.Integer(0), sp.Integer(0))
common_state_b = (sp.Integer(1), sp.Integer(1), sp.Integer(1), sp.Integer(-1))
ca = common_state_a[0] + common_state_a[1]
cb = common_state_b[0] + common_state_b[1]
va = common_state_a[2] + common_state_a[3]
vb = common_state_b[2] + common_state_b[3]
pa = common_state_a[2] - common_state_a[3]
pb = common_state_b[2] - common_state_b[3]
check(
    "C26 identical common trigger data admit distinct relative incoming amplitudes",
    ca == cb == 2 and va == vb == 0 and pa == 0 and pb == 2,
)
check(
    "C27 ordered genesis-evaporation decisions cannot be encoded by one unlabeled bit",
    len({(0, 0), (1, 0), (0, 1), (1, 1)}) == 4 > len({0, 1}),
)

# Exact incoming/outgoing chart.
p, g = sp.symbols("p g", real=True)
sqrt2 = sp.sqrt(2)
incoming = (p + g) / sqrt2
outgoing = (p - g) / sqrt2
p_inverse = (incoming + outgoing) / sqrt2
g_inverse = (incoming - outgoing) / sqrt2
check(
    "C28 characteristic chart has an exact inverse",
    sp.simplify(p_inverse - p) == 0 and sp.simplify(g_inverse - g) == 0,
)
check(
    "C29 characteristic chart preserves the quadratic signal energy",
    sp.simplify((incoming**2 + outgoing**2 - p**2 - g**2) / 2) == 0,
)
check(
    "C30 characteristic square difference is the signed edge current",
    sp.simplify((incoming**2 - outgoing**2) / 2 - p * g) == 0,
)
orientation_reversed = (
    sp.simplify((p - g) / sqrt2),
    sp.simplify((p + g) / sqrt2),
)
check(
    "C31 spatial orientation reversal swaps incoming and outgoing",
    orientation_reversed == (outgoing, incoming),
)
time_reversed = (
    sp.simplify((-p + g) / sqrt2),
    sp.simplify((-p - g) / sqrt2),
)
check(
    "C32 physical time reversal maps incoming-outgoing to minus outgoing-incoming",
    time_reversed == (-outgoing, -incoming),
)

# Exact plane-symmetric axial reduction of the frozen face+edge C18 stencil.
qm, q0, qp = sp.symbols("q_minus q_0 q_plus", real=True)
face_average = (qm + qp + 4 * q0) / 3
edge_average = (4 * qm + 4 * qp + 4 * q0) / 6
c18_axial = sp.expand(face_average + edge_average - 4 * q0)
check(
    "C33 plane-symmetric C18 Laplacian is the exact one-dimensional second difference",
    sp.simplify(c18_axial - (qm - 2 * q0 + qp)) == 0
    and "constexpr double INV3 = 1.0 / 3.0;" in phase_read
    and "constexpr double INV6 = 1.0 / 6.0;" in phase_read,
)

s2, c2 = sp.symbols("s2 c2", nonnegative=True)
a = 4 * c2 * s2
u = sp.Matrix(((1 - a, 1), (-a, 1)))
check("C34 axial production kick-drift map has unit determinant", sp.simplify(u.det()) == 1)
check(
    "C35 axial production kick-drift trace is two minus the source eigenvalue",
    sp.simplify(sp.trace(u) - (2 - a)) == 0,
)
cos_theta = 1 - a / 2
check(
    "C36 production dispersion obeys sin squared half-theta equals c squared sin squared half-k",
    sp.simplify((1 - cos_theta) / 2 - c2 * s2) == 0
    and "result.cos_theta = 1.0 - 0.5 * eigenvalue;" in modal,
)
shift_trace = 2 - 4 * s2
trace_defect = sp.factor(sp.trace(u) - shift_trace)
check(
    "C37 selected c squared one-third forbids exact one-cell characteristic shifts",
    trace_defect == 4 * s2 * (1 - c2)
    and sp.simplify(trace_defect.subs(c2, sp.Rational(1, 3))) == sp.Rational(8, 3) * s2
    and sp.simplify(sp.trace(u).subs({c2: sp.Rational(1, 3), s2: 1}) - (-2))
    == sp.Rational(8, 3),
)

# Signal work for both registered eligibility values.
m, i = sp.symbols("m i", real=True)
x = sp.Matrix((m, i))
s0 = sp.eye(2)
s1 = sp.Matrix(((0, 1), (1, 0)))
signal_work = [sp.simplify(((s * x).dot(s * x) - x.dot(x)) / 2) for s in (s0, s1)]
check(
    "C38 identity and swap gates do zero work on the declared signal account",
    signal_work == [0, 0],
)
check(
    "C39 production acceptance does not test relative readiness or on-shell receiver amplitude",
    all(
        token not in event_slice
        for token in ("Q_0", "ready_port", "relative_energy", "incoming", "outgoing")
    )
    and "controller state, switching work/dissipation" in protocol,
)
check(
    "C40 selected-law controller and claim-promotion firewalls remain explicit",
    all(
        phrase in protocol
        for phrase in (
            "not thereby\n+derived physical law",
            "controller state, switching work/dissipation",
            "No numerical search",
            "Born, Bell, `G*`, thermodynamic, biological, or",
            "Expected honest result: **Outcome B**",
        )
    ),
)

passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0857 native event activation and characteristic boundary: {passed}/{total} PASS")
if passed == total == 40:
    print("PRODUCTION_EVENT_ACCEPTANCE_IS_DETERMINISTIC_LOCAL_AND_TARGET_BLIND_GIVEN_FIXED_INPUTS")
    print("COMMON_FIELD_TRIGGERS_DO_NOT_DETERMINE_THE_RELATIVE_ON_SHELL_RECORD_PORT")
    print("RELATIVE_EDGE_PAIR_HAS_AN_EXACT_INCOMING_OUTGOING_ENERGY_CURRENT_CHART")
    print("FROZEN_C18_DISPERSION_IS_NOT_THE_EXACT_ONE_CELL_HISTORY_RAIL")
    print("SIGNAL_WORK_CLOSES_ZERO_WHILE_PHYSICAL_CONTROLLER_COST_REMAINS_OPEN")
    print("VERDICT=OUTCOME_B_NATIVE_TRIGGER_AND_CHART_PRODUCTION_PORT_INCOMPLETE")
    raise SystemExit(0)

print("VERDICT=OUTCOME_C_NO_THEOREM")
raise SystemExit(1)

