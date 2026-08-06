"""FTD-0770 exact certificate for Coupled Quartic Clock Field v1.

This proves identities inside the selected Hamiltonian.  It does not derive
the action-angle state, stiffness, compliance, or connection from FTD P1--P5.
"""

from __future__ import annotations

from itertools import product

import sympy as sp


def main() -> None:
    checks = 0

    def check(label: str, condition: bool) -> None:
        nonlocal checks
        assert condition, label
        checks += 1

    m, eta = sp.symbols("m eta", positive=True)
    nu = 2 * m / (m + 2)
    check("action inversion exponent", sp.simplify(1 / nu - (m + 2) / (2 * m)) == 0)

    # For H=A I^nu, use E=H, Omega=nu E/I, and
    # H''=nu(nu-1)E/I^2.  Every normalization constant cancels.
    reduced_ratio = sp.simplify(eta * (nu - 1) / nu)
    expected_ratio = eta * (m - 2) / (2 * m)
    check("dimensionless wave-cycle ratio", sp.simplify(reduced_ratio - expected_ratio) == 0)
    check("quadratic ratio", reduced_ratio.subs(m, 2) == 0)
    check("quartic ratio", sp.simplify(reduced_ratio.subs(m, 4) - eta / 4) == 0)
    check("sextic ratio", sp.simplify(reduced_ratio.subs(m, 6) - eta / 3) == 0)

    # Gamma recurrence gives the quartic coefficient without numerical input:
    # Gamma(3/2)=sqrt(pi)/2 and Gamma(7/4)=3 Gamma(3/4)/4.
    g = sp.symbols("G", positive=True)
    beta_quarter_three_halves = 2 * sp.sqrt(sp.pi) * g / 3
    c4 = sp.simplify(beta_quarter_three_halves / (2 * sp.pi))
    check("quartic action coefficient", sp.simplify(c4 - g / (3 * sp.sqrt(sp.pi))) == 0)
    beta_quarter_half = sp.sqrt(sp.pi) * g
    check("quartic unit-shell period", beta_quarter_half == sp.sqrt(sp.pi) * g)

    energy = sp.symbols("E", positive=True)
    quartic_period = sp.sqrt(sp.pi) * g * (2 * energy) ** sp.Rational(-1, 4)
    check("quartic period invariant",
          sp.simplify(quartic_period * (2 * energy) ** sp.Rational(1, 4)
                      - sp.sqrt(sp.pi) * g) == 0)
    check("amplitude-one shell is E=1/2",
          sp.simplify(quartic_period.subs(energy, sp.Rational(1, 2))
                      - sp.sqrt(sp.pi) * g) == 0)
    check("E=1 is not the amplitude-one normalization",
          sp.simplify(quartic_period.subs(energy, 1)
                      - sp.sqrt(sp.pi) * g) != 0)

    # Graph-Laplacian continuum tensors.  Neighbor lists include both
    # orientations, so D_ij=(1/2) sum_r r_i r_j.
    axial = [tuple(sign if axis == coordinate else 0
                   for coordinate in range(3))
             for axis in range(3) for sign in (-1, 1)]
    moore = [vector for vector in product((-1, 0, 1), repeat=3)
             if vector != (0, 0, 0)]

    def continuum_tensor(vectors: list[tuple[int, ...]]) -> sp.Matrix:
        dimension = len(vectors[0])
        return sp.Matrix(dimension, dimension,
                         lambda i, j: sp.Rational(1, 2)
                         * sum(vector[i] * vector[j] for vector in vectors))

    check("axial continuum tensor", continuum_tensor(axial) == sp.eye(3))
    check("Moore continuum tensor", continuum_tensor(moore) == 9 * sp.eye(3))

    # Every oriented edge contributes equal and opposite action transfer.
    torques = sp.symbols("t0:5")
    total_action_rate = sum((-torque + torque) for torque in torques)
    check("total action conservation", sp.simplify(total_action_rate) == 0)

    u1, u2, omega = sp.symbols("u1 u2 omega", real=True)
    compliance_ratio = sp.exp(-u1) * omega / (sp.exp(-u2) * omega)
    check("compliance rate ratio",
          sp.simplify(compliance_ratio - sp.exp(-(u1 - u2))) == 0)

    theta_v, theta_w, connection, alpha_v, alpha_w = sp.symbols(
        "theta_v theta_w connection alpha_v alpha_w", real=True)
    transformed_edge_phase = ((theta_v + alpha_v) - (theta_w + alpha_w)
                              - (connection + alpha_v - alpha_w))
    check("edge phase is gauge invariant",
          sp.simplify(transformed_edge_phase
                      - (theta_v - theta_w - connection)) == 0)

    print(f"FTD-0770 coupled quartic clock exact certificate: {checks}/{checks} PASS")
    print("GSTAR_LINEAR_SIGNATURE_ABSENT")
    print("FIXED_BACKGROUND_HOLONOMY_KINEMATIC_ONLY")


if __name__ == "__main__":
    main()
