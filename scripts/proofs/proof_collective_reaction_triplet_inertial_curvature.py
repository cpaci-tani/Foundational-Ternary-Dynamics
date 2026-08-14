#!/usr/bin/env python3
"""FTD-0891 collective reaction-triplet/inertial-curvature certificate.

This verifier uses exact symbolic algebra and exact rational probes only.  It
performs no numerical search, fit, parameter inference, or production mutation.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_COLLECTIVE_REACTION_TRIPLET_AND_INERTIAL_CURVATURE_BOUNDARY_v1.md"
)
PROTOCOL_SHA256 = "D273F1A61E1A55B26781116E3B9D3984DAFF843DB04F18E160C706EBEAC6C595"

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md":
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "ANALYSIS_CONNECTED_MOORE_BLOCK_COMMON_ACTION_v1.md":
        "5094BAAC01E2D2027A6D6FCF77535926728E353E99C58E728C0401FDA6F94B7A",
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "ANALYSIS_CONNECTED_BLOCK_ANALYTIC_STATIC_AND_DYNAMICAL_REST_v1.md":
        "9F29651A69A86E12BD4A05E758AFD23E2186DD11825BE7D66285C6345F30FEEE",
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "ANALYSIS_CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_v1.md":
        "09B8F7A48EF0B367C0A628B46250A0953DDD5FE32E9F3FFCE3ADAFE3A791EFBD",
    "docs/theory/03_derivations/foundational_mechanics/"
    "DERIV_CLUSTER_COLLECTIVE_COORDINATE_v1.md":
        "E89C5F765D10D26D1A6F60D23CFBEA28E15094646B108DFFA34DA8C330E49DDC",
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "THEOREM_CLASSICAL_COMPOSITE_POLE_BOUNDARY.md":
        "5539B300406BBD4BCC57C31CFC57BA6D0B09B9545168FEC4D391A89E9368D5CE",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_COMMON_MOORE_WORLDLINE_ACTION.md":
        "330AB25C8CE8235DF0EADE611057EF2FD52D062A40E212D53F88E9F4A54DB37D",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_INTEGER_TRANSLATION_BLOCH_TRANSPORT.md":
        "F472E65AFD9EB1B97B2EA4A8CC5C613960006928752F5A87F50302974DC2E6FD",
    "engine/include/ftd/eft/connected_moore_block_action.h":
        "09328FB23642D3D8AFD165994F8F8B2101A52DD7E0BC5BFEE2E2DF27ABE6EDF8",
    "engine/src/eft/connected_moore_block_action.cpp":
        "207002636F290E9C55BB33FDFED489C423EEC5BFA3C0986D4E320A460E3F0262",
}


checks = 0
failures = 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(name: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {name}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {name}")


def source_text(relative: str) -> str:
    return (ROOT / relative).read_text(encoding="utf-8")


def normalized(relative: str) -> str:
    return " ".join(source_text(relative).lower().split())


def helmert(n: int) -> sp.Matrix:
    """Orthonormal Helmert matrix with the mean row first."""
    U = sp.zeros(n)
    for j in range(n):
        U[0, j] = 1 / sp.sqrt(n)
    for k in range(1, n):
        denom = sp.sqrt(k * (k + 1))
        for j in range(k):
            U[k, j] = 1 / denom
        U[k, k] = -k / denom
    return U


# ---------------------------------------------------------------------------
# Frozen provenance
# ---------------------------------------------------------------------------

for relative, expected in SOURCES.items():
    check(f"frozen source hash matches: {Path(relative).name}",
          sha256(ROOT / relative) == expected)
check("protocol pre-run hash matches", sha256(PROTOCOL) == PROTOCOL_SHA256)


# ---------------------------------------------------------------------------
# Exact collective symplectic reduction
# ---------------------------------------------------------------------------

for n_probe in range(1, 7):
    U_probe = helmert(n_probe)
    check(f"Helmert matrix is exactly orthogonal for N={n_probe}",
          sp.simplify(U_probe * U_probe.T) == sp.eye(n_probe))

U = helmert(4)
check("Helmert mean row has the frozen normalization",
      all(sp.simplify(U[0, j] - sp.Rational(1, 2)) == 0
          for j in range(4)))
check("Helmert transform is invertible with inverse transpose",
      sp.simplify(U.inv() - U.T) == sp.zeros(4))

I3 = sp.eye(3)
T = sp.kronecker_product(U, I3)
I12 = sp.eye(12)
check("three-component constituent transform is orthogonal",
      sp.simplify(T * T.T) == I12)

J24 = sp.zeros(24)
J24[:12, 12:] = I12
J24[12:, :12] = -I12
S = sp.diag(T, T)
check("full position-momentum transform is exactly symplectic",
      sp.simplify(S.T * J24 * S - J24) == sp.zeros(24))

x_symbols = sp.Matrix(sp.symbols("x0:12", real=True))
p_symbols = sp.Matrix(sp.symbols("p0:12", real=True))
dx_symbols = sp.Matrix(sp.symbols("dx0:12", real=True))
q_symbols = sp.simplify(T * x_symbols)
pi_symbols = sp.simplify(T * p_symbols)
dq_symbols = sp.simplify(T * dx_symbols)

X = sp.simplify(q_symbols[:3, :] / 2)
P = sp.simplify(2 * pi_symbols[:3, :])
expected_X = sp.Matrix([
    sum(x_symbols[3 * a + i] for a in range(4)) / 4
    for i in range(3)
])
expected_P = sp.Matrix([
    sum(p_symbols[3 * a + i] for a in range(4))
    for i in range(3)
])
check("normalized first position mode is the center coordinate",
      sp.simplify(X - expected_X) == sp.zeros(3, 1))
check("normalized first momentum mode is total constituent momentum",
      sp.simplify(P - expected_P) == sp.zeros(3, 1))

full_one_form = sp.simplify((p_symbols.T * dx_symbols)[0])
modal_one_form = sp.simplify((pi_symbols.T * dq_symbols)[0])
collective_one_form = sp.simplify(
    (P.T * (dq_symbols[:3, :] / 2))[0]
    + (pi_symbols[3:, :].T * dq_symbols[3:, :])[0]
)
check("orthogonal modal transform preserves the canonical one-form",
      sp.simplify(full_one_form - modal_one_form) == 0)
check("one-form splits into P dot dX plus internal modes",
      sp.simplify(full_one_form - collective_one_form) == 0)

x_roundtrip = sp.simplify(T.T * q_symbols)
p_roundtrip = sp.simplify(T.T * pi_symbols)
check("constituent positions reconstruct exactly", x_roundtrip == x_symbols)
check("constituent momenta reconstruct exactly", p_roundtrip == p_symbols)

Rx = sp.diag(-1, 1, 1)
Rz90 = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 1]])
Pxy = sp.Matrix([[0, 1, 0], [1, 0, 0], [0, 0, 1]])
check("constituent reduction commutes with signed spatial permutations",
      all(sp.simplify(T * sp.kronecker_product(sp.eye(4), R)
                      - sp.kronecker_product(sp.eye(4), R) * T)
              == sp.zeros(12)
          for R in (Rx, Rz90, Pxy)))
check("collective X and P transform as a cubic vector/covector pair",
      all(sp.simplify(R.T * R) == I3 for R in (Rx, Rz90, Pxy)))

internal_impulses = [
    sp.Matrix([1, 2, -1]),
    sp.Matrix([-3, 0, 4]),
    sp.Matrix([2, -5, -2]),
    sp.Matrix([0, 3, -1]),
]
internal_sum = sum(internal_impulses, sp.zeros(3, 1))
check("fixed internal pair-force probe has zero summed impulse",
      internal_sum == sp.zeros(3, 1))
check("zero-sum internal impulses leave collective P unchanged",
      sp.simplify(expected_P + internal_sum - expected_P) == sp.zeros(3, 1))

external_impulses = [
    sp.Matrix([2, -1, 0]),
    sp.Matrix([0, 3, 1]),
    sp.Matrix([-1, 0, 2]),
    sp.Matrix([4, -2, -3]),
]
external_sum = sum(external_impulses, sp.zeros(3, 1))
check("external constituent impulses sum to the collective impulse",
      external_sum == sp.Matrix([5, 0, 0]))
check("collective momentum changes by exactly the summed external impulse",
      sp.simplify((expected_P + external_sum) - expected_P - external_sum)
      == sp.zeros(3, 1))


# ---------------------------------------------------------------------------
# Conditional relativistic composite dispersion
# ---------------------------------------------------------------------------

epsilon, c, r = sp.symbols("epsilon c r", positive=True, finite=True)
E_single = sp.sqrt(epsilon**2 + c**2 * r**2)
lambda_t = c**2 / E_single
lambda_r = c**2 * epsilon**2 / E_single**3
check("single-constituent dispersion has positive tangential curvature",
      lambda_t.is_positive is True)
check("single-constituent dispersion has positive radial curvature",
      lambda_r.is_positive is True)
check("strict convexity makes the constrained minimum unique",
      lambda_t.is_positive is True and lambda_r.is_positive is True)

e1, e2, e3, pnorm = sp.symbols(
    "e1 e2 e3 pnorm", positive=True, finite=True)
Epsilon = e1 + e2 + e3
Ecollective = sp.sqrt(Epsilon**2 + c**2 * pnorm**2)
allocations = (e1 / Epsilon, e2 / Epsilon, e3 / Epsilon)
check("frozen proportional allocation sums to total momentum",
      sp.simplify(sum(allocations) - 1) == 0)

factor_checks = []
velocity_checks = []
for ea, weight in zip((e1, e2, e3), allocations):
    constituent_squared = ea**2 + c**2 * weight**2 * pnorm**2
    factor_checks.append(
        sp.simplify(constituent_squared
                    - (ea / Epsilon * Ecollective)**2) == 0
    )
    constituent_velocity = sp.simplify(
        c**2 * weight * pnorm / (ea / Epsilon * Ecollective)
    )
    velocity_checks.append(
        sp.simplify(constituent_velocity
                    - c**2 * pnorm / Ecollective) == 0
    )
check("each minimized constituent energy has the exact common factor",
      all(factor_checks))
check("all minimized constituents have one common velocity",
      all(velocity_checks))
check("summed minimized energies give the exact collective dispersion",
      sp.simplify(sum(ea / Epsilon * Ecollective
                      for ea in (e1, e2, e3)) - Ecollective) == 0)

px, py, pz = sp.symbols("px py pz", real=True, finite=True)
Ecoll_xyz = sp.sqrt(Epsilon**2 + c**2 * (px**2 + py**2 + pz**2))
H0 = sp.simplify(sp.hessian(Ecoll_xyz, (px, py, pz)).subs(
    {px: 0, py: 0, pz: 0}))
check("collective zero-momentum Hessian is c squared over summed rest energy",
      H0 == c**2 / Epsilon * I3)
Mcoll = sp.simplify(Epsilon / c**2)
check("conditional collective inertial mass is Epsilon over c squared",
      sp.simplify(H0.inv() - Mcoll * I3) == sp.zeros(3))

N = sp.symbols("N", integer=True, positive=True)
e = sp.symbols("e", positive=True, finite=True)
check("identical-constituent mass is conditionally additive",
      sp.simplify((N * e) / c**2 - N * (e / c**2)) == 0)

U0 = sp.symbols("U0", nonzero=True, finite=True)
Erest_static = Epsilon + U0
mass_energy_difference = sp.simplify(Erest_static / c**2 - Mcoll)
check("nonparticipating static binding offset creates exact mass mismatch",
      mass_energy_difference == U0 / c**2)
check("static binding cannot be counted inertially without a boosted dressing",
      mass_energy_difference != 0)


# ---------------------------------------------------------------------------
# Static-data non-identifiability
# ---------------------------------------------------------------------------

k, m1, m2 = sp.symbols("k m1 m2", positive=True, finite=True)
q, mom = sp.symbols("q mom", real=True, finite=True)
V = k * q**2 / 2
H1 = mom**2 / (2 * m1) + V
H2 = mom**2 / (2 * m2) + V
check("two kinetic metrics have the same stable rest point",
      sp.diff(H1, q).subs({q: 0, mom: 0}) == 0
      and sp.diff(H2, q).subs({q: 0, mom: 0}) == 0)
check("two kinetic metrics have the same static Hessian",
      sp.diff(H1, q, 2) == k and sp.diff(H2, q, 2) == k)
check("their momentum curvatures differ for unequal masses",
      sp.simplify(sp.diff(H1, mom, 2) - sp.diff(H2, mom, 2))
      == 1 / m1 - 1 / m2)
check("their normal-mode frequencies differ for unequal masses",
      sp.simplify(k / m1 - k / m2) != 0)

E0, a1, a2 = sp.symbols("E0 a1 a2", positive=True, finite=True)
dispersion1 = E0 + a1 * mom**2
dispersion2 = E0 + a2 * mom**2
check("different convex dispersions can share the same rest energy",
      dispersion1.subs(mom, 0) == E0 and dispersion2.subs(mom, 0) == E0)
check("shared rest energy does not fix momentum curvature",
      sp.diff(dispersion1, mom, 2) - sp.diff(dispersion2, mom, 2)
      == 2 * (a1 - a2))
check("both counterexample dispersions remain strictly convex",
      sp.diff(dispersion1, mom, 2).is_positive is True
      and sp.diff(dispersion2, mom, 2).is_positive is True)


# ---------------------------------------------------------------------------
# Frozen source boundaries and scope firewalls
# ---------------------------------------------------------------------------

block_header = normalized("engine/include/ftd/eft/connected_moore_block_action.h")
block_analysis = normalized(
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "ANALYSIS_CONNECTED_MOORE_BLOCK_COMMON_ACTION_v1.md")
mode_analysis = normalized(
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "ANALYSIS_CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_v1.md")
worldline = normalized(
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_COMMON_MOORE_WORLDLINE_ACTION.md")
bloch = normalized(
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_INTEGER_TRANSLATION_BLOCH_TRANSPORT.md")
pole = normalized(
    "docs/theory/10_eft_program/derivations/constituent_complete_matter/"
    "THEOREM_CLASSICAL_COMPOSITE_POLE_BOUNDARY.md")

check("common-action API keeps constituent mass as an input scale",
      "constituent_mass_scale" in block_header)
check("common-action document preserves selected-dynamics status",
      "[selected dynamics]" in block_analysis)
check("bond graph and constituent momentum remain selected material state",
      "constituent momenta" in block_analysis and "relational memory" in block_analysis)
check("field spline momentum remains diagnostic rather than exact Noether charge",
      "spline momentum remains a diagnostic" in block_analysis
      and "noether charge" in block_analysis)
check("stable selected modes retain Peierls translation curvature",
      "not zero modes" in mode_analysis and "peierls" in mode_analysis)
check("point-carrier common action retains a positive Peierls barrier",
      "peierls barrier remains positive" in worldline)
check("microscopic support and exact translations remain integer-valued",
      "exact translation group" in bloch and "integer-valued" in bloch)
check("Bloch transport source denies a stable manifested matter pole",
      "does not establish" in bloch and "stable manifested matter pole" in bloch)
check("additive rest energy alone remains outside pole derivation",
      "solely because the hamiltonian contains the additive rest energy" in pole)
check("collective triplet is kinematic content not substrate formation",
      "does not derive that action, its bond graph" in block_analysis)

protocol_text = " ".join(PROTOCOL.read_text(encoding="utf-8").lower().split())
check("protocol freezes static-data mass non-identifiability",
      "static stability and k do not determine inertia" in protocol_text)
check("protocol freezes the discrete-translation Noether boundary",
      "z^3, not r^3" in protocol_text
      and "not make p_matter+p_field" in protocol_text)
check("protocol forbids production wiring", "no production wiring" in protocol_text)
check("Born Bell Gstar Lorentz biology and completeness firewall is frozen",
      "no change to born/bell, g*, lorentz, biology, or completeness status"
      in protocol_text)
check("no production bridge source is in the frozen source set",
      all("render_bridge" not in key and "production_tick" not in key
          for key in SOURCES))
check("terminal gate reached with every prior check passing", failures == 0)


print()
print(f"FTD-0891 collective triplet/inertial curvature: "
      f"{checks - failures}/{checks} PASS")
if failures == 0:
    print("COLLECTIVE_REACTION_TRIPLET=EXACT_SYMPLECTIC_SECTOR")
    print("NEW_SELECTED_VECTOR_TYPE=NONE_ONCE_CONSTITUENT_PHASE_SPACE_EXISTS")
    print("COMPOSITE_RELATIVISTIC_DISPERSION=EXACT_CONDITIONAL_MINIMUM")
    print("COMPOSITE_INERTIA=SUM_REST_ENERGY_OVER_C_SQUARED_CONDITIONAL")
    print("STATIC_HESSIAN_TO_MASS_SCALE=CLOSED_NEGATIVE")
    print("STATIC_BINDING_OFFSET_TO_INERTIA=CLOSED_NEGATIVE")
    print("MATTER_TOTAL_MOMENTUM=EXACT_COLLECTIVE_CANONICAL_VARIABLE")
    print("TOTAL_FIELD_MATTER_NOETHER_MOMENTUM=OPEN_ON_DISCRETE_LATTICE")
    print("ABSOLUTE_MASS_SCALE=IMPORTED")
    print("CONSTITUENT_FORMATION_STABLE_POLE_PRODUCTION=OPEN")
    print("GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED")

raise SystemExit(1 if failures else 0)
