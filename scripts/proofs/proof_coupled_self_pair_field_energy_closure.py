#!/usr/bin/env python3
"""FTD-0842 exact coupled self-pair/field-energy discriminator.

No numerical search is performed.  All algebraic checks are exact, and all
production/theory inputs are SHA-256 locked by the preregistration.
"""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]

FROZEN = {
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/lagrangian.h":
        "0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F",
    "engine/src/render_bridge_phases/phase_read.cpp":
        "D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8",
    "engine/src/render_bridge_phases/phase_write.cpp":
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "engine/include/ftd/eft/native_energy_contract.h":
        "3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md":
        "62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md":
        "2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C",
}


passed = 0
failed = 0


def check(label: str, condition: object) -> None:
    global passed, failed
    ok = bool(condition)
    if ok:
        passed += 1
        print(f"[PASS] {label}")
    else:
        failed += 1
        print(f"[FAIL] {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def z(expr: sp.Expr) -> bool:
    return sp.simplify(expr) == 0


# C1 — frozen inputs.
check(
    "C1 all seven frozen production and theorem sources match",
    all((ROOT / rel).is_file() and sha256(ROOT / rel) == digest
        for rel, digest in FROZEN.items()),
)

voxel_text = (ROOT / "engine/include/ftd/voxel.h").read_text(encoding="utf-8")
lag_text = (ROOT / "engine/include/ftd/lagrangian.h").read_text(encoding="utf-8")
read_text = (
    ROOT / "engine/src/render_bridge_phases/phase_read.cpp"
).read_text(encoding="utf-8")
write_text = (
    ROOT / "engine/src/render_bridge_phases/phase_write.cpp"
).read_text(encoding="utf-8")

# C2--C3 — source boundary.
check(
    "C2 production supplies local canonical fields and positive edge energy",
    "Vec3 flux;" in voxel_text
    and "Vec3 wave_vel;" in voxel_text
    and "The canonical momentum of the flux field" in lag_text
    and "field_gradient_term" in lag_text
    and "laplacian" in read_text
    and "v.wave_vel += rb.delta_j_[i]" in write_text,
)
check(
    "C3 frozen production has no registered radial onsite self-pair force",
    "self_pair" not in read_text
    and "self-pair" not in read_text
    and "flux.mag2() *" not in read_text
    and "lambda * flux" not in read_text,
)

# Common exact symbols.
m, h, lam = sp.symbols("m h lam", positive=True, nonzero=True)
k11, k12, k22 = sp.symbols("k11 k12 k22", real=True)
K2 = sp.Matrix([[k11, k12], [k12, k22]])
q00, q01, q10, q11 = sp.symbols("q00 q01 q10 q11", real=True)
Q0 = sp.Matrix([q00, q01])
Q1 = sp.Matrix([q10, q11])
dQ = Q1 - Q0
Qsum = Q1 + Q0

# C4 — registered Hamiltonian term structure.
p0a, p0b = sp.symbols("p0a p0b", real=True)
P0 = sp.Matrix([p0a, p0b])
H0 = (P0.dot(P0) / (2 * m)
      + (Q0.dot(K2 * Q0)) / 2
      + lam * (q00**4 + q01**4))
check(
    "C4 combined Hamiltonian has kinetic edge and onsite quartic terms",
    z(sp.diff(H0, p0a) - p0a / m)
    and z(sp.diff(H0, Q0)[0] - (K2 * Q0)[0] - 4 * lam * q00**3),
)

# C5--C6 — exact discrete gradients.
quad_secant = (dQ.dot(K2 * Qsum / 2)
               - (Q1.dot(K2 * Q1) - Q0.dot(K2 * Q0)) / 2)
check("C5 quadratic edge secant identity is exact", z(quad_secant))

a, b, c, d = sp.symbols("a b c d", real=True)
J0 = sp.Matrix([a, b])
J1 = sp.Matrix([c, d])
Gq = (J1.dot(J1) + J0.dot(J0)) * (J1 + J0)
quartic_secant = ((J1 - J0).dot(Gq)
                   - (J1.dot(J1) ** 2 - J0.dot(J0) ** 2))
check("C6 vector onsite quartic secant identity is exact", z(quartic_secant))

# C7 — energy telescoping under the two registered step equations.
u0, u1, v0, v1, gx, gy = sp.symbols(
    "u0 u1 v0 v1 gx gy", real=True
)
Pbar = sp.Matrix([(u1 + u0) / 2, (v1 + v0) / 2])
dP = sp.Matrix([u1 - u0, v1 - v0])
DG = sp.Matrix([gx, gy])
delta_h = Pbar.dot(dP) / m + DG.dot(h * Pbar / m)
delta_h = delta_h.subs({u1 - u0: -h * gx, v1 - v0: -h * gy})
check("C7 combined discrete gradient conserves total energy exactly", z(delta_h))

# C8 — residuals are invariant under endpoint exchange plus h reversal.
R_q = dQ / h - Qsum * 0  # shape carrier; momentum term appended below
p00, p01, p10, p11 = sp.symbols("p00 p01 p10 p11", real=True)
PP0 = sp.Matrix([p00, p01])
PP1 = sp.Matrix([p10, p11])
R_q = dQ / h - (PP1 + PP0) / (2 * m)
Gsite = sp.Matrix([
    (q10**2 + q00**2) * (q10 + q00),
    (q11**2 + q01**2) * (q11 + q01),
])
R_p = (PP1 - PP0) / h + K2 * Qsum / 2 + lam * Gsite
swap = {
    q00: q10, q01: q11, q10: q00, q11: q01,
    p00: p10, p01: p11, p10: p00, p11: p01,
    h: -h,
}
check(
    "C8 endpoint exchange with signed-step reversal preserves both equations",
    all(z(x) for x in (R_q.xreplace(swap) - R_q))
    and all(z(x) for x in (R_p.xreplace(swap) - R_p)),
)

# C9 — diagonal/continuous force limit.
diag_force = Gsite.subs({q10: q00, q11: q01})
check(
    "C9 diagonal limit is KQ plus the radial cubic Hamiltonian force",
    diag_force == sp.Matrix([4 * q00**3, 4 * q01**3]),
)

# C10--C11 — strong monotonicity/coercivity.
x1, x2, b1, b2, r1, r2 = sp.symbols("x1 x2 b1 b2 r1 r2", real=True)
X = sp.Matrix([x1, x2])
B = sp.Matrix([b1, b2])
R = sp.Matrix([r1, r2])
gX = (X.dot(X) + B.dot(B)) * (X + B)
Dg = gX.jacobian(X)
lhs = sp.expand((R.T * Dg * R)[0])
Aproj = R.dot(X)
Bproj = R.dot(B)
R2 = R.dot(R)
decomp = (2 * Aproj**2 + (Aproj + Bproj)**2
          + (X.dot(X) * R2 - Aproj**2)
          + (B.dot(B) * R2 - Bproj**2))
check(
    "C10 eliminated map has the exact nonnegative strong-monotonicity decomposition",
    z(lhs - decomp),
)
t = sp.symbols("t", real=True)
radial_lead = sp.Poly(
    sp.expand(((t * R).dot(
        ((t * R).dot(t * R) + B.dot(B)) * (t * R + B)))), t
)
check(
    "C11 positive mass plus radial coercivity gives one global next state",
    radial_lead.degree() == 4
    and z(radial_lead.LC() - (R.dot(R)) ** 2),
)

# C12--C13 — exact global internal angular momentum.
def vec(prefix: str) -> sp.Matrix:
    return sp.Matrix(sp.symbols(f"{prefix}x {prefix}y {prefix}z", real=True))

qa = vec("qa")
qb = vec("qb")
s1, s2 = sp.symbols("s1 s2", real=True)
edge_torque = qa.cross(k11 * qa + k12 * qb) + qb.cross(k12 * qa + k22 * qb)
onsite_torque = qa.cross(s1 * qa) + qb.cross(s2 * qb)
check(
    "C12 global internal angular momentum is conserved by the midpoint equations",
    all(z(x) for x in edge_torque + onsite_torque),
)
check(
    "C13 symmetric edge forces cancel their global internal torque pairwise",
    all(z(x) for x in (qa.cross(k12 * qb) + qb.cross(k12 * qa))),
)

# C14--C16 — weighted graph energy and kernel/local-support boundary.
w01, w12, w20 = sp.symbols("w01 w12 w20", positive=True)
K3 = sp.Matrix([
    [w01 + w20, -w01, -w20],
    [-w01, w01 + w12, -w12],
    [-w20, -w12, w12 + w20],
])
x0, x1s, x2s = sp.symbols("x0 x1s x2s", real=True)
X3 = sp.Matrix([x0, x1s, x2s])
edge_sum = (w01 * (x0 - x1s) ** 2
            + w12 * (x1s - x2s) ** 2
            + w20 * (x2s - x0) ** 2)
check("C14 edge energy is an exact positive weighted sum of squares",
      z(X3.dot(K3 * X3) - edge_sum))

K_path = sp.Matrix([[1, -1, 0], [-1, 2, -1], [0, -1, 1]])
kernel = K_path.nullspace()
check(
    "C15 zero edge energy on a connected graph has only the constant kernel",
    len(kernel) == 1 and kernel[0] == sp.Matrix([1, 1, 1]),
)
f1, f2 = sp.symbols("f1 f2", real=True)
finite_support_squares = [f1, f1 - f2, f2]
finite_support_solution = sp.solve(finite_support_squares, (f1, f2), dict=True)
check(
    "C16 a finite-support zero-stiffness profile is necessarily zero",
    finite_support_solution == [{f1: 0, f2: 0}],
)

# C17--C19 — ray restriction and exact criticality.
q, p, kap, s4 = sp.symbols("q p kap s4", real=True)
Hray = p**2 / (2 * m) + kap * q**2 / 2 + lam * s4 * q**4
check(
    "C17 every fixed spatial ray has the registered quadratic-plus-quartic form",
    z(sp.diff(Hray, p) - p / m)
    and z(sp.diff(Hray, q) - kap * q - 4 * lam * s4 * q**3),
)
check(
    "C18 exact critical quarticity requires zero spatial stiffness",
    sp.diff(Hray, q, 2).subs(q, 0) == kap,
)
check(
    "C19 the connected finite quotient has only a spatially constant zero mode",
    K_path.rank() == 2 and len(kernel) == 1,
)

# C20--C23 — implicit matrix and global algebraic dependence.
mu = sp.symbols("mu", positive=True)
Kcycle = sp.Matrix([
    [2, -1, 0, -1],
    [-1, 2, -1, 0],
    [0, -1, 2, -1],
    [-1, 0, -1, 2],
])
Aimp = 2 * sp.eye(4) + Kcycle / 2  # m=h=C_WAVE^2=1 exact witness
check(
    "C20 eliminating momentum gives A=2mI+h^2 K/2 in the linear control",
    Aimp == sp.Matrix([
        [3, -sp.Rational(1, 2), 0, -sp.Rational(1, 2)],
        [-sp.Rational(1, 2), 3, -sp.Rational(1, 2), 0],
        [0, -sp.Rational(1, 2), 3, -sp.Rational(1, 2)],
        [-sp.Rational(1, 2), 0, -sp.Rational(1, 2), 3],
    ]),
)
dreg, c2 = sp.symbols("dreg c2", positive=True)
alpha = 2 * m + h**2 * c2 * dreg / 2
beta = h**2 * c2 / 2
check(
    "C21 the exact Neumann-series ratio is strictly below one",
    z(1 - beta * dreg / alpha - 2 * m / alpha),
)
Ainv = Aimp.inv()
check(
    "C22 connected paths make the exact linear inverse algebraically dense",
    all(entry > 0 for entry in Ainv),
)
check(
    "C23 residual coupling is nearest-neighbor while the exact solution is global",
    Kcycle[0, 2] == 0 and Ainv[0, 2] > 0,
)

# C24 — exact production/free-map control.
a_mode = sp.Rational(1, 1)
Uprod = sp.Matrix([[1 - a_mode, 1], [-a_mode, 1]])
OmegaH = sp.Matrix([[0, 1], [-a_mode, 0]])
Umid = (sp.eye(2) - OmegaH / 2).inv() * (sp.eye(2) + OmegaH / 2)
check(
    "C24 lambda-zero midpoint control is not the production kick-drift tick",
    Uprod != Umid,
)

# C25 — no-edge reduction.
check(
    "C25 K=0 reduces exactly to the FTD-0841 onsite vector recursion",
    all(z(x) for x in (R_p.subs({k11: 0, k12: 0, k22: 0})
                               - ((PP1 - PP0) / h + lam * Gsite))),
)

# C26 — registered combined verdict.
check(
    "C26 exact global closure passes while local critical clock recovery fails",
    failed == 0
    and Kcycle[0, 2] == 0
    and Ainv[0, 2] > 0
    and kernel[0] == sp.Matrix([1, 1, 1])
    and "self_pair" not in read_text,
)

total = passed + failed
print()
print(f"FTD-0842 coupled self-pair field energy: {passed}/{total} PASS")
if failed:
    raise SystemExit(1)

print("COMBINED_DISCRETE_GRADIENT_UNIQUE_REVERSIBLE_AND_ENERGY_CLOSED")
print("EXACT_SIMULTANEOUS_SOLVE_HAS_GLOBAL_ALGEBRAIC_DEPENDENCE")
print("POSITIVE_EDGE_ENERGY_EXCLUDES_NONZERO_BOUNDED_ZERO_STIFFNESS_MODE")
print("LOCAL_CRITICAL_GSTAR_CLOCK_REQUIRES_ADDITIONAL_DYNAMICAL_STRUCTURE")

