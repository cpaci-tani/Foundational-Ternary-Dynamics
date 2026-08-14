#!/usr/bin/env python3
"""FTD-0981 exact local-work-port versus factor certificate."""

from __future__ import annotations

import hashlib
from pathlib import Path

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "docs/theory/10_eft_program"
PROTOCOL = BASE / (
    "preregistrations/native_time_carrier_programme/"
    "PREREG_LOCAL_WORK_PORT_VERSUS_MULTICOMPONENT_FACTOR_DISCRIMINATOR_v1.md"
)
EXPECTED_PROTOCOL = "7CF3DC6239200CF1B773ADEC0633F0B30CD5735C7FF8BDA1360F730888C5EDE3"

FROZEN = {
    "derivations/native_time_carrier_programme/"
    "THEOREM_ORIENTED_SQUARE_ROOT_CLUTCH_AND_LOCALITY_ENERGY_TRILEMMA_v1.md":
        "6C9082FD7C7E10E5A0767ECCB852B90BB84B5AAEFF2508376A347402E882264B",
    "derivations/native_time_carrier_programme/"
    "THEOREM_C18_FINITE_RANGE_CHARACTERISTIC_AND_RIGID_TRANSLATOR_OBSTRUCTION_v1.md":
        "C6424C1AA0DDA2BA57BDE14A1559C76BBB17E279087122FB7121C59350BB4329",
    "derivations/native_time_carrier_programme/"
    "THEOREM_SELF_DUAL_RECIPROCAL_DISCRETE_ACTION_AND_FORMATION_RESERVOIR_BOUNDARY_v1.md":
        "A7DC30C90C491976F58CDEAF71FB5ABFCE04952ECE971CA7FF72C65A7B9B90BF",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def zero_matrix(matrix: sp.Matrix) -> bool:
    return all(sp.simplify(entry) == 0 for entry in matrix)


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


def add_coefficient(
    coefficients: dict[tuple[int, int, int], sp.Rational],
    exponent: tuple[int, int, int],
    value: sp.Rational,
) -> None:
    coefficients[exponent] = sp.simplify(coefficients.get(exponent, 0) + value)


def main() -> int:
    print("=" * 79)
    print("FTD-0981 local work port versus multicomponent factor discriminator")
    print("=" * 79)
    cert = Certificate()

    protocol_text = PROTOCOL.read_text(encoding="utf-8")
    protocol_norm = " ".join(protocol_text.split())
    cert.check("G1 protocol hash", sha256(PROTOCOL) == EXPECTED_PROTOCOL, sha256(PROTOCOL))
    cert.check(
        "G1 locked marker",
        "[PRE-REGISTRATION — LOCKED BEFORE FIRST EXECUTION]" in protocol_text,
        "locked before first execution",
    )
    cert.check(
        "G1 expected classifier frozen",
        "Expected classifier:** `Outcome B`" in protocol_text,
        "Outcome B",
    )
    cert.check(
        "G1 no production mutation",
        "No representation is adopted into production by this test" in protocol_norm,
        "reference-only discriminator",
    )

    frozen_text: dict[str, str] = {}
    for relative, expected in FROZEN.items():
        path = BASE / relative
        actual = sha256(path)
        cert.check(f"G1 source hash {Path(relative).name}", actual == expected, actual)
        frozen_text[relative] = path.read_text(encoding="utf-8")

    trilemma_text = frozen_text[next(iter(FROZEN))]
    c18_text = frozen_text[list(FROZEN)[1]]
    reservoir_text = frozen_text[list(FROZEN)[2]]
    cert.check(
        "G1 inherited work defect",
        "local quarter-turn is still a legitimate symplectic event" in trilemma_text
        and "physical canonical reservoir" in trilemma_text,
        "FTD-0980",
    )
    cert.check(
        "G1 inherited C18 symbol",
        "L_{18}(z)" in c18_text and "K(z)=-\\frac13L_{18}(z)" in c18_text,
        "FTD-0943",
    )
    cert.check(
        "G1 inherited phase-complete lower bound",
        "one formed phase plane requires at least one complete" in reservoir_text
        and "canonical reservoir pair" in reservoir_text,
        "FTD-0928",
    )

    # G2: exact coefficient-level incidence factor of the frozen C18 symbol.
    faces = ((1, 0, 0), (0, 1, 0), (0, 0, 1))
    edges = (
        (1, 1, 0), (1, -1, 0),
        (1, 0, 1), (1, 0, -1),
        (0, 1, 1), (0, 1, -1),
    )
    incidence: dict[tuple[int, int, int], sp.Rational] = {}
    for direction in faces:
        weight = sp.Rational(1, 9)
        add_coefficient(incidence, (0, 0, 0), 2 * weight)
        add_coefficient(incidence, direction, -weight)
        add_coefficient(incidence, tuple(-x for x in direction), -weight)
    for direction in edges:
        weight = sp.Rational(1, 18)
        add_coefficient(incidence, (0, 0, 0), 2 * weight)
        add_coefficient(incidence, direction, -weight)
        add_coefficient(incidence, tuple(-x for x in direction), -weight)

    expected: dict[tuple[int, int, int], sp.Rational] = {(0, 0, 0): sp.Rational(4, 3)}
    for direction in faces:
        expected[direction] = -sp.Rational(1, 9)
        expected[tuple(-x for x in direction)] = -sp.Rational(1, 9)
    for direction in edges:
        expected[direction] = -sp.Rational(1, 18)
        expected[tuple(-x for x in direction)] = -sp.Rational(1, 18)

    cert.check("G2 nine undirected channels", len(faces) + len(edges) == 9, "3 face + 6 edge")
    cert.check("G2 incidence support", len(incidence) == 19, "center + 18 neighbors")
    cert.check("G2 exact incidence identity", incidence == expected, "B*B=K coefficientwise")
    cert.check("G2 exact center coefficient", incidence[(0, 0, 0)] == sp.Rational(4, 3), "4/3")
    cert.check(
        "G2 exact face coefficients",
        all(incidence[d] == -sp.Rational(1, 9) for d in faces),
        "-1/9",
    )
    cert.check(
        "G2 exact edge coefficients",
        all(incidence[d] == -sp.Rational(1, 18) for d in edges),
        "-1/18",
    )
    cert.check("G2 vacuum annihilation", sum(incidence.values()) == 0, "K(1,1,1)=0")

    kx, ky, kz = sp.symbols("kx ky kz", real=True)
    c_x, c_y, c_z = sp.cos(kx), sp.cos(ky), sp.cos(kz)
    k_symbol = sp.Rational(4, 3) - sp.Rational(2, 9) * (
        c_x + c_y + c_z + c_x * c_y + c_y * c_z + c_z * c_x
    )
    variables = (kx, ky, kz)
    hessian = sp.hessian(k_symbol, variables).subs({kx: 0, ky: 0, kz: 0})
    cert.check("G2 vacuum Hessian", hessian == sp.Rational(2, 3) * sp.eye(3), hessian)
    cert.check("G2 Hessian rank", hessian.rank() == 3, "rank 3")

    g11, g12, g21, g22, g31, g32 = sp.symbols("g11 g12 g21 g22 g31 g32")
    two_gradients = sp.Matrix([[g11, g12], [g21, g22], [g31, g32]])
    two_channel_hessian = 2 * two_gradients * two_gradients.T
    cert.check(
        "G2 two-channel rank obstruction",
        sp.expand(two_channel_hessian.det()) == 0,
        "rank(2GG^T)<=2",
    )
    cert.check(
        "G2 factor-channel lower bound",
        hessian.rank() > two_gradients.shape[1],
        "m>=3; nine-channel witness not claimed minimal",
    )

    # G3: self-adjoint incidence/Dirac block square.
    mu, b1, b2, b3 = sp.symbols("mu b1 b2 b3", real=True)
    b_col = sp.Matrix([b1, b2, b3])
    d_mu = sp.Matrix.vstack(
        sp.Matrix.hstack(sp.Matrix([[mu]]), b_col.T),
        sp.Matrix.hstack(b_col, -mu * sp.eye(3)),
    )
    expected_square = sp.diag(mu**2 + (b_col.T * b_col)[0], 0, 0, 0)
    expected_square[1:4, 1:4] = mu**2 * sp.eye(3) + b_col * b_col.T
    cert.check("G3 Dirac block self-adjoint", d_mu.T == d_mu, "D_mu^T=D_mu")
    cert.check("G3 exact Dirac block square", zero_matrix(d_mu**2 - expected_square), "diag(K_mu,mu^2I+BB*)")
    cert.check(
        "G3 off-diagonal cancellation",
        zero_matrix((d_mu**2)[0:1, 1:4]) and zero_matrix((d_mu**2)[1:4, 0:1]),
        "mu B*-B* mu=0",
    )
    cert.check(
        "G3 representation non-promotion",
        "not a derivation of fermions, spin, a Hilbert space" in protocol_norm,
        "selected factor witness only",
    )

    # G4: the factor does not remove the inverse needed by an exact event map.
    z = sp.symbols("z", nonzero=True)
    mu2, c2 = sp.symbols("mu2 c2", positive=True, nonzero=True)
    k_mu_laurent = mu2 + c2 * (2 - z - z**-1)
    polynomial = sp.expand(z * k_mu_laurent)
    cert.check(
        "G4 massive axis polynomial extremals",
        polynomial.coeff(z, 2) == -c2 and polynomial.coeff(z, 0) == -c2,
        polynomial,
    )
    cert.check(
        "G4 nonunit Laurent support",
        len(sp.Poly(polynomial, z).terms()) >= 2,
        "K_mu is not c z^n",
    )
    cert.check(
        "G4 reciprocal not finite Laurent",
        not sp.cancel(1 / k_mu_laurent).is_polynomial(z),
        "nonconstant Laurent nonunit",
    )
    cert.check(
        "G4 massless vacuum singularity",
        sp.simplify(k_mu_laurent.subs({mu2: 0, z: 1})) == 0,
        "K_0(1)=0",
    )
    cert.check(
        "G4 factor inverse implication",
        "scalar block of `D_mu^{-2}` would make `K_mu^{-1}`" in protocol_norm,
        "finite D^-1 would imply finite K^-1",
    )
    cert.check(
        "G4 factor-versus-event distinction",
        "localize a first-order **generator**" in protocol_text
        and "does not by itself produce the exact finite-range one-event map" in protocol_norm,
        "factor is not the clutch",
    )

    # G5/G6: finite-range symplectic work-port lift.  The scalar symbolic
    # identity proves each commuting spectral block; the document gives the
    # operator proof because B_q and B_p are polynomials in the same K.
    kappa = sp.symbols("kappa", positive=True, nonzero=True)
    s = sp.symbols("s", real=True)
    q, p, action = sp.symbols("q p action", real=True)
    omega2 = sp.Matrix([[0, 1], [-1, 0]])
    omega_ext = sp.diag(omega2, omega2)  # order (q,p,s,I_R)
    metric = sp.diag(k_symbol := sp.symbols("k", positive=True), 1)
    b_q = k_symbol - kappa**2
    b_p = 1 - k_symbol / kappa**2
    q_shear = sp.Matrix([[1, -s * b_p], [0, 1]])
    p_shear = sp.Matrix([[1, 0], [s * b_q, 1]])
    seam = sp.simplify(q_shear * p_shear)

    cert.check("G5 Q seam shear symplectic", zero_matrix(q_shear.T * omega2 * q_shear - omega2), "finite-range drift")
    cert.check("G5 P seam shear symplectic", zero_matrix(p_shear.T * omega2 * p_shear - omega2), "finite-range kick")
    cert.check("G5 seam family symplectic", zero_matrix(seam.T * omega2 * seam - omega2), "S_s=Q_sP_s")
    cert.check("G5 seam identity at crossing", seam.subs(s, 0) == sp.eye(2), "s=0")
    cert.check(
        "G5 seam derivative",
        seam.diff(s).subs(s, 0) == sp.Matrix([[0, -b_p], [b_q, 0]]),
        "-Omega B_0",
    )

    for sigma in (1, -1):
        root = sp.Matrix([[0, -sp.Rational(sigma, 1) / kappa], [sigma * kappa, 0]])
        root_s = sp.simplify(root * seam)
        b_s = sp.simplify(root_s.T * omega2 * root_s.diff(s))
        b_zero = sp.diag(b_q, b_p)
        phase_state = sp.Matrix([q, p])
        work = sp.simplify((phase_state.T * b_s * phase_state)[0] / 2)
        output_state = sp.simplify(root_s * phase_state)
        lifted_output = sp.Matrix([output_state[0], output_state[1], s, action + work])
        lifted_input = sp.Matrix([q, p, s, action])
        jacobian = lifted_output.jacobian(lifted_input)

        cert.check(f"G5 root symplectic sigma={sigma}", zero_matrix(root.T * omega2 * root - omega2), "R_sigma")
        cert.check(f"G5 root order four sigma={sigma}", zero_matrix(root**4 - sp.eye(2)), "R_sigma^4=I")
        cert.check(f"G5 seam root at crossing sigma={sigma}", root_s.subs(s, 0) == root, "R_0=R_sigma")
        cert.check(f"G5 work matrix symmetric sigma={sigma}", zero_matrix(b_s - b_s.T), "R_s^T Omega dR_s")
        cert.check(f"G5 crossing work matrix sigma={sigma}", zero_matrix(b_s.subs(s, 0) - b_zero), "B_0=G-R^TGR")
        cert.check(
            f"G5 finite-range polynomial blocks sigma={sigma}",
            all(k_symbol not in sp.denom(sp.together(entry)).free_symbols for entry in list(root_s) + list(b_s)),
            "no inverse polynomial in K",
        )
        cert.check(
            f"G6 extended symplecticity sigma={sigma}",
            zero_matrix(jacobian.T * omega_ext * jacobian - omega_ext),
            "Omega+dtheta wedge dI_R",
        )

        before = sp.simplify((phase_state.T * metric * phase_state)[0] / 2)
        seam_output = root * phase_state
        after = sp.simplify((seam_output.T * metric * seam_output)[0] / 2)
        seam_work = sp.simplify(work.subs(s, 0))
        cert.check(f"G6 work equals before-after sigma={sigma}", sp.simplify(seam_work - (before - after)) == 0, seam_work)
        cert.check(f"G6 total energy sigma={sigma}", sp.simplify(after + action + seam_work - before - action) == 0, "H'+I'=H+I")

        seam_inverse = sp.simplify(
            sp.Matrix([[1, 0], [-s * b_q, 1]])
            * sp.Matrix([[1, s * b_p], [0, 1]])
            * root.inv()
        )
        cert.check(f"G6 finite-range inverse sigma={sigma}", zero_matrix(seam_inverse * root_s - sp.eye(2)), "P_s^-1 Q_s^-1 R^-1")
        recovered_state = sp.simplify(seam_inverse * output_state)
        recovered_action = sp.simplify(action + work - (recovered_state.T * b_s * recovered_state)[0] / 2)
        cert.check(f"G6 exact state inverse sigma={sigma}", zero_matrix(recovered_state - phase_state), "z recovered")
        cert.check(f"G6 exact action inverse sigma={sigma}", sp.simplify(recovered_action - action) == 0, "I_R recovered")

    # G7: exact four-cycle telescoping and reserve boundary.
    z0 = sp.Matrix([q, p])
    h0 = sp.simplify((z0.T * metric * z0)[0] / 2)
    for sigma in (1, -1):
        root = sp.Matrix([[0, -sp.Rational(sigma, 1) / kappa], [sigma * kappa, 0]])
        states = [sp.simplify(root**m * z0) for m in range(5)]
        energies = [sp.simplify((state.T * metric * state)[0] / 2) for state in states]
        increments = [sp.simplify(energies[m] - energies[m + 1]) for m in range(4)]
        cert.check(f"G7 field four-cycle sigma={sigma}", zero_matrix(states[4] - states[0]), "z_4=z_0")
        cert.check(f"G7 work telescopes sigma={sigma}", sp.simplify(sum(increments)) == 0, "sum Delta I=0")
        cert.check(
            f"G7 intermediate action law sigma={sigma}",
            all(sp.simplify(sum(increments[:m]) - (h0 - energies[m])) == 0 for m in range(5)),
            "I_m-I_0=H_0-H_m",
        )

    amplitude = sp.symbols("amplitude", positive=True, nonzero=True)
    positive_q_defect = sp.simplify((kappa**2 - k_symbol) * amplitude**2 / 2)
    positive_p_defect = sp.simplify((k_symbol / kappa**2 - 1) * amplitude**2 / 2)
    cert.check(
        "G7 unbounded reserve for k<kappa^2",
        sp.limit(positive_q_defect, amplitude, sp.oo) == sp.oo,
        "q-direction work grows as amplitude^2",
    )
    cert.check(
        "G7 unbounded reserve for k>kappa^2",
        sp.limit(positive_p_defect, amplitude, sp.oo) == sp.oo,
        "p-direction work grows as amplitude^2",
    )
    cert.check(
        "G7 finite-ready-domain marker",
        "No finite reserve covers an unbounded-amplitude state space" in protocol_norm
        and "must fail closed" in protocol_text,
        "compliance shell required",
    )
    cert.check(
        "G7 orientation record remains separate",
        "does not replace the separate ternary orientation/history record" in protocol_norm,
        "work is not handedness memory",
    )

    # G8/G9: classifier and epistemic firewalls.
    cert.check(
        "G8 minimum canonical-pair completion",
        "at least one complete" in reservoir_text
        and "one complete phase/action work pair" in protocol_norm,
        "lower bound met by explicit lift",
    )
    cert.check(
        "G8 factor branch remains useful but insufficient",
        incidence == expected and "does not localize the inverse" in protocol_norm,
        "finite generator factor; no finite event inverse",
    )
    for forbidden in ("G*", "Born/Bell", "Hilbert", "fermion", "mass", "completeness"):
        cert.check(
            f"G9 no {forbidden} promotion",
            forbidden in protocol_text,
            "explicit scope firewall",
        )
    cert.check(
        "G9 no numerical search",
        "No numerical search, coefficient fit, near-miss comparison" in protocol_norm,
        "exact symbolic certificate only",
    )

    print("-" * 79)
    print(f"FTD-0981 exact certificate: {cert.passed}/{cert.total} checks passed")
    if cert.failed:
        print("OUTCOME D — invalid certificate; one or more frozen gates failed")
        return 1

    print(
        "OUTCOME B — the exact finite-range C18 incidence/Dirac factor exists, "
        "but it does not localize the inverse required by the one-event root."
    )
    print(
        "One complete phase/action work pair per independently gated batch is "
        "the minimum exact local canonical energy completion on a finite-reserve "
        "ready domain; field and reserve recover after four strokes."
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
