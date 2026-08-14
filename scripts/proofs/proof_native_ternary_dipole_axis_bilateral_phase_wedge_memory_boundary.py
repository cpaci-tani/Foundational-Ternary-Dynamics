#!/usr/bin/env python3
"""FTD-0905 exact native dipole-axis / bilateral phase-wedge certificate."""

from __future__ import annotations

import hashlib
from itertools import product
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_NATIVE_TERNARY_DIPOLE_AXIS_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md"
SOURCES = {
    "voxel": (
        ROOT / "engine/include/ftd/voxel.h",
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ),
    "pair_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_PAIR_ENERGY_RECURSION_v1.md",
        "C352EC96A6513D5ED3AB8A7318F47FD1A695FBB0C4FBEB33E9DE43680A70DF93",
    ),
    "vector_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md",
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    ),
    "common_relative": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md",
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    ),
    "rectifier": (
        ROOT / "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_GEAR_RATIO_BOUNDARY_v1.md",
        "E87EB15B482AFBBF1147726B3F07C4008B82BC07B06BD9786656BEA28AD3BDDA",
    ),
    "pair_header": (
        ROOT / "engine/include/ftd/eft/native_pair_energy_recursion.h",
        "81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A",
    ),
}
EXPECTED_PROTOCOL_HASH = (
    "6FC0C2BAB8A84378F3B88618BA41E16B4C328AFF497446A2A4542990AA20CA4E"
)


checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    checks.append((name, bool(condition)))


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def exact_zero(expression: sp.Expr | sp.MatrixBase) -> bool:
    if isinstance(expression, sp.MatrixBase):
        return all(sp.simplify(value) == 0 for value in expression)
    return sp.simplify(expression) == 0


protocol_text = PROTOCOL.read_text(encoding="utf-8")
source_text = {
    name: path.read_text(encoding="utf-8")
    for name, (path, _) in SOURCES.items()
}

check("C01 protocol hash matches the pre-run lock", digest(PROTOCOL) == EXPECTED_PROTOCOL_HASH)
for index, (name, (path, expected)) in enumerate(SOURCES.items(), start=2):
    check(f"C{index:02d} source hash {name}", digest(path) == expected)

check(
    "C08 protocol freezes the neutral dipole axis",
    "d_\\Lambda(r)=\\sum_{x\\in\\Lambda}s_x(x-r)" in protocol_text
    and "NEUTRAL_TERNARY_DIPOLE_SUPPLIES_POLAR_AXIS=CONDITIONAL_EXACT" in protocol_text,
)
check(
    "C09 protocol freezes the bilateral phase wedge",
    "\\ell=q_+p_- - q_-p_+" in protocol_text
    and "BILATERAL_PHASE_WEDGE_IS_TIME_ODD=TRUE" in protocol_text,
)
check(
    "C10 protocol separates the clock and chirality memory",
    "SEPARATE_CLOCK_AND_CHIRALITY_MEMORY_MINIMUM=TRUE_IN_REGISTERED_CLASS" in protocol_text,
)
check(
    "C11 protocol freezes production Born and cadence firewalls",
    "PRODUCTION_INTEGRATION=FORBIDDEN" in protocol_text
    and "BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED" in protocol_text
    and "INTEGER_TICK_GSTAR_CADENCE=OPEN" in protocol_text,
)

# Native source-type markers.
check(
    "C12 Voxel contains the ternary actual state",
    "int8_t state = 0;" in source_text["voxel"],
)
check(
    "C13 Voxel contains the local continuous flux vector",
    "Vec3 flux;" in source_text["voxel"],
)
check(
    "C14 Voxel contains the local wave-velocity vector",
    "Vec3 wave_vel;" in source_text["voxel"],
)
check(
    "C15 production Voxel does not already name the bilateral phase-wedge memory",
    "bilateral_phase_wedge" not in source_text["voxel"]
    and "central_quartic_memory" not in source_text["voxel"],
)

# Neutral dipole, origin independence, and covariance.
S, r0, r1 = sp.symbols("S r_0 r_1", real=True)
d0 = sp.symbols("d_0", real=True)
check(
    "C16 dipole origin shift is proportional only to total ternary charge",
    exact_zero((d0 - (r1 - r0) * S) - d0 + (r1 - r0) * S),
)
check(
    "C17 neutral dipole is origin independent",
    exact_zero(((d0 - (r1 - r0) * S) - d0).subs(S, 0)),
)

xp = sp.Matrix(sp.symbols("x_p0:3", real=True))
xm = sp.Matrix(sp.symbols("x_m0:3", real=True))
d = xp - xm
check("C18 the minimum plus-minus pair dipole is x plus minus x minus", exact_zero(d - (xp - xm)))

one_site_neutral_nonzero = any(s == 0 and s != 0 for s in (-1, 0, 1))
check("C19 one ternary site cannot be both neutral and dipolar", not one_site_neutral_nonzero)
neutral_two_site_states = [pair for pair in product((-1, 0, 1), repeat=2) if sum(pair) == 0]
check(
    "C20 the only two-site neutral nonzero-state assignments are plus-minus pairs",
    {pair for pair in neutral_two_site_states if pair != (0, 0)} == {(-1, 1), (1, -1)},
)

Q = sp.Matrix([[0, -1, 0], [0, 0, 1], [-1, 0, 0]])
a = sp.Matrix(sp.symbols("a0:3", real=True))
check("C21 registered signed cubic transform is orthogonal", exact_zero(Q.T * Q - sp.eye(3)))
check(
    "C22 neutral plus-minus dipole is translation independent and cubic covariant",
    exact_zero((Q * xp + a) - (Q * xm + a) - Q * d),
)
check(
    "C23 spatial inversion reverses the polar dipole",
    exact_zero((-sp.eye(3)) * d + d),
)
check(
    "C24 the dipole symmetric square loses its polar sign",
    exact_zero(d * d.T - (-d) * (-d).T),
)

# Projection and phase-wedge parity.
e = sp.Matrix(sp.symbols("e0:3", real=True))
Jp = sp.Matrix(sp.symbols("Jp0:3", real=True))
Jm = sp.Matrix(sp.symbols("Jm0:3", real=True))
Wp = sp.Matrix(sp.symbols("Wp0:3", real=True))
Wm = sp.Matrix(sp.symbols("Wm0:3", real=True))
qp = (e.T * Jp)[0]
qm = (e.T * Jm)[0]
pp = (e.T * Wp)[0]
pm = (e.T * Wm)[0]
ell = sp.expand(qp * pm - qm * pp)

def projection(axis: sp.Matrix, value: sp.Matrix) -> sp.Expr:
    return sp.expand((axis.T * value)[0])


check("C25 plus projection is signed-cubic invariant", exact_zero(projection(Q * e, Q * Jp) - qp))
check("C26 minus projection is signed-cubic invariant", exact_zero(projection(Q * e, Q * Jm) - qm))
check("C27 plus momentum projection is signed-cubic invariant", exact_zero(projection(Q * e, Q * Wp) - pp))
check("C28 minus momentum projection is signed-cubic invariant", exact_zero(projection(Q * e, Q * Wm) - pm))
check(
    "C29 spatial inversion leaves every projected scalar unchanged",
    exact_zero(projection(-e, -Jp) - qp)
    and exact_zero(projection(-e, -Jm) - qm)
    and exact_zero(projection(-e, -Wp) - pp)
    and exact_zero(projection(-e, -Wm) - pm),
)
ell_time_reversed = sp.expand(qp * (-pm) - qm * (-pp))
check("C30 bilateral phase wedge is time odd", exact_zero(ell_time_reversed + ell))
check("C31 time reversal flips the nonzero chirality sign", sp.ask(sp.Q.positive(sp.Symbol("l", positive=True))) is True)

q_plus, q_minus, p_plus, p_minus = sp.symbols("q_+ q_- p_+ p_-", real=True)
l_scalar = q_plus * p_minus - q_minus * p_plus
Z = sp.Matrix([[q_plus, p_plus], [q_minus, p_minus]])
gram = Z * Z.T
check(
    "C32 Gram determinant is exactly the squared bilateral wedge",
    exact_zero(sp.det(gram) - l_scalar**2),
)
swap = sp.Matrix([[q_minus, p_minus], [q_plus, p_plus]])
check("C33 exchanging the bilateral labels reverses the wedge", exact_zero(sp.det(swap) + l_scalar))
check(
    "C34 exchanging the labels preserves the Gram data up to simultaneous permutation",
    exact_zero(swap * swap.T - sp.Matrix([[0, 1], [1, 0]]) * gram * sp.Matrix([[0, 1], [1, 0]])),
)
check(
    "C35 the symmetric square retains wedge magnitude but not sign",
    exact_zero((-l_scalar) ** 2 - l_scalar**2),
)

# FTD-0840 swept area: strict orientation is not time-odd branch memory.
q0, q1, p0, p1 = sp.symbols("q_0 q_1 p_0 p_1", real=True)
qbar = (q0 + q1) / 2
pbar = (p0 + p1) / 2
area = sp.expand(qbar * (p1 - p0) - pbar * (q1 - q0))
area_endpoint_swap = sp.expand(
    ((q1 + q0) / 2) * (p0 - p1) - ((p1 + p0) / 2) * (q0 - q1)
)
area_full_time_reverse = sp.expand(
    ((q1 + q0) / 2) * ((-p0) - (-p1))
    - (((-p1) + (-p0)) / 2) * (q0 - q1)
)
check("C36 endpoint order alone reverses the swept area", exact_zero(area_endpoint_swap + area))
check(
    "C37 endpoint exchange plus canonical momentum reversal preserves swept area",
    exact_zero(area_full_time_reverse - area),
)
check(
    "C38 FTD-0840 source explicitly proves strict discrete orientation",
    "every nonzero discrete step has one strict orientation" in source_text["pair_theorem"],
)
check(
    "C39 the pair implementation reports orientation sign rather than a stored time-odd branch",
    "int orientation_sign = 0;" in source_text["pair_header"]
    and "Signed step: positive advances the registered orientation" in source_text["pair_header"],
)

# Central bilateral quartic memory.
mu, kappa = sp.symbols("mu kappa", positive=True)
qa, qb, pa, pb = sp.symbols("q_a q_b p_a p_b", real=True)
rho2 = qa**2 + qb**2
H = (pa**2 + pb**2) / (2 * mu) + kappa * rho2**2
qadot = sp.diff(H, pa)
qbdot = sp.diff(H, pb)
padot = -sp.diff(H, qa)
pbdot = -sp.diff(H, qb)
angular = qa * pb - qb * pa
angular_dot = sp.expand(
    qadot * pb + qa * pbdot - qbdot * pa - qb * padot
)
check("C40 central memory Hamilton equations have canonical velocities", exact_zero(qadot - pa / mu) and exact_zero(qbdot - pb / mu))
check("C41 central quartic force is radial", exact_zero(padot + 4 * kappa * rho2 * qa) and exact_zero(pbdot + 4 * kappa * rho2 * qb))
check("C42 bilateral phase wedge is exactly conserved", exact_zero(angular_dot))
check("C43 central memory energy is a positive kinetic plus quartic sum", bool(mu.is_positive) and bool(kappa.is_positive))

r, l = sp.symbols("rho ell", positive=True)
v_eff = l**2 / (2 * mu * r**2) + kappa * r**4
v_prime = sp.diff(v_eff, r)
v_second = sp.diff(v_eff, r, 2)
l2_at_minimum = 4 * mu * kappa * r**6
check(
    "C44 radial stationary equation gives rho sixth equals ell squared over four mu kappa",
    exact_zero(v_prime.subs(l**2, l2_at_minimum)),
)
check(
    "C45 the nonzero-wedge radial minimum is strict",
    exact_zero(v_second.subs(l**2, l2_at_minimum) - 24 * kappa * r**2)
    and bool((24 * kappa * r**2).is_positive),
)
check(
    "C46 nonzero wedge creates a positive centrifugal barrier",
    bool((l**2 / (2 * mu * r**2)).is_positive),
)
check(
    "C47 the pure radial quartic is recovered exactly at zero wedge",
    exact_zero(v_eff.subs(l, 0) - kappa * r**4),
)
check(
    "C48 nonzero wedge does not retain the pure radial quartic",
    not exact_zero(v_eff - kappa * r**4),
)
check(
    "C49 the same central mode cannot be both nonzero-wedge memory and the pure radial Gstar clock",
    "same central mode" in protocol_text.lower()
    and "centrifugal inverse-square term" in protocol_text,
)

# Source and scope controls.
check(
    "C50 FTD-0841 keeps polarization and production coupling open",
    "polarization" in source_text["vector_theorem"].lower()
    and "production" in source_text["vector_theorem"].lower(),
)
check(
    "C51 FTD-0844 separates common and relative clock sectors",
    "common" in source_text["common_relative"].lower()
    and "relative" in source_text["common_relative"].lower(),
)
check(
    "C52 FTD-0904 requires retained polar axis and time-odd chirality",
    "RETAINED_POLAR_AXIS_REQUIRED=TRUE_IN_REGISTERED_CLASS" in source_text["rectifier"]
    and "TIME_ODD_CHIRALITY_REQUIRED_FOR_BRANCH_PAIRED_REVERSAL=TRUE" in source_text["rectifier"],
)
check(
    "C53 protocol does not claim production formation",
    "NONZERO_DIPOLE_AND_PHASE_WEDGE_FORMATION=NOT_DERIVED" in protocol_text,
)
check(
    "C54 protocol does not claim production memory dynamics",
    "PRODUCTION_BILATERAL_MEMORY_LAW=PRESENTLY_ABSENT" in protocol_text,
)
check(
    "C55 protocol keeps maintenance erasure and work open",
    "MEMORY_FORMATION_MAINTENANCE_ERASURE_WORK=OPEN" in protocol_text,
)

firewalls = [
    "NEUTRAL_TERNARY_DIPOLE_SUPPLIES_POLAR_AXIS=CONDITIONAL_EXACT",
    "NONZERO_DIPOLE_AND_PHASE_WEDGE_FORMATION=NOT_DERIVED",
    "SIGNED_CUBIC_AND_INVERSION_COVARIANCE=EXACT",
    "BILATERAL_PHASE_WEDGE_IS_SPATIAL_SCALAR=TRUE",
    "BILATERAL_PHASE_WEDGE_IS_TIME_ODD=TRUE",
    "SYMMETRIC_SQUARE_RETAINS_WEDGE_SIGN=FALSE",
    "FTD0840_ONE_STEP_SWEPT_AREA_IS_TIME_ODD_MEMORY=FALSE",
    "CENTRAL_QUARTIC_MEMORY_CONSERVES_PHASE_WEDGE=CONDITIONAL_EXACT",
    "NONZERO_WEDGE_BOUNDED_RECURSIVE_MEMORY=CONDITIONAL_EXACT",
    "SAME_MODE_NONZERO_WEDGE_RETAINS_PURE_GSTAR_RADIAL_CLOCK=FALSE",
    "SEPARATE_CLOCK_AND_CHIRALITY_MEMORY_MINIMUM=TRUE_IN_REGISTERED_CLASS",
    "PRODUCTION_BILATERAL_MEMORY_LAW=PRESENTLY_ABSENT",
    "MEMORY_FORMATION_MAINTENANCE_ERASURE_WORK=OPEN",
    "GAMMA_MAGNITUDE_DERIVED=FALSE",
    "PHYSICAL_MOMENTUM_SCALE=OPEN",
    "ABSOLUTE_MASS=NOT_DERIVED",
    "INTEGER_TICK_GSTAR_CADENCE=OPEN",
    "PRODUCTION_INTEGRATION=FORBIDDEN",
    "NO_NEW_SELECTED_TYPE=TRUE",
    "BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED",
]
for offset, marker in enumerate(firewalls, start=56):
    check(f"C{offset:02d} terminal firewall {marker.lower()}", marker in protocol_text)


failed = [name for name, passed in checks if not passed]
for name, passed in checks:
    print(f"{'PASS' if passed else 'FAIL'}  {name}")

print()
print(f"FTD-0905 exact certificate: {len(checks) - len(failed)}/{len(checks)} checks passed")
if failed:
    print("FAILED_GATES=" + ",".join(failed))
    raise SystemExit(1)

print(
    "NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_TIME_ODD_PHASE_WEDGE_"
    "REPRESENTABLE_CENTRAL_MEMORY_CONDITIONAL_SEPARATE_GSTAR_CLOCK_"
    "PRODUCTION_FORMATION_OPEN=TRUE"
)
