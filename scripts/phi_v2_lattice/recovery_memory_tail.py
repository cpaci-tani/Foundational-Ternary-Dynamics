"""Exact remaining squared memory energy of the unchanged parity field law.

This is offline finite-operator arithmetic. It neither advances a microscopic
state nor infers a summable memory tail or a continuum limit.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction as F
from itertools import permutations, product
import hashlib
import json
from numbers import Integral
import re

import numpy as np

from . import recovery_full_memory as UPSTREAM

LAW_ID = "phi-hydro-parity-field-sector-1"
LABELS = tuple(q for q in product((-1, 0, 1), repeat=4) if sum(x*x for x in q) == 2)
VELOCITIES = tuple(q[:3] for q in LABELS)
V = tuple(sorted(set(VELOCITIES)))
DELTA_SUPPORT = tuple(sorted({tuple(a[j]+b[j] for j in range(3)) for a in V for b in V}))
D = tuple(sorted(set(product((-1, 0, 1), repeat=3)) |
                 {tuple(2*s if j == axis else 0 for j in range(3))
                  for axis in range(3) for s in (-1, 1)}))
SUPPORT = tuple(sorted({tuple(e[j]-d[j] for j in range(3)) for d in D for e in D}))
RANGES = tuple((185*r//64, 185*(r+1)//64) for r in range(64))
STRESSES = (("axis", (1, 0, 0), (0, 1, 0)), ("axis_second", (1, 0, 0), (0, 0, 1)),
            ("face_in_plane", (1, 1, 0), (1, -1, 0)), ("face_normal", (1, 1, 0), (0, 0, 1)),
            ("body_first", (1, 1, 1), (1, -1, 0)), ("body_second", (1, 1, 1), (1, 1, -2)))
FOURIER = (("zero", (0, 0, 0)),) + tuple(
    (name+suffix, tuple(m*x for x in n))
    for name, n in (("axis", (1, 0, 0)), ("face", (1, 1, 0)), ("body", (1, 1, 1)))
    for suffix, m in (("_half_pi", 1), ("_pi", 2)))
HORIZONS = (0, 1, 2, 3, 4, 8, 16, 32, 64, 128, 256, 512, 1024)
DECIMAL = re.compile(r"(?:0|-[1-9][0-9]*|[1-9][0-9]*)\Z")
DEN = 1 << 864
VECTORS = ((1,)*24,) + tuple(tuple(v[j] for v in VELOCITIES) for j in range(3))


def integer(value, name="integer", lo=None, hi=None):
    if isinstance(value, bool) or not isinstance(value, Integral):
        raise ValueError(name+" must be an exact integer")
    value = int(value)
    if (lo is not None and value < lo) or (hi is not None and value > hi):
        raise ValueError(name+" is outside its declared range")
    return value


def decimal(value):
    if type(value) is not str or not DECIMAL.fullmatch(value):
        raise ValueError("noncanonical integer string")
    return int(value)


def rational(value):
    if type(value) is not str or value.count("/") != 1:
        raise ValueError("exact reduced n/d string required")
    a, b = value.split("/")
    n, d = decimal(a), decimal(b)
    if d <= 0:
        raise ValueError("positive denominator required")
    q = F(n, d)
    if (q.numerator, q.denominator) != (n, d):
        raise ValueError("unreduced rational")
    return q


def fraction_string(value):
    if isinstance(value, bool) or not isinstance(value, (F, Integral)):
        raise ValueError("exact rational required")
    q = F(value)
    return f"{q.numerator}/{q.denominator}"


def canonical_bytes(value):
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True,
                       allow_nan=False)+"\n").encode("ascii")


def read_json(data):
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError("duplicate JSON key")
            result[key] = value
        return result
    def invalid(value):
        raise ValueError("nonfinite JSON value: "+value)
    return json.loads(data, object_pairs_hook=unique, parse_constant=invalid)


def sha256(data):
    return hashlib.sha256(data).hexdigest()


def displacement(value):
    if not isinstance(value, (tuple, list)) or len(value) != 3:
        raise ValueError("three integer coordinates required")
    return tuple(integer(x) for x in value)


def matrix(value, parse=integer):
    if not isinstance(value, (list, tuple)) or len(value) != 24:
        raise ValueError("24 matrix rows required")
    if any(not isinstance(row, (tuple, list)) or len(row) != 24 for row in value):
        raise ValueError("24 matrix columns required")
    return tuple(tuple(parse(x) for x in row) for row in value)


def _bound(value, bits=877):
    if any(abs(int(x)) >= (1 << bits) for row in value for x in row):
        raise ArithmeticError("registered integer bound exceeded")


def _tuple(value):
    return tuple(tuple(integer(x) for x in row) for row in value)


def _serialized(value):
    return [[str(integer(x)) for x in row] for row in value]


def geometry():
    if (len(LABELS), len(V), len(DELTA_SUPPORT), len(D), len(SUPPORT)) != (24, 18, 93, 33, 185):
        raise AssertionError("registered support counts")
    return {"labels": LABELS, "delta_support": DELTA_SUPPORT,
            "nonzero_candidate_support": D, "support": SUPPORT, "ranges": RANGES}


@dataclass(frozen=True)
class Prepared:
    labels: tuple
    jnum: tuple
    delta: tuple
    upstream_gamma: str
    upstream_pivots: tuple

    def __post_init__(self):
        if (self.labels != LABELS or type(self.labels) is not tuple
                or type(self.jnum) is not tuple or any(type(row) is not tuple for row in self.jnum)
                or type(self.delta) is not tuple or type(self.upstream_pivots) is not tuple
                or any(type(item) is not tuple or len(item) != 2 or type(item[0]) is not tuple
                       or type(item[1]) is not tuple or any(type(row) is not tuple for row in item[1])
                       for item in self.delta)):
            raise ValueError("Prepared requires owned immutable tuple data")
        # Validation accepts Integral scalars, but arithmetic must never retain
        # NumPy fixed-width scalars inside object arrays or Python tuples.
        object.__setattr__(self, "labels", tuple(tuple(integer(x) for x in q) for q in self.labels))
        object.__setattr__(self, "jnum", matrix(self.jnum))
        object.__setattr__(self, "delta", tuple((displacement(d), matrix(values)) for d, values in self.delta))
        if any(abs(x) > 1 << 24 for row in self.jnum for x in row):
            raise ValueError("Prepared J exceeds its finite domain")
        if tuple(d for d, _ in self.delta) != DELTA_SUPPORT:
            raise ValueError("Prepared requires all93 ordered displacements")
        for d, values in self.delta:
            displacement(d)
            matrix(values)
            if any(abs(x) > 1 << 433 for row in values for x in row):
                raise ValueError("Prepared Delta exceeds its finite domain")
            if d not in D and any(x for row in values for x in row):
                raise ValueError("Prepared structural zero is nonzero")
        rational(self.upstream_gamma)
        if len(self.upstream_pivots) != 24:
            raise ValueError("Prepared requires24 pivots")
        for value in self.upstream_pivots:
            rational(value)

    def mapping(self):
        return {"schema": "strict-memory-tail-prepared-1", "law_id": LAW_ID,
                "labels": [list(q) for q in self.labels], "j_denominator_exponent": 24,
                "delta_denominator_exponent": 432, "jnum": _serialized(self.jnum),
                "delta": [{"displacement": list(d), "numerators": _serialized(m)}
                          for d, m in self.delta], "upstream_gamma": self.upstream_gamma,
                "upstream_pivots": list(self.upstream_pivots)}


def prepare(full_k1_mapping, local_census_mapping):
    """Parse and align only; no W2, stress, Gamma predicate or new PSD check."""
    c, t = full_k1_mapping, local_census_mapping
    if not isinstance(c, dict) or c.get("schema") != "strict-full-memory-certificate-1" or c.get("law_id") != LAW_ID:
        raise ValueError("complete upstream certificate identity required")
    labels = tuple(tuple(integer(x) for x in q) for q in t["labels"])
    if labels != LABELS or integer(t["jacobian_denominator"]) != 1 << 24:
        raise ValueError("local census label/denominator mismatch")
    jnum = matrix(t["jacobian_numerator"])
    _bound(jnum, 25)
    if any(abs(x) > 1 << 24 for row in jnum for x in row):
        raise ValueError("J entry exceeds contraction bound")
    if not isinstance(c.get("delta"), list) or len(c["delta"]) != 93:
        raise ValueError("complete93-site Delta required")
    rows = []
    for row in c["delta"]:
        d = displacement(row["displacement"])
        values = matrix(row["matrix"], rational)
        if any((1 << 432) % q.denominator for r in values for q in r):
            raise ValueError("Delta denominator does not divide2^432")
        nums = tuple(tuple(q.numerator*((1 << 432)//q.denominator) for q in r) for r in values)
        if any(abs(x) > 1 << 433 for r in nums for x in r):
            raise ValueError("Delta exceeds contraction-difference bound")
        if d not in D and any(x for r in nums for x in r):
            raise ValueError("nonzero structurally forbidden coefficient")
        rows.append((d, nums))
    if tuple(sorted(d for d, _ in rows)) != DELTA_SUPPORT:
        raise ValueError("Delta coverage or duplicate displacement")
    gamma = fraction_string(rational(c["Gamma"]))
    if not isinstance(c.get("impulse_PSD_pivots"), list) or len(c["impulse_PSD_pivots"]) != 24:
        raise ValueError("24 upstream PSD pivots required")
    pivots = tuple(fraction_string(rational(x)) for x in c["impulse_PSD_pivots"])
    return Prepared(labels, jnum, tuple(sorted(rows)), gamma, pivots)


def restore_prepared(value):
    if (not isinstance(value, dict) or value.get("schema") != "strict-memory-tail-prepared-1"
            or value.get("law_id") != LAW_ID or integer(value["j_denominator_exponent"]) != 24
            or integer(value["delta_denominator_exponent"]) != 432):
        raise ValueError("prepared identity")
    c = {"schema": "strict-full-memory-certificate-1", "law_id": LAW_ID,
         "Gamma": value["upstream_gamma"], "impulse_PSD_pivots": value["upstream_pivots"],
         "delta": [{"displacement": r["displacement"], "matrix":
                    [[fraction_string(F(decimal(x), 1 << 432)) for x in row] for row in r["numerators"]]}
                   for r in value["delta"]]}
    return prepare(c, {"labels": value["labels"], "jacobian_denominator": 1 << 24,
                       "jacobian_numerator": matrix(value["jnum"], decimal)})


def pair_count(h):
    h = displacement(h)
    return sum(tuple(d[j]+h[j] for j in range(3)) in D for d in D)


def counters(index):
    h = SUPPORT[integer(index, "coefficient index", 0, 184)]
    return {"site_pairs": pair_count(h), "primary_products": pair_count(h)*24**3,
            "reference_products": pair_count(h)*24**3,
            "primary_j_products": 24**3 if h == (0, 0, 0) else 0,
            "reference_j_products": 24**3 if h == (0, 0, 0) else 0}


def _base(prepared):
    j = np.asarray(prepared.jnum, dtype=object)
    return ((np.eye(24, dtype=object)*(1 << 48)-j.T@j)*(1 << 816))


def _primary(prepared, h):
    values = {d: np.asarray(m, dtype=object) for d, m in prepared.delta if d in D}
    out = _base(prepared) if h == (0, 0, 0) else np.zeros((24, 24), dtype=object)
    _bound(out)
    for d in D:
        e = tuple(d[j]+h[j] for j in range(3))
        if e in values:
            term = values[d].T@values[e]
            _bound(term)
            out -= term
            _bound(out)
    return _tuple(out)


def _reference(prepared, h):
    """Independent site-pair selection and row-outer-product implementation."""
    out = [[0]*24 for _ in range(24)]
    if h == (0, 0, 0):
        for a in range(24):
            for i in range(24):
                out[a][i] = (((1 << 48) if a == i else 0) -
                             sum(prepared.jnum[b][a]*prepared.jnum[b][i] for b in range(24))) << 816
    rows = tuple((d, m) for d, m in prepared.delta if d in D)
    for d, left in rows:
        for e, right in rows:
            if tuple(e[j]-d[j] for j in range(3)) != h:
                continue
            for b in range(24):
                for a in range(24):
                    la, dest = left[b][a], out[a]
                    for i in range(24):
                        dest[i] -= la*right[b][i]
            _bound(out)
    return _tuple(out)


def coefficient_range(prepared, start, stop, emit=None):
    start, stop = integer(start, "start", 0, 185), integer(stop, "stop", 0, 185)
    if stop < start or not isinstance(prepared, Prepared):
        raise ValueError("invalid coefficient range or prepared data")
    result = []
    for index in range(start, stop):
        h = SUPPORT[index]
        primary, reference = _primary(prepared, h), _reference(prepared, h)
        if primary != reference:
            raise AssertionError("independent integer contractions disagree")
        record = {"schema": "strict-memory-tail-coefficient-1", "law_id": LAW_ID,
                  "index": index, "displacement": list(h), "denominator_exponent": 864,
                  "numerators": _serialized(primary), "independent_equal": True,
                  "counters": counters(index)}
        if emit is not None:
            emit(record)
        result.append(record)
    return result


def validate_coefficient(record, index=None):
    if (not isinstance(record, dict) or record.get("schema") != "strict-memory-tail-coefficient-1"
            or record.get("law_id") != LAW_ID or record.get("independent_equal") is not True):
        raise ValueError("coefficient identity/independence")
    i = integer(record["index"], "coefficient index", 0, 184)
    if index is not None and i != integer(index):
        raise ValueError("coefficient sequence mismatch")
    if (displacement(record["displacement"]) != SUPPORT[i]
            or integer(record["denominator_exponent"]) != 864
            or record.get("counters") != counters(i)
            or any(type(x) is not int for x in record["counters"].values())):
        raise ValueError("coefficient geometry/counters")
    value = matrix(record["numerators"], decimal)
    _bound(value)
    return value


def _complete(records):
    if not isinstance(records, (tuple, list)) or len(records) != 185:
        raise ValueError("all185 ordered coefficient records required")
    return {SUPPORT[i]: np.asarray(validate_coefficient(row, i), dtype=object) for i, row in enumerate(records)}


def _annihilates(value):
    for u in VECTORS:
        v = np.asarray(u, dtype=object)
        if any(value@v) or any(v@value):
            raise AssertionError("conserved annihilation failed")


def _actions():
    for axes in permutations(range(3)):
        for signs in product((-1, 1), repeat=3):
            for s4 in (-1, 1):
                image = tuple(LABELS.index(tuple(signs[j]*q[axes[j]] for j in range(3))+(s4*q[3],))
                              for q in LABELS)
                yield axes, signs, image


def _coefficient_identities(prepared, values):
    delta = {d: np.asarray(m, dtype=object) for d, m in prepared.delta}
    j = np.asarray(prepared.jnum, dtype=object)
    if not np.array_equal(j, j.T):
        raise AssertionError("J symmetry")
    for u in VECTORS:
        if not np.array_equal(j@np.asarray(u, dtype=object), np.asarray(u, dtype=object)*(1 << 24)):
            raise AssertionError("J conserved vector")
    for d, m in delta.items():
        if not np.array_equal(m, m.T):
            raise AssertionError("Delta endpoint reciprocity")
        _annihilates(m)
    for h, m in values.items():
        if not np.array_equal(values[tuple(-x for x in h)], m.T):
            raise AssertionError("W2 reciprocity")
        _annihilates(m)
    for axes, signs, image in _actions():
        for table in (delta, values):
            for d, m in table.items():
                transformed = tuple(signs[j]*d[axes[j]] for j in range(3))
                if not np.array_equal(table[transformed][np.ix_(image, image)], m):
                    raise AssertionError("complete coefficient cube/R4 covariance")
    return {"conserved_vectors": 4, "delta_endpoint_reciprocity": True,
            "W2_reciprocity": True, "two_sided_conserved": True,
            "cube_and_R4_actions": 96, "delta_matrices": 93, "W2_matrices": 185}


def _fourier(table, q):
    real, imag = np.zeros((24, 24), dtype=object), np.zeros((24, 24), dtype=object)
    phases = ((1, 0), (0, -1), (-1, 0), (0, 1))
    for h, m in table.items():
        a, b = phases[sum(q[j]*h[j] for j in range(3)) % 4]
        real += a*m
        imag += b*m
    _bound(real, 885)
    _bound(imag, 885)
    return real, imag


def _quad(value, z):
    return sum(z[a]*int(value[a][i])*z[i] for a in range(24) for i in range(24))


def horizon_factor(n):
    n = integer(n, "cycle horizon", 0)
    return 0 if n < 4 else (n-3)*(n-2)*(2*n-5)//6


def observe(prepared, complete_coefficient_records):
    values = _complete(complete_coefficient_records)
    checks = _coefficient_identities(prepared, values)
    delta = {d: np.asarray(m, dtype=object) for d, m in prepared.delta}
    base = _base(prepared)
    local = base.copy()
    for m in delta.values():
        local -= m.T@m
    if not np.array_equal(local, values[(0, 0, 0)]):
        raise AssertionError("central spatial coefficient")
    pivots = UPSTREAM._psd_exact([[F(int(x), DEN) for x in row] for row in local])
    if tuple(map(fraction_string, pivots)) != prepared.upstream_pivots:
        raise AssertionError("upstream central PSD pivots disagree")
    total_delta = sum(delta.values(), np.zeros((24, 24), dtype=object))
    zero = base-total_delta.T@total_delta
    if not np.array_equal(sum(values.values(), np.zeros((24, 24), dtype=object)), zero):
        raise AssertionError("zero-wavevector contraction")
    trace_uniform = F(sum(int(base[i, i]) for i in range(24)), DEN)
    if trace_uniform < 0:
        raise AssertionError("negative injection trace")
    controls = []
    for name, q in FOURIER:
        wr, wi = _fourier(values, q)
        dr, di = _fourier(delta, q)
        direct_real = base-dr.T@dr-di.T@di
        direct_imag = di.T@dr-dr.T@di
        if not np.array_equal(wr, direct_real) or not np.array_equal(wi, direct_imag):
            raise AssertionError("Gaussian-rational Fourier Gram identity")
        if not np.array_equal(wr, wr.T) or not np.array_equal(wi, -wi.T):
            raise AssertionError("Fourier Hermitian identity")
        _annihilates(wr)
        _annihilates(wi)
        trace = F(sum(int(wr[i, i]) for i in range(24)), DEN)
        if not 0 <= trace <= trace_uniform:
            raise AssertionError("Fourier trace budget")
        controls.append({"name": name, "pi_over_two_coefficients": list(q),
                         "wavevector_unit": "pi/2", "denominator_exponent": 864,
                         "real_numerators": _serialized(wr), "imag_numerators": _serialized(wi),
                         "trace": fraction_string(trace), "gram_identity": True})
    stresses = []
    returned = total_delta.T@total_delta
    for name, n, t in STRESSES:
        z = tuple(sum(n[j]*v[j] for j in range(3))*sum(t[j]*v[j] for j in range(3)) for v in VELOCITIES)
        g = 12*sum(x*x for x in n)*sum(x*x for x in t)
        triple = tuple(F(_quad(m, z), g*DEN) for m in (base, returned, zero))
        if min(triple) < 0 or triple[0] != triple[1]+triple[2]:
            raise AssertionError("exact stress energy accounting")
        stresses.append({"name": name, "n": list(n), "t": list(t), "normalization": g,
                         **dict(zip(("injected", "returned1", "remaining2"), map(fraction_string, triple))),
                         "first_return_exhausts_source": triple[2] == 0})
    gamma = F(max(sum(abs(int(m[a, i])) for m in delta.values() for i in range(24)) for a in range(24)), 1 << 432)
    if gamma != rational(prepared.upstream_gamma):
        raise AssertionError("upstream Gamma differs")
    stable = gamma <= F(921, 524288)
    horizons = []
    for n in HORIZONS:
        factor = horizon_factor(n)
        horizons.append({"cycles": n, "microticks": 2*n, "integer_factor": factor,
                         "C1_bound_established": stable,
                         "squared_bound_by_control": {r["name"]: fraction_string(factor*rational(r["trace"])) for r in controls},
                         "uniform_k_squared_bound": fraction_string(factor*trace_uniform)})
    lifts = []
    for L in (9, 10):
        for origin in ((0, 0, 0), (L-1, L-1, L-1)):
            sites = {tuple((d[j]+origin[j]) % L for j in range(3)) for d in SUPPORT}
            if len(sites) != 185:
                raise AssertionError("periodic coefficient lift aliases")
            lifts.append({"L": L, "origin": list(origin), "sites": len(sites)})
    groups = {}
    for row in stresses:
        groups.setdefault(row["remaining2"], []).append(row["name"])
    return {"checks": checks, "central_spatial_numerators": _serialized(local),
            "zero_wavevector_numerators": _serialized(zero), "denominator_exponent": 864,
            "central_PSD_pivots": list(map(fraction_string, pivots)), "fourier_controls": controls,
            "stress_budgets": stresses, "remaining_stress_equality_classes": list(groups.values()),
            "Gamma": fraction_string(gamma), "stability_threshold": "921/524288",
            "stability": "PASS_SUFFICIENT_UNIFORM_C1" if stable else "NOT_ESTABLISHED_BY_THIS_BOUND",
            "finite_horizons": horizons, "uniform_injection_trace": fraction_string(trace_uniform),
            "periodic_lifts": lifts, "all_k_PSD": "finite-law Gram identity and continuous dense-torus extension"}


def certificate(prepared, complete_coefficient_records):
    observations = observe(prepared, complete_coefficient_records)
    return {"schema": "strict-memory-tail-certificate-1", "law_id": LAW_ID,
            "W2": complete_coefficient_records, **observations,
            "arithmetic": {"coefficient_count": 185, "integer_entries": 106560,
                           "products_per_convolution": 15054336, "complete_convolutions": 2,
                           "J_Gram_products_per_method": 13824, "denominator_exponent": 864},
            "independence_scope": "two complete integer W2 contractions; no second complete upstream central count",
            "initial_preparation": "affine first-degree perturbations of uniform half-occupancy counting reference",
            "initial_unresolved_score": "y0=0; required for the displayed finite-horizon error bound",
            "polarity": "identical independent24-channel banks; no cross-bank response",
            "physical_cycle_microticks": 2, "sampled_grid_is_continuum_proof": False,
            "physical_time_prerequisites": {
                "selected_scale_calibration": None,
                "energy_tail_sufficient_condition": "C^2 Tr W_m=o(tau^3) for fixed m or m small relative to N~tau^-1",
                "absolute_tail_sufficient_condition": "C R_m=o(tau)",
                "stability_requirement": "C must be uniform over the scaling family, or retained explicitly in both conditions",
                "unresolved_initial_score": "y0=0",
                "evaluated": False,
                "scope": "finite W2 and sufficient stability alone establish neither scaling condition"},
            "full_Q_strict_contraction": False, "fixed_L_reachable_decay": True,
            "uniform_absolute_memory_tail": False, "continuum_recovered": False,
            "canonical_adoption": False}
