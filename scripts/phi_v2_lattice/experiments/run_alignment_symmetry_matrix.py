"""Complete registered cubic/phase formation matrix; Python finite-law tests.

Each transformed preparation executes the actual original and successor laws.
No reduced dynamics, physical units, random seeds, or fitted criterion is used.
"""
from __future__ import annotations

import os
for _name in ("OPENBLAS_NUM_THREADS", "OMP_NUM_THREADS", "MKL_NUM_THREADS"):
    os.environ[_name] = "1"

import argparse
from concurrent.futures import ProcessPoolExecutor, as_completed
import hashlib
import json
from pathlib import Path
import time

import numpy as np
from phi_v2_lattice import alignment as Tables, staged_alignment as A
from phi_v2_lattice import channels as C, geometry as G, staged as P, state as S, tick as T
from phi_v2_lattice._proofs import readout
from proof_moore_bond_capacity_type_census import signed_permutation_matrices, matrix_vector
from proof_shared_edge_hodge_flag_bcc_propagation import transform_flag

ROOT = Path(__file__).resolve().parents[3]
BACKGROUNDS = tuple(i for i in range(9) if i != S.BLANK_IDX)
CENTERS = ((3, 3, 3), (0, 0, 6))
HORIZON = 56
MATRICES = tuple(signed_permutation_matrices())
SIGNS = np.asarray([readout(S.z_of(i))[1] if i != S.BLANK_IDX else 0 for i in range(9)])


def require(condition, message):
    if not condition:
        raise ValueError(message)


def arrays(state):
    return tuple(getattr(state.lattice if name in A.NAMES[:5] else state, name) for name in A.NAMES)


def inventory(state):
    st = state.lattice
    return tuple(int(st.bank[:, offset:offset + 192].sum())
                 + int((SIGNS[st.sc] == eps).sum()) + int((SIGNS[st.fcc] == eps).sum())
                 for eps, offset in ((1, 0), (-1, 192)))


def run_group(group):
    gi, shift = group
    matrix = MATRICES[gi]
    mapping = tuple(C.STATE_INDEX[(transform_flag(matrix, flag), (k + shift) % 4)]
                    for flag, k in C.STATES)
    original_tables = C.load_collision_tables()
    Tables.load_collision_tables()
    final_digest = hashlib.sha256()
    cases = []
    for witness in ("contact", "separated", "nonrestoring"):
        for code in BACKGROUNDS:
            for eps, offset in ((1, 0), (-1, 192)):
                for center in CENTERS:
                    case = (gi, shift, witness, code, eps, center)
                    try:
                        st = S.blank(7)
                        st.sc.fill(code)
                        st.fcc.fill(code)
                        st.ell.fill(2 if witness == "separated" else 0)
                        entries = ({"contact": (((0, 0, 0), 0), ((0, 0, 0), 5)),
                                    "separated": (((0, -1, 0), 113), ((0, 1, 0), 94)),
                                    "nonrestoring": (((0, 0, 0), 0), ((1, 0, 0), 1))})[witness]
                        positions = []
                        for relative, channel in entries:
                            transformed = matrix_vector(matrix, relative)
                            xyz = tuple((center[i] + transformed[i]) % 7 for i in range(3))
                            positions.append(xyz)
                            st.bank[G.site_index(7, *xyz), mapping[channel] + offset] = True
                        state = A.initialize(st)
                        baseline = P.initialize(st)
                        site = G.site_index(7, *center)
                        if witness == "contact":
                            baseline.lattice.bank[site, offset + mapping[5]] = False
                            baseline.lattice.bank[site, offset + mapping[1]] = True
                        formation = 2 if witness == "contact" else 6
                        totals = inventory(state)
                        tracked_channels = [mapping[c] + offset for _, c in entries]
                        separation = tuple((b - a) % 7 for a, b in zip(*positions))
                        for tick in range(1, HORIZON + 1):
                            if witness == "separated" and tick == 6:
                                baseline.lattice.bank[site] = False
                                baseline.lattice.bank[site, [offset + mapping[C.U(C.U(c))]
                                                           for c in (0, 1)]] = True
                            state, events = A.step(state)
                            baseline, expected = P.step(baseline, original_tables)
                            require(state.microtick == baseline.microtick == tick, "clock")
                            for name, actual, reference in zip(A.NAMES, arrays(state), arrays(baseline)):
                                if name == "bank" and witness == "contact" and tick < formation:
                                    continue
                                require(np.array_equal(actual, reference), f"complete array {name} tick {tick}")
                            if witness != "nonrestoring" and tick == formation:
                                incoming = (0, 5) if witness == "contact" else (167, 170)
                                outgoing = (4, 5) if witness == "contact" else (166, 167)
                                require(events.collisions == [(site, eps,
                                    tuple(sorted(mapping[c] for c in incoming)),
                                    tuple(sorted(mapping[c] for c in outgoing)))], "formation event")
                                require(events.absorptions == expected.absorptions
                                        and events.crossings == expected.crossings
                                        and events.gate_holds == expected.gate_holds, "other events")
                            else:
                                require(events == expected, f"events tick {tick}")
                            require(not events.absorptions and not events.crossings and not events.gate_holds,
                                    "unexpected relation transaction")
                            require(inventory(state) == totals, "polarity inventory")
                            require(not state.lattice.s.any() and not state.admitted_sc.any(), "background")
                            sites, channels = np.nonzero(state.lattice.bank)
                            paired = (len(sites) == 2 and sites[0] == sites[1]
                                      and C.STATES[int(channels[0]) % 192][0]
                                      == C.STATES[int(channels[1]) % 192][0])
                            require(paired == (witness != "nonrestoring" and tick >= formation),
                                    "formation predicate")
                            if witness == "nonrestoring":
                                require(events == T.TickEvents(), "negative-control collision")
                                if tick % 4 == 3:
                                    tracked_channels = [C.U(c) for c in tracked_channels]
                                xy = [G.coords(7, int(np.flatnonzero(state.lattice.bank[:, c])[0]))
                                      for c in tracked_channels]
                                require(tuple((b - a) % 7 for a, b in zip(*xy)) == separation,
                                        "negative-control restoring response")
                        restored = A.restore(A.checkpoint(state))
                        require(A.checkpoint(restored) == A.checkpoint(state), "checkpoint")
                        final_digest.update(A.checkpoint(state))
                        cases.append(case)
                    except Exception as error:
                        raise RuntimeError(f"case={case}: {error}") from error
    return {"matrix_index": gi, "phase_shift": shift, "cases": len(cases),
            "successor_microticks": len(cases) * HORIZON,
            "final_checkpoint_digest": final_digest.hexdigest()}


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--workers", type=int, default=24)
    args = parser.parse_args()
    require(args.workers > 0, "workers must be positive")
    directory = args.output
    require(not directory.exists(), "output exists; preserve prior attempt")
    directory.mkdir(parents=True)
    source_files = [Path(__file__), ROOT / "scripts/phi_v2_lattice/alignment.py",
                    ROOT / "scripts/phi_v2_lattice/staged_alignment.py"]
    baseline_path = ROOT / "engine/docs/evidence/strict-recovery-wave4-baseline-2026-09-08.json"
    baseline_bytes = baseline_path.read_bytes()
    baseline = json.loads(baseline_bytes)
    baseline_drift = [p for p, expected in baseline["source_sha256"].items()
                      if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != expected.lower()]
    require(not baseline_drift, f"pre-existing baseline source drift: {baseline_drift}")
    paths = sorted(set(source_files + [ROOT / p for p in baseline["source_sha256"]]))
    hashes = {p.relative_to(ROOT).as_posix(): hashlib.sha256(p.read_bytes()).hexdigest() for p in paths}
    lock = {"schema": "alignment-symmetry-matrix-1", "law": A.LAW_ID,
            "baseline_sha256": hashlib.sha256(baseline_bytes).hexdigest(),
            "baseline_source_count": len(baseline["source_sha256"]),
            "baseline_drift": baseline_drift,
            "table": A.COLLISION_HASH, "checkpoint_schema": A.SCHEMA,
            "groups": [[g, s] for g in range(48) for s in range(4)],
            "backgrounds": list(BACKGROUNDS), "polarities": [1, -1],
            "centers": CENTERS, "witnesses": ["contact", "separated", "nonrestoring"],
            "horizon_microticks": HORIZON, "source_sha256": hashes}
    (directory / "lock.json").write_text(json.dumps(lock, indent=2, sort_keys=True) + "\n")
    started = time.monotonic()
    results = []
    with ProcessPoolExecutor(max_workers=args.workers) as pool:
        futures = {pool.submit(run_group, tuple(group)): group for group in lock["groups"]}
        for future in as_completed(futures):
            group = futures[future]
            try:
                result = future.result()
            except Exception as error:
                failure = {"group": group, "error": repr(error), "completed_groups": len(results)}
                (directory / "failure.json").write_text(json.dumps(failure, indent=2) + "\n")
                for pending in futures:
                    pending.cancel()
                raise
            results.append(result)
            (directory / f"group-{group[0]:02d}-{group[1]}.json").write_text(json.dumps(result, sort_keys=True) + "\n")
            if len(results) % 8 == 0:
                print(json.dumps({"completed_groups": len(results), "total_groups": 192,
                                  "elapsed_seconds": round(time.monotonic() - started, 2)}), flush=True)
    drift = [p for p, expected in hashes.items() if hashlib.sha256((ROOT / p).read_bytes()).hexdigest() != expected]
    require(not drift, f"source drift during matrix: {drift}")
    report = {"schema": "alignment-symmetry-matrix-result-1", "law": A.LAW_ID,
              "passed": True, "case_count": sum(r["cases"] for r in results),
              "successor_microticks": sum(r["successor_microticks"] for r in results),
              "horizon_microticks": HORIZON, "source_drift": drift,
              "elapsed_seconds": time.monotonic() - started,
              "groups": sorted(results, key=lambda r: (r["matrix_index"], r["phase_shift"])),
              "physical_binding_identified": False}
    (directory / "report.json").write_text(json.dumps(report, indent=2, sort_keys=True) + "\n")
    print(json.dumps({k: v for k, v in report.items() if k != "groups"}), flush=True)


if __name__ == "__main__":
    main()
