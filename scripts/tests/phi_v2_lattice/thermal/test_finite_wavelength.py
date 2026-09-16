"""Exact transfer and leakage certificates; these are not transport tests."""
from fractions import Fraction as Q
import gzip
from hashlib import sha256
from itertools import permutations, product
import json
from pathlib import Path

from flint import fmpz, fmpz_mat, fmpz_poly
import pytest

from scripts.phi_v2_lattice.thermal import finite_wavelength as F
from scripts.phi_v2_lattice.thermal import runtime_v2 as R
from scripts.phi_v2_lattice.thermal.analysis import shear_tangent_leakage
from scripts.experiments import build_thermal_candidate2_finite_wavelength_evidence as E


@pytest.fixture(scope="module")
def collision():
    return F.collide(F.initial_scores())


def test_entire_integer_factorization_agrees_with_existing_rational_operator():
    from scripts.experiments.build_thermal_candidate2_collision_operator_evidence import build_operator
    _, variance, K, M, moved = build_operator()
    factors = F.collision_factors()
    assert moved == 44856
    assert tuple(variance) == factors.variance
    for i in range(F.CHANNELS):
        for j in range(F.CHANNELS):
            assert K[i][j] == factors.vacuum*Q(factors.correction[i][j], 1 << 60)
            assert M[i][j] == Q(factors.numerator[i][j], factors.denominator)


def test_five_invariants_and_pinned_shear_are_exact(collision):
    initial = F.initial_scores()
    for i in range(F.CHANNELS):
        for j in range(5*F.DEGREE):
            assert collision.coefficients[i, j] == initial.coefficients[i, j]*collision.denominator
    reference = shear_tangent_leakage()
    incoming = F.norm_squared(initial, 5)
    retained = F.norm_squared(collision, 5)
    loss = F.subtract(incoming, retained)
    assert incoming[0] == reference["incoming_squared_norm"]
    assert retained[0] == reference["projected_squared_norm"]
    assert loss[0] == reference["lost_squared_norm"]
    assert loss[1:] == F.ZERO[1:]
    assert sha256(str(loss[0]/incoming[0]).encode("ascii")).hexdigest() == (
        "747dfc05e8f19ee0f4254370ddb6c5e4133be497acc72540c2734f6ee35b9596")


def monomial(exponent):
    # Independent polynomial remainder, not the streaming index implementation.
    return fmpz_poly([0]*(exponent % 16)+[1]) % fmpz_poly([1]+[0]*7+[1])


def evaluate_occupation_score(state, scores, m):
    total = fmpz_poly()
    for site, bits in enumerate(state.bank):
        position = (site % state.L, site//state.L % state.L, site//state.L**2)
        phase = monomial(sum(a*b for a, b in zip(m, position)))
        for c in R.occupied(bits):
            total += phase*fmpz_poly([scores.coefficients[c, j] for j in range(8)])
    return total % F.MODULUS


@pytest.mark.parametrize("m", F.WAVEVECTORS)
@pytest.mark.parametrize("phase", (1, 2, 3))
def test_forward_density_stream_sign_against_actual_finite_runtime(m, phase):
    state = R.Records.empty(F.L)
    state.microtick = phase
    for position, velocity in (((15, 0, 3), (3, -2, 1)), ((0, 15, 8), (-2, 3, -3)),
                               ((4, 7, 15), (0, 1, -2)), ((1, 1, 1), (-1, 0, 0))):
        x, y, z = position
        state.bank[x+F.L*(y+F.L*z)] |= 1 << R.channel(velocity)
    scores = F.initial_scores((tuple(range(F.CHANNELS)),))
    before = evaluate_occupation_score(state, scores, m)
    updated = F.stream(scores, m, phase)
    R.step(state)
    # U H evaluated on F(n) equals H(n). Centering terms cancel (each
    # nonzero torus Fourier sum is zero; m=0 streaming leaves h unchanged).
    assert evaluate_occupation_score(state, updated, m) == before


def test_full_cycle_order_and_stream_composition(collision):
    m = (1, 1, 0)
    result = F.advance(F.initial_scores(), m, 0, 4)
    for i, velocity in enumerate(F.VELOCITIES):
        for j in (1, 5, 8):
            expected = (monomial(-sum(a*b for a, b in zip(m, velocity)))
                        * fmpz_poly([collision.coefficients[i, 8*j+p] for p in range(8)])) % F.MODULUS
            assert [result.coefficients[i, 8*j+p] for p in range(8)] == [expected[p] for p in range(8)]
    assert result.denominator == collision.denominator
    # An initially invariant density becomes non-invariant after streaming;
    # reversed M D would give a different cycle and must not pass unnoticed.
    density = F.initial_scores((F.score_values()[0],))
    correct = F.advance(density, m, 0, 4)
    reversed_order = F.collide(F.advance(density, m, 1, 3))
    assert correct.coefficients != reversed_order.coefficients


def test_streaming_preserves_norm_with_genuinely_complex_coefficients():
    coefficients = fmpz_mat([[((i+3*j) % 11)-5 for j in range(8)] for i in range(F.CHANNELS)])
    h = F.Scores(coefficients, 7)
    initial = F.norm_squared(h, 0)
    assert F.conjugate(initial) == initial
    for q in (1, 2, 3):
        h = F.stream(h, (1, 1, 1), q)
        assert F.norm_squared(h, 0) == initial


def test_streaming_covariance_for_all_cubic_images_of_locked_modes():
    channel = {v: i for i, v in enumerate(F.VELOCITIES)}
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            transform = lambda v: tuple(signs[j]*v[axes[j]] for j in range(3))
            indices = [channel[transform(v)] for v in F.VELOCITIES]
            for m in F.WAVEVECTORS:
                for q in (1, 2, 3):
                    original = F.phase_exponents(m, q)
                    rotated = F.phase_exponents(transform(m), q)
                    assert original == tuple(rotated[i] for i in indices)


def test_complex_collision_leakage_against_all_actual_pair_score_increments():
    from scripts.phi_v2_lattice.thermal.equivariant import collision_map
    h = F.advance(F.initial_scores((F.score_values()[5],)), (1, 1, 0), 1, 3)
    mh = F.collide(h)
    poly = [fmpz_poly([h.coefficients[i, p] for p in range(8)]) for i in range(F.CHANNELS)]
    total = fmpz_poly()
    for (a, b), (c, d) in collision_map().items():
        delta = poly[c]+poly[d]-poly[a]-poly[b]
        conjugate = fmpz_poly(list(F.conjugate(tuple(delta[p] for p in range(8)))))
        total += (1 << (54-F.ENERGY2[a]-F.ENERGY2[b]))*delta*conjugate
    total %= F.MODULUS
    factor = F.collision_factors().vacuum/Q(1 << 60)
    exact_increment_norm = tuple(factor*int(total[p]) for p in range(8))
    projected_delta = F.Scores(mh.coefficients-mh.denominator*h.coefficients, mh.denominator)
    independent_loss = F.subtract(exact_increment_norm, F.norm_squared(projected_delta, 0))
    norm_loss = F.subtract(F.norm_squared(h, 0), F.norm_squared(mh, 0))
    assert independent_loss == norm_loss
    assert F.nonnegative_interval(norm_loss)[0] > 0


@pytest.mark.parametrize("value", (Q(0), Q(1), Q(2), Q(1, 7), Q(10**40, 3)))
def test_root_enclosures_by_exact_integer_inequalities(value):
    lower, upper = F.sqrt_bracket(value)
    assert lower**2 <= value <= upper**2
    assert upper-lower <= Q(1, F.ROOT_DENOMINATOR)


def test_real_cyclotomic_intervals_are_certified_and_fail_closed():
    # zeta^2-zeta^6 = sqrt(2), a non-rational real element.
    root2 = (Q(0), Q(0), Q(1), Q(0), Q(0), Q(0), Q(-1), Q(0))
    lo, hi = F.real_interval(root2)
    assert 0 < lo and lo**2 <= 2 <= hi**2
    lo, hi = F.real_interval(tuple(-x for x in root2))
    assert lo < hi < 0
    assert lo**2 >= 2 >= hi**2
    with pytest.raises(ValueError, match="real canonical"):
        F.real_interval((Q(0), Q(1))+(Q(0),)*6)
    with pytest.raises(ValueError, match="no clamping"):
        F.nonnegative_interval((Q(-1),)+(Q(0),)*7)
    with pytest.raises(ValueError, match="contractive"):
        F.budget_row(Q(1), (Q(2),)+(Q(0),)*7, [])


def test_phase_domain_and_clock_overflow_are_checked_before_work():
    h = F.initial_scores((F.score_values()[0],))
    for phase in (0, 4, True, 1.0):
        with pytest.raises(ValueError): F.stream(h, (1, 0, 0), phase)
    for m in ((True, 0, 0), (1.0, 0, 0), (1, 0), (17, 0, 0)):
        with pytest.raises(ValueError): F.advance(h, m, 0, 0)
    for tick, horizon in ((-1, 0), (True, 1), (0, -1), (R.MAX_TICK, 1)):
        with pytest.raises(ValueError): F.advance(h, (0, 0, 0), tick, horizon)
    assert F.advance(h, (0, 0, 0), R.MAX_TICK, 0) is h
    for start in (1, 2, 3):
        streamed = F.advance(h, (1, 0, 0), start, 4-start)
        assert streamed.denominator == 1  # collision is at input phase zero only


def test_multistep_bound_on_independent_full_boolean_model_with_return():
    # Uniform measure on eight states. C swaps 000 and 111, fixing the rest;
    # S cycles bits. This is a separate toy theorem check, NOT candidate-2.
    states = tuple(range(8))
    basis = [[Q((s >> i) & 1)-Q(1, 2) for s in states] for i in range(3)]
    inner = lambda a, b: sum((x*y for x, y in zip(a, b)), Q(0))/8
    norm = lambda a: inner(a, a)
    def projection(a):
        coefficients = [4*inner(a, b) for b in basis]
        return [sum((c*b[s] for c, b in zip(coefficients, basis)), Q(0)) for s in states]
    collision_inverse = [7, 1, 2, 3, 4, 5, 6, 0]
    stream_inverse = [((s << 1) & 7) | (s >> 2) for s in states]
    h = [sum(b[s] for b in basis) for s in states]
    true, projected = h[:], h[:]
    initial_norm = norm(h)
    losses = []
    returned = False
    for tick in range(16):
        inverse = collision_inverse if tick % 4 == 0 else stream_inverse
        true = [true[inverse[s]] for s in states]
        pushed = [projected[inverse[s]] for s in states]
        next_projected = projection(pushed)
        loss = norm(pushed)-norm(next_projected)
        if tick % 4 == 0:
            losses.append((loss,)+(Q(0),)*7)
        else:
            assert loss == 0
        projected = next_projected
        budget = F.budget_row(initial_norm, (norm(projected),)+(Q(0),)*7, losses)
        defect = [a-b for a, b in zip(true, projected)]
        projected_defect = projection(defect)
        returned |= norm(projected_defect) > 0
        assert norm(defect)/initial_norm <= budget["full_relative_norm_upper"]**2
        assert norm(projected_defect)/initial_norm <= budget["projected_relative_norm_upper"]**2
        if tick < 4:
            assert norm(projected_defect) == 0
        if tick == 4:
            # C S^3 C = identity on this score: true norm fully returns,
            # whereas repeated projection has discarded non-additive terms.
            assert true == h
            assert norm(projected_defect) > 0
    assert returned


def test_immutable_pair_writes_refuse_replacement_and_are_deterministic(tmp_path):
    a, b = tmp_path/"a"/"receipt.json", tmp_path/"b"/"receipt.json"
    E.write_evidence(a, {"scope": "test"}, {"value": "1/3"})
    E.write_evidence(b, {"scope": "test"}, {"value": "1/3"})
    assert a.read_bytes() == b.read_bytes()
    assert a.with_suffix(".exact.json.gz").read_bytes() == b.with_suffix(".exact.json.gz").read_bytes()
    original = a.read_bytes()
    with pytest.raises(FileExistsError): E.write_evidence(a, {}, {})
    assert a.read_bytes() == original
    orphan = tmp_path/"orphan.json"
    orphan.with_suffix(".exact.json.gz").write_bytes(b"historical")
    with pytest.raises(FileExistsError): E.write_evidence(orphan, {}, {})
    assert not orphan.exists()


def test_preregistration_is_frozen_and_symmetry_orbits_are_distinct():
    representatives = {tuple(sorted(abs(x) for x in m)) for m in F.WAVEVECTORS}
    assert len(representatives) == len(F.WAVEVECTORS) == 4
    assert F.SCORE_NAMES == ("1", "vx", "vy", "vz", "E2", "xy_T2g", "yz_T2g", "zx_T2g",
                             "x2_minus_y2_Eg", "2z2_minus_x2_minus_y2_Eg", "E2_squared_A1g")
    lock = E.ROOT/E.LOCK
    if not lock.exists():
        pytest.skip("preregistration is local-only under the repository documentation policy")
    assert sha256(lock.read_bytes()).hexdigest() == E.LOCK_HASH


def test_retained_receipt_hashes_full_domain_and_exact_accounting():
    receipt = E.ROOT/"engine/docs/evidence/thermal-candidate2-finite-wavelength-2026-09-13.json"
    if not receipt.exists():
        pytest.skip("local-only immutable receipt absent; reproduce with the named builder and lock")
    data = receipt.read_bytes()
    assert sha256(data).hexdigest() == "97f0cc9ccc02f1d00b2fc828ba7cc4b8d1d6b191236650ef3c8d354073d34698"
    record = json.loads(data)
    compressed = receipt.with_suffix(".exact.json.gz").read_bytes()
    assert sha256(compressed).hexdigest() == record["exact_artifact"]["sha256"] == (
        "86d9a2e884912d377f575c150f699c21e8591f9bf6c2abbce6682a57a24d2bfd")
    raw = gzip.decompress(compressed)
    assert sha256(raw).hexdigest() == record["exact_artifact"]["uncompressed_sha256"]
    exact = json.loads(raw)
    for path, pinned in record["sources_sha256"].items():
        assert sha256((E.ROOT/path).read_bytes()).hexdigest() == pinned, path
    assert tuple(map(tuple, exact["collision"]["B_integer_rows"])) == F.collision_factors().correction
    assert record["certification"]["rigorous_finite_time_linear_score_comparison_bound"]
    for claim in ("full_state_product_closure", "actual_multistep_correlations_computed", "continuum_limit",
                  "thermal_relaxation", "transport", "canonical_adoption"):
        assert record["certification"][claim] is False
    assert record["certification"]["public_fluid_admission"] == "NONE"

    def decode(poly):
        # FLINT reads the pinned large integers without weakening Python's
        # process-global integer-string guard for arbitrary input.
        denominator = int(fmpz(poly["denominator"]))
        return tuple(Q(int(fmpz(x)), denominator) for x in poly["numerators"])

    initial = [decode(x) for x in exact["initial_squared_norms"]]
    assert len(record["cases"]) == len(exact["wavevectors"]) == 4
    for m, case, exact_case in zip(F.WAVEVECTORS, record["cases"], exact["wavevectors"]):
        assert tuple(case["m"]) == tuple(exact_case["m"]) == m
        assert exact_case["streaming_zeta_exponents_q123"] == [list(F.phase_exponents(m, q)) for q in (1, 2, 3)]
        assert [row["input_clock"] for row in exact_case["collisions"]] == [0, 4, 8, 12]
        assert [row["clock"] for row in case["endpoints"]] == list(range(17))
        assert [row["clock"] for row in exact_case["endpoint_score_hashes"]] == list(range(17))
        losses = [[] for _ in initial]
        previous = initial
        budgets = [{name: E.small_budget(F.budget_row(initial[j][0], initial[j], []))
                    for j, name in enumerate(F.SCORE_NAMES)}]
        for collision in exact_case["collisions"]:
            retained = [decode(x) for x in collision["retained_squared_norms"]]
            discarded = [decode(x) for x in collision["discarded_squared_norms"]]
            assert len(retained) == len(discarded) == 11
            for j, loss in enumerate(discarded):
                assert F.subtract(previous[j], retained[j]) == loss
                assert F.nonnegative_interval(loss)[0] >= 0
                losses[j].append(loss)
                assert F.subtract(initial[j], retained[j]) == tuple(
                    sum((entry[p] for entry in losses[j]), Q(0)) for p in range(8))
            budgets.append({name: E.small_budget(F.budget_row(initial[j][0], retained[j], losses[j]))
                            for j, name in enumerate(F.SCORE_NAMES)})
            previous = retained
        for endpoint in case["endpoints"]:
            clock = endpoint["clock"]
            assert endpoint["phase"] == clock % 4
            assert endpoint["scores"] == budgets[(clock+3)//4]
