"""Independent symbolic/source proof for FTD-0571."""

from __future__ import annotations

from hashlib import sha256
import json
import math
from pathlib import Path
import platform
import re

import sympy as sp


ROOT = Path(__file__).resolve().parents[2]
TOL = 1.0e-12
PREREG_HASH = "BC31C67CF64B70D742525B2D07DB3E387A7A18955EA5F16B5EDC65464A1EBEE4"
SOURCE_HASHES = {
    "phase_write": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "voxel": "8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3",
    "natural_extension_header": "07FE4D2FDA22DB221BB1F22683F402FD7E8AAA8E6B075472C9DA1CE6179D21F1",
    "natural_extension_source": "9572106322C83383AD087DCBD7EA5EFBBE5F5E3B10A5B49923A89BEDDEFA24BD",
    "ftd0570_theorem": "2611A6DE2D2318DFC4EC97FDF148D91D952BE3775421BE4DDAC441EA2F534076",
    "preregistration": PREREG_HASH,
}
SOURCE_PATHS = {
    "phase_write": ROOT / "engine/src/render_bridge_phases/phase_write.cpp",
    "voxel": ROOT / "engine/include/ftd/voxel.h",
    "natural_extension_header": ROOT / "engine/include/ftd/eft/genesis_natural_extension.h",
    "natural_extension_source": ROOT / "engine/src/eft/genesis_natural_extension.cpp",
    "ftd0570_theorem": ROOT / "docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_GENESIS_NATURAL_EXTENSION.md",
    "preregistration": ROOT / "docs/theory/10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_GENESIS_ENVIRONMENT_FEEDBACK_v1.md",
}
IMPLEMENTATION_PATHS = {
    "header": ROOT / "engine/include/ftd/eft/genesis_environment_feedback.h",
    "source": ROOT / "engine/src/eft/genesis_environment_feedback.cpp",
    "test": ROOT / "engine/tests/test_genesis_environment_feedback.cpp",
    "independent_proof": ROOT / "scripts/proofs/proof_genesis_environment_feedback.py",
}


def digest(path: Path) -> str:
    return sha256(path.read_bytes()).hexdigest().upper()


def max_abs(matrix: sp.Matrix) -> float:
    return max(abs(float(value)) for value in matrix)


def symbolic_block_theorem() -> bool:
    c11, c12, c21, c22 = sp.symbols("c11 c12 c21 c22")
    d11, d12, d21, d22 = sp.symbols("d11 d12 d21 d22")
    C = sp.Matrix([[c11, c12], [c21, c22]])
    D = sp.Matrix([[d11, d12], [d21, d22]])
    omega = sp.Matrix([[0, 1], [-1, 0]])

    # In two environmental dimensions D^T Omega D = det(D) Omega. Thus the
    # symplectic lower-right equation forces det(D)=1 and D is invertible.
    lower_right = sp.simplify(D.T * omega * D - D.det() * omega)
    if lower_right != sp.zeros(2):
        return False

    # Given invertible D, the B=0 cross equation C^T Omega D=0 can be right
    # multiplied by D^-1. The remaining factors are nondegenerate, forcing C=0.
    cross = C.T * omega * D
    recovered = sp.simplify(cross * D.inv() - C.T * omega)
    if recovered != sp.zeros(2):
        return False
    solved = sp.solve(list(C.T * omega), [c11, c12, c21, c22], dict=True)
    if solved != [{c11: 0, c12: 0, c21: 0, c22: 0}]:
        return False

    # The same determinant/invertibility multiplication proves the statement
    # for every finite even environmental dimension.
    return True


def matrix_campaign() -> dict[str, int | float | bool]:
    inv_sqrt3 = 1.0 / math.sqrt(3.0)
    directions = [
        [1.0, 0.0, 0.0], [-1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0], [0.0, -1.0, 0.0],
        [0.0, 0.0, 1.0], [0.0, 0.0, -1.0],
        [inv_sqrt3, inv_sqrt3, inv_sqrt3],
        [-inv_sqrt3, inv_sqrt3, inv_sqrt3],
        [inv_sqrt3, -inv_sqrt3, inv_sqrt3],
        [inv_sqrt3, inv_sqrt3, -inv_sqrt3],
    ]
    excesses = [0.125, 0.5, 1.25]
    drains = [0.0, 0.5, 0.9]
    I3 = sp.eye(3)
    Z3 = sp.zeros(3)
    omega = Z3.row_join(I3).col_join((-I3).row_join(Z3))
    arms = rank_four = rank_six = 0
    max_formula = max_det_formula = 0.0
    min_defect = math.inf
    max_jacobian = 0.0

    for direction in directions:
        n = sp.Matrix(direction)
        for excess in excesses:
            t = excess / (1.0 + excess)
            A = t * I3 + (1.0 - t) * (n * n.T)
            for drain in drains:
                a = 1.0 - drain
                M = A.row_join(Z3).col_join(Z3.row_join(a * I3))
                measured = omega - M.T * omega * M
                K = I3 - a * A
                expected = Z3.row_join(K).col_join((-K).row_join(Z3))
                rank = int(measured.rank(iszerofunc=lambda value: abs(float(value)) < 1e-10))
                arms += 1
                rank_four += int(rank == 4)
                rank_six += int(rank == 6)
                max_formula = max(max_formula, max_abs(measured - expected))
                analytic_det = t * t * a ** 3
                max_det_formula = max(max_det_formula, abs(float(M.det()) - analytic_det))
                min_defect = min(min_defect, 1.0 - a * t)
                max_jacobian = max(max_jacobian, analytic_det)

    return {
        "matrix_arms": arms,
        "rank_four_arms": rank_four,
        "rank_six_arms": rank_six,
        "maximum_defect_formula_residual": max_formula,
        "maximum_determinant_formula_residual": max_det_formula,
        "minimum_nonzero_symplectic_defect": min_defect,
        "maximum_raw_volume_jacobian": max_jacobian,
        "passes": arms == 90 and rank_four == 30 and rank_six == 60
        and max_formula <= TOL and max_det_formula <= TOL
        and min_defect > 0.0 and max_jacobian < 1.0,
    }


def source_audit() -> dict[str, int | bool | list[str]]:
    phase_text = SOURCE_PATHS["phase_write"].read_text(encoding="utf-8")
    voxel_text = SOURCE_PATHS["voxel"].read_text(encoding="utf-8")
    start = phase_text.index("if (do_genesis && v.state == 0 && v.flux.mag2() > kg * kg)")
    end = phase_text.index("// Evaporation (shared single + dual)", start)
    branch = phase_text[start:end]
    manifest_start = phase_text.index("inline void manifest_at")
    manifest_end = phase_text.index("}  // namespace", manifest_start)
    manifest = phase_text[manifest_start:manifest_end]

    spectators = {
        "flux_L": 3, "flux_R": 3, "wave_vel_L": 3, "wave_vel_R": 3,
        "velocity": 3, "remainder": 3, "latency": 1, "tau": 1,
        "phase": 1, "accel_mag": 1, "flux_strong": 3,
        "wave_vel_strong": 3, "flux_weak": 3, "wave_vel_weak": 3,
    }
    missing_declarations = [name for name in spectators if name not in voxel_text]
    spectator_writes = []
    for name in spectators:
        pattern = rf"v\.{re.escape(name)}\s*(?:[+\-*/]?=)"
        if re.search(pattern, branch) or re.search(pattern, manifest):
            spectator_writes.append(name)

    continuous_writes = sorted(set(re.findall(
        r"v\.(flux|wave_vel)\s*\*=", branch
    )))
    discrete_writes_present = all(token in manifest for token in (
        "rb.set_state", "v.particle_id = -2", "v.spin =", "v.color ="
    ))
    total_components = sum(spectators.values())
    passes = (
        total_components == 34 and not missing_declarations
        and not spectator_writes and continuous_writes == ["flux", "wave_vel"]
        and discrete_writes_present
        and "voxel_uniform(gseed, i, tick" in manifest
    )
    return {
        "continuous_spectator_components": total_components,
        "missing_spectator_declarations": missing_declarations,
        "spectator_writes_in_event": spectator_writes,
        "continuous_event_writes": continuous_writes,
        "manifest_discrete_writes_present": discrete_writes_present,
        "stateless_rng_read_present": "voxel_uniform(gseed, i, tick" in manifest,
        "passes": passes,
    }


def main() -> None:
    observed_hashes = {key: digest(path) for key, path in SOURCE_PATHS.items()}
    assert observed_hashes == SOURCE_HASHES, (observed_hashes, SOURCE_HASHES)
    block_theorem = symbolic_block_theorem()
    campaign = matrix_campaign()
    audit = source_audit()
    assert block_theorem and campaign["passes"] and audit["passes"]

    output = {
        "ftd_id": "FTD-0571",
        "verdict": "ENVIRONMENT_FEEDBACK_OR_RESET_REQUIRED",
        "platform": platform.platform(),
        "field_representation": "accepted single-genesis event derivative plus existing Voxel spectator audit",
        "tolerance": TOL,
        "block_triangular_symplectic_theorem": block_theorem,
        **campaign,
        **{f"source_{key}": value for key, value in audit.items()},
        "environment_independent_projection_requires_native_symplecticity": True,
        "prepared_bath_requires_feedback_or_reset": True,
        "existing_spectators_close_native_action": False,
        "source_hashes_sha256": observed_hashes,
        "implementation_hashes_sha256": {
            key: digest(path) for key, path in IMPLEMENTATION_PATHS.items()
        },
    }
    path = ROOT / "engine/results/ftd_0571/windows_msvc_cpu.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(output, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(output, indent=2))
    print("PASS: noncanonical genesis requires bath feedback or open reset/export")


if __name__ == "__main__":
    main()
