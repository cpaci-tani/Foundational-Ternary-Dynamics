#!/usr/bin/env python3
"""Exact phase-complete common closure and C4 linear-selection theorem.

The existing rank-30 common Maxwell/tensor closure fixes one C4 quadrature
slice.  The native one-record alphabet contains all four C4 phases.  Their
two real quadrature rows are orthogonal to the phase-independent scalar row.
Consequently a phase-blind right collision doubles every tensor closure while
leaving every Maxwell closure unchanged.

The complete closure census is therefore

    8 x (tensor20, Maxwell7,  common27)
    8 x (tensor20, Maxwell13, common33)
   16 x (tensor40, Maxwell10, common50)
   16 x (tensor52, Maxwell19, common71)

on each cotangent layer.  Chaining this lift to the earlier curl-closure
certificate raises the minimum phase-complete target-containing carrier from
the fixed-slice dimension 30 to dimension 50.

For the selected witness the tensor-40 generator is exactly two C4-related
copies of the tensor-20 generator, and all tensor/Maxwell cross blocks vanish.
This is not accidental: C4 acts as J^2=-I on the tensor quadratures and as
identity on phase-independent Maxwell rows, so a C4-equivariant linear map
cannot intertwine them.  Vacuum linear decoupling is therefore a selection
rule; common coupling must be nonlinear/phase-neutral or matter-mediated.

This is a type/closure and selection-rule theorem, not a native Maxwell,
spin-2, gravity, lensing, coupling-normalization, or unified-action result.
"""

from __future__ import annotations

from collections import Counter

from sympy import Matrix, eye, kronecker_product
from sympy.polys.matrices import DomainMatrix

from proof_cotangent_common_maxwell_tensor_collision_closure_price import (
    maxwell_rows,
)
from proof_cotangent_rank20_collision_closure_tt_leakage import (
    SELECTED_COLLISION_INDEX,
    displacement_sum,
    invariant_closure,
    tensor_rows,
)
from proof_cotangent_right_regular_collision_spin2_closure_obstruction import (
    GROUP,
    right_regular_permutation,
)


PHASE_QUADRATURES = Matrix([[1, 0, -1, 0], [0, 1, 0, -1]])
PHASE_CONSTANT = Matrix([[1, 1, 1, 1]])
PHASE_SHIFT = Matrix(
    [
        [0, 0, 0, 1],
        [1, 0, 0, 0],
        [0, 1, 0, 0],
        [0, 0, 1, 0],
    ]
)
PHASE_COMPLEX_STRUCTURE = Matrix([[0, -1], [1, 0]])
EXPECTED_CENSUS = Counter(
    {
        (20, 7, 27): 8,
        (20, 13, 33): 8,
        (40, 10, 50): 16,
        (52, 19, 71): 16,
    }
)


def exact_rank(matrix: Matrix) -> int:
    return DomainMatrix.from_Matrix(matrix).rank()


def verify_phase_types() -> int:
    assert exact_rank(PHASE_QUADRATURES) == 2
    assert exact_rank(PHASE_CONSTANT) == 1
    assert exact_rank(Matrix.vstack(PHASE_QUADRATURES, PHASE_CONSTANT)) == 3
    assert PHASE_QUADRATURES * PHASE_CONSTANT.T == Matrix.zeros(2, 1)
    assert PHASE_QUADRATURES * PHASE_QUADRATURES.T == 2 * eye(2)
    assert PHASE_CONSTANT * PHASE_CONSTANT.T == Matrix([[4]])
    assert PHASE_SHIFT**4 == eye(4)
    assert PHASE_CONSTANT * PHASE_SHIFT == PHASE_CONSTANT
    assert (
        PHASE_QUADRATURES * PHASE_SHIFT
        == PHASE_COMPLEX_STRUCTURE * PHASE_QUADRATURES
    )
    assert PHASE_COMPLEX_STRUCTURE**2 == -eye(2)
    assert exact_rank(PHASE_COMPLEX_STRUCTURE - eye(2)) == 2
    return 11


def verify_phase_complete_closure_census() -> int:
    checks = 0
    for layer in range(3):
        census = Counter()
        representatives = set()
        for frame in GROUP:
            collision = right_regular_permutation(frame)
            tensor_closure = invariant_closure(tensor_rows(layer), collision)
            maxwell_closure = invariant_closure(maxwell_rows(layer), collision)
            signature = (
                2 * tensor_closure.rows,
                maxwell_closure.rows,
                2 * tensor_closure.rows + maxwell_closure.rows,
            )
            census[signature] += 1
            checks += 1

            if signature not in representatives:
                phase_tensor = kronecker_product(
                    PHASE_QUADRATURES, tensor_closure
                )
                phase_maxwell = kronecker_product(
                    PHASE_CONSTANT, maxwell_closure
                )
                common = Matrix.vstack(phase_tensor, phase_maxwell)
                assert exact_rank(phase_tensor) == signature[0]
                assert exact_rank(phase_maxwell) == signature[1]
                assert exact_rank(common) == signature[2]
                representatives.add(signature)
                checks += 3
        assert census == EXPECTED_CENSUS
        assert representatives == set(EXPECTED_CENSUS)
        checks += 2
    return checks


def selected_phase_complete_carrier():
    collision = right_regular_permutation(GROUP[SELECTED_COLLISION_INDEX])
    tensor = tensor_rows(0)
    maxwell = maxwell_rows(0)
    tensor_closure = Matrix.vstack(tensor, tensor * collision)
    maxwell_closure = Matrix.vstack(
        maxwell, maxwell[4:7, :] * collision
    )
    phase_tensor = kronecker_product(
        PHASE_QUADRATURES, tensor_closure
    )
    phase_maxwell = kronecker_product(
        PHASE_CONSTANT, maxwell_closure
    )
    common = Matrix.vstack(phase_tensor, phase_maxwell)
    full_collision = kronecker_product(eye(4), collision)

    assert exact_rank(tensor_closure) == 20
    assert exact_rank(maxwell_closure) == 10
    assert exact_rank(phase_tensor) == 40
    assert exact_rank(phase_maxwell) == 10
    assert exact_rank(common) == 50
    assert exact_rank(Matrix.vstack(common, common * full_collision)) == 50
    return (
        tensor_closure,
        maxwell_closure,
        phase_tensor,
        phase_maxwell,
        common,
        full_collision,
        6,
    )


def projected_axis(rows: Matrix, collision: Matrix, displacement: Matrix) -> Matrix:
    metric = (rows * rows.T).inv()
    carrier_collision = rows * collision * rows.T * metric
    assert rows * collision == carrier_collision * rows
    return (
        carrier_collision.inv()
        * rows
        * displacement
        * collision
        * rows.T
        * metric
    )


def verify_selected_c4_selection_rule() -> int:
    (
        tensor_closure,
        maxwell_closure,
        phase_tensor,
        phase_maxwell,
        common,
        full_collision,
        checks,
    ) = selected_phase_complete_carrier()
    full_phase_shift = kronecker_product(PHASE_SHIFT, eye(48))
    common_metric = (common * common.T).inv()
    common_phase_action = (
        common * full_phase_shift * common.T * common_metric
    )
    expected_phase_action = Matrix.diag(
        kronecker_product(PHASE_COMPLEX_STRUCTURE, eye(20)),
        eye(10),
    )
    assert common * full_phase_shift == common_phase_action * common
    assert common_phase_action == expected_phase_action
    assert common_phase_action**4 == eye(50)
    checks += 3

    tensor_phase_action = expected_phase_action[0:40, 0:40]
    assert exact_rank(tensor_phase_action - eye(40)) == 40
    # Hence X Q_T = X and Q_T Y = Y have only X=0 and Y=0: no linear
    # intertwiners between the nontrivial tensor C4 type and trivial Maxwell
    # type can commute with the phase advance.
    checks += 1

    for axis in range(3):
        displacement = displacement_sum(axis)
        full_displacement = kronecker_product(eye(4), displacement)
        tensor_axis = projected_axis(
            tensor_closure, full_collision[0:48, 0:48], displacement
        )
        maxwell_axis = projected_axis(
            maxwell_closure, full_collision[0:48, 0:48], displacement
        )
        phase_tensor_axis = projected_axis(
            phase_tensor, full_collision, full_displacement
        )
        common_axis = projected_axis(common, full_collision, full_displacement)

        assert phase_tensor_axis == kronecker_product(eye(2), tensor_axis)
        assert common_axis[0:40, 0:40] == phase_tensor_axis
        assert common_axis[40:50, 40:50] == maxwell_axis
        assert common_axis[0:40, 40:50] == Matrix.zeros(40, 10)
        assert common_axis[40:50, 0:40] == Matrix.zeros(10, 40)
        assert common_phase_action * common_axis == common_axis * common_phase_action
        checks += 6
    return checks


def verify_conditional_phase_complete_price() -> int:
    physical_dimension = 8
    required_reduction = 50 - physical_dimension
    assert required_reduction == 42
    solutions = tuple(
        (first_class, second_class)
        for first_class in range(22)
        for second_class in range(43)
        if 2 * first_class + second_class == required_reduction
    )
    assert solutions == tuple((first, 42 - 2 * first) for first in range(22))
    assert (0, 42) in solutions
    assert (21, 0) in solutions
    # A prior twenty-dimensional synchronization/reality quotient would
    # recover the fixed-slice rank 30 and its conditional reduction price 22,
    # but that quotient is not supplied by the phase-blind action.
    assert 50 - 20 == 30
    assert 42 - 20 == 22
    return 6


def main() -> None:
    checks = verify_phase_types()
    checks += verify_phase_complete_closure_census()
    checks += verify_selected_c4_selection_rule()
    checks += verify_conditional_phase_complete_price()

    print("C4 phase types: quadrature rank2 orthogonal to constant rank1")
    print("phase-complete closure census per cotangent layer:")
    print("  8 x (tensor20, Maxwell7,  common27)")
    print("  8 x (tensor20, Maxwell13, common33)")
    print(" 16 x (tensor40, Maxwell10, common50)")
    print(" 16 x (tensor52, Maxwell19, common71)")
    print("minimum phase-complete target-containing common carrier=50")
    print("selected tensor40 generator=I2 tensor tensor20 generator")
    print("C4-equivariant linear tensor/Maxwell cross blocks vanish exactly")
    print("conditional phase-complete physical reduction price: 2F+S=42")
    print("an imposed 20-dimensional phase-reality reduction would lower that price to 22")
    print(
        "PASS: cotangent phase-complete common closure and C4 selection "
        f"({checks} exact checks)"
    )
    print(
        "Open: native phase-reality/synchronization rule or nonlinear shared "
        "vertex, layer-covariant poles, constraints, gravity, lensing, and alpha"
    )


if __name__ == "__main__":
    main()
