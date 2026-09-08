"""Reproduce bounded foundation evidence; stdout JSON unless --output is named.

Run with PYTHONPATH=scripts: python -m phi_v2_lattice.experiments.run_strict_foundation
This is a fixed regression witness, not a numerical model/constant search.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import sys

import numpy as np

from .. import channels as C, continuum_contract as F, geometry as G
from .. import resolved as B, staged as P, state as S, tick as T


def report() -> dict:
    tables = C.load_collision_tables()
    a = S.blank(7)
    x, y, z = (G.site_index(7, *p) for p in ((3, 3, 3), (4, 3, 3), (2, 3, 3)))
    a.bank[x, [0, 34]] = True
    b = S.copy(a)
    b.bank[y, 2] = True
    ax, _ = T.tick(a, tables)
    bx, _ = T.tick(b, tables)
    observed = [np.flatnonzero(st.bank[z]).tolist() for st in (ax, bx)]
    if observed != [[113], [115]]:
        raise RuntimeError("reference witness changed: inspect law/table/version before proceeding")

    cycles = []
    for name, reference in (("uncontested", a), ("contested", b)):
        candidate = P.initialize(reference)
        initial_tokens = P.work_units(candidate)
        fingerprints = []
        for cycle in range(3):
            expected, _ = T.tick(reference, tables)
            for stage in range(4):
                before = candidate
                candidate, events = P.step(candidate, tables)
                saved = P.checkpoint(candidate)
                if P.checkpoint(P.restore(saved)) != saved:
                    raise RuntimeError("complete checkpoint mismatch")
                if P.work_units(candidate) != initial_tokens:
                    raise RuntimeError("token accounting failure")
                # Width one on L7 catches every incidence change; a whole-box
                # observation alone would permit cancelling local errors.
                transfers = B.boundary_transfers(before, candidate, events, 1)
                if any(transfers.residual()):
                    raise RuntimeError("incidence current balance failure")
                fingerprints.append(hashlib.sha256(saved).hexdigest())
            for field in ("s", "ell", "bank", "sc", "fcc"):
                if not np.array_equal(getattr(expected, field), getattr(candidate.lattice, field)):
                    raise RuntimeError(f"reference cycle comparison failed: {field}")
            reference = expected
        cycles.append({"preparation": name, "cycles": 3, "microticks": 12,
                       "tokens": initial_tokens, "checkpoint_sha256": fingerprints})

    witness = F.product_tangent_witness(tables[0])
    budget = F.half_reference_event_budget(10 ** 18)
    root = Path(__file__).resolve().parents[3]
    sources = {}
    # Record actually imported project sources, including the proof helpers
    # used to define channels and records. No credentials/environment exported.
    for module in tuple(sys.modules.values()):
        raw = getattr(module, "__file__", None)
        if not raw:
            continue
        path = Path(raw).resolve()
        if path.suffix == ".py" and path.is_relative_to(root):
            sources[path.relative_to(root).as_posix()] = hashlib.sha256(path.read_bytes()).hexdigest()
    return {
        "schema": "strict-foundation-evidence-1",
        "law_id": P.LAW_ID,
        "collision_sha256": C.COLLISION_HASH,
        "reference_radius_one_gate": "FAIL: exact distance-two witness",
        "reference_witness_output_banks": observed,
        "candidate_execution_scope": "two L7 preparations, three cycles each; not universal proof",
        "candidate_cycles": cycles,
        "product_tangent_mixed_difference": witness.mixed_difference,
        "conditional_half_reference_any_event_upper_bound": str(budget.probability_any_upper_bound),
        "trajectory_continuum_certified": False,
        "canonical_adoption": False,
        "source_sha256": dict(sorted(sources.items())),
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    encoded = json.dumps(report(), indent=2, sort_keys=True) + "\n"
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(encoded, encoding="utf-8")
    else:
        print(encoded, end="")


if __name__ == "__main__":
    main()
