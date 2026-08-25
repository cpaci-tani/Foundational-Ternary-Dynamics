#!/usr/bin/env python3
"""Exact C18 two-record momentum-sector census and collision boundary.

The certificate enumerates unordered pairs of distinct directed C18 channels,
groups them by shell content and total lattice momentum, and constructs the
target-blind involution which swaps the two microstates of every doubleton
sector.  It also tests the stabilizer obstruction to routing two unequal
spatial-scalar phase payloads through those scatterings.

No numerical target or physical constant is used.
"""

from __future__ import annotations

from collections import Counter, defaultdict
from itertools import combinations

from proof_c18_equivariant_single_record_collision_no_go import (
    FCC_DIRECTIONS,
    SC_DIRECTIONS,
    add,
)
from proof_moore_bond_capacity_type_census import (
    Matrix3,
    Vector,
    matrix_vector,
    signed_permutation_matrices,
)


Pair = tuple[Vector, Vector]
SectorKey = tuple[str, Vector]
PhaseRecord = tuple[Vector, int]
PhasePair = tuple[PhaseRecord, PhaseRecord]
DIRECTIONS: tuple[Vector, ...] = SC_DIRECTIONS + FCC_DIRECTIONS


def canonical_pair(left: Vector, right: Vector) -> Pair:
    return tuple(sorted((left, right)))  # type: ignore[return-value]


def shell(direction: Vector) -> str:
    if direction in SC_DIRECTIONS:
        return "S"
    if direction in FCC_DIRECTIONS:
        return "F"
    raise ValueError(f"not a C18 direction: {direction}")


def sector_key(pair: Pair) -> SectorKey:
    shell_content = "".join(sorted((shell(pair[0]), shell(pair[1]))))
    return shell_content, add(pair[0], pair[1])


def transform_pair(matrix: Matrix3, pair: Pair) -> Pair:
    return canonical_pair(
        matrix_vector(matrix, pair[0]),
        matrix_vector(matrix, pair[1]),
    )


def dot(left: Vector, right: Vector) -> int:
    return sum(left[index] * right[index] for index in range(3))


def norm_squared(vector: Vector) -> int:
    return dot(vector, vector)


def canonical_phase_pair(left: PhaseRecord, right: PhaseRecord) -> PhasePair:
    return tuple(sorted((left, right), key=lambda record: record[0]))  # type: ignore[return-value]


def transform_phase_pair(matrix: Matrix3, state: PhasePair) -> PhasePair:
    return canonical_phase_pair(
        (matrix_vector(matrix, state[0][0]), state[0][1]),
        (matrix_vector(matrix, state[1][0]), state[1][1]),
    )


def shift_phase_pair(state: PhasePair, shift: int) -> PhasePair:
    return canonical_phase_pair(
        (state[0][0], (state[0][1] + shift) % 4),
        (state[1][0], (state[1][1] + shift) % 4),
    )


def build_sectors(pairs: tuple[Pair, ...]) -> dict[SectorKey, tuple[Pair, ...]]:
    sectors: defaultdict[SectorKey, list[Pair]] = defaultdict(list)
    for pair in pairs:
        sectors[sector_key(pair)].append(pair)
    return {key: tuple(sorted(values)) for key, values in sectors.items()}


def doubleton_swap(
    pair: Pair,
    sectors: dict[SectorKey, tuple[Pair, ...]],
) -> Pair:
    sector = sectors[sector_key(pair)]
    if len(sector) != 2:
        return pair
    return sector[1] if pair == sector[0] else sector[0]


def payload_routing_obstructed(
    pair: Pair,
    outgoing: Pair,
    group: tuple[Matrix3, ...],
) -> bool:
    """Whether the pointwise input stabilizer exchanges output directions.

    With unequal scalar payloads attached to the two input directions, every
    symmetry fixing each input direction also fixes the complete input.  If
    one such symmetry swaps the two output directions, no equivariant routing
    can assign the two unequal payloads to individual output channels.
    """

    for matrix in group:
        fixes_input_pointwise = all(
            matrix_vector(matrix, direction) == direction for direction in pair
        )
        swaps_output = (
            matrix_vector(matrix, outgoing[0]) == outgoing[1]
            and matrix_vector(matrix, outgoing[1]) == outgoing[0]
        )
        if fixes_input_pointwise and swaps_output:
            return True
    return False


def phase_complete_collision(
    state: PhasePair,
    sectors: dict[SectorKey, tuple[Pair, ...]],
) -> PhasePair:
    """Minimum phase-complete collision on exact two-token occupancy.

    Every grazing FCC doubleton (|P|^2=2) has a unique maximum-dot routing of
    each payload to the alternative direction pair.  Every axial FCC
    doubleton (|P|^2=4) has a reflection ambiguity, so it scatters only when
    the two phases are equal and routing is physically indistinguishable.
    All other states fail closed to identity.
    """

    pair = canonical_pair(state[0][0], state[1][0])
    sector = sectors[sector_key(pair)]
    if len(sector) != 2:
        return state
    outgoing = sector[1] if pair == sector[0] else sector[0]
    momentum_norm = norm_squared(sector_key(pair)[1])

    if momentum_norm == 2:
        routed: list[PhaseRecord] = []
        for incoming_direction, phase in state:
            scores = tuple(dot(incoming_direction, direction) for direction in outgoing)
            if scores[0] == scores[1]:
                raise AssertionError("grazing sector lost unique maximum-dot routing")
            routed.append((outgoing[0 if scores[0] > scores[1] else 1], phase))
        return canonical_phase_pair(routed[0], routed[1])

    if momentum_norm == 4 and state[0][1] == state[1][1]:
        phase = state[0][1]
        return canonical_phase_pair((outgoing[0], phase), (outgoing[1], phase))

    return state


def main() -> None:
    checks = 0
    group = tuple(signed_permutation_matrices())
    pairs = tuple(canonical_pair(left, right) for left, right in combinations(DIRECTIONS, 2))
    sectors = build_sectors(pairs)

    assert len(group) == 48
    assert len(DIRECTIONS) == 18
    assert len(pairs) == 153
    assert sum(len(sector) for sector in sectors.values()) == 153
    checks += 4

    histogram = Counter((key[0], len(sector)) for key, sector in sectors.items())
    state_histogram = Counter((sector_key(pair)[0], len(sectors[sector_key(pair)])) for pair in pairs)
    assert histogram == Counter(
        {
            ("SS", 1): 12,
            ("SS", 3): 1,
            ("FS", 1): 24,
            ("FS", 3): 8,
            ("FS", 4): 6,
            ("FF", 1): 24,
            ("FF", 2): 18,
            ("FF", 6): 1,
        }
    )
    checks += 1

    collision = {pair: doubleton_swap(pair, sectors) for pair in pairs}
    assert set(collision) == set(pairs)
    assert set(collision.values()) == set(pairs)
    assert all(collision[collision[pair]] == pair for pair in pairs)
    checks += 3

    nontrivial = tuple(pair for pair in pairs if collision[pair] != pair)
    doubleton_sectors = tuple(sector for sector in sectors.values() if len(sector) == 2)
    doubleton_momentum_histogram = Counter(
        norm_squared(sector_key(sector[0])[1]) for sector in doubleton_sectors
    )
    assert doubleton_momentum_histogram == Counter({2: 12, 4: 6})
    assert len(nontrivial) == 2 * len(doubleton_sectors)
    checks += 2

    for pair in pairs:
        outgoing = collision[pair]
        assert sector_key(outgoing) == sector_key(pair)
        for matrix in group:
            transformed = transform_pair(matrix, pair)
            assert collision[transformed] == transform_pair(matrix, outgoing)
            checks += 1

    obstructed_doubletons = 0
    for sector in doubleton_sectors:
        left, right = sector
        obstructed = payload_routing_obstructed(left, right, group)
        if obstructed:
            obstructed_doubletons += 1
        assert obstructed == payload_routing_obstructed(right, left, group)
        assert obstructed == (norm_squared(sector_key(left)[1]) == 4)
        checks += 3

    phase_states = tuple(
        canonical_phase_pair((pair[0], left_phase), (pair[1], right_phase))
        for pair in pairs
        for left_phase in range(4)
        for right_phase in range(4)
    )
    assert len(phase_states) == 153 * 16
    phase_collision = {
        state: phase_complete_collision(state, sectors) for state in phase_states
    }
    assert set(phase_collision) == set(phase_states)
    assert set(phase_collision.values()) == set(phase_states)
    assert all(phase_collision[phase_collision[state]] == state for state in phase_states)
    checks += 4

    phase_nontrivial = tuple(
        state for state in phase_states if phase_collision[state] != state
    )
    assert len(phase_nontrivial) == 432
    checks += 1

    for state in phase_states:
        outgoing = phase_collision[state]
        incoming_pair = canonical_pair(state[0][0], state[1][0])
        outgoing_pair = canonical_pair(outgoing[0][0], outgoing[1][0])
        assert sector_key(incoming_pair) == sector_key(outgoing_pair)
        assert sorted(record[1] for record in state) == sorted(
            record[1] for record in outgoing
        )
        for shift in range(4):
            assert phase_collision[shift_phase_pair(state, shift)] == shift_phase_pair(
                outgoing, shift
            )
            checks += 1
        for matrix in group:
            assert phase_collision[transform_phase_pair(matrix, state)] == transform_phase_pair(
                matrix, outgoing
            )
            checks += 1

    print(f"PASS: C18 two-record momentum-sector census ({checks} exact checks)")
    print(f"pairs={len(pairs)}, sectors={len(sectors)}, doubleton_sectors={len(doubleton_sectors)}")
    print(f"sector_histogram={sorted(histogram.items())}")
    print(f"state_histogram={sorted(state_histogram.items())}")
    print(f"doubleton_momentum_histogram={sorted(doubleton_momentum_histogram.items())}")
    print(f"nontrivial_scattering_states={len(nontrivial)}")
    print(f"unequal_scalar_phase_routing_obstructions={obstructed_doubletons}")
    print(f"phase_complete_nontrivial_states={len(phase_nontrivial)}")


if __name__ == "__main__":
    main()
