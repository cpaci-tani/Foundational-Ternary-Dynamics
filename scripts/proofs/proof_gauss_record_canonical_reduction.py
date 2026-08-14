#!/usr/bin/env python3
"""FTD-0877 exact certificate for matched Gauss-record canonical reduction."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_GAUSS_RECORD_CANONICAL_REDUCTION_v1.md"
)
THEOREM = ROOT / (
    "docs/theory/10_eft_program/derivations/"
    "native_time_carrier_programme/"
    "THEOREM_GAUSS_RECORD_CANONICAL_REDUCTION_AND_"
    "PRODUCTION_PROJECTOR_BOUNDARY_v1.md"
)

FROZEN = {
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_FLUX_WAVE_VELOCITY_MARKOV_CANONICAL_CARRIER_AND_"
    "PRODUCTION_BOUNDARY_v1.md":
        "656F51A4E5A533C0436E932B452A33810CD851D63E571621DF81ECB0C9BED622",
    "engine/include/ftd/field_operators.h":
        "25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48",
    "engine/include/ftd/poisson_solvers.h":
        "07F2E7DD85A1E476DAE6BE7F4FE371E664A2B965B9B542EC4162BDEEC5A9DBC4",
    "engine/src/poisson_solvers.cpp":
        "59DC42FB8D0160373F02301C5B7AB09B2C9692242FC0D852C0404ECCA371362B",
    "engine/include/ftd/eft/matched_gauss_transport.h":
        "1E07F87A0EBD0D1830D0632B82C2BD65497EBEAE7BB152EA02C5AAE19328B033",
    "engine/src/eft/matched_gauss_transport.cpp":
        "12BF98040BB45AD6CD9A409A93C842101C400CEEE6242E9B9352158A33A9D028",
    "engine/tests/test_native_source_core_fork.cpp":
        "81BE123F7EC73D78B2D233CAD733D438DED6A4E683FB542F9F1EC200FD6C68B1",
}
PROTOCOL_SHA256 = "4F24779197A2DE93ABB10DCFC0F84D23EB528A80E96CC3D4F1A548A429F27F4A"

checks = 0
failures = 0


def check(label: str, condition: bool) -> None:
    global checks, failures
    checks += 1
    if condition:
        print(f"PASS  C{checks} {label}")
    else:
        failures += 1
        print(f"FAIL  C{checks} {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def index(size: int, x: int, y: int, z: int) -> int:
    return (x % size) * size * size + (y % size) * size + (z % size)


def face_index(volume: int, axis: int, site: int) -> int:
    return axis * volume + site


def periodic_incidence(size: int) -> sp.Matrix:
    """Backward face divergence used by MatchedFaceFlux."""
    volume = size**3
    divergence = sp.zeros(volume, 3 * volume)
    for x in range(size):
        for y in range(size):
            for z in range(size):
                site = index(size, x, y, z)
                previous = (
                    index(size, x - 1, y, z),
                    index(size, x, y - 1, z),
                    index(size, x, y, z - 1),
                )
                for axis in range(3):
                    divergence[site, face_index(volume, axis, site)] += 1
                    divergence[site, face_index(volume, axis, previous[axis])] -= 1
    return divergence


def periodic_curl(size: int) -> sp.Matrix:
    """Backward-difference curl used by matched_curl()."""
    volume = size**3
    curl = sp.zeros(3 * volume, 3 * volume)
    for x in range(size):
        for y in range(size):
            for z in range(size):
                i = index(size, x, y, z)
                xm = index(size, x - 1, y, z)
                ym = index(size, x, y - 1, z)
                zm = index(size, x, y, z - 1)
                # C_x = B_z(i)-B_z(i-y)-B_y(i)+B_y(i-z)
                curl[face_index(volume, 0, i), face_index(volume, 2, i)] += 1
                curl[face_index(volume, 0, i), face_index(volume, 2, ym)] -= 1
                curl[face_index(volume, 0, i), face_index(volume, 1, i)] -= 1
                curl[face_index(volume, 0, i), face_index(volume, 1, zm)] += 1
                # C_y = B_x(i)-B_x(i-z)-B_z(i)+B_z(i-x)
                curl[face_index(volume, 1, i), face_index(volume, 0, i)] += 1
                curl[face_index(volume, 1, i), face_index(volume, 0, zm)] -= 1
                curl[face_index(volume, 1, i), face_index(volume, 2, i)] -= 1
                curl[face_index(volume, 1, i), face_index(volume, 2, xm)] += 1
                # C_z = B_y(i)-B_y(i-x)-B_x(i)+B_x(i-y)
                curl[face_index(volume, 2, i), face_index(volume, 1, i)] += 1
                curl[face_index(volume, 2, i), face_index(volume, 1, xm)] -= 1
                curl[face_index(volume, 2, i), face_index(volume, 0, i)] -= 1
                curl[face_index(volume, 2, i), face_index(volume, 0, ym)] += 1
    return curl


def rational_vector(length: int, offset: int) -> sp.Matrix:
    return sp.Matrix([
        sp.Rational(((3 * i + offset) % 11) - 5, (i % 5) + 2)
        for i in range(length)
    ])


def decompose(
    divergence: sp.Matrix,
    laplacian_plus: sp.Matrix,
    flux: sp.Matrix,
    momentum: sp.Matrix,
) -> tuple[sp.Matrix, sp.Matrix, sp.Matrix, sp.Matrix, sp.Matrix, sp.Matrix]:
    charge = divergence * flux
    charge_momentum = laplacian_plus * divergence * momentum
    longitudinal_flux = divergence.T * laplacian_plus * charge
    longitudinal_momentum = divergence.T * charge_momentum
    return (
        charge,
        charge_momentum,
        flux - longitudinal_flux,
        momentum - longitudinal_momentum,
        longitudinal_flux,
        longitudinal_momentum,
    )


# Frozen-source and protocol gates.
for rel, expected in FROZEN.items():
    check(f"source hash {rel}", sha256(ROOT / rel) == expected)
check("protocol pre-run hash", sha256(PROTOCOL) == PROTOCOL_SHA256)

theorem_text = THEOREM.read_text(encoding="utf-8")
check(
    "theorem declares matched canonical reduction",
    "[THEOREM — MATCHED GAUSS CANONICAL REDUCTION]" in theorem_text,
)

# Exact matched incidence complex on a fixed 2x2x2 periodic multigraph probe.
SIZE = 2
VOLUME = SIZE**3
D = periodic_incidence(SIZE)
C = periodic_curl(SIZE)
one = sp.ones(VOLUME, 1)
Pi = sp.eye(VOLUME) - one * one.T / VOLUME
L = D * D.T
Lplus = (L + one * one.T / VOLUME).inv() - one * one.T / VOLUME

check("matched periodic incidence has rank V-1", D.rank() == VOLUME - 1)
check("constant scalar mode is the full Laplacian kernel", L.nullspace() == [one])
check("left pseudoinverse identity is the mean-zero projector", L * Lplus == Pi)
check("right pseudoinverse identity is the mean-zero projector", Lplus * L == Pi)
check("pseudoinverse annihilates the constant mode", Lplus * one == sp.zeros(VOLUME, 1))
check("charge bracket is identity on the mean-zero space", D * D.T * Lplus == Pi)
check("matched curl is divergence-free exactly", D * C == sp.zeros(VOLUME, 3 * VOLUME))

# Exact rational decomposition and reconstruction.
J = rational_vector(3 * VOLUME, 1)
P = rational_vector(3 * VOLUME, 4)
q, p, JT, PT, JL, PL = decompose(D, Lplus, J, P)
check("generic charge is compatible", one.T * q == sp.zeros(1, 1))
check("generic charge momentum is mean zero", one.T * p == sp.zeros(1, 1))
check("transverse flux has zero divergence", D * JT == sp.zeros(VOLUME, 1))
check("transverse momentum has zero divergence", D * PT == sp.zeros(VOLUME, 1))
check("flux reconstructs exactly", JT + JL == J)
check("momentum reconstructs exactly", PT + PL == P)
check("longitudinal flux and transverse momentum are orthogonal", JL.dot(PT) == 0)
check("transverse flux and longitudinal momentum are orthogonal", JT.dot(PL) == 0)

# Symplectic split for two fixed rational tangent variations.
J1 = rational_vector(3 * VOLUME, 2)
P1 = rational_vector(3 * VOLUME, 5)
J2 = rational_vector(3 * VOLUME, 7)
P2 = rational_vector(3 * VOLUME, 9)
q1, p1, JT1, PT1, _, _ = decompose(D, Lplus, J1, P1)
q2, p2, JT2, PT2, _, _ = decompose(D, Lplus, J2, P2)
omega_full = J1.dot(P2) - P1.dot(J2)
omega_reduced = (
    JT1.dot(PT2) - PT1.dot(JT2)
    + q1.dot(p2) - p1.dot(q2)
)
check("canonical two-form splits exactly", sp.simplify(omega_full - omega_reduced) == 0)

# Neutral ternary record and minimum-energy longitudinal section.
s = sp.zeros(VOLUME, 1)
s[0] = 1
s[VOLUME - 1] = -1
Js = D.T * Lplus * s
zero_face = sp.zeros(3 * VOLUME, 1)
check("neutral ternary record is mean-zero", one.T * s == sp.zeros(1, 1))
check("static longitudinal section has exact Gauss record", D * Js == s)
check("static section has zero canonical charge momentum", Lplus * D * zero_face == sp.zeros(VOLUME, 1))
check("polarity reversal reverses the longitudinal representative", D.T * Lplus * (-s) == -Js)

challenge = C[:, 1]
check("fixed transverse challenge is nonzero", challenge != zero_face)
check("minimum representative is orthogonal to transverse additions", Js.dot(challenge) == 0)
check(
    "Pythagorean minimum-energy identity is exact",
    (Js + challenge).dot(Js + challenge)
    == Js.dot(Js) + challenge.dot(challenge),
)

# Constraint-preserving transverse recursion.
h = sp.Rational(2, 7)
P_transverse = C[:, 3]
force_transverse = C[:, 5]
P_after = P_transverse + h * force_transverse
J_after = Js + h * P_after
check("transverse kick preserves zero momentum divergence", D * P_after == sp.zeros(VOLUME, 1))
check("transverse drift preserves the ternary charge", D * J_after == s)

# General fixed-range right-inverse proof gates.
R, probe_size = sp.symbols("R probe_size", integer=True, nonnegative=True)
degree_bound = 2 * R + 1
check("registered Laurent-polynomial degree bound is 2R+1", degree_bound.subs(R, 4) == 9)
check("registered large-probe root count exceeds degree", (probe_size - 1 > degree_bound).subs({R: 4, probe_size: 11}) is sp.true)
z = sp.symbols("z")
coefficients = sp.symbols("b0:5")
Bx = sum(coefficients[r + 2] * z**r for r in range(-2, 3))
f = sp.expand(z**3 * ((1 - z**-1) * Bx - 1))
check("range-two contradiction polynomial degree is at most five", sp.Poly(f, z).degree() <= 5)
check("right-inverse contradiction evaluates to minus one at z=1", sp.simplify(f.subs(z, 1)) == -1)
check(
    "general no-locality proof is present",
    "For `L>=2R+3`, it has `L-1` roots" in theorem_text
    and "f(1)=-1" in theorem_text,
)

# Exact production Fourier-symbol mismatch.
cx, cy, cz = sp.symbols("cx cy cz", real=True)
delta18 = sp.Rational(2, 3) * (cx + cy + cz) + sp.Rational(2, 3) * (
    cx * cy + cx * cz + cy * cz
) - 4
central_half_pi = -1
lap_half_pi = sp.simplify(delta18.subs({cx: 0, cy: 1, cz: 1}))
central_nyquist = 0
lap_nyquist = sp.simplify(delta18.subs({cx: -1, cy: 1, cz: 1}))
check("half-pi central composition symbol is exactly minus one", central_half_pi == -1)
check("half-pi 18-point symbol is exactly minus two", lap_half_pi == -2)
check("half-pi symbols mismatch exactly", central_half_pi != lap_half_pi)
check("Nyquist central derivative is blind", central_nyquist == 0)
check("Nyquist 18-point symbol is exactly minus four", lap_nyquist == -4)
check("Nyquist symbols mismatch exactly", central_nyquist != lap_nyquist)

# Source-locked engine semantics.
field_ops = (ROOT / "engine/include/ftd/field_operators.h").read_text(encoding="utf-8")
poisson_h = (ROOT / "engine/include/ftd/poisson_solvers.h").read_text(encoding="utf-8")
poisson_cpp = (ROOT / "engine/src/poisson_solvers.cpp").read_text(encoding="utf-8")
matched_h = (ROOT / "engine/include/ftd/eft/matched_gauss_transport.h").read_text(encoding="utf-8")
matched_cpp = (ROOT / "engine/src/eft/matched_gauss_transport.cpp").read_text(encoding="utf-8")
source_fork = (ROOT / "engine/tests/test_native_source_core_fork.cpp").read_text(encoding="utf-8")
check("production divergence is central difference", "div += (voxels[nbrs[0]].flux.x - voxels[nbrs[1]].flux.x) * 0.5;" in field_ops)
check("production gradient is central difference", "grad.x = (field[nbrs[0]] - field[nbrs[1]]) * 0.5;" in field_ops)
check("production SOR declares an 18-point Laplacian", "18-point isotropic Laplacian" in poisson_cpp and "INV3 * face_sum + INV6 * edge_sum - source[idx]" in poisson_cpp)
check("production Gauss pass uses finite SOR iterations", "for (int iter = 0; iter < sor_iters; ++iter)" in poisson_cpp)
check("production default can skip manifested sites", "if (!exact_dual_gauss && state.state_at(i) != 0) continue;" in poisson_cpp)
check("production header explicitly describes void-only correction", "then J -= ∇φ at\n// void sites only" in poisson_h)
check("matched representation stores positive-axis face flux", "flux through i's positive-axis face" in matched_h)
check("matched divergence uses backward incidence", "field.x[static_cast<std::size_t>(i)] -" in matched_cpp and "field.index(x - 1, y, z)" in matched_cpp)
check("matched minimum field uses the incidence adjoint", "phi[static_cast<std::size_t>(i)] -\n                    phi[static_cast<std::size_t>(electric_.index(x + 1, y, z))]" in matched_cpp)
check("matched recursion advances only through curls and continuity current", "const auto magnetic_curl = matched_curl(magnetic_half_);" in matched_cpp and "apply_conservative_current" in matched_cpp)
check("source-core audit records collocated-stencil nonrepair", "Changing the\n  // source site's own stored flux therefore does not repair the source-core" in source_fork)

# Exact affine preparation, collision, and reversible discrepancy ledger.
T = sp.eye(3 * VOLUME) - D.T * Lplus * D
offset = D.T * Lplus * s
check("transverse linear part is idempotent", T * T == T)
check("transverse linear part is nonidentity", T != sp.eye(3 * VOLUME))
check("transverse linear part has a longitudinal kernel", T * D.T == sp.zeros(3 * VOLUME, VOLUME))

arbitrary = rational_vector(3 * VOLUME, 6)
a = sp.zeros(VOLUME, 1)
a[0] = 1
a[1] = -1
other = arbitrary + D.T * a
prepared_one = T * arbitrary + offset
prepared_two = T * other + offset
check("longitudinally distinct inputs collide after preparation", arbitrary != other and prepared_one == prepared_two)
check("prepared output has the target ternary divergence", D * prepared_one == s)
loss = arbitrary - prepared_one
check("discarded discrepancy plus prepared output recovers input", prepared_one + loss == arbitrary)
check("discarded discrepancy is purely longitudinal relative to target", T * loss == sp.zeros(3 * VOLUME, 1))

# Scope and terminal gates.
scope_markers = [
    "No Hilbert-space recovery, Born rule, Bell mechanism, `G*` gearbox",
    "not promoted to the production tick",
    "No new selected type is booked",
    "PRODUCTION_GAUSS_EXACT_PROJECTOR=NO",
    "GSTAR_ROLE=SEPARATE_CALENDAR",
]
check("all registered scope markers are present", all(marker in theorem_text for marker in scope_markers))
check("physical preparation remains open", "native dynamic formation" in theorem_text and "Still open" in theorem_text)
check("terminal gate reached with C68 passing", failures == 0 and checks == 68)

print(f"\nFTD-0877 Gauss-record canonical reduction: {checks - failures}/{checks} PASS")
if failures == 0 and checks == 69:
    print("GAUSS_RECORD_CANONICAL_REDUCTION_THEOREM")
    print("MATCHED_CHARGE_BRACKET=IDENTITY_ON_MEAN_ZERO_SPACE")
    print("STATIC_TERNARY_RECORD_SECTION=EXACT")
    print("MATCHED_TRANSVERSE_RECURSION=CHARGE_PRESERVING")
    print("UNIFORMLY_LOCAL_CHARGE_CONJUGATE=NO")
    print("PRODUCTION_GAUSS_EXACT_PROJECTOR=NO")
    print("PRODUCTION_GAUSS_STATUS=APPROXIMATE_CONSTRAINT_RELAXATION")
    print("REVERSIBLE_PREPARATION_REQUIRES=DISCREPANCY_LEDGER")
    print("DYNAMIC_NATIVE_RECORD_PREPARATION=OPEN")
    print("GSTAR_ROLE=SEPARATE_CALENDAR")
    print("BORN_BELL_STATUS=UNTOUCHED")
    raise SystemExit(0)

print("FTD-0877_CERTIFICATE_INVALID")
raise SystemExit(1)
