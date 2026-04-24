"""
GPU computation plan - Priority 4: Structure-2 two-U(1) BCC scalar loop.

This is a fixed verification calculation, not a numerical search.

It implements two quantities:

1. strict:
   Gauge-invariant transverse kinetic correction from a complex scalar
   minimally coupled by Peierls phases on the 8 BCC diagonal links.
   The bubble and seagull terms are both included.

2. literal:
   The handoff's q=0 bubble-only diagnostic. This intentionally omits the
   seagull term and is not used as the decisive physics result.

By default the script uses the periodic Peierls-link integration cell,
k_i in [-2pi/a, 2pi/a), so the strict test is Ward-valid. The optional
--bz framework mode uses the older Structure-1 cutoff convention,
k_i in [-pi/a, pi/a), to reproduce the handoff's literal bubble-only
diagnostic. Framework BZ strict runs are useful diagnostics, but they are not
the physical gauge-invariant result because the Ward check fails there.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import numpy as np

try:
    import cupy as cp
except Exception as exc:  # pragma: no cover - exercised only on non-CUDA hosts
    cp = None
    CUPY_IMPORT_ERROR = exc
else:
    CUPY_IMPORT_ERROR = None


# Fixed framework inputs.
A_LAT = 2.0 / 3.0
G_STAR = math.gamma(0.25) / math.gamma(0.75)
THETA = math.acosh(2.0 * math.sqrt(G_STAR))
TANH_THETA = math.tanh(THETA)
SINH_THETA = math.sinh(THETA)
COTH_THETA = math.cosh(THETA) / math.sinh(THETA)
M_SQ = 16.0 * G_STAR * G_STAR * TANH_THETA
X_TREE = 137.036171458155
X_S1 = 137.035999210
K_PP = 8.0 * G_STAR * G_STAR
K_DD = K_PP
K_PD = 4.0 * (G_STAR ** 1.5) * SINH_THETA
PRIMARY_EIGEN_DERIV = 0.5
HANDOFF_DERIV = 0.5 * (1.0 + COTH_THETA)
X_MINUS = 8.0 * G_STAR * G_STAR * (1.0 - TANH_THETA)
M_SCALAR = math.sqrt(M_SQ)
M_GEOMETRIC = math.sqrt(X_TREE * X_MINUS)


@dataclass(frozen=True)
class BzConfig:
    name: str
    half_width: float
    norm_factor_multiplier: float


@dataclass(frozen=True)
class MatterCase:
    case_id: str
    label: str
    q_p: float
    q_d: float
    mass: float
    note: str

    @property
    def m_sq(self) -> float:
        return self.mass * self.mass

    @property
    def x_plus_factor(self) -> float:
        # Eigenvector for x_+ of [[A,B],[B,A]] is (1,1)/sqrt(2).
        return 0.5 * (self.q_p + self.q_d) ** 2

    @property
    def x_minus_factor(self) -> float:
        # Eigenvector for x_- is (1,-1)/sqrt(2).
        return 0.5 * (self.q_p - self.q_d) ** 2

    @property
    def pp_factor(self) -> float:
        return self.q_p * self.q_p


PRESET_CASES: dict[str, MatterCase] = {
    "S2-A": MatterCase(
        "S2-A", "scalar q=(1,0), M=sqrt(M^2)", 1.0, 0.0, M_SCALAR,
        "natural handoff matter content; already smoke-tested",
    ),
    "S2-B": MatterCase(
        "S2-B", "scalar q=(1,1), M=sqrt(M^2)", 1.0, 1.0, M_SCALAR,
        "same scalar mass, equal charge under both U(1) factors",
    ),
    "S2-C": MatterCase(
        "S2-C", "scalar q=(1,-1), M=sqrt(M^2)", 1.0, -1.0, M_SCALAR,
        "same scalar mass, opposite charge under dark U(1)",
    ),
    "S2-D": MatterCase(
        "S2-D", "scalar q=(1,0), M=x_-", 1.0, 0.0, X_MINUS,
        "lighter mass tied to the dark/eigenvalue root",
    ),
    "S2-E": MatterCase(
        "S2-E", "scalar q=(1,0), M=sqrt(x_+ x_-)", 1.0, 0.0, M_GEOMETRIC,
        "geometric mean mass sqrt(16 G*^3)",
    ),
}


def bz_config(name: str) -> BzConfig:
    if name == "framework":
        return BzConfig(name=name, half_width=math.pi / A_LAT, norm_factor_multiplier=1.0)
    if name == "periodic":
        return BzConfig(name=name, half_width=2.0 * math.pi / A_LAT, norm_factor_multiplier=2.0)
    raise ValueError(f"unknown BZ convention: {name}")


def parse_csv_numbers(text: str, cast=float) -> list:
    vals = []
    for part in text.split(","):
        part = part.strip()
        if part:
            vals.append(cast(part))
    if not vals:
        raise ValueError("empty numeric list")
    return vals


def parse_case_ids(text: str) -> list[str]:
    vals = [part.strip() for part in text.split(",") if part.strip()]
    if not vals:
        raise ValueError("empty case list")
    unknown = [case_id for case_id in vals if case_id not in PRESET_CASES]
    if unknown:
        raise ValueError(f"unknown case id(s): {', '.join(unknown)}")
    return vals


def emit(message: str = "") -> None:
    print(message, flush=True)


def xp_name(xp) -> str:
    return "cupy" if cp is not None and xp is cp else "numpy"


def to_float(value, xp) -> float:
    return float(value.get()) if cp is not None and xp is cp else float(value)


def synchronize(xp) -> None:
    if cp is not None and xp is cp:
        cp.cuda.Device().synchronize()


def k_grid_1d(N: int, bz: BzConfig, xp):
    n = xp.arange(N, dtype=xp.float64)
    return bz.half_width * (2.0 * n / float(N) - 1.0)


def integral_norm(N: int, bz: BzConfig) -> float:
    # (Delta k / 2pi)^3, with Delta k = 2*half_width/N.
    return (bz.half_width / (math.pi * float(N))) ** 3


def denom_bcc(kx, ky, kz, xp, a: float = A_LAT, m_sq: float = M_SQ):
    ax = 0.5 * a * kx
    ay = 0.5 * a * ky
    az = 0.5 * a * kz
    sigma = (8.0 / (a * a)) * (
        1.0 - xp.cos(ax) * xp.cos(ay) * xp.cos(az)
    )
    return m_sq + sigma


def seagull_diag(kx, ky, kz, xp, a: float = A_LAT):
    # W_ii(k) = (1/a^2) sum_delta delta_i^2 exp(i k.delta).
    # For all i, Re W_ii = 2 cos(kx a/2) cos(ky a/2) cos(kz a/2).
    return 2.0 * xp.cos(0.5 * a * kx) * xp.cos(0.5 * a * ky) * xp.cos(0.5 * a * kz)


def vertex_diag(component: str, kx, ky, kz, Qz: float, xp, a: float = A_LAT):
    # V_i(k,Q) = -(i/a^2) sum_delta delta_i exp(i (k + Q/2).delta).
    # For Q along z this reduces to a real expression.
    ax = 0.5 * a * kx
    ay = 0.5 * a * ky
    az_mid = 0.5 * a * (kz + 0.5 * Qz)
    pref = 4.0 / a
    if component == "x":
        return pref * xp.sin(ax) * xp.cos(ay) * xp.cos(az_mid)
    if component == "y":
        return pref * xp.cos(ax) * xp.sin(ay) * xp.cos(az_mid)
    if component == "z":
        return pref * xp.cos(ax) * xp.cos(ay) * xp.sin(az_mid)
    raise ValueError(f"unknown component: {component}")


def q_from_mode(N: int, mode: float, a: float = A_LAT) -> float:
    # Finite-volume momentum for length L = N*a.
    return 2.0 * math.pi * mode / (float(N) * a)


def qhat_bcc_sq(Qz: float, a: float = A_LAT) -> float:
    return (8.0 / (a * a)) * (1.0 - math.cos(0.5 * Qz * a))


def chunk_size(N: int, chunk_bytes: int) -> int:
    bytes_per_plane = N * N * 8
    return max(1, min(N, int(chunk_bytes) // int(bytes_per_plane)))


def strict_pi_for_q(
    N: int,
    Qz: float,
    bz: BzConfig,
    xp,
    chunk_bytes: int,
    m_sq: float,
    components: Iterable[str] = ("x", "y"),
) -> dict[str, float]:
    k1d = k_grid_1d(N, bz, xp)
    norm = integral_norm(N, bz)
    chunk = chunk_size(N, chunk_bytes)
    totals = {component: xp.zeros((), dtype=xp.float64) for component in components}

    ky = k1d[None, :, None]
    kz = k1d[None, None, :]

    for start in range(0, N, chunk):
        stop = min(N, start + chunk)
        kx = k1d[start:stop][:, None, None]
        Dk = denom_bcc(kx, ky, kz, xp, m_sq=m_sq)
        Dkq = denom_bcc(kx, ky, kz + Qz, xp, m_sq=m_sq)
        W = seagull_diag(kx, ky, kz, xp)
        for component in components:
            V = vertex_diag(component, kx, ky, kz, Qz, xp)
            integrand = W / Dk - (V * V) / (Dk * Dkq)
            totals[component] = totals[component] + xp.sum(integrand)

    synchronize(xp)
    return {component: to_float(total, xp) * norm for component, total in totals.items()}


def literal_bubble(
    N: int,
    bz: BzConfig,
    xp,
    chunk_bytes: int,
    m_sq: float,
) -> dict[str, float]:
    k1d = k_grid_1d(N, bz, xp)
    norm = integral_norm(N, bz)
    chunk = chunk_size(N, chunk_bytes)
    totals = {component: xp.zeros((), dtype=xp.float64) for component in ("x", "y", "z")}

    ky = k1d[None, :, None]
    kz = k1d[None, None, :]

    for start in range(0, N, chunk):
        stop = min(N, start + chunk)
        kx = k1d[start:stop][:, None, None]
        Dk = denom_bcc(kx, ky, kz, xp, m_sq=m_sq)
        denom = Dk * Dk
        for component in ("x", "y", "z"):
            V = vertex_diag(component, kx, ky, kz, 0.0, xp)
            totals[component] = totals[component] + xp.sum((V * V) / denom)

    synchronize(xp)
    out = {component: to_float(total, xp) * norm for component, total in totals.items()}
    out["sum"] = out["x"] + out["y"] + out["z"]
    out["avg"] = out["sum"] / 3.0
    return out


def fit_delta_k(qhat2: list[float], delta_vals: list[float]) -> tuple[float, float, float]:
    if len(qhat2) == 1:
        return delta_vals[0], 0.0, 0.0
    x = np.array(qhat2, dtype=float)
    y = np.array(delta_vals, dtype=float)
    slope, intercept = np.polyfit(x, y, 1)
    residual = y - (slope * x + intercept)
    spread = float(np.max(y) - np.min(y))
    rms = float(np.sqrt(np.mean(residual * residual)))
    return float(intercept), float(slope), max(spread, rms)


def classify_residual(residual_ppb: float) -> str:
    ar = abs(residual_ppb)
    if ar < 30.0:
        return "AGREES with Structure-1 under strict primary mapping"
    if ar <= 300.0:
        return "AMBIGUOUS, requires Symanzik-calibrated or MC follow-up"
    return "DOES NOT reproduce Structure-1 closure"


def print_constants(bz: BzConfig) -> None:
    emit("=" * 78)
    emit(" GPU Priority 4: Structure-2 two-U(1) BCC scalar loop")
    emit("=" * 78)
    emit(f"   a                      = {A_LAT:.16g}")
    emit(f"   G*                     = {G_STAR:.16g}")
    emit(f"   theta                  = {THETA:.16g}")
    emit(f"   M^2                    = {M_SQ:.16g}")
    emit(f"   sqrt(M^2)              = {M_SCALAR:.16g}")
    emit(f"   x_tree                 = {X_TREE:.12f}")
    emit(f"   x_-                    = {X_MINUS:.12f}")
    emit(f"   sqrt(x_+ x_-)          = {M_GEOMETRIC:.12f}")
    emit(f"   x_S1                   = {X_S1:.12f}")
    emit(f"   K_PP = K_DD            = {K_PP:.12f}")
    emit(f"   K_PD                   = {K_PD:.12f}")
    emit(f"   handoff mapping factor = {HANDOFF_DERIV:.12f}")
    emit(f"   BZ convention          = {bz.name}  half-width={bz.half_width:.12f}")
    emit()


class RunLogger:
    def __init__(self, out_dir: str | None, run_name: str):
        self.out_dir = Path(out_dir) if out_dir else None
        self.jsonl = None
        self.strict_csv = None
        self.literal_csv = None
        self.strict_writer = None
        self.literal_writer = None
        if self.out_dir:
            self.out_dir.mkdir(parents=True, exist_ok=True)
            self.jsonl = (self.out_dir / f"{run_name}.jsonl").open("w", encoding="utf-8")
            self.strict_csv = (self.out_dir / f"{run_name}_strict_rows.csv").open(
                "w", encoding="utf-8", newline=""
            )
            self.literal_csv = (self.out_dir / f"{run_name}_literal_rows.csv").open(
                "w", encoding="utf-8", newline=""
            )
            self.strict_writer = csv.DictWriter(
                self.strict_csv,
                fieldnames=[
                    "event", "case_id", "bz", "N", "q_item", "Qz", "qhat2",
                    "Pi_xx", "Pi_yy", "dK_x", "dK_y", "dK_avg", "isotropy",
                    "wall_s", "mass", "m_sq", "q_p", "q_d",
                ],
            )
            self.literal_writer = csv.DictWriter(
                self.literal_csv,
                fieldnames=[
                    "event", "case_id", "bz", "N", "Pi_xx", "Pi_yy", "Pi_zz",
                    "Pi_sum", "Pi_avg", "wall_s", "mass", "m_sq", "q_p", "q_d",
                ],
            )
            self.strict_writer.writeheader()
            self.literal_writer.writeheader()
            self.flush()

    def record(self, event: dict) -> None:
        if not self.jsonl:
            return
        self.jsonl.write(json.dumps(event, sort_keys=True) + "\n")
        self.flush()

    def strict_row(self, row: dict) -> None:
        if not self.strict_writer:
            return
        self.strict_writer.writerow(row)
        self.flush()

    def literal_row(self, row: dict) -> None:
        if not self.literal_writer:
            return
        self.literal_writer.writerow(row)
        self.flush()

    def flush(self) -> None:
        for fh in (self.jsonl, self.strict_csv, self.literal_csv):
            if fh:
                fh.flush()

    def close(self) -> None:
        for fh in (self.jsonl, self.strict_csv, self.literal_csv):
            if fh:
                fh.close()


def run_cpu_cross_check(args, bz: BzConfig, q_modes: list[float], case: MatterCase) -> None:
    if args.cpu_check <= 0:
        return
    if cp is None:
        emit("CPU cross-check skipped: CuPy is unavailable.")
        return
    N = int(args.cpu_check)
    Qz = q_from_mode(N, q_modes[0]) if args.q_units == "mode" else float(q_modes[0])
    emit("CPU cross-check:")
    emit(f"   case={case.case_id}, N={N}, Qz={Qz:.8e}, BZ={bz.name}")
    t0 = time.perf_counter()
    cpu = strict_pi_for_q(N, Qz, bz, np, args.chunk_bytes, case.m_sq, ("x", "y"))
    cpu_lit = literal_bubble(N, bz, np, args.chunk_bytes, case.m_sq)
    dt_cpu = time.perf_counter() - t0
    t0 = time.perf_counter()
    gpu = strict_pi_for_q(N, Qz, bz, cp, args.chunk_bytes, case.m_sq, ("x", "y"))
    gpu_lit = literal_bubble(N, bz, cp, args.chunk_bytes, case.m_sq)
    dt_gpu = time.perf_counter() - t0
    emit(f"   strict Pi_xx CPU/GPU: {cpu['x']:+.12e} / {gpu['x']:+.12e}"
          f"  diff={gpu['x'] - cpu['x']:+.3e}")
    emit(f"   strict Pi_yy CPU/GPU: {cpu['y']:+.12e} / {gpu['y']:+.12e}"
          f"  diff={gpu['y'] - cpu['y']:+.3e}")
    emit(f"   literal sum CPU/GPU:  {cpu_lit['sum']:+.12e} / {gpu_lit['sum']:+.12e}"
          f"  diff={gpu_lit['sum'] - cpu_lit['sum']:+.3e}")
    emit(f"   wall CPU/GPU: {dt_cpu:.2f}s / {dt_gpu:.2f}s")
    emit()


def run_strict(
    args,
    bz: BzConfig,
    Ns: list[int],
    q_items: list[float],
    xp,
    case: MatterCase,
    logger: RunLogger,
) -> dict | None:
    emit("=" * 78)
    emit("Strict gauge-invariant transverse kinetic correction")
    emit("=" * 78)
    emit(f"case: {case.case_id} - {case.label}")
    emit(f"backend: {xp_name(xp)}")
    emit()
    header = (
        f"{'N':>6} {'q item':>8} {'Qz':>14} {'qhat^2':>14} "
        f"{'Pi_xx':>16} {'Pi_yy':>16} {'dK_avg':>16} {'iso':>10} {'wall':>8}"
    )
    emit(header)
    emit("-" * len(header))

    last_delta_k = None
    last_N = None
    last_ward = None

    for N in Ns:
        tN = time.perf_counter()
        ward = strict_pi_for_q(N, 0.0, bz, xp, args.chunk_bytes, case.m_sq, ("x", "y"))
        last_ward = ward
        qhat2_vals = []
        dK_vals = []

        for q_item in q_items:
            Qz = q_from_mode(N, q_item) if args.q_units == "mode" else float(q_item)
            qh2 = qhat_bcc_sq(Qz)
            t0 = time.perf_counter()
            pi = strict_pi_for_q(N, Qz, bz, xp, args.chunk_bytes, case.m_sq, ("x", "y"))
            dt = time.perf_counter() - t0
            dK_x = pi["x"] / qh2
            dK_y = pi["y"] / qh2
            dK_avg = 0.5 * (dK_x + dK_y)
            iso = abs(pi["x"] - pi["y"]) / max(abs(pi["x"]), abs(pi["y"]), 1e-300)
            qhat2_vals.append(qh2)
            dK_vals.append(dK_avg)
            emit(
                f"{N:6d} {q_item:8g} {Qz:14.6e} {qh2:14.6e} "
                f"{pi['x']:+16.8e} {pi['y']:+16.8e} {dK_avg:+16.8e} "
                f"{iso:10.2e} {dt:8.2f}"
            )
            row = {
                "event": "strict_row", "case_id": case.case_id, "bz": bz.name,
                "N": N, "q_item": q_item, "Qz": Qz, "qhat2": qh2,
                "Pi_xx": pi["x"], "Pi_yy": pi["y"],
                "dK_x": dK_x, "dK_y": dK_y, "dK_avg": dK_avg,
                "isotropy": iso, "wall_s": dt, "mass": case.mass,
                "m_sq": case.m_sq, "q_p": case.q_p, "q_d": case.q_d,
            }
            logger.strict_row(row)
            logger.record(row)

        intercept, slope, stability = fit_delta_k(qhat2_vals, dK_vals)
        last_delta_k = intercept
        last_N = N
        wall_N = time.perf_counter() - tN
        emit(
            f"       Ward Pi_xx(0)={ward['x']:+.8e}  Pi_yy(0)={ward['y']:+.8e}  "
            f"linear dK(Q->0)={intercept:+.8e}  slope={slope:+.8e}  "
            f"stability={stability:.3e}  N wall={wall_N:.2f}s"
        )
        logger.record({
            "event": "strict_N_summary", "case_id": case.case_id, "bz": bz.name,
            "N": N, "ward_x": ward["x"], "ward_y": ward["y"],
            "delta_K_unit": intercept, "slope": slope, "stability": stability,
            "wall_s": wall_N, "mass": case.mass, "m_sq": case.m_sq,
            "q_p": case.q_p, "q_d": case.q_d,
        })
        emit()

    if last_delta_k is None or last_N is None or last_ward is None:
        return None

    delta_x_primary = case.x_plus_factor * last_delta_k
    delta_x_dark = case.x_minus_factor * last_delta_k
    delta_x_handoff = HANDOFF_DERIV * case.pp_factor * last_delta_k
    x_s2_primary = X_TREE + delta_x_primary
    x_s2_handoff = X_TREE + delta_x_handoff
    residual_primary_ppb = (x_s2_primary - X_S1) / X_TREE * 1e9
    residual_handoff_ppb = (x_s2_handoff - X_S1) / X_TREE * 1e9
    ward_abs = max(abs(last_ward["x"]), abs(last_ward["y"]))

    verdict = classify_residual(residual_primary_ppb)

    emit("Strict result using largest N:")
    emit(f"   case                      = {case.case_id} - {case.label}")
    emit(f"   N                         = {last_N}")
    emit(f"   unit-charge delta_K       = {last_delta_k:+.12e}")
    emit(f"   x_+ charge factor         = {case.x_plus_factor:.12f}")
    emit(f"   x_- charge factor         = {case.x_minus_factor:.12f}")
    emit(f"   Ward max |Pi_ii(0)|       = {ward_abs:.3e}")
    emit(f"   primary delta_x_plus      = {delta_x_primary:+.12e}")
    emit(f"   primary delta_x_minus     = {delta_x_dark:+.12e}")
    emit(f"   primary x_S2              = {x_s2_primary:.12f}")
    emit(f"   primary residual vs S1    = {residual_primary_ppb:+.3f} ppb")
    emit(f"   primary verdict           = {verdict}")
    emit(f"   handoff PP-only delta_x   = {delta_x_handoff:+.12e}")
    emit(f"   handoff-mapped x_S2       = {x_s2_handoff:.12f}")
    emit(f"   handoff residual vs S1    = {residual_handoff_ppb:+.3f} ppb")
    if ward_abs > args.ward_tol:
        emit()
        emit("   VALIDATION WARNING:")
        emit(f"   Ward check exceeds tolerance {args.ward_tol:.1e}.")
        emit("   Treat the strict physics classification as invalid until the")
        emit("   BZ convention or Peierls-link normalization is reconciled.")
    emit()
    result = {
        "event": "strict_case_result", "case_id": case.case_id, "label": case.label,
        "bz": bz.name, "N": last_N, "unit_delta_K": last_delta_k,
        "x_plus_factor": case.x_plus_factor, "x_minus_factor": case.x_minus_factor,
        "ward_abs": ward_abs, "primary_delta_x": delta_x_primary,
        "primary_delta_x_minus": delta_x_dark, "primary_x_S2": x_s2_primary,
        "primary_residual_ppb": residual_primary_ppb, "primary_verdict": verdict,
        "handoff_delta_x": delta_x_handoff, "handoff_x_S2": x_s2_handoff,
        "handoff_residual_ppb": residual_handoff_ppb, "mass": case.mass,
        "m_sq": case.m_sq, "q_p": case.q_p, "q_d": case.q_d,
    }
    logger.record(result)
    return result


def run_literal(
    args,
    bz: BzConfig,
    Ns: list[int],
    xp,
    case: MatterCase,
    logger: RunLogger,
) -> dict | None:
    emit("=" * 78)
    emit("Literal q=0 bubble-only diagnostic")
    emit("=" * 78)
    emit(f"case: {case.case_id} - {case.label}")
    emit(f"backend: {xp_name(xp)}")
    emit()
    header = (
        f"{'N':>6} {'Pi_xx':>16} {'Pi_yy':>16} {'Pi_zz':>16} "
        f"{'sum':>16} {'avg':>16} {'wall':>8}"
    )
    emit(header)
    emit("-" * len(header))
    last = None
    last_N = None
    for N in Ns:
        t0 = time.perf_counter()
        out = literal_bubble(N, bz, xp, args.chunk_bytes, case.m_sq)
        out = {key: value * case.pp_factor for key, value in out.items()}
        dt = time.perf_counter() - t0
        last = out
        last_N = N
        emit(
            f"{N:6d} {out['x']:+16.8e} {out['y']:+16.8e} {out['z']:+16.8e} "
            f"{out['sum']:+16.8e} {out['avg']:+16.8e} {dt:8.2f}"
        )
        row = {
            "event": "literal_row", "case_id": case.case_id, "bz": bz.name,
            "N": N, "Pi_xx": out["x"], "Pi_yy": out["y"], "Pi_zz": out["z"],
            "Pi_sum": out["sum"], "Pi_avg": out["avg"], "wall_s": dt,
            "mass": case.mass, "m_sq": case.m_sq, "q_p": case.q_p,
            "q_d": case.q_d,
        }
        logger.literal_row(row)
        logger.record(row)
    emit()

    if last is None or last_N is None:
        return None
    delta_x_sum = HANDOFF_DERIV * last["sum"]
    delta_x_avg = HANDOFF_DERIV * last["avg"]
    x_sum = X_TREE + delta_x_sum
    x_avg = X_TREE + delta_x_avg
    res_sum_ppb = (x_sum - X_S1) / X_TREE * 1e9
    res_avg_ppb = (x_avg - X_S1) / X_TREE * 1e9
    emit("Literal diagnostic using largest N:")
    emit(f"   case                      = {case.case_id} - {case.label}")
    emit(f"   N                         = {last_N}")
    emit(f"   Pi_bubble_sum             = {last['sum']:+.12e}")
    emit(f"   Pi_bubble_avg             = {last['avg']:+.12e}")
    emit(f"   handoff x from sum        = {x_sum:.12f}")
    emit(f"   handoff residual from sum = {res_sum_ppb:+.3f} ppb")
    emit(f"   handoff x from avg        = {x_avg:.12f}")
    emit(f"   handoff residual from avg = {res_avg_ppb:+.3f} ppb")
    emit("   classification            = diagnostic only, not gauge-invariant")
    emit()
    result = {
        "event": "literal_case_result", "case_id": case.case_id, "label": case.label,
        "bz": bz.name, "N": last_N, "Pi_bubble_sum": last["sum"],
        "Pi_bubble_avg": last["avg"], "handoff_x_sum": x_sum,
        "handoff_residual_sum_ppb": res_sum_ppb, "handoff_x_avg": x_avg,
        "handoff_residual_avg_ppb": res_avg_ppb, "mass": case.mass,
        "m_sq": case.m_sq, "q_p": case.q_p, "q_d": case.q_d,
    }
    logger.record(result)
    return result


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("--mode", choices=("strict", "literal", "both"), default="both")
    p.add_argument("--N", default="64,128,256,512,1024,2048",
                   help="comma-separated N ladder or a single N")
    p.add_argument("--q-list", default="1,2,3,4",
                   help="comma-separated q modes by default, or absolute Qz with --q-units absolute")
    p.add_argument("--q-units", choices=("mode", "absolute"), default="mode")
    p.add_argument("--chunk-bytes", type=int, default=int(4e9))
    p.add_argument("--cpu-check", type=int, default=16,
                   help="small N for CPU/GPU cross-check; set 0 to skip")
    p.add_argument("--bz", choices=("framework", "periodic"), default="periodic")
    p.add_argument("--ward-tol", type=float, default=1e-8)
    p.add_argument("--cases", default="S2-A",
                   help="comma-separated fixed cases from S2-A,S2-B,S2-C,S2-D,S2-E")
    p.add_argument("--out-dir", default="scripts/exploration/outputs",
                   help="write streaming CSV/JSONL logs here; set empty string to disable")
    p.add_argument("--run-name", default=None,
                   help="base filename for logs; default auto-generates one")
    args = p.parse_args()

    if cp is None:
        raise RuntimeError(f"CuPy is required for the GPU run: {CUPY_IMPORT_ERROR}")

    Ns = [int(v) for v in parse_csv_numbers(args.N, int)]
    q_items = [float(v) for v in parse_csv_numbers(args.q_list, float)]
    case_ids = parse_case_ids(args.cases)
    bz = bz_config(args.bz)
    cp.cuda.Device(0).use()
    run_name = args.run_name or (
        f"priority4_{bz.name}_{args.mode}_{'_'.join(case_ids)}_"
        f"N{'-'.join(str(n) for n in Ns)}"
    )
    logger = RunLogger(args.out_dir or None, run_name)

    try:
        print_constants(bz)
        emit(f"   GPU                    = {cp.cuda.runtime.getDeviceProperties(0)['name'].decode()}")
        emit(f"   N ladder               = {Ns}")
        emit(f"   q-list                 = {q_items} ({args.q_units})")
        emit(f"   fixed cases            = {case_ids}")
        emit(f"   chunk bytes            = {args.chunk_bytes}")
        if args.out_dir:
            emit(f"   log dir                = {Path(args.out_dir).resolve()}")
            emit(f"   run name               = {run_name}")
        emit()
        logger.record({
            "event": "run_start", "bz": bz.name, "mode": args.mode,
            "Ns": Ns, "q_items": q_items, "q_units": args.q_units,
            "case_ids": case_ids, "chunk_bytes": args.chunk_bytes,
            "run_name": run_name,
        })

        strict_results = []
        literal_results = []
        for case_id in case_ids:
            case = PRESET_CASES[case_id]
            emit("#" * 78)
            emit(f"CASE {case.case_id}: {case.label}")
            emit(f"   q=(P={case.q_p:g}, D={case.q_d:g}), M={case.mass:.12f}, M^2={case.m_sq:.12f}")
            emit(f"   x_+ factor={case.x_plus_factor:.12f}, x_- factor={case.x_minus_factor:.12f}")
            emit(f"   note: {case.note}")
            emit("#" * 78)
            emit()
            if args.mode in ("strict", "both"):
                run_cpu_cross_check(args, bz, q_items, case)
                result = run_strict(args, bz, Ns, q_items, cp, case, logger)
                if result:
                    strict_results.append(result)
            if args.mode in ("literal", "both"):
                result = run_literal(args, bz, Ns, cp, case, logger)
                if result:
                    literal_results.append(result)

        if strict_results:
            emit("=" * 78)
            emit("Strict matter-case summary")
            emit("=" * 78)
            emit(f"{'case':<6} {'N':>6} {'unit dK':>14} {'x+ factor':>10} {'x_S2':>16} {'res ppb':>12}  verdict")
            emit("-" * 78)
            for result in strict_results:
                emit(
                    f"{result['case_id']:<6} {result['N']:6d} "
                    f"{result['unit_delta_K']:+14.6e} "
                    f"{result['x_plus_factor']:10.4f} "
                    f"{result['primary_x_S2']:16.9f} "
                    f"{result['primary_residual_ppb']:12.3f}  "
                    f"{result['primary_verdict']}"
                )
            emit()
        logger.record({"event": "run_end", "strict_results": strict_results, "literal_results": literal_results})
    finally:
        logger.close()


if __name__ == "__main__":
    main()
