from __future__ import annotations

import math
from pathlib import Path
import sys

import pytest


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts/experiments/temporal_interior"))

from maintained_gstar_clock_v1 import (  # noqa: E402
    G_STAR,
    MaintenanceController,
    QuarticClockParameters,
    critical_quartic_period,
    harmonic_period,
    initialized_turning_point,
    normalized_detuning,
    step_maintained_clock,
    total_energy,
)


def _measure_period(amplitude: float) -> float:
    parameters = QuarticClockParameters(mass=1.3, quartic_coupling=0.7)
    controller = MaintenanceController(
        detuning_gain=0.0,
        amplitude_energy_gain=0.0,
        detuning_tolerance=1.0e-12,
        relative_amplitude_tolerance=2.0e-5,
    )
    state = initialized_turning_point(amplitude)
    exact = critical_quartic_period(amplitude, parameters.mass, parameters.quartic_coupling)
    dt = exact / 2400.0
    periods: list[float] = []
    for _ in range(int(7.0 * exact / dt)):
        result = step_maintained_clock(state, parameters, controller, dt)
        if result.measured_period is not None:
            periods.append(result.measured_period)
    assert len(periods) >= 4
    return sum(periods[-3:]) / 3.0


def _measure_harmonic_control_period(amplitude: float) -> float:
    """Velocity-Verlet control with no quartic term or G* input."""

    mass = 1.3
    stiffness = 0.7
    exact = harmonic_period(mass, stiffness)
    dt = exact / 2400.0
    q = amplitude
    p = 0.0
    time = 0.0
    last_crossing: float | None = None
    periods: list[float] = []
    for _ in range(int(7.0 * exact / dt)):
        old_q = q
        old_time = time
        half_p = p - 0.5 * dt * stiffness * q
        q += dt * half_p / mass
        p = half_p - 0.5 * dt * stiffness * q
        time += dt
        if old_q < 0.0 <= q and p > 0.0:
            fraction = -old_q / (q - old_q)
            crossing = old_time + fraction * dt
            if last_crossing is not None:
                periods.append(crossing - last_crossing)
            last_crossing = crossing
    assert len(periods) >= 4
    return sum(periods[-3:]) / 3.0


@pytest.mark.parametrize("amplitude", [0.5, 1.0, 1.8])
def test_unmaintained_critical_clock_recovers_gstar_period(amplitude: float) -> None:
    measured = _measure_period(amplitude)
    expected = critical_quartic_period(amplitude, 1.3, 0.7)
    assert measured == pytest.approx(expected, rel=3.0e-6)
    recovered = measured * amplitude * math.sqrt(2.0 * 0.7 / (math.pi * 1.3))
    assert recovered == pytest.approx(G_STAR, rel=3.0e-6)


def test_dimensionless_detuning_has_the_crossover_normalization() -> None:
    parameters = QuarticClockParameters(mass=1.0, quartic_coupling=0.5)
    state = initialized_turning_point(2.0, quadratic_coefficient=0.4)
    delta = normalized_detuning(state, parameters)
    assert delta == pytest.approx(0.1)
    assert 1.0 / (delta + 2.0) == pytest.approx(
        2.0 * parameters.quartic_coupling * state.target_amplitude**2
        / (state.quadratic_coefficient + 4.0 * parameters.quartic_coupling * state.target_amplitude**2)
    )


def test_harmonic_control_is_amplitude_independent_and_contains_no_gstar() -> None:
    low = _measure_harmonic_control_period(0.5)
    high = _measure_harmonic_control_period(1.8)
    expected = harmonic_period(1.3, 0.7)
    assert low == pytest.approx(expected, rel=3.0e-6)
    assert high == pytest.approx(expected, rel=3.0e-6)
    assert low == pytest.approx(high, rel=1.0e-10)

    quartic_low = critical_quartic_period(0.5, 1.3, 0.7)
    quartic_high = critical_quartic_period(1.8, 1.3, 0.7)
    assert quartic_low / quartic_high == pytest.approx(1.8 / 0.5)


def test_feedback_recovers_criticality_and_maintains_amplitude_with_energy_audit() -> None:
    parameters = QuarticClockParameters(mass=1.0, quartic_coupling=0.5, damping=0.003)
    controller = MaintenanceController(
        detuning_gain=0.08,
        amplitude_energy_gain=1.0,
        detuning_tolerance=2.0e-6,
        relative_amplitude_tolerance=2.0e-6,
    )
    state = initialized_turning_point(1.0, quadratic_coefficient=0.15)
    start_energy = total_energy(state, parameters)
    exact = critical_quartic_period(1.0, parameters.mass, parameters.quartic_coupling)
    dt = exact / 1600.0

    for _ in range(int(12.0 * exact / dt)):
        step_maintained_clock(state, parameters, controller, dt)

    assert state.compliant_gates >= 8
    assert abs(normalized_detuning(state, parameters)) <= controller.detuning_tolerance
    assert state.audit.amplitude_controller_work > 0.0
    assert state.audit.dissipated_energy > 0.0
    assert state.audit.controller_effort > 0.0
    assert abs(state.audit.balance_error) < 2.0e-11

    end_energy = total_energy(state, parameters)
    reconstructed_change = (
        state.audit.disturbance_work
        + state.audit.signed_controller_work
        - state.audit.dissipated_energy
        + state.audit.numerical_residual
    )
    assert end_energy - start_energy == pytest.approx(reconstructed_change, abs=2.0e-11)


def test_persistent_context_blind_detuning_requires_persistent_control_effort() -> None:
    parameters = QuarticClockParameters(mass=1.0, quartic_coupling=0.5)
    controller = MaintenanceController(
        detuning_gain=0.1,
        amplitude_energy_gain=1.0,
        detuning_tolerance=2.0e-5,
        relative_amplitude_tolerance=2.0e-6,
    )
    state = initialized_turning_point(1.0)
    exact = critical_quartic_period(1.0, parameters.mass, parameters.quartic_coupling)
    dt = exact / 1200.0

    for _ in range(int(8.0 * exact / dt)):
        step_maintained_clock(
            state,
            parameters,
            controller,
            dt,
            disturbance_delta_mu=1.0e-6,
        )

    assert abs(normalized_detuning(state, parameters)) < 1.0e-5
    assert state.audit.controller_effort > 0.0
    assert abs(state.audit.detuning_controller_work) > 0.0
    assert abs(state.audit.balance_error) < 2.0e-11


def test_unregistered_sparse_detuning_pulses_recover_without_context_input() -> None:
    parameters = QuarticClockParameters(mass=1.0, quartic_coupling=0.5, damping=0.001)
    controller = MaintenanceController(
        detuning_gain=0.08,
        amplitude_energy_gain=1.0,
        detuning_tolerance=2.0e-5,
        relative_amplitude_tolerance=3.0e-6,
    )
    state = initialized_turning_point(1.0)
    exact = critical_quartic_period(1.0, parameters.mass, parameters.quartic_coupling)
    dt = exact / 1400.0
    pulse_by_tick = {257: 0.025, 2011: -0.018, 5203: 0.013}

    for tick in range(int(11.0 * exact / dt)):
        step_maintained_clock(
            state,
            parameters,
            controller,
            dt,
            disturbance_delta_mu=pulse_by_tick.get(tick, 0.0),
        )

    assert state.compliant_gates >= 7
    assert abs(normalized_detuning(state, parameters)) <= controller.detuning_tolerance
    assert state.audit.disturbance_work != 0.0
    assert state.audit.controller_effort > 0.0
    assert state.audit.amplitude_controller_work > 0.0
    assert abs(state.audit.balance_error) < 3.0e-11
