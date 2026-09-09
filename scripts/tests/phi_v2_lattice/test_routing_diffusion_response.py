from fractions import Fraction

import flint
import pytest

from phi_v2_lattice import routing_diffusion_response as Q


def test_registered_intervals_cover_microscopic_to_heat_error():
    before=flint.ctx.prec
    report=Q.certificate()
    assert flint.ctx.prec==before
    assert len(report["probes"])==12
    assert all(p["certified_below_bound"] for p in report["probes"])
    assert {p["minimum_fresh_periodic_L"] for p in report["probes"]}=={49,193,769,3073}
    assert not report["large_lattices_materialized"]
    assert not report["interacting_fluid_continuum_recovered"]


def test_bound_combines_both_taylor_remainders_exactly():
    a=Fraction(1,8);time=Fraction(1,6)
    # For k=(1,1,1), sum purefourths=3 and ||Delta² cos||=9.
    assert Q.heat_error_bound(a,time,1,3,9)==Fraction(1,384)
    # N*(a^4/72)*(3+9) for N=64 is the same full error budget.
    assert Q.heat_error_bound(a,time,1,3,9)==64*a**4*Fraction(12,72)


def test_exact_endpoint_census_moments_and_path_mass():
    for t in range(6):
        counts=Q.endpoint_counts(t);total=6**t
        assert sum(counts.values())==total
        for j in range(3):
            assert sum(n*x[j] for x,n in counts.items())==0
            assert 3*sum(n*x[j]**2 for x,n in counts.items())==t*total
        assert all(sum(n*x[i]*x[j] for x,n in counts.items())==0 for i,j in ((0,1),(0,2),(1,2)))


@pytest.mark.parametrize("args", [(0,1,1,1,1),(1,-1,1,1,1),(1,1,0,1,1),(1,1,1,-1,1),
                                  (1,1,1,1,-1),(1.,1,1,1,1),(True,1,1,1,1)])
def test_bound_domain_rejected(args):
    with pytest.raises(ValueError):Q.heat_error_bound(*args)


@pytest.mark.parametrize("t", [True,-1,1.0,13])
def test_diagnostic_census_domain(t):
    with pytest.raises(ValueError):Q.endpoint_counts(t)
