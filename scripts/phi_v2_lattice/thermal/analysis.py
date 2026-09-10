"""Exact bounded preflight, never a fitted fluid-recovery certificate.

Moment feasibility, stationary ensemble weights, and additive collision invariants
are separate mathematical objects. No simulation or numerical near-miss search.
"""
from __future__ import annotations

from fractions import Fraction as Q
from .runtime import VELOCITIES, ENERGY2, collision_classes, collision_identity


def d1q7_weights(theta: Q) -> dict[int, Q]:
    t = Q(theta)
    if not Q(3, 4) <= t <= Q(5, 4):
        raise ValueError("outside the analytically positive moment-feasibility interval")
    q = [(-15*t**3+42*t*t-49*t+36)/36,
         t*(5*t*t-13*t+12)/16,
         t*(-5*t*t+10*t-3)/40,
         t*(15*t*t-15*t+4)/720]
    return {v: q[abs(v)] for v in range(-3, 4)}


def stationary_occupations(fugacity=Q(1, 8), energy_activity=Q(1, 2)) -> tuple[Q, ...]:
    z, x = Q(fugacity), Q(energy_activity)
    if z <= 0 or x <= 0:
        raise ValueError("activities must be positive")
    odds = tuple(z*x**e for e in ENERGY2)
    return tuple(p/(1+p) for p in odds)


def equilibrium_moments(fugacity=Q(1, 8), energy_activity=Q(1, 2)) -> dict:
    f = stationary_occupations(fugacity, energy_activity)
    moment = lambda powers: sum((p * product_power(v, powers) for p, v in zip(f, VELOCITIES)), Q())
    m2, m4, m22 = moment((2, 0, 0)), moment((4, 0, 0)), moment((2, 2, 0))
    m6, m42, m222 = moment((6, 0, 0)), moment((4, 2, 0)), moment((2, 2, 2))
    return {"number": sum(f), "Mxx": m2, "Mxxxx": m4, "Mxxyy": m22,
            "Mxxxxxx": m6, "Mxxxxyy": m42, "Mxxyyzz": m222,
            "fourth_isotropy_residual": m4-3*m22,
            "sixth_isotropy_residual": m6-5*m42,
            "sixth_mixed_residual": m42-3*m222,
            "claim": "exact stationary ensemble moments; no relaxation or continuum closure"}


def product_power(v, powers):
    answer = 1
    for value, exponent in zip(v, powers):
        answer *= value**exponent
    return answer


def exactly_two_probability(occupations) -> Q:
    """Three coefficients of product[(1-f)+f t], using exact rationals."""
    probabilities = [Q(1), Q(0), Q(0)]
    for f in occupations:
        f = Q(f)
        if not 0 <= f <= 1:
            raise ValueError("invalid occupation")
        probabilities = [probabilities[0]*(1-f),
                         probabilities[1]*(1-f)+probabilities[0]*f,
                         probabilities[2]*(1-f)+probabilities[1]*f]
    return probabilities[2]


def collision_census() -> dict:
    """Modular rank lower bound + five known rational nullvectors.

    Exact elimination in a prime field, with no numerical rank tolerance. Stop
    once a minor reaches the known upper bound: processing more rows cannot
    strengthen that certificate. Every row is an admissible pair difference.
    """
    import numpy as np
    modulus = 1_000_003
    assert all(modulus % d for d in range(2, 1001))  # sqrt(modulus) < 1001
    basis, examined = {}, 0
    for members in collision_classes():
        a, b = members[0]
        for c, d in members[1:]:
            row = np.zeros(len(VELOCITIES), dtype=np.int64)
            for i, sign in ((c, 1), (d, 1), (a, -1), (b, -1)):
                row[i] += sign
            row %= modulus
            examined += 1
            for pivot, old in basis.items():
                if row[pivot]:
                    row = (row - row[pivot]*old) % modulus
            nonzero = np.flatnonzero(row)
            if len(nonzero):
                pivot = int(nonzero[0])
                basis[pivot] = row * pow(int(row[pivot]), -1, modulus) % modulus
            if len(basis) == len(VELOCITIES)-5:
                break
        if len(basis) == len(VELOCITIES)-5:
            break
    rank = len(basis)
    return {"law": "phi-thermal-pair-candidate-1", "table_hash": collision_identity(),
            "distinct_pairs": sum(map(len, collision_classes())), "classes": len(collision_classes()),
            "total_graph_rows": sum(len(m)-1 for m in collision_classes()),
            "rows_examined": examined, "prime": modulus, "rank_mod_prime": rank,
            "known_independent_rational_nullvectors": 5,
            "complete_additive_invariants": rank == len(VELOCITIES)-5,
            "scope": "rational additive velocity invariants only; no ergodicity or closure claim"}


def equivariant_collision_census() -> dict:
    """Rank of the selected v2 involution, not the graph of allowed collisions.

    All arithmetic is exact in a prime field. Five independent rational
    nullvectors (1, vx, vy, vz, E2) bound rank above by 338. A modular rank of
    338 proves the matching rational lower bound; no floating tolerance enters.
    """
    import numpy as np
    from .equivariant import construction, collision_identity, LAW_ID
    pairs, successor, _, momentum, energy = construction()
    if not (np.array_equal(momentum, momentum[successor])
            and np.array_equal(energy, energy[successor])):
        raise ValueError("claimed invariant upper bound is invalid")
    modulus = 1_000_003
    assert all(modulus % d for d in range(2, 1001))
    basis, examined = {}, 0
    for i, target in enumerate(successor):
        if i >= target:
            continue  # One row per actual nontrivial involution edge.
        a, b = pairs[i]
        c, d = pairs[target]
        row = np.zeros(len(VELOCITIES), dtype=np.int64)
        for index, sign in ((c, 1), (d, 1), (a, -1), (b, -1)):
            row[index] += sign
        row %= modulus
        examined += 1
        for pivot, old in basis.items():
            if row[pivot]:
                row = (row-row[pivot]*old) % modulus
        nonzero = np.flatnonzero(row)
        if len(nonzero):
            pivot = int(nonzero[0])
            basis[pivot] = row*pow(int(row[pivot]), -1, modulus) % modulus
        if len(basis) == len(VELOCITIES)-5:
            break
    rank = len(basis)
    return {"law": LAW_ID, "table_hash": collision_identity(),
            "distinct_pairs": len(pairs), "selected_edges": int(np.count_nonzero(successor != np.arange(len(pairs)))//2),
            "rows_examined": examined, "prime": modulus, "rank_mod_prime": rank,
            "known_independent_rational_nullvectors": 5,
            "complete_additive_invariants": rank == len(VELOCITIES)-5,
            "scope": "rational additive velocity invariants only; no ergodicity or closure claim"}


def equivariant_collision_activity(fugacity=Q(1, 8), energy_activity=Q(1, 2)) -> dict:
    """Exact one-site event probabilities in the stationary product ensemble.

    A rate per collision stage is not a mixing time or transport coefficient.
    These are analytical feasibility estimates, not time-series measurements.
    """
    from .equivariant import collision_map
    f = stationary_occupations(fugacity, energy_activity)
    odds = tuple(p/(1-p) for p in f)
    vacuum = Q(1)
    for p in f:
        vacuum *= 1-p
    total_weight, active_weight, thermal_weight = Q(), Q(), Q()
    for (a, b), (c, d) in collision_map().items():
        weight = odds[a]*odds[b]
        total_weight += weight
        if (a, b) != (c, d):
            active_weight += weight
        if sorted((ENERGY2[a], ENERGY2[b])) != sorted((ENERGY2[c], ENERGY2[d])):
            thermal_weight += weight
    return {"exactly_two": vacuum*total_weight, "changed_pair": vacuum*active_weight,
            "shell_energy_exchange": vacuum*thermal_weight,
            "scope": "stationary one-site collision-stage probabilities; no relaxation guarantee"}


def shear_tangent_obstruction() -> dict:
    """Exact first-order failure of autonomous product-marginal evolution.

    At positive stationary product occupation, tangent scores are additive in
    the channel bits. Collision fixes vacuum and every singleton, so if a
    pushed-forward score remains additive its coefficients must be unchanged.
    This actual collision changes the shear score h(v)=vx*vy. The nonzero
    Boolean mixed difference proves a correlation component at first order.
    Centering the score by its stationary expectation cancels in the difference.
    """
    from .runtime import channel
    from .equivariant import collision_map
    pair = tuple(channel(v) for v in ((-3, -3, -3), (-2, -2, -3)))
    expected = tuple(channel(v) for v in ((-3, -2, -3), (-2, -3, -3)))
    output = collision_map()[pair]
    if output != expected:
        raise ValueError("the frozen shear tangent witness no longer matches this law")
    shear = lambda c: VELOCITIES[c][0]*VELOCITIES[c][1]
    incoming, outgoing = sum(map(shear, pair)), sum(map(shear, output))
    return {"input": tuple(VELOCITIES[c] for c in pair),
            "output": tuple(VELOCITIES[c] for c in output),
            "incoming_score": incoming, "outgoing_score": outgoing,
            "pushed_score_mixed_difference": outgoing-incoming,
            "scope": "product tangent closure fails at first order; hydrodynamic approximation not ruled out"}


def shear_tangent_leakage(fugacity=Q(1, 8), energy_activity=Q(1, 2)) -> dict:
    """Exact loss under projection onto the full product tangent space.

    Norm: L2 of the positive stationary one-site product measure. Preparation:
    the infinitesimal centered score H=sum(vx*vy)(n_v-f_v), at the stated fixed
    activities. Horizon: one collision microtick only. This is a finite exact
    expectation, with no sample error, trajectory, continuum, or relaxation fit.

    C is an involution. The score increment delta=H(Cn)-H(n) is supported only
    on exactly-two states. With a_i=E[(n_i-f_i)delta] and var_i=f_i(1-f_i), the
    orthogonal projection of H(Cn) has coefficients h_i+a_i/var_i. Hence lost
    squared norm is E[delta^2]-sum(a_i^2/var_i). This retains all 343 marginal
    directions, not merely five conserved moments.
    """
    from .equivariant import collision_map
    f = stationary_occupations(fugacity, energy_activity)
    odds = tuple(p/(1-p) for p in f)
    variance = tuple(p*(1-p) for p in f)
    h = tuple(v[0]*v[1] for v in VELOCITIES)
    vacuum = Q(1)
    for p in f:
        vacuum *= 1-p
    weighted_mean, weighted_square = Q(), Q()
    weighted_marginals = [Q() for _ in f]
    for (a, b), (c, d) in collision_map().items():
        delta = h[c]+h[d]-h[a]-h[b]
        if delta == 0:
            continue
        weight = odds[a]*odds[b]
        weighted_mean += weight*delta
        weighted_square += weight*delta*delta
        weighted_marginals[a] += weight*delta
        weighted_marginals[b] += weight*delta
    if weighted_mean != 0:
        raise ValueError("stationary mean-score invariance failed")
    marginals = tuple(vacuum*w for w in weighted_marginals)
    increment_squared_norm = vacuum*weighted_square
    incoming_squared_norm = sum((hv*hv*var for hv, var in zip(h, variance)), Q())
    projected_squared_norm = sum(((hv*var+a)**2/var for hv, var, a in zip(h, variance, marginals)), Q())
    lost_squared_norm = increment_squared_norm-sum((a*a/var for a, var in zip(marginals, variance)), Q())
    if lost_squared_norm != incoming_squared_norm-projected_squared_norm or lost_squared_norm < 0:
        raise ValueError("orthogonal projection accounting failed")
    return {"fugacity": Q(fugacity), "energy_activity": Q(energy_activity),
            "incoming_squared_norm": incoming_squared_norm,
            "projected_squared_norm": projected_squared_norm,
            "lost_squared_norm": lost_squared_norm,
            "lost_squared_fraction": lost_squared_norm/incoming_squared_norm,
            "scope": "exact one-collision shear tangent projection loss; no multi-step or hydrodynamic error bound"}
