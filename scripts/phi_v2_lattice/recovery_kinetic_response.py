"""Exact sparse collision response; no marginal reprojection or time iteration."""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction
from functools import lru_cache
from itertools import combinations
from math import gcd, isqrt, lcm
from numbers import Integral

import numpy as np

from . import channels as C, geometry as G, staged as P

PROTOCOL_ID = "strict-kinetic-response-1"
REFERENCE_P = Fraction(1, 96)
PAIRS = tuple(combinations(range(192), 2))


def _integer(value, name, maximum=None):
    if (isinstance(value, bool) or not isinstance(value, Integral) or value < 0
            or (maximum is not None and value > maximum)):
        raise ValueError(f"invalid {name}")
    return int(value)


def _rational(value):
    if isinstance(value, bool) or not isinstance(value, (Fraction, Integral)):
        raise ValueError("exact rational preparation required")
    return Fraction(value)


def _probability(p):
    p = _rational(p)
    if not 0 < p < 1:
        raise ValueError("response requires an interior reference")
    return p


def _probe(h):
    if len(h) != 192:
        raise ValueError("probe requires 192 channel coordinates")
    return tuple(_rational(x) for x in h)


@dataclass(frozen=True)
class CollisionOperator:
    layer: int
    integer_correction: tuple[tuple[int, ...], ...]
    inverse_pairs: tuple[tuple[int, int], ...]
    collision_hash: str = C.COLLISION_HASH


def collision_operator(layer: int) -> CollisionOperator:
    layer = _integer(layer, "collision layer", 2)
    return _collision_operator_cached(layer)


@lru_cache(maxsize=3)
def _collision_operator_cached(layer: int) -> CollisionOperator:
    tables = C.load_collision_tables()
    if C._hash_tables(tables) != C.COLLISION_HASH:
        raise ValueError("frozen collision identity changed")
    table = tables[layer]
    if set(table) != set(PAIRS) or set(table.values()) != set(PAIRS):
        raise ValueError("pair collision must be a complete permutation")
    inverse = {after: before for before, after in table.items()}
    correction = np.zeros((192, 192), dtype=np.int64)
    for before, after in table.items():
        for source in before:
            for target in after:
                correction[target, source] += 1
            for target in before:
                correction[target, source] -= 1
    return CollisionOperator(layer, tuple(tuple(map(int,row)) for row in correction),
                             tuple(inverse[pair] for pair in PAIRS))


def marginal_jacobian(layer, p=REFERENCE_P) -> tuple[tuple[Fraction, ...], ...]:
    p = _probability(p)
    r = p*(1-p)**189
    matrix = collision_operator(layer).integer_correction
    return tuple(tuple(Fraction(i == j) + r*matrix[i][j] for j in range(192))
                 for i in range(192))


def projected_collision_probe(layer, h, p=REFERENCE_P) -> tuple[Fraction, ...]:
    """Apply J exactly; a shared integer denominator keeps long budgets finite."""
    p, h = _probability(p), _probe(h)
    common = lcm(*(value.denominator for value in h))
    support = [(i,value.numerator*(common//value.denominator))
               for i,value in enumerate(h) if value]
    r = p*(1-p)**189
    matrix = collision_operator(layer).integer_correction
    return tuple(value+r*Fraction(sum(row[i]*x for i,x in support),common)
                 for value,row in zip(h,matrix))


def score_norm_squared(h, p=REFERENCE_P) -> Fraction:
    p, h = _probability(p), _probe(h)
    return sum((value*value for value in h),Fraction(0))/(p*(1-p))


def collision_leakage_squared(layer, h, p=REFERENCE_P) -> Fraction:
    """All unresolved Boolean orders, under the full stationary field measure."""
    h, p = _probe(h), _probability(p)
    projected = projected_collision_probe(layer,h,p)
    difference = score_norm_squared(h,p)-score_norm_squared(projected,p)
    if difference < 0:
        raise ValueError("negative leakage: projected operator is not contractive")
    return difference


def sqrt_upper(value, denominator=10**6) -> Fraction:
    """Certified rational enclosure with absolute overshoot below 1/denominator."""
    value = _rational(value)
    denominator = _integer(denominator,"root enclosure denominator")
    if value < 0 or denominator == 0:
        raise ValueError("nonnegative radicand and positive denominator required")
    numerator = value.numerator*denominator**2
    ceiling = (numerator+value.denominator-1)//value.denominator
    root = isqrt(ceiling)
    if root*root < ceiling:
        root += 1
    return Fraction(root,denominator)


@dataclass(frozen=True)
class TangentBudget:
    L: int
    tick_start: int
    tick_end: int
    initial_layer: int
    final_layer: int
    p: Fraction
    final_projected_probe: tuple[Fraction, ...]
    collision_leakages_squared: tuple[tuple[int, Fraction], ...]
    initial_score_norm_squared: Fraction
    final_projected_norm_squared: Fraction
    duhamel_error_upper: Fraction
    norm_error_upper: Fraction
    root_enclosure_denominator: int
    preparation: str = "spatially uniform channel-score perturbation in one polarity"
    norm: str = "full finite-reference weighted L2; every site included"
    law_id: str = P.LAW_ID
    finite_amplitude_or_hydrodynamics_certified: bool = False


def homogeneous_tangent_budget(L, microticks, h, *, start_tick=0, layer=0,
                               p=REFERENCE_P, root_denominator=10**6) -> TangentBudget:
    """Compare true full tangent with projected stages via actual-clock residuals.

    The true full score is not reprojected by the theorem. Only its comparison
    sequence is computed here. Streaming is exact for spatially uniform h.
    """
    L = _integer(L,"periodic size")
    if L < 3:
        raise ValueError("periodic size must be at least three")
    microticks = _integer(microticks,"microtick horizon")
    start_tick = _integer(start_tick,"starting tick")
    layer = _integer(layer,"current layer",2)
    root_denominator = _integer(root_denominator,"root enclosure denominator")
    if root_denominator == 0:
        raise ValueError("positive root enclosure denominator required")
    h, p = _probe(h), _probability(p)
    initial_layer = layer
    initial_norm = L**3*score_norm_squared(h,p)
    residuals = []
    budget = Fraction(0)
    for tick in range(start_tick,start_tick+microticks):
        if tick % 4 == 1:
            before = score_norm_squared(h,p)
            projected = projected_collision_probe(layer,h,p)
            leakage = L**3*(before-score_norm_squared(projected,p))
            if leakage < 0:
                raise ValueError("negative leakage: projected operator is not contractive")
            residuals.append((tick,leakage))
            budget += sqrt_upper(leakage,root_denominator)
            h = projected
            layer = (layer-1)%3
        elif tick % 4 == 2:
            streamed = [Fraction(0)]*192
            for c,value in enumerate(h):
                streamed[C.U(c)] = value
            h = tuple(streamed)
    final_norm = L**3*score_norm_squared(h,p)
    triangle = sqrt_upper(initial_norm,root_denominator)+sqrt_upper(final_norm,root_denominator)
    return TangentBudget(L,start_tick,start_tick+microticks,initial_layer,layer,p,h,
                         tuple(residuals),initial_norm,final_norm,budget,min(budget,triangle),
                         root_denominator)


def product_density_remainder_squared(h, epsilon, *, p=REFERENCE_P, copies=1) -> Fraction:
    """Exact finite product-density remainder; no Boolean-state enumeration.

    This algebraic helper accepts any finite channel list. The homogeneous
    FTD specialization below uses all192 channels and copies=L^3.
    """
    h = tuple(_rational(value) for value in h)
    epsilon,p = _rational(epsilon),_probability(p)
    copies = _integer(copies,"independent coordinate copies")
    if copies == 0 or any(not 0 <= p+epsilon*value <= 1 for value in h):
        raise ValueError("positive copies and valid perturbed occupancy probabilities required")
    variance = p*(1-p)
    second_moment = Fraction(1)
    linear_norm = Fraction(0)
    for value in h:
        term = epsilon*epsilon*value*value/variance
        second_moment *= 1+term
        linear_norm += term
    remainder = second_moment**copies-1-copies*linear_norm
    if remainder < 0:
        raise ValueError("negative exact product-density remainder")
    return remainder


@dataclass(frozen=True)
class FiniteAmplitudeBudget:
    tangent: TangentBudget
    epsilon: Fraction
    initial_product_remainder_squared: Fraction
    initial_product_remainder_upper: Fraction
    transported_tangent_error_upper: Fraction
    full_density_error_upper: Fraction
    comparison: str = "unit-integral linear density 1+epsilon*g; may be signed"
    exact_finite_amplitude_bound: bool = True
    nonlinear_kinetic_or_hydrodynamic_closure_certified: bool = False


def homogeneous_finite_amplitude_budget(L, microticks, h, epsilon, *, start_tick=0,
                                         layer=0, p=REFERENCE_P, root_denominator=10**6):
    """Bound full finite-ensemble evolution versus the external linear density."""
    L = _integer(L,"periodic size")
    if L < 3:
        raise ValueError("periodic size must be at least three")
    h,p,epsilon = _probe(h),_probability(p),_rational(epsilon)
    remainder = product_density_remainder_squared(h,epsilon,p=p,copies=L**3)
    tangent = homogeneous_tangent_budget(L,microticks,h,start_tick=start_tick,layer=layer,
                                         p=p,root_denominator=root_denominator)
    remainder_upper = sqrt_upper(remainder,root_denominator)
    tangent_upper = abs(epsilon)*tangent.norm_error_upper
    return FiniteAmplitudeBudget(tangent,epsilon,remainder,remainder_upper,tangent_upper,
                                remainder_upper+tangent_upper)


@dataclass(frozen=True)
class TangentResponse:
    layer: int
    p: Fraction
    probe: tuple[Fraction, ...]
    mean_derivative: tuple[Fraction, ...]
    pair_covariances: tuple[tuple[int, int, Fraction], ...]
    maximum_pair_covariance: Fraction
    protocol_id: str = PROTOCOL_ID
    law_id: str = P.LAW_ID
    physical_collision_interval: tuple[int, int] = (1, 2)
    iterated_closure_certified: bool = False


def collision_response(layer, h, p=REFERENCE_P) -> TangentResponse:
    p, h = _probability(p), _probe(h)
    operator = collision_operator(layer)
    support = [(i, x) for i, x in enumerate(h) if x]
    kh = tuple(sum((row[i]*x for i, x in support), Fraction(0))
               for row in operator.integer_correction)
    r = p*(1-p)**189
    means = tuple(x+r*y for x, y in zip(h, kh))
    covariances = []
    for (j, k), (a, b) in zip(PAIRS, operator.inverse_pairs):
        value = r*(h[a]+h[b]-h[j]-h[k]-p*(kh[j]+kh[k]))
        if value:
            covariances.append((j, k, value))
    maximum = max((abs(value) for _, _, value in covariances), default=Fraction(0))
    return TangentResponse(operator.layer, p, h, means, tuple(covariances), maximum)


def streamed_pair(L, site, j, k, polarity=1):
    L = _integer(L, "periodic size")
    if L < 3:
        raise ValueError("periodic size must be at least three")
    site = _integer(site, "site", L**3-1)
    j, k = _integer(j, "channel", 191), _integer(k, "channel", 191)
    if (j == k or isinstance(polarity, bool) or not isinstance(polarity, Integral)
            or polarity not in (-1, 1)):
        raise ValueError("two distinct channels and signed polarity required")
    offset = 192 if polarity < 0 else 0
    return tuple(sorted((G.shift(L, site, C.tangent(c+offset)), C.U(c+offset))
                        for c in (j, k)))


def moment_checks(values) -> dict:
    """Exact additive invariant test, including the first failing finite row."""
    values = _probe(values)
    failures = []
    for layer, table in enumerate(C.load_collision_tables()):
        failures.append(next(((before,after) for before,after in sorted(table.items())
                              if sum(values[c] for c in before) != sum(values[c] for c in after)), None))
    stream_failure = next((c for c in range(192) if values[c] != values[C.U(c)]), None)
    return {"collision_invariant_layers": tuple(f is None for f in failures),
            "collision_counterexamples": tuple(failures),
            "streaming_invariant": stream_failure is None,
            "streaming_counterexample": stream_failure}


def declared_moments() -> dict:
    """Preregistered finite readouts; no selection by agreement with a target."""
    moments = {"constant": (1,)*192}
    for axis in range(3):
        moments[f"tangent_{axis}"] = tuple(C.tangent(c)[axis] for c in range(192))
    for component in range(6):
        moments[f"field_{component}"] = tuple(C.field_value_of(c)[component] for c in range(192))
    for layer in range(3):
        for component in range(6):
            moments[f"layer_{layer}_{component}"] = tuple(C.layer_value_of(c,layer)[component] for c in range(192))
    return {name: moment_checks(values) for name, values in moments.items()}


def common_additive_gram() -> tuple[tuple[int, ...], ...]:
    """Integer Gram operator; its rational kernel is ALL common invariants.

    Includes all three collision layers and U streaming. The polarity sectors
    are disjoint, so the 384-channel global kernel is two independent copies.
    """
    gram = np.zeros((192, 192), dtype=np.int64)
    def add(indices):
        delta = {}
        for index, value in indices:
            delta[index] = delta.get(index, 0)+value
        for i, a in delta.items():
            for j, b in delta.items():
                gram[i,j] += a*b
    for table in C.load_collision_tables():
        for before, after in table.items():
            add([(c,-1) for c in before]+[(c,1) for c in after])
    for c in range(192):
        add([(c,-1),(C.U(c),1)])
    return tuple(tuple(map(int,row)) for row in gram)


def common_additive_classification() -> dict:
    """Exact fraction-free nullspace, normalized to primitive integer rows."""
    from sympy import Matrix
    from sympy.polys.matrices import DomainMatrix
    gram = Matrix(common_additive_gram())
    raw = DomainMatrix.from_Matrix(gram).nullspace().to_Matrix()
    basis = []
    for row in raw.tolist():
        divisor = 0
        for value in row:
            divisor = gcd(divisor, int(value))
        normalized = tuple(int(value)//divisor for value in row)
        if next(value for value in normalized if value) < 0:
            normalized = tuple(-value for value in normalized)
        if gram*Matrix(normalized) != Matrix.zeros(192,1):
            raise ValueError("invalid exact nullspace vector")
        basis.append(normalized)
    return {"constraint_rank": 192-len(basis), "dimension_per_polarity": len(basis),
            "primitive_integer_basis": tuple(basis),
            "dimension_both_polarities": 2*len(basis),
            "scope": "fixed spatially uniform additive field-channel weights; all three collision layers and U",
            "time_dependent_or_nonlinear_invariants_classified": False}
