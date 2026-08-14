from __future__ import annotations

import math
from pathlib import Path
import sys

import pytest


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "scripts/visualization"))

from viz_gaussian_prime_cone import (  # noqa: E402
    GaussianPrimeKind,
    classify_gaussian_prime,
    cone_lift,
    gaussian_primes_in_box,
    geometry_diagnostics,
    dihedral_orbit,
    moore_minimum_ticks,
    paraboloid_lift,
    reflect_across_diagonal,
    rotate_quarter,
    three_plane_lifts,
    unit_orbit,
)


def test_gaussian_prime_classification_uses_the_correct_norm() -> None:
    ramified = classify_gaussian_prime(1, 1)
    split = classify_gaussian_prime(2, 3)
    inert = classify_gaussian_prime(3, 0)

    assert ramified is not None
    assert ramified.kind is GaussianPrimeKind.RAMIFIED
    assert ramified.norm == 2

    assert split is not None
    assert split.kind is GaussianPrimeKind.SPLIT
    assert split.norm == split.rational_prime == 13

    assert inert is not None
    assert inert.kind is GaussianPrimeKind.INERT
    assert inert.rational_prime == 3
    assert inert.norm == 9

    assert classify_gaussian_prime(1, 0) is None
    assert classify_gaussian_prime(2, 0) is None
    assert classify_gaussian_prime(2, 2) is None


def test_paraboloid_and_cone_lifts_are_distinct_and_exact() -> None:
    split = classify_gaussian_prime(2, 3)
    inert = classify_gaussian_prime(3, 0)
    assert split is not None and inert is not None

    assert paraboloid_lift(split) == (2.0, 3.0, 13.0)
    assert cone_lift(split) == pytest.approx((2.0, 3.0, math.sqrt(13.0)))

    assert paraboloid_lift(inert) == (3.0, 0.0, 9.0)
    assert cone_lift(inert) == (3.0, 0.0, 3.0)


def test_three_plane_lifts_are_minkowski_null() -> None:
    points = gaussian_primes_in_box(18)
    diagnostics = geometry_diagnostics(points)
    assert diagnostics["points"] > 0
    assert diagnostics["max_2_plus_1_null_residual"] < 1e-12
    assert diagnostics["max_3_plus_1_null_residual"] < 1e-12

    split = classify_gaussian_prime(2, 3)
    assert split is not None
    lifts = three_plane_lifts(split)
    assert {lift.plane for lift in lifts} == {"xy", "yz", "zx"}
    assert all(abs(lift.minkowski_null_residual) < 1e-12 for lift in lifts)


def test_diagonal_is_prime_free_beyond_ramified_associates() -> None:
    for coordinate in range(2, 80):
        assert classify_gaussian_prime(coordinate, coordinate) is None
        assert classify_gaussian_prime(coordinate, -coordinate) is None


def test_exact_unit_and_dihedral_periods_are_not_radial_time_periods() -> None:
    assert unit_orbit(2, 3) == ((2, 3), (-3, 2), (-2, -3), (3, -2))
    assert rotate_quarter(*unit_orbit(2, 3)[-1]) == (2, 3)
    assert reflect_across_diagonal(2, 3) == (3, 2)
    assert len(dihedral_orbit(2, 3)) == 8

    # The ramified diagonal prime is fixed by reflection up to its unit orbit.
    assert len(dihedral_orbit(1, 1)) == 4


def test_quadratic_channel_weights_are_normalized() -> None:
    point = classify_gaussian_prime(2, 3)
    assert point is not None
    x_component, y_component = point.normalized_components
    assert x_component * x_component == pytest.approx(4.0 / 13.0)
    assert y_component * y_component == pytest.approx(9.0 / 13.0)
    assert x_component * x_component + y_component * y_component == pytest.approx(1.0)


def test_euclidean_cone_is_not_the_native_moore_distance() -> None:
    point = classify_gaussian_prime(2, 3)
    assert point is not None
    assert moore_minimum_ticks(point.a, point.b) == 3
    assert point.radius == pytest.approx(math.sqrt(13.0))
    assert point.radius != moore_minimum_ticks(point.a, point.b)
