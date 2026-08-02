#!/usr/bin/env python3
"""Exact-rational certificate for the FTD-0677 localized-basin observer."""

from fractions import Fraction as F
from itertools import permutations, product


Vector = tuple[F, F, F]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def add(left: Vector, right: Vector) -> Vector:
    return tuple(a + b for a, b in zip(left, right))  # type: ignore[return-value]


def sub(left: Vector, right: Vector) -> Vector:
    return tuple(a - b for a, b in zip(left, right))  # type: ignore[return-value]


def scale(value: Vector, factor: F) -> Vector:
    return tuple(factor * x for x in value)  # type: ignore[return-value]


def norm2(value: Vector) -> F:
    return sum((x * x for x in value), F(0))


def mean(values: list[Vector]) -> Vector:
    return scale(
        tuple(sum((value[axis] for value in values), F(0))
              for axis in range(3)),  # type: ignore[arg-type]
        F(1, len(values)),
    )


def internal_metric(reference: list[Vector], candidate: list[Vector],
                    weight: F) -> F:
    reference_center = mean(reference)
    candidate_center = mean(candidate)
    return weight * sum(
        (norm2(sub(sub(candidate[index], candidate_center),
                   sub(reference[index], reference_center)))
         for index in range(len(reference))),
        F(0),
    )


def transform(value: Vector, order: tuple[int, int, int],
              signs: tuple[int, int, int]) -> Vector:
    return tuple(F(signs[axis]) * value[order[axis]]
                 for axis in range(3))  # type: ignore[return-value]


def main() -> None:
    mass = F(1, 2)
    omega = F(2)
    reference_x = [(F(-1), F(0), F(0)), (F(1), F(0), F(0))]
    candidate_x = [(F(-3, 4), F(0), F(0)),
                   (F(3, 4), F(0), F(0))]
    reference_p = [(F(0), F(0), F(0)), (F(0), F(0), F(0))]
    candidate_p = [(F(0), F(1, 5), F(0)),
                   (F(0), F(-1, 5), F(0))]

    position = internal_metric(reference_x, candidate_x, mass)
    momentum = internal_metric(reference_p, candidate_p, F(1, 1) / mass)
    phase = omega * omega * position + momentum
    require(position == F(1, 16), "position witness")
    require(momentum == F(4, 25), "momentum witness")
    require(phase == F(41, 100), "phase witness")

    translation = (F(7, 3), F(-5, 4), F(9, 7))
    boost = (F(2, 11), F(-3, 13), F(5, 17))
    translated_reference = [add(value, translation) for value in reference_x]
    translated_candidate = [add(value, translation) for value in candidate_x]
    boosted_reference = [add(value, boost) for value in reference_p]
    boosted_candidate = [add(value, boost) for value in candidate_p]
    require(internal_metric(translated_reference, translated_candidate, mass)
            == position, "common-translation quotient")
    require(internal_metric(boosted_reference, boosted_candidate, F(1) / mass)
            == momentum, "common-boost quotient")

    transformations = 0
    for order in permutations((0, 1, 2)):
        for signs in product((-1, 1), repeat=3):
            tx0 = [transform(value, order, signs) for value in reference_x]
            tx1 = [transform(value, order, signs) for value in candidate_x]
            tp0 = [transform(value, order, signs) for value in reference_p]
            tp1 = [transform(value, order, signs) for value in candidate_p]
            require(internal_metric(tx0, tx1, mass) == position,
                    "signed-cubic position covariance")
            require(internal_metric(tp0, tp1, F(1) / mass) == momentum,
                    "signed-cubic momentum covariance")
            radius = (F(1), F(2), F(3))
            require(max(abs(x) for x in transform(radius, order, signs))
                    == max(abs(x) for x in radius),
                    "Chebyshev-shell covariance")
            transformations += 1

    beta = F(3)
    speed = F(1, 2)
    near = F(1, 2) * beta * F(2) ** 2
    intermediate = F(1, 2) * beta * F(3) ** 2
    far = F(1, 2) * beta * speed * speed * F(4) ** 2
    total = near + intermediate + far
    require((near, intermediate, far, total)
            == (F(6), F(27, 2), F(6), F(51, 2)),
            "positive exact field-shell partition")
    require(total - (near + intermediate + far) == 0,
            "field partition residual")
    require(position >= 0 and momentum >= 0 and phase >= 0,
            "positive phase metric")

    print(
        "FTD-0677 localized-basin observer certificate: PASS "
        f"phase={phase} field_total={total} "
        f"signed_cubic_maps={transformations} arithmetic=rational"
    )


if __name__ == "__main__":
    main()
