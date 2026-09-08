"""Exact invariant census (gate H0). No floats, no random draws."""
import json
import pytest

from phi_v2_lattice import recovery_hydro_invariants as H


def test_u_has_period_twelve_on_one_polarity():
    U, powers = H.u_powers()
    assert len(powers) == 12 and powers[0] == list(range(192))
    assert [U[i] for i in powers[11]] == list(range(192))


def test_layer_covariance_holds_exactly():
    assert H.covariance_violations() == 0


def test_fixed_kernel_is_population_only():
    kernel = H.fixed_kernel()
    assert kernel == ((1,) * 192,)


@pytest.mark.parametrize("l0", range(3))
def test_locked_kernel_dimension_is_seven(l0):
    assert len(H.locked_kernel(l0)) == 7


def test_tangent_lies_in_locked_span_but_not_fixed_span():
    assert H.span_membership(H.locked_kernel(0), H.tangent_rows())
    assert not H.span_membership(H.fixed_kernel(), H.tangent_rows())


def test_constant_plus_layer_zero_values_equal_locked_kernel_span():
    rows = ((1,) * 192,) + H.layer_rows(0)
    assert H.span_membership(H.locked_kernel(0), rows)
    assert H.span_membership(rows, H.locked_kernel(0))


@pytest.mark.parametrize("l0,power", [(2, 1), (1, 2)])
def test_locked_kernels_are_pullbacks_of_layer_zero_kernel(l0, power):
    # C_{q-1} = U C_q U^{-1}: the l0 kernel equals {w o U^{-power} : w in kernel(0)}.
    _, powers = H.u_powers()
    inverse = [0] * 192
    for i, j in enumerate(powers[power]):
        inverse[j] = i
    pulled = tuple(tuple(w[inverse[c]] for c in range(192)) for w in H.locked_kernel(0))
    assert H.span_membership(H.locked_kernel(l0), pulled)
    assert H.span_membership(pulled, H.locked_kernel(l0))


@pytest.mark.parametrize("name,xxxx,xxyy,iso", [
    ("body_diagonal_8", 8, 8, False),
    ("face_edge_18", 10, 4, False),
    ("moore_26", 18, 12, False),
    ("fchc_projected_18", 12, 4, True),
])
def test_fourth_rank_isotropy_table(name, xxxx, xxyy, iso):
    result = H.fourth_rank_isotropy(H.VELOCITY_SETS[name])
    assert (result.T_xxxx, result.T_xxyy, result.isotropic4) == (xxxx, xxyy, iso)
    assert result.isotropic2


def test_census_is_exact_and_serializable():
    census = H.census()
    encoded = json.dumps(census, sort_keys=True)
    assert census["fixed_dimension_per_polarity"] == 1
    assert census["locked_dimension_per_polarity"] == {"0": 7, "1": 7, "2": 7}
    assert census["covariance_violations"] == 0
    assert "float" not in encoded and "." not in json.dumps(census["isotropy"])
