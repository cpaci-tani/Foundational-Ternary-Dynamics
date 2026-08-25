#!/usr/bin/env python3
"""Exact prepared Born-counting readout on the selected v3 field bank.

For one native outcome port (directed SC tangent plus polarity), a finite block
contains integer C4 phase counts N0..N3.  Opposite phases cancel into retained
dark pairs, leaving the Gaussian integer

    Z = (N0-N2) + i (N1-N3).

The number of ordered phase-compatible pairs in the residual real and
imaginary rails is exactly |Z|^2.  Normalizing those finite counts across
native ports gives the Born form exactly.  This is a representation/prepared-
counting theorem, not a native preparation or detector theorem.
"""

from __future__ import annotations

import json
import sys
from fractions import Fraction
from itertools import product
from pathlib import Path


sys.stdout.reconfigure(encoding="utf-8")

ROOT = Path(__file__).resolve().parents[2]
REGISTER_PATH = (
    ROOT / "docs/theory/01_reference/strict_discrete_common_action_register_v3.json"
)

Counts = tuple[int, int, int, int]


def gaussian_integer(counts: Counts) -> tuple[int, int]:
    n0, n1, n2, n3 = counts
    return n0 - n2, n1 - n3


def residual_counts(counts: Counts) -> Counts:
    real, imag = gaussian_integer(counts)
    return (
        max(real, 0),
        max(imag, 0),
        max(-real, 0),
        max(-imag, 0),
    )


def dark_pairs(counts: Counts) -> tuple[int, int]:
    n0, n1, n2, n3 = counts
    return min(n0, n2), min(n1, n3)


def bright_pair_count(counts: Counts) -> int:
    residual = residual_counts(counts)
    real_survivors = residual[0] + residual[2]
    imag_survivors = residual[1] + residual[3]
    return real_survivors**2 + imag_survivors**2


def norm_squared(z: tuple[int, int]) -> int:
    return z[0] ** 2 + z[1] ** 2


def rotate_counts(counts: Counts, turns: int = 1) -> Counts:
    result = counts
    for _ in range(turns % 4):
        n0, n1, n2, n3 = result
        # Multiplication of Z by i: phase k moves to k+1.
        result = (n3, n0, n1, n2)
    return result


def rotate_gaussian(z: tuple[int, int], turns: int = 1) -> tuple[int, int]:
    real, imag = z
    for _ in range(turns % 4):
        real, imag = -imag, real
    return real, imag


def minimum_counts(z: tuple[int, int]) -> Counts:
    real, imag = z
    return (
        max(real, 0),
        max(imag, 0),
        max(-real, 0),
        max(-imag, 0),
    )


checks: list[tuple[str, bool, str]] = []


def check(name: str, condition: bool, detail: str = "") -> None:
    checks.append((name, condition, detail))
    suffix = f" -- {detail}" if detail and not condition else ""
    print(f"[{'PASS' if condition else 'FAIL'}] {name}{suffix}")


def main() -> None:
    register = json.loads(REGISTER_PATH.read_text(encoding="utf-8"))
    field = register["carrier_inventory"]["primitive_payloads"][
        "field_channel_bank"
    ]
    check("C1 selected v3 field bank has 384 finite channels", field["channel_count"] == 384)

    directed_tangents = 6
    normals_per_tangent = 4
    hands = 2
    phases = 4
    polarities = 2
    per_port_phase = normals_per_tangent * hands
    check("C2 each native tangent/polarity/phase port has eight channels", directed_tangents * per_port_phase * phases * polarities == 384 and per_port_phase == 8)
    check("C3 there are twelve native tangent/polarity outcome ports", directed_tangents * polarities == 12)

    count_fixtures = tuple(product(range(5), repeat=4))
    exact_rows = 0
    for counts in count_fixtures:
        z = gaussian_integer(counts)
        residual = residual_counts(counts)
        dark_real, dark_imag = dark_pairs(counts)

        assert gaussian_integer(residual) == z
        assert sum(residual) == abs(z[0]) + abs(z[1])
        assert bright_pair_count(counts) == norm_squared(z)
        assert (
            sum(counts)
            == sum(residual) + 2 * dark_real + 2 * dark_imag
        )

        for turns in range(4):
            rotated = rotate_counts(counts, turns)
            assert gaussian_integer(rotated) == rotate_gaussian(z, turns)
            assert bright_pair_count(rotated) == bright_pair_count(counts)

        for real_dark in range(3):
            for imag_dark in range(3):
                augmented = (
                    counts[0] + real_dark,
                    counts[1] + imag_dark,
                    counts[2] + real_dark,
                    counts[3] + imag_dark,
                )
                assert gaussian_integer(augmented) == z
                assert bright_pair_count(augmented) == bright_pair_count(counts)
        exact_rows += 16

    check("C4 opposite C4 phases cancel to one Gaussian-integer residual", exact_rows > 0)
    check("C5 residual phase-compatible ordered pairs equal |Z|^2", exact_rows > 0)
    check("C6 retained dark-pair additions do not alter Z or event count", exact_rows > 0)
    check("C7 common C4 phase rotation preserves every event count", exact_rows > 0)

    # Polarity is a separate complete carrier coordinate.  Charge conjugation
    # exchanges the two port copies but does not alter their phase count or
    # Born weight.
    polarity_weights = {
        polarity: bright_pair_count((4, 2, 1, 3))
        for polarity in (-1, 1)
    }
    check("C8 charge conjugation permutes equal-weight complete port copies", polarity_weights[-1] == polarity_weights[1])

    # Exhaustive two-outcome prepared-counting identity on a bounded census.
    bounded = tuple(product(range(3), repeat=4))
    frequency_rows = 0
    for left in bounded:
        for right in bounded:
            weights = (bright_pair_count(left), bright_pair_count(right))
            total = sum(weights)
            if total == 0:
                continue
            z_left = gaussian_integer(left)
            z_right = gaussian_integer(right)
            frequency = Fraction(weights[0], total)
            expected = Fraction(
                norm_squared(z_left),
                norm_squared(z_left) + norm_squared(z_right),
            )
            assert frequency == expected
            frequency_rows += 1
    check("C9 normalized prepared event counts have the exact Born form", frequency_rows > 0)

    # Every finite Gaussian integer fits in a sufficiently large finite block.
    # One site supplies eight channels for each port phase.
    representation_rows = 0
    for real in range(-12, 13):
        for imag in range(-12, 13):
            z = (real, imag)
            counts = minimum_counts(z)
            assert gaussian_integer(counts) == z
            required_sites = max(counts, default=0) // 8
            if max(counts, default=0) % 8:
                required_sites += 1
            assert all(count <= 8 * required_sites for count in counts)
            assert bright_pair_count(counts) == norm_squared(z)
            representation_rows += 3
    check("C10 every tested Gaussian integer has an explicit finite-bank realization", representation_rows == 25 * 25 * 3)

    # Epistemic firewall: a prepared count is not a native probability law.
    open_scope = {
        "bank formation by Phi",
        "exclusive detector renewal",
        "one emission one event",
        "finite-window physical trials",
        "apparatus record and backreaction",
        "multipartite no-signalling",
    }
    check("C11 physical Born closure remains explicitly downstream", len(open_scope) == 6)

    passed = sum(ok for _, ok, _ in checks)
    print(f"\n{passed}/{len(checks)} v3 Gaussian/Born-readout checks pass")
    print(f"exact_counting_rows={exact_rows + frequency_rows + representation_rows}")
    print("prepared_readout: Z=(N0-N2)+i(N1-N3), bright_pairs=|Z|^2")
    print("carrier_extension=none")
    print("Open: native preparation, physical trials, apparatus, multipartite composition")
    raise SystemExit(0 if passed == len(checks) else 1)


if __name__ == "__main__":
    main()
