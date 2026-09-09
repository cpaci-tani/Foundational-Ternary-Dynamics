# scripts/phi_v2_lattice/hydro/campaign.py
"""H2-prime registered response campaign for phi-hydro-staged-candidate-1: cases,
preparations, observable, exact H1'-conditioned predictions, power calculation, lock,
run, summary. Mirrors scripts/phi_v2_lattice/recovery_hydro_campaign.py (Phase 1) for
the successor law, amended per the owner ruling of 2026-09-08 (four (direction,
polarization) shear cells, each against its own exact constant; no isotropy pass band).

When FTD_HYDRO_ALLOW_MISSING_PREREG=1, prepare_campaign/validate_lock record "MISSING"
for any instrument path that does not exist on disk (unit fixtures only, exercised before
Task 12 writes PREREG_STRICT_HYDRO_VISCOSITY.md); the campaign of record runs without that
variable, so a missing instrument source is then a hard error.

Design notes (this module's own derivations, not retyped from H1'):
  - Every prediction is built by projecting the EXACT deterministic occupancy-probability
    field used by `prepare.py` (no RNG) onto the registered wavevector, then evolving it
    with `boltzmann.numeric_stage_map`. This makes `predict(case, 0)` equal, by
    construction, to E[observe(prepare(case), case)] -- no hand-derived amplitude formula
    to get subtly wrong; the only source of test disagreement is the RNG's own statistical
    noise, exactly the quantity `noise_floor` characterizes.
  - `predicted_rate` uses the EXACT (Fraction) small-k H1' constant times |k|^2 for every
    cell (nu_T2 / nu_E / the cubic mean for shear-type cells; the exact
    `longitudinal_damping_normalized` constant for sound/density cells) -- a leading-order
    approximation at the finite registered k, used for ACCEPTANCE; `predict`'s full
    `numeric_stage_map` (exact at any k, to float64) is used for the plotted/trajectory
    curve. This split is deliberate (see task-11-brief.md and the amending instructions).
  - The Taylor-Green preparation is a genuine two-wavevector superposition
    (sin(a)cos(b) = 1/2[sin(a+b)+sin(a-b)]); this module tracks only its k=(1,1,0)*2pi/L,
    polarization (1,-1,0) component (the other, k=(1,-1,0)/(1,1,0), is related by the same
    cubic symmetry and is not separately registered). This is a diagnostic/coverage case,
    not part of the brief's registered acceptance rows.
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
from fractions import Fraction
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import types

import numpy as np

from .. import geometry as G
from . import boltzmann as B
from . import channels as H
from . import codec as CODEC
from . import prepare as R
from . import staged as Staged

PROTOCOL_ID = "strict-hydro-viscosity-1"
DENSITY = Fraction(1, 4)
SEEDS = 8
EPSILONS = (Fraction(1, 20), Fraction(1, 10), Fraction(1, 5))
WAVENUMBERS = (1, 2)
U0 = Fraction(1, 10)
ACCEPTANCE = {"rate_sigma": 3, "rate_relative": Fraction(1, 20), "rms_relative": Fraction(1, 10),
              "noise_multiple": 3}
RUNNER = "engine/build_strict_hydro_cuda/ftd_hydro_campaign"
STAGE_MICROTICKS = 4

# The four registered (direction, polarization) shear cells and which exact H1' constant
# each probes (amendment of record, 2026-09-08): (1,0,0)/(0,1,0) and (1,1,0)/(0,0,1) -> nu_T2,
# (1,1,0)/(1,-1,0) -> nu_E, (1,1,1)/(1,-1,0) -> the cubic mean (2 nu_E + nu_T2)/3.
SHEAR_CELLS = (
    ((1, 0, 0), (0, 1, 0), "T2"),
    ((1, 1, 0), (0, 0, 1), "T2"),
    ((1, 1, 0), (1, -1, 0), "E"),
    ((1, 1, 1), (1, -1, 0), "cubic"),
)
SOUND_DENSITY_DIRECTIONS = ((1, 0, 0), (1, 1, 0), (1, 1, 1))


@dataclass(frozen=True)
class HydroCase:
    case_id: str
    arm: str          # shear | sound | density | taylor_green | galilean
    L: int
    mx: int
    my: int
    mz: int
    tx: int
    ty: int
    tz: int
    eps: Fraction
    seed: int
    u0: Fraction


def _tag(vec) -> str:
    return "".join(str(x) if x >= 0 else f"m{-x}" for x in vec)


def _eps_tag(eps: Fraction) -> str:
    return f"{eps.numerator}_{eps.denominator}"


def _case_id(arm, direction, polarization, m, eps: Fraction, seed: int) -> str:
    return f"{arm}-n{_tag(direction)}-t{_tag(polarization)}-m{m}-e{_eps_tag(eps)}-s{seed}"


def cases(L: int) -> tuple[HydroCase, ...]:
    out = []
    for direction, polarization, _kind in SHEAR_CELLS:
        for m in WAVENUMBERS:
            for eps in EPSILONS:
                for seed in range(SEEDS):
                    mx, my, mz = (m * c for c in direction)
                    out.append(HydroCase(_case_id("shear", direction, polarization, m, eps, seed), "shear", L,
                                         mx, my, mz, *polarization, eps, seed, Fraction(0)))
    for direction in SOUND_DENSITY_DIRECTIONS:
        for m in WAVENUMBERS:
            eps = Fraction(1, 10)
            for seed in range(SEEDS):
                mx, my, mz = (m * c for c in direction)
                out.append(HydroCase(_case_id("sound", direction, direction, m, eps, seed), "sound", L,
                                     mx, my, mz, *direction, eps, seed, Fraction(0)))
    for direction in SOUND_DENSITY_DIRECTIONS:
        m, eps = 1, Fraction(1, 10)
        for seed in range(SEEDS):
            mx, my, mz = (m * c for c in direction)
            out.append(HydroCase(_case_id("density", direction, (0, 0, 0), m, eps, seed), "density", L,
                                 mx, my, mz, 0, 0, 0, eps, seed, Fraction(0)))
    eps = Fraction(1, 10)
    for seed in range(SEEDS):
        out.append(HydroCase(_case_id("taylor_green", (1, 1, 0), (1, -1, 0), 1, eps, seed), "taylor_green", L,
                             1, 1, 0, 1, -1, 0, eps, seed, Fraction(0)))
    eps = Fraction(1, 10)
    for seed in range(SEEDS):
        out.append(HydroCase(_case_id("galilean", (1, 0, 0), (0, 1, 0), 1, eps, seed), "galilean", L,
                             1, 0, 0, 0, 1, 0, eps, seed, U0))
    return tuple(out)


def _cell_key(case: HydroCase):
    return (case.arm, case.mx, case.my, case.mz, case.tx, case.ty, case.tz, str(case.eps), str(case.u0))


def _cells(L: int) -> dict:
    """One representative case per (arm, k, polarization, eps, u0) cell (seed dropped)."""
    out = {}
    for case in cases(L):
        out.setdefault(_cell_key(case), case)
    return out


def _unit_direction(case: HydroCase) -> tuple[int, int, int]:
    m = max(abs(case.mx), abs(case.my), abs(case.mz))
    if m == 0:
        raise ValueError("zero wavevector is not registered")
    return (case.mx // m, case.my // m, case.mz // m)


def _k_squared(case: HydroCase, L: int | None = None) -> float:
    L = case.L if L is None else L
    return (2.0 * np.pi / L) ** 2 * float(case.mx ** 2 + case.my ** 2 + case.mz ** 2)


_VERDICT_CACHE: dict = {}


def _verdict() -> dict:
    if "v" not in _VERDICT_CACHE:
        _VERDICT_CACHE["v"] = B.verdict(DENSITY)
    return _VERDICT_CACHE["v"]


def _shear_constant(direction: tuple, polarization: tuple) -> Fraction:
    v = _verdict()
    cubic = v["cubic_shear_constants"]
    nu_T2, nu_E = Fraction(cubic["nu_T2"]), Fraction(cubic["nu_E"])
    for cell_direction, cell_polarization, kind in SHEAR_CELLS:
        if cell_direction == direction and cell_polarization == polarization:
            if kind == "T2":
                return nu_T2
            if kind == "E":
                return nu_E
            return (2 * nu_E + nu_T2) / 3
    raise ValueError(f"unregistered shear cell {direction}/{polarization}")


def _longitudinal_damping(direction: tuple) -> Fraction:
    return Fraction(_verdict()["longitudinal_damping_normalized"][str(direction)])


def predicted_rate(case: HydroCase) -> float:
    """Exact small-k H1' constant times |k|^2 (per stage); see module docstring."""
    k2 = _k_squared(case)
    direction = _unit_direction(case)
    if case.arm in ("shear", "galilean", "taylor_green"):
        nu = _shear_constant(direction, (int(case.tx), int(case.ty), int(case.tz)))
        return float(nu) * k2
    if case.arm in ("sound", "density"):
        return float(_longitudinal_damping(direction)) * k2
    raise ValueError(f"no registered exact predicted rate for arm {case.arm}")


def _effective_t(case: HydroCase) -> np.ndarray:
    """The polarization/direction vector actually baked into the RNG-free probability
    field: raw (tx,ty,tz) for shear/galilean/taylor_green (prepare.shear_wave does not
    normalize `transverse`), unit-normalized for sound (prepare.sound_wave normalizes
    `direction` internally). Density has no such vector; callers must not call this for it."""
    t = np.array([case.tx, case.ty, case.tz], dtype=float)
    if case.arm == "sound":
        return t / np.linalg.norm(t)
    return t


def _phi_vector(case: HydroCase) -> np.ndarray:
    if case.arm == "density":
        return np.ones(H.N_VEL)
    V = np.array(H.VELOCITIES, dtype=float)
    return V @ _effective_t(case)


def _project(vec4: np.ndarray, case: HydroCase):
    """Reduce the raw (mass, px, py, pz) 4-vector to the single scalar mode this cell's
    physics actually excites: the mass channel for density, the momentum projected onto
    the (effective) polarization vector for every other arm."""
    if case.arm == "density":
        return vec4[0]
    return complex(_effective_t(case) @ vec4[1:4])


def _coords(L: int) -> np.ndarray:
    idx = np.arange(L ** 3)
    return np.stack(G.coords(L, idx), axis=1).astype(float)


def _probability_field(case: HydroCase) -> np.ndarray:
    """Exact (RNG-free) per-(site,velocity) occupancy probability used by `prepare(case)`,
    reproducing scripts/phi_v2_lattice/hydro/prepare.py's own formulas exactly."""
    L, d, eps = case.L, float(DENSITY), float(case.eps)
    coords = _coords(L)
    k = 2.0 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / L
    if case.arm == "density":
        probs = np.clip(d * (1.0 + eps * np.cos(coords @ k)), 0.0, 1.0)
        return np.repeat(probs[:, None], H.N_VEL, axis=1)
    V = np.array(H.VELOCITIES, dtype=float)
    if case.arm == "shear":
        t = _effective_t(case)
        u = eps * np.cos(coords @ k)[:, None] * t[None, :]
        base = np.full(H.N_VEL, d)
    elif case.arm == "sound":
        n = _effective_t(case)
        u = eps * np.cos(coords @ k)[:, None] * n[None, :]
        base = np.full(H.N_VEL, d)
    elif case.arm == "galilean":
        t = _effective_t(case)
        u0v = np.array([float(case.u0), 0.0, 0.0])
        u = u0v[None, :] + eps * np.cos(coords @ k)[:, None] * t[None, :]
        base = B.equilibrium(d, float(case.u0))
    elif case.arm == "taylor_green":
        kx = 2.0 * np.pi / L
        u = np.zeros((L ** 3, 3))
        u[:, 0] = eps * np.sin(kx * coords[:, 0]) * np.cos(kx * coords[:, 1])
        u[:, 1] = -eps * np.cos(kx * coords[:, 0]) * np.sin(kx * coords[:, 1])
        base = np.full(H.N_VEL, d)
    else:
        raise ValueError(f"unknown arm {case.arm}")
    return np.clip(base[None, :] * (1.0 + (u @ V.T) / float(R.C2)), 0.0, 1.0)


def _h0(case: HydroCase) -> np.ndarray:
    """The exact (to float64) Fourier component at the registered wavevector of the
    deterministic probability field: h0[v] = sum_x exp(-i k.x) p(x, v)."""
    L = case.L
    coords = _coords(L)
    k = 2.0 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / L
    phase = np.exp(-1j * (coords @ k))
    p = _probability_field(case)
    return phase @ p.astype(complex)


def _occupancy(case: HydroCase) -> np.ndarray:
    if case.arm == "galilean":
        return B.equilibrium(float(DENSITY), float(case.u0))
    return np.full(H.N_VEL, float(DENSITY))


def prepare(case: HydroCase) -> Staged.StagedState:
    L, d, eps, seed = case.L, DENSITY, case.eps, case.seed
    if case.arm == "shear":
        lattice = R.shear_wave(L, seed, d, eps, direction=(case.mx, case.my, case.mz), m=1,
                               transverse=(case.tx, case.ty, case.tz))
    elif case.arm == "sound":
        lattice = R.sound_wave(L, seed, d, eps, direction=(case.mx, case.my, case.mz), m=1)
    elif case.arm == "density":
        lattice = R.density_wave(L, seed, d, eps, direction=(case.mx, case.my, case.mz), m=1)
    elif case.arm == "taylor_green":
        lattice = R.taylor_green(L, seed, d, eps)
    elif case.arm == "galilean":
        u0 = float(case.u0)
        lattice = R.shear_wave(L, seed, d, eps, direction=(case.mx, case.my, case.mz), m=1,
                               transverse=(case.tx, case.ty, case.tz), base=B.equilibrium(float(d), u0),
                               background=(u0, 0.0, 0.0))
    else:
        raise ValueError(f"unknown arm {case.arm}")
    return Staged.initialize(lattice)


def observe(state: Staged.StagedState, case: HydroCase) -> np.ndarray:
    """The four projected moments (mass, px, py, pz) at the registered wavevector,
    aggregated over polarity 0 (the only polarity any preparation here populates)."""
    L = case.L
    W = np.array(B.weights_rows(), dtype=float)
    coords = _coords(L)
    k = 2.0 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / L
    phase = np.exp(-1j * (coords @ k))
    bank = state.lattice.bank[:, 0:96].reshape(L ** 3, H.N_PHASE, H.N_VEL)
    n_v = bank.sum(axis=1).astype(float)
    return phase @ (n_v @ W.T)


_JACOBIAN_CACHE: dict = {}


def _cached_jacobian(occupancy: np.ndarray) -> np.ndarray:
    """Memoized `boltzmann.numeric_jacobian`: it depends only on `occupancy`, which takes
    just two distinct values across every registered case (uniform DENSITY, or the
    Galilean arm's drifted equilibrium) -- caching here (never touching boltzmann.py
    itself, which is frozen) turns O(case count) 8-second exhaustive-table passes into
    O(distinct occupancies)."""
    key = tuple(np.round(occupancy, 15).tolist())
    if key not in _JACOBIAN_CACHE:
        _JACOBIAN_CACHE[key] = B.numeric_jacobian(occupancy)
    return _JACOBIAN_CACHE[key]


def _cached_stage_map(k: np.ndarray, occupancy: np.ndarray) -> np.ndarray:
    """Same contract and result as `boltzmann.numeric_stage_map(k, occupancy)`, with the
    expensive Jacobian memoized (see `_cached_jacobian`)."""
    V = np.array(H.VELOCITIES, dtype=float)
    phase = np.exp(-1j * (V @ k))
    return phase[:, None] * (np.eye(H.N_VEL) + _cached_jacobian(occupancy))


def predict(case: HydroCase, stages: int) -> np.ndarray:
    """m(n) = W P(k)^n h0, n = 0..stages (float64; see module docstring)."""
    k = 2.0 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / case.L
    Pk = _cached_stage_map(k, _occupancy(case))
    W = np.array(B.weights_rows(), dtype=float)
    h = _h0(case)
    out = np.zeros((stages + 1, 4), dtype=complex)
    for n in range(stages + 1):
        out[n] = W @ h
        h = Pk @ h
    return out


def noise_floor(case: HydroCase) -> float:
    """Analytic per-stage noise floor after averaging SEEDS seeds:
    sigma = sqrt(L^3 d (1-d) sum_v phi_v^2) / sqrt(SEEDS)."""
    L, d = case.L, float(DENSITY)
    phi = _phi_vector(case)
    return float(np.sqrt((L ** 3) * d * (1.0 - d) * np.sum(phi ** 2)) / np.sqrt(SEEDS))


def _amplitude(case: HydroCase) -> float:
    return abs(_project(predict(case, 0)[0], case))


def _se_gamma(gamma: float, sigma: float, amplitude: float, lo: int, hi: int) -> float:
    """Closed-form expected standard error of a log-linear rate fit over [lo, hi] with
    white noise sigma on an exponential of amplitude `amplitude` and rate `gamma`."""
    if amplitude <= 0:
        return float("inf")
    j = np.arange(lo, hi + 1, dtype=float)
    if j.size < 2:
        return float("inf")
    jbar = j.mean()
    denom = float(np.sqrt(np.sum((j - jbar) ** 2 * np.exp(-2.0 * gamma * j))))
    if denom <= 0:
        return float("inf")
    return sigma / (amplitude * denom)


def _window(stages: int) -> tuple[int, int]:
    return stages // 4, (3 * stages) // 4


def power_calculation(L: int, stages: int) -> dict:
    lo, hi = _window(stages)
    rows = []
    for key, case in sorted(_cells(L).items()):
        gamma = predicted_rate(case)
        amplitude = _amplitude(case)
        sigma = noise_floor(case)
        se_gamma = _se_gamma(gamma, sigma, amplitude, lo, hi)
        power = gamma / se_gamma if se_gamma > 0 else 0.0
        noise_ratio = sigma / amplitude if amplitude > 0 else float("inf")
        budget_ratio = (float(case.eps) / float(R.C2)) ** 2 / float(ACCEPTANCE["rate_relative"])
        powered = bool(power >= 5 and noise_ratio <= Fraction(1, 3) and budget_ratio <= Fraction(1, 3))
        rows.append({"cell": list(key), "case_id_representative": case.case_id, "predicted_rate": gamma,
                     "amplitude": amplitude, "noise_floor": sigma, "se_gamma": se_gamma, "power": power,
                     "noise_ratio": noise_ratio, "budget_ratio": budget_ratio, "powered": powered})
    return {"L": L, "stages": stages, "window": [lo, hi], "cells": rows,
            "powered_count": sum(1 for r in rows if r["powered"]),
            "underpowered_count": sum(1 for r in rows if not r["powered"])}


def _throughput_microticks_per_second(L: int) -> float | None:
    root = Path(__file__).resolve().parents[3]
    path = root / "engine/docs/evidence/strict-hydro4-throughput.json"
    if not path.is_file():
        return None
    payload = json.loads(path.read_text(encoding="utf-8"))
    runs = payload.get("runs", [])
    exact = [r for r in runs if r.get("L") == L]
    chosen = exact[0] if exact else (min(runs, key=lambda r: abs(r.get("L", 0) - L)) if runs else None)
    return float(chosen["microticks_per_second"]) if chosen else None


def horizon(L: int, budget_seconds: float = 7200.0) -> int:
    """Three e-folds of the slowest m=1 shear prediction, rounded up to a multiple of 8,
    capped by the probe's measured throughput within `budget_seconds` (uncapped, just
    rounded, when no throughput evidence is on disk yet)."""
    slow = min(predicted_rate(case) for case in cases(L)
               if case.arm == "shear" and max(abs(case.mx), abs(case.my), abs(case.mz)) == 1)
    needed = int(np.ceil(3.0 / slow))
    needed = ((needed + 7) // 8) * 8
    rate = _throughput_microticks_per_second(L)
    if rate is None:
        return max(8, needed)
    n_cases = len(cases(L))
    affordable_stages = int(budget_seconds * rate // (n_cases * STAGE_MICROTICKS))
    affordable_stages = max(8, (affordable_stages // 8) * 8)
    return max(8, min(needed, affordable_stages))


def registration(L: int) -> dict:
    stages = horizon(L)
    return {"protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID, "density": str(DENSITY),
            "epsilons": [str(e) for e in EPSILONS], "wavenumbers": list(WAVENUMBERS), "u0": str(U0),
            "seeds": SEEDS, "L": L, "stages": stages, "microticks_per_stage": STAGE_MICROTICKS,
            "acceptance": {k: str(v) for k, v in ACCEPTANCE.items()},
            "shear_cells": [[list(d), list(p), kind] for d, p, kind in SHEAR_CELLS],
            "case_count": len(cases(L)), "table_hash": H.TABLE_HASH,
            "boundary": "periodic",
            "background": "frozen doubly-occupied SC/FCC relations; Bernoulli fluid bank at density d"}


def _json(value) -> str:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), default=list)


def _sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _linux(path) -> str:
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def _case_dict(case: HydroCase) -> dict:
    d = asdict(case)
    d["eps"] = str(case.eps)
    d["u0"] = str(case.u0)
    return d


def _case_from_dict(d: dict) -> HydroCase:
    d = dict(d)
    d["eps"] = Fraction(d["eps"])
    d["u0"] = Fraction(d["u0"])
    return HydroCase(**d)


def instrument_paths() -> set[str]:
    root = Path(__file__).resolve().parents[3]
    paths = {"engine/strict/hydro/campaign_main.cpp", "engine/strict/hydro/campaign/CMakeLists.txt",
             "engine/strict/hydro/CMakeLists.txt", "engine/strict/hydro/cuda.cmake",
             "engine/strict/hydro/hydro_runtime.cpp", "engine/strict/hydro/hydro_runtime.h",
             "engine/strict/hydro/hydro_cuda.cu", "engine/strict/hydro/hydro_cuda.h",
             "engine/strict/hydro/hydro_tables.h", "engine/strict/hydro/hydro_sha256.h",
             "engine/docs/PREREG_STRICT_HYDRO_VISCOSITY.md"}
    seeds = (__name__, "phi_v2_lattice", "phi_v2_lattice.geometry", "phi_v2_lattice.state",
             "phi_v2_lattice._proofs", "phi_v2_lattice.tick", "phi_v2_lattice.hydro",
             "phi_v2_lattice.hydro.channels", "phi_v2_lattice.hydro.state", "phi_v2_lattice.hydro.tick",
             "phi_v2_lattice.hydro.staged", "phi_v2_lattice.hydro.codec", "phi_v2_lattice.hydro.prepare",
             "phi_v2_lattice.hydro.invariants", "phi_v2_lattice.hydro.boltzmann",
             "phi_v2_lattice.recovery_hydro_dispersion", "phi_v2_lattice.recovery_hydro_verdict",
             "phi_v2_lattice.recovery_kinetic_response")
    pending = [sys.modules[name] for name in seeds if name in sys.modules]
    seen = set()
    while pending:
        module = pending.pop()
        if module.__name__ in seen:
            continue
        seen.add(module.__name__)
        raw = getattr(module, "__file__", None)
        if not raw:
            continue
        path = Path(raw).resolve()
        if path.suffix != ".py" or not path.is_relative_to(root):
            continue
        paths.add(path.relative_to(root).as_posix())
        for value in tuple(vars(module).values()):
            if isinstance(value, types.ModuleType):
                pending.append(value)
            elif isinstance(value, (types.FunctionType, type)):
                parent = sys.modules.get(getattr(value, "__module__", ""))
                if parent is not None:
                    pending.append(parent)
    return paths


def _allow_missing_prereg() -> bool:
    return os.environ.get("FTD_HYDRO_ALLOW_MISSING_PREREG") == "1"


def _hash_instrument_path(root, relative) -> str:
    full = root / relative
    if not full.is_file():
        if _allow_missing_prereg():
            return "MISSING"
        raise ValueError(f"instrument source path does not exist: {relative}")
    return _sha(full.read_bytes())


def prepare_campaign(directory, L: int) -> dict:
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    if (directory / "lock.json").exists():
        raise ValueError("campaign already locked; do not overwrite registration")
    root = Path(__file__).resolve().parents[3]
    runner = root / RUNNER
    if not runner.is_file():
        raise ValueError("build the real CUDA hydro campaign runner before locking")
    reg = registration(L)
    stages = reg["stages"]
    manifest, lines, predictions = [], [], {}
    for case in cases(L):
        path = directory / (case.case_id + ".bin")
        blob = CODEC.encode(prepare(case))
        path.write_bytes(blob)
        manifest.append({"case": _case_dict(case), "initial_sha256": _sha(blob)})
        lines.append("\t".join([case.case_id, _linux(path), str(case.L), str(case.mx), str(case.my),
                                str(case.mz), "0"]))
        predicted = predict(case, stages)
        predictions[case.case_id] = [[[float(v.real), float(v.imag)] for v in row] for row in predicted]
    (directory / "predictions.json").write_bytes((_json(predictions) + "\n").encode("utf-8"))
    lock = {"registration": reg, "registration_sha256": _sha(_json(reg).encode()),
            "manifest": manifest, "manifest_sha256": _sha(_json(manifest).encode()),
            "predictions_sha256": _sha((directory / "predictions.json").read_bytes()),
            "instrument_sha256": {p: _hash_instrument_path(root, p) for p in sorted(instrument_paths())},
            "runner_sha256": _sha(runner.read_bytes())}
    (directory / "lock.json").write_text(_json(lock) + "\n", encoding="utf-8")
    (directory / "manifest.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return {"case_count": len(manifest), "manifest_sha256": lock["manifest_sha256"],
            "registration_sha256": lock["registration_sha256"], "stages": stages,
            "microticks": len(manifest) * stages * STAGE_MICROTICKS}


def validate_lock(directory) -> dict:
    directory = Path(directory)
    lock = json.loads((directory / "lock.json").read_text())
    root = Path(__file__).resolve().parents[3]
    if set(lock["instrument_sha256"]) != instrument_paths():
        raise ValueError("incomplete instrument source closure")
    if any(_hash_instrument_path(root, p) != h for p, h in lock["instrument_sha256"].items()):
        raise ValueError("instrument source changed after registration lock")
    runner = root / RUNNER
    if not runner.is_file():
        raise ValueError("CUDA hydro campaign runner missing")
    if _sha(runner.read_bytes()) != lock["runner_sha256"]:
        raise ValueError("CUDA runner changed after registration lock")
    reg = lock["registration"]
    if _sha(_json(reg).encode()) != lock["registration_sha256"] or _json(reg) != _json(registration(reg["L"])):
        raise ValueError("registration lock changed")
    if _sha(_json(lock["manifest"]).encode()) != lock["manifest_sha256"]:
        raise ValueError("manifest lock changed")
    if [row["case"] for row in lock["manifest"]] != [_case_dict(c) for c in cases(reg["L"])]:
        raise ValueError("manifest case inventory differs from registration")
    if _sha((directory / "predictions.json").read_bytes()) != lock["predictions_sha256"]:
        raise ValueError("locked predictions changed")
    expected = []
    for row in lock["manifest"]:
        c = row["case"]
        path = directory / (c["case_id"] + ".bin")
        if _sha(path.read_bytes()) != row["initial_sha256"]:
            raise ValueError("initial preparation hash changed")
        expected.append("\t".join([c["case_id"], _linux(path), str(c["L"]), str(c["mx"]), str(c["my"]),
                                   str(c["mz"]), "0"]))
    if (directory / "manifest.tsv").read_text(encoding="utf-8") != "\n".join(expected) + "\n":
        raise ValueError("executable manifest differs from lock")
    return lock


def run_campaign(directory) -> dict:
    directory = Path(directory).resolve()
    lock = validate_lock(directory)
    root = Path(__file__).resolve().parents[3]
    runner = root / RUNNER
    preflight = {"lock_sha256": _sha((directory / "lock.json").read_bytes()),
                 "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
                 "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
                 "case_count": len(lock["manifest"]), "validation": "complete preflight passed"}
    (directory / "preflight.json").write_text(_json(preflight) + "\n", encoding="utf-8")
    table_path = H.table_path()
    args = [table_path, directory / "manifest.tsv", directory / "trace.jsonl"]
    linux_args = [_linux(p) for p in args] if os.name == "nt" else [str(p) for p in args]
    runner_arg = _linux(runner) if os.name == "nt" else str(runner)
    command = (["wsl", "-d", "Ubuntu-22.04", "--", runner_arg] + linux_args if os.name == "nt"
               else [str(runner)] + linux_args)
    started = time.perf_counter()
    result = subprocess.run(command + [str(lock["registration"]["stages"])], capture_output=True, text=True,
                            timeout=4 * 3600)
    elapsed = time.perf_counter() - started
    (directory / "execution.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (directory / "execution.stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("GPU hydro campaign runner failed; partial evidence retained: " + result.stderr)
    if validate_lock(directory) != lock or _sha((directory / "lock.json").read_bytes()) != preflight["lock_sha256"]:
        raise ValueError("campaign inputs changed during GPU execution")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith("{")]
    if not devices or devices[0].get("backend") != "cuda_device_kernels":
        raise ValueError("missing real GPU provenance")
    execution = {"schema": "strict-hydro4-execution-1",
                 "preflight_sha256": _sha((directory / "preflight.json").read_bytes()),
                 "postflight": "complete frozen-source/input validation passed", "device": devices[0],
                 "elapsed_seconds": elapsed,
                 "physical_microticks": len(lock["manifest"]) * lock["registration"]["stages"] * STAGE_MICROTICKS,
                 "trace_sha256": _sha((directory / "trace.jsonl").read_bytes())}
    (directory / "execution.json").write_text(_json(execution) + "\n", encoding="utf-8")
    return execution


def _fit_rate(series: np.ndarray, lo: int, hi: int, gamma_weight: float | None = None) -> float:
    """Least-squares slope of ln|series| against n over [lo, hi].

    `gamma_weight=None` is plain (unweighted) OLS. `gamma_weight=g` instead performs
    weighted least squares with weight exp(-2 g n) -- the inverse-variance weight implied
    by white noise `sigma` on an exponential of rate `g` (Var(ln y_n) ~ (sigma/A0)^2
    exp(2 g n)) -- which is the estimator `_se_gamma`'s closed form actually characterizes
    (its denominator, Sum (j-jbar)^2 exp(-2 gamma j), is exactly this WLS estimator's
    variance with the weighted mean approximated by the simple mean `jbar`). Used with
    gamma_weight=predicted_rate for gamma_meas so the acceptance tolerance's `SE_gamma` is
    self-consistent with the estimator it is meant to describe."""
    window = series[lo:hi + 1]
    magnitude = np.abs(window)
    n = np.arange(lo, hi + 1, dtype=float)
    mask = magnitude > 0
    if mask.sum() < 2:
        return float("nan")
    n, y = n[mask], np.log(magnitude[mask])
    if gamma_weight is None:
        slope = np.polyfit(n, y, 1)[0]
    else:
        w = np.exp(-2.0 * gamma_weight * n)
        xw, yw = np.sum(w * n) / np.sum(w), np.sum(w * y) / np.sum(w)
        slope = np.sum(w * (n - xw) * (y - yw)) / np.sum(w * (n - xw) ** 2)
    return -float(slope)


def _fit_angular_rate(series: np.ndarray, lo: int, hi: int) -> float:
    """Least-squares slope of the unwrapped phase of `series` against n over [lo, hi]
    (used only for oscillatory cells: sound speed, Galilean advection)."""
    window = series[lo:hi + 1]
    n = np.arange(lo, hi + 1)
    phase = np.unwrap(np.angle(window))
    return float(np.polyfit(n, phase, 1)[0])


def _cell_group_key(case_dict: dict):
    return (case_dict["arm"], case_dict["mx"], case_dict["my"], case_dict["mz"], case_dict["tx"],
            case_dict["ty"], case_dict["tz"], case_dict["eps"], case_dict["u0"])


def summarize_campaign(directory, write: bool = True) -> dict:
    """Recompute the campaign report from the trace; write=False skips report.json (read-only audits)."""
    directory = Path(directory)
    lock = validate_lock(directory)
    execution = json.loads((directory / "execution.json").read_text())
    preflight_bytes = (directory / "preflight.json").read_bytes()
    preflight = json.loads(preflight_bytes)
    if (execution["preflight_sha256"] != _sha(preflight_bytes)
            or preflight["lock_sha256"] != _sha((directory / "lock.json").read_bytes())):
        raise ValueError("execution/preflight receipt is not tied to current lock")
    for name in ("registration_sha256", "manifest_sha256", "runner_sha256", "instrument_sha256"):
        if preflight[name] != lock[name]:
            raise ValueError("preflight identity differs from accepted lock")
    if preflight["case_count"] != len(lock["manifest"]) or execution["device"].get("backend") != "cuda_device_kernels":
        raise ValueError("execution capability or case inventory mismatch")
    if (execution["postflight"] != "complete frozen-source/input validation passed"
            or execution["trace_sha256"] != _sha((directory / "trace.jsonl").read_bytes())):
        raise ValueError("missing or mismatched executed campaign provenance")
    predictions = json.loads((directory / "predictions.json").read_text())
    reg = lock["registration"]
    stages = reg["stages"]
    table_sha256 = _sha(H.table_path().read_bytes())
    traces: dict[str, np.ndarray] = {}
    with (directory / "trace.jsonl").open(encoding="utf-8") as stream:
        header_line = stream.readline()
        if not header_line:
            raise ValueError("empty trace")
        header = json.loads(header_line)
        if not header.get("header") or header.get("table_sha256") != table_sha256:
            raise ValueError("trace header missing or table hash mismatch")
        for row in lock["manifest"]:
            c = row["case"]
            case = _case_from_dict(c)
            records = []
            for _ in range(stages + 1):
                line = stream.readline()
                if not line:
                    raise ValueError("trace ended before the registered stage count")
                rec = json.loads(line)
                if rec["case_id"] != c["case_id"]:
                    raise ValueError("case order mismatch in trace")
                records.append(rec)
            initial = CODEC.decode((directory / (c["case_id"] + ".bin")).read_bytes())
            measured0 = np.array([complex(a, b) for a, b in records[0]["moments"]])
            if not np.allclose(measured0, observe(initial, case), atol=1e-6):
                raise ValueError("initial GPU observer mismatch")
            mass0, momentum0 = records[0]["mass"], tuple(records[0]["total_momentum"])
            for rec in records:
                if rec["mass"] != mass0 or tuple(rec["total_momentum"]) != momentum0:
                    raise ValueError("mass/momentum conservation violated across stages")
            traces[c["case_id"]] = np.array([[complex(a, b) for a, b in rec["moments"]] for rec in records])
    lo, hi = _window(stages)
    groups: dict = {}
    for row in lock["manifest"]:
        groups.setdefault(_cell_group_key(row["case"]), []).append(row["case"])
    power = power_calculation(reg["L"], stages)
    powered_keys = {tuple(r["cell"]) for r in power["cells"] if r["powered"]}
    results, shear_nu_meas, sound_c_meas, density_rows, galilean_row = [], {}, {}, [], None
    for key, case_dicts in sorted(groups.items()):
        cases_here = [_case_from_dict(c) for c in case_dicts]
        example = cases_here[0]
        modes = np.array([[_project(traces[c.case_id][n], c) for n in range(stages + 1)] for c in cases_here])
        mean = modes.mean(axis=0)
        predicted_series = np.array([_project(np.array([complex(*p) for p in predictions[example.case_id][n]]), example)
                                     for n in range(stages + 1)])
        gamma_pred = predicted_rate(example)
        gamma_meas = _fit_rate(mean, lo, hi, gamma_weight=gamma_pred)
        rate_seeds = np.array([_fit_rate(m, lo, hi) for m in modes])
        se_gamma_seeds = float(np.std(rate_seeds, ddof=1) / np.sqrt(len(cases_here))) if len(cases_here) > 1 else float("nan")
        amplitude = _amplitude(example)
        rel_rms = float(np.sqrt(np.sum(np.abs(mean - predicted_series) ** 2) / np.sum(np.abs(predicted_series) ** 2))) \
            if np.sum(np.abs(predicted_series) ** 2) > 0 else float("nan")
        sigma = noise_floor(example)
        this_noise_ratio = sigma / amplitude if amplitude > 0 else float("inf")
        powered = key in powered_keys
        rate_tol = max(ACCEPTANCE["rate_sigma"] * se_gamma_seeds, float(ACCEPTANCE["rate_relative"]) * abs(gamma_pred)) \
            if not np.isnan(se_gamma_seeds) else float(ACCEPTANCE["rate_relative"]) * abs(gamma_pred)
        rms_tol = max(float(ACCEPTANCE["rms_relative"]), float(ACCEPTANCE["noise_multiple"]) * this_noise_ratio)
        rate_ok = bool(abs(gamma_meas - gamma_pred) <= rate_tol)
        rms_ok = bool(rel_rms <= rms_tol)
        row = {"cell": list(key), "arm": example.arm, "seeds": len(cases_here), "powered": powered,
               "predicted_rate": gamma_pred, "measured_rate": gamma_meas, "rate_standard_error": se_gamma_seeds,
               "rate_tolerance": rate_tol, "relative_rms": rel_rms, "rms_tolerance": rms_tol,
               "rate_pass": rate_ok, "rms_pass": rms_ok,
               "mean_trajectory": [[float(v.real), float(v.imag)] for v in mean],
               "k": [example.mx, example.my, example.mz]}
        if example.arm == "shear":
            k2 = _k_squared(example)
            nu_meas = gamma_meas / k2 if k2 else float("nan")
            direction = _unit_direction(example)
            polarization = (int(example.tx), int(example.ty), int(example.tz))
            nu_exact = float(_shear_constant(direction, polarization))
            row["nu_measured"] = nu_meas
            row["nu_exact"] = nu_exact
            shear_nu_meas[(direction, polarization)] = nu_meas
        if example.arm == "sound":
            omega_meas = _fit_angular_rate(mean, lo, hi)
            k_norm = float(np.sqrt(_k_squared(example)))
            c_s_meas = omega_meas / k_norm if k_norm else float("nan")
            row["omega_measured"] = omega_meas
            row["c_s_measured"] = c_s_meas
            row["c_s_exact"] = float(np.sqrt(float(Fraction(_verdict()["sound_speed_squared"]))))
            sound_c_meas[_unit_direction(example)] = c_s_meas
        if example.arm == "density":
            row["direction"] = list(_unit_direction(example))
            density_rows.append(row)
        if example.arm == "galilean":
            omega_meas = _fit_angular_rate(mean, lo, hi)
            k_norm = float(np.sqrt(_k_squared(example)))
            u0 = float(example.u0)
            row["omega_measured"] = omega_meas
            row["advection_ratio_measured"] = omega_meas / (k_norm * u0) if k_norm and u0 else float("nan")
            row["g_exact"] = float(B.nonlinear_coefficients(DENSITY)["g"])
            galilean_row = row
        results.append(row)
    nu_T2 = shear_nu_meas.get(((1, 0, 0), (0, 1, 0)))
    nu_E = shear_nu_meas.get(((1, 1, 0), (1, -1, 0)))
    nu_111 = shear_nu_meas.get(((1, 1, 1), (1, -1, 0)))
    ratio_meas = (nu_E / nu_T2) if (nu_T2 is not None and nu_E is not None and nu_T2) else None
    cubic_exact = _verdict()["cubic_shear_constants"]
    ratio_exact = float(Fraction(cubic_exact["nu_E"]) / Fraction(cubic_exact["nu_T2"])) if cubic_exact else None
    cubic_pred_from_meas = ((2 * nu_E + nu_T2) / 3) if (nu_T2 is not None and nu_E is not None) else None
    powered_results = [r for r in results if r["powered"]]
    passes = sum(1 for r in powered_results if r["rate_pass"] and r["rms_pass"])
    output = {"schema": "strict-hydro4-report-1", "protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID,
              "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
              "predictions_sha256": lock["predictions_sha256"], "trace_sha256": execution["trace_sha256"],
              "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
              "execution": execution, "registration": reg, "power": power,
              "groups": len(results), "powered_groups": len(powered_results),
              "powered_groups_passing": passes,
              "verdict_at_registered_scope": (passes == len(powered_results)) if powered_results else None,
              "anisotropy_ratio_measured": ratio_meas, "anisotropy_ratio_exact": ratio_exact,
              "cubic_identity_from_measured_constants": cubic_pred_from_meas, "nu_111_measured": nu_111,
              "density_rows": density_rows, "galilean_row": galilean_row, "results": results}
    if write:
        (directory / "report.json").write_text(_json(output) + "\n", encoding="utf-8")
    return output
