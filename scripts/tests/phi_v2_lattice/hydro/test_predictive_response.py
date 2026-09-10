"""Instrument controls only; none execute the registered microscopic cases."""
from fractions import Fraction as F
from math import factorial

import numpy as np
import pytest

from phi_v2_lattice.experiments import predictive_response as P


def test_identity_collision_has_only_the_source_response_column():
    even, odd = P.column_counts(np.arange(8, dtype=np.uint32), channels=3)
    assert [b - a for a, b in zip(even, odd)] == [4, 0, 0]
    assert [a + b for a, b in zip(even, odd)] == [4, 4, 4]


def test_input_channel_swap_is_detected_with_correct_sign():
    masks = np.arange(8, dtype=np.uint32)
    swapped = ((masks & 1) << 1) | ((masks & 2) >> 1) | (masks & 4)
    even, odd = P.column_counts(swapped, channels=3)
    assert [b - a for a, b in zip(even, odd)] == [0, 4, 0]


def test_incomplete_alphabet_is_rejected():
    with pytest.raises(ValueError, match="complete local alphabet"):
        P.column_counts(np.arange(7, dtype=np.uint32), channels=3)


def test_fixed_oblique_profile_and_reverse_stream_correlation():
    a = P.profile()
    assert a.shape == (16, 16, 16) and set(np.unique(a)) == {-1, 1}
    assert int(a.sum()) == 0
    assert F(int((a * np.roll(a, -2, axis=0)).sum()), a.size) == F(1, 2)


def test_radius_proves_simultaneous_family_bound_without_floating_tolerance():
    assert P.N * P.RADIUS ** 2 / 2 == 9
    exp9_lower = sum((F(9 ** k, factorial(k)) for k in range(25)), F(0))
    assert F(48, 1) / exp9_lower < F(1, 100)


def test_dyadic_input_recipe_uses_disjoint_fair_bits():
    words = np.arange(4, dtype=np.uint32) << 24
    assert (((words >> 24) & 3) < 3).sum() == 3
    assert (((words >> 24) & 3) < 1).sum() == 1
    masks = ((words | 0xabcdef) & 0xfffffe) | ((((words >> 24) & 3) < 3).astype(np.uint32))
    assert all((int(mask) >> 1) == (0xabcdef >> 1) for mask in masks)
