"""Exact conservation, exclusion, determinism and causal support of the successor law."""
import numpy as np
import pytest

from phi_v2_lattice import geometry as G
from phi_v2_lattice.hydro import channels as H, prepare as R, staged as S, state as ST


@pytest.fixture(scope="module")
def table():
    return H.load_table()


def momentum_and_mass(state, pol):
    bank = state.lattice.bank[:, pol * 96:(pol + 1) * 96]
    counts = bank.reshape(-1, 4, 24).sum(axis=(0, 1))
    mass = int(counts.sum())
    P = tuple(int(sum(int(counts[v]) * H.VELOCITIES[v][a] for v in range(24))) for a in range(3))
    return mass, P


@pytest.mark.parametrize("L", [4, 7])
def test_frozen_background_fluid_conserves_mass_and_momentum_exactly(table, L):
    state = S.initialize(R.fluid(L, seed=3, density=1 / 4))
    before = momentum_and_mass(state, 0)
    for _ in range(16):
        state, ev = S.step(state, table)
        assert not ev.absorptions and not ev.crossings
    assert momentum_and_mass(state, 0) == before


def test_exclusion_is_preserved_and_streaming_never_collides(table):
    state = S.initialize(R.fluid(5, seed=9, density=3 / 8))
    for _ in range(12):
        state, _ = S.step(state, table)
        bank = state.lattice.bank.reshape(-1, 2, 4, 24)
        assert bank.sum(axis=2).max() <= 1


def test_collision_events_change_velocities_but_not_class(table):
    first = next(s for s in range(1, 1 << 24) if int(table[s]) != s)
    st = ST.blank(4)
    x = G.site_index(4, 1, 1, 1)
    for v in range(24):
        if first >> v & 1:
            st.bank[x, H.channel(0, v % 4, v)] = True
    state = S.initialize(R.with_background(st))
    state, _ = S.step(state, table)      # phase 0 -> 1
    state, ev = S.step(state, table)     # collision stage
    assert ev.collisions == [(x, 0, first, int(table[first]))]
    row = state.lattice.bank[x].reshape(2, 4, 24)
    assert sorted(np.nonzero(row[0].any(axis=0))[0].tolist()) == [v for v in range(24) if int(table[first]) >> v & 1]
    assert sorted(np.nonzero(row[0])[0].tolist()) == sorted(v % 4 for v in range(24) if first >> v & 1)


def test_causal_support_is_radius_one_per_stage(table):
    L = 7
    a = S.initialize(R.fluid(L, seed=5, density=1 / 8))
    b = S.initialize(R.fluid(L, seed=5, density=1 / 8))
    x = G.site_index(L, 3, 3, 3)
    b.lattice.bank[x, H.channel(0, 0, 5)] ^= True
    for _ in range(4):
        a, _ = S.step(a, table); b, _ = S.step(b, table)
    differing = np.nonzero((a.lattice.bank != b.lattice.bank).any(axis=1))[0]
    assert len(differing) > 0
    for i in differing.tolist():
        c = G.coords(L, i)
        assert all(min(abs(c[k] - 3), L - abs(c[k] - 3)) <= 1 for k in range(3))


def test_step_is_deterministic(table):
    s1 = S.initialize(R.fluid(4, seed=11, density=1 / 4))
    s2 = S.initialize(R.fluid(4, seed=11, density=1 / 4))
    for _ in range(8):
        s1, e1 = S.step(s1, table); s2, e2 = S.step(s2, table)
        assert e1 == e2 and (s1.lattice.bank == s2.lattice.bank).all()


def test_validate_rejects_exclusion_violation(table):
    st = R.frozen_background(3)
    st.bank[0, H.channel(0, 0, 0)] = True
    st.bank[0, H.channel(0, 1, 0)] = True
    with pytest.raises(ValueError):
        S.initialize(st)
