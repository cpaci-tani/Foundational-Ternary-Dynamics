"""Canonical velocity set and group action; exact integer checks only."""
import numpy as np

from phi_v2_lattice.hydro import channels as H


def test_velocity_set_has_twelve_faces_and_twelve_edges_in_canonical_order():
    assert len(H.VELOCITIES) == 24 and sum(H.IS_FACE) == 12
    faces = [v for v, f in zip(H.VELOCITIES, H.IS_FACE) if f]
    edges = [v for v, f in zip(H.VELOCITIES, H.IS_FACE) if not f]
    assert all(sum(abs(x) for x in v) == 1 for v in faces)
    assert all(sum(abs(x) for x in v) == 2 and 0 in v for v in edges)
    assert H.VELOCITIES[0] == (1, 0, 0) and H.FACE_W[0] == 0 and H.FACE_W[1] == 1
    assert H.VELOCITIES[2] == (-1, 0, 0) and H.VELOCITIES[12] == (1, 1, 0)
    assert len(set(zip(H.VELOCITIES, H.FACE_W))) == 24


def test_fourth_rank_isotropy_of_velocity_multiset():
    xxxx = sum(v[0] ** 4 for v in H.VELOCITIES)
    xxyy = sum(v[0] ** 2 * v[1] ** 2 for v in H.VELOCITIES)
    assert (xxxx, xxyy) == (12, 4) and xxxx == 3 * xxyy


def test_group_has_96_elements_and_acts_by_permutation():
    assert len(H.GROUP) == 96 and H.PERM.shape == (96, 24)
    assert all(sorted(row) == list(range(24)) for row in H.PERM.tolist())
    assert H.PERM[0].tolist() == list(range(24))
    for g, (matrix, wflip) in enumerate(H.GROUP):
        for c, v in enumerate(H.VELOCITIES):
            image = tuple(int(sum(matrix[i][j] * v[j] for j in range(3))) for i in range(3))
            c2 = int(H.PERM[g, c])
            assert H.VELOCITIES[c2] == image
            if H.IS_FACE[c]:
                assert H.FACE_W[c2] == (H.FACE_W[c] ^ wflip)


def test_multiplication_inverse_and_conjugation_tables():
    identity = np.arange(24)
    for g in range(96):
        assert (H.PERM[H.INVERSE[g]][H.PERM[g]] == identity).all()
        for h in range(0, 96, 7):
            assert (H.PERM[H.MUL[g, h]] == H.PERM[g][H.PERM[h]]).all()
            assert H.CONJ[g, h] == H.MUL[H.MUL[g, h], H.INVERSE[g]]


def test_channel_packing_round_trips():
    for pol in range(2):
        for k in range(4):
            for v in range(24):
                c = H.channel(pol, k, v)
                assert 0 <= c < 192 and H.unpack(c) == (pol, k, v)


def test_relation_targets_cover_sc_and_fcc():
    kinds = [H.relation_target(v)[0] for v in range(24)]
    assert kinds[:12] == ["sc"] * 12 and kinds[12:] == ["fcc"] * 12
    assert H.relation_target(0) == ("sc", (0, 0, 0), (0,))
    assert H.relation_target(2) == ("sc", (-1, 0, 0), (0,))
    idx = H.VELOCITIES.index
    assert H.relation_target(idx((1, 1, 0))) == ("fcc", (0, 0, 0), (2, 0))
    assert H.relation_target(idx((-1, 1, 0))) == ("fcc", (-1, 0, 0), (2, 1))
    assert H.relation_target(idx((1, -1, 0))) == ("fcc", (0, -1, 0), (2, 1))
    assert H.relation_target(idx((-1, -1, 0))) == ("fcc", (-1, -1, 0), (2, 0))
