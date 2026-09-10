"""Independent finite-response verifier; never imports a simulator or predictor.

The locked experiment uses the existing hydro collision table. This module
derives its one-column counting expectation independently, and reconstructs
one complete cycle from frozen phase-zero input masks by table lookup and
literal periodic shifts. Its sampling statement is conditional on independent
preparation bits; a deterministic seed does not prove that premise.
"""
from __future__ import annotations

import hashlib
import json
import math
import argparse
import re
import time
from fractions import Fraction
from pathlib import Path

import numpy as np


L = 16
CASES = 32
SITES = L ** 3
CHANNELS = 24
TABLE_SHA256 = "abf25cf26072c03b5b7865fe84d3f31c270263d2c061e7bf3d81e27d783b5375"
ENCODING_SHA256 = "3c10c134dadf3aa6f32f31ba588996e3c4755af67d4c804db567f5c4b361270c"
LAW = "phi-hydro-staged-candidate-1"
DELTA = Fraction(1, 4)
ALPHA = Fraction(1, 100)
ARRAY_SIZES = {"s": 1, "bank": 192, "sc": 6, "fcc": 12,
               "admitted_sc": 3, "admitted_fcc": 6, "gate_sc": 3, "gate_fcc": 6}
ROOT = Path(__file__).resolve().parents[3]


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def _pairs(pairs):
    result = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def _nonfinite(value):
    raise ValueError(f"nonfinite JSON number: {value}")


def owned_bytes(path, expected_sha256: str, limit: int) -> bytes:
    with Path(path).open("rb") as stream:
        data = stream.read(limit + 1)
    if len(data) > limit or sha256(data) != expected_sha256:
        raise ValueError(f"size or SHA256 mismatch: {path}")
    return data


def owned_json(path, expected_sha256: str, limit: int = 1 << 20):
    return json.loads(owned_bytes(path, expected_sha256, limit),
                      object_pairs_hook=_pairs, parse_constant=_nonfinite)


def velocities():
    """Literal face-first, then plane/sign order from the frozen alphabet."""
    result = []
    for axis in range(3):
        for sign in (1, -1):
            for _fourth_label in (0, 1):
                vector = [0, 0, 0]
                vector[axis] = sign
                result.append(tuple(vector))
    for first, second in ((0, 1), (0, 2), (1, 2)):
        for a, b in ((1, 1), (1, -1), (-1, 1), (-1, -1)):
            vector = [0, 0, 0]
            vector[first], vector[second] = a, b
            result.append(tuple(vector))
    return tuple(result)


def slab():
    x, y, z = np.indices((L, L, L), dtype=np.int64)
    return np.where((x + 2 * y + 3 * z) % L < L // 2, 1, -1).astype(np.int64)


def table_column(raw_table: bytes):
    """J[c,0] = (count(output c | input0=1)-count(...=0))/2^23.

    Uses adjacent even/odd input words and integer population sums only;
    no marginal formula or coefficient from the author's predictor is used.
    """
    if len(raw_table) != 4 * (1 << 24) or sha256(raw_table) != TABLE_SHA256:
        raise ValueError("wrong collision table bytes")
    table = np.frombuffer(raw_table, dtype="<u4")
    numerator = [0] * CHANNELS
    output_ones = [0] * CHANNELS
    for start in range(0, len(table), 1 << 18):
        pairs = table[start:start + (1 << 18)].reshape(-1, 2)
        if np.any(pairs >= (1 << 24)):
            raise ValueError("collision output exceeds 24-bit alphabet")
        for c in range(CHANNELS):
            off = int(np.count_nonzero(pairs[:, 0] & (1 << c)))
            on = int(np.count_nonzero(pairs[:, 1] & (1 << c)))
            numerator[c] += on - off
            output_ones[c] += on + off
    if output_ones != [1 << 23] * CHANNELS:
        raise ValueError("table does not preserve uniform one-slot reference means")
    return tuple(Fraction(n, 1 << 23) for n in numerator)


def exact_predictions(column):
    if len(column) != CHANNELS or any(type(x) is not Fraction for x in column):
        raise ValueError("expected 24 exact rational column entries")
    profile = slab()
    mean = []
    no_collision = []
    reversed_stream = []
    profile_autocorrelations = []
    for c, velocity in enumerate(velocities()):
        # A reversed stream is observed at x+v, so its collision source is x+2v.
        shifted = np.roll(profile, tuple(-2 * v for v in velocity), axis=(0, 1, 2))
        rho = Fraction(int(np.sum(profile * shifted, dtype=np.int64)), SITES)
        base = int(c == 0)
        mean.append(DELTA * (column[c] - base))
        no_collision.append(Fraction(0))
        reversed_stream.append(DELTA * (rho * column[c] - base))
        profile_autocorrelations.append(rho)
    return {"correct": tuple(mean), "no_collision": tuple(no_collision),
            "reversed_stream": tuple(reversed_stream),
            "reverse_profile_autocorrelations": tuple(profile_autocorrelations)}


def reconstruct_case(raw_masks: bytes, raw_table: bytes):
    """Independent one-cycle finite field reconstruction, used only after release."""
    if len(raw_masks) != SITES * 4:
        raise ValueError("input mask file must contain exactly L^3 little-endian uint32 words")
    masks = np.frombuffer(raw_masks, dtype="<u4").reshape((L, L, L))
    if np.any(masks >= (1 << CHANNELS)):
        raise ValueError("input mask exceeds finite alphabet")
    if len(raw_table) != 4 * (1 << 24):
        raise ValueError("table length mismatch")
    table = np.frombuffer(raw_table, dtype="<u4")
    collided = table[masks]
    bank = np.zeros((L, L, L, 192), dtype=np.uint8)
    profile = slab()
    deltas = []
    before_count = 0
    for c, velocity in enumerate(velocities()):
        before = ((masks >> c) & 1).astype(np.int64)
        collision = ((collided >> c) & 1).astype(np.uint8)
        bank[..., 24 + c] = np.roll(collision, velocity, axis=(0, 1, 2))
        observed = np.roll(bank[..., 24 + c], tuple(-v for v in velocity),
                           axis=(0, 1, 2)).astype(np.int64)
        deltas.append(int(np.sum(profile * (observed - before), dtype=np.int64)))
        before_count += int(np.sum(before, dtype=np.int64))
    after_count = int(np.sum(bank, dtype=np.int64))
    if before_count != after_count:
        raise ValueError("raw table cycle violated total occupancy conservation")
    arrays = {name: bytes(SITES * size) for name, size in ARRAY_SIZES.items()}
    arrays["bank"] = bank.tobytes(order="C")
    arrays["sc"] = bytes([4]) * (SITES * ARRAY_SIZES["sc"])
    arrays["fcc"] = bytes([4]) * (SITES * ARRAY_SIZES["fcc"])
    return {"microtick": "4", "deltas": deltas, "count_before": before_count,
            "count_after": after_count,
            "array_sha256": {name: sha256(data) for name, data in arrays.items()},
            "arrays_concatenated_sha256": sha256(b"".join(arrays.values()))}


def _integer(value, expected=None):
    if type(value) is not int or (expected is not None and value != expected):
        raise ValueError(f"wrong concrete integer: {value!r}; expected {expected!r}")
    return value


def _decimal(value):
    if type(value) is not str or re.fullmatch(r"0|-?[1-9][0-9]*", value) is None:
        raise ValueError("expected canonical decimal integer string")
    return int(value)


def _rational(value):
    if type(value) is not str:
        raise ValueError("rational must be a string")
    result = Fraction(value)
    if str(result) != value:
        raise ValueError("rational must have canonical spelling")
    return result


def _root_path(relative):
    if type(relative) is not str or Path(relative).is_absolute():
        raise ValueError("expected a repository-relative path")
    path = (ROOT / relative).resolve()
    if not path.is_relative_to(ROOT):
        raise ValueError("path escapes repository")
    return path


def _file_hash(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        while block := stream.read(1 << 20):
            digest.update(block)
    return digest.hexdigest()


def _check_pins(pins):
    if type(pins) is not dict or not pins:
        raise ValueError("nonempty source hash map required")
    for relative, expected in pins.items():
        if (type(expected) is not str or re.fullmatch(r"[0-9a-f]{64}", expected) is None
                or _file_hash(_root_path(relative)) != expected):
            raise ValueError(f"source pin mismatch: {relative}")


def _same_rationals(actual, expected, name):
    if type(actual) is not list or len(actual) != CHANNELS:
        raise ValueError(f"wrong {name} vector length")
    if tuple(_rational(x) for x in actual) != tuple(expected):
        raise ValueError(f"independent exact {name} disagrees with registration")


def audit_registration(lock, raw_table):
    if lock.get("schema") != "ftd-hydro-predictive-response-v1":
        raise ValueError("registration schema mismatch")
    law, experiment, acceptance = (lock[k] for k in ("law", "experiment", "acceptance"))
    if (law["id"] != LAW or law["table_sha256"] != TABLE_SHA256
            or law["encoding_sha256"] != ENCODING_SHA256):
        raise ValueError("law identity mismatch")
    _integer(law["blank"], 4)
    if (type(law["velocities"]) is not list or len(law["velocities"]) != CHANNELS
            or any(type(row) is not list or len(row) != 3
                   or any(type(x) is not int for x in row) for row in law["velocities"])
            or tuple(map(tuple, law["velocities"])) != velocities()):
        raise ValueError("literal velocity alphabet mismatch")
    for key, expected in (("L", L), ("replicates", CASES), ("microticks", 4),
                          ("source_velocity", 0), ("profile_positive_residues", 8),
                          ("N", CASES * SITES), ("initial_phase", 0), ("polarity", 0)):
        _integer(experiment[key], expected)
    for key, expected in (("base_probability", "1/2"), ("boundary", "periodic"),
                          ("relations", "blank"), ("manifestation", "zero")):
        if experiment[key] != expected:
            raise ValueError(f"wrong experiment.{key}")
    if (type(experiment["profile_direction"]) is not list
            or any(type(x) is not int for x in experiment["profile_direction"])
            or experiment["profile_direction"] != [1, 2, 3]
            or experiment["delta"] != "1/4"):
        raise ValueError("preparation profile mismatch")
    if acceptance["radius"] != "3/256" or acceptance["family_alpha"] != "1/100":
        raise ValueError("changed error budget")
    _integer(acceptance["log_upper"], 9)
    radius = Fraction(3, 256)
    if CASES * SITES * radius ** 2 / 2 != 9:
        raise AssertionError("Hoeffding exponent arithmetic")
    # exp(9) exceeds this finite positive Taylor sum; no numerical logarithm.
    exp9_lower = sum((Fraction(9 ** k, math.factorial(k)) for k in range(33)), Fraction(0))
    if exp9_lower <= Fraction(2 * CHANNELS) / ALPHA:
        raise AssertionError("simultaneous Hoeffding bound not established")
    column = table_column(raw_table)
    prediction = exact_predictions(column)
    declared = lock["prediction"]
    _integer(declared["column_denominator"], 1 << 23)
    numbers = declared["column_numerators"]
    if type(numbers) is not list or len(numbers) != CHANNELS:
        raise ValueError("wrong registered column length")
    if tuple(Fraction(_integer(n), 1 << 23) for n in numbers) != column:
        raise ValueError("independent raw-table column count mismatch")
    for key, registered in (("correct", "response"), ("no_collision", "no_collision"),
                            ("reversed_stream", "reverse_stream")):
        _same_rationals(declared[registered], prediction[key], registered)
    gaps = {key: max(abs(a - b) for a, b in zip(prediction["correct"], prediction[key]))
            for key in ("no_collision", "reversed_stream")}
    if any(gap <= 2 * radius for gap in gaps.values()):
        raise ValueError("fixed design cannot distinguish a registered control")
    return prediction, {"radius": str(radius), "hoeffding_exponent": 9,
                        "exp9_exact_taylor_lower": str(exp9_lower),
                        "family_failure_upper": str(Fraction(2 * CHANNELS) / exp9_lower),
                        "control_distances": {k: str(v) for k, v in gaps.items()}}


def run(lock_path, input_path, wasm_path, prediction_only=False):
    started = time.monotonic()
    lock_path = Path(lock_path).resolve()
    lock_hash = _file_hash(lock_path)
    lock = owned_json(lock_path, lock_hash)
    _check_pins(lock["sources"])
    for role in ("module", "binary", "table"):
        relative = _root_path(lock["runtime"][role]).relative_to(ROOT).as_posix()
        if relative not in lock["sources"]:
            raise ValueError(f"runtime {role} missing from pinned closure")
    own_relative = Path(__file__).resolve().relative_to(ROOT).as_posix()
    if lock["sources"].get(own_relative) != _file_hash(__file__):
        raise ValueError("independent verifier source absent from registration")
    table_path = _root_path(lock["runtime"]["table"])
    raw_table = owned_bytes(table_path, TABLE_SHA256, 4 * (1 << 24))
    prediction, budget = audit_registration(lock, raw_table)
    evidence = {str(lock_path): lock_hash, str(table_path): TABLE_SHA256}
    report = {"schema": "ftd-hydro-response-independent-audit-v1",
              "status": "PASS_SCOPED_PREDICTION", "registration_sha256": lock_hash,
              "budget": budget, "prediction": [str(x) for x in prediction["correct"]],
              "sampling_assumption": "independent external fair bits; determinism or replay does not establish independence",
              "scope": "one fixed finite preparation and one four-microtick hydro cycle",
              "physical_recovery": False, "microscopic_reproduction_performed": False}
    if not prediction_only:
        input_path, wasm_path = Path(input_path).resolve(), Path(wasm_path).resolve()
        input_hash, wasm_hash = _file_hash(input_path), _file_hash(wasm_path)
        inputs, wasm = owned_json(input_path, input_hash), owned_json(wasm_path, wasm_hash)
        if (inputs.get("schema") != "ftd-hydro-response-input-v1"
                or inputs.get("registration_sha256") != lock_hash
                or wasm.get("schema") != "ftd-hydro-response-wasm-v1"
                or wasm.get("registration_sha256") != lock_hash
                or wasm.get("input_sha256") != input_hash):
            raise ValueError("input/result identity chain mismatch")
        raw = {}
        for name in ("masks", "entropy"):
            if inputs[name + "_file"] != name + ".bin":
                raise ValueError("unexpected input artifact name")
            path = input_path.parent / inputs[name + "_file"]
            raw[name] = owned_bytes(path, inputs[name + "_sha256"], CASES * SITES * 4)
            if len(raw[name]) != CASES * SITES * 4:
                raise ValueError("incorrect complete input length")
            evidence[str(path)] = inputs[name + "_sha256"]
        words = np.frombuffer(raw["entropy"], dtype="<u4").reshape(CASES, SITES)
        generated = (words & np.uint32(0xFFFFFE)).copy()
        threshold = np.where(slab().reshape(1, SITES) > 0, 3, 1)
        generated |= (((words >> 24) & 3) < threshold).astype(np.uint32)
        if generated.astype("<u4", copy=False).tobytes() != raw["masks"]:
            raise ValueError("frozen masks disagree with exact entropy preparation")
        if type(wasm["cases"]) is not list or len(wasm["cases"]) != CASES:
            raise ValueError("WASM case coverage mismatch")
        total = [0] * CHANNELS
        reproduced = []
        for index, record in enumerate(wasm["cases"]):
            _integer(record["index"], index)
            if record["microtick"] != "4":
                raise ValueError("WASM terminal clock mismatch")
            start = index * SITES * 4
            result = reconstruct_case(raw["masks"][start:start + SITES * 4], raw_table)
            observed = record["delta_sums"]
            if (type(observed) is not list or len(observed) != CHANNELS
                    or [_decimal(x) for x in observed] != result["deltas"]):
                raise ValueError(f"WASM channel sums disagree in case {index}")
            if record["array_sha256"] != result["array_sha256"]:
                raise ValueError(f"WASM full array hashes disagree in case {index}")
            for key, expected in (("field_tokens_before", result["count_before"]),
                                  ("field_tokens_after", result["count_after"])):
                if _decimal(record[key]) != expected:
                    raise ValueError(f"WASM occupancy count disagrees in case {index}")
            total = [a + b for a, b in zip(total, result["deltas"])]
            reproduced.append({"index": index, "array_sha256": result["array_sha256"],
                               "delta_sums": list(map(str, result["deltas"]))})
        if (type(wasm["total_delta_sums"]) is not list
                or len(wasm["total_delta_sums"]) != CHANNELS
                or [_decimal(x) for x in wasm["total_delta_sums"]] != total):
            raise ValueError("WASM aggregate sums disagree")
        radius = Fraction(3, 256)
        residuals = [Fraction(value, CASES * SITES) - expected
                     for value, expected in zip(total, prediction["correct"])]
        passed = all(abs(x) <= radius for x in residuals)
        report.update(status="PASS_SCOPED_RESPONSE" if passed else "STATISTICAL_MISMATCH",
                      microscopic_reproduction_performed=True, cases_reproduced=CASES,
                      sites_reproduced=CASES * SITES, replicas=reproduced,
                      total_delta_sums=list(map(str, total)),
                      residuals=[str(x) for x in residuals],
                      all_24_within_fixed_budget=passed)
        evidence.update({str(input_path): input_hash, str(wasm_path): wasm_hash})
    _check_pins(lock["sources"])
    for path, expected in evidence.items():
        if _file_hash(path) != expected:
            raise ValueError(f"evidence changed during independent audit: {path}")
    report.update(evidence_sha256=evidence, source_sha256=lock["sources"],
                  source_and_input_postcheck=True, elapsed_seconds=time.monotonic() - started)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("lock")
    parser.add_argument("input")
    parser.add_argument("wasm_output")
    parser.add_argument("audit_output")
    parser.add_argument("--prediction-only", action="store_true")
    args = parser.parse_args()
    output = Path(args.audit_output)
    if output.exists():
        raise FileExistsError("independent audit output must be new")
    try:
        report = run(args.lock, args.input, args.wasm_output, args.prediction_only)
    except Exception as error:
        report = {"schema": "ftd-hydro-response-independent-audit-v1", "status": "FAILED",
                  "error_type": type(error).__name__, "error": str(error),
                  "physical_recovery": False}
    with output.open("x", encoding="utf-8", newline="\n") as stream:
        json.dump(report, stream, indent=2, sort_keys=True, allow_nan=False)
        stream.write("\n")
    if not report["status"].startswith("PASS_SCOPED"):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
