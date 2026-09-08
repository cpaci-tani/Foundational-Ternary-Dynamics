"""Exact block conservation and an explicit obstruction to count-only closure."""
import numpy as np
import pytest

from phi_v2_lattice import channels as C, coarse as B, conservation as K
from phi_v2_lattice import geometry as G, prepare as P, state as S, tick as T
from phi_v2_lattice._proofs import encode


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


@pytest.mark.parametrize("width", [1, 2, 4])
def test_live_block_continuity_and_nested_restriction(tables, width):
    st = P.sparse_material(4, seed=5, n_tokens=12, field_occupation=.02)
    initial_work = K.work_units(st)
    crossings = 0
    for _ in range(8):
        old = S.copy(st)
        new, ev = T.tick(st, tables)
        crossings += len(ev.crossings)
        block = B.restrict(st, width)
        assert B.merge(B.restrict(st, 1), width) == block
        assert sum(block.field_tokens) + sum(block.relation_tokens) == initial_work
        assert sum(sum(row) for row in block.manifestation_counts) == 4 ** 3
        assert all(r == 0 for r in B.continuity_residual(st, new, ev, width))
        for attr in ("s", "ell", "bank", "sc", "fcc"):
            np.testing.assert_array_equal(getattr(st, attr), getattr(old, attr))
        st = new
    assert crossings > 0


@pytest.mark.parametrize("kind,owner,indices", [
    ("sc", (1, 0, 0), (0,)),       # interior block face
    ("sc", (3, 0, 0), (0,)),       # periodic seam
    ("fcc", (1, 1, 0), (2, 0)),   # positive diagonal across two block axes
    ("fcc", (1, 1, 0), (2, 1)),   # negative diagonal with shifted tail
])
def test_boundary_current_is_nonvacuous_and_detects_missing_events(tables, kind, owner, indices):
    st = S.blank(4)
    site = G.site_index(4, *owner)
    getattr(st, kind)[(site,) + indices + (1,)] = S.idx_of(encode(0, +1))
    new, ev = T.tick(st, tables)
    assert B.boundary_current(st, ev, 2)
    assert set(B.continuity_residual(st, new, ev, 2)) == {0}
    assert any(B.continuity_residual(st, new, T.TickEvents(), 2))
    assert B.boundary_current(st, ev, 4) == {}


def test_counts_do_not_determine_next_coarse_state(tables):
    """Same complete block readout, different hidden phase, different next Q."""
    owner = G.site_index(4, 1, 0, 0)
    a = P.isolated_relation(4, owner=owner, phase=0)
    b = P.isolated_relation(4, owner=owner, phase=1)
    assert B.restrict(a, 2) == B.restrict(b, 2)
    next_a, _ = T.tick(a, tables)
    next_b, _ = T.tick(b, tables)
    assert B.restrict(next_a, 2).incidence != B.restrict(next_b, 2).incidence


@pytest.mark.parametrize("width", [0, -1, 3, 1.5, True])
def test_invalid_partition_is_rejected(width):
    with pytest.raises(ValueError):
        B.restrict(S.blank(4), width)
