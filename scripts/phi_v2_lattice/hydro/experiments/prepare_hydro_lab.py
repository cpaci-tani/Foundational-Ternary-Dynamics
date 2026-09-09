# scripts/phi_v2_lattice/hydro/experiments/prepare_hydro_lab.py
"""Scale 0 hydro laboratory preparations for `phi-hydro-staged-candidate-1` (Task 13;
docs/superpowers/specs/2026-09-08-hydro-scenarios-and-fchc-successor-design.md sec A.7,
amended by the owner ruling of 2026-09-08; task-13-brief.md). Mirrors
`engine/strict/prepare_lab.py` for the successor law.

Writes six declared preparations at L in {16, 32}, density d = 1/4, epsilon = 1/10,
seed 0 into an ignored build directory:
  - `<preparation>_<L>.bin` -- the native FTDHY01 transport (`codec.encode`), which the
    browser laboratory converts client-side into the WASM checkpoint JSON schema
    ("ftd-hydro-checkpoint-1") -- see `engine/strict/web/hydro/hydro-lab.js`.
  - `<preparation>_<L>.json` -- a sidecar carrying the registered (direction, m,
    polarization) mode this preparation's laboratory chart tracks, a 200-stage
    noiseless projected-mode prediction built from `boltzmann.numeric_stage_map`
    (the same construction `campaign.py`'s own `predict()` uses -- `m(n) = W P(k)^n h0`
    -- reimplemented here from the public `boltzmann`/`prepare`/`channels` helpers
    rather than imported from `campaign.py`, which is owned by a concurrent GPU
    campaign task (Task 12) and is never touched or depended on here), and the H1'
    constants of record (`c_s2`, `nu_T2`, `nu_E`, `g`) as exact Fraction strings and
    floats.
  - `manifest.json` -- SHA256 of every generated file, the collision table's hash16,
    and the law id.

`--check` runs `generate()` twice into two temporary directories and asserts the two
runs write byte-identical files (the same discipline as `run_hydro_evidence.py
--check`, extended from one JSON blob to a directory tree). The test
`scripts/tests/phi_v2_lattice/hydro/test_lab_preparations.py` does the equivalent
comparison directly against `generate()`.

Invocation (mirrors the other `hydro/experiments` scripts):
    PYTHONPATH=scripts python -m phi_v2_lattice.hydro.experiments.prepare_hydro_lab
"""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import tempfile

import numpy as np

from .. import boltzmann as B
from .. import channels as H
from .. import codec as CODEC
from .. import prepare as R
from .. import staged as Staged
from ... import geometry as G

ROOT = Path(__file__).resolve().parents[4]
DEFAULT_OUTPUT = ROOT / "engine" / "build_strict_hydro" / "lab"

DENSITY = Fraction(1, 4)
EPSILON = Fraction(1, 10)
SEED = 0
SIZES = (16, 32)
STAGES = 200


def _coords(L: int) -> np.ndarray:
    idx = np.arange(L ** 3)
    return np.stack(G.coords(L, idx), axis=1).astype(float)


def _k(direction, m, L):
    return 2.0 * np.pi * m * np.array(direction, dtype=float) / L


def _shear_drift(direction, m, transverse):
    """Reproduces `prepare.shear_wave`'s internal drift exactly (RNG-free), for the
    deterministic Fourier component `_h0` below needs -- never calls the RNG-drawing
    `fluid()`/`shear_wave()` path itself."""
    t = np.array(transverse, dtype=float)

    def drift(c, L):
        k = _k(direction, m, L)
        return float(EPSILON) * np.cos(c @ k)[:, None] * t[None, :]

    return drift


def _sound_drift(direction, m):
    """Reproduces `prepare.sound_wave`'s internal drift exactly: the polarization is the
    *normalized* direction (matching `prepare.py`'s own `n /= np.linalg.norm(n)`)."""
    n = np.array(direction, dtype=float)
    n = n / np.linalg.norm(n)

    def drift(c, L):
        k = _k(direction, m, L)
        return float(EPSILON) * np.cos(c @ k)[:, None] * n[None, :]

    return drift


def _taylor_green_drift(c, L):
    """Reproduces `prepare.taylor_green`'s internal drift exactly. This is a genuine
    two-wavevector superposition (sin(a)cos(b) = 1/2[sin(a+b)+sin(a-b)]); the registered
    wavevector this module tracks (direction (1,1,0), m=1) is one of its two components
    -- the other, (1,-1,0), is related by the same cubic symmetry the owner ruling names
    and is not separately registered here, matching `campaign.py`'s own documented
    convention for this arm."""
    kx = 2.0 * np.pi / L
    u = np.zeros_like(c)
    u[:, 0] = float(EPSILON) * np.sin(kx * c[:, 0]) * np.cos(kx * c[:, 1])
    u[:, 1] = -float(EPSILON) * np.cos(kx * c[:, 0]) * np.sin(kx * c[:, 1])
    return u


def _shear_layer_drift(c, L):
    """Reproduces `prepare.shear_layer`'s internal drift exactly: a square-wave (not
    sinusoidal) transverse profile in y, velocity along x. Its Fourier series carries
    only odd harmonics of (0,1,0)*2pi/L; the registered wavevector tracked here is the
    fundamental (m=1) -- an unregistered choice (this arm has no SHEAR_CELLS row), placed
    in the (1,0,0)/(0,1,0) cubic symmetry class (nu_T2) by the axis-aligned
    direction/polarization pair, per the owner ruling's own classification argument."""
    u = np.zeros_like(c)
    u[:, 0] = float(EPSILON) * np.where(c[:, 1] < L / 2, 1.0, -1.0)
    return u


def _vortex_pair_drift(c, L):
    """Reproduces `prepare.vortex_pair`'s internal drift exactly: a Taylor-Green-like
    pattern rotated into the (y,z) plane with velocity components (x,z). The same
    sin(a)cos(b) decomposition used for `_taylor_green_drift` gives two wavevector
    components, (0,1,1) with polarization (1,0,-1) and (0,1,-1) with polarization
    (1,0,1) (worked by hand from the drift formula below); the registered wavevector
    tracked here is the former -- an unregistered choice, placed in the (1,1,0)/(1,-1,0)
    cubic symmetry class (nu_E) by the face-diagonal direction with orthogonal in-plane
    polarization, per the owner ruling's own classification argument."""
    kx = 2.0 * np.pi / L
    u = np.zeros_like(c)
    u[:, 0] = float(EPSILON) * np.sin(kx * c[:, 1]) * np.cos(kx * c[:, 2])
    u[:, 2] = -float(EPSILON) * np.cos(kx * c[:, 1]) * np.sin(kx * c[:, 2])
    return u


# Six declared preparations. `direction`/`m` fix the registered wavevector
# k = 2 pi m direction / L that both the sidecar prediction and the laboratory's
# `moments` observable track; `polarization`/`polarization_vector` fix the momentum
# projection (raw declared integers for the shear-type cells, per the owner ruling of
# 2026-09-08's SHEAR_CELLS table; normalized for the sound cell, matching
# `campaign.py`'s `_effective_t` -- reimplemented here, campaign.py itself is never
# imported). `constant_probed` names which of the four constants of record this cell's
# decay rate approaches as k -> 0 (see the per-drift docstrings above for shear-layer and
# vortex-pair, which are not in the registered SHEAR_CELLS table).
PREPARATIONS = (
    dict(name="hydro-shear-wave-t2",
         build=lambda L: R.shear_wave(L, SEED, DENSITY, EPSILON, direction=(1, 0, 0), m=1, transverse=(0, 1, 0)),
         direction=(1, 0, 0), m=1, polarization=(0, 1, 0), polarization_vector=(0.0, 1.0, 0.0),
         drift=_shear_drift((1, 0, 0), 1, (0, 1, 0)), constant_probed="nu_T2"),
    dict(name="hydro-shear-wave-e",
         build=lambda L: R.shear_wave(L, SEED, DENSITY, EPSILON, direction=(1, 1, 0), m=1, transverse=(1, -1, 0)),
         direction=(1, 1, 0), m=1, polarization=(1, -1, 0), polarization_vector=(1.0, -1.0, 0.0),
         drift=_shear_drift((1, 1, 0), 1, (1, -1, 0)), constant_probed="nu_E"),
    dict(name="hydro-sound-wave",
         build=lambda L: R.sound_wave(L, SEED, DENSITY, EPSILON, direction=(1, 0, 0), m=1),
         direction=(1, 0, 0), m=1, polarization=(1, 0, 0), polarization_vector=(1.0, 0.0, 0.0),
         drift=_sound_drift((1, 0, 0), 1), constant_probed="c_s2"),
    dict(name="hydro-taylor-green",
         build=lambda L: R.taylor_green(L, SEED, DENSITY, EPSILON),
         direction=(1, 1, 0), m=1, polarization=(1, -1, 0), polarization_vector=(1.0, -1.0, 0.0),
         drift=_taylor_green_drift, constant_probed="nu_E"),
    dict(name="hydro-shear-layer",
         build=lambda L: R.shear_layer(L, SEED, DENSITY, EPSILON),
         direction=(0, 1, 0), m=1, polarization=(1, 0, 0), polarization_vector=(1.0, 0.0, 0.0),
         drift=_shear_layer_drift, constant_probed="nu_T2"),
    dict(name="hydro-vortex-pair",
         build=lambda L: R.vortex_pair(L, SEED, DENSITY, EPSILON),
         direction=(0, 1, 1), m=1, polarization=(1, 0, -1), polarization_vector=(1.0, 0.0, -1.0),
         drift=_vortex_pair_drift, constant_probed="nu_E"),
)


def _h0(entry: dict, L: int) -> np.ndarray:
    """The exact (to float64) Fourier component at the registered wavevector of the
    deterministic probability field: h0[v] = sum_x exp(-i k.x) p(x, v). Mirrors
    `campaign.py`'s `_h0`/`_probability_field`, built from the public
    `prepare.occupancy_probabilities` instead."""
    coords = _coords(L)
    k = _k(entry["direction"], entry["m"], L)
    p = R.occupancy_probabilities(L, float(DENSITY), drift=lambda c: entry["drift"](c, L), base=None)
    phase = np.exp(-1j * (coords @ k))
    return phase @ p.astype(complex)


def _prediction(entry: dict, L: int, jacobian: np.ndarray, stages: int = STAGES) -> list:
    """m(n) = W P(k)^n h0 for n = 0..stages, projected onto the declared polarization
    vector and reduced to |amplitude| -- mirrors `campaign.py`'s `predict()` +
    `_project()`. `jacobian` is `boltzmann.numeric_jacobian(occupancy)` at the uniform
    reference occupancy (none of these six preparations drift the reference occupancy
    itself -- only the Galilean arm in `campaign.py` does that, and it is not one of the
    six preparations here), computed once by the caller and reused across every
    (preparation, L) combination since `numeric_stage_map`'s expensive part depends only
    on occupancy, not on k."""
    k = _k(entry["direction"], entry["m"], L)
    V = np.array(H.VELOCITIES, dtype=float)
    phase_v = np.exp(-1j * (V @ k))
    Pk = phase_v[:, None] * (np.eye(H.N_VEL) + jacobian)
    Wt = np.array(B.weights_rows(), dtype=float)
    t = np.array(entry["polarization_vector"], dtype=float)
    h = _h0(entry, L)
    amplitudes = []
    for _ in range(stages + 1):
        moments = Wt @ h
        amplitudes.append(float(abs(t @ moments[1:4])))
        h = Pk @ h
    return amplitudes


def _constants() -> dict:
    """The H1' constants of record at the registered density, as exact Fraction strings
    and floats: `c_s2`, `nu_T2`, `nu_E` (`boltzmann.verdict`), `g`
    (`boltzmann.nonlinear_coefficients`)."""
    verdict = B.verdict(DENSITY)
    nl = B.nonlinear_coefficients(DENSITY)
    cubic = verdict["cubic_shear_constants"]
    if cubic is None:
        raise RuntimeError("H1' verdict did not resolve cubic_shear_constants at d=1/4")
    values = {
        "c_s2": Fraction(verdict["sound_speed_squared"]),
        "nu_T2": Fraction(cubic["nu_T2"]),
        "nu_E": Fraction(cubic["nu_E"]),
        "g": Fraction(nl["g"]),
    }
    return {name: {"exact": str(value), "float": float(value)} for name, value in values.items()}


def generate(output_dir: Path) -> dict:
    """Write every preparation, its sidecar, and the manifest into `output_dir`
    (created if missing). Returns the manifest dict."""
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    constants = _constants()
    occupancy = np.full(H.N_VEL, float(DENSITY))
    jacobian = B.numeric_jacobian(occupancy)

    files: dict[str, str] = {}
    preparations = []
    for entry in PREPARATIONS:
        for L in SIZES:
            lattice = entry["build"](L)
            state = Staged.initialize(lattice)
            data = CODEC.encode(state)
            bin_name = f"{entry['name']}_{L}.bin"
            bin_sha256 = hashlib.sha256(data).hexdigest()
            (output_dir / bin_name).write_bytes(data)
            files[bin_name] = bin_sha256

            amplitude = _prediction(entry, L, jacobian)
            k_integers = tuple(entry["m"] * c for c in entry["direction"])
            k_squared = (2.0 * np.pi / L) ** 2 * float(sum(c * c for c in k_integers))
            sidecar = {
                "preparation": entry["name"],
                "law_id": Staged.LAW_ID,
                "L": L,
                "density": {"exact": str(DENSITY), "float": float(DENSITY)},
                "epsilon": {"exact": str(EPSILON), "float": float(EPSILON)},
                "seed": SEED,
                "direction": list(entry["direction"]),
                "m": entry["m"],
                "k_integers": list(k_integers),
                "k_squared": k_squared,
                "polarization": list(entry["polarization"]),
                "polarization_vector": list(entry["polarization_vector"]),
                "constant_probed": entry["constant_probed"],
                "constants": constants,
                "prediction": {"stages": STAGES, "microticks_per_stage": 4, "amplitude": amplitude},
                "bin_file": bin_name,
                "bin_sha256": bin_sha256,
            }
            json_name = f"{entry['name']}_{L}.json"
            payload = json.dumps(sidecar, indent=2, sort_keys=True) + "\n"
            (output_dir / json_name).write_text(payload, encoding="utf-8")
            files[json_name] = hashlib.sha256(payload.encode("utf-8")).hexdigest()
            preparations.append({"name": entry["name"], "L": L, "bin": bin_name, "json": json_name,
                                  "bin_sha256": bin_sha256})

    manifest = {
        "law_id": Staged.LAW_ID,
        "table_hash": H.TABLE_HASH,
        "table_hash16": H.TABLE_HASH[:16],
        "encoding_hash": H.ENCODING_HASH,
        "density": {"exact": str(DENSITY), "float": float(DENSITY)},
        "epsilon": {"exact": str(EPSILON), "float": float(EPSILON)},
        "seed": SEED,
        "sizes": list(SIZES),
        "stages": STAGES,
        "preparations": preparations,
        "files": files,
        "status": "selected_finite_preparations_not_physical_particle_seeds",
    }
    (output_dir / "manifest.json").write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    return manifest


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument("--check", action="store_true",
                         help="generate twice into temporary directories and assert byte-identical files")
    args = parser.parse_args()

    if args.check:
        with tempfile.TemporaryDirectory() as first_dir, tempfile.TemporaryDirectory() as second_dir:
            generate(Path(first_dir))
            generate(Path(second_dir))
            first_files = sorted(Path(first_dir).iterdir())
            second_files = sorted(Path(second_dir).iterdir())
            if [p.name for p in first_files] != [p.name for p in second_files]:
                raise SystemExit("determinism check FAILED: file lists differ")
            for a, b in zip(first_files, second_files):
                if a.read_bytes() != b.read_bytes():
                    raise SystemExit(f"determinism check FAILED: {a.name} differs between runs")
        print(f"determinism check OK: two in-process runs wrote byte-identical files ({len(first_files)} files each)")
        return

    manifest = generate(args.output)
    print(f"Wrote {len(manifest['preparations'])} preparations to {args.output}")


if __name__ == "__main__":
    main()
