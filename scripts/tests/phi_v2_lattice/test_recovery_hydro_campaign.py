"""Preparation, observable, prediction and lock protocol for gate H2. No GPU trajectory here."""
from fractions import Fraction
import json
import os
from pathlib import Path
import subprocess

import numpy as np
import pytest

from phi_v2_lattice import native_codec as N_
from phi_v2_lattice import recovery_hydro_campaign as Camp
from phi_v2_lattice import recovery_hydro_dispersion as D

ROOT = Path(__file__).resolve().parents[3]


def test_cases_enumerate_registered_factors():
    cases = Camp.cases(L=16, stroboscopes=4)
    assert len(cases) == 3 * 2 * 7 * Camp.SEEDS  # directions x m x modes x seeds
    assert len({c.case_id for c in cases}) == len(cases)


def test_preparation_is_deterministic_and_bounded():
    case = Camp.cases(L=8, stroboscopes=1)[0]
    a, b = Camp.prepare(case), Camp.prepare(case)
    assert N_.encode(a) == N_.encode(b)
    probabilities = Camp.probabilities(case)
    assert probabilities.min() >= 0 and probabilities.max() <= 1
    assert a.lattice.bank[:, 192:].sum() == 0  # other polarity empty


def test_observable_matches_direct_definition():
    case = Camp.cases(L=8, stroboscopes=1)[0]
    state = Camp.prepare(case)
    W = np.array(Camp.weights_rows(), dtype=float)
    L = case.L
    expected = np.zeros(7, dtype=complex)
    for i in range(L ** 3):
        x, y, z = i // (L * L), (i // L) % L, i % L
        phase = np.exp(-2j * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
        expected += phase * (W @ state.lattice.bank[i, :192].astype(float))
    assert np.allclose(Camp.observe_moments(state, case), expected)


def test_prediction_starts_at_expected_initial_amplitude():
    case = Camp.cases(L=16, stroboscopes=3)[0]
    predicted = Camp.predict(case, 3)
    assert predicted.shape == (4, 7)
    # h0 = N p eps phi / 2 with phi the max-normalized column `mode` of V, so W h0 has a single
    # nonzero component at `mode` equal to N p eps / (2 * max|V column|), since W V = I exactly.
    W = np.array(Camp.weights_rows(), dtype=float)
    projected = W @ Camp.mode_shape(case.mode)
    amplitude = 16 ** 3 * float(Camp.P) * float(Camp.EPSILON) / 2
    assert np.allclose(predicted[0], amplitude * projected, rtol=1e-9, atol=1e-9 * amplitude)
    assert np.count_nonzero(np.abs(projected) > 1e-9) == 1 and abs(projected[case.mode]) > 1e-9


def test_instrument_parity_at_zero_stroboscopes(tmp_path):
    runner = ROOT / "engine/build_strict_hydro/ftd_strict_hydro_campaign"
    if not runner.is_file():
        pytest.skip("optional hydro campaign runner not built")
    case = Camp.cases(L=4, stroboscopes=0)[0]
    state = Camp.prepare(case)
    (tmp_path / "c.bin").write_bytes(N_.encode(state))
    Camp.write_weights(tmp_path / "weights.tsv")
    manifest = f"{case.case_id}\t{Camp._linux(tmp_path / 'c.bin')}\t{case.L}\t{case.mx}\t{case.my}\t{case.mz}\t0\n"
    (tmp_path / "manifest.tsv").write_text(manifest, encoding="utf-8")
    args = [Camp._linux(p) for p in (runner, tmp_path / "manifest.tsv", tmp_path / "trace.jsonl", tmp_path / "weights.tsv")]
    command = (["wsl", "-d", "Ubuntu-22.04", "--"] + args if os.name == "nt" else args) + ["0"]
    result = subprocess.run(command, capture_output=True, text=True, timeout=300)
    assert result.returncode == 0, result.stderr
    trace = json.loads((tmp_path / "trace.jsonl").read_text().splitlines()[0])
    measured = np.array([complex(a, b) for a, b in trace["stroboscopes"][0]["moments"]])
    assert np.allclose(measured, Camp.observe_moments(state, case), atol=1e-6)
    assert trace["stroboscopes"][0]["population"] == int(state.lattice.bank[:, :192].sum())


def test_lock_rejects_tampering(tmp_path, monkeypatch):
    if not (ROOT / "engine/build_strict_hydro/ftd_strict_hydro_campaign").is_file():
        pytest.skip("optional hydro campaign runner required for lock fixture")
    one = Camp.cases(L=4, stroboscopes=1)[:1]
    monkeypatch.setattr(Camp, "cases", lambda L, stroboscopes: one)
    monkeypatch.setenv("FTD_HYDRO_ALLOW_MISSING_PREREG", "1")
    Camp.prepare_campaign(tmp_path, L=4, stroboscopes=1)
    Camp.validate_lock(tmp_path)
    (tmp_path / (one[0].case_id + ".bin")).write_bytes(b"\x00" * 10)
    with pytest.raises(ValueError):
        Camp.validate_lock(tmp_path)
