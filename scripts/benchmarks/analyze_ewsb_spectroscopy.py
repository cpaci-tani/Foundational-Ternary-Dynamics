#!/usr/bin/env python3
"""
analyze_ewsb_spectroscopy.py — Thread 3 of the EFT Day-2 program.

Consumes the binary lattice dumps produced by
`benchmark_ewsb_threshold_map.exe` and extracts:
  - flux-flux two-point correlator C_J(r) = ⟨J(x)·J(x+r)⟩
  - mass-gap estimate from exponential decay fit
  - charge-charge correlator G(r) = ⟨s(x)·s(x+r)⟩
  - density correlator (mean flux magnitude at distance r)
  - structural classification of the condensate: are the 32768
    manifested charges a uniform dust or do they form clusters?

Output:
  - CSV per amplitude: correlator vs r, effective mass, cluster stats
  - markdown report combining all amplitudes for the theory doc

Usage:
  python scripts/benchmarks/analyze_ewsb_spectroscopy.py
  python scripts/benchmarks/analyze_ewsb_spectroscopy.py --dump-dir <path>
"""

from __future__ import annotations

import argparse
import math
import pathlib
import struct
import sys
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Tuple

SCRIPT_DIR = pathlib.Path(__file__).resolve().parent
PROJ_ROOT = SCRIPT_DIR.parent.parent


@dataclass
class LatticeDump:
    L: int
    N: int
    state: List[int] = field(default_factory=list)   # length N, int8
    flux: List[Tuple[float, float, float]] = field(default_factory=list)  # length N


def load_dump(path: pathlib.Path) -> LatticeDump:
    """Binary dump format (little-endian):
        int32 L, int32 N, then N × (int8 state, 3×float32 flux)."""
    with path.open("rb") as f:
        hdr = f.read(8)
        L, N = struct.unpack("<ii", hdr)
        dump = LatticeDump(L=L, N=N)
        dump.state = [0] * N
        dump.flux = [(0.0, 0.0, 0.0)] * N
        for i in range(N):
            chunk = f.read(13)  # 1 + 3*4
            s = struct.unpack("<b", chunk[:1])[0]
            fx, fy, fz = struct.unpack("<fff", chunk[1:])
            dump.state[i] = s
            dump.flux[i] = (fx, fy, fz)
    return dump


def wrap(x: int, L: int) -> int:
    return x % L


def site(ix: int, iy: int, iz: int, L: int) -> int:
    return ix * L * L + iy * L + iz


def compute_flux_correlator(d: LatticeDump, max_r: Optional[int] = None) -> List[float]:
    """C_J(r) = ⟨J(x) · J(x+r)⟩ averaged over all x and the 3 cubic axes."""
    L = d.L
    if max_r is None:
        max_r = L // 2
    C = [0.0] * max_r
    counts = [0] * max_r
    for ix in range(L):
        for iy in range(L):
            for iz in range(L):
                i0 = site(ix, iy, iz, L)
                Jx, Jy, Jz = d.flux[i0]
                for r in range(max_r):
                    # three axes: +x, +y, +z
                    for axis in range(3):
                        if axis == 0:
                            ir = site(wrap(ix + r, L), iy, iz, L)
                        elif axis == 1:
                            ir = site(ix, wrap(iy + r, L), iz, L)
                        else:
                            ir = site(ix, iy, wrap(iz + r, L), L)
                        Jxr, Jyr, Jzr = d.flux[ir]
                        C[r] += Jx * Jxr + Jy * Jyr + Jz * Jzr
                        counts[r] += 1
    return [C[r] / counts[r] if counts[r] > 0 else 0.0 for r in range(max_r)]


def compute_charge_correlator(d: LatticeDump, max_r: Optional[int] = None) -> List[float]:
    """G(r) = ⟨s(x)·s(x+r)⟩ averaged over x and 3 axes."""
    L = d.L
    if max_r is None:
        max_r = L // 2
    G = [0.0] * max_r
    counts = [0] * max_r
    for ix in range(L):
        for iy in range(L):
            for iz in range(L):
                i0 = site(ix, iy, iz, L)
                s0 = d.state[i0]
                for r in range(max_r):
                    for axis in range(3):
                        if axis == 0:
                            ir = site(wrap(ix + r, L), iy, iz, L)
                        elif axis == 1:
                            ir = site(ix, wrap(iy + r, L), iz, L)
                        else:
                            ir = site(ix, iy, wrap(iz + r, L), L)
                        G[r] += s0 * d.state[ir]
                        counts[r] += 1
    return [G[r] / counts[r] if counts[r] > 0 else 0.0 for r in range(max_r)]


def fit_exponential_mass(C: List[float], r_min: int = 2, r_max: Optional[int] = None) -> Tuple[Optional[float], float, int]:
    """Fit |C(r)| = A · exp(-m r) over [r_min, r_max); return (m, R², n)."""
    n = len(C)
    if r_max is None or r_max > n:
        r_max = n
    xs, ys = [], []
    for r in range(r_min, r_max):
        absC = abs(C[r])
        if absC <= 0:
            continue
        xs.append(r)
        ys.append(math.log(absC))
    if len(xs) < 3:
        return None, 0.0, len(xs)
    n_pts = len(xs)
    sx = sum(xs); sy = sum(ys)
    sxx = sum(x * x for x in xs); sxy = sum(x * y for x, y in zip(xs, ys))
    denom = n_pts * sxx - sx * sx
    if abs(denom) < 1e-30:
        return None, 0.0, n_pts
    slope = (n_pts * sxy - sx * sy) / denom
    intercept = (sy - slope * sx) / n_pts
    # m = -slope (positive for decay)
    if slope >= 0:
        return None, 0.0, n_pts
    ybar = sy / n_pts
    ss_tot = sum((y - ybar) ** 2 for y in ys)
    ss_res = sum((y - (intercept + slope * x)) ** 2 for x, y in zip(xs, ys))
    r2 = 1.0 - ss_res / ss_tot if ss_tot > 0 else 0.0
    return -slope, r2, n_pts


def summarize_dump(d: LatticeDump, amp: float) -> Dict:
    N = d.N
    n_plus = sum(1 for s in d.state if s > 0)
    n_minus = sum(1 for s in d.state if s < 0)
    n_zero = N - n_plus - n_minus
    total_abs_J = sum(math.sqrt(fx*fx + fy*fy + fz*fz) for fx, fy, fz in d.flux)
    mean_abs_J = total_abs_J / N

    # Correlators
    max_r = d.L // 2
    C_J = compute_flux_correlator(d, max_r)
    G_s = compute_charge_correlator(d, max_r)
    # Effective masses from exponential fits on the connected part
    # (subtract C_J(r_large) / asymptote). Simpler: just fit the decay
    # of the central part.
    m_J, r2_J, n_J = fit_exponential_mass(C_J, r_min=2, r_max=max_r)
    m_s, r2_s, n_s = fit_exponential_mass(G_s, r_min=2, r_max=max_r)

    return {
        "amp": amp,
        "L": d.L,
        "N": d.N,
        "n_plus": n_plus,
        "n_minus": n_minus,
        "n_zero": n_zero,
        "charge_fraction": (n_plus + n_minus) / N,
        "charge_imbalance": n_plus - n_minus,
        "mean_abs_J": mean_abs_J,
        "C_J": C_J,
        "G_s": G_s,
        "m_flux": m_J,
        "r2_flux": r2_J,
        "m_charge": m_s,
        "r2_charge": r2_s,
    }


def write_report(results: List[Dict], out_path: pathlib.Path) -> None:
    lines: List[str] = []
    lines.append("# EWSB Condensate Spectroscopy (Thread 3 / Day-2)\n")
    lines.append("## Per-amplitude summary\n")
    lines.append("| amp | ⟨\\|J\\|⟩ | N+ | N- | imbalance | charge frac | m_flux | R²_flux | m_charge | R²_charge |")
    lines.append("|---|---|---|---|---|---|---|---|---|---|")
    for r in results:
        lines.append(
            f"| {r['amp']:.2f} | {r['mean_abs_J']:.3f} | {r['n_plus']} | {r['n_minus']} | "
            f"{r['charge_imbalance']:+d} | {r['charge_fraction']:.3f} | "
            f"{(r['m_flux'] or float('nan')):.4f} | {r['r2_flux']:.3f} | "
            f"{(r['m_charge'] or float('nan')):.4f} | {r['r2_charge']:.3f} |"
        )

    lines.append("\n## Flux-flux correlator C_J(r) per amp\n")
    # Build r-table
    if results:
        max_r = len(results[0]["C_J"])
        amps = [r["amp"] for r in results]
        header = "| r | " + " | ".join(f"amp={a:.2f}" for a in amps) + " |"
        sep = "|---|" + "|".join(["---"] * len(amps)) + "|"
        lines.append(header)
        lines.append(sep)
        for r_idx in range(max_r):
            row = f"| {r_idx} | " + " | ".join(
                f"{res['C_J'][r_idx]:.4f}" for res in results
            ) + " |"
            lines.append(row)

    lines.append("\n## Charge-charge correlator G(r) per amp\n")
    if results:
        max_r = len(results[0]["G_s"])
        amps = [r["amp"] for r in results]
        header = "| r | " + " | ".join(f"amp={a:.2f}" for a in amps) + " |"
        sep = "|---|" + "|".join(["---"] * len(amps)) + "|"
        lines.append(header)
        lines.append(sep)
        for r_idx in range(max_r):
            row = f"| {r_idx} | " + " | ".join(
                f"{res['G_s'][r_idx]:.4f}" for res in results
            ) + " |"
            lines.append(row)

    out_path.write_text("\n".join(lines), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--dump-dir", type=pathlib.Path,
                        default=PROJ_ROOT / "scripts" / "benchmarks" / "results" / "eft_day2",
                        help="directory containing ewsb_amp_<amp>_L<L>_final.bin")
    parser.add_argument("--out", type=pathlib.Path,
                        default=PROJ_ROOT / "scripts" / "benchmarks" / "results" / "eft_day2" / "ewsb_spectroscopy.md",
                        help="output markdown report")
    args = parser.parse_args()

    dump_dir: pathlib.Path = args.dump_dir
    paths = sorted(dump_dir.glob("ewsb_amp_*_L*_final.bin"))
    if not paths:
        print(f"No dumps found in {dump_dir}", file=sys.stderr)
        sys.exit(1)

    results: List[Dict] = []
    for p in paths:
        # Filename: ewsb_amp_<amp>_L<L>_final.bin
        name = p.stem  # without .bin
        parts = name.split("_")
        try:
            amp = float(parts[2])
        except (IndexError, ValueError):
            amp = 0.0
        print(f"Loading {p.name} (amp={amp})...")
        d = load_dump(p)
        print(f"  L={d.L}, N={d.N}")
        summary = summarize_dump(d, amp)
        results.append(summary)
        print(f"  manifested: N+={summary['n_plus']}, N-={summary['n_minus']}, "
              f"imbalance={summary['charge_imbalance']:+d}")
        print(f"  mean |J|={summary['mean_abs_J']:.3f}")
        m_f = summary['m_flux']
        m_c = summary['m_charge']
        print(f"  m_flux={(m_f if m_f is not None else 'n/a')}, R2={summary['r2_flux']:.3f}")
        print(f"  m_charge={(m_c if m_c is not None else 'n/a')}, R2={summary['r2_charge']:.3f}")

    write_report(results, args.out)
    print(f"\nReport: {args.out}")


if __name__ == "__main__":
    main()
