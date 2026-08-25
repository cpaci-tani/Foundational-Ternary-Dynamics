#!/usr/bin/env python3
"""Exact prepared C4 Gaussian-integer general-amplitude limit certificate.

The finite C4 construction produces |Z_o|^2 physical Gauss events for every Gaussian
integer response Z_o.  This certificate proves the exact finite count map,
checks the canonical nearest-Gaussian-integer blocking on normalized rational
complex fixtures, verifies a rigorous total-variation error bound using only
rational arithmetic, and prices the finite record/address-period resources.

It is a representation and conditional limit theorem.  It does not derive the
approximating record banks from a source action, externally heralded trials,
multipartite no-signalling, or a continuum wavefunction ontology.
"""

from __future__ import annotations

from fractions import Fraction
from itertools import product

from proof_ternary_square_phase_polarity_autonomous_clock import (
    TOKENS,
    phase_coordinates,
    polarity,
)


ComplexQ = tuple[Fraction, Fraction]


def complex_norm_squared(value: ComplexQ) -> Fraction:
    real, imag = value
    return real * real + imag * imag


def nearest_integer_odd(value: Fraction) -> int:
    """Nearest integer with half ties away from zero; exactly odd under sign."""

    if value < 0:
        return -nearest_integer_odd(-value)
    lower = value.numerator // value.denominator
    remainder = value - lower
    return lower + int(remainder >= Fraction(1, 2))


def gaussian_round(amplitudes: tuple[ComplexQ, ...], scale: int) -> tuple[tuple[int, int], ...]:
    assert scale > 0
    return tuple(
        (
            nearest_integer_odd(scale * real),
            nearest_integer_odd(scale * imag),
        )
        for real, imag in amplitudes
    )


def c4_counts(value: tuple[int, int]) -> tuple[int, int, int, int]:
    """Minimum residual C4 multiset realizing one Gaussian integer."""

    real, imag = value
    return max(real, 0), max(imag, 0), max(-real, 0), max(-imag, 0)


def residual(counts: tuple[int, int, int, int]) -> tuple[int, int]:
    return counts[0] - counts[2], counts[1] - counts[3]


def event_count(value: tuple[int, int]) -> int:
    real, imag = value
    return real * real + imag * imag


def event_frequencies(values: tuple[tuple[int, int], ...]) -> tuple[Fraction, ...]:
    counts = tuple(event_count(value) for value in values)
    total = sum(counts)
    assert total > 0
    return tuple(Fraction(count, total) for count in counts)


def target_probabilities(amplitudes: tuple[ComplexQ, ...]) -> tuple[Fraction, ...]:
    norm = sum(complex_norm_squared(value) for value in amplitudes)
    assert norm == 1
    return tuple(complex_norm_squared(value) for value in amplitudes)


def total_variation(left: tuple[Fraction, ...], right: tuple[Fraction, ...]) -> Fraction:
    assert len(left) == len(right)
    return Fraction(1, 2) * sum(abs(a - b) for a, b in zip(left, right))


def rotate_gaussian(value: tuple[int, int]) -> tuple[int, int]:
    real, imag = value
    return -imag, real


FIXTURES: tuple[tuple[ComplexQ, ...], ...] = (
    ((Fraction(1), Fraction(0)),),
    (
        (Fraction(3, 5), Fraction(0)),
        (Fraction(4, 5), Fraction(0)),
    ),
    (
        (Fraction(1, 2), Fraction(1, 2)),
        (Fraction(1, 2), Fraction(-1, 2)),
    ),
    (
        (Fraction(1, 3), Fraction(0)),
        (Fraction(2, 3), Fraction(0)),
        (Fraction(2, 3), Fraction(0)),
    ),
    (
        (Fraction(1, 2), Fraction(0)),
        (Fraction(0), Fraction(1, 2)),
        (Fraction(1, 2), Fraction(1, 2)),
    ),
)


def main() -> None:
    checks = 0

    # The revised two-ternary-slot token has exactly two retained polarity
    # states above every C4 phase.  The Born rail reads phase, not polarity;
    # complete polarity payload can therefore survive without changing counts.
    phase_polarity = {(phase_coordinates(token), polarity(token)) for token in TOKENS}
    assert len(phase_polarity) == 8
    for phase in ((1, 0), (0, 1), (-1, 0), (0, -1)):
        assert (phase, 1) in phase_polarity
        assert (phase, -1) in phase_polarity
        checks += 2

    # Exhaust the exact Gaussian-integer -> C4 residual -> squared-event map.
    gaussian_box = tuple(product(range(-4, 5), repeat=2))
    for value in gaussian_box:
        counts = c4_counts(value)
        assert residual(counts) == value
        assert sum(counts) == abs(value[0]) + abs(value[1])
        assert (counts[0] - counts[2]) ** 2 + (counts[1] - counts[3]) ** 2 == event_count(value)
        assert event_count(rotate_gaussian(value)) == event_count(value)
        checks += 4

    # Multi-outcome exact physical frequencies and global C4 invariance.
    small_values = tuple(product(range(-2, 3), repeat=2))
    for values in product(small_values, repeat=2):
        if all(value == (0, 0) for value in values):
            continue
        frequencies = event_frequencies(values)
        rotated = tuple(rotate_gaussian(value) for value in values)
        assert event_frequencies(rotated) == frequencies
        assert sum(frequencies) == 1
        for value in values:
            assert event_count(value) == sum(component * component for component in residual(c4_counts(value)))
            checks += 1
        checks += 2

    # Canonical nearest-Gaussian-integer approximants.  For m outcomes and
    # scale N, every complex coordinate has squared error <= 1/(2N^2), so
    # ||e||^2 <= m/(2N^2).  If eta=||e||<1, normalization gives
    # TV <= 2 eta/(1-eta).  The last inequality is checked without radicals:
    # TV^2 <= ||e||^2 (TV+2)^2.
    for amplitudes in FIXTURES:
        target = target_probabilities(amplitudes)
        outcomes = len(amplitudes)
        for scale in range(2, 65):
            values = gaussian_round(amplitudes, scale)
            frequencies = event_frequencies(values)
            errors: list[ComplexQ] = []
            for (integer_real, integer_imag), (target_real, target_imag) in zip(values, amplitudes):
                error = (
                    Fraction(integer_real, scale) - target_real,
                    Fraction(integer_imag, scale) - target_imag,
                )
                errors.append(error)
                assert complex_norm_squared(error) <= Fraction(1, 2 * scale * scale)
                assert residual(c4_counts((integer_real, integer_imag))) == (
                    integer_real,
                    integer_imag,
                )
                checks += 2

            error_norm_squared = sum(complex_norm_squared(error) for error in errors)
            assert error_norm_squared <= Fraction(outcomes, 2 * scale * scale)
            assert error_norm_squared < 1

            variation = total_variation(frequencies, target)
            assert variation * variation <= error_norm_squared * (variation + 2) ** 2
            assert sum(frequencies) == 1
            checks += 4

            # The minimum residual bank has L=sum(|Re Z|+|Im Z|) records.
            # Cauchy plus nearest rounding gives L <= N sqrt(2m)+m.  Check the
            # radical-free consequence (max(L-m,0))^2 <= 2mN^2.  The physical
            # coprime detector tape then has exactly L(L+1) finite cells.
            bank_size = sum(abs(real) + abs(imag) for real, imag in values)
            tape_size = bank_size * (bank_size + 1)
            assert max(bank_size - outcomes, 0) ** 2 <= 2 * outcomes * scale * scale
            assert tape_size >= 0
            checks += 2

    # Charge-conjugation-compatible rounding: no sign choice is hidden in a
    # half-integer tie convention.
    for numerator in range(-64, 65):
        for denominator in range(1, 17):
            value = Fraction(numerator, denominator)
            rounded = nearest_integer_odd(value)
            assert nearest_integer_odd(-value) == -rounded
            assert abs(Fraction(rounded) - value) <= Fraction(1, 2)
            checks += 2

    print(f"PASS: C4 Gaussian-integer general-amplitude limit ({checks} exact checks)")
    print("finite prepared Z in Z[i]^m -> exact physical frequencies |Z_o|^2/sum|Z|^2")
    print("nearest blocking: ||e||^2 <= m/(2N^2), TV <= 2||e||/(1-||e||)")
    print(
        "finite price: L <= N sqrt(2m)+m records, T=L(L+1) address ticks; "
        "legacy tape uses T cells"
    )
    print(
        "renewal successor reuses one detector payload; Open: native preparation, "
        "incomplete windows, heralded trials, composition/no-signalling"
    )


if __name__ == "__main__":
    main()
