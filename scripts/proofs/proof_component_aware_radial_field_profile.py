"""Exact-rational certificate for FTD-0683 component-aware radial profiles."""

from fractions import Fraction
from hashlib import sha256
from itertools import permutations, product
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_COMPONENT_AWARE_RADIAL_FIELD_PROFILE_v1.md"
EXPECTED_PROTOCOL_HASH = "4B79D37C5914DD0D5CFBDFB013FD04DCFA76CC32C5C2BD0D7A07EE9001C3425A"


def check(label: str, condition: bool) -> None:
    if not condition:
        raise AssertionError(label)


def radius2(point, origin):
    return max(abs(2 * (point[i] - origin[i])) for i in range(3))


def transform(vector, permutation, signs):
    return tuple(signs[i] * vector[permutation[i]] for i in range(3))


def profile(points, origin):
    bins = {}
    total = Fraction(0)
    first = Fraction(0)
    second = Fraction(0)
    for point, weight in points:
        r2 = radius2(point, origin)
        check("doubled radius is integral", r2.denominator == 1)
        r2 = int(r2)
        bins[r2] = bins.get(r2, Fraction(0)) + weight
        total += weight
        radius = Fraction(r2, 2)
        first += weight * radius
        second += weight * radius * radius
    cumulative = []
    running = Fraction(0)
    for r2 in range(max(bins, default=0) + 1):
        running += bins.get(r2, Fraction(0))
        cumulative.append(running)
    return bins, cumulative, total, first, second


def quantile(cumulative, total, numerator, denominator):
    if total == 0:
        return 0
    threshold = Fraction(numerator, denominator) * total
    return next(index for index, value in enumerate(cumulative)
                if value >= threshold)


def main() -> None:
    check("protocol hash",
          sha256(PROTOCOL.read_bytes()).hexdigest().upper()
          == EXPECTED_PROTOCOL_HASH)

    origin = (Fraction(2), Fraction(3), Fraction(1))
    # Actual staggered carrier locations for E_x,E_y,E_z,B_x,B_y,B_z.
    points = [
        ((Fraction(5, 2), Fraction(3), Fraction(1)), Fraction(4)),
        ((Fraction(2), Fraction(9, 2), Fraction(1)), Fraction(3)),
        ((Fraction(5), Fraction(3), Fraction(3, 2)), Fraction(9)),
        ((Fraction(2), Fraction(7, 2), Fraction(3, 2)), Fraction(2)),
        ((Fraction(5, 2), Fraction(7), Fraction(3, 2)), Fraction(4)),
        ((Fraction(9, 2), Fraction(7, 2), Fraction(1)), Fraction(5)),
    ]
    reference = profile(points, origin)
    bins, cumulative, total, first, second = reference
    check("partition", sum(bins.values(), Fraction(0)) == total)
    check("cumulative closure", cumulative[-1] == total)
    check("monotone", all(left <= right
                           for left, right in zip(cumulative, cumulative[1:])))
    check("moments positive", first > 0 and second > 0)
    for numerator, denominator in ((1, 2), (9, 10), (99, 100)):
        q = quantile(cumulative, total, numerator, denominator)
        threshold = Fraction(numerator, denominator) * total
        check("quantile reaches threshold", cumulative[q] >= threshold)
        check("quantile minimal", q == 0 or cumulative[q - 1] < threshold)

    maps = 0
    for permutation in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            transformed_origin = transform(origin, permutation, signs)
            transformed_points = []
            for point, weight in points:
                transformed_points.append(
                    (transform(point, permutation, signs), weight))
            check("signed cubic profile covariance",
                  profile(transformed_points, transformed_origin) == reference)
            maps += 1
    check("all signed cubic maps", maps == 48)

    translation = (Fraction(3), Fraction(-2), Fraction(5))
    translated_origin = tuple(origin[i] + translation[i] for i in range(3))
    translated_points = [
        (tuple(point[i] + translation[i] for i in range(3)), weight)
        for point, weight in points
    ]
    check("integer translation covariance",
          profile(translated_points, translated_origin) == reference)
    check("zero profile quantiles",
          quantile([Fraction(0)], Fraction(0), 1, 2) == 0
          and quantile([Fraction(0)], Fraction(0), 9, 10) == 0)

    print("FTD-0683 component-aware radial profile certificate: PASS "
          f"maps={maps} carriers=6 arithmetic=rational total={total}")


if __name__ == "__main__":
    main()
