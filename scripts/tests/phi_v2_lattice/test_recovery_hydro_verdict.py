# scripts/tests/phi_v2_lattice/test_recovery_hydro_verdict.py
"""Verdict clause logic on synthetic 7x7 operators; the real operators are classified only by the evidence runner."""
from fractions import Fraction

import flint
import pytest

from phi_v2_lattice import recovery_hydro_dispersion as D
from phi_v2_lattice import recovery_hydro_verdict as V


def _mat(rows):
    return D.Exact.from_rows(rows)


def _w_with_constant_first():
    """A synthetic W whose first row is the constant weight; only its row space matters here."""
    rows = [[1] * 192]
    for k in range(1, 7):
        rows.append([1 if (c % 7) == k else 0 for c in range(192)])
    return _mat(rows)


class Synthetic:
    """Dispersion-like objects with direct 7x7 operators."""
    def __init__(self, direction, M1, M2, W):
        self.direction, self.M1, self.M2, self.W = direction, M1, M2, W


def _isotropic_case(direction, cs2=Fraction(1, 3), nu=Fraction(1, 6), zeta=Fraction(1, 10)):
    """Density + 3 momenta form an NS block; three extra modes decay independently."""
    n = direction
    n2 = sum(x * x for x in n)
    M1 = [[Fraction(0)] * 7 for _ in range(7)]
    # rows act on functionals: a M1 gives the flux functional of a
    for a in range(3):
        M1[0][1 + a] = Fraction(n[a])          # d rho -> n . j
        M1[1 + a][0] = cs2 * n[a]              # d j_a -> cs2 n_a rho
    M2 = [[Fraction(0)] * 7 for _ in range(7)]
    for a in range(3):
        for b in range(3):
            M2[1 + a][1 + b] = nu * n2 * (a == b) + (zeta + nu / 3) * n[a] * n[b]
    for k in range(4, 7):
        M2[k][k] = Fraction(k) * n2
    return _mat(M1), _mat(M2)


def _anisotropic_case(direction):
    M1, M2 = _isotropic_case(direction)
    n = direction
    # cubic term: extra transverse damping proportional to sum n_a^4 breaks isotropy
    rows = [[Fraction(int(M2[i, j].p), int(M2[i, j].q)) for j in range(7)] for i in range(7)]
    for a in range(3):
        rows[1 + a][1 + a] += Fraction(1, 5) * n[a] ** 4
    return M1, _mat(rows)


def test_density_functional_reproduces_constant_weight():
    W = _w_with_constant_first()
    a = V.density_functional(W)
    assert a * W == _mat([[1] * 192])


def test_closure_block_of_isotropic_case_is_four_dimensional():
    W = _w_with_constant_first()
    ops = []
    for n in D.DIRECTIONS:
        M1, M2 = _isotropic_case(n)
        ops += [M1, M2]
    block = V.closure_block(V.density_functional(W), ops)
    assert block.nrows() == 4


def test_isotropic_synthetic_is_ns_class():
    W = _w_with_constant_first()
    disp = {n: Synthetic(n, *_isotropic_case(n), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "NS-class isotropic"
    assert verdict["sound_speed_squared_normalized"] == {str(n): "1/3" for n in D.DIRECTIONS}
    assert verdict["transverse_polynomial_normalized"]["(1, 0, 0)"] == ["-1/3", "1/36"]


def test_anisotropic_synthetic_is_flagged():
    W = _w_with_constant_first()
    disp = {n: Synthetic(n, *_anisotropic_case(n), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "anisotropic momentum hydrodynamics"
    assert verdict["clauses"]["block_dimension_is_four"] is True
    assert verdict["clauses"]["transverse_isotropic"] is False


def test_direction_dependent_sound_speed_is_other():
    W = _w_with_constant_first()
    speeds = {(1, 0, 0): Fraction(1, 3), (1, 1, 0): Fraction(1, 2), (1, 1, 1): Fraction(1, 2)}
    disp = {n: Synthetic(n, *_isotropic_case(n, cs2=speeds[n]), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "other"
    assert verdict["clauses"]["sound_pair_all_directions"] is True
    assert verdict["clauses"]["sound_speed_direction_independent"] is False
    assert verdict["clauses"]["clause_2_holds"] is False


def _seven_dim_case(direction):
    """Isotropic M1 unchanged; M2 perturbed so the density-functional closure reaches
    all seven coordinates (rho, j, and the three independently-decaying modes)."""
    M1, M2 = _isotropic_case(direction)
    rows = [[Fraction(int(M2[i, j].p), int(M2[i, j].q)) for j in range(7)] for i in range(7)]
    rows[0][4] = Fraction(1)
    rows[4][5] = Fraction(1)
    rows[5][6] = Fraction(1)
    return M1, _mat(rows)


def test_seven_dimensional_closure_is_other_and_certified_does_not_raise():
    W = _w_with_constant_first()
    exact = {n: Synthetic(n, *_seven_dim_case(n), W) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(exact)
    assert verdict["block_dimension"] == 7
    assert verdict["label"] == "other"
    assert verdict["block_dimension_first_order_only"] == 4
    assert len(verdict["charpoly_M1_on_block"]) == 3

    block = V.closure_block(V.density_functional(W),
                             [m for n in D.DIRECTIONS for m in (exact[n].M1, exact[n].M2)])
    assert block.nrows() == 7
    flint.ctx.prec = 256
    balls = {}
    for n in D.DIRECTIONS:
        M1, M2 = _seven_dim_case(n)
        balls[n] = Synthetic(
            n,
            flint.arb_mat([[D.Certified.scalar(Fraction(int(M1[i, j].p), int(M1[i, j].q))) for j in range(7)]
                           for i in range(7)]),
            flint.arb_mat([[D.Certified.scalar(Fraction(int(M2[i, j].p), int(M2[i, j].q))) for j in range(7)]
                           for i in range(7)]),
            W)
    report = V.certified_verdict(balls, block)
    assert report["block_dimension"] == 7
    assert report["sound_speed_direction_independent"] == "NOT APPLICABLE (block dimension 7 != 4)"
    assert report["transverse_isotropic"] == "NOT APPLICABLE (block dimension 7 != 4)"
    assert report["longitudinal_isotropic"] == "NOT APPLICABLE (block dimension 7 != 4)"
    assert report["values"] == {}
    assert len(report["charpoly_M1_certified"]) == 3
    for coeffs in report["charpoly_M1_certified"].values():
        assert len(coeffs) == 8


def test_diffusive_only_synthetic():
    W = _w_with_constant_first()
    disp = {}
    for n in D.DIRECTIONS:
        _, M2 = _isotropic_case(n)
        disp[n] = Synthetic(n, D.Exact.zeros(7, 7), M2, W)
    verdict = V.exact_verdict(disp)
    assert verdict["label"] == "diffusive only"


def test_certified_verdict_marks_proved_consistent_or_undecided():
    W = _w_with_constant_first()
    exact = {n: Synthetic(n, *_anisotropic_case(n), W) for n in D.DIRECTIONS}
    block = V.closure_block(V.density_functional(W), [m for n in D.DIRECTIONS for m in (exact[n].M1, exact[n].M2)])
    flint.ctx.prec = 256
    balls = {}
    for n in D.DIRECTIONS:
        M1, M2 = _anisotropic_case(n)
        balls[n] = Synthetic(n, flint.arb_mat([[D.Certified.scalar(Fraction(int(M1[i, j].p), int(M1[i, j].q))) for j in range(7)] for i in range(7)]),
                             flint.arb_mat([[D.Certified.scalar(Fraction(int(M2[i, j].p), int(M2[i, j].q))) for j in range(7)] for i in range(7)]), W)
    report = V.certified_verdict(balls, block)
    assert report["transverse_isotropic"] == "REFUTED"
    assert report["sound_speed_direction_independent"] == "CONSISTENT"
