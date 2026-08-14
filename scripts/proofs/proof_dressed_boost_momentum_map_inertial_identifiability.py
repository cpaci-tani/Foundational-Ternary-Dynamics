#!/usr/bin/env python3
"""FTD-0893 exact dressed-boost momentum-map identifiability certificate."""

from __future__ import annotations

import hashlib
import re
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_DRESSED_BOOST_MOMENTUM_MAP_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md"
)
PROTOCOL_HASH = "D397CC9777B8EF6CE18E2AF4D06060B9245977D6BAB34171CC89DBB472609E42"

SOURCES = {
    "collective_theorem": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "native_time_carrier_programme/"
        "THEOREM_COLLECTIVE_REACTION_TRIPLET_AND_INERTIAL_CURVATURE_BOUNDARY_v1.md",
        "CFD4E0EE4E0193BD435D7A6F9DF42EF589551078322C925A46A7E3693CDB2371",
    ),
    "spline_defect": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "common_action_mechanics_reciprocity/"
        "ANALYSIS_SPLINE_POYNTING_NOETHER_DEFECT_v1.md",
        "2D63051782D1648F51FE9EA8A7B90FE9FF38827C119D9C8033A12953F5389DF5",
    ),
    "common_action": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "constituent_complete_matter/"
        "ANALYSIS_CELL_MEASURE_COMMON_ACTION_CLOSURE_v1.md",
        "6F87DE2CC0559492322453E824D971BEBFE680512C6C1A8D4CCCCF5324F48A68",
    ),
    "mobile_dressing": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "constituent_complete_matter/"
        "ANALYSIS_MOBILE_DRESSING_STRUCTURE_FACTOR_v2.md",
        "D7859E19D50EE0D6B913D838C60BF1B24146B2AC50B94DA0163645CCB685601C",
    ),
    "static_boost": (
        ROOT / "docs/theory/10_eft_program/derivations/"
        "constituent_complete_matter/"
        "ANALYSIS_REST_QUALIFIED_MOVING_DRESSING_RELATIVE_ORBIT_v1.md",
        "53B38713C5E545A68C8B0B6D188E2953220E1530F16D1C327FC41608C0CB0371",
    ),
    "common_header": (
        ROOT / "engine/include/ftd/eft/connected_moore_block_action.h",
        "09328FB23642D3D8AFD165994F8F8B2101A52DD7E0BC5BFEE2E2DF27ABE6EDF8",
    ),
    "common_source": (
        ROOT / "engine/src/eft/connected_moore_block_action.cpp",
        "207002636F290E9C55BB33FDFED489C423EEC5BFA3C0986D4E320A460E3F0262",
    ),
    "spline_header": (
        ROOT / "engine/include/ftd/eft/spline_poynting_momentum.h",
        "AEF46732679E23CA187EBCFFAC288AAFAE88BEE0409AE9698C58A532D9728474",
    ),
    "spline_source": (
        ROOT / "engine/src/eft/spline_poynting_momentum.cpp",
        "C2ECAFEEAA4B77E71673D5560C8606AF34FDE50084E7B3AA44A2199B3929B300",
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


# Frozen source and protocol lock.
check("protocol hash matches the pre-run lock", sha256(PROTOCOL) == PROTOCOL_HASH)
texts: dict[str, str] = {}
for name, (path, expected_hash) in SOURCES.items():
    check(f"source hash {name}", sha256(path) == expected_hash)
    texts[name] = plain(path.read_text(encoding="utf-8"))

protocol_text = plain(PROTOCOL.read_text(encoding="utf-8"))
check(
    "protocol freezes the conditional mass tensor",
    "m = b a^-1 b^t" in protocol_text
    and "conditional on both a and b" in protocol_text,
)
check(
    "protocol freezes momentum-map non-identifiability",
    "do not identify physical inertia without the momentum map" in protocol_text,
)
check(
    "protocol freezes the production and Born-Gstar firewalls",
    "production_integration=forbidden" in protocol_text
    and "born_target_used=false" in protocol_text
    and "native_gstar_synchronization=false" in protocol_text,
)

# Exact rank-three rational witness of the general KKT theorem.
A6 = sp.diag(
    sp.Matrix([[4, 1], [1, 3]]),
    sp.Matrix([[3, -1], [-1, 2]]),
    sp.Matrix([[5, 2], [2, 6]]),
)
B6 = sp.Matrix(
    [
        [1, 2, 0, 0, 1, 0],
        [0, 1, 1, -1, 0, 1],
        [1, 0, 0, 2, -1, 1],
    ]
)
P1, P2, P3 = sp.symbols("P1 P2 P3", real=True)
P3v = sp.Matrix([P1, P2, P3])
M3_general = sp.simplify(B6 * A6.inv() * B6.T)
M3_general_inv = sp.simplify(M3_general.inv())
y_star = sp.simplify(A6.inv() * B6.T * M3_general_inv * P3v)
lagrange = sp.simplify(M3_general_inv * P3v)
energy_star = sp.simplify((y_star.T * A6 * y_star)[0] / 2)
energy_expected = sp.simplify((P3v.T * M3_general_inv * P3v)[0] / 2)

check("higher-dimensional energy Hessian is symmetric", A6 == A6.T)
check(
    "higher-dimensional energy Hessian is positive definite",
    all(A6[:i, :i].det() > 0 for i in range(1, A6.rows + 1)),
)
check("higher-dimensional momentum map has rank three", B6.rank() == 3)
check("general dressed tensor is symmetric", M3_general == M3_general.T)
check(
    "general dressed tensor is positive definite",
    all(M3_general[:i, :i].det() > 0 for i in range(1, 4)),
)
check("KKT minimizer satisfies the momentum constraint", B6 * y_star == P3v)
check(
    "KKT minimizer satisfies stationarity",
    sp.simplify(A6 * y_star - B6.T * lagrange) == sp.zeros(6, 1),
)
check(
    "KKT minimum energy has inverse-mass curvature",
    sp.simplify(energy_star - energy_expected) == 0,
)
null_basis = B6.nullspace()
Z = sp.Matrix.hstack(*null_basis)
check("rank-three constraint leaves the expected null space", Z.shape == (6, 3))
check("null perturbations preserve fixed momentum", B6 * Z == sp.zeros(3, 3))
check(
    "stationary direction is energy-orthogonal to fixed-momentum perturbations",
    sp.simplify(Z.T * A6 * y_star) == sp.zeros(3, 1),
)
null_energy = sp.simplify(Z.T * A6 * Z)
check(
    "all nonzero null perturbations raise quadratic energy",
    all(null_energy[:i, :i].det() > 0 for i in range(1, 4)),
)

# Symbolic one-axis matter/field realization.
a, k, g, b_m, b_f = sp.symbols("a k g b_m b_f", real=True)
P, E0, U0, s = sp.symbols("P E0 U0 s", real=True, nonzero=True)
m, m_f = sp.symbols("m m_f", positive=True)
A2 = sp.Matrix([[a, g], [g, k]])
b = sp.Matrix([b_m, b_f])
det_A = sp.factor(A2.det())
mass = sp.factor((b.T * A2.inv() * b)[0])
mass_expected = sp.factor(
    (k * b_m**2 - 2 * g * b_m * b_f + a * b_f**2) / det_A
)
allocation = sp.simplify(A2.inv() * b * P / mass)
energy = sp.simplify((allocation.T * A2 * allocation)[0] / 2)

check("two-channel determinant is a k minus g squared", det_A == a * k - g**2)
check("two-channel inverse is exact", sp.simplify(A2 * A2.inv()) == sp.eye(2))
check("two-channel mass formula is exact", sp.simplify(mass - mass_expected) == 0)
check("allocated amplitudes satisfy physical momentum", sp.simplify((b.T * allocation)[0] - P) == 0)
check(
    "allocated amplitudes satisfy constrained stationarity",
    sp.simplify(A2 * allocation - b * P / mass) == sp.zeros(2, 1),
)
check("minimum energy is P squared over two M", sp.simplify(energy - P**2 / (2 * mass)) == 0)
positive_decomposition = (
    k * (b_m - g * b_f / k) ** 2 + det_A * b_f**2 / k
)
check(
    "mass numerator has an exact positive square decomposition",
    sp.simplify(sp.together(positive_decomposition)
                - (k * b_m**2 - 2 * g * b_m * b_f + a * b_f**2)) == 0,
)
check(
    "matter-only uncoupled control returns input mass",
    sp.simplify(mass.subs({g: 0, b_m: 1, b_f: 0, a: 1 / m}) - m) == 0,
)
check(
    "independent matter and field channels add conditionally",
    sp.simplify(
        mass.subs({g: 0, b_m: 1, b_f: 1, a: 1 / m, k: 1 / m_f})
        - (m + m_f)
    ) == 0,
)
check(
    "kinetic cross-coupling changes the conditional mass",
    mass.subs({a: 2, k: 3, g: 1, b_m: 1, b_f: 1}) == sp.Rational(3, 5)
    and mass.subs({a: 2, k: 3, g: 0, b_m: 1, b_f: 1}) == sp.Rational(5, 6),
)
check(
    "field-like odd amplitude participates in the constrained minimum",
    sp.simplify(allocation[1].subs({a: 2, k: 3, g: 1, b_m: 1, b_f: 1}))
    != 0,
)

# Identifiability controls.
effective_energy = E0 + U0 + P**2 / (2 * mass)
check("static offset does not enter mass", not mass.has(E0) and not mass.has(U0))
check(
    "static offset does not change momentum curvature",
    sp.simplify(sp.diff(effective_energy, P, 2) - 1 / mass) == 0,
)
scaled_mass = sp.simplify(
    mass.subs({b_m: s * b_m, b_f: s * b_f}, simultaneous=True)
)
check("rescaling the same momentum map rescales mass quadratically", sp.simplify(scaled_mass - s**2 * mass) == 0)
check(
    "same energy Hessian admits unequal inertias",
    mass.subs({a: 2, k: 3, g: 1, b_m: 1, b_f: 0})
    != mass.subs({a: 2, k: 3, g: 1, b_m: 2, b_f: 0}),
)
zeta, zeta_prime, c_path, q_path = sp.symbols(
    "zeta zeta_prime c_path q_path", real=True, nonzero=True
)
path_energy = q_path * zeta**2 / 2
renormalized_path_energy = sp.expand(path_energy.subs(zeta, zeta_prime / c_path))
check(
    "unconstrained path curvature depends on parameter normalization",
    sp.diff(renormalized_path_energy, zeta_prime, 2) == q_path / c_path**2,
)
check(
    "path normalization ambiguity is independent of static offset",
    sp.diff(E0 + U0 + renormalized_path_energy, zeta_prime, 2)
    == q_path / c_path**2,
)

# Three-axis cubic replication.
I3 = sp.eye(3)
A_cubic = sp.kronecker_product(I3, A2)
B_cubic = sp.kronecker_product(I3, b.T)
M_cubic = sp.simplify(B_cubic * A_cubic.inv() * B_cubic.T)
R = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, -1]])
T = sp.kronecker_product(R, sp.eye(2))
check("signed cubic test transform is orthogonal", R.T * R == I3)
check("replicated energy Hessian is cubic invariant", sp.simplify(T.T * A_cubic * T - A_cubic) == sp.zeros(6))
check("replicated momentum map is cubic covariant", sp.simplify(B_cubic * T - R * B_cubic) == sp.zeros(3, 6))
check("replicated dressed mass is an isotropic cubic tensor", sp.simplify(M_cubic - mass * I3) == sp.zeros(3))
check("cubic transform preserves the dressed mass tensor", sp.simplify(R * M_cubic * R.T - M_cubic) == sp.zeros(3))

# Frozen-corpus boundary and scope checks.
check(
    "FTD-0892 keeps exact total field-matter momentum open",
    "exact total physical momentum law needs an additional closure" in texts["collective_theorem"]
    and "not an exact coupled charge" in texts["collective_theorem"],
)
check(
    "natural spline-Poynting coupled closure is closed negative",
    "closed negative — natural spline field momentum closure" in texts["spline_defect"]
    and "does not absorb the compact gait's reaction" in texts["spline_defect"],
)
check(
    "common action freezes mass as an input rather than a result",
    "the mass factor controls dispersion" in texts["common_action"]
    and "particle-mass, or production claim follows" in texts["common_action"],
)
check(
    "measured co-moving dressing is not promoted to a momentum law",
    "measured co-moving dressing constructive" in texts["mobile_dressing"]
    and "a co-moving energetic dressing" in texts["mobile_dressing"],
)
check(
    "static field assignment fails the moving-orbit control",
    "static boost closed negative" in texts["static_boost"]
    and "is not a uniformly moving object" in texts["static_boost"],
)
check(
    "common-action API records matter and spline field momenta separately",
    "matter_momentum_before" in texts["common_header"]
    and "spline_field_momentum_before" in texts["common_header"],
)
check(
    "spline observer does not alter production",
    "observer-only b-spline poynting momentum candidate" in texts["spline_header"]
    and "does not alter the matched fields or production tick" in texts["spline_header"],
)
check(
    "discrete translation and absolute-mass boundaries are frozen",
    "z^3 translation covariance does not by itself create" in protocol_text
    and "absolute_mass_scale=not_derived" in protocol_text,
)
check(
    "stable pole and production remain open",
    "stable_matter_pole=open" in protocol_text
    and "production_integration=forbidden" in protocol_text,
)
check(
    "no selected type or Born-Gstar claim is added",
    "no_new_selected_vector_type=true" in protocol_text
    and "gstar_born_bell_lorentz_completeness=untouched" in protocol_text,
)

failed = [name for name, passed in checks if not passed]
print()
print(f"FTD-0893 exact certificate: {len(checks) - len(failed)}/{len(checks)} PASS")
if failed:
    print("EXECUTION_INVALID")
    for name in failed:
        print(f"  - {name}")
    raise SystemExit(1)

print("DRESSED_BOOST_MOMENTUM_MAP_CONDITIONAL_THEOREM_PASS")
print("ENERGY_HESSIAN_ALONE_MASS_IDENTIFIABILITY_CLOSED_NEGATIVE")
print("TOTAL_FIELD_MATTER_MOMENTUM_MAP_OPEN")
print("ABSOLUTE_MASS_SCALE_NOT_DERIVED")
print("PRODUCTION_BORN_GSTAR_UNCHANGED")
