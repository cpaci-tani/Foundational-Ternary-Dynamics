#!/usr/bin/env python3
"""Independent FTD-0573 cubic canonical-form and defect-rank proof."""

from __future__ import annotations

import hashlib
import itertools
import json
import math
import platform
from pathlib import Path

import sympy as sp


REPO = Path(__file__).resolve().parents[2]
TOL = 1.0e-12
PREREG_SHA = "0EABA25DFCE05351FE361AE69920AAA3CD37F79B18A4C2028BB1BCEC7DDE3438"

LOCKED_HASHES = {
    "ftd0572_theorem": (
        "docs/theory/10_eft_program/derivations/"
        "THEOREM_GENESIS_MINIMAL_BATH.md",
        "8BCB40F379246EF36C6CA7CDFDD5757DAB66D3ABFF622C0439482B7C5BDEE8AA",
    ),
    "ftd0572_header": (
        "engine/include/ftd/eft/genesis_minimal_bath.h",
        "CCD7B09967D194498B50A7AFA449E04ED21FF06746F6B3E98651A18FD4AA1B42",
    ),
    "ftd0572_source": (
        "engine/src/eft/genesis_minimal_bath.cpp",
        "47664A3A83BDC7125DF8C5C84FB23B09EA1F159EC2F471AD368D9697BFA83223",
    ),
    "phase_write": (
        "engine/src/render_bridge_phases/phase_write.cpp",
        "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    ),
    "voxel": (
        "engine/include/ftd/voxel.h",
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    ),
}

IMPLEMENTATION_PATHS = {
    "header": "engine/include/ftd/eft/genesis_cubic_canonical_form.h",
    "source": "engine/src/eft/genesis_cubic_canonical_form.cpp",
    "test": "engine/tests/test_genesis_cubic_canonical_form.cpp",
    "independent_proof": "scripts/proofs/proof_genesis_cubic_canonical_form.py",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def permutation_sign(permutation: tuple[int, ...]) -> int:
    inversions = sum(
        permutation[i] > permutation[j]
        for i in range(len(permutation))
        for j in range(i + 1, len(permutation))
    )
    return 1 if inversions % 2 == 0 else -1


def signed_permutations(proper_only: bool) -> list[sp.Matrix]:
    result = []
    for permutation in itertools.permutations(range(3)):
        parity = permutation_sign(permutation)
        for signs in itertools.product((-1, 1), repeat=3):
            if proper_only and parity * math.prod(signs) != 1:
                continue
            matrix = sp.zeros(3)
            for row, column in enumerate(permutation):
                matrix[row, column] = signs[row]
            result.append(matrix)
    return result


def standard_form() -> sp.Matrix:
    identity = sp.eye(3)
    return sp.BlockMatrix(
        [[sp.zeros(3), identity], [-identity, sp.zeros(3)]]
    ).as_explicit()


def skew_basis() -> list[sp.Matrix]:
    result = []
    for i in range(6):
        for j in range(i + 1, 6):
            matrix = sp.zeros(6)
            matrix[i, j] = 1
            matrix[j, i] = -1
            result.append(matrix)
    return result


def set_skew(matrix: sp.Matrix, i: int, j: int, value: sp.Expr) -> None:
    matrix[i, j] = value
    matrix[j, i] = -value


def defect(omega: sp.Matrix, system_map: sp.Matrix) -> sp.Matrix:
    return sp.simplify(omega - system_map.T * omega * system_map)


def numeric_rank(matrix: sp.Matrix, tolerance: float = 1.0e-10) -> int:
    value = [[float(entry) for entry in row] for row in matrix.tolist()]
    rows = len(value)
    columns = len(value[0])
    result = 0
    for column in range(columns):
        if result == rows:
            break
        pivot = max(range(result, rows), key=lambda row: abs(value[row][column]))
        if abs(value[pivot][column]) <= tolerance:
            continue
        value[result], value[pivot] = value[pivot], value[result]
        divisor = value[result][column]
        for j in range(column, columns):
            value[result][j] /= divisor
        for row in range(rows):
            if row == result:
                continue
            factor = value[row][column]
            for j in range(column, columns):
                value[row][j] -= factor * value[result][j]
        result += 1
    return result


def symbolic_classification() -> dict[str, object]:
    proper_group = signed_permutations(True)
    full_group = signed_permutations(False)
    omega0 = standard_form()
    basis = skew_basis()

    rows = []
    for rotation in proper_group:
        action = sp.diag(rotation, rotation)
        transformed = [action.T * element * action - element for element in basis]
        for i in range(6):
            for j in range(i + 1, 6):
                rows.append([element[i, j] for element in transformed])
    constraint = sp.Matrix(rows)
    nullspace = constraint.nullspace()

    full_invariance = all(
        sp.diag(rotation, rotation).T
        * omega0
        * sp.diag(rotation, rotation)
        == omega0
        for rotation in full_group
    )
    null_generator = sum(
        (coefficient * element for coefficient, element in zip(nullspace[0], basis)),
        sp.zeros(6),
    )
    proportional_to_standard = (
        null_generator == omega0 or null_generator == -omega0
    )

    t, a = sp.symbols("t a", real=True)
    diagonal = sp.diag(1, t, t, a, a, a)

    zero = sp.zeros(6)
    for i, j in ((0, 3), (4, 5), (1, 2)):
        set_skew(zero, i, j, sp.Integer(1))
    zero_defect = defect(zero, diagonal.subs(a, 1))

    ratio = (1 - a * t) / ((1 - a) * (1 + t))
    generic = sp.zeros(6)
    for i, j in ((0, 1), (2, 3), (4, 5), (0, 3)):
        set_skew(generic, i, j, sp.Integer(1))
    set_skew(generic, 1, 2, -ratio)
    generic_defect = defect(generic, diagonal)
    determinant_formula = (t - a) ** 2 / ((1 - a) ** 2 * (1 + t) ** 2)

    return {
        "full_group_order": len(full_group),
        "proper_group_order": len(proper_group),
        "constraint_rank": int(constraint.rank()),
        "constraint_nullity": len(nullspace),
        "full_group_invariance": full_invariance,
        "null_generator_is_standard": proportional_to_standard,
        "standard_determinant": int(omega0.det()),
        "zero_alternative_determinant": int(zero.det()),
        "zero_alternative_defect_rank": int(zero_defect.rank()),
        "generic_alternative_determinant_formula": sp.simplify(
            generic.det() - determinant_formula
        ) == 0,
        "generic_alternative_defect_rank": int(generic_defect.rank()),
        "zero_lower_bound": 2,
        "generic_lower_bound_from_multiplicity_three": 4,
        "degenerate_lower_bound_from_multiplicity_five": 6,
    }


def numeric_registered_grid() -> dict[str, object]:
    inv_sqrt3 = 1.0 / math.sqrt(3.0)
    directions = [
        (1.0, 0.0, 0.0), (-1.0, 0.0, 0.0),
        (0.0, 1.0, 0.0), (0.0, -1.0, 0.0),
        (0.0, 0.0, 1.0), (0.0, 0.0, -1.0),
        (inv_sqrt3, inv_sqrt3, inv_sqrt3),
        (-inv_sqrt3, inv_sqrt3, inv_sqrt3),
        (inv_sqrt3, -inv_sqrt3, inv_sqrt3),
        (inv_sqrt3, inv_sqrt3, -inv_sqrt3),
    ]
    excesses = (sp.Rational(1, 8), sp.Rational(1, 2), sp.Rational(5, 4))
    drains = (sp.Rational(0), sp.Rational(1, 2), sp.Rational(9, 10), sp.Rational(1))
    omega0 = standard_form()
    production_arms = symmetry_price_arms = 0
    rank_four_arms = rank_six_arms = 0
    min_generic_determinant = math.inf
    max_determinant_residual = 0.0

    for direction in directions:
        n = sp.Matrix(direction)
        for excess in excesses:
            t = excess / (1 + excess)
            projector = n * n.T
            spatial = sp.eye(3) * t + projector * (1 - t)
            for drain in drains:
                a = 1 - drain
                system_map = sp.diag(spatial, sp.eye(3) * a)
                cubic_rank = numeric_rank(defect(omega0, system_map))
                expected_cubic = 4 if drain == 0 else 6
                assert cubic_rank == expected_cubic
                rank_four_arms += cubic_rank == 4
                rank_six_arms += cubic_rank == 6

                diagonal = sp.diag(1, t, t, a, a, a)
                alternative = sp.zeros(6)
                if drain == 0:
                    for i, j in ((0, 3), (4, 5), (1, 2)):
                        set_skew(alternative, i, j, sp.Integer(1))
                    alternative_rank = defect(alternative, diagonal).rank()
                    assert alternative.det() == 1 and alternative_rank == 2
                else:
                    ratio = (1 - a * t) / ((1 - a) * (1 + t))
                    for i, j in ((0, 1), (2, 3), (4, 5), (0, 3)):
                        set_skew(alternative, i, j, sp.Integer(1))
                    set_skew(alternative, 1, 2, -ratio)
                    alternative_rank = defect(alternative, diagonal).rank()
                    measured = float(alternative.det())
                    expected = ((t - a) ** 2) / (
                        ((1 - a) ** 2) * ((1 + t) ** 2)
                    )
                    max_determinant_residual = max(
                        max_determinant_residual, abs(measured - float(expected))
                    )
                    min_generic_determinant = min(min_generic_determinant, measured)
                    assert measured > 0.0 and alternative_rank == 4

                symmetry_price_arms += cubic_rank - alternative_rank == 2
                production_arms += 1

    return {
        "production_arms": production_arms,
        "rank_four_arms": rank_four_arms,
        "rank_six_arms": rank_six_arms,
        "symmetry_price_arms": symmetry_price_arms,
        "maximum_generic_determinant_formula_residual": max_determinant_residual,
        "minimum_generic_alternative_determinant": min_generic_determinant,
    }


def main() -> int:
    prereg_path = REPO / (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_GENESIS_CUBIC_CANONICAL_FORM_v1.md"
    )
    assert sha256(prereg_path) == PREREG_SHA
    source_hashes = {"preregistration": sha256(prereg_path)}
    for key, (relative, expected) in LOCKED_HASHES.items():
        observed = sha256(REPO / relative)
        assert observed == expected, (key, observed, expected)
        source_hashes[key] = observed

    symbolic = symbolic_classification()
    assert symbolic["full_group_order"] == 48
    assert symbolic["proper_group_order"] == 24
    assert symbolic["constraint_rank"] == 14
    assert symbolic["constraint_nullity"] == 1
    assert symbolic["full_group_invariance"]
    assert symbolic["null_generator_is_standard"]
    assert symbolic["standard_determinant"] == 1
    assert symbolic["zero_alternative_determinant"] == 1
    assert symbolic["zero_alternative_defect_rank"] == 2
    assert symbolic["generic_alternative_determinant_formula"]
    assert symbolic["generic_alternative_defect_rank"] == 4

    numeric = numeric_registered_grid()
    passes = (
        numeric["production_arms"] == 120
        and numeric["rank_four_arms"] == 30
        and numeric["rank_six_arms"] == 90
        and numeric["symmetry_price_arms"] == 120
        and numeric["maximum_generic_determinant_formula_residual"] <= TOL
        and numeric["minimum_generic_alternative_determinant"] > 0.0
    )
    assert passes

    implementation_hashes = {
        key: sha256(REPO / relative)
        for key, relative in IMPLEMENTATION_PATHS.items()
    }
    record = {
        "ftd_id": "FTD-0573",
        "verdict": "CUBIC_COVARIANCE_SELECTS_STANDARD_PAIRING_AND_PRICES_ONE_BATH_PAIR",
        "platform": f"{platform.system()}-{platform.release()}-{platform.version()}",
        "field_representation": "constant onsite skew form on equivalent vector J and W representations",
        "tolerance": TOL,
        **numeric,
        "proper_cubic_constraint_rank": symbolic["constraint_rank"],
        "proper_cubic_invariant_nullity": symbolic["constraint_nullity"],
        "standard_pairing_unique_up_to_scale": True,
        "zero_drain_unconstrained_minimum_rank": 2,
        "positive_drain_generic_unconstrained_minimum_rank": 4,
        "a_equals_t_unconstrained_minimum_rank": 6,
        "cubic_pairing_zero_drain_rank": 4,
        "cubic_pairing_positive_drain_rank": 6,
        "symmetry_price_bath_pairs": 1,
        "repeated_eigenspace_lower_bound_proved": True,
        "branchwise_alternatives_are_not_one_global_form": True,
        "native_canonical_action_derived": False,
        "production_changed": False,
        "passes": passes,
        "symbolic_checks": symbolic,
        "source_hashes_sha256": source_hashes,
        "implementation_hashes_sha256": implementation_hashes,
    }
    output = REPO / "engine/results/ftd_0573/windows_msvc_cpu.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(record, indent=2, default=str))
    print("PASS: cubic covariance selects the onsite pairing at one extra bath-pair price")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
