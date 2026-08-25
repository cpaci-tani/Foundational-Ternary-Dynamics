#!/usr/bin/env python3
"""Exact C18 charge-even constraint-bundle and axial-context price.

The certificate constructs an EM-neutral transverse constraint bundle and
proves the axial stabilizer/two-owner obstruction.  It is a finite structural
census, not a numerical search or physical coupling fit.
"""

from __future__ import annotations

import copy
import hashlib
import itertools
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]

LOCKED_HASHES = {
    ROOT / "scripts/proofs/proof_c18_two_record_momentum_sector_census.py":
        "1EC3D1E23B5264B8CF422C376F222B39166B86CF07F473306CF4D8F2199A0A82",
    ROOT / "scripts/proofs/proof_c18_equivariant_single_record_collision_no_go.py":
        "F759EE742070EF3EBF31E0268D89DE2E1B6A9C5877A1977A4947BDE0BEEC76E3",
    ROOT / "scripts/proofs/proof_c18_scalar_stf_vector_constraint_absorption_seam.py":
        "29C6BC475F6DDFCC3FC73DA5683D0F14ECAC96233924084920F682389D1B1F6E",
    ROOT / "scripts/proofs/proof_c18_phase_neutral_shared_charge_stress_vertex.py":
        "13334840F23DBB1D70EFD59B805D97E462EDFC5B4EEC00D5C9FFF784ECEAAF35",
    ROOT / "scripts/proofs/proof_c4_phase_parity_half_admitted_two_polarization_carrier.py":
        "743CE826C259905DEF31CE1F2324EE8C5DB6EF8E04A8AE9B94227833D31F6000",
    ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_C18_CHARGE_EVEN_CONSTRAINT_BUNDLE_AND_AXIAL_CONTEXT_PRICE_v1.md":
        "42A7275C86D27FCBA5264744F6B19EA1E176C5ABF05E37D04DE404C972CCA38B",
}

Vector = tuple[int, int, int]
Matrix3 = tuple[
    tuple[int, int, int],
    tuple[int, int, int],
    tuple[int, int, int],
]

AXES: tuple[Vector, ...] = (
    (1, 0, 0), (-1, 0, 0),
    (0, 1, 0), (0, -1, 0),
    (0, 0, 1), (0, 0, -1),
)


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def add(a: Vector, b: Vector) -> Vector:
    return tuple(a[i] + b[i] for i in range(3))  # type: ignore[return-value]


def sub(a: Vector, b: Vector) -> Vector:
    return tuple(a[i] - b[i] for i in range(3))  # type: ignore[return-value]


def scale(c: int, a: Vector) -> Vector:
    return tuple(c * entry for entry in a)  # type: ignore[return-value]


def dot(a: Vector, b: Vector) -> int:
    return sum(a[i] * b[i] for i in range(3))


def negate(a: Vector) -> Vector:
    return scale(-1, a)


def determinant_3(matrix: Matrix3) -> int:
    a, b, c = matrix
    return (
        a[0] * (b[1] * c[2] - b[2] * c[1])
        - a[1] * (b[0] * c[2] - b[2] * c[0])
        + a[2] * (b[0] * c[1] - b[1] * c[0])
    )


def mat_vec(matrix: Matrix3, vector: Vector) -> Vector:
    return tuple(
        sum(matrix[i][j] * vector[j] for j in range(3))
        for i in range(3)
    )  # type: ignore[return-value]


def signed_permutation_matrices() -> tuple[Matrix3, ...]:
    matrices: list[Matrix3] = []
    for permutation in itertools.permutations(range(3)):
        for signs in itertools.product((-1, 1), repeat=3):
            rows = []
            for row in range(3):
                entries = [0, 0, 0]
                entries[permutation[row]] = signs[row]
                rows.append(tuple(entries))
            matrices.append(tuple(rows))  # type: ignore[arg-type]
    assert len(matrices) == 48
    assert {determinant_3(matrix) for matrix in matrices} == {-1, 1}
    return tuple(matrices)


@dataclass(frozen=True)
class Record:
    record_id: str
    shell: str
    owner: str
    direction: Vector | None
    phase: int
    charge: int
    layer: int = 0
    source_route: str = "seed:0"


@dataclass(frozen=True)
class BundleState:
    records: tuple[Record, ...]
    occupied: tuple[Vector, ...] = ()

    def by_id(self) -> dict[str, Record]:
        return {record.record_id: record for record in self.records}


def make_input(
    r: Vector,
    phases: tuple[int, int, int],
    charge: int,
    *,
    prefix: str = "b",
    layer: int = 0,
    source_route: str = "seed:0",
) -> BundleState:
    return BundleState((
        Record(
            f"{prefix}:sc", "SC", "active", negate(r), phases[0], charge,
            layer, source_route,
        ),
        Record(
            f"{prefix}:f+", "FCC", "reserve", None, phases[1], charge,
            layer, source_route,
        ),
        Record(
            f"{prefix}:f-", "FCC", "reserve", None, phases[2], charge,
            layer, source_route,
        ),
    ))


def apply_bundle(
    state: BundleState,
    r: Vector,
    n: Vector,
    *,
    inverse: bool = False,
) -> BundleState | None:
    if r not in AXES or n not in AXES or dot(r, n) != 0:
        return None
    if len(state.records) != 3 or len(state.by_id()) != 3:
        return None

    records = list(state.records)
    sc = next((record for record in records if record.shell == "SC"), None)
    fcc = [record for record in records if record.shell == "FCC"]
    if sc is None or len(fcc) != 2:
        return None
    if len({record.phase for record in records}) != 1:
        return None

    target_by_id = {
        next(record.record_id for record in fcc if record.record_id.endswith("f+")):
            negate(add(r, n)),
        next(record.record_id for record in fcc if record.record_id.endswith("f-")):
            negate(sub(r, n)),
    } if {
        record.record_id[-2:] for record in fcc
    } == {"f+", "f-"} else None
    if target_by_id is None:
        return None
    target_fcc = set(target_by_id.values())
    if not inverse and target_fcc.intersection(state.occupied):
        return None

    if not inverse:
        if sc.owner != "active" or sc.direction != negate(r):
            return None
        if any(record.owner != "reserve" or record.direction is not None for record in fcc):
            return None

        output = [
            Record(
                sc.record_id, "SC", "active", r,
                sc.phase, sc.charge, sc.layer, sc.source_route
            )
        ]
        for record in fcc:
            output.append(Record(
                record.record_id, "FCC", "active", target_by_id[record.record_id],
                record.phase, record.charge, record.layer, record.source_route
            ))
        return BundleState(
            tuple(sorted(output, key=lambda record: record.record_id)),
            state.occupied,
        )

    if sc.owner != "active" or sc.direction != r:
        return None
    if any(record.owner != "active" for record in fcc):
        return None
    if any(record.direction != target_by_id[record.record_id] for record in fcc):
        return None

    output = [
        Record(
            sc.record_id, "SC", "active", negate(r),
            sc.phase, sc.charge, sc.layer, sc.source_route
        )
    ]
    for record in fcc:
        output.append(Record(
            record.record_id, "FCC", "reserve", None,
            record.phase, record.charge, record.layer, record.source_route
        ))
    return BundleState(
        tuple(sorted(output, key=lambda record: record.record_id)),
        state.occupied,
    )


def moment(state: BundleState, shell: str) -> Vector:
    total = (0, 0, 0)
    for record in state.records:
        if record.shell == shell and record.owner == "active":
            assert record.direction is not None
            total = add(total, record.direction)
    return total


def moment_pair(state: BundleState) -> tuple[Vector, Vector]:
    j_sc = moment(state, "SC")
    j_fcc = moment(state, "FCC")
    return add(j_sc, j_fcc), sub(j_sc, j_fcc)


def transform_state(state: BundleState, matrix: Matrix3) -> BundleState:
    transformed = []
    for record in state.records:
        direction = (
            None if record.direction is None
            else mat_vec(matrix, record.direction)
        )
        transformed.append(Record(
            record.record_id,
            record.shell,
            record.owner,
            direction,
            record.phase,
            record.charge,
            record.layer,
            record.source_route,
        ))
    occupied = tuple(sorted(mat_vec(matrix, direction) for direction in state.occupied))
    return BundleState(
        tuple(sorted(transformed, key=lambda record: record.record_id)),
        occupied,
    )


def phase_shift(state: BundleState, amount: int) -> BundleState:
    return BundleState(tuple(
        Record(
            record.record_id, record.shell, record.owner, record.direction,
            (record.phase + amount) % 4, record.charge, record.layer,
            record.source_route,
        )
        for record in state.records
    ), state.occupied)


def charge_conjugate(state: BundleState) -> BundleState:
    return BundleState(tuple(
        Record(
            record.record_id, record.shell, record.owner, record.direction,
            record.phase, -record.charge, record.layer, record.source_route,
        )
        for record in state.records
    ), state.occupied)


def bundle_census_checks() -> int:
    checks = 0
    ordered_orthogonal = tuple(
        (r, n) for r in AXES for n in AXES if dot(r, n) == 0
    )
    assert len(ordered_orthogonal) == 24
    checks += 1

    for r, n in ordered_orthogonal:
        for phase in range(4):
            for charge in (-1, 1):
                phases = (phase, phase, phase)
                before = make_input(
                    r, phases, charge, source_route="genesis:17/line:5"
                )
                after = apply_bundle(before, r, n)
                assert after is not None
                restored = apply_bundle(after, r, n, inverse=True)
                assert restored == BundleState(
                    tuple(sorted(
                        before.records, key=lambda record: record.record_id
                    )),
                    before.occupied,
                )
                checks += 2

                before_em, before_c = moment_pair(before)
                after_em, after_c = moment_pair(after)
                assert sub(after_em, before_em) == (0, 0, 0)
                assert sub(after_c, before_c) == scale(4, r)
                checks += 2

                before_payload = sorted(
                    (
                        record.record_id, record.phase, record.charge,
                        record.source_route,
                    )
                    for record in before.records
                )
                after_payload = sorted(
                    (
                        record.record_id, record.phase, record.charge,
                        record.source_route,
                    )
                    for record in after.records
                )
                assert before_payload == after_payload
                checks += 1

                for phase_amount in range(4):
                    shifted_then_applied = apply_bundle(
                        phase_shift(before, phase_amount), r, n
                    )
                    applied_then_shifted = phase_shift(after, phase_amount)
                    assert shifted_then_applied == applied_then_shifted
                    checks += 1

                conjugated_then_applied = apply_bundle(
                    charge_conjugate(before), r, n
                )
                applied_then_conjugated = charge_conjugate(after)
                assert conjugated_then_applied == applied_then_conjugated
                assert (
                    sub(
                        moment_pair(applied_then_conjugated)[1],
                        moment_pair(charge_conjugate(before))[1],
                    )
                    == scale(4, r)
                )
                checks += 2

    return checks


def cubic_covariance_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()
    for r in AXES:
        for n in AXES:
            if dot(r, n) != 0:
                continue
            before = make_input(r, (1, 1, 1), 1)
            after = apply_bundle(before, r, n)
            assert after is not None
            for matrix in group:
                transformed_before = transform_state(before, matrix)
                transformed_after = transform_state(after, matrix)
                image = apply_bundle(
                    transformed_before,
                    mat_vec(matrix, r),
                    mat_vec(matrix, n),
                )
                assert image == transformed_after
                checks += 1
    return checks


def failure_and_atomicity_checks() -> int:
    checks = 0
    r, n = (1, 0, 0), (0, 1, 0)
    valid = make_input(r, (2, 2, 2), 1, source_route="route:valid")

    variants: list[BundleState] = []
    records = list(valid.records)
    variants.append(BundleState(tuple(records[:2])))  # missing reserve
    variants.append(BundleState(tuple(records + [records[0]])))  # duplicate id
    variants.append(BundleState(tuple(
        Record(
            record.record_id,
            record.shell,
            "active" if record.shell == "FCC" else record.owner,
            (1, 1, 0) if record.shell == "FCC" else record.direction,
            record.phase,
            record.charge,
            record.layer,
            record.source_route,
        )
        for record in records
    )))
    variants.append(BundleState(tuple(
        Record(
            record.record_id,
            record.shell,
            record.owner,
            r if record.shell == "SC" else record.direction,
            record.phase,
            record.charge,
            record.layer,
            record.source_route,
        )
        for record in records
    )))
    variants.append(make_input(r, (0, 1, 0), 1))  # inconsistent C4 phase
    variants.append(BundleState(
        valid.records,
        (negate(add(r, n)),),
    ))  # occupied target channel

    for variant in variants:
        snapshot = copy.deepcopy(variant)
        assert apply_bundle(variant, r, n) is None
        assert variant == snapshot
        checks += 2

    # Disjoint record sets commute.
    first = make_input(r, (0, 0, 0), 1, prefix="a", layer=0)
    second = make_input((0, 1, 0), (3, 3, 3), -1, prefix="b", layer=1)
    first_after = apply_bundle(first, r, n)
    second_after = apply_bundle(second, (0, 1, 0), (0, 0, 1))
    assert first_after is not None and second_after is not None
    combined_ab = tuple(sorted(
        first_after.records + second_after.records,
        key=lambda record: record.record_id,
    ))
    combined_ba = tuple(sorted(
        second_after.records + first_after.records,
        key=lambda record: record.record_id,
    ))
    assert combined_ab == combined_ba
    checks += 1

    # Two bundles sharing one SC owner cannot be admitted atomically.
    n2 = (0, 0, 1)
    one_after = apply_bundle(valid, r, n)
    assert one_after is not None
    assert apply_bundle(one_after, r, n2) is None
    checks += 2

    return checks


def event_source_times_216(r: Vector, q: Vector) -> tuple[Fraction, Fraction, Fraction]:
    # 216 * [(rr^T-I/3)/18] q = 12(rr^T-I/3)q.
    rq = dot(r, q)
    return tuple(
        Fraction(12 * rq * r[i] - 4 * q[i])
        for i in range(3)
    )  # type: ignore[return-value]


def source_matching_checks() -> int:
    checks = 0
    for r in AXES:
        for q in AXES:
            v = sub(scale(3 * dot(r, q), r), q)
            target = scale(4, v)
            exact = event_source_times_216(r, q)
            assert exact == tuple(Fraction(entry) for entry in target)
            checks += 1

            if dot(r, q) == 0:
                before = make_input(negate(q), (0, 0, 0), 1)
                after = apply_bundle(before, negate(q), r)
                assert after is not None
                delta_c = sub(moment_pair(after)[1], moment_pair(before)[1])
                assert delta_c == target == scale(-4, q)
                assert sub(moment_pair(after)[0], moment_pair(before)[0]) == (0, 0, 0)
                checks += 3
            else:
                sign_axis = r if q == r else negate(r)
                transverse = tuple(axis for axis in AXES if dot(axis, r) == 0)
                # Choose one representative from each unoriented transverse line.
                choices: list[Vector] = []
                for axis in transverse:
                    if negate(axis) not in choices:
                        choices.append(axis)
                assert len(choices) == 2

                deltas = []
                for n in choices:
                    before = make_input(sign_axis, (0, 0, 0), 1)
                    after = apply_bundle(before, sign_axis, n)
                    assert after is not None
                    deltas.append(sub(moment_pair(after)[1], moment_pair(before)[1]))
                assert all(delta == scale(4, sign_axis) for delta in deltas)
                assert deltas[0] != target
                assert add(deltas[0], deltas[1]) == target
                checks += 4

    return checks


def canonical_axis_line(axis: Vector) -> Vector:
    for candidate in ((1, 0, 0), (0, 1, 0), (0, 0, 1)):
        if axis == candidate or axis == negate(candidate):
            return candidate
    raise AssertionError(axis)


def axial_stabilizer_checks() -> int:
    checks = 0
    group = signed_permutation_matrices()

    for r in AXES:
        for q in (r, negate(r)):
            stabilizer = tuple(
                matrix
                for matrix in group
                if mat_vec(matrix, r) == r and mat_vec(matrix, q) == q
            )
            assert len(stabilizer) == 8
            checks += 1

            transverse_lines = {
                canonical_axis_line(axis)
                for axis in AXES
                if dot(axis, r) == 0
            }
            assert len(transverse_lines) == 2
            checks += 1

            for selected in transverse_lines:
                orbit = {
                    canonical_axis_line(mat_vec(matrix, selected))
                    for matrix in stabilizer
                }
                assert orbit == transverse_lines
                assert any(
                    canonical_axis_line(mat_vec(matrix, selected)) != selected
                    for matrix in stabilizer
                )
                checks += 2

                # A scalar C4 phase is fixed by the spatial stabilizer and
                # therefore cannot reduce this two-line orbit.
                for phase in range(4):
                    phase_orbit = {
                        (canonical_axis_line(mat_vec(matrix, selected)), phase)
                        for matrix in stabilizer
                    }
                    assert len(phase_orbit) == 2
                    checks += 1

            # The unordered pair of both plane choices is invariant.
            for matrix in stabilizer:
                image_pair = {
                    canonical_axis_line(mat_vec(matrix, axis))
                    for axis in transverse_lines
                }
                assert image_pair == transverse_lines
                checks += 1

    return checks


def fcc_d3_decomposition(vector: Vector) -> tuple[int, int, int] | None:
    """Return coefficients in three FCC generators, or None off D3."""
    x, y, z = vector
    if (x + y + z) % 2 != 0:
        return None
    # a(1,1,0)+b(1,-1,0)+c(1,0,1)=(x,y,z).
    a = (x - z + y) // 2
    b = (x - z - y) // 2
    c = z
    return a, b, c


def lattice_minimality_checks() -> int:
    checks = 0
    # FCC integer first moments lie in D3: coordinate sum is even.
    coefficients = range(-2, 3)
    fcc_directions = (
        (1, 1, 0), (1, -1, 0),
        (1, 0, 1), (1, 0, -1),
        (0, 1, 1), (0, 1, -1),
    )
    for weights in itertools.product(coefficients, repeat=6):
        vector = (0, 0, 0)
        for weight, direction in zip(weights, fcc_directions):
            vector = add(vector, scale(weight, direction))
        assert sum(vector) % 2 == 0
        checks += 1

    # Constructive converse: every integer vector with even coordinate sum
    # is generated by three FCC directions.  The bounded census exercises
    # the exact formula; the formula itself is valid for arbitrary integers.
    basis = ((1, 1, 0), (1, -1, 0), (1, 0, 1))
    for vector in itertools.product(range(-4, 5), repeat=3):
        coefficients = fcc_d3_decomposition(vector)
        if sum(vector) % 2 != 0:
            assert coefficients is None
        else:
            assert coefficients is not None
            reconstructed = (0, 0, 0)
            for coefficient, direction in zip(coefficients, basis):
                reconstructed = add(
                    reconstructed, scale(coefficient, direction)
                )
            assert reconstructed == vector
        checks += 1

    # An axial D3 vector k r has even k.  Under EM neutrality
    # Delta J_C=2 Delta J_SC, so the minimum nonzero axial step is 4 r.
    for r in AXES:
        for coefficient in range(-4, 5):
            axial = scale(coefficient, r)
            assert (
                fcc_d3_decomposition(axial) is not None
            ) == (coefficient % 2 == 0)
            checks += 1
        positive = tuple(
            coefficient
            for coefficient in range(1, 6)
            if fcc_d3_decomposition(scale(coefficient, r)) is not None
        )
        assert min(positive) == 2
        assert scale(2 * min(positive), r) == scale(4, r)
        checks += 2

    return checks


def main() -> None:
    checks = 0
    for path, expected in LOCKED_HASHES.items():
        actual = sha256(path)
        assert actual == expected, (path, actual, expected)
        checks += 1

    checks += bundle_census_checks()
    checks += cubic_covariance_checks()
    checks += failure_and_atomicity_checks()
    checks += source_matching_checks()
    checks += axial_stabilizer_checks()
    checks += lattice_minimality_checks()

    print("one transverse plane bundle is phase complete, charge even, and EM neutral")
    print("its spare-vector increment is the minimum axial lattice step 4 r")
    print("one transverse bundle realizes 216 T_r q when q is perpendicular to r")
    print("axial incidence requires two plane bundles and twice the spare-vector load")
    print("the axial D4 stabilizer forbids a context-free one-plane selector")
    print("one C18 line owner cannot execute both axial bundles atomically")
    print(
        f"PASS: C18 charge-even constraint bundle and axial-context price "
        f"({checks} exact checks)"
    )
    print(
        "OUTCOME B: exact transverse finite bundle; axial source needs a second "
        "owner, distributed support, or explicit paired plane context"
    )


if __name__ == "__main__":
    main()
