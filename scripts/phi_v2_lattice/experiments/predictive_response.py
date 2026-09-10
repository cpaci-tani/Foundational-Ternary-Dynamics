"""Locally preregistered one-cycle kinetic response, not a continuum certificate.

Commands: register DIRECTORY; prepare-input LOCK; summarize LOCK INPUT WASM OUTPUT.
Registration must precede entropy generation and every microscopic execution.
"""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
from fractions import Fraction as F
import hashlib
import json
from math import factorial
import os
from pathlib import Path
import subprocess
import sys

import numpy as np

ROOT = Path(__file__).resolve().parents[3]
if str(ROOT / "scripts") not in sys.path:
    sys.path.insert(0, str(ROOT / "scripts"))
from phi_v2_lattice.hydro import channels as H  # noqa: E402

SCHEMA = "ftd-hydro-predictive-response-v1"
TABLE = "engine/build_strict_hydro_tables/hydro_collision_abf25cf26072c03b.u32"
RUNTIME = {"table": TABLE, "module": "engine/build_strict_hydro_wasm/ftd_hydro_wasm.mjs",
           "binary": "engine/build_strict_hydro_wasm/ftd_hydro_wasm.wasm"}
L, REPLICATES, N = 16, 32, 16 ** 3 * 32
RADIUS = F(3, 256)


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def write_new(path, value):
    with Path(path).open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(value, stream, sort_keys=True, indent=2, allow_nan=False)
        stream.write("\n")


def profile():
    xyz = np.indices((L, L, L))
    return np.where((xyz[0] + 2 * xyz[1] + 3 * xyz[2]) % L < 8, 1, -1).astype(np.int8)


def column_counts(table, channels=24):
    """Exact uniform conditional output counts; source channel fixed to bit zero."""
    if len(table) != 1 << channels:
        raise ValueError("complete local alphabet required")
    even, odd = [0] * channels, [0] * channels
    for start in range(0, len(table), 1 << 18):
        chunk = table[start:start + (1 << 18)]
        for c in range(channels):
            even[c] += int(np.count_nonzero((chunk[::2] >> c) & 1))
            odd[c] += int(np.count_nonzero((chunk[1::2] >> c) & 1))
    return even, odd


def exact_prediction(table):
    even, odd = column_counts(table)
    denominator = 1 << 23
    if any(a + b != denominator for a, b in zip(even, odd)):
        raise ValueError("uniform half-occupation output marginal is not stationary")
    numerators = [b - a for a, b in zip(even, odd)]
    J = [F(value, denominator) for value in numerators]
    a = profile()
    response, reverse = [], []
    for c, velocity in enumerate(H.VELOCITIES):
        corr = F(int((a * np.roll(a, tuple(-2 * v for v in velocity), axis=(0, 1, 2))).sum()), L ** 3)
        response.append((J[c] - int(c == 0)) / 4)
        reverse.append((corr * J[c] - int(c == 0)) / 4)
    return {"column_numerators": numerators, "column_denominator": denominator,
            "conditional_output_ones": {"input_zero": even, "input_one": odd},
            "response": list(map(str, response)), "reverse_stream": list(map(str, reverse)),
            "no_collision": ["0"] * 24}


def source_paths():
    # All Python candidate sources are pinned, including transitive definitions.
    paths = list((ROOT / "scripts/phi_v2_lattice").rglob("*.py"))
    paths += list((ROOT / "engine/strict/hydro").glob("*.cpp"))
    paths += list((ROOT / "engine/strict/hydro").glob("*.h"))
    paths += [ROOT / "engine/strict/frozen_tables.h",
              ROOT / "engine/strict/hydro/predictive_response_runner.mjs",
              ROOT / "engine/build_predictive_response/tools/node-v24.11.0-linux-x64/bin/node",
              ROOT / "engine/docs/PREREG_STRICT_PREDICTIVE_RESPONSE_2026_09_09.md",
              ROOT / "scripts/tests/phi_v2_lattice/hydro/test_predictive_response.py"]
    paths += [ROOT / value for value in RUNTIME.values()]
    return sorted(set(path.relative_to(ROOT).as_posix() for path in paths))


def verify_sources(lock):
    if lock.get("schema") != SCHEMA:
        raise ValueError("foreign registration")
    for name, digest in lock["sources"].items():
        if sha(ROOT / name) != digest:
            raise ValueError(f"registered source or artifact changed: {name}")


def register(directory):
    directory = Path(directory)
    directory.mkdir(parents=True, exist_ok=True)
    if sha(ROOT / TABLE) != H.TABLE_HASH:
        raise ValueError("collision table identity mismatch")
    table = np.fromfile(ROOT / TABLE, dtype="<u4")
    prediction = exact_prediction(table)
    p = list(map(F, prediction["response"]))
    distances = {name: max(abs(x - F(y)) for x, y in zip(p, prediction[name]))
                 for name in ("no_collision", "reverse_stream")}
    # 24 simultaneous two-sided Hoeffding bounds with range length two.
    # N*r^2/2 = 9; exp(9)>4800 is proved by a finite positive Taylor sum.
    lower_exp9 = sum((F(9 ** k, factorial(k)) for k in range(25)), F(0))
    assert N * RADIUS ** 2 / 2 == 9 and lower_exp9 > 4800
    sources = {path: sha(ROOT / path) for path in source_paths()}
    lock = {
        "schema": SCHEMA, "registered_utc": datetime.now(timezone.utc).isoformat(),
        "base_revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip(),
        "registration_scope": "local content-hash freeze before entropy/outcomes; no external timestamp authority",
        "law": {"id": "phi-hydro-staged-candidate-1", "table_sha256": H.TABLE_HASH,
                "encoding_sha256": H.ENCODING_HASH, "blank": 4, "velocities": H.VELOCITIES},
        "runtime": RUNTIME, "sources": sources,
        "experiment": {"L": L, "replicates": REPLICATES, "N": N, "microticks": 4,
                       "source_velocity": 0, "profile_direction": [1, 2, 3],
                       "profile_positive_residues": 8, "delta": "1/4",
                       "base_probability": "1/2", "initial_phase": 0, "polarity": 0,
                       "boundary": "periodic", "relations": "blank", "manifestation": "zero"},
        "statistic": "mean over original source sites of a(x)*(final(x+v_c,phase1,c)-initial(x,phase0,c))",
        "prediction": prediction,
        "acceptance": {"radius": str(RADIUS), "family_alpha": "1/100", "log_upper": 9,
                       "exp9_rational_lower": str(lower_exp9),
                       "all_24_channels_required": True, "sampling_error": "two-sided union Hoeffding",
                       "roundoff_error": "0; integer reductions and rational comparisons",
                       "linearization_error": "0; one varied Bernoulli probability makes the expectation affine",
                       "closure_error": "0 for this first collision/stream cycle only",
                       "control_min_distance_strictly_greater_than": str(2 * RADIUS),
                       "maximum_runtime_seconds": 60, "adaptive_retry": False},
        "preflight": {"control_distances": {k: str(v) for k, v in distances.items()},
                      "powered": all(value > 2 * RADIUS for value in distances.values())},
        "sampling_assumption": "Independent fair OS entropy bits are an external preparation assumption, not ontic randomness or a proved property of a PRNG.",
        "input_recipe": "One fresh uint32 per site; bits1..23 fair; bit0 replaced by (((word>>24)&3)<(3 if a(x)=+1 else 1)). Remaining bits discarded. No selection or reroll.",
        "claim_limit": "One-cycle microscopic kinetic response in this law/preparation; no viscosity, long-time closure, physical-unit identification, or common field/matter recovery.",
        "environment": {"python": sys.version, "numpy": np.__version__},
    }
    verify_sources(lock)
    target = directory / "registration.json"
    write_new(target, lock)
    write_new(directory / "registration-digest.json", {"sha256": sha(target)})
    return {"registration": str(target), "sha256": sha(target), "preflight": lock["preflight"]}


def prepare_input(registration):
    registration = Path(registration)
    lock = json.loads(registration.read_text())
    verify_sources(lock)
    if not lock["preflight"]["powered"]:
        raise ValueError("registered design is underpowered; replacement cases are forbidden")
    directory = registration.parent
    for name in ("entropy.bin", "masks.bin", "input.json"):
        if (directory / name).exists():
            raise FileExistsError("input already exists; no redraw permitted")
    entropy = os.urandom(N * 4)
    with (directory / "entropy.bin").open("xb") as stream:
        stream.write(entropy)
    words = np.frombuffer(entropy, dtype="<u4")
    positive = np.tile(profile().reshape(-1) > 0, REPLICATES)
    bit = ((words >> 24) & 3) < np.where(positive, 3, 1)
    masks = ((words & np.uint32(0xfffffe)) | bit.astype(np.uint32)).astype("<u4")
    with (directory / "masks.bin").open("xb") as stream:
        stream.write(masks.tobytes())
    value = {"schema": "ftd-hydro-response-input-v1", "created_utc": datetime.now(timezone.utc).isoformat(),
             "registration_sha256": sha(registration), "masks_file": "masks.bin",
             "masks_sha256": sha(directory / "masks.bin"), "entropy_file": "entropy.bin",
             "entropy_sha256": sha(directory / "entropy.bin"), "independent_bits_assumed": True}
    verify_sources(lock)
    write_new(directory / "input.json", value)
    return value


def summarize(registration, inputs, result, output):
    lock, inp, observed = [json.loads(Path(p).read_text()) for p in (registration, inputs, result)]
    verify_sources(lock)
    if observed["registration_sha256"] != sha(registration) or observed["input_sha256"] != sha(inputs):
        raise ValueError("foreign result lineage")
    if inp["registration_sha256"] != sha(registration):
        raise ValueError("foreign input lineage")
    for name in ("masks", "entropy"):
        if sha(Path(inputs).parent / inp[name + "_file"]) != inp[name + "_sha256"]:
            raise ValueError("input bytes changed after execution")
    if (observed.get("schema") != "ftd-hydro-response-wasm-v1"
            or [row["index"] for row in observed["cases"]] != list(range(REPLICATES))
            or len(observed["total_delta_sums"]) != 24
            or any(len(row["delta_sums"]) != 24 for row in observed["cases"])):
        raise ValueError("incomplete or foreign observation")
    for c, value in enumerate(observed["total_delta_sums"]):
        if int(value) != sum(int(row["delta_sums"][c]) for row in observed["cases"]):
            raise ValueError("case and aggregate observations disagree")
    rows = []
    for c, (count, expected) in enumerate(zip(observed["total_delta_sums"], lock["prediction"]["response"])):
        measured, expected = F(int(count), N), F(expected)
        error = abs(measured - expected)
        rows.append({"channel": c, "predicted": str(expected), "observed": str(measured),
                     "absolute_error": str(error), "inside_registered_band": error <= RADIUS})
    complete = len(rows) == 24 and len(observed["cases"]) == REPLICATES
    accounting = complete and all(r["microtick"] == "4" and r["field_tokens_before"] == r["field_tokens_after"] for r in observed["cases"])
    value = {"schema": "ftd-hydro-response-verdict-v1", "registration_sha256": sha(registration),
             "input_sha256": sha(inputs), "wasm_result_sha256": sha(result), "rows": rows,
             "radius": str(RADIUS), "max_absolute_error": str(max(F(r["absolute_error"]) for r in rows)),
             "complete": complete, "accounting": accounting,
             "control_observed_distances": {name: str(max(abs(F(int(x), N) - F(y))
                 for x, y in zip(observed["total_delta_sums"], lock["prediction"][name])))
                 for name in ("no_collision", "reverse_stream")},
             "disposition": "PASS_SCOPED_RESPONSE" if accounting and all(r["inside_registered_band"] for r in rows) else "FAIL_REGISTERED_RESPONSE",
             "scope": lock["claim_limit"]}
    write_new(output, value)
    return value


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", choices=("register", "prepare-input", "summarize"))
    parser.add_argument("paths", nargs="+")
    args = parser.parse_args()
    function = {"register": register, "prepare-input": prepare_input, "summarize": summarize}[args.command]
    print(json.dumps(function(*args.paths), indent=2))


if __name__ == "__main__":
    main()
