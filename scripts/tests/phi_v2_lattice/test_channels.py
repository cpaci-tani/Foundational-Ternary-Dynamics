import itertools
import pytest
from phi_v2_lattice import channels as C

def test_channel_indexing_is_bijective():
    seen = set()
    for i in range(C.N_STATES):
        for eps in (+1, -1):
            c = C.channel(i, eps)
            assert 0 <= c < C.N_CHANNELS
            assert C.unpack(c) == (i, eps)
            seen.add(c)
    assert len(seen) == C.N_CHANNELS

def test_U_is_the_certified_internal_tick_with_period_twelve():
    """U = internal_tick lifted to channels: the Hodge flag cycles with period 3
    (d -> h n -> d x n -> d, the Z3 that matches the collision-layer decrement) and the
    C4 phase with period 4, so the full state has period lcm(3,4) = 12. Polarity is a
    separate copy label and is never touched."""
    for c in range(C.N_CHANNELS):
        w = c
        for _ in range(12):
            assert C.polarity(w) == C.polarity(c)
            w = C.U(w)
        assert w == c
    # the phase alone returns after four applications (the full state does not);
    # the flag alone returns after three
    for c in range(C.N_CHANNELS):
        w4 = c
        for _ in range(4):
            w4 = C.U(w4)
        assert C.phase(w4) == C.phase(c)
        assert w4 != c
        w3 = C.U(C.U(C.U(c)))
        assert C.tangent(w3) == C.tangent(c)
        assert C.phase(w3) == (C.phase(c) + 3) % 4

def test_half_turn_is_a_phase_only_shift():
    """Spec 3.2: the manifested-departure half-turn is k -> k+2 with the FLAG fixed
    (a channel permutation that cannot create a streaming write collision). It is NOT U∘U,
    which also rotates the flag."""
    for c in range(C.N_CHANNELS):
        h = C.half_turn(c)
        assert C.tangent(h) == C.tangent(c) and C.polarity(h) == C.polarity(c)
        assert C.phase(h) == (C.phase(c) + 2) % 4
        assert C.half_turn(h) == c
        assert h != C.U(C.U(c)) or C.tangent(C.U(C.U(c))) == C.tangent(c)

def test_tangent_is_an_sc_unit_vector():
    for c in range(C.N_CHANNELS):
        d = C.tangent(c)
        assert sorted(abs(x) for x in d) == [0, 0, 1]

def test_collision_tables_are_fixed_point_free_involutions_preserving_records_and_fields():
    tables = C.load_collision_tables()
    assert len(tables) == 3
    for layer, table in enumerate(tables):
        assert len(table) == 18_336
        for before, after in table.items():
            assert before != after                      # fixed-point free
            assert table[after] == before               # involution
            assert before[0] < before[1] and after[0] < after[1]
            fb = tuple(a + b for a, b in zip(C.layer_value_of(C.channel(before[0], +1), layer),
                                             C.layer_value_of(C.channel(before[1], +1), layer)))
            fa = tuple(a + b for a, b in zip(C.layer_value_of(C.channel(after[0], +1), layer),
                                             C.layer_value_of(C.channel(after[1], +1), layer)))
            assert fb == fa                             # six layer-appropriate (E,B) sums

def test_collision_hash_is_the_frozen_one():
    assert C.COLLISION_HASH == "D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25"
    C.load_collision_tables()   # raises if the rebuilt table does not hash to COLLISION_HASH
