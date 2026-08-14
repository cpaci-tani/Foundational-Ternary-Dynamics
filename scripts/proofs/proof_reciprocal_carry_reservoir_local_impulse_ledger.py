#!/usr/bin/env python3
"""FTD-0897 exact reciprocal-carry reservoir certificate."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_RECIPROCAL_CARRY_RESERVOIR_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md"
)
PROTOCOL_HASH = "A6775AD78DA96BB606871EB6C924148CB45498DE1097EB955BD99057587B3E97"

SOURCES = {
    "bloch_lift_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md",
        "0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973",
    ),
    "dressed_mass_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md",
        "378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C",
    ),
    "face_balance": (
        ROOT / "docs/theory/07_assessment/"
        "common_action_mechanics_reciprocity/"
        "AUDIT_EXACT_MOMENTUM_FACE_BALANCE.md",
        "72364E30BC10216661E64FAC67B13810EE1CEB2903AF7C2A408337EA16615AAF",
    ),
    "hard_contact": (
        ROOT / "docs/theory/07_assessment/"
        "framework_boundary_imports_consumption/"
        "AUDIT_HARD_CONTACT_CORNER_ACTION.md",
        "6495C6B6C055BA41DB56C577E932A5BDEEB3F5E814053BE2B327FE686992BB71",
    ),
    "reaction_transport": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md",
        "56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27",
    ),
    "phase_rail": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_LOCAL_CANONICAL_HAMILTONIAN_PARITY_RAIL_AND_SCALAR_LOCALITY_BOUNDARY_v1.md",
        "982C3B9D00798920A1BDAB96C75EBC9DB3A08111E8900F1D630382B0249B25F6",
    ),
    "bloch_header": (
        ROOT / "engine/include/ftd/eft/bloch_quasimomentum_lift.h",
        "69FEB5EC624AB3FDD685325273354885B502535930BB2A6D73E84EC10E60EED8",
    ),
    "face_header": (
        ROOT / "engine/include/ftd/eft/momentum_face_balance.h",
        "B9F435FF75E7EE133A9393294E45B1C316E026472A0C93FCF457077BDE6A6567",
    ),
    "reaction_header": (
        ROOT / "engine/include/ftd/eft/cubic_reaction_vector_source_transport.h",
        "3835884B7CA8EBD949AA758BFA245B3784BE4FCBAC8E8FADCFC6C69A6AD99ADC",
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
    cleaned = text.replace("`", " ").replace("*", " ").replace("_", " ")
    return re.sub(r"\s+", " ", cleaned.lower()).strip()


def carry(x: sp.Expr) -> sp.Expr:
    return sp.floor((x + sp.pi) / (2 * sp.pi))


def wrap(x: sp.Expr) -> sp.Expr:
    return sp.simplify(x - 2 * sp.pi * carry(x))


def pair_step(k1: sp.Expr, k2: sp.Expr, reservoir: sp.Expr, q: sp.Expr):
    c1 = carry(k1 + q)
    c2 = carry(k2 - q)
    k1_after = wrap(k1 + q)
    k2_after = wrap(k2 - q)
    reservoir_after = sp.simplify(reservoir + c1 + c2)
    return k1_after, k2_after, reservoir_after, c1, c2


# Frozen source and protocol lock.
check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == PROTOCOL_HASH)
texts: dict[str, str] = {}
for name, (path, expected_hash) in SOURCES.items():
    check(f"source hash {name}", sha256(path) == expected_hash)
    texts[name] = plain(path.read_text(encoding="utf-8"))

protocol_text = plain(PROTOCOL.read_text(encoding="utf-8"))
check(
    "protocol freezes conditional exact carry closure",
    "reciprocal carry update=exact conditional on supplied opposite increment"
    in protocol_text,
)
check(
    "protocol freezes unique reservoir increment and conditional reversal",
    "reciprocal reservoir increment=unique given branch and conservation"
    in protocol_text
    and "full state reversal=exact if increment reversibly available"
    in protocol_text,
)
check(
    "protocol leaves origin energy scale and physical map open",
    "interaction increment origin=open" in protocol_text
    and "reservoir energy law=open" in protocol_text
    and "physical momentum scale=open" in protocol_text
    and "total field matter momentum map=open" in protocol_text,
)

# Exact wrap/carry decomposition and branch controls.
angles = [
    -17 * sp.pi / 3,
    -5 * sp.pi,
    -sp.pi,
    -7 * sp.pi / 8,
    0,
    9 * sp.pi / 10,
    sp.pi,
    23 * sp.pi / 4,
]
check(
    "all registered angles wrap into the principal branch",
    all(-sp.pi <= wrap(angle) < sp.pi for angle in angles),
)
check(
    "wrap plus reciprocal carry reconstructs every angle",
    all(
        sp.simplify(wrap(angle) + 2 * sp.pi * carry(angle) - angle) == 0
        for angle in angles
    ),
)
check("positive pi wraps to negative pi with carry one", wrap(sp.pi) == -sp.pi and carry(sp.pi) == 1)
check("negative pi belongs to the principal branch", wrap(-sp.pi) == -sp.pi and carry(-sp.pi) == 0)
check("multiple positive zones produce an integer carry", carry(23 * sp.pi / 4) == 3)
check("multiple negative zones produce an integer carry", carry(-17 * sp.pi / 3) == -3)
for angle, winding in ((sp.pi / 7, 4), (-5 * sp.pi / 9, -3), (sp.pi, 2)):
    check(
        f"wrap is reciprocal-periodic for angle {angle} winding {winding}",
        sp.simplify(wrap(angle + 2 * sp.pi * winding) - wrap(angle)) == 0,
    )
    check(
        f"carry shifts by the reciprocal winding for angle {angle} winding {winding}",
        carry(angle + 2 * sp.pi * winding) == carry(angle) + winding,
    )

# One-axis local pair transactions.
arms = [
    (sp.pi / 8, -sp.pi / 7, 2, sp.pi / 10),
    (3 * sp.pi / 4, sp.pi / 2, -2, sp.pi / 2),
    (-3 * sp.pi / 4, -sp.pi / 2, 5, -sp.pi / 2),
    (sp.pi / 4, -sp.pi / 4, -3, 9 * sp.pi / 2),
    (3 * sp.pi / 4, -2 * sp.pi / 3, 7, sp.pi / 4),
]
for index, (k1, k2, reservoir, q) in enumerate(arms, start=1):
    k1_after, k2_after, reservoir_after, c1, c2 = pair_step(
        k1, k2, reservoir, q
    )
    before = sp.simplify(k1 + k2 + 2 * sp.pi * reservoir)
    after = sp.simplify(k1_after + k2_after + 2 * sp.pi * reservoir_after)
    check(f"arm {index} labels remain principal", -sp.pi <= k1_after < sp.pi and -sp.pi <= k2_after < sp.pi)
    check(f"arm {index} real aggregate is exactly conserved", sp.simplify(after - before) == 0)
    check(
        f"arm {index} reservoir change equals the two carries",
        sp.simplify(reservoir_after - reservoir - c1 - c2) == 0,
    )
    inverse = pair_step(k1_after, k2_after, reservoir_after, -q)
    check(
        f"arm {index} inverse carries negate the forward carries",
        inverse[3] == -c1 and inverse[4] == -c2,
    )
    check(
        f"arm {index} full-state reversal is exact",
        sp.simplify(inverse[0] - k1) == 0
        and sp.simplify(inverse[1] - k2) == 0
        and sp.simplify(inverse[2] - reservoir) == 0,
    )

# Uniqueness of the integer reservoir update.
d, c1_symbol, c2_symbol = sp.symbols("d c1_symbol c2_symbol", integer=True)
k1_symbol, k2_symbol, q_symbol, reservoir_symbol = sp.symbols(
    "k1_symbol k2_symbol q_symbol reservoir_symbol", real=True
)
k1_after_symbol = k1_symbol + q_symbol - 2 * sp.pi * c1_symbol
k2_after_symbol = k2_symbol - q_symbol - 2 * sp.pi * c2_symbol
conservation_residual = sp.expand(
    k1_after_symbol + k2_after_symbol + 2 * sp.pi * (reservoir_symbol + d)
    - (k1_symbol + k2_symbol + 2 * sp.pi * reservoir_symbol)
)
check(
    "conservation residual isolates the unique carry sum",
    sp.simplify(conservation_residual - 2 * sp.pi * (d - c1_symbol - c2_symbol)) == 0,
)
check(
    "solving exact conservation uniquely fixes the reservoir increment",
    sp.solve(sp.Eq(conservation_residual, 0), d) == [c1_symbol + c2_symbol],
)

# Three-axis componentwise conservation.
k_first = sp.Matrix([3 * sp.pi / 4, -4 * sp.pi / 5, sp.pi / 3])
k_second = sp.Matrix([sp.pi / 2, -sp.pi / 2, 5 * sp.pi / 6])
reservoir3 = sp.Matrix([2, -1, 4])
increment3 = sp.Matrix([sp.pi / 2, -3 * sp.pi / 4, 7 * sp.pi / 3])
first_after = sp.zeros(3, 1)
second_after = sp.zeros(3, 1)
reservoir_after3 = sp.zeros(3, 1)
carry_first3 = sp.zeros(3, 1)
carry_second3 = sp.zeros(3, 1)
for axis in range(3):
    step = pair_step(
        k_first[axis], k_second[axis], reservoir3[axis], increment3[axis]
    )
    first_after[axis], second_after[axis], reservoir_after3[axis] = step[:3]
    carry_first3[axis], carry_second3[axis] = step[3:]
before3 = sp.simplify(k_first + k_second + 2 * sp.pi * reservoir3)
after3 = sp.simplify(first_after + second_after + 2 * sp.pi * reservoir_after3)
check("three-axis aggregate is exactly conserved", sp.simplify(after3 - before3) == sp.zeros(3, 1))
check(
    "three-axis reservoir update is componentwise carry addition",
    sp.simplify(
        reservoir_after3 - reservoir3 - carry_first3 - carry_second3
    )
    == sp.zeros(3, 1),
)

# Signed cubic covariance away from the principal branch endpoint.
R = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, -1]])
check("signed cubic witness is orthogonal", R.T * R == sp.eye(3))
transformed_before = sp.simplify(R * before3)
transformed_after = sp.simplify(R * after3)
check("exact conserved aggregate is a cubic vector", transformed_before == transformed_after)
check("cubic transformation preserves the aggregate norm", sp.simplify((transformed_before.T * transformed_before)[0] - (before3.T * before3)[0]) == 0)

# Individual-winding and aggregate-reservoir equivalence.
w1, w2 = sp.symbols("w1 w2", integer=True)
particle_before = k1_symbol + 2 * sp.pi * w1 + k2_symbol + 2 * sp.pi * w2
particle_after = (
    k1_after_symbol + 2 * sp.pi * (w1 + c1_symbol)
    + k2_after_symbol + 2 * sp.pi * (w2 + c2_symbol)
)
check("individual lifted particle sum is exactly conserved", sp.simplify(particle_after - particle_before) == 0)
aggregate_before = k1_symbol + k2_symbol + 2 * sp.pi * (w1 + w2)
aggregate_after = k1_after_symbol + k2_after_symbol + 2 * sp.pi * (w1 + w2 + c1_symbol + c2_symbol)
check("aggregate reservoir partition equals individual winding partition", sp.simplify(aggregate_after - particle_after) == 0 and sp.simplify(aggregate_before - particle_before) == 0)

# Multi-event telescoping.
k1_run = sp.pi / 5
k2_run = -2 * sp.pi / 7
reservoir_run = sp.Integer(3)
initial_total = sp.simplify(k1_run + k2_run + 2 * sp.pi * reservoir_run)
carry_sum = sp.Integer(0)
for q_event in (3 * sp.pi / 4, -11 * sp.pi / 6, 13 * sp.pi / 5, -sp.pi / 3):
    step = pair_step(k1_run, k2_run, reservoir_run, q_event)
    k1_run, k2_run, reservoir_run = step[:3]
    carry_sum += step[3] + step[4]
    check(
        f"event {q_event} preserves the running aggregate",
        sp.simplify(k1_run + k2_run + 2 * sp.pi * reservoir_run - initial_total) == 0,
    )
check("reservoir changes telescope over all events", sp.simplify(reservoir_run - 3 - carry_sum) == 0)

# Scale, mass, and energy non-identifiability.
p_star, scale, a = sp.symbols("p_star scale a", positive=True)
physical_total = p_star * initial_total
check("physical total retains an independent conversion scale", physical_total.has(p_star))
check("rescaling the unit rescales the same conserved aggregate", sp.simplify(physical_total.subs(p_star, scale * p_star) - scale * physical_total) == 0)
A = sp.diag(2, 3)
B0 = sp.Matrix([[1, -2]])
mass0 = sp.simplify((B0 * A.inv() * B0.T)[0])
mass_scaled = sp.simplify(((p_star * B0) * A.inv() * (p_star * B0).T)[0])
check("the physical scale squares into the FTD-0893 mass tensor", sp.simplify(mass_scaled - p_star**2 * mass0) == 0)

k_band = sp.symbols("k_band", real=True)
w_band = sp.symbols("w_band", integer=True)
band_energy = 1 - sp.cos(k_band)
check("periodic band energy is exactly blind to winding", sp.trigsimp((1 - sp.cos(k_band + 2 * sp.pi * w_band)) - band_energy) == 0)
W = sp.symbols("W", integer=True)
energy_zero = sp.Integer(0)
energy_quadratic = a * W**2
check("the same carry state admits inequivalent reservoir energy laws", energy_zero != energy_quadratic)
check("carry conservation alone contains no energy parameter", not conservation_residual.has(a))

energy_arm = arms[1]
energy_step = pair_step(*energy_arm)
band_before = 2 - sp.cos(energy_arm[0]) - sp.cos(energy_arm[1])
band_after = 2 - sp.cos(energy_step[0]) - sp.cos(energy_step[1])
check("opposite quasimomentum increment need not conserve band energy", sp.simplify(band_after - band_before) != 0)

# Frozen-corpus scope checks.
check(
    "FTD-0896 leaves winding update and conversion scale open",
    "winding update and the conversion scale remain open" in texts["bloch_lift_theorem"],
)
check(
    "FTD-0893 still requires an independently closed total momentum map",
    "we need an independently closed total-momentum map" in texts["dressed_mass_theorem"],
)
check(
    "FTD-0514 transports known momentum but does not originate the impulse",
    "known constituent worldline" in texts["face_balance"]
    and "exact balance after an impulse is not an origin for the impulse"
    in texts["face_balance"],
)
check(
    "FTD-0516 supplies only a selected restricted impulse origin",
    "one selected hard-contact matter action generates the restricted impulse"
    in texts["hard_contact"]
    and "or create a production collision transaction" in texts["hard_contact"],
)
check(
    "FTD-0890 reaction transport remains conditional and nonproduction",
    "conditional on the already selected relativistic dispersion"
    in texts["reaction_transport"]
    and "this closes a reference source-transport gearbox"
    in texts["reaction_transport"]
    and "does not derive stable matter" in texts["reaction_transport"],
)
check(
    "existing phase rail transports an imposed rather than derived energy scale",
    "imposed scale omega a^2/2 is transported exactly, not derived"
    in texts["phase_rail"],
)
check(
    "Bloch API explicitly leaves winding dynamics and momentum unit underived",
    "does not derive winding dynamics, a physical momentum unit"
    in texts["bloch_header"],
)
check(
    "face-balance API consumes momentum as input",
    "const vec3& momentum" in texts["face_header"],
)
check(
    "reaction API consumes a required matter impulse and denies mass derivation",
    "required matter impulse" in texts["reaction_header"]
    and "not a production matter law, a mass derivation"
    in texts["reaction_header"],
)

# Terminal firewalls.
terminal_markers = (
    "interaction increment origin=open",
    "reservoir partition=not selected",
    "reservoir energy law=open",
    "physical momentum scale=open",
    "total field matter momentum map=open",
    "absolute mass scale=not derived",
    "production integration=forbidden",
    "no new selected vector type=true",
    "gstar born bell lorentz completeness=untouched",
)
for marker in terminal_markers:
    check(f"terminal firewall {marker}", marker in protocol_text)

passed = sum(result for _, result in checks)
total = len(checks)
print(f"\nFTD-0897 exact certificate: {passed}/{total} checks passed")
verdict = passed == total
print(
    "RECIPROCAL_CARRY_RESERVOIR_EXACT_PHYSICAL_ORIGIN_SCALE_ENERGY_OPEN="
    f"{'TRUE' if verdict else 'FALSE'}"
)
raise SystemExit(0 if verdict else 1)
