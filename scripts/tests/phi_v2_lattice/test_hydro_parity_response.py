"""Conditional finite-matrix response gates, separate from microscopic closure."""
from fractions import Fraction

import pytest

from phi_v2_lattice import hydro_parity as H
from phi_v2_lattice import hydro_parity_response as R


@pytest.mark.parametrize("p", [Fraction(1, 2), Fraction(1, 3), Fraction(1, 4)])
def test_exact_shear_sound_and_four_dimensional_blocks(p):
    c, _ = H.reference_coefficients(p)
    tau = 1/(14880*c)-Fraction(1, 2)
    for n, (t, u) in R.PROBES:
        assert R.shear_rate(n, t, p) == R.shear_rate(n, u, p) == tau/3
        assert R.shear_form(n, t, u, p) == 0
        assert R.sound_coefficients(n, p) == (Fraction(1, 2), tau/4)
        A, B = R.conserved_generator_blocks(n, p)
        norm = sum(x*x for x in n)
        # A^3=(|n|^2/2) A; rank two with density/longitudinal sound pair.
        multiply = lambda X, Y: [[sum(X[i][k]*Y[k][j] for k in range(4)) for j in range(4)] for i in range(4)]
        A3 = multiply(multiply(A, A), A)
        assert A3 == [[Fraction(norm, 2)*x for x in row] for row in A]
        assert all(B[0][i] == B[i][0] == 0 for i in range(4))
        assert [sum(B[i+1][j+1]*n[j] for j in range(3)) for i in range(3)] == [tau*norm*x/2 for x in n]


def test_normalization_arbitrary_direction_isotropy_and_density_reflection():
    p = Fraction(1, 3)
    reference = R.shear_rate((1, 0, 0), (0, 1, 0), p)
    assert R.shear_rate((2, 2, 0), (3, -3, 0), p) == reference
    assert R.shear_rate((2, 1, 3), (1, -2, 0), p) == reference
    assert R.shear_form((2, 1, 3), (1, -2, 0), (6, 3, -5), p) == 0
    assert R.sound_coefficients((2, 1, 3), p) == R.sound_coefficients((1, 0, 0), p)
    assert R.shear_rate((1, 0, 0), (0, 1, 0), 1-p) == reference


def test_response_certificate_reports_scoped_pass_and_remaining_scale_price():
    report = R.certificate()
    assert report["conditional_isotropic_response_gate"]
    assert report["stress_relaxation_per_cycle_at_half"] == "465/262144"
    assert report["inverse_stress_relaxation_cycles_at_half"] == "262144/465"
    assert report["fourth_momentum_multiplier_at_half"] == "0"
    for probe in report["probes"]:
        assert probe["shear_rates_per_cycle"] == ["523823/2790"]*2
        assert probe["sound_speed_squared_per_cycle"] == "1/2"
        assert probe["sound_log_damping_per_cycle"] == "523823/3720"
    assert report["physical_microticks_per_cycle"] == 2
    assert report["feasibility"]["microscopic_ensemble_gate_required"]
    assert not report["continuum_recovered"] and not report["canonical_adoption"]


@pytest.mark.parametrize("n,t,p", [((0, 0, 0), (1, 0, 0), Fraction(1, 2)),
    ((1, 0, 0), (1, 0, 0), Fraction(1, 2)), ((1, 0, 0), (0, True, 0), Fraction(1, 2)),
    ((1, 0, 0), (0, 1, 0), 0.5), ((1, 0, 0), (0, 1, 0), Fraction(0)),
    ((1, 0, 0), (0, 1, 0), Fraction(1))])
def test_invalid_response_probe_rejected(n, t, p):
    with pytest.raises(ValueError):
        R.shear_rate(n, t, p)
