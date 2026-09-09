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
  - `predicted_rate` (fix round, task-11-fix1-report.md) returns the decay rate per stage
    of the mode `predict()` actually evolves at the registered (finite) wavevector: the
    same weighted log-linear fit `gamma_meas` uses, run on the noiseless
    `predict(case, stages)` trajectory over the registered fit window -- bias-matched to
    the acceptance estimator, so residual multi-mode/window contamination shows up
    identically on both sides of the acceptance comparison. This replaces the original
    design (the EXACT small-k H1' constant times |k|^2), which task-11-review.md found
    disagreed with the numeric trajectory `predict()` evolves by 5.7-27% at the registered
    k -- exceeding the acceptance tolerance for several cells even with zero measurement
    noise. `predicted_rate_eigenvalue` reads the same quantity directly off the dominant
    eigenvalue of `numeric_stage_map` (the eigenvector with the largest overlap against the
    cell's projected observable) as a non-circular cross-check; `horizon` uses it (not
    `predicted_rate`) to size the campaign, since `predicted_rate` itself needs a stage
    count/fit window to resolve. The exact (Fraction) small-k H1' constants
    (`nu_T2`/`nu_E`/cubic mean / `longitudinal_damping_normalized`) are kept as
    `nu_exact_limit`, a REPORTED reference value (the k -> 0 limit), never used for
    acceptance; `k4_systematic` reports the O(k^4) relative gap between the two.
  - The Taylor-Green preparation is a genuine two-wavevector superposition
    (sin(a)cos(b) = 1/2[sin(a+b)+sin(a-b)]); this module tracks only its k=(1,1,0)*2pi/L,
    polarization (1,-1,0) component (the other, k=(1,-1,0)/(1,1,0), is related by the same
    cubic symmetry and is not separately registered). This is a diagnostic/coverage case,
    not part of the brief's registered acceptance rows.
  - `POWER_THRESHOLD` (fix round 2, task-11-fix2-report.md) tightens `powered` from
    `power >= 5` to `power >= 60` (== `ACCEPTANCE["rate_sigma"]/ACCEPTANCE["rate_relative"]`,
    derived, not a bare literal). task-11-rereview1.md found the old threshold degenerate:
    near `power = 5`, the analytic-SE acceptance term (`rate_sigma*SE_gamma_analytic`) could
    reach ~60% of `gamma_pred_numeric`, swallowing the registered 5% relative floor, so a
    genuine 10% rate error passed acceptance in 11/18 powered L=32 cells. `power >= 60`
    guarantees the opposite ordering in every powered cell by construction. `SEEDS` is
    raised 8 -> 32 alongside (both `SE_gamma_analytic` and the empirical `SE_gamma_seeds`
    scale as `1/sqrt(SEEDS)`) to recover cells the tightened threshold would otherwise drop.
    `horizon`'s stage-count budget cap is raised from 2 to 4 hours (`HORIZON_BUDGET_SECONDS`,
    shared with `run_campaign`'s subprocess timeout) since it binds at L=48 once SEEDS=32
    quadruples the per-stage case cost; the "three e-folds" floor itself is kept (raising it
    does not help -- see `horizon`'s docstring for the investigation).
"""
from __future__ import annotations

from dataclasses import asdict, dataclass
from datetime import datetime, timezone
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
# task-11-fix2 (controller ruling, 2026-09-08): raised 8 -> 32. SE_gamma_analytic (and the
# empirical per-seed SE_gamma_seeds) both scale as 1/sqrt(SEEDS); quadrupling SEEDS halves
# both, recovering cells the tightened POWER_THRESHOLD below would otherwise drop entirely.
SEEDS = 32
EPSILONS = (Fraction(1, 20), Fraction(1, 10), Fraction(1, 5))
WAVENUMBERS = (1, 2)
U0 = Fraction(1, 10)
ACCEPTANCE = {"rate_sigma": 3, "rate_relative": Fraction(1, 20), "rms_relative": Fraction(1, 10),
              "noise_multiple": 3}
# task-11-fix2 (controller ruling, 2026-09-08): a cell is powered only when the analytic-SE
# acceptance term can never be the loosening term relative to the registered 5% relative
# floor -- i.e. rate_sigma*SE_gamma_analytic <= rate_relative*|gamma_pred_numeric| -- which
# is exactly power = gamma_pred_numeric/SE_gamma_analytic >= rate_sigma/rate_relative = 60.
# Replaces the prior `power >= 5` criterion: task-11-rereview1.md found it degenerate (the
# analytic-SE term reached up to ~60% of gamma_pred_numeric -- 12x the intended 5% floor --
# for 11/18 powered L=32 cells, silently accepting a 10% rate error). Derived from
# ACCEPTANCE, not a bare literal, so it cannot drift from the acceptance rule it enforces.
POWER_THRESHOLD = float(ACCEPTANCE["rate_sigma"]) / float(ACCEPTANCE["rate_relative"])
RUNNER = "engine/build_strict_hydro_cuda/ftd_hydro_campaign"
STAGE_MICROTICKS = 4
# task-11-fix2: 4 hours of GPU time, `horizon`'s stage-count PLANNING cap only (task-12
# pre-lock robustness edit, controller ruling 2026-09-08: this budget no longer also gates
# `run_campaign`'s subprocess timeout -- see RUN_TIMEOUT_MULTIPLIER/run_timeout_seconds
# below. Sharing one constant between "how many stages can we afford" and "how long may the
# GPU process run before we kill it" meant any real-world throughput regression relative to
# the probe (thermal throttling, a different device, WSL2 scheduling noise) would race the
# in-progress run against the same number that sized its own stage count, leaving zero
# slack. The two concerns are now independent: this constant still sizes `horizon`.
HORIZON_BUDGET_SECONDS = 4 * 3600.0
# task-12 pre-lock robustness edit: `run_campaign`'s subprocess timeout is three times the
# REGISTERED wall-time ESTIMATE (`registration["estimated_wall_seconds"]`), not the horizon
# budget cap -- at the locked L=48 estimate (~3.955h) this is ~11.9h, matching the brief's
# "roughly 12h" figure, with ample slack over the single-probe-run estimate for ordinary
# run-to-run throughput variance. RUN_TIMEOUT_MINIMUM_SECONDS floors the timeout for
# tiny/smoke-scale registrations (e.g. the one-case, 8-stage L=8 instrument smoke test),
# where the compute estimate is a fraction of a second but WSL2 dispatch + CUDA context
# startup is not -- the floor never binds at any real registered L (32 or 48: 3x the
# estimate is already tens of thousands of seconds), so it changes nothing about the
# registered campaign's actual timeout.
RUN_TIMEOUT_MULTIPLIER = 3.0
RUN_TIMEOUT_MINIMUM_SECONDS = 300.0


def run_timeout_seconds(registration: dict) -> float:
    """`run_campaign`'s subprocess timeout: `RUN_TIMEOUT_MULTIPLIER` (3) times the
    registered wall-time estimate (`registration["estimated_wall_seconds"]`), floored at
    `RUN_TIMEOUT_MINIMUM_SECONDS` so a tiny/smoke-scale registration (whose compute estimate
    is dwarfed by fixed WSL2/CUDA dispatch overhead) is not killed before it can finish, and
    falling back to `RUN_TIMEOUT_MULTIPLIER * HORIZON_BUDGET_SECONDS` if no throughput
    evidence was on disk at registration time (`estimated_wall_seconds is None`). Always
    satisfies `run_timeout_seconds(reg) >= RUN_TIMEOUT_MULTIPLIER * reg["estimated_wall_seconds"]`
    when that estimate exists, by construction (the floor can only raise the value)."""
    estimated = registration.get("estimated_wall_seconds")
    base = estimated if estimated is not None else HORIZON_BUDGET_SECONDS
    return max(RUN_TIMEOUT_MULTIPLIER * base, RUN_TIMEOUT_MINIMUM_SECONDS)

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
# Canonical registered representative cell per named shear quantity (task-12-fix1, controller
# ruling): the top-level anisotropy_ratio_measured/cubic_identity_from_measured_constants/
# nu_111_measured fields are derived from exactly these three, at m=1, eps=1/10 -- never from
# whichever (m, eps) a dict happened to collapse onto (see summarize_campaign's
# task-12-report.md reporting-code caveat).
_SHEAR_T2_CELL = ((1, 0, 0), (0, 1, 0))
_SHEAR_T2_ALT_CELL = ((1, 1, 0), (0, 0, 1))
_SHEAR_E_CELL = ((1, 1, 0), (1, -1, 0))
_SHEAR_CUBIC_CELL = ((1, 1, 1), (1, -1, 0))
_SHEAR_CANONICAL_M = 1
_SHEAR_CANONICAL_EPS = str(Fraction(1, 10))


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


def _exact_rate_constant(case: HydroCase) -> Fraction:
    """The exact (Fraction) small-k H1' constant this cell's decay rate approaches as
    k -> 0: nu_T2 / nu_E / the cubic mean for shear-type cells, the exact
    `longitudinal_damping_normalized` constant for sound/density cells. Reported as
    `nu_exact_limit` -- a REFERENCE value, never used for acceptance (see `predicted_rate`
    and the module docstring's task-11-fix1 note)."""
    direction = _unit_direction(case)
    if case.arm in ("shear", "galilean", "taylor_green"):
        return _shear_constant(direction, (int(case.tx), int(case.ty), int(case.tz)))
    if case.arm in ("sound", "density"):
        return _longitudinal_damping(direction)
    raise ValueError(f"no registered exact rate constant for arm {case.arm}")


def _eigen_mode(case: HydroCase) -> tuple[complex, np.ndarray]:
    """Eigenpair of the numeric per-stage map P(k) = numeric_stage_map(k, occupancy) whose
    eigenvector has the largest overlap with this cell's projected physical observable
    (transverse momentum along t-hat for shear/galilean/taylor_green, the longitudinal
    momentum pair for sound, the mass channel for density -- `_project` already
    discriminates these by `case.arm`). This is the direct, non-circular cross-check named
    in task-11-review.md's fix instructions; the acceptance-relevant `predicted_rate`
    instead fits the noiseless prediction trajectory (see its docstring for why the two
    usually agree closely but are not defined to be identical)."""
    k = 2.0 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / case.L
    Pk = _cached_stage_map(k, _occupancy(case))
    W = np.array(B.weights_rows(), dtype=float)
    eigvals, eigvecs = np.linalg.eig(Pk)
    overlaps = np.array([abs(_project(W @ eigvecs[:, i], case)) for i in range(eigvecs.shape[1])])
    idx = int(np.argmax(overlaps))
    return complex(eigvals[idx]), eigvecs[:, idx]


def predicted_rate_eigenvalue(case: HydroCase) -> tuple[float, float]:
    """Cross-check decay rate and oscillation frequency per stage, read directly off the
    dominant eigenvalue located by `_eigen_mode` -- NOT the fit-on-prediction acceptance
    estimator `predicted_rate`. Reported in the registration/power table
    (`predicted_rate_eigenvalue`/`predicted_frequency_eigenvalue`) alongside
    `gamma_pred_numeric` so a reviewer can see the two independent extractions agree; for
    sound cells the predicted oscillation frequency reported in `summarize_campaign` comes
    from this function's frequency, per task-11-review.md's fix instructions."""
    eigval, _ = _eigen_mode(case)
    magnitude = abs(eigval)
    rate = -float(np.log(magnitude)) if magnitude > 0 else float("inf")
    frequency = float(np.angle(eigval))
    return rate, frequency


_PREDICTED_RATE_FIXED_POINT_ITERS = 100
_PREDICTED_RATE_FIXED_POINT_RTOL = 1e-9


def predicted_rate(case: HydroCase, stages: int | None = None) -> float:
    """Decay rate per stage of the mode `predict()` actually evolves at the registered
    (finite) wavevector: the same weighted log-linear fit `gamma_meas` uses (see
    `_fit_rate`), run on the noiseless `predict(case, stages)` trajectory over the
    registered fit window (`_window(stages)`) -- bias-matched to the acceptance estimator,
    so residual multi-mode/window contamination present in a real fit shows up identically
    on both sides of the acceptance comparison, leaving only genuine measurement
    disagreement as the residual.

    The weight is resolved by FIXED-POINT iteration, not a single pass: `gamma_meas` in
    `summarize_campaign` is `_fit_rate(mean, lo, hi, gamma_weight=gamma_pred)`, i.e. it
    weights by whatever this function returns, so for the noiseless case (`mean ==
    projected`) to reproduce `gamma_pred` exactly, `gamma_pred` must be a fixed point of
    `g -> _fit_rate(projected, lo, hi, gamma_weight=g)`. Seeding the iteration at the cheap
    `predicted_rate_eigenvalue` cross-check and repeating `_fit_rate` until it stops moving
    finds that point (empirically an alternating, geometrically-shrinking sequence -- a
    handful of iterations suffice for well-behaved shear cells where the eigenvalue guess
    is already close, but sound/density cells at the registered k showed a genuinely
    weight-SENSITIVE fit (eigenvalue and single-shot-fit rates differing by ~2x, because
    several comparably-sized modes, not one dominant mode, contribute over the fit window)
    -- a single-shot fit at the eigenvalue guess is then NOT a fixed point, and re-fitting
    with `gamma_weight=gamma_pred` (as `summarize_campaign` does) would move again,
    spuriously failing acceptance on a hypothetical PERFECT measurement exactly like the
    blocking defect this function replaces, just relocated from the weight-choice instead
    of the exact-vs-numeric choice. Iterating to convergence removes that residual
    circularity for every arm, not only the arms where the single-shot guess happened to
    already be close. `stages` defaults to `horizon(case.L)` for standalone callers;
    `power_calculation`/`summarize_campaign` pass the already-resolved registered stage
    count explicitly so this never re-enters `horizon` while `horizon` is itself resolving
    that count (`horizon` uses `predicted_rate_eigenvalue`, not this function, for exactly
    that reason)."""
    if stages is None:
        stages = horizon(case.L)
    lo, hi = _window(stages)
    trajectory = predict(case, stages)
    projected = np.array([_project(row, case) for row in trajectory])
    gamma, _ = predicted_rate_eigenvalue(case)
    for _ in range(_PREDICTED_RATE_FIXED_POINT_ITERS):
        next_gamma = _fit_rate(projected, lo, hi, gamma_weight=gamma)
        if not np.isfinite(next_gamma):
            return next_gamma
        if abs(next_gamma - gamma) <= _PREDICTED_RATE_FIXED_POINT_RTOL * max(abs(gamma), 1e-300):
            return float(next_gamma)
        gamma = next_gamma
    return float(gamma)


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
    """Per-cell power/noise/budget table. `gamma` (== `gamma_pred_numeric`) is the
    fit-on-prediction `predicted_rate` at the REGISTERED `stages`/window (task-11-fix1: the
    powered inventory is recomputed against the numeric prediction, not the exact small-k
    Taylor constant); `predicted_rate_eigenvalue`/`predicted_frequency_eigenvalue` are the
    non-circular eigenvalue cross-check. Cells with a registered exact H1' constant also
    report `nu_exact_limit` (the k -> 0 limit) and `k4_systematic`, the relative gap between
    the numeric prediction and that limit.

    `powered` (task-11-fix2) requires `power = gamma/SE_gamma_analytic >= POWER_THRESHOLD`
    (== 60), equivalently `rate_sigma*SE_gamma_analytic <= rate_relative*|gamma|` -- so in
    every powered cell the analytic-SE acceptance term is, by construction, never looser
    than the registered 5% relative floor. This replaces the prior `power >= 5` criterion:
    at `power` near 5, `rate_sigma*SE_gamma_analytic` could reach ~60% of `gamma` (12x the
    intended floor), which task-11-rereview1.md showed let a genuine 10% rate error pass
    acceptance in 11/18 powered L=32 cells."""
    lo, hi = _window(stages)
    rows = []
    for key, case in sorted(_cells(L).items()):
        gamma = predicted_rate(case, stages)
        eig_rate, eig_frequency = predicted_rate_eigenvalue(case)
        amplitude = _amplitude(case)
        sigma = noise_floor(case)
        se_gamma = _se_gamma(gamma, sigma, amplitude, lo, hi)
        power = gamma / se_gamma if se_gamma > 0 else 0.0
        noise_ratio = sigma / amplitude if amplitude > 0 else float("inf")
        budget_ratio = (float(case.eps) / float(R.C2)) ** 2 / float(ACCEPTANCE["rate_relative"])
        powered = bool(power >= POWER_THRESHOLD and noise_ratio <= Fraction(1, 3) and budget_ratio <= Fraction(1, 3))
        row = {"cell": list(key), "case_id_representative": case.case_id,
               "predicted_rate": gamma, "gamma_pred_numeric": gamma,
               "predicted_rate_eigenvalue": eig_rate, "predicted_frequency_eigenvalue": eig_frequency,
               "amplitude": amplitude, "noise_floor": sigma, "se_gamma": se_gamma, "power": power,
               "noise_ratio": noise_ratio, "budget_ratio": budget_ratio, "powered": powered}
        try:
            nu_exact_limit = float(_exact_rate_constant(case))
        except ValueError:
            nu_exact_limit = None
        if nu_exact_limit is not None:
            k2 = _k_squared(case)
            row["nu_exact_limit"] = nu_exact_limit
            row["k4_systematic"] = ((gamma - nu_exact_limit * k2) / gamma) if gamma else float("nan")
        rows.append(row)
    return {"L": L, "stages": stages, "window": [lo, hi], "power_threshold": POWER_THRESHOLD, "cells": rows,
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


def horizon(L: int, budget_seconds: float = HORIZON_BUDGET_SECONDS) -> int:
    """Three e-folds of the slowest m=1 shear prediction, rounded up to a multiple of 8,
    capped by the probe's measured throughput within `budget_seconds` (uncapped, just
    rounded, when no throughput evidence is on disk yet). Sized off
    `predicted_rate_eigenvalue` (not the fit-on-prediction `predicted_rate`): the latter
    needs a stage count / fit window to compute a rate from, which is exactly the quantity
    this function solves for -- the eigenvalue cross-check is a close, non-circular
    stand-in for sizing purposes only (see `predicted_rate`'s docstring).

    `budget_seconds` (task-11-fix2): the declared cap is `HORIZON_BUDGET_SECONDS`, 4 hours of
    GPU time at the probe rate -- the SAME constant `run_campaign` passes as its own
    subprocess timeout, so `horizon` never registers a stage count the runner itself would
    be killed before completing. At L=32 this cap does not bind (three e-folds, 112 stages,
    fits well inside 4h even at SEEDS=32's 1120-case inventory); at L=48 it does bind (three
    e-folds would need 240 stages, but only 80 fit the 4h budget at 1120 cases) -- the
    binding case is exactly why the cap must be declared, not left at the old 2-hour (7200s)
    default, which would have capped L=48 at 40 stages instead.

    The "three e-folds of the slowest shear rate" floor is kept as-is, not raised, even
    though POWER_THRESHOLD is far stricter now: sweeping the e-fold multiplier from 3 to 16
    (task-11-fix2 investigation) does not monotonically help, because `_window(stages)`
    sizes the fit window as a FIXED FRACTION of `stages` ([stages/4, 3*stages/4]), not an
    absolute range -- lengthening `stages` beyond what the slowest mode needs pushes faster
    modes' fit windows further into their own decay, where their amplitude (and thus their
    SE_gamma_analytic) is worse, not better. At L=32 the powered count is highest at exactly
    the 3-e-fold floor (7 cells) and falls to 4-5 for every larger multiplier tried; raising
    the floor would only shrink the registered inventory. Recovering more powered cells
    instead comes from raising SEEDS (which this fix also does) and, at L=48, from the
    budget cap itself binding above the 3-e-fold floor's own effect."""
    slow = min(predicted_rate_eigenvalue(case)[0] for case in cases(L)
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


def _estimated_wall_seconds(L: int, stages: int, n_cases: int) -> float | None:
    """Estimated GPU wall-clock time for the full registered campaign at `L`: n_cases *
    stages * STAGE_MICROTICKS physical microticks, at the probe's own measured throughput
    for that `L` (`engine/docs/evidence/strict-hydro4-throughput.json`; 91.9 microticks/s at
    L=32, 25.17 at L=48). None when no throughput evidence is on disk yet."""
    rate = _throughput_microticks_per_second(L)
    if rate is None:
        return None
    return n_cases * stages * STAGE_MICROTICKS / rate


def registration(L: int) -> dict:
    stages = horizon(L)
    n_cases = len(cases(L))
    wall_seconds = _estimated_wall_seconds(L, stages, n_cases)
    return {"protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID, "density": str(DENSITY),
            "epsilons": [str(e) for e in EPSILONS], "wavenumbers": list(WAVENUMBERS), "u0": str(U0),
            "seeds": SEEDS, "L": L, "stages": stages, "microticks_per_stage": STAGE_MICROTICKS,
            "acceptance": {k: str(v) for k, v in ACCEPTANCE.items()},
            "power_threshold": POWER_THRESHOLD,
            "powered_criterion": ("power = gamma_pred_numeric/SE_gamma_analytic >= power_threshold "
                                   "(== rate_sigma/rate_relative == 60), AND noise_ratio <= 1/3, AND "
                                   "budget_ratio <= 1/3 (task-11-fix2, controller ruling 2026-09-08; "
                                   "replaces the prior power>=5 criterion task-11-rereview1.md found "
                                   "degenerate for 11/18 powered L=32 cells). By construction, in every "
                                   "powered cell rate_sigma*SE_gamma_analytic <= rate_relative*"
                                   "|gamma_pred_numeric| -- the analytic-SE acceptance term is never the "
                                   "binding (loosening) one relative to the registered 5% relative floor"),
            "rate_tolerance_rule": ("max(rate_sigma*SE_gamma_seeds[weighted], "
                                     "rate_sigma*SE_gamma_analytic[power_calculation], "
                                     "rate_relative*|gamma_pred_numeric|); SE_gamma_seeds now uses the "
                                     "same weighted (gamma_weight=gamma_pred_numeric) per-seed fit as "
                                     "gamma_meas itself (task-11-fix1 consistency note), and the analytic "
                                     "SE from power_calculation is kept as a registered floor. In every "
                                     "powered cell (task-11-fix2) the second term is <= the third by "
                                     "construction of power_threshold -- see powered_criterion"),
            "shear_cells": [[list(d), list(p), kind] for d, p, kind in SHEAR_CELLS],
            "case_count": n_cases, "table_hash": H.TABLE_HASH,
            "estimated_wall_seconds": wall_seconds,
            "estimated_wall_seconds_rule": ("case_count * stages * microticks_per_stage / "
                                             "throughput_microticks_per_second(L), from "
                                             "engine/docs/evidence/strict-hydro4-throughput.json; "
                                             "None when no throughput evidence is on disk"),
            "horizon_budget_seconds": HORIZON_BUDGET_SECONDS,
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


def _resolve_amended_hash(lock: dict, relative_path: str, locked_hash: str) -> tuple[str, list[dict]]:
    """Follow the `lock['amendments']` chain (if any) recorded for `relative_path`,
    starting from `locked_hash` (the ORIGINAL `instrument_sha256` value recorded at
    `prepare_campaign` time -- never overwritten by an amendment). Returns
    `(expected_current_hash, chain_applied)`; `expected_current_hash == locked_hash` and
    `chain_applied == []` when this path carries no amendment. Raises ValueError if an
    amendment's own `hash_at_lock` does not match where the chain so far has arrived (a
    malformed or out-of-order amendments list is treated as untrusted, not silently
    accepted)."""
    expected = locked_hash
    chain = []
    for amendment in lock.get("amendments", []):
        if amendment.get("file") != relative_path:
            continue
        if amendment.get("hash_at_lock") != expected:
            raise ValueError(f"amendment chain broken for {relative_path}")
        expected = amendment["hash_after"]
        chain.append(amendment)
    return expected, chain


def _record_amendment(directory: Path, relative_path: str, reason: str) -> dict:
    """Bring `relative_path`'s locked hash forward to its CURRENT on-disk hash by
    appending one `{file, hash_at_lock, hash_after, reason, timestamp}` entry to
    `lock.json`'s `amendments` list (creating the list if absent), chaining from the
    latest hash already recorded for this path (the original `instrument_sha256` value if
    this is the path's first amendment). Rewrites `lock.json` byte-exact (LF only, no
    Windows text-mode CRLF translation, matching the rest of this module's writes) and
    returns the updated lock dict. A no-op (lock.json untouched, returned unchanged) if
    the current hash already equals the latest recorded hash for this path -- so calling
    this twice in a row (e.g. a re-run resume attempt) never appends a duplicate entry.
    Does NOT touch `instrument_sha256` itself -- that dict remains the original
    registration witness; `validate_lock`/`_resolve_amended_hash` bridge the gap."""
    directory = Path(directory)
    root = Path(__file__).resolve().parents[3]
    lock = json.loads((directory / "lock.json").read_bytes())
    if relative_path not in lock["instrument_sha256"]:
        raise ValueError(f"{relative_path} is not a locked instrument source; cannot amend")
    current = _hash_instrument_path(root, relative_path)
    latest, _ = _resolve_amended_hash(lock, relative_path, lock["instrument_sha256"][relative_path])
    if current == latest:
        return lock
    amendment = {"file": relative_path, "hash_at_lock": latest, "hash_after": current,
                 "reason": reason, "timestamp": datetime.now(timezone.utc).isoformat()}
    lock = dict(lock)
    lock["amendments"] = list(lock.get("amendments", [])) + [amendment]
    (directory / "lock.json").write_bytes((_json(lock) + "\n").encode("utf-8"))
    return lock


def validate_lock(directory) -> dict:
    directory = Path(directory)
    lock = json.loads((directory / "lock.json").read_text())
    root = Path(__file__).resolve().parents[3]
    if set(lock["instrument_sha256"]) != instrument_paths():
        raise ValueError("incomplete instrument source closure")
    amendments_verified = []
    for p, locked_hash in lock["instrument_sha256"].items():
        expected, chain = _resolve_amended_hash(lock, p, locked_hash)
        if _hash_instrument_path(root, p) != expected:
            raise ValueError("instrument source changed after registration lock")
        amendments_verified.extend(chain)
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
    if amendments_verified:
        # Report the amendment(s) actually exercised to bridge a hash mismatch in THIS
        # call, without mutating the on-disk lock.json or the returned dict's other keys
        # (run_campaign's own `validate_lock(directory) != lock` re-check stays valid: two
        # consecutive calls against unchanged files produce the identical extra key).
        lock = dict(lock)
        lock["amendments_verified"] = amendments_verified
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
    timeout_seconds = run_timeout_seconds(lock["registration"])
    started = time.perf_counter()
    result = subprocess.run(command + [str(lock["registration"]["stages"])], capture_output=True, text=True,
                            timeout=timeout_seconds)
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


def _split_lines(data: bytes) -> list[bytes]:
    """Split raw trace bytes on '\\n' without any text-mode newline translation (the
    instrument, run through WSL2 Linux, always writes bare '\\n'; Windows text-mode
    read/write would otherwise round-trip '\\n' through '\\r\\n' -- the exact bug
    task-13-fix1 found and fixed for the lab's manifest sidecars). A trailing empty
    element from a final '\\n' is dropped; a truncated final write (no trailing '\\n',
    e.g. a kill mid-line) is kept as the last element so callers can detect and discard
    it."""
    lines = data.split(b"\n")
    if lines and lines[-1] == b"":
        lines.pop()
    return lines


def _parse_trace_lines(lines: list[bytes]):
    """Parse a list of raw trace lines (as `_split_lines` returns): the first line must be
    the header record; every subsequent non-empty line is attempted as one stage record.
    A line that fails to decode as UTF-8 JSON (a write truncated mid-record by an external
    kill) is dropped, not attributed to any case, and counted in `undecodable` -- exactly
    the fate the resume brief requires for the cut line.

    Returns `(header_bytes, header_dict, per_case, undecodable)`, where `per_case` maps
    case_id -> the list of `(stage:int, raw_line:bytes)` pairs found, IN FILE ORDER
    (duplicates or out-of-order stage numbers are not resolved here -- callers that need a
    complete, correctly-ordered block validate that themselves)."""
    if not lines:
        raise ValueError("empty trace")
    header_bytes = lines[0]
    header = json.loads(header_bytes.decode("utf-8"))
    if not header.get("header"):
        raise ValueError("first line of trace is not a header record")
    per_case: dict[str, list[tuple[int, bytes]]] = {}
    undecodable = 0
    for raw in lines[1:]:
        if not raw:
            continue
        try:
            rec = json.loads(raw.decode("utf-8"))
        except (json.JSONDecodeError, UnicodeDecodeError):
            undecodable += 1
            continue
        per_case.setdefault(rec["case_id"], []).append((int(rec["stage"]), raw))
    return header_bytes, header, per_case, undecodable


def _complete_case_block(records: list[tuple[int, bytes]], stages: int) -> list[bytes] | None:
    """`records` (as `_parse_trace_lines` groups them) is a COMPLETE, correctly-ordered
    block for one case iff it has exactly `stages + 1` entries whose stage numbers are
    EXACTLY `{0, ..., stages}` and already appear in ascending file order (the order the
    instrument's own `for (s = 0; s <= stages; ++s)` loop writes them in -- summarize_campaign
    trusts file order as stage order, so this function re-verifies that trust rather than
    silently re-sorting a block that arrived out of order). Returns the raw line list
    (ready to concatenate into the final trace) on success, `None` otherwise."""
    stage_numbers = [s for s, _ in records]
    if len(records) != stages + 1 or set(stage_numbers) != set(range(stages + 1)) \
            or stage_numbers != sorted(stage_numbers):
        return None
    return [raw for _, raw in records]


def resume_campaign(directory) -> dict:
    """Resume a campaign interrupted mid-run (host restart, power loss, etc.), honestly:
    keep every trace record of every case whose stage sequence (0..stages) is complete in
    `trace.jsonl.part`; discard the records of any incomplete case (the one case cut
    mid-write, plus its truncated/undecodable last line) and re-run the instrument ONLY on
    that case plus every case the interrupted run never reached (both in the original
    `manifest.tsv` order); then assemble `trace.jsonl` as the original header followed by
    each manifest case's block, IN MANIFEST ORDER, drawn from whichever source (kept or
    resumed) has it -- `summarize_campaign` requires each case's `stages + 1`-record block
    to be contiguous and in that exact order, so this function sorts the two sources' blocks
    into it rather than concatenating kept-then-resumed.

    No registered logic changes: same manifest.tsv (only re-run on a FILTERED subset of its
    rows, `manifest_resume.tsv`), same table, same stage count, same instrument binary,
    same acceptance/prediction code (untouched by this function). Every case's tick is an
    independent, deterministic function of its own frozen, hash-verified preparation with
    no cross-case coupling, so keeping one run's records for some cases and another run's
    records for the rest reproduces exactly what a single uninterrupted run would have
    written, case by case -- concatenation is exact, not an approximation.

    Adding this function changes `campaign.py`'s own hash, which the lock recorded via
    `instrument_paths()`; this function begins by recording that as a declared lock
    amendment (`_record_amendment`, idempotent) before calling `validate_lock` -- see the
    module's amendment machinery above. It then writes a fresh `preflight.json` (mirroring
    `run_campaign`'s own step) tied to the now-amended `lock.json`, and a final
    `execution.json` receipt in `run_campaign`'s own shape (`schema`, `preflight_sha256`,
    `postflight`, `device`, `elapsed_seconds`, `physical_microticks`, `trace_sha256` for the
    COMPLETE 1120-case campaign) plus an `interruption` section documenting exactly what
    happened, so the evidence itself declares the resume rather than looking like an
    ordinary uninterrupted run.

    Refuses to run if `trace.jsonl` already exists (nothing to resume) or if
    `trace.jsonl.part` is missing (nothing to resume from). Never deletes
    `trace.jsonl.part` or `trace_resume.jsonl`; refuses to overwrite a pre-existing
    `trace_resume.jsonl` from a prior resume attempt rather than silently clobbering it."""
    directory = Path(directory).resolve()
    root = Path(__file__).resolve().parents[3]
    trace_path = directory / "trace.jsonl"
    part_path = directory / "trace.jsonl.part"
    if trace_path.exists():
        raise ValueError("trace.jsonl already exists; nothing to resume")
    if not part_path.is_file():
        raise ValueError("no trace.jsonl.part found; nothing to resume from")

    # Original-start provenance, captured BEFORE this function overwrites preflight.json
    # below (its mtime is the run_campaign-written marker closest to "the GPU subprocess
    # was about to be launched"; task-12-brief.md names it as the primary source).
    preflight_path = directory / "preflight.json"
    if preflight_path.is_file():
        original_start = preflight_path.stat().st_mtime
        original_start_source = ("preflight.json mtime -- written by the original run_campaign "
                                  "immediately before it invoked the GPU subprocess")
        original_preflight_bytes = preflight_path.read_bytes()
    else:
        original_start = part_path.stat().st_ctime
        original_start_source = ("trace.jsonl.part filesystem creation time (Windows st_ctime; "
                                  "preflight.json was missing)")
        original_preflight_bytes = None
    kill_time = part_path.stat().st_mtime

    # Record the amendment (idempotent) and validate against the NOW-amended lock.json.
    _record_amendment(directory, "scripts/phi_v2_lattice/hydro/campaign.py",
                       "resume machinery added after the OS-restart interruption; registered "
                       "logic unchanged -- see the audit's diff")
    lock = validate_lock(directory)
    stages = lock["registration"]["stages"]
    table_sha256 = _sha(H.table_path().read_bytes())

    # Fresh preflight.json tied to the NOW-amended lock.json (mirrors run_campaign's own
    # step, written before the GPU subprocess runs so the post-subprocess re-check below
    # can detect any tampering with lock.json during execution, exactly like run_campaign's
    # own `_sha(lock.json) != preflight["lock_sha256"]` check).
    preflight = {"lock_sha256": _sha((directory / "lock.json").read_bytes()),
                 "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
                 "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
                 "case_count": len(lock["manifest"]), "validation": "complete preflight passed"}
    (directory / "preflight.json").write_bytes((_json(preflight) + "\n").encode("utf-8"))

    part_bytes = part_path.read_bytes()
    header_bytes, header, per_case, part_undecodable = _parse_trace_lines(_split_lines(part_bytes))
    if header.get("table_sha256") != table_sha256:
        raise ValueError("interrupted trace header does not match the registered table")
    if header.get("stages") != stages:
        raise ValueError("interrupted trace header does not match the registered stage count")

    complete_blocks: dict[str, list[bytes]] = {}
    discarded_partial_case_ids: list[str] = []
    for case_id, records in per_case.items():
        block = _complete_case_block(records, stages)
        if block is None:
            discarded_partial_case_ids.append(case_id)
        else:
            complete_blocks[case_id] = block

    manifest_text = (directory / "manifest.tsv").read_text(encoding="utf-8")
    manifest_lines = [line for line in manifest_text.split("\n") if line.strip()]
    manifest_case_ids = [line.split("\t", 1)[0] for line in manifest_lines]
    locked_case_ids = [row["case"]["case_id"] for row in lock["manifest"]]
    if manifest_case_ids != locked_case_ids:
        raise ValueError("manifest.tsv case order does not match the locked manifest")

    resume_case_ids = [cid for cid in manifest_case_ids if cid not in complete_blocks]
    if not resume_case_ids:
        raise ValueError("nothing to resume: every manifest case already has a complete trace block")
    resume_id_set = set(resume_case_ids)
    resume_lines = [line for line in manifest_lines if line.split("\t", 1)[0] in resume_id_set]
    manifest_resume_path = directory / "manifest_resume.tsv"
    manifest_resume_path.write_text("\n".join(resume_lines) + "\n", encoding="utf-8")

    trace_resume_path = directory / "trace_resume.jsonl"
    if trace_resume_path.exists():
        raise ValueError("trace_resume.jsonl already exists; refusing to overwrite a prior resume attempt")

    runner = root / RUNNER
    table_path = H.table_path()
    args = [table_path, manifest_resume_path, trace_resume_path]
    linux_args = [_linux(p) for p in args] if os.name == "nt" else [str(p) for p in args]
    runner_arg = _linux(runner) if os.name == "nt" else str(runner)
    command = (["wsl", "-d", "Ubuntu-22.04", "--", runner_arg] + linux_args if os.name == "nt"
               else [str(runner)] + linux_args)
    timeout_seconds = run_timeout_seconds(lock["registration"])
    started = time.perf_counter()
    result = subprocess.run(command + [str(stages)], capture_output=True, text=True, timeout=timeout_seconds)
    resume_elapsed = time.perf_counter() - started
    (directory / "resume.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (directory / "resume.stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("GPU hydro campaign resume runner failed; partial evidence retained: " + result.stderr)
    if validate_lock(directory) != lock or _sha((directory / "lock.json").read_bytes()) != preflight["lock_sha256"]:
        raise ValueError("campaign inputs changed during the resume GPU execution")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith("{")]
    if not devices or devices[0].get("backend") != "cuda_device_kernels":
        raise ValueError("missing real GPU provenance on resume")

    resume_bytes = trace_resume_path.read_bytes()
    _, resume_header, resume_per_case, resume_undecodable = _parse_trace_lines(_split_lines(resume_bytes))
    if resume_header.get("table_sha256") != table_sha256 or resume_header.get("stages") != stages:
        raise ValueError("resume trace header does not match the registered law/stage count")
    if resume_undecodable:
        raise ValueError("resume trace contains undecodable lines")
    missing = resume_id_set - set(resume_per_case)
    if missing:
        raise ValueError(f"resume trace missing cases: {sorted(missing)}")
    resume_blocks: dict[str, list[bytes]] = {}
    for case_id in resume_case_ids:
        block = _complete_case_block(resume_per_case[case_id], stages)
        if block is None:
            raise ValueError(f"resume trace incomplete for case {case_id}")
        resume_blocks[case_id] = block

    final_lines = [header_bytes]
    for case_id in manifest_case_ids:
        block = complete_blocks.get(case_id)
        final_lines.extend(block if block is not None else resume_blocks[case_id])
    final_bytes = b"\n".join(final_lines) + b"\n"
    trace_path.write_bytes(final_bytes)

    part_sha256 = _sha(part_bytes)
    resume_trace_sha256 = _sha(resume_bytes)
    final_sha256 = _sha(final_bytes)
    runner_sha256 = _sha(runner.read_bytes())

    original_elapsed = max(0.0, kill_time - original_start)
    interruption = {
        "original_start": datetime.fromtimestamp(original_start, tz=timezone.utc).isoformat(),
        "original_start_source": original_start_source,
        "kill_time": datetime.fromtimestamp(kill_time, tz=timezone.utc).isoformat(),
        "kill_time_source": "trace.jsonl.part mtime (its last successful write before the host restart)",
        "cause": "host restart by an operating-system update",
        "original_run_elapsed_seconds": original_elapsed,
        "resume_elapsed_seconds": resume_elapsed,
        "part_undecodable_lines": part_undecodable,
        "counts": {
            "total_cases": len(manifest_case_ids),
            "kept_complete_cases": len(complete_blocks),
            "discarded_partial_case_ids": sorted(discarded_partial_case_ids),
            "resumed_cases": len(resume_case_ids),
        },
        "trace_part_sha256": part_sha256,
        "trace_resume_sha256": resume_trace_sha256,
        "trace_final_sha256": final_sha256,
        "resume_instrument": {"command": command + [str(stages)], "binary": str(runner),
                              "binary_sha256": runner_sha256},
        "original_preflight_sha256": _sha(original_preflight_bytes) if original_preflight_bytes is not None else None,
        "concatenation_exactness": ("every case is an independent deterministic computation of its own "
                                     "frozen, hash-verified preparation -- phi-hydro-staged-candidate-1's "
                                     "tick has no cross-case coupling -- so concatenating the kept-complete "
                                     "blocks with the freshly re-run resume blocks, in manifest order, "
                                     "reproduces exactly what a single uninterrupted run would have "
                                     "written, case by case; this is exact, not an approximation"),
    }
    execution = {"schema": "strict-hydro4-execution-1",
                 "preflight_sha256": _sha((directory / "preflight.json").read_bytes()),
                 "postflight": "complete frozen-source/input validation passed", "device": devices[0],
                 "elapsed_seconds": original_elapsed + resume_elapsed,
                 "physical_microticks": len(lock["manifest"]) * stages * STAGE_MICROTICKS,
                 "trace_sha256": final_sha256, "interruption": interruption}
    (directory / "execution.json").write_bytes((_json(execution) + "\n").encode("utf-8"))
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


def _rate_seeds_and_se(modes: np.ndarray, lo: int, hi: int, gamma_weight: float) -> tuple[np.ndarray, float]:
    """Per-seed rate fits and their standard error, extracted with the SAME weighted
    estimator (`gamma_weight`) `gamma_meas` itself uses -- the consistency-note fix
    (task-11-review.md): the pre-fix code fit the per-seed rates unweighted
    (`_fit_rate(m, lo, hi)`, no `gamma_weight`) while `gamma_meas` used the weighted
    estimator, so the point estimate and the standard error gating it disagreed on what
    estimator they characterized. Factored out of `summarize_campaign` so this is
    unit-testable on a synthetic multi-seed series without the GPU/lock pipeline."""
    rate_seeds = np.array([_fit_rate(m, lo, hi, gamma_weight=gamma_weight) for m in modes])
    se = float(np.std(rate_seeds, ddof=1) / np.sqrt(len(modes))) if len(modes) > 1 else float("nan")
    return rate_seeds, se


def _cell_group_key(case_dict: dict):
    return (case_dict["arm"], case_dict["mx"], case_dict["my"], case_dict["mz"], case_dict["tx"],
            case_dict["ty"], case_dict["tz"], case_dict["eps"], case_dict["u0"])


def _shear_group_summary(shear_nu_meas: dict, shear_nu_pred: dict, shear_details: dict,
                         m: int, eps_str: str) -> dict:
    """One row of the per-(m, eps) shear table (task-12-fix1): every one of the four
    registered (direction, polarization) cells at this (m, eps), plus the anisotropy ratio,
    the cubic identity, and nu(1,1,1) derived from the SAME canonical cell selection the
    top-level report fields use -- nu_T2 from `_SHEAR_T2_CELL`, nu_E from `_SHEAR_E_CELL`,
    nu(1,1,1) read directly off `_SHEAR_CUBIC_CELL` -- so the top-level fields
    (`anisotropy_ratio_measured` etc.) are always exactly the `m=1, eps=1/10` row of this
    table, never a value assembled from mismatched (m, eps) cells. `shear_nu_meas`/
    `shear_nu_pred`/`shear_details` must be keyed by the FULL cell identity
    `(direction, polarization, m, eps_str)` -- this is the fix for the collapse bug
    task-12-report.md found (the pre-fix dicts keyed only on `(direction, polarization)`,
    silently overwriting every m=1 cell with its m=2 counterpart)."""
    def _entry(direction, polarization, kind):
        key = (direction, polarization, m, eps_str)
        entry = {"direction": list(direction), "polarization": list(polarization), "kind": kind,
                 "nu_measured": shear_nu_meas.get(key), "nu_pred_numeric": shear_nu_pred.get(key)}
        entry.update(shear_details.get(key) or {"nu_exact_limit": None, "powered": False,
                                                 "rate_pass": None, "rms_pass": None})
        return entry

    nu_T2 = shear_nu_meas.get((*_SHEAR_T2_CELL, m, eps_str))
    nu_E = shear_nu_meas.get((*_SHEAR_E_CELL, m, eps_str))
    nu_T2_pred = shear_nu_pred.get((*_SHEAR_T2_CELL, m, eps_str))
    nu_E_pred = shear_nu_pred.get((*_SHEAR_E_CELL, m, eps_str))
    nu_111 = shear_nu_meas.get((*_SHEAR_CUBIC_CELL, m, eps_str))
    nu_111_pred = shear_nu_pred.get((*_SHEAR_CUBIC_CELL, m, eps_str))
    ratio = (nu_E / nu_T2) if (nu_T2 is not None and nu_E is not None and nu_T2) else None
    ratio_pred = (nu_E_pred / nu_T2_pred) if (nu_T2_pred is not None and nu_E_pred is not None and nu_T2_pred) \
        else None
    cubic = ((2 * nu_E + nu_T2) / 3) if (nu_T2 is not None and nu_E is not None) else None
    cubic_pred = ((2 * nu_E_pred + nu_T2_pred) / 3) if (nu_T2_pred is not None and nu_E_pred is not None) else None
    return {"m": m, "eps": eps_str,
            "cells": [_entry(*_SHEAR_T2_CELL, "T2"), _entry(*_SHEAR_T2_ALT_CELL, "T2"),
                     _entry(*_SHEAR_E_CELL, "E"), _entry(*_SHEAR_CUBIC_CELL, "cubic")],
            "anisotropy_ratio_measured": ratio, "anisotropy_ratio_pred_numeric": ratio_pred,
            "cubic_identity_from_measured_constants": cubic, "cubic_identity_from_pred_numeric": cubic_pred,
            "nu_111_measured": nu_111, "nu_111_pred_numeric": nu_111_pred,
            "is_registered_top_level_source": (m == _SHEAR_CANONICAL_M and eps_str == _SHEAR_CANONICAL_EPS)}


def _shear_derived_by_m_eps(shear_nu_meas: dict, shear_nu_pred: dict, shear_details: dict) -> list[dict]:
    """The complete per-(m, eps) shear table -- one `_shear_group_summary` row for every
    registered (m, eps) combination (task-12-fix1: "emit a per-(m, eps) table ... so no
    information is hidden"). `WAVENUMBERS`/`EPSILONS` are the module's own registered grids,
    not retyped."""
    return [_shear_group_summary(shear_nu_meas, shear_nu_pred, shear_details, m, str(eps))
            for m in WAVENUMBERS for eps in EPSILONS]


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
    power_by_cell = {tuple(r["cell"]): r for r in power["cells"]}
    results, shear_nu_meas, shear_nu_pred, shear_details, sound_c_meas, density_rows, galilean_row = \
        [], {}, {}, {}, {}, [], None
    for key, case_dicts in sorted(groups.items()):
        cases_here = [_case_from_dict(c) for c in case_dicts]
        example = cases_here[0]
        modes = np.array([[_project(traces[c.case_id][n], c) for n in range(stages + 1)] for c in cases_here])
        mean = modes.mean(axis=0)
        predicted_series = np.array([_project(np.array([complex(*p) for p in predictions[example.case_id][n]]), example)
                                     for n in range(stages + 1)])
        power_row = power_by_cell[key]
        # gamma_pred is the fit-on-prediction predicted_rate at the REGISTERED stages/window
        # (== power_row["gamma_pred_numeric"], already computed there -- reused rather than
        # recomputed to avoid a second predict() pass over the same trajectory).
        gamma_pred = power_row["gamma_pred_numeric"]
        gamma_meas = _fit_rate(mean, lo, hi, gamma_weight=gamma_pred)
        # Consistency-note fix (task-11-fix1, see _rate_seeds_and_se): the per-seed fits
        # feeding the operative SE now use the SAME weighted estimator (gamma_weight=
        # gamma_pred) as gamma_meas itself, so the point estimate and the standard error
        # gating it agree on what estimator they characterize (previously these were
        # unweighted OLS fits, held against a gamma_meas built from the weighted estimator).
        rate_seeds, se_gamma_seeds = _rate_seeds_and_se(modes, lo, hi, gamma_pred)
        se_gamma_analytic = power_row["se_gamma"]
        amplitude = _amplitude(example)
        rel_rms = float(np.sqrt(np.sum(np.abs(mean - predicted_series) ** 2) / np.sum(np.abs(predicted_series) ** 2))) \
            if np.sum(np.abs(predicted_series) ** 2) > 0 else float("nan")
        sigma = noise_floor(example)
        this_noise_ratio = sigma / amplitude if amplitude > 0 else float("inf")
        powered = key in powered_keys
        # Registered floor (task-11-fix1, mirrors registration()'s "rate_tolerance_rule"):
        # the analytic SE from power_calculation is kept as a floor alongside the empirical
        # (now weighted) per-seed scatter, so a campaign with very few effectively-varying
        # seeds still gets a defensible tolerance.
        rate_tol_components = [float(ACCEPTANCE["rate_relative"]) * abs(gamma_pred)]
        if not np.isnan(se_gamma_seeds):
            rate_tol_components.append(float(ACCEPTANCE["rate_sigma"]) * se_gamma_seeds)
        if np.isfinite(se_gamma_analytic) and se_gamma_analytic > 0:
            rate_tol_components.append(float(ACCEPTANCE["rate_sigma"]) * se_gamma_analytic)
        rate_tol = max(rate_tol_components)
        rms_tol = max(float(ACCEPTANCE["rms_relative"]), float(ACCEPTANCE["noise_multiple"]) * this_noise_ratio)
        rate_ok = bool(abs(gamma_meas - gamma_pred) <= rate_tol)
        rms_ok = bool(rel_rms <= rms_tol)
        row = {"cell": list(key), "arm": example.arm, "seeds": len(cases_here), "powered": powered,
               "predicted_rate": gamma_pred, "gamma_pred_numeric": gamma_pred,
               "predicted_rate_eigenvalue": power_row["predicted_rate_eigenvalue"],
               "measured_rate": gamma_meas, "rate_standard_error": se_gamma_seeds,
               "rate_standard_error_analytic": se_gamma_analytic, "rate_tolerance": rate_tol,
               "relative_rms": rel_rms, "rms_tolerance": rms_tol,
               "rate_pass": rate_ok, "rms_pass": rms_ok,
               "mean_trajectory": [[float(v.real), float(v.imag)] for v in mean],
               "k": [example.mx, example.my, example.mz]}
        if "nu_exact_limit" in power_row:
            row["nu_exact_limit"] = power_row["nu_exact_limit"]
            row["k4_systematic"] = power_row["k4_systematic"]
        if example.arm == "shear":
            k2 = _k_squared(example)
            nu_meas = gamma_meas / k2 if k2 else float("nan")
            nu_pred_numeric = gamma_pred / k2 if k2 else float("nan")
            direction = _unit_direction(example)
            polarization = (int(example.tx), int(example.ty), int(example.tz))
            m = max(abs(example.mx), abs(example.my), abs(example.mz))
            eps_str = str(example.eps)
            row["nu_measured"] = nu_meas
            row["nu_pred_numeric"] = nu_pred_numeric
            row["direction"] = list(direction)
            row["polarization"] = list(polarization)
            row["m"] = m
            row["eps"] = eps_str
            # Keyed by the FULL cell identity (direction, polarization, m, eps) -- task-12-fix1
            # (see task-12-report.md's reporting-code caveat). The pre-fix dict keyed only on
            # (direction, polarization): every m=2 cell normalizes to the SAME unit direction as
            # its m=1 counterpart (`_unit_direction((2,2,0)) == (1,1,0)`), so later-sorted groups
            # silently overwrote earlier ones and the top-level fields below landed on whichever
            # (m, eps) happened to sort last -- an underpowered m=2/eps=1/5 cell, not the clean
            # registered representative. Nothing below this point collapses across (m, eps) now.
            shear_key = (direction, polarization, m, eps_str)
            shear_nu_meas[shear_key] = nu_meas
            shear_nu_pred[shear_key] = nu_pred_numeric
            shear_details[shear_key] = {"nu_exact_limit": row.get("nu_exact_limit"), "powered": powered,
                                        "rate_pass": rate_ok, "rms_pass": rms_ok}
        if example.arm == "sound":
            omega_meas = _fit_angular_rate(mean, lo, hi)
            k_norm = float(np.sqrt(_k_squared(example)))
            c_s_meas_raw = omega_meas / k_norm if k_norm else float("nan")
            omega_pred = power_row["predicted_frequency_eigenvalue"]
            c_s_pred = omega_pred / k_norm if k_norm else float("nan")
            # Branch-sign fix (task-12-fix1, see task-12-report.md's sign-ambiguity finding):
            # a sound mode is a complex-conjugate +-k branch pair; `_fit_angular_rate`'s
            # phase-unwrap can lock onto either branch depending on realized seed noise, while
            # `predicted_frequency_eigenvalue` always reports one fixed branch. `c_s_measured`
            # is now sign-corrected to the PREDICTED branch (the physically meaningful, narrative
            # comparison); the untouched signed fit and its bare magnitude are both still
            # reported alongside a note. This is a narrative field only -- acceptance gates on
            # `gamma`/`rel_rms` above, unaffected by this branch.
            if np.isfinite(c_s_pred) and c_s_pred != 0 and np.isfinite(c_s_meas_raw):
                c_s_meas = float(np.copysign(abs(c_s_meas_raw), c_s_pred))
            else:
                c_s_meas = c_s_meas_raw
            row["omega_measured"] = omega_meas
            row["omega_predicted"] = omega_pred
            row["c_s_measured"] = c_s_meas
            row["c_s_measured_raw_signed_fit"] = c_s_meas_raw
            row["c_s_measured_magnitude"] = abs(c_s_meas_raw) if np.isfinite(c_s_meas_raw) else c_s_meas_raw
            row["c_s_branch_note"] = ("sound is a complex-conjugate +-k branch pair; c_s_measured is "
                                      "sign-corrected to the predicted branch, c_s_measured_raw_signed_fit is "
                                      "the unmodified phase-unwrap fit -- narrative only, not gated")
            row["c_s_predicted"] = c_s_pred
            row["c_s_exact"] = float(np.sqrt(float(Fraction(_verdict()["sound_speed_squared"]))))
            sound_c_meas[_unit_direction(example)] = c_s_meas
        if example.arm == "density":
            row["direction"] = list(_unit_direction(example))
            density_rows.append(row)
        if example.arm == "galilean":
            omega_meas = _fit_angular_rate(mean, lo, hi)
            k_norm = float(np.sqrt(_k_squared(example)))
            u0 = float(example.u0)
            advection_raw = omega_meas / (k_norm * u0) if k_norm and u0 else float("nan")
            omega_pred = power_row["predicted_frequency_eigenvalue"]
            advection_pred = omega_pred / (k_norm * u0) if k_norm and u0 else float("nan")
            # Same branch-sign character as the sound cells' c_s (task-12-fix1, see
            # task-12-report.md): sign-correct the measured ratio to the predicted branch,
            # keep the untouched signed fit and its magnitude, narrative only, not gated.
            if np.isfinite(advection_pred) and advection_pred != 0 and np.isfinite(advection_raw):
                advection_meas = float(np.copysign(abs(advection_raw), advection_pred))
            else:
                advection_meas = advection_raw
            row["omega_measured"] = omega_meas
            row["omega_predicted"] = omega_pred
            row["advection_ratio_measured"] = advection_meas
            row["advection_ratio_measured_raw_signed_fit"] = advection_raw
            row["advection_ratio_measured_magnitude"] = abs(advection_raw) if np.isfinite(advection_raw) \
                else advection_raw
            row["advection_ratio_predicted"] = advection_pred
            row["advection_branch_note"] = ("same oscillatory complex-conjugate branch ambiguity as the sound "
                                            "cells' c_s_measured; advection_ratio_measured is sign-corrected to "
                                            "the predicted branch -- narrative only, not gated")
            row["g_exact"] = float(B.nonlinear_coefficients(DENSITY)["g"])
            galilean_row = row
        results.append(row)
    # Top-level derived quantities (task-12-fix1, controller ruling): derived from the
    # explicitly registered representative cells only -- the `m=1, eps=1/10` row of the
    # per-(m, eps) table below, never from a dict collapse across wavenumber/epsilon (see
    # `_shear_group_summary`'s docstring and task-12-report.md's reporting-code caveat).
    shear_derived_by_m_eps = _shear_derived_by_m_eps(shear_nu_meas, shear_nu_pred, shear_details)
    canonical_group = next((g for g in shear_derived_by_m_eps if g["is_registered_top_level_source"]), None)
    ratio_meas = canonical_group["anisotropy_ratio_measured"] if canonical_group else None
    # The measured ratio and the cubic identity are compared to the SAME quantities computed
    # from the finite-k numeric predictions (ratio_pred_numeric / cubic_identity_from_pred_
    # numeric), with the exact small-k H1' constants kept alongside, explicitly labelled as
    # the k -> 0 limits (task-11-fix1).
    ratio_pred_numeric = canonical_group["anisotropy_ratio_pred_numeric"] if canonical_group else None
    cubic_identity_from_measured_constants = canonical_group["cubic_identity_from_measured_constants"] \
        if canonical_group else None
    cubic_identity_from_pred_numeric = canonical_group["cubic_identity_from_pred_numeric"] if canonical_group else None
    nu_111_meas = canonical_group["nu_111_measured"] if canonical_group else None
    nu_111_pred = canonical_group["nu_111_pred_numeric"] if canonical_group else None
    cubic_exact = _verdict()["cubic_shear_constants"]
    nu_T2_exact_k0, nu_E_exact_k0 = Fraction(cubic_exact["nu_T2"]), Fraction(cubic_exact["nu_E"])
    ratio_exact_k0_limit = float(nu_E_exact_k0 / nu_T2_exact_k0) if cubic_exact else None
    cubic_identity_exact_k0_limit = float((2 * nu_E_exact_k0 + nu_T2_exact_k0) / 3) if cubic_exact else None
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
              "anisotropy_ratio_measured": ratio_meas, "anisotropy_ratio_pred_numeric": ratio_pred_numeric,
              "anisotropy_ratio_exact_k0_limit": ratio_exact_k0_limit,
              "cubic_identity_from_measured_constants": cubic_identity_from_measured_constants,
              "cubic_identity_from_pred_numeric": cubic_identity_from_pred_numeric,
              "cubic_identity_exact_k0_limit": cubic_identity_exact_k0_limit,
              "nu_111_measured": nu_111_meas, "nu_111_pred_numeric": nu_111_pred,
              "shear_derived_quantities_by_m_eps": shear_derived_by_m_eps,
              "density_rows": density_rows, "galilean_row": galilean_row, "results": results}
    if write:
        (directory / "report.json").write_text(_json(output) + "\n", encoding="utf-8")
    return output
