"""Static pre-execution conformance certificate for FTD-0746."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_v1.md"
RUNNER = ROOT / "engine/tests/test_causal_horizon_environmental_persistence.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"

PROTOCOL_SHA256 = "B98DB9B18050D1799814ABD0B6C70936BF631AEF258CF969FC8D15E7B8DCA9A0"
BASELINE_SHA256 = "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C"

source = RUNNER.read_text(encoding="utf-8")
protocol = PROTOCOL.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")
checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


check("locked protocol SHA-256", sha256(PROTOCOL) == PROTOCOL_SHA256)
check("frozen FTD-0745 baseline SHA-256", sha256(BASELINE) == BASELINE_SHA256)
check("runner embeds protocol SHA-256", PROTOCOL_SHA256 in source)
check("runner embeds baseline SHA-256", BASELINE_SHA256 in source)
check("protocol placeholder removed", "UNLOCKED" not in source)
check("held-out volume L=321", "constexpr int kHorizonL = 321;" in source)
check("held-out horizon T=312", "constexpr int kHorizonTicks = 312;" in source)
check("support radius R0=4", "kHorizonSupportRadius = 4" in source)
check("contact formula", "kHorizonL-2*kHorizonSupportRadius" in source)
check("prefix horizon 184", "kHorizonPrefixTicks = 184" in source)
check("six-shell ladder", "{8,12,16,24,32,48}" in source)
check("late window starts 281", "kHorizonLateBegin = 281" in source)
check("arrival deadline 300", "kHorizonArrivalDeadline = 300" in source)
check("post-arrival window starts 301", "kHorizonPostArrivalBegin = 301" in source)
check("observer/action gate 1e-10", "kHorizonGate = 1e-10" in source)
check("tail threshold unchanged 1e-8", "kHorizonTailThreshold = 1e-8" in source)
check("tail final threshold 1e-9", "kHorizonTailFinalThreshold = 1e-9" in source)
check("near-field floor 5e-4", "kHorizonNearMinimum = 5e-4" in source)
check("near-field dynamic range four", "kHorizonNearDynamicRange = 4.0" in source)
check("frozen OLS construction documented", all(token in protocol for token in (
    "-32.7142857142857", "6.76428571428571", "291.9714"
)))
check("dt=1/4", "options.dt=0.25" in source)
check("derived compact-pair law", "ConnectedBindingLaw::DerivedCompactPair" in source)
check("well depth 0.01", "compact_pair_well_depth=0.01" in source)
check("cutoff squared 3/2", "compact_pair_cutoff_distance_squared=1.5" in source)
check("solve tolerance 2e-14", "solve_tolerance=2e-14" in source)
check("384 nonlinear iterations", "max_iterations=384" in source)
check("sparse current", "use_sparse_local_current=true" in source)
check("local residual evaluation", "use_local_residual_evaluation=true" in source)
check("unbound separation", "direction,false,1.30,0.0120" in source)
check("finite compact preparation", "prepare_finite_support_derived_compact_pair" in source)
check("three command-selected rays", all(token in source for token in (
    'slug=="face"', 'slug=="edge"', 'slug=="body"'
)))
check("all three principal directions", all(token in source for token in (
    "kDirections[0]", "kDirections[1]", "kDirections[2]"
)))
check("no reverse solver in successor", "solve_connected_moore_block_reverse" not in source)
check("summary states inverse not tested", '"inverse_tested\\\": false' in source)
check("batched qualified observer", "evaluate_batched_regional_energy_profile" in source)
check("source exchange outside support", "outside_source_residual<=kHorizonGate" in source)
check("current support radius three", "maximum_source_radius<=3" in source)
check("160-tick core tail", ">=160" in source)
check("late near-field minimum gate", "late_inside_8_minimum>=kHorizonNearMinimum" in source)
check("late near-field dynamic gate", "kHorizonNearDynamicRange*arm.late_inside_8_minimum" in source)
check("radius-48 initial exterior gate", "rows.front().outside[r48]<=1e-12" in source)
check("radius-48 threshold gate", "maximum_outside[r48]>kHorizonTailThreshold" in source)
check("radius-48 deadline gate", "first_tail_tick[r48]<=kHorizonArrivalDeadline" in source)
check("radius-48 no-return gate", "minimum_outward_increment[r48]>=-kHorizonGate" in source)
check("radius-48 final floor", "final_outside[r48]>kHorizonTailFinalThreshold" in source)
check("post-arrival per-tick floor", "value>kHorizonTailFinalThreshold" in source)
check("pair-field balance", "pair_field_balance<=1e-8" in source)
check("exact causal prefix discrete fields", "now.source_entries==old.source_entries" in source)
check("exact causal prefix all radii", "for(std::size_t i=0;i<kHorizonRadii.size();++i)" in source)
check("prefix scalar gate", "prefix_scalar_difference<=kHorizonGate" in source)
check("standard JSON finite/null serializer", all(token in source for token in (
    "void horizon_json_number", "std::isfinite(value)", 'output<<"null"'
)))
check("all optional summary scalars use serializer", source.count("horizon_json_number(json,") == 14)
check("per-arm CSV output", 'stem+".csv"' in source)
check("per-arm JSON output", 'stem+".json"' in source)
check("no-argument smoke cannot execute physics", "if(argc==1)" in source and "return 0;" in source)
check("invalid arm rejected", "if(argc!=2) return 2" in source)
for verdict in (
    "CAUSAL_HORIZON_EXECUTION_INVALID",
    "CAUSAL_HORIZON_PREFIX_DRIFT",
    "CAUSAL_HORIZON_CORE_NOT_PERSISTENT",
    "CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE",
    "CAUSAL_HORIZON_R48_ARRIVAL_FAIL",
    "CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT",
    "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
):
    check(f"locked verdict {verdict}", verdict in source and verdict in protocol)
check("registered CMake target", all(token in cmake for token in (
    "test_causal_horizon_environmental_persistence",
    "tests/test_causal_horizon_environmental_persistence.cpp",
    "CTEST_NAME causal_horizon_environmental_persistence",
)))

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0746 protocol conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
