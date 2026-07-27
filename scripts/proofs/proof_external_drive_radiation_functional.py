#!/usr/bin/env python3
"""Independent verifier for the FTD-0559 external-drive energy theorem."""

from __future__ import annotations

import cmath
import math


C2 = 1.0 / 3.0
WORK_TOL = 1.0e-12
RESPONSE_TOL = 1.0e-10


def symbol(k: tuple[float, float, float]) -> float:
    cx, cy, cz = (math.cos(value) for value in k)
    return 4.0 - (2.0 / 3.0) * (cx + cy + cz) - (2.0 / 3.0) * (
        cx * cy + cx * cz + cy * cz
    )


def theta(a: float) -> float:
    return 2.0 * math.asin(0.5 * math.sqrt(a))


def energy(state: tuple[complex, complex], a: float) -> float:
    j, w = state
    return abs(w) ** 2 + a * abs(j) ** 2 - a * (j.conjugate() * w).real


def unforced(state: tuple[complex, complex], a: float) -> tuple[complex, complex]:
    j, w = state
    next_w = w - a * j
    return j + next_w, next_w


def forced(state: tuple[complex, complex], a: float, drive: complex) -> tuple[complex, complex]:
    j, w = unforced(state, a)
    return j + drive, w + drive


def work(unforced_end: tuple[complex, complex], forced_end: tuple[complex, complex], a: float, drive: complex) -> float:
    midpoint_j = 0.5 * (unforced_end[0] + forced_end[0])
    midpoint_w = 0.5 * (unforced_end[1] + forced_end[1])
    return (drive.conjugate() * (a * midpoint_j + (2.0 - a) * midpoint_w)).real


def geometric(count: int, angle: float) -> complex:
    return sum(cmath.exp(1j * angle * index) for index in range(1, count + 1))


def closed_response(phase: float, omega: float, ticks: int) -> tuple[complex, complex]:
    denominator = 2j * math.sin(phase)
    response = (geometric(ticks, omega + phase) - geometric(ticks, omega - phase)) / denominator
    prior = 0j if ticks == 1 else (
        geometric(ticks - 1, omega + phase) - geometric(ticks - 1, omega - phase)
    ) / denominator
    common = cmath.exp(-1j * omega * ticks)
    return common * response, common * (response - cmath.exp(1j * omega) * prior)


def main() -> None:
    modes = ((17, 1, -2, 3), (19, -3, 2, 1), (23, 4, 1, -2), (29, -2, -3, 1))
    triples = (
        (0.31 - 0.17j, -0.22 + 0.41j, 0.07 + 0.03j),
        (-0.19 + 0.28j, 0.36 - 0.11j, -0.04 + 0.09j),
        (0.08 + 0.13j, -0.27 - 0.32j, 0.05 - 0.06j),
    )
    worst_work = 0.0
    for size, nx, ny, nz in modes:
        scale = 2.0 * math.pi / size
        a = C2 * symbol((scale * nx, scale * ny, scale * nz))
        for j, w, drive in triples:
            state = (j, w)
            proposal = unforced(state, a)
            endpoint = forced(state, a, drive)
            residual = abs(energy(endpoint, a) - energy(state, a) - work(proposal, endpoint, a, drive))
            worst_work = max(worst_work, residual)
    assert worst_work <= WORK_TOL

    worst_response = 0.0
    worst_ledger = 0.0
    maximum_coefficient_error = 0.0
    arms = 0
    for momentum in ((math.pi, 0.0, 0.0), (math.pi, math.pi, 0.0), (math.pi, math.pi, math.pi)):
        a = C2 * symbol(momentum)
        phase = theta(a)
        for resonant in (True, False):
            omega = phase if resonant else phase + 0.3
            for ticks in (16, 32, 64, 128):
                state = (0j, 0j)
                ledger = 0.0
                for tick in range(ticks):
                    drive = cmath.exp(-1j * omega * tick)
                    proposal = unforced(state, a)
                    endpoint = forced(state, a, drive)
                    ledger += work(proposal, endpoint, a, drive)
                    state = endpoint
                closed = closed_response(phase, omega, ticks)
                worst_response = max(worst_response, abs(state[0] - closed[0]), abs(state[1] - closed[1]))
                worst_ledger = max(worst_ledger, abs(energy(state, a) - ledger))
                if resonant:
                    maximum_coefficient_error = max(
                        maximum_coefficient_error,
                        abs(energy(state, a) / ticks**2 - 0.5),
                    )
                arms += 1
    assert arms == 24
    assert worst_response <= RESPONSE_TOL
    assert worst_ledger <= RESPONSE_TOL

    worst_fejer = 0.0
    points = 4096
    for ticks in (16, 32, 64, 128):
        average = sum(
            abs(geometric(ticks, -math.pi + 2.0 * math.pi * index / points)) ** 2 / ticks
            for index in range(points)
        ) / points
        worst_fejer = max(worst_fejer, abs(average - 1.0))
    assert worst_fejer <= WORK_TOL

    print("FTD-0559 external-drive radiation functional: PASS")
    print(f"maximum_work_identity_residual={worst_work:.17g}")
    print(f"maximum_response_residual={worst_response:.17g}")
    print(f"maximum_cumulative_work_residual={worst_ledger:.17g}")
    print(f"maximum_fejer_residual={worst_fejer:.17g}")
    print(f"maximum_resonant_coefficient_error={maximum_coefficient_error:.17g}")
    print("physical_particle_power_status=OPEN")
    print("verdict=EXTERNAL_DRIVE_RADIATION_FUNCTIONAL_DERIVED")


if __name__ == "__main__":
    main()
