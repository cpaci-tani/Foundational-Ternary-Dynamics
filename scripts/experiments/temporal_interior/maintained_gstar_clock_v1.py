"""Maintained critical-quartic clock reference model.

This module is an imposed mathematical/physical witness, not a clock derived
from FTD P1--P5.  Unlike the lightweight contextual-actualization interface,
it evolves a dimensional oscillator

    H = p^2/(2m) + mu q^2/2 + lambda q^4

and keeps a term-by-term energy audit.  The detuning controller changes the
quadratic coefficient ``mu``; the amplitude controller restores the target
energy at the positive-going ``q=0`` Poincare section.  Neither controller
accepts a measurement context or outcome.

At ``mu=0`` the exact period is

    T A = sqrt(pi) G* sqrt(m/(2 lambda)).

The model is deliberately isolated under ``scripts/experiments``.  It tests
the coherence and energetic price of maintaining a G*-clock but supplies no
substrate carrier, Born rule, or production-engine integration.
"""

from __future__ import annotations

from dataclasses import dataclass, field
import math


G_STAR = 2.9586751191886388923


@dataclass(frozen=True)
class QuarticClockParameters:
    mass: float
    quartic_coupling: float
    damping: float = 0.0

    def __post_init__(self) -> None:
        if not math.isfinite(self.mass) or self.mass <= 0.0:
            raise ValueError("mass must be finite and positive")
        if not math.isfinite(self.quartic_coupling) or self.quartic_coupling <= 0.0:
            raise ValueError("quartic coupling must be finite and positive")
        if not math.isfinite(self.damping) or self.damping < 0.0:
            raise ValueError("damping must be finite and nonnegative")


@dataclass(frozen=True)
class MaintenanceController:
    detuning_gain: float = 0.05
    amplitude_energy_gain: float = 1.0
    detuning_tolerance: float = 1.0e-6
    relative_amplitude_tolerance: float = 1.0e-6

    def __post_init__(self) -> None:
        if not 0.0 <= self.detuning_gain <= 1.0:
            raise ValueError("detuning gain must lie in [0,1]")
        if not 0.0 <= self.amplitude_energy_gain <= 1.0:
            raise ValueError("amplitude energy gain must lie in [0,1]")
        if self.detuning_tolerance < 0.0:
            raise ValueError("detuning tolerance must be nonnegative")
        if self.relative_amplitude_tolerance < 0.0:
            raise ValueError("amplitude tolerance must be nonnegative")


@dataclass
class EnergyAudit:
    disturbance_work: float = 0.0
    detuning_controller_work: float = 0.0
    amplitude_controller_work: float = 0.0
    controller_effort: float = 0.0
    dissipated_energy: float = 0.0
    numerical_residual: float = 0.0
    balance_error: float = 0.0

    @property
    def signed_controller_work(self) -> float:
        return self.detuning_controller_work + self.amplitude_controller_work


@dataclass
class MaintainedClockState:
    q: float
    p: float
    quadratic_coefficient: float
    target_amplitude: float
    global_tick: int = 0
    global_time: float = 0.0
    section_crossings: int = 0
    compliant_gates: int = 0
    last_crossing_time: float | None = None
    last_period: float | None = None
    audit: EnergyAudit = field(default_factory=EnergyAudit)

    def __post_init__(self) -> None:
        if not all(
            math.isfinite(value)
            for value in (self.q, self.p, self.quadratic_coefficient, self.target_amplitude)
        ):
            raise ValueError("clock state must be finite")
        if self.target_amplitude <= 0.0:
            raise ValueError("target amplitude must be positive")


@dataclass(frozen=True)
class MaintainedClockStep:
    crossed_section: bool
    gate_open: bool
    crossing_time: float | None
    measured_period: float | None
    normalized_detuning: float
    estimated_amplitude: float
    disturbance_work: float
    detuning_controller_work: float
    amplitude_controller_work: float
    dissipated_energy: float
    numerical_residual: float
    balance_error: float


def critical_quartic_period(
    amplitude: float,
    mass: float,
    quartic_coupling: float,
) -> float:
    """Exact period for ``p^2/(2m) + lambda q^4`` at amplitude ``A``."""

    if not all(
        math.isfinite(value) and value > 0.0
        for value in (amplitude, mass, quartic_coupling)
    ):
        raise ValueError("amplitude, mass, and quartic coupling must be positive")
    return (
        math.sqrt(math.pi)
        * G_STAR
        * math.sqrt(mass / (2.0 * quartic_coupling))
        / amplitude
    )


def harmonic_period(mass: float, stiffness: float) -> float:
    if not all(math.isfinite(value) and value > 0.0 for value in (mass, stiffness)):
        raise ValueError("mass and stiffness must be positive")
    return 2.0 * math.pi * math.sqrt(mass / stiffness)


def potential_energy(q: float, quadratic_coefficient: float, quartic_coupling: float) -> float:
    return 0.5 * quadratic_coefficient * q * q + quartic_coupling * q**4


def total_energy(state: MaintainedClockState, parameters: QuarticClockParameters) -> float:
    return (
        state.p * state.p / (2.0 * parameters.mass)
        + potential_energy(
            state.q,
            state.quadratic_coefficient,
            parameters.quartic_coupling,
        )
    )


def target_energy(state: MaintainedClockState, parameters: QuarticClockParameters) -> float:
    return potential_energy(
        state.target_amplitude,
        state.quadratic_coefficient,
        parameters.quartic_coupling,
    )


def amplitude_from_energy(
    energy: float,
    quadratic_coefficient: float,
    quartic_coupling: float,
) -> float:
    """Return the positive turning amplitude for the symmetric orbit."""

    if not math.isfinite(energy) or energy < 0.0:
        raise ValueError("energy must be finite and nonnegative")
    discriminant = 0.25 * quadratic_coefficient**2 + 4.0 * quartic_coupling * energy
    amplitude_squared = (
        -0.5 * quadratic_coefficient + math.sqrt(discriminant)
    ) / (2.0 * quartic_coupling)
    if amplitude_squared < 0.0:
        raise ValueError("energy does not define a positive symmetric-orbit amplitude")
    return math.sqrt(amplitude_squared)


def normalized_detuning(
    state: MaintainedClockState,
    parameters: QuarticClockParameters,
) -> float:
    """Dimensionless distance from the critical point.

    With ``delta = mu/(2 lambda A_target^2)``, the exact crossover modulus
    is ``k^2 = 1/(delta + 2)``.  The critical quartic point is ``delta=0``.
    """

    return state.quadratic_coefficient / (
        2.0 * parameters.quartic_coupling * state.target_amplitude**2
    )


def estimated_amplitude(
    state: MaintainedClockState,
    parameters: QuarticClockParameters,
) -> float:
    return amplitude_from_energy(
        total_energy(state, parameters),
        state.quadratic_coefficient,
        parameters.quartic_coupling,
    )


def is_compliant(
    state: MaintainedClockState,
    parameters: QuarticClockParameters,
    controller: MaintenanceController,
) -> bool:
    amplitude = estimated_amplitude(state, parameters)
    relative_error = abs(amplitude / state.target_amplitude - 1.0)
    return (
        abs(normalized_detuning(state, parameters)) <= controller.detuning_tolerance
        and relative_error <= controller.relative_amplitude_tolerance
    )


def _force(q: float, quadratic_coefficient: float, quartic_coupling: float) -> float:
    return -quadratic_coefficient * q - 4.0 * quartic_coupling * q**3


def step_maintained_clock(
    state: MaintainedClockState,
    parameters: QuarticClockParameters,
    controller: MaintenanceController,
    dt: float,
    disturbance_delta_mu: float = 0.0,
) -> MaintainedClockStep:
    """Advance one context-blind global step and close the energy ledger.

    ``disturbance_delta_mu`` is an externally supplied change of the plant's
    quadratic coefficient.  Parameter work is exact at fixed ``q``:
    ``dW = q^2 dmu/2``.  Conservative motion uses velocity Verlet; damping is
    an exact exponential momentum split; amplitude restoration is an energy
    kick at the positive-going ``q=0`` section.
    """

    if not math.isfinite(dt) or dt <= 0.0:
        raise ValueError("dt must be finite and positive")
    if not math.isfinite(disturbance_delta_mu):
        raise ValueError("disturbance must be finite")

    start_energy = total_energy(state, parameters)
    old_q = state.q
    old_time = state.global_time

    disturbance_work = 0.5 * state.q**2 * disturbance_delta_mu
    state.quadratic_coefficient += disturbance_delta_mu

    detuning_delta_mu = -controller.detuning_gain * state.quadratic_coefficient
    detuning_work = 0.5 * state.q**2 * detuning_delta_mu
    state.quadratic_coefficient += detuning_delta_mu
    state.audit.controller_effort += abs(detuning_work)

    before_conservative = total_energy(state, parameters)
    first_force = _force(
        state.q,
        state.quadratic_coefficient,
        parameters.quartic_coupling,
    )
    half_p = state.p + 0.5 * dt * first_force
    state.q += dt * half_p / parameters.mass
    second_force = _force(
        state.q,
        state.quadratic_coefficient,
        parameters.quartic_coupling,
    )
    state.p = half_p + 0.5 * dt * second_force
    after_conservative = total_energy(state, parameters)
    numerical_residual = after_conservative - before_conservative

    kinetic_before_damping = state.p**2 / (2.0 * parameters.mass)
    if parameters.damping > 0.0:
        state.p *= math.exp(-parameters.damping * dt / parameters.mass)
    kinetic_after_damping = state.p**2 / (2.0 * parameters.mass)
    dissipated_energy = kinetic_before_damping - kinetic_after_damping

    crossed_section = old_q < 0.0 <= state.q and state.p > 0.0
    crossing_time: float | None = None
    measured_period: float | None = None
    amplitude_work = 0.0
    if crossed_section:
        fraction = -old_q / (state.q - old_q) if state.q != old_q else 1.0
        crossing_time = old_time + fraction * dt
        state.section_crossings += 1
        if state.last_crossing_time is not None:
            measured_period = crossing_time - state.last_crossing_time
            state.last_period = measured_period
        state.last_crossing_time = crossing_time

        before_amplitude_control = total_energy(state, parameters)
        desired_energy = target_energy(state, parameters)
        corrected_energy = before_amplitude_control + controller.amplitude_energy_gain * (
            desired_energy - before_amplitude_control
        )
        local_potential = potential_energy(
            state.q,
            state.quadratic_coefficient,
            parameters.quartic_coupling,
        )
        if corrected_energy < local_potential:
            corrected_energy = local_potential
        state.p = math.sqrt(
            max(0.0, 2.0 * parameters.mass * (corrected_energy - local_potential))
        )
        amplitude_work = total_energy(state, parameters) - before_amplitude_control
        state.audit.controller_effort += abs(amplitude_work)

    state.global_tick += 1
    state.global_time += dt

    gate_open = crossed_section and is_compliant(state, parameters, controller)
    if gate_open:
        state.compliant_gates += 1

    end_energy = total_energy(state, parameters)
    accounted_change = (
        disturbance_work
        + detuning_work
        + amplitude_work
        - dissipated_energy
        + numerical_residual
    )
    balance_error = (end_energy - start_energy) - accounted_change

    state.audit.disturbance_work += disturbance_work
    state.audit.detuning_controller_work += detuning_work
    state.audit.amplitude_controller_work += amplitude_work
    state.audit.dissipated_energy += dissipated_energy
    state.audit.numerical_residual += numerical_residual
    state.audit.balance_error += balance_error

    return MaintainedClockStep(
        crossed_section=crossed_section,
        gate_open=gate_open,
        crossing_time=crossing_time,
        measured_period=measured_period,
        normalized_detuning=normalized_detuning(state, parameters),
        estimated_amplitude=estimated_amplitude(state, parameters),
        disturbance_work=disturbance_work,
        detuning_controller_work=detuning_work,
        amplitude_controller_work=amplitude_work,
        dissipated_energy=dissipated_energy,
        numerical_residual=numerical_residual,
        balance_error=balance_error,
    )


def initialized_turning_point(
    amplitude: float,
    quadratic_coefficient: float = 0.0,
) -> MaintainedClockState:
    return MaintainedClockState(
        q=amplitude,
        p=0.0,
        quadratic_coefficient=quadratic_coefficient,
        target_amplitude=amplitude,
    )

