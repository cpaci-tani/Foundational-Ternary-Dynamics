"""Exact operator and actual-law counterexamples; no statistical campaigns."""
from dataclasses import replace
from fractions import Fraction
from itertools import combinations

import pytest
from sympy import I, Matrix, Rational, expand, symbols

from phi_v2_lattice import channels as C, continuum_contract as R


@pytest.fixture(scope="module")
def tables():
    return C.load_collision_tables()


def test_half_reference_probability_matches_explicit_small_census():
    for n in range(2, 9):
        hits = sum(mask.bit_count() == 2 for mask in range(2 ** n))
        assert R.half_occupation_collision_probability(n) == Fraction(hits, 2 ** n)
    assert R.half_occupation_collision_probability() == Fraction(18336, 2 ** 192)


def test_exact_budget_is_conditional_and_stays_small_without_sampling():
    budget = R.half_reference_event_budget(10 ** 18)
    assert budget.expected_events == Fraction(18336 * 10 ** 18, 2 ** 192)
    assert budget.probability_any_upper_bound < Fraction(1, 10 ** 35)
    assert "each counted opportunity" in budget.assumption
    assert R.half_reference_event_budget(0).expected_events == 0
    assert R.half_reference_event_budget(2 ** 192).probability_any_upper_bound == 1


@pytest.mark.parametrize("bad", [-1, True, 1.5])
def test_invalid_event_counts_rejected(bad):
    with pytest.raises(ValueError):
        R.half_reference_event_budget(bad)


def test_metadata_cannot_conflate_trajectory_and_tangent():
    identity = R.EvolutionIdentity(
        R.EvolutionKind.MICROSCOPIC_TRAJECTORY, "Phi-v2", C.COLLISION_HASH,
        "spec-section-5", "counting-L2", 12, "layer-E-B", "original-global-tick",
        "width-2-channel-sums")
    assert replace(identity, kind=R.EvolutionKind.MARGINAL_TANGENT) != identity
    with pytest.raises(ValueError):
        replace(identity, kind="some simulation")
    with pytest.raises(ValueError):
        replace(identity, tick_unit_id="")
    with pytest.raises(ValueError):
        replace(identity, horizon_ticks=-1)
    assert not R.selected_v2_disposition().trajectory_continuum_certified
    assert len(R.selected_v2_disposition().missing_certificates) == 4


def test_projected_one_step_equality_does_not_control_iteration():
    full, project = R.projection_iteration_counterexample()
    reduced_lift = project * full * project
    assert full.T * full == Matrix.eye(2)  # even a norm-one contraction
    assert reduced_lift == Matrix.zeros(2)
    assert project * full ** 2 * project == project
    assert project * full ** 2 * project != reduced_lift ** 2
    assert (Matrix.eye(2) - project) * full * project != Matrix.zeros(2)


@pytest.mark.parametrize("layer", [0, 1, 2])
def test_actual_collision_creates_nonproduct_tangent(tables, layer):
    table = tables[layer]
    witness = R.product_tangent_witness(table)
    assert table[witness.source_pair] == witness.target_pair
    assert table[witness.target_pair] == witness.source_pair
    assert witness.score_values == (0, 0, 0, 1)
    assert witness.mixed_difference == 1
    # Evaluate actual perturbed input density through the inverse collision.
    eps = Fraction(1, 8)
    c, d = witness.target_pair
    output_density = []
    for output in ((), (c,), (d,), (c, d)):
        before = table[tuple(sorted(output))] if len(output) == 2 else output
        occupied = witness.perturbed_channel in before
        p = Fraction(1, 2) + eps
        density = (2 * p if occupied else 2 * (1 - p))
        output_density.append(density)
    assert output_density[3] - output_density[1] - output_density[2] + output_density[0] == 4 * eps


def test_small_product_family_has_actual_output_covariance(tables):
    witness = R.product_tangent_witness(tables[0])
    a, b = witness.source_pair
    outcomes = ((), (a,), (b,), witness.target_pair)
    c, d = witness.target_pair
    mean_c = Fraction(sum(c in state for state in outcomes), 4)
    mean_d = Fraction(sum(d in state for state in outcomes), 4)
    joint = Fraction(sum(c in state and d in state for state in outcomes), 4)
    assert joint - mean_c * mean_d in (Fraction(1, 8), Fraction(3, 16))


def test_frozen_collision_really_preserves_all_slow_readouts(tables):
    # All rows, all layers: necessary input for collision cancellation in the
    # projected first-order moment, independently of the parent PASS strings.
    for layer, table in enumerate(tables):
        values = [C.layer_value_of(c, layer) for c in range(C.N_STATES)]
        assert len(table) == len(tuple(combinations(range(C.N_STATES), 2)))
        for (a, b), (c, d) in table.items():
            assert tuple(x + y for x, y in zip(values[a], values[b])) == tuple(
                x + y for x, y in zip(values[c], values[d]))


def test_reconstructed_operator_has_declared_transverse_and_scalar_blocks():
    axes = R.projected_first_order_axes()
    kx, ky, kz, lam = symbols("kx ky kz lam")
    generator = kx * axes[0] + ky * axes[1] + kz * axes[2]
    k2 = kx ** 2 + ky ** 2 + kz ** 2
    expected = lam * (lam ** 2 + k2 / 27) * (lam ** 2 + k2 / 36) ** 2
    assert expand(generator.charpoly(lam).as_expr() - expected) == 0
    transverse = axes[2].extract((1, 2, 4, 5), (1, 2, 4, 5))
    assert transverse ** 2 == -Rational(1, 36) * Matrix.eye(4)
    assert axes[2].extract((0, 3), (0, 3)) == Matrix([[0, -I / 3], [-I / 9, 0]])
    # Arbitrary-direction transverse states stay transverse at first order.
    e = Matrix([1, -1, 0])
    b = Matrix([1, 1, -2])
    state = Matrix([0, *e, *b])
    result = (axes[0] + axes[1] + axes[2]) * state
    assert result[0] == sum(result[1:4]) == sum(result[4:7]) == 0
