# scripts/tests/phi_v2_lattice/hydro/test_boltzmann.py
from fractions import Fraction
import flint
import numpy as np
from phi_v2_lattice.hydro import boltzmann as B, channels as H


def test_collision_counts_conserve_number_and_momentum_columnwise():
    C, D = B.collision_counts()
    for n in range(25):
        assert C[n].sum(axis=0).tolist() == [0] * 24 and D[n].sum(axis=0).tolist() == [0] * 24
        for a in range(3):
            va = np.array([v[a] for v in H.VELOCITIES])
            assert (va @ C[n]).tolist() == [0] * 24 and (va @ D[n]).tolist() == [0] * 24


def test_K_is_exact_and_annihilated_by_conserved_weights():
    K = B.K_exact(Fraction(1, 4))
    for row in B.weights_rows():
        assert flint.fmpq_mat([list(row)]) * K == flint.fmpq_mat(1, 24)
    assert K != flint.fmpq_mat(24, 24)


def test_verdict_at_quarter_density():
    v = B.verdict(Fraction(1, 4))
    assert v["block_dimension"] == 4
    # H1' NS-class gate CLOSED NEGATIVE at the registered scope (2026-09-08); pins the verdict of record, see engine/docs/evidence/strict-hydro4-dispersion-exact.json
    assert v["label"] == "anisotropic momentum hydrodynamics"
    assert set(v["sound_speed_squared_normalized"].values()) == {"1/2"}   # hand route: sum_i v_ix^2 / 24
    assert v["stable"] is True and Fraction(v["shear_viscosity"]) > 0
    assert v["cubic_identity_holds"] is True
    assert v["stable"] is True
    assert Fraction(v["cubic_shear_constants"]["nu_E"]) > Fraction(v["cubic_shear_constants"]["nu_T2"]) > 0


def test_nonlinear_coefficients_match_hand_derivation():
    for d in (Fraction(1, 4), Fraction(1, 8), Fraction(3, 8)):
        g = B.nonlinear_coefficients(d)
        assert g["g"] == Fraction(2, 3) * (1 - 2 * d) / (1 - d)
        assert g["q1"] == Fraction(-2) / (1 - d)
        assert g["pressure_0"] == 12 * d


def test_numeric_jacobian_matches_exact_at_uniform_occupancy():
    K = B.K_exact(Fraction(1, 4))
    Kn = B.numeric_jacobian(np.full(24, 0.25))
    exact = np.array([[float(Fraction(int(K[i, j].p), int(K[i, j].q))) for j in range(24)] for i in range(24)])
    assert np.abs(Kn - exact).max() < 1e-12


def test_equilibrium_reproduces_density_and_drift():
    occ = B.equilibrium(Fraction(1, 4), 0.1)
    V = np.array(H.VELOCITIES, dtype=float)
    assert abs(occ.sum() - 6.0) < 1e-12 and abs(occ @ V[:, 0] - 0.6) < 1e-12
