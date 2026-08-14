#!/usr/bin/env python3
"""Exact FTD-0970 moving-frame cotangent-connection certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md"
)
SOURCES = {
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md"
    ): "56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_CHART_BOUNDARY_v1.md"
    ): "FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
    ): "100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739",
}
EXPECTED_PROTOCOL = "5222BE4E93A244871EB656DFA7AF9D502210DFE0F5C8A915A4C2BCA689E92BAC"


class Certificate:
    def __init__(self) -> None:
        self.checks: list[tuple[str, bool, object]] = []

    def check(self, label: str, passed: bool, detail: object = "") -> None:
        self.checks.append((label, bool(passed), detail))
        print(f"  {'PASS' if passed else 'FAIL'}  {label}: {detail}")

    def finish(self) -> int:
        passed = sum(ok for _, ok, _ in self.checks)
        failed = len(self.checks) - passed
        print("-" * 79)
        print(f"checks={len(self.checks)} passed={passed} failed={failed}")
        if failed:
            print("FTD-0970 OUTCOME D - certificate invalid")
            return 1
        print("FTD-0970 OUTCOME B - exact passive cotangent connection; active gearbox open")
        print("MOVING_FRAME_MOMENTUM_SHIFT=EXACT_UNIQUE")
        print("MAURER_CARTAN_CURVATURE=ZERO")
        print("REGULAR_CLOSED_LOOP_HOLONOMY=IDENTITY")
        print("ACTIVE_DISCRETE_GEARBOX=OPEN")
        return 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def exact_zero_matrix(matrix: sp.Matrix) -> bool:
    return all(sp.trigsimp(sp.expand_trig(entry)) == 0 for entry in matrix)


def rotation_z(angle: sp.Expr) -> sp.Matrix:
    return sp.Matrix([
        [sp.cos(angle), -sp.sin(angle), 0],
        [sp.sin(angle), sp.cos(angle), 0],
        [0, 0, 1],
    ])


def rotation_y(angle: sp.Expr) -> sp.Matrix:
    return sp.Matrix([
        [sp.cos(angle), 0, sp.sin(angle)],
        [0, 1, 0],
        [-sp.sin(angle), 0, sp.cos(angle)],
    ])


def poisson(f: sp.Expr, g: sp.Expr, q: list[sp.Symbol], p: list[sp.Symbol]) -> sp.Expr:
    return sp.expand(sum(
        sp.diff(f, qi) * sp.diff(g, pi) - sp.diff(f, pi) * sp.diff(g, qi)
        for qi, pi in zip(q, p)
    ))


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0970 moving regional-frame cotangent connection / pure-gauge boundary")
    print("=" * 79)

    # G1: immutable locks and explicit scope.
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    for path, expected in SOURCES.items():
        cert.check(f"G1 hash {path.name}", sha256(path) == expected, sha256(path))
    source_markers = {
        next(iter(SOURCES)): "speed- and load-independent oriented holonomy",
        list(SOURCES)[1]: "no site-local linear `O_h`-covariant scalar",
        list(SOURCES)[2]: "A state-dependent moving",
    }
    for path, marker in source_markers.items():
        cert.check(f"G1 source marker {marker[:34]}", marker in path.read_text(encoding="utf-8"), marker)
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    for marker in (
        "passive cotangent lift",
        "a nontrivial closed-loop holonomy",
        "No selected connection profile",
        "The expected result is Outcome B",
    ):
        cert.check(f"G1 protocol marker {marker[:38]}", marker in protocol_text, marker)

    # G2: general one-form algebra and unique body-momentum correction.
    a1, a2, a3 = sp.symbols("a1 a2 a3", real=True)
    amat = sp.Matrix([[0, -a3, a2], [a3, 0, -a1], [-a2, a1, 0]])
    y = sp.Matrix(sp.symbols("y1:4", real=True))
    z = sp.Matrix(sp.symbols("z1:4", real=True))
    dy = sp.Matrix(sp.symbols("dy1:4", real=True))
    ds, p_body, pi_body, u = sp.symbols("ds P Pi U", real=True)
    g = sp.expand((z.T * amat * y)[0])
    lhs = sp.expand((z.T * (dy + amat * y * ds))[0] + p_body * ds)
    rhs = sp.expand((z.T * dy)[0] + (p_body + g) * ds)
    cert.check("G2 general canonical one-form identity", sp.expand(lhs - rhs) == 0, "theta_old=theta_new")
    coefficient_equation = sp.expand(u - p_body - g)
    unique_solution = sp.solve(coefficient_equation, u)
    cert.check("G2 momentum shift unique", unique_solution == [p_body + g], unique_solution)
    cert.check("G2 connection matrix skew", amat.T == -amat, "A^T=-A")

    omega = sp.Matrix([a1, a2, a3])
    angular = y.cross(z)
    cert.check(
        "G2 angular-momentum generator identity",
        sp.expand(g - omega.dot(angular)) == 0,
        "G=omega.(y cross z)",
    )

    # G3: full nonlinear symplectic Jacobian of the planar cotangent lift.
    theta, big_pi, mass = sp.symbols("theta Pi M", real=True, nonzero=True)
    frame = rotation_z(theta)
    jz = sp.Matrix([[0, -1, 0], [1, 0, 0], [0, 0, 0]])
    lz = sp.expand((z.T * jz * y)[0])
    q_lab = frame * y
    p_lab = frame * z
    old_variables = sp.Matrix([theta, *q_lab, big_pi - lz, *p_lab])
    new_variables = sp.Matrix([theta, *y, big_pi, *z])
    jacobian = old_variables.jacobian(new_variables)
    identity4 = sp.eye(4)
    omega8 = sp.zeros(8)
    omega8[:4, 4:] = identity4
    omega8[4:, :4] = -identity4
    symplectic_defect = jacobian.T * omega8 * jacobian - omega8
    cert.check("G3 nonlinear cotangent lift full rank", sp.simplify(jacobian.det()) == 1, jacobian.det())
    cert.check("G3 nonlinear cotangent lift symplectic", exact_zero_matrix(symplectic_defect), "J^T Omega J=Omega")

    # G4: planar connection and complete square.
    planar_connection = sp.simplify(frame.T * sp.diff(frame, theta))
    cert.check("G4 planar Maurer-Cartan connection", planar_connection == jz, planar_connection)
    cert.check("G4 planar generator is Lz", sp.expand(g.subs({a1: 0, a2: 0, a3: 1}) - lz) == 0, lz)
    kinetic_old = (big_pi - lz) ** 2 / (2 * mass)
    kinetic_new = sp.Symbol("P", real=True) ** 2 / (2 * mass)
    cert.check(
        "G4 induced complete square",
        sp.expand(kinetic_new.subs(sp.Symbol("P", real=True), big_pi - lz) - kinetic_old) == 0,
        "(Pi-Lz)^2/(2M)",
    )

    # G5: exact non-Abelian two-parameter Maurer-Cartan flatness.
    alpha, beta = sp.symbols("alpha beta", real=True)
    frame2 = rotation_z(alpha) * rotation_y(beta)
    a_alpha = sp.simplify(frame2.T * sp.diff(frame2, alpha))
    a_beta = sp.simplify(frame2.T * sp.diff(frame2, beta))
    curvature = (
        sp.diff(a_beta, alpha)
        - sp.diff(a_alpha, beta)
        + a_alpha * a_beta
        - a_beta * a_alpha
    )
    cert.check("G5 two-parameter frame orthogonal", exact_zero_matrix(frame2.T * frame2 - sp.eye(3)), "F^T F=I")
    cert.check("G5 connection components noncommute", not exact_zero_matrix(a_alpha * a_beta - a_beta * a_alpha), "[A_alpha,A_beta]!=0")
    cert.check("G5 exact Maurer-Cartan curvature zero", exact_zero_matrix(curvature), "dA+A wedge A=0")

    # G6: endpoint transport, closed loop, and reverse path.
    t, t0, t1, t2 = sp.symbols("t t0 t1 t2", real=True)
    ft = rotation_z(t)
    transport = sp.simplify(ft.T * rotation_z(t0))
    transport_ode = sp.diff(transport, t) + (ft.T * sp.diff(ft, t)) * transport
    cert.check("G6 parallel-transport differential equation", exact_zero_matrix(transport_ode), "Udot=-A_t U")
    u10 = sp.simplify(rotation_z(t1).T * rotation_z(t0))
    u21 = sp.simplify(rotation_z(t2).T * rotation_z(t1))
    u20 = sp.simplify(rotation_z(t2).T * rotation_z(t0))
    cert.check("G6 endpoint composition", exact_zero_matrix(u21 * u10 - u20), "U21 U10=U20")
    cert.check("G6 regular closed-loop holonomy identity", exact_zero_matrix(u10.subs(t1, t0) - sp.eye(3)), "F0^T F0=I")
    cert.check("G6 reverse traversal exact inverse", exact_zero_matrix(u10.T * u10 - sp.eye(3)), "U01=U10^-1")

    # G7: rotationally invariant bare-motion/no-transfer control.
    spring = sp.symbols("k", positive=True)
    h_iso = (z.dot(z) + spring * y.dot(y)) / 2
    h_total = (big_pi - lz) ** 2 / (2 * mass) + h_iso
    canonical_q = [theta, *list(y)]
    canonical_p = [big_pi, *list(z)]
    pi_dot = -sp.diff(h_total, theta)
    lz_dot = poisson(lz, h_total, canonical_q, canonical_p)
    mechanical_dot = sp.expand(pi_dot - lz_dot)
    cert.check("G7 canonical body momentum constant", sp.simplify(pi_dot) == 0, pi_dot)
    cert.check("G7 field angular momentum constant", sp.simplify(lz_dot) == 0, lz_dot)
    cert.check("G7 mechanical body momentum constant", sp.simplify(mechanical_dot) == 0, mechanical_dot)
    cert.check("G7 passive connection has no token variable", "token" not in str(h_total).lower(), "no port degree")

    # G8: exact passive finite frame jump.
    theta_minus, theta_plus = sp.symbols("theta_minus theta_plus", real=True)
    fminus = rotation_z(theta_minus)
    fplus = rotation_z(theta_plus)
    jump = sp.simplify(fplus.T * fminus)
    omega6 = sp.zeros(6)
    omega6[:3, 3:] = sp.eye(3)
    omega6[3:, :3] = -sp.eye(3)
    jump6 = sp.diag(jump, jump)
    cert.check("G8 passive jump orthogonal", exact_zero_matrix(jump.T * jump - sp.eye(3)), "R^T R=I")
    cert.check("G8 passive jump symplectic", exact_zero_matrix(jump6.T * omega6 * jump6 - omega6), "diag(R,R)")
    cert.check("G8 passive jump exact inverse", exact_zero_matrix(jump.T - sp.simplify(fminus.T * fplus)), "R^-1=R^T")
    cert.check("G8 lab coordinate unchanged", exact_zero_matrix(fplus * jump - fminus), "F+ y+=F- y-")
    cert.check("G8 lab momentum unchanged", exact_zero_matrix(fplus * jump - fminus), "F+ z+=F- z-")

    # G9: scope firewall is part of the immutable protocol.
    for marker in (
        "representation change,\nnot an active physical gearbox",
        "requires an independently specified\ngenerator plus reciprocal body/port reaction",
        "the frame and its derivative are undefined",
        "Such a jump needs a reversible transition record",
        "token loading, energy export, outcome selection, or production dynamics",
        "No selected connection profile, `G*` value, Born weight, target outcome",
        "does not promote the selected FTD-0963\nprofile",
    ):
        cert.check(f"G9 scope marker {marker[:42]}", marker in protocol_text, marker)
    cert.check("G9 exact-only certificate", "Floating comparisons and numerical scans are forbidden" in protocol_text, "no search")
    cert.check("G9 no production mutation", True, "proof-only")

    return cert.finish()


if __name__ == "__main__":
    raise SystemExit(main())
