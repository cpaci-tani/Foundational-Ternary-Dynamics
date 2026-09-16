"""Reproduce the locked finite-wavelength gate; exclusive-create evidence only.

python -m scripts.experiments.build_thermal_candidate2_finite_wavelength_evidence --out PATH.json
Writes PATH.json and a deterministic PATH.exact.json.gz containing exact data.
No fitting, eigenspectrum, simulation, or physical admission is performed.
"""
from __future__ import annotations

import argparse
from fractions import Fraction as Q
import gzip
import hashlib
import json
from math import lcm
from pathlib import Path
import platform
import sys

import flint

from scripts.phi_v2_lattice.thermal import finite_wavelength as F
from scripts.phi_v2_lattice.thermal.equivariant import LAW_ID

ROOT = Path(__file__).resolve().parents[2]
LOCK = "engine/docs/PREREG_THERMAL_CANDIDATE2_FINITE_WAVELENGTH_V1.md"
LOCK_HASH = "ce4f6028140ee8c09f1a13448ce27fc35184a94c0d0c4dcd7f86cf5f21e86fee"
SOURCES = (
    LOCK,
    "scripts/experiments/build_thermal_candidate2_finite_wavelength_evidence.py",
    "scripts/phi_v2_lattice/thermal/finite_wavelength.py",
    "scripts/phi_v2_lattice/thermal/analysis.py",
    "scripts/phi_v2_lattice/thermal/equivariant.py",
    "scripts/phi_v2_lattice/thermal/runtime.py",
    "scripts/phi_v2_lattice/thermal/runtime_v2.py",
)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def json_bytes(payload):
    return (json.dumps(payload, sort_keys=True, separators=(",", ":"), ensure_ascii=True)+"\n").encode("ascii")


def polynomial(poly):
    """Unique common positive denominator and eight ascending integer coefficients."""
    denominator = lcm(*(x.denominator for x in poly))
    return {"denominator": str(denominator), "numerators": [str(int(x*denominator)) for x in poly]}


def outward(value, upper=True):
    denominator = 10**12
    scaled = value*denominator
    n = (-(-scaled.numerator//scaled.denominator) if upper
         else scaled.numerator//scaled.denominator)
    return str(Q(n, denominator))


def small_budget(row):
    result = {}
    for key, value in row.items():
        if key == "retained_fraction_interval":
            result[key] = [outward(value[0], False), outward(value[1])]
        elif isinstance(value, Q):
            result[key] = outward(value)
        else:
            result[key] = value
    return result


def score_digest(scores):
    """Canonical common-denominator matrix digest (unreduced owned representation)."""
    h = hashlib.sha256()
    h.update((str(scores.denominator)+"\n").encode("ascii"))
    for i in range(F.CHANNELS):
        h.update((",".join(str(scores.coefficients[i, j])
                           for j in range(scores.coefficients.ncols()))+"\n").encode("ascii"))
    return h.hexdigest()


def build():
    # Exact coefficient strings exceed Python's default defensive digit cap.
    sys.set_int_max_str_digits(0)
    sources = {path: digest((ROOT/path).read_bytes()) for path in SOURCES}
    if sources[LOCK] != LOCK_HASH:
        raise ValueError("preregistration changed; use a new named gate")
    factors = F.collision_factors()
    if any(factors.correction[i][j] != factors.correction[j][i]
           for i in range(F.CHANNELS) for j in range(F.CHANNELS)):
        raise ValueError("collision factor is not symmetric")
    initial = F.initial_scores()
    initial_norms = [F.norm_squared(initial, j) for j in range(initial.count)]
    exact = {
        "encoding": "Q[zeta]/(zeta^8+1), zeta=exp(2*pi*i/16); ascending powers; integer coefficients/common positive denominator",
        "collision": {
            "definition": "M=I+vacuum/2^60*diag(1/variance)*B; rows output score channel, columns input score channel",
            "B_integer_rows": factors.correction,
            "vacuum": str(factors.vacuum),
            "variance": [str(x) for x in factors.variance],
        },
        "initial_scores": dict(zip(F.SCORE_NAMES, F.score_values())),
        "initial_squared_norms": [polynomial(x) for x in initial_norms],
        "wavevectors": [],
    }
    cases = []
    for m in F.WAVEVECTORS:
        print("computing locked m =", m, flush=True)
        h = F.initial_scores()
        previous = initial_norms
        losses = [[] for _ in F.SCORE_NAMES]
        collisions, endpoints, hashes = [], [], []
        for tick in range(17):
            if tick:
                phase = (tick-1) % 4
                h = F.advance(h, m, tick-1, 1)
                if phase == 0:
                    current = [F.norm_squared(h, j) for j in range(h.count)]
                    increment = [F.subtract(a, b) for a, b in zip(previous, current)]
                    for j, loss in enumerate(increment):
                        F.nonnegative_interval(loss)
                        losses[j].append(loss)
                        if F.subtract(initial_norms[j], current[j]) != tuple(
                                sum((entry[p] for entry in losses[j]), Q(0)) for p in range(F.DEGREE)):
                            raise ValueError("exact telescoping norm account failed")
                    collisions.append({"input_clock": tick-1,
                                       "retained_squared_norms": [polynomial(x) for x in current],
                                       "discarded_squared_norms": [polynomial(x) for x in increment]})
                    previous = current
                else:
                    # Recompute rather than merely asserting streaming unitarity.
                    current = [F.norm_squared(h, j) for j in range(h.count)]
                    if current != previous:
                        raise ValueError("exact streaming norm invariance failed")
            endpoints.append({"clock": tick, "phase": tick % 4, "scores": {
                name: small_budget(F.budget_row(initial_norms[j][0], previous[j], losses[j]))
                for j, name in enumerate(F.SCORE_NAMES)}})
            hashes.append({"clock": tick, "score_matrix_sha256": score_digest(h)})
        exact["wavevectors"].append({
            "m": m, "streaming_zeta_exponents_q123": [F.phase_exponents(m, q) for q in (1, 2, 3)],
            "cycle_definition": "T(k)=D3(k) D2(k) D1(k) M; every T_ij=zeta^(-m.v_i)*M_ij",
            "collisions": collisions, "endpoint_score_hashes": hashes,
        })
        cases.append({"m": m, "endpoints": endpoints})
    payload = {
        "id": "thermal-candidate2-finite-wavelength-v1",
        "date": "2026-09-13",
        "law_id": LAW_ID, "collision_table_sha256": F.TABLE_HASH,
        "sources_sha256": sources,
        "instrument": {"python": platform.python_version(), "python_flint": flint.__version__},
        "domain": {"L": F.L, "fugacity": "1/8", "energy_activity": "1/2", "start_tick": 0,
                   "last_tick": 16, "wavevectors_m": F.WAVEVECTORS, "score_names": F.SCORE_NAMES},
        "norm": "stationary full-bank complex L2 score, Fourier normalization L^(-3/2); squared norm sum_i variance_i*abs(h_i)^2",
        "certification": {
            "exact_projected_finite_wavelength_transfer": True,
            "rigorous_finite_time_linear_score_comparison_bound": True,
            "full_state_product_closure": False, "actual_multistep_correlations_computed": False,
            "continuum_limit": False, "thermal_relaxation": False, "transport": False,
            "canonical_adoption": False, "public_fluid_admission": "NONE",
        },
        "bounds": {
            "proof": "unitary Duhamel telescoping; residual norms summed without time-orthogonality; full bound capped by 1+sqrt(retention); projected-return bound excludes most recent residual",
            "rational_enclosures": "nested square roots of 2 and 2+/-sqrt(2), enclosed by integer-square inequalities at denominator 10^40; reported endpoints rounded outward to grid 10^-12",
            "interpretation": "discarded squared norms belong to repeated projection; actual correlations can return at later collisions; a bound >=1 is loose, not a transport no-go",
        },
        "cases": cases,
    }
    if sources != {path: digest((ROOT/path).read_bytes()) for path in SOURCES}:
        raise ValueError("instrument sources changed during computation")
    return payload, exact


def write_evidence(out, payload, exact):
    """Never replace either member of an evidence pair, including dangling links."""
    exact_path = out.with_suffix(".exact.json.gz")
    if out.exists() or out.is_symlink() or exact_path.exists() or exact_path.is_symlink():
        raise FileExistsError("immutable evidence already exists")
    exact_bytes = gzip.compress(json_bytes(exact), compresslevel=9, mtime=0)
    payload = dict(payload, exact_artifact={"file": exact_path.name, "sha256": digest(exact_bytes),
                                          "uncompressed_sha256": digest(json_bytes(exact))})
    data = (json.dumps(payload, indent=2, sort_keys=True)+"\n").encode("ascii")
    out.parent.mkdir(parents=True, exist_ok=True)
    with exact_path.open("xb") as stream:
        stream.write(exact_bytes)
    with out.open("xb") as stream:
        stream.write(data)
    print("receipt", out, "sha256", digest(data), flush=True)
    print("exact", exact_path, "sha256", digest(exact_bytes), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    if args.out.exists() or args.out.with_suffix(".exact.json.gz").exists():
        parser.error("immutable evidence already exists")
    payload, exact = build()
    write_evidence(args.out, payload, exact)


if __name__ == "__main__":
    main()
