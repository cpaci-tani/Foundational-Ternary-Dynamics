"""Complete-state/event parity against an actually built C++ candidate CLI.

Set FTD_STRICT_NATIVE_CLI to override the standalone build location. Missing
binaries skip these tests explicitly; source-only tests are not backend proof.
"""
from dataclasses import asdict
import json
import os
from pathlib import Path
import struct
import subprocess

import numpy as np
import pytest

from phi_v2_lattice import channels as C, geometry as G, native_codec as N
from phi_v2_lattice import prepare as P, staged as S, state as A
from phi_v2_lattice._proofs import encode

ROOT = Path(__file__).resolve().parents[3]


@pytest.fixture(scope="module")
def cli():
    suffix = ".exe" if os.name == "nt" else ""
    path = Path(os.environ.get("FTD_STRICT_NATIVE_CLI", ROOT / "engine/build_strict_native" / ("ftd_strict_cli" + suffix)))
    if not path.is_file():
        pytest.skip("strict native CLI not built; configure engine/strict separately")
    return str(path)


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def preparation(L, kind):
    if kind == "isolated":
        return P.isolated_relation(L, phase=0, polarity=-1)
    if kind == "sparse":
        return P.sparse_material(L, seed=517, n_tokens=12, field_occupation=.006)
    if kind == "r5":
        return P.r5_vacuum(L, seed=519)
    st = A.blank(L)
    if kind == "witness":
        x = G.site_index(L, 1, 1, 1)
        st.bank[x, [0,34]] = True
        st.bank[G.shift(L, x, (1,0,0)), 2] = True
        return st
    # Seam, both FCC diagonal orientations, negative polarity and lagged s.
    st.s[:] = np.arange(L**3) % 3 - 1
    st.ell[:] = np.arange(L**3) % 3
    owner = G.site_index(L,L-1,L-1,L-1)
    st.sc[owner,0,1] = A.idx_of(encode(0,-1))
    st.fcc[owner,2,0,0] = A.idx_of(encode(0,+1))
    st.fcc[owner,2,1,1] = A.idx_of(encode(0,-1))
    st.bank[owner, [0,34,194]] = True
    return st


def invoke(cli, tmp_path, blob, ticks):
    source, target, events = (tmp_path / name for name in ("input.bin","output.bin","events.json"))
    source.write_bytes(blob)
    run = subprocess.run([cli,str(source),str(target),str(ticks),str(events)], capture_output=True, text=True, timeout=30)
    return run, target, events


@pytest.mark.parametrize("L", [3,4,7])
@pytest.mark.parametrize("kind", ["isolated","sparse","r5","seam","witness"])
def test_complete_state_and_ordered_events_match_each_physical_stage(cli,tables,tmp_path,L,kind):
    state = S.initialize(preparation(L,kind))
    total = S.work_units(state)
    native = N.encode(state)
    for _ in range(8):  # two complete cycles, every intermediate phase
        state, expected = S.step(state,tables)
        run, target, event_file = invoke(cli,tmp_path,native,1)
        assert run.returncode == 0, run.stderr
        native = target.read_bytes()
        assert native == N.encode(state)
        assert json.loads(event_file.read_text()) == json.loads(json.dumps([asdict(expected)]))
        assert S.work_units(N.decode(native)) == total


def test_batch_matches_individual_steps_and_replay(cli,tables,tmp_path):
    state = S.initialize(preparation(4,"sparse"))
    initial = N.encode(state)
    expected = []
    for _ in range(12):
        state, events = S.step(state,tables)
        expected.append(asdict(events))
    run, output, events = invoke(cli,tmp_path,initial,12)
    assert run.returncode == 0, run.stderr
    assert output.read_bytes() == N.encode(state)
    assert json.loads(events.read_text()) == json.loads(json.dumps(expected))


@pytest.mark.parametrize("kind", ["magic","collision","encoding","law","length","trailing","dimension",
                                  "s","ell","bank","sc","fcc","admitted","gate_sc","gate_fcc","admitted_payload"])
def test_malformed_inputs_reject_without_touching_existing_outputs(cli,tmp_path,kind):
    data = bytearray(N.encode(S.initialize(A.blank(3))))
    offsets = {"magic":0,"collision":20,"encoding":52,"law":84,"s":116,"ell":143,
               "bank":170,"sc":116+386*27,"fcc":116+392*27,"admitted":116+404*27,
               "gate_sc":116+407*27,"gate_fcc":116+410*27}
    if kind in offsets:
        data[offsets[kind]] = 9 if kind in ("sc","fcc") else 3
    elif kind == "length": data.pop()
    elif kind == "trailing": data.append(0)
    elif kind == "dimension": struct.pack_into("<I",data,8,2)
    else:
        struct.pack_into("<Q",data,12,1)
        data[116+404*27] = 1  # admitted flag but reserve remains blank
    for name in ("output.bin","events.json"): (tmp_path/name).write_bytes(b"unchanged")
    run, output, events = invoke(cli,tmp_path,bytes(data),0)
    assert run.returncode != 0
    assert output.read_bytes() == events.read_bytes() == b"unchanged"
    with pytest.raises(ValueError): N.decode(bytes(data))


def test_unsigned_clock_overflow_and_invalid_cli_counts_fail_closed(cli,tmp_path):
    state = S.initialize(A.blank(3))
    state.microtick = 2**64-1
    blob = N.encode(state)
    run, output, _ = invoke(cli,tmp_path,blob,0)
    assert run.returncode == 0 and output.read_bytes() == blob
    for ticks in (1,2**64,-1,"1.5","+1"):
        run, output, _ = invoke(cli,tmp_path,blob,ticks)
        assert run.returncode != 0 and output.read_bytes() == blob
    state.microtick = 2**64
    with pytest.raises(ValueError): N.encode(state)


def test_codec_header_layout_and_exact_large_tick_roundtrip():
    state = S.initialize(A.blank(3))
    state.microtick = 2**63+1
    blob = N.encode(state)
    assert len(blob) == 116+416*27
    assert blob[:8] == b"FTDSC01\0"
    assert struct.unpack_from("<Q",blob,12)[0] == state.microtick
    assert N.encode(N.decode(blob)) == blob


def test_generated_header_matches_hash_validated_python_data():
    run = subprocess.run([os.sys.executable,str(ROOT / "engine/strict/generate_tables.py"),"--check"],
                         capture_output=True,text=True,timeout=120)
    assert run.returncode == 0, run.stdout+run.stderr
