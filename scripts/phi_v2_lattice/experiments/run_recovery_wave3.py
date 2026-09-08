# scripts/phi_v2_lattice/experiments/run_recovery_wave3.py
"""Exact wave-3 evidence: invariant census (H0) and dispersion verdict (H1)."""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import sys

import flint
import numpy as np

from .. import channels as C, staged as P
from .. import recovery_hydro_invariants as H
from .. import recovery_hydro_dispersion as D
from .. import recovery_hydro_verdict as V


def _entries(M, n, m, exact=True):
    if exact:
        return [[str(Fraction(int(M[i, j].p), int(M[i, j].q))) for j in range(m)] for i in range(n)]
    return [[{"mid": str(M[i, j].mid()), "rad": str(M[i, j].rad())} for j in range(m)] for i in range(n)]


def report() -> dict:
    census = H.census()
    exact = {n: D.exact_dispersion(n) for n in D.DIRECTIONS}
    verdict = V.exact_verdict(exact)
    block = V.closure_block(V.density_functional(exact[D.DIRECTIONS[0]].W),
                            [m for n in D.DIRECTIONS for m in (exact[n].M1, exact[n].M2)])
    mode_table_float64 = {str(n): V.mode_table_float64(exact[n].M1, exact[n].M2, n) for n in D.DIRECTIONS}
    certified, residuals, certified_verdicts = {}, {}, {}
    for p in D.REFERENCE_PS:
        balls = {n: D.certified_dispersion(n, p, exact[n]) for n in D.DIRECTIONS}
        certified[str(p)] = {str(n): {"M1": _entries(balls[n].M1, 7, 7, False),
                                      "M2": _entries(balls[n].M2, 7, 7, False)} for n in D.DIRECTIONS}
        residuals[str(p)] = {str(n): {key: str(value) for key, value in D.certified_residuals(balls[n]).items()}
                             for n in D.DIRECTIONS}
        certified_verdicts[str(p)] = V.certified_verdict(balls, block)
    # Unit-modulus spectrum of P0 at the proxy coupling (float64 report only, never an acceptance):
    # eigenvalues with |lambda| > 1 - 1e-9 other than the seven at 1 indicate staggered modes.
    P0 = exact[D.DIRECTIONS[0]].P0
    dense = np.array([[float(Fraction(int(P0[i, j].p), int(P0[i, j].q))) for j in range(192)] for i in range(192)])
    values = np.linalg.eigvals(dense)
    unit_modulus = sorted(((float(v.real), float(v.imag)) for v in values if abs(v) > 1 - 1e-9),
                          key=lambda z: (round(z[0], 9), round(z[1], 9)))
    result = {
        "schema": "strict-recovery-wave3-exact-1", "law_id": P.LAW_ID, "collision_sha256": C.COLLISION_HASH,
        "calculation_kind": "exact_finite_operators_and_certified_enclosures_not_a_measurement_campaign",
        "invariants": census,
        "dispersion": {
            "unit_modulus_spectrum_float64_report": {"count": len(unit_modulus), "values": unit_modulus,
                                                     "note": "float64 report; seven eigenvalues at 1 are the conserved modes"},
            "proxy_r": str(D.PROXY_R),
            "physical_r": {str(p): str(D.physical_r(p)) for p in D.REFERENCE_PS},
            "physical_r_float": {str(p): float(D.physical_r(p)) for p in D.REFERENCE_PS},
            "exact": {str(n): {"rank_complement": exact[n].rank_complement,
                               "M1": _entries(exact[n].M1, 7, 7), "M2": _entries(exact[n].M2, 7, 7)}
                      for n in D.DIRECTIONS},
            "certified": certified, "certified_residuals": residuals},
        "verdict": {"exact": verdict, "certified": certified_verdicts,
                   "mode_table_float64": mode_table_float64},
        "limits": {"closure": "linearized Boltzmann product closure assumed",
                   "correlation_leakage_bounded": False, "physical_units": False,
                   "navier_stokes_clay_claim": False, "canonical_adoption": False},
    }
    root = Path(__file__).resolve().parents[3]
    sources = {}
    for module in tuple(sys.modules.values()):
        path = getattr(module, "__file__", None)
        if path:
            path = Path(path).resolve()
            if path.suffix == ".py" and path.is_relative_to(root):
                sources[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    result["source_sha256"] = dict(sorted(sources.items()))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report(), indent=2, sort_keys=True) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
