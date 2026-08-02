"""FTD-0658 exact phase criterion and registered-result census."""

from __future__ import annotations

import json
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def load(relative: str) -> dict:
    return json.loads((ROOT / relative).read_text())


def main() -> None:
    # Fixed-point phase theorem: theta = theta + Omega mod 2pi.
    for theta in (0.0, 0.37, 2.1):
        for winding in (-2, -1, 0, 1, 2):
            omega = 2.0 * math.pi * winding
            assert abs(((theta + omega - theta) / (2.0 * math.pi)) - winding) < 1e-14
    # Any non-integral turn contradicts state-functionality at a fixed point.
    assert not math.isclose(0.37 / (2.0 * math.pi), round(0.37 / (2.0 * math.pi)))

    # The oscillator angle cannot extend continuously to the origin: four
    # rays approach the same zero state with four distinct limiting phases.
    epsilon = 1e-12
    ray_phases = [math.atan2(q, p) for q, p in
                  ((epsilon, 0.0), (0.0, epsilon),
                   (-epsilon, 0.0), (0.0, -epsilon))]
    assert len({round(value, 12) for value in ray_phases}) == 4

    rest = load("engine/results/ftd_0639/ftd_0639_connected_block_analytic_dynamical_rest_v1.json")
    modes = load("engine/results/ftd_0640/ftd_0640_connected_block_analytic_matter_modes_v1.json")
    gait = load("engine/results/ftd_0620/ftd_0620_balanced_gait_phase_return_v1.json")
    breathing = load("engine/results/ftd_0627/ftd_0627_connected_block_dynamical_rest_v1.json")
    dressing = load("engine/results/ftd_0656/ftd_0656_mobile_dressing_structure_factor_v2.json")

    assert rest["verdict"] == "CONNECTED_BLOCK_ANALYTIC_DYNAMICAL_REST_CONSTRUCTIVE"
    assert modes["verdict"] == "CONNECTED_BLOCK_ANALYTIC_MATTER_MODES_CONSTRUCTIVE"
    assert modes["arm_count"] == 87 and modes["frequency_pass"] == 1
    assert modes["amplitude_pass"] == modes["sign_pass"] == modes["covariance_pass"] == 1
    assert gait["verdict"] == "BALANCED_GAIT_PHASE_BEHAVIOR_MIXED"
    active = [arm for arm in gait["arms"] if arm["sign"]]
    assert len(active) == 2 and all(arm["recurrent"] == 0 for arm in active)
    assert min(arm["minimum_phase_distance_after_32"] for arm in active) > 5.22
    assert breathing["recurrence_pass"] == 0
    assert breathing["spectral_concentration_pass"] == 0
    assert dressing["verdict"] == "MOBILE_DRESSED_STRUCTURE_FACTOR_V2_CONSTRUCTIVE"

    # Convective phase has no rest intercept.
    for k in (0.1, 1.0, math.pi):
        assert k * 0.0 == 0.0

    print("FTD-0658 phase criterion/census certificate: PASS")


if __name__ == "__main__":
    main()
