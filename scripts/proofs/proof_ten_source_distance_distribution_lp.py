#!/usr/bin/env python3
"""Independent high-precision verifier for preregistered FTD-0596.

The verifier does not invoke HiGHS.  It reconstructs the complete displacement
scheme, evaluates every Fourier constraint needed by each sparse primal at 90
decimal digits, evaluates every sparse dual inequality at 90 digits, applies
the preregistered coefficient padding, and checks the independent C++ verifier.
"""

from __future__ import annotations

import csv
import hashlib
import itertools
import json
import math
from pathlib import Path
import subprocess

import mpmath as mp


mp.mp.dps = 90

ROOT = Path(__file__).resolve().parents[2]
PREREG = (
    ROOT / "docs/theory/10_eft_program/preregistrations/"
    "PREREG_TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_v1.md"
)
PREREG_SHA = "D69E9AFE8FCB2ECA487D285AC0B4A85D57FF1182B68FE613E32B0CADE7D3F2FA"
RESULT = ROOT / "engine/results/ftd_0596/windows_msvc_cpu.json"
CERTIFICATE = ROOT / "engine/results/ftd_0596/solver_certificate.csv"
EXECUTABLE = (
    ROOT / "engine/build/Release/test_ten_source_distance_distribution_lp.exe"
)
ARTIFACTS = {
    "header_sha256": ROOT / "engine/include/ftd/eft/"
    "ten_source_distance_distribution_lp.h",
    "source_sha256": ROOT / "engine/src/eft/"
    "ten_source_distance_distribution_lp.cpp",
    "test_sha256": ROOT / "engine/tests/"
    "test_ten_source_distance_distribution_lp.cpp",
    "generator_sha256": ROOT / "scripts/proofs/"
    "generate_ten_source_distance_distribution_lp.py",
    "proof_sha256": Path(__file__).resolve(),
    "certificate_sha256": CERTIFICATE,
}
VOLUMES = (9, 17, 33, 65)
K_GENESIS = mp.mpf("1.5163860591519780")
COEFFICIENT_TOL = mp.mpf("5e-12")
FOURIER_TOL = mp.mpf("1e-10")
GAP_TOL = mp.mpf("1e-8")
DUAL_PAD_FLOOR = mp.mpf("1e-12")
EDGE_CAPS = (0, 0, 1, 2, 4, 5, 7, 9, 12, 13)


class Proof:
    def __init__(self) -> None:
        self.rows: list[tuple[bool, str, str]] = []

    def check(self, name: str, condition: bool, note: str) -> None:
        self.rows.append((bool(condition), name, note))

    def report(self) -> bool:
        print("=" * 79)
        print("FTD-0596 ten-source distance-distribution LP")
        print("=" * 79)
        for passed, name, note in self.rows:
            print(f"  {'PASS' if passed else 'FAIL':4s}  {name}: {note}")
        passed = sum(row[0] for row in self.rows)
        print("-" * 79)
        print(f"checks={len(self.rows)} passed={passed} "
              f"failed={len(self.rows)-passed}")
        return passed == len(self.rows)


def representatives(lattice_size: int) -> list[tuple[int, int, int]]:
    half = lattice_size // 2
    return [
        (a, b, c)
        for a in range(half + 1)
        for b in range(a, half + 1)
        for c in range(b, half + 1)
        if (a, b, c) != (0, 0, 0)
    ]


def orbit_members(lattice_size: int, value: tuple[int, int, int]
                  ) -> list[tuple[int, int, int]]:
    members: set[tuple[int, int, int]] = set()
    for permutation in sorted(set(itertools.permutations(value))):
        sign_sets = [(-1, 1) if component else (1,)
                     for component in permutation]
        for signs in itertools.product(*sign_sets):
            members.add(tuple(
                (sign * component) % lattice_size
                for sign, component in zip(signs, permutation)))
    return sorted(members)


def load_certificate() -> dict[int, dict[str, object]]:
    volumes: dict[int, dict[str, object]] = {}
    with CERTIFICATE.open(newline="", encoding="utf-8") as stream:
        for row in csv.DictReader(stream):
            lattice_size = int(row["L"])
            volume = volumes.setdefault(lattice_size, {
                "kernel": [], "partitions": {},
            })
            kind = row["kind"]
            if kind == "volume":
                volume.update({
                    "orbit_count": int(row["index"]),
                    "maximum_kernel": mp.mpf(row["value"]),
                    "shell_count": int(row["aux1"]),
                    "pulse_operator": mp.mpf(row["aux2"]),
                    "common_step": mp.mpf(row["aux3"]),
                    "maximizing_removed": int(row["aux4"]),
                    "bound": mp.mpf(row["aux5"]),
                    "margin": mp.mpf(row["aux6"]),
                    "maximum_displacement": tuple(
                        map(int, row["aux7"].split(":"))),
                    "valid": bool(int(row["aux8"])),
                })
            elif kind == "kappa":
                index = int(row["index"])
                kernel = volume["kernel"]
                while len(kernel) <= index:
                    kernel.append(mp.mpf("0"))
                kernel[index] = mp.mpf(row["value"])
            else:
                removed = int(row["r"])
                partitions = volume["partitions"]
                partition = partitions.setdefault(removed, {
                    "a": [], "y": [], "z": [],
                })
                if kind == "partition":
                    partition.update({
                        "bound": mp.mpf(row["value"]),
                        "gram": mp.mpf(row["aux1"]),
                        "primal": mp.mpf(row["aux2"]),
                        "certified": mp.mpf(row["aux3"]),
                        "lambda": mp.mpf(row["aux4"]),
                        "epsilon": mp.mpf(row["aux5"]),
                        "delta": mp.mpf(row["aux6"]),
                        "minimum_fourier": mp.mpf(row["aux7"]),
                        "minimum_dual_slack": mp.mpf(row["aux8"]),
                    })
                elif kind in ("a", "y", "z"):
                    partition[kind].append(
                        (int(row["index"]), mp.mpf(row["value"])))
    return volumes


class HighPrecisionScheme:
    def __init__(self, lattice_size: int,
                 volume: dict[str, object]) -> None:
        self.lattice_size = lattice_size
        self.representatives = representatives(lattice_size)
        self.members = [orbit_members(lattice_size, value)
                        for value in self.representatives]
        self.kernel: list[mp.mpf] = volume["kernel"]
        self.cosine = [
            mp.cos(2 * mp.pi * phase / lattice_size)
            for phase in range(lattice_size)
        ]
        covered = set().union(*(set(value) for value in self.members))
        self.coverage = (
            len(covered) == lattice_size ** 3 - 1
            and (0, 0, 0) not in covered
            and sum(len(value) for value in self.members)
            == lattice_size ** 3 - 1
        )
        self._p_cache: dict[tuple[int, int], mp.mpf] = {}

    def p(self, momentum_index: int, displacement_index: int) -> mp.mpf:
        key = (momentum_index, displacement_index)
        if key in self._p_cache:
            return self._p_cache[key]
        momentum = self.representatives[momentum_index]
        members = self.members[displacement_index]
        value = mp.fsum(
            self.cosine[
                sum(momentum[axis] * displacement[axis]
                    for axis in range(3)) % self.lattice_size]
            for displacement in members
        ) / len(members)
        self._p_cache[key] = value
        return value

    def prepare(self, partitions: dict[int, dict[str, object]]) -> None:
        primal_support = sorted({
            index
            for removed in range(2, 10)
            for index, _ in partitions[removed]["a"]
        })
        dual_support = sorted({
            index
            for removed in range(2, 10)
            for index, _ in partitions[removed]["y"]
        })
        for momentum in range(len(self.representatives)):
            for displacement in primal_support:
                self.p(momentum, displacement)
        for momentum in dual_support:
            for displacement in range(len(self.representatives)):
                self.p(momentum, displacement)


def verify_partition(P: Proof, scheme: HighPrecisionScheme,
                     volume: dict[str, object], removed: int
                     ) -> tuple[mp.mpf, mp.mpf]:
    partition = volume["partitions"][removed]
    h = mp.mpf(removed - 1)
    upper = [min(mp.mpf(len(members)), h) for members in scheme.members]
    axial = scheme.representatives.index((0, 0, 1))
    upper[axial] = min(
        upper[axial], mp.mpf(2 * EDGE_CAPS[removed]) / removed)
    a = [mp.mpf("0")] * len(scheme.representatives)
    z = [mp.mpf("0")] * len(scheme.representatives)
    for index, value in partition["a"]:
        a[index] += value
    for index, value in partition["z"]:
        z[index] += value
    normalization_residual = abs(mp.fsum(a) - h)
    upper_residual = max(
        [mp.mpf("0")] + [a[index] - upper[index]
                           for index in range(len(a))])
    P.check(f"L={scheme.lattice_size} r={removed} normalization",
            normalization_residual <= mp.mpf("1e-10"),
            mp.nstr(normalization_residual, 6))
    P.check(f"L={scheme.lattice_size} r={removed} upper bounds",
            upper_residual <= mp.mpf("1e-10"),
            mp.nstr(upper_residual, 6))

    primal = mp.fsum(a[index] * scheme.kernel[index]
                     for index in range(len(a)))
    P.check(f"L={scheme.lattice_size} r={removed} primal objective",
            abs(primal - partition["primal"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(primal - partition["primal"]), 6))
    minimum_fourier = min(
        1 + mp.fsum(
            value * scheme.p(momentum, index)
            for index, value in partition["a"])
        for momentum in range(len(scheme.representatives))
    )
    P.check(f"L={scheme.lattice_size} r={removed} global Fourier positivity",
            minimum_fourier >= -FOURIER_TOL,
            mp.nstr(minimum_fourier, 8))

    y_sum = mp.fsum(value for _, value in partition["y"])
    pressure = [
        mp.fsum(value * scheme.p(momentum, displacement)
                for momentum, value in partition["y"])
        for displacement in range(len(scheme.representatives))
    ]
    epsilon = max(
        [mp.mpf("0")] + [
            scheme.kernel[index] - partition["lambda"] - z[index]
            + pressure[index]
            for index in range(len(scheme.representatives))
        ])
    delta = COEFFICIENT_TOL * (1 + y_sum) + DUAL_PAD_FLOOR
    padded_lambda = partition["lambda"] + epsilon + delta
    minimum_dual = min(
        padded_lambda + z[index] - pressure[index] - scheme.kernel[index]
        for index in range(len(scheme.representatives))
    )
    certified = (
        h * padded_lambda + y_sum
        + mp.fsum(upper[index] * z[index]
                  for index in range(len(z))))
    P.check(f"L={scheme.lattice_size} r={removed} epsilon",
            abs(epsilon - partition["epsilon"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(epsilon - partition["epsilon"]), 6))
    P.check(f"L={scheme.lattice_size} r={removed} padding",
            abs(delta - partition["delta"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(delta - partition["delta"]), 6))
    P.check(f"L={scheme.lattice_size} r={removed} dual feasibility",
            minimum_dual >= -mp.mpf("1e-12"),
            mp.nstr(minimum_dual, 8))
    P.check(f"L={scheme.lattice_size} r={removed} certified objective",
            abs(certified - partition["certified"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(certified - partition["certified"]), 6))
    gap = certified - primal
    P.check(f"L={scheme.lattice_size} r={removed} primal/dual gap",
            gap >= -mp.mpf("1e-10") and gap <= GAP_TOL,
            mp.nstr(gap, 8))
    gram = removed * (1 + certified)
    common_step = volume["common_step"]
    pulse_operator = volume["pulse_operator"]
    bound = (common_step * mp.sqrt(10 - removed)
             + pulse_operator * mp.sqrt(gram))
    P.check(f"L={scheme.lattice_size} r={removed} Gram factor",
            abs(gram - partition["gram"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(gram - partition["gram"]), 6))
    P.check(f"L={scheme.lattice_size} r={removed} partition bound",
            abs(bound - partition["bound"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(bound - partition["bound"]), 6))
    return bound, minimum_fourier


P = Proof()
actual_prereg = hashlib.sha256(PREREG.read_bytes()).hexdigest().upper()
P.check("frozen preregistration hash", actual_prereg == PREREG_SHA,
        actual_prereg)
record = json.loads(RESULT.read_text(encoding="utf-8"))
P.check("run identifier", record["identifier"] == "FTD-0596",
        record["identifier"])
P.check("recorded preregistration hash",
        record["preregistration_sha256"] == actual_prereg,
        record["preregistration_sha256"])
for field, path in ARTIFACTS.items():
    actual_hash = hashlib.sha256(path.read_bytes()).hexdigest().upper()
    P.check(f"artifact hash {field}", record[field] == actual_hash,
            actual_hash)
P.check("registered source count", record["registered_source_count"] == 10,
        str(record["registered_source_count"]))
P.check("registered threshold",
        mp.mpf(str(record["threshold"])) == K_GENESIS,
        str(record["threshold"]))
P.check("no configuration search",
        record["configuration_search_performed"] is False,
        str(record["configuration_search_performed"]))
P.check("no history search", record["history_search_performed"] is False,
        str(record["history_search_performed"]))
P.check("no extra cut", record["extra_cut_added"] is False,
        str(record["extra_cut_added"]))
P.check("production unchanged", record["production_changed"] is False,
        str(record["production_changed"]))
P.check("90-decimal-digit reconstruction", mp.mp.dps >= 80,
        f"dps={mp.mp.dps}")

certificate = load_certificate()
all_bounds: list[mp.mpf] = []
for lattice_size in VOLUMES:
    volume = certificate[lattice_size]
    scheme = HighPrecisionScheme(lattice_size, volume)
    P.check(f"L={lattice_size} exact orbit coverage", scheme.coverage,
            f"orbits={len(scheme.representatives)}")
    P.check(f"L={lattice_size} orbit count",
            len(scheme.representatives) == volume["orbit_count"],
            str(len(scheme.representatives)))
    P.check(f"L={lattice_size} kernel count",
            len(scheme.kernel) == len(scheme.representatives),
            str(len(scheme.kernel)))
    scheme.prepare(volume["partitions"])
    bounds = [volume["partitions"][0]["bound"],
              volume["partitions"][1]["bound"]]
    for removed in range(2, 10):
        bound, _ = verify_partition(P, scheme, volume, removed)
        bounds.append(bound)
    bounds.append(volume["partitions"][10]["bound"])
    maximizing = max(range(11), key=bounds.__getitem__)
    maximum_bound = bounds[maximizing]
    all_bounds.append(maximum_bound)
    P.check(f"L={lattice_size} maximizing partition",
            maximizing == volume["maximizing_removed"], str(maximizing))
    P.check(f"L={lattice_size} certified maximum",
            abs(maximum_bound - volume["bound"]) <= COEFFICIENT_TOL,
            mp.nstr(abs(maximum_bound - volume["bound"]), 6))
    P.check(f"L={lattice_size} margin",
            abs((K_GENESIS - maximum_bound) - volume["margin"])
            <= COEFFICIENT_TOL,
            mp.nstr(abs((K_GENESIS - maximum_bound)
                        - volume["margin"]), 6))

completed = subprocess.run(
    [str(EXECUTABLE)], check=True, capture_output=True, text=True)
P.check("independent C++ verifier verdict",
        "verdict,TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE"
        in completed.stdout, "C++ certificate accepted")
P.check("independent C++ primal gate",
        "summary,volumes,4,primal,true,dual,true" in completed.stdout,
        "four volumes")

all_closed = all(bound < K_GENESIS for bound in all_bounds)
expected_verdict = (
    "ARBITRARY_REMOVAL_N_LE_10_CLOSED_BY_DISTANCE_DISTRIBUTION_LP"
    if all_closed else "TEN_SOURCE_DISTANCE_DISTRIBUTION_LP_INCONCLUSIVE")
P.check("registered verdict", record["verdict"] == expected_verdict,
        record["verdict"])
P.check("registered Delsarte result is inconclusive", not all_closed,
        "at least one certified maximum exceeds threshold")

raise SystemExit(0 if P.report() else 1)
