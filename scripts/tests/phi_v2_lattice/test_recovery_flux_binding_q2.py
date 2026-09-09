"""Checks of the exact representation against the independent full runtime."""
from dataclasses import replace
from itertools import product

import numpy as np
import pytest

from scripts.phi_v2_lattice import flux_binding as full
from scripts.phi_v2_lattice import recovery_flux_binding_q2 as q2


def test_complete_record_count_and_codec():
    records = q2.records()
    assert len(records) == len(set(records)) == 1548
    assert [sum(len(r.path) == k for r in records) for k in range(3)] == [36, 432, 1080]
    for i in range(q2.STATE_COUNT):
        assert q2.encode(q2.decode(i)) == i


@pytest.mark.parametrize("L", [6, 8, 10])
def test_all_path_credit_geometries_and_phase_color_seams_against_runtime(L):
    # Complete path/credit geometries and separately all phase/color schedules;
    # the registered campaign covers their whole Cartesian product with velocities.
    records = q2.records()
    probes = [q2.QuotientState(i % 12, q2.COLORS[(i // 12) % 8], r)
              for i, r in enumerate(records)]
    probes += [q2.QuotientState(p, color, records[(p * 8 + c) * 13 % len(records)])
               for p in range(12) for c, color in enumerate(q2.COLORS)]
    for state in probes:
        runtime = q2.materialize(state, L)
        full.validate(runtime)
        out, delta = q2.transition(state)
        actual, _ = full.step(runtime)
        expected = q2.materialize(out, L, tuple((state.color[a]+delta[a]) % L for a in range(3)),
                                  microtick=state.phase+1)
        assert actual.microtick == expected.microtick
        for name in full.NAMES:
            np.testing.assert_array_equal(getattr(actual, name), getattr(expected, name))
        assert full.account_units(actual) == 2
        assert full.populations(actual) == (1, 1)


def test_even_translation_and_large_ordinal_lift():
    for phase, color in product(range(12), q2.COLORS):
        q = q2.QuotientState(phase, color, q2.records()[907])
        base = tuple(c + 4 for c in color)
        tick = 12 * 10**5000 + phase
        state = q2.materialize(q, 10, base, tick)
        actual, _ = full.step(state)
        out, delta = q2.transition(q)
        expected = q2.materialize(out, 10, tuple((base[a]+delta[a]) % 10 for a in range(3)), tick+1)
        assert actual.microtick == expected.microtick
        for name in full.NAMES:
            np.testing.assert_array_equal(getattr(actual, name), getattr(expected, name))


@pytest.mark.parametrize("change", [
    {"phase": True}, {"phase": 12}, {"color": (0, 0, 2)}, {"color": [0, 0, 0]},
    {"record": q2.Record((0, 1), (0, 0), (0, 0))},
    {"record": q2.Record((), (0, 0), (0, 0))},
    {"record": q2.Record((0,), (0, 7), (1, 0))},
    {"record": q2.Record((0,), (False, 0), (1, 0))},
])
def test_invalid_quotient_rejected(change):
    with pytest.raises(ValueError):
        q2.transition(replace(q2.decode(0), **change))


@pytest.mark.parametrize("kwargs", [{"L": 4}, {"L": 7}, {"microtick": 1},
                                      {"positive_position": (1, 0, 0)}])
def test_invalid_lift_rejected(kwargs):
    with pytest.raises(ValueError):
        q2.materialize(q2.decode(0), **kwargs)


def test_frozen_runtime_gate_and_published_owned_arrays():
    report = q2.verify_runtime_range(18000, 18050)
    assert report["checked"] == 50
    assert len(report["complete_outputs_sha256"]) == 64
