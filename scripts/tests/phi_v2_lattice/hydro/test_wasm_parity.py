"""Actual compiled hydro-WASM parity, distinct from fake-adapter protocol tests.

Set FTD_HYDRO_WASM_MODULE to the freshly built ftd_hydro_wasm.mjs. Without that
explicit artifact the suite skips; a skip is never a backend certificate.

Beyond the Task 8 state/event parity (mirroring test_native_parity.py, but through the
JSON "ftd-hydro-checkpoint-2" schema instead of the native binary transport), this also
checks the WASM-only observables that have no native-CLI counterpart: `fields` (per-block
field-token counts and lattice-gas momentum, checked exactly against a block sum computed
directly here) and `moments:kx,ky,kz,pol` (a discrete Fourier projection, checked against
a direct summation computed here, within floating-point tolerance).
"""
import cmath
import json
import math
import os
from pathlib import Path
import subprocess

import pytest

from phi_v2_lattice import geometry as G
from phi_v2_lattice.hydro import channels as H, codec as N, prepare as R, staged as S
from .test_native_parity import (
    _face_velocity,
    _edge_velocity,
    _single_particle,
    _three_body_collision,
    _seam,
    native_json,
)

ROOT = Path(__file__).resolve().parents[4]
DRIVER = ROOT / "engine/web/tests/hydro-wasm-parity-driver.mjs"
MICROTICKS = 12


@pytest.fixture(scope="module")
def wasm_module():
    configured = os.environ.get("FTD_HYDRO_WASM_MODULE")
    if not configured:
        pytest.skip("actual hydro WASM module not requested")
    path = Path(configured).resolve()
    assert path.is_file(), path
    return path


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


def _python_fields(lattice, width):
    """Direct per-block field-token/momentum sum, matching wasm_bindings_hydro.cpp's
    fields_lab() exactly: block index ((x//w)*side + y//w)*side + z//w, field_tokens
    split by polarity, momentum the integer sum of occupied channels' velocity vectors."""
    L = lattice.L
    side = L // width
    count = side ** 3
    field_tokens = [[0, 0] for _ in range(count)]
    momentum = [[[0, 0, 0], [0, 0, 0]] for _ in range(count)]
    for i in range(L ** 3):
        x, y, z = G.coords(L, i)
        b = ((x // width) * side + y // width) * side + z // width
        bank = lattice.bank[i]
        for pol in range(2):
            for k in range(4):
                for v in range(24):
                    if not bank[H.channel(pol, k, v)]:
                        continue
                    field_tokens[b][pol] += 1
                    vel = H.VELOCITIES[v]
                    for d in range(3):
                        momentum[b][pol][d] += vel[d]
    return field_tokens, momentum


def _python_moments(lattice, k_vec, pol):
    """Direct sum_x e^{-ik.x} (N_x, Px_x, Py_x, Pz_x) for one polarity, matching
    wasm_bindings_hydro.cpp's moments_lab() exactly (same channel/velocity convention)."""
    L = lattice.L
    kx, ky, kz = k_vec
    factor = 2.0 * math.pi / L
    sums = [complex(0.0, 0.0) for _ in range(4)]
    for i in range(L ** 3):
        x, y, z = G.coords(L, i)
        bank = lattice.bank[i]
        n = px = py = pz = 0
        for k in range(4):
            for v in range(24):
                if not bank[H.channel(pol, k, v)]:
                    continue
                n += 1
                vel = H.VELOCITIES[v]
                px += vel[0]; py += vel[1]; pz += vel[2]
        phase = factor * (kx * x + ky * y + kz * z)
        e = cmath.exp(-1j * phase)
        for idx, value in enumerate((n, px, py, pz)):
            sums[idx] += value * e
    return sums


@pytest.mark.parametrize("name", ["fluid_L4", "fluid_L7", "fluid_pol1", "three_body_collision",
                                  "single_face_w0", "single_face_w1", "single_edge", "seam_plus_x"])
def test_wasm_matches_python_reference_every_microtick(wasm_module, table_path, table, tmp_path, prepared, name):
    state = S.initialize(prepared[name])
    input_path, output_path = tmp_path / "checkpoint.json", tmp_path / "rows.json"
    input_path.write_bytes(N.checkpoint(state))
    completed = subprocess.run(
        ["node", str(DRIVER), str(wasm_module), str(table_path), str(input_path), str(output_path), str(MICROTICKS)],
        capture_output=True, text=True, timeout=120)
    assert completed.returncode == 0, completed.stdout + completed.stderr
    rows = json.loads(output_path.read_text(encoding="utf-8"))
    assert len(rows) == MICROTICKS

    for row in rows:
        state, expected = S.step(state, table)

        # State: restore the WASM's JSON checkpoint through the same Python oracle used
        # to build the input, then compare via the native binary transport (numpy arrays
        # do not support direct dataclass equality).
        restored = N.restore(row["checkpoint"].encode("utf-8"))
        assert N.encode(restored) == N.encode(state)

        # Events: identical schema to the native CLI's events_json (same C++ function).
        assert row["advance"]["events"] == native_json(expected)
        assert row["advance"]["diagnostics"]["microtick"] == str(state.microtick)
        assert row["diagnostics"]["microtick"] == str(state.microtick)
        assert row["diagnostics"]["law"] == S.LAW_ID
        assert row["diagnostics"]["table_hash"] == H.TABLE_HASH
        assert row["diagnostics"]["work_units"] == str(S.work_units(state))

        # counts: internal consistency (field_tokens sums to the total bank population).
        counts = row["counts"]
        assert counts["law"] == S.LAW_ID
        assert counts["status"] == "exact_observation"
        assert sum(int(v) for v in counts["field_tokens"]) == int(state.lattice.bank.sum())

        # fields, exactly, at L=4 width=2 (and any other even L this fixture set reaches).
        L = state.lattice.L
        if L % 2 == 0:
            expected_tokens, expected_momentum = _python_fields(state.lattice, 2)
            blocks = row["fields"]["blocks"]
            assert len(blocks) == len(expected_tokens)
            for block, exp_tokens, exp_momentum in zip(blocks, expected_tokens, expected_momentum):
                assert [int(v) for v in block["field_tokens"]] == exp_tokens
                got_momentum = [[int(v) for v in axis] for axis in block["momentum"]]
                assert got_momentum == exp_momentum
        else:
            assert row["fields"] is None

        # moments:1,0,0,0, within floating-point tolerance, for polarity 0.
        expected_moments = _python_moments(state.lattice, (1, 0, 0), 0)
        assert row["moments"]["status"] == "approximate_observation"
        assert row["moments"]["arithmetic"] == "ieee754_binary64"
        assert row["moments"]["error_bound_status"] == "not_certified"
        assert row["moments"]["k"] == ["1", "0", "0"]
        assert row["moments"]["polarity"] == "0"
        got_moments = row["moments"]["moments"]
        assert len(got_moments) == 4
        for (re, im), expected_value in zip(got_moments, expected_moments):
            assert abs(re - expected_value.real) < 1e-9
            assert abs(im - expected_value.imag) < 1e-9


def test_wasm_checkpoint_integer_domains_and_rejection_atomicity(wasm_module, table_path, table, tmp_path):
    from phi_v2_lattice.hydro import state as lattice_state

    state = S.initialize(lattice_state.blank(4))
    input_path, output_path = tmp_path / "checkpoint.json", tmp_path / "boundaries.json"
    input_path.write_bytes(N.checkpoint(state))
    completed = subprocess.run(
        ["node", str(DRIVER), str(wasm_module), str(table_path), str(input_path), str(output_path), "--boundary"],
        capture_output=True, text=True, timeout=120)
    assert completed.returncode == 0, completed.stdout + completed.stderr
    result = json.loads(output_path.read_text(encoding="utf-8"))
    assert len(result["rejected"]) == 205
    assert len(set(result["rejected"])) == len(result["rejected"])
    assert result["counts"]["status"] == result["fields"]["status"] == "exact_observation"
    moments = result["extremeMoments"]
    assert moments["status"] == "approximate_observation"
    assert moments["arithmetic"] == "ieee754_binary64"
    assert moments["error_bound_status"] == "not_certified"
    assert moments["k"] == [str(-(2**63)), str(2**63 - 1), "0"]
    assert moments["polarity"] == "1"
    for row in result["accepted"]:
        before = N.restore(row["before"].encode())
        tick = int(row["tick"])
        assert before.microtick == tick
        assert row["diagnostics"]["microtick"] == str(tick)
        assert row["diagnostics"]["phase"] == str(tick % 4)
        payload = json.loads(row["before"])
        assert payload["schema"] == "ftd-hydro-checkpoint-2"
        assert payload["microtick"] == str(tick)
        restored = N.restore(row["after"].encode())
        if tick < 2**64 - 1:
            expected_state, events = S.step(before, table)
            assert N.encode(restored) == N.encode(expected_state)
            assert row["advance"]["events"] == native_json(events)
        else:
            assert N.encode(restored) == N.encode(before)


def test_wasm_lexical_integer_tokens_and_periodic_moment_aliases(wasm_module, table_path, tmp_path):
    from phi_v2_lattice.hydro import state as lattice_state

    state = S.initialize(lattice_state.blank(4))
    input_path, output_path = tmp_path / "checkpoint.json", tmp_path / "lexical-alias.json"
    input_path.write_bytes(N.checkpoint(state))
    completed = subprocess.run(
        ["node", str(DRIVER), str(wasm_module), str(table_path), str(input_path), str(output_path), "--lexical-alias"],
        capture_output=True, text=True, timeout=120)
    assert completed.returncode == 0, completed.stdout + completed.stderr
    result = json.loads(output_path.read_text(encoding="utf-8"))
    assert result["actualAdvanceCalls"] == 0
    assert len(result["rejected"]) == len(set(result["rejected"])) == 30
    assert result["legacyDiagnostics"]["microtick"] == "1"
    assert len(result["aliases"]) == 10
    lattice = N.restore(result["fixture"].encode()).lattice
    for row in result["aliases"]:
        requested, reduced = tuple(map(int, row["requested"])), tuple(map(int, row["reduced"]))
        assert all((a - b) % lattice.L == 0 for a, b in zip(requested, reduced))
        assert row["raw"]["k"] == list(row["requested"])
        assert row["raw"]["moments"] == row["canonical"]["moments"]
        assert row["raw"]["status"] == "approximate_observation"
        expected = _python_moments(lattice, reduced, row["pol"])
        for (re, im), value in zip(row["raw"]["moments"], expected):
            assert abs(re - value.real) < 1e-12
            assert abs(im - value.imag) < 1e-12
