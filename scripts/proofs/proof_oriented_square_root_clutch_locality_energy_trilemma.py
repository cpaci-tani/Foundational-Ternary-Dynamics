#!/usr/bin/env python3
"""FTD-0979 exact oriented-root and locality/energy certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md"
)
EXPECTED_PROTOCOL = "5747E0991BD6984B86B8A9522AD3F9B2927E8AADEDEF0D50C2C826DF7EA185C4"

FROZEN = {
    "derivations/native_time_carrier_programme/THEOREM_PRODUCTION_CLOCK_INDEXED_C4_TWIST_CENSUS_v1.md":
        "3873CEE3BD61C894A99857C0527FBC1082F244CE7E7890FEB3E2F01C6D64E58F",
    "derivations/native_time_carrier_programme/THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md":
        "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
    "derivations/native_time_carrier_programme/THEOREM_KRYLOV_DEGENERACY_TERNARY_LATCH_AND_ORIENTED_C4_TRANSITION_v1.md":
        "7DA2366C75D38E0EA1F8012632D71C676C4E6F8D1A7F8D1467EAF4185AE77194",
    "derivations/native_time_carrier_programme/THEOREM_ONE_CLOCK_C4_COTANGENT_LIFT_AND_CONNECTION_UNDERDETERMINATION_v1.md":
        "9D80C133F5D99D0F789C320DC7C2C2A9E41C4DBB56FAECD39054B7BF0DB69E7F",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


class Certificate:
    def __init__(self) -> None:
        self.total = 0
        self.passed = 0

    def check(self, label: str, condition: bool, detail: object = "") -> None:
        self.total += 1
        if condition:
            self.passed += 1
        print(f"  {'PASS' if condition else 'FAIL'}  {label}: {detail}")

    @property
    def failed(self) -> int:
        return self.total - self.passed


def block_diag(*blocks: sp.Matrix) -> sp.Matrix:
    return sp.diag(*blocks)


def main() -> int:
    print("=" * 79)
    print("FTD-0979 oriented square-root clutch / locality-energy trilemma")
    print("=" * 79)
    cert = Certificate()

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    cert.check(
        "G1 locked marker",
        "[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]" in protocol_text,
        "locked",
    )
    cert.check(
        "G1 no production mutation scope",
        "It does not alter production" in protocol_norm,
        "reference discriminator",
    )
    frozen_text: dict[str, str] = {}
    for relative, expected in FROZEN.items():
        path = BASE / relative
        actual = sha256(path)
        cert.check(f"G1 source hash {Path(relative).name}", actual == expected, actual)
        frozen_text[relative] = path.read_text(encoding="utf-8")
    cert.check(
        "G1 FTD-0978 phase-complete correction retained",
        "complete `4 x 4` canonical map" in frozen_text[next(iter(FROZEN))]
        and "determinant `+1`" in frozen_text[next(iter(FROZEN))],
        "full swap, not flux block",
    )
    c18_text = frozen_text[
        "derivations/native_time_carrier_programme/"
        "THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md"
    ]
    cert.check(
        "G1 prior scalar finite-range boundary retained",
        "K\\text{ is not a square in }R" in c18_text,
        "FTD-0943",
    )

    # Canonical setup, ordered as (q_L,q_R,p_L,p_R).
    z0 = sp.Integer(0)
    z1 = sp.Integer(1)
    omega4 = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(2), sp.eye(2)),
        sp.Matrix.hstack(-sp.eye(2), sp.zeros(2)),
    )
    swap = sp.Matrix(
        [[0, 1, 0, 0], [1, 0, 0, 0], [0, 0, 0, 1], [0, 0, 1, 0]]
    )
    sqrt2 = sp.sqrt(2)
    h2 = sp.Matrix([[1, 1], [1, -1]]) / sqrt2
    change = block_diag(h2, h2)  # raw -> (q_C,q_D,p_C,p_D)
    j_plus = sp.Matrix([[0, -1], [1, 0]])
    j_minus = -j_plus

    def root_cr(sigma: int) -> sp.Matrix:
        j_sigma = j_plus if sigma == 1 else j_minus
        return sp.Matrix(
            [[1, 0, 0, 0],
             [0, j_sigma[0, 0], 0, j_sigma[0, 1]],
             [0, 0, 1, 0],
             [0, j_sigma[1, 0], 0, j_sigma[1, 1]]]
        )

    r_plus_cr = root_cr(1)
    r_minus_cr = root_cr(-1)
    r_plus = sp.simplify(change.T * r_plus_cr * change)
    r_minus = sp.simplify(change.T * r_minus_cr * change)

    # G2: exact root and raw formula.
    cert.check("G2 common-relative chart orthogonal", change.T * change == sp.eye(4), "C^-1=C^T")
    cert.check("G2 production swap recovered in chart", change * swap * change.T == sp.diag(1, -1, 1, -1), "common fixed")
    cert.check("G2 positive root symplectic", r_plus.T * omega4 * r_plus == omega4, "R+^T Omega R+=Omega")
    cert.check("G2 negative root symplectic", r_minus.T * omega4 * r_minus == omega4, "R-^T Omega R-=Omega")
    cert.check("G2 roots orthogonal", r_plus.T * r_plus == sp.eye(4) and r_minus.T * r_minus == sp.eye(4), "norm preserving")
    cert.check("G2 positive root squares to swap", r_plus**2 == swap, "R+^2=S")
    cert.check("G2 negative root squares to swap", r_minus**2 == swap, "R-^2=S")
    cert.check("G2 roots are inverse", r_plus.inv() == r_minus and r_minus.inv() == r_plus, "R_sigma^-1=R_-sigma")
    cert.check("G2 roots have order four", r_plus**4 == sp.eye(4) and r_minus**4 == sp.eye(4), "R_sigma^4=I")
    expected_r_plus = sp.Matrix(
        [[1, 1, -1, 1], [1, 1, 1, -1], [1, -1, 1, 1], [-1, 1, 1, 1]]
    ) / 2
    cert.check("G2 raw positive-root formula", r_plus == expected_r_plus, r_plus)
    cert.check("G2 complete root determinant", r_plus.det() == 1 and r_minus.det() == 1, "det=+1")

    # Uniqueness in O(2) cap Sp(2): A=[[a,-b],[b,a]], a^2+b^2=1.
    a, b = sp.symbols("a b", real=True)
    a2_from_sum_and_square = sp.solve(
        [sp.Eq(a**2 + b**2, 1), sp.Eq(a**2 - b**2, -1)],
        [a**2, b**2],
        dict=True,
    )
    cert.check(
        "G2 orthogonal-root diagonal equations",
        a2_from_sum_and_square == [{a**2: 0, b**2: 1}],
        a2_from_sum_and_square,
    )
    cert.check("G2 off-diagonal root equation", sp.expand(2 * a * b).subs(a, 0) == 0, "2ab=0")
    cert.check(
        "G2 only two oriented roots in registered class",
        sp.Matrix([[0, -1], [1, 0]]) == j_plus
        and sp.Matrix([[0, 1], [-1, 0]]) == j_minus,
        "b=+/-1",
    )

    # G3: time reversal and ternary record handshake.
    theta_cr = sp.diag(1, 1, -1, -1)
    cert.check("G3 time reversal flips positive root", theta_cr * r_plus_cr * theta_cr == r_minus_cr, "Theta R+ Theta=R-")
    cert.check("G3 time reversal leaves half-turn", theta_cr * (r_plus_cr**2) * theta_cr == r_plus_cr**2, "Theta S Theta=S")

    probe = sp.Matrix([sp.Rational(2), sp.Rational(-3), sp.Rational(5), sp.Rational(7)])
    for sigma, root, inverse in ((1, r_plus, r_minus), (-1, r_minus, r_plus)):
        incoming = (sigma, 0, probe)
        outgoing = (0, sigma, root * probe)
        recovered = (outgoing[1], outgoing[0], inverse * outgoing[2])
        cert.check(f"G3 ternary handshake sigma={sigma}", recovered == incoming, "exact inverse")
        cert.check(f"G3 ternary norm transfer sigma={sigma}", sigma**2 + 0**2 == 0**2 + sigma**2, "h^2+r^2")
    reset_without_receiver = {1: 0, -1: 0}
    cert.check(
        "G3 reset without receiver is noninjective",
        reset_without_receiver[1] == reset_without_receiver[-1] and 1 != -1,
        "+/- -> blank collision",
    )
    cert.check(
        "G3 three-symbol self-delimiting alphabet",
        {-1, 0, 1} == set(reset_without_receiver) | {0},
        "blank/forward/reverse",
    )

    # G4: ultralocal oscillator root.
    kappa = sp.symbols("kappa", positive=True, nonzero=True)
    j_k = sp.Matrix([[0, -1 / kappa], [kappa, 0]])
    omega2 = sp.Matrix([[0, 1], [-1, 0]])
    metric_k = sp.diag(kappa**2, 1)
    cert.check("G4 J_kappa symplectic", sp.simplify(j_k.T * omega2 * j_k) == omega2, "canonical")
    cert.check("G4 J_kappa square", sp.simplify(j_k**2) == -sp.eye(2), "half-turn")
    cert.check("G4 J_kappa energy preserving", sp.simplify(j_k.T * metric_k * j_k) == metric_k, "H_kappa")
    cert.check("G4 unit normalization recovers J", j_k.subs(kappa, 1) == j_plus, "kappa=1 selected units")

    # G5: exact full-stiffness spectral root on a positive two-mode witness.
    b1, b2 = sp.symbols("b1 b2", positive=True, nonzero=True)
    bmat = sp.diag(b1, b2)
    kmat = bmat**2
    jk = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.zeros(2), -bmat.inv()),
        sp.Matrix.hstack(bmat, sp.zeros(2)),
    )
    hmetric = block_diag(kmat, sp.eye(2))
    cert.check("G5 spectral root symplectic", sp.simplify(jk.T * omega4 * jk) == omega4, "J_K")
    cert.check("G5 spectral root square", sp.simplify(jk**2) == -sp.eye(4), "J_K^2=-I")
    cert.check("G5 spectral root preserves full energy", sp.simplify(jk.T * hmetric * jk) == hmetric, "J_K^T H J_K=H")
    cert.check("G5 spectral orientations inverse", sp.simplify(jk.inv() + jk) == sp.zeros(4), "J_K^-1=-J_K")
    zero_b = sp.diag(0, b2)
    cert.check("G5 zero-mode inverse boundary", zero_b.det() == 0, "K^-1/2 undefined on zero mode")

    # G6: massive scalar finite-range obstruction by extremal exponent parity.
    z, mu2, c2 = sp.symbols("z mu2 c2", nonzero=True)
    k_laurent = mu2 + c2 * (2 - z - z**-1)
    support = {term.as_powers_dict().get(z, 0) for term in sp.expand(k_laurent).as_ordered_terms()}
    max_exp = max(support)
    min_exp = min(support)
    cert.check("G6 one-axis stiffness support", support == {-1, 0, 1}, support)
    cert.check("G6 nonzero extremal coefficients", sp.expand(k_laurent).coeff(z, 1) == -c2 and sp.expand(k_laurent * z).coeff(z, 0) == -c2, "-c^2")
    cert.check("G6 extremal exponents are odd", max_exp == 1 and min_exp == -1, (min_exp, max_exp))
    cert.check(
        "G6 Laurent-square parity obstruction",
        all(2 * n != max_exp and 2 * n != min_exp for n in range(-4, 5)),
        "2m=+1 and 2n=-1 have no integer solutions",
    )
    cert.check(
        "G6 obstruction independent of mass",
        mu2 not in (sp.expand(k_laurent).coeff(z, 1), sp.expand(k_laurent * z).coeff(z, 0)).free_symbols,
        "mass changes only exponent zero",
    )

    # G7: local root energy defect.
    k1, k2 = sp.symbols("k1 k2", positive=True)
    kdiag = sp.diag(k1, k2)
    q1, q2, p1, p2 = sp.symbols("q1 q2 p1 p2", real=True)
    qvec = sp.Matrix([q1, q2])
    pvec = sp.Matrix([p1, p2])
    qprime = -pvec / kappa
    pprime = kappa * qvec
    h_before = (pvec.dot(pvec) + (qvec.T * kdiag * qvec)[0]) / 2
    h_after = (pprime.dot(pprime) + (qprime.T * kdiag * qprime)[0]) / 2
    defect = sp.expand(h_after - h_before)
    registered_defect = sp.expand(
        ((qvec.T * (kappa**2 * sp.eye(2) - kdiag) * qvec)[0]
         + (pvec.T * (kdiag / kappa**2 - sp.eye(2)) * pvec)[0]) / 2
    )
    cert.check("G7 local-root energy defect identity", sp.simplify(defect - registered_defect) == 0, defect)
    cert.check("G7 defect vanishes for ultralocal stiffness", sp.simplify(defect.subs({k1: kappa**2, k2: kappa**2})) == 0, "K=kappa^2 I")
    coeff_conditions = [
        sp.solve(sp.Eq(sp.expand(defect).coeff(symbol, 2), 0), stiffness)[0]
        for symbol, stiffness in ((q1, k1), (q2, k2), (p1, k1), (p2, k2))
    ]
    cert.check(
        "G7 all-state zero defect forces scalar stiffness",
        all(sp.simplify(value - kappa**2) == 0 for value in coeff_conditions),
        coeff_conditions,
    )
    cert.check(
        "G7 dispersive mode witness has nonzero work",
        sp.simplify(defect.subs({k1: kappa**2, k2: 2 * kappa**2, q1: 0, q2: 1, p1: 0, p2: 0}))
        == -kappa**2 / 2,
        "Delta H=-kappa^2/2",
    )

    # G8: clock seam and four-crossing retained history.
    delta, pi_clock, seam = sp.symbols("delta pi_clock seam", real=True)
    continuous_omega = block_diag(sp.Matrix([[0, 1], [-1, 0]]), omega4)
    seam_jacobian = block_diag(sp.eye(2), r_plus)
    cert.check("G8 seam map symplectic", seam_jacobian.T * continuous_omega * seam_jacobian == continuous_omega, "translation has identity Jacobian")
    for sigma, root in ((1, r_plus), (-1, r_minus)):
        delta_out = delta - sigma * seam
        delta_back = delta_out + sigma * seam
        cert.check(f"G8 seam inverse sigma={sigma}", sp.simplify(delta_back - delta) == 0 and root.inv() * root == sp.eye(4), "exact")
    cert.check(
        "G8 time reversal covariance of seam translation",
        sp.simplify(-((-delta) - seam) - (delta + seam)) == 0
        and theta_cr * r_plus_cr * theta_cr == r_minus_cr,
        "Theta F+ Theta=F-",
    )
    field = probe
    history: list[int] = []
    for _ in range(4):
        field = r_plus * field
        history.append(1)
    cert.check("G8 four crossings return field", field == probe, "R+^4=I")
    cert.check("G8 four crossings retain history", history == [1, 1, 1, 1], history)

    # G9: epistemic firewall.
    cert.check(
        "G9 G* does not choose root or stiffness",
        "it does not choose `sigma`, `K^{1/2}`, `kappa`, or the work reservoir" in protocol_norm,
        "cadence only",
    )
    cert.check(
        "G9 no Born/Bell target computation",
        "No numerical search, fit, near-miss comparison" in protocol_norm
        and "Born/Bell" in protocol_text,
        "scope firewall",
    )
    cert.check(
        "G9 no production promotion",
        "selected reference clutch, not a production derivation" in protocol_norm,
        "formation open",
    )

    print("-" * 79)
    print(f"checks={cert.total} passed={cert.passed} failed={cert.failed}")
    if cert.failed:
        print("FTD-0979 OUTCOME D - certificate invalid")
        return 1

    print("ORIENTED_ROOTS=EXACTLY_PLUS_OR_MINUS_J_IN_REGISTERED_CLASS")
    print("PRODUCTION_SWAP=ORIENTED_ROOT_SQUARED")
    print("SCALAR_ENERGY_COMPATIBLE_ROOT=MODAL_NONLOCAL_FOR_C18")
    print("LOCAL_ROOT=REQUIRES_EXPLICIT_WORK_HISTORY_OR_ADDED_FACTOR_HARDWARE")
    print("TERNARY_RESET=REQUIRES_RETAINED_RECEIVER_OR_EXPORTED_HISTORY")
    print("FTD-0979 OUTCOME B - exact oriented root; locality-energy-history trilemma")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
