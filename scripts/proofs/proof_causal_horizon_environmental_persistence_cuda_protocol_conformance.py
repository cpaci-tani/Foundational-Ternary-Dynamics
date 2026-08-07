"""Static pre-execution conformance certificate for FTD-0747."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/constituent_complete_matter/PREREG_CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CUDA_v2.md"
RUNNER = ROOT / "engine/tests/campaign_causal_horizon_environmental_persistence_cuda.cpp"
PARENT = ROOT / "engine/tests/test_causal_horizon_environmental_persistence.cpp"
CUDA = ROOT / "engine/cuda/cuda_matched_field_pipeline.cu"
CMAKE = ROOT / "engine/CMakeLists.txt"
BASELINE = ROOT / "engine/results/ftd_0745/ftd_0745_finite_support_environmental_closure_v1.csv"

PROTOCOL_SHA256 = "1FB4A49897D8FEC333C686A54D44A90EA6E51D799EDBD9168F8D313287F4FD5F"
BASELINE_SHA256 = "58D85CB5B593E54EC687DC334CF4894572779CDD4BDB4916246D01550D86C41C"

protocol = PROTOCOL.read_text(encoding="utf-8")
runner = RUNNER.read_text(encoding="utf-8")
parent = PARENT.read_text(encoding="utf-8")
cuda = CUDA.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")
source = runner + parent
checks: list[tuple[str, bool]] = []


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


check("locked protocol hash", sha256(PROTOCOL) == PROTOCOL_SHA256)
check("frozen baseline hash", sha256(BASELINE) == BASELINE_SHA256)
check("runner embeds protocol hash", PROTOCOL_SHA256 in runner)
check("unlock token removed", 'kCudaHorizonProtocolSha256[] = "UNLOCKED"' not in runner)
check("held-out run refuses unlocked source", "held-out execution refused" in runner)
check("L=321", "kHorizonL = 321" in parent)
check("T=312", "kHorizonTicks = 312" in parent)
check("contact tick formula", "kHorizonL-2*kHorizonSupportRadius" in parent)
check("six radii", "{8,12,16,24,32,48}" in parent)
check("three selected rays", all(token in parent for token in (
    'slug=="face"', 'slug=="edge"', 'slug=="body"')))
check("dt quarter", "options.dt=0.25" in runner)
check("selected pair law", "ConnectedBindingLaw::DerivedCompactPair" in runner)
check("unchanged depth", "compact_pair_well_depth=0.01" in runner)
check("unchanged cutoff", "compact_pair_cutoff_distance_squared=1.5" in runner)
check("locked root tolerance", "solve_tolerance=2e-14" in runner)
check("locked root iterations", "max_iterations=384" in runner)
check("sparse current", "use_sparse_local_current=true" in runner)
check("local residual", "use_local_residual_evaluation=true" in runner)
check("deferred diagnostics explicitly CUDA-only", "defer_volume_diagnostics=true" in runner)
check("CUDA prepared field update", "pipeline.prepare_forward(lambda)" in runner)
check("prepared implicit root", "solve_connected_moore_block_forward_prepared" in runner)
check("CUDA sparse source", "pipeline.apply_sparse_current" in runner)
check("CUDA regional observer", "pipeline.observe" in runner)
check("CUDA volume diagnostics", "pipeline.diagnose_common_action" in runner)
check("completed common action", "complete_connected_moore_block_volume_diagnostics" in runner)
check("device state advances", "pipeline.advance()" in runner)
check("no reverse solve", "solve_connected_moore_block_reverse" not in runner)
check("matched curl kernels", all(token in cuda for token in (
    "prepare_magnetic_kernel", "prepare_electric_kernel", "apply_current_kernel")))
check("GPU Gauss diagnostic", "gauss_residual_kernel" in cuda)
check("GPU local momentum", "local_translation_momentum_kernel" in cuda)
check("GPU spline momentum", all(token in cuda for token in (
    "convolve_axis_kernel", "dot_reduce_kernel", "make_spline_stencil")))
check("FTD-0745 prefix", "horizon_prefix_difference" in source)
check("source radius gate", "maximum_source_radius<=3" in runner)
check("160 tick core", ">=160" in runner)
check("near-field gate", "late_inside_8_minimum>=kHorizonNearMinimum" in runner)
check("arrival deadline", "first_tail_tick[r48]<=kHorizonArrivalDeadline" in runner)
check("post-arrival gate", "minimum_outward_increment[r48]>=-kHorizonGate" in runner)
check("standard JSON null serializer", all(token in parent for token in (
    "void horizon_json_number", "std::isfinite(value)", 'output<<"null"')))
check("FTD-0747 result folder", '"ftd_0747"' in runner)
check("CUDA backend label", '"wsl2_cuda_matched_face_edge"' in runner)
check("registered CUDA target", all(token in cmake for token in (
    "campaign_causal_horizon_environmental_persistence_cuda",
    "tests/campaign_causal_horizon_environmental_persistence_cuda.cpp")))
for verdict in (
    "CAUSAL_HORIZON_EXECUTION_INVALID",
    "CAUSAL_HORIZON_PREFIX_DRIFT",
    "CAUSAL_HORIZON_CORE_NOT_PERSISTENT",
    "CAUSAL_HORIZON_NEAR_FIELD_NOT_STABLE",
    "CAUSAL_HORIZON_R48_ARRIVAL_FAIL",
    "CAUSAL_HORIZON_POST_ARRIVAL_NOT_PERSISTENT",
    "CAUSAL_HORIZON_ENVIRONMENTAL_PERSISTENCE_CONSTRUCTIVE",
):
    check(f"locked verdict {verdict}", verdict in parent and verdict in protocol)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0747 CUDA protocol conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
