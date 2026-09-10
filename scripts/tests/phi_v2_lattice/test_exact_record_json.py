"""Exact publication boundaries, with no evolution or physical campaign."""
import json
import sys

import pytest

from phi_v2_lattice import exact_json as J, checkpoint as K, causal_window as W
from phi_v2_lattice import staged as P, state as S, runtime as R


def test_existing_compact_json_bytes_are_unchanged():
    value = {"b": [None, True, False, -23, 2**80, "line\n\u03a6"], "a": {"z": 0}}
    assert J.dumps(value) == json.dumps(value, sort_keys=True, separators=(",", ":"))
    state = P.initialize(S.blank(3))
    blob = K.checkpoint(state)
    assert blob == json.dumps(json.loads(blob), sort_keys=True, separators=(",", ":")).encode()


@pytest.mark.parametrize("text", ["", "00", "-0", "+1", "1.0", "1e3", " 1", "\u0661", "--1"])
def test_decimal_strings_are_canonical(text):
    with pytest.raises(ValueError):
        J.integer_from_text(text)


def test_large_ordinals_and_origins_round_trip_without_global_changes():
    limit = sys.get_int_max_str_digits()
    huge = 10**5000
    text = "1" + "0" * 5000
    assert J.integer_text(huge) == text
    assert J.integer_from_text("-" + text) == -huge
    state = P.initialize(S.blank(3))
    state.microtick = huge
    blob = K.checkpoint(state)
    assert K.restore(blob).microtick == huge
    window = W.CausalWindow(state, origin=(-huge, huge, 0))
    restored = W.CausalWindow.restore(window.checkpoint())
    assert restored.checkpoint() == window.checkpoint()
    assert restored.observe().tick_interval == (huge, huge)
    owner = R.StrictRuntime(S.blank(3))
    owner.restore(blob)
    assert owner.observe().to_wire()["tick_end"] == text
    assert sys.get_int_max_str_digits() == limit


def test_exact_json_rejects_approximate_or_behavior_bearing_values():
    class CustomInt(int):
        pass

    for value in (1.5, float("nan"), CustomInt(1), {1: "ambiguous key"}):
        with pytest.raises(TypeError):
            J.dumps(value)


@pytest.mark.parametrize("text", ["1.5", "1e1000", "NaN", "Infinity", "-Infinity"])
def test_exact_json_never_admits_floating_point_tokens(text):
    with pytest.raises(ValueError):
        J.loads(text)


def test_container_metaclass_cannot_impersonate_a_concrete_list():
    calls = []

    class Impersonator(type):
        def __eq__(cls, other):
            calls.append("eq")
            return True

    class Foreign(metaclass=Impersonator):
        def __iter__(self):
            calls.append("iter")
            yield 7

    with pytest.raises(TypeError):
        J.dumps(Foreign())
    assert not calls
