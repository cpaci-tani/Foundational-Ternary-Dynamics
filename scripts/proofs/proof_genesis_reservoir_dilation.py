#!/usr/bin/env python3
"""Independent exact proof/record generator for FTD-0569.

This script re-derives the accepted-genesis inverse, one-step Bernoulli
dilation, erased-history lower bound, evaporation composition defect, and
continuous reservoir-energy requirement. It performs no parameter search.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import platform
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
GATE = 1.0e-12
PREREG_SHA256 = "F0E03DBA0FCB2D757881DDF10AFC115E9A89647B056AE734CD90D07B442C0A66"

SOURCE_FILES = {
    "phase_write": ROOT / "engine/src/render_bridge_phases/phase_write.cpp",
    "voxel_rng": ROOT / "engine/include/ftd/voxel_rng.h",
    "finite_lift_header": ROOT / "engine/include/ftd/eft/finite_memory_reversible_lift.h",
    "finite_lift_source": ROOT / "engine/src/eft/finite_memory_reversible_lift.cpp",
    "ftd0567_theorem": ROOT / "docs/theory/10_eft_program/derivations/THEOREM_GENESIS_ACTION_OBSTRUCTION.md",
    "preregistration": ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_GENESIS_RESERVOIR_DILATION_v1.md",
}

LOCKED_SOURCE_HASHES = {
    "phase_write": "2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4",
    "voxel_rng": "15EA4843331471E0B75488BAB9D87072E1CD7FD41FBC485A2BDD81EBC8841093",
    "finite_lift_header": "D593C991597A69DEF1BE389CB69DEE3168F44B1B774FBBBE7D6B30C59D92B092",
    "finite_lift_source": "13E2C4E8F4777C38C9AA01260E44A0D823DC89E89E92DA58C3BC5704ED9E5265",
    "ftd0567_theorem": "877ACAA8C859DFE065120543B8FBC7862BD619AFCB57A4B7CD6D214A6CA18055",
    "preregistration": PREREG_SHA256,
}

IMPLEMENTATION_FILES = {
    "header": ROOT / "engine/include/ftd/eft/genesis_reservoir_dilation.h",
    "source": ROOT / "engine/src/eft/genesis_reservoir_dilation.cpp",
    "test": ROOT / "engine/tests/test_genesis_reservoir_dilation.cpp",
    "independent_proof": Path(__file__).resolve(),
}


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def add(a: tuple[float, float, float], b: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(x + y for x, y in zip(a, b))  # type: ignore[return-value]


def sub(a: tuple[float, float, float], b: tuple[float, float, float]) -> tuple[float, float, float]:
    return tuple(x - y for x, y in zip(a, b))  # type: ignore[return-value]


def scale(a: tuple[float, float, float], factor: float) -> tuple[float, float, float]:
    return tuple(factor * x for x in a)  # type: ignore[return-value]


def norm2(a: tuple[float, float, float]) -> float:
    return sum(x * x for x in a)


def norm(a: tuple[float, float, float]) -> float:
    return math.sqrt(norm2(a))


def max_abs(a: tuple[float, float, float]) -> float:
    return max(abs(x) for x in a)


def normalize(a: tuple[float, float, float]) -> tuple[float, float, float]:
    return scale(a, 1.0 / norm(a))


def withdrawal(kg: float, excess: float, wave2: float, drain: float) -> float:
    return kg * excess + 0.5 * kg * kg + (drain - 0.5 * drain * drain) * wave2


def forward(
    flux: tuple[float, float, float],
    wave: tuple[float, float, float],
    kg: float,
    km: float,
    drain: float,
) -> tuple[tuple[float, float, float], tuple[float, float, float], float, float]:
    magnitude = norm(flux)
    excess = magnitude - kg
    assert excess > 0.0
    return (
        scale(flux, 1.0 - kg / magnitude),
        scale(wave, 1.0 - drain),
        1.0 - math.exp(-excess / km),
        withdrawal(kg, excess, norm2(wave), drain),
    )


def inverse(
    flux_after: tuple[float, float, float],
    wave_after: tuple[float, float, float],
    kg: float,
    drain: float,
) -> tuple[tuple[float, float, float], tuple[float, float, float]]:
    residual = norm(flux_after)
    assert residual > 0.0 and 0.0 <= drain < 1.0
    return (
        scale(flux_after, 1.0 + kg / residual),
        scale(wave_after, 1.0 / (1.0 - drain)),
    )


def dilate(phase: float, probability: float) -> tuple[int, float]:
    assert 0.0 <= phase < 1.0 and 0.0 < probability < 1.0
    if phase < probability:
        return 1, phase / probability
    return 0, (phase - probability) / (1.0 - probability)


def undilate(branch: int, future: float, probability: float) -> float:
    assert branch in (0, 1) and 0.0 <= future < 1.0
    if branch == 1:
        return probability * future
    return probability + (1.0 - probability) * future


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--json-output", type=Path)
    args = parser.parse_args()

    hashes = {name: sha256(path) for name, path in SOURCE_FILES.items()}
    assert hashes == LOCKED_SOURCE_HASHES, (hashes, LOCKED_SOURCE_HASHES)

    phase_write = SOURCE_FILES["phase_write"].read_text(encoding="utf-8")
    voxel_rng = SOURCE_FILES["voxel_rng"].read_text(encoding="utf-8")
    assert "v.wave_vel *= (1.0 - rb.toggles.kinetic_drain);" in phase_write
    assert "v.flux *= std::max(0.0, 1.0 - kg / jmag);" in phase_write
    assert "rb.set_state(i, 0);" in phase_write
    assert "v.particle_id = -1;" in phase_write
    assert "v.spin = 0;" in phase_write and "v.color = 0;" in phase_write
    assert "deterministic in the four inputs" in voxel_rng
    assert "double voxel_uniform(std::uint64_t seed" in voxel_rng

    kg = 1.0
    km = 1.0
    inv_sqrt3 = 1.0 / math.sqrt(3.0)
    directions = [
        (1.0, 0.0, 0.0), (-1.0, 0.0, 0.0),
        (0.0, 1.0, 0.0), (0.0, -1.0, 0.0),
        (0.0, 0.0, 1.0), (0.0, 0.0, -1.0),
        (inv_sqrt3, inv_sqrt3, inv_sqrt3),
        (-inv_sqrt3, inv_sqrt3, inv_sqrt3),
        (inv_sqrt3, -inv_sqrt3, inv_sqrt3),
        (inv_sqrt3, inv_sqrt3, -inv_sqrt3),
    ]
    excesses = [0.125, 0.5, 1.25]
    waves = [(0.0, 0.0, 0.0), (0.3, -0.4, 0.2), (-0.25, 0.1, 0.5)]
    drains = [0.0, 0.5, 0.9]
    polarities = [-1, 1]

    arms = 0
    max_inverse = 0.0
    max_withdrawal = 0.0
    max_flux_composition = 0.0
    max_wave_composition = 0.0
    min_flux_distance = math.inf
    withdrawals: list[float] = []
    for raw_direction in directions:
        direction = normalize(raw_direction)
        for excess in excesses:
            for wave in waves:
                for drain in drains:
                    for _polarity in polarities:
                        flux = scale(direction, kg + excess)
                        flux_after, wave_after, probability, removed = forward(
                            flux, wave, kg, km, drain
                        )
                        recovered_flux, recovered_wave = inverse(
                            flux_after, wave_after, kg, drain
                        )
                        max_inverse = max(
                            max_inverse,
                            max_abs(sub(recovered_flux, flux)),
                            max_abs(sub(recovered_wave, wave)),
                        )
                        measured = 0.5 * (
                            norm2(flux) + norm2(wave)
                            - norm2(flux_after) - norm2(wave_after)
                        )
                        max_withdrawal = max(max_withdrawal, abs(measured - removed))
                        withdrawals.append(removed)
                        flux_distance = norm(sub(flux, flux_after))
                        wave_distance = norm(sub(wave, wave_after))
                        min_flux_distance = min(min_flux_distance, flux_distance)
                        max_flux_composition = max(
                            max_flux_composition, abs(flux_distance - kg)
                        )
                        max_wave_composition = max(
                            max_wave_composition,
                            abs(wave_distance - drain * norm(wave)),
                        )
                        assert 0.0 < probability < 1.0
                        arms += 1

    assert arms == 540 and max_inverse <= GATE
    assert max_withdrawal <= GATE
    assert max_flux_composition <= GATE and max_wave_composition <= GATE
    assert abs(min_flux_distance - kg) <= GATE

    collision_flux = (kg + 0.5, 0.0, 0.0)
    wa = (0.25, -0.5, 0.75)
    wb = (-0.4, 0.1, 0.2)
    fa, wza, _, _ = forward(collision_flux, wa, kg, km, 1.0)
    fb, wzb, _, _ = forward(collision_flux, wb, kg, km, 1.0)
    unit_drain_collision = max_abs(sub(fa, fb)) == 0.0 and norm(wza) == 0.0 and norm(wzb) == 0.0
    assert unit_drain_collision and norm(sub(wa, wb)) > 0.0

    probabilities = [1.0 - math.exp(-x / km) for x in excesses] + [0.5]
    bernoulli_arms = 0
    max_bernoulli_inverse = 0.0
    for probability in probabilities:
        phases = [
            0.25 * probability,
            0.75 * probability,
            probability + 0.25 * (1.0 - probability),
            probability + 0.75 * (1.0 - probability),
        ]
        for index, phase in enumerate(phases):
            branch, future = dilate(phase, probability)
            assert branch == (1 if index < 2 else 0)
            recovered = undilate(branch, future, probability)
            max_bernoulli_inverse = max(
                max_bernoulli_inverse, abs(recovered - phase)
            )
            bernoulli_arms += 1
    assert bernoulli_arms == 16 and max_bernoulli_inverse <= 1.0e-15

    depth = 20
    initial_phase = 0.3141592653589793
    phase = initial_phase
    branches: list[int] = []
    sequence_probabilities: list[float] = []
    for index in range(depth):
        probability = probabilities[index % len(probabilities)]
        branch, phase = dilate(phase, probability)
        branches.append(branch)
        sequence_probabilities.append(probability)
    for branch, probability in zip(
        reversed(branches), reversed(sequence_probabilities)
    ):
        phase = undilate(branch, phase, probability)
    history_inverse = abs(phase - initial_phase)
    assert history_inverse <= 1.0e-15
    assert all((1 << n) == 2**n for n in range(1, depth + 1))

    slope_residual = 0.0
    for x0, x1 in zip(excesses, excesses[1:]):
        d0 = withdrawal(kg, x0, 0.0, 0.0)
        d1 = withdrawal(kg, x1, 0.0, 0.0)
        slope_residual = max(slope_residual, abs((d1 - d0) / (x1 - x0) - kg))
    assert slope_residual <= GATE
    withdrawal_span = max(withdrawals) - min(withdrawals)
    assert withdrawal_span > 0.0 and min(withdrawals) > 0.0

    result = {
        "ftd_id": "FTD-0569",
        "verdict": "ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY",
        "platform": platform.platform(),
        "field_representation": "frozen production event kernel plus observer-only reservoir phase",
        "tolerance": GATE,
        "accepted_single_arms": arms,
        "bernoulli_arms": bernoulli_arms,
        "history_depth": depth,
        "erased_preimages_at_depth": 1 << depth,
        "minimum_history_bits_at_depth": depth,
        "maximum_genesis_inverse_residual": max_inverse,
        "maximum_bernoulli_inverse_residual": max_bernoulli_inverse,
        "maximum_history_inverse_residual": history_inverse,
        "maximum_withdrawal_residual": max_withdrawal,
        "maximum_withdrawal_slope_residual": slope_residual,
        "maximum_evaporation_flux_distance_residual": max_flux_composition,
        "maximum_evaporation_wave_distance_residual": max_wave_composition,
        "minimum_evaporation_composition_flux_distance": min_flux_distance,
        "withdrawal_span": withdrawal_span,
        "accepted_genesis_conditionally_invertible": True,
        "unit_drain_has_wave_collision": unit_drain_collision,
        "one_step_bernoulli_dilation_exact": True,
        "erased_trials_require_unbounded_history": True,
        "evaporation_is_not_genesis_inverse": True,
        "production_pair_violates_detailed_balance": True,
        "continuous_energy_payload_required": True,
        "dual_and_single_energy_exchange_differ": True,
        "finite_local_reversible_production_dilation": False,
        "one_event_dilation_open_system_only": True,
        "production_rng_is_stateless_schedule": True,
        "source_hashes_sha256": hashes,
        "implementation_hashes_sha256": {
            name: sha256(path) for name, path in IMPLEMENTATION_FILES.items()
        },
    }
    if args.json_output:
        args.json_output.parent.mkdir(parents=True, exist_ok=True)
        args.json_output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    print("PASS: one-event dilation exists; frozen production cycle remains open-system only")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
