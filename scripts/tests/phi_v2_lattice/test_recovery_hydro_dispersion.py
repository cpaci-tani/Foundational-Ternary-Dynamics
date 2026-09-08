# scripts/tests/phi_v2_lattice/test_recovery_hydro_dispersion.py
"""Exact operator identities for gate H1. Verdict quantities are not classified here."""
from fractions import Fraction

import flint
import numpy as np
import pytest

from phi_v2_lattice import recovery_hydro_dispersion as D
from phi_v2_lattice import recovery_hydro_invariants as H
from phi_v2_lattice import recovery_kinetic_response as R
from phi_v2_lattice import channels as C

N = 192


@pytest.fixture(scope="module")
def exact100():
    return D.exact_dispersion((1, 0, 0))


def test_physical_r_matches_response_module_convention():
    p = Fraction(1, 96)
    assert D.physical_r(p) == p * (1 - p) ** 189 == R.REFERENCE_P * (1 - R.REFERENCE_P) ** 189


def test_proxy_offset_is_declared_and_small():
    r = D.physical_r(Fraction(1, 96))
    assert abs((D.PROXY_R - r) / r) < Fraction(1, 1000)


def test_complement_rank_is_185_at_proxy(exact100):
    assert exact100.rank_complement == 185


def test_locked_kernel_is_left_null_space_and_biorthogonal(exact100):
    P0 = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)[0]
    I = D.Exact.identity(N)
    assert exact100.W * (I - P0) == D.Exact.zeros(7, N)
    assert exact100.W * exact100.V == D.Exact.identity(7)
    assert (I - P0) * exact100.V == D.Exact.zeros(N, 7)


def test_reduced_resolvent_identities():
    series = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)
    bases = D.exact_bases(series[0], D.PROXY_R)
    resolvent = D.reduced_resolvent(D.Exact, series[0], bases.W, bases.V)
    I = D.Exact.identity(N)
    Q = I - bases.V * bases.W
    assert (I - series[0]) * resolvent == Q
    assert bases.W * resolvent == D.Exact.zeros(7, N)
    assert resolvent * bases.V == D.Exact.zeros(N, 7)


def test_parity_of_expansion_coefficients():
    plus = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)
    minus = D.period_series(D.Exact, (-1, 0, 0), D.PROXY_R)
    assert plus[0] == minus[0]
    assert plus[1] == -minus[1]
    assert plus[2] == minus[2]


def test_first_order_coefficient_equals_single_insertion_sum():
    """Independent construction: A1 = sum_t (prod_{s>t} S0 J_s)(PU D J_t)(prod_{s<t} S0 J_s)."""
    A = D.Exact
    PU = D.streaming_permutation(A)
    Dm, _ = D.direction_matrices(A, (1, 1, 0))
    J = [D.collision_jacobian(A, q, D.PROXY_R) for q in range(3)]
    stages = [PU * J[(0 - t) % 3] for t in range(12)]
    total = A.zeros(N, N)
    for t in range(12):
        left = A.identity(N)
        for s in range(t + 1, 12):
            left = stages[s] * left
        right = A.identity(N)
        for s in range(t):
            right = stages[s] * right
        total += left * (PU * Dm * J[(0 - t) % 3]) * right
    assert D.period_series(A, (1, 1, 0), D.PROXY_R)[1] == total


def test_numeric_map_at_zero_wavevector_matches_exact_p0():
    exact = D.period_series(D.Exact, (1, 0, 0), D.PROXY_R)[0]
    numeric = D.numeric_period_map((0.0, 0.0, 0.0), None, r=float(D.PROXY_R))
    dense = np.array([[float(exact[i, j].p) / float(exact[i, j].q) for j in range(N)] for i in range(N)])
    assert np.max(np.abs(numeric - dense)) < 1e-12


def test_certified_track_encloses_exact_track_at_proxy(exact100):
    certified = D.certified_dispersion((1, 0, 0), None, exact100, r=D.PROXY_R)
    for name in ("M1", "M2"):
        ball, exact = getattr(certified, name), getattr(exact100, name)
        for i in range(7):
            for j in range(7):
                assert D.encloses(ball[i, j], exact[i, j]), (name, i, j)
                assert ball[i, j].rad() < flint.arb("1e-40")


def test_certified_physical_bases_are_consistent(exact100):
    certified = D.certified_dispersion((1, 0, 0), Fraction(1, 96), exact100)
    residual = D.certified_residuals(certified)
    bound = flint.arb("1e-40")
    assert residual["max_left_null_radius"] < bound
    assert residual["max_biorthogonality_error"] < bound
    assert residual["max_right_null_error"] < bound
