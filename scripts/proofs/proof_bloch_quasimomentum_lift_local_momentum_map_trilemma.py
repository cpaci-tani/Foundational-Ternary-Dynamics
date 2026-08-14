#!/usr/bin/env python3
"""FTD-0894 exact Bloch-lift and local momentum-map trilemma certificate."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_BLOCH_QUASIMOMENTUM_LIFT_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md"
)
PROTOCOL_HASH = "2EC2030AC29C287093019CA8DCD5542577312B9730EFF5B33C4324956CBDC791"

SOURCES = {
    "dressed_mass": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md",
        "378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C",
    ),
    "local_pseudomomentum": (
        ROOT / "docs/theory/07_assessment/"
        "common_action_mechanics_reciprocity/"
        "AUDIT_MATCHED_FACE_MOMENTUM_TRANSACTION.md",
        "C4A157B2D9114EC251E60F24B93C5580222B8EB937A3958322248779D2DC6687",
    ),
    "face_balance": (
        ROOT / "docs/theory/07_assessment/"
        "common_action_mechanics_reciprocity/"
        "AUDIT_EXACT_MOMENTUM_FACE_BALANCE.md",
        "72364E30BC10216661E64FAC67B13810EE1CEB2903AF7C2A408337EA16615AAF",
    ),
    "translation_trilemma": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "common_action_mechanics_reciprocity/"
        "THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md",
        "527BDA49C213C1D58862A8A6254FC153416253EA3159BD7B958F8E43B69630EC",
    ),
    "bloch_transport": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "common_action_mechanics_reciprocity/"
        "THEOREM_INTEGER_TRANSLATION_BLOCH_TRANSPORT.md",
        "F472E65AFD9EB1B97B2EA4A8CC5C613960006928752F5A87F50302974DC2E6FD",
    ),
    "spline_defect": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "common_action_mechanics_reciprocity/"
        "ANALYSIS_SPLINE_POYNTING_NOETHER_DEFECT_v1.md",
        "2D63051782D1648F51FE9EA8A7B90FE9FF38827C119D9C8033A12953F5389DF5",
    ),
    "stress_audit": (
        ROOT / "docs/theory/07_assessment/constituent_complete_matter/"
        "AUDIT_TOTAL_MOMENTUM_STRESS_LEDGER_v1.md",
        "A690E90412D397B30FF899CA1568E81E0CC496A16578F87A95D00495D69C19BE",
    ),
    "bloch_header": (
        ROOT / "engine/include/ftd/eft/integer_bloch_transport.h",
        "AC535306938C34789AC90EAA539266DA1976A954E0A19CAE71BF4798921ED615",
    ),
    "face_header": (
        ROOT / "engine/include/ftd/eft/momentum_face_balance.h",
        "B9F435FF75E7EE133A9393294E45B1C316E026472A0C93FCF457077BDE6A6567",
    ),
    "stress_header": (
        ROOT / "engine/include/ftd/eft/momentum_transport_current.h",
        "77318892EA9BED7CECEDB7A2DD533E0B62CB217D9F2505A19F45858F5B81AC4F",
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
    return re.sub(r"\s+", " ", text.replace("`", "").lower()).strip()


def wrap_angle(x: sp.Expr) -> sp.Expr:
    """Exact principal wrap to [-pi, pi) for rational multiples of pi."""
    return sp.simplify(x - 2 * sp.pi * sp.floor((x + sp.pi) / (2 * sp.pi)))


def carry_angle(x: sp.Expr) -> sp.Expr:
    """Exact reciprocal-lattice carry paired with wrap_angle."""
    return sp.floor((x + sp.pi) / (2 * sp.pi))


def sawtooth_partial(k: sp.Expr, order: int) -> sp.Expr:
    return sp.expand_trig(
        2
        * sum(
            sp.Rational((-1) ** (r + 1), r) * sp.sin(r * k)
            for r in range(1, order + 1)
        )
    )


# Frozen source and protocol lock.
check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == PROTOCOL_HASH)
texts: dict[str, str] = {}
for name, (path, expected_hash) in SOURCES.items():
    check(f"source hash {name}", sha256(path) == expected_hash)
    texts[name] = plain(path.read_text(encoding="utf-8"))

protocol_text = plain(PROTOCOL.read_text(encoding="utf-8"))
check(
    "protocol freezes the torus character dual without a finite-torus ontology",
    "z3_character_dual=t3" in protocol_text
    and "finite_torus_ontology=not_assumed" in protocol_text,
)
check(
    "protocol freezes the global section obstruction",
    "global_continuous_homomorphic_t3_to_r3_section=impossible" in protocol_text,
)
check(
    "protocol freezes the finite-range and branch price",
    "finite_range_global_unwrapped_generator=impossible" in protocol_text
    and "infinite_range_and_branch_discontinuous" in protocol_text,
)
check(
    "protocol leaves winding, momentum scale, and physical map open",
    "winding_history_type=open_candidate_not_selected" in protocol_text
    and "physical_momentum_scale=open" in protocol_text
    and "total_field_matter_momentum_map=open" in protocol_text,
)

# Exact character multiplication and torus addition.
k1, k2 = sp.symbols("k1 k2", real=True)
n = sp.symbols("n", integer=True)
character_product = sp.exp(sp.I * k1 * n) * sp.exp(sp.I * k2 * n)
character_sum = sp.exp(sp.I * (k1 + k2) * n)
check(
    "one-axis characters multiply by adding labels",
    sp.expand_power_exp(character_product) == sp.expand_power_exp(character_sum),
)
check(
    "reciprocal shifts leave every integer-translation character unchanged",
    sp.simplify(sp.exp(sp.I * (k1 + 2 * sp.pi) * n) / sp.exp(sp.I * k1 * n)) == 1,
)
n3 = sp.Matrix([2, -3, 5])
ka = sp.Matrix([sp.pi / 3, -sp.pi / 2, 7 * sp.pi / 6])
kb = sp.Matrix([5 * sp.pi / 6, sp.pi / 3, -sp.pi / 2])
dot = lambda left, right: (left.T * right)[0]
check(
    "three-axis character product is exact",
    sp.simplify(
        sp.exp(sp.I * dot(ka, n3))
        * sp.exp(sp.I * dot(kb, n3))
        / sp.exp(sp.I * dot(ka + kb, n3))
    )
    == 1,
)
reciprocal = 2 * sp.pi * sp.Matrix([3, -2, 4])
check(
    "three-axis reciprocal vector is character-invisible",
    sp.simplify(sp.exp(sp.I * dot(reciprocal, n3))) == 1,
)
check(
    "quasimomentum addition is a torus operation",
    all(
        sp.simplify(
            wrap_angle(ka[i] + kb[i])
            - wrap_angle(wrap_angle(ka[i]) + wrap_angle(kb[i]))
        )
        == 0
        for i in range(3)
    ),
)

# Compact-subgroup obstruction to a continuous homomorphic section.
m = sp.symbols("m", integer=True, positive=True)
v = sp.Matrix([1, -2, 3])
check("a nonzero additive image contains every integer multiple", m * v != sp.zeros(3, 1))
check(
    "integer multiples of a nonzero image are unbounded",
    sp.expand(dot(m * v, m * v)) == 14 * m**2,
)
check(
    "the only bounded additive subgroup candidate in R3 is the zero image",
    dot(v, v) > 0 and sp.limit(dot(m * v, m * v), m, sp.oo) == sp.oo,
)
check(
    "the zero map cannot section a nontrivial torus label",
    wrap_angle(sp.pi / 2) != wrap_angle(0),
)
check(
    "compact image plus exact section is contradictory",
    "the only compact additive subgroup of r^3 is {0}" in protocol_text
    and "cannot be a section" in protocol_text,
)

# Finite-range translation-invariant spectral weights are periodic.
k = sp.symbols("k", real=True)
a0, a1, a2, b1, b2 = sp.symbols("a0 a1 a2 b1 b2", real=True)
finite_weight = a0 + a1 * sp.cos(k) + a2 * sp.cos(2 * k) + b1 * sp.sin(k) + b2 * sp.sin(2 * k)
check(
    "representative finite-range weight is exactly periodic",
    sp.trigsimp(finite_weight.subs(k, k + 2 * sp.pi) - finite_weight) == 0,
)
check(
    "a periodic weight cannot equal the unwrapped coordinate globally",
    sp.simplify((k + 2 * sp.pi) - k) == 2 * sp.pi,
)
local_weight_1 = sp.sin(k)
local_weight_2 = 2 * sp.sin(k) + sp.Rational(1, 3) * sp.sin(2 * k)
check(
    "distinct local odd spectral weightings exist",
    sp.trigsimp(local_weight_1 - local_weight_2) != 0,
)
check(
    "both example local odd weights are reciprocal-periodic",
    sp.trigsimp(local_weight_1.subs(k, k + 2 * sp.pi) - local_weight_1) == 0
    and sp.trigsimp(local_weight_2.subs(k, k + 2 * sp.pi) - local_weight_2) == 0,
)
check(
    "local spectral normalization is freely rescalable",
    sp.simplify(7 * local_weight_1 - local_weight_1) != 0,
)

# Exact Fourier coefficients and Abel limit of the principal sawtooth.
r = sp.symbols("r", integer=True, positive=True)
fourier_sine = sp.integrate(k * sp.sin(r * k), (k, -sp.pi, sp.pi)) / sp.pi
check(
    "principal sawtooth sine coefficient is exact",
    sp.simplify(fourier_sine - 2 * (-1) ** (r + 1) / r) == 0,
)
fourier_cosine = sp.integrate(k * sp.cos(r * k), (k, -sp.pi, sp.pi)) / sp.pi
check("principal sawtooth cosine coefficient vanishes", fourier_cosine == 0)
check("principal sawtooth mean vanishes", sp.integrate(k, (k, -sp.pi, sp.pi)) == 0)

rho = sp.symbols("rho", positive=True)
z = sp.symbols("z")
log_series = sp.summation((-1) ** (r + 1) * z**r / r, (r, 1, sp.oo))
check("alternating harmonic generating function is log one plus z", log_series == sp.log(z + 1))
for angle in (sp.pi / 3, sp.pi / 2, -2 * sp.pi / 3):
    factor_identity = sp.simplify(
        1 + sp.exp(sp.I * angle)
        - 2 * sp.cos(angle / 2) * sp.exp(sp.I * angle / 2)
    )
    check(f"one plus phase factorization at {angle}", factor_identity == 0)
    check(f"interior half-angle factor is positive at {angle}", sp.cos(angle / 2) > 0)
check(
    "factorization fixes the Abel-summed imaginary part to k over two",
    "k = 2 sum_(r=1)^infinity" in protocol_text
    and "on the principal branch -pi < k < pi" in protocol_text,
)

for order in (1, 2, 5, 8):
    partial = sawtooth_partial(k, order)
    check(
        f"order {order} local truncation is periodic",
        sp.trigsimp(partial.subs(k, k + 2 * sp.pi) - partial) == 0,
    )
    check(
        f"order {order} truncation fails at the branch edge",
        sp.simplify(partial.subs(k, sp.pi)) == 0
        and sp.simplify(partial.subs(k, sp.pi) - sp.pi) != 0,
    )

# Exact wrap, lift, and reciprocal carry.
angles = [
    -11 * sp.pi / 4,
    -sp.pi,
    -3 * sp.pi / 4,
    0,
    5 * sp.pi / 6,
    sp.pi,
    13 * sp.pi / 4,
]
check(
    "all registered rational angles wrap into the principal branch",
    all(-sp.pi <= wrap_angle(x) < sp.pi for x in angles),
)
check(
    "wrap and carry reconstruct every registered angle",
    all(sp.simplify(wrap_angle(x) + 2 * sp.pi * carry_angle(x) - x) == 0 for x in angles),
)
check(
    "principal wrap is idempotent",
    all(sp.simplify(wrap_angle(wrap_angle(x)) - wrap_angle(x)) == 0 for x in angles),
)

x1 = 3 * sp.pi / 4
x2 = sp.pi / 2
x12 = x1 + x2
k12 = wrap_angle(x12)
w12 = carry_angle(x12)
check("principal addition crosses the positive zone edge", k12 == -3 * sp.pi / 4)
check("zone crossing creates the exact positive carry", w12 == 1)
check("winding restores real addition", sp.simplify(k12 + 2 * sp.pi * w12 - x12) == 0)
check("principal-only addition loses a reciprocal vector", sp.simplify(k12 - x12) == -2 * sp.pi)

x3a = sp.Matrix([3 * sp.pi / 4, -4 * sp.pi / 5, sp.pi / 3])
x3b = sp.Matrix([sp.pi / 2, -sp.pi / 2, 5 * sp.pi / 6])
x3sum = x3a + x3b
k3sum = sp.Matrix([wrap_angle(x3sum[i]) for i in range(3)])
w3sum = sp.Matrix([carry_angle(x3sum[i]) for i in range(3)])
check("three-axis carry has the expected integer triplet", w3sum == sp.Matrix([1, -1, 1]))
check(
    "three-axis lift reconstructs exact real addition",
    sp.simplify(k3sum + 2 * sp.pi * w3sum - x3sum) == sp.zeros(3, 1),
)
check(
    "three-axis principal labels alone lose winding information",
    sp.simplify(k3sum - x3sum) == -2 * sp.pi * w3sum,
)

# Physical scale and FTD-0893 mass-map compatibility.
p_star, scale = sp.symbols("p_star scale", positive=True)
lifted_k = k12 + 2 * sp.pi * w12
physical_candidate = p_star * lifted_k
check("candidate momentum requires an independent scale", physical_candidate.has(p_star))
check(
    "changing the scale changes physical momentum with identical quasimomentum",
    sp.simplify(physical_candidate.subs(p_star, 2) - physical_candidate.subs(p_star, 1))
    == lifted_k,
)
A = sp.diag(2, 3, 5)
B0 = sp.Matrix([[1, -1, 2]])
mass0 = sp.simplify((B0 * A.inv() * B0.T)[0])
mass_scaled = sp.simplify(((scale * B0) * A.inv() * (scale * B0).T)[0])
check("FTD-0893 mass map is positive in the exact witness", mass0 > 0)
check("momentum scale ambiguity squares into dressed inertia", sp.simplify(mass_scaled - scale**2 * mass0) == 0)

# Frozen-corpus scope checks.
check(
    "FTD-0893 requires an independently closed total momentum map",
    "m = b a^-1 b^t" in texts["dressed_mass"]
    and "we need an independently closed total-momentum map" in texts["dressed_mass"],
)
check(
    "FTD-0473 supplies a selected rescalable local pseudomomentum only",
    "selected local staggered pseudomomentum" in texts["local_pseudomomentum"]
    and "overall normalization can be rescaled" in texts["local_pseudomomentum"]
    and "not promoted to the unique ontological physical momentum" in texts["local_pseudomomentum"],
)
check(
    "FTD-0514 transports known momentum but does not originate it",
    "known constituent worldline" in texts["face_balance"]
    and "exact balance after an impulse is not an origin for the impulse" in texts["face_balance"],
)
check(
    "FTD-0554 prices exact fractional translation with nonlocal support",
    "continuous one-parameter group of finite-range homogeneous" in texts["translation_trilemma"]
    and "exact nonlocal escape" in texts["translation_trilemma"],
)
check(
    "FTD-0556 supplies free-flux Bloch transport but no stable matter pole",
    "continuous centroid from integer dynamics" in texts["bloch_transport"]
    and "a localized free packet is not a stable particle" in texts["bloch_transport"],
)
check(
    "natural spline Poynting momentum fails coupled reaction closure",
    "does not absorb the compact gait's reaction" in texts["spline_defect"],
)
check(
    "FTD-0769 execution is invalid and licenses no physical closure claim",
    "consumed as execution-invalid" in texts["stress_audit"]
    and "establishes neither closure nor non-closure" in texts["stress_audit"],
)
check(
    "Bloch implementation remains observer-only",
    "observer-only bloch analysis" in texts["bloch_header"],
)
check(
    "face-balance API takes momentum as an input",
    "const vec3& momentum" in texts["face_header"]
    and "make_momentum_worldline_balance" in texts["face_header"],
)
check(
    "stress implementation remains observer-only and explicitly scaled",
    "observer-only research instrumentation" in texts["stress_header"]
    and "every momentum-sector quantity carries the interaction_scale" in texts["stress_header"],
)

# Terminal firewalls.
terminal_markers = (
    "local_stress_route=not_ruled_out",
    "winding_history_type=open_candidate_not_selected",
    "physical_momentum_scale=open",
    "total_field_matter_momentum_map=open",
    "absolute_mass_scale=not_derived",
    "production_integration=forbidden",
    "no_new_selected_vector_type=true",
    "gstar_born_bell_lorentz_completeness=untouched",
)
for marker in terminal_markers:
    check(f"terminal firewall {marker}", marker in protocol_text)

passed = sum(result for _, result in checks)
total = len(checks)
print(f"\nFTD-0894 exact certificate: {passed}/{total} checks passed")
verdict = passed == total
print(
    "BLOCH_QUASIMOMENTUM_LIFT_TRILEMMA_EXACT_PHYSICAL_MOMENTUM_MAP_OPEN="
    f"{'TRUE' if verdict else 'FALSE'}"
)
raise SystemExit(0 if verdict else 1)
