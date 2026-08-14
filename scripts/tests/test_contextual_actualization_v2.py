from __future__ import annotations

import math
from pathlib import Path
import sys

import pytest

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts/experiments/temporal_interior"))

from contextual_actualization_v2 import (  # noqa: E402
    ClockController,
    CriticalClockState,
    SelectorState,
    chsh_value,
    factorization_residual,
    marginals,
    midpoint_pushforward,
    singlet_joint_weights,
    step_critical_clock,
)


def test_quantile_pushforward_and_determinism() -> None:
    weights = (0.125, 0.25, 0.375, 0.25)
    assert midpoint_pushforward(weights, 8000) == weights
    selector = SelectorState(0.42)
    assert selector.select(weights) == selector.select(weights)


def test_singlet_reference_is_contextual_and_no_signalling() -> None:
    weights = singlet_joint_weights(0.0, 0.0)
    assert factorization_residual(weights) >= 0.25 - 1e-15
    for right in (-1.2, -0.2, 0.8, 1.4):
        left_marginal, right_marginal = marginals(singlet_joint_weights(0.3, right))
        assert left_marginal == (0.5, 0.5)
        assert right_marginal == (0.5, 0.5)


def test_chsh_reference_reaches_tsirelson_not_pr() -> None:
    value = abs(chsh_value(0.0, 0.5 * math.pi, 0.25 * math.pi, -0.25 * math.pi))
    assert value == pytest.approx(2.0 * math.sqrt(2.0), abs=1e-14)
    assert value < 4.0


def test_clock_feedback_is_context_blind_and_audited() -> None:
    controller = ClockController(
        detuning_gain=0.25,
        amplitude_gain=0.2,
        detuning_tolerance=1e-8,
        relative_amplitude_tolerance=1e-8,
    )
    clock = CriticalClockState(0, -0.1, 0.0, 0, 0.6, 1.0, 0.3)
    for _ in range(140):
        step_critical_clock(clock, 0.1, controller)
    assert clock.compliant(controller)
    assert clock.gate_count >= 1
    assert clock.feedback_work > 0.0
    assert clock.dissipated_energy > 0.0
