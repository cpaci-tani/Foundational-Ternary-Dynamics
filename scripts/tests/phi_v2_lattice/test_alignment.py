"""Exhaust the selected finite table, without a continuum or binding claim."""
import numpy as np
import pytest

from phi_v2_lattice import alignment as A, channels as C


def test_complete_finite_certificate_and_unchanged_original_identity():
    before = C._hash_tables(C.load_collision_tables())
    result = A.finite_certificate()
    assert result["rows"] == 55008
    assert result["changed_rows"] == 576
    assert result["changed_rows_per_layer"] == [192] * 3
    assert result["preimage_counts_per_layer"] == [{0: 192, 1: 17952, 2: 192}] * 3
    assert result["polarity_row_checks"] == 110016
    assert result["cubic_transformations"] == 48
    assert result["phase_shifts"] == 4
    assert not result["phase_reversal_covariant"]
    assert C._hash_tables(C.load_collision_tables()) == before == C.COLLISION_HASH
    assert A.COLLISION_HASH != C.COLLISION_HASH


def test_locked_contact_pairs_every_layer_and_immutable_table():
    tables = A.load_collision_tables()
    for q in range(3):
        incoming, aligned, outgoing = (0, 5), (0, 1), (4, 5)
        for _ in range((-q) % 3):
            incoming, aligned, outgoing = [tuple(sorted(C.U(c) for c in p))
                                           for p in (incoming, aligned, outgoing)]
        assert tables[q][incoming] == tables[q][aligned] == outgoing
        with pytest.raises(TypeError):
            tables[q][incoming] = incoming


@pytest.mark.parametrize("occupied", [[], [0], [0, 5, 10], [0, 5, 10, 19], [0, 197]])
@pytest.mark.parametrize("layer", range(3))
def test_ineligible_banks_unchanged(occupied, layer):
    row = np.zeros(384, dtype=bool)
    row[occupied] = True
    saved = row.copy()
    out, events = A.collide_row(row, layer)
    np.testing.assert_array_equal(out, saved)
    np.testing.assert_array_equal(row, saved)
    assert events == []
    assert not np.shares_memory(out, row)


def test_both_eligible_polarity_banks_are_disjoint():
    row = np.zeros(384, dtype=bool)
    row[[0, 5, 192, 197]] = True
    out, events = A.collide_row(row, 0)
    assert np.flatnonzero(out).tolist() == [4, 5, 196, 197]
    assert events == [(1, (0, 5), (4, 5)), (-1, (0, 5), (4, 5))]
    assert np.flatnonzero(row).tolist() == [0, 5, 192, 197]


@pytest.mark.parametrize("pair", [(0, 0), (-1, 1), (0, 192), (True, 1), (0., 1), (), None])
def test_invalid_pair_rejected(pair):
    with pytest.raises(ValueError):
        A.alignment_pair(pair)


@pytest.mark.parametrize("layer", [-1, 3, True, 1.0, None])
def test_invalid_layer_rejected_without_mutation(layer):
    row = np.zeros(384, dtype=bool)
    with pytest.raises(ValueError):
        A.collide_row(row, layer)
    assert not row.any()


def test_noncanonical_boolean_bank_rejected():
    row = np.zeros(384, dtype=bool)
    row.view(np.uint8)[0] = 2
    saved = row.tobytes()
    with pytest.raises(ValueError):
        A.collide_row(row, 0)
    assert row.tobytes() == saved
