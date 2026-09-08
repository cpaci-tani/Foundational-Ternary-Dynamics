"""Complete-state/event parity between the Python phi-hydro-staged-candidate-1 reference and
an actually built CUDA (GPU) candidate CLI, run through WSL2 on Windows.

Set FTD_HYDRO_CUDA_CLI to the WSL-side path of the built ftd_hydro_cuda_cli binary (e.g.
/mnt/c/.../engine/build_strict_hydro_cuda/ftd_hydro_cuda_cli). The suite is skipped entirely
when that variable is unset, or when the device is unavailable. This measures correctness;
it makes no rendering/FPS certificate.

The fixture builders for the eight Task 8 native-parity states are reused directly from
test_native_parity (import, not duplication) so both suites exercise byte-identical states;
one additional fixture (`crossing_and_hold`) is defined here because it exists only to
exercise the CUDA admission/relation kernels' crossing and gate-hold paths over a longer
32-microtick run.
"""
import json
import os
from pathlib import Path
import subprocess

import pytest

from phi_v2_lattice import geometry as G
from phi_v2_lattice.hydro import channels as H, codec as N, prepare as R, staged as S, state as ST
from .test_native_parity import (
    _face_velocity,
    _edge_velocity,
    _single_particle,
    _three_body_collision,
    _seam,
    native_json,
)

ROOT = Path(__file__).resolve().parents[4]


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def _to_wsl(path):
    return _linux(path) if os.name == "nt" else str(path)


@pytest.fixture(scope="module")
def cuda():
    executable = os.environ.get("FTD_HYDRO_CUDA_CLI")
    if not executable:
        pytest.skip("FTD_HYDRO_CUDA_CLI not set; hydro CUDA binary not selected")
    prefix = ["wsl", "-d", "Ubuntu-22.04", "--", executable] if os.name == "nt" else [executable]
    result = subprocess.run(prefix + ["--device"], capture_output=True, text=True, timeout=30)
    if result.returncode:
        pytest.skip("CUDA device unavailable: " + result.stderr)
    info = json.loads(result.stdout)
    assert info["backend"] == "cuda_device_kernels"
    return prefix, info


@pytest.fixture(scope="module")
def table_path():
    path = H.table_path()
    if not path.is_file():
        pytest.skip("hydro collision table blob not built; run generate_tables.py")
    return path


@pytest.fixture(scope="module")
def table():
    return H.load_table()


@pytest.fixture(scope="module")
def native_cli():
    """Optional native CLI path for the three-way (Python/CUDA/native) crossing-fixture check.
    Returns None (never skips the module) when the native binary is not built."""
    suffix = ".exe" if os.name == "nt" else ""
    path = Path(os.environ.get("FTD_HYDRO_NATIVE_CLI", ROOT / "engine/build_strict_hydro_native" / ("ftd_hydro_cli" + suffix)))
    return str(path) if path.is_file() else None


def invoke_cuda(cuda, table_path, tmp_path, blob, microticks):
    prefix, _ = cuda
    source, target, events = (tmp_path / name for name in ("input.bin", "output.bin", "events.json"))
    source.write_bytes(blob)
    args = [_to_wsl(table_path), _to_wsl(source), _to_wsl(target), str(microticks), _to_wsl(events)]
    run = subprocess.run(prefix + args, capture_output=True, text=True, timeout=60)
    return run, target, events


def invoke_native(native_cli, table_path, tmp_path, blob, microticks):
    source, target, events = (tmp_path / name for name in ("native_input.bin", "native_output.bin", "native_events.json"))
    source.write_bytes(blob)
    run = subprocess.run([native_cli, str(table_path), str(source), str(target), str(microticks), str(events)],
                         capture_output=True, text=True, timeout=60)
    return run, target, events


def _crossing_and_hold_fixture(L, seed, density, site=0, pol=0):
    """A Bernoulli fluid with one SC edge and one FCC diagonal (both owned by `site`) blanked
    and forced to receive exactly one phase-2 absorption proposal each in the first cycle:
    every OTHER source channel that could compete for the same two relations is explicitly
    cleared (both polarities, all four passive phases) before the two chosen channels are set.
    The reserved tokens then phase-rotate every following cycle (ROTATE's two disjoint
    4-cycles), and whether each rotation crosses sides or holds at the gate depends on the
    surrounding fluid's population parity — over 32 microticks (8 cycles) this is verified
    below to produce at least one crossing and at least one gate hold."""
    st = R.fluid(L, seed, density)
    st.sc[site, 0] = ST.BLANK_IDX
    st.fcc[site, 0, 0] = ST.BLANK_IDX

    def force(kind, idx):
        chosen = None
        for v in range(24):
            target_kind, offset, target_idx = H.relation_target(v)
            if target_kind != kind or target_idx != idx:
                continue
            src = G.shift(L, site, offset)
            for p in range(2):
                for k in range(4):
                    st.bank[src, H.channel(p, k, v)] = False
            if chosen is None:
                chosen = (src, v)
        return chosen

    src_sc, v_sc = force("sc", (0,))
    src_fcc, v_fcc = force("fcc", (0, 0))
    st.bank[src_sc, H.channel(pol, 2, v_sc)] = True
    st.bank[src_fcc, H.channel(pol, 2, v_fcc)] = True
    return st


# (seed, site) chosen by exhaustive search over the Python reference (see task-9-report.md):
# both relations absorb at microtick 0 and the 32-microtick run produces >=1 crossing and
# >=1 gate hold for this exact fixture.
_CROSSING_SEED, _CROSSING_SITE, _CROSSING_L, _CROSSING_DENSITY = 19, 0, 5, 0.25
_CROSSING_TICKS = 32


@pytest.fixture(scope="module")
def prepared(table):
    return {
        "fluid_L4": R.fluid(4, 3, 0.25),
        "fluid_L7": R.fluid(7, 3, 0.25),
        "fluid_pol1": R.fluid(4, 2, 0.375, pol=1),
        "three_body_collision": _three_body_collision(table),
        "single_face_w0": _single_particle(4, _face_velocity(0)),
        "single_face_w1": _single_particle(4, _face_velocity(1)),
        "single_edge": _single_particle(4, _edge_velocity()),
        "seam_plus_x": _seam(4),
        "crossing_and_hold": _crossing_and_hold_fixture(_CROSSING_L, _CROSSING_SEED, _CROSSING_DENSITY, _CROSSING_SITE),
    }


def test_crossing_fixture_actually_exercises_crossings_and_holds(table, prepared):
    """The crossing_and_hold fixture must prove what it is for before it is trusted as a CUDA
    parity witness: at least one relation crossing and at least one gate hold in the Python
    reference's own event log over the 32-microtick run."""
    state = S.initialize(prepared["crossing_and_hold"])
    absorptions = crossings = gate_holds = 0
    for _ in range(_CROSSING_TICKS):
        state, ev = S.step(state, table)
        absorptions += len(ev.absorptions)
        crossings += len(ev.crossings)
        gate_holds += len(ev.gate_holds)
    assert absorptions == 2
    assert crossings >= 1
    assert gate_holds >= 1


@pytest.mark.parametrize("name,ticks", [
    ("fluid_L4", 12), ("fluid_L7", 12), ("fluid_pol1", 12), ("three_body_collision", 12),
    ("single_face_w0", 12), ("single_face_w1", 12), ("single_edge", 12), ("seam_plus_x", 12),
    ("crossing_and_hold", _CROSSING_TICKS),
])
def test_cuda_matches_python_reference_every_microtick(cuda, table_path, table, tmp_path, prepared, name, ticks):
    state = S.initialize(prepared[name])
    blob = N.encode(state)
    for _ in range(ticks):
        state, expected = S.step(state, table)
        run, target, event_file = invoke_cuda(cuda, table_path, tmp_path, blob, 1)
        assert run.returncode == 0, run.stderr
        blob = target.read_bytes()
        assert blob == N.encode(state)
        assert json.loads(event_file.read_text()) == native_json(expected)
        assert json.loads(run.stderr)["backend"] == "cuda_device_kernels"


def test_crossing_fixture_native_cli_agrees_every_microtick(native_cli, table_path, table, tmp_path, prepared):
    """`compare bytes and events after every microtick against both Python and, if cheap, the
    native CLI` (task-9-brief.md) for the crossing_and_hold fixture specifically: independent
    of CUDA/WSL availability, the native (CPU) CLI must also reproduce the Python reference
    across all 32 microticks -- cheap relative to the WSL/GPU round trip, so it runs whenever
    the native binary is built, regardless of whether the `cuda` fixture would skip."""
    if not native_cli:
        pytest.skip("hydro native CLI not built; configure engine/strict/hydro separately")
    state = S.initialize(prepared["crossing_and_hold"])
    blob = N.encode(state)
    for _ in range(_CROSSING_TICKS):
        state, expected = S.step(state, table)
        run, target, event_file = invoke_native(native_cli, table_path, tmp_path, blob, 1)
        assert run.returncode == 0, run.stderr
        blob = target.read_bytes()
        assert blob == N.encode(state)
        assert json.loads(event_file.read_text()) == native_json(expected)
