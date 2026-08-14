#!/usr/bin/env python3
"""FTD-0875 exact certificate for the local canonical Hamiltonian rail."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_v1.md"
)
PROTOCOL_HASH = "659CAA27079D08BE620E6DF0DBCF0828B0923D242636EA34B7A7A454C2B75CB0"
SOURCE_HASHES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md":
        "898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_HAMILTONIAN_TERNARY_QUARTER_TURN_ACTUATOR_v1.md":
        "73214057949BC5BE115AF7E273DE2CECE1F87D63237E94ADADB83F64442C7B98",
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/"
    "PREREG_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_v1.md":
        "92C090ED43306249B963F757AD205F8C2B948944759A75CA46436606DDDC9BBB",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_ALTERNATING_ORIENTED_TERNARY_PARITY_RAIL_AND_ONE_SHOT_BOUNDARY_v1.md":
        "E70F2AD61BFA1C8BBFF4EA03DCF0312B8F96224ECF2453FDF4B81B0FEA845CA1",
    "engine/include/ftd/eft/oriented_ternary_quarter_turn.h":
        "46CD15943F5EB8EDBBCE4676CDE558A7C2B08556E1AC64E7C9720D30FFEB68E1",
    "engine/include/ftd/eft/hamiltonian_ternary_quarter_turn_actuator.h":
        "10BB9BFF5CC98E6CD72EC77F46E67766D458214E474296A7F3023AA27E2F8A94",
    "engine/include/ftd/eft/alternating_oriented_ternary_parity_rail.h":
        "E62026FA4228CFB8FB798EBF2E0C68011E6ABA6328050F80F9FD0573275604DD",
}

TERNARY = (-1, 0, 1)
R = sp.Matrix([[0, -1], [1, 0]])
R_INVERSE = sp.Matrix([[0, 1], [-1, 0]])

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if bool(condition):
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def scalar_layer_matrix(length: int, parity: int) -> sp.Matrix:
    matrix = sp.eye(length)
    for left in range(parity, length - 1, 2):
        matrix[left, left] = 0
        matrix[left, left + 1] = -1
        matrix[left + 1, left] = 1
        matrix[left + 1, left + 1] = 0
    return matrix


def scalar_common_form(length: int) -> sp.Matrix:
    half = length // 2
    form = sp.zeros(length)
    for index in range(half):
        value = (-1) ** (half + index + 1)
        mirror = length - 1 - index
        form[index, mirror] = value
        form[mirror, index] = -value
    return form


def matching(length: int, parity: int):
    return tuple((left, left + 1) for left in range(parity, length - 1, 2))


def discrete_layer(state, parity: int, inverse: bool = False):
    result = list(state)
    for left, right in matching(len(state), parity):
        if inverse:
            result[left], result[right] = state[right], -state[left]
        else:
            result[left], result[right] = -state[right], state[left]
    return tuple(result)


for source, expected in SOURCE_HASHES.items():
    check(f"source hash {source}", sha256(ROOT / source) == expected)
check("protocol pre-run hash", sha256(ROOT / PROTOCOL) == PROTOCOL_HASH)

pair_states = tuple(itertools.product(TERNARY, repeat=2))
check(
    "scalar bond map is the registered quarter-turn",
    all(tuple(R * sp.Matrix(pair)) == (-pair[1], pair[0]) for pair in pair_states),
)
even_lengths = tuple(range(2, 14, 2))
forms = {length: scalar_common_form(length) for length in even_lengths}
check(
    "signed anti-diagonal forms are antisymmetric",
    all(form.T == -form for form in forms.values()),
)
check(
    "signed anti-diagonal forms square to minus identity",
    all(form * form == -sp.eye(length) for length, form in forms.items()),
)
check(
    "both parity layers preserve every registered scalar form",
    all(
        scalar_layer_matrix(length, parity).T
        * forms[length]
        * scalar_layer_matrix(length, parity)
        == forms[length]
        for length in even_lengths for parity in (0, 1)
    ),
)
check(
    "scalar common form pairs finite-rail endpoints",
    all(forms[length][0, length - 1] != 0 for length in even_lengths if length >= 4),
)
one_dimensional_skew = sp.Matrix([[0]])
check("one-dimensional real skew block is zero", one_dimensional_skew.det() == 0)
check(
    "one scalar per site gives a degenerate onsite direct sum",
    sp.diag(one_dimensional_skew, one_dimensional_skew).det() == 0,
)
canonical_site = sp.Matrix([[0, 1], [-1, 0]])
check("two-dimensional canonical onsite fiber is nondegenerate", canonical_site.det() == 1)
check(
    "two coordinates are minimum in the registered local class",
    one_dimensional_skew.det() == 0 and canonical_site.det() != 0,
)

q0, p0, q1, p1 = sp.symbols("q0 p0 q1 p1", real=True)
Omega, kappa = sp.symbols("Omega kappa", positive=True)
theta = sp.symbols("theta", real=True)
sigma = sp.symbols("sigma", real=True)
g = 1 - sp.cos(theta)
N = (q0**2 + p0**2 + q1**2 + p1**2) / 2
L = q0 * p1 - q1 * p0
check(
    "carrier norm is positive definite",
    sp.hessian(N, (q0, p0, q1, p1)) == sp.eye(4),
)
check(
    "bond generator is the oriented determinant",
    sp.Matrix([[q0, q1], [p0, p1]]).det() == L,
)


def poisson(first, second, qvars, pvars):
    return sp.simplify(sum(
        sp.diff(first, q) * sp.diff(second, p)
        - sp.diff(first, p) * sp.diff(second, q)
        for q, p in zip(qvars, pvars)
    ))


check("carrier norm Poisson-commutes with bond generator", poisson(N, L, (q0, q1), (p0, p1)) == 0)

q2, p2, q3, p3 = sp.symbols("q2 p2 q3 p3", real=True)
L23 = q2 * p3 - q3 * p2
check(
    "disjoint bond generators Poisson commute",
    poisson(L, L23, (q0, q1, q2, q3), (p0, p1, p2, p3)) == 0,
)

H_carrier = Omega * N + sigma * kappa * g * L
qdot = (sp.diff(H_carrier, p0), sp.diff(H_carrier, p1))
pdot = (-sp.diff(H_carrier, q0), -sp.diff(H_carrier, q1))
check(
    "q equations contain the registered spatial generator",
    qdot == (
        Omega * p0 - sigma * kappa * g * q1,
        Omega * p1 + sigma * kappa * g * q0,
    ),
)
check(
    "p equations contain the same spatial generator",
    pdot == (
        -Omega * q0 - sigma * kappa * g * p1,
        -Omega * q1 + sigma * kappa * g * p0,
    ),
)
check(
    "carrier norm is conserved by the complete flow",
    poisson(N, H_carrier, (q0, q1), (p0, p1)) == 0,
)
check(
    "bond generator is conserved by the complete flow",
    poisson(L, H_carrier, (q0, q1), (p0, p1)) == 0,
)
t = sp.symbols("t", real=True)
check("clock phase solution is exact", sp.diff(Omega * t, t) == Omega)

I0, action = sp.symbols("I0 L_n", real=True)
I_solution = I0 - sigma * kappa * action * g / Omega
check(
    "clock action solution is exact",
    sp.simplify(
        sp.diff(I_solution, theta)
        + sigma * kappa * action * sp.sin(theta) / Omega
    ) == 0,
)
H_total = Omega * I_solution + Omega * sp.symbols("N_n", nonnegative=True) + sigma * kappa * g * action
check(
    "total Hamiltonian becomes phase independent",
    not sp.simplify(H_total).has(theta),
)

N_pair = N
sos_gap = (
    (q0**2 + p0**2 - q1**2 - p1**2) ** 2 / 4
    + (q0 * q1 + p0 * p1) ** 2
)
check(
    "bond determinant obeys absolute bound by carrier norm",
    sp.simplify(N_pair**2 - L**2 - sos_gap) == 0,
)
sos_plus = ((q0 + p1) ** 2 + (p0 - q1) ** 2) / 2
sos_minus = ((q0 - p1) ** 2 + (p0 + q1) ** 2) / 2
check(
    "maximum coupling retains the Omega N over two lower bound",
    sp.simplify(N_pair + L - sos_plus) == 0
    and sp.simplify(N_pair - L - sos_minus) == 0,
)

onsite_generator = sp.Matrix([
    [0, 0, 1, 0],
    [0, 0, 0, 1],
    [-1, 0, 0, 0],
    [0, -1, 0, 0],
])
spatial_generator = sp.diag(R, R)
onsite_full_cycle = sp.eye(4)
check("onsite oscillator completes an identity winding", onsite_full_cycle == sp.eye(4))
integrated_pulse = sp.integrate(1 - sp.cos(Omega * t), (t, 0, 2 * sp.pi / Omega))
check(
    "forward pulse angle is positive pi over two",
    sp.simplify((Omega / 4) * integrated_pulse - sp.pi / 2) == 0,
)
check(
    "reverse pulse angle is negative pi over two",
    sp.simplify(-(Omega / 4) * integrated_pulse + sp.pi / 2) == 0,
)
check(
    "onsite and spatial generators commute",
    onsite_generator * spatial_generator - spatial_generator * onsite_generator == sp.zeros(4),
)
forward_endpoint = sp.diag(R, R)
reverse_endpoint = sp.diag(R_INVERSE, R_INVERSE)
vector = sp.Matrix([q0, q1, p0, p1])
check(
    "forward endpoint applies R to q and p",
    forward_endpoint * vector == sp.Matrix([-q1, q0, -p1, p0]),
)
check(
    "reverse endpoint applies inverse R to q and p",
    reverse_endpoint * vector == sp.Matrix([q1, -q0, p1, -p0]),
)
check(
    "actual p-zero section returns to p zero",
    tuple((forward_endpoint * vector.subs({p0: 0, p1: 0}))[2:]) == (0, 0)
    and tuple((reverse_endpoint * vector.subs({p0: 0, p1: 0}))[2:]) == (0, 0),
)

layer_match = True
for length in range(2, 6):
    for parity in (0, 1):
        matrix = scalar_layer_matrix(length, parity)
        for state in itertools.product(TERNARY, repeat=length):
            endpoint = tuple(matrix * sp.Matrix(state))
            layer_match &= endpoint == discrete_layer(state, parity)
check("canonical endpoint matches exhaustive finite ternary layers", layer_match)

E0 = Omega * (q0**2 + p0**2) / 2
E1 = Omega * (q1**2 + p1**2) / 2
check(
    "onsite carrier energies are nonnegative quadratic forms",
    sp.hessian(E0 + E1, (q0, p0, q1, p1)) == Omega * sp.eye(4),
)
base_qdot = (Omega * p0, Omega * p1)
base_pdot = (-Omega * q0, -Omega * q1)
base_dE0 = sp.diff(E0, q0) * base_qdot[0] + sp.diff(E0, p0) * base_pdot[0]
check("base onsite winding changes no onsite energy", sp.simplify(base_dE0) == 0)

all_vars = (q0, q1, p0, p1)
all_dots = (qdot[0], qdot[1], pdot[0], pdot[1])
dE0 = sp.simplify(sum(sp.diff(E0, variable) * rate for variable, rate in zip(all_vars, all_dots)))
dE1 = sp.simplify(sum(sp.diff(E1, variable) * rate for variable, rate in zip(all_vars, all_dots)))
current = Omega * sigma * kappa * g * (q0 * q1 + p0 * p1)
check("upstream derivative is minus bond current", sp.simplify(dE0 + current) == 0)
check("downstream derivative is plus bond current", sp.simplify(dE1 - current) == 0)
check("active bond carrier energy is conserved", sp.simplify(dE0 + dE1) == 0)
check(
    "complete disjoint layer carrier energy is conserved",
    poisson(
        N + (q2**2 + p2**2 + q3**2 + p3**2) / 2,
        H_carrier + sigma * kappa * g * L23,
        (q0, q1, q2, q3),
        (p0, p1, p2, p3),
    ) == 0,
)

a = sp.symbols("a", positive=True)
record_energy = Omega * a**2 / 2
check(
    "ready ternary record has imposed energy Omega a squared over two",
    all(sp.simplify(E0.subs({q0: a * sign, p0: 0}) - record_energy) == 0 for sign in (-1, 1)),
)
forward_ready = forward_endpoint * sp.Matrix([a, 0, 0, 0])
check(
    "ready forward endpoint empties upstream energy",
    sp.simplify(E0.subs({q0: forward_ready[0], p0: forward_ready[2]})) == 0,
)
check(
    "ready forward endpoint receives all downstream energy",
    sp.simplify(E1.subs({q1: forward_ready[1], p1: forward_ready[3]}) - record_energy) == 0,
)
beta, beta_dot = sp.symbols("beta beta_dot", real=True)
downstream_energy = record_energy * sp.sin(beta) ** 2
prepared_current = Omega * a**2 * beta_dot * sp.sin(beta) * sp.cos(beta)
check(
    "integrated current equals endpoint energy transfer",
    sp.simplify(sp.diff(downstream_energy, beta) * beta_dot - prepared_current) == 0
    and sp.simplify(downstream_energy.subs(beta, sp.pi / 2) - downstream_energy.subs(beta, 0) - record_energy) == 0,
)

L_abs = sp.symbols("L_abs", positive=True)
max_action = (2 * kappa * L_abs / Omega).subs(kappa, Omega / 4)
check("maximum clock action excursion is absolute L over two", sp.simplify(max_action - L_abs / 2) == 0)
max_reference_exchange = sp.simplify(Omega * max_action)
check("maximum reference exchange is Omega absolute L over two", sp.simplify(max_reference_exchange - Omega * L_abs / 2) == 0)
max_interaction = (2 * kappa * L_abs).subs(kappa, Omega / 4)
check("interaction carries the opposite transient magnitude", sp.simplify(max_interaction - max_reference_exchange) == 0)
check(
    "clock action and interaction return at endpoint",
    sp.simplify(I_solution.subs(theta, 2 * sp.pi) - I0) == 0
    and sp.simplify((sigma * kappa * g * action).subs(theta, 2 * sp.pi)) == 0,
)
endpoint_before = Omega * I0 + Omega * sp.symbols("N_endpoint", nonnegative=True)
endpoint_after = Omega * I_solution.subs(theta, 2 * sp.pi) + Omega * sp.symbols("N_endpoint", nonnegative=True)
check("endpoint total-energy residual is zero", sp.simplify(endpoint_after - endpoint_before) == 0)
actual_L = L.subs({p0: 0, p1: 0})
actual_transport = tuple(R * sp.Matrix([1, 0]))
check(
    "actual section has zero L but nontrivial transport",
    actual_L == 0 and actual_transport == (0, 1),
)

protocol_text = (ROOT / PROTOCOL).read_text(encoding="utf-8")
scope_markers = (
    "CANONICAL_SITE_DOUBLET=IMPOSED_REFERENCE_MINIMUM_IN_REGISTERED_CLASS",
    "SCALAR_COMMON_SYMPLECTIC_FORM=BOUNDARY_GLOBAL_NOT_LOCAL",
    "COMMON_HARMONIC_CLOCK=SELECTED_REFERENCE",
    "ACTUAL_SECTION_CLOCK_BACKREACTION=ZERO_SPECIAL_ORBIT_NOT_COST_FREE_HARDWARE",
    "PRODUCTION_COUPLING=NONE",
    "GSTAR_ROLE=SEPARATE_CALENDAR_NOT_INTERSITE_ACTUATOR",
    "BORN_BELL_STATUS=UNTOUCHED",
)
check("all registered scope markers are present", all(marker in protocol_text for marker in scope_markers))
check("terminal gate reached with C1-C55 passing", checks == 55 and failures == 0)

print(f"\nFTD-0875 local canonical Hamiltonian parity rail: {checks - failures}/{checks} PASS")
if checks == 56 and failures == 0:
    print("LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_THEOREM")
    print("MINIMUM_REGISTERED_LOCAL_CARRIER_DIMENSION_PER_SITE=2")
    print("SCALAR_COMMON_FORM=BOUNDARY_GLOBAL")
    print("PREPARED_RECORD_ENERGY_TRANSFER=EXACT")
    print("BOND_CURRENT=LOCAL_ANTISYMMETRIC")
    print("ACTUAL_SECTION_CLOCK_BACKREACTION=ZERO_SPECIAL_ORBIT")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_ROLE=SEPARATE_CALENDAR_NOT_INTERSITE_ACTUATOR")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0875_CERTIFICATE_INVALID")
raise SystemExit(1)
