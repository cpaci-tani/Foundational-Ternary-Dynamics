"""Complete-state/event parity between the Python phi-hydro-staged-candidate-1 reference and
an actually built native (C++) candidate CLI.

Set FTD_HYDRO_NATIVE_CLI to override the standalone build location. Missing binaries skip
these tests explicitly; source-only tests are not backend proof.
"""
import json
import os
from pathlib import Path
import subprocess

import pytest

from phi_v2_lattice import geometry as G
from phi_v2_lattice.hydro import channels as H, codec as N, prepare as R, staged as S, state as ST

ROOT = Path(__file__).resolve().parents[4]


@pytest.fixture(scope="module")
def cli():
    suffix = ".exe" if os.name == "nt" else ""
    path = Path(os.environ.get("FTD_HYDRO_NATIVE_CLI", ROOT / "engine/build_strict_hydro_native" / ("ftd_hydro_cli" + suffix)))
    if not path.is_file():
        pytest.skip("hydro native CLI not built; configure engine/strict/hydro separately")
    return str(path)


@pytest.fixture(scope="module")
def table_path():
    path = H.table_path()
    if not path.is_file():
        pytest.skip("hydro collision table blob not built; run generate_tables.py")
    return str(path)


@pytest.fixture(scope="module")
def table():
    return H.load_table()


def invoke(cli, table_path, tmp_path, blob, microticks):
    source, target, events = (tmp_path / name for name in ("input.bin", "output.bin", "events.json"))
    source.write_bytes(blob)
    run = subprocess.run([cli, table_path, str(source), str(target), str(microticks), str(events)],
                         capture_output=True, text=True, timeout=60)
    return run, target, events


def native_json(ev):
    """Translate the Python reference's TickEvents into the native events_json schema:
    collision polarity is signed (+1 pol 0 / -1 pol 1), absorption kind is numeric (0 sc,
    1 fcc); crossings/gate_holds already match (kind, owner, idx[, direction])."""
    absorptions = [[x, c, 0 if kind == "sc" else 1, owner, idx[0], idx[1] if len(idx) > 1 else 0]
                   for x, c, kind, owner, idx in ev.absorptions]
    collisions = [[site, 1 if pol == 0 else -1, mask, out] for site, pol, mask, out in ev.collisions]
    crossings = [[kind, owner, list(idx), direction] for kind, owner, idx, direction in ev.crossings]
    gate_holds = [[kind, owner, list(idx)] for kind, owner, idx in ev.gate_holds]
    return {"absorptions": absorptions, "collisions": collisions, "crossings": crossings, "gate_holds": gate_holds}


def _blank_both_kinds(st):
    """Blank one SC edge and one FCC diagonal so both absorption kinds are exercisable."""
    st.sc[0, 0] = ST.BLANK_IDX
    st.fcc[0, 0, 0] = ST.BLANK_IDX
    return st


def _single_particle(L, velocity, pol=0):
    """A single phase-2 particle of the given velocity at site 0, aimed at a blank relation."""
    st = R.frozen_background(L)
    _blank_both_kinds(st)
    kind, offset, idx = H.relation_target(velocity)
    owner = G.shift(L, 0, offset)
    if kind == "sc":
        st.sc[owner, idx[0]] = ST.BLANK_IDX
    else:
        st.fcc[owner, idx[0], idx[1]] = ST.BLANK_IDX
    st.bank[0, H.channel(pol, 2, velocity)] = True
    return st


def _face_velocity(w):
    return next(v for v in range(24) if H.IS_FACE[v] and H.FACE_W[v] == w)


def _edge_velocity():
    return next(v for v in range(24) if not H.IS_FACE[v])


def _three_body_collision(table):
    """The first collision-table mask that is not fixed, placed at one site (test_law.py's
    test_collision_events_change_velocities_but_not_class)."""
    first = next(s for s in range(1, 1 << 24) if int(table[s]) != s)
    st = ST.blank(4)
    x = G.site_index(4, 1, 1, 1)
    for v in range(24):
        if first >> v & 1:
            st.bank[x, H.channel(0, v % 4, v)] = True
    return R.with_background(st)


def _seam(L):
    """Particles at x = L-1 moving +x, so one-hop streaming wraps the periodic boundary."""
    st = R.frozen_background(L)
    for y in range(2):
        for z in range(2):
            x = G.site_index(L, L - 1, y, z)
            st.bank[x, H.channel(0, 0, 0)] = True  # face +x, w0, pol 0, phase 0
            st.bank[x, H.channel(1, 1, 1)] = True  # face +x, w1, pol 1, phase 1
    return st


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
    }


@pytest.mark.parametrize("name", ["fluid_L4", "fluid_L7", "fluid_pol1", "three_body_collision",
                                  "single_face_w0", "single_face_w1", "single_edge", "seam_plus_x"])
def test_native_matches_python_reference_every_microtick(cli, table_path, table, tmp_path, prepared, name):
    state = S.initialize(prepared[name])
    native = N.encode(state)
    for _ in range(12):
        state, expected = S.step(state, table)
        run, target, event_file = invoke(cli, table_path, tmp_path, native, 1)
        assert run.returncode == 0, run.stderr
        native = target.read_bytes()
        assert native == N.encode(state)
        assert json.loads(event_file.read_text()) == native_json(expected)
