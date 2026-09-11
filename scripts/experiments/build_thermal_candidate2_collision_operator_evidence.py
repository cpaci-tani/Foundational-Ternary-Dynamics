"""Exact linearized collision operator of phi-thermal-equivariant-candidate-2.

Run in module mode (scripts/ is a package):

    python -m scripts.experiments.build_thermal_candidate2_collision_operator_evidence --out PATH

Scope. One site, the fixed stationary product ensemble of record (fugacity
1/8, energy activity 1/2), one collision microtick, wavenumber zero. For an
additive centered score H(n) = sum_i h_i (n_i - f_i) the collision C maps H to
H(Cn); its orthogonal projection back onto the additive sector (in L2 of the
stationary one-site product measure) is linear in h. That linear map is

    M = I + V^-1 K,    K_ij = sum over moved pairs (a,b)->(c,d) with i in {a,b}
                             of mu({a,b}) * ([j in {c,d}] - [j in {a,b}]),

with V = diag(f_i (1 - f_i)) and mu({a,b}) the stationary probability that a
site holds exactly the channels a and b. M is the repeated product-projection
(linearized Boltzmann) collision operator at k = 0. Its eigenvalues are
per-collision-microtick retention factors of additive modes UNDER THAT
CLOSURE. The closure discards a non-additive component every collision; the
discarded fraction per score is reported as its leakage. Nothing here is a
relaxation time, transport coefficient, equation of state, or measured decay
of the actual law, and no parameter or preparation was searched.

Exact objects (rational arithmetic): K, M, the five invariant fixed points,
rank(M - I), the Rayleigh quotients, retained norms and leakages of named
scores, and the O_h orbit-average (A_1g) block with its characteristic
polynomial. Float64 objects (from the exact matrix, symmetric form): the full
eigenvalue clusters; these are structural readouts, not the values of record.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import platform
import sys
from datetime import date
from fractions import Fraction as Q
from itertools import permutations, product
from pathlib import Path

import numpy as np
import sympy as sp

from scripts.phi_v2_lattice.thermal import analysis as A
from scripts.phi_v2_lattice.thermal.equivariant import LAW_ID, collision_identity, collision_map
from scripts.phi_v2_lattice.thermal.runtime import CHANNELS, ENERGY2, VELOCITIES

EVIDENCE_ID = "thermal-candidate2-collision-operator-v1"
FUGACITY, ENERGY_ACTIVITY = Q(1, 8), Q(1, 2)


def rational(x: Q) -> str:
    return str(Q(x))


def build_operator():
    f = A.stationary_occupations(FUGACITY, ENERGY_ACTIVITY)
    odds = [p / (1 - p) for p in f]
    var = [p * (1 - p) for p in f]
    vac = Q(1)
    for p in f:
        vac *= 1 - p
    n = CHANNELS
    K = [[Q(0)] * n for _ in range(n)]
    moved = 0
    for (a, b), (c, d) in collision_map().items():
        if (a, b) == (c, d):
            continue
        moved += 1
        w = vac * odds[a] * odds[b]
        for i in (a, b):
            row = K[i]
            row[c] += w
            row[d] += w
            row[a] -= w
            row[b] -= w
    M = [[(Q(1) if i == j else Q(0)) + K[i][j] / var[i] for j in range(n)] for i in range(n)]
    return f, var, K, M, moved


def orbits_of_velocities():
    index = {v: i for i, v in enumerate(VELOCITIES)}
    orbit_of = [-1] * CHANNELS
    orbits = []
    for i, v in enumerate(VELOCITIES):
        if orbit_of[i] >= 0:
            continue
        members = set()
        for axes in permutations(range(3)):
            for signs in product((-1, 1), repeat=3):
                members.add(index[tuple(v[ax] * sg for ax, sg in zip(axes, signs))])
        for j in members:
            orbit_of[j] = len(orbits)
        orbits.append(sorted(members))
    return orbits, orbit_of


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()

    f, var, K, M, moved = build_operator()
    n = CHANNELS
    inner = lambda u, w: sum((var[i] * u[i] * w[i] for i in range(n)), Q(0))
    apply = lambda h: [sum((M[i][j] * h[j] for j in range(n)), Q(0)) for i in range(n)]

    # exact checks
    k_symmetric = all(K[i][j] == K[j][i] for i in range(n) for j in range(i + 1, n))
    invariants = {
        "1": [Q(1)] * n,
        "vx": [Q(v[0]) for v in VELOCITIES],
        "vy": [Q(v[1]) for v in VELOCITIES],
        "vz": [Q(v[2]) for v in VELOCITIES],
        "E2": [Q(e) for e in ENERGY2],
    }
    invariant_fixed = {name: apply(h) == h for name, h in invariants.items()}
    # Exact rank via python-flint (the project's tool for this; SymPy's rank on a
    # 343x343 rational matrix is impractically slow).
    import flint
    Mq = flint.fmpq_mat(n, n, [flint.fmpq(x.numerator, x.denominator) for row in M for x in row])
    Iq = flint.fmpq_mat(n, n, [flint.fmpq(1) if i == j else flint.fmpq(0) for i in range(n) for j in range(n)])
    rank_m_minus_i = int((Mq - Iq).rank())
    shear_ref = A.shear_tangent_leakage(FUGACITY, ENERGY_ACTIVITY)

    # All partners of each cubic irrep are listed so their exact equality is on record.
    scores = {
        "xy_T2g": [Q(v[0] * v[1]) for v in VELOCITIES],
        "yz_T2g": [Q(v[1] * v[2]) for v in VELOCITIES],
        "zx_T2g": [Q(v[2] * v[0]) for v in VELOCITIES],
        "x2_minus_y2_Eg": [Q(v[0] * v[0] - v[1] * v[1]) for v in VELOCITIES],
        "2z2_minus_x2_minus_y2_Eg": [Q(2 * v[2] * v[2] - v[0] * v[0] - v[1] * v[1]) for v in VELOCITIES],
        "E2_squared_A1g": [Q(e * e) for e in ENERGY2],
    }
    score_rows = {}
    for name, h in scores.items():
        Mh = apply(h)
        hh = inner(h, h)
        retained, rayleigh = inner(Mh, Mh) / hh, inner(Mh, h) / hh
        score_rows[name] = {
            "incoming_squared_norm": rational(hh),
            "retained_norm_fraction": rational(retained),
            "rayleigh_quotient": rational(rayleigh),
            "leakage_fraction": rational(1 - retained),
            "retained_norm_fraction_float": float(retained),
            "rayleigh_quotient_float": float(rayleigh),
            "leakage_fraction_float": float(1 - retained),
        }
    shear_matches_spec = (
        Q(score_rows["xy_T2g"]["incoming_squared_norm"]) == shear_ref["incoming_squared_norm"]
        and Q(score_rows["xy_T2g"]["retained_norm_fraction"]) * shear_ref["incoming_squared_norm"] == shear_ref["projected_squared_norm"]
    )

    # exact A_1g (orbit-average) block
    orbits, orbit_of = orbits_of_velocities()
    k = len(orbits)
    B = [[sum((M[orbits[Op][0]][j] for j in orbits[O]), Q(0)) for O in range(k)] for Op in range(k)]
    Bs = sp.Matrix(k, k, lambda r, c: sp.Rational(B[r][c].numerator, B[r][c].denominator))
    lam = sp.symbols("lam")
    charpoly = Bs.charpoly(lam)
    factors = sp.factor_list(charpoly.as_expr())[1]
    # The integer coefficients of the exact factors run to thousands of digits
    # once denominators are cleared (Python 3.11+ guards str() of such ints).
    # They are pinned by SHA-256 of their comma-joined decimal text and are
    # reproducible from this hashed builder; only degrees, digit counts and
    # 30-digit roots are stored inline.
    sys.set_int_max_str_digits(0)
    a1g_factors = []
    for fac, mult in factors:
        poly = sp.Poly(fac, lam)
        coeffs = [str(c) for c in poly.all_coeffs()]
        a1g_factors.append({
            "degree": int(poly.degree()),
            "multiplicity": int(mult),
            "integer_coefficients_sha256": hashlib.sha256(chr(44).join(coeffs).encode("ascii")).hexdigest(),
            "leading_coefficient_digits": len(coeffs[0]),
            "max_coefficient_digits": max(len(c) for c in coeffs),
            "real_roots_30_digits": sorted((str(sp.N(r, 30)) for r in poly.nroots(n=30) if abs(sp.im(r)) < 1e-25), reverse=True),
        })

    # float64 spectrum of the exact operator in symmetric form
    s = np.sqrt(np.array([float(x) for x in var]))
    Mf = np.array([[float(x) for x in row] for row in M])
    S = (s[:, None] * Mf) / s[None, :]
    ev = np.linalg.eigvalsh((S + S.T) / 2)[::-1]
    clusters = []
    for e in ev:
        if clusters and abs(e - clusters[-1][0]) < 1e-9:
            clusters[-1][1] += 1
        else:
            clusters.append([float(e), 1])
    activity = A.equivariant_collision_activity(FUGACITY, ENERGY_ACTIVITY)

    builder_sha = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    payload = {
        "id": EVIDENCE_ID,
        "date": date.today().isoformat(),
        "instrument": {
            "builder": "scripts/experiments/build_thermal_candidate2_collision_operator_evidence.py",
            "builder_sha256": builder_sha,
            "python": platform.python_version(),
            "sympy": sp.__version__,
            "numpy": np.__version__,
        },
        "law": {"id": LAW_ID, "collision_table_sha256": collision_identity(), "moved_pairs": moved},
        "preparation": {"fugacity": rational(FUGACITY), "energy_activity": rational(ENERGY_ACTIVITY),
                        "stationary_exactly_two": rational(activity["exactly_two"]),
                        "stationary_changed_pair": rational(activity["changed_pair"]),
                        "stationary_shell_energy_exchange": rational(activity["shell_energy_exchange"])},
        "operator": {
            "definition": "M = I + V^-1 K on additive centered scores; K_ij = sum over moved pairs (a,b)->(c,d), i in {a,b}, of mu({a,b}) ([j in {c,d}] - [j in {a,b}]); V = diag(f(1-f)); one site, one collision microtick, k = 0",
            "dimension": n,
            "K_symmetric_exact": k_symmetric,
            "invariants_exact_fixed_points": invariant_fixed,
            "rank_M_minus_I_exact": rank_m_minus_i,
            "rank_matches_additive_invariant_census_338": rank_m_minus_i == 338,
            "shear_norms_match_pinned_spec_instrument_exact": shear_matches_spec,
        },
        "scores_exact": score_rows,
        "a1g_orbit_average_block_exact": {
            "orbit_count": k,
            "orbits": [{"E2": ENERGY2[o[0]], "representative": list(VELOCITIES[o[0]]), "size": len(o)} for o in orbits],
            "charpoly_factors": a1g_factors,
        },
        "spectrum_float64": {
            "note": "eigenvalues of the exact operator in symmetric form, float64; structural readout only, not values of record",
            "min": float(ev.min()), "max": float(ev.max()),
            "distinct_clusters": len(clusters),
            "negative_eigenvalue_count": int(sum(m for v, m in clusters if v < 0)),
            "clusters_value_multiplicity": clusters,
        },
        "scope": (
            "Product-closure (linearized Boltzmann) collision operator at k=0 for one site at the fixed activities. "
            "Eigenvalues are per-collision-microtick retention factors of additive modes under that closure only. "
            "The closure discards a non-additive component each collision; each score's leakage_fraction is that discarded share. "
            "Establishes no relaxation time, viscosity, equation of state, heat transport, thermalization, or measured decay of the actual law. "
            "No coefficient, preparation, or activity search was performed."
        ),
    }
    text = json.dumps(payload, indent=2, ensure_ascii=True) + chr(10)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_bytes(text.encode("ascii"))
    print("wrote", args.out, "sha256", hashlib.sha256(args.out.read_bytes()).hexdigest(), "builder", builder_sha[:16])


if __name__ == "__main__":
    main()
