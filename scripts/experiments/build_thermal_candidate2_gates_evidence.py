"""Candidate-2 (equivariant) against candidate-1's three exact blockers.

Same definitions as candidate-1's negative witnesses in
scripts/tests/phi_v2_lattice/thermal/test_thermal.py, applied to BOTH
candidates side by side. Exact rationals throughout; floats for display only.
Emits the evidence JSON for engine/docs/evidence/.

This is a bounded exact check of three specified gates. It establishes no
relaxation, transport, equation of state, or fluid recovery, and it performs
no parameter or preparation search.

Run from the repository root, in module mode (scripts/ is a package):
    python -m scripts.experiments.build_thermal_candidate2_gates_evidence --out <json>
"""
from __future__ import annotations

import argparse
import inspect
import json
from fractions import Fraction as Q
from hashlib import sha256
from pathlib import Path

import numpy as np

from scripts.phi_v2_lattice.thermal import runtime as R
from scripts.phi_v2_lattice.thermal import equivariant as E
from scripts.phi_v2_lattice.thermal.analysis import (
    d1q7_weights, stationary_occupations, equilibrium_moments)

ID = "thermal-candidate2-gates-v1"
DATE = "2026-09-10"


def exact(q: Q) -> dict:
    s = str(q)
    return {"exact": s, "float": float(q),
            "sha256_of_str_fraction": sha256(s.encode("ascii")).hexdigest()}


def build() -> dict:
    cm1, cm2 = R.collision_map(), E.collision_map()
    pairs, successor2, images, momentum, energy = E.construction()
    by_code = {(int(a), int(b)): i for i, (a, b) in enumerate(pairs)}

    def get1(k):
        return cm1.get(k, k)  # candidate-1 leaves singleton buckets fixed

    successor1 = np.array([by_code[tuple(get1((int(a), int(b))))] for a, b in pairs],
                          dtype=np.int64)
    identity = np.arange(len(pairs))

    def vel(p):
        return [list(R.VELOCITIES[c]) for c in p]

    # ---- Gate 1: pointwise cubic covariance --------------------------------
    def rotate(c):
        return R.channel((R.VELOCITIES[c][0], -R.VELOCITIES[c][2], R.VELOCITIES[c][1]))

    pair = (R.channel((-1, 0, 0)), R.channel((1, 0, 0)))
    rin = tuple(sorted(map(rotate, pair)))
    assert rin == pair
    gate1 = {"witness": "R(x,y,z)=(x,-z,y) applied to {(-1,0,0),(1,0,0)}; R fixes the input pair"}
    for name, cm, succ in (("candidate-1", cm1, successor1), ("candidate-2", cm2, successor2)):
        F_in = tuple(cm.get(pair, pair))
        F_R_in = tuple(cm.get(rin, rin))
        R_F_in = tuple(sorted(map(rotate, F_in)))
        violations = sum(int(np.count_nonzero(succ[g] != g[succ])) for g in images)
        gate1[name] = {
            "witness": {"F(input)": vel(F_in), "F(R input)": vel(F_R_in),
                        "R(F input)": vel(R_F_in), "covariant": F_R_in == R_F_in},
            "exhaustive": {"group_elements": int(images.shape[0]), "pairs": int(len(pairs)),
                           "comparisons": int(images.size), "violations": violations,
                           "exactly_equivariant": violations == 0},
        }

    # ---- Gate 3: exact isotropy of the stationary Gibbs measure -------------
    m = equilibrium_moments()  # z=1/8, x=1/2
    gate3 = {
        "preparation": {"fugacity": "1/8", "energy_activity": "1/2"},
        "equilibrium_moments_signature": str(inspect.signature(equilibrium_moments)),
        "collision_map_enters_computation": False,
        "M400/(3*M220)": exact(m["Mxxxx"] / (3 * m["Mxxyy"])),
        "M600/(5*M420)": exact(m["Mxxxxxx"] / (5 * m["Mxxxxyy"])),
        "M420/(3*M222)": exact(m["Mxxxxyy"] / (3 * m["Mxxyyzz"])),
        "fourth_residual_nonzero": m["fourth_isotropy_residual"] != 0,
        "sixth_residual_nonzero": m["sixth_isotropy_residual"] != 0,
        "mixed_sixth_residual_nonzero": m["sixth_mixed_residual"] != 0,
        "structural_note": (
            "Both candidates' additive-invariant census is rank 338 = 343-5, i.e. exactly "
            "{1, vx, vy, vz, |v|^2}. The homogeneous stationary product family is therefore "
            "the same Fermi-Dirac f(|v|^2) for any collision in this invariant class, and these "
            "residuals are properties of that family on the {-3..3}^3 set at the fixed "
            "activities. No collision choice within the class moves this gate. This is not a "
            "claim about other velocity sets, other invariant classes, or other activities; "
            "no parameter search was run."),
    }

    # ---- Gate 2: moment-feasible D1Q7 tensor preparation stationarity -------
    q = d1q7_weights(Q(1))
    probs = [q[x] * q[y] * q[z] / 8 for x, y, z in R.VELOCITIES]
    odds = [f / (1 - f) for f in probs]
    p0 = (R.channel((0, 0, 0)), R.channel((2, 0, 0)))
    a, b = p0
    gate2 = {"preparation": "D1Q7 tensor weights q(vx)q(vy)q(vz)/8 at theta=1",
             "v1_witness_pair": vel(p0)}
    for name, cm in (("candidate-1", cm1), ("candidate-2", cm2)):
        c, d = cm.get(p0, p0)
        moved = nonstat = 0
        for (x, y), (u, w) in cm.items():
            if (x, y) == (u, w):
                continue
            moved += 1
            if odds[x] * odds[y] != odds[u] * odds[w]:
                nonstat += 1
        gate2[name] = {
            "v1_witness": {"output": vel((c, d)), "pair_fixed_under_this_map": (c, d) == p0,
                           "odds_ratio": exact(odds[a] * odds[b] / (odds[c] * odds[d]))},
            "exhaustive": {"moved_collisions": moved, "moved_with_nonstationary_ratio": nonstat,
                           "preparation_stationary": nonstat == 0},
        }

    # ---- sanity: the Gibbs family IS stationary under candidate-2 ----------
    f = stationary_occupations()
    go = [p / (1 - p) for p in f]
    bad = sum(1 for (x, y), (u, w) in cm2.items() if go[x] * go[y] != go[u] * go[w])

    return {
        "id": ID, "date": DATE,
        "instrument": {
            "path": "scripts/experiments/build_thermal_candidate2_gates_evidence.py",
            "sha256": sha256(Path(__file__).read_bytes()).hexdigest(),
            "definitions_from": ["scripts/phi_v2_lattice/thermal/analysis.py",
                                 "scripts/tests/phi_v2_lattice/thermal/test_thermal.py"],
            "arithmetic": "exact rationals (fractions.Fraction); floats for display only"},
        "laws": {
            "candidate-1": {"id": R.LAW_ID, "table_hash": R.collision_identity(),
                            "moved_pairs": int(np.count_nonzero(successor1 != identity))},
            "candidate-2": {"id": E.LAW_ID, "table_hash": E.collision_identity(),
                            "moved_pairs": int(np.count_nonzero(successor2 != identity))}},
        "gate1_pointwise_cubic_covariance": gate1,
        "gate2_moment_preparation_stationarity": gate2,
        "gate3_stationary_gibbs_isotropy": gate3,
        "sanity_gibbs_stationary_under_candidate2": {"violations": bad, "holds": bad == 0},
        "verdict": {
            "gate1": "candidate-2 CLOSES it: 0 violations in 2,815,344 (candidate-1: 1,682,522)",
            "gate2": ("candidate-2 FAILS it: 43,944 of 44,856 moved collisions nonstationary; "
                      "the v1 witness pair is fixed under v2 so that single ratio is 1, which is "
                      "why v1's witness could not be transferred blindly"),
            "gate3": "UNCHANGED and structurally collision-independent within the {N,P,E2} class",
        },
        "scope": ("Three specified exact gates only. No relaxation, equation of state, viscosity, "
                  "heat transport, thermalization, or fluid recovery is established or refuted. "
                  "No coefficient, preparation, or activity search was performed."),
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", type=Path)
    args = parser.parse_args()
    text = json.dumps(build(), indent=2) + "\n"
    if args.out:
        # LF explicitly: core.autocrlf normalizes on commit, so writing CRLF makes
        # the on-disk sha256 differ from the stored blob. Hashes cited from this
        # file must be the blob's. The c4-census evidence is stored LF-only.
        with open(args.out, "w", encoding="ascii", newline="\n") as fh:
            fh.write(text)
        print(f"wrote {args.out} ({len(text)} bytes)")
    else:
        print(text)


if __name__ == "__main__":
    main()
