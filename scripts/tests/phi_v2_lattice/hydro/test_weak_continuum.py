"""Adversarial admission and exact spacetime checks, not a fluid campaign."""
from fractions import Fraction as F
import copy

import pytest

from scripts.phi_v2_lattice.experiments import weak_continuum as E


def cycle(phase=0, frozen=True):
    lattice = E.prepare.frozen_background(4) if frozen else E.state.blank(4)
    lattice.bank[63, E.H.channel(0, phase, 12)] = True
    initial = E.staged.initialize(lattice)
    before = initial
    for _ in range(2):
        before, _ = E.staged.step(before)
    final = before
    for _ in range(2):
        final, _ = E.staged.step(final)
    return tuple(E.codec.checkpoint(st) for st in (initial, before, final))


def test_spacetime_quartic_seam_has_exact_identity_and_complete_bound():
    result = E.spacetime_cycle(*cycle())
    assert result["source_left"] == [0]*4
    assert result["endpoint"] == result["exact_transport"]
    assert result["first_residual"][0] != 0
    for order in ("first", "second"):
        assert all(abs(r) <= b for r, b in zip(result[order+"_residual"], result[order+"_bound"]))


def test_actual_absorption_time_requires_a_signed_correction():
    result = E.spacetime_cycle(*cycle(phase=2, frozen=False))
    assert result["absorbed_tokens"] == 1
    assert result["source_left"][0] < 0
    assert result["source_event_correction"][0] > 0
    assert result["source_event"][0] != result["source_left"][0]
    assert result["source_event"][0]+result["source_event_correction"][0] == result["endpoint"][0]


def test_uniform_caps_and_exact_horizon_sums():
    assert E.caps() == (144, 64, 80, 96)
    for L in (4, 8):
        h = F(1, L)
        first = sum(h*h*(1+n*h+F(1, 2)) for n in range(L))
        second = sum(h**3*(2*(1+n*h)+1) for n in range(L))
        assert first == h*(2-h/2) <= 2*h
        assert second == h*h*(4-h) <= 4*h*h


def test_native_event_conversion_preserves_signed_polarity_and_absorption_owner():
    events = dict(absorptions=[(7, 98, "sc", 3, (2,)), (5, 31, "fcc", 2, (1, 1))],
                  collisions=[(4, 0, 5, 80), (2, 1, 80, 5)],
                  crossings=[("sc", 3, (0,), -1)], gate_holds=[("fcc", 5, (2, 1))])
    converted = E.native_events(events)
    assert converted["absorptions"] == [[7, 98, 0, 3, 2, 0], [5, 31, 1, 2, 1, 1]]
    assert converted["collisions"] == [[4, 1, 5, 80], [2, -1, 80, 5]]
    for foreign in (True, 1.0):
        corrupt = copy.deepcopy(converted)
        corrupt["collisions"][0][1] = foreign
        assert E.canonical(corrupt) != E.canonical(converted)


def test_empty_source_pins_and_duplicate_fields_are_rejected():
    with pytest.raises(ValueError, match="incomplete registration"):
        E.verify_pins(dict(schema=E.SCHEMA, sources={}))
    with pytest.raises(ValueError, match="duplicate JSON"):
        E._unique([("a", 1), ("a", 2)])


def test_foreign_law_is_rejected_before_any_source_read():
    lock = dict(schema=E.SCHEMA, sources={"made-up": "0"*64}, input_file="input.json",
                law={"id": "foreign"})
    with pytest.raises(ValueError, match="registered law"):
        E.verify_pins(lock)


def test_incomplete_compression_is_rejected_before_a_checkpoint_is_used():
    import base64
    import zlib
    data = zlib.compressobj(wbits=-15)
    compressed = data.compress(b"{}")+data.flush()
    for bad in (compressed[:-1], compressed+b"extra"):
        record = dict(checkpoint_codec="deflate-raw-base64",
                      checkpoint_deflate_raw_base64=base64.b64encode(bad).decode("ascii"))
        with pytest.raises(ValueError, match="compression"):
            E.decode_stage(record)
