"""FTD-0825 contextual-actualization reference model.

Everything in this module is an imposed mathematical witness for the v2
interfaces.  It consumes state--effect weights and therefore does not derive
the Born rule or a physical equilibrium measure from the FTD substrate.

The clock fields with legacy names ``operational_duration``, ``feedback_work``,
and ``dissipated_energy`` are dimensionless interface diagnostics.  They are
not a physical time or energy audit.  The dimensional Hamiltonian reference
model lives in ``maintained_gstar_clock_v1.py``.
"""

from __future__ import annotations

from dataclasses import dataclass
import math
from typing import Iterable, Sequence

G_STAR = 2.9586751191886388923
TAU = 2.0 * math.pi
OUTCOMES = ((-1, -1), (-1, 1), (1, -1), (1, 1))


def normalize_weights(weights: Iterable[float]) -> tuple[float, ...]:
    values = tuple(float(value) for value in weights)
    if not values or any(not math.isfinite(value) or value < 0.0 for value in values):
        raise ValueError("weights must be a nonempty finite nonnegative sequence")
    total = sum(values)
    if not total > 0.0:
        raise ValueError("weights must have positive total")
    return tuple(value / total for value in values)


@dataclass
class SelectorState:
    """Deterministic reference rotor u in [0,1)."""

    u: float

    def __post_init__(self) -> None:
        if not math.isfinite(self.u) or not 0.0 <= self.u < 1.0:
            raise ValueError("selector coordinate must lie in [0,1)")

    def select(self, weights: Sequence[float]) -> int:
        normalized = normalize_weights(weights)
        cumulative = 0.0
        for index, weight in enumerate(normalized):
            cumulative += weight
            if self.u < cumulative or index + 1 == len(normalized):
                return index
        raise AssertionError("normalized quantile partition was incomplete")

    def advance(self) -> None:
        self.u = math.fmod(2.0 * self.u, 1.0)


def singlet_joint_weights(left_angle: float, right_angle: float) -> tuple[float, ...]:
    correlation_axis = math.cos(left_angle - right_angle)
    return tuple(
        0.25 * (1.0 - left * right * correlation_axis)
        for left, right in OUTCOMES
    )


def correlation(weights: Sequence[float]) -> float:
    normalized = normalize_weights(weights)
    if len(normalized) != len(OUTCOMES):
        raise ValueError("Bell reference requires four joint outcome weights")
    return sum(weight * left * right for weight, (left, right) in zip(normalized, OUTCOMES))


def marginals(weights: Sequence[float]) -> tuple[tuple[float, float], tuple[float, float]]:
    normalized = normalize_weights(weights)
    if len(normalized) != len(OUTCOMES):
        raise ValueError("Bell reference requires four joint outcome weights")
    left = [0.0, 0.0]
    right = [0.0, 0.0]
    for weight, (a, b) in zip(normalized, OUTCOMES):
        left[0 if a == -1 else 1] += weight
        right[0 if b == -1 else 1] += weight
    return (tuple(left), tuple(right))


def chsh_value(a0: float, a1: float, b0: float, b1: float) -> float:
    e00 = correlation(singlet_joint_weights(a0, b0))
    e01 = correlation(singlet_joint_weights(a0, b1))
    e10 = correlation(singlet_joint_weights(a1, b0))
    e11 = correlation(singlet_joint_weights(a1, b1))
    return e00 + e01 + e10 - e11


def factorization_residual(weights: Sequence[float]) -> float:
    normalized = normalize_weights(weights)
    left, right = marginals(normalized)
    product = (
        left[0] * right[0],
        left[0] * right[1],
        left[1] * right[0],
        left[1] * right[1],
    )
    return max(abs(observed - independent) for observed, independent in zip(normalized, product))


def critical_quartic_period(amplitude: float, mass: float, coupling: float) -> float:
    if not all(math.isfinite(value) and value > 0.0 for value in (amplitude, mass, coupling)):
        raise ValueError("amplitude, mass, and coupling must be finite and positive")
    return math.sqrt(math.pi) * G_STAR * math.sqrt(mass / (2.0 * coupling)) / amplitude


@dataclass(frozen=True)
class ClockController:
    detuning_gain: float = 0.2
    amplitude_gain: float = 0.15
    detuning_tolerance: float = 1.0e-6
    relative_amplitude_tolerance: float = 1.0e-6
    section_phase: float = 0.0

    def __post_init__(self) -> None:
        if not 0.0 < self.detuning_gain <= 1.0:
            raise ValueError("detuning gain must lie in (0,1]")
        if not 0.0 < self.amplitude_gain <= 1.0:
            raise ValueError("amplitude gain must lie in (0,1]")
        if self.detuning_tolerance < 0.0 or self.relative_amplitude_tolerance < 0.0:
            raise ValueError("clock tolerances must be nonnegative")


@dataclass
class CriticalClockState:
    global_tick: int
    phase: float
    operational_duration: float
    gate_count: int
    amplitude: float
    target_amplitude: float
    detuning: float
    feedback_work: float = 0.0
    dissipated_energy: float = 0.0

    def compliant(self, controller: ClockController) -> bool:
        if self.target_amplitude <= 0.0:
            return False
        relative_error = abs(self.amplitude / self.target_amplitude - 1.0)
        return (
            abs(self.detuning) <= controller.detuning_tolerance
            and relative_error <= controller.relative_amplitude_tolerance
        )


@dataclass(frozen=True)
class ClockStep:
    gate_open: bool
    detuning_correction: float
    amplitude_correction: float


def step_critical_clock(
    state: CriticalClockState,
    phase_increment: float,
    controller: ClockController,
) -> ClockStep:
    """Advance the context-blind, dimensionless clock-interface witness."""

    if not math.isfinite(phase_increment) or phase_increment <= 0.0:
        raise ValueError("phase increment must be finite and positive")
    if state.target_amplitude <= 0.0 or state.amplitude <= 0.0:
        raise ValueError("clock amplitudes must be positive")

    detuning_correction = -controller.detuning_gain * state.detuning
    amplitude_correction = controller.amplitude_gain * (
        state.target_amplitude - state.amplitude
    )
    state.detuning += detuning_correction
    state.amplitude += amplitude_correction
    state.feedback_work += abs(detuning_correction) + abs(amplitude_correction)
    state.dissipated_energy += 0.5 * (
        detuning_correction * detuning_correction
        + amplitude_correction * amplitude_correction
    )

    previous_phase = state.phase
    state.phase += phase_increment
    state.operational_duration += phase_increment
    state.global_tick += 1

    previous_index = math.floor((previous_phase - controller.section_phase) / TAU)
    current_index = math.floor((state.phase - controller.section_phase) / TAU)
    crossed_section = current_index > previous_index
    gate_open = crossed_section and state.compliant(controller)
    if gate_open:
        state.gate_count += current_index - previous_index
    return ClockStep(gate_open, detuning_correction, amplitude_correction)


def midpoint_pushforward(weights: Sequence[float], sample_count: int) -> tuple[float, ...]:
    if sample_count <= 0:
        raise ValueError("sample count must be positive")
    counts = [0] * len(weights)
    for index in range(sample_count):
        selector = SelectorState((index + 0.5) / sample_count)
        counts[selector.select(weights)] += 1
    return tuple(count / sample_count for count in counts)
