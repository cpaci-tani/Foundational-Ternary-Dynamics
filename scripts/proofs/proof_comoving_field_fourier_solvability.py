"""FTD-0711 exact Fourier certificate for the co-moving field equation."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
RESULT_0710 = ROOT / "engine/results/ftd_0710"
SUMMARY_0710 = RESULT_0710 / (
    "ftd_0710_prescribed_trajectory_comoving_field_shooting_v1.json"
)
FIELD_0710 = RESULT_0710 / (
    "ftd_0710_prescribed_trajectory_comoving_field_rhs_v1.csv"
)
RUNNER_0710 = ROOT / (
    "engine/tests/test_prescribed_trajectory_comoving_field_shooting.cpp"
)
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_COMOVING_FIELD_FOURIER_SOLVABILITY_v1.md"
)
RESULT = ROOT / "engine/results/ftd_0711"
SUMMARY = RESULT / "ftd_0711_comoving_field_fourier_solvability_v1.json"
MODES = RESULT / "ftd_0711_comoving_field_fourier_modes_v1.csv"
SOLUTION = RESULT / "ftd_0711_comoving_field_correction_v1.csv"

PROTOCOL_0710 = "82E52438F5483C5C3A427B31D9B068314778B804C2320EEBFFCA1EA6EE593A4B"
PROTOCOL = "BD9B05437801F23A7A773F8B455E447BB32CDAD01C91962D2C1E743422921E5F"
EXPECTED_HASHES = {
    SUMMARY_0710: "194AA2AA9AB989CDF2AFED59E71E6565555EB7B639EC8D814160C630E528122A",
    FIELD_0710: "76618236A4F6DB01B27666247245E689D68FBD2CA86A56E051D90DAE38C38A0D",
    RUNNER_0710: "88A971564428691FBED81AA5AD0A67CD035CBC827316957891C324BA6E368F8C",
    PREREG: PROTOCOL,
}
L = 33
LAMBDA = 1.0 / math.sqrt(3.0)
RANK_RELATIVE = 1e-12
SOURCE_RELATIVE = 1e-12


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def load_fields() -> tuple[np.ndarray, np.ndarray]:
    rhs = np.zeros((6, L, L, L), dtype=np.float64)
    gmres = np.zeros_like(rhs)
    seen = np.zeros((L, L, L), dtype=np.bool_)
    with FIELD_0710.open(newline="", encoding="utf-8") as handle:
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
            gmres[:, x, y, z] = [
                float(row[f"gmres_{sector}_{axis}"])
                for sector in ("electric", "magnetic")
                for axis in ("x", "y", "z")
            ]
    assert np.all(seen)
    assert np.all(np.isfinite(rhs)) and np.all(np.isfinite(gmres))
    return rhs, gmres


def fourier_operator() -> tuple[np.ndarray, np.ndarray, np.ndarray, np.ndarray]:
    k = 2.0 * np.pi * np.fft.fftfreq(L)
    kx, ky, kz = np.meshgrid(k, k, k, indexing="ij")
    dx, dy, dz = (1.0 - np.exp(-1j * value) for value in (kx, ky, kz))
    shape = (L, L, L, 3, 3)
    curl = np.zeros(shape, dtype=np.complex128)
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
    phase = np.exp(1j * kx)
    operator = phase[..., None, None] * (update @ update)
    operator -= np.eye(6, dtype=np.complex128)
    return operator, kx, ky, kz


def fft_components(values: np.ndarray) -> np.ndarray:
    transformed = np.fft.fftn(values, axes=(1, 2, 3), norm="ortho")
    return np.moveaxis(transformed, 0, -1)


def ifft_components(values: np.ndarray) -> np.ndarray:
    components = np.moveaxis(values, -1, 0)
    return np.fft.ifftn(components, axes=(1, 2, 3), norm="ortho")


def norm(values: np.ndarray) -> float:
    return float(np.linalg.norm(values.ravel()))


def main() -> None:
    for path, expected in EXPECTED_HASHES.items():
        assert sha256(path) == expected, path
    parent = json.loads(SUMMARY_0710.read_text(encoding="utf-8"))
    assert parent["protocol_sha256"] == PROTOCOL_0710
    assert parent["verdict"] == "PRESCRIBED_TRAJECTORY_FIELD_SHOOTING_NOT_RESOLVED"
    assert parent["volume"] == L and parent["field_dof"] == 6 * L**3

    rhs, gmres = load_fields()
    rhs_hat, gmres_hat = fft_components(rhs), fft_components(gmres)
    operator, kx, ky, kz = fourier_operator()

    gmres_residual_hat = rhs_hat - np.einsum(
        "...ij,...j->...i", operator, gmres_hat
    )
    gmres_residual = ifft_components(gmres_residual_hat)
    gmres_l2 = norm(gmres_residual_hat)
    gmres_max = float(np.max(np.abs(gmres_residual.real)))
    parent_l2_error = abs(gmres_l2 - parent["final_field_l2_residual"])
    parent_max_error = abs(gmres_max - parent["complete_field_residual"])
    operator_crosscheck = parent_l2_error <= 1e-10 and parent_max_error <= 1e-10

    left, singular, right_h = np.linalg.svd(operator, full_matrices=True)
    threshold = RANK_RELATIVE * np.maximum(1.0, singular[..., :1])
    retained = singular > threshold
    coefficients = np.einsum("...ji,...j->...i", left.conj(), rhs_hat)
    inverse_coefficients = np.zeros_like(coefficients)
    np.divide(
        coefficients,
        singular,
        out=inverse_coefficients,
        where=retained,
    )
    solution_hat = np.einsum(
        "...ji,...j->...i", right_h.conj(), inverse_coefficients
    )
    residual_hat = np.einsum(
        "...ij,...j->...i", operator, solution_hat
    ) - rhs_hat
    null_coefficients = np.where(retained, 0.0, coefficients)

    solution_complex = ifft_components(solution_hat)
    residual_complex = ifft_components(residual_hat)
    solution = solution_complex.real
    residual = residual_complex.real
    reality_residual = max(
        float(np.max(np.abs(solution_complex.imag))),
        float(np.max(np.abs(residual_complex.imag))),
    )
    spectral_l2 = norm(residual_hat)
    spectral_max = float(np.max(np.abs(residual_hat)))
    real_l2 = norm(residual)
    real_max = float(np.max(np.abs(residual)))
    null_projection = norm(null_coefficients)
    parseval_residual = max(
        abs(norm(rhs) - norm(rhs_hat)),
        abs(norm(solution) - norm(solution_hat)),
        abs(real_l2 - spectral_l2),
    )

    source_norm = np.linalg.norm(rhs_hat, axis=-1)
    source_active = source_norm > SOURCE_RELATIVE * float(np.max(source_norm))
    active_retained = retained & source_active[..., None]
    retained_values = singular[active_retained]
    minimum_active_singular = (
        float(np.min(retained_values)) if retained_values.size else math.inf
    )
    amplification = norm(solution_hat) / norm(rhs_hat)
    maximum_solution = float(np.max(np.abs(solution)))
    zero_singular_count = int(np.size(retained) - np.count_nonzero(retained))
    incompatible_mode_count = int(
        np.count_nonzero(np.linalg.norm(null_coefficients, axis=-1) > 1e-12)
    )

    finite = all(
        math.isfinite(value)
        for value in (
            gmres_l2,
            gmres_max,
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
        operator_crosscheck
        and finite
        and reality_residual <= 1e-10
        and parseval_residual <= 1e-10
    )
    solved = spectral_max <= 1e-9 and real_max <= 1e-9
    ill_conditioned = (
        minimum_active_singular < 1e-8 or amplification > 1e6
    )
    if not validation:
        verdict = "COMOVING_FIELD_FOURIER_SOLVABILITY_EXECUTION_INVALID"
    elif solved:
        verdict = (
            "FINITE_VOLUME_COMOVING_FIELD_SOLUTION_ILL_CONDITIONED"
            if ill_conditioned
            else "FINITE_VOLUME_COMOVING_FIELD_SOLUTION_REGULAR"
        )
    elif null_projection > 1e-9:
        verdict = "FINITE_VOLUME_COMOVING_SOURCE_NULLSPACE_INCOMPATIBLE"
    else:
        verdict = "COMOVING_FIELD_FOURIER_SOLVABILITY_EXECUTION_INVALID"

    RESULT.mkdir(parents=True, exist_ok=True)
    record = {
        "ftd_id": "FTD-0711",
        "protocol_sha256": PROTOCOL,
        "parent_protocol_sha256": PROTOCOL_0710,
        "verdict": verdict,
        "production_changed": False,
        "volume": L,
        "field_dof": int(6 * L**3),
        "operator_crosscheck_pass": operator_crosscheck,
        "validation_pass": validation,
        "solution_pass": solved,
        "ill_conditioned": ill_conditioned,
        "parent_l2_crosscheck_error": parent_l2_error,
        "parent_max_crosscheck_error": parent_max_error,
        "gmres_residual_l2_reconstructed": gmres_l2,
        "gmres_residual_max_reconstructed": gmres_max,
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
        "input_summary_sha256": EXPECTED_HASHES[SUMMARY_0710],
        "input_field_sha256": EXPECTED_HASHES[FIELD_0710],
        "input_runner_sha256": EXPECTED_HASHES[RUNNER_0710],
    }
    SUMMARY.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")

    with MODES.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "nx",
                "ny",
                "nz",
                "kx",
                "ky",
                "kz",
                "source_norm",
                "minimum_retained_singular",
                "null_projection_norm",
                "residual_norm",
                "solution_norm",
            ]
        )
        for nx in range(L):
            for ny in range(L):
                for nz in range(L):
                    mode_retained = singular[nx, ny, nz][retained[nx, ny, nz]]
                    writer.writerow(
                        [
                            nx,
                            ny,
                            nz,
                            repr(float(kx[nx, ny, nz])),
                            repr(float(ky[nx, ny, nz])),
                            repr(float(kz[nx, ny, nz])),
                            repr(float(source_norm[nx, ny, nz])),
                            repr(float(np.min(mode_retained)))
                            if mode_retained.size
                            else "",
                            repr(float(np.linalg.norm(
                                null_coefficients[nx, ny, nz]
                            ))),
                            repr(float(np.linalg.norm(residual_hat[nx, ny, nz]))),
                            repr(float(np.linalg.norm(solution_hat[nx, ny, nz]))),
                        ]
                    )

    with SOLUTION.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(
            [
                "x",
                "y",
                "z",
                "electric_x",
                "electric_y",
                "electric_z",
                "magnetic_x",
                "magnetic_y",
                "magnetic_z",
            ]
        )
        for x in range(L):
            for y in range(L):
                for z in range(L):
                    writer.writerow(
                        [x, y, z]
                        + [repr(float(solution[c, x, y, z])) for c in range(6)]
                    )

    print(f"FTD-0711 verdict={verdict}")
    print(
        f"GMRES crosscheck l2={parent_l2_error:.3e} max={parent_max_error:.3e}"
    )
    print(
        f"spectral residual l2={spectral_l2:.6e} max={spectral_max:.6e} "
        f"null={null_projection:.6e}"
    )
    print(
        f"sigma_active={minimum_active_singular:.6e} "
        f"amplification={amplification:.6e} max_solution={maximum_solution:.6e}"
    )
    if verdict == "COMOVING_FIELD_FOURIER_SOLVABILITY_EXECUTION_INVALID":
        raise SystemExit(1)


if __name__ == "__main__":
    main()
