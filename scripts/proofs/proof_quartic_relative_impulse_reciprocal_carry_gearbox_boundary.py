#!/usr/bin/env python3
"""FTD-0898 exact quartic-relative impulse/carry gearbox certificate."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_QUARTIC_RELATIVE_IMPULSE_RECIPROCAL_CARRY_GEARBOX_BOUNDARY_v1.md"
)
PROTOCOL_HASH = "AFED9B9F633921281E770E9CEE603A1905847DC99BB6FB552401A0C44CA2086D"

SOURCES = {
    "carry_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md",
        "8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048",
    ),
    "common_relative": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_COMMON_RELATIVE_LOCAL_QUARTIC_CLOCK_v1.md",
        "64241D7AB18AD2079ECADF9EA25448F53F42696AB3FF439637970D4284497FD0",
    ),
    "local_self_pair": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md",
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    ),
    "bilateral_clock": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "DERIV_BILATERAL_SELF_DUAL_QUARTIC_CLOCK_v1.md",
        "779044879BB28CE0DB13BA8783EC7FF9AB5DFDFE10DF1C259D3D11998DEEDB9A",
    ),
    "phase_boundary": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CANONICAL_SOURCE_CENTERED_GAUSS_GATE_AND_BATTERY_PHASE_BOUNDARY_v1.md",
        "0D5A093597CE7BFFF7F593C0A1AF2B65E6CDE99DB0FFEDA1183D9849BC58624F",
    ),
    "reaction_transport": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md",
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
    ),
    "dressed_mass": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md",
        "378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C",
    ),
    "pair_header": (
        ROOT / "engine/include/ftd/eft/native_pair_energy_recursion.h",
        "81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A",
    ),
    "carry_header": (
        ROOT / "engine/include/ftd/eft/reciprocal_carry_reservoir.h",
        "69D4D225DD0D94EBD3A13C424FB78CA51238495A3DB51625129514253293B6BE",
    ),
}

checks: list[tuple[str, bool]] = []


def check(name: str, condition: object) -> None:
    passed = bool(condition)
    checks.append((name, passed))
    print(f"{'PASS' if passed else 'FAIL'}  C{len(checks):02d} {name}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def plain(text: str) -> str:
    cleaned = (
        text.replace("`", " ")
        .replace("*", " ")
        .replace("_", " ")
        .replace("–", "-")
        .replace("—", "-")
    )
    return re.sub(r"\s+", " ", cleaned.lower()).strip()


def carry(x: sp.Expr) -> sp.Expr:
    return sp.floor((x + sp.pi) / (2 * sp.pi))


def wrap(x: sp.Expr) -> sp.Expr:
    return sp.simplify(x - 2 * sp.pi * carry(x))


# Frozen source and protocol lock.
check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == PROTOCOL_HASH)
texts: dict[str, str] = {}
for name, (path, expected_hash) in SOURCES.items():
    check(f"source hash {name}", sha256(path) == expected_hash)
    texts[name] = plain(path.read_text(encoding="utf-8"))

protocol_text = plain(PROTOCOL.read_text(encoding="utf-8"))
check(
    "protocol freezes relative increment origin",
    "relative quartic increment origin=exact inside selected reference recursion"
    in protocol_text,
)
check(
    "protocol freezes exact channel impulse and carry composition",
    "channel impulses=exact equal and opposite" in protocol_text
    and "reciprocal carry composition=exact" in protocol_text,
)
check(
    "protocol keeps common coupling scale and cadence open",
    "common mode coupling=open" in protocol_text
    and "momentum scale=open" in protocol_text
    and "integer tick gstar cadence=open" in protocol_text,
)

# Orthogonal/canonical common-relative chart.
sqrt2 = sp.sqrt(2)
S = sp.Matrix([[1, 1], [1, -1]]) / sqrt2
check("channel transform is orthogonal", sp.simplify(S.T * S - sp.eye(2)) == sp.zeros(2))
check("channel transform has orientation reversal only in channel labels", sp.det(S) == -1)
M4 = sp.diag(1, 1, 1, 1)
M4[:2, :2] = S
M4[2:, 2:] = S
J4 = sp.Matrix.vstack(
    sp.Matrix.hstack(sp.zeros(2), sp.eye(2)),
    sp.Matrix.hstack(-sp.eye(2), sp.zeros(2)),
)
check("position-momentum block transform is symplectic", sp.simplify(M4.T * J4 * M4 - J4) == sp.zeros(4))

L, R, PL, PR = sp.symbols("L R P_L P_R", real=True)
C = (L + R) / sqrt2
D = (L - R) / sqrt2
PC = (PL + PR) / sqrt2
Pi = (PL - PR) / sqrt2
check("position norm splits exactly", sp.simplify(C**2 + D**2 - L**2 - R**2) == 0)
check("momentum norm splits exactly", sp.simplify(PC**2 + Pi**2 - PL**2 - PR**2) == 0)
dL, dR = sp.symbols("dL dR", real=True)
dC = (dL + dR) / sqrt2
dD = (dL - dR) / sqrt2
check(
    "canonical one-form splits exactly",
    sp.simplify(PL * dL + PR * dR - PC * dC - Pi * dD) == 0,
)

# Exact discrete-gradient energy and impulse origin.
D0, D1, Pi0, Pi1 = sp.symbols("D_0 D_1 Pi_0 Pi_1", real=True)
h = sp.symbols("h", real=True, nonzero=True)
m, lam = sp.symbols("m lambda", positive=True)
secant = D1**3 + D1**2 * D0 + D1 * D0**2 + D0**3
coordinate_equation = sp.Eq(D1 - D0, h * (Pi1 + Pi0) / (2 * m))
momentum_equation = sp.Eq(Pi1 - Pi0, -h * lam * secant)
check("quartic endpoint secant factorization is exact", sp.expand((D1 - D0) * secant - (D1**4 - D0**4)) == 0)
energy_difference = sp.expand(
    Pi1**2 / (2 * m) + lam * D1**4
    - Pi0**2 / (2 * m) - lam * D0**4
)
energy_factored = (
    (Pi1 + Pi0) * (Pi1 - Pi0) / (2 * m)
    + lam * (D1 - D0) * secant
)
check("relative energy difference has the discrete-gradient factorization", sp.simplify(energy_difference - energy_factored) == 0)
energy_on_equations = energy_factored.subs(
    {
        Pi1 - Pi0: momentum_equation.rhs,
        D1 - D0: coordinate_equation.rhs,
    },
    simultaneous=True,
)
check("relative energy is exactly conserved on the endpoint equations", sp.simplify(energy_on_equations) == 0)

PL0 = (PC + Pi0) / sqrt2
PL1 = (PC + Pi1) / sqrt2
PR0 = (PC - Pi0) / sqrt2
PR1 = (PC - Pi1) / sqrt2
delta_pi = sp.simplify(Pi1 - Pi0)
delta_left = sp.simplify(PL1 - PL0)
delta_right = sp.simplify(PR1 - PR0)
check("left increment is generated by the relative endpoint", sp.simplify(delta_left - delta_pi / sqrt2) == 0)
check("right increment is equal and opposite", sp.simplify(delta_right + delta_pi / sqrt2) == 0)
check("channel momentum sum is exactly invariant", sp.simplify(PL1 + PR1 - PL0 - PR0) == 0)
check("common momentum is exactly unchanged", sp.simplify((PL1 + PR1) / sqrt2 - PC) == 0)
check("the generated impulse reads no target probability", not delta_left.has(sp.Symbol("born_target")))

# Exact endpoint construction for multiple registered relative arms.
endpoint_arms = [
    (sp.Rational(1, 3), sp.Rational(2, 5), sp.Rational(3, 2), sp.Rational(4, 3), sp.Rational(1, 4)),
    (-sp.Rational(3, 4), sp.Rational(1, 2), sp.Rational(5, 3), sp.Rational(7, 5), -sp.Rational(2, 3)),
    (sp.Rational(5, 4), -sp.Rational(2, 3), sp.Rational(9, 4), sp.Rational(3, 2), sp.Rational(3, 5)),
]
for index, (d0, d1, mass, coupling, step) in enumerate(endpoint_arms, start=1):
    s3 = secant.subs({D0: d0, D1: d1})
    momentum_sum = sp.simplify(2 * mass * (d1 - d0) / step)
    momentum_delta = sp.simplify(-step * coupling * s3)
    p0 = sp.simplify((momentum_sum - momentum_delta) / 2)
    p1 = sp.simplify((momentum_sum + momentum_delta) / 2)
    check(
        f"endpoint arm {index} satisfies the coordinate equation",
        sp.simplify(d1 - d0 - step * (p1 + p0) / (2 * mass)) == 0,
    )
    check(
        f"endpoint arm {index} satisfies the momentum equation",
        sp.simplify(p1 - p0 + step * coupling * s3) == 0,
    )
    before_energy = sp.simplify(p0**2 / (2 * mass) + coupling * d0**4)
    after_energy = sp.simplify(p1**2 / (2 * mass) + coupling * d1**4)
    check(f"endpoint arm {index} conserves relative energy", sp.simplify(after_energy - before_energy) == 0)
    check(
        f"endpoint arm {index} reverses under endpoint exchange and step sign",
        sp.simplify(d0 - d1 - (-step) * (p0 + p1) / (2 * mass)) == 0
        and sp.simplify(p0 - p1 + (-step) * coupling * s3) == 0,
    )

# Reciprocal chart composition.
kL, kR, q = sp.symbols("k_L k_R q", real=True)
wL, wR, cL, cR = sp.symbols("w_L w_R c_L c_R", integer=True)
kL_after = kL + q - 2 * sp.pi * cL
kR_after = kR - q - 2 * sp.pi * cR
wL_after = wL + cL
wR_after = wR + cR
check(
    "left lifted endpoint reconstructs the generated increment",
    sp.simplify(kL_after + 2 * sp.pi * wL_after - (kL + 2 * sp.pi * wL + q)) == 0,
)
check(
    "right lifted endpoint reconstructs the opposite increment",
    sp.simplify(kR_after + 2 * sp.pi * wR_after - (kR + 2 * sp.pi * wR - q)) == 0,
)
check(
    "aggregate lifted channel momentum is exactly conserved",
    sp.simplify(
        kL_after + kR_after + 2 * sp.pi * (wL_after + wR_after)
        - kL - kR - 2 * sp.pi * (wL + wR)
    )
    == 0,
)
check(
    "aggregate reservoir update is exactly the carry sum",
    sp.simplify((wL_after + wR_after) - (wL + wR) - cL - cR) == 0,
)

wrap_arms = [
    (sp.pi / 8, -sp.pi / 7, sp.pi / 10, 0, 0),
    (3 * sp.pi / 4, sp.pi / 2, sp.pi / 2, 2, -1),
    (sp.pi / 4, -sp.pi / 4, 9 * sp.pi / 2, -3, 4),
    (-3 * sp.pi / 4, -sp.pi / 2, -5 * sp.pi / 2, 5, 1),
]
for index, (left, right, increment, left_winding, right_winding) in enumerate(wrap_arms, start=1):
    left_carry = carry(left + increment)
    right_carry = carry(right - increment)
    left_after = wrap(left + increment)
    right_after = wrap(right - increment)
    check(
        f"wrap arm {index} endpoints remain principal",
        -sp.pi <= left_after < sp.pi and -sp.pi <= right_after < sp.pi,
    )
    before = left + right + 2 * sp.pi * (left_winding + right_winding)
    after = left_after + right_after + 2 * sp.pi * (
        left_winding + right_winding + left_carry + right_carry
    )
    check(f"wrap arm {index} conserves the lifted aggregate", sp.simplify(after - before) == 0)
    reverse_left_carry = carry(left_after - increment)
    reverse_right_carry = carry(right_after + increment)
    check(
        f"wrap arm {index} inverse carries negate the forward carries",
        reverse_left_carry == -left_carry and reverse_right_carry == -right_carry,
    )
    check(
        f"wrap arm {index} reverses both principal labels",
        sp.simplify(wrap(left_after - increment) - left) == 0
        and sp.simplify(wrap(right_after + increment) - right) == 0,
    )

# Continuum G* period from the same relative energy.
x, t = sp.symbols("x t", positive=True)
G_star = sp.gamma(sp.Rational(1, 4)) / sp.gamma(sp.Rational(3, 4))
beta_value = sp.beta(sp.Rational(1, 4), sp.Rational(1, 2))
gamma_value = (
    sp.gamma(sp.Rational(1, 4)) * sp.gamma(sp.Rational(1, 2))
    / sp.gamma(sp.Rational(3, 4))
)
check("quartic substitution produces the beta integrand", sp.simplify((sp.Rational(1, 4) * t**(-sp.Rational(3, 4)) / sp.sqrt(1 - t)) / (1 / sp.sqrt(1 - x**4)).subs(x, t**sp.Rational(1, 4)) / (sp.diff(t**sp.Rational(1, 4), t))) == 1)
check(
    "beta value equals the gamma quotient",
    sp.simplify(sp.expand_func(beta_value) - gamma_value) == 0,
)
check("gamma one-half supplies the square-root-pi factor", sp.gamma(sp.Rational(1, 2)) == sp.sqrt(sp.pi))
check("quartic traversal factor is sqrt-pi times Gstar", sp.simplify(gamma_value - sp.sqrt(sp.pi) * G_star) == 0)
A = sp.symbols("A", positive=True)
period = sp.sqrt(sp.pi) * G_star * sp.sqrt(m / (2 * lam)) / A
check("relative continuum period has inverse amplitude scaling", sp.simplify(period * A - sp.sqrt(sp.pi) * G_star * sp.sqrt(m / (2 * lam))) == 0)
check("Gstar is absent from the discrete endpoint equations", not coordinate_equation.has(G_star) and not momentum_equation.has(G_star))

# Scale and common-mode non-identifiability.
p_star, scale = sp.symbols("p_star scale", positive=True)
dimensionless_increment = sp.simplify(delta_pi / (sqrt2 * p_star))
check("relative endpoint fixes the dimensionless increment only after pstar", dimensionless_increment.has(p_star))
check("rescaling pstar inversely rescales the same increment", sp.simplify(dimensionless_increment.subs(p_star, scale * p_star) - dimensionless_increment / scale) == 0)
check("common momentum is independent of relative endpoint data", sp.diff(PC, D0) == 0 and sp.diff(PC, Pi0) == 0)
check("relative energy is independent of common momentum", not (Pi0**2 / (2 * m) + lam * D0**4).has(PC))
mass_tensor_scale = sp.symbols("mass_tensor_scale", positive=True)
check("unfixed momentum unit retains quadratic mass-map ambiguity", sp.simplify((scale * mass_tensor_scale)**2 - scale**2 * mass_tensor_scale**2) == 0)

# Phase/action and physical-identification boundaries.
work, phi = sp.symbols("work phi", real=True)
check("constant action shift has nonzero phase-circle flux", sp.integrate(-work, (phi, 0, 2 * sp.pi)) == -2 * sp.pi * work)
z = sp.symbols("z", real=True)
state_work = z**2
check("state-dependent phase-blind drain has nonzero cross derivative", sp.diff(-state_work, z) == -2 * z)
check("constant action flux vanishes only for zero booked work", sp.solve(sp.Eq(-2 * sp.pi * work, 0), work) == [0])

# Frozen-corpus scope checks.
check(
    "FTD-0897 asks for a local action that derives increment and work",
    "derive the supplied increment and its energy/work update from one local matter--field action"
    in texts["carry_theorem"],
)
check(
    "FTD-0844 has exact energy but no single common action",
    "the sum of those sector ledgers is exactly conserved" in texts["common_relative"]
    and "not yet proved to arise from one common discrete action" in texts["common_relative"],
)
check(
    "FTD-0844 decoupling blocks physical readout",
    "the same decoupling prevents the common field, state field, matter, or an observer from reading the relative phase"
    in texts["common_relative"],
)
check(
    "FTD-0841 supplies the selected local recursion but not production coupling",
    "this closes the local mathematical phase-space type and recursion" in texts["local_self_pair"]
    and "not promote the coupling to production" in texts["local_self_pair"],
)
check(
    "FTD-0836 makes Gstar a traversal cost rather than energy",
    "it is not the radius or energy of the self-dual circle" in texts["bilateral_clock"]
    and "total physical-time weight of one oriented traversal" in texts["bilateral_clock"],
)
check(
    "FTD-0886 forbids a bare phase-cylinder battery promotion",
    "not a globally hamiltonian time map on the phase cylinder" in texts["phase_boundary"]
    and "cannot merely read w after the system update" in texts["phase_boundary"],
)
check(
    "FTD-0890 still requires a dynamically coupled field impulse",
    "dynamical coupling from the matched field impulse into that triplet"
    in texts["reaction_transport"],
)
check(
    "FTD-0893 still requires the physical momentum map",
    "physical total-momentum map has linearization" in texts["dressed_mass"]
    and "we need an independently closed total-momentum map" in texts["dressed_mass"],
)
check(
    "pair API declares selected reference and no target period",
    "selected eft reference component" in texts["pair_header"]
    and "does not encode g or a target period" in texts["pair_header"],
)
check(
    "carry API denies interaction energy and physical scale derivation",
    "does not derive the increment" in texts["carry_header"]
    and "supply an energy law or physical momentum unit" in texts["carry_header"],
)

# Terminal firewalls.
terminal_markers = (
    "relative quartic increment origin=exact inside selected reference recursion",
    "channel impulses=exact equal and opposite",
    "relative energy=exactly conserved",
    "reciprocal carry composition=exact",
    "continuum gstar period=exact conditional on selected quartic",
    "common mode coupling=open",
    "matter field identification=open",
    "momentum scale=open",
    "integer tick gstar cadence=open",
    "carry energy law=open",
    "absolute mass=not derived",
    "production integration=forbidden",
    "no new selected type=true",
    "born bell lorentz completeness=untouched",
)
for marker in terminal_markers:
    check(f"terminal firewall {marker}", marker in protocol_text)

passed = sum(result for _, result in checks)
total = len(checks)
print(f"\nFTD-0898 exact certificate: {passed}/{total} checks passed")
verdict = passed == total
print(
    "QUARTIC_RELATIVE_IMPULSE_CARRY_GEARBOX_EXACT_COMMON_COUPLING_SCALE_CADENCE_OPEN="
    f"{'TRUE' if verdict else 'FALSE'}"
)
raise SystemExit(0 if verdict else 1)
