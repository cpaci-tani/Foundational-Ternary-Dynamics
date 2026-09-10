"""Small malformed-record and transport controls; no table census or campaign."""
import base64
from dataclasses import replace
import json

import numpy as np
import pytest

from phi_v2_lattice import exact_json as J
from phi_v2_lattice.hydro import channels as H, codec as C, staged as P, state as S


def blank():
    return P.initialize(S.blank(3))


def payload():
    return json.loads(C.checkpoint(blank()))


@pytest.mark.parametrize("tick", [0, 1, 2**53+1, 2**64-1, 10**5000],
                         ids=["zero", "one", "above-js", "uint64-max", "large-python"])
def test_exact_v2_and_legacy_v1_python_clocks(tick):
    state = replace(blank(), microtick=tick)
    wire = C.checkpoint(state)
    assert json.loads(wire)["microtick"] == J.integer_text(tick)
    assert C.restore(wire).microtick == tick
    legacy = json.loads(wire)
    legacy.update(schema=C.LEGACY_SCHEMA, microtick=tick)
    assert C.restore(J.dumps(legacy).encode()).microtick == tick


@pytest.mark.parametrize("field,value", [
    ("encoding", "0"*64), ("boundary", "unknown"), ("law", "foreign"),
    ("table", "0"*64), ("schema", "unknown"), ("L", 3.5), ("L", True),
    ("L", "3"), ("L", 0), ("microtick", "01"), ("microtick", "-1"),
    ("microtick", 1), ("microtick", 1.5), ("microtick", True),
])
def test_restore_rejects_incompatible_or_coerced_records(field, value):
    obj = payload()
    obj[field] = value
    with pytest.raises(ValueError):
        C.restore(json.dumps(obj).encode())


@pytest.mark.parametrize("name", ["bank", "admitted_sc", "admitted_fcc", "gate_sc", "gate_fcc"])
def test_json_never_normalizes_noncanonical_boolean_bytes(name):
    obj = payload()
    raw = bytearray(base64.b64decode(obj["arrays"][name]))
    raw[0] = 2
    obj["arrays"][name] = base64.b64encode(raw).decode()
    with pytest.raises(ValueError, match="boolean"):
        C.restore(json.dumps(obj).encode())


def test_duplicate_fields_extra_arrays_and_noncanonical_base64_are_rejected():
    original = C.checkpoint(blank())
    with pytest.raises(ValueError, match="duplicate"):
        C.restore(b'{"L":3,' + original[1:])
    obj = payload()
    obj["arrays"]["extra"] = ""
    with pytest.raises(ValueError):
        C.restore(json.dumps(obj).encode())
    for text in ("====", payload()["arrays"]["s"] + "\n", "!" + payload()["arrays"]["s"][1:]):
        obj = payload()
        obj["arrays"]["s"] = text
        with pytest.raises(ValueError):
            C.restore(json.dumps(obj).encode())


def test_concrete_state_boundary_rejects_callbacks_before_invocation():
    calls = []

    class BadArray(np.ndarray):
        def copy(self, *args, **kwargs):
            calls.append("copy")
            return super().copy(*args, **kwargs)

    class BadInt(int):
        def __int__(self):
            calls.append("int")
            return 3

    class BadState(P.StagedState):
        pass

    state = blank()
    candidates = [replace(state, microtick=BadInt(0)),
                  replace(state, lattice=replace(state.lattice, L=BadInt(3))),
                  replace(state, lattice=replace(state.lattice, bank=state.lattice.bank.view(BadArray))),
                  BadState(**state.__dict__)]
    for candidate in candidates:
        with pytest.raises(ValueError):
            P.validate(candidate)
    assert not calls


def test_pinned_table_has_immutable_storage_and_steps_do_not_reread_it(monkeypatch):
    table = H.load_table()
    assert not table.flags.writeable
    table.dtype = np.uint8  # caller-owned metadata must not poison later loads
    fresh = H.load_table()
    assert fresh.dtype == np.dtype("<u4") and fresh.shape == (1 << H.N_VEL,)
    assert H.checked_table(fresh).base is fresh.base

    class BadTable(np.ndarray):
        def __getitem__(self, key):
            raise AssertionError("custom law executed")

    with pytest.raises(ValueError):
        H.checked_table(fresh.view(BadTable))
    with pytest.raises(ValueError):
        H.checked_table(np.zeros(4, dtype=np.uint32))

    def no_disk_read(path):
        raise AssertionError("normal microtick reread the collision table")

    monkeypatch.setattr(H.Path, "read_bytes", no_disk_read)
    state = blank()
    for _ in range(4):
        state, _ = P.step(state)
    assert state.microtick == 4 and P.work_units(state) == 0
