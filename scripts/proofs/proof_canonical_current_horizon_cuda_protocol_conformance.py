"""Static pre-execution conformance certificate for FTD-0748."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/engine_infrastructure_rg/PREREG_CANONICAL_CURRENT_HORIZON_CUDA_v1.md"
RUNNER = ROOT / "engine/tests/campaign_canonical_current_horizon_cuda.cpp"
HEADER = ROOT / "engine/include/ftd/eft/quadratic_coat_face_current.h"
SOURCE = ROOT / "engine/src/eft/quadratic_coat_face_current.cpp"
TEST = ROOT / "engine/tests/test_quadratic_coat_face_current.cpp"
PARENT = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"
PROTOCOL_HASH = "D01039341BCA3098C9F837549A26199CCE5BB6660C84A7C86C5037D17A2B0C46"
BASELINE_HASH = "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


texts = {path: path.read_text(encoding="utf-8")
         for path in (PROTOCOL, RUNNER, HEADER, SOURCE, TEST, PARENT, CMAKE)}
checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


p = texts[PROTOCOL]
r = texts[RUNNER]
h = texts[HEADER]
s = texts[SOURCE]
t = texts[TEST]
parent = texts[PARENT]
cmake = texts[CMAKE]

check("protocol hash locked", digest(PROTOCOL) == PROTOCOL_HASH)
check("baseline hash frozen", digest(BASELINE) == BASELINE_HASH)
check("runner embeds protocol hash", PROTOCOL_HASH in r)
check("unlock token removed", 'kCanonicalProtocolSha256[]="UNLOCKED"' not in r)
check("held-out refusal remains", "held-out execution refused" in r)
check("parent runner is inherited without paste", "#include \"campaign_causal_horizon_environmental_persistence_cuda.cpp\"" in r)
check("L=321 inherited", "kHorizonL" in r and "L=321" in p)
check("T=312 inherited", "kHorizonTicks" in r and "ticks `0..312`" in p)
check("three selected rays", "face|edge|body" in r)
check("qualification capped at eight", "ticks>8" in r)
check("dt quarter", "options.dt=0.25" in r)
check("selected pair law", "DerivedCompactPair" in r)
check("depth unchanged", "compact_pair_well_depth=0.01" in r)
check("cutoff unchanged", "compact_pair_cutoff_distance_squared=1.5" in r)
check("solve tolerance locked", "options.solve_tolerance=2e-14" in r)
check("root iterations locked", "options.max_iterations=384" in r)
check("sparse local current", "options.use_sparse_local_current=true" in r)
check("CUDA prepared update", "pipeline.prepare_forward(lambda)" in r)
check("CUDA raw current remains applied", "pipeline.apply_sparse_current(step.segments,options.polarity_scale)" in r)
check("CUDA observer remains", "pipeline.observe(lambda,center,radii,kHorizonGate)" in r)
check("canonical aggregation API declared", "aggregate_quadratic_coat_face_current" in h)
check("periodic oriented-face key", "wrap(entry.face.x,result.L)" in s and "std::map<Key,long double>" in s)
check("raw contribution count recorded", "raw_contributions" in h and "raw_contributions" in r)
check("net L1 recorded", "net_l1" in h and "net_l1" in r)
check("cancelled L1 recorded", "cancelled_l1" in h and "cancelled_l1" in r)
check("discarded L1 recorded", "discarded_l1" in h and "discarded_l1" in r)
check("aggregation moment recorded", "aggregation_moment_residual" in h and "moment_residual" in r)
check("support threshold is inherited H1 gate", "segments,polarity_scale,kHorizonGate" in r)
check("moment gate locked", "kCanonicalMomentGate=1e-12" in r)
check("discarded L1 gate locked", "support.discarded_l1<=kHorizonGate" in r)
check("net source radius gate locked", "support.source_radius<=3" in r)
check("splitting invariance tested", "invariant under entry splitting" in t)
check("periodic-image invariance tested", "periodic images" in t)
check("opposite cancellation tested", "cancel before support is counted" in t)
check("tolerance accounting tested", "quarantines only its reported L1 mass" in t)
check("raw baseline count excluded only by substitution", "prefix_arm.rows" in r and "source_entries=" in r)
check("original scalar prefix evaluator reused", "horizon_prefix_difference" in r)
check("aggregation verdict ordered before prefix", r.index("CURRENT_AGGREGATION_INVALID") < r.index("PREFIX_DRIFT"))
check("H2 core gate unchanged", "horizon_negative_onset" in r and ">=160" in r)
check("H3 near-field gate unchanged", "kHorizonNearMinimum" in r and "kHorizonNearDynamicRange" in r)
check("H4 arrival gate unchanged", "kHorizonArrivalDeadline" in r and "kHorizonTailThreshold" in r)
check("H5 persistence gate unchanged", "kHorizonPostArrivalBegin" in r and "kHorizonTailFinalThreshold" in r)
check("constructive token exact", "CANONICAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE" in r)
check("new result folder", '"ftd_0748"' in r)
check("main and support records", "write_horizon_records" in r and "write_canonical_support_records" in r)
check("CUDA backend label", "wsl2_cuda_canonical_net_current" in r)
check("CMake target registered", "campaign_canonical_current_horizon_cuda" in cmake)
check("disclosed nonblind scope", "not blind new evidence" in p and "protocol/observer correction" in p)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0748 protocol conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
