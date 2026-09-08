"""Optional real-device parity: CUDA execution uses WSL2 on Windows.

Build isolated engine/strict with FTD_STRICT_CUDA=ON first. Set
FTD_STRICT_CUDA_REQUIRED=1 to turn unavailable binaries/devices into failures.
This suite measures correctness; it makes no rendering/FPS certificate.
"""
from dataclasses import asdict
import json
import os
from pathlib import Path
import subprocess
import numpy as np
import pytest
from phi_v2_lattice import channels as C, geometry as G, native_codec as N
from phi_v2_lattice import prepare as Z, state as S, staged as P
from phi_v2_lattice._proofs import encode

ROOT = Path(__file__).resolve().parents[3]


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


@pytest.fixture(scope="module")
def cuda():
    executable = Path(os.environ.get("FTD_STRICT_CUDA_CLI", ROOT / "engine/build_strict_cuda/ftd_strict_cuda_cli"))
    required = os.environ.get("FTD_STRICT_CUDA_REQUIRED") == "1"
    if not executable.is_file():
        if required: pytest.fail(f"required CUDA CLI absent: {executable}")
        pytest.skip("optional strict CUDA binary not built")
    prefix = (["wsl", "-d", "Ubuntu-22.04", "--", _linux(executable)] if os.name == "nt"
              else [str(executable)])
    result = subprocess.run(prefix + ["--device"], capture_output=True, text=True, timeout=30)
    if result.returncode:
        if required: pytest.fail(result.stderr)
        pytest.skip("CUDA device unavailable: " + result.stderr)
    info = json.loads(result.stdout)
    assert info["backend"] == "cuda_device_kernels"
    assert info["compute_major"] >= 5
    return prefix, info


def _run(cuda, folder, blob, ticks, *, sentinel=False):
    prefix, _ = cuda
    initial, final, events = (folder / name for name in ("input.bin", "output.bin", "events.json"))
    initial.write_bytes(blob)
    if sentinel:
        final.write_bytes(b"unchanged-state")
        events.write_bytes(b"unchanged-events")
    paths = [_linux(p) if os.name == "nt" else str(p) for p in (initial, final, events)]
    result = subprocess.run(prefix + [paths[0], paths[1], str(ticks), paths[2]],
                            capture_output=True, text=True, timeout=60)
    assert initial.read_bytes() == blob
    return result, final, events


def _preparation(kind):
    if kind == "r5":
        return Z.r5_vacuum(3, seed=947)
    L = 9 if kind == "multi_block" else (7 if kind == "causal_witness" else (5 if kind == "sparse" else 3))
    st = S.blank(L)
    if kind in ("dense", "sparse", "multi_block"):
        rng = np.random.default_rng(941)
        st.s[:] = rng.integers(-1, 2, st.s.shape)
        st.ell[:] = rng.integers(0, 3, st.ell.shape)
        st.bank[:] = rng.random(st.bank.shape) < (.31 if kind == "dense" else .006)
        st.sc[:] = rng.integers(0, 9, st.sc.shape)
        st.fcc[:] = rng.integers(0, 9, st.fcc.shape)
    elif kind == "causal_witness":
        st.bank[G.site_index(L, 3, 3, 3), [0, 34]] = True
        st.bank[G.site_index(L, 4, 3, 3), 2] = True
    elif kind == "expiry":
        st.bank[0, 34] = True
    elif kind == "fcc_seam":
        owner = G.site_index(L, L-1, L-1, L-1)
        st.fcc[owner, :, :, 1] = S.idx_of(encode(0, +1))
        st.sc[owner, :, 0] = S.idx_of(encode(0, -1))
    return st


@pytest.mark.parametrize("kind", ["blank", "sparse", "dense", "multi_block", "causal_witness", "expiry", "fcc_seam", "r5"])
@pytest.mark.parametrize("phase", range(4))
def test_real_device_every_phase_complete_state_events_and_work(cuda, tmp_path, kind, phase):
    state = P.initialize(_preparation(kind))
    tables = C.load_collision_tables()
    for _ in range(phase): state, _ = P.step(state, tables)
    before = N.encode(state)
    expected = state
    logs = []
    work = P.work_units(state)
    for _ in range(9):
        expected, ev = P.step(expected, tables)
        assert P.work_units(expected) == work
        logs.append(asdict(ev))
    result, final, events = _run(cuda, tmp_path, before, 9)
    assert result.returncode == 0, result.stderr
    assert final.read_bytes() == N.encode(expected)
    assert P.work_units(N.decode(final.read_bytes())) == work
    assert json.loads(events.read_text()) == json.loads(json.dumps(logs))
    assert json.loads(result.stderr)["backend"] == "cuda_device_kernels"
    assert N.encode(state) == before


@pytest.mark.parametrize("corruption", ["law", "bool", "truncated", "overflow", "negative"])
def test_failed_batch_preserves_input_and_existing_outputs(cuda, tmp_path, corruption):
    state = P.initialize(S.blank(3))
    ticks = 1
    if corruption == "overflow": state.microtick = 2 ** 64 - 1
    blob = bytearray(N.encode(state))
    if corruption == "law": blob[84] ^= 1
    elif corruption == "bool": blob[N.HEADER.size + 2 * 27] = 2
    elif corruption == "truncated": blob = blob[:-1]
    elif corruption == "negative": ticks = -1
    result, final, events = _run(cuda, tmp_path, bytes(blob), ticks, sentinel=True)
    assert result.returncode != 0
    assert final.read_bytes() == b"unchanged-state"
    assert events.read_bytes() == b"unchanged-events"


def test_zero_ticks_is_exact_noop_with_empty_external_log(cuda, tmp_path):
    state = P.initialize(_preparation("sparse"))
    before = N.encode(state)
    result, final, events = _run(cuda, tmp_path, before, 0)
    assert result.returncode == 0, result.stderr
    assert final.read_bytes() == before
    assert json.loads(events.read_text()) == []
