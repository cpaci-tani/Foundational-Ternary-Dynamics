"""Task 13: the Scale 0 hydro laboratory's preparation generator.

Determinism (two in-process runs write byte-identical files), every `.bin` decodes with
`codec.decode`, and the sidecar constants equal `boltzmann.verdict`/`nonlinear_coefficients`
at the registered density -- never re-derived here, compared directly against the same
functions the generator itself calls.
"""
from fractions import Fraction
from pathlib import Path

import pytest

from phi_v2_lattice.hydro import boltzmann as B
from phi_v2_lattice.hydro import channels as H
from phi_v2_lattice.hydro import codec as CODEC
from phi_v2_lattice.hydro import staged as Staged
from phi_v2_lattice.hydro.experiments import prepare_hydro_lab as LAB


@pytest.fixture(scope="module")
def generated(tmp_path_factory):
    out = tmp_path_factory.mktemp("hydro_lab")
    manifest = LAB.generate(out)
    return out, manifest


def test_determinism_two_runs_byte_identical(tmp_path_factory):
    first_dir = tmp_path_factory.mktemp("hydro_lab_run1")
    second_dir = tmp_path_factory.mktemp("hydro_lab_run2")
    LAB.generate(first_dir)
    LAB.generate(second_dir)
    first_files = sorted(p.name for p in first_dir.iterdir())
    second_files = sorted(p.name for p in second_dir.iterdir())
    assert first_files == second_files
    for name in first_files:
        assert (first_dir / name).read_bytes() == (second_dir / name).read_bytes(), name


def test_manifest_lists_every_preparation_and_file(generated):
    out, manifest = generated
    assert manifest["law_id"] == Staged.LAW_ID == "phi-hydro-staged-candidate-1"
    assert manifest["table_hash16"] == H.TABLE_HASH[:16]
    assert manifest["table_hash"] == H.TABLE_HASH
    assert manifest["encoding_hash"] == H.ENCODING_HASH
    assert len(manifest["preparations"]) == len(LAB.PREPARATIONS) * len(LAB.SIZES)
    names = {entry["name"] for entry in LAB.PREPARATIONS}
    assert names == {
        "hydro-shear-wave-t2", "hydro-shear-wave-e", "hydro-sound-wave",
        "hydro-taylor-green", "hydro-shear-layer", "hydro-vortex-pair",
    }
    for record in manifest["preparations"]:
        assert (out / record["bin"]).is_file()
        assert (out / record["json"]).is_file()
        assert manifest["files"][record["bin"]] == record["bin_sha256"]
        assert record["json"] in manifest["files"]


@pytest.mark.parametrize("entry", LAB.PREPARATIONS, ids=lambda e: e["name"])
@pytest.mark.parametrize("L", LAB.SIZES)
def test_bin_decodes_and_matches_hash(generated, entry, L):
    out, manifest = generated
    name = f"{entry['name']}_{L}.bin"
    data = (out / name).read_bytes()
    import hashlib
    assert hashlib.sha256(data).hexdigest() == manifest["files"][name]
    state = CODEC.decode(data)
    assert state.lattice.L == L
    assert state.microtick == 0
    Staged.validate(state)


@pytest.mark.parametrize("entry", LAB.PREPARATIONS, ids=lambda e: e["name"])
@pytest.mark.parametrize("L", LAB.SIZES)
def test_sidecar_constants_match_boltzmann(generated, entry, L):
    import json
    out, _manifest = generated
    sidecar = json.loads((out / f"{entry['name']}_{L}.json").read_text(encoding="utf-8"))
    assert sidecar["preparation"] == entry["name"]
    assert sidecar["L"] == L
    assert sidecar["law_id"] == Staged.LAW_ID
    assert sidecar["constant_probed"] == entry["constant_probed"]
    assert sidecar["direction"] == list(entry["direction"])
    assert sidecar["polarization"] == list(entry["polarization"])

    verdict = B.verdict(LAB.DENSITY)
    nl = B.nonlinear_coefficients(LAB.DENSITY)
    cubic = verdict["cubic_shear_constants"]
    expected = {
        "c_s2": Fraction(verdict["sound_speed_squared"]),
        "nu_T2": Fraction(cubic["nu_T2"]),
        "nu_E": Fraction(cubic["nu_E"]),
        "g": Fraction(nl["g"]),
    }
    for key, value in expected.items():
        entry_constants = sidecar["constants"][key]
        assert Fraction(entry_constants["exact"]) == value
        assert entry_constants["float"] == pytest.approx(float(value), rel=1e-15)

    prediction = sidecar["prediction"]
    assert prediction["stages"] == LAB.STAGES
    assert len(prediction["amplitude"]) == LAB.STAGES + 1
    assert all(isinstance(v, float) and v >= 0.0 for v in prediction["amplitude"])
    # The registered mode decays (or at worst does not grow) over the tracked horizon --
    # every H1' verdict at this density is stable (see boltzmann.verdict's "stable" flag).
    assert verdict["stable"] is True
    assert prediction["amplitude"][-1] <= prediction["amplitude"][0] * 1.001


def test_prediction_matches_direct_recomputation_for_one_case(generated):
    """Cross-check `_prediction`'s output against an independent recomputation of
    `m(n) = W P(k)^n h0` for one preparation, guarding against a silent construction bug
    that both the generator and this test would otherwise share if the test only called
    the generator's own helpers."""
    import numpy as np

    out, _manifest = generated
    import json
    entry = next(e for e in LAB.PREPARATIONS if e["name"] == "hydro-shear-wave-t2")
    L = 16
    sidecar = json.loads((out / f"{entry['name']}_{L}.json").read_text(encoding="utf-8"))

    occupancy = np.full(H.N_VEL, float(LAB.DENSITY))
    jacobian = B.numeric_jacobian(occupancy)
    k = 2.0 * np.pi * np.array(entry["direction"], dtype=float) / L
    V = np.array(H.VELOCITIES, dtype=float)
    phase_v = np.exp(-1j * (V @ k))
    Pk = phase_v[:, None] * (np.eye(H.N_VEL) + jacobian)
    W = np.array(B.weights_rows(), dtype=float)
    idx = np.arange(L ** 3)
    coords = np.stack([idx // (L * L), (idx // L) % L, idx % L], axis=1).astype(float)
    from phi_v2_lattice.hydro import prepare as HR
    p = HR.occupancy_probabilities(L, float(LAB.DENSITY), drift=lambda c: entry["drift"](c, L), base=None)
    phase = np.exp(-1j * (coords @ k))
    h = phase @ p.astype(complex)
    t = np.array(entry["polarization_vector"], dtype=float)
    recomputed = []
    for _ in range(LAB.STAGES + 1):
        moments = W @ h
        recomputed.append(float(abs(t @ moments[1:4])))
        h = Pk @ h

    amplitude = sidecar["prediction"]["amplitude"]
    assert len(amplitude) == len(recomputed)
    for a, b in zip(amplitude, recomputed):
        assert a == pytest.approx(b, rel=1e-9, abs=1e-12)
