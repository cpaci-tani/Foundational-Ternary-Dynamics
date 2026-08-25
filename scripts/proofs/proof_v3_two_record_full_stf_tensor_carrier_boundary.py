#!/usr/bin/env python3
"""Exact two-record STF carrier and selected-collision boundary for FTD-v3.

The prior one-record audit found only the diagonal Eg part of a spatial STF
tensor.  This certificate asks the next representation-theoretic question:
does a simultaneous pair of already selected field records supply the missing
T2g shear without adding a primitive carrier?

The answer is yes at readout level.  The exact cross stress of two records
spans Eg + T2g (rank five), and common-phase pairs span a full C4 quadrature
doublet (rank ten).  The selected Phi-v2 collision does not conserve that
composite: exactly 192 input pairs per C3 layer change their cross stress.
Therefore the carrier obstruction is removed only at composite-readout level;
the protected slow tensor mode, pole, constraints, and gravity remain open.
"""

from __future__ import annotations

import sys
from itertools import combinations

from sympy import Matrix

import proof_global_c3_cotangent_layer_equivariant_collision as collision_proof
from proof_global_c3_cotangent_layer_hodge_maxwell_target import (
    internal_tick,
    layer_value,
)
from proof_hodge_flag_pair_collision_invariant_space import (
    one_particle_states,
    transform_state,
)
from proof_moore_bond_capacity_type_census import (
    matrix_vector,
    signed_permutation_matrices,
)


sys.stdout.reconfigure(encoding="utf-8")

PHASE_COORDINATES = ((1, 0), (0, 1), (-1, 0), (0, -1))


def cross_stress3(left, right, layer: int) -> tuple[tuple[int, ...], ...]:
    """Three times the symmetric trace-free cross stress.

    Multiplication by three keeps the complete census integral without
    changing its rank or covariance.
    """

    left_value = layer_value(left, layer)
    right_value = layer_value(right, layer)
    left_e, left_b = left_value[:3], left_value[3:]
    right_e, right_b = right_value[:3], right_value[3:]
    dot_sum = sum(a * b for a, b in zip(left_e, right_e)) + sum(
        a * b for a, b in zip(left_b, right_b)
    )
    return tuple(
        tuple(
            3
            * (
                left_e[i] * right_e[j]
                + right_e[i] * left_e[j]
                + left_b[i] * right_b[j]
                + right_b[i] * left_b[j]
            )
            - (2 * dot_sum if i == j else 0)
            for j in range(3)
        )
        for i in range(3)
    )


def stf5(tensor) -> tuple[int, ...]:
    return (
        tensor[0][0],
        tensor[1][1],
        tensor[0][1],
        tensor[0][2],
        tensor[1][2],
    )


def transform_tensor(matrix, tensor) -> tuple[tuple[int, ...], ...]:
    transformed = Matrix(matrix) * Matrix(tensor) * Matrix(matrix).T
    return tuple(
        tuple(int(transformed[i, j]) for j in range(3)) for i in range(3)
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    states = one_particle_states()
    state_index = {state: index for index, state in enumerate(states)}
    pairs = tuple(combinations(range(len(states)), 2))
    group = tuple(signed_permutation_matrices())

    check("C1 selected bank contains 192 records and 18,336 distinct pairs", len(states) == 192 and len(pairs) == 18_336)

    layer_ranks = []
    eg_ranks = []
    t2g_ranks = []
    distinct_tensors = []
    for layer in range(3):
        rows = []
        for left_index, right_index in pairs:
            tensor = cross_stress3(
                states[left_index], states[right_index], layer
            )
            assert tensor == tuple(zip(*tensor))
            assert sum(tensor[i][i] for i in range(3)) == 0
            rows.append(stf5(tensor))
        matrix = Matrix(rows)
        layer_ranks.append(matrix.rank())
        eg_ranks.append(matrix[:, :2].rank())
        t2g_ranks.append(matrix[:, 2:].rank())
        distinct_tensors.append(len(set(rows)))

    check("C2 every two-record cross stress is symmetric and trace free", bool(pairs))
    check("C3 two-record readouts span the full STF rank five on every C3 layer", layer_ranks == [5, 5, 5], str(layer_ranks))
    check("C4 the diagonal sector is exactly Eg rank two", eg_ranks == [2, 2, 2], str(eg_ranks))
    check("C5 the off-diagonal sector is exactly T2g rank three", t2g_ranks == [3, 3, 3], str(t2g_ranks))
    check("C6 each layer has the same finite 43-tensor image", distinct_tensors == [43, 43, 43], str(distinct_tensors))

    # Same-phase pairs use their common C4 phase as the two tensor
    # quadratures.  Their finite span is the complete ten-dimensional
    # Eg+T2g doublet.
    quadrature_ranks = []
    for layer in range(3):
        rows = []
        for left_index, right_index in pairs:
            left, right = states[left_index], states[right_index]
            if left[1] != right[1]:
                continue
            tensor = stf5(cross_stress3(left, right, layer))
            real, imag = PHASE_COORDINATES[left[1]]
            rows.append(
                tuple(real * entry for entry in tensor)
                + tuple(imag * entry for entry in tensor)
            )
        assert len(rows) == 4_512
        quadrature_ranks.append(Matrix(rows).rank())
    check("C7 common-phase pairs span a full rank-ten C4 tensor doublet", quadrature_ranks == [10, 10, 10], str(quadrature_ranks))

    # Exact covariance needs only one representative of each finite tensor
    # image.  Transforming both records agrees with M Sigma M^T for all 48
    # signed-cubic maps.
    covariance_rows = 0
    for layer in range(3):
        representatives = {}
        for left_index, right_index in pairs:
            pair = (states[left_index], states[right_index])
            representatives.setdefault(stf5(cross_stress3(*pair, layer)), pair)
        assert len(representatives) == 43
        for pair in representatives.values():
            tensor = cross_stress3(*pair, layer)
            for matrix in group:
                transformed_pair = tuple(transform_state(matrix, state) for state in pair)
                assert cross_stress3(*transformed_pair, layer) == transform_tensor(
                    matrix, tensor
                )
                covariance_rows += 1
    check("C8 complete finite tensor image is signed-cubic covariant", covariance_rows == 3 * 43 * 48)

    # The internal flag/phase tick and C3 layer decrement transport the full
    # composite readout without introducing a pair identity.
    clock_rows = 0
    for left_index, right_index in pairs:
        left, right = states[left_index], states[right_index]
        for layer in range(3):
            assert cross_stress3(
                internal_tick(left), internal_tick(right), (layer - 1) % 3
            ) == cross_stress3(left, right, layer)
            clock_rows += 1
    check("C9 full two-record tensor is covariantly carried by the native clock", clock_rows == 3 * 18_336)

    print("Running parent selected-collision certificate...")
    collision_proof.main()
    data = collision_proof.CERTIFICATE_DATA
    assert data is not None
    assert data["states"] == states
    changed_counts = []
    for layer, collision in enumerate(data["collisions"]):
        changed = 0
        for before, after in collision.items():
            before_tensor = cross_stress3(
                states[before[0]], states[before[1]], layer
            )
            after_tensor = cross_stress3(
                states[after[0]], states[after[1]], layer
            )
            changed += before_tensor != after_tensor
        changed_counts.append(changed)
    check("C10 selected Phi-v2 collision changes 192 pair tensors per layer", changed_counts == [192, 192, 192], str(changed_counts))
    check("C11 number plus E/B conservation does not protect the composite STF sector", all(count > 0 for count in changed_counts))

    missing = {
        "tensor-protecting collision",
        "massless tensor pole",
        "scalar/vector constraints",
        "universal common-stress coupling",
        "shared matter/radiation cone",
        "lensing and nonlinear bootstrap",
    }
    check("C12 gravity remains open beyond composite carrier availability", len(missing) == 6)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 two-record tensor-carrier checks pass")
    print(f"pair_rows_per_layer={len(pairs)}")
    print(f"signed_cubic_covariance_rows={covariance_rows}")
    print(f"collision_changed_tensor_pairs={changed_counts}")
    print("carrier_status=full_Eg_plus_T2g_composite_readout_present")
    print("gravity_status=unprotected_no_pole_no_constraints_no_universal_coupling")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
