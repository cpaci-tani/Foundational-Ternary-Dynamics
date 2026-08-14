"""Run the FTD-0825 imposed contextual-actualization reference pipeline."""

from __future__ import annotations

import math

from contextual_actualization_v2 import (
    ClockController,
    CriticalClockState,
    SelectorState,
    chsh_value,
    singlet_joint_weights,
    step_critical_clock,
)


def main() -> None:
    contexts = (
        (0.0, 0.25 * math.pi),
        (0.0, -0.25 * math.pi),
        (0.5 * math.pi, 0.25 * math.pi),
        (0.5 * math.pi, -0.25 * math.pi),
    )
    rotor = SelectorState(0.3141592653589793)
    outcomes = []
    for left, right in contexts:
        weights = singlet_joint_weights(left, right)
        outcomes.append(rotor.select(weights))
        rotor.advance()

    controller = ClockController(
        detuning_gain=0.25,
        amplitude_gain=0.2,
        detuning_tolerance=1e-8,
        relative_amplitude_tolerance=1e-8,
    )
    clock = CriticalClockState(0, -0.1, 0.0, 0, 0.7, 1.0, 0.2)
    for _ in range(160):
        step_critical_clock(clock, 0.1, controller)

    value = chsh_value(0.0, 0.5 * math.pi, 0.25 * math.pi, -0.25 * math.pi)
    print("FTD-0825 CONTEXTUAL ACTUALIZATION REFERENCE SIMULATOR")
    print(f"deterministic_joint_outcome_indices={outcomes}")
    print(f"reference_chsh={value:.17g}")
    print(f"clock_global_tick={clock.global_tick}")
    print(f"clock_local_duration={clock.operational_duration:.17g}")
    print(f"clock_gate_count={clock.gate_count}")
    print(f"clock_feedback_work={clock.feedback_work:.17g}")
    print("scope=IMPOSED_REFERENCE_NOT_SUBSTRATE_BORN_RECOVERY")


if __name__ == "__main__":
    main()
