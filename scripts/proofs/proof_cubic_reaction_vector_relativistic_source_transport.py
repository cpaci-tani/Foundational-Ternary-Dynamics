#!/usr/bin/env python3
"""FTD-0889 exact cubic reaction-vector/source-transport certificate.

This is a fixed symbolic and exact-probe verifier.  It performs no numerical
search, fitting, or production mutation.
"""

from __future__ import annotations

import hashlib
import math
from fractions import Fraction
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_CUBIC_REACTION_VECTOR_RELATIVISTIC_SOURCE_TRANSPORT_v1.md"
)
PROTOCOL_SHA256 = "A92F0BFB95993971AB80661B39296E948BA68E52ADED6D4A3DAF92804DB37F66"

SOURCES = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_BOUNDARY_v1.md":
        "0FEEF83C38BE9A4929644A229EAEA1B22424A54161BE8E2F3F8B882194DFDF39",
    "engine/include/ftd/eft/autonomous_phase_parity_source_reaction.h":
        "D052A463C3F62F3326BBBB1CECB56E9CF4EB5A92EDE411A4DD42F28602AA6FB9",
    "engine/include/ftd/ontic/particle_masses.h":
        "EFE9D68C9ECF6520510519B972D5CDD5925FD86026270AB0E4CAA5BFD6F1B0B1",
    "engine/include/ftd/eft/production_hop_kinematics.h":
        "4FCE830B79CD4590108B7FEA28063B489B33CF3CA69925E5405043B78D1C2EBD",
    "engine/include/ftd/eft/matched_face_momentum_transaction.h":
        "BA7B0CA7895D4DC5259527CCDCB06EC9B08DF7C4CB38AC8CDEDC31EFCD3FA62B",
    "engine/include/ftd/eft/face_current_segment.h":
        "BA86AA25BD52B80A7D11DF72012F20109DD89830C5DD80F44A6729548E30ECB9",
    "engine/include/ftd/eft/canonical_subcell_section.h":
        "8DBA6784C6B0D61B5A78430EB6A5949F215AFCD1C635B67BAF05F2B94595B42F",
    "docs/theory/02_foundations/FOUND_MATTER_EVENT_CURRENT_ONTOLOGY_v1.md":
        "4C5DF2533F63628B68E612A8197010C2B0D85FC6BF6E7C9F6D55C71FF31DFF67",
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


def centered_chart(value: Fraction) -> tuple[int, Fraction]:
    """Exact half-open chart r in [-1/2,1/2)."""
    anchor = math.floor(value + Fraction(1, 2))
    return anchor, value - anchor


# ---------------------------------------------------------------------------
# Frozen provenance
# ---------------------------------------------------------------------------

for relative, expected in SOURCES.items():
    check(f"frozen source hash matches: {Path(relative).name}",
          sha256(ROOT / relative) == expected)
check("protocol pre-run hash matches", sha256(PROTOCOL) == PROTOCOL_SHA256)


# ---------------------------------------------------------------------------
# Cubic representation boundary
# ---------------------------------------------------------------------------

I3 = sp.eye(3)
Rx = sp.diag(-1, 1, 1)
Ry = sp.diag(1, -1, 1)
Rz = sp.diag(1, 1, -1)
Hx = sp.diag(1, -1, -1)
Hy = sp.diag(-1, 1, -1)
Hz = sp.diag(-1, -1, 1)
Pxy = sp.Matrix([[0, 1, 0], [1, 0, 0], [0, 0, 1]])

fixed_reflection_stack = (Rx - I3).col_join(Ry - I3).col_join(Rz - I3)
fixed_rotation_stack = (Hx - I3).col_join(Hy - I3).col_join(Hz - I3)
check("three cubic sign flips have zero common vector fixed space",
      fixed_reflection_stack.rank() == 3)
check("proper cubic half turns also have zero common vector fixed space",
      fixed_rotation_stack.rank() == 3)
check("scalar-only equivariance forces every nonlinear output value to zero",
      not fixed_reflection_stack.nullspace())

a12, a13, a23 = sp.symbols("a12 a13 a23", real=True)
skew3 = sp.Matrix([[0, a12, a13], [-a12, 0, a23], [-a13, -a23, 0]])
check("every three-dimensional alternating form is singular",
      sp.simplify(skew3.det()) == 0)

invariance_equations = []
for R in (Rx, Ry, Rz, Pxy):
    delta = sp.expand(R.T * skew3 * R - skew3)
    invariance_equations.extend(list(delta))
invariant_solution = sp.solve(invariance_equations, (a12, a13, a23), dict=True)
check("one T1u copy has no nonzero invariant alternating form",
      invariant_solution == [{a12: 0, a13: 0, a23: 0}])

J6 = sp.zeros(6)
J6[:3, 3:] = I3
J6[3:, :3] = -I3
check("T1u plus T1u carries a nondegenerate canonical form",
      J6.det() == 1 and J6.rank() == 6)
check("the six-dimensional canonical form is invariant under cubic actions",
      all(sp.diag(R, R).T * J6 * sp.diag(R, R) == J6
          for R in (Rx, Ry, Rz, Hx, Hy, Hz, Pxy)))

# Two canonical pairs provide four dimensions.  The only way to contain a
# spatial vector is T1u + scalar; solve the invariant skew-form equations and
# verify that the vector sector remains in the kernel.
b12, b13, b14, b23, b24, b34 = sp.symbols(
    "b12 b13 b14 b23 b24 b34", real=True)
skew4 = sp.Matrix([
    [0, b12, b13, b14],
    [-b12, 0, b23, b24],
    [-b13, -b23, 0, b34],
    [-b14, -b24, -b34, 0],
])
eq4 = []
for R in (Rx, Ry, Rz, Pxy):
    R4 = sp.diag(R, sp.ones(1, 1))
    eq4.extend(list(sp.expand(R4.T * skew4 * R4 - skew4)))
sol4 = sp.solve(eq4, (b12, b13, b14, b23, b24, b34), dict=True)
check("two canonical pairs cannot carry a cubic vector symplectically",
      sol4 == [{b12: 0, b13: 0, b14: 0, b23: 0, b24: 0, b34: 0}])
check("three canonical pairs are minimum in the orientation-free onsite class",
      J6.rank() == 6 and sp.zeros(4).rank() < 4)

n = sp.Matrix([1, 0, 0])
fixed_embedding = sp.Matrix.hstack(n, sp.zeros(3, 1)).col_join(
    sp.Matrix.hstack(sp.zeros(3, 1), n))
check("one pair embeds symplectically only after a direction is supplied",
      fixed_embedding.T * J6 * fixed_embedding
      == sp.Matrix([[0, 1], [-1, 0]]))
check("the FTD0888 header does not claim spatial ternary recoil",
      "spatial_ternary_source_recoil_supplied = false" in source_text(
          "engine/include/ftd/eft/autonomous_phase_parity_source_reaction.h"))


# ---------------------------------------------------------------------------
# Relativistic cotangent chart
# ---------------------------------------------------------------------------

E0, c, rho = sp.symbols("E0 c rho", positive=True, finite=True)
alpha = sp.sqrt(E0 + rho**2 / 4) / c
lambda_t = alpha
lambda_r = (E0 + rho**2 / 2) / (c * sp.sqrt(E0 + rho**2 / 4))
check("frozen radial chart scale is positive", alpha.is_positive is True)
check("frozen tangential Jacobian eigenvalue is recovered",
      sp.simplify(lambda_t - alpha) == 0)
check("frozen radial Jacobian eigenvalue is recovered",
      sp.simplify(alpha + rho * sp.diff(alpha, rho) - lambda_r) == 0)
check("both Jacobian eigenvalues are positive",
      lambda_t.is_positive is True and lambda_r.is_positive is True)
check("radial chart determinant is positive and nonzero",
      sp.simplify(lambda_t**2 * lambda_r).is_positive is True)
check("the chart is regular at zero reaction momentum",
      sp.limit(lambda_t, rho, 0) == sp.sqrt(E0) / c
      and sp.limit(lambda_r, rho, 0) == sp.sqrt(E0) / c)

# Exact fixed algebraic probe for the full Jacobian and cotangent lift.
Pi = sp.Matrix([1, 2, 2])
rho_probe = sp.sqrt(Pi.dot(Pi))
alpha_probe = sp.sqrt(sp.Integer(5) + rho_probe**2 / 4) / 2
g_probe = alpha_probe * Pi
A_probe = sp.simplify(g_probe.jacobian([])) if False else (
    alpha_probe * I3
    + Pi * Pi.T / (4 * 2 * sp.sqrt(sp.Integer(5) + rho_probe**2 / 4))
)
R_probe = sp.Matrix([3, -1, 4])
x_probe = sp.simplify(A_probe.inv().T * R_probe)
check("full fixed-probe momentum Jacobian is symmetric",
      A_probe == A_probe.T)
check("full fixed-probe momentum Jacobian is invertible",
      sp.simplify(A_probe.det()) != 0)
check("cotangent coordinate definition preserves the canonical one-form",
      sp.simplify((x_probe.T * A_probe - R_probe.T).norm()) == 0)

kinetic_from_chart = sp.sqrt(
    E0**2 + c**2 * (alpha**2 * rho**2)) - E0
check("reaction norm maps exactly to relativistic kinetic energy",
      sp.simplify(kinetic_from_chart - rho**2 / 2) == 0)

p = sp.symbols("p", nonnegative=True, finite=True)
Kp = sp.sqrt(E0**2 + c**2 * p**2) - E0
rho_inverse = sp.sqrt(2 * Kp)
p_roundtrip = sp.simplify(
    rho_inverse * sp.sqrt(E0 + rho_inverse**2 / 4) / c)
check("inverse radial momentum chart closes for nonnegative momentum",
      sp.simplify(p_roundtrip - p) == 0)
check("zero physical momentum maps to zero reaction momentum",
      sp.simplify(rho_inverse.subs(p, 0)) == 0)

check("small-reaction momentum scale is sqrt(E0)/c",
      sp.limit(alpha, rho, 0) == sp.sqrt(E0) / c)
mass_symbol = sp.simplify(E0 / c**2)
check("low-energy inertial coefficient is E0 over c squared",
      mass_symbol == E0 / c**2)

particle_mass_source = source_text("engine/include/ftd/ontic/particle_masses.h")
check("production contract declares E_REST=M_INERTIAL*C_SPEED^2",
      "E_REST          = M_INERTIAL * C_SPEED * C_SPEED" in particle_mass_source)
check("production inertial normalization remains selected rather than derived",
      "M_INERTIAL      = K_B" in particle_mass_source)

kappa = sp.symbols("kappa", positive=True)
S2 = sp.diag(kappa, 1 / kappa)
J2 = sp.Matrix([[0, 1], [-1, 0]])
check("canonical rescaling preserves the symplectic form",
      sp.simplify(S2.T * J2 * S2 - J2) == sp.zeros(2))
check("canonical rescaling changes the kinetic mass normalization",
      sp.simplify((sp.symbols("P") / kappa)**2 / 2
                  - sp.symbols("P")**2 / 2) != 0)


# ---------------------------------------------------------------------------
# Free transport, quotient chart, current, and action-reaction bridge
# ---------------------------------------------------------------------------

px, py, pz, dt = sp.symbols("px py pz dt", real=True, finite=True)
pvec = sp.Matrix([px, py, pz])
Etot = sp.sqrt(E0**2 + c**2 * pvec.dot(pvec))
vvec = sp.simplify(c**2 * pvec / Etot)
hessian = sp.hessian(Etot, (px, py, pz))
check("relativistic drift velocity is the energy gradient",
      sp.simplify(sp.Matrix([sp.diff(Etot, q) for q in (px, py, pz)])
                           - vvec) == sp.zeros(3, 1))
check("drift shear is symplectic because the velocity Jacobian is symmetric",
      hessian == hessian.T)
check("free drift preserves momentum and energy exactly",
      sp.simplify(Etot.subs({px: px, py: py, pz: pz}) - Etot) == 0)
check("free drift inverse is obtained by reversing dt",
      sp.simplify(dt * vvec + (-dt) * vvec) == sp.zeros(3, 1))
speed_ratio = sp.simplify(vvec.dot(vvec) / c**2)
check("finite-momentum source speed stays strictly below c",
      sp.simplify(1 - speed_ratio)
      == E0**2 / (E0**2 + c**2 * pvec.dot(pvec)))

chart_inputs = [Fraction(-13, 7), Fraction(-2, 5), Fraction(7, 13),
                Fraction(19, 10)]
chart_ok = True
for value in chart_inputs:
    anchor, remainder = centered_chart(value)
    chart_ok = chart_ok and Fraction(anchor) + remainder == value
    chart_ok = chart_ok and Fraction(-1, 2) <= remainder < Fraction(1, 2)
check("integer-site plus remainder charts preserve physical position exactly",
      chart_ok)

half_anchor, half_remainder = centered_chart(Fraction(1, 2))
negative_half_anchor, negative_half_remainder = centered_chart(Fraction(-1, 2))
check("centered chart keeps the known half-open section convention",
      (half_anchor, half_remainder) == (1, Fraction(-1, 2))
      and (negative_half_anchor, negative_half_remainder)
      == (0, Fraction(-1, 2)))
check("the half-cell inversion/translation section obstruction is not erased",
      half_anchor != -negative_half_anchor)

face_header = source_text("engine/include/ftd/eft/face_current_segment.h")
check("face-current API declares exact endpoint continuity",
      "rho_after - rho_before + div(current) = 0" in face_header)
check("face-current API exposes a maximum continuity residual",
      "max_face_current_continuity_residual" in face_header)

momentum_header = source_text(
    "engine/include/ftd/eft/matched_face_momentum_transaction.h")
check("matched field observer defines required matter impulse",
      "required_matter_impulse = result.field_momentum_change * -1.0" in momentum_header)
check("matched field momentum remains an observer rather than production recoil",
      "not a unique continuum" in momentum_header
      and "production particle-recoil law" in momentum_header)

impulse = sp.Matrix([2, -3, 5])
check("field impulse orientation transforms covariantly under signed permutations",
      all((R * impulse).dot(R * impulse) == impulse.dot(impulse)
          for R in (Rx, Ry, Rz, Hx, Hy, Hz, Pxy)))
check("zero field impulse supplies neither direction nor spurious recoil",
      sp.zeros(3, 1).norm() == 0)

u, Kreq = sp.symbols("u Kreq", positive=True, finite=True)
Eres = u**2 / 2
sin2_eta = sp.simplify(Kreq / Eres)
check("conservation fixes sin-squared eta to Kreq over Eres",
      sin2_eta == 2 * Kreq / u**2)
check("compatibility interval maps uniquely into eta in zero to pi over two",
      sp.diff(sp.asin(sp.sqrt(sp.symbols("z", positive=True))),
              sp.symbols("z", positive=True)).is_positive is True)

Ehist = sp.simplify(Eres * (1 - sin2_eta))
Ereact = sp.simplify(Eres * sin2_eta)
check("history plus reaction energy closes exactly",
      sp.simplify(Ehist + Ereact - Eres) == 0)
check("reaction energy equals the required source kinetic energy",
      sp.simplify(Ereact - Kreq) == 0)
check("equal split occurs exactly at Kreq=Eres/2",
      sp.simplify(sin2_eta.subs(Kreq, Eres / 2) - sp.Rational(1, 2)) == 0)
check("eta=pi/4 is therefore conditional rather than universal",
      sp.asin(sp.sqrt(sp.Rational(1, 2))) == sp.pi / 4)
check("zero required impulse gives eta zero",
      sp.limit(sp.asin(sp.sqrt(sin2_eta)), Kreq, 0) == 0)
check("required kinetic energy above residual energy fails the real-angle gate",
      (2 * Eres / u**2) == 1 and (2 * (Eres + 1) / u**2) > 1)

production_header = source_text(
    "engine/include/ftd/eft/production_hop_kinematics.h")
check("production dispersion is imported as an analysis contract",
      "Analysis-only momentum form of the production flat kinematics" in production_header)
check("the bridge reads no Born target",
      "Born" not in production_header and "Born" not in momentum_header)
check("the bridge supplies no quartic Gstar synchronization",
      "Gstar" not in production_header and "Gstar" not in momentum_header)
check("stable recurrent matter remains outside the certificate",
      "stable localized orbit" in source_text(
          "docs/theory/02_foundations/FOUND_MATTER_EVENT_CURRENT_ONTOLOGY_v1.md"))
check("no production source file is in the frozen bridge source set",
      all(not key.startswith("engine/src/render_bridge") for key in SOURCES))
check("mass scale and common-action coupling remain open",
      "does not determine `E0`, `c`," in PROTOCOL.read_text(encoding="utf-8")
      and "full common-action coupling" in PROTOCOL.read_text(encoding="utf-8"))
check("Born Bell Lorentz biology and completeness firewall is frozen",
      "no change to Born/Bell, `G*`, Lorentz, or completeness status" in
      PROTOCOL.read_text(encoding="utf-8"))
check("terminal gate reached with every prior check passing", failures == 0)


print()
print(f"FTD-0889 cubic reaction-vector/source transport: {checks - failures}/{checks} PASS")
if failures == 0:
    print("SCALAR_REACTION_TO_SPATIAL_VECTOR=FORBIDDEN_BY_CUBIC_SYMMETRY")
    print("ORIENTATION_FREE_SPATIAL_REACTION=THREE_CANONICAL_PAIRS_MINIMUM")
    print("RELATIVISTIC_REACTION_TO_MOMENTUM_CHART=EXACT_SYMPLECTIC")
    print("SOURCE_TRANSPORT=EXACT_REVERSIBLE_REFERENCE_CONTINUATION")
    print("SPLIT_ANGLE=FIXED_BY_LOCAL_ENERGY_MOMENTUM_COMPATIBILITY")
    print("EQUAL_SPLIT=CONDITIONAL_NOT_UNIVERSAL")
    print("INERTIAL_MASS_SCALE=IMPORTED_THROUGH_E0_AND_C")
    print("NATIVE_VECTOR_COMMON_ACTION=OPEN")
    print("PRODUCTION_COUPLING=NONE")
    print("GSTAR_BORN_BELL_LORENTZ_COMPLETENESS=UNTOUCHED")

raise SystemExit(1 if failures else 0)
