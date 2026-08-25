#!/usr/bin/env python3
"""Exact rank-20 tensor constraint-count obstruction.

The selected rank-20 collision closure has a rank-16 co-rotating first moment
for every registered nonzero primitive wavevector.  It therefore owns four
right zero modes and four metric-dual conserved left rows.  Even under the
optimistic assumption that all four conserved rows become first-class
constraints, Dirac counting leaves twelve phase-space dimensions:

    20 - 2*4 = 12,

not the four dimensions of two helicity-two polarizations plus conjugates.
The rank-20 carrier requires total reduction 2F+S=16.  Four first-class
constraints would still require eight second-class reductions, or an
equivalent additional four first-class constraints.

This is exact fixed-C4-quadrature dimension/linear-algebra accounting.  It
does not prove that the zero modes are first-class constraints or generate a
gauge symmetry.  The phase-complete carrier has a separate larger price.
"""

from __future__ import annotations

from sympy import Matrix

from proof_c18_tensor_doublet_tt_reduction import primitive_wavevectors
from proof_cotangent_rank20_collision_closure_tt_leakage import (
    co_rotating_axes,
    selected_rank20_carrier,
)


PHASE_SPACE_DIMENSION = 20
HELICITY_TWO_PHASE_SPACE_DIMENSION = 4


def main() -> None:
    _rows, closure, collision = selected_rank20_carrier()
    axes = co_rotating_axes(closure, collision)
    energy_metric = (closure * closure.T).inv()
    checks = 0

    wavevectors = primitive_wavevectors(2)
    assert len(wavevectors) == 98
    checks += 1
    for wavevector in wavevectors:
        operator = sum(
            (wavevector[axis] * axes[axis] for axis in range(3)),
            Matrix.zeros(20, 20),
        )
        right_kernel = operator.nullspace()
        left_kernel = operator.T.nullspace()
        assert operator.rank() == 16
        assert len(right_kernel) == len(left_kernel) == 4
        checks += 2

        for vector in right_kernel:
            conserved_row = vector.T * energy_metric
            assert conserved_row * operator == Matrix.zeros(1, 20)
            checks += 1
        for row_vector in left_kernel:
            assert row_vector.T * operator == Matrix.zeros(1, 20)
            checks += 1

    required_reduction = (
        PHASE_SPACE_DIMENSION - HELICITY_TWO_PHASE_SPACE_DIMENSION
    )
    assert required_reduction == 16

    four_first_class_remainder = PHASE_SPACE_DIMENSION - 2 * 4
    assert four_first_class_remainder == 12
    assert four_first_class_remainder != HELICITY_TWO_PHASE_SPACE_DIMENSION

    solutions = tuple(
        (first_class, second_class)
        for first_class in range(9)
        for second_class in range(17)
        if 2 * first_class + second_class == required_reduction
    )
    assert solutions == (
        (0, 16),
        (1, 14),
        (2, 12),
        (3, 10),
        (4, 8),
        (5, 6),
        (6, 4),
        (7, 2),
        (8, 0),
    )
    assert min(first + second for first, second in solutions) == 8
    assert (4, 8) in solutions
    checks += 7

    print("all 98 primitive nonzero wavevectors: rank(A)=16, nullity=4")
    print("four metric-dual left rows are exactly conserved by the first moment")
    print("optimistic four-first-class count: 20-2*4=12, not helicity-two phase space 4")
    print("required total reduction: 2F+S=16")
    print("minimum first-class-only price: F=8; with F=4, second-class price S=8")
    print(
        "PASS: cotangent rank20 constraint-count obstruction "
        f"({checks} exact checks)"
    )
    print(
        "Open: action-derived constraint algebra, gauge generators, invariant "
        "physical kernel, Maxwell compatibility, static gravity, and lensing"
    )


if __name__ == "__main__":
    main()
