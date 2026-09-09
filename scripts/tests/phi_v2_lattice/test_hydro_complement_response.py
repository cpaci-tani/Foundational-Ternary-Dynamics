"""Independent reduced stress checks for the full rational kinetic inverse."""
from fractions import Fraction

import pytest

from phi_v2_lattice import hydro_complement as H
from phi_v2_lattice import hydro_complement_response as R


def test_exact_reduced_stress_blocks():
    Q = H.collision_gram()
    f, e, s = [], [], []
    for x, y, z in H.VELOCITIES:
        diagonal = x * x - y * y
        f.append(diagonal if x*x + y*y + z*z == 1 else 0)
        e.append(diagonal if x*x + y*y + z*z == 2 else 0)
        s.append(x * y)
    apply = lambda v: [sum(q * w for q, w in zip(row, v)) for row in Q]
    assert apply(s) == [35712 * x for x in s]
    assert apply(f) == [31264 * x - 80 * y for x, y in zip(f, e)]
    assert apply(e) == [-80 * x + 35632 * y for x, y in zip(f, e)]
    assert sum(x*x for x in f) == sum(x*x for x in e) == 8
    assert sum(x*y for x, y in zip(f, e)) == 0


@pytest.mark.parametrize("p", [Fraction(1, 2), Fraction(1, 3), Fraction(1, 4)])
def test_conditional_anisotropy_has_exact_density_independent_sign(p):
    c = p ** 11 * (1-p) ** 11 / 2
    A = Fraction(1, 107136)
    D = Fraction(1397, 139249056)
    axis = R.shear_rate((1, 0, 0), (0, 1, 0), p)
    diagonal = R.shear_rate((1, 1, 0), (1, -1, 0), p)
    body = R.shear_rate((1, 1, 1), (1, -1, 0), p)
    assert axis == A/c-Fraction(1, 6)
    assert diagonal == D/c-Fraction(1, 6)
    assert body == (A+2*D)/(3*c)-Fraction(1, 6)
    assert diagonal-axis == (D-A)/c > 0
    for n, (t, u) in R.PROBES:
        assert R.shear_form(n, t, u, p) == 0


def test_euclidean_direction_and_transverse_normalisation():
    p = Fraction(1, 2)
    assert R.shear_rate((2, 2, 0), (3, -3, 0), p) == R.shear_rate((1, 1, 0), (1, -1, 0), p)
    report = R.certificate()
    assert report["isotropic_shear_gate"] is False
    assert report["nonconserved_jacobian_eigenvalue_lower_bound"] == "7541/8192"
    assert report["probes"][0]["rates_per_cycle"] == ["130793/1674"] * 2
    assert report["probes"][1]["rates_per_cycle"] == ["730979825/8703066", "130793/1674"]
    assert report["probes"][2]["rates_per_cycle"] == ["597603427573/7284466242"] * 2


@pytest.mark.parametrize("n,t,p", [((0,0,0),(1,0,0),Fraction(1,2)),
    ((1,0,0),(1,0,0),Fraction(1,2)),((1,0,0),(0,True,0),Fraction(1,2)),
    ((1,0,0),(0,1,0),0.5),((1,0,0),(0,1,0),Fraction(0))])
def test_invalid_shear_probe_rejected(n, t, p):
    with pytest.raises(ValueError):
        R.shear_rate(n, t, p)
