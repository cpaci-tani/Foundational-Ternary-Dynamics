"""Static pre-execution conformance certificate for FTD-0745.

This reads source and hashes only. It runs no candidate dynamics and performs
no parameter search.
"""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_FINITE_SUPPORT_ENVIRONMENTAL_CLOSURE_v1.md"
RUNNER = ROOT / "engine/tests/test_finite_support_environmental_closure.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
BASELINE = ROOT / "engine/results/ftd_0739/ftd_0739_finite_support_outgoing_tail_formation_v1.csv"
EXPECTED_PROTOCOL_SHA256 = "D5FB9923FCBF69E2DFD75300FEE4C381AE28EAA10843BF0D52B2D60FCE456888"
EXPECTED_BASELINE_SHA256 = "E9B9B2FCE0FDA1350DBD6195AE039E99004141C86CB8A3F195ACE5CF24ADC622"

checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


runner = RUNNER.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")
check(
    "locked protocol SHA-256",
    hashlib.sha256(PROTOCOL.read_bytes()).hexdigest().upper()
    == EXPECTED_PROTOCOL_SHA256,
)
check(
    "frozen FTD-0739 baseline SHA-256",
    hashlib.sha256(BASELINE.read_bytes()).hexdigest().upper()
    == EXPECTED_BASELINE_SHA256,
)
check("runner embeds protocol SHA-256", EXPECTED_PROTOCOL_SHA256 in runner)
check("runner embeds baseline SHA-256", EXPECTED_BASELINE_SHA256 in runner)

for label, token in (
    ("held-out volume L=193", "constexpr int kEnvironmentL = 193;"),
    ("held-out horizon T=184", "constexpr int kEnvironmentTicks = 184;"),
    ("support radius R0=4", "constexpr int kEnvironmentSupportRadius = 4;"),
    ("contact formula", "kEnvironmentL-2*kEnvironmentSupportRadius"),
    ("six-shell ladder", "kEnvironmentRadii{8,12,16,24,32,48}"),
    ("frozen prefix horizon", "constexpr int kBaselineTicks = 136;"),
    ("late window 32", "constexpr int kLateWindow = 32;"),
    ("observer/action gate 1e-10", "constexpr double kEnvironmentGate = 1e-10;"),
    ("tail threshold 1e-8", "constexpr double kTailThreshold = 1e-8;"),
    ("tail final threshold 1e-9", "constexpr double kTailFinalThreshold = 1e-9;"),
    ("near-field floor 5e-4", "constexpr double kLateNearMinimum = 5e-4;"),
    ("near-field range 4", "constexpr double kLateNearDynamicRange = 4.0;"),
    ("dt=1/4", "options.dt=0.25;"),
    ("derived compact-pair law", "ConnectedBindingLaw::DerivedCompactPair"),
    ("well depth 0.01", "options.compact_pair_well_depth=0.01;"),
    ("cutoff squared 3/2", "options.compact_pair_cutoff_distance_squared=1.5;"),
    ("solve tolerance 2e-14", "options.solve_tolerance=2e-14;"),
    ("384 nonlinear iterations", "options.max_iterations=384;"),
    ("sparse current", "options.use_sparse_local_current=true;"),
    ("local residual evaluation", "options.use_local_residual_evaluation=true;"),
    ("unbound separation", "unbound?1.30:1.00"),
    ("unbound momentum", "unbound?0.0120:kBoundMomentum"),
    ("finite compact preparation", "kEnvironmentSupportRadius,1e-13,4096"),
    ("five-history matrix", "normalization.valid&&arms.size()==5"),
    ("batched qualified observer", "evaluate_batched_regional_energy_profile"),
    ("outside source residual", "total_source-region.source_exchange_into_field"),
    ("outside source routed to arrival gate", "arm.arrival_pass=arm.maximum_outside_source<=kEnvironmentGate;"),
    ("current support radius three", "arm.maximum_source_radius<=3"),
    ("minimum 64-tick core tail", "kEnvironmentTicks-arm.energetic_onset_tick+1>=64"),
    ("late near-field minimum gate", "arm.late_inside_8_minimum>=kLateNearMinimum"),
    ("late near-field dynamic gate", "kLateNearDynamicRange*arm.late_inside_8_minimum"),
    ("ordered shell arrival", "arm.first_tail_tick[i]>=arm.first_tail_tick[i-1]"),
    ("no-return increment gate", "arm.minimum_outward_increment[i]>=-kEnvironmentGate"),
    ("exact discrete prefix fields", "row.regional_valid==old.regional_valid"),
    ("prefix radius-eight transport", "row.transport_into[0]"),
    ("prefix radius-twelve cumulative", "row.cumulative_outward[1]"),
    ("prefix scalar gate", "prefix_difference<=kEnvironmentGate"),
    ("full polarity discrete comparison", "a.regional_valid!=b.regional_valid"),
    ("full polarity transport comparison", "a.cumulative_outward[j]-b.cumulative_outward[j]"),
    ("recoil threshold", "row.recoil_defect<=1e-9"),
    ("speed threshold", "row.speed_excess<=1e-12"),
    ("per-step energy threshold", "row.total_energy_residual<=1e-8"),
    ("pair-field balance", "arm.pair_field_balance<=1e-8"),
    ("inverse threshold", "arm.inverse_recovery<=1e-8"),
    ("forward and reverse solve", "solve_connected_moore_block_reverse"),
    ("CSV output", "ftd_0745_finite_support_environmental_closure_v1.csv"),
    ("JSON output", "ftd_0745_finite_support_environmental_closure_v1.json"),
):
    check(label, token in runner)

verdicts = (
    "ENVIRONMENTAL_CLOSURE_EXECUTION_INVALID",
    "ENVIRONMENTAL_CLOSURE_CAUSAL_PREFIX_DRIFT",
    "ENVIRONMENTAL_CLOSURE_BOUND_CONTROL_UNSTABLE",
    "ENVIRONMENTAL_CLOSURE_POLARITY_SENSITIVE",
    "ENVIRONMENTAL_CLOSURE_CORE_NOT_PERSISTENT",
    "ENVIRONMENTAL_CLOSURE_NEAR_FIELD_NOT_STABLE",
    "ENVIRONMENTAL_CLOSURE_ARRIVAL_LAW_FAIL",
    "ENVIRONMENTAL_CLOSURE_OUTGOING_COMPONENT_RETURNS",
    "FINITE_LADDER_ENVIRONMENTAL_CLOSURE_CONSTRUCTIVE",
)
for verdict in verdicts:
    check(f"locked verdict {verdict}", f'verdict="{verdict}"' in runner)

check(
    "registered CTest target",
    "ftd_add_test(test_finite_support_environmental_closure" in cmake
    and "CTEST_NAME finite_support_environmental_closure" in cmake
    and "TIMEOUT 28800" in cmake,
)
check("protocol placeholder removed", "UNLOCKED" not in runner)
check(
    "outside-source failure is not routed to execution-invalid",
    "&&row.outside_source_residual<=kEnvironmentGate;" not in runner,
)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0745 protocol conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
