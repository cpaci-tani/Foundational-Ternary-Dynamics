from fractions import Fraction
from itertools import product

import flint
import numpy as np
import pytest

from phi_v2_lattice import recorded_routing as R
from phi_v2_lattice import recorded_routing_response as Q
from phi_v2_lattice import routing_diffusion_response as OLD


def test_registered_twelve_intervals_equal_predecessor_and_cover_all_eight_directions():
    before = flint.ctx.prec
    report = Q.certificate()
    assert flint.ctx.prec == before
    assert report['probes'] == OLD.certificate()['probes']
    assert len(report['probes']) == 12 and report['directions_covered_per_probe'] == 8
    assert all(p['certified_below_bound'] for p in report['probes'])
    assert report['corner_vectors'] == [list(d) for d in R.CORNERS]
    assert not report['large_lattices_materialized']
    assert not report['complete_v3_law_approved']
    assert not report['interacting_fluid_continuum_recovered']


@pytest.mark.parametrize("module", [Q, OLD], ids=["recorded", "routing"])
@pytest.mark.parametrize("ambient_precision", [79, 257])
@pytest.mark.parametrize("fail", [False, True], ids=["success", "exception"])
def test_certificate_uses_exact_precision_and_restores_caller_context(
        module, ambient_precision, fail, monkeypatch):
    previous_precision = flint.ctx.prec
    original_bound = module.heat_error_bound
    observed_precision = []

    def check_precision(*args):
        observed_precision.append(flint.ctx.prec)
        if fail:
            raise RuntimeError("comparison interrupted")
        return original_bound(*args)

    monkeypatch.setattr(module, "heat_error_bound", check_precision)
    flint.ctx.prec = ambient_precision
    try:
        if fail:
            with pytest.raises(RuntimeError, match="comparison interrupted"):
                module.certificate()
        else:
            report = module.certificate()
            assert report["precision_bits"] == 192
            assert len(report["probes"]) == 12
        assert observed_precision == [192] * (1 if fail else 12)
        assert flint.ctx.prec == ambient_precision
    finally:
        flint.ctx.prec = previous_precision


@pytest.mark.parametrize("code", range(8))
def test_exact_signed_freshness_inequality_and_word_probability(code):
    D = R.CORNERS[code]
    T, L = 4, 13
    for h in range(1, T+1):
        for delta in product(range(-h, h+1), repeat=3):
            if sum(abs(x) for x in delta) > h: continue
            z = tuple(D[j]*delta[j]+2*h for j in range(3))
            assert all(h <= x <= 3*h < L and x % L != 0 for x in z)
    assert Q.path_probability(L, T, code) == Fraction(1, 6**T)
    with pytest.raises(ValueError): Q.path_probability(12, T, code)


def test_endpoint_count_moments_and_exact_full_error_bound():
    for T in range(6):
        counts = Q.endpoint_counts(T)
        assert sum(counts.values()) == 6**T
        for j in range(3):
            assert sum(n*x[j] for x,n in counts.items()) == 0
            assert 3*sum(n*x[j]**2 for x,n in counts.items()) == T*6**T
        for i,j in ((0,1), (0,2), (1,2)):
            assert sum(n*x[i]*x[j] for x,n in counts.items()) == 0
    a = Fraction(1,8)
    assert Q.heat_error_bound(a,Fraction(1,6),1,3,9) == Fraction(1,384)
    assert Q.heat_error_bound(a,Fraction(1,6),1,3,9) == 64*a**4*Fraction(12,72)


@pytest.mark.parametrize("code", range(8))
def test_mean_prediction_uses_valid_complete_background_and_is_owned(code):
    initial = np.zeros((11,11,11,2), dtype=np.int64)
    initial[5,5,5,0] = 1
    background = R.seed_background(11,code)
    mean = Q.predict_densities(initial,3,background)
    for point,count in Q.endpoint_counts(3).items():
        assert mean[tuple(5+x for x in point)+(0,)] == pytest.approx(count/216)
    assert initial.sum() == 1 and mean.sum() == pytest.approx(1)
    assert not np.shares_memory(initial,mean)
    with pytest.raises(ValueError): Q.predict_densities(initial,4,background)
    for bad in (initial.astype(complex),initial.astype('timedelta64[D]')):
        with pytest.raises(ValueError): Q.predict_densities(bad,1,background)
    background[0,0,0] = (code+1)%8
    with pytest.raises(ValueError): Q.predict_densities(initial,1,background)


@pytest.mark.parametrize("args", [(0,1,1,1,1),(1,-1,1,1,1),(1,1,0,1,1),(1,1,1,-1,1),
                                  (1,1,1,1,-1),(1.,1,1,1,1),(True,1,1,1,1)])
def test_bound_domain_rejected(args):
    with pytest.raises(ValueError): Q.heat_error_bound(*args)


@pytest.mark.parametrize("t", [True,-1,1.0,13])
def test_diagnostic_census_domain(t):
    with pytest.raises(ValueError): Q.endpoint_counts(t)
