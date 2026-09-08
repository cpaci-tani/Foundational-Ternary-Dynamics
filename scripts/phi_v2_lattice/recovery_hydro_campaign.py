# scripts/phi_v2_lattice/recovery_hydro_campaign.py
"""Registered hydrodynamic response campaign (gate H2): preparation, observable, prediction, locks.

When the environment variable FTD_HYDRO_ALLOW_MISSING_PREREG is set to "1", prepare_campaign and
validate_lock record "MISSING" as the hash of any instrument path that does not exist on disk
(needed only for isolated tests exercising the lock protocol before Task 9 writes the PREREG doc);
the campaign of record runs without that variable, so a missing instrument path is then a hard error.
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

from . import native_codec as N_
from . import staged as Staged
from . import recovery_hydro_dispersion as D
from . import recovery_hydro_invariants as H
from . import recovery_kinetic_reference as Ref

PROTOCOL_ID = "strict-hydro-response-1"
P_REFERENCE = Fraction(1, 96)
P = P_REFERENCE
EPSILON = Fraction(1, 4)
SEEDS = 8
DIRECTIONS = D.DIRECTIONS
WAVENUMBERS = (1, 2)
PERIOD = 48
ACCEPTANCE = {"relative_rms_max": Fraction(1, 10), "rate_band_sigma": 2}
RUNNER = "engine/build_strict_hydro/ftd_strict_hydro_campaign"


@dataclass(frozen=True)
class HydroCase:
    case_id: str
    L: int
    mx: int
    my: int
    mz: int
    mode: int
    seed: int
    polarity_offset: int = 0


def cases(L: int, stroboscopes: int) -> tuple[HydroCase, ...]:
    out = []
    for direction in DIRECTIONS:
        for m in WAVENUMBERS:
            for mode in range(7):
                for seed_index in range(SEEDS):
                    tag = "".join(str(x) for x in direction)
                    out.append(HydroCase(f"h_d{tag}_m{m}_mode{mode}_s{seed_index}", L,
                                         m * direction[0], m * direction[1], m * direction[2],
                                         mode, 20260907 * 1000 + seed_index))
    return tuple(out)


def _exact_proxy():
    return D.exact_dispersion((1, 0, 0))


_PROXY = None


def _proxy():
    global _PROXY
    if _PROXY is None:
        _PROXY = _exact_proxy()
    return _PROXY


def weights_rows() -> tuple[tuple[int, ...], ...]:
    """Stage-0 conserved weights, in the fixed order used by W in the dispersion module."""
    return H.locked_kernel(0)


def mode_shape(mode: int) -> np.ndarray:
    """Right equilibrium mode `mode` (column of V at the proxy coupling), scaled to max |phi| = 1."""
    V = _proxy().V
    column = np.array([float(Fraction(int(V[c, mode].p), int(V[c, mode].q))) for c in range(192)])
    return column / np.max(np.abs(column))


def probabilities(case: HydroCase) -> np.ndarray:
    L = case.L
    idx = np.arange(L ** 3)
    x, y, z = idx // (L * L), (idx // L) % L, idx % L
    phase = np.cos(2 * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
    phi = mode_shape(case.mode)
    return float(P) * (1 + float(EPSILON) * np.outer(phase, phi))


def prepare(case: HydroCase) -> Staged.StagedState:
    rng = np.random.default_rng(case.seed)
    L = case.L
    bank = np.zeros((L ** 3, 384), dtype=bool)
    bank[:, case.polarity_offset:case.polarity_offset + 192] = rng.random((L ** 3, 192)) < probabilities(case)
    return Ref.prepare_bank(bank, L, layer=0)


def observe_moments(state: Staged.StagedState, case: HydroCase) -> np.ndarray:
    L = case.L
    W = np.array(weights_rows(), dtype=float)
    idx = np.arange(L ** 3)
    x, y, z = idx // (L * L), (idx // L) % L, idx % L
    phase = np.exp(-2j * np.pi * (case.mx * x + case.my * y + case.mz * z) / L)
    bank = state.lattice.bank[:, case.polarity_offset:case.polarity_offset + 192].astype(float)
    return phase @ (bank @ W.T)


def predict(case: HydroCase, stroboscopes: int) -> np.ndarray:
    """Linear-Boltzmann prediction m(n) = W P(k)^n h0 with h0 = N p eps phi / 2 (float64)."""
    L = case.L
    k = 2 * np.pi * np.array([case.mx, case.my, case.mz], dtype=float) / L
    Pk = D.numeric_period_map(k, P)
    W = np.array(weights_rows(), dtype=float)
    h = (L ** 3) * float(P) * float(EPSILON) / 2 * mode_shape(case.mode).astype(complex)
    out = np.zeros((stroboscopes + 1, 7), dtype=complex)
    for n in range(stroboscopes + 1):
        out[n] = W @ h
        h = Pk @ h
    return out


def slowest_rate(L: int) -> float:
    """Smallest per-period decay -ln|lambda| among the seven slow modes over the registered wavevectors."""
    rates = []
    for direction in DIRECTIONS:
        for m in WAVENUMBERS:
            k = 2 * np.pi * m * np.array(direction, dtype=float) / L
            values = np.linalg.eigvals(D.numeric_period_map(k, P))
            slow = np.sort(np.abs(values))[-7:]
            if np.any(slow > 1 + 1e-9):
                raise ValueError(f"linear instability at k={k}: |lambda|={slow.max()}; report, do not register")
            rates.append(float(np.min(-np.log(np.minimum(slow, 1.0)))))
    return min(rates)


def horizon(L: int, microticks_per_second: float, budget_seconds: float = 7200.0) -> int:
    """At least three e-folds of the slowest mode, capped by the throughput budget."""
    needed = int(np.ceil(3.0 / slowest_rate(L)))
    affordable = int(budget_seconds * microticks_per_second // (len(cases(L, 1)) * PERIOD))
    return max(1, min(needed, affordable))


def registration(L: int, stroboscopes: int) -> dict:
    return {"protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID, "p": str(P), "epsilon": str(EPSILON),
            "L": L, "stroboscopes": stroboscopes, "period_microticks": PERIOD, "directions": DIRECTIONS,
            "wavenumbers": WAVENUMBERS, "modes": list(range(7)), "seeds": SEEDS,
            "proxy_r_for_mode_shapes": str(D.PROXY_R), "acceptance": {k: str(v) for k, v in ACCEPTANCE.items()},
            "weights_sha256": _sha("\n".join("\t".join(map(str, r)) for r in weights_rows()).encode()),
            "boundary": "periodic", "background": "homogeneous doubly occupied A9, s=0, ell=0, single polarity"}


def _json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), default=list)


def _sha(data):
    return hashlib.sha256(data).hexdigest()


def _linux(path):
    value = str(Path(path).resolve()).replace("\\", "/")
    return "/mnt/" + value[0].lower() + value[2:] if len(value) > 1 and value[1] == ":" else value


def write_weights(path):
    Path(path).write_text("\n".join("\t".join(map(str, r)) for r in weights_rows()) + "\n", encoding="utf-8")


def instrument_paths() -> set[str]:
    root = Path(__file__).resolve().parents[2]
    paths = {"engine/strict/recovery_hydro/hydro_main.cpp", "engine/strict/recovery_hydro/CMakeLists.txt",
             "engine/strict/CMakeLists.txt", "engine/strict/cuda.cmake", "engine/strict/staged_runtime.cpp",
             "engine/strict/staged_cuda.cu", "engine/strict/frozen_tables.h", "engine/strict/staged_runtime.h",
             "engine/strict/staged_cuda.h", "engine/docs/PREREG_STRICT_HYDRODYNAMIC_RESPONSE.md"}
    seeds = (__name__, "phi_v2_lattice", "phi_v2_lattice.channels", "phi_v2_lattice.state", "phi_v2_lattice._proofs",
             "phi_v2_lattice.geometry", "phi_v2_lattice.staged", "phi_v2_lattice.tick", "phi_v2_lattice.native_codec",
             "phi_v2_lattice.checkpoint", "phi_v2_lattice.recovery_hydro_dispersion",
             "phi_v2_lattice.recovery_hydro_invariants", "phi_v2_lattice.recovery_kinetic_response",
             "phi_v2_lattice.recovery_kinetic_reference")
    pending = [sys.modules[name] for name in seeds]
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


def _allow_missing_prereg():
    return os.environ.get("FTD_HYDRO_ALLOW_MISSING_PREREG") == "1"


def _hash_instrument_path(root, relative):
    full = root / relative
    if not full.is_file():
        if _allow_missing_prereg():
            return "MISSING"
        raise ValueError(f"instrument source path does not exist: {relative}")
    return _sha(full.read_bytes())


def prepare_campaign(directory, L: int, stroboscopes: int) -> dict:
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    if (directory / "lock.json").exists():
        raise ValueError("campaign already locked; do not overwrite registration")
    root = Path(__file__).resolve().parents[2]
    runner = root / RUNNER
    if not runner.is_file():
        raise ValueError("build the real CUDA hydro runner before locking")
    write_weights(directory / "weights.tsv")
    manifest, lines, predictions = [], [], {}
    for case in cases(L, stroboscopes):
        path = directory / (case.case_id + ".bin")
        blob = N_.encode(prepare(case))
        path.write_bytes(blob)
        manifest.append({"case": asdict(case), "initial_sha256": _sha(blob)})
        lines.append("\t".join([case.case_id, _linux(path), str(case.L), str(case.mx), str(case.my), str(case.mz),
                                str(case.polarity_offset)]))
        predicted = predict(case, stroboscopes)
        predictions[case.case_id] = [[[float(v.real), float(v.imag)] for v in row] for row in predicted]
    reg = registration(L, stroboscopes)
    (directory / "predictions.json").write_bytes((_json(predictions) + "\n").encode("utf-8"))
    lock = {"registration": reg, "registration_sha256": _sha(_json(reg).encode()),
            "manifest": manifest, "manifest_sha256": _sha(_json(manifest).encode()),
            "predictions_sha256": _sha((directory / "predictions.json").read_bytes()),
            "instrument_sha256": {p: _hash_instrument_path(root, p) for p in sorted(instrument_paths())},
            "runner_sha256": _sha(runner.read_bytes()),
            "weights_sha256": _sha((directory / "weights.tsv").read_bytes())}
    (directory / "lock.json").write_text(_json(lock) + "\n", encoding="utf-8")
    (directory / "manifest.tsv").write_text("\n".join(lines) + "\n", encoding="utf-8")
    return {"case_count": len(manifest), "manifest_sha256": lock["manifest_sha256"],
            "registration_sha256": lock["registration_sha256"],
            "microticks": len(manifest) * stroboscopes * PERIOD}


def validate_lock(directory) -> dict:
    directory = Path(directory)
    lock = json.loads((directory / "lock.json").read_text())
    root = Path(__file__).resolve().parents[2]
    if set(lock["instrument_sha256"]) != instrument_paths():
        raise ValueError("incomplete instrument source closure")
    if any(_hash_instrument_path(root, p) != h for p, h in lock["instrument_sha256"].items()):
        raise ValueError("instrument source changed after registration lock")
    if _sha((root / RUNNER).read_bytes()) != lock["runner_sha256"]:
        raise ValueError("CUDA runner changed after registration lock")
    reg = lock["registration"]
    if _sha(_json(reg).encode()) != lock["registration_sha256"] or _json(reg) != _json(registration(reg["L"], reg["stroboscopes"])):
        raise ValueError("registration lock changed")
    if _sha(_json(lock["manifest"]).encode()) != lock["manifest_sha256"]:
        raise ValueError("manifest lock changed")
    if [row["case"] for row in lock["manifest"]] != [asdict(c) for c in cases(reg["L"], reg["stroboscopes"])]:
        raise ValueError("manifest case inventory differs from registration")
    if _sha((directory / "predictions.json").read_bytes()) != lock["predictions_sha256"]:
        raise ValueError("locked predictions changed")
    if _sha((directory / "weights.tsv").read_bytes()) != lock["weights_sha256"]:
        raise ValueError("weights changed")
    expected = []
    for row in lock["manifest"]:
        c = row["case"]
        path = directory / (c["case_id"] + ".bin")
        if _sha(path.read_bytes()) != row["initial_sha256"]:
            raise ValueError("initial preparation hash changed")
        expected.append("\t".join([c["case_id"], _linux(path), str(c["L"]), str(c["mx"]), str(c["my"]), str(c["mz"]),
                                   str(c["polarity_offset"])]))
    if (directory / "manifest.tsv").read_text(encoding="utf-8") != "\n".join(expected) + "\n":
        raise ValueError("executable manifest differs from lock")
    return lock


def run_campaign(directory) -> dict:
    directory = Path(directory).resolve()
    lock = validate_lock(directory)
    root = Path(__file__).resolve().parents[2]
    runner = root / RUNNER
    preflight = {"lock_sha256": _sha((directory / "lock.json").read_bytes()),
                 "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
                 "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
                 "case_count": len(lock["manifest"]), "validation": "complete preflight passed"}
    (directory / "preflight.json").write_text(_json(preflight) + "\n", encoding="utf-8")
    args = [runner, directory / "manifest.tsv", directory / "trace.jsonl", directory / "weights.tsv"]
    command = (["wsl", "-d", "Ubuntu-22.04", "--"] + [_linux(p) for p in args] if os.name == "nt"
               else [str(p) for p in args])
    started = time.perf_counter()
    result = subprocess.run(command + [str(lock["registration"]["stroboscopes"])], capture_output=True, text=True,
                            timeout=4 * 3600)
    elapsed = time.perf_counter() - started
    (directory / "execution.stdout.txt").write_text(result.stdout, encoding="utf-8")
    (directory / "execution.stderr.txt").write_text(result.stderr, encoding="utf-8")
    if result.returncode:
        raise RuntimeError("GPU hydro runner failed; partial evidence retained: " + result.stderr)
    if validate_lock(directory) != lock or _sha((directory / "lock.json").read_bytes()) != preflight["lock_sha256"]:
        raise ValueError("campaign inputs changed during GPU execution")
    devices = [json.loads(line) for line in result.stderr.splitlines() if line.startswith("{")]
    if len(devices) != 1 or devices[0].get("backend") != "cuda_device_kernels":
        raise ValueError("missing real GPU provenance")
    execution = {"schema": "strict-hydro-execution-1", "preflight_sha256": _sha((directory / "preflight.json").read_bytes()),
                 "postflight": "complete frozen-source/input validation passed", "device": devices[0],
                 "elapsed_seconds": elapsed,
                 "physical_microticks": len(lock["manifest"]) * lock["registration"]["stroboscopes"] * PERIOD,
                 "trace_sha256": _sha((directory / "trace.jsonl").read_bytes())}
    (directory / "execution.json").write_text(_json(execution) + "\n", encoding="utf-8")
    return execution


def _fit_rate(series: np.ndarray) -> float:
    """Declared estimator: least-squares slope of ln|m(n)| against n over the horizon."""
    magnitude = np.abs(series)
    n = np.arange(len(series))
    mask = magnitude > 0
    slope = np.polyfit(n[mask], np.log(magnitude[mask]), 1)[0] if mask.sum() > 1 else float("nan")
    return -float(slope)


def summarize_campaign(directory) -> dict:
    directory = Path(directory)
    lock = validate_lock(directory)
    execution = json.loads((directory / "execution.json").read_text())
    preflight_bytes = (directory / "preflight.json").read_bytes()
    preflight = json.loads(preflight_bytes)
    if execution["preflight_sha256"] != _sha(preflight_bytes) or preflight["lock_sha256"] != _sha((directory / "lock.json").read_bytes()):
        raise ValueError("execution/preflight receipt is not tied to current lock")
    if execution["trace_sha256"] != _sha((directory / "trace.jsonl").read_bytes()):
        raise ValueError("trace changed after execution")
    predictions = json.loads((directory / "predictions.json").read_text())
    reg = lock["registration"]
    traces = {}
    with (directory / "trace.jsonl").open(encoding="utf-8") as stream:
        for expected, line in zip(lock["manifest"], stream, strict=True):
            trace = json.loads(line)
            c = expected["case"]
            if trace["case_id"] != c["case_id"]:
                raise ValueError("case order mismatch")
            initial = N_.decode((directory / (c["case_id"] + ".bin")).read_bytes())
            case = HydroCase(**c)
            measured0 = np.array([complex(a, b) for a, b in trace["stroboscopes"][0]["moments"]])
            if not np.allclose(measured0, observe_moments(initial, case), atol=1e-6):
                raise ValueError("initial GPU observer mismatch")
            final = N_.decode((directory / (c["case_id"] + ".final.bin")).read_bytes())
            if trace["stroboscopes"][-1]["sha256"] != _sha(N_.encode(final)):
                raise ValueError("final checkpoint hash mismatch")
            traces[c["case_id"]] = np.array([[complex(a, b) for a, b in s["moments"]] for s in trace["stroboscopes"]])
    groups = {}
    for row in lock["manifest"]:
        c = row["case"]
        groups.setdefault((c["mx"], c["my"], c["mz"], c["mode"]), []).append(c["case_id"])
    results, passes = [], 0
    for key, ids in sorted(groups.items()):
        stack = np.array([traces[i] for i in ids])          # seeds x (n+1) x 7
        mean = stack.mean(axis=0)
        stderr = stack.std(axis=0, ddof=1) / np.sqrt(len(ids))
        mode = key[3]
        predicted = np.array([[complex(a, b) for a, b in row] for row in predictions[ids[0]]])
        rel_rms = float(np.sqrt(np.sum(np.abs(mean[:, mode] - predicted[:, mode]) ** 2)
                                / np.sum(np.abs(predicted[:, mode]) ** 2)))
        rate_measured = _fit_rate(mean[:, mode])
        rate_seeds = np.array([_fit_rate(t[:, mode]) for t in stack])
        rate_predicted = _fit_rate(predicted[:, mode])
        band = ACCEPTANCE["rate_band_sigma"] * float(np.std(rate_seeds, ddof=1) / np.sqrt(len(ids)))
        rms_ok = rel_rms <= float(ACCEPTANCE["relative_rms_max"])
        rate_ok = abs(rate_measured - rate_predicted) <= band
        passes += rms_ok and rate_ok
        results.append({"k": key[:3], "mode": mode, "seeds": len(ids), "relative_rms": rel_rms,
                        "rate_measured": rate_measured, "rate_predicted": rate_predicted, "rate_band": band,
                        "rms_pass": rms_ok, "rate_pass": rate_ok,
                        "mean_trajectory": [[float(v.real), float(v.imag)] for v in mean[:, mode]],
                        "stderr_trajectory": [[float(v.real), float(v.imag)] for v in stderr[:, mode]]})
    output = {"schema": "strict-hydro-report-1", "protocol_id": PROTOCOL_ID, "law_id": Staged.LAW_ID,
              "registration_sha256": lock["registration_sha256"], "manifest_sha256": lock["manifest_sha256"],
              "predictions_sha256": lock["predictions_sha256"], "trace_sha256": execution["trace_sha256"],
              "runner_sha256": lock["runner_sha256"], "instrument_sha256": lock["instrument_sha256"],
              "execution": execution, "registration": reg, "groups": len(results), "groups_passing": passes,
              "boltzmann_closure_verified_at_registered_scope": passes == len(results),
              "results": results}
    (directory / "report.json").write_text(_json(output) + "\n", encoding="utf-8")
    return output
