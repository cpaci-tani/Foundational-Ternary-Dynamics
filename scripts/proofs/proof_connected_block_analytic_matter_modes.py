"""Independent FTD-0640 complete analytic matter-mode certificate."""
from __future__ import annotations

import csv
import hashlib
import json
import math
from collections import defaultdict
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
RESULT = ROOT / "engine/results/ftd_0640"
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_v1.md"
SHA = "203168B41C4B611695A7DF0AA9D311EF2A23AED10CA69900B46C834BA1DD7BDC"
REPRESENTATIVE = (0, 1, 3, 5, 6, 8, 12, 18, 24, 31, 39, 45, 47)


def relative(a: float, b: float) -> float:
    return abs(a - b) / max(1e-300, abs(a), abs(b))


def main() -> None:
    assert hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper() == SHA
    summary = json.loads((RESULT / "ftd_0640_connected_block_analytic_matter_modes_v1.json").read_text())
    arms = list(csv.DictReader((RESULT / "ftd_0640_connected_block_analytic_matter_modes_arms_v1.csv").open()))
    modes = list(csv.DictReader((RESULT / "ftd_0640_connected_block_analytic_matter_modes_modes_v1.csv").open()))
    ticks = list(csv.DictReader((RESULT / "ftd_0640_connected_block_analytic_matter_modes_ticks_v1.csv").open()))

    assert summary["protocol_sha256"] == SHA
    assert summary["parent_result_sha256"] == "DFA39E27F0317165D2A85E7778BBC7DA5691D1449DEEF20B4990C2AB9A1E7BD6"
    assert summary["verdict"] == "CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE"
    assert summary["production_changed"] is False
    assert summary["arm_count"] == 87 and summary["ticks_each_direction"] == 256
    assert all(summary[key] == 1 for key in (
        "coverage_pass", "execution_pass", "bounded_pass", "frequency_pass",
        "purity_pass", "amplitude_pass", "sign_pass", "covariance_pass"))
    assert len(arms) == 87 and len(modes) == 96 and len(ticks) == 87 * 256

    mode_map = {(int(row["orientation"]), int(row["mode"])): row for row in modes}
    arm_map = {(int(row["orientation"]), int(row["mode"]), row["kind"]): row for row in arms}
    tick_map: dict[str, list[dict[str, str]]] = defaultdict(list)
    for row in ticks:
        tick_map[row["label"]].append(row)

    for orientation in (0, 1):
        spectrum = np.array([float(mode_map[(orientation, m)]["hessian_eigen"]) for m in range(48)])
        assert np.all(np.diff(spectrum) >= -1e-12) and spectrum[0] > 1e-5
        vectors = np.array([[float(mode_map[(orientation, m)][f"v{i}"]) for i in range(48)]
                            for m in range(48)])
        assert np.max(np.abs(0.511 * vectors @ vectors.T - np.eye(48))) <= 1e-10

    # Post-result structural diagnostic (not an FTD-0640 verdict gate): the
    # six soft eigenvectors are the finite object's lattice-dressed rigid
    # translations and rotations, with a small measured deformation mixing.
    state_rows = [row for row in csv.DictReader(
        (ROOT / "engine/results/ftd_0638/ftd_0638_connected_block_analytic_static_refinement_states_v1.csv").open())
        if row["orientation"] == "0"]
    state_rows.sort(key=lambda row: int(row["particle"]))
    positions = np.array([[float(row[f"{axis}1"]) for axis in "xyz"] for row in state_rows])
    positions -= positions.mean(axis=0)
    rigid = []
    for axis in range(3):
        displacement = np.zeros((16, 3))
        displacement[:, axis] = 1.0
        rigid.append(displacement.ravel())
    for axis in np.eye(3):
        rigid.append(np.cross(np.broadcast_to(axis, positions.shape), positions).ravel())
    rigid_q, _ = np.linalg.qr(np.array(rigid).T)
    translation_q = rigid_q[:, :3]
    rotation_q = rigid_q[:, 3:]
    rigid_fractions = []
    for m in range(48):
        vector = np.array([float(mode_map[(0, m)][f"v{i}"]) for i in range(48)])
        vector /= np.linalg.norm(vector)
        translation = float(np.linalg.norm(translation_q.T @ vector) ** 2)
        rotation = float(np.linalg.norm(rotation_q.T @ vector) ** 2)
        rigid_fractions.append((translation, rotation))
    assert all(rigid_fractions[m][1] >= 1 - 1e-6 for m in (0, 3, 4))
    assert all(rigid_fractions[m][0] >= 1 - 1e-6 for m in (1, 2, 5))
    soft_vectors = np.array([[float(mode_map[(0, m)][f"v{i}"]) for m in range(6)]
                             for i in range(48)])
    soft_q, _ = np.linalg.qr(soft_vectors)
    principal_cosines = np.linalg.svd(rigid_q.T @ soft_q, compute_uv=False)
    rigid_subspace_defect = float(np.max(1.0 - principal_cosines ** 2))
    assert rigid_subspace_defect <= 1e-6
    internal_gap = (float(mode_map[(0, 6)]["hessian_eigen"])
                    / float(mode_map[(0, 5)]["hessian_eigen"]))
    assert internal_gap > 200
    spectrum_covariance = max(relative(
        float(mode_map[(0, m)]["hessian_eigen"]),
        float(mode_map[(1, m)]["hessian_eigen"])) for m in range(48))
    assert spectrum_covariance <= 1e-9
    assert abs(spectrum_covariance - float(summary["spectrum_covariance"])) <= 1e-15

    worst_common = worst_drift = worst_recovery = 0.0
    measured_phase: dict[tuple[int, int, str], float] = {}
    measured_leakage: dict[tuple[int, int, str], float] = {}
    for arm in arms:
        key = (int(arm["orientation"]), int(arm["mode"]), arm["kind"])
        assert all(arm[field] == "1" for field in (
            "initialization", "forward", "reverse", "bounded", "sector", "no_hops"))
        assert abs(float(arm["initial_max_displacement"]) - float(arm["target"])) <= 1e-12
        assert int(arm["max_multiplicity"]) <= 8
        assert float(arm["min_separation"]) >= 0.9
        assert float(arm["max_center"]) <= 1e-4
        assert float(arm["max_state"]) <= 1e-3
        assert float(arm["max_drift"]) <= 1e-12
        assert float(arm["max_common"]) <= 1e-10
        assert float(arm["recovery"]) <= 1e-10
        assert int(arm["jacobian_refreshes"]) >= 2
        assert int(arm["jacobian_reuses"]) > 0

        rows = sorted(tick_map[arm["label"]], key=lambda row: int(row["tick"]))
        assert len(rows) == 256 and [int(row["tick"]) for row in rows] == list(range(1, 257))
        target = int(arm["mode"])
        q = np.array([float(row[f"q{target}"]) for row in rows])
        numerator = float(np.dot(q[1:-1], q[2:] + q[:-2]))
        denominator = float(2.0 * np.dot(q[1:-1], q[1:-1]))
        phase = math.acos(float(np.clip(numerator / denominator, -1.0, 1.0)))
        assert abs(phase - float(arm["phase"])) <= 2e-13
        measured_phase[key] = phase

        target_group = int(mode_map[(key[0], target)]["group"])
        group_power: dict[int, float] = defaultdict(float)
        for row in rows:
            for m in range(48):
                group = int(mode_map[(key[0], m)]["group"])
                group_power[group] += float(row[f"q{m}"]) ** 2
        target_rms = math.sqrt(group_power[target_group] / len(rows))
        leakage = max((math.sqrt(power / len(rows)) for group, power in group_power.items()
                       if group != target_group), default=0.0) / target_rms
        assert abs(leakage - float(arm["leakage"])) <= 2e-13
        assert leakage <= 0.10
        measured_leakage[key] = leakage

        predicted = float(mode_map[(key[0], target)]["phase"])
        assert abs(predicted - float(arm["predicted_phase"])) <= 1e-15
        if arm["kind"] == "primary":
            assert relative(phase, predicted) <= 0.02
        worst_common = max(worst_common, float(arm["max_common"]))
        worst_drift = max(worst_drift, float(arm["max_drift"]))
        worst_recovery = max(worst_recovery, float(arm["recovery"]))

    assert len([key for key in arm_map if key[2] == "primary"]) == 48
    amplitude_residual = sign_residual = covariance_residual = 0.0
    for m in REPRESENTATIVE:
        primary = arm_map[(0, m, "primary")]
        half = arm_map[(0, m, "half")]
        negative = arm_map[(0, m, "negative")]
        cyclic = arm_map[(1, m, "cyclic")]
        p_phase = measured_phase[(0, m, "primary")]
        h_phase = measured_phase[(0, m, "half")]
        n_phase = measured_phase[(0, m, "negative")]
        y_phase = measured_phase[(1, m, "cyclic")]
        energy_ratio = float(primary["initial_excess"]) / float(half["initial_excess"])
        phase_amplitude = relative(p_phase, h_phase)
        amplitude_residual = max(amplitude_residual, phase_amplitude, abs(energy_ratio - 4.0))
        assert phase_amplitude <= 0.005 and 3.9 <= energy_ratio <= 4.1
        assert relative(p_phase, n_phase) <= 0.005
        p_rows = tick_map[primary["label"]]
        n_rows = tick_map[negative["label"]]
        signed = max(abs(float(p[f"q{m}"]) + float(n[f"q{m}"]))
                     / abs(float(primary["modal_amplitude"])) for p, n in zip(p_rows, n_rows))
        sign_residual = max(sign_residual, relative(p_phase, n_phase), signed)
        assert signed <= 0.05
        covariance_residual = max(covariance_residual, relative(p_phase, y_phase))
        assert relative(p_phase, y_phase) <= 0.005

    assert abs(amplitude_residual - float(summary["amplitude_residual"])) <= 2e-12
    assert abs(sign_residual - float(summary["sign_residual"])) <= 2e-12
    assert abs(covariance_residual - float(summary["covariance_residual"])) <= 2e-12
    assert abs(worst_common - float(summary["worst_common_residual"])) <= 1e-20
    assert abs(worst_drift - float(summary["worst_energy_drift"])) <= 1e-20
    assert abs(worst_recovery - float(summary["worst_recovery"])) <= 1e-20
    print("FTD-0640 certificate: 48 analytic modes and 39 controls pass 44,544 exact common-action ticks")
    print(f"post-result geometry: 3 translations + 3 rotations (subspace defect {rigid_subspace_defect:.3e}), then a {internal_gap:.6f}x internal-mode gap")


if __name__ == "__main__":
    main()
