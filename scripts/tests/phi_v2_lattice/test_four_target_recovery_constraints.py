"""Short, exact witnesses for the four-target theory analysis.

No physical calibration, parameter scan, census, or successor law is used.
These controls expose limitations of the existing credit-exchange candidate.
"""
from dataclasses import fields

import numpy as np
import pytest

from phi_v2_lattice import credit_exchange_binding as E


V = np.asarray(((1, 0, 0), (-1, 0, 0), (0, 1, 0),
                (0, -1, 0), (0, 0, 1), (0, 0, -1)), dtype=np.int64)


def prepare(eta=0, headings=None, plane=None):
    L = 4
    direction = np.zeros((L**3, 2), dtype=np.uint8)
    credit = np.zeros_like(direction)
    attempted = np.zeros_like(direction)
    flux = np.zeros((L**3, 3), dtype=np.int8)
    if headings is not None:
        direction[0] = np.asarray(headings) + 1
        credit[0] = 1
    if plane is not None:
        # Unit circulation: 0 -> e_a -> e_a+e_b -> e_b -> 0.
        a, b = plane
        strides = (L*L, L, 1)
        flux[0, a] = 1
        flux[strides[a], b] = 1
        flux[strides[b], a] = -1
        flux[0, b] = -1
    return E.initialize(L, direction, credit, flux, E.seed_background(L),
                        E.seed_charge_frame(L, eta), attempted)


@pytest.mark.parametrize('eta', (0, 1))
@pytest.mark.parametrize('chosen_heading', range(6))
def test_contact_erasure_merges_distinct_heading_momenta(eta, chosen_heading):
    outputs = []
    for erased_heading in range(6):
        headings = [erased_heading, erased_heading]
        headings[eta] = chosen_heading
        before = prepare(eta, headings)
        after, events = E.step(before)
        assert E.populations(before) == E.populations(after) == (1, 1)
        assert E.account_units(before) == E.account_units(after) == 2
        assert np.array_equal(before.flux, after.flux)
        assert len(events.onsite_alignments) == 1
        assert not events.moves and not events.credit_exchanges
        assert np.array_equal(after.direction[0], [chosen_heading+1]*2)
        if erased_heading == (chosen_heading ^ 1):
            # This refutes the heading-sum identification, not every possible P.
            assert np.array_equal(V[headings].sum(axis=0), [0, 0, 0])
            assert np.array_equal(V[after.direction[0]-1].sum(axis=0), 2*V[chosen_heading])
        outputs.append(after)
    # Same complete successor, including exact clock and all identity metadata.
    for after in outputs[1:]:
        for field in fields(outputs[0]):
            a, b = getattr(outputs[0], field.name), getattr(after, field.name)
            if isinstance(a, np.ndarray):
                assert np.array_equal(a, b)
            else:
                assert a == b


@pytest.mark.parametrize('plane', ((0, 1), (0, 2), (1, 2)))
def test_nonzero_carrier_free_circulation_is_stationary_through_all_phases(plane):
    state = prepare(plane=plane)
    original = {name: getattr(state, name).copy() for name in E.NAMES}
    assert E.account_units(state) == 4
    for tick in range(19):
        state, events = E.step(state)
        assert state.microtick == tick+1
        assert E.populations(state) == (0, 0)
        assert E.account_units(state) == 4
        assert all(not getattr(events, field.name) for field in fields(events))
        for name, values in original.items():
            assert np.array_equal(getattr(state, name), values)
