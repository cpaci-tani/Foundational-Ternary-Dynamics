"""
Phase II.5 g-2 measurement and Schwinger comparison.

Pre-reg: docs/theory/10_eft_program/PREREG_PHASE_II_WILSON_DIRAC_G2.md
Spec:    docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md

Reads the time series produced by `engine/tests/benchmark_dirac_electron_in_B.cpp`,
extracts the spin-precession frequency omega_s via FFT, computes
    a_e_lattice = omega_s / omega_c_classical - 1
where omega_c_classical = qB/m is the analytic non-relativistic cyclotron
frequency, and compares against the QED tree-level Schwinger anomaly
    a_e_Schwinger = alpha / (2 pi)
with alpha = 1/x_+ taken from the master-quadratic FTD-native coupling
(FTD-0125 [DERIVED]).

Verdict per pre-registered outcome table (PREREG_PHASE_II §4):
    A: rel_err < 5%       -> Schwinger reproduction
    B: 5% <= rel_err < 50% -> partial agreement
    C: rel_err >= 50%      -> no detectable Schwinger anomaly

Usage:
    python proof_phase_ii_g_minus_2.py <csv_path> [options]
    python proof_phase_ii_g_minus_2.py --run        # rebuilds + runs benchmark
"""

from __future__ import annotations

import argparse
import csv
import math
import os
import subprocess
import sys
from pathlib import Path

import numpy as np

REPO_ROOT = Path(__file__).resolve().parent.parent.parent
DEFAULT_CSV = REPO_ROOT / "engine" / "build" / "orbit_static.csv"
BENCHMARK_EXE = REPO_ROOT / "engine" / "build" / "Release" / "benchmark_dirac_electron_in_B.exe"

# Master-quadratic FTD-native coupling (FTD-0125 [DERIVED]).
# x_+ = 8 G^*^2 + 4 G^* * sqrt(4 G^*^2 - G^*) where G^* = Gamma(1/4)/Gamma(3/4).
# Numerically: x_+ ≈ 137.0359990368... = 1/alpha_FTD; alpha_FTD = 1/x_+ ≈ 0.0072973525...
ALPHA_FTD = 1.0 / 137.0359990368
A_E_SCHWINGER = ALPHA_FTD / (2.0 * math.pi)


def run_benchmark(L: int, n_flux: int, m: float, sigma: float,
                  n_steps: int, dt: float, csv_path: Path) -> int:
    """Run the C++ benchmark with stationary electron (p_y_units=0)."""
    if not BENCHMARK_EXE.exists():
        print(f"ERROR: benchmark executable not found: {BENCHMARK_EXE}", file=sys.stderr)
        print("Build with: cmake --build engine/build --config Release "
              "--target benchmark_dirac_electron_in_B", file=sys.stderr)
        return 1

    cmd = [
        str(BENCHMARK_EXE),
        "--L", str(L),
        "--n_flux", str(n_flux),
        "--m", str(m),
        "--sigma", str(sigma),
        "--p_y_units", "0",
        "--steps", str(n_steps),
        "--dt", str(dt),
        "--csv", str(csv_path),
    ]
    print("Running benchmark:", " ".join(cmd))
    result = subprocess.run(cmd, capture_output=True, text=True)
    print(result.stdout)
    if result.returncode != 0:
        print("Benchmark stderr:", result.stderr, file=sys.stderr)
    return result.returncode


def load_csv(csv_path: Path) -> dict[str, np.ndarray]:
    """Load orbit CSV into named numpy arrays."""
    with open(csv_path) as f:
        rows = list(csv.DictReader(f))
    if not rows:
        raise RuntimeError(f"empty CSV: {csv_path}")
    return {col: np.array([float(r[col]) for r in rows]) for col in rows[0].keys()}


def fft_peak_frequency(t: np.ndarray, signal: np.ndarray,
                       n_top: int = 5) -> tuple[float, list[tuple[float, float]]]:
    """Return (peak_omega, top_n_modes) from real-FFT power spectrum.
    Modes are sorted by descending power; mode 0 (DC) is excluded.
    """
    centered = signal - signal.mean()
    n = len(t)
    dt = float(t[1] - t[0])
    fft = np.fft.fft(centered)
    freqs = np.fft.fftfreq(n, dt)
    mask = freqs > 0
    power = np.abs(fft[mask]) ** 2
    freqs_pos = freqs[mask]
    sorted_idx = np.argsort(power)[::-1][:n_top]
    modes = [(2.0 * math.pi * float(freqs_pos[i]), float(power[i])) for i in sorted_idx]
    return modes[0][0], modes


def analyze(csv_path: Path, L: int, n_flux: int, m: float, verbose: bool = True) -> dict:
    """Extract omega_s, compute a_e_lattice, return verdict dict."""
    data = load_csv(csv_path)
    t = data["t"]
    sx = data["sx"]
    sy = data["sy"]

    # Cyclotron frequency from analytic non-relativistic prediction:
    # qB = phase per plaquette = 2 pi n_flux / L^2 (in lattice units, a=1).
    qB = 2.0 * math.pi * n_flux / (L * L)
    omega_c_classical = qB / m

    # Spin precession frequency from FFT. Use sx since it has full initial
    # excursion (sx(0)=1). sy peaks at quarter-period (small initial value
    # but identical FFT structure since sx, sy are pi/2-related under pure
    # precession).
    omega_s_sx, modes_sx = fft_peak_frequency(t, sx, n_top=5)

    a_e_lattice = omega_s_sx / omega_c_classical - 1.0
    rel_err = abs(a_e_lattice - A_E_SCHWINGER) / A_E_SCHWINGER

    if rel_err < 0.05:
        verdict = "A: SCHWINGER MATCH"
    elif rel_err < 0.50:
        verdict = "B: SCHWINGER NEAR-MATCH"
    else:
        verdict = "C: SCHWINGER MISS"

    if verbose:
        print()
        print("=" * 72)
        print("Phase II.5 g-2 measurement and Schwinger comparison")
        print("=" * 72)
        print(f"Input: {csv_path}  ({len(t)} samples, t in [0, {t[-1]:.2f}])")
        print(f"Lattice: L={L}, n_flux={n_flux}, m={m}")
        print(f"qB (per plaquette) = {qB:.6e}")
        print()
        print("Cyclotron prediction (non-rel, Dirac g=2):")
        print(f"  omega_c_classical = qB / m = {omega_c_classical:.6e}")
        print(f"  period             = 2 pi / omega_c = {2*math.pi/omega_c_classical:.4f}")
        print()
        print("Spin precession (measured from sx FFT, top 5 modes):")
        for i, (om, pw) in enumerate(modes_sx):
            tag = "  <- selected" if i == 0 else ""
            print(f"  rank {i+1}: omega = {om:.6e}  power = {pw:.4e}{tag}")
        print()
        print(f"  selected omega_s = {omega_s_sx:.6e}")
        print(f"  ratio g_lattice/2 = omega_s/omega_c = {omega_s_sx/omega_c_classical:.6f}")
        print()
        print("Anomaly extraction:")
        print(f"  a_e_lattice  = omega_s / omega_c - 1 = {a_e_lattice:.6e}")
        print(f"  a_e_Schwinger = alpha_FTD / (2 pi)   = {A_E_SCHWINGER:.6e}")
        print(f"  alpha_FTD = 1/x_+                     = {ALPHA_FTD:.10f}")
        print(f"  rel_err = |a_lattice - a_Schwinger| / a_Schwinger = {rel_err:.4f}")
        print()
        print(f"Pre-registered outcome verdict: {verdict}")
        print("=" * 72)

    return {
        "csv_path": str(csv_path),
        "L": L,
        "n_flux": n_flux,
        "m": m,
        "qB": qB,
        "omega_c_classical": omega_c_classical,
        "omega_s_measured": omega_s_sx,
        "fft_top5_modes": modes_sx,
        "g_lattice_over_2": omega_s_sx / omega_c_classical,
        "a_e_lattice": a_e_lattice,
        "a_e_Schwinger": A_E_SCHWINGER,
        "alpha_FTD": ALPHA_FTD,
        "rel_err": rel_err,
        "verdict": verdict,
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--csv", type=Path, default=DEFAULT_CSV,
                        help=f"path to orbit CSV (default: {DEFAULT_CSV})")
    parser.add_argument("--L", type=int, default=24)
    parser.add_argument("--n_flux", type=int, default=4)
    parser.add_argument("--m", type=float, default=1.0)
    parser.add_argument("--sigma", type=float, default=1.5)
    parser.add_argument("--steps", type=int, default=2000)
    parser.add_argument("--dt", type=float, default=0.04)
    parser.add_argument("--run", action="store_true",
                        help="run the benchmark before analysis")
    args = parser.parse_args()

    if args.run:
        rc = run_benchmark(args.L, args.n_flux, args.m, args.sigma,
                           args.steps, args.dt, args.csv)
        if rc != 0:
            sys.exit(rc)

    if not args.csv.exists():
        print(f"ERROR: CSV not found: {args.csv}", file=sys.stderr)
        print("Pass --run to generate, or specify --csv <path>", file=sys.stderr)
        sys.exit(1)

    result = analyze(args.csv, args.L, args.n_flux, args.m)

    # Exit code reflects the verdict (0 = outcome A; 1 = outcome B/C).
    if result["verdict"].startswith("A"):
        sys.exit(0)
    else:
        sys.exit(1)


if __name__ == "__main__":
    main()
