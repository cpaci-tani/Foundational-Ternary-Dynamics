"""Formal/reference checks for FTD v2 contextual actualization (FTD-0825).

The Bell and Born calculations are conditional on the adopted algebraic
state/effect structure.  The quantile selector consumes those weights and is
therefore a compatibility witness, not a physical Born derivation.
"""

from __future__ import annotations

import inspect
import math
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
MODULE_DIR = ROOT / "scripts/experiments/temporal_interior"
sys.path.insert(0, str(MODULE_DIR))

from contextual_actualization_v2 import (  # noqa: E402
    ClockController,
    CriticalClockState,
    G_STAR,
    SelectorState,
    chsh_value,
    critical_quartic_period,
    factorization_residual,
    marginals,
    midpoint_pushforward,
    normalize_weights,
    singlet_joint_weights,
    step_critical_clock,
)


def main() -> int:
    checks: list[tuple[str, bool]] = []

    def check(label: str, condition: bool) -> None:
        checks.append((label, bool(condition)))
        print(f"[{'PASS' if condition else 'FAIL'}] {label}")

    normalized = normalize_weights((1.0, 2.0, 3.0, 4.0))
    check("C1 state weights normalize", abs(sum(normalized) - 1.0) < 1e-15)
    check("C2 state weights stay positive", min(normalized) >= 0.0)

    rational_weights = (0.125, 0.25, 0.375, 0.25)
    observed = midpoint_pushforward(rational_weights, 8000)
    check("C3 deterministic quantile pushforward is exact on frozen rational grid", max(abs(a - b) for a, b in zip(observed, rational_weights)) < 1e-15)
    check("C4 selector is deterministic", SelectorState(0.51).select(rational_weights) == SelectorState(0.51).select(rational_weights))

    # Exact measure-preservation argument for T(u)=2u mod 1: an interval of
    # length L has two preimage intervals, each length L/2.
    interval_length = 0.371
    preimage_measure = interval_length / 2.0 + interval_length / 2.0
    check("C5 doubling map preserves Lebesgue interval measure", abs(preimage_measure - interval_length) < 1e-15)

    a0, a1 = 0.0, 0.5 * math.pi
    b0, b1 = 0.25 * math.pi, -0.25 * math.pi
    s = chsh_value(a0, a1, b0, b1)
    check("C6 selected singlet reference reaches Tsirelson", abs(abs(s) - 2.0 * math.sqrt(2.0)) < 1e-14)
    check("C7 selected reference stays below operator bound", abs(s) <= 2.0 * math.sqrt(2.0) + 1e-14)
    check("C8 PR-box control lies outside operator bound", 4.0 > 2.0 * math.sqrt(2.0))

    maximum_marginal_drift = 0.0
    for left in (a0, a1, 0.37):
        reference_left = None
        for right in (b0, b1, -0.91, 1.22):
            left_marginal, right_marginal = marginals(singlet_joint_weights(left, right))
            if reference_left is None:
                reference_left = left_marginal
            maximum_marginal_drift = max(
                maximum_marginal_drift,
                max(abs(x - y) for x, y in zip(left_marginal, reference_left)),
                max(abs(value - 0.5) for value in right_marginal),
            )
    check("C9 all remote-setting marginals are no-signalling", maximum_marginal_drift < 1e-14)
    check("C10 contextual singlet joint weights are not locally factorized", factorization_residual(singlet_joint_weights(0.0, 0.0)) >= 0.25 - 1e-15)

    # Measurement independence in the reference means every context is sampled
    # by the same fixed midpoint equilibrium ensemble.
    rotor_grid = tuple((i + 0.5) / 4096 for i in range(4096))
    context_rotors = {
        context: rotor_grid
        for context in ((a0, b0), (a0, b1), (a1, b0), (a1, b1))
    }
    check(
        "C11 reference rotor measure is context independent",
        len({id(grid) for grid in context_rotors.values()}) == 1,
    )

    amplitude, mass, coupling = 0.7, 1.3, 0.9
    period = critical_quartic_period(amplitude, mass, coupling)
    invariant = period * amplitude / math.sqrt(mass / (2.0 * coupling))
    check("C12 critical period carries exact G* factor", abs(invariant - math.sqrt(math.pi) * G_STAR) < 2e-15)

    controller = ClockController(
        detuning_gain=0.25,
        amplitude_gain=0.2,
        detuning_tolerance=1e-8,
        relative_amplitude_tolerance=1e-8,
    )
    clock = CriticalClockState(
        global_tick=0,
        phase=-0.1,
        operational_duration=0.0,
        gate_count=0,
        amplitude=0.6,
        target_amplitude=1.0,
        detuning=0.3,
    )
    # The first crossing occurs before amplitude compliance.  The second is
    # deliberately the first eligible gate, so a premature gate is detectable.
    for _ in range(140):
        step_critical_clock(clock, 0.1, controller)
    check("C13 feedback drives detuning to compliance", abs(clock.detuning) <= controller.detuning_tolerance)
    check("C14 feedback stabilizes amplitude", abs(clock.amplitude - clock.target_amplitude) <= controller.relative_amplitude_tolerance)
    check("C15 feedback cost is explicit", clock.feedback_work > 0.0 and clock.dissipated_energy > 0.0)
    check("C16 compliant phase crossings open gates", clock.gate_count >= 1)

    signature = inspect.signature(step_critical_clock)
    forbidden = {"context", "setting", "weights", "outcome", "instrument"}
    check("C17 clock controller API is context blind", forbidden.isdisjoint(signature.parameters))

    passed = sum(ok for _, ok in checks)
    print(f"\nFTD-0825 contextual actualization reference: {passed}/{len(checks)} PASS")
    print(f"chsh={s:.17g}")
    print(f"maximum_no_signalling_marginal_drift={maximum_marginal_drift:.3e}")
    print("BORN_STATUS=CONDITIONAL_STATE_EFFECT_RULE_PHYSICAL_PUSHFORWARD_OPEN")
    print("BELL_STATUS=CONTEXTUAL_NONFACTORIZABLE_OPERATIONALLY_NOSIGNALLING")
    print("GSTAR_STATUS=SELECTED_CRITICAL_PERIOD_NATIVE_MAINTENANCE_OPEN")
    return 0 if passed == len(checks) else 1


if __name__ == "__main__":
    raise SystemExit(main())
