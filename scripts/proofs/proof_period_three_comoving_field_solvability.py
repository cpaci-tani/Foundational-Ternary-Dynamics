"""FTD-0716 exact Fourier certificate for the period-three co-moving field."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0716"
SOURCE_SUMMARY = RESULT / "ftd_0716_period_three_comoving_field_source_v1.json"
SOURCE = RESULT / "ftd_0716_period_three_comoving_field_source_v1.csv"
SUMMARY = RESULT / "ftd_0716_period_three_comoving_field_solvability_v1.json"
MODES = RESULT / "ftd_0716_period_three_comoving_field_modes_v1.csv"
SOLUTION = RESULT / "ftd_0716_period_three_comoving_field_correction_v1.csv"
PARENT = ROOT / "engine/results/ftd_0715/ftd_0715_period_three_internal_momentum_lift_v1.json"
RUNNER = ROOT / "engine/tests/test_period_three_comoving_field_source.cpp"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_PERIOD_THREE_COMOVING_FIELD_SOLVABILITY_v1.md"
)
PROTOCOL = "5F74489C3BD5F7DCC28B99442DE13FBA36AC9110F9099065FF70C65F6041BE19"
PARENT_PROTOCOL = "668C2D55EBB59572CE6C1E01928E4AE9A94E0913C964F5E45A69CCC8B5C2B4F9"
EXPECTED_HASHES = {
    SOURCE_SUMMARY: "74C74D6051FDC3176B470B1F8C96161AAFC9E083A4A8A611BF11B705F8AB4923",
    SOURCE: "A56F37763CE14EFA8534DE2644374C00390805EED80E2763EB59B240B6169825",
    PARENT: "210E0D6D1DCC8DE331B48E99C73E44BF935757E16C6AD005B725ADB15A8A36A9",
    RUNNER: "C6724C1E3D63E00E9D749B6CCBBB582667DEB6B8ED19FF34797F274A45C34FE4",
    PREREG: PROTOCOL,
}
L = 33
LAMBDA = 1.0 / math.sqrt(3.0)
RANK_RELATIVE = 1e-12
SOURCE_RELATIVE = 1e-12


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def load_source() -> np.ndarray:
    rhs = np.zeros((6, L, L, L), dtype=np.float64)
    seen = np.zeros((L, L, L), dtype=np.bool_)
    with SOURCE.open(newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            x, y, z = (int(row[key]) for key in ("x", "y", "z"))
            assert 0 <= x < L and 0 <= y < L and 0 <= z < L
            assert not seen[x, y, z]
            seen[x, y, z] = True
            rhs[:, x, y, z] = [
                float(row[f"rhs_{sector}_{axis}"])
                for sector in ("electric", "magnetic")
                for axis in ("x", "y", "z")
            ]
    assert np.all(seen) and np.all(np.isfinite(rhs))
    return rhs


def fourier_operator() -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    kx, ky, kz = np.meshgrid(k, k, k, indexing="ij")
    dx, dy, dz = (1.0 - np.exp(-1j * value) for value in (kx, ky, kz))
    curl = np.zeros((L, L, L, 3, 3), dtype=np.complex128)
    curl[..., 0, 1] = -dz
    curl[..., 0, 2] = dy
    curl[..., 1, 0] = dz
    curl[..., 1, 2] = -dx
    curl[..., 2, 0] = -dy
    curl[..., 2, 1] = dx
    curl_t = np.swapaxes(curl.conj(), -1, -2)
    identity3 = np.eye(3, dtype=np.complex128)
    update = np.zeros((L, L, L, 6, 6), dtype=np.complex128)
    update[..., :3, :3] = identity3 - LAMBDA**2 * (curl @ curl_t)
    update[..., :3, 3:] = LAMBDA * curl
    update[..., 3:, :3] = -LAMBDA * curl_t
    update[..., 3:, 3:] = identity3
    update3 = update @ update @ update
    operator = np.exp(1j * kx)[..., None, None] * update3
    operator -= np.eye(6, dtype=np.complex128)
    return operator, kx, ky, kz


def fft_components(values: np.ndarray) -> np.ndarray:
    return np.moveaxis(
        np.fft.fftn(values, axes=(1, 2, 3), norm="ortho"), 0, -1
    )


def ifft_components(values: np.ndarray) -> np.ndarray:
    return np.fft.ifftn(
        np.moveaxis(values, -1, 0), axes=(1, 2, 3), norm="ortho"
    )


def flat_norm(values: np.ndarray) -> float:
    return float(np.linalg.norm(values.ravel()))


def main() -> None:
    for path, expected in EXPECTED_HASHES.items():
        assert sha256(path) == expected, path
    parent = json.loads(PARENT.read_text(encoding="utf-8"))
    source_summary = json.loads(SOURCE_SUMMARY.read_text(encoding="utf-8"))
    assert parent["protocol_sha256"] == PARENT_PROTOCOL
    assert parent["verdict"] == "PERIOD_THREE_MOMENTUM_LIFT_CONSTRUCTIVE"
    assert source_summary["protocol_sha256"] == PROTOCOL
    assert source_summary["verdict"] == "PERIOD_THREE_COMOVING_FIELD_SOURCE_CONSTRUCTIVE"
    assert source_summary["volume"] == L
    assert source_summary["segments"] == 48
    assert source_summary["continuity_residual"] <= 1e-12
    assert source_summary["causal_excess"] <= 1e-12

    rhs = load_source()
    rhs_l2 = flat_norm(rhs)
    rhs_max = float(np.max(np.abs(rhs)))
    source_crosscheck = (
        abs(rhs_l2 - source_summary["source_l2"]) <= 1e-12
        and abs(rhs_max - source_summary["source_maximum"]) <= 1e-15
    )
    rhs_hat = fft_components(rhs)
    operator, kx, ky, kz = fourier_operator()
    left, singular, right_h = np.linalg.svd(operator, full_matrices=True)
    threshold = RANK_RELATIVE * np.maximum(1.0, singular[..., :1])
    retained = singular > threshold
    coefficients = np.einsum("...ji,...j->...i", left.conj(), rhs_hat)
    inverse_coefficients = np.zeros_like(coefficients)
    np.divide(coefficients, singular, out=inverse_coefficients, where=retained)
    solution_hat = np.einsum(
        "...ji,...j->...i", right_h.conj(), inverse_coefficients
    )
    residual_hat = np.einsum("...ij,...j->...i", operator, solution_hat) - rhs_hat
    null_coefficients = np.where(retained, 0.0, coefficients)

    solution_complex = ifft_components(solution_hat)
    residual_complex = ifft_components(residual_hat)
    solution = solution_complex.real
    residual = residual_complex.real
    reality_residual = max(
        float(np.max(np.abs(solution_complex.imag))),
        float(np.max(np.abs(residual_complex.imag))),
    )
    spectral_l2 = flat_norm(residual_hat)
    spectral_max = float(np.max(np.abs(residual_hat)))
    real_l2 = flat_norm(residual)
    real_max = float(np.max(np.abs(residual)))
    null_projection = flat_norm(null_coefficients)
    parseval_residual = max(
        abs(flat_norm(rhs) - flat_norm(rhs_hat)),
        abs(flat_norm(solution) - flat_norm(solution_hat)),
        abs(real_l2 - spectral_l2),
    )

    source_norm = np.linalg.norm(rhs_hat, axis=-1)
    source_active = source_norm > SOURCE_RELATIVE * float(np.max(source_norm))
    active_retained = retained & source_active[..., None]
    retained_values = singular[active_retained]
    minimum_active_singular = (
        float(np.min(retained_values)) if retained_values.size else math.inf
    )
    amplification = flat_norm(solution_hat) / flat_norm(rhs_hat)
    maximum_solution = float(np.max(np.abs(solution)))
    zero_singular_count = int(np.size(retained) - np.count_nonzero(retained))
    incompatible_mode_count = int(
        np.count_nonzero(np.linalg.norm(null_coefficients, axis=-1) > 1e-12)
    )
    finite = all(
        math.isfinite(value)
        for value in (
            rhs_l2,
            rhs_max,
            spectral_l2,
            spectral_max,
            real_l2,
            real_max,
            null_projection,
            parseval_residual,
            reality_residual,
            minimum_active_singular,
            amplification,
            maximum_solution,
        )
    )
    validation = (
        source_crosscheck
        and finite
        and reality_residual <= 1e-10
        and parseval_residual <= 1e-10
    )
    solved = spectral_max <= 1e-9 and real_max <= 1e-9
    ill_conditioned = minimum_active_singular < 1e-8 or amplification > 1e6
    if not validation:
        verdict = "PERIOD_THREE_COMOVING_FIELD_SOLVABILITY_EXECUTION_INVALID"
    elif solved:
        verdict = (
            "PERIOD_THREE_COMOVING_FIELD_SOLUTION_ILL_CONDITIONED"
            if ill_conditioned
            else "PERIOD_THREE_COMOVING_FIELD_SOLUTION_REGULAR"
        )
    elif null_projection > 1e-9:
        verdict = "PERIOD_THREE_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE"
    else:
        verdict = "PERIOD_THREE_COMOVING_FIELD_SOLVABILITY_EXECUTION_INVALID"

    record = {
        "ftd_id": "FTD-0716",
        "protocol_sha256": PROTOCOL,
        "parent_protocol_sha256": PARENT_PROTOCOL,
        "verdict": verdict,
        "production_changed": False,
        "volume": L,
        "field_dof": int(6 * L**3),
        "source_crosscheck_pass": source_crosscheck,
        "validation_pass": validation,
        "solution_pass": solved,
        "ill_conditioned": ill_conditioned,
        "source_l2": rhs_l2,
        "source_maximum": rhs_max,
        "spectral_residual_l2": spectral_l2,
        "spectral_residual_max": spectral_max,
        "real_residual_l2": real_l2,
        "real_residual_max": real_max,
        "nullspace_source_projection_l2": null_projection,
        "reality_residual": reality_residual,
        "parseval_residual": parseval_residual,
        "minimum_retained_source_active_singular_value": minimum_active_singular,
        "solution_amplification": amplification,
        "maximum_solution_component": maximum_solution,
        "zero_singular_value_count": zero_singular_count,
        "incompatible_mode_count": incompatible_mode_count,
        "input_source_summary_sha256": EXPECTED_HASHES[SOURCE_SUMMARY],
        "input_source_sha256": EXPECTED_HASHES[SOURCE],
        "input_parent_sha256": EXPECTED_HASHES[PARENT],
        "input_runner_sha256": EXPECTED_HASHES[RUNNER],
    }
    SUMMARY.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")

    with MODES.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "nx", "ny", "nz", "kx", "ky", "kz", "source_norm",
                "minimum_retained_singular", "null_projection_norm",
                "residual_norm", "solution_norm",
            ]
        )
        for nx in range(L):
            for ny in range(L):
                for nz in range(L):
                    mode_retained = singular[nx, ny, nz][retained[nx, ny, nz]]
                    writer.writerow(
                        [
                            nx, ny, nz,
                            repr(float(kx[nx, ny, nz])),
                            repr(float(ky[nx, ny, nz])),
                            repr(float(kz[nx, ny, nz])),
                            repr(float(source_norm[nx, ny, nz])),
                            repr(float(np.min(mode_retained))) if mode_retained.size else "",
                            repr(float(np.linalg.norm(null_coefficients[nx, ny, nz]))),
                            repr(float(np.linalg.norm(residual_hat[nx, ny, nz]))),
                            repr(float(np.linalg.norm(solution_hat[nx, ny, nz]))),
                        ]
                    )

    with SOLUTION.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            ["x", "y", "z", "electric_x", "electric_y", "electric_z",
             "magnetic_x", "magnetic_y", "magnetic_z"]
        )
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    writer.writerow(
                        [x, y, z]
                        + [repr(float(solution[c, x, y, z])) for c in range(6)]
                    )

    print(f"FTD-0716 verdict={verdict}")
    print(
        f"residual spectral={spectral_max:.6e} real={real_max:.6e} "
        f"null={null_projection:.6e} incompatible_modes={incompatible_mode_count}"
    )
    print(
        f"sigma_active={minimum_active_singular:.6e} "
        f"amplification={amplification:.6e} max_solution={maximum_solution:.6e}"
    )
    if verdict == "PERIOD_THREE_COMOVING_FIELD_SOLVABILITY_EXECUTION_INVALID":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
