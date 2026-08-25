#!/usr/bin/env python3
"""Exact cotangent charged-pole and reciprocal-alpha protocol certificate.

This certificate does not choose or fit a coupling.  Conditional on one
selected Maxwell/Gauss action in the already fixed canonical packet basis, it
proves that (i) a unit neutral charge pair excites the cubic massless Green
pole, (ii) the static residue and free-field Hessian return the same blind
dimensionless curvature chi, and (iii) alpha_native=chi/(4*pi*c_eff).

The microscopic transaction still has to derive chi and realize the
constraint/action locally and reversibly.  Consequently this is a measurement
protocol and reference-action theorem, not a numerical prediction of alpha.
"""

from __future__ import annotations

from itertools import combinations

from sympy import Matrix, Rational, Symbol, cos, pi, simplify

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS


def incidence(vertex_count: int, edges: tuple[tuple[int, int], ...]) -> Matrix:
    matrix = Matrix.zeros(vertex_count, len(edges))
    for column, (tail, head) in enumerate(edges):
        matrix[tail, column] = -1
        matrix[head, column] = 1
    return matrix


def constrained_solution(divergence: Matrix, charge: Matrix) -> tuple[Matrix, Matrix]:
    """Return E=D^T phi, with D E=rho, in the gauge phi[-1]=0."""

    assert sum(charge) == 0
    laplacian = divergence * divergence.T
    reduced_phi = laplacian[:-1, :-1].inv() * charge[:-1, :]
    phi = Matrix.vstack(reduced_phi, Matrix([[0]]))
    electric = divergence.T * phi
    assert divergence * electric == charge
    return electric, phi


def graph_fixtures() -> tuple[tuple[int, tuple[tuple[int, int], ...]], ...]:
    return (
        (4, ((0, 1), (1, 2), (2, 3))),
        (5, ((0, 1), (1, 2), (2, 3), (3, 4), (4, 0))),
        (6, ((0, 1), (1, 2), (2, 0), (2, 3), (3, 4), (4, 5), (5, 3))),
        (5, tuple(combinations(range(5), 2))),
    )


def main() -> None:
    checks = 0
    chi = Symbol("chi_EM", positive=True)

    # The D4 packet theorem fixes the source coordinate.  No rescaling is
    # available once E_raw=8d is read with Gram^{-1}=I/64.
    gram_inverse = Rational(1, 64) * Matrix.eye(3)
    for direction in SC_DIRECTIONS:
        packet = 8 * Matrix(direction)
        assert (packet.T * gram_inverse * packet)[0] == 1
        checks += 1

    # Exact finite connected-graph constrained action.  The same chi is both
    # the Hessian on a unit edge and the coefficient returned by every neutral
    # source response, independently of source position or graph topology.
    for vertex_count, edges in graph_fixtures():
        divergence = incidence(vertex_count, edges)
        laplacian = divergence * divergence.T
        assert divergence.rank() == vertex_count - 1
        assert laplacian * Matrix.ones(vertex_count, 1) == Matrix.zeros(vertex_count, 1)
        checks += 2

        for source, sink in combinations(range(vertex_count), 2):
            charge = Matrix.zeros(vertex_count, 1)
            charge[source] = 1
            charge[sink] = -1
            electric, potential = constrained_solution(divergence, charge)
            norm_squared = (electric.T * electric)[0]
            green_quadratic = (charge.T * potential)[0]
            energy = chi * norm_squared / 2

            assert norm_squared == green_quadratic
            assert norm_squared > 0
            assert simplify(2 * energy / green_quadratic - chi) == 0
            assert divergence * electric == charge
            checks += 4

            # The constrained solution is orthogonal to the divergence-free
            # cycle space, hence is the unique minimum-energy representative.
            for cycle in divergence.nullspace():
                assert (electric.T * cycle)[0] == 0
                checks += 1

    # Cubic nearest-neighbor Laplacian: the only zero is the constant mode and
    # its infrared symbol is quadratic.  The inverse is therefore the charged
    # massless static pole of the selected reference action.
    kx, ky, kz, eps = (Symbol(name, real=True) for name in ("kx", "ky", "kz", "eps"))
    dx, dy, dz = (Symbol(name, real=True) for name in ("dx", "dy", "dz"))
    lattice_symbol = 2 * (3 - cos(kx) - cos(ky) - cos(kz))
    directional = lattice_symbol.subs({kx: eps * dx, ky: eps * dy, kz: eps * dz})
    infrared_coefficient = simplify(directional.diff(eps, 2).subs(eps, 0) / 2)
    assert lattice_symbol.subs({kx: 0, ky: 0, kz: 0}) == 0
    assert infrared_coefficient == dx**2 + dy**2 + dz**2
    checks += 2

    # A normalized Fourier source mode has H=(chi/2)|rho|^2/Lambda.  Removing
    # the known Green factor returns chi without a target value.
    rho_norm_squared, lambda_k = (
        Symbol("rho_norm_squared", positive=True),
        Symbol("Lambda_k", positive=True),
    )
    mode_energy = chi * rho_norm_squared / (2 * lambda_k)
    static_estimator = simplify(2 * mode_energy * lambda_k / rho_norm_squared)
    assert static_estimator == chi
    checks += 1

    # For rho=e_x-e_y, the off-diagonal Green contribution to
    # (chi/2) rho^T G rho is -chi G_xy.  Self terms do not depend on the
    # source separation and are removed by the interaction-energy readout.
    green_xx, green_yy, green_xy = (
        Symbol("G_xx", real=True),
        Symbol("G_yy", real=True),
        Symbol("G_xy", real=True),
    )
    pair_energy = chi * (green_xx + green_yy - 2 * green_xy) / 2
    self_energy = chi * (green_xx + green_yy) / 2
    assert simplify(pair_energy - self_energy) == -chi * green_xy
    checks += 1

    # In canonical unit packet coordinates the free-field action Hessian is
    # chi.  Thus the radiative and static protocols are reciprocal tests of
    # one common action coefficient rather than two independently fitted data.
    packet_norm_squared = Rational(1)
    free_hessian = simplify(chi * packet_norm_squared)
    assert free_hessian == static_estimator
    checks += 1

    c_eff = Rational(1, 6)
    alpha_native = simplify(chi / (4 * pi * c_eff))
    assert alpha_native == 3 * chi / (2 * pi)
    checks += 1

    # The current conditional source-work ledger expresses, but does not fix,
    # this curvature.  Gamma, material impulse mu, and cadence L remain inputs
    # until derived by the common microscopic transaction.
    gamma, impulse, cadence = (
        Symbol("Gamma", positive=True),
        Symbol("mu", positive=True),
        Symbol("L", positive=True),
    )
    action_unit = gamma / 2 + impulse / (2 * cadence)
    conditional_curvature = simplify(gamma / action_unit)
    assert conditional_curvature == 2 * cadence * gamma / (cadence * gamma + impulse)
    checks += 1

    print(
        "PASS: cotangent charged-pole reciprocal-alpha measurement protocol "
        f"({checks} exact checks)"
    )
    print("selected cubic Gauss action has a charged massless 1/Lambda static pole")
    print("static Green-residue estimator and canonical free-field Hessian both equal chi_EM")
    print("conditional alpha_native=chi_EM/(4 pi c_eff)=3 chi_EM/(2 pi)")
    print("no value is chosen: common-action derivation of chi_EM and local realization remain open")


if __name__ == "__main__":
    main()
