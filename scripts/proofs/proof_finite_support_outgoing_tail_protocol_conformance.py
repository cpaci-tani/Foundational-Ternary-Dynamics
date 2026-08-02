"""Static pre-execution conformance certificate for FTD-0739.

This certificate compares the locked protocol hash and load-bearing literal
contracts against the C++ campaign.  It runs no dynamics and performs no
parameter search.
"""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/PREREG_FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_v1.md"
RUNNER = ROOT / "engine/tests/test_finite_support_outgoing_tail_formation.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
EXPECTED_PROTOCOL_SHA256 = (
    "9AA9B806877F07F9567291E73B58E6157CFBDAE425DE843B85D3753CECA7868E"
)


checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


protocol_bytes = PROTOCOL.read_bytes()
runner = RUNNER.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

check(
    "locked protocol SHA-256",
    hashlib.sha256(protocol_bytes).hexdigest().upper() == EXPECTED_PROTOCOL_SHA256,
)
check("runner embeds locked SHA-256", EXPECTED_PROTOCOL_SHA256 in runner)

for label, token in (
    ("volume L=145", "constexpr int kFiniteL = 145;"),
    ("horizon T=136", "constexpr int kFiniteTicks = 136;"),
    ("support radius R0=4", "constexpr int kFiniteSupportRadius = 4;"),
    ("contact tick formula", "kFiniteL-2*kFiniteSupportRadius"),
    ("inner shell radius 8", "constexpr double kInnerShell = 8.0;"),
    ("outer shell radius 12", "constexpr double kOuterShell = 12.0;"),
    ("gate tolerance 1e-10", "constexpr double kFiniteGate = 1e-10;"),
    ("dt=1/4", "options.dt=0.25;"),
    ("derived compact-pair law", "ConnectedBindingLaw::DerivedCompactPair"),
    ("well depth 0.01", "options.compact_pair_well_depth=0.01;"),
    ("cutoff squared 3/2", "options.compact_pair_cutoff_distance_squared=1.5;"),
    ("solve tolerance 2e-14", "options.solve_tolerance=2e-14;"),
    ("384 nonlinear iterations", "options.max_iterations=384;"),
    ("sparse current", "options.use_sparse_local_current=true;"),
    ("local residual evaluation", "options.use_local_residual_evaluation=true;"),
    ("unbound separation 1.30", "unbound ? 1.30 : 1.00"),
    ("unbound momentum 0.0120", "unbound ? 0.0120 : kBoundMomentum"),
    ("finite preparation tolerance", "kFiniteSupportRadius,1e-13,4096"),
    ("five-arm matrix size", "normalization.valid&&arms.size()==5"),
    ("four unbound histories", "arm.family==\"unbound\""),
    ("one bound history", "arm.family==\"bound\""),
    ("136 forward roots", "tick=1; tick<=kFiniteTicks; ++tick"),
    ("136 reverse roots", "solve_connected_moore_block_reverse"),
    ("273 rows per complete arm", "2*kFiniteTicks+1"),
    ("regional radius-8 observer", "center,kInnerShell"),
    ("regional radius-12 observer", "center,kOuterShell"),
    ("source radius gate 3", "arm.maximum_source_radius<=3"),
    ("negative-core onset at most 120", "arm.energetic_onset_tick<=120"),
    ("negative tail at least 16 ticks", "kFiniteTicks-arm.energetic_onset_tick+1>=16"),
    ("first-passage residual 1e-8", "arm.maximum_first_passage_residual<=1e-8"),
    ("outside tail threshold", "arm.maximum_outside_12>1e-6"),
    ("outward transport threshold", "arm.maximum_cumulative_outward_12>1e-6"),
    ("final outside energy threshold", "arm.final_outside_12>1e-7"),
    ("recoil threshold 1e-9", "row.recoil_defect<=1e-9"),
    ("speed threshold 1e-12", "row.speed_excess<=1e-12"),
    ("inverse threshold 1e-8", "arm.inverse_recovery<=1e-8"),
    ("explicit per-step energy gate", "arm.maximum_energy_residual<=1e-8"),
    ("pair-field balance gate", "arm.pair_field_balance<=1e-8"),
    ("full persisted-row polarity comparison", "lhs.boundary_transport_into_12"),
    ("reverse bound-control check", "for(const auto& row:arm.rows)"),
    ("CSV serialization", "ftd_0739_finite_support_outgoing_tail_formation_v1.csv"),
    ("JSON serialization", "ftd_0739_finite_support_outgoing_tail_formation_v1.json"),
):
    check(label, token in runner)

verdicts = (
    "FINITE_SUPPORT_FORMATION_EXECUTION_INVALID",
    "FINITE_SUPPORT_BOUND_CONTROL_UNSTABLE",
    "FINITE_SUPPORT_FORMATION_POLARITY_SENSITIVE",
    "FINITE_SUPPORT_NO_DURABLE_NEGATIVE_CORE_ALL_RAYS",
    "FINITE_SUPPORT_CAPTURE_ENERGY_LEDGER_MISMATCH",
    "FINITE_SUPPORT_CORE_WITHOUT_OUTGOING_TAIL",
    "FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_CONSTRUCTIVE",
)
for verdict in verdicts:
    check(f"locked verdict {verdict}", f'verdict="{verdict}"' in runner)

for obsolete in (
    "FINITE_SUPPORT_OUTGOING_TAIL_FORMATION_EXECUTION_INVALID",
    'verdict="NO_DURABLE_NEGATIVE_CORE_ALL_RAYS"',
    "FINITE_SUPPORT_PAIR_FIELD_ENERGY_LEDGER_MISMATCH",
):
    check(f"obsolete verdict absent: {obsolete}", obsolete not in runner)

check(
    "registered CTest target",
    "ftd_add_test(test_finite_support_outgoing_tail_formation" in cmake
    and "CTEST_NAME finite_support_outgoing_tail_formation" in cmake,
)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0739 protocol conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
