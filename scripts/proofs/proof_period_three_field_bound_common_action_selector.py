"""FTD-0718 deterministic homogeneous-field common-action selector.

The C++ observer supplies only the locked matter history and the force from the
FTD-0716 particular field.  This script constructs the complete registered
real, divergence-free kernel of exp(i k_x) U(k)^3-I, evaluates its analytic
quadratic-spline orbit response, and writes the Moore--Penrose minimum-norm
field correction for an independent C++ replay.
"""
from __future__ import annotations

import csv
import hashlib
import json
import math
from dataclasses import dataclass
from pathlib import Path

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0718"
SEED = RESULT / "ftd_0718_period_three_force_seed_v1.csv"
CORRECTION = RESULT / "ftd_0718_period_three_field_bound_correction_v1.csv"
SOLVE = RESULT / "ftd_0718_period_three_field_bound_selector_solve_v1.json"
REPLAY = RESULT / "ftd_0718_period_three_field_bound_selector_replay_v1.json"
PREREG = ROOT / (
    "docs/theory/10_eft_program/preregistrations/"
    "PREREG_PERIOD_THREE_FIELD_BOUND_COMMON_ACTION_SELECTOR_v1.md"
)
PROTOCOL = "EAC3AF4476F6F7FF4223B2D2B9BA864E151D0625B17175BFD4F79555C6CCED10"
L = 33
VOLUME = L**3
SQRT_VOLUME = math.sqrt(VOLUME)
LAMBDA = 1.0 / math.sqrt(3.0)
BETA = 0.021892057692994273
RELATIVE = 1e-12

NODES = np.array(
    [
        -0.960289856497536231683560868569,
        -0.796666477413626739591553936476,
        -0.525532409916328985817739049189,
        -0.183434642495649804939476142360,
        0.183434642495649804939476142360,
        0.525532409916328985817739049189,
        0.796666477413626739591553936476,
        0.960289856497536231683560868569,
    ],
    dtype=np.float64,
)
WEIGHTS = np.array(
    [
        0.101228536290376259152531354310,
        0.222381034453374470544355934426,
        0.313706645877887287337962201987,
        0.362683783378361982965150449277,
        0.362683783378361982965150449277,
        0.313706645877887287337962201987,
        0.222381034453374470544355934426,
        0.101228536290376259152531354310,
    ],
    dtype=np.float64,
)


@dataclass(frozen=True)
class SeedRow:
    particle: int
    tick: int
    charge: int
    start: np.ndarray
    end: np.ndarray
    velocity: np.ndarray
    residual: np.ndarray


@dataclass
class ModeRecord:
    index: tuple[int, int, int]
    wave: np.ndarray
    basis: np.ndarray
    update: np.ndarray
    self_conjugate: bool
    first_column: int


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def load_seed() -> list[SeedRow]:
    rows: list[SeedRow] = []
    with SEED.open(newline="", encoding="utf-8") as handle:
        for row in csv.DictReader(handle):
            vector = lambda prefix: np.array(  # noqa: E731
                [float(row[f"{prefix}_{axis}"]) for axis in "xyz"],
                dtype=np.float64,
            )
            rows.append(
                SeedRow(
                    particle=int(row["particle"]),
                    tick=int(row["tick"]),
                    charge=int(row["charge"]),
                    start=vector("start"),
                    end=vector("end"),
                    velocity=vector("velocity"),
                    residual=vector("residual"),
                )
            )
    assert len(rows) == 48
    assert [(row.tick, row.particle) for row in rows] == [
        (tick, particle) for tick in range(3) for particle in range(16)
    ]
    return rows


def update_matrix(wave: np.ndarray) -> np.ndarray:
    dx, dy, dz = 1.0 - np.exp(-1j * wave)
    curl = np.zeros((3, 3), dtype=np.complex128)
    curl[0, 1], curl[0, 2] = -dz, dy
    curl[1, 0], curl[1, 2] = dz, -dx
    curl[2, 0], curl[2, 1] = -dy, dx
    curl_t = curl.conj().T
    update = np.zeros((6, 6), dtype=np.complex128)
    update[:3, :3] = np.eye(3) - LAMBDA**2 * (curl @ curl_t)
    update[:3, 3:] = LAMBDA * curl
    update[3:, :3] = -LAMBDA * curl_t
    update[3:, 3:] = np.eye(3)
    return update


def kernel_at(wave: np.ndarray) -> tuple[np.ndarray, np.ndarray, float, float]:
    update = update_matrix(wave)
    operator = np.exp(1j * wave[0]) * np.linalg.matrix_power(update, 3)
    operator -= np.eye(6)
    _, singular, right_h = np.linalg.svd(operator, full_matrices=True)
    threshold = RELATIVE * max(1.0, float(singular[0]))
    null = right_h.conj().T[:, singular <= threshold]
    if null.shape[1] == 0:
        return null, update, 0.0, 0.0
    divergence = np.array(
        [*(1.0 - np.exp(-1j * wave)), 0.0, 0.0, 0.0],
        dtype=np.complex128,
    )
    restricted = (divergence @ null).reshape(1, -1)
    _, div_singular, div_right_h = np.linalg.svd(restricted, full_matrices=True)
    div_threshold = RELATIVE * max(1.0, float(div_singular[0]))
    div_rank = int(np.count_nonzero(div_singular > div_threshold))
    intersection = null @ div_right_h.conj().T[:, div_rank:]
    if intersection.shape[1]:
        intersection, _ = np.linalg.qr(intersection)
    operator_residual = float(
        np.max(np.abs(operator @ intersection)) if intersection.size else 0.0
    )
    divergence_residual = float(
        np.max(np.abs(divergence @ intersection)) if intersection.size else 0.0
    )
    return intersection, update, operator_residual, divergence_residual


def b1(value: float) -> float:
    return max(0.0, 1.0 - abs(value))


def b2(value: float) -> float:
    absolute = abs(value)
    if absolute < 0.5:
        return 0.75 - absolute * absolute
    if absolute < 1.5:
        return 0.5 * (1.5 - absolute) ** 2
    return 0.0


def one_dimensional_sum(
    wave: float, position: float, component_axis: bool, face: bool
) -> complex:
    lower = math.floor(position) - 2
    result = 0.0j
    for site in range(lower, lower + 5):
        if face:
            weight = b1(position - site - 0.5) if component_axis else b2(
                position - site
            )
        else:
            weight = b2(position - site) if component_axis else b1(
                position - site - 0.5
            )
        result += weight * np.exp(1j * wave * site)
    return result


def interpolation_factor(
    wave: np.ndarray, position: np.ndarray, component: int, face: bool
) -> complex:
    result = 1.0 + 0.0j
    for axis in range(3):
        result *= one_dimensional_sum(
            float(wave[axis]), float(position[axis]), axis == component, face
        )
    return result


def path_breaks(start: np.ndarray, end: np.ndarray) -> list[float]:
    breaks = [0.0, 1.0]
    for axis in range(3):
        delta = float(end[axis] - start[axis])
        if delta == 0.0:
            continue
        lower, upper = sorted((float(start[axis]), float(end[axis])))
        for knot in range(math.floor(lower) - 2, math.ceil(upper) + 3):
            tau = (knot + 0.5 - float(start[axis])) / delta
            if 0.0 < tau < 1.0:
                breaks.append(tau)
    breaks.sort()
    unique = [breaks[0]]
    for value in breaks[1:]:
        if abs(value - unique[-1]) > 32.0 * np.finfo(float).eps:
            unique.append(value)
    return unique


def orbit_factors(row: SeedRow, wave: np.ndarray, face: bool) -> np.ndarray:
    result = np.zeros(3, dtype=np.complex128)
    breaks = path_breaks(row.start, row.end)
    displacement = row.end - row.start
    for lower, upper in zip(breaks[:-1], breaks[1:]):
        midpoint = 0.5 * (lower + upper)
        half = 0.5 * (upper - lower)
        for node, weight in zip(NODES, WEIGHTS):
            tau = midpoint + half * float(node)
            position = row.start + displacement * tau
            for component in range(3):
                result[component] += half * weight * interpolation_factor(
                    wave, position, component, face
                )
    return result


def response_at(
    rows: list[SeedRow], wave: np.ndarray, update: np.ndarray
) -> np.ndarray:
    response = np.zeros((3 * len(rows), 6), dtype=np.complex128)
    powers = [np.eye(6, dtype=np.complex128)]
    powers.extend([update, update @ update, update @ update @ update])
    for row_index, row in enumerate(rows):
        tick = row.tick
        electric_map = 0.5 * (powers[tick][:3, :] + powers[tick + 1][:3, :])
        magnetic_map = powers[tick + 1][3:, :]
        face_factor = orbit_factors(row, wave, True)
        edge_factor = orbit_factors(row, wave, False)
        electric_map = face_factor[:, None] * electric_map / SQRT_VOLUME
        magnetic_map = edge_factor[:, None] * magnetic_map / SQRT_VOLUME
        cross_map = np.empty_like(magnetic_map)
        for column in range(6):
            cross_map[:, column] = np.cross(row.velocity, magnetic_map[:, column])
        response[3 * row_index : 3 * row_index + 3, :] = (
            BETA * row.charge * (electric_map + cross_map)
        )
    return response


def build_response(rows: list[SeedRow]) -> tuple[np.ndarray, list[ModeRecord], dict]:
    frequencies = 2.0 * math.pi * np.fft.fftfreq(L)
    columns: list[np.ndarray] = []
    modes: list[ModeRecord] = []
    maximum_operator = 0.0
    maximum_divergence = 0.0
    raw_nullity = 0
    divergence_free_nullity = 0
    for nx in range(L):
        for ny in range(L):
            for nz in range(L):
                index = (nx, ny, nz)
                partner = ((-nx) % L, (-ny) % L, (-nz) % L)
                if index > partner:
                    continue
                wave = np.array(
                    [frequencies[nx], frequencies[ny], frequencies[nz]],
                    dtype=np.float64,
                )
                basis, update, operator_residual, divergence_residual = kernel_at(wave)
                maximum_operator = max(maximum_operator, operator_residual)
                maximum_divergence = max(maximum_divergence, divergence_residual)
                if basis.shape[1] == 0:
                    continue
                raw_nullity += basis.shape[1] * (1 if index == partner else 2)
                response = response_at(rows, wave, update) @ basis
                first = len(columns)
                if index == partner:
                    for column in range(basis.shape[1]):
                        columns.append(response[:, column].real)
                else:
                    root_two = math.sqrt(2.0)
                    for column in range(basis.shape[1]):
                        columns.append(root_two * response[:, column].real)
                        columns.append(-root_two * response[:, column].imag)
                divergence_free_nullity += len(columns) - first
                modes.append(
                    ModeRecord(index, wave, basis, update, index == partner, first)
                )
    matrix = np.column_stack(columns) if columns else np.zeros((144, 0))
    diagnostics = {
        "raw_real_divergence_free_nullity": raw_nullity,
        "real_basis_columns": divergence_free_nullity,
        "maximum_kernel_operator_residual": maximum_operator,
        "maximum_kernel_divergence_residual": maximum_divergence,
    }
    return matrix, modes, diagnostics


def reconstruct(modes: list[ModeRecord], coefficients: np.ndarray) -> np.ndarray:
    coordinates = np.arange(L, dtype=np.float64)
    x, y, z = np.meshgrid(coordinates, coordinates, coordinates, indexing="ij")
    field = np.zeros((6, L, L, L), dtype=np.float64)
    for mode in modes:
        amplitude = np.zeros(6, dtype=np.complex128)
        cursor = mode.first_column
        if mode.self_conjugate:
            for column in range(mode.basis.shape[1]):
                amplitude += coefficients[cursor] * mode.basis[:, column]
                cursor += 1
            phase = np.ones((L, L, L), dtype=np.complex128)
            field += (amplitude[:, None, None, None] * phase).real / SQRT_VOLUME
        else:
            for column in range(mode.basis.shape[1]):
                cosine = coefficients[cursor]
                sine = coefficients[cursor + 1]
                amplitude += math.sqrt(2.0) * (cosine + 1j * sine) * mode.basis[:, column]
                cursor += 2
            phase = np.exp(
                1j * (mode.wave[0] * x + mode.wave[1] * y + mode.wave[2] * z)
            )
            field += (amplitude[:, None, None, None] * phase).real / SQRT_VOLUME
    return field


def write_field(field: np.ndarray) -> None:
    RESULT.mkdir(parents=True, exist_ok=True)
    with CORRECTION.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle, lineterminator="\n")
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
                        [x, y, z, *[format(field[c, x, y, z], ".17g") for c in range(6)]]
                    )


def main() -> None:
    assert sha256(PREREG) == PROTOCOL
    rows = load_seed()
    target = np.concatenate([row.residual for row in rows])
    matrix, modes, diagnostics = build_response(rows)
    coefficients, _, rank, singular = np.linalg.lstsq(matrix, target, rcond=RELATIVE)
    predicted = matrix @ coefficients
    residual = predicted - target
    field = reconstruct(modes, coefficients)
    write_field(field)
    component_residual = float(np.max(np.abs(residual)))
    vector_residual = float(
        np.max(np.linalg.norm(residual.reshape(-1, 3), axis=1))
    )
    verdict = (
        "PERIOD_THREE_HOMOGENEOUS_FORCE_SELECTOR_SOLVED"
        if vector_residual <= 1e-10
        else "PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT"
    )
    record = {
        "ftd_id": "FTD-0718",
        "protocol_sha256": PROTOCOL,
        "verdict": verdict,
        "production_changed": False,
        "volume": L,
        "response_rows": int(matrix.shape[0]),
        "response_columns": int(matrix.shape[1]),
        "response_rank": int(rank),
        "response_rank_tolerance": RELATIVE,
        "minimum_retained_response_singular_value": float(singular[rank - 1])
        if rank
        else 0.0,
        "maximum_response_singular_value": float(singular[0]) if singular.size else 0.0,
        "target_l2": float(np.linalg.norm(target)),
        "predicted_l2": float(np.linalg.norm(predicted)),
        "coefficient_l2": float(np.linalg.norm(coefficients)),
        "field_l2": float(np.linalg.norm(field.ravel())),
        "field_maximum": float(np.max(np.abs(field))),
        "maximum_component_force_residual": component_residual,
        "maximum_vector_force_residual": vector_residual,
        "seed_sha256": sha256(SEED),
        "correction_sha256": sha256(CORRECTION),
        **diagnostics,
    }
    SOLVE.write_text(json.dumps(record, indent=2) + "\n", encoding="utf-8")
    assert matrix.shape == (144, 1094)
    assert rank == 35
    assert verdict == "PERIOD_THREE_HOMOGENEOUS_FIELD_FORCE_SPACE_INSUFFICIENT"
    assert vector_residual > 0.3
    assert float(np.linalg.norm(coefficients)) > 1e9
    if REPLAY.exists():
        replay = json.loads(REPLAY.read_text(encoding="utf-8"))
        assert replay["protocol_sha256"] == PROTOCOL
        assert replay["maximum_force_residual"] > 0.3
    print(json.dumps(record, indent=2))


if __name__ == "__main__":
    main()
