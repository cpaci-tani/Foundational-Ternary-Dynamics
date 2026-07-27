#!/usr/bin/env python3
"""Independent FTD-0572 minimum-bath proof and run-of-record generator."""

from __future__ import annotations

import hashlib
import json
import math
import platform
from pathlib import Path

import sympy as sp


REPO = Path(__file__).resolve().parents[2]
TOL = 1.0e-12
PREREG_SHA = "26C87DB4BFF2800D07C687031A606728F2982933ABBAD55A73E0BF010DEB4B1C"

LOCKED_HASHES = {
    "ftd0571_theorem": (
        "docs/theory/10_eft_program/derivations/"
        "THEOREM_GENESIS_ENVIRONMENT_FEEDBACK.md",
        "B7BD5590D5DDD2B5A23DC4E5DF0B6BD3DDC7ED124FD93A4C66C8BA3FE80B45B6",
    ),
    "ftd0571_header": (
        "engine/include/ftd/eft/genesis_environment_feedback.h",
        "2F8B7A7610E06E49957B35ED795A3A9DCF43BF0FE2288B4296D7B2214FCC76AB",
    ),
    "ftd0571_source": (
        "engine/src/eft/genesis_environment_feedback.cpp",
        "4DE62DC51CF6C660020D8FC8DEE9D38BE11C5FF2A774C08CE0E3707346B28CCB",
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
    "header": "engine/include/ftd/eft/genesis_minimal_bath.h",
    "source": "engine/src/eft/genesis_minimal_bath.cpp",
    "test": "engine/tests/test_genesis_minimal_bath.cpp",
    "independent_proof": "scripts/proofs/proof_genesis_minimal_bath.py",
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def transpose(a: list[list[float]]) -> list[list[float]]:
    return [list(row) for row in zip(*a)]


def multiply(a: list[list[float]], b: list[list[float]]) -> list[list[float]]:
    return [
        [sum(a[i][k] * b[k][j] for k in range(len(b))) for j in range(len(b[0]))]
        for i in range(len(a))
    ]


def subtract(a: list[list[float]], b: list[list[float]]) -> list[list[float]]:
    return [[a[i][j] - b[i][j] for j in range(len(a[0]))] for i in range(len(a))]


def max_abs(a: list[list[float]]) -> float:
    return max(abs(value) for row in a for value in row)


def rank(a: list[list[float]], tolerance: float = 1.0e-10) -> int:
    value = [row[:] for row in a]
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
        value[result] = [entry / divisor for entry in value[result]]
        for row in range(rows):
            if row == result:
                continue
            factor = value[row][column]
            value[row] = [
                value[row][j] - factor * value[result][j]
                for j in range(columns)
            ]
        result += 1
    return result


def canonical(pairs: int) -> list[list[float]]:
    size = 2 * pairs
    omega = [[0.0 for _ in range(size)] for _ in range(size)]
    for i in range(pairs):
        omega[i][i + pairs] = 1.0
        omega[i + pairs][i] = -1.0
    return omega


def pair_canonical() -> list[list[float]]:
    return [
        [0.0, 1.0, 0.0, 0.0],
        [-1.0, 0.0, 0.0, 0.0],
        [0.0, 0.0, 0.0, 1.0],
        [0.0, 0.0, -1.0, 0.0],
    ]


def pair_dilation(lam: float, a: float) -> list[list[float]]:
    if a == 0.0:
        return [
            [lam, 0.0, 1.0, 0.0],
            [0.0, 0.0, 0.0, 1.0],
            [-1.0, 0.0, 0.0, 0.0],
            [0.0, -1.0, 0.0, lam],
        ]
    beta = math.sqrt(max(0.0, 1.0 - a * lam))
    return [
        [lam, 0.0, beta, 0.0],
        [0.0, a, 0.0, beta],
        [-beta / a, 0.0, 1.0, 0.0],
        [0.0, -a * beta, 0.0, a * lam],
    ]


def apply(a: list[list[float]], value: list[float]) -> list[float]:
    return [sum(row[j] * value[j] for j in range(len(value))) for row in a]


def system_map(direction: tuple[float, float, float], t: float, a: float) -> list[list[float]]:
    result = [[0.0 for _ in range(6)] for _ in range(6)]
    for i in range(3):
        for j in range(3):
            result[i][j] = (t if i == j else 0.0) + (1.0 - t) * direction[i] * direction[j]
        result[i + 3][i + 3] = a
    return result


def symbolic_checks() -> dict[str, bool]:
    lam, a, beta = sp.symbols("lambda a beta", nonzero=True, real=True)
    omega = sp.diag(sp.Matrix([[0, 1], [-1, 0]]), sp.Matrix([[0, 1], [-1, 0]]))
    dilation = sp.Matrix(
        [
            [lam, 0, beta, 0],
            [0, a, 0, beta],
            [-beta / a, 0, 1, 0],
            [0, -a * beta, 0, a * lam],
        ]
    )
    residual = (dilation.T * omega * dilation - omega).subs(beta**2, 1 - a * lam)
    residual = residual.applyfunc(sp.simplify)
    boundary = sp.Matrix(
        [
            [lam, 0, 1, 0],
            [0, 0, 0, 1],
            [-1, 0, 0, 0],
            [0, -1, 0, lam],
        ]
    )
    q, p = sp.symbols("q p", real=True)
    initial = sp.Matrix([q, p, 0, 0])
    twice = sp.simplify(dilation * dilation * initial)
    commutator = sp.Matrix([[0, 1], [-1, 0]]) * sp.diag(lam, a) \
        - sp.diag(lam, a) * sp.Matrix([[0, 1], [-1, 0]])
    return {
        "positive_a_pair_symplectic": residual == sp.zeros(4),
        "zero_a_pair_symplectic": boundary.T * omega * boundary == omega,
        "two_step_q_formula": sp.simplify(
            twice[0] - lam**2 * q + beta**2 * q / a
        ) == 0,
        "two_step_p_formula": sp.simplify(
            twice[1] - a**2 * p + a * beta**2 * p
        ) == 0,
        "passive_commutator_requires_equal_scales": commutator
        == sp.Matrix([[0, a - lam], [a - lam, 0]]),
    }


def main() -> int:
    prereg_path = REPO / (
        "docs/theory/10_eft_program/preregistrations/"
        "PREREG_GENESIS_MINIMAL_BATH_v1.md"
    )
    assert sha256(prereg_path) == PREREG_SHA
    source_hashes = {}
    for key, (relative, expected) in LOCKED_HASHES.items():
        observed = sha256(REPO / relative)
        assert observed == expected, (key, observed, expected)
        source_hashes[key] = observed
    source_hashes["preregistration"] = sha256(prereg_path)

    symbolic = symbolic_checks()
    assert all(symbolic.values()), symbolic

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
    excesses = [0.125, 0.5, 1.25]
    drains = [0.0, 0.5, 0.9, 1.0]
    omega6 = canonical(3)
    omega6_inverse = [[-entry for entry in row] for row in omega6]
    omega4 = pair_canonical()
    prepared = [0.7, -0.4, 0.0, 0.0]

    matrix_arms = pair_arms = defective_pair_arms = 0
    rank_four_arms = rank_six_arms = 0
    max_symplectic = max_projection = max_twice = 0.0
    max_rank_gap = 0
    minimum_deviation = math.inf
    minimum_commutator = math.inf
    ranks_saturate = True

    for direction in directions:
        for excess in excesses:
            t = excess / (1.0 + excess)
            for drain in drains:
                a = 1.0 - drain
                m = system_map(direction, t, a)
                defect = subtract(
                    omega6, multiply(transpose(m), multiply(omega6, m))
                )
                dual_defect = subtract(
                    omega6_inverse,
                    multiply(m, multiply(omega6_inverse, transpose(m))),
                )
                defect_rank = rank(defect)
                dual_rank = rank(dual_defect)
                max_rank_gap = max(max_rank_gap, abs(defect_rank - dual_rank))
                expected_rank = 4 if drain == 0.0 else 6
                assert defect_rank == expected_rank
                matrix_arms += 1
                rank_four_arms += defect_rank == 4
                rank_six_arms += defect_rank == 6

                commutator = subtract(multiply(omega6, m), multiply(m, omega6))
                minimum_commutator = min(minimum_commutator, max_abs(commutator))

                coupled_pairs = 0
                for lam in (1.0, t, t):
                    pair_arms += 1
                    factor = 1.0 - a * lam
                    if factor <= 1.0e-14:
                        continue
                    coupled_pairs += 1
                    defective_pair_arms += 1
                    dilation = pair_dilation(lam, a)
                    residual = subtract(
                        multiply(transpose(dilation), multiply(omega4, dilation)),
                        omega4,
                    )
                    max_symplectic = max(max_symplectic, max_abs(residual))
                    b_rank = rank([row[2:4] for row in dilation[0:2]])
                    c_rank = rank([row[0:2] for row in dilation[2:4]])
                    ranks_saturate = ranks_saturate and b_rank == 2 and c_rank == 2

                    once = apply(dilation, prepared)
                    max_projection = max(
                        max_projection,
                        abs(once[0] - lam * prepared[0]),
                        abs(once[1] - a * prepared[1]),
                    )
                    twice = apply(dilation, once)
                    measured_dq = twice[0] - lam * lam * prepared[0]
                    measured_dp = twice[1] - a * a * prepared[1]
                    expected_dq = -prepared[0] if a == 0.0 else -factor * prepared[0] / a
                    expected_dp = -prepared[1] if a == 0.0 else -a * factor * prepared[1]
                    max_twice = max(
                        max_twice,
                        abs(measured_dq - expected_dq),
                        abs(measured_dp - expected_dp),
                    )
                    minimum_deviation = min(
                        minimum_deviation, math.hypot(measured_dq, measured_dp)
                    )
                assert 2 * coupled_pairs == defect_rank

    passes = (
        matrix_arms == 120
        and pair_arms == 360
        and defective_pair_arms == 330
        and rank_four_arms == 30
        and rank_six_arms == 90
        and max_rank_gap == 0
        and max_symplectic <= TOL
        and max_projection <= TOL
        and max_twice <= TOL
        and minimum_deviation > 0.0
        and minimum_commutator > 0.0
        and ranks_saturate
    )
    assert passes

    implementation_hashes = {
        key: sha256(REPO / relative)
        for key, relative in IMPLEMENTATION_PATHS.items()
    }
    record = {
        "ftd_id": "FTD-0572",
        "verdict": "MINIMAL_FEEDBACK_DILATION_REQUIRES_RESET_OR_ACTIVE_ENERGY_RESERVOIR",
        "platform": f"{platform.system()}-{platform.release()}-{platform.version()}",
        "field_representation": "accepted genesis derivative plus minimum canonical bath pairs",
        "tolerance": TOL,
        "matrix_arms": matrix_arms,
        "pair_arms": pair_arms,
        "defective_pair_arms": defective_pair_arms,
        "rank_four_arms": rank_four_arms,
        "rank_six_arms": rank_six_arms,
        "minimum_bath_pairs_zero_drain": 2,
        "minimum_bath_pairs_positive_drain": 3,
        "maximum_pair_symplectic_residual": max_symplectic,
        "maximum_prepared_projection_residual": max_projection,
        "maximum_two_step_formula_residual": max_twice,
        "minimum_nonzero_two_step_deviation": minimum_deviation,
        "minimum_passive_commutator": minimum_commutator,
        "maximum_primal_dual_defect_rank_gap": max_rank_gap,
        "symbolic_checks": symbolic,
        "rank_lower_bound_proved": True,
        "feedback_and_record_ranks_saturate": ranks_saturate,
        "minimum_dilation_constructed": True,
        "fixed_zero_bath_section_cannot_repeat": True,
        "passive_equal_weight_energy_obstructed": True,
        "native_environment_derived": False,
        "production_changed": False,
        "passes": passes,
        "source_hashes_sha256": source_hashes,
        "implementation_hashes_sha256": implementation_hashes,
    }
    output = REPO / "engine/results/ftd_0572/windows_msvc_cpu.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(record, indent=2))
    print("PASS: minimum genesis bath is constructive but reset/active-energy priced")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
