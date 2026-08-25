#!/usr/bin/env python3
"""Exact C4 Born/radiation kernel separation and contextual-mixer boundary.

The four phase-address basis states carry the real regular representation of
C4.  It splits exactly into the trivial scalar, alternating scalar, and real
two-dimensional quadrature sectors.  The raw equal-weight Born form is twice
the quadrature projector, whereas the present cotangent field readout is
phase-blind and therefore uses the trivial projector.  These positive kernels
are orthogonal, not one universal "phase compatibility" Gram.

On the existing directional-port handoff family, identifying the opposite
phase-band coefficient with the raw Born value b=-1 makes positive emission
impossible under positive semidefiniteness and exact handoff conservation.
The current field readout instead has b=+1 because ``layer_value`` ignores the
C4 phase address.  Thus one native action may contain both sectors, but it
cannot identify their readout kernels.

Every symmetric C4-invariant quadratic form is block diagonal on the three
regular-representation sectors.  A fixed-context mixer can couple the trivial
and quadrature sectors, but necessarily fails to commute with the C4 shift;
its full four-context conjugacy orbit is covariant and has zero preferred
phase.  This is an exact algebraic target for contextual actualization, not a
physical Born pushforward or a selected interaction coefficient.
"""

from __future__ import annotations

from itertools import product

from sympy import Matrix, Rational, simplify

from proof_c4_paired_history_born_count import (
    amplitude_components,
    canonical_residual_rails,
    coherent_norm_squared,
)
from proof_global_c3_cotangent_layer_hodge_maxwell_target import layer_value
from proof_hodge_flag_pair_collision_invariant_space import one_particle_states


def c4_shift() -> Matrix:
    """Column-action permutation e_p -> e_(p+1)."""

    shift = Matrix.zeros(4, 4)
    for phase in range(4):
        shift[(phase + 1) % 4, phase] = 1
    return shift


def flatten(matrix: Matrix) -> Matrix:
    return Matrix([matrix[row, column] for row in range(4) for column in range(4)])


def symmetric_basis() -> tuple[Matrix, ...]:
    output = []
    for row in range(4):
        for column in range(row, 4):
            basis = Matrix.zeros(4, 4)
            basis[row, column] = 1
            basis[column, row] = 1
            output.append(basis)
    return tuple(output)


def main() -> None:
    checks = 0
    shift = c4_shift()
    identity = Matrix.eye(4)
    assert shift**4 == identity
    checks += 1

    trivial = Matrix([1, 1, 1, 1])
    alternating = Matrix([1, -1, 1, -1])
    real = Matrix([1, 0, -1, 0])
    imaginary = Matrix([0, 1, 0, -1])

    p_trivial = trivial * trivial.T / 4
    p_alternating = alternating * alternating.T / 4
    p_quadrature = (real * real.T + imaginary * imaginary.T) / 2
    projectors = (p_trivial, p_alternating, p_quadrature)

    for projector in projectors:
        assert projector.T == projector
        assert projector * projector == projector
        assert projector * shift == shift * projector
        checks += 3
    for left_index, left in enumerate(projectors):
        for right_index, right in enumerate(projectors):
            if left_index != right_index:
                assert left * right == Matrix.zeros(4, 4)
                checks += 1
    assert sum(projectors, start=Matrix.zeros(4, 4)) == identity
    assert tuple(projector.rank() for projector in projectors) == (1, 1, 2)
    checks += 2

    # The two readout kernels occupy orthogonal central sectors.
    field_kernel = trivial * trivial.T
    born_kernel = real * real.T + imaginary * imaginary.T
    alternating_kernel = alternating * alternating.T
    assert field_kernel == 4 * p_trivial
    assert born_kernel == 2 * p_quadrature
    assert alternating_kernel == 4 * p_alternating
    assert field_kernel * born_kernel == Matrix.zeros(4, 4)
    assert born_kernel[0, 2] == -1
    assert field_kernel[0, 2] == 1
    checks += 6

    # Raw history multiplicities realize the Born quadratic form exactly.
    for raw_counts in product(range(5), repeat=4):
        counts = Matrix(raw_counts)
        real_amplitude, imaginary_amplitude = amplitude_components(raw_counts)
        assert (counts.T * born_kernel * counts)[0] == coherent_norm_squared(raw_counts)
        assert (counts.T * born_kernel * counts)[0] == (
            real_amplitude**2 + imaginary_amplitude**2
        )
        rotated = shift * counts
        assert (rotated.T * born_kernel * rotated)[0] == (
            counts.T * born_kernel * counts
        )[0]
        checks += 3

    # Canonical cancellation is a nonlinear quotient followed by positive
    # same-rail counting; it is not the raw bilinear kernel under a new name.
    phase_zero = (1, 0, 0, 0)
    phase_two = (0, 0, 1, 0)
    canceled = tuple(left + right for left, right in zip(phase_zero, phase_two))
    assert canonical_residual_rails(phase_zero) == (1, 0)
    assert canonical_residual_rails(phase_two) == (1, 0)
    assert canonical_residual_rails(canceled) == (0, 0)
    checks += 3

    # The present cotangent Maxwell readout is exactly phase blind for every
    # native flag and every cotangent layer.  Its normalized phase Gram is J.
    for flag, _phase in one_particle_states():
        for layer in range(3):
            values = tuple(layer_value((flag, phase), layer) for phase in range(4))
            assert all(value == values[0] for value in values)
            norm = sum(component * component for component in values[0])
            assert norm == 2
            empirical = Matrix(
                4,
                4,
                lambda row, column: Rational(
                    sum(
                        left * right
                        for left, right in zip(values[row], values[column])
                    ),
                    norm,
                ),
            )
            assert empirical == field_kernel
            checks += 3

    # Directly imposing the raw Born opposite-phase coefficient b=-1 on the
    # handoff-conserving directional-port Gram makes emitted work nonpositive.
    # With c=-a the two nonzero eigenvalues are 2(1+a), 2(1-a), while
    # Delta H_emit=-(1+a)/2=-lambda_plus/4.
    for numerator in range(-16, 17):
        a = Rational(numerator, 16)
        if not (-1 <= a <= 1):
            continue
        b = Rational(-1)
        c = -a
        eigenvalues = (
            1 + a + b + c,
            1 - a + b - c,
            1 + a - b - c,
            1 - a - b + c,
        )
        emission = simplify((b - a) / 2)
        free_energy = simplify((1 + b) / 2)
        assert eigenvalues == (0, 0, 2 * (1 + a), 2 * (1 - a))
        assert all(value >= 0 for value in eigenvalues)
        assert emission == -eigenvalues[2] / 4
        assert emission <= 0
        assert free_energy == 0
        checks += 5

    # Exhaust the symmetric commutant: every invariant quadratic form is a
    # linear combination of the three central projectors, so it cannot mix
    # the field-trivial and Born-quadrature sectors.
    basis = symmetric_basis()
    constraint = Matrix.hstack(*(flatten(element * shift - shift * element) for element in basis))
    nullspace = constraint.nullspace()
    assert len(basis) == 10
    assert len(nullspace) == 3
    projector_span = Matrix.hstack(*(flatten(projector) for projector in projectors))
    assert projector_span.rank() == 3
    assert all(projector * shift == shift * projector for projector in projectors)
    for vector in nullspace:
        candidate = sum(
            (coefficient * element for coefficient, element in zip(vector, basis)),
            start=Matrix.zeros(4, 4),
        )
        assert Matrix.hstack(projector_span, flatten(candidate)).rank() == 3
        for left_index, left in enumerate(projectors):
            for right_index, right in enumerate(projectors):
                if left_index != right_index:
                    assert left * candidate * right == Matrix.zeros(4, 4)
                    checks += 1
        checks += 1
    checks += 4

    # A minimum fixed-context off-diagonal vertex mixes P0 and PQ.  It is not
    # C4 invariant at fixed context, but its conjugacy orbit is covariant and
    # does not retain a preferred global phase when all contexts are summed.
    detector_axis = real
    mixer = trivial * detector_axis.T + detector_axis * trivial.T
    assert p_trivial * mixer * p_quadrature != Matrix.zeros(4, 4)
    assert mixer * shift != shift * mixer
    context_orbit = tuple(
        shift**turns * mixer * shift ** (-turns) for turns in range(4)
    )
    assert len({tuple(flatten(element)) for element in context_orbit}) == 4
    assert sum(context_orbit, start=Matrix.zeros(4, 4)) == Matrix.zeros(4, 4)
    for turns, context in enumerate(context_orbit):
        successor = shift * context * shift ** (-1)
        assert successor == context_orbit[(turns + 1) % 4]
        checks += 1
    checks += 4

    print("C4 regular record space = trivial(1) + alternating(1) + quadrature(2)")
    print("current phase-blind field kernel = 4 P_trivial")
    print("raw equal-weight Born kernel = 2 P_quadrature")
    print("the two kernels are orthogonal; opposite-phase weights are +1 versus -1")
    print("Born b=-1 plus PSD handoff conservation gives Delta H_emit <= 0")
    print("every symmetric C4-invariant quadratic form is sector block diagonal")
    print("a fixed detector-context mixer can join P_trivial and P_quadrature")
    print("its four-context orbit is covariant and has no preferred summed phase")
    print(f"PASS: C4 Born/radiation kernel separation ({checks} exact checks)")
    print(
        "Boundary: the common action must derive the context mixer, its work "
        "ledger, physical history preparation, and event-frequency pushforward"
    )


if __name__ == "__main__":
    main()
