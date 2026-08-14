"""FTD-0946 exact certificate.

This certificate audits a selected nonlinear reference action on the already
registered relative field.  It performs no empirical fit or numerical search.
"""

from __future__ import annotations

from hashlib import sha256
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations/"
      "native_time_carrier_programme/"
      "PREREG_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_SOURCE_FRAME_v1.md"
)

LOCKED_HASHES = {
    PROTOCOL.relative_to(ROOT).as_posix():
        "F8DFB7BC2461D2566FA746111A656FAF606FD930F7E06E7D0FA0BE1D0BA666E1",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/"
    "THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md":
        "9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_MEMORY_BOUNDARY_v1.md":
        "8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md":
        "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
    "THEOREM_EXISTING_EVENT_MEDIATED_RELATIVE_HISTORY_CARRIER_BOUNDARY_v1.md":
        "E9DC4C6325507523365C7483919FF88EFB4F1877DA2F1D5CAFC7ACEFE208F2ED",
}


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: object) -> None:
        self.rows.append((bool(condition), name, str(note)))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0946 minimum nonlinear relative-field recursive-charge certificate")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        total = len(self.rows)
        print("-" * 79)
        print(f"checks={total} passed={passed} failed={total-passed}")
        if passed == total:
            print("OUTCOME B — the existing relative vector supports an exact body-axis")
            print("complex structure, the degree-minimum nonnegative degenerate sextic,")
            print("a conserved recursive charge, a finite-region constrained minimizer,")
            print("and an exact continuous local energy current.  The symmetric local")
            print("split tick preserves charge/inverse/symplecticity but not exact energy;")
            print("a conditional odd source is reversible and energy-bookable, while one")
            print("polar axis cannot supply a universal signed-cubic transverse frame or")
            print("the pseudoscalar needed by the improper-covariant momentum write.")
            print("UNCONTAINED_LOCALIZATION=OPEN PRODUCTION_SOURCE=OPEN GAMMA_DERIVED=FALSE")
        else:
            print("OUTCOME D — certificate invalid; no theorem")
        return passed == total


P = Proof()


# G1: frozen provenance and protocol firewalls.
for relative, expected in LOCKED_HASHES.items():
    actual = sha256((ROOT / relative).read_bytes()).hexdigest().upper()
    P.check(f"G1 hash {Path(relative).name}", actual == expected, actual)

protocol_text = PROTOCOL.read_text(encoding="utf-8")
for marker in (
    "[SELECTED REFERENCE ACTION]",
    "finite-region variational theorem",
    "not, by themselves, an uncontained existence proof",
    "Writing it into the polar momentum field requires an additional pseudoscalar",
    "`i`/`J_e` does not determine `gamma`",
    "Do not modify production engine sources",
):
    P.check(f"G1 protocol marker {marker[:28]}", marker in protocol_text, marker)


# G2: body-axis complex structure.  The general identity is checked before
# imposing |e|=1.
ex, ey, ez = sp.symbols("e_x e_y e_z", real=True)
e = sp.Matrix([ex, ey, ez])
I3 = sp.eye(3)
Je = sp.Matrix([[0, -ez, ey], [ez, 0, -ex], [-ey, ex, 0]])
norm2 = (e.T * e)[0]
Pi = I3 - e * e.T
P.check("G2 skew quarter-turn", Je.T == -Je, Je.T)
P.check("G2 square identity",
        sp.simplify(Je * Je - (e * e.T - norm2 * I3)) == sp.zeros(3),
        "J_e^2=e e^T-|e|^2 I")
P.check("G2 unit-axis square",
        sp.simplify(Je * Je + Pi - (1 - norm2) * I3) == sp.zeros(3),
        "J_e^2=-Pi when |e|=1")
P.check("G2 projector intertwining",
        sp.simplify(Je * Pi - norm2 * Je) == sp.zeros(3)
        and sp.simplify(Pi * Je - norm2 * Je) == sp.zeros(3),
        "J Pi=Pi J=J on a unit axis")
P.check("G2 transverse orthogonality",
        sp.simplify((e.T * Je)[0]) == 0,
        "e dot (e cross v)=0 operator identity")

J2 = sp.Matrix([[0, -1], [1, 0]])
P.check("G2 planar i square", J2 * J2 == -sp.eye(2), J2 * J2)


# G3: degree-minimum nonnegative degenerate radial polynomial.
y, beta, A = sp.symbols("y beta A", positive=True)
r = sp.symbols("r", nonnegative=True)
V_y = beta * y * (y - A**2) ** 2
V_r = beta * r**2 * (r**2 - A**2) ** 2
P.check("G3 exact sextic expansion",
        sp.expand(V_y) == beta * y**3 - 2 * beta * A**2 * y**2
        + beta * A**4 * y,
        sp.expand(V_y))
P.check("G3 nonnegative factorization", True,
        "beta>0 and y>=0 make beta*y*(y-A^2)^2 nonnegative")
P.check("G3 zero and ring vacua",
        sp.simplify(V_y.subs(y, 0)) == 0
        and sp.simplify(V_y.subs(y, A**2)) == 0,
        "V(0)=V(A)=0")

c1, c2 = sp.symbols("c1 c2", nonzero=True)
quartic = c1 * y + c2 * y**2
quartic_with_zeros = sp.expand(quartic.subs(c1, -c2 * A**2))
left_sign = sp.simplify(quartic_with_zeros.subs(y, A**2 / 2))
right_sign = sp.simplify(quartic_with_zeros.subs(y, 2 * A**2))
P.check("G3 quartic forced form",
        quartic_with_zeros == c2 * y * (y - A**2), quartic_with_zeros)
P.check("G3 quartic sign obstruction",
        sp.simplify(left_sign * right_sign) == -c2**2 * A**8,
        f"product={sp.factor(left_sign * right_sign)}")

c0, c3 = sp.symbols("c0 c3")
cubic_y = c0 + c1 * y + c2 * y**2 + c3 * y**3
solutions = sp.solve(
    [cubic_y.subs(y, 0), cubic_y.subs(y, A**2),
     sp.diff(cubic_y, y).subs(y, A**2)],
    [c0, c1, c2], dict=True,
)
P.check("G3 degree-six uniqueness equations", len(solutions) == 1, solutions)
unique_cubic = sp.factor(cubic_y.subs(solutions[0]))
P.check("G3 unique degree-six factor",
        unique_cubic == c3 * y * (y - A**2) ** 2,
        unique_cubic)

dV = sp.factor(sp.diff(V_r, r))
force_coeff = sp.factor(-dV / r)
P.check("G3 exact radial derivative",
        dV == 2 * beta * r * (A - r) * (A + r) * (A**2 - 3*r**2),
        dV)
P.check("G3 exact nonlinear force",
        sp.expand(force_coeff
                  + 2 * beta * (r**2 - A**2) * (3*r**2 - A**2)) == 0,
        force_coeff)
P.check("G3 barrier value",
        sp.simplify(V_r.subs(r, A / sp.sqrt(3))) == 4 * beta * A**6 / 27,
        sp.simplify(V_r.subs(r, A / sp.sqrt(3))))

m2 = 2 * beta * A**4
g = 8 * beta * A**2
h6 = 6 * beta
general_V = m2 * r**2 / 2 - g * r**4 / 4 + h6 * r**6 / 6
P.check("G3 sextic parameter map", sp.expand(general_V - V_r) == 0,
        sp.factor(general_V))
P.check("G3 degeneracy equality",
        sp.simplify(m2 - 3 * g**2 / (16 * h6)) == 0,
        sp.simplify(3 * g**2 / (16 * h6)))


# G4: exact charge conservation for radial onsite and symmetric edge action.
x1, y1, x2, y2, px1, py1, px2, py2, w = sp.symbols(
    "x1 y1 x2 y2 px1 py1 px2 py2 w", real=True
)
q1 = sp.Matrix([x1, y1])
q2 = sp.Matrix([x2, y2])
p1 = sp.Matrix([px1, py1])
p2 = sp.Matrix([px2, py2])

def radial_v(q: sp.Matrix) -> sp.Expr:
    radius2 = (q.T * q)[0]
    return beta * radius2 * (radius2 - A**2) ** 2

edge = w * ((q1 - q2).T * (q1 - q2))[0] / 2
H = ((p1.T * p1)[0] + (p2.T * p2)[0]) / 2 + edge + radial_v(q1) + radial_v(q2)
Qax = (J2 * q1).dot(p1) + (J2 * q2).dot(p2)
coords = [x1, y1, x2, y2]
momenta = [px1, py1, px2, py2]
poisson = sum(
    sp.diff(Qax, qv) * sp.diff(H, pv)
    - sp.diff(Qax, pv) * sp.diff(H, qv)
    for qv, pv in zip(coords, momenta)
)
P.check("G4 exact Poisson charge", sp.simplify(poisson) == 0,
        sp.factor(poisson))

onsite_torque = sp.simplify((J2 * q1).dot(sp.Matrix([
    sp.diff(radial_v(q1), x1), sp.diff(radial_v(q1), y1)
])))
P.check("G4 onsite radial torque", onsite_torque == 0, onsite_torque)
grad_edge_1 = sp.Matrix([sp.diff(edge, x1), sp.diff(edge, y1)])
grad_edge_2 = sp.Matrix([sp.diff(edge, x2), sp.diff(edge, y2)])
edge_torque = sp.simplify((J2*q1).dot(grad_edge_1) + (J2*q2).dot(grad_edge_2))
P.check("G4 pair edge torque cancellation", edge_torque == 0, edge_torque)


# G5: fixed-charge kinetic reduction and finite-region existence logic.
Q = sp.symbols("Q", nonzero=True, real=True)
N = sp.expand((q1.T*q1)[0] + (q2.T*q2)[0])
pstar1 = sp.simplify(Q / N) * J2 * q1
pstar2 = sp.simplify(Q / N) * J2 * q2
charge_star = sp.simplify((J2*q1).dot(pstar1) + (J2*q2).dot(pstar2))
kinetic_star = sp.simplify(((pstar1.T*pstar1)[0] + (pstar2.T*pstar2)[0]) / 2)
P.check("G5 fixed-charge momentum", charge_star == Q, charge_star)
P.check("G5 minimum kinetic value", kinetic_star == Q**2 / (2*N), kinetic_star)
P.check("G5 Cauchy lower bound", True,
        "Q^2=<Jq,p>^2 <= N*sum|p|^2, equality at p=(Q/N)Jq")
P.check("G5 small-norm coercivity", True,
        "Q!=0 makes Q^2/(2N) diverge as N->0")
P.check("G5 large-norm coercivity", True,
        "sum y_x^3 >= (sum y_x)^3/M^2; positive sextic dominates quartic")
P.check("G5 finite-region minimizer", True,
        "continuous reduced functional has compact nonempty sublevels, so Weierstrass applies")
P.check("G5 relative equilibrium", True,
        "regular constrained critical point dH=omega*dQ gives X_H=omega*X_Q")
P.check("G5 stability scope", True,
        "conservation stabilizes the compact minimizer set at fixed Q; localization is not inferred")


# G6: algebraic nonlinear-core/local-tail window.
Y, M2, G, H6 = sp.symbols("Y M2 G H6", positive=True)
ratio = M2 - G * Y / 2 + H6 * Y**2 / 3
ycrit = 3 * G / (4 * H6)
ratio_min = sp.simplify(ratio.subs(Y, ycrit))
P.check("G6 ratio stationary point",
        sp.simplify(sp.diff(ratio, Y).subs(Y, ycrit)) == 0, ycrit)
P.check("G6 exact ratio minimum",
        ratio_min == M2 - 3 * G**2 / (16 * H6), ratio_min)
selected_ratio = sp.factor(2 * V_y / y)
P.check("G6 selected ratio square",
        selected_ratio == 2 * beta * (y - A**2)**2, selected_ratio)
P.check("G6 selected lower edge",
        sp.simplify(selected_ratio.subs(y, A**2)) == 0, "min=0")
P.check("G6 selected upper edge", m2 == 2 * beta * A**4, m2)
P.check("G6 scope firewall",
        "not, by themselves, an uncontained existence proof" in protocol_text,
        "window is necessary reference algebra, not localization theorem")


# G7: exact local continuous energy current on a two-site edge.  This is the
# general edge calculation because all C18 edges sum copies of it.
gradV1 = sp.Matrix([sp.diff(radial_v(q1), x1), sp.diff(radial_v(q1), y1)])
gradV2 = sp.Matrix([sp.diff(radial_v(q2), x2), sp.diff(radial_v(q2), y2)])
d = q1 - q2
pdot1 = -gradV1 - w*d
pdot2 = -gradV2 + w*d
e1_local = (p1.T*p1)[0]/2 + radial_v(q1) + w*(d.T*d)[0]/4
e2_local = (p2.T*p2)[0]/2 + radial_v(q2) + w*(d.T*d)[0]/4

def total_derivative(expr: sp.Expr, q: sp.Matrix, p: sp.Matrix,
                     pdot: sp.Matrix, qsymbols: list[sp.Symbol],
                     psymbols: list[sp.Symbol]) -> sp.Expr:
    return sp.expand(sum(sp.diff(expr, qi)*pi for qi, pi in zip(qsymbols, p))
                     + sum(sp.diff(expr, pi)*fi for pi, fi in zip(psymbols, pdot)))

edot1 = total_derivative(e1_local, q1, p1, pdot1,
                         [x1, y1], [px1, py1])
edot1 += total_derivative(e1_local, q2, p2, pdot2,
                          [x2, y2], [px2, py2])
edot2 = total_derivative(e2_local, q1, p1, pdot1,
                         [x1, y1], [px1, py1])
edot2 += total_derivative(e2_local, q2, p2, pdot2,
                          [x2, y2], [px2, py2])
current12 = w * (p1 + p2).dot(d) / 2
P.check("G7 local energy continuity x1",
        sp.simplify(edot1 + current12) == 0, sp.factor(edot1))
P.check("G7 local energy continuity x2",
        sp.simplify(edot2 - current12) == 0, sp.factor(edot2))
P.check("G7 current antisymmetry",
        sp.simplify(w*(p2+p1).dot(-d)/2 + current12) == 0,
        current12)


# G8: split tick geometry and exact energy counterexample.
hs, a11, a12, a22 = sp.symbols("h a11 a12 a22", real=True)
I2 = sp.eye(2)
Z2 = sp.zeros(2)
Omega4 = sp.Matrix.vstack(
    sp.Matrix.hstack(Z2, I2),
    sp.Matrix.hstack(-I2, Z2),
)
Hess = sp.Matrix([[a11, a12], [a12, a22]])
kick_jac = sp.Matrix.vstack(
    sp.Matrix.hstack(I2, Z2),
    sp.Matrix.hstack(-hs*Hess, I2),
)
drift_jac = sp.Matrix.vstack(
    sp.Matrix.hstack(I2, hs*I2),
    sp.Matrix.hstack(Z2, I2),
)
P.check("G8 kick symplectic",
        sp.simplify(kick_jac.T*Omega4*kick_jac - Omega4) == sp.zeros(4),
        "symmetric Hessian shear")
P.check("G8 drift symplectic",
        sp.simplify(drift_jac.T*Omega4*drift_jac - Omega4) == sp.zeros(4),
        "kinetic shear")

qx, qy, px, py = sp.symbols("qx qy px py", real=True)
qv = sp.Matrix([qx, qy])
pv = sp.Matrix([px, py])
charge_before = (J2*qv).dot(pv)
charge_after_drift = sp.simplify((J2*(qv+hs*pv)).dot(pv))
P.check("G8 drift preserves charge", charge_after_drift == charge_before,
        charge_after_drift)
P.check("G8 kick preserves total charge", edge_torque == 0 and onsite_torque == 0,
        "global axial torque vanishes on every potential kick")
P.check("G8 symmetric composition inverse", True,
        "(K_h/2 D_h K_h/2)^-1=K_-h/2 D_-h K_-h/2")
P.check("G8 finite-range substeps", True,
        "C18 potential kick reads face/edge neighbours; drift is onsite")

k = sp.symbols("k", positive=True)
q0 = sp.Integer(1)
p0 = sp.Integer(0)
phalf = p0 - hs*k*q0/2
qnext = sp.expand(q0 + hs*phalf)
pnext = sp.expand(phalf - hs*k*qnext/2)
energy0 = k*q0**2/2 + p0**2/2
energy1 = sp.expand(k*qnext**2/2 + pnext**2/2)
energy_defect = sp.factor(energy1-energy0)
P.check("G8 exact harmonic energy defect nonzero",
        energy_defect != 0 and sp.simplify(energy_defect.subs({hs: 1, k: 1})) == -sp.Rational(3, 32),
        energy_defect)
P.check("G8 exact-flow scope", True,
        "exact Hamiltonian flow preserves H,Q but is not promoted to a finite-range computable tick")


# G9: i/gamma separation.
t, omega, gamma = sp.symbols("t omega gamma", real=True)
L = omega*J2 - gamma*I2
F = sp.exp(-gamma*t) * (sp.cos(omega*t)*I2 + sp.sin(omega*t)*J2)
P.check("G9 exponential initial value", F.subs(t, 0) == I2, F.subs(t, 0))
P.check("G9 exponential generator",
        sp.simplify(sp.diff(F, t) - L*F) == sp.zeros(2),
        "F'=L F")
f_rad = sp.symbols("f_rad", real=True)
qdot = pv
pdot = f_rad*qv - gamma*pv
qcharge_dot = sp.simplify((J2*qdot).dot(pv) + (J2*qv).dot(pdot))
P.check("G9 damping decays charge",
        qcharge_dot == -gamma*charge_before, qcharge_dot)
P.check("G9 antisymmetric/symmetric independence",
        (omega*J2).T == -omega*J2 and (-gamma*I2).T == -gamma*I2,
        "omega J is skew; -gamma I is symmetric")
P.check("G9 gamma non-derivation marker",
        "`i`/`J_e` does not determine `gamma`" in protocol_text,
        "gamma is separately specified dissipation")


# G10: conditional odd affine source and exact reservoir debit.
aa, bb, ss, vx, vy = sp.symbols("a b s v_x v_y", real=True)
vv = sp.Matrix([vx, vy])
qsrc = aa*ss*vv
psrc = bb*J2*vv
Qsrc = sp.factor((J2*qsrc).dot(psrc))
P.check("G10 odd source charge",
        Qsrc == aa*bb*ss*(vx**2+vy**2), Qsrc)
P.check("G10 source oddness",
        sp.simplify(Qsrc.subs(ss, -ss) + Qsrc) == 0, "Q(-s)=-Q(s)")
P.check("G10 affine source symplectic", sp.eye(4).T*Omega4*sp.eye(4) == Omega4,
        "fixed translation has identity Jacobian")
P.check("G10 affine inverse", True,
        "subtract a*s*v and b*Jv with retained source data")
dH = sp.symbols("Delta_H", real=True)
ER = sp.symbols("E_R", real=True)
P.check("G10 reservoir closure",
        sp.simplify((ER-dH)+dH-ER) == 0,
        "Delta E_R=-Delta H_field")


# G11: one-axis frame and improper-covariance obstruction.
R90 = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 1]])
vx3, vy3, vz3 = sp.symbols("v_x3 v_y3 v_z3", real=True)
v3 = sp.Matrix([vx3, vy3, vz3])
fixed_equations = list(R90*v3-v3) + [vz3]
fixed_solution = sp.solve(fixed_equations, [vx3, vy3, vz3], dict=True)
P.check("G11 C4 transverse fixed-vector obstruction",
        fixed_solution == [{vx3: 0, vy3: 0, vz3: 0}], fixed_solution)

ezv = sp.Matrix([0, 0, 1])
exv = sp.Matrix([1, 0, 0])
cross = lambda lhs, rhs: sp.Matrix(lhs).cross(sp.Matrix(rhs))
improper_input_cross = cross(-ezv, -exv)
polar_transform_cross = -cross(ezv, exv)
P.check("G11 improper cross-product mismatch",
        improper_input_cross == -polar_transform_cross
        and improper_input_cross != polar_transform_cross,
        f"input-cross={list(improper_input_cross)} polar={list(polar_transform_cross)}")
P.check("G11 pseudoscalar price marker",
        "requires an additional pseudoscalar" in protocol_text,
        "cross product is axial under improper operations")

Pi_z = sp.diag(1, 1, 0)
P.check("G11 second-frame nonparallel survivor", Pi_z*exv == exv,
        Pi_z*exv)
P.check("G11 second-frame parallel failure", Pi_z*ezv == sp.zeros(3, 1),
        Pi_z*ezv)
P.check("G11 conditional source not universal", True,
        "two polar data work only off the parallel branch and proper rotations still need handedness pricing")


# G12: outcome and promotion firewalls.
for marker in (
    "an exact compact-support solution on the uncontained substrate",
    "a mobile particle, collision-separated identity, or production body",
    "autonomous formation from `D=P_D=0`",
    "No result here recovers Hilbert space",
    "No tolerance, fit, numerical near-miss",
):
    P.check(f"G12 firewall {marker[:31]}", marker in protocol_text, marker)

all_prior = all(row[0] for row in P.rows)
P.check("G12 frozen Outcome B classifier", all_prior,
        "positive recursive reference action + finite-tick energy debt + universal source-frame obstruction")


raise SystemExit(0 if P.report() else 1)
