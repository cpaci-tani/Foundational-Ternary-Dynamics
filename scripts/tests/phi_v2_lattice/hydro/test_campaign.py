# scripts/tests/phi_v2_lattice/hydro/test_campaign.py
"""H2-prime campaign apparatus: cases, preparations, observable, prediction, power
calculation, and the lock/run/summary protocol for phi-hydro-staged-candidate-1.

No registered campaign is exercised here (that is Task 12); the CUDA-gated tests below
use a throwaway single-case fixture at a tiny L purely to prove the instrument itself
(the built ftd_hydro_campaign binary) agrees with the Python reference, and skip cleanly
when that binary has not been built.
"""
from __future__ import annotations

from fractions import Fraction
import json
from pathlib import Path

import numpy as np
import pytest

from phi_v2_lattice.hydro import campaign as C
from phi_v2_lattice.hydro import channels as H
from phi_v2_lattice.hydro import codec as CODEC

ROOT = Path(__file__).resolve().parents[4]


def _runner_built() -> bool:
    return (ROOT / C.RUNNER).is_file()


# --- cases -------------------------------------------------------------------------


def test_case_count_matches_registered_inventory():
    cases = C.cases(16)
    assert len(cases) == (4 * 2 * 3 + 3 * 2 + 3 + 1 + 1) * C.SEEDS == 280
    assert len({c.case_id for c in cases}) == len(cases)


def test_case_id_encodes_arm_direction_polarization_wavenumber_epsilon_seed():
    cases = C.cases(16)
    shear_110_e = [c for c in cases if c.arm == "shear" and c.mx == 1 and c.my == 1 and c.mz == 0
                  and c.tx == 1 and c.ty == -1 and c.tz == 0 and c.eps == Fraction(1, 10) and c.seed == 0]
    assert len(shear_110_e) == 1
    assert shear_110_e[0].case_id == "shear-n110-t1m10-m1-e1_10-s0"


def test_shear_cells_cover_all_four_registered_direction_polarization_pairs():
    cases = C.cases(16)
    shear = [c for c in cases if c.arm == "shear"]
    pairs = {(C._unit_direction(c), (c.tx, c.ty, c.tz)) for c in shear}
    assert pairs == {(d, p) for d, p, _ in C.SHEAR_CELLS}


# --- preparation ---------------------------------------------------------------------


def test_preparation_is_deterministic_and_channel_exclusive():
    case = [c for c in C.cases(8) if c.arm == "shear"][0]
    a, b = C.prepare(case), C.prepare(case)
    assert CODEC.encode(a) == CODEC.encode(b)
    # at most one of the 4 passive phases occupied per (site, pol, velocity): staged.validate
    # already enforces this on construction (StagedState.validate), so a successful prepare()
    # (which calls Staged.initialize -> validate) is itself the check; this test just names it.
    bank = a.lattice.bank.reshape(-1, 2, 4, 24)
    assert bank.sum(axis=2).max() <= 1


@pytest.mark.parametrize("arm", ["shear", "sound", "density", "taylor_green", "galilean"])
def test_probability_field_is_bounded(arm):
    case = [c for c in C.cases(8) if c.arm == arm][0]
    p = C._probability_field(case)
    assert p.shape == (8 ** 3, 24)
    assert p.min() >= 0.0 and p.max() <= 1.0


# --- observable ------------------------------------------------------------------------


def test_observe_matches_direct_projection_definition():
    """Independent re-derivation of `observe`, not a call to it: recomputes the same
    Fourier-projected (mass, px, py, pz) moments directly from the raw bank with explicit
    per-site loops, guarding the vectorized reshape/aggregation in `observe` itself."""
    case = [c for c in C.cases(8) if c.arm == "shear"][0]
    state = C.prepare(case)
    L = case.L
    V = [tuple(v) for v in H.VELOCITIES]
    expected = np.zeros(4, dtype=complex)
    for i in range(L ** 3):
        x, y, z = i // (L * L), (i // L) % L, i % L
        phase = np.exp(-2j * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
        n_v = state.lattice.bank[i, 0:96].reshape(4, 24).sum(axis=0)
        mass = float(n_v.sum())
        momentum = [float(sum(n_v[v] * V[v][a] for v in range(24))) for a in range(3)]
        expected += phase * np.array([mass] + momentum, dtype=complex)
    assert np.allclose(C.observe(state, case), expected, atol=1e-9)


@pytest.mark.parametrize("arm", ["shear", "sound", "density", "galilean", "taylor_green"])
def test_predict_matches_observed_initial_amplitude_within_5_percent(arm):
    """predict(case, 0) must equal E[observe(prepare(case), case)]: the Python-visible
    proof is a many-seed average (a fixed, deterministic seed sequence, so this is
    reproducible, not flaky) converging on the analytic h0 within the registered 5%
    tolerance (task-11-brief.md `Tests`), for every arm at L=16."""
    cases = [c for c in C.cases(16) if c.arm == arm and c.eps == Fraction(1, 5)]
    if not cases:
        cases = [c for c in C.cases(16) if c.arm == arm]
    base = cases[0]
    predicted = C.predict(base, 0)[0]
    n_seeds, offset = 400, 31337
    observed = np.zeros(4, dtype=complex)
    for i in range(n_seeds):
        case = C.HydroCase(base.case_id, base.arm, base.L, base.mx, base.my, base.mz, base.tx, base.ty, base.tz,
                           base.eps, i + offset, base.u0)
        observed = observed + C.observe(C.prepare(case), case)
    observed = observed / n_seeds
    relative_error = np.linalg.norm(observed - predicted) / max(np.linalg.norm(predicted), 1e-30)
    assert relative_error < 0.05, f"{arm}: relative error {relative_error} >= 5%"


# --- power calculation -----------------------------------------------------------------


def test_se_gamma_closed_form_matches_synthetic_exponential_with_known_noise():
    """A synthetic A0*exp(-gamma n) + white Gaussian noise(sigma) series, fit by the same
    weighted log-linear estimator gamma_meas uses (`_fit_rate(..., gamma_weight=gamma)`),
    should have empirical scatter across many independent trials within a generous factor
    of the analytic `_se_gamma` closed form -- a sanity check that the closed form is the
    right order of magnitude and does not silently invert or drop a factor, not a claim
    that the two agree exactly (the closed form linearizes ln(signal + noise), which is
    only approximate; see campaign.py's own derivation note above `_fit_rate`)."""
    rng = np.random.default_rng(12345)
    gamma, A0, sigma, stages = 0.02, 1.0e5, 300.0, 64
    lo, hi = C._window(stages)
    n = np.arange(stages + 1, dtype=float)
    clean = A0 * np.exp(-gamma * n)
    trials = 1500
    rates = np.empty(trials)
    for t in range(trials):
        noisy = clean + rng.normal(0.0, sigma, size=clean.shape)
        rates[t] = C._fit_rate(noisy, lo, hi, gamma_weight=gamma)
    empirical_se = float(np.std(rates))
    closed_form_se = C._se_gamma(gamma, sigma, A0, lo, hi)
    assert closed_form_se > 0
    ratio = empirical_se / closed_form_se
    assert 0.3 < ratio < 3.0, f"empirical/closed-form SE ratio {ratio} far from 1"


def test_power_calculation_flags_low_amplitude_high_epsilon_cells_as_underpowered():
    power = C.power_calculation(16, 8)
    assert power["powered_count"] + power["underpowered_count"] == len(power["cells"]) == len(C._cells(16))
    for row in power["cells"]:
        assert row["powered"] == (row["power"] >= 5 and row["noise_ratio"] <= 1 / 3 and row["budget_ratio"] <= 1 / 3)


def test_horizon_is_positive_multiple_of_eight():
    h = C.horizon(16)
    assert h > 0 and h % 8 == 0


def test_registration_round_trips_through_json():
    reg = C.registration(16)
    assert json.loads(C._json(reg)) == json.loads(C._json(C.registration(16)))
    assert reg["case_count"] == 280
    assert reg["L"] == 16


# --- lock protocol (mirrors scripts/tests/phi_v2_lattice/test_recovery_hydro_campaign.py) --


def test_lock_rejects_tampering(tmp_path, monkeypatch):
    if not _runner_built():
        pytest.skip("optional hydro campaign runner (ftd_hydro_campaign) not built")
    one = [c for c in C.cases(4) if c.arm == "shear" and max(abs(c.mx), abs(c.my), abs(c.mz)) == 1][:1]
    monkeypatch.setattr(C, "cases", lambda L: one)
    monkeypatch.setenv("FTD_HYDRO_ALLOW_MISSING_PREREG", "1")
    C.prepare_campaign(tmp_path, L=4)
    C.validate_lock(tmp_path)
    (tmp_path / (one[0].case_id + ".bin")).write_bytes(b"\x00" * 10)
    with pytest.raises(ValueError):
        C.validate_lock(tmp_path)


def test_summarize_rejects_forged_backend(tmp_path, monkeypatch):
    if not _runner_built():
        pytest.skip("optional hydro campaign runner (ftd_hydro_campaign) not built")
    one = [c for c in C.cases(4) if c.arm == "shear" and max(abs(c.mx), abs(c.my), abs(c.mz)) == 1][:1]
    monkeypatch.setattr(C, "cases", lambda L: one)
    monkeypatch.setenv("FTD_HYDRO_ALLOW_MISSING_PREREG", "1")
    C.prepare_campaign(tmp_path, L=4)
    lock = json.loads((tmp_path / "lock.json").read_text())
    preflight = {"lock_sha256": C._sha((tmp_path / "lock.json").read_bytes()),
                 "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
                 "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
                 "case_count": len(lock["manifest"]), "validation": "complete preflight passed"}
    (tmp_path / "preflight.json").write_text(C._json(preflight) + "\n", encoding="utf-8")
    (tmp_path / "trace.jsonl").write_text("", encoding="utf-8")
    execution = {"schema": "strict-hydro4-execution-1",
                 "preflight_sha256": C._sha((tmp_path / "preflight.json").read_bytes()),
                 "postflight": "complete frozen-source/input validation passed",
                 "device": {"backend": "forged_not_real_gpu"},
                 "elapsed_seconds": 0.0, "physical_microticks": 0,
                 "trace_sha256": C._sha((tmp_path / "trace.jsonl").read_bytes())}
    (tmp_path / "execution.json").write_text(C._json(execution) + "\n", encoding="utf-8")
    with pytest.raises(ValueError, match="execution capability or case inventory mismatch"):
        C.summarize_campaign(tmp_path)


# --- instrument smoke test (Task 11's own end-to-end proof; Task 12 runs the real thing) --


def test_instrument_smoke_one_case_eight_stages_at_l8(tmp_path, monkeypatch):
    """Runs exactly one case through the real, built ftd_hydro_campaign for 8 stages at
    L=8 (the smoke test named in task-11-brief.md): the JSONL trace's stage-0 moments must
    match the Python `observe` projection of the same preparation to ~1e-9, and mass/total
    momentum must be exactly conserved (integer-equal) at every one of the 9 records."""
    if not _runner_built():
        pytest.skip("optional hydro campaign runner (ftd_hydro_campaign) not built")
    one = [c for c in C.cases(8) if c.arm == "shear" and max(abs(c.mx), abs(c.my), abs(c.mz)) == 1][:1]
    monkeypatch.setattr(C, "cases", lambda L: one)
    monkeypatch.setenv("FTD_HYDRO_ALLOW_MISSING_PREREG", "1")
    monkeypatch.setattr(C, "horizon", lambda L, budget_seconds=7200.0: 8)
    C.prepare_campaign(tmp_path, L=8)
    lock = json.loads((tmp_path / "lock.json").read_text())
    assert lock["registration"]["stages"] == 8
    execution = C.run_campaign(tmp_path)
    assert execution["device"]["backend"] == "cuda_device_kernels"
    report = C.summarize_campaign(tmp_path)
    assert report["groups"] == 1
    case = one[0]
    initial = CODEC.decode((tmp_path / (case.case_id + ".bin")).read_bytes())
    expected0 = C.observe(initial, case)
    with (tmp_path / "trace.jsonl").open(encoding="utf-8") as stream:
        stream.readline()  # header
        first = json.loads(stream.readline())
        measured0 = np.array([complex(a, b) for a, b in first["moments"]])
        assert np.allclose(measured0, expected0, atol=1e-9)
        mass0, momentum0 = first["mass"], tuple(first["total_momentum"])
        records = [first] + [json.loads(line) for line in stream if line.strip()]
    assert len(records) == 9  # stages 0..8 inclusive
    for rec in records:
        assert rec["mass"] == mass0
        assert tuple(rec["total_momentum"]) == momentum0
