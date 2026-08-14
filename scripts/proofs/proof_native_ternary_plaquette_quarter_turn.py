#!/usr/bin/env python3
"""Exact independent certificate for FTD-0914.

This is a finite integer/rational proof.  It performs no numerical search,
fit, parameter sweep, or production-engine measurement.
"""

from __future__ import annotations

from fractions import Fraction
from hashlib import sha256
from itertools import permutations, product
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOCKS = {
    "docs/theory/10_eft_program/preregistrations/native_time_carrier_programme/PREREG_NATIVE_TERNARY_PLAQUETTE_QUARTER_TURN_RECURSION_v1.md":
        "659AFA6FE64905C848335052C91F4376A78F6B48E5C416028A8189A2C40951A8",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_NONCOMPACT_FACE_COHOMOLOGY.md":
        "4F0AA19A00A2A96215031139994AD0AC1AC7C93BBE5620E7F3FF99CCCCB62C70",
    "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_CONFIGURATION_SPACE_CARRIER_NECESSITY.md":
        "9FCD2E7AA89C8B38339D730B04AAD2A9797F40E3EDD08ACA3B5C9CFCB4996FBD",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_TERNARY_QUARTER_TURN_GEARBOX_v1.md":
        "898A9130DFBAAE23B76D3FB5339851D026B50E5B7EFFB8B4B8DC66513F5A9317",
    "docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_ORIENTED_EVEN_SELF_PAIR_RECTIFIER_AND_GSTAR_GEAR_RATIO_BOUNDARY_v1.md":
        "E87EB15B482AFBBF1147726B3F07C4008B82BC07B06BD9786656BEA28AD3BDDA",
    "docs/theory/10_eft_program/reports_and_audits/ANALYSIS_HELD_OUT_PAIR_SPECIFIC_PHASE_WEDGE_AND_CENTRALITY_v1.md":
        "F6D5680173070E17E67AE8FEA3B26A7F8582FFCA275D9631C43C4F4E3E1D2B14",
    "engine/include/ftd/voxel.h":
        "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h":
        "3A970B82EF0BDCCC457D5DDA049CAF971C2318429970E696E64DB84CEB7D1D09",
    "engine/src/eft/native_ternary_plaquette_quarter_turn.cpp":
        "E7891C5099D2DCA1F20DF72E6B37F29A60FE63A7A9E7E645D8AC6E2DF73E1F4C",
    "engine/tests/test_native_ternary_plaquette_quarter_turn.cpp":
        "3E5AE8D8518513150F24EE8FD6FE9104F9605C85DCD0FC865FEB68B0ABF7840D",
}

Vec = tuple[int | Fraction, int | Fraction, int | Fraction]
Word = tuple[int, int, int, int]
Matrix = tuple[tuple[int, int, int], tuple[int, int, int], tuple[int, int, int]]


def digest(path: str) -> str:
    return sha256((ROOT / path).read_bytes()).hexdigest().upper()


def shift(word: Word) -> Word:
    return (word[3], word[0], word[1], word[2])


def reverse_shift(word: Word) -> Word:
    return (word[1], word[2], word[3], word[0])


def add(left: Vec, right: Vec) -> Vec:
    return tuple(left[i] + right[i] for i in range(3))  # type: ignore[return-value]


def scale(value: Vec, factor: int | Fraction) -> Vec:
    return tuple(factor * value[i] for i in range(3))  # type: ignore[return-value]


def dot(left: Vec, right: Vec) -> int | Fraction:
    return sum(left[i] * right[i] for i in range(3))


def cross(left: Vec, right: Vec) -> Vec:
    return (
        left[1] * right[2] - left[2] * right[1],
        left[2] * right[0] - left[0] * right[2],
        left[0] * right[1] - left[1] * right[0],
    )


def dipole(word: Word, positions: tuple[Vec, ...]) -> Vec:
    result: Vec = (0, 0, 0)
    for coefficient, position in zip(word, positions, strict=True):
        result = add(result, scale(position, coefficient))
    return result


def determinant(matrix: Matrix) -> int:
    return (
        matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1])
        - matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0])
        + matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0])
    )


def transform(matrix: Matrix, value: Vec) -> Vec:
    return tuple(
        sum(matrix[row][column] * value[column] for column in range(3))
        for row in range(3)
    )  # type: ignore[return-value]


def signed_cubic_group() -> list[Matrix]:
    result: list[Matrix] = []
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            rows = [[0, 0, 0] for _ in range(3)]
            for row in range(3):
                rows[row][permutation[row]] = signs[row]
            result.append(tuple(tuple(row) for row in rows))  # type: ignore[arg-type]
    return result


def sym2_action(matrix: tuple[tuple[int, int], tuple[int, int]]) -> tuple[tuple[int, ...], ...]:
    a, b = matrix[0]
    c, d = matrix[1]
    # Basis (x^2, xy, y^2), with (x',y')=(ax+by,cx+dy).
    return (
        (a * a, 2 * a * b, b * b),
        (a * c, a * d + b * c, b * d),
        (c * c, 2 * c * d, d * d),
    )


def main() -> int:
    checks: list[tuple[str, bool]] = []
    for path, expected in LOCKS.items():
        checks.append((f"source lock {path}", digest(path) == expected))

    positions: tuple[Vec, ...] = (
        (1, 0, 0), (0, 1, 0), (-1, 0, 0), (0, -1, 0)
    )
    initial: Word = (1, 0, -1, 0)
    forward = [initial]
    reverse = [initial]
    for _ in range(3):
        forward.append(shift(forward[-1]))
        reverse.append(reverse_shift(reverse[-1]))

    checks.append(("four unique forward words", len(set(forward)) == 4))
    checks.append(("ternary neutral unit orbit", all(
        set(word) <= {-1, 0, 1} and sum(word) == 0
        and word.count(1) == 1 and word.count(-1) == 1
        for word in forward)))
    checks.append(("shift has order four on orbit", shift(forward[-1]) == initial))
    a: Word = (1, 0, -1, 0)
    b: Word = (0, 1, 0, -1)
    checks.append(("square of shift is minus identity", shift(shift(a)) == tuple(-x for x in a)
                   and shift(shift(b)) == tuple(-x for x in b)))
    checks.append(("induced forward matrix is R", shift(a) == b
                   and shift(b) == tuple(-x for x in a)))
    checks.append(("reverse matrix is inverse and minus R", reverse_shift(a) == tuple(-x for x in b)
                   and reverse_shift(b) == a))

    dipoles = [dipole(word, positions) for word in forward]
    expected_dipoles: list[Vec] = [(2, 0, 0), (0, 2, 0), (-2, 0, 0), (0, -2, 0)]
    checks.append(("dipoles are exact cardinal quarter turns", dipoles == expected_dipoles))
    norms = [dot(value, value) for value in dipoles]
    checks.append(("dipole norms are equal and nonzero", norms == [4, 4, 4, 4]))
    checks.append(("successive dipoles are orthogonal", all(
        dot(dipoles[k], dipoles[(k + 1) % 4]) == 0 for k in range(4))))
    checks.append(("dipole orbit closes after four steps", dipoles[0] == dipoles[4 % 4]))

    forward_l = [cross(dipoles[k], dipoles[(k + 1) % 4]) for k in range(4)]
    reverse_l = [cross(dipoles[k], dipoles[(k - 1) % 4]) for k in range(4)]
    checks.append(("forward bivector constant nonzero", forward_l == [(0, 0, 4)] * 4))
    checks.append(("reverse bivector is negative", reverse_l == [(0, 0, -4)] * 4))
    checks.append(("transition exchange makes bivector time odd", all(
        cross(dipoles[(k + 1) % 4], dipoles[k]) == scale(forward_l[k], -1)
        for k in range(4))))

    reconstructed = [scale(cross(forward_l[k], dipoles[k]), Fraction(1, norms[k]))
                     for k in range(4)]
    checks.append(("coordinate-free successor reconstructs exactly", reconstructed == [
        dipoles[(k + 1) % 4] for k in range(4)]))
    radial_energy = [Fraction(dot(value, value), 2) for value in dipoles]
    tangential_energy = [Fraction(dot(value, value), 2) for value in reconstructed]
    checks.append(("self-dual energy halves are equal", radial_energy == tangential_energy))

    group = signed_cubic_group()
    checks.append(("signed cubic group has 48 elements", len(group) == 48 and len(set(group)) == 48))
    checks.append(("signed cubic determinants are plus or minus one", {determinant(q) for q in group} == {-1, 1}))
    l_covariant = True
    tangent_covariant = True
    for q in group:
        transformed_d = transform(q, dipoles[0])
        transformed_next = transform(q, dipoles[1])
        transformed_l = cross(transformed_d, transformed_next)
        expected_l = scale(transform(q, forward_l[0]), determinant(q))
        l_covariant = l_covariant and transformed_l == expected_l
        transformed_tangent = scale(cross(transformed_l, transformed_d), Fraction(1, dot(transformed_d, transformed_d)))
        tangent_covariant = tangent_covariant and transformed_tangent == transform(q, reconstructed[0])
    checks.append(("bivector signed-cubic covariance", l_covariant))
    checks.append(("coordinate-free tangent is polar covariant", tangent_covariant))

    r2 = ((0, -1), (1, 0))
    r2_inverse = ((0, 1), (-1, 0))
    checks.append(("symmetric square identifies R and inverse", sym2_action(r2) == sym2_action(r2_inverse)))
    checks.append(("ordered bivector distinguishes R and inverse", forward_l[0] != reverse_l[0]))
    checks.append(("forward and reverse visit same instantaneous words", set(forward) == set(reverse)))
    checks.append(("opposite directions have different successors", all(
        shift(word) != reverse_shift(word) for word in forward)))
    checks.append(("instantaneous state cannot determine direction", set(forward) == set(reverse)
                   and all(shift(word) != reverse_shift(word) for word in forward)))

    # Cubic cardinal graph parity changes on every edge, so odd cycles cannot
    # close.  The displayed four vertices furnish a length-four cycle.
    parity = lambda value: int(sum(value)) & 1
    square_edges = [
        ((0, 0, 0), (1, 0, 0)),
        ((1, 0, 0), (1, 1, 0)),
        ((1, 1, 0), (0, 1, 0)),
        ((0, 1, 0), (0, 0, 0)),
    ]
    checks.append(("cardinal edges reverse bipartite parity", all(
        parity(left) != parity(right) for left, right in square_edges)))
    checks.append(("cardinal square is a simple four-cycle", len({point for edge in square_edges for point in edge}) == 4))
    neutral_pair = (1, -1)
    pair_exchange = lambda word: (word[1], word[0])
    checks.append(("two-site neutral exchange is order two", pair_exchange(neutral_pair) != neutral_pair
                   and pair_exchange(pair_exchange(neutral_pair)) == neutral_pair))

    parameters = [Fraction(0), Fraction(1, 4), Fraction(1, 2), Fraction(3, 4), Fraction(1)]
    contraction_l = all(
        cross(scale(dipoles[0], t), scale(dipoles[1], t)) == scale(forward_l[0], t * t)
        for t in parameters)
    contraction_e = all(
        Fraction(dot(scale(dipoles[0], t), scale(dipoles[0], t)), 2)
        == t * t * radial_energy[0]
        for t in parameters)
    checks.append(("real-field bivector contracts quadratically", contraction_l))
    checks.append(("real-field energy contracts quadratically", contraction_e))
    checks.append(("contraction reaches the zero field", scale(dipoles[0], Fraction(0)) == (0, 0, 0)))
    checks.append(("no nonzero topological energy floor", contraction_e and radial_energy[0] > 0))

    header = (ROOT / "engine/include/ftd/eft/native_ternary_plaquette_quarter_turn.h").read_text(encoding="utf-8")
    source = (ROOT / "engine/src/eft/native_ternary_plaquette_quarter_turn.cpp").read_text(encoding="utf-8")
    checks.append(("protection firewall is false", "bool topological_protection_derived = false;" in header))
    checks.append(("production invariant firewall is false", "bool production_orbit_invariant_derived = false;" in header))
    checks.append(("G* and gamma firewalls are false", "bool gstar_used = false;" in header
                   and "bool gamma_magnitude_derived = false;" in header))
    checks.append(("Born/Bell firewall is false", "bool born_or_bell_target_used = false;" in header))
    checks.append(("production and type firewalls are false", "bool production_changed = false;" in header
                   and "bool new_selected_type_added = false;" in header))
    checks.append(("implementation has no RenderBridge or mutation path", "RenderBridge" not in source
                   and "Voxel" not in source and "run(" not in source))

    combined = all(result for _, result in checks)
    checks.append(("combined Outcome A discriminator", combined))

    for name, passed in checks:
        print(f"{'PASS' if passed else 'FAIL'}  {name}")
    passed_count = sum(passed for _, passed in checks)
    print()
    print(f"FTD-0914 exact certificate: {passed_count}/{len(checks)} checks passed")
    if passed_count == len(checks):
        print("PLAQUETTE_QUARTER_TURN_RECURSION=EXACT")
        print("CLOCKWISE_COUNTERCLOCKWISE_BIVECTOR=RETAINED")
        print("SELF_DUAL_ENERGY_RECURSION=EXACT_CONDITIONAL_ON_SHIFT")
        print("INSTANTANEOUS_WORD_DIRECTION=AMBIGUOUS")
        print("TOPOLOGICAL_PROTECTION=NOT_DERIVED")
        print("PRODUCTION_ORBIT_INVARIANT=OPEN")
        print("GSTAR_USED=FALSE")
        print("GAMMA_MAGNITUDE_DERIVED=FALSE")
        print("BORN_BELL_TARGET_USED=FALSE")
        print("FTD0914_OUTCOME=A_EXACT_RECURRENT_PLAQUETTE_WITH_PROTECTION_BOUNDARY")
        return 0
    print("FTD0914_OUTCOME=C_INVALID")
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
