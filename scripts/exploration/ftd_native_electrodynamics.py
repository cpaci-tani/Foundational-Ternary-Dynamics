#!/usr/bin/env python3
"""Fixed native electrodynamics probes for the FTD engine stencil.

This script measures quantities produced by the engine-native 18-point Moore
Laplacian. It does not compare against alpha, fit parameters, or search for
near misses. All reported numbers are in the explicit normalization stated
below.

Operator convention:

    lap f = (1/3) * face_sum + (1/6) * edge_sum - 4 f
    sigma_18(k) = -lap_symbol(k)

The long-wavelength response is normalized by sigma_18(k) = |k|^2 + O(k^4).
"""

from __future__ import annotations

import argparse
import json
import math
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable

import numpy as np


C_SPEED = 1.0 / math.sqrt(3.0)


def sigma18(kx, ky, kz):
    """Positive Fourier symbol for the engine 18-point Laplacian."""
    cx = np.cos(kx)
    cy = np.cos(ky)
    cz = np.cos(kz)
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz) - (2.0 / 3.0) * (
        cx * cy + cx * cz + cy * cz
    )


def omega18(kx: float, ky: float, kz: float) -> float:
    """Leapfrog dispersion: 4 sin^2(omega/2) = c^2 sigma_18(k)."""
    sig = float(sigma18(kx, ky, kz))
    arg = 0.5 * math.sqrt(max(0.0, C_SPEED * C_SPEED * sig))
    return 2.0 * math.asin(min(1.0, arg))


def parse_int_list(text: str) -> list[int]:
    values = []
    for part in text.split(","):
        part = part.strip()
        if part:
            value = int(part)
            if value <= 1:
                raise argparse.ArgumentTypeError("N values must be > 1")
            values.append(value)
    if not values:
        raise argparse.ArgumentTypeError("empty list")
    return values


def parse_optional_int_list(text: str | None) -> list[int] | None:
    if text is None or text.strip() == "":
        return None
    values = parse_int_list(text)
    if any(value < 1 for value in values):
        raise argparse.ArgumentTypeError("r values must be >= 1")
    return values


def unique_axis_radii(n: int) -> list[int]:
    candidates = [n // 32, n // 24, n // 16, n // 12, n // 8, n // 6, n // 4]
    radii = sorted({r for r in candidates if 1 <= r <= n // 4})
    return radii


def axis_radii_for_n(n: int, requested: list[int] | None) -> list[int]:
    if requested is None:
        return unique_axis_radii(n)
    return [r for r in requested if r <= n // 4]


def high_symmetry_summary() -> dict[str, float]:
    points = {
        "000": (0.0, 0.0, 0.0),
        "pi00": (math.pi, 0.0, 0.0),
        "pipi0": (math.pi, math.pi, 0.0),
        "pipipi": (math.pi, math.pi, math.pi),
    }
    return {name: float(sigma18(*coords)) for name, coords in points.items()}


def mode_summary(n: int) -> list[dict[str, object]]:
    modes = [(1, 0, 0), (1, 1, 0), (1, 1, 1), (2, 1, 0), (2, 1, 1)]
    out = []
    for mx, my, mz in modes:
        kx = 2.0 * math.pi * mx / n
        ky = 2.0 * math.pi * my / n
        kz = 2.0 * math.pi * mz / n
        q2 = kx * kx + ky * ky + kz * kz
        q = math.sqrt(q2)
        sig = float(sigma18(kx, ky, kz))
        omega = omega18(kx, ky, kz)
        out.append(
            {
                "mode": [mx, my, mz],
                "q": q,
                "sigma_over_q2": sig / q2,
                "omega": omega,
                "phase_speed": omega / q,
                "phase_speed_over_c": omega / (C_SPEED * q),
            }
        )
    return out


def green_response(n: int, radii: Iterable[int]) -> dict[str, object]:
    """Compute BZ average G(0) and axis Green response G(r,0,0).

    The convention is

        G(r) = (1/N^3) sum_{k != 0} cos(k_x r) / sigma_18(k).

    The zero mode is omitted because the periodic Laplacian has a constant
    null mode. In the infinite-volume continuum limit, 4 pi r G(r) -> 1.
    """
    k = 2.0 * math.pi * np.fft.fftfreq(n)
    ky, kz = np.meshgrid(k, k, indexing="ij")
    radii = list(radii)
    axis_sums = {r: 0.0 for r in radii}
    g0_sum = 0.0

    for ix, kx in enumerate(k):
        sig = sigma18(kx, ky, kz)
        if ix == 0:
            sig = sig.copy()
            sig[0, 0] = np.inf
        inv = 1.0 / sig
        g0_sum += float(np.sum(inv))
        for r in radii:
            axis_sums[r] += float(math.cos(float(kx) * r) * np.sum(inv))

    volume = float(n**3)
    axis = []
    for r in radii:
        g = axis_sums[r] / volume
        axis.append(
            {
                "r": r,
                "G_axis": g,
                "four_pi_r_G": 4.0 * math.pi * r * g,
            }
        )

    g0 = g0_sum / volume
    return {
        "N": n,
        "G0_mean_BZ_zero_omitted": g0,
        "W18_engine_reference_4G0": 4.0 * g0,
        "axis": axis,
    }


def run(n_list: list[int], r_list: list[int] | None) -> dict[str, object]:
    results = {
        "generated_utc": datetime.now(timezone.utc).isoformat(),
        "normalization": {
            "laplacian": "lap=(1/3)face_sum+(1/6)edge_sum-4f",
            "sigma18": "4-(2/3)(cx+cy+cz)-(2/3)(cxcy+cxcz+cycz)",
            "green": "G(r)=N^-3 sum_{k!=0} cos(k_x r)/sigma18(k)",
            "continuum_axis_check": "4*pi*r*G_axis(r) -> 1 away from source and boundaries",
            "dispersion": "4 sin^2(omega/2)=C_SPEED^2 sigma18(k)",
        },
        "native_constants": {
            "C_SPEED": C_SPEED,
            "C_L_long_wavelength": 1.0,
            "K_T_canonical": 1.0,
            "Z_j_signed_transport": 1.0,
        },
        "sigma_high_symmetry": high_symmetry_summary(),
        "N_results": [],
    }
    for n in n_list:
        radii = axis_radii_for_n(n, r_list)
        results["N_results"].append(
            {
                "N": n,
                "modes": mode_summary(n),
                "green": green_response(n, radii),
            }
        )
    return results


def print_report(results: dict[str, object]) -> None:
    print("FTD native electrodynamics fixed probe")
    print("No alpha matching, parameter fitting, or numerical search.")
    print()
    nc = results["native_constants"]
    print("Native constants / normalizations")
    print(f"  C_SPEED                  = {nc['C_SPEED']:.15f}")
    print(f"  C_L_long_wavelength      = {nc['C_L_long_wavelength']:.15f}")
    print(f"  K_T_canonical            = {nc['K_T_canonical']:.15f}")
    print(f"  Z_j_signed_transport     = {nc['Z_j_signed_transport']:.15f}")
    print()
    print("sigma18 high-symmetry values")
    for name, value in results["sigma_high_symmetry"].items():
        print(f"  {name:6s} {value:.15f}")
    print()

    for row in results["N_results"]:
        n = row["N"]
        print(f"N = {n}")
        print("  small-k modes")
        for mode in row["modes"]:
            label = ",".join(str(x) for x in mode["mode"])
            print(
                "    "
                f"({label:5s}) sigma/q^2={mode['sigma_over_q2']:.12f} "
                f"v_ph={mode['phase_speed']:.12f} "
                f"v_ph/C={mode['phase_speed_over_c']:.12f}"
            )
        green = row["green"]
        print(f"  G0_mean_BZ_zero_omitted = {green['G0_mean_BZ_zero_omitted']:.12f}")
        print(f"  W18_engine_reference_4G0 = {green['W18_engine_reference_4G0']:.12f}")
        print("  axis Coulomb check: 4*pi*r*G_axis")
        for item in green["axis"]:
            print(
                f"    r={item['r']:4d} G={item['G_axis']:.12e} "
                f"4pi r G={item['four_pi_r_G']:.12f}"
            )
        print()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--N-list",
        type=parse_int_list,
        default=[32, 64, 128, 256],
        help="comma-separated grid sizes; default: 32,64,128,256",
    )
    parser.add_argument(
        "--json-out",
        type=Path,
        default=Path("scripts/exploration/outputs/ftd_native_electrodynamics.json"),
        help="path for machine-readable output",
    )
    parser.add_argument(
        "--r-list",
        type=parse_optional_int_list,
        default=None,
        help="optional comma-separated axis radii; values above N/4 are omitted",
    )
    args = parser.parse_args()

    results = run(args.N_list, args.r_list)
    print_report(results)

    args.json_out.parent.mkdir(parents=True, exist_ok=True)
    args.json_out.write_text(json.dumps(results, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote JSON: {args.json_out}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
