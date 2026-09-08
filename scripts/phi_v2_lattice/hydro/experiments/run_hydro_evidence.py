# scripts/phi_v2_lattice/hydro/experiments/run_hydro_evidence.py
"""H1-prime exact evidence: census, verdict, nonlinear coefficients, and the P0 spectrum
of `phi-hydro-staged-candidate-1` at d in {1/4, 1/8, 3/8} (gate H1' of
`docs/superpowers/specs/2026-09-08-hydro-scenarios-and-fchc-successor-design.md` sec A.3,
contract `engine/docs/DERIV_STRICT_HYDRO_DISPERSION.md`).

`--check` re-runs the report a second time in the same process and asserts the two
outputs are byte-identical after dropping `runtime_seconds` from each, without writing
the second run to disk.
"""
from __future__ import annotations

import argparse
from fractions import Fraction
import hashlib
import json
from pathlib import Path
import time

import flint
import numpy as np

from .. import boltzmann as B
from .. import channels as C
from .. import invariants as I

_DENSITIES = (Fraction(1, 4), Fraction(1, 8), Fraction(3, 8))
_ROOT = Path(__file__).resolve().parents[4]
_CONTRACT_PATH = _ROOT / "engine" / "docs" / "DERIV_STRICT_HYDRO_DISPERSION.md"
_MARKER = "<!-- RESULTS MARKER: nothing above this line changes after results exist -->"


def _fmat_to_strings(M) -> list[list[str]]:
    return [[str(Fraction(int(M[i, j].p), int(M[i, j].q))) for j in range(M.ncols())] for i in range(M.nrows())]


def _contract_sha256_pre_marker() -> str:
    """SHA256 of the contract's bytes up to and including the marker line."""
    data = _CONTRACT_PATH.read_bytes()
    text = data.decode("utf-8")
    idx = text.index(_MARKER)
    end = idx + len(_MARKER)
    # include the trailing newline that terminates the marker line, if present
    if end < len(text) and text[end] == "\n":
        end += 1
    prefix = text[:end].encode("utf-8")
    return hashlib.sha256(prefix).hexdigest()


def _p0_spectrum(direction, d: Fraction) -> dict:
    """float64 eigenvalue report of P0 = J(d) (booking only, never an acceptance value):
    count of eigenvalues with |lambda - 1| < 1e-12, the largest modulus among the rest,
    and every eigenvalue sorted by (real, imag). J does not depend on `direction` (only
    A1, A2 do), so any declared direction gives the same P0 at a given d."""
    P0 = B.period_series(direction, d)[0]
    dense = np.array([[float(Fraction(int(P0[i, j].p), int(P0[i, j].q))) for j in range(B.N)] for i in range(B.N)])
    values = np.linalg.eigvals(dense)
    unit = [v for v in values if abs(v - 1) < 1e-12]
    rest = [v for v in values if abs(v - 1) >= 1e-12]
    rest_modulus = max((abs(v) for v in rest), default=0.0)
    sorted_values = sorted(((float(v.real), float(v.imag)) for v in values), key=lambda z: (round(z[0], 12), round(z[1], 12)))
    return {
        "unit_eigenvalue_count": len(unit),
        "largest_non_unit_modulus": float(rest_modulus),
        "eigenvalues_sorted": sorted_values,
    }


def _density_report(d: Fraction) -> dict:
    v = B.verdict(d)
    nl = {k: str(val) for k, val in B.nonlinear_coefficients(d).items()}
    m1_m2 = {}
    for n in B.DIRECTIONS:
        disp = B.dispersion(n, d)
        m1_m2[str(n)] = {"M1": _fmat_to_strings(disp.M1), "M2": _fmat_to_strings(disp.M2)}
    return {
        "verdict": v,
        "nonlinear_coefficients": nl,
        "P0_spectrum_float64_report": _p0_spectrum(B.DIRECTIONS[0], d),
        "M1_M2": m1_m2,
    }


def _source_sha256() -> dict:
    hydro_dir = Path(B.__file__).resolve().parent
    names = ("boltzmann.py", "tables.py", "channels.py", "invariants.py")
    return {name: hashlib.sha256((hydro_dir / name).read_bytes()).hexdigest() for name in names}


def report() -> dict:
    started = time.time()
    census = I.census()
    result = {
        "law": census["law"],
        "table_hash": C.TABLE_HASH,
        "contract_sha256_pre_marker": _contract_sha256_pre_marker(),
        "census": census,
        "densities": {str(d): _density_report(d) for d in _DENSITIES},
        "source_sha256": _source_sha256(),
    }
    result["runtime_seconds"] = time.time() - started
    return result


def _drop_runtime(payload: dict) -> dict:
    payload = dict(payload)
    payload.pop("runtime_seconds", None)
    return payload


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=_ROOT / "engine" / "docs" / "evidence" / "strict-hydro4-dispersion-exact.json")
    parser.add_argument("--check", action="store_true", help="re-run in-process and assert byte-identical output (runtime_seconds dropped)")
    args = parser.parse_args()

    first = report()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(first, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    if args.check:
        second = report()
        a = json.dumps(_drop_runtime(first), indent=2, sort_keys=True)
        b = json.dumps(_drop_runtime(second), indent=2, sort_keys=True)
        if a != b:
            raise SystemExit("determinism check FAILED: two in-process runs differ (excluding runtime_seconds)")
        print("determinism check OK: two in-process runs are byte-identical (excluding runtime_seconds)")


if __name__ == "__main__":
    main()
