"""Exact candidate-2 score compression and finite-time comparison bounds.

No full-state closure is assumed. U is the forward density-score pushforward;
streaming is exp(-ik.d), collision is its involutive pullback. See the locked
protocol and the proof in SPEC_FINITE_THERMAL_EQUIVARIANT_V2.md.
"""
from __future__ import annotations

from dataclasses import dataclass
from fractions import Fraction as Q
from functools import lru_cache
from math import isqrt, lcm, prod

from flint import fmpz_mat, fmpz_poly

from .analysis import stationary_occupations
from .equivariant import collision_identity, collision_map
from .runtime import CHANNELS, ENERGY2, MAX_TICK, VELOCITIES

L = 16
DEGREE = 8
MODULUS = fmpz_poly([1] + [0]*7 + [1])
TABLE_HASH = "726908e917ef3723d02b1f9d50285fe64b1ac4e0b614b45ef3e5b1a6971f7d6d"
WAVEVECTORS = ((0, 0, 0), (1, 0, 0), (1, 1, 0), (1, 1, 1))
SCORE_NAMES = ("1", "vx", "vy", "vz", "E2", "xy_T2g", "yz_T2g", "zx_T2g",
               "x2_minus_y2_Eg", "2z2_minus_x2_minus_y2_Eg", "E2_squared_A1g")
ZERO = (Q(0),)*DEGREE
ROOT_DENOMINATOR = 10**40


def _integer(value, name, low, high):
    if type(value) is not int or not low <= value <= high:
        raise ValueError(f"invalid {name}")
    return value


def wavevector(m):
    if not isinstance(m, (list, tuple)) or len(m) != 3:
        raise ValueError("three integer Fourier indices required")
    return tuple(_integer(x, "Fourier index", -L, L) % L for x in m)


def phase_exponents(m, phase):
    """Integer powers of zeta for a forward density score, not an observable."""
    m = wavevector(m)
    _integer(phase, "streaming phase", 1, 3)
    return tuple(-sum(a*((1 if v > 0 else -1) if abs(v) >= phase else 0)
                      for a, v in zip(m, velocity)) % L for velocity in VELOCITIES)


def score_values():
    return tuple(tuple((1, x, y, z, e, x*y, y*z, z*x, x*x-y*y,
                        2*z*z-x*x-y*y, e*e)[j]
                       for (x, y, z), e in zip(VELOCITIES, ENERGY2))
                 for j in range(len(SCORE_NAMES)))


@dataclass(frozen=True)
class CollisionFactors:
    """Immutable exact factorization: M = I + vacuum/(2^60) V^-1 B."""
    correction: tuple[tuple[int, ...], ...]
    variance: tuple[Q, ...]
    vacuum: Q
    numerator: tuple[tuple[int, ...], ...]
    denominator: int
    norm_weights: tuple[int, ...]
    norm_denominator: int


@lru_cache(maxsize=1)
def collision_factors():
    if collision_identity() != TABLE_HASH:
        raise ValueError("frozen collision identity changed")
    f = stationary_occupations(Q(1, 8), Q(1, 2))
    variance = tuple(p*(1-p) for p in f)
    vacuum = prod((1-p for p in f), start=Q(1))
    b = [[0]*CHANNELS for _ in range(CHANNELS)]
    for (a, b0), (c, d) in collision_map().items():
        # odds_a*odds_b = 2^(-Ea-Eb-6), including both directions.
        w = 1 << (54-ENERGY2[a]-ENERGY2[b0])
        for i in (a, b0):
            for j, sign in ((c, 1), (d, 1), (a, -1), (b0, -1)):
                b[i][j] += sign*w
    denominator = vacuum.denominator*(1 << 90)
    rows = []
    for i, energy in enumerate(ENERGY2):
        u = 1 << (energy+3)
        factor = vacuum.numerator*(u+1)**2*(1 << (27-energy))
        rows.append(tuple(denominator*(i == j)+factor*b[i][j] for j in range(CHANNELS)))
    norm_denominator = lcm(*(v.denominator for v in variance))
    return CollisionFactors(tuple(map(tuple, b)), variance, vacuum, tuple(rows), denominator,
                            tuple(int(v*norm_denominator) for v in variance), norm_denominator)


@dataclass
class Scores:
    """Owned integer coefficient matrix / common denominator in Q(zeta_16).

    Each eight-column block is one 343-channel score. Operations return new
    matrices; no mutable matrix is cached as part of the law.
    """
    coefficients: fmpz_mat
    denominator: int = 1

    def __post_init__(self):
        if (not isinstance(self.coefficients, fmpz_mat) or self.coefficients.nrows() != CHANNELS
                or self.coefficients.ncols() == 0 or self.coefficients.ncols() % DEGREE
                or type(self.denominator) is not int or self.denominator <= 0):
            raise ValueError("invalid exact channel score matrix")

    @property
    def count(self):
        return self.coefficients.ncols()//DEGREE


def initial_scores(values=None):
    values = score_values() if values is None else tuple(tuple(row) for row in values)
    if not values or any(len(row) != CHANNELS or any(type(x) is not int for x in row) for row in values):
        raise ValueError("integer initial channel scores required")
    matrix = fmpz_mat(CHANNELS, len(values)*DEGREE)
    for j, row in enumerate(values):
        for i, value in enumerate(row):
            matrix[i, DEGREE*j] = value
    return Scores(matrix)


def collide(scores):
    factors = collision_factors()
    matrix = fmpz_mat(factors.numerator)
    return Scores(matrix*scores.coefficients, factors.denominator*scores.denominator)


def stream(scores, m, phase):
    exponents = phase_exponents(m, phase)
    output = fmpz_mat(CHANNELS, scores.coefficients.ncols())
    for i, exponent in enumerate(exponents):
        for j in range(scores.count):
            for power in range(DEGREE):
                destination = (power+exponent) % L
                output[i, DEGREE*j+destination % DEGREE] = (
                    (1 if destination < DEGREE else -1)*scores.coefficients[i, DEGREE*j+power])
    return Scores(output, scores.denominator)


def advance(scores, m, start_tick, microticks):
    """Projected comparison trajectory, using the actual four-phase clock."""
    wavevector(m)
    _integer(start_tick, "starting tick", 0, MAX_TICK)
    _integer(microticks, "microtick horizon", 0, MAX_TICK-start_tick)
    for tick in range(start_tick, start_tick+microticks):
        scores = collide(scores) if tick % 4 == 0 else stream(scores, m, tick % 4)
    return scores


def conjugate(poly):
    """Canonical coefficients of complex conjugation, zeta^-j=-zeta^(8-j)."""
    return (poly[0],)+tuple(-poly[DEGREE-j] for j in range(1, DEGREE))


def norm_squared(scores, index):
    _integer(index, "score index", 0, scores.count-1)
    factors = collision_factors()
    total = fmpz_poly()
    for i, weight in enumerate(factors.norm_weights):
        h = tuple(scores.coefficients[i, DEGREE*index+j] for j in range(DEGREE))
        total += weight*(fmpz_poly(list(h))*fmpz_poly(list(conjugate(h))))
    total %= MODULUS
    denominator = factors.norm_denominator*scores.denominator**2
    return tuple(Q(int(total[j]), denominator) for j in range(DEGREE))


def subtract(a, b):
    return tuple(x-y for x, y in zip(a, b))


def sqrt_bracket(value, denominator=ROOT_DENOMINATOR):
    """Rational lower/upper endpoints proved by integer square inequalities."""
    if not isinstance(value, (Q, int)) or isinstance(value, bool) or value < 0:
        raise ValueError("nonnegative rational radicand required")
    if type(denominator) is not int or denominator <= 0:
        raise ValueError("positive integer root denominator required")
    value = Q(value)
    scaled = value.numerator*denominator**2
    lower = isqrt(scaled//value.denominator)
    upper = lower if lower*lower*value.denominator == scaled else lower+1
    return Q(lower, denominator), Q(upper, denominator)


@lru_cache(maxsize=1)
def _cosine_intervals():
    a, b = sqrt_bracket(Q(2))
    c1 = (sqrt_bracket(2+a)[0]/2, sqrt_bracket(2+b)[1]/2)
    c2 = (a/2, b/2)
    c3 = (sqrt_bracket(2-b)[0]/2, sqrt_bracket(2-a)[1]/2)
    negative = lambda x: (-x[1], -x[0])
    return ((Q(1), Q(1)), c1, c2, c3, (Q(0), Q(0)),
            negative(c3), negative(c2), negative(c1))


def real_interval(poly):
    """Certified rational enclosure at the selected root, never float rounding."""
    if len(poly) != DEGREE or tuple(poly) != conjugate(poly):
        raise ValueError("a real canonical cyclotomic polynomial is required")
    lo, hi = Q(0), Q(0)
    for value, (a, b) in zip(poly, _cosine_intervals()):
        lo += value*(a if value >= 0 else b)
        hi += value*(b if value >= 0 else a)
    return lo, hi


def nonnegative_interval(poly):
    lo, hi = real_interval(poly)
    if lo < 0:
        raise ValueError("negative or uncertified leakage/norm; no clamping allowed")
    return lo, hi


def budget_row(initial_norm, retained, losses):
    """Normalized Duhamel and projected-return bounds; all inputs exact.

    Losses are those of the comparison path. They are not independent and are
    not identified with the actual trajectory's correlation population.
    """
    if initial_norm <= 0:
        raise ValueError("a positive initial norm is required")
    normalized = [tuple(x/initial_norm for x in loss) for loss in losses]
    roots = [sqrt_bracket(nonnegative_interval(loss)[1])[1] for loss in normalized]
    retained_ratio = tuple(x/initial_norm for x in retained)
    lower, upper = nonnegative_interval(retained_ratio)
    if upper > 1:
        raise ValueError("retention not certified contractive")
    duhamel = sum(roots, Q(0))
    triangle = 1+sqrt_bracket(upper)[1]
    full_upper = min(duhamel, triangle)
    # The last residual is orthogonal to P, and intervening streams preserve
    # P and P-perp. Only earlier collision residuals can have returned to P.
    projected_upper = min(sum(roots[:-1], Q(0)), triangle)
    return {
        "retained_fraction_interval": (lower, upper),
        "duhamel_relative_norm_upper": duhamel,
        "full_relative_norm_upper": full_upper,
        "projected_relative_norm_upper": projected_upper,
        "actual_correlation_relative_norm_upper": min(full_upper, Q(1)),
        "full_bound_below_initial_norm": full_upper < 1,
        "collision_count": len(losses),
    }
