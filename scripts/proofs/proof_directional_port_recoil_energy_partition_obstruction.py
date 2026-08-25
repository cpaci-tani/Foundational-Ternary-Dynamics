#!/usr/bin/env python3
"""Exact directional-port recoil-energy partition obstruction.

The C6 recoil-current vertex was first evaluated with fully coarse
standing/outgoing field norms 1 and 2, a capacity change -1, and a unit
material displacement.  Its dimensionless
capacity+field ledger closes only because translational kinetic energy has not
yet been included.

This certificate classifies every signed-cubic-invariant quadratic kinetic
form.  The form is uniquely mu*I, so one unit SC recoil costs mu/2.  With

    H_capacity = I_* g,
    H_field    = Gamma h_F,

exact emission energy conservation requires

    I_* = Gamma + mu/2,
    Gamma/I_* = 1 - mu/(2 I_*).

Thus the no-recoil choice Gamma=I_* is incompatible with any positive
quadratic recoil inertia.  The physical coupling can only be measured from a
common work partition once the matter kinetic coefficient is derived.  No
fine-structure root or experimental value is used or compared.

Successor note: the exact C4-trivial field-sector certificate selects the
handoff-conserving metric with standing/outgoing norms 1/2 and 1.  It replaces
the physical partition by I_*=(Gamma+mu)/2 and work fraction
Gamma/(Gamma+mu).  The algebra checked here remains the scoped obstruction for
the earlier fully coarse normalization.
"""

from __future__ import annotations

from sympy import Matrix, Rational, simplify, symbols

from proof_c18_equivariant_single_record_collision_no_go import SC_DIRECTIONS
from proof_moore_bond_capacity_type_census import signed_permutation_matrices


def main() -> None:
    checks = 0
    a11, a22, a33, a12, a13, a23 = symbols(
        "a11 a22 a33 a12 a13 a23", real=True
    )
    variables = (a11, a22, a33, a12, a13, a23)
    kinetic_matrix = Matrix(
        [
            [a11, a12, a13],
            [a12, a22, a23],
            [a13, a23, a33],
        ]
    )
    group = tuple(Matrix(matrix) for matrix in signed_permutation_matrices())

    constraint_rows = []
    for transformation in group:
        residual = transformation.T * kinetic_matrix * transformation - kinetic_matrix
        for entry in residual:
            constraint_rows.append(
                [entry.coeff(variable) for variable in variables]
            )
    constraints = Matrix(constraint_rows)
    assert constraints.rank() == 5
    nullspace = constraints.nullspace()
    assert len(nullspace) == 1
    generator = nullspace[0]
    assert generator[0] == generator[1] == generator[2]
    assert generator[0] != 0
    assert generator[3] == generator[4] == generator[5] == 0
    checks += 6

    mu, action_unit, field_scale, momentum_scale = symbols(
        "mu I_star Gamma P_star", positive=True
    )
    isotropic = mu * Matrix.eye(3)
    for transformation in group:
        assert transformation.T * isotropic * transformation == isotropic
        checks += 1

    # Every SC recoil direction has the same positive quadratic price.
    for direction_tuple in SC_DIRECTIONS:
        direction = Matrix(direction_tuple)
        kinetic_energy = (direction.T * isotropic * direction)[0] / 2
        canonical_momentum = isotropic * direction
        assert kinetic_energy == mu / 2
        assert canonical_momentum == mu * direction
        checks += 2

    # Standing: capacity=1, field norm=1, no recoil.
    # Outgoing: capacity=0, field norm=2, unit recoil.
    standing_energy = action_unit + field_scale
    outgoing_energy = 2 * field_scale + mu / 2
    energy_defect = outgoing_energy - standing_energy
    assert energy_defect == field_scale + mu / 2 - action_unit
    conserving_field_scale = action_unit - mu / 2
    assert energy_defect.subs(field_scale, conserving_field_scale) == 0
    coupling_fraction = conserving_field_scale / action_unit
    assert simplify(coupling_fraction - (1 - mu / (2 * action_unit))) == 0
    checks += 4

    # The previous no-recoil unit match Gamma=I_* leaves exactly the positive
    # recoil cost as an uncancelled defect.
    assert energy_defect.subs(field_scale, action_unit) == mu / 2
    checks += 1

    # If the normalized field momentum r is assigned physical scale P_*,
    # reciprocal momentum conservation with p_M=-mu*r fixes P_*=mu.
    field_momentum = momentum_scale * Matrix([1, 0, 0])
    matter_momentum = -mu * Matrix([1, 0, 0])
    momentum_residual = field_momentum + matter_momentum
    assert momentum_residual.subs(momentum_scale, mu) == Matrix.zeros(3, 1)
    combined_fraction = coupling_fraction.subs(mu, momentum_scale)
    assert simplify(
        combined_fraction - (1 - momentum_scale / (2 * action_unit))
    ) == 0
    checks += 2

    # Exact rational controls for the allowed positive partition interval.
    for fraction in (Rational(1, 8), Rational(1, 4), Rational(1, 2), Rational(3, 4)):
        trial_mu = 2 * action_unit * fraction
        trial_field = conserving_field_scale.subs(mu, trial_mu)
        assert trial_field / action_unit == 1 - fraction
        assert energy_defect.subs({mu: trial_mu, field_scale: trial_field}) == 0
        checks += 2

    print("O_h-invariant symmetric quadratic kinetic forms: one ray, A=mu*I")
    print("unit SC recoil kinetic energy=mu/2 and canonical momentum=mu*r")
    print("standing energy=I_*+Gamma; outgoing energy=2*Gamma+mu/2")
    print("exact conservation: I_*=Gamma+mu/2")
    print("native field-work fraction: Gamma/I_*=1-mu/(2 I_*)")
    print("momentum-scale closure: P_*=mu")
    print("successor correction: selected field metric gives I_*=(Gamma+mu)/2")
    print("successor work fraction: Gamma/(Gamma+mu); canonical momentum map remains open")
    print(
        f"PASS: directional-port recoil energy partition obstruction ({checks} exact checks)"
    )
    print(
        "Open: derive mu and I_* from formed matter/common action, preserve emitted "
        "Maxwell energy, then measure rather than insert the physical coupling"
    )


if __name__ == "__main__":
    main()
