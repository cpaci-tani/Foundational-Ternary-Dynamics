"""Exact finite operator regressions; no random trajectories or fitted targets."""
from fractions import Fraction

import numpy as np
import pytest

from phi_v2_lattice import channels as C, staged as P
from phi_v2_lattice import recovery_kinetic_reference as Ref
from phi_v2_lattice import recovery_kinetic_response as R


def probe(kind):
    h = [Fraction(0)]*192
    if kind == "uniform": return [Fraction(1)]*192
    h[0] = Fraction(1)
    if kind == "balanced": h[1] = Fraction(-1)
    return h


@pytest.mark.parametrize("layer",range(3))
def test_every_jacobian_column_is_actual_integer_pair_incidence_correction(layer):
    operator = R.collision_operator(layer)
    table = C.load_collision_tables()[layer]
    # Independently form incidence matrices and contract their difference.
    before = np.zeros((192,len(R.PAIRS)),dtype=np.int64)
    after = np.zeros_like(before)
    for column,pair in enumerate(R.PAIRS):
        before[list(pair),column] = 1
        after[list(table[pair]),column] = 1
    expected = (after-before) @ before.T
    np.testing.assert_array_equal(operator.integer_correction,expected)
    assert not expected.sum(axis=0).any()  # Conserved total-number derivative.
    assert not expected.sum(axis=1).any()  # Uniform reference-family tangent.
    jacobian = R.marginal_jacobian(layer)
    r = R.REFERENCE_P*(1-R.REFERENCE_P)**189
    for j in range(192):
        for i in range(192):
            assert jacobian[j][i] == (i == j)+r*int(expected[j,i])


@pytest.mark.parametrize("layer",range(3))
@pytest.mark.parametrize("kind",("single","balanced","uniform"))
def test_response_matches_direct_product_weight_derivative_on_every_pair(layer,kind):
    p,h = R.REFERENCE_P,probe(kind)
    table = C.load_collision_tables()[layer]
    target = table[(0,1)]
    total = sum(h)
    w = p*p*(1-p)**190
    direct_mean = h.copy()
    direct_raw = p*sum(h[c] for c in target)
    for source,out in table.items():
        selected = sum(h[c] for c in source)
        weight_derivative = w*(selected/p-(total-selected)/(1-p))
        for j in out: direct_mean[j] += weight_derivative
        for j in source: direct_mean[j] -= weight_derivative
        direct_raw += weight_derivative*((out == target)-(source == target))
    result = R.collision_response(layer,h)
    assert result.mean_derivative == tuple(direct_mean)
    covariance = direct_raw-p*sum(direct_mean[c] for c in target)
    assert dict(((j,k),v) for j,k,v in result.pair_covariances).get(target,0) == covariance
    assert sum(result.mean_derivative) == sum(h)
    assert sum(value for _,_,value in result.pair_covariances) == 0
    if kind == "uniform":
        assert result.mean_derivative == tuple(h)
        assert result.pair_covariances == ()
    elif kind == "single":
        assert result.maximum_pair_covariance > 0
        assert covariance != 0
    assert not result.iterated_closure_certified


def test_one_channel_finite_rational_secant_is_exact_and_has_covariance_remainder():
    p = R.REFERENCE_P; epsilon = p/2; table = C.load_collision_tables()[0]
    target = table[(0,1)]
    def finite(delta):
        means = [p+delta*(j == 0) for j in range(192)]
        raw = means[target[0]]*means[target[1]]
        for source,out in table.items():
            weight = ((p+delta)*p*(1-p)**190 if 0 in source
                      else p*p*(1-p-delta)*(1-p)**189)
            for j in out: means[j] += weight
            for j in source: means[j] -= weight
            raw += weight*((out == target)-(source == target))
        return means,raw-means[target[0]]*means[target[1]]
    plus,covplus = finite(epsilon)
    minus,covminus = finite(-epsilon)
    response = R.collision_response(0,probe("single"))
    assert tuple((a-b)/(2*epsilon) for a,b in zip(plus,minus)) == response.mean_derivative
    predicted = dict(((j,k),v) for j,k,v in response.pair_covariances)[target]
    assert (covplus-covminus)/(2*epsilon) == predicted
    # This is an exact polynomial identity, not a convergence test for a small
    # floating epsilon. The product of output means supplies the epsilon^2 term.
    assert covplus == epsilon*predicted-epsilon**2*response.mean_derivative[target[0]]*response.mean_derivative[target[1]]


@pytest.mark.parametrize("polarity",(-1,1))
def test_spatial_covariance_labels_match_actual_streaming_on_seam(polarity):
    table = C.load_collision_tables()[0];j,k = table[(0,1)]
    bank = np.zeros((27,384),dtype=bool)
    offset = 192 if polarity < 0 else 0
    bank[26,[j+offset,k+offset]] = True
    state = Ref.prepare_bank(bank,3)
    # Prepare a legal phase2 full record, then supply the registered selected
    # post-collision pair to test only actual streaming's spatial relabeling.
    state = Ref.phase_lift(state,2)
    state.lattice.bank[:] = bank
    after,_ = P.step(state)
    observed = tuple(sorted(zip(*np.nonzero(after.lattice.bank))))
    assert observed == R.streamed_pair(3,26,j,k,polarity)


def test_scalar_conservation_and_reported_counterexamples_use_actual_maps():
    checks = R.declared_moments()
    assert checks["constant"]["collision_invariant_layers"] == (True,True,True)
    assert checks["constant"]["streaming_invariant"]
    for name,row in checks.items():
        if name.startswith("tangent_"):
            assert row["streaming_counterexample"] is not None
    for layer in range(3):
        for component in range(6):
            assert checks[f"layer_{layer}_{component}"]["collision_invariant_layers"][layer]


def test_common_invariant_gram_is_positive_sum_of_exact_constraints():
    gram = np.array(R.common_additive_gram(),dtype=np.int64)
    np.testing.assert_array_equal(gram,gram.T)
    assert not gram.sum(axis=0).any()
    # A declared non-invariant moment has positive exact quadratic penalty.
    tangent = np.array([C.tangent(c)[0] for c in range(192)],dtype=np.int64)
    assert int(tangent @ gram @ tangent) > 0


def test_complete_exact_common_additive_kernel_contains_only_polarity_counts():
    classification = R.common_additive_classification()
    assert classification["constraint_rank"] == 191
    assert classification["primitive_integer_basis"] == ((1,)*192,)
    assert classification["dimension_both_polarities"] == 2
    assert not classification["time_dependent_or_nonlinear_invariants_classified"]


@pytest.mark.parametrize("bad",(True,0.5,Fraction(0),Fraction(1)))
def test_response_rejects_inexact_or_boundary_reference(bad):
    with pytest.raises(ValueError): R.collision_response(0,probe("single"),bad)


def test_invalid_channel_probe_and_domain_fail_closed():
    with pytest.raises(ValueError): R.collision_response(0,[0]*191)
    with pytest.raises(ValueError): R.collision_response(3,probe("single"))
    with pytest.raises(ValueError): R.streamed_pair(2,0,0,1)
    with pytest.raises(ValueError): R.streamed_pair(3,0,0,0)
    R.collision_operator(np.int64(1))
    with pytest.raises(ValueError): R.collision_operator(True)
    with pytest.raises(ValueError): R.streamed_pair(3,0,0,1,1.0)


@pytest.mark.parametrize("layer",range(3))
def test_full_boolean_leakage_matches_sparse_score_difference_not_only_pair_covariances(layer):
    p,h = R.REFERENCE_P,probe("single")
    table = C.load_collision_tables()[layer]
    score_change_norm = (1-p)**188*sum(
        (sum(h[i] for i in before)-sum(h[i] for i in after))**2
        for before,after in table.items())
    projected = R.projected_collision_probe(layer,h)
    retained_change = R.score_norm_squared(tuple(a-b for a,b in zip(projected,h)),p)
    leakage = R.collision_leakage_squared(layer,h)
    assert leakage == score_change_norm-retained_change > 0
    result = R.collision_response(layer,h)
    pair_projection_norm = sum((value*value for _,_,value in result.pair_covariances),Fraction(0))/(p*(1-p))**2
    assert leakage > pair_projection_norm  # Strictly positive orders above two.
    assert R.collision_leakage_squared(layer,probe("uniform")) == 0


@pytest.mark.parametrize("value",(Fraction(0),Fraction(1),Fraction(2),Fraction(1,7),Fraction(10**30,3)))
def test_rational_square_root_enclosures_are_proven_by_integer_inequalities(value):
    bound = R.sqrt_upper(value,10**6)
    assert bound*bound >= value
    if value:
        assert (bound-Fraction(1,10**6))**2 < value
    else:
        assert bound == 0
    with pytest.raises(ValueError): R.sqrt_upper(-1)


def test_homogeneous_budget_uses_all_physical_phases_and_exact_telescoping_norm_loss():
    for start in range(4):
        for horizon in range(9):
            result = R.homogeneous_tangent_budget(3,horizon,probe("single"),start_tick=start,layer=start%3)
            expected = tuple(t for t in range(start,start+horizon) if t%4 == 1)
            assert tuple(t for t,_ in result.collision_leakages_squared) == expected
            assert result.final_layer == (start%3-len(expected))%3
            assert result.initial_score_norm_squared-result.final_projected_norm_squared == sum(
                (value for _,value in result.collision_leakages_squared),Fraction(0))
            assert result.duhamel_error_upper == sum(
                (R.sqrt_upper(value) for _,value in result.collision_leakages_squared),Fraction(0))
            assert not result.finite_amplitude_or_hydrodynamics_certified
    exact = R.homogeneous_tangent_budget(3,8,probe("uniform"))
    assert exact.norm_error_upper == 0
    no_collision = R.homogeneous_tangent_budget(3,1,probe("single"),start_tick=2)
    expected_probe = [Fraction(0)]*192;expected_probe[C.U(0)] = 1
    assert no_collision.final_projected_probe == tuple(expected_probe)


def test_negative_leakage_is_rejected_instead_of_clamped(monkeypatch):
    monkeypatch.setattr(R,"projected_collision_probe",lambda layer,h,p: tuple(2*x for x in h))
    with pytest.raises(ValueError,match="negative leakage"):
        R.collision_leakage_squared(0,probe("single"))
    with pytest.raises(ValueError,match="negative leakage"):
        R.homogeneous_tangent_budget(3,2,probe("single"))


def test_duhamel_bound_against_exact_full_score_iteration_on_eight_state_model():
    # Independent finite Hilbert-space countercheck of the general inequality.
    # This three-bit model is NOT the FTD collision law. Its only nontrivial
    # permutation swaps two equally weighted exactly-two states, so it retains
    # higher Boolean correlations which a first-degree projection discards.
    p = R.REFERENCE_P
    states = range(8)
    weights = [p**x.bit_count()*(1-p)**(3-x.bit_count()) for x in states]
    initial = [(Fraction(x&1)-p)/(p*(1-p)) for x in states]
    actual,approximate = initial.copy(),initial.copy()
    permutation = [0,1,2,6,4,5,3,7]  # 011 <-> 110 changes the selected bit0.
    def norm(values): return sum((w*v*v for w,v in zip(weights,values)),Fraction(0))
    def project(values):
        coefficients = [sum((weights[x]*values[x]*(Fraction((x>>i)&1)-p) for x in states),Fraction(0)) for i in range(3)]
        return [sum((coefficients[i]*(Fraction((x>>i)&1)-p)/(p*(1-p)) for i in range(3)),Fraction(0)) for x in states]
    budget = Fraction(0)
    for _ in range(8):
        actual = [actual[permutation[x]] for x in states]
        evolved = [approximate[permutation[x]] for x in states]
        approximate = project(evolved)
        residual = [a-b for a,b in zip(evolved,approximate)]
        budget += R.sqrt_upper(norm(residual))
        assert norm([a-b for a,b in zip(actual,approximate)]) <= budget*budget
    assert actual == initial  # Actual discarded modes return; never reset them.
    assert approximate != initial


@pytest.mark.parametrize("copies",(1,2))
def test_product_density_remainder_equals_full_small_bernoulli_enumeration(copies):
    # Generic3/6-bit algebraic proof fixture, not an FTD trajectory ensemble.
    p,epsilon = Fraction(1,3),Fraction(1,10)
    local = (Fraction(1),Fraction(-1),Fraction(1,2))
    h = local*copies
    norm = Fraction(0)
    for state in range(1<<len(h)):
        bits = [(state>>i)&1 for i in range(len(h))]
        reference,prepared = Fraction(1),Fraction(1)
        for bit,value in zip(bits,h):
            shifted = p+epsilon*value
            reference *= p if bit else 1-p
            prepared *= shifted if bit else 1-shifted
        score = sum((value*(bit-p)/(p*(1-p)) for value,bit in zip(h,bits)),Fraction(0))
        remainder = prepared/reference-1-epsilon*score
        norm += reference*remainder*remainder
    assert norm == R.product_density_remainder_squared(local,epsilon,p=p,copies=copies)


def test_finite_amplitude_bound_combines_exact_all_site_remainder_and_tangent_budget():
    h,epsilon = probe("single"),R.REFERENCE_P/2
    result = R.homogeneous_finite_amplitude_budget(3,8,h,epsilon)
    assert result.initial_product_remainder_squared == R.product_density_remainder_squared(h,epsilon,copies=27)
    assert result.initial_product_remainder_upper**2 >= result.initial_product_remainder_squared
    assert result.transported_tangent_error_upper == abs(epsilon)*result.tangent.norm_error_upper
    assert result.full_density_error_upper == result.initial_product_remainder_upper+result.transported_tangent_error_upper
    assert "may be signed" in result.comparison
    assert result.exact_finite_amplitude_bound
    assert not result.nonlinear_kinetic_or_hydrodynamic_closure_certified


def test_zero_amplitude_zero_probe_and_uniform_reference_family_are_honest():
    assert R.homogeneous_finite_amplitude_budget(3,8,probe("single"),0).full_density_error_upper == 0
    assert R.homogeneous_finite_amplitude_budget(3,8,[0]*192,Fraction(100)).full_density_error_upper == 0
    initial = R.homogeneous_finite_amplitude_budget(3,0,probe("uniform"),Fraction(1,192))
    later = R.homogeneous_finite_amplitude_budget(3,8,probe("uniform"),Fraction(1,192))
    assert later.transported_tangent_error_upper == 0
    assert later.full_density_error_upper == initial.full_density_error_upper > 0
    # The exact new product reference is stationary; its linear truncation is
    # still missing nonlinear Boolean terms and thus has a nonzero remainder.


def test_finite_amplitude_endpoint_occupancy_is_allowed_but_invalid_probabilities_are_not():
    p=R.REFERENCE_P
    assert R.product_density_remainder_squared([1],-p) == 0
    assert R.product_density_remainder_squared([1],1-p) == 0
    assert R.product_density_remainder_squared([1],-p,copies=2) > 0
    for epsilon in (-2*p,1,p/2+0.0):
        with pytest.raises(ValueError): R.product_density_remainder_squared([1],epsilon)
    with pytest.raises(ValueError): R.product_density_remainder_squared([1],p,copies=0)
    with pytest.raises(ValueError): R.product_density_remainder_squared([True],p)
