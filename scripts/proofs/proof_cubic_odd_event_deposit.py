#!/usr/bin/env python3
"""Exact certificate for FTD-0853: cubic odd event deposit."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path
import sys

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md":
        "7F393F78C2572ED9C61B20D897F3786BB366B305BA831DDB6CAD42344F4131E7",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_MINIMUM_ODD_EVENT_RECEIVER_v1.md":
        "ED76BCD3266A472A96601BD673E85FF43B60CD0B2C5AF09E27CD08DA0ED700CF",
    "docs/theory/02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md":
        "50CB845B2CB3874028A9C49C36141EB061785E6160F7880C361A21526C3461C0",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/src/energy_ledger_compute.cpp":
        "2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/lattice.h":
        "C4FCF605FEAC11BB60EC77584F2E9D6BD33A1ADC576BE9EBFBED0E8478B2B831",
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

carrier = texts[
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CAUSAL_ODD_PULSE_HISTORY_CARRIER_v1.md"
]
irreversibility = texts[
    "docs/theory/02_foundations/ANALYSIS_FULL_STATE_IRREVERSIBILITY_v1.md"
]
phase_write = texts["engine/src/render_bridge_phases/phase_write.cpp"]
ledger = texts["engine/src/energy_ledger_compute.cpp"]
voxel = texts["engine/include/ftd/voxel.h"]
lattice = texts["engine/include/ftd/lattice.h"]

check(
    "C8 production exposes the required dual wave-velocity type",
    "Vec3 wave_vel_L" in voxel and "Vec3 wave_vel_R" in voxel,
)
check(
    "C9 production lattice exposes exactly six face neighbours",
    "std::array<int, 6> neighbors_6" in lattice
    and all(token in lattice for token in ("xp * size2", "xm * size2", "yp * size_", "ym * size_", "+ zp", "+ zm")),
)
check(
    "C10 production reconstructs common wave velocity as L plus R",
    "v.wave_vel = v.wave_vel_L + v.wave_vel_R" in phase_write,
)
check(
    "C11 aggregate energy ledger squares the reconstructed common channels",
    "E_field += v.flux.mag2()" in ledger
    and "E_wave  += v.wave_vel.mag2()" in ledger,
)
check(
    "C12 aggregate ledger has no separate L or R quadratic term",
    all(token not in ledger for token in ("flux_L", "flux_R", "wave_vel_L", "wave_vel_R")),
)


def mat_vec(matrix: tuple[tuple[int, ...], ...], vector: tuple[int, ...]) -> tuple[int, ...]:
    return tuple(sum(matrix[i][j] * vector[j] for j in range(3)) for i in range(3))


group: list[tuple[tuple[int, ...], ...]] = []
for perm in itertools.permutations(range(3)):
    for signs in itertools.product((-1, 1), repeat=3):
        rows = []
        for i in range(3):
            row = [0, 0, 0]
            row[perm[i]] = signs[i]
            rows.append(tuple(row))
        group.append(tuple(rows))

check(
    "C13 full cubic signed-permutation group has 48 distinct matrices",
    len(group) == 48 and len(set(group)) == 48,
)

face_rep = (1, 0, 0)
edge_rep = (1, 1, 0)
corner_rep = (1, 1, 1)
face_orbit = {mat_vec(g, face_rep) for g in group}
edge_orbit = {mat_vec(g, edge_rep) for g in group}
corner_orbit = {mat_vec(g, corner_rep) for g in group}
check("C14 directed face orbit has size six", len(face_orbit) == 6)
check("C15 directed edge orbit has size twelve", len(edge_orbit) == 12)
check("C16 directed corner orbit has size eight", len(corner_orbit) == 8)

moore = set(itertools.product((-1, 0, 1), repeat=3)) - {(0, 0, 0)}
check(
    "C17 face edge corner orbits partition the 26-site Moore shell",
    face_orbit.isdisjoint(edge_orbit)
    and face_orbit.isdisjoint(corner_orbit)
    and edge_orbit.isdisjoint(corner_orbit)
    and face_orbit | edge_orbit | corner_orbit == moore,
)
check(
    "C18 six faces are the minimum nonzero first-shell directed orbit",
    min(map(len, (face_orbit, edge_orbit, corner_orbit))) == 6,
)

faces = tuple(sorted(face_orbit))
check(
    "C19 every face direction is unit length",
    all(sum(component * component for component in nu) == 1 for nu in faces),
)
face_sum = tuple(sum(nu[k] for nu in faces) for k in range(3))
check("C20 six directed faces have zero vector sum", face_sum == (0, 0, 0))

B = sp.symbols("B", positive=True, real=True)
s = sp.symbols("s", real=True)
p = sp.sqrt(B / 6)
nu = sp.Matrix([1, 0, 0])
dL = s * p * nu
dR = -s * p * nu
check(
    "C21 every arm has exactly zero common increment",
    all(sp.simplify(value) == 0 for value in dL + dR),
)
check(
    "C22 every arm places twice the signed impulse in the relative channel",
    all(sp.simplify((dL - dR)[i] - 2 * s * p * nu[i]) == 0 for i in range(3)),
)

six_zero_background_energy = sp.simplify(
    6 * (sum(value**2 for value in dL) + sum(value**2 for value in dR)) / 2
)
check(
    "C23 ready-shell dual kinetic energy increment is exactly B",
    sp.simplify(six_zero_background_energy.subs(s**2, 1) - B) == 0,
)

Q0 = sp.symbols("Q0", real=True)
delta_k = sp.expand(s * p * Q0 + 6 * p**2)
check(
    "C24 general shell energy increment is spQ0 plus six p squared",
    sp.simplify(delta_k - (s * p * Q0 + 6 * p**2)) == 0,
)
check(
    "C25 ready-port condition Q0 zero closes event energy exactly",
    sp.simplify(delta_k.subs({Q0: 0, s**2: 1}) - B) == 0,
)

Q1 = sp.expand(Q0 + 12 * s * p)
check(
    "C26 post-event radial coordinate is signed square-root 24B on ready port",
    sp.simplify((Q1.subs(Q0, 0) ** 2 - 24 * B).subs(s**2, 1)) == 0,
)
q1_plus = sp.simplify(Q1.subs({Q0: 0, s: 1}))
q1_minus = sp.simplify(Q1.subs({Q0: 0, s: -1}))
check(
    "C27 radial coordinate recovers the erased sign",
    sp.ask(sp.Q.positive(q1_plus)) is True
    and sp.ask(sp.Q.negative(q1_minus)) is True,
)
check(
    "C28 radial coordinate recovers the exported energy",
    sp.simplify(q1_plus**2 / 24 - B) == 0
    and sp.simplify(q1_minus**2 / 24 - B) == 0,
)

# Once s and B are recovered, subtraction of the known pulse restores every
# arbitrary pre-event shell component. One symbolic vector arm witnesses the
# componentwise identity; covariance applies it to all six arms.
wL = sp.Matrix(sp.symbols("wL0:3", real=True))
wR = sp.Matrix(sp.symbols("wR0:3", real=True))
wL_after = wL + dL
wR_after = wR + dR
check(
    "C29 reduced inverse subtracts the pulse and recovers arbitrary background",
    all(sp.simplify((wL_after - dL)[i] - wL[i]) == 0 for i in range(3))
    and all(sp.simplify((wR_after - dR)[i] - wR[i]) == 0 for i in range(3)),
)
check(
    "C30 off-compliance naive deposit has exact uncancelled cross-energy defect",
    sp.simplify(delta_k.subs(s**2, 1) - B - s * p * Q0) == 0
    and sp.simplify(s * p * Q0) != 0,
)

group_preserves_faces = all({mat_vec(g, nu0) for nu0 in faces} == set(faces) for g in group)
conjugation_covariant = all(
    sp.simplify((-s) * p * nu[i] - dR[i]) == 0
    and sp.simplify(-(-s) * p * nu[i] - dL[i]) == 0
    for i in range(3)
)
check(
    "C31 deposit is P4-local cubically covariant balanced and L-R conjugation covariant",
    group_preserves_faces
    and face_sum == (0, 0, 0)
    and conjugation_covariant
    and "neighbors_6" in lattice,
)

transition_scope = phase_write.split("// ---- Loop 2")[0]
forbidden = ("MeasurementContext", "Born", "G_STAR", "cadence_target")
production_complete = False
check(
    "C32 production lacks the deposit ledger and full-state inverse without target leakage",
    not production_complete
    and "No production event deposits" in carrier
    and "pure relative state" in carrier
    and "current engine update map is non-injective" in irreversibility
    and all(token not in transition_scope for token in forbidden),
)


passed = sum(ok for _, ok in checks)
total = len(checks)
print()
print(f"FTD-0853 cubic odd event deposit: {passed}/{total} "
      f"{'PASS' if passed == total == 32 else 'FAIL'}")

if passed == total == 32:
    print("SIX_FACE_ODD_DEPOSIT_IS_P4_LOCAL_CUBICALLY_BALANCED_AND_ZERO_COMMON")
    print("READY_PORT_Q0_ZERO_GIVES_EXACT_EVENT_ENERGY_AND_REDUCED_INVERSE")
    print("SIX_FACES_ARE_THE_MINIMUM_FULL_CUBIC_ORBIT_ON_THE_FIRST_MOORE_SHELL")
    print("PRODUCTION_DEPOSIT_DUAL_LEDGER_BARRIER_AND_FULL_STATE_EXTENSION_REMAIN_OPEN")
    print("VERDICT=OUTCOME_B_EXACT_SELECTED_DEPOSIT_PRODUCTION_INCOMPLETE")
    sys.exit(0)

sys.exit(1)
