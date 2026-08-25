#!/usr/bin/env python3
"""Exact checks for the C4 paired-history counting identity.

The result is combinatorial.  It does not show that FTD dynamics performs the
opposite-phase cancellation, constructs the ordered-pair basins, or samples
those basins with the required measure.  Accordingly it is not a physical
derivation of the Born rule.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product


Count4 = tuple[int, int, int, int]


def rotate_counts(counts: Count4, quarter_turns: int = 1) -> Count4:
    """Multiply every history phase by i**quarter_turns."""

    turns = quarter_turns % 4
    return tuple(counts[(index - turns) % 4] for index in range(4))  # type: ignore[return-value]


def amplitude_components(counts: Count4) -> tuple[int, int]:
    n_0, n_1, n_2, n_3 = counts
    return (n_0 - n_2, n_1 - n_3)


def canonical_residual_rails(counts: Count4) -> tuple[int, int]:
    """Cancel opposite phases and return unsigned real/imaginary rail sizes."""

    real, imag = amplitude_components(counts)
    return (abs(real), abs(imag))


def coherent_norm_squared(counts: Count4) -> int:
    real, imag = amplitude_components(counts)
    return real * real + imag * imag


def compatible_ordered_pair_count(counts: Count4) -> int:
    """Cardinality of (R x R) disjoint-union (I x I), including self-pairs."""

    real_residual, imag_residual = canonical_residual_rails(counts)
    return real_residual * real_residual + imag_residual * imag_residual


def born_weight_from_pair_basins(outcomes: tuple[Count4, ...], selected: int) -> Fraction:
    basin_sizes = tuple(compatible_ordered_pair_count(counts) for counts in outcomes)
    total = sum(basin_sizes)
    assert total > 0
    return Fraction(basin_sizes[selected], total)


def coherent_weight(outcomes: tuple[Count4, ...], selected: int) -> Fraction:
    norms = tuple(coherent_norm_squared(counts) for counts in outcomes)
    total = sum(norms)
    assert total > 0
    return Fraction(norms[selected], total)


def main() -> None:
    checks = 0
    count_domain = tuple(product(range(9), repeat=4))

    for raw_counts in count_domain:
        counts: Count4 = raw_counts
        residual_real, residual_imag = canonical_residual_rails(counts)
        n_0, n_1, n_2, n_3 = counts

        # Canonical opposite-phase cancellation leaves one sign on each rail.
        assert residual_real == n_0 + n_2 - 2 * min(n_0, n_2)
        assert residual_imag == n_1 + n_3 - 2 * min(n_1, n_3)
        checks += 2

        # The compatible ordered-pair basin has exactly the coherent C4 norm.
        assert compatible_ordered_pair_count(counts) == coherent_norm_squared(counts)
        checks += 1

        # A common C4 phase rotation cannot change the count or norm.
        for turns in range(4):
            rotated = rotate_counts(counts, turns)
            assert compatible_ordered_pair_count(rotated) == compatible_ordered_pair_count(counts)
            assert coherent_norm_squared(rotated) == coherent_norm_squared(counts)
            checks += 2

    # Perfect destructive interference is a zero basin, not a small basin.
    for n_real, n_imag in product(range(9), repeat=2):
        destructive: Count4 = (n_real, n_imag, n_real, n_imag)
        assert amplitude_components(destructive) == (0, 0)
        assert compatible_ordered_pair_count(destructive) == 0
        checks += 2

    # Conditional pushforward: if physical microstates are exactly the
    # disjoint ordered-pair basins and carry the uniform counting measure,
    # their outcome probabilities are the normalized coherent norms.
    sample_outcomes: tuple[tuple[Count4, ...], ...] = (
        ((1, 0, 0, 0), (0, 1, 0, 0)),
        ((3, 1, 1, 0), (0, 4, 0, 1), (2, 2, 2, 1)),
        ((8, 0, 3, 0), (1, 7, 0, 2), (4, 4, 4, 4), (0, 0, 0, 2)),
    )
    for outcomes in sample_outcomes:
        for selected in range(len(outcomes)):
            assert born_weight_from_pair_basins(outcomes, selected) == coherent_weight(
                outcomes, selected
            )
            checks += 1
        assert sum(
            (born_weight_from_pair_basins(outcomes, index) for index in range(len(outcomes))),
            start=Fraction(0, 1),
        ) == 1
        checks += 1

    print(f"PASS: C4 paired-history counting identity ({checks} exact checks)")
    print("Boundary: exact counting theorem, not a physical Born pushforward")


if __name__ == "__main__":
    main()
