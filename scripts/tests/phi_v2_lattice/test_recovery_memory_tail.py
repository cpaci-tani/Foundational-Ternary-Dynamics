"""Synthetic arithmetic controls only; never load the accepted full-K1 data."""
from dataclasses import replace
from fractions import Fraction as F

import numpy as np
import pytest

from phi_v2_lattice import recovery_memory_tail as M


def toy_prepared():
    zero = tuple((0,)*24 for _ in range(24))
    identity = tuple(tuple((1 << 24) if a == i else 0 for i in range(24)) for a in range(24))
    return M.Prepared(M.LABELS, identity, tuple((d, zero) for d in M.DELTA_SUPPORT), "0/1", ("0/1",)*24)


def zero_records():
    return [{"schema": "strict-memory-tail-coefficient-1", "law_id": M.LAW_ID, "index": i,
             "displacement": list(h), "denominator_exponent": 864, "independent_equal": True,
             "numerators": [["0"]*24 for _ in range(24)], "counters": M.counters(i)}
            for i, h in enumerate(M.SUPPORT)]


def test_geometry_complete_not_selected_by_coefficients():
    g = M.geometry()
    assert tuple(map(len, (g["labels"], M.V, g["delta_support"], g["nonzero_candidate_support"], g["support"]))) == (24, 18, 93, 33, 185)
    assert len(M.RANGES) == 64
    assert [i for a, b in M.RANGES for i in range(a, b)] == list(range(185))
    assert len([h for h in M.SUPPORT if max(map(abs, h)) <= 2]) == 125
    assert len([h for h in M.SUPPORT if max(map(abs, h)) == 3]) == 54
    assert sum(M.counters(i)["primary_products"] for i in range(185)) == 15054336
    assert sum(M.counters(i)["reference_products"] for i in range(185)) == 15054336
    assert sum(M.counters(i)["primary_j_products"] for i in range(185)) == 13824


@pytest.mark.parametrize("bad", [True, 0.0, "1", None, np.float64(1)])
def test_integer_rejects_noninteger_types(bad):
    with pytest.raises(ValueError):
        M.integer(bad)


@pytest.mark.parametrize("bad", ["+1", "01", "-0", " 1", "1e2", "", "1\n"])
def test_decimal_encoding_is_exact(bad):
    with pytest.raises(ValueError):
        M.decimal(bad)


@pytest.mark.parametrize("bad", ["0/2", "2/4", "1/-2", "1/0", "1", "1.0/2", "01/2"])
def test_rational_encoding_rejects_unreduced_or_ambiguous_values(bad):
    with pytest.raises(ValueError):
        M.rational(bad)


@pytest.mark.parametrize("raw", [b'{"a":1,"a":2}', b'{"a":NaN}', b'{"a":Infinity}'])
def test_json_rejects_ambiguous_input(raw):
    with pytest.raises(ValueError):
        M.read_json(raw)


def test_prepared_codec_roundtrip_and_no_mutable_aliases():
    p = toy_prepared()
    data = p.mapping()
    restored = M.restore_prepared(M.read_json(M.canonical_bytes(data)))
    assert restored == p
    data["delta"][0]["numerators"][0][0] = "99"
    data["labels"][0][0] = 99
    assert restored == p
    with pytest.raises(ValueError):
        replace(p, jnum=list(p.jnum))


@pytest.mark.parametrize("mutation", ["duplicate", "shape", "float", "structural", "denominator", "bound"])
def test_prepare_input_rejection(mutation):
    p = toy_prepared().mapping()
    c = {"schema": "strict-full-memory-certificate-1", "law_id": M.LAW_ID,
         "Gamma": "0/1", "impulse_PSD_pivots": ["0/1"]*24,
         "delta": [{"displacement": r["displacement"], "matrix": [["0/1"]*24 for _ in range(24)]} for r in p["delta"]]}
    t = {"labels": [list(q) for q in M.LABELS], "jacobian_denominator": 1 << 24,
         "jacobian_numerator": [list(row) for row in toy_prepared().jnum]}
    if mutation == "duplicate": c["delta"][1]["displacement"] = c["delta"][0]["displacement"]
    elif mutation == "shape": c["delta"][0]["matrix"].pop()
    elif mutation == "float": t["jacobian_numerator"][0][0] = float(1 << 24)
    elif mutation == "structural": c["delta"][0]["matrix"][0][0] = "1/1"
    else:
        r = next(r for r in c["delta"] if tuple(r["displacement"]) == (0, 0, 0))
        r["matrix"][0][0] = "1/3" if mutation == "denominator" else "3/1"
    with pytest.raises(ValueError):
        M.prepare(c, t)


def test_two_contractions_with_nonsymmetric_entry_and_displacement_sign():
    p = toy_prepared()
    values = dict(p.delta)
    left, right = [list(row) for row in values[(-2, 0, 0)]], [list(row) for row in values[(2, 0, 0)]]
    left[3][5], right[3][11] = -7, 13
    values[(-2, 0, 0)], values[(2, 0, 0)] = tuple(map(tuple, left)), tuple(map(tuple, right))
    p = replace(p, delta=tuple(sorted(values.items())))
    for h, entry in (((4, 0, 0), (5, 11)), ((-4, 0, 0), (11, 5))):
        index = M.SUPPORT.index(h)
        row = M.coefficient_range(p, index, index+1)[0]
        m = M.validate_coefficient(row, index)
        assert m[entry[0]][entry[1]] == 91
        assert sum(abs(x) for r in m for x in r) == 91
        assert row["counters"]["site_pairs"] == 1


def test_central_gram_is_transposed_and_integer_aligned():
    p = toy_prepared()
    j = [[0]*24 for _ in range(24)]
    j[7][3], j[7][9] = 2, -5
    p = replace(p, jnum=tuple(map(tuple, j)))
    i = M.SUPPORT.index((0, 0, 0))
    row = M.coefficient_range(p, i, i+1)[0]
    m = M.validate_coefficient(row)
    assert m[3][9] == m[9][3] == 10 << 816
    assert m[3][3] == ((1 << 48)-4) << 816
    assert m[7][7] == 1 << 864


def test_reference_disagreement_prevents_emission(monkeypatch):
    p = toy_prepared()
    seen = []
    wrong = [[0]*24 for _ in range(24)]
    wrong[0][0] = 1
    monkeypatch.setattr(M, "_reference", lambda *args: tuple(map(tuple, wrong)))
    with pytest.raises(AssertionError, match="disagree"):
        M.coefficient_range(p, 0, 1, seen.append)
    assert not seen


def test_gaussian_fourier_orientation_is_negative_exponent():
    a = np.zeros((24, 24), dtype=object)
    a[2, 5] = 19
    real, imag = M._fourier({(1, 0, 0): a}, (1, 0, 0))
    assert not any(real.ravel()) and imag[2, 5] == -19
    real, imag = M._fourier({(-1, 0, 0): a}, (1, 0, 0))
    assert imag[2, 5] == 19


def test_all_zero_synthetic_certificate_all_entries_and_horizons():
    cert = M.certificate(toy_prepared(), zero_records())
    assert len(cert["W2"]) == 185 and len(cert["fourier_controls"]) == 7
    assert len(cert["stress_budgets"]) == 6 and len(cert["finite_horizons"]) == 13
    assert cert["stability"] == "PASS_SUFFICIENT_UNIFORM_C1"
    assert all(r["remaining2"] == "0/1" for r in cert["stress_budgets"])
    assert [r["normalization"] for r in cert["stress_budgets"]] == [12, 12, 48, 24, 72, 216]
    assert all(r["uniform_k_squared_bound"] == "0/1" for r in cert["finite_horizons"])
    assert not cert["uniform_absolute_memory_tail"] and not cert["continuum_recovered"]
    scope = cert["physical_time_prerequisites"]
    assert scope["selected_scale_calibration"] is None and scope["evaluated"] is False
    assert scope["energy_tail_sufficient_condition"].startswith("C^2 Tr W_m=o(tau^3)")
    assert scope["absolute_tail_sufficient_condition"] == "C R_m=o(tau)"
    assert scope["unresolved_initial_score"] == "y0=0"
    assert cert["initial_unresolved_score"].startswith("y0=0;")


@pytest.mark.parametrize("n", [0, 1, 2, 3, 4, 8, 16, 1024])
def test_horizon_multiplicity_is_sum_of_integer_squares(n):
    assert M.horizon_factor(n) == sum(i*i for i in range(1, max(1, n-2)))


def test_corrupted_complete_coefficient_is_detected():
    rows = zero_records()
    rows[0]["numerators"][0][0] = "1"
    with pytest.raises(AssertionError):
        M.observe(toy_prepared(), rows)
    rows = zero_records()
    rows[0]["counters"]["site_pairs"] = True
    with pytest.raises(ValueError):
        M.validate_coefficient(rows[0])
    with pytest.raises(ValueError):
        M.observe(toy_prepared(), rows[:-1])


def test_exact_fraction_normalization_has_no_float_division():
    assert M.fraction_string(F(1, 24)) == "1/24"
    with pytest.raises(ValueError):
        M.fraction_string(1/24)


@pytest.mark.parametrize("field", ["j_bound", "delta_bound", "structural"])
def test_direct_prepared_constructor_obeys_same_finite_domain(field):
    p = toy_prepared()
    if field == "j_bound":
        j = [list(row) for row in p.jnum]
        j[0][0] = (1 << 24)+1
        with pytest.raises(ValueError):
            replace(p, jnum=tuple(map(tuple, j)))
    else:
        rows = dict(p.delta)
        d = (0, 0, 0) if field == "delta_bound" else M.DELTA_SUPPORT[0]
        m = [list(row) for row in rows[d]]
        m[0][0] = (1 << 433)+1 if field == "delta_bound" else 1
        rows[d] = tuple(map(tuple, m))
        with pytest.raises(ValueError):
            replace(p, delta=tuple(sorted(rows.items())))


def test_nonzero_exact_projector_toy_stress_and_unavailable_C1_bound():
    # E=Enum/24; J=E+(I-E)/4 and Delta(0)=3(I-E)/8.
    # Thus W2=51(I-E)/64. These are synthetic rational operators, not a
    # new parity result. All six stress budgets follow from rank4 isotropy.
    enum = tuple(tuple(1+2*sum(M.VELOCITIES[a][j]*M.VELOCITIES[i][j] for j in range(3))
                       for i in range(24)) for a in range(24))
    q = tuple(tuple((24 if a == i else 0)-enum[a][i] for i in range(24)) for a in range(24))
    j = tuple(tuple(((8 if a == i else 0)+enum[a][i]) << 19 for i in range(24)) for a in range(24))
    delta = tuple(tuple(x << 426 for x in row) for row in q)
    expected = tuple(tuple(17*x*(1 << 855) for x in row) for row in q)
    pivots = M.UPSTREAM._psd_exact([[F(x, M.DEN) for x in row] for row in expected])
    gamma = F(max(sum(abs(x) for x in row) for row in q), 64)
    zero = tuple((0,)*24 for _ in range(24))
    p = M.Prepared(M.LABELS, j, tuple((d, delta if d == (0, 0, 0) else zero) for d in M.DELTA_SUPPORT),
                   M.fraction_string(gamma), tuple(map(M.fraction_string, pivots)))
    i = M.SUPPORT.index((0, 0, 0))
    actual = M.coefficient_range(p, i, i+1)[0]
    assert M.validate_coefficient(actual) == expected
    rows = zero_records()
    rows[i] = actual
    cert = M.certificate(p, rows)
    assert cert["stability"] == "NOT_ESTABLISHED_BY_THIS_BOUND"
    assert all((r["injected"], r["returned1"], r["remaining2"]) == ("5/16", "3/64", "17/64") for r in cert["stress_budgets"])
    assert all(r["trace"] == "255/16" for r in cert["fourier_controls"])
    assert cert["uniform_injection_trace"] == "75/4"
    assert all(not r["C1_bound_established"] for r in cert["finite_horizons"])
    n4 = next(r for r in cert["finite_horizons"] if r["cycles"] == 4)
    assert n4["squared_bound_by_control"]["zero"] == "255/16"


def test_numpy_integrals_are_normalized_before_object_arithmetic():
    p = toy_prepared()
    zero = tuple((0,)*24 for _ in range(24))
    values = dict(p.delta)
    m = [list(row) for row in zero]
    m[0][0] = np.uint64(1 << 40)
    values[(np.uint64(0), np.uint64(0), np.uint64(0))] = tuple(map(tuple, m))
    p = replace(p, labels=tuple(tuple(np.int64(x) for x in q) for q in M.LABELS),
                jnum=zero, delta=tuple(sorted(values.items())))
    assert type(dict(p.delta)[(0, 0, 0)][0][0]) is int
    assert all(type(x) is int for q in p.labels for x in q)
    assert all(type(x) is int for d, _ in p.delta for x in d)
    index = M.SUPPORT.index((0, 0, 0))
    with np.errstate(over="raise"):
        row = M.coefficient_range(p, index, index+1)[0]
    assert M.validate_coefficient(row)[0][0] == (1 << 864)-(1 << 80)
