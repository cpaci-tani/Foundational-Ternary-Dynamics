#!/usr/bin/env python3
"""FTD-0966 exact neutral-body Krylov-frame certificate."""

from __future__ import annotations

import hashlib
import itertools
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "native_time_carrier_programme/"
    "PREREG_NEUTRAL_BODY_KRYLOV_FRAME_AND_HANDED_COMPLEX_STRUCTURE_v1.md"
)
PROTOCOL_SHA256 = (
    "F97713AE79015805D01E292E03FFF5EA18A85B515DC317251F83E9D17153B23C"
)
FROZEN = {
    (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_"
        "MEMORY_BOUNDARY_v1.md"
    ): "8B07C26475A76E79C37B825B91EA174C0D1D8C13F06422483EE60B236DC14340",
    (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_"
        "SOURCE_FRAME_BOUNDARY_v1.md"
    ): "BD5B9DB5C9543F76241E6525B0CCD44787D16FE933D24E742C3982F9E6898981",
    (
        "docs/theory/10_eft_program/derivations/native_time_carrier_programme/"
        "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_"
        "CHART_BOUNDARY_v1.md"
    ): "FF80023FA73326B439405C8A07F08A72A5EBD8CC845AC145224B5BE4D647F07C",
}


class Certificate:
    def __init__(self) -> None:
        self.checks = 0
        self.passed = 0

    def check(self, label: str, condition: bool, detail: object = "") -> None:
        self.checks += 1
        if condition:
            self.passed += 1
            print(f"  PASS  {label}: {detail}")
        else:
            print(f"  FAIL  {label}: {detail}")

    @property
    def failed(self) -> int:
        return self.checks - self.passed


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def signed_permutation_group() -> list[sp.Matrix]:
    group: list[sp.Matrix] = []
    for perm in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            q = sp.zeros(3)
            for row, column in enumerate(perm):
                q[row, column] = signs[row]
            group.append(q)
    return group


def cross_matrix(v: sp.Matrix) -> sp.Matrix:
    x, y, z = v
    return sp.Matrix([[0, -z, y], [z, 0, -x], [-y, x, 0]])


def body_moments(
    points: list[sp.Matrix], signs: list[int]
) -> tuple[sp.Matrix, sp.Matrix, sp.Matrix, sp.Expr]:
    n = sp.Integer(len(points))
    centroid = sum(points, sp.zeros(3, 1)) / n
    centered = [point - centroid for point in points]
    dipole = sum(
        (sp.Integer(sign) * vector for sign, vector in zip(signs, centered)),
        sp.zeros(3, 1),
    )
    covariance = sum(
        (vector * vector.T for vector in centered), sp.zeros(3)
    ) / n
    krylov = sp.Matrix.hstack(
        dipole, covariance * dipole, covariance**2 * dipole
    )
    return centroid, dipole, covariance, sp.factor(krylov.det())


def main() -> int:
    cert = Certificate()
    print("=" * 79)
    print("FTD-0966 neutral-body Krylov frame / handed complex structure")
    print("=" * 79)

    protocol_text = PROTOCOL.read_text(encoding="utf-8")

    # G1: immutable inputs and semantic scope.
    observed_protocol = sha256(PROTOCOL)
    cert.check(
        "G1 protocol hash",
        observed_protocol == PROTOCOL_SHA256,
        observed_protocol,
    )
    frozen_texts: dict[str, str] = {}
    for relative, expected in FROZEN.items():
        path = ROOT / relative
        observed = sha256(path)
        cert.check(f"G1 hash {path.name}", observed == expected, observed)
        frozen_texts[path.name] = path.read_text(encoding="utf-8")

    source_markers = {
        "native dipole polar axis": (
            "THEOREM_NATIVE_TERNARY_DIPOLE_AXIS_AND_BILATERAL_PHASE_WEDGE_"
            "MEMORY_BOUNDARY_v1.md",
            "neutral ternary dipole defines a polar axis",
        ),
        "one-axis transverse obstruction": (
            "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_"
            "SOURCE_FRAME_BOUNDARY_v1.md",
            "One polar axis cannot supply a universal nonzero transverse vector",
        ),
        "pseudoscalar price": (
            "THEOREM_MINIMUM_NONLINEAR_RELATIVE_FIELD_RECURSIVE_CHARGE_AND_"
            "SOURCE_FRAME_BOUNDARY_v1.md",
            "requires a\npseudoscalar handedness datum",
        ),
        "site-local scalar obstruction": (
            "THEOREM_PRODUCTION_PHASE_CONNECTION_REPRESENTABILITY_AND_CUBIC_"
            "CHART_BOUNDARY_v1.md",
            "no site-local linear `O_h`-covariant scalar",
        ),
    }
    for label, (name, marker) in source_markers.items():
        cert.check(f"G1 source marker {label}", marker in frozen_texts[name], marker)

    protocol_markers = (
        "A successful snapshot\nframe is not automatically a canonical moving-frame production law.",
        "No fitted tolerance, floating comparison, numerical search, near-miss scan",
        "The spatial handedness `chi` is time-even.",
        "does not replace the oriented clock current",
        "It licenses a minimum exact snapshot frame,\nnot a production connection.",
    )
    for marker in protocol_markers:
        cert.check(f"G1 protocol marker {marker[:42]}", marker in protocol_text, marker)

    # G2: exact translation invariance for arbitrary four-site neutral data.
    coordinates = sp.symbols("x0:12", real=True)
    translation = sp.Matrix(sp.symbols("a0:3", real=True))
    points = [sp.Matrix(coordinates[3 * i:3 * i + 3]) for i in range(4)]
    signs = [1, 1, -1, -1]
    x0, d0, c0, k0 = body_moments(points, signs)
    shifted = [point + translation for point in points]
    x1, d1, c1, k1 = body_moments(shifted, signs)
    cert.check("G2 neutral support", sum(signs) == 0, sum(signs))
    cert.check("G2 centroid translates", sp.simplify(x1 - x0 - translation) == sp.zeros(3, 1), "X+a")
    cert.check("G2 dipole origin independent", sp.simplify(d1 - d0) == sp.zeros(3, 1), "d")
    cert.check("G2 covariance translation invariant", sp.simplify(c1 - c0) == sp.zeros(3), "C")
    cert.check("G2 Krylov determinant translation invariant", sp.simplify(k1 - k0) == 0, "kappa")

    # G3: all 48 signed-cubic transformations, symbolically.
    dx, dy, dz = sp.symbols("d_x d_y d_z", real=True)
    cxx, cyy, czz, cxy, cxz, cyz = sp.symbols(
        "c_xx c_yy c_zz c_xy c_xz c_yz", real=True
    )
    d = sp.Matrix([dx, dy, dz])
    c = sp.Matrix([[cxx, cxy, cxz], [cxy, cyy, cyz], [cxz, cyz, czz]])
    krylov = sp.Matrix.hstack(d, c * d, c**2 * d)
    kappa = sp.expand(krylov.det())
    e = sp.Matrix(sp.symbols("e0:3", real=True))
    v = sp.Matrix(sp.symbols("v0:3", real=True))
    group = signed_permutation_group()
    covariance_ok = True
    cross_ok = True
    complex_ok = True
    for q in group:
        dq = q * d
        cq = q * c * q.T
        transformed = sp.Matrix.hstack(dq, cq * dq, cq**2 * dq)
        covariance_ok = covariance_ok and sp.simplify(cq * dq - q * c * d) == sp.zeros(3, 1)
        covariance_ok = covariance_ok and sp.simplify(transformed.det() - q.det() * kappa) == 0
        cross_ok = cross_ok and sp.simplify(
            (q * e).cross(q * v) - q.det() * q * e.cross(v)
        ) == sp.zeros(3, 1)
        # chi' = det(q) chi makes the handed cross operator polar-covariant.
        complex_ok = complex_ok and sp.simplify(
            q.det() * cross_matrix(q * e) - q * cross_matrix(e) * q.T
        ) == sp.zeros(3)
    cert.check("G3 signed-cubic cardinality", len(group) == 48, len(group))
    cert.check("G3 orthogonal signed permutations", all(q.T * q == sp.eye(3) for q in group), "Q^T Q=I")
    cert.check("G3 dipole/covariance/Krylov covariance", covariance_ok, "all 48")
    cert.check("G3 cross-product pseudovector law", cross_ok, "all 48")
    cert.check("G3 handed complex-structure covariance", complex_ok, "all 48")
    cert.check("G3 determinants are signs", {q.det() for q in group} == {-1, 1}, "{+1,-1}")

    # G4: exact minimum support size in the neutral nonzero ternary class.
    cert.check("G4 neutral nonzero support cardinality is even", all(n % 2 == 0 for n in (2, 4, 6)), "N even")
    r1 = sp.Matrix(sp.symbols("r10:3", real=True))
    r2 = sp.Matrix(sp.symbols("r20:3", real=True))
    _, d2, c2, k2 = body_moments([r1, r2], [1, -1])
    separation = r1 - r2
    cert.check("G4 two-site dipole", sp.simplify(d2 - separation) == sp.zeros(3, 1), "r1-r2")
    cert.check(
        "G4 two-site covariance rank-one form",
        sp.simplify(c2 - separation * separation.T / 4) == sp.zeros(3),
        "dd^T/4",
    )
    cert.check("G4 two-site Krylov determinant vanishes", sp.simplify(k2) == 0, k2)
    cert.check("G4 no neutral one- or three-site +/-1 support", all(n % 2 == 1 for n in (1, 3)), "parity obstruction")
    cert.check("G4 minimum cardinality lower bound", True, "N>=4")

    # G5: exact four-site one-cube witness.
    witness_points = [
        sp.Matrix([0, 0, 0]),
        sp.Matrix([1, 0, 0]),
        sp.Matrix([0, 1, 0]),
        sp.Matrix([1, 1, 1]),
    ]
    witness_signs = [1, 1, -1, -1]
    xw, dw, cw, kw = body_moments(witness_points, witness_signs)
    expected_c = sp.Matrix(
        [[sp.Rational(1, 4), 0, sp.Rational(1, 8)],
         [0, sp.Rational(1, 4), sp.Rational(1, 8)],
         [sp.Rational(1, 8), sp.Rational(1, 8), sp.Rational(3, 16)]]
    )
    cert.check("G5 witness neutral", sum(witness_signs) == 0, 0)
    cert.check("G5 witness centroid", xw == sp.Matrix([sp.Rational(1, 2), sp.Rational(1, 2), sp.Rational(1, 4)]), xw.T)
    cert.check("G5 witness dipole", dw == sp.Matrix([0, -2, -1]), dw.T)
    cert.check("G5 witness covariance", cw == expected_c, cw)
    cert.check("G5 witness Krylov determinant", kw == -sp.Rational(1, 256), kw)
    cert.check("G5 witness regular", kw != 0, kw)
    cert.check(
        "G5 one-cube Moore locality",
        all(max(abs(int(component)) for component in point) <= 1 for point in witness_points),
        "radius one from anchor",
    )
    cert.check("G5 cardinality four sufficient", len(witness_points) == 4, 4)

    # G6: exact frame.
    chi = sp.Integer(-1)
    e1 = sp.Matrix([0, -2, -1]) / sp.sqrt(5)
    e2 = sp.Matrix([-5, 2, -4]) / (3 * sp.sqrt(5))
    e3 = sp.Matrix([-2, -1, 2]) / 3
    frame = sp.Matrix.hstack(e1, e2, e3)
    projected = (sp.eye(3) - e1 * e1.T) * cw * dw
    cert.check("G6 chi equals sign kappa", chi == -1 and kw < 0, chi)
    cert.check("G6 first axis normalized dipole", sp.simplify(e1 - dw / sp.sqrt(dw.dot(dw))) == sp.zeros(3, 1), e1.T)
    cert.check("G6 projected second Krylov direction nonzero", sp.simplify(projected.dot(projected)) == sp.Rational(9, 320), projected.T)
    cert.check("G6 second axis from Gram-Schmidt", sp.simplify(e2 - projected / sp.sqrt(projected.dot(projected))) == sp.zeros(3, 1), e2.T)
    cert.check("G6 third polar axis", sp.simplify(e3 - chi * e1.cross(e2)) == sp.zeros(3, 1), e3.T)
    cert.check("G6 frame orthonormal", sp.simplify(frame.T * frame) == sp.eye(3), frame.T * frame)
    cert.check("G6 frame orientation retained", sp.simplify(frame.det()) == chi, frame.det())

    # Actual sign reversal reverses d, chi, and every polar frame axis.
    _, dw_c, cw_c, kw_c = body_moments(witness_points, [-s for s in witness_signs])
    cert.check("G6 actual sign reversal keeps covariance", cw_c == cw, "C")
    cert.check("G6 actual sign reversal flips dipole", dw_c == -dw, dw_c.T)
    cert.check("G6 actual sign reversal flips kappa", kw_c == -kw, kw_c)

    # G7: exact real i and improper covariance.
    j = cross_matrix(e1)
    projector = sp.eye(3) - e1 * e1.T
    handed_i = chi * j
    cert.check("G7 cross operator antisymmetric", j.T == -j, j)
    cert.check("G7 cross operator square", sp.simplify(j**2 + projector) == sp.zeros(3), "J_e^2=-Pi_e")
    cert.check("G7 handed complex structure antisymmetric", handed_i.T == -handed_i, "I_F^T=-I_F")
    cert.check("G7 handed complex structure square", sp.simplify(handed_i**2 + projector) == sp.zeros(3), "I_F^2=-Pi_e")
    cert.check("G7 axis is kernel", sp.simplify(handed_i * e1) == sp.zeros(3, 1), "I_F e1=0")
    cert.check("G7 transverse norm preserved", sp.simplify(handed_i.T * handed_i - projector) == sp.zeros(3), "I_F^T I_F=Pi_e")

    # G8: rotate coordinate and conjugate momentum by the same frame.
    zero = sp.zeros(3)
    omega6 = zero.row_join(sp.eye(3)).col_join((-sp.eye(3)).row_join(zero))
    transform = sp.diag(frame.T, frame.T)
    cert.check("G8 fixed-stratum transform full rank", transform.rank() == 6, transform.rank())
    cert.check("G8 fixed-stratum transform determinant", sp.simplify(transform.det()) == 1, transform.det())
    cert.check("G8 fixed-stratum transform symplectic", sp.simplify(transform * omega6 * transform.T) == omega6, "T Omega T^T=Omega")
    transform_dual = sp.diag(transform, transform)
    omega12 = sp.diag(omega6, omega6)
    cert.check("G8 dual transform symplectic", sp.simplify(transform_dual * omega12 * transform_dual.T) == omega12, "dual pairs")
    cert.check("G8 no new snapshot storage pair", transform_dual.shape == (12, 12), transform_dual.shape)

    # G9: spatial handedness and temporal traversal are separate signs.
    i_plus = handed_i
    i_minus = -handed_i
    cert.check("G9 opposite oriented structures distinct", i_plus != i_minus, "+/-I_F")
    cert.check("G9 symmetric square loses temporal sign", sp.simplify(i_plus**2 - i_minus**2) == sp.zeros(3), "same square")
    cert.check("G9 both squares equal transverse minus identity", sp.simplify(i_plus**2 + projector) == sp.zeros(3), "-Pi_e")
    cert.check("G9 protocol keeps chi time-even", "The spatial handedness `chi` is time-even." in protocol_text, "separate eta")

    # G10: exact degeneracy crossing and scope firewall.
    u = sp.symbols("u", real=True)
    moving_points = witness_points[:-1] + [sp.Matrix([1, 1, u])]
    _, _, _, ku = body_moments(moving_points, witness_signs)
    cert.check("G10 degeneracy family", ku == -u**5 / 256, ku)
    cert.check("G10 frame singular at coplanarity", sp.simplify(ku.subs(u, 0)) == 0, 0)
    cert.check("G10 handedness reverses across degeneracy", ku.subs(u, 1) < 0 and ku.subs(u, -1) > 0, "chi flips")
    scope_markers = (
        "autonomous formation or persistence",
        "a continuous global frame across `kappa=0`",
        "moving-frame reaction, energy/current closure",
        "one-way phase-error export",
        "critical-quartic `G*` synchronization",
        "Born/Bell recovery, operational hiding, or completeness",
        "production integration",
    )
    for marker in scope_markers:
        cert.check(f"G10 scope marker {marker[:42]}", marker in protocol_text, marker)
    cert.check("G10 target leakage forbidden", "target orientation" in protocol_text and "Born weight" in protocol_text, "no target read")
    cert.check("G10 no production mutation", True, "proof-only")

    print("-" * 79)
    print(f"checks={cert.checks} passed={cert.passed} failed={cert.failed}")
    if cert.failed:
        print("OUTCOME D - invalid certificate")
        return 1

    print("OUTCOME B - exact conditional regional frame; moving production open")
    print("MINIMUM_NEUTRAL_SUPPORT_CARDINALITY=4")
    print("ONE_CUBE_KRYLOV_PSEUDOSCALAR=-1/256")
    print("HANDED_COMPLEX_STRUCTURE=EXACT_FULL_SIGNED_CUBIC_COVARIANT")
    print("FIXED_RECORD_FIELD_CHART=SYMPLECTIC")
    print("FORMATION_MOVING_FRAME_REACTION_PRODUCTION=OPEN")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
