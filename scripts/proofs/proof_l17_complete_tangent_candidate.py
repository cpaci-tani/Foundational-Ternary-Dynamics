"""Independent primitive-artifact replay for the locked FTD-0774 run.

The C++ scalar verdict and gate booleans are never inputs to the reconstructed
verdict.  This script validates provenance and the frozen artifact schemas,
rebuilds H_red and the positive quadratic form K, reads every stored tangent
vector, recomputes projected clusters and held-out matrix/vector gates, and
then applies the ordered verdict map in the preregistration.

The result corpus is intentionally not optional.  Before the runner has
emitted it, this certificate exits 2 with an explicit ``CORPUS ABSENT``
message; a missing or malformed partial corpus exits 1.
"""

from __future__ import annotations

import csv
import hashlib
import itertools
import json
import math
import re
import subprocess
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

import numpy as np


ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT
    / "docs/theory/10_eft_program/preregistrations"
    / "PREREG_L17_COMPLETE_TANGENT_CANDIDATE_v1.md"
)
PROTOCOL_SHA256 = (
    "0604AF560EA193BDE9E339ADB3FB28C0631B43D204186BEDA977EB700DD7F27E"
)
SOURCE_COMMIT = "93748ac2021e4db5a9b8583cc28493332c716ac0"
RUNNER_SHA256 = "0AF8F7BE9D6962F893F4CB975D75C172C91564CAB7F044A9D4533AA0ED7296C0"
SUPPORT_SHA256 = "C28E3B2768BDAC9542A9AF16CE67608CBD1E4D7E46ED2AEAD3D06640722BC9C3"
RESULT_DIR = ROOT / "engine/results/ftd_0774"
STEM = RESULT_DIR / "ftd_0774_l17_complete_tangent_candidate_v1"
RESULT_JSON = STEM.with_suffix(".json")
PREFLIGHT = Path(str(STEM) + "_preflight.csv")
HESSIAN = Path(str(STEM) + "_hessian.csv")
PROJECTED = Path(str(STEM) + "_projected_matrices.csv")
CLUSTERS = Path(str(STEM) + "_clusters.csv")
METRICS = Path(str(STEM) + "_candidate_metrics.csv")
VECTORS = Path(str(STEM) + "_candidate_vectors.bin")
VECTOR_INDEX = Path(str(STEM) + "_candidate_vectors_index.csv")
GRAMS = Path(str(STEM) + "_gram_blocks.csv")
HASHES = Path(str(STEM) + "_hashes.csv")
EXECUTION_STATUS = Path(str(STEM) + "_execution_status.csv")
PREFLIGHT_DERIVATIVE_STATUS = Path(
    str(STEM) + "_preflight_derivative_status.csv"
)
RUNTIME = Path(str(STEM) + "_runtime.csv")
ENERGY_CONTROL = Path(str(STEM) + "_energy_control.csv")
CACHE_CONTROL = Path(str(STEM) + "_cache_control.csv")
FIELD_CONTROL = Path(str(STEM) + "_field_control.csv")
KRYLOV_STATUS = Path(str(STEM) + "_krylov_status.csv")
RUNNER = ROOT / "engine/tests/test_l17_complete_tangent_candidate.cpp"
SUPPORT = ROOT / "engine/tests/support/connected_moore_tangent_codec.h"
EMBEDDED_SOURCES = {
    "compiled_closure_0": ROOT / "engine/tests/test_connected_block_analytic_matter_modes.cpp",
    "compiled_closure_1": ROOT / "engine/tests/test_connected_block_analytic_dynamical_rest.cpp",
    "compiled_closure_2": ROOT / "engine/tests/test_connected_block_analytic_static_refinement.cpp",
    "compiled_closure_3": ROOT / "engine/tests/test_connected_block_analytic_envelope_hessian.cpp",
}

PARENT_HASHES = {
    ROOT / "engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_v1.json":
        "435493EDC8E5DA5B34CF416EB6445C537A1F6ED9ABFCE02BB032DE2486C1B18C",
    ROOT / "engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv":
        "8A717BC9DFE3A43FB21A6B46EF723BD2649D5F1F5BC2174BBA6027D25550214F",
    ROOT / "engine/results/ftd_0639/ftd_0639_connected_block_analytic_dynamical_rest_v1.json":
        "DFA39E27F0317165D2A85E7778BBC7DA5691D1449DEEF20B4990C2AB9A1E7BD6",
    ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json":
        "AB43D342CFE48BEF452955E56B1EDC34F9EE51911F7D899932E7E542877E6B9A",
    ROOT / "engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv":
        "FE9F916443F8A8BF8F04B53067741919B203AF4C726D9DD67134B0BB43ECEFFD",
    ROOT / "engine/results/ftd_0641/ftd_0641_connected_block_independent_field_modes_v1.json":
        "EA24EF12476533DB8395C0E64C1E381A6605662EAA9ED35C1E38D66D560189E6",
}

ARTIFACTS = (
    RESULT_JSON, PREFLIGHT, HESSIAN, PROJECTED, CLUSTERS, METRICS,
    VECTORS, VECTOR_INDEX, GRAMS, EXECUTION_STATUS,
    PREFLIGHT_DERIVATIVE_STATUS, RUNTIME, ENERGY_CONTROL, CACHE_CONTROL,
    FIELD_CONTROL, KRYLOV_STATUS, HASHES,
)

PREFLIGHT_FIELDS = (
    "record_kind", "probe", "h", "direction", "sign", "valid",
    "common_residual", "energy_drift", "recovery", "gauss_pre",
    "gauss_clean", "hodge_correction", "reconstruction", "harmonic_face",
    "harmonic_edge", "sigma_min", "condition", "scale_difference",
    "observer_regression", "jacobian_refreshes", "jacobian_reuses",
    "cache_fallbacks", "k_norm", "energy_slope", "energy_second",
    "energy_relative", "derivative_scale_relative", "composition_residual",
    "detail",
)
HESSIAN_FIELDS = ("row", "column", "value")
PROJECTED_FIELDS = (
    "construction", "stage", "dimension", "matrix", "row", "column", "value",
)
CLUSTER_FIELDS = (
    "construction", "stage", "cluster_id", "rank", "index", "mu", "phase",
    "seed_overlap", "seed_linked", "eligible", "in_window", "candidate_id",
)
METRIC_FIELDS = (
    "candidate_id", "construction", "stage", "dimension", "cluster_id", "rank",
    "mu_min", "mu_max", "phase_mean", "phase_split", "seed_overlap",
    "ritz_residual", "prior_angle", "h1_angle", "sign_angle", "rotation_angle",
    "t_invariance", "tinv_invariance", "tinv_t_residual", "t_tinv_residual",
    "adjoint_residual", "orthogonality_residual", "modulus_residual",
    "conjugacy_residual", "conjugacy_separation", "intertwining_residual",
    "gram_min", "gram_max", "gram_ratio", "qualified", "detail",
)
VECTOR_INDEX_FIELDS = (
    "candidate_id", "construction", "stage", "vector_kind", "column",
    "chart_dimension", "byte_offset", "byte_length",
)
GRAM_FIELDS = (
    "candidate_id", "construction", "stage", "block", "row", "column", "value",
)
HASH_FIELDS = ("artifact", "sha256", "bytes")
EXECUTION_STATUS_FIELDS = (
    "evaluation_id", "record_kind", "construction", "stage", "operation",
    "power", "column", "h", "direction", "sign", "valid", "metadata",
    "sector", "finite", "gauss", "poisson_absolute", "endpoint_chart",
    "common_residual", "energy_drift", "recovery", "codec_gauss_pre",
    "codec_gauss_clean", "hodge_correction", "reconstruction",
    "harmonic_face", "harmonic_edge", "tangent_source_mean_rel",
    "hodge_source_mean_rel", "tangent_poisson_relative",
    "hodge_poisson_relative", "detail",
)
RUNTIME_FIELDS = ("name", "value")
ENERGY_CONTROL_FIELDS = (
    "probe", "sign", "h", "metadata", "sector", "finite", "gauss",
    "poisson_absolute", "increment",
)
CACHE_CONTROL_FIELDS = (
    "probe", "h", "direction", "sign", "retraction_metadata",
    "retraction_sector", "retraction_finite", "retraction_gauss",
    "retraction_poisson_absolute", "direct_accepted", "inverse_accepted",
    "observer_accepted", "endpoint_chart", "population_accepted", "reuse_accepted",
    "direct_population_agreement", "direct_reuse_agreement",
    "population_iterations", "cache_valid_after_population",
    "population_refreshes", "population_reuses", "population_fallbacks",
    "reuse_refreshes", "reuse_reuses", "reuse_fallbacks", "cache_semantics",
    "valid",
)
FIELD_CONTROL_FIELDS = (
    "record_kind", "tick", "target_amplitude", "q", "divergence",
    "signal_energy", "background_energy", "recovery_face_absolute",
    "recovery_edge_absolute", "recovery_relative", "maximum_divergence",
    "maximum_energy_drift",
)
KRYLOV_STATUS_FIELDS = (
    "construction", "generated_power_count", "accepted_dimension",
    "prior_dimension", "last_nonempty_power", "last_nonempty_start",
    "last_nonempty_end", "deflation_count", "happy_breakdown",
    "exhausted_16_powers", "bookkeeping_valid", "projected_final_present",
    "terminal_t_invariance", "terminal_tinv_invariance",
    "terminal_invariance_eligible",
)

L = 17
N_SITE = L**3
D_RAW = 6 * N_SITE + 96
D_INDEPENDENT = D_RAW - (N_SITE - 1)
M_INERTIAL = 0.511
C_SPEED = 1.0 / math.sqrt(3.0)
G_C = 0.0854245431028543695
PHI_INT = 1.0911648733663635
PHASE_WINDOW = 0.08
H0 = 2e-6
H1 = 1e-6
H_ENERGY = 2e-4
PROBE_NAMES = (
    "q6", "q7", "p6", "p7",
    "matter_mix_0", "matter_mix_1", "matter_mix_2", "matter_mix_3",
    "f_e", "f_b", "f_e_plus_f_b", "f_e_minus_f_b",
    "h_E_x", "h_B_x", "q6_plus_f_e", "p6_plus_f_b",
)

INVALID = "L17_COMPLETE_TANGENT_EXECUTION_INVALID"
UNRESOLVED = "L17_FIRST_DOUBLET_TANGENT_SOLVE_UNRESOLVED"
NOT_QUALIFIED = "L17_FIRST_DOUBLET_LOCKED_CANDIDATES_NOT_QUALIFIED"
CONSTRUCTIVE = "L17_FIRST_DOUBLET_POSITIVE_TANGENT_CANDIDATE_CONSTRUCTIVE"
COMPANION = "PRODUCTION_NATIVE_BRIDGE_OPEN"
NUMERIC_FAILURE_DETAILS = frozenset({
    "acceptable_column_above_cap",
    "inconsistent_krylov_bookkeeping",
    "invalid_candidate_dimension",
    "invalid_general_real4_eigensolver",
    "invalid_seed_block_dimension",
    "invalid_symmetric_eigensolver",
    "nonfinite_candidate_gram",
    "nonfinite_candidate_match",
    "nonfinite_candidate_metric",
    "nonfinite_filter_output",
    "nonfinite_intertwining_residual",
    "nonfinite_normalized_basis",
    "nonfinite_operator_average",
    "nonfinite_or_negative_candidate_norm",
    "nonfinite_or_negative_candidate_total_norm",
    "nonfinite_negative_or_zero_candidate_block_norm",
    "nonfinite_or_negative_k_norm",
    "nonfinite_projected_matrix",
    "nonfinite_terminal_invariance",
    "postflight_execution_failure",
    "postflight_artifact_write_failure",
    "postflight_artifact_schema_failure",
    "postflight_manifest_or_hash_failure",
    "postflight_artifact_reset_failure",
    "postflight_status_write_failure",
})


class CertificateError(RuntimeError):
    """A fail-closed artifact or replay error."""


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def truth(value: Any) -> bool:
    if isinstance(value, bool):
        return value
    return str(value).strip().lower() in {"1", "true", "yes", "pass"}


def finite_float(value: Any, label: str) -> float:
    try:
        number = float(value)
    except (TypeError, ValueError) as exc:
        raise CertificateError(f"{label}: expected finite float, got {value!r}") from exc
    if not math.isfinite(number):
        raise CertificateError(f"{label}: nonfinite value {value!r}")
    return number


def optional_float(row: dict[str, str], field: str) -> float | None:
    value = row[field].strip()
    return None if value == "" else finite_float(value, field)


def strict_csv_bool(row: dict[str, str], field: str) -> bool:
    value = row[field].strip()
    if value not in {"0", "1"}:
        raise CertificateError(
            f"{row.get('record_kind', 'row')}/{field}: expected 0 or 1, got {value!r}"
        )
    return value == "1"


def optional_csv_bool(row: dict[str, str], field: str) -> bool | None:
    if row[field].strip() == "":
        return None
    return strict_csv_bool(row, field)


def optional_int(row: dict[str, str], field: str) -> int | None:
    value = row[field].strip()
    if value == "":
        return None
    if re.fullmatch(r"-?(?:0|[1-9][0-9]*)", value) is None:
        raise CertificateError(
            f"{row.get('record_kind', 'row')}/{field}: expected integer, got {value!r}"
        )
    return int(value)


def strict_int_text(value: str, label: str) -> int:
    """Parse a canonical base-ten integer without float truncation."""
    if re.fullmatch(r"-?(?:0|[1-9][0-9]*)", value) is None:
        raise CertificateError(f"{label}: expected exact integer, got {value!r}")
    return int(value)


def expected_prior_dimension(accepted: int, last_start: int) -> int:
    return 48 if accepted == 64 else (0 if accepted <= 4 else last_start)


def krylov_structure_replay(
    generated: int, accepted: int, prior: int, last_power: int,
    last_start: int, last_end: int, deflations: int,
    happy: bool, exhausted: bool,
) -> bool:
    """Exact block-Krylov dimension/termination bookkeeping."""
    nonempty_shape = (
        last_power == -1 and last_start == 0 and last_end == 0
        if accepted == 0
        else (
            0 <= last_power < generated
            and 0 <= last_start < last_end == accepted
            and 1 <= last_end - last_start <= 4
        )
    )
    terminal_power = (
        last_power == generated - 2 if happy
        else last_power == generated - 1
    )
    return bool(
        1 <= generated <= 16 and 0 <= accepted <= 64
        and prior == expected_prior_dimension(accepted, last_start)
        and nonempty_shape and terminal_power
        and deflations >= 0
        and deflations == 4 * generated - accepted
        and not (happy and exhausted)
        and exhausted == (generated == 16 and not happy)
        and (happy or exhausted or accepted == 64)
    )


def harmonic_detail_replay(row: dict[str, str]) -> tuple[float, float, float, float, float, float]:
    """Rebuild the two relative harmonic-coordinate residuals from hex payloads."""
    pieces = row["detail"].split(";")
    expected_order = (
        "face_raw", "face_rebuilt", "edge_raw", "edge_rebuilt",
        "tangent_source_mean_abs", "tangent_source_mean_rel",
        "hodge_source_mean_abs", "hodge_source_mean_rel",
    )
    if len(pieces) != 9 or pieces[0] not in {"pass", "fail"}:
        raise CertificateError(f"malformed harmonic detail grammar {row['detail']!r}")
    encoded: dict[str, list[float]] = {}
    observed_order: list[str] = []
    hexfloat = re.compile(r"^[+-]?0x[0-9a-f]+(?:\.[0-9a-f]*)?p[+-]?\d+$", re.I)
    for piece in pieces[1:]:
        if "=" not in piece:
            raise CertificateError(f"malformed harmonic detail component {piece!r}")
        name, payload = piece.split("=", 1)
        observed_order.append(name)
        tokens = payload.split("/")
        expected_count = 3 if name in expected_order[:4] else 1
        if len(tokens) != expected_count or any(hexfloat.fullmatch(token) is None for token in tokens):
            raise CertificateError(f"non-hex harmonic payload {piece!r}")
        try:
            values = [float.fromhex(value) for value in tokens]
        except ValueError as exc:
            raise CertificateError(f"malformed harmonic hex payload {piece!r}") from exc
        encoded[name] = values
    required_names = set(expected_order)
    if tuple(observed_order) != expected_order or set(encoded) != required_names:
        raise CertificateError(
            f"{row.get('probe', row.get('operation', '?'))}/"
            f"{row.get('h', '')}/{row.get('direction', '')}: "
            "incomplete harmonic detail"
        )

    def residual(raw: list[float], rebuilt: list[float]) -> float:
        numerator = max(abs(lhs - rhs) for lhs, rhs in zip(raw, rebuilt))
        denominator = max([abs(value) for value in raw] + [1e-30])
        return numerator / denominator

    return (
        residual(encoded["face_raw"], encoded["face_rebuilt"]),
        residual(encoded["edge_raw"], encoded["edge_rebuilt"]),
        encoded["tangent_source_mean_abs"][0],
        encoded["tangent_source_mean_rel"][0],
        encoded["hodge_source_mean_abs"][0],
        encoded["hodge_source_mean_rel"][0],
    )


def fro_relative(lhs: np.ndarray, rhs: np.ndarray, floor: float = 1e-30) -> float:
    return float(np.linalg.norm(lhs - rhs) / max(np.linalg.norm(rhs), floor))


def k_gram_square(gram: np.ndarray, label: str) -> float:
    """Return a serialized block's K-square without clipping any diagonal."""
    if gram.ndim != 2 or gram.shape[0] != gram.shape[1]:
        raise CertificateError(f"{label}: K Gram is not square")
    if not np.all(np.isfinite(gram)):
        raise CertificateError(f"{label}: nonfinite K Gram")
    if np.any(np.diag(gram) < 0.0):
        raise CertificateError(f"{label}: negative K square")
    if fro_relative(gram, gram.T) > 1e-10:
        raise CertificateError(f"{label}: asymmetric K Gram")
    square = float(np.trace(gram))
    if not math.isfinite(square) or square < 0.0:
        raise CertificateError(f"{label}: invalid total K square")
    return square


def terminal_invariance_replay(
    tv_gram: np.ndarray, t_residual_gram: np.ndarray,
    tinv_gram: np.ndarray, tinv_residual_gram: np.ndarray,
    label: str,
) -> tuple[float, float]:
    """Direct terminal invariance from serialized image/residual self-Grams."""
    tv_square = k_gram_square(tv_gram, f"{label}/TV")
    t_residual_square = k_gram_square(
        t_residual_gram, f"{label}/T residual"
    )
    tinv_square = k_gram_square(tinv_gram, f"{label}/TINV")
    tinv_residual_square = k_gram_square(
        tinv_residual_gram, f"{label}/TINV residual"
    )
    if tv_square <= 0.0 or tinv_square <= 0.0:
        raise CertificateError(f"{label}: terminal image K norm is nonpositive")
    return (
        math.sqrt(t_residual_square) / math.sqrt(tv_square),
        math.sqrt(tinv_residual_square) / math.sqrt(tinv_square),
    )


def read_csv_exact(path: Path, fields: tuple[str, ...]) -> list[dict[str, str]]:
    with path.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        if tuple(reader.fieldnames or ()) != fields:
            raise CertificateError(
                f"{path.name}: schema mismatch; expected {fields}, got {reader.fieldnames}"
            )
        rows: list[dict[str, str]] = []
        for line_number, row in enumerate(reader, start=2):
            if None in row or any(row.get(field) is None for field in fields):
                raise CertificateError(
                    f"{path.name}:{line_number}: row does not have exactly "
                    f"{len(fields)} fields"
                )
            rows.append(row)
        return rows


def matrix_from_rows(
    rows: Iterable[dict[str, str]], row_field: str = "row", column_field: str = "column"
) -> np.ndarray:
    entries = list(rows)
    if not entries:
        return np.empty((0, 0), dtype=np.float64)
    coordinates: dict[tuple[int, int], float] = {}
    for entry in entries:
        key = (int(entry[row_field]), int(entry[column_field]))
        if key in coordinates:
            raise CertificateError(f"duplicate matrix coordinate {key}")
        coordinates[key] = finite_float(entry["value"], "matrix value")
    nrow = max(row for row, _ in coordinates) + 1
    ncol = max(column for _, column in coordinates) + 1
    if len(coordinates) != nrow * ncol:
        raise CertificateError("matrix coordinates are not a complete rectangle")
    result = np.empty((nrow, ncol), dtype=np.float64)
    for (row, column), value in coordinates.items():
        result[row, column] = value
    return result


@dataclass(frozen=True)
class CandidateKey:
    candidate_id: str
    construction: str
    stage: str


@dataclass
class CandidateReplay:
    key: CandidateKey
    dimension: int
    cluster_id: int
    mus: np.ndarray
    phase_mean: float
    phase_split: float
    seed_overlap: float
    ritz_residual: float
    t_invariance: float
    tinv_invariance: float
    tinv_t_residual: float
    t_tinv_residual: float
    adjoint_residual: float
    orthogonality_residual: float
    modulus_residual: float
    conjugacy_residual: float
    conjugacy_separation: float
    gram_min: float
    gram_max: float
    gram_ratio: float
    phases: np.ndarray
    core_qualified: bool
    prior_angle: float = math.inf
    h1_angle: float = math.inf
    sign_angle: float = math.inf
    rotation_angle: float = math.inf
    intertwining_residual: float = math.inf
    matched_prior: CandidateKey | None = None
    matched_h1: CandidateKey | None = None
    matched_sign: CandidateKey | None = None
    matched_rotation: CandidateKey | None = None
    qualified: bool = False


@dataclass(frozen=True)
class KrylovReplay:
    construction: str
    generated: int
    accepted: int
    prior: int
    last_power: int
    last_start: int
    last_end: int
    deflations: int
    happy: bool
    exhausted: bool
    structural: bool
    projected_final: bool
    terminal_t: float | None
    terminal_tinv: float | None
    terminal_eligible_reported: bool


class KForm:
    def __init__(self, hessian: np.ndarray, beta: float, lam: float):
        if hessian.shape != (48, 48):
            raise CertificateError(f"H_red shape is {hessian.shape}, expected (48, 48)")
        if not (math.isfinite(beta) and beta > 0.0):
            raise CertificateError("beta must be finite and positive")
        self.hessian = hessian
        self.beta = beta
        self.lam = lam

    @staticmethod
    def curl(edge_flat: np.ndarray) -> np.ndarray:
        """Engine matched backward-difference curl C."""
        edge = edge_flat.reshape((3, L, L, L))
        ex, ey, ez = edge
        out_x = ez - np.roll(ez, 1, axis=1) - ey + np.roll(ey, 1, axis=2)
        out_y = ex - np.roll(ex, 1, axis=2) - ez + np.roll(ez, 1, axis=0)
        out_z = ey - np.roll(ey, 1, axis=0) - ex + np.roll(ex, 1, axis=1)
        return np.stack((out_x, out_y, out_z)).reshape(3 * N_SITE)

    @staticmethod
    def curl_adjoint(face_flat: np.ndarray) -> np.ndarray:
        """Exact periodic transpose C^T of the engine's backward curl."""
        face = face_flat.reshape((3, L, L, L))
        fx, fy, fz = face
        out_x = np.roll(fz, -1, axis=1) - fz - np.roll(fy, -1, axis=2) + fy
        out_y = np.roll(fx, -1, axis=2) - fx - np.roll(fz, -1, axis=0) + fz
        out_z = np.roll(fy, -1, axis=0) - fy - np.roll(fx, -1, axis=1) + fx
        return np.stack((out_x, out_y, out_z)).reshape(3 * N_SITE)

    def block(self, left: np.ndarray, right: np.ndarray) -> np.ndarray:
        if left.ndim == 1:
            left = left[:, None]
        if right.ndim == 1:
            right = right[:, None]
        if left.shape[0] != D_RAW or right.shape[0] != D_RAW:
            raise CertificateError("K input has the wrong raw chart dimension")
        ldx, ldp = left[:48], left[48:96]
        rdx, rdp = right[:48], right[48:96]
        e0, e1 = 96, 96 + 3 * N_SITE
        le, lb = left[e0:e1], left[e1:]
        re, rb = right[e0:e1], right[e1:]
        cte_r = np.column_stack([self.curl_adjoint(re[:, j]) for j in range(re.shape[1])])
        cte_l = np.column_stack([self.curl_adjoint(le[:, j]) for j in range(le.shape[1])])
        return (
            ldp.T @ rdp / M_INERTIAL
            + ldx.T @ self.hessian @ rdx
            + self.beta
            * (
                le.T @ re
                + lb.T @ rb
                - self.lam * (lb.T @ cte_r + cte_l.T @ rb) / 2.0
            )
        )

    def norm_f(self, value: np.ndarray) -> float:
        gram = self.block(value, value)
        diagonal = np.diag(gram)
        if not np.all(np.isfinite(diagonal)):
            raise CertificateError("nonfinite K square")
        if np.any(diagonal < 0.0):
            raise CertificateError(
                f"negative K square {float(np.min(diagonal))}"
            )
        trace = float(np.trace(gram))
        if not math.isfinite(trace):
            raise CertificateError("nonfinite K norm square")
        if trace < 0.0:
            raise CertificateError(f"negative K norm square {trace}")
        return math.sqrt(trace)


def conjugate_pairing(eigenvalues: np.ndarray) -> tuple[float, float, list[tuple[int, int]]]:
    pairings = (
        [(0, 1), (2, 3)],
        [(0, 2), (1, 3)],
        [(0, 3), (1, 2)],
    )
    scored: list[tuple[float, list[tuple[int, int]]]] = []
    for pairing in pairings:
        residual = max(
            abs(eigenvalues[i] - np.conj(eigenvalues[j]))
            / max(abs(eigenvalues[i]), abs(eigenvalues[j]), 1e-30)
            for i, j in pairing
        )
        scored.append((float(residual), pairing))
    scored.sort(key=lambda item: item[0])
    return scored[0][0], scored[1][0] - scored[0][0], scored[0][1]


@dataclass(frozen=True)
class ExecutionEvaluation:
    evaluation_id: int
    construction: str
    stage: str
    operation: str
    power: int | None
    column: int
    h: float
    direction: str
    passed: bool


def replay_derivative_bundle(
    group: list[dict[str, str]], evaluation_id: int
) -> ExecutionEvaluation:
    """Reconstruct one exact five-row centered-derivative evaluation."""
    expected_layout = (
        ("retraction", 1), ("endpoint", 1), ("retraction", -1),
        ("endpoint", -1), ("codec", 0),
    )
    if len(group) != 5 or tuple(
        (row["record_kind"], optional_int(row, "sign")) for row in group
    ) != expected_layout:
        raise CertificateError(
            f"derivative evaluation {evaluation_id} has the wrong five-row layout"
        )
    if any(optional_int(row, "evaluation_id") != evaluation_id for row in group):
        raise CertificateError(f"derivative evaluation {evaluation_id} has mixed IDs")
    common_fields = (
        "construction", "stage", "operation", "power", "column", "h",
        "direction",
    )
    if any(
        tuple(row[field] for field in common_fields)
        != tuple(group[0][field] for field in common_fields)
        for row in group[1:]
    ):
        raise CertificateError(
            f"derivative evaluation {evaluation_id} has inconsistent metadata"
        )
    construction = group[0]["construction"]
    stage = group[0]["stage"]
    operation = group[0]["operation"]
    power = optional_int(group[0], "power")
    column = optional_int(group[0], "column")
    h = optional_float(group[0], "h")
    direction = group[0]["direction"]
    if column is None or h is None or direction not in {"forward", "reverse"}:
        raise CertificateError(f"derivative evaluation {evaluation_id} lacks coordinates")

    component_passes: list[bool] = []
    for row in (group[0], group[2]):
        metadata = optional_csv_bool(row, "metadata")
        sector = optional_csv_bool(row, "sector")
        finite = optional_csv_bool(row, "finite")
        gauss = optional_float(row, "gauss")
        poisson = optional_float(row, "poisson_absolute")
        replayed = bool(
            metadata is True and sector is True and finite is True
            and gauss is not None and 0.0 <= gauss <= 1e-10
            and poisson is not None and 0.0 <= poisson <= 1e-13
        )
        if strict_csv_bool(row, "valid") != replayed:
            raise CertificateError(
                f"derivative evaluation {evaluation_id} retraction flag mismatch"
            )
        component_passes.append(replayed)
    for row in (group[1], group[3]):
        direct_step = optional_csv_bool(row, "metadata")
        inverse_step = optional_csv_bool(row, "sector")
        endpoint_chart = optional_csv_bool(row, "endpoint_chart")
        common = optional_float(row, "common_residual")
        drift = optional_float(row, "energy_drift")
        recovery = optional_float(row, "recovery")
        replayed = bool(
            direct_step is True and inverse_step is True
            and endpoint_chart is True
            and common is not None and 0.0 <= common <= 1e-10
            and drift is not None and 0.0 <= drift <= 1e-12
            and recovery is not None and 0.0 <= recovery <= 1e-10
        )
        if strict_csv_bool(row, "valid") != replayed:
            raise CertificateError(
                f"derivative evaluation {evaluation_id} endpoint flag mismatch"
            )
        component_passes.append(replayed)

    codec = group[4]
    codec_finite = optional_csv_bool(codec, "finite")
    codec_fields = (
        "codec_gauss_pre", "codec_gauss_clean", "hodge_correction",
        "reconstruction", "harmonic_face", "harmonic_edge",
        "tangent_source_mean_rel", "hodge_source_mean_rel",
        "tangent_poisson_relative", "hodge_poisson_relative",
    )
    codec_values = [optional_float(codec, field) for field in codec_fields]
    codec_replayed = False
    if codec_finite is True and all(
        value is not None and value >= 0.0 for value in codec_values
    ):
        (
            gauss_pre, gauss_clean, hodge, reconstruction, harmonic_face,
            harmonic_edge, tangent_mean, hodge_mean, tangent_poisson,
            hodge_poisson,
        ) = (float(value) for value in codec_values)
        detail = harmonic_detail_replay(codec)
        codec_replayed = bool(
            abs(harmonic_face - detail[0]) <= 2e-15
            and abs(harmonic_edge - detail[1]) <= 2e-15
            and all(value >= 0.0 for value in detail[2:])
            and abs(tangent_mean - detail[3]) <= 2e-15
            and abs(hodge_mean - detail[5]) <= 2e-15
            and gauss_pre <= 2e-7 and gauss_clean <= 1e-10
            and hodge <= 2e-4 and reconstruction <= 2e-4
            and harmonic_face <= 1e-12 and harmonic_edge <= 1e-12
            and tangent_mean <= 1e-13 and hodge_mean <= 1e-13
            and tangent_poisson <= 1e-13 and hodge_poisson <= 1e-13
        )
        if codec["detail"].split(";", 1)[0] != (
            "pass" if codec_replayed else "fail"
        ):
            raise CertificateError(
                f"derivative evaluation {evaluation_id} codec detail mismatch"
            )
    if strict_csv_bool(codec, "valid") != codec_replayed:
        raise CertificateError(
            f"derivative evaluation {evaluation_id} codec flag mismatch"
        )
    component_passes.append(codec_replayed)
    return ExecutionEvaluation(
        evaluation_id=evaluation_id,
        construction=construction,
        stage=stage,
        operation=operation,
        power=power,
        column=column,
        h=h,
        direction=direction,
        passed=all(component_passes),
    )


def replay_numeric_failure(row: dict[str, str]) -> None:
    """Validate a one-row non-derivative failure from its primitive diagnostic."""
    if strict_csv_bool(row, "valid"):
        raise CertificateError("numeric execution failure is marked valid")
    if optional_int(row, "sign") != 0 or row["direction"] != "forward":
        raise CertificateError("numeric execution failure has invalid sign/direction")
    construction = row["construction"]
    if construction not in {"primary", "h1", "sign", "rotation", "artifact"}:
        raise CertificateError(f"unknown numeric-failure construction {construction!r}")
    h = optional_float(row, "h")
    if h != (H1 if construction == "h1" else H0):
        raise CertificateError("numeric execution failure violates the h lock")
    finite = optional_csv_bool(row, "finite")
    if finite is None:
        raise CertificateError("numeric execution failure omits diagnostic finiteness")
    diagnostic = optional_float(row, "gauss")
    secondary = optional_float(row, "poisson_absolute")
    power = optional_int(row, "power")
    column = optional_int(row, "column")
    detail = row["detail"]
    operation = row["operation"]
    stage = row["stage"]

    def integral(value: float | None, label: str) -> int:
        if value is None or not value.is_integer():
            raise CertificateError(f"bookkeeping failure has nonintegral {label}")
        return int(value)

    predicate = False
    used_fields = {"finite", "gauss", "detail"}
    if detail == "nonfinite_or_negative_k_norm":
        predicate = bool(
            operation == "mgs_norm" and stage == "krylov"
            and power is not None and 0 <= power <= 15
            and column is not None and 0 <= column <= 3
            and ((not finite and diagnostic is None)
                 or (finite and diagnostic is not None and diagnostic < 0.0))
        )
    elif detail == "acceptable_column_above_cap":
        used_fields.add("poisson_absolute")
        predicate = bool(
            operation == "basis_cap" and stage == "krylov"
            and power is not None and 0 <= power <= 15
            and column is not None and 0 <= column <= 3
            and finite and diagnostic is not None and diagnostic >= 64.0
            and secondary is not None and secondary >= 1e-12
        )
    elif detail == "inconsistent_krylov_bookkeeping":
        used_fields.update({
            "poisson_absolute", "common_residual", "energy_drift", "recovery",
            "codec_gauss_pre", "codec_gauss_clean",
        })
        generated = power
        accepted = column
        prior = integral(diagnostic, "prior dimension")
        last_power = integral(secondary, "last power")
        last_start = integral(optional_float(row, "common_residual"), "last start")
        last_end = integral(optional_float(row, "energy_drift"), "last end")
        deflations = integral(optional_float(row, "recovery"), "deflations")
        happy = integral(optional_float(row, "codec_gauss_pre"), "happy flag")
        exhausted = integral(
            optional_float(row, "codec_gauss_clean"), "exhausted flag"
        )
        if generated is None or accepted is None:
            raise CertificateError("bookkeeping failure omits generated/accepted")
        structural = bool(
            happy in (0, 1) and exhausted in (0, 1)
            and krylov_structure_replay(
                generated, accepted, prior, last_power, last_start,
                last_end, deflations, bool(happy), bool(exhausted),
            )
        )
        predicate = bool(
            operation == "bookkeeping" and stage == "krylov"
            and finite and not structural
        )
    elif detail == "nonfinite_projected_matrix":
        predicate = bool(
            operation == "projected_nonfinite" and stage in {"prior", "final"}
            and power is None and column is None and not finite
            and diagnostic is None
        )
    elif detail == "invalid_symmetric_eigensolver":
        predicate = bool(
            operation in {
                "symmetric_eigensolver", "candidate_symmetric_eigensolver",
                "candidate_gram_eigensolver",
            }
            and stage in {"prior", "final"} and power is None and column is None
            and not finite
        )
    elif detail == "nonfinite_terminal_invariance":
        predicate = bool(
            operation == "terminal_invariance" and stage == "final"
            and power is None and column is None and not finite
            and diagnostic is None
        )
    elif detail == "invalid_general_real4_eigensolver":
        predicate = bool(
            operation == "general_real4_eigensolver"
            and stage in {"prior", "final"} and power is None and column is None
            and ((not finite and diagnostic is None)
                 or (finite and diagnostic is not None and diagnostic > 1e-10))
        )
    elif detail == "invalid_candidate_dimension":
        predicate = bool(
            operation == "candidate_dimension" and stage in {"prior", "final"}
            and power is None and column is None and finite
            and diagnostic is not None and diagnostic.is_integer()
            and int(diagnostic) != 4
        )
    elif detail == "nonfinite_candidate_gram":
        predicate = bool(
            operation == "candidate_gram" and stage in {"prior", "final"}
            and power is None and column is None and not finite
            and diagnostic is None
        )
    elif detail == "nonfinite_or_negative_candidate_norm":
        predicate = bool(
            operation == "candidate_ritz_norm" and stage in {"prior", "final"}
            and power is None and column is not None and 0 <= column <= 3
            and ((not finite and diagnostic is None)
                 or (finite and diagnostic is not None and diagnostic < 0.0))
        )
    elif detail == "nonfinite_or_negative_candidate_total_norm":
        predicate = bool(
            operation == "candidate_ritz_total"
            and stage in {"prior", "final"}
            and power is None and column is None
            and ((not finite and diagnostic is None)
                 or (finite and diagnostic is not None and diagnostic < 0.0))
        )
    elif detail == "nonfinite_negative_or_zero_candidate_block_norm":
        used_fields.add("poisson_absolute")
        predicate = bool(
            operation == "candidate_block_norm"
            and stage in {"prior", "final"}
            and power is None and column is None
            and (
                (not finite and diagnostic is None)
                or (finite and diagnostic is not None and diagnostic <= 0.0)
                or secondary is None or secondary <= 0.0
            )
        )
    elif detail == "nonfinite_normalized_basis":
        predicate = bool(
            operation == "normalized_basis" and stage == "krylov"
            and power is not None and 0 <= power <= 15
            and column is not None and 0 <= column <= 3
            and not finite and diagnostic is None
        )
    elif detail == "nonfinite_operator_average":
        coordinate_ok = bool(
            (
                stage == "krylov" and power is not None and 1 <= power <= 15
                and column is not None and 0 <= column <= 3
            )
            or (
                stage in {"prior", "final"} and power is None
                and column is not None and column >= 0
            )
        )
        predicate = bool(
            operation == "operator_average" and coordinate_ok
            and not finite and diagnostic is None
        )
    elif detail == "nonfinite_filter_output":
        predicate = bool(
            operation == "filter_output" and stage == "krylov"
            and power is not None and 1 <= power <= 15
            and column is not None and 0 <= column <= 3
            and not finite and diagnostic is None
        )
    elif detail == "nonfinite_candidate_metric":
        predicate = bool(
            operation == "candidate_metric" and stage in {"prior", "final"}
            and power is None and column is None
            and not finite and diagnostic is None
        )
    elif detail == "invalid_seed_block_dimension":
        predicate = bool(
            operation == "seed_block" and stage == "krylov"
            and power is None and column is None and finite
            and diagnostic is not None and diagnostic.is_integer()
            and int(diagnostic) != 4
        )
    elif detail == "nonfinite_candidate_match":
        predicate = bool(
            operation == "candidate_match" and stage == "final"
            and power is None and column is None
            and not finite and diagnostic is None
        )
    elif detail == "nonfinite_intertwining_residual":
        predicate = bool(
            operation == "intertwining" and stage == "final"
            and power is None and column is None
            and not finite and diagnostic is None
        )
    elif detail in {
        "postflight_execution_failure",
        "postflight_artifact_write_failure",
        "postflight_artifact_schema_failure",
        "postflight_manifest_or_hash_failure",
        "postflight_artifact_reset_failure",
        "postflight_status_write_failure",
    }:
        used_fields.add("poisson_absolute")
        operation_by_detail = {
            "postflight_execution_failure": ("postflight_execution", 0.0),
            "postflight_artifact_write_failure": ("artifact_write", 1.0),
            "postflight_artifact_schema_failure": ("artifact_schema", 2.0),
            "postflight_manifest_or_hash_failure": ("artifact_manifest", 3.0),
            "postflight_artifact_reset_failure": ("artifact_reset", 4.0),
            "postflight_status_write_failure": ("status_write", 1.5),
        }
        expected_operation, expected_code = operation_by_detail[detail]
        predicate = bool(
            construction == "artifact" and stage == "final"
            and operation == expected_operation
            and power is None and column is None and h == H0
            and finite and diagnostic == expected_code and secondary == 1.0
        )
    else:
        raise CertificateError(f"unknown numeric execution failure {detail!r}")
    if not predicate:
        raise CertificateError(
            f"numeric execution failure {detail!r} is not proven by its primitives"
        )

    diagnostic_fields = {
        "metadata", "sector", "finite", "gauss", "poisson_absolute",
        "endpoint_chart", "common_residual", "energy_drift", "recovery",
        "codec_gauss_pre", "codec_gauss_clean", "hodge_correction",
        "reconstruction", "harmonic_face", "harmonic_edge",
        "tangent_source_mean_rel", "hodge_source_mean_rel",
        "tangent_poisson_relative", "hodge_poisson_relative", "detail",
    }
    if any(row[field] != "" for field in diagnostic_fields - used_fields):
        raise CertificateError(
            f"numeric execution failure {detail!r} carries extraneous diagnostics"
        )


def replay_execution_status(
    rows: list[dict[str, str]], preflight_ready: bool
) -> tuple[bool, bool, list[ExecutionEvaluation]]:
    """Replay the component ledger without using a producer aggregate flag."""
    if not preflight_ready:
        if rows:
            raise CertificateError(
                "post-preflight execution ledger must be empty when preflight did not pass"
            )
        return False, False, []
    if not rows:
        raise CertificateError("passing preflight has no post-preflight execution terminal")

    terminal = rows[-1]
    if terminal["record_kind"] != "terminal" or any(
        row["record_kind"] == "terminal" for row in rows[:-1]
    ):
        raise CertificateError("execution ledger must end in exactly one terminal row")
    terminal_id = optional_int(terminal, "evaluation_id")
    terminal_valid = strict_csv_bool(terminal, "valid")
    if terminal_id is None:
        raise CertificateError("execution terminal is missing its next evaluation ID")
    if not (
        terminal["construction"] == "all"
        and terminal["stage"] == "final"
        and terminal["operation"] in {"run_complete", "run_abort"}
        and terminal["power"] == ""
        and terminal["column"] == ""
        and terminal["h"] == ""
        and terminal["direction"] == ""
        and optional_int(terminal, "sign") == 0
        and terminal_valid == (terminal["operation"] == "run_complete")
    ):
        raise CertificateError("malformed execution terminal semantics")
    terminal_diagnostics = (
        "metadata", "sector", "finite", "gauss", "poisson_absolute",
        "endpoint_chart", "common_residual", "energy_drift", "recovery",
        "codec_gauss_pre", "codec_gauss_clean", "hodge_correction",
        "reconstruction", "harmonic_face", "harmonic_edge",
        "tangent_source_mean_rel", "hodge_source_mean_rel",
        "tangent_poisson_relative", "hodge_poisson_relative", "detail",
    )
    if any(terminal[field] != "" for field in terminal_diagnostics):
        raise CertificateError("execution terminal carries forbidden diagnostics")
    terminal_complete = terminal["operation"] == "run_complete"

    body = rows[:-1]
    groups_by_id: dict[int, list[dict[str, str]]] = {}
    observed_id_order: list[int] = []
    for row in body:
        evaluation_id = optional_int(row, "evaluation_id")
        if evaluation_id is None:
            raise CertificateError("execution ledger row omits evaluation ID")
        if evaluation_id not in groups_by_id:
            observed_id_order.append(evaluation_id)
            groups_by_id[evaluation_id] = []
        groups_by_id[evaluation_id].append(row)
    if observed_id_order != list(range(terminal_id)):
        raise CertificateError("execution evaluation IDs are not zero-based contiguous")
    expected_layout = (
        ("retraction", 1), ("endpoint", 1), ("retraction", -1),
        ("endpoint", -1), ("codec", 0),
    )
    evaluations: list[ExecutionEvaluation] = []
    numeric_failure_ids: list[int] = []
    numeric_failure_details: list[tuple[int, str]] = []
    for evaluation_id in range(terminal_id):
        group = groups_by_id[evaluation_id]
        if len(group) == 1 and group[0]["record_kind"] == "numeric":
            if terminal_complete:
                raise CertificateError("run_complete contains a numeric failure row")
            replay_numeric_failure(group[0])
            numeric_failure_ids.append(evaluation_id)
            numeric_failure_details.append((evaluation_id, group[0]["detail"]))
            continue
        if len(group) != 5:
            raise CertificateError(
                f"execution evaluation {evaluation_id} is neither a five-row "
                "derivative bundle nor one numeric failure row"
            )
        common_fields = (
            "construction", "stage", "operation", "power", "column", "h",
            "direction",
        )
        if any(
            tuple(row[field] for field in common_fields)
            != tuple(group[0][field] for field in common_fields)
            for row in group[1:]
        ):
            raise CertificateError(
                f"execution evaluation {evaluation_id} has inconsistent group metadata"
            )
        if tuple(
            (row["record_kind"], optional_int(row, "sign")) for row in group
        ) != expected_layout:
            raise CertificateError(
                f"execution evaluation {evaluation_id} has the wrong five-row layout"
            )

        construction = group[0]["construction"]
        stage = group[0]["stage"]
        operation = group[0]["operation"]
        power = optional_int(group[0], "power")
        column = optional_int(group[0], "column")
        h = optional_float(group[0], "h")
        direction = group[0]["direction"]
        if construction not in {"primary", "h1", "sign", "rotation"}:
            raise CertificateError(f"unknown execution construction {construction!r}")
        expected_h = H1 if construction == "h1" else H0
        if h is None or h != expected_h or direction not in {"forward", "reverse"}:
            raise CertificateError(
                f"execution evaluation {evaluation_id} violates h/direction locks"
            )
        if column is None:
            raise CertificateError(f"execution evaluation {evaluation_id} has no column")
        if stage == "krylov":
            structural = (
                operation in {"filter_v", "filter_w"}
                and power is not None and 1 <= power <= 15
                and 0 <= column <= 3
            )
        elif stage in {"prior", "final"} and operation == "basis_image":
            structural = power is None and column >= 0
        elif stage in {"prior", "final"} and re.fullmatch(
            r"candidate_(?:image|composition):[^,]+", operation
        ):
            structural = power is None and 0 <= column <= 3
        else:
            structural = False
        if not structural:
            raise CertificateError(
                f"execution evaluation {evaluation_id} has invalid operation metadata"
            )

        component_passes: list[bool] = []
        for row in (group[0], group[2]):
            metadata = optional_csv_bool(row, "metadata")
            sector = optional_csv_bool(row, "sector")
            finite = optional_csv_bool(row, "finite")
            gauss = optional_float(row, "gauss")
            poisson = optional_float(row, "poisson_absolute")
            replayed = bool(
                metadata is True and sector is True and finite is True
                and gauss is not None and 0.0 <= gauss <= 1e-10
                and poisson is not None and 0.0 <= poisson <= 1e-13
            )
            if strict_csv_bool(row, "valid") != replayed:
                raise CertificateError(
                    f"execution evaluation {evaluation_id} retraction flag mismatch"
                )
            component_passes.append(replayed)
        for row in (group[1], group[3]):
            direct_step = optional_csv_bool(row, "metadata")
            inverse_step = optional_csv_bool(row, "sector")
            endpoint_chart = optional_csv_bool(row, "endpoint_chart")
            common = optional_float(row, "common_residual")
            drift = optional_float(row, "energy_drift")
            recovery = optional_float(row, "recovery")
            replayed = bool(
                direct_step is True and inverse_step is True
                and endpoint_chart is True
                and common is not None and 0.0 <= common <= 1e-10
                and drift is not None and 0.0 <= drift <= 1e-12
                and recovery is not None and 0.0 <= recovery <= 1e-10
            )
            if strict_csv_bool(row, "valid") != replayed:
                raise CertificateError(
                    f"execution evaluation {evaluation_id} endpoint flag mismatch"
                )
            component_passes.append(replayed)

        codec = group[4]
        codec_finite = optional_csv_bool(codec, "finite")
        codec_fields = (
            "codec_gauss_pre", "codec_gauss_clean", "hodge_correction",
            "reconstruction", "harmonic_face", "harmonic_edge",
            "tangent_source_mean_rel", "hodge_source_mean_rel",
            "tangent_poisson_relative", "hodge_poisson_relative",
        )
        codec_values = [optional_float(codec, field) for field in codec_fields]
        codec_replayed = False
        if codec_finite is True and all(
            value is not None and value >= 0.0 for value in codec_values
        ):
            (
                gauss_pre, gauss_clean, hodge, reconstruction, harmonic_face,
                harmonic_edge, tangent_mean, hodge_mean, tangent_poisson,
                hodge_poisson,
            ) = (float(value) for value in codec_values)
            detail = harmonic_detail_replay(codec)
            detail_matches = bool(
                abs(harmonic_face - detail[0]) <= 2e-15
                and abs(harmonic_edge - detail[1]) <= 2e-15
                and all(value >= 0.0 for value in detail[2:])
                and abs(tangent_mean - detail[3]) <= 2e-15
                and abs(hodge_mean - detail[5]) <= 2e-15
            )
            codec_replayed = bool(
                codec_finite is True and detail_matches
                and gauss_pre <= 2e-7 and gauss_clean <= 1e-10
                and hodge <= 2e-4 and reconstruction <= 2e-4
                and harmonic_face <= 1e-12 and harmonic_edge <= 1e-12
                and tangent_mean <= 1e-13 and hodge_mean <= 1e-13
                and tangent_poisson <= 1e-13 and hodge_poisson <= 1e-13
            )
            if codec["detail"].split(";", 1)[0] != (
                "pass" if codec_replayed else "fail"
            ):
                raise CertificateError(
                    f"execution evaluation {evaluation_id} codec detail flag mismatch"
                )
        if strict_csv_bool(codec, "valid") != codec_replayed:
            raise CertificateError(
                f"execution evaluation {evaluation_id} codec flag mismatch"
            )
        component_passes.append(codec_replayed)
        evaluations.append(ExecutionEvaluation(
            evaluation_id=evaluation_id,
            construction=construction,
            stage=stage,
            operation=operation,
            power=power,
            column=column,
            h=h,
            direction=direction,
            passed=all(component_passes),
        ))

    coordinate_directions: dict[tuple[Any, ...], set[str]] = {}
    for evaluation in evaluations:
        coordinate = (
            evaluation.construction, evaluation.stage, evaluation.operation,
            evaluation.power, evaluation.column, evaluation.h,
        )
        directions = coordinate_directions.setdefault(coordinate, set())
        if evaluation.direction in directions:
            raise CertificateError(f"duplicate execution direction for {coordinate}")
        directions.add(evaluation.direction)
    if any(directions != {"forward", "reverse"}
           for directions in coordinate_directions.values()):
        raise CertificateError("execution evaluations do not form exact direction pairs")

    filter_operations: dict[tuple[str, int, int], set[str]] = {}
    for evaluation in evaluations:
        if evaluation.stage != "krylov":
            continue
        assert evaluation.power is not None
        filter_operations.setdefault(
            (evaluation.construction, evaluation.power, evaluation.column), set()
        ).add(evaluation.operation)
    if terminal_complete:
        if any(operations != {"filter_v", "filter_w"}
               for operations in filter_operations.values()):
            raise CertificateError("Krylov filter ledger lacks an exact v/w pair")
        for construction in {key[0] for key in filter_operations}:
            powers = {
                power for candidate, power, _ in filter_operations
                if candidate == construction
            }
            if powers != set(range(1, max(powers) + 1)):
                raise CertificateError(
                    f"{construction} Krylov filter powers are not contiguous from one"
                )
            for power in powers:
                columns = {
                    column for candidate, candidate_power, column in filter_operations
                    if candidate == construction and candidate_power == power
                }
                if columns != set(range(4)):
                    raise CertificateError(
                        f"{construction} Krylov power {power} lacks four filter columns"
                    )

    candidate_operations: dict[tuple[str, str, str, int], set[str]] = {}
    for evaluation in evaluations:
        match = re.fullmatch(
            r"candidate_(image|composition):([^,]+)", evaluation.operation
        )
        if match is None:
            continue
        candidate_operations.setdefault(
            (evaluation.construction, evaluation.stage, match.group(2), evaluation.column),
            set(),
        ).add(match.group(1))
    if terminal_complete and any(
        operations != {"image", "composition"}
        for operations in candidate_operations.values()
    ):
        raise CertificateError("candidate ledger lacks an exact image/composition pair")

    group_passes = [evaluation.passed for evaluation in evaluations]
    for evaluation_id, detail in numeric_failure_details:
        if detail == "postflight_execution_failure":
            prior_failure = any(
                candidate_id < evaluation_id
                for candidate_id in numeric_failure_ids
            ) or any(
                evaluation.evaluation_id < evaluation_id
                and not evaluation.passed
                for evaluation in evaluations
            )
            if not prior_failure:
                raise CertificateError(
                    "postflight execution marker has no preceding proven failure"
                )
        if detail == "postflight_artifact_reset_failure":
            prior_postflight = any(
                candidate_id < evaluation_id and candidate_detail in {
                    "postflight_execution_failure",
                    "postflight_artifact_write_failure",
                    "postflight_artifact_schema_failure",
                    "postflight_manifest_or_hash_failure",
                    "postflight_status_write_failure",
                }
                for candidate_id, candidate_detail in numeric_failure_details
            )
            if not prior_postflight:
                raise CertificateError(
                    "postflight reset marker has no preceding abort cause"
                )
    complete = terminal_complete
    abort = terminal["operation"] == "run_abort"
    if complete and not all(group_passes):
        raise CertificateError("run_complete follows a failed execution evaluation")
    if abort and not (numeric_failure_ids or any(not passed for passed in group_passes)):
        raise CertificateError("run_abort does not contain a recorded failed evaluation")
    return complete, abort, evaluations


def replay_preflight_derivative_status(
    rows: list[dict[str, str]], ready: bool
) -> dict[tuple[str, str, float, str, int], ExecutionEvaluation]:
    """Replay the exact 98-group preflight derivative/codec ledger."""
    if not ready:
        if rows:
            raise CertificateError(
                "preflight derivative ledger exists before its prerequisite gates"
            )
        return {}
    if not rows:
        raise CertificateError("preflight derivative ledger is absent")
    terminal = rows[-1]
    terminal_id = optional_int(terminal, "evaluation_id")
    if not (
        terminal_id == 98 and terminal["record_kind"] == "terminal"
        and terminal["construction"] == "preflight"
        and terminal["stage"] == "final"
        and terminal["operation"] == "preflight_record_complete"
        and terminal["power"] == "" and terminal["column"] == ""
        and terminal["h"] == "" and terminal["direction"] == ""
        and optional_int(terminal, "sign") == 0
        and strict_csv_bool(terminal, "valid")
    ):
        raise CertificateError("malformed preflight derivative terminal")
    terminal_diagnostics = (
        "metadata", "sector", "finite", "gauss", "poisson_absolute",
        "endpoint_chart", "common_residual", "energy_drift", "recovery",
        "codec_gauss_pre", "codec_gauss_clean", "hodge_correction",
        "reconstruction", "harmonic_face", "harmonic_edge",
        "tangent_source_mean_rel", "hodge_source_mean_rel",
        "tangent_poisson_relative", "hodge_poisson_relative", "detail",
    )
    if any(terminal[field] != "" for field in terminal_diagnostics):
        raise CertificateError("preflight derivative terminal carries diagnostics")

    body = rows[:-1]
    groups: dict[int, list[dict[str, str]]] = {}
    order: list[int] = []
    for row in body:
        evaluation_id = optional_int(row, "evaluation_id")
        if evaluation_id is None:
            raise CertificateError("preflight derivative row omits evaluation ID")
        if evaluation_id not in groups:
            groups[evaluation_id] = []
            order.append(evaluation_id)
        groups[evaluation_id].append(row)
    if order != list(range(98)):
        raise CertificateError("preflight derivative IDs are not exact/contiguous")
    replayed: dict[tuple[str, str, float, str, int], ExecutionEvaluation] = {}
    for evaluation_id in range(98):
        evaluation = replay_derivative_bundle(groups[evaluation_id], evaluation_id)
        if evaluation.construction != "preflight":
            raise CertificateError("invalid preflight derivative construction")
        expected_power = (
            0 if evaluation.stage == "probe" and evaluation.h == H0
            else 1 if evaluation.stage == "probe" and evaluation.h == H1
            else None
        )
        if evaluation.power != expected_power:
            raise CertificateError("invalid preflight derivative power coordinate")
        key = (
            evaluation.stage, evaluation.operation, evaluation.h,
            evaluation.direction, evaluation.column,
        )
        if key in replayed:
            raise CertificateError(f"duplicate preflight derivative coordinate {key}")
        replayed[key] = evaluation

    expected = {
        ("probe", f"probe:{probe}", h, direction, column)
        for column, probe in enumerate(PROBE_NAMES)
        for h in (H0, H1)
        for direction in ("forward", "reverse")
    }
    expected.update({
        ("composition", "reverse_forward", H0, "reverse", column)
        for column in range(16)
    })
    expected.update({
        ("composition", "forward_reverse", H0, "forward", column)
        for column in range(16)
    })
    expected.update({
        ("zero", "zero", H0, direction, 0)
        for direction in ("forward", "reverse")
    })
    if set(replayed) != expected:
        raise CertificateError("preflight derivative coverage is not the exact 98-key set")
    expected_order: list[tuple[str, str, float, str, int]] = []
    for h in (H0, H1):
        for column, probe in enumerate(PROBE_NAMES):
            for direction in ("forward", "reverse"):
                expected_order.append(
                    ("probe", f"probe:{probe}", h, direction, column)
                )
    for column in range(16):
        expected_order.append(
            ("composition", "reverse_forward", H0, "reverse", column)
        )
        expected_order.append(
            ("composition", "forward_reverse", H0, "forward", column)
        )
    expected_order.extend((
        ("zero", "zero", H0, "forward", 0),
        ("zero", "zero", H0, "reverse", 0),
    ))
    if any(
        replayed[key].evaluation_id != evaluation_id
        for evaluation_id, key in enumerate(expected_order)
    ):
        raise CertificateError("preflight derivative IDs violate locked order")
    return replayed


def replay_field_control(rows: list[dict[str, str]], ready: bool) -> dict[str, float]:
    """Reconstruct the locked 256-tick source-free control from q primitives."""
    if not ready:
        if rows:
            raise CertificateError("field-control trace exists before prerequisites")
        return {}
    if len(rows) != 258:
        raise CertificateError("field-control trace must contain 258 exact rows")
    initial, samples, summary = rows[0], rows[1:257], rows[257]
    target = 1e-7
    if any(initial[field] != "" for field in (
        "q", "divergence", "recovery_face_absolute",
        "recovery_edge_absolute", "recovery_relative",
        "maximum_divergence", "maximum_energy_drift",
    )):
        raise CertificateError("field-control initial row carries extra fields")
    if any(summary[field] != "" for field in (
        "q", "divergence", "signal_energy", "background_energy",
    )):
        raise CertificateError("field-control summary row carries extra fields")
    if not (
        initial["record_kind"] == "initial"
        and optional_int(initial, "tick") == -1
        and finite_float(initial["target_amplitude"], "field target") == target
        and summary["record_kind"] == "summary"
        and optional_int(summary, "tick") == 256
        and finite_float(summary["target_amplitude"], "field target") == target
    ):
        raise CertificateError("field-control initial/summary rows are malformed")
    initial_signal = finite_float(initial["signal_energy"], "initial signal energy")
    initial_background = finite_float(
        initial["background_energy"], "initial background energy"
    )
    q: list[float] = []
    divergences: list[float] = []
    signal_energies: list[float] = []
    background_energies: list[float] = []
    for tick, row in enumerate(samples):
        if any(row[field] != "" for field in (
            "recovery_face_absolute", "recovery_edge_absolute",
            "recovery_relative", "maximum_divergence", "maximum_energy_drift",
        )):
            raise CertificateError(f"field-control sample {tick} carries summary fields")
        if not (
            row["record_kind"] == "sample"
            and optional_int(row, "tick") == tick
            and finite_float(row["target_amplitude"], "field target") == target
        ):
            raise CertificateError(f"field-control sample {tick} is malformed")
        q.append(finite_float(row["q"], f"field q[{tick}]"))
        divergence = finite_float(row["divergence"], "field divergence")
        if divergence < 0.0:
            raise CertificateError("field-control divergence is negative")
        divergences.append(divergence)
        signal_energies.append(
            finite_float(row["signal_energy"], "field signal energy")
        )
        background_energies.append(
            finite_float(row["background_energy"], "field background energy")
        )
    denominator = 2.0 * math.fsum(value * value for value in q[1:255])
    if not (math.isfinite(denominator) and denominator > 0.0):
        raise CertificateError("field phase denominator is nonpositive")
    numerator = math.fsum(
        q[index] * (q[index + 1] + q[index - 1])
        for index in range(1, 255)
    )
    cosine_ratio = numerator / denominator
    if not (-1.0 <= cosine_ratio <= 1.0):
        raise CertificateError("field phase cosine lies outside [-1,1]")
    phase = math.acos(cosine_ratio)
    predicted = 2.0 * math.asin(C_SPEED * math.sin(math.pi / L))
    phase_relative = abs(phase - predicted) / predicted
    recurrence = max(
        abs(q[index + 1] + q[index - 1]
            - 2.0 * math.cos(predicted) * q[index]) / target
        for index in range(1, 255)
    )
    maximum_divergence = max(divergences)
    maximum_energy_drift = max(
        max(abs(value - initial_signal) for value in signal_energies),
        max(abs(value - initial_background) for value in background_energies),
    )
    recovery_face = finite_float(
        summary["recovery_face_absolute"], "field face recovery"
    )
    recovery_edge = finite_float(
        summary["recovery_edge_absolute"], "field edge recovery"
    )
    reported_recovery = finite_float(
        summary["recovery_relative"], "field recovery"
    )
    recovery = max(recovery_face, recovery_edge) / target
    reported_divergence = finite_float(
        summary["maximum_divergence"], "field maximum divergence"
    )
    reported_drift = finite_float(
        summary["maximum_energy_drift"], "field maximum energy drift"
    )
    if recovery_face < 0.0 or recovery_edge < 0.0:
        raise CertificateError("field recovery is negative")
    if abs(reported_recovery - recovery) > 2e-15 * max(1.0, recovery):
        raise CertificateError("field recovery summary mismatch")
    if abs(reported_divergence - maximum_divergence) > 2e-15 * max(
        1.0, maximum_divergence
    ):
        raise CertificateError("field maximum divergence summary mismatch")
    if abs(reported_drift - maximum_energy_drift) > 2e-15 * max(
        1.0, maximum_energy_drift
    ):
        raise CertificateError("field maximum energy-drift summary mismatch")
    return {
        "phase": phase,
        "phase_relative": phase_relative,
        "recurrence": recurrence,
        "recovery": recovery,
        "maximum_divergence": maximum_divergence,
        "maximum_energy_drift": maximum_energy_drift,
        "pass": bool(
            phase_relative <= 1e-8 and recurrence <= 1e-8 and recovery <= 1e-8
        ),
    }


def replay_cache_control(
    rows: list[dict[str, str]], ready: bool
) -> tuple[bool, dict[tuple[str, float, str, int], dict[str, Any]]]:
    """Replay exact per-endpoint cache acceptance and reuse semantics."""
    if not ready:
        if rows:
            raise CertificateError("cache-control rows exist before endpoint preflight")
        return False, {}
    expected_keys = {
        (probe, h, direction, sign)
        for probe in PROBE_NAMES
        for h in (H0, H1)
        for direction in ("forward", "reverse")
        for sign in (-1, 1)
    }
    replayed: dict[tuple[str, float, str, int], dict[str, Any]] = {}
    global_reuse = False
    for row in rows:
        if row["direction"] not in {"forward", "reverse"}:
            raise CertificateError("invalid cache-control direction")
        sign = strict_int_text(row["sign"], "cache sign")
        key = (
            row["probe"], finite_float(row["h"], "cache h"),
            row["direction"], sign,
        )
        if key in replayed:
            raise CertificateError(f"duplicate cache-control key {key}")
        metadata = strict_csv_bool(row, "retraction_metadata")
        sector = strict_csv_bool(row, "retraction_sector")
        finite = strict_csv_bool(row, "retraction_finite")
        gauss = finite_float(row["retraction_gauss"], "cache retraction Gauss")
        poisson = finite_float(
            row["retraction_poisson_absolute"], "cache retraction Poisson"
        )
        retraction_pass = bool(
            metadata and sector and finite and 0.0 <= gauss <= 1e-10
            and 0.0 <= poisson <= 1e-13
        )
        direct = strict_csv_bool(row, "direct_accepted")
        inverse = strict_csv_bool(row, "inverse_accepted")
        observer = strict_csv_bool(row, "observer_accepted")
        endpoint_chart = strict_csv_bool(row, "endpoint_chart")
        population = strict_csv_bool(row, "population_accepted")
        reuse = strict_csv_bool(row, "reuse_accepted")
        population_agreement = finite_float(
            row["direct_population_agreement"],
            "cache direct/population agreement",
        )
        reuse_agreement = finite_float(
            row["direct_reuse_agreement"], "cache direct/reuse agreement"
        )
        population_iterations = strict_int_text(
            row["population_iterations"], "cache population iterations"
        )
        cache_valid = strict_csv_bool(row, "cache_valid_after_population")
        population_refreshes = strict_int_text(
            row["population_refreshes"], "cache population refreshes"
        )
        population_reuses = strict_int_text(
            row["population_reuses"], "cache population reuses"
        )
        population_fallbacks = strict_int_text(
            row["population_fallbacks"], "cache population fallbacks"
        )
        reuse_refreshes = strict_int_text(
            row["reuse_refreshes"], "cache reuse refreshes"
        )
        reuse_reuses = strict_int_text(
            row["reuse_reuses"], "cache reuse count"
        )
        reuse_fallbacks = strict_int_text(
            row["reuse_fallbacks"], "cache reuse fallbacks"
        )
        counters = (
            population_iterations, population_refreshes, population_reuses,
            population_fallbacks, reuse_refreshes, reuse_reuses,
            reuse_fallbacks,
        )
        if any(value < 0 for value in counters):
            raise CertificateError(f"negative cache counter for {key}")
        semantics = True
        if population_refreshes > 0 and cache_valid:
            semantics = reuse_reuses > 0
        if population_iterations == 0 and not cache_valid:
            semantics = semantics and all(value == 0 for value in (
                population_refreshes, population_reuses,
                reuse_refreshes, reuse_reuses,
            ))
        if strict_csv_bool(row, "cache_semantics") != semantics:
            raise CertificateError(f"cache-semantics flag mismatch for {key}")
        cache_pass = bool(
            population and reuse
            and 0.0 <= population_agreement <= 1e-10
            and 0.0 <= reuse_agreement <= 1e-10
            and population_fallbacks == 0 and reuse_fallbacks == 0
            and semantics
        )
        global_reuse = global_reuse or bool(
            population_refreshes > 0 and cache_valid and reuse_reuses > 0
        )
        replayed[key] = {
            "cache_pass": cache_pass,
            "reported_valid": strict_csv_bool(row, "valid"),
            "retraction": retraction_pass,
            "direct": direct, "inverse": inverse, "observer": observer,
            "endpoint_chart": endpoint_chart,
            "gauss": gauss, "poisson": poisson,
            "population": population, "reuse": reuse,
            "population_agreement": population_agreement,
            "reuse_agreement": reuse_agreement,
            "population_iterations": population_iterations,
            "cache_valid_after_population": cache_valid,
            "population_refreshes": population_refreshes,
            "population_reuses": population_reuses,
            "reuse_refreshes": reuse_refreshes,
            "reuse_reuses": reuse_reuses,
            "population_fallbacks": population_fallbacks,
            "reuse_fallbacks": reuse_fallbacks,
        }
    if set(replayed) != expected_keys:
        raise CertificateError("cache-control artifact lacks exact 128-key coverage")
    cache_pass = (
        all(record["cache_pass"] for record in replayed.values())
        and global_reuse
    )
    return cache_pass, replayed


def replay_energy_control(
    rows: list[dict[str, str]], ready: bool
) -> dict[tuple[str, int], dict[str, Any]]:
    """Replay signed energy retractions without trusting energy-row booleans."""
    if not ready:
        if rows:
            raise CertificateError("energy-control rows exist before prerequisites")
        return {}
    expected = {(probe, sign) for probe in PROBE_NAMES for sign in (-1, 1)}
    replayed: dict[tuple[str, int], dict[str, Any]] = {}
    for row in rows:
        key = (
            row["probe"], strict_int_text(row["sign"], "energy-control sign")
        )
        if key in replayed:
            raise CertificateError(f"duplicate energy-control key {key}")
        if finite_float(row["h"], "energy-control h") != H_ENERGY:
            raise CertificateError(f"energy-control h lock failed for {key}")
        metadata = strict_csv_bool(row, "metadata")
        sector = strict_csv_bool(row, "sector")
        finite = strict_csv_bool(row, "finite")
        gauss = finite_float(row["gauss"], "energy-control Gauss")
        poisson = finite_float(
            row["poisson_absolute"], "energy-control Poisson"
        )
        increment = finite_float(row["increment"], "energy increment")
        valid = bool(
            metadata and sector and finite and 0.0 <= gauss <= 1e-10
            and 0.0 <= poisson <= 1e-13
        )
        replayed[key] = {
            "valid": valid, "gauss": gauss, "increment": increment,
        }
    if set(replayed) != expected:
        raise CertificateError("energy-control artifact lacks exact 32-key coverage")
    return replayed


def replay_runtime(rows: list[dict[str, str]]) -> dict[str, Any]:
    """Derive locked options and representative gates from runtime primitives."""
    values: dict[str, str] = {}
    for row in rows:
        if not row["name"] or row["name"] in values:
            raise CertificateError(f"duplicate/blank runtime key {row['name']!r}")
        values[row["name"]] = row["value"]
    expected = {
        "option_wave_speed", "option_dt", "option_binding_stiffness",
        "option_binding_law", "option_compact_pair_well_depth",
        "option_compact_pair_cutoff_distance_squared",
        "option_constituent_mass_scale", "option_polarity_scale",
        "option_field_energy_scale", "option_gate_tolerance",
        "option_solve_tolerance", "option_finite_difference_scale",
        "option_max_iterations", "option_allow_shared_anchor_chart",
        "option_use_sparse_local_current", "option_use_local_residual_evaluation",
        "option_use_low_rank_identity_broyden",
        "option_use_matrix_free_newton_krylov", "option_defer_volume_diagnostics",
        "option_measure_final_root_regularity", "option_root_momentum_seed_size",
        "reference_L", "initialized_valid", "initializer_graph_connected",
        "initializer_graph_local", "initializer_site_projection_valid",
        "initializer_poisson_iterations", "initializer_poisson_residual",
        "initializer_gauss_residual", "initializer_curl_adjoint_residual",
        "constituent_count",
        "orientation_axis", "width", "normalization_valid",
        "normalization_field_scale", "normalization_current_scale",
        "normalization_energy_scale", "normalization_native_susceptibility",
        "normalization_mapped_susceptibility",
        "normalization_native_action_work_coefficient",
        "normalization_mapped_field_work_coefficient",
        "normalization_susceptibility_residual",
        "normalization_work_residual",
        "normalization_work_coefficient", "beta", "density_jet_valid",
        "density_charge_residual", "derivative_charge_residual",
        "density_derivative_moment_residual",
        "mode6_valid", "mode7_valid",
        "mode6_number", "mode6_group", "mode6_phase", "mode7_number",
        "mode7_group", "mode7_phase", "mode_mass_gram_00",
        "mode_mass_gram_01", "mode_mass_gram_11",
    }
    if set(values) != expected:
        raise CertificateError(
            f"runtime key set mismatch: missing={sorted(expected-set(values))}, "
            f"extra={sorted(set(values)-expected)}"
        )

    def number(name: str) -> float:
        return finite_float(values[name], f"runtime {name}")

    def integer(name: str) -> int:
        value = values[name]
        if re.fullmatch(r"-?(?:0|[1-9][0-9]*)", value) is None:
            raise CertificateError(f"runtime {name} is not an exact integer")
        return int(value)

    def boolean(name: str) -> bool:
        if values[name] not in {"0", "1"}:
            raise CertificateError(f"runtime {name} is not an exact boolean")
        return values[name] == "1"

    options_ok = bool(
        abs(number("option_wave_speed") - C_SPEED) <= 2e-16
        and number("option_dt") == 1.0
        and number("option_binding_stiffness") == 1.0
        and values["option_binding_law"] == "FixedEdgeQuartic"
        and number("option_compact_pair_well_depth") == 0.01
        and number("option_compact_pair_cutoff_distance_squared") == 1.5
        and number("option_constituent_mass_scale") == 1.0
        and number("option_polarity_scale") == 1.0
        and number("option_field_energy_scale") == 1.0
        and number("option_gate_tolerance") == 1e-10
        and number("option_solve_tolerance") == 2e-13
        and number("option_finite_difference_scale") == 2e-7
        and integer("option_max_iterations") == 64
        and boolean("option_allow_shared_anchor_chart")
        and boolean("option_use_sparse_local_current")
        and boolean("option_use_local_residual_evaluation")
        and not boolean("option_use_low_rank_identity_broyden")
        and not boolean("option_use_matrix_free_newton_krylov")
        and not boolean("option_defer_volume_diagnostics")
        and not boolean("option_measure_final_root_regularity")
        and integer("option_root_momentum_seed_size") == 0
    )
    def close(lhs: float, rhs: float, tolerance: float = 2e-15) -> bool:
        return abs(lhs - rhs) <= tolerance * max(1.0, abs(rhs))

    poisson_iterations = integer("initializer_poisson_iterations")
    initializer_derived = bool(
        boolean("initializer_graph_connected")
        and boolean("initializer_graph_local")
        and boolean("initializer_site_projection_valid")
        and 0 <= poisson_iterations <= 4096
        and 0.0 <= number("initializer_poisson_residual") <= 1e-13
        and 0.0 <= number("initializer_gauss_residual") <= 1e-11
        and 0.0 <= number("initializer_curl_adjoint_residual") <= 1e-11
    )
    if boolean("initialized_valid") != initializer_derived:
        raise CertificateError("initialized_valid disagrees with runtime primitives")

    c2 = C_SPEED * C_SPEED
    native_susceptibility = G_C / c2
    field_scale = native_susceptibility
    current_scale = field_scale
    energy_scale = c2
    mapped_susceptibility = field_scale
    native_action_work = G_C * field_scale
    mapped_field_work = energy_scale * field_scale * current_scale
    susceptibility_residual = mapped_susceptibility - native_susceptibility
    work_residual = mapped_field_work - native_action_work
    serialized_susceptibility_residual = number(
        "normalization_susceptibility_residual"
    )
    serialized_work_residual = number("normalization_work_residual")
    normalization_equalities = bool(all((
        close(number("normalization_field_scale"), field_scale),
        close(number("normalization_current_scale"), current_scale),
        close(number("normalization_energy_scale"), energy_scale),
        close(
            number("normalization_native_susceptibility"),
            native_susceptibility,
        ),
        close(
            number("normalization_mapped_susceptibility"),
            mapped_susceptibility,
        ),
        close(
            number("normalization_native_action_work_coefficient"),
            native_action_work,
        ),
        close(
            number("normalization_mapped_field_work_coefficient"),
            mapped_field_work,
        ),
        close(
            serialized_susceptibility_residual,
            susceptibility_residual,
        ),
        close(serialized_work_residual, work_residual),
    )))
    normalization_derived = bool(
        normalization_equalities
        and abs(serialized_susceptibility_residual) <= 1e-15
        and abs(serialized_work_residual) <= 1e-15
    )
    if boolean("normalization_valid") != normalization_derived:
        raise CertificateError("normalization_valid disagrees with runtime replay")
    coefficient = number("normalization_work_coefficient")
    if not close(coefficient, mapped_field_work):
        raise CertificateError("normalization work-coefficient alias mismatch")
    beta = number("beta")
    representative_structure = bool(
        integer("reference_L") == L
        and integer("constituent_count") == 16
        and integer("orientation_axis") == 0 and integer("width") == 2
    )
    representative_ok = bool(
        representative_structure and initializer_derived
        and normalization_derived and coefficient > 0.0
        and beta > 0.0
        and close(beta, mapped_field_work * number("option_field_energy_scale"))
    )
    density_residuals = (
        number("density_charge_residual"),
        number("derivative_charge_residual"),
        number("density_derivative_moment_residual"),
    )
    density_ok = all(0.0 <= residual <= 1e-12 for residual in density_residuals)
    if boolean("density_jet_valid") != density_ok:
        raise CertificateError("density_jet_valid disagrees with residual primitives")
    mode6_numeric = bool(
        integer("mode6_number") == 6
        and integer("mode6_group") == 4
        and abs(number("mode6_phase") - PHI_INT) <= 1e-12
    )
    mode7_numeric = bool(
        integer("mode7_number") == 7
        and integer("mode7_group") == 4
        and abs(number("mode7_phase") - PHI_INT) <= 1e-12
    )
    mode_mass_gram = bool(
        abs(number("mode_mass_gram_00") - 1.0) <= 1e-10
        and abs(number("mode_mass_gram_01")) <= 1e-10
        and abs(number("mode_mass_gram_11") - 1.0) <= 1e-10
    )
    mode6_derived = mode6_numeric and mode_mass_gram
    mode7_derived = mode7_numeric and mode_mass_gram
    if boolean("mode6_valid") != mode6_derived:
        raise CertificateError("mode6_valid disagrees with mode primitives")
    if boolean("mode7_valid") != mode7_derived:
        raise CertificateError("mode7_valid disagrees with mode primitives")
    modes_ok = mode6_derived and mode7_derived
    return {
        "options": options_ok, "representative": representative_ok,
        "density": density_ok, "modes": modes_ok, "beta": beta,
    }


def replay_krylov_status(
    rows: list[dict[str, str]], ready: bool
) -> dict[str, KrylovReplay]:
    """Reconstruct dimensions and termination semantics from exact integers."""
    if not ready:
        if rows:
            raise CertificateError("Krylov status exists without run_complete")
        return {}
    order = ("primary", "h1", "sign", "rotation")
    if len(rows) != 4 or tuple(row["construction"] for row in rows) != order:
        raise CertificateError("Krylov status must be four rows in locked order")
    replayed: dict[str, KrylovReplay] = {}
    for row in rows:
        construction = row["construction"]
        generated = strict_int_text(
            row["generated_power_count"], f"{construction} generated powers"
        )
        accepted = strict_int_text(
            row["accepted_dimension"], f"{construction} accepted dimension"
        )
        prior = strict_int_text(
            row["prior_dimension"], f"{construction} prior dimension"
        )
        last_power = strict_int_text(
            row["last_nonempty_power"], f"{construction} last power"
        )
        last_start = strict_int_text(
            row["last_nonempty_start"], f"{construction} last start"
        )
        last_end = strict_int_text(
            row["last_nonempty_end"], f"{construction} last end"
        )
        deflations = strict_int_text(
            row["deflation_count"], f"{construction} deflations"
        )
        happy = strict_csv_bool(row, "happy_breakdown")
        exhausted = strict_csv_bool(row, "exhausted_16_powers")
        reported_bookkeeping = strict_csv_bool(row, "bookkeeping_valid")
        projected_final = strict_csv_bool(row, "projected_final_present")
        terminal_reported = strict_csv_bool(row, "terminal_invariance_eligible")
        structural = krylov_structure_replay(
            generated, accepted, prior, last_power, last_start,
            last_end, deflations, happy, exhausted,
        )
        if reported_bookkeeping != structural:
            raise CertificateError(
                f"{construction} bookkeeping flag does not match primitives"
            )
        if projected_final != (accepted >= 4):
            raise CertificateError(
                f"{construction} projected-final flag does not match dimension"
            )
        terminal_t = optional_float(row, "terminal_t_invariance")
        terminal_tinv = optional_float(row, "terminal_tinv_invariance")
        if projected_final:
            if terminal_t is None or terminal_tinv is None:
                raise CertificateError(
                    f"{construction} projected final lacks terminal invariance"
                )
            if terminal_t < 0.0 or terminal_tinv < 0.0:
                raise CertificateError(
                    f"{construction} terminal invariance is negative"
                )
        elif terminal_t is not None or terminal_tinv is not None or terminal_reported:
            raise CertificateError(
                f"{construction} sub-four solve carries terminal claims"
            )
        replayed[construction] = KrylovReplay(
            construction=construction, generated=generated, accepted=accepted,
            prior=prior, last_power=last_power, last_start=last_start,
            last_end=last_end, deflations=deflations, happy=happy,
            exhausted=exhausted, structural=structural,
            projected_final=projected_final, terminal_t=terminal_t,
            terminal_tinv=terminal_tinv,
            terminal_eligible_reported=terminal_reported,
        )
    return replayed


def main() -> int:
    missing = [path for path in ARTIFACTS if not path.is_file()]
    if missing:
        print("FTD-0774 independent tangent certificate: CORPUS ABSENT")
        for path in missing:
            print(f"MISSING {path.relative_to(ROOT)}")
        print("Run the locked C++ FTD-0774 producer before replay.")
        return 2

    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))

    try:
        result = json.loads(RESULT_JSON.read_text(encoding="utf-8"))
        preflight_rows = read_csv_exact(PREFLIGHT, PREFLIGHT_FIELDS)
        hessian_rows = read_csv_exact(HESSIAN, HESSIAN_FIELDS)
        projected_rows = read_csv_exact(PROJECTED, PROJECTED_FIELDS)
        cluster_rows = read_csv_exact(CLUSTERS, CLUSTER_FIELDS)
        metric_rows = read_csv_exact(METRICS, METRIC_FIELDS)
        index_rows = read_csv_exact(VECTOR_INDEX, VECTOR_INDEX_FIELDS)
        gram_rows = read_csv_exact(GRAMS, GRAM_FIELDS)
        execution_status_rows = read_csv_exact(
            EXECUTION_STATUS, EXECUTION_STATUS_FIELDS
        )
        preflight_derivative_status_rows = read_csv_exact(
            PREFLIGHT_DERIVATIVE_STATUS, EXECUTION_STATUS_FIELDS
        )
        runtime_rows = read_csv_exact(RUNTIME, RUNTIME_FIELDS)
        energy_control_rows = read_csv_exact(
            ENERGY_CONTROL, ENERGY_CONTROL_FIELDS
        )
        cache_control_rows = read_csv_exact(CACHE_CONTROL, CACHE_CONTROL_FIELDS)
        field_control_rows = read_csv_exact(FIELD_CONTROL, FIELD_CONTROL_FIELDS)
        krylov_status_rows = read_csv_exact(KRYLOV_STATUS, KRYLOV_STATUS_FIELDS)
        hash_rows = read_csv_exact(HASHES, HASH_FIELDS)
        gram_artifacts: dict[tuple[CandidateKey, str], np.ndarray] = {}
        for (candidate_id, construction, stage, block), rows_iter in itertools.groupby(
            sorted(
                gram_rows,
                key=lambda row: (
                    row["candidate_id"], row["construction"], row["stage"],
                    row["block"], int(row["row"]), int(row["column"]),
                ),
            ),
            key=lambda row: (
                row["candidate_id"], row["construction"], row["stage"], row["block"]
            ),
        ):
            gram_artifacts[(CandidateKey(candidate_id, construction, stage), block)] = (
                matrix_from_rows(list(rows_iter))
            )

        # --------------------------------------------------------------
        # Frozen provenance and byte-level corpus integrity.
        required_json_keys = {
            "ftd_id", "protocol_sha256", "source_commit", "production_changed",
            "verdict", "companion_verdict", "protocol_locked", "provenance_pass",
            "source_gate_pass", "representative_pass", "options_pass",
            "chart_raw_dimension", "chart_independent_dimension", "hessian_pass",
            "gradient_pass", "seed_metric_pass", "energy_form_pass", "endpoint_preflight_pass",
            "regularity_pass", "cache_control_pass", "field_control_pass",
            "preflight_pass", "artifact_schema_pass", "krylov_executed", "krylov_resolved",
            "eligible_candidate_count", "qualified_candidate_count",
            "selected_candidate_id", "beta", "lambda", "h0", "h1", "hE",
            "internal_phase", "mu0", "hessian", "preflight_maxima", "construction_dimensions",
            "artifact_sha256",
        }
        check("JSON frozen top-level schema", not (required_json_keys - set(result)))
        check("locked protocol hash", sha256(PREREG) == PROTOCOL_SHA256)
        check("frozen runner hash", sha256(RUNNER) == RUNNER_SHA256)
        check("frozen tangent support hash", sha256(SUPPORT) == SUPPORT_SHA256)
        check("FTD identity", result.get("ftd_id") == "FTD-0774")
        check("embedded protocol hash", result.get("protocol_sha256") == PROTOCOL_SHA256)
        check("embedded source commit", result.get("source_commit") == SOURCE_COMMIT)
        check("production unchanged", result.get("production_changed") is False)
        check("protocol marked locked", result.get("protocol_locked") is True)
        check("producer artifact-schema validator passed",
              result.get("artifact_schema_pass") is True)
        check("raw chart dimension", result.get("chart_raw_dimension") == D_RAW)
        check(
            "independent chart dimension",
            result.get("chart_independent_dimension") == D_INDEPENDENT,
        )
        check("h0 lock", finite_float(result.get("h0"), "h0") == H0)
        check("h1 lock", finite_float(result.get("h1"), "h1") == H1)
        check("energy step lock", finite_float(result.get("hE"), "hE") == H_ENERGY)
        internal_phase = finite_float(result.get("internal_phase"), "internal_phase")
        mu0 = finite_float(result.get("mu0"), "mu0")
        check("registered internal phase", internal_phase == PHI_INT)
        check("target cosine reconstructed", abs(mu0 - math.cos(internal_phase)) <= 2e-16)
        check(
            "lambda lock",
            abs(finite_float(result.get("lambda"), "lambda") - C_SPEED) <= 2e-16,
        )
        parent_ok = all(path.is_file() and sha256(path) == digest for path, digest in PARENT_HASHES.items())

        manifest: dict[str, dict[str, str]] = {}
        for row in hash_rows:
            name = row["artifact"]
            if name in manifest:
                raise CertificateError(f"duplicate hash-manifest artifact {name}")
            manifest[name] = row
        manifest_targets = {
            "protocol": PREREG, "runner": RUNNER, "support": SUPPORT,
            "json": RESULT_JSON, "preflight": PREFLIGHT, "hessian": HESSIAN,
            "projected_matrices": PROJECTED, "clusters": CLUSTERS,
            "candidate_metrics": METRICS, "candidate_vectors": VECTORS,
            "candidate_vectors_index": VECTOR_INDEX, "gram_blocks": GRAMS,
            "execution_status": EXECUTION_STATUS,
            "preflight_derivative_status": PREFLIGHT_DERIVATIVE_STATUS,
            "runtime": RUNTIME, "energy_control": ENERGY_CONTROL,
            "cache_control": CACHE_CONTROL, "field_control": FIELD_CONTROL,
            "krylov_status": KRYLOV_STATUS,
            **EMBEDDED_SOURCES,
        }
        manifest_ok = True
        for name, path in manifest_targets.items():
            row = manifest.get(name)
            manifest_ok &= bool(
                path.is_file()
                and row is not None
                and row["sha256"].upper() == sha256(path)
                and int(row["bytes"]) == path.stat().st_size
            )
        check("hash manifest covers protocol, source, JSON, and primitives", manifest_ok)
        check("hash manifest has exactly the frozen non-self entries",
              set(manifest) == set(manifest_targets))
        json_artifact_hashes = result.get("artifact_sha256")
        primitive_by_json_key = {
            "preflight": PREFLIGHT, "hessian": HESSIAN,
            "projected_matrices": PROJECTED, "clusters": CLUSTERS,
            "candidate_metrics": METRICS, "candidate_vectors": VECTORS,
            "candidate_vectors_index": VECTOR_INDEX, "gram_blocks": GRAMS,
            "execution_status": EXECUTION_STATUS,
            "preflight_derivative_status": PREFLIGHT_DERIVATIVE_STATUS,
            "runtime": RUNTIME, "energy_control": ENERGY_CONTROL,
            "cache_control": CACHE_CONTROL, "field_control": FIELD_CONTROL,
            "krylov_status": KRYLOV_STATUS,
        }
        json_hashes_ok = isinstance(json_artifact_hashes, dict) and all(
            json_artifact_hashes.get(key) == sha256(path)
            for key, path in primitive_by_json_key.items()
        )
        check("JSON primitive hashes independently reproduce", json_hashes_ok)

        source_gate = subprocess.run(
            ["git", "diff", "--quiet", SOURCE_COMMIT, "--", "engine/include/ftd", "engine/src/eft"],
            cwd=ROOT,
            check=False,
        ).returncode == 0
        runner_text = RUNNER.read_text(encoding="utf-8") if RUNNER.is_file() else ""
        support_text = SUPPORT.read_text(encoding="utf-8") if SUPPORT.is_file() else ""
        check("runner embeds protocol hash", PROTOCOL_SHA256 in runner_text)
        check("runner embeds all parent hashes", all(digest in runner_text for digest in PARENT_HASHES.values()))
        check("runner embeds the full compiled test-source closure",
              all(sha256(path) in runner_text for path in EMBEDDED_SOURCES.values()))
        runner_complete = bool(
            all(verdict in runner_text for verdict in (INVALID, UNRESOLVED, NOT_QUALIFIED, CONSTRUCTIVE))
            and "endpoint_path_not_yet_reached" not in runner_text
            and "DEV SCAFFOLD" not in runner_text.upper()
        )
        check("runner is the complete ordered implementation", runner_complete)

        def cxx_string_constant(name: str) -> str:
            match = re.search(
                rf"constexpr\s+char\s+{re.escape(name)}\[\]\s*=\s*(.*?);",
                runner_text, re.S,
            )
            if match is None:
                return ""
            return "".join(re.findall(r'"([^"\\]*)"', match.group(1)))

        check(
            "runner carries the frozen component-ledger schema and branches",
            cxx_string_constant("kExecutionStatusHeader")
            == ",".join(EXECUTION_STATUS_FIELDS)
            and "_execution_status.csv" in runner_text
            and '"run_complete"' in runner_text
            and '"run_abort"' in runner_text,
        )
        frozen_headers = {
            "kPreflightHeader": PREFLIGHT_FIELDS,
            "kHessianHeader": HESSIAN_FIELDS,
            "kProjectedHeader": PROJECTED_FIELDS,
            "kClustersHeader": CLUSTER_FIELDS,
            "kCandidateMetricsHeader": METRIC_FIELDS,
            "kCandidateIndexHeader": VECTOR_INDEX_FIELDS,
            "kGramHeader": GRAM_FIELDS,
            "kRuntimeHeader": RUNTIME_FIELDS,
            "kEnergyControlHeader": ENERGY_CONTROL_FIELDS,
            "kCacheControlHeader": CACHE_CONTROL_FIELDS,
            "kFieldControlHeader": FIELD_CONTROL_FIELDS,
            "kKrylovStatusHeader": KRYLOV_STATUS_FIELDS,
        }
        check(
            "runner carries every exact primitive schema",
            all(
                cxx_string_constant(name) == ",".join(fields)
                for name, fields in frozen_headers.items()
            ),
        )
        runner_numeric_details = set(re.findall(
            r'"((?:acceptable|inconsistent|invalid|nonfinite|postflight)_[a-z0-9_]+)"',
            runner_text,
        ))
        runner_numeric_details.discard("postflight_execution")
        check(
            "runner numeric-failure vocabulary exactly matches replay predicates",
            runner_numeric_details == NUMERIC_FAILURE_DETAILS,
        )
        compact_runner = re.sub(r"\s+", "", runner_text)
        compact_support = re.sub(r"\s+", "", support_text)
        option_fragments = (
            "options.wave_speed=ftd::C_SPEED;", "options.dt=1.0;",
            "options.binding_stiffness=1.0;",
            "options.binding_law=ftd::eft::ConnectedBindingLaw::FixedEdgeQuartic;",
            "options.compact_pair_well_depth=0.01;",
            "options.compact_pair_cutoff_distance_squared=1.5;",
            "options.constituent_mass_scale=1.0;", "options.polarity_scale=1.0;",
            "options.field_energy_scale=1.0;", "options.gate_tolerance=1e-10;",
            "options.solve_tolerance=2e-13;",
            "options.finite_difference_scale=2e-7;", "options.max_iterations=64;",
            "options.allow_shared_anchor_chart=true;",
            "options.use_sparse_local_current=true;",
            "options.use_local_residual_evaluation=true;",
            "options.use_low_rank_identity_broyden=false;",
            "options.use_matrix_free_newton_krylov=false;",
            "options.defer_volume_diagnostics=false;",
            "options.measure_final_root_regularity=false;",
            "options.root_momentum_seed.clear();",
        )
        static_options_ok = all(
            fragment in compact_runner for fragment in option_fragments
        )
        static_representative_ok = bool(
            "load_refined_state(0);" in compact_runner
            and "initialize_connected_moore_block(17,2,0,0,0.5,1e-13,4096);"
            in compact_runner
        )
        check("runner statically carries every locked endpoint option",
              static_options_ok)
        check("runner statically carries the locked L17 representative",
              static_representative_ok)
        check("support statically carries locked redress and Hodge solves",
              "redress_connected_moore_block_with_fibre_limit(geometry,8,1e-13,4096);"
              in compact_support
              and "solve_longitudinal(divergence(residue),1e-13,4096);"
              in compact_support)
        check(
            "reported provenance gate matches parent replay",
            truth(result.get("provenance_pass")) == parent_ok,
        )
        check(
            "reported source gate matches Git replay",
            truth(result.get("source_gate_pass")) == source_gate,
        )
        # Runtime rows are the verdict inputs.  Producer aggregate booleans
        # below are checked only for agreement with this independent replay.
        runtime_ready = parent_ok and source_gate and static_options_ok
        if runtime_ready:
            if not runtime_rows:
                raise CertificateError("locked front gates have no runtime audit")
            runtime_replay = replay_runtime(runtime_rows)
        else:
            if runtime_rows:
                raise CertificateError("runtime audit exists before locked front gates")
            runtime_replay = {
                "options": False, "representative": False,
                "density": False, "modes": False, "beta": math.nan,
            }
        options_ok = static_options_ok and (
            runtime_replay["options"] if runtime_ready else True
        )
        representative_ok = bool(
            runtime_ready and static_representative_ok
            and runtime_replay["representative"]
        )
        check("reported options gate matches runtime/source replay",
              truth(result.get("options_pass")) == options_ok)
        check("reported representative gate matches runtime replay",
              truth(result.get("representative_pass")) == representative_ok)
        if runtime_ready:
            check(
                "reported beta matches runtime primitive",
                abs(finite_float(result.get("beta"), "beta")
                    - float(runtime_replay["beta"]))
                <= 2e-15 * max(1.0, abs(float(runtime_replay["beta"]))),
            )
        front_valid = bool(
            parent_ok and source_gate and options_ok and representative_ok
        )

        # --------------------------------------------------------------
        # H_red and exact L=17 field positivity.
        matrix_entries = [row for row in hessian_rows if int(row["row"]) >= 0]
        eigen_entries = sorted(
            (row for row in hessian_rows if int(row["row"]) == -1),
            key=lambda row: int(row["column"]),
        )
        h_red = matrix_from_rows(matrix_entries) if matrix_entries else np.empty((0, 0))
        hessian_ok = False
        if h_red.size:
            if h_red.shape != (48, 48):
                raise CertificateError(f"H_red shape {h_red.shape}, expected (48, 48)")
            antisymmetry = fro_relative(h_red, h_red.T)
            computed_eigenvalues, computed_eigenvectors = np.linalg.eigh((h_red + h_red.T) / 2.0)
            stored_eigenvalues = np.array(
                [finite_float(row["value"], "H eigenvalue") for row in eigen_entries]
            )
            if stored_eigenvalues.shape != (48,):
                raise CertificateError("Hessian artifact must carry exactly 48 eigenvalues")
            eigen_residual = float(
                np.linalg.norm(computed_eigenvalues - stored_eigenvalues)
                / max(np.linalg.norm(stored_eigenvalues), 1e-30)
            )
            orthogonality = float(
                np.linalg.norm(computed_eigenvectors.T @ computed_eigenvectors - np.eye(48))
            )
            beta = float(runtime_replay["beta"])
            field_lower = beta * (1.0 - math.cos(math.pi / 34.0))
            hessian_ok = bool(
                antisymmetry <= 1e-12
                and eigen_residual <= 1e-7
                and orthogonality <= 1e-10
                and computed_eigenvalues[0] > 1e-5
                and beta > 0.0
                and field_lower > 0.0
                and runtime_replay["modes"]
            )
            check("reported Hessian gate matches independent replay",
                  truth(result.get("hessian_pass")) == hessian_ok)
            check("serialized Hessian diagnostics match replay", all((
                isinstance(result.get("hessian"), dict),
                abs(float(result["hessian"]["antisymmetry"]) - antisymmetry) <= 2e-12,
                abs(float(result["hessian"]["eigen_residual"]) - eigen_residual) <= 2e-8,
                abs(float(result["hessian"]["orthogonality"]) - orthogonality) <= 2e-10,
                abs(float(result["hessian"]["lambda_min"]) - float(computed_eigenvalues[0])) <= 2e-10,
                abs(float(result["hessian"]["lambda_max"]) - float(computed_eigenvalues[-1])) <= 2e-10,
                abs(float(result["hessian"]["field_lower_bound"]) - field_lower) <= 2e-12,
            )))
        else:
            beta = float(runtime_replay["beta"])
            check("empty Hessian reports a failed Hessian gate", not truth(result.get("hessian_pass")))

        # --------------------------------------------------------------
        # Preflight gates reconstructed only from serialized primitives.
        kinds = (
            "gradient", "seed_metric", "energy", "field_control",
            "endpoint", "derivative", "zero_control",
        )
        if any(row["record_kind"] not in kinds for row in preflight_rows):
            raise CertificateError("unknown preflight record kind")
        by_kind = {
            kind: [row for row in preflight_rows if row["record_kind"] == kind]
            for kind in kinds
        }
        field_maxima = result.get("preflight_maxima")
        if not isinstance(field_maxima, dict):
            raise CertificateError("preflight_maxima must be an object")

        def required(row: dict[str, str], field: str) -> float:
            value = optional_float(row, field)
            if value is None:
                raise CertificateError(
                    f"{row['record_kind']}/{row['probe']}: missing {field}"
                )
            return value

        def scalar_close(lhs: float, rhs: float, tolerance: float = 2e-12) -> bool:
            return abs(lhs - rhs) <= tolerance * max(1.0, abs(rhs))

        preflight_blocks = {
            name: matrix
            for (key, name), matrix in gram_artifacts.items()
            if key == CandidateKey("", "preflight", "final")
        }
        locked_preflight_blocks = {
            "PROBE_K_PROBE", "PROBE_K_T_H0", "TINV_H0_K_PROBE",
            "PROBE_K_T_H1", "TINV_H1_K_PROBE",
        }
        if not set(preflight_blocks).issubset(locked_preflight_blocks):
            raise CertificateError("unknown preflight Gram block")
        probe_gram = preflight_blocks.get("PROBE_K_PROBE")

        if not front_valid:
            if preflight_rows or hessian_rows or preflight_blocks:
                raise CertificateError("numerical artifacts exist before front gates")
        gradient_rows = by_kind["gradient"]
        gradient_local_ok = False
        gradient_ok = False
        gradient_global = math.inf
        if front_valid:
            if len(gradient_rows) != 4 or tuple(
                row["probe"] for row in gradient_rows
            ) != ("dx", "dp", "eT", "b"):
                raise CertificateError("gradient rows lack exact ordered coverage")
            global_values = [required(row, "energy_slope") for row in gradient_rows]
            if any(value < 0.0 for value in global_values) or any(
                value != global_values[0] for value in global_values[1:]
            ):
                raise CertificateError("gradient global primitive is inconsistent")
            gradient_global = global_values[0]
            row_passes: list[bool] = []
            for row in gradient_rows:
                component = required(row, "k_norm")
                local = bool(
                    0.0 <= component <= 1e-10
                    and 0.0 <= gradient_global <= 1e-10
                )
                if strict_csv_bool(row, "valid") != local:
                    raise CertificateError("gradient valid flag mismatch")
                if row["detail"] != ("pass" if local else "block_gradient_gate"):
                    raise CertificateError("gradient detail mismatch")
                row_passes.append(local)
            gradient_local_ok = all(row_passes)
            gradient_ok = bool(gradient_local_ok and runtime_replay["density"])
            gradient_components = {
                row["probe"]: required(row, "k_norm") for row in gradient_rows
            }
            check("gradient maximum JSON cross-check", all((
                scalar_close(
                    finite_float(field_maxima.get("gradient"), "gradient"),
                    gradient_global, 2e-15,
                ),
                scalar_close(
                    finite_float(field_maxima.get("gradient_dx"), "gradient dx"),
                    gradient_components["dx"], 2e-15,
                ),
                scalar_close(
                    finite_float(field_maxima.get("gradient_dp"), "gradient dp"),
                    gradient_components["dp"], 2e-15,
                ),
                scalar_close(
                    finite_float(field_maxima.get("gradient_eT"), "gradient eT"),
                    gradient_components["eT"], 2e-15,
                ),
                scalar_close(
                    finite_float(field_maxima.get("gradient_b"), "gradient b"),
                    gradient_components["b"], 2e-15,
                ),
            )))
        elif gradient_rows:
            raise CertificateError("gradient rows exist before front gates")

        seed_rows = by_kind["seed_metric"]
        numerical_front = bool(front_valid and hessian_ok and gradient_ok)
        if not numerical_front and (
            seed_rows or by_kind["energy"] or by_kind["field_control"]
            or energy_control_rows or field_control_rows or probe_gram is not None
        ):
            raise CertificateError("seed/energy/field artifacts exist before their gates")
        if (len(seed_rows) == 1) != (probe_gram is not None):
            raise CertificateError("seed row and probe Gram must appear together")
        if len(seed_rows) > 1:
            raise CertificateError("duplicate seed-metric row")
        seed_constructed = bool(numerical_front and seed_rows)
        b0_gram_residual = math.inf
        b0_ok = False
        if seed_constructed:
            seed_row = seed_rows[0]
            if seed_row["probe"] != "B0" or probe_gram is None:
                raise CertificateError("malformed seed-metric artifact")
            if probe_gram.shape != (16, 16) or not np.all(np.isfinite(probe_gram)):
                raise CertificateError("probe K Gram is not finite 16x16")
            probe_diagonal = np.diag(probe_gram)
            if np.any(probe_diagonal <= 0.0):
                raise CertificateError("probe K Gram has nonpositive diagonal")
            if np.linalg.norm(probe_gram - probe_gram.T) > 1e-10:
                raise CertificateError("probe K Gram is not symmetric")
            b0_gram_residual = float(
                np.linalg.norm(probe_gram[:4, :4] - np.eye(4))
            )
            b0_ok = b0_gram_residual <= 1e-10
            if strict_csv_bool(seed_row, "valid") != b0_ok:
                raise CertificateError("seed-metric valid flag mismatch")
            if not scalar_close(
                required(seed_row, "k_norm"), b0_gram_residual, 2e-15
            ):
                raise CertificateError("seed-metric scalar mismatch")
            if seed_row["detail"] != ("pass" if b0_ok else "seed_metric_gate"):
                raise CertificateError("seed-metric detail mismatch")
            check(
                "B0 Gram JSON cross-check",
                scalar_close(
                    finite_float(field_maxima.get("b0_gram"), "B0 Gram"),
                    b0_gram_residual, 2e-15,
                ),
            )

        control_ready = seed_constructed
        energy_replay = replay_energy_control(energy_control_rows, control_ready)
        energy_rows = by_kind["energy"]
        energy_ok = False
        maximum_energy_slope = math.inf
        maximum_energy_relative = math.inf
        if control_ready:
            if len(energy_rows) != 16 or tuple(
                row["probe"] for row in energy_rows
            ) != PROBE_NAMES:
                raise CertificateError("energy rows lack exact ordered probe coverage")
            assert probe_gram is not None
            energy_passes: list[bool] = []
            slopes: list[float] = []
            relatives: list[float] = []
            for index, row in enumerate(energy_rows):
                probe = PROBE_NAMES[index]
                plus = energy_replay[(probe, 1)]
                minus = energy_replay[(probe, -1)]
                expected_square = float(probe_gram[index, index])
                if expected_square <= 0.0:
                    raise CertificateError(f"{probe} has nonpositive K square")
                slope = (plus["increment"] - minus["increment"]) / (2.0 * H_ENERGY)
                second = (plus["increment"] + minus["increment"]) / (H_ENERGY ** 2)
                relative = abs(second - expected_square) / expected_square
                k_norm = math.sqrt(expected_square)
                local = bool(
                    plus["valid"] and minus["valid"]
                    and abs(slope) <= 1e-8 and relative <= 1e-6
                )
                if strict_csv_bool(row, "valid") != local:
                    raise CertificateError(f"{probe} energy valid flag mismatch")
                comparisons = (
                    required(row, "h") == H_ENERGY,
                    scalar_close(
                        required(row, "gauss_pre"),
                        max(plus["gauss"], minus["gauss"]), 2e-15,
                    ),
                    scalar_close(required(row, "k_norm"), k_norm, 2e-15),
                    scalar_close(required(row, "energy_slope"), slope, 2e-12),
                    scalar_close(required(row, "energy_second"), second, 2e-12),
                    scalar_close(required(row, "energy_relative"), relative, 2e-12),
                )
                if not all(comparisons):
                    raise CertificateError(f"{probe} energy primitive mismatch")
                if row["detail"] != ("pass" if local else "energy_form_gate"):
                    raise CertificateError(f"{probe} energy detail mismatch")
                energy_passes.append(local)
                slopes.append(abs(slope))
                relatives.append(relative)
            energy_ok = all(energy_passes)
            maximum_energy_slope = max(slopes)
            maximum_energy_relative = max(relatives)
            check("energy maxima JSON cross-check", all((
                scalar_close(
                    finite_float(field_maxima.get("energy_slope"), "energy slope"),
                    maximum_energy_slope, 2e-12,
                ),
                scalar_close(
                    finite_float(field_maxima.get("energy_relative"), "energy relative"),
                    maximum_energy_relative, 2e-12,
                ),
            )))
        elif energy_rows:
            raise CertificateError("energy rows exist without signed controls")

        field_replay = replay_field_control(field_control_rows, control_ready)
        field_rows = by_kind["field_control"]
        field_ok = False
        if control_ready:
            if len(field_rows) != 1:
                raise CertificateError("field-control preflight row is not unique")
            field_row = field_rows[0]
            field_ok = bool(field_replay["pass"])
            if strict_csv_bool(field_row, "valid") != field_ok:
                raise CertificateError("field-control valid flag mismatch")
            if not all((
                field_row["probe"] == "family100_n1_p0_e0",
                scalar_close(required(field_row, "recovery"), field_replay["recovery"]),
                scalar_close(
                    required(field_row, "energy_drift"),
                    field_replay["maximum_energy_drift"],
                ),
                scalar_close(
                    required(field_row, "composition_residual"),
                    field_replay["recurrence"],
                ),
            )):
                raise CertificateError("field-control preflight scalar mismatch")
            if field_row["detail"] != (
                "pass" if field_ok else "source_free_field_control"
            ):
                raise CertificateError("field-control detail mismatch")
            check("field-control JSON cross-check", all((
                scalar_close(
                    finite_float(
                        field_maxima.get("field_phase_relative"), "field phase"
                    ), field_replay["phase_relative"],
                ),
                scalar_close(
                    finite_float(field_maxima.get("field_recurrence"), "field recurrence"),
                    field_replay["recurrence"],
                ),
                scalar_close(
                    finite_float(field_maxima.get("field_recovery"), "field recovery"),
                    field_replay["recovery"],
                ),
            )))
        elif field_rows:
            raise CertificateError("field preflight row exists without trace")

        endpoint_ready = bool(
            numerical_front and b0_ok and energy_ok and field_ok
        )
        preflight_evaluations = replay_preflight_derivative_status(
            preflight_derivative_status_rows, endpoint_ready
        )
        cache_ok, cache_replay = replay_cache_control(
            cache_control_rows, endpoint_ready
        )
        endpoint_rows = by_kind["endpoint"]
        derivative_rows = by_kind["derivative"]
        zero_rows = by_kind["zero_control"]
        endpoint_ok = False
        regularity_ok = False
        adjoint_ok = False
        if endpoint_ready:
            expected_endpoint_keys = {
                (probe, h, direction, sign)
                for probe in PROBE_NAMES for h in (H0, H1)
                for direction in ("forward", "reverse") for sign in (-1, 1)
            }
            endpoint_by_key: dict[tuple[str, float, str, int], dict[str, str]] = {}
            for row in endpoint_rows:
                sign = strict_int_text(row["sign"], "endpoint sign")
                key = (row["probe"], required(row, "h"), row["direction"], sign)
                if key in endpoint_by_key:
                    raise CertificateError(f"duplicate endpoint key {key}")
                endpoint_by_key[key] = row
            if set(endpoint_by_key) != expected_endpoint_keys:
                raise CertificateError("endpoint rows lack exact 128-key coverage")

            groups_by_id: dict[int, list[dict[str, str]]] = {}
            for row in preflight_derivative_status_rows[:-1]:
                evaluation_id = optional_int(row, "evaluation_id")
                if evaluation_id is None:
                    raise CertificateError("preflight derivative row lacks ID")
                groups_by_id.setdefault(evaluation_id, []).append(row)
            raw_groups = {
                key: groups_by_id[evaluation.evaluation_id]
                for key, evaluation in preflight_evaluations.items()
            }
            probe_index = {probe: index for index, probe in enumerate(PROBE_NAMES)}
            signed_passes: list[bool] = []
            regularity_passes: list[bool] = []
            for key, row in endpoint_by_key.items():
                probe, h, direction, sign = key
                bundle_key = (
                    "probe", f"probe:{probe}", h, direction, probe_index[probe]
                )
                group = raw_groups[bundle_key]
                retraction_row = group[0] if sign == 1 else group[2]
                endpoint_component = group[1] if sign == 1 else group[3]
                cache = cache_replay[key]
                retraction_pass = bool(
                    optional_csv_bool(retraction_row, "metadata") is True
                    and optional_csv_bool(retraction_row, "sector") is True
                    and optional_csv_bool(retraction_row, "finite") is True
                    and 0.0 <= required(retraction_row, "gauss") <= 1e-10
                    and 0.0 <= required(retraction_row, "poisson_absolute") <= 1e-13
                )
                endpoint_pass = strict_csv_bool(endpoint_component, "valid")
                if not all((
                    cache["retraction"] == retraction_pass,
                    scalar_close(cache["gauss"], required(retraction_row, "gauss"), 2e-15),
                    scalar_close(cache["poisson"], required(retraction_row, "poisson_absolute"), 2e-15),
                    cache["direct"] == strict_csv_bool(endpoint_component, "metadata"),
                    cache["inverse"] == strict_csv_bool(endpoint_component, "sector"),
                    cache["endpoint_chart"] == strict_csv_bool(endpoint_component, "endpoint_chart"),
                    scalar_close(required(row, "gauss_pre"), required(retraction_row, "gauss"), 2e-15),
                    scalar_close(required(row, "common_residual"), required(endpoint_component, "common_residual"), 2e-15),
                    scalar_close(required(row, "energy_drift"), required(endpoint_component, "energy_drift"), 2e-15),
                    scalar_close(required(row, "recovery"), required(endpoint_component, "recovery"), 2e-15),
                )):
                    raise CertificateError(f"endpoint/cache/ledger mismatch for {key}")
                sigma = required(row, "sigma_min")
                condition = required(row, "condition")
                scale = required(row, "scale_difference")
                observer = required(row, "observer_regression")
                regular = bool(
                    cache["observer"] and sigma >= 1e-3
                    and 0.0 <= condition <= 1e4
                    and 0.0 <= scale <= 1e-5
                    and 0.0 <= observer <= 1e-12
                )
                expected_valid = bool(
                    retraction_pass and endpoint_pass and regular
                    and cache["cache_pass"]
                )
                if strict_csv_bool(row, "valid") != expected_valid:
                    raise CertificateError(f"endpoint valid flag mismatch for {key}")
                if cache["reported_valid"] != bool(
                    retraction_pass and endpoint_pass and cache["cache_pass"]
                ):
                    raise CertificateError(f"cache valid flag mismatch for {key}")
                expected_refreshes = (
                    cache["population_refreshes"] + cache["reuse_refreshes"]
                )
                expected_reuses = cache["population_reuses"] + cache["reuse_reuses"]
                expected_fallbacks = (
                    cache["population_fallbacks"] + cache["reuse_fallbacks"]
                )
                if not all((
                    required(row, "jacobian_refreshes") == expected_refreshes,
                    required(row, "jacobian_reuses") == expected_reuses,
                    required(row, "cache_fallbacks") == expected_fallbacks,
                )):
                    raise CertificateError(f"endpoint cache counters mismatch for {key}")
                signed_passes.append(retraction_pass and endpoint_pass)
                regularity_passes.append(regular)
            endpoint_root_ok = all(signed_passes)
            regularity_ok = all(regularity_passes)

            derivative_by_key: dict[tuple[str, float, str], dict[str, str]] = {}
            for row in derivative_rows:
                key = (row["probe"], required(row, "h"), row["direction"])
                if key in derivative_by_key:
                    raise CertificateError(f"duplicate derivative key {key}")
                derivative_by_key[key] = row
            expected_derivative_keys = {
                (probe, h, direction)
                for probe in PROBE_NAMES for h in (H0, H1)
                for direction in ("forward", "reverse")
            }
            if set(derivative_by_key) != expected_derivative_keys:
                raise CertificateError("derivative rows lack exact 64-key coverage")
            derivative_passes: list[bool] = []
            for (probe, h, direction), row in derivative_by_key.items():
                evaluation = preflight_evaluations[
                    ("probe", f"probe:{probe}", h, direction, probe_index[probe])
                ]
                group = raw_groups[
                    ("probe", f"probe:{probe}", h, direction, probe_index[probe])
                ]
                codec = group[4]
                if strict_csv_bool(row, "valid") != evaluation.passed:
                    raise CertificateError("derivative valid flag mismatch")
                for preflight_field, ledger_field in (
                    ("gauss_pre", "codec_gauss_pre"),
                    ("gauss_clean", "codec_gauss_clean"),
                    ("hodge_correction", "hodge_correction"),
                    ("reconstruction", "reconstruction"),
                    ("harmonic_face", "harmonic_face"),
                    ("harmonic_edge", "harmonic_edge"),
                ):
                    if not scalar_close(
                        required(row, preflight_field),
                        required(codec, ledger_field), 2e-15,
                    ):
                        raise CertificateError("derivative codec scalar mismatch")
                if row["detail"] != codec["detail"]:
                    raise CertificateError("derivative codec detail mismatch")
                scale = required(row, "derivative_scale_relative")
                composition = optional_float(row, "composition_residual")
                numeric = bool(
                    required(row, "k_norm") > 0.0
                    and 0.0 <= scale <= 1e-3
                    and (
                        (h == H0 and composition is not None
                         and 0.0 <= composition <= 1e-4)
                        or (h == H1 and composition is None)
                    )
                )
                derivative_passes.append(evaluation.passed and numeric)
            derivative_ok = all(derivative_passes)
            composition_ok = all(
                evaluation.passed
                for key, evaluation in preflight_evaluations.items()
                if key[0] == "composition"
            )
            if not composition_ok:
                # The primitive ledger itself proves this failure; producer
                # aggregate flags remain mere cross-checks below.
                pass

            if len(zero_rows) != 1:
                raise CertificateError("zero-control row is not unique")
            zero_row = zero_rows[0]
            zero_bundle_pass = all(
                preflight_evaluations[("zero", "zero", H0, direction, 0)].passed
                for direction in ("forward", "reverse")
            )
            zero_numeric = bool(
                zero_row["probe"] == "zero" and required(zero_row, "h") == H0
                and 0.0 <= required(zero_row, "recovery") <= 1e-10
                and required(zero_row, "k_norm") == 0.0
                and required(zero_row, "composition_residual") == 0.0
            )
            zero_ok = zero_bundle_pass and zero_numeric
            if strict_csv_bool(zero_row, "valid") != zero_ok:
                raise CertificateError("zero-control valid flag mismatch")

            required_adjoint_names = (
                "PROBE_K_T_H0", "TINV_H0_K_PROBE",
                "PROBE_K_T_H1", "TINV_H1_K_PROBE",
            )
            if any(name not in preflight_blocks for name in required_adjoint_names):
                raise CertificateError("preflight adjoint Gram set is incomplete")
            assert probe_gram is not None
            diagonal = np.diag(probe_gram)
            if np.any(diagonal <= 0.0):
                raise CertificateError("negative/nonpositive adjoint probe K square")
            denominators = np.sqrt(diagonal[:, None] * diagonal[None, :])
            adjoint_residuals: list[float] = []
            for h_label in ("H0", "H1"):
                left = preflight_blocks[f"PROBE_K_T_{h_label}"]
                right = preflight_blocks[f"TINV_{h_label}_K_PROBE"]
                if left.shape != (16, 16) or right.shape != (16, 16):
                    raise CertificateError("preflight adjoint Gram is not 16x16")
                adjoint_residuals.append(float(np.max(
                    np.abs(left - right) / denominators
                )))
            maximum_adjoint = max(adjoint_residuals)
            adjoint_ok = maximum_adjoint <= 1e-4
            check(
                "adjoint maximum JSON cross-check",
                scalar_close(
                    finite_float(field_maxima.get("adjoint"), "adjoint maximum"),
                    maximum_adjoint,
                ),
            )
            common_maximum = max(
                required(row, "common_residual") for row in endpoint_rows
            )
            endpoint_energy_drift = max(
                required(row, "energy_drift") for row in endpoint_rows
            )
            recovery_maximum = max(
                required(row, "recovery") for row in endpoint_rows
            )
            sigma_minimum = min(required(row, "sigma_min") for row in endpoint_rows)
            condition_maximum = max(
                required(row, "condition") for row in endpoint_rows
            )
            regularity_scale_maximum = max(
                required(row, "scale_difference") for row in endpoint_rows
            )
            observer_maximum = max(
                required(row, "observer_regression") for row in endpoint_rows
            )
            derivative_scale_maximum = max(
                required(row, "derivative_scale_relative")
                for row in derivative_rows
            )
            composition_maximum = max(
                required(row, "composition_residual")
                for row in derivative_rows
                if required(row, "h") == H0
            )
            diagnostic_codecs = [
                raw_groups[key][4]
                for key in preflight_evaluations
                if key[0] in {"probe", "composition"}
            ]
            codec_divergence_maximum = max(
                max(
                    required(row, "codec_gauss_pre"),
                    required(row, "codec_gauss_clean"),
                )
                for row in diagnostic_codecs
            )
            hodge_maximum = max(
                required(row, "hodge_correction") for row in diagnostic_codecs
            )
            reconstruction_maximum = max(
                required(row, "reconstruction") for row in diagnostic_codecs
            )
            harmonic_maximum = max(
                max(
                    required(row, "harmonic_face"),
                    required(row, "harmonic_edge"),
                )
                for row in diagnostic_codecs
            )
            maximum_replays = {
                "common_residual": common_maximum,
                "energy_drift": endpoint_energy_drift,
                "recovery": recovery_maximum,
                "scale_relative": derivative_scale_maximum,
                "composition": composition_maximum,
                "minimum_sigma": sigma_minimum,
                "maximum_condition": condition_maximum,
                "regularity_scale": regularity_scale_maximum,
                "observer_regression": observer_maximum,
                "codec_divergence": codec_divergence_maximum,
                "hodge_correction": hodge_maximum,
                "reconstruction": reconstruction_maximum,
                "harmonic": harmonic_maximum,
            }
            check(
                "endpoint/codec maxima JSON cross-check",
                all(
                    scalar_close(
                        finite_float(field_maxima.get(name), name), value
                    )
                    for name, value in maximum_replays.items()
                ),
            )
            endpoint_ok = bool(
                endpoint_root_ok and derivative_ok and composition_ok
                and zero_ok and adjoint_ok
            )
        elif (
            endpoint_rows or derivative_rows or zero_rows
            or preflight_derivative_status_rows or cache_control_rows
        ):
            raise CertificateError("endpoint artifacts exist before endpoint gates")

        expected_preflight_blocks = (
            locked_preflight_blocks if endpoint_ready
            else {"PROBE_K_PROBE"} if seed_constructed
            else set()
        )
        if set(preflight_blocks) != expected_preflight_blocks:
            raise CertificateError("preflight Gram block coverage mismatch")
        expected_preflight_row_count = (
            215 if endpoint_ready else 22 if seed_constructed else 4 if front_valid else 0
        )
        if len(preflight_rows) != expected_preflight_row_count:
            raise CertificateError("preflight row count does not match execution phase")

        preflight_ok = bool(
            b0_ok and energy_ok and field_ok and endpoint_ok
            and regularity_ok and cache_ok
        )
        check("reported gradient gate matches primitives",
              truth(result.get("gradient_pass")) == gradient_ok)
        check("reported energy-form gate matches primitives",
              truth(result.get("energy_form_pass")) == energy_ok)
        check("reported B0 seed-metric gate matches primitives",
              truth(result.get("seed_metric_pass")) == b0_ok)
        check("reported endpoint gate matches primitives",
              truth(result.get("endpoint_preflight_pass")) == endpoint_ok)
        check("reported regularity gate matches primitives",
              truth(result.get("regularity_pass")) == regularity_ok)
        check("reported cache-control gate matches primitives",
              truth(result.get("cache_control_pass")) == cache_ok)
        check("reported field-control gate matches primitives",
              truth(result.get("field_control_pass")) == field_ok)
        check(
            "preflight Gram coverage is exact",
            sum(
                1 for row in gram_rows
                if CandidateKey(
                    row["candidate_id"], row["construction"], row["stage"]
                ) == CandidateKey("", "preflight", "final")
            ) == 256 * len(expected_preflight_blocks),
        )

        check("reported aggregate preflight gate matches replay",
              truth(result.get("preflight_pass")) == preflight_ok)

        pre_status_execution_valid = front_valid and hessian_ok and preflight_ok
        status_complete, status_abort, execution_evaluations = (
            replay_execution_status(
                execution_status_rows, pre_status_execution_valid
            )
        )
        execution_valid = pre_status_execution_valid and status_complete
        pre_krylov_execution_valid = execution_valid
        krylov_replays = replay_krylov_status(
            krylov_status_rows, status_complete
        )
        krylov_bookkeeping_valid = bool(
            krylov_replays
            and all(status.structural for status in krylov_replays.values())
        ) if status_complete else False
        if status_complete:
            execution_valid = execution_valid and krylov_bookkeeping_valid
        candidate_gram_rows = [
            row for row in gram_rows
            if not (
                row["candidate_id"] == ""
                and row["construction"] == "preflight"
                and row["stage"] == "final"
                and row["block"] in {
                    "PROBE_K_PROBE", "PROBE_K_T_H0", "TINV_H0_K_PROBE",
                    "PROBE_K_T_H1", "TINV_H1_K_PROBE",
                }
            )
        ]
        downstream_empty = bool(
            not projected_rows and not cluster_rows and not metric_rows
            and not index_rows and VECTORS.stat().st_size == 0
            and not candidate_gram_rows and not krylov_status_rows
        )
        check(
            "recorded post-preflight failure leaves candidate corpus empty",
            not status_abort or downstream_empty,
        )
        check(
            "post-preflight ledger is absent only before a passing preflight",
            pre_status_execution_valid or not execution_status_rows,
        )
        check(
            "post-preflight ledger has exactly one reconstructed terminal branch",
            (status_complete ^ status_abort)
            if pre_status_execution_valid else not (status_complete or status_abort),
        )

        # --------------------------------------------------------------
        # Projected matrices and exact cluster enumeration.
        allowed_constructions = {"primary", "h1", "sign", "rotation"}
        allowed_stages = {"prior", "final"}
        projected_groups: dict[tuple[str, str, int, str], list[dict[str, str]]] = {}
        for row in projected_rows:
            construction, stage = row["construction"], row["stage"]
            if construction not in allowed_constructions or stage not in allowed_stages:
                raise CertificateError(f"unknown construction/stage {construction}/{stage}")
            key = (construction, stage, int(row["dimension"]), row["matrix"])
            projected_groups.setdefault(key, []).append(row)
        projected_matrices = {key: matrix_from_rows(rows) for key, rows in projected_groups.items()}
        matrix_sets: dict[tuple[str, str, int], dict[str, np.ndarray]] = {}
        for (construction, stage, dimension, name), matrix in projected_matrices.items():
            matrix_sets.setdefault((construction, stage, dimension), {})[name] = matrix

        reconstructed_clusters: dict[tuple[str, str, int], list[dict[str, Any]]] = {}
        terminal_invariance: dict[tuple[str, str, int], tuple[float, float, bool]] = {}
        status_matrices: dict[tuple[str, str, int], np.ndarray] = {}
        krylov_generated_blocks: dict[str, int] = {}
        projected_valid = bool(matrix_sets)
        for key, matrices in matrix_sets.items():
            construction, stage, dimension = key
            matrix_names = {
                "A_S", "A_T", "A_TINV", "SEED", "V_K_V",
                "TV_K_TV", "TINV_V_K_TINV_V",
                "T_RESIDUAL_K_T_RESIDUAL",
                "TINV_RESIDUAL_K_TINV_RESIDUAL",
            }
            expected_matrix_names = matrix_names | (
                {"KRYLOV_STATUS"} if stage == "final" else set()
            )
            if set(matrices) != expected_matrix_names:
                raise CertificateError(f"{key}: projected matrix set is {set(matrices)}")
            (
                a_s, a_t, a_tinv, seed, v_k_v, tv_k_tv,
                tinv_v_k_tinv_v, t_residual_gram, tinv_residual_gram,
            ) = (
                matrices["A_S"], matrices["A_T"], matrices["A_TINV"],
                matrices["SEED"], matrices["V_K_V"], matrices["TV_K_TV"],
                matrices["TINV_V_K_TINV_V"],
                matrices["T_RESIDUAL_K_T_RESIDUAL"],
                matrices["TINV_RESIDUAL_K_TINV_RESIDUAL"],
            )
            if any(matrix.shape != (dimension, dimension) for matrix in (
                a_s, a_t, a_tinv, v_k_v, tv_k_tv, tinv_v_k_tinv_v,
                t_residual_gram, tinv_residual_gram,
            )):
                raise CertificateError(f"{key}: projected square-matrix shape mismatch")
            if seed.shape != (dimension, 4):
                raise CertificateError(f"{key}: projected seed shape {seed.shape}")
            symmetry = fro_relative(a_s, a_s.T)
            projected_valid &= symmetry <= 1e-4
            projected_valid &= float(np.linalg.norm(v_k_v - np.eye(dimension))) <= 1e-10
            self_grams = {
                "V_K_V": v_k_v, "TV_K_TV": tv_k_tv,
                "TINV_V_K_TINV_V": tinv_v_k_tinv_v,
                "T_RESIDUAL_K_T_RESIDUAL": t_residual_gram,
                "TINV_RESIDUAL_K_TINV_RESIDUAL": tinv_residual_gram,
            }
            for block_name, gram in self_grams.items():
                k_gram_square(gram, f"{key}/{block_name}")
            forward_invariance, reverse_invariance = terminal_invariance_replay(
                tv_k_tv, t_residual_gram,
                tinv_v_k_tinv_v, tinv_residual_gram,
                str(key),
            )
            projected_valid &= (
                fro_relative(
                    t_residual_gram, tv_k_tv - a_t.T @ a_t
                ) <= 2e-8
                and fro_relative(
                    tinv_residual_gram,
                    tinv_v_k_tinv_v - a_tinv.T @ a_tinv,
                ) <= 2e-8
            )
            invariant = bool(
                forward_invariance <= 2e-4
                and reverse_invariance <= 2e-4
            )
            terminal_invariance[key] = (forward_invariance, reverse_invariance, invariant)
            if "KRYLOV_STATUS" in matrices:
                status_matrices[key] = matrices["KRYLOV_STATUS"]
            projected_valid &= fro_relative(a_s, (a_t + a_tinv) / 2.0) <= 2e-4
            mus, eigenvectors = np.linalg.eigh((a_s + a_s.T) / 2.0)
            projected_valid &= bool(np.all(mus >= -1.0 - 2e-4) and np.all(mus <= 1.0 + 2e-4))
            exact_domain = bool(np.all(mus >= -1.0) and np.all(mus <= 1.0))
            if not exact_domain:
                reconstructed_clusters[key] = []
                projected_valid = False
                continue
            phases = np.arccos(mus)
            order = np.argsort(phases)
            groups: list[list[int]] = []
            for index in order:
                if not groups or phases[index] - phases[groups[-1][-1]] > 5e-4:
                    groups.append([int(index)])
                else:
                    groups[-1].append(int(index))
            records: list[dict[str, Any]] = []
            for cluster_id, indices in enumerate(groups):
                z = eigenvectors[:, indices]
                overlap = float(np.linalg.norm(z.T @ seed) ** 2)
                in_window = bool(np.all(np.abs(phases[indices] - PHI_INT) <= PHASE_WINDOW))
                seed_linked = overlap >= 0.10
                eligible = in_window and seed_linked and len(indices) == 4
                records.append({
                    "cluster_id": cluster_id,
                    "indices": indices,
                    "mus": mus[indices],
                    "phases": phases[indices],
                    "seed_overlap": overlap,
                    "seed_linked": seed_linked,
                    "in_window": in_window,
                    "eligible": eligible,
                })
            reconstructed_clusters[key] = records
        terminal_eligibility: dict[str, bool] = {}
        for construction, status_row in krylov_replays.items():
            prior_keys = [
                key for key in matrix_sets
                if key[0] == construction and key[1] == "prior"
            ]
            final_keys = [
                key for key in matrix_sets
                if key[0] == construction and key[1] == "final"
            ]
            expected_prior_keys = 1 if status_row.prior > 0 else 0
            if len(prior_keys) != expected_prior_keys or (
                prior_keys and prior_keys[0][2] != status_row.prior
            ):
                raise CertificateError(
                    f"{construction}: projected prior does not match status"
                )
            expected_final_keys = 1 if status_row.projected_final else 0
            if len(final_keys) != expected_final_keys or (
                final_keys and final_keys[0][2] != status_row.accepted
            ):
                raise CertificateError(
                    f"{construction}: projected final does not match status"
                )
            krylov_generated_blocks[construction] = status_row.generated
            if not final_keys:
                terminal_eligibility[construction] = False
                continue
            final_key = final_keys[0]
            projected_status = status_matrices.get(final_key)
            if projected_status is None or projected_status.shape != (1, 13):
                raise CertificateError(
                    f"{construction}: final lacks 1x13 projected status"
                )
            values = projected_status[0]
            integer_indices = (0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12)
            if any(
                not math.isfinite(float(values[index]))
                or not float(values[index]).is_integer()
                for index in integer_indices
            ):
                raise CertificateError(
                    f"{construction}: projected status contains nonintegral fields"
                )
            expected_integers = (
                status_row.generated, status_row.accepted, status_row.prior,
                status_row.last_power, status_row.last_start,
                status_row.last_end, status_row.deflations,
                int(status_row.happy), int(status_row.exhausted),
            )
            if tuple(int(values[index]) for index in range(9)) != expected_integers:
                raise CertificateError(
                    f"{construction}: projected status disagrees with CSV primitives"
                )
            if int(values[12]) != int(status_row.structural):
                raise CertificateError(
                    f"{construction}: projected bookkeeping flag mismatch"
                )
            replay_t, replay_tinv, replay_eligible = terminal_invariance[final_key]
            if status_row.terminal_t is None or status_row.terminal_tinv is None:
                raise CertificateError(f"{construction}: missing terminal scalars")
            check(
                f"{construction} terminal invariance primitive replay",
                scalar_close(status_row.terminal_t, replay_t, 2e-10)
                and scalar_close(status_row.terminal_tinv, replay_tinv, 2e-10)
                and scalar_close(float(values[9]), replay_t, 2e-10)
                and scalar_close(float(values[10]), replay_tinv, 2e-10)
                and int(values[11]) == int(replay_eligible)
                and status_row.terminal_eligible_reported == replay_eligible,
            )
            terminal_eligibility[construction] = replay_eligible
        if set(key[0] for key in matrix_sets) - set(krylov_replays):
            raise CertificateError("projected matrices have unknown Krylov construction")

        basis_evaluations = {
            (
                evaluation.construction, evaluation.stage,
                evaluation.column, evaluation.direction,
            )
            for evaluation in execution_evaluations
            if evaluation.operation == "basis_image"
        }
        expected_basis_evaluations = {
            (construction, stage, column, direction)
            for construction, stage, dimension in matrix_sets
            for column in range(dimension)
            for direction in ("forward", "reverse")
        }
        check(
            "basis-image ledger exactly covers every serialized prior/final matrix set",
            not status_complete or basis_evaluations == expected_basis_evaluations,
        )
        for construction, generated in krylov_generated_blocks.items():
            observed_filter = {
                (
                    evaluation.power, evaluation.operation,
                    evaluation.column, evaluation.direction,
                )
                for evaluation in execution_evaluations
                if evaluation.construction == construction
                and evaluation.stage == "krylov"
            }
            expected_filter = {
                (power, operation, column, direction)
                for power in range(1, generated)
                for operation in ("filter_v", "filter_w")
                for column in range(4)
                for direction in ("forward", "reverse")
            }
            check(
                f"{construction} filter ledger matches Krylov generated-power count",
                observed_filter == expected_filter,
            )
        check(
            "Krylov-executed flag matches complete status primitive presence",
            truth(result.get("krylov_executed")) == bool(krylov_replays),
        )
        check(
            "invalid execution does not fabricate Krylov matrices",
            pre_krylov_execution_valid or not matrix_sets,
        )

        # Compare every reported cluster member with the independent eigensolve.
        reported_by_group: dict[tuple[str, str, int], list[dict[str, str]]] = {}
        for row in cluster_rows:
            matches = [key for key in reconstructed_clusters if key[0] == row["construction"] and key[1] == row["stage"]]
            if len(matches) != 1:
                raise CertificateError(f"cluster row has ambiguous matrix group: {row}")
            reported_by_group.setdefault(matches[0], []).append(row)
        cluster_match = True
        candidate_cluster_map: dict[CandidateKey, dict[str, Any]] = {}
        for key, clusters in reconstructed_clusters.items():
            rows = reported_by_group.get(key, [])
            expected_members = sum(len(cluster["indices"]) for cluster in clusters)
            cluster_match &= len(rows) == expected_members
            by_id = {cluster["cluster_id"]: cluster for cluster in clusters}
            rows_by_cluster: dict[int, list[dict[str, str]]] = {}
            for row in rows:
                rows_by_cluster.setdefault(int(row["cluster_id"]), []).append(row)
            for cluster_id, cluster_rows_for_id in rows_by_cluster.items():
                cluster = by_id.get(cluster_id)
                if cluster is None:
                    cluster_match = False
                    continue
                observed_indices = [int(row["index"]) for row in cluster_rows_for_id]
                cluster_match &= (
                    len(observed_indices) == len(set(observed_indices))
                    and set(observed_indices) == set(cluster["indices"])
                )
                candidate_ids = {row["candidate_id"] for row in cluster_rows_for_id}
                if cluster["eligible"]:
                    cluster_match &= len(candidate_ids) == 1 and "" not in candidate_ids
                    if len(candidate_ids) == 1 and "" not in candidate_ids:
                        candidate_key = CandidateKey(
                            next(iter(candidate_ids)), key[0], key[1]
                        )
                        cluster_match &= candidate_key not in candidate_cluster_map
                        candidate_cluster_map[candidate_key] = cluster
                else:
                    cluster_match &= candidate_ids == {""}
                for row in cluster_rows_for_id:
                    index = int(row["index"])
                    if index not in cluster["indices"]:
                        cluster_match = False
                        continue
                    member_position = cluster["indices"].index(index)
                    expected_mu = float(cluster["mus"][member_position])
                    expected_phase = float(cluster["phases"][member_position])
                    cluster_match &= abs(float(row["mu"]) - expected_mu) <= 2e-12
                    cluster_match &= abs(float(row["phase"]) - expected_phase) <= 2e-12
                    cluster_match &= int(row["rank"]) == len(cluster["indices"])
                    cluster_match &= abs(float(row["seed_overlap"]) - cluster["seed_overlap"]) <= 2e-10
                    cluster_match &= truth(row["seed_linked"]) == cluster["seed_linked"]
                    cluster_match &= truth(row["in_window"]) == cluster["in_window"]
                    cluster_match &= truth(row["eligible"]) == cluster["eligible"]
            cluster_match &= set(rows_by_cluster) == set(by_id)
        check("cluster enumeration replay", cluster_match)

        metric_by_key: dict[CandidateKey, dict[str, str]] = {}
        for row in metric_rows:
            metric_key = CandidateKey(
                row["candidate_id"], row["construction"], row["stage"]
            )
            if metric_key in metric_by_key:
                raise CertificateError(f"duplicate candidate metric key {metric_key}")
            metric_by_key[metric_key] = row
        check(
            "candidate metric IDs exactly match eligible cluster IDs",
            set(metric_by_key) == set(candidate_cluster_map),
        )

        # --------------------------------------------------------------
        # Binary tangent vectors and direct K replay.
        payload = VECTORS.read_bytes()
        vectors: dict[tuple[CandidateKey, str, int], np.ndarray] = {}
        occupied: list[tuple[int, int]] = []
        for row in index_rows:
            key = CandidateKey(row["candidate_id"], row["construction"], row["stage"])
            kind, column = row["vector_kind"], int(row["column"])
            dimension = int(row["chart_dimension"])
            offset, length = int(row["byte_offset"]), int(row["byte_length"])
            if dimension != D_RAW or length != 8 * D_RAW or offset % 8:
                raise CertificateError(f"invalid vector index row {row}")
            if offset < 0 or offset + length > len(payload):
                raise CertificateError(f"vector payload range outside binary: {row}")
            if any(not (offset + length <= lo or offset >= hi) for lo, hi in occupied):
                raise CertificateError(f"overlapping vector payload range: {row}")
            occupied.append((offset, offset + length))
            value = np.frombuffer(payload, dtype="<f8", count=D_RAW, offset=offset).copy()
            if not np.all(np.isfinite(value)):
                raise CertificateError(f"nonfinite tangent vector {key}/{kind}/{column}")
            vector_key = (key, kind, column)
            if vector_key in vectors:
                raise CertificateError(f"duplicate vector index {vector_key}")
            vectors[vector_key] = value
        ordered_ranges = sorted(occupied)
        payload_exact = (
            (not ordered_ranges and len(payload) == 0)
            or (
                bool(ordered_ranges)
                and ordered_ranges[0][0] == 0
                and ordered_ranges[-1][1] == len(payload)
                and all(left[1] == right[0] for left, right in zip(ordered_ranges, ordered_ranges[1:]))
            )
        )
        check("binary payload exactly and contiguously indexed", payload_exact)
        locked_vector_kinds = (
            "U", "SU", "TU", "TINVU", "TINV_TU", "T_TINVU",
        )
        expected_vector_coordinates = {
            (key, kind, column)
            for key in candidate_cluster_map
            for kind in locked_vector_kinds
            for column in range(4)
        }
        check(
            "eligible candidates have exact 24-vector full-chart coverage",
            set(vectors) == expected_vector_coordinates,
        )
        observed_candidate_evaluations = {
            (
                evaluation.construction, evaluation.stage,
                evaluation.operation, evaluation.column, evaluation.direction,
            )
            for evaluation in execution_evaluations
            if evaluation.operation.startswith("candidate_")
        }
        expected_candidate_evaluations = {
            (
                key.construction, key.stage,
                f"candidate_{operation}:{key.candidate_id}", column, direction,
            )
            for key in candidate_cluster_map
            for operation in ("image", "composition")
            for column in range(4)
            for direction in ("forward", "reverse")
        }
        check(
            "candidate component ledger exactly matches eligible candidate IDs",
            not status_complete
            or observed_candidate_evaluations == expected_candidate_evaluations,
        )

        k_form = KForm(h_red, beta, finite_float(result.get("lambda"), "lambda")) if h_red.size else None
        if k_form is not None:
            index = np.arange(3 * N_SITE, dtype=np.float64)
            edge_test = ((index % 17.0) - 8.0) / 17.0
            face_test = (((3.0 * index + 5.0) % 19.0) - 9.0) / 19.0
            adjoint_left = float(np.dot(k_form.curl(edge_test), face_test))
            adjoint_right = float(np.dot(edge_test, k_form.curl_adjoint(face_test)))
            check(
                "independent matched-curl transpose indexing",
                abs(adjoint_left - adjoint_right)
                <= 2e-12 * max(1.0, abs(adjoint_left), abs(adjoint_right)),
            )

        def vector_block(key: CandidateKey, kind: str) -> np.ndarray:
            columns = sorted(column for candidate, vector_kind, column in vectors if candidate == key and vector_kind == kind)
            if columns != list(range(len(columns))) or not columns:
                raise CertificateError(f"{key}/{kind}: missing or non-contiguous columns")
            return np.column_stack([vectors[(key, kind, column)] for column in columns])

        candidate_replays: dict[CandidateKey, CandidateReplay] = {}
        if execution_valid:
            for key, cluster in candidate_cluster_map.items():
                u = vector_block(key, "U")
                su = vector_block(key, "SU")
                tu = vector_block(key, "TU")
                tinvu = vector_block(key, "TINVU")
                tinv_tu = vector_block(key, "TINV_TU")
                t_tinvu = vector_block(key, "T_TINVU")
                if any(block.shape != (D_RAW, 4) for block in (u, su, tu, tinvu, tinv_tu, t_tinvu)):
                    raise CertificateError(f"{key}: candidate vector block is not D_raw x 4")
                check(f"{key} S vector identity", fro_relative(su, (tu + tinvu) / 2.0) <= 1e-10)
                assert k_form is not None
                u_ku = k_form.block(u, u)
                r = k_form.block(u, tu)
                r_minus = k_form.block(u, tinvu)
                direct_grams = {
                    "U_K_U": u_ku,
                    "U_K_TU": r,
                    "U_K_TINVU": r_minus,
                    "TU_K_TU": k_form.block(tu, tu),
                    "TINVU_K_TINVU": k_form.block(tinvu, tinvu),
                    "U_K_TINV_TU": k_form.block(u, tinv_tu),
                    "U_K_T_TINVU": k_form.block(u, t_tinvu),
                    "R_T_R": r.T @ r,
                }
                direct_gram_ok = True
                for block_name, computed in direct_grams.items():
                    if not np.all(np.isfinite(computed)):
                        raise CertificateError(
                            f"{key}/{block_name}: nonfinite K Gram"
                        )
                    if block_name in {
                        "U_K_U", "TU_K_TU", "TINVU_K_TINVU", "R_T_R",
                    } and np.any(np.diag(computed) < 0.0):
                        raise CertificateError(
                            f"{key}/{block_name}: negative K square"
                        )
                    stored = gram_artifacts.get((key, block_name))
                    if stored is None or stored.shape != computed.shape:
                        direct_gram_ok = False
                    else:
                        direct_gram_ok &= fro_relative(stored, computed) <= 2e-10
                check(f"{key} direct K-Gram replay", direct_gram_ok)
                mus = np.asarray(cluster["mus"], dtype=float)
                if mus.shape != (4,):
                    raise CertificateError(f"{key}: eligible cluster rank is not four")
                ritz_terms = []
                for column in range(4):
                    residual = su[:, column] - mus[column] * u[:, column]
                    ritz_terms.append(k_form.norm_f(residual) ** 2)
                ritz = math.sqrt(sum(ritz_terms) / 4.0)
                tu_norm = k_form.norm_f(tu)
                tinvu_norm = k_form.norm_f(tinvu)
                t_invariance = k_form.norm_f(tu - u @ r) / max(tu_norm, 1e-30)
                tinv_invariance = k_form.norm_f(tinvu - u @ r_minus) / max(tinvu_norm, 1e-30)
                tinv_t_residual = k_form.norm_f(tinv_tu - u) / max(k_form.norm_f(u), 1e-30)
                t_tinv_residual = k_form.norm_f(t_tinvu - u) / max(k_form.norm_f(u), 1e-30)
                adjoint = fro_relative(r_minus, r.T)
                orthogonality = float(np.linalg.norm(r.T @ r - np.eye(4)))
                eigenvalues = np.linalg.eigvals(r)
                conjugacy, separation, pairing = conjugate_pairing(eigenvalues)
                modulus = float(np.max(np.abs(np.abs(eigenvalues) - 1.0)))
                phases = np.array([math.atan2(abs(value.imag), value.real) for value in eigenvalues])
                phase_mean = float(np.mean(phases))
                phase_split = float(np.max(phases) - np.min(phases))
                products_positive = all(
                    (eigenvalues[i] * eigenvalues[j]).real > 0.0
                    and abs((eigenvalues[i] * eigenvalues[j]).imag)
                    <= 1e-8 * max(abs(eigenvalues[i] * eigenvalues[j]), 1e-30)
                    for i, j in pairing
                )
                raw_gram = gram_artifacts.get((key, "RAW_CANDIDATE_GRAM"))
                if raw_gram is None or raw_gram.shape != (4, 4):
                    raise CertificateError(f"{key}: missing 4x4 RAW_CANDIDATE_GRAM")
                if not np.all(np.isfinite(raw_gram)):
                    raise CertificateError(f"{key}: nonfinite raw-candidate K Gram")
                raw_gram_symmetry = fro_relative(raw_gram, raw_gram.T)
                gram_eigenvalues = np.linalg.eigvalsh((raw_gram + raw_gram.T) / 2.0)
                gram_min, gram_max = float(gram_eigenvalues[0]), float(gram_eigenvalues[-1])
                if gram_min < 0.0 or np.any(np.diag(raw_gram) < 0.0):
                    raise CertificateError(f"{key}: negative raw-candidate K square")
                gram_ratio = gram_min / max(gram_max, 1e-30)
                seed_gram = gram_artifacts.get((key, "U_K_B0"))
                if seed_gram is None:
                    raise CertificateError(f"{key}: missing U_K_B0 primitive")
                seed_overlap = float(np.linalg.norm(seed_gram) ** 2)
                seed_overlap_matches_projection = (
                    abs(seed_overlap - float(cluster["seed_overlap"])) <= 2e-8
                )
                u_orthonormal = float(np.linalg.norm(u_ku - np.eye(4))) <= 1e-10
                core = bool(
                    direct_gram_ok
                    and seed_overlap_matches_projection
                    and u_orthonormal
                    and raw_gram_symmetry <= 1e-10
                    and ritz <= 2e-4
                    and t_invariance <= 2e-4
                    and tinv_invariance <= 2e-4
                    and tinv_t_residual <= 1e-4
                    and t_tinv_residual <= 1e-4
                    and adjoint <= 2e-4
                    and orthogonality <= 2e-4
                    and np.all(np.abs(eigenvalues.imag) >= 1e-6)
                    and conjugacy <= 1e-8
                    and separation > 1e-10
                    and products_positive
                    and modulus <= 2e-4
                    and np.all(np.abs(phases - PHI_INT) <= PHASE_WINDOW)
                    and phase_split <= 1e-4
                    and seed_overlap >= 0.10
                    and gram_ratio >= 1e-6
                )
                candidate_replays[key] = CandidateReplay(
                    key=key, dimension=next(group[2] for group in reconstructed_clusters if group[0] == key.construction and group[1] == key.stage),
                    cluster_id=int(cluster["cluster_id"]), mus=mus,
                    phase_mean=phase_mean, phase_split=phase_split,
                    seed_overlap=seed_overlap, ritz_residual=ritz,
                    t_invariance=t_invariance, tinv_invariance=tinv_invariance,
                    tinv_t_residual=tinv_t_residual, t_tinv_residual=t_tinv_residual,
                    adjoint_residual=adjoint, orthogonality_residual=orthogonality,
                    modulus_residual=modulus, conjugacy_residual=conjugacy,
                    conjugacy_separation=separation, gram_min=gram_min,
                    gram_max=gram_max, gram_ratio=gram_ratio, phases=phases,
                    core_qualified=core,
                )

        def match(
            primary: CandidateReplay,
            construction: str,
            stage: str,
            tolerance: float,
        ) -> tuple[CandidateKey | None, float, bool]:
            assert k_form is not None
            left = vector_block(primary.key, "U")
            scored: list[tuple[float, CandidateKey, float]] = []
            for other in candidate_replays.values():
                if (
                    other.key == primary.key
                    or other.key.construction != construction
                    or other.key.stage != stage
                ):
                    continue
                right = vector_block(other.key, "U")
                cross = k_form.block(left, right)
                singular = np.linalg.svd(cross, compute_uv=False)
                if not np.all(np.isfinite(singular)):
                    raise CertificateError(
                        f"{primary.key}: matched-subspace singular values are nonfinite"
                    )
                overlap = float(np.linalg.norm(cross) ** 2)
                angle_square = 1.0 - float(singular[-1]) ** 2
                angle = math.sqrt(max(0.0, angle_square))
                scored.append((overlap, other.key, angle))
            scored.sort(key=lambda item: item[0], reverse=True)
            if not scored:
                return None, math.inf, False
            unique = len(scored) == 1 or scored[0][0] - scored[1][0] > 1e-8
            valid = scored[0][0] > 3.9 and unique and scored[0][2] <= tolerance
            return scored[0][1], scored[0][2], valid

        prior_match_ok: dict[CandidateKey, bool] = {}
        prior_core_ok: dict[CandidateKey, bool] = {}
        for candidate in candidate_replays.values():
            if candidate.key.stage != "final":
                continue
            prior_key, prior_angle, prior_ok = match(
                candidate, candidate.key.construction, "prior", 1e-3
            )
            status = krylov_replays[candidate.key.construction]
            if (
                candidate.dimension == 4 and status.prior == 0
                and prior_key is None
            ):
                prior_ok = True
                prior_core = True
                prior_angle = math.inf
            elif prior_key is None:
                prior_ok = False
                prior_core = False
            else:
                prior_core = bool(
                    prior_ok and candidate_replays[prior_key].core_qualified
                )
            candidate.matched_prior = prior_key
            candidate.prior_angle = prior_angle
            prior_match_ok[candidate.key] = prior_ok
            prior_core_ok[candidate.key] = prior_core
            stored_prior = gram_artifacts.get((candidate.key, "U_K_PRIOR"))
            if prior_key is None:
                prior_gram_ok = stored_prior is None or stored_prior.size == 0
            else:
                assert k_form is not None
                computed_prior = k_form.block(
                    vector_block(candidate.key, "U"), vector_block(prior_key, "U")
                )
                prior_gram_ok = bool(
                    stored_prior is not None
                    and stored_prior.shape == computed_prior.shape
                    and fro_relative(stored_prior, computed_prior) <= 2e-10
                )
            check(f"{candidate.key} prior/final K-Gram replay", prior_gram_ok)

        primary_candidates = [candidate for candidate in candidate_replays.values()
                              if candidate.key.construction == "primary" and candidate.key.stage == "final"]
        resolved_candidates: list[CandidateReplay] = []
        for candidate in primary_candidates:
            prior_key = candidate.matched_prior
            prior_angle = candidate.prior_angle
            prior_ok = prior_match_ok.get(candidate.key, False)
            h1_key, h1_angle, h1_ok = match(candidate, "h1", "final", 1e-2)
            sign_key, sign_angle, sign_ok = match(candidate, "sign", "final", 1e-6)
            rotation_key, rotation_angle, rotation_ok = match(candidate, "rotation", "final", 1e-6)
            candidate.matched_prior, candidate.prior_angle = prior_key, prior_angle
            candidate.matched_h1, candidate.h1_angle = h1_key, h1_angle
            candidate.matched_sign, candidate.sign_angle = sign_key, sign_angle
            candidate.matched_rotation, candidate.rotation_angle = rotation_key, rotation_angle
            assert k_form is not None
            primary_u = vector_block(candidate.key, "U")
            cross_gram_ok = True
            for block_name, matched_key in (
                ("U_K_H1", h1_key), ("U_K_SIGN", sign_key),
                ("U_K_ROT45", rotation_key),
            ):
                stored = gram_artifacts.get((candidate.key, block_name))
                if matched_key is None:
                    cross_gram_ok &= stored is None or stored.size == 0
                    continue
                computed = k_form.block(primary_u, vector_block(matched_key, "U"))
                cross_gram_ok &= bool(
                    stored is not None
                    and stored.shape == computed.shape
                    and fro_relative(stored, computed) <= 2e-10
                )
            if h1_key is not None and k_form is not None:
                u = primary_u
                w = vector_block(h1_key, "U")
                c_cross = k_form.block(u, w)
                r = k_form.block(u, vector_block(candidate.key, "TU"))
                r_h1 = k_form.block(w, vector_block(h1_key, "TU"))
                candidate.intertwining_residual = float(
                    np.linalg.norm(r @ c_cross - c_cross @ r_h1) / max(np.linalg.norm(r), 1e-30)
                )
                h1_phase_ok = abs(candidate.phase_mean - candidate_replays[h1_key].phase_mean) <= 1e-3
                h1_candidate_ok = bool(
                    candidate_replays[h1_key].core_qualified
                    and prior_core_ok.get(h1_key, False)
                )
                for block_name, computed in (("C_H1", c_cross), ("R_H1", r_h1)):
                    stored = gram_artifacts.get((candidate.key, block_name))
                    cross_gram_ok &= bool(
                        stored is not None
                        and stored.shape == computed.shape
                        and fro_relative(stored, computed) <= 2e-10
                    )
            else:
                h1_phase_ok = False
                h1_candidate_ok = False
            sign_candidate_ok = bool(
                sign_key is not None
                and candidate_replays[sign_key].core_qualified
                and prior_core_ok.get(sign_key, False)
            )
            rotation_candidate_ok = bool(
                rotation_key is not None
                and candidate_replays[rotation_key].core_qualified
                and prior_core_ok.get(rotation_key, False)
            )
            check(f"{candidate.key} cross-construction K-Gram replay", cross_gram_ok)
            h1_prior_ok = h1_key is not None and prior_match_ok.get(h1_key, False)
            sign_prior_ok = sign_key is not None and prior_match_ok.get(sign_key, False)
            rotation_prior_ok = (
                rotation_key is not None and prior_match_ok.get(rotation_key, False)
            )
            resolved = bool(
                prior_ok and h1_ok and h1_prior_ok
                and sign_ok and sign_prior_ok
                and rotation_ok and rotation_prior_ok
            )
            if resolved:
                resolved_candidates.append(candidate)
            candidate.qualified = bool(
                resolved and cross_gram_ok and candidate.core_qualified
                and prior_core_ok.get(candidate.key, False)
                and h1_candidate_ok and h1_phase_ok
                and sign_candidate_ok and rotation_candidate_ok
                and candidate.intertwining_residual <= 1e-3
            )

        rank_gt_four = any(
            cluster["in_window"] and cluster["seed_linked"] and len(cluster["indices"]) > 4
            for clusters in reconstructed_clusters.values() for cluster in clusters
        )
        eligible_primary_count = sum(
            1
            for key, clusters in reconstructed_clusters.items()
            if key[0] == "primary" and key[1] == "final"
            for cluster in clusters
            if cluster["eligible"]
        )
        required_terminal_constructions = {"primary", "h1", "sign", "rotation"}
        terminals_eligible = bool(
            required_terminal_constructions.issubset(terminal_eligibility)
            and all(terminal_eligibility[name] for name in required_terminal_constructions)
        )
        solve_resolved = bool(
            projected_valid and terminals_eligible
            and not rank_gt_four and resolved_candidates
        )
        qualified = [candidate for candidate in resolved_candidates if candidate.qualified]

        # Every candidate-metric scalar is a cross-check, never a verdict input.
        metric_match = set(metric_by_key) == set(candidate_replays)
        replay_fields = (
            "phase_mean", "phase_split", "seed_overlap", "ritz_residual",
            "prior_angle", "h1_angle", "sign_angle", "rotation_angle",
            "t_invariance", "tinv_invariance", "tinv_t_residual", "t_tinv_residual",
            "adjoint_residual", "orthogonality_residual", "modulus_residual",
            "conjugacy_residual", "conjugacy_separation", "intertwining_residual",
            "gram_min", "gram_max", "gram_ratio",
        )
        for key, replay in candidate_replays.items():
            row = metric_by_key.get(key)
            if row is None:
                metric_match = False
                continue
            metric_match &= int(row["dimension"]) == replay.dimension
            metric_match &= int(row["cluster_id"]) == replay.cluster_id
            metric_match &= int(row["rank"]) == 4
            metric_match &= abs(float(row["mu_min"]) - float(np.min(replay.mus))) <= 2e-10
            metric_match &= abs(float(row["mu_max"]) - float(np.max(replay.mus))) <= 2e-10
            for field in replay_fields:
                reported = optional_float(row, field)
                computed = float(getattr(replay, field))
                if reported is None:
                    metric_match &= not math.isfinite(computed)
                elif not math.isfinite(computed):
                    metric_match = False
                else:
                    metric_match &= abs(reported - computed) <= 2e-8 * max(1.0, abs(computed))
            metric_match &= truth(row["qualified"]) == replay.qualified
        check("candidate metric rows replay", metric_match)

        if not execution_valid:
            verdict = INVALID
        elif not solve_resolved:
            verdict = UNRESOLVED
        elif not qualified:
            verdict = NOT_QUALIFIED
        else:
            verdict = CONSTRUCTIVE

        check("independent ordered verdict matches record", result.get("verdict") == verdict)
        expected_companion = None if verdict == INVALID else COMPANION
        check("companion verdict ordered", result.get("companion_verdict") == expected_companion)
        check("eligible candidate count", result.get("eligible_candidate_count") == eligible_primary_count)
        check("qualified candidate count", result.get("qualified_candidate_count") == len(qualified))
        check("reported Krylov resolution is only a cross-check", truth(result.get("krylov_resolved")) == solve_resolved)
        selected = None
        if qualified:
            selected = sorted(
                qualified,
                key=lambda candidate: (
                    -candidate.seed_overlap,
                    candidate.ritz_residual,
                    candidate.phase_mean,
                ),
            )[0].key.candidate_id
        check("selected candidate tie-break replay", result.get("selected_candidate_id") == selected)

        reported_dimensions = result.get("construction_dimensions")
        dimension_replay: dict[str, int | None] = {}
        for construction, stage, label in (
            ("primary", "prior", "primary_prior"),
            ("primary", "final", "primary_final"),
            ("h1", "prior", "h1_prior"),
            ("h1", "final", "h1_final"),
            ("sign", "final", "sign_final"),
            ("rotation", "final", "rotation_final"),
        ):
            dimensions = sorted({key[2] for key in matrix_sets if key[0] == construction and key[1] == stage})
            if len(dimensions) > 1:
                raise CertificateError(f"multiple dimensions for {construction}/{stage}")
            dimension_replay[label] = dimensions[0] if dimensions else None
        check("construction dimensions replay", reported_dimensions == dimension_replay)

    except (CertificateError, KeyError, ValueError, OSError, json.JSONDecodeError) as exc:
        print("FTD-0774 independent tangent certificate: ARTIFACT/REPLAY INVALID")
        print(f"ERROR {exc}")
        return 1

    failures = [label for label, passed in checks if not passed]
    print(
        "FTD-0774 independent tangent certificate: "
        f"{len(checks) - len(failures)}/{len(checks)} checks PASS"
    )
    print(f"protocol_sha256={PROTOCOL_SHA256}")
    print(f"execution_valid={str(execution_valid).lower()}")
    print(f"solve_resolved={str(solve_resolved).lower()}")
    print(f"eligible_candidate_count={eligible_primary_count}")
    print(f"qualified_candidate_count={len(qualified)}")
    print(f"verdict={verdict}")
    for label in failures:
        print(f"FAIL {label}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
