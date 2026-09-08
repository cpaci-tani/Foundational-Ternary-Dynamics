"""Exhaustive checks of the generated collision table; no sampling."""
import hashlib
import itertools

import numpy as np
import pytest

from phi_v2_lattice.hydro import channels as H, tables as T


@pytest.fixture(scope="module")
def table():
    return H.load_table()


def test_table_hash_is_pinned_and_matches_file(table):
    assert H.TABLE_HASH != "UNSET"
    assert hashlib.sha256(table.tobytes()).hexdigest() == H.TABLE_HASH


def test_involution_conservation_and_equivariance(table):
    report = T.verify_table(table)
    assert report["involution"] and report["mass_conserved"] and report["momentum_conserved"]
    assert report["equivariant"]
    assert 0 < report["fixed_states"] < (1 << 24)


def test_singletons_and_every_two_particle_state_are_fixed(table):
    for v in range(24):
        assert int(table[1 << v]) == (1 << v)
    for a, b in itertools.combinations(range(24), 2):   # 276 states; consequence of flip-parity, see plan
        s = (1 << a) | (1 << b)
        assert int(table[s]) == s


def test_three_body_collisions_exist_and_preserve_class(table):
    first = next(s for s in range(1, 1 << 24) if int(table[s]) != s)
    assert bin(first).count("1") >= 3
    image = int(table[first])
    assert bin(image).count("1") == bin(first).count("1")
    assert T.momentum_of(image) == T.momentum_of(first)


def test_perm_application_matches_channels_perm():
    states = np.array([1 << c for c in range(24)], dtype=np.uint32)
    for g in range(96):
        assert T.apply_perm(g, states).tolist() == [1 << int(H.PERM[g, c]) for c in range(24)]


def test_no_admissible_pair_left_unpaired(table):
    assert T.unpaired_admissible(table) == 0
