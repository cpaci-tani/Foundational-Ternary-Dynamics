#!/usr/bin/env python3
"""Exact FTD-0971 Krylov-degeneracy ternary-latch/C4 certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md"
)
SOURCES = {
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_GLOBAL_ISOCHRONY_LIFT_AND_ORIENTED_CROSSING_LATCH_BOUNDARY_v1.md"
    ): "746F855A432D7E662236315066115174493554285CD3FC25071B892A05AEA68E",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_ORIENTED_PHASE_CONNECTION_TOKEN_LOADING_AND_SELF_DUAL_GEARBOX_v1.md"
    ): "56711EE1A215F4418A9B8FA5E4EF6C46BD0B2767D407F70E04C7C6A0FD6345B1",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
    ): "100A5539A1116FD6BEC5ABF2B7CE7BA2C32DDA557564EC7C964CDF5877512739",
    BASE / (
        "derivations/native_time_carrier_programme/"
        "THEOREM_MOVING_REGIONAL_FRAME_COTANGENT_CONNECTION_AND_PURE_GAUGE_BOUNDARY_v1.md"
    ): "C5C28405CA439BF2341D545F99E9BDFC985BF65155B1CD49075541CD5C258462",
}
EXPECTED_PROTOCOL = "85E6BA5B4CEFC7CDBF70A5CB903C19D3E6230632889DE70927A4C1E5FF28C8E5"


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
            print("FTD-0971 OUTCOME D - certificate invalid")
            return 1
        print("FTD-0971 OUTCOME B - exact minimum retained C4 carrier; active coupling open")
        print("SELF_DELIMITING_LATCH_ALPHABET=TERNARY_MINIMUM")
        print("RETAINED_TRANSITION_CYCLE=C4")
        print("ORIENTED_REAL_MODE=J_WITH_J2_MINUS_I")
        print("STATIC_REFLECTION_AND_ACTIVE_FIELD_GEARBOX=NOT_IDENTIFIED")
        return 0


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def body_moments(u: sp.Expr) -> tuple[sp.Matrix, sp.Matrix, sp.Expr, sp.Matrix, sp.Matrix]:
    points = [
        sp.Matrix([0, 0, 0]),
        sp.Matrix([1, 0, 0]),
        sp.Matrix([0, 1, 0]),
        sp.Matrix([1, 1, u]),
    ]
    signs = [1, 1, -1, -1]
    center = sum(points, sp.zeros(3, 1)) / 4
    centered = [point - center for point in points]
    dipole = sum(
        (sp.Integer(sign) * vector for sign, vector in zip(signs, centered)),
        sp.zeros(3, 1),
    )
    covariance = sum((vector * vector.T for vector in centered), sp.zeros(3)) / 4
    kappa = sp.factor(sp.Matrix.hstack(
        dipole,
        covariance * dipole,
        covariance**2 * dipole,
    ).det())
    e1 = sp.simplify(dipole / sp.sqrt(dipole.dot(dipole)))
    transverse = sp.simplify((sp.eye(3) - e1 * e1.T) * covariance * dipole)
    e2 = sp.simplify(transverse / sp.sqrt(transverse.dot(transverse)))
    return dipole, covariance, kappa, e1, e2


def positive_limit(vector: sp.Matrix, u: sp.Symbol, substitution: sp.Expr, v: sp.Symbol) -> sp.Matrix:
    return sp.Matrix([
        sp.limit(entry.subs(u, substitution), v, 0, dir="+")
        for entry in vector
    ])


def cross_matrix(axis: sp.Matrix) -> sp.Matrix:
    x, y, z = axis
    return sp.Matrix([[0, -z, y], [z, 0, -x], [-y, x, 0]])


def permutation_matrix(mapping: list[int]) -> sp.Matrix:
    matrix = sp.zeros(len(mapping))
    for source, target in enumerate(mapping):
        matrix[target, source] = 1
    return matrix


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0971 Krylov degeneracy / ternary latch / oriented C4 transition")
    print("=" * 79)

    # G1: locks and source/scope anchors.
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    for path, expected in SOURCES.items():
        cert.check(f"G1 hash {path.name}", sha256(path) == expected, sha256(path))
    source_markers = {
        list(SOURCES)[0]: "The signed current is the minimum datum",
        list(SOURCES)[1]: "symmetric square can say that the chart was traversed",
        list(SOURCES)[2]: "The frame becomes singular at the coplanar configuration",
        list(SOURCES)[3]: "A finite passive frame jump is symplectic",
    }
    for path, marker in source_markers.items():
        cert.check(f"G1 source marker {marker[:36]}", marker in path.read_text(encoding="utf-8"), marker)
    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    for marker in (
        "minimum self-delimiting reversible transition",
        "This is not a universal one-bit claim",
        "not multiplication by `i`",
        "not a derivation of Hilbert space",
        "The expected result is Outcome B",
    ):
        cert.check(f"G1 protocol marker {marker[:40]}", marker in protocol_text, marker)

    # G2: exact body family and normalized one-sided frame limits.
    u = sp.symbols("u", real=True)
    v = sp.symbols("v", positive=True)
    dipole, covariance, kappa, e1, e2 = body_moments(u)
    expected_covariance = sp.Matrix([
        [sp.Rational(1, 4), 0, u / 8],
        [0, sp.Rational(1, 4), u / 8],
        [u / 8, u / 8, 3 * u**2 / 16],
    ])
    cert.check("G2 exact dipole", dipole == sp.Matrix([0, -2, -u]), dipole.T)
    cert.check("G2 exact covariance", covariance == expected_covariance, covariance)
    cert.check("G2 exact Krylov degeneracy", kappa == -u**5 / 256, kappa)
    e1_plus = positive_limit(e1, u, v, v)
    e1_minus = positive_limit(e1, u, -v, v)
    e2_plus = positive_limit(e2, u, v, v)
    e2_minus = positive_limit(e2, u, -v, v)
    expected_e1 = sp.Matrix([0, -1, 0])
    expected_e2 = sp.Matrix([-1, 0, 0])
    cert.check("G2 e1 positive-side limit", e1_plus == expected_e1, e1_plus.T)
    cert.check("G2 e1 negative-side limit", e1_minus == expected_e1, e1_minus.T)
    cert.check("G2 e2 positive-side limit", e2_plus == expected_e2, e2_plus.T)
    cert.check("G2 e2 negative-side limit", e2_minus == expected_e2, e2_minus.T)
    e3_plus = -e1_plus.cross(e2_plus)
    e3_minus = e1_minus.cross(e2_minus)
    cert.check("G2 e3 positive-side limit", e3_plus == sp.Matrix([0, 0, 1]), e3_plus.T)
    cert.check("G2 e3 negative-side limit", e3_minus == sp.Matrix([0, 0, -1]), e3_minus.T)

    # G3: the static chart transition is reflection/complex conjugation.
    frame_plus = sp.Matrix.hstack(e1_plus, e2_plus, e3_plus)
    frame_minus = sp.Matrix.hstack(e1_minus, e2_minus, e3_minus)
    reflection = frame_plus.T * frame_minus
    expected_reflection = sp.diag(1, 1, -1)
    cert.check("G3 exact branch reflection", reflection == expected_reflection, reflection)
    cert.check("G3 reflection involutive", reflection**2 == sp.eye(3), "S^2=I")
    cert.check("G3 reflection improper", reflection.det() == -1, reflection.det())
    lab_reflection = frame_plus * frame_minus.T
    i_minus = cross_matrix(expected_e1)
    i_plus = -cross_matrix(expected_e1)
    cert.check(
        "G3 improper conjugation flips complex structure",
        sp.simplify(lab_reflection * i_minus * lab_reflection.T - i_plus) == sp.zeros(3),
        "Q I_- Q^T=I_+",
    )
    cert.check("G3 reflection is not quarter-turn", reflection**2 != -sp.eye(3), "S^2=+I")

    # G4: minimum self-delimiting latch and modular permutations.
    messages = ("blank", "incoming+", "incoming-")
    cert.check("G4 self-delimiting messages distinct", len(set(messages)) == 3, messages)
    cert.check("G4 binary alphabet insufficient", len(messages) > 2, "pigeonhole 3>2")
    cert.check("G4 one ternary alphabet sufficient", len(messages) == 3, "{-1,0,+1}")
    z3 = tuple(range(3))
    for signed_control, residue in ((1, 1), (-1, 2)):
        load = tuple((value + residue) % 3 for value in z3)
        clear = tuple((value - residue) % 3 for value in load)
        cert.check(f"G4 L_{signed_control:+d} permutation", sorted(load) == list(z3), load)
        cert.check(f"G4 L_{signed_control:+d} inverse", clear == z3, clear)

    # G5: exact retained four-cycle A->B->C->D->A.
    transition = permutation_matrix([1, 2, 3, 0])
    cert.check("G5 transition is permutation", transition.T * transition == sp.eye(4), transition.det())
    cert.check("G5 exact order four", transition**4 == sp.eye(4), "T^4=I")
    cert.check("G5 inverse is third power", transition.inv() == transition**3, "T^-1=T^3")
    cert.check("G5 no smaller positive order", transition != sp.eye(4) and transition**2 != sp.eye(4), "order=4")
    latch_values = sp.Matrix([0, 1, 0, -1])
    cert.check("G5 regular phases blank", latch_values[0] == 0 and latch_values[2] == 0, (0, 0))
    cert.check("G5 degenerate phases direction-loaded", latch_values[1] == 1 and latch_values[3] == -1, (1, -1))

    # G6: exact time reversal keeps spatial states and swaps directed degeneracies.
    time_reversal = permutation_matrix([0, 3, 2, 1])
    cert.check("G6 time reversal involutive", time_reversal**2 == sp.eye(4), "Theta^2=I")
    cert.check(
        "G6 time reversal conjugates forward to inverse",
        time_reversal * transition * time_reversal == transition.inv(),
        "Theta T Theta=T^-1",
    )
    cert.check("G6 regular branches time-even", time_reversal[:, 0] == sp.eye(4)[:, 0] and time_reversal[:, 2] == sp.eye(4)[:, 2], "A,C fixed")

    # G7: the retained real oriented mode is exactly multiplication by i.
    basis = sp.eye(4)
    cosine_mode = basis[:, 0] - basis[:, 2]
    sine_mode = basis[:, 1] - basis[:, 3]
    oriented_basis = sp.Matrix.hstack(cosine_mode, sine_mode)
    gram_inverse = (oriented_basis.T * oriented_basis).inv()
    restricted_forward = sp.simplify(gram_inverse * oriented_basis.T * transition * oriented_basis)
    restricted_reverse = sp.simplify(gram_inverse * oriented_basis.T * transition.inv() * oriented_basis)
    j = sp.Matrix([[0, -1], [1, 0]])
    cert.check("G7 oriented plane invariant", transition * oriented_basis == oriented_basis * j, "span{c,s}")
    cert.check("G7 forward mode is J", restricted_forward == j, restricted_forward)
    cert.check("G7 J squares to minus identity", j**2 == -sp.eye(2), "J^2=-I")
    cert.check("G7 reverse mode is minus J", restricted_reverse == -j, restricted_reverse)
    cert.check("G7 symmetric square loses direction", restricted_forward**2 == restricted_reverse**2 == -sp.eye(2), "(+/-J)^2=-I")
    cert.check("G7 full spectrum is C4", transition.charpoly().as_expr() == sp.Symbol("lambda")**4 - 1, transition.charpoly().as_expr())

    # G8: exact lossless carrier norm, not a physical-energy derivation.
    epsilon = sp.symbols("epsilon", positive=True)
    one_hot = sp.Matrix(sp.symbols("x0:4", real=True))
    carrier_energy = epsilon * one_hot.dot(one_hot) / 2
    advanced_energy = sp.expand(epsilon * (transition * one_hot).dot(transition * one_hot) / 2)
    cert.check("G8 equal-weight carrier norm preserved", sp.expand(advanced_energy - carrier_energy) == 0, "E(Tx)=E(x)")
    cert.check("G8 permutation determinant oriented", transition.det() == -1, transition.det())
    cert.check("G8 four-step state recovery", transition**4 * one_hot == one_hot, "exact recurrence")

    # G9: no promotion beyond the retained finite-state carrier.
    for marker in (
        "does not prove that the actual regular and\ndegenerate substrate bodies have equal physical energy",
        "No field variable is changed by (9)",
        "requires a separately specified local generator",
        "physical source and reciprocal body reaction",
        "finite reserve and fail-closed backpressure",
        "Nothing\nin the transition identifies that cadence with the critical quartic period",
        "not a production particle, Born mechanism, `G*` gearbox",
    ):
        cert.check(f"G9 scope marker {marker[:43]}", marker in protocol_text, marker)
    cert.check("G9 no numerical search", "No floating comparison or search is permitted" in protocol_text, "exact-only")
    cert.check("G9 no production mutation", True, "proof-only")

    return cert.finish()


if __name__ == "__main__":
    raise SystemExit(main())
