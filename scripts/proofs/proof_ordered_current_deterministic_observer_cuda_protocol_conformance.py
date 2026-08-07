"""Static pre-execution conformance certificate for FTD-0750."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/engine_infrastructure_rg/PREREG_ORDERED_CURRENT_DETERMINISTIC_OBSERVER_CUDA_v1.md"
PREEXEC = ROOT / "docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_ORDERED_CURRENT_DETERMINISTIC_OBSERVER_CUDA_PREEXEC_v1.md"
RUNNER = ROOT / "engine/tests/campaign_ordered_current_observer_cuda.cpp"
CUDA_HEADER = ROOT / "engine/include/ftd/eft/cuda_matched_field_pipeline.h"
CUDA_SOURCE = ROOT / "engine/cuda/cuda_matched_field_pipeline.cu"
UNIT = ROOT / "engine/tests/test_cuda_ordered_current_observer.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
RESULTS = ROOT / "engine/results/ftd_0750"
EXPECTED_PROTOCOL = "C3A3E787A201F1E429E1ED8D8D81B9F06B508A413B41A6B5E2584ED1BFD13385"

checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


protocol = PROTOCOL.read_text(encoding="utf-8")
preexec = PREEXEC.read_text(encoding="utf-8")
runner = RUNNER.read_text(encoding="utf-8")
header = CUDA_HEADER.read_text(encoding="utf-8")
cuda = CUDA_SOURCE.read_text(encoding="utf-8")
unit = UNIT.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

check("protocol SHA-256 is frozen", sha256(PROTOCOL) == EXPECTED_PROTOCOL)
check("runner embeds protocol SHA-256", EXPECTED_PROTOCOL in runner)
check("held-out execution refuses UNLOCKED", '=="UNLOCKED"' in runner)
check("pre-execution absence is immutably witnessed", (
    not RESULTS.exists()
    or "Immediately before held-out execution, `engine/results/ftd_0750/` did not\nexist."
       in preexec
))
check("FTD-0750 identifier serialized", '"FTD-0750"' in runner)
check("ordered backend serialized", "wsl2_cuda_ordered_current_observer" in runner)
check("replicates restricted to a/b", 'replicate!="a"&&replicate!="b"' in runner)
check("qualification capped at eight ticks", "ticks>8" in runner)
check("runner transforms inherited deposition", (
    "#define apply_sparse_current apply_ordered_sparse_current" in runner
    and "#undef apply_sparse_current" in runner
))
check("runner transforms inherited observer", (
    "#define observe observe_deterministic" in runner
    and "#undef observe" in runner
))
check("inherited run and verdict retained", (
    "run_canonical_horizon_cuda_arm" in runner
    and "canonical_horizon_verdict" in runner
))

check("ordered research API declared", "apply_ordered_sparse_current" in header)
check("deterministic observer API declared", "observe_deterministic" in header)
check("legacy APIs remain declared", (
    "apply_sparse_current" in header and "BatchedRegionalEnergyProfile observe(" in header
))
ordered_method = cuda[cuda.index("bool CudaMatchedFieldPipeline::apply_ordered_sparse_current"):
                      cuda.index("BatchedRegionalEnergyProfile CudaMatchedFieldPipeline::observe(")]
check("ordered method groups by canonical axis/index", (
    "std::map<Key,std::vector<double>> grouped" in ordered_method
))
check("ordered method preserves encounter order", (
    "grouped[{entry.axis,index}].push_back(entry.value)" in ordered_method
))
ordered_kernel = cuda[cuda.index("__global__ void apply_ordered_current_kernel"):
                      cuda.index("__global__ void scatter_density_kernel")]
check("ordered kernel has one group writer", "i<group_count" in ordered_kernel)
check("ordered kernel uses explicit rounded multiply", "__dmul_rn" in ordered_kernel)
check("ordered kernel uses explicit rounded addition", "__dadd_rn" in ordered_kernel)
check("ordered kernel has no floating deposition atomic", "atomicAdd" not in ordered_kernel)
observer_kernel = cuda[cuda.index("__global__ void deterministic_regional_profile_kernel"):
                       cuda.index("double milliseconds_since")]
check("deterministic observer has fixed selected-radius bound", (
    "kDeterministicMaximumRadii" in observer_kernel
))
check("deterministic observer uses fixed reduction tree", (
    "for(int stride=blockDim.x/2;stride>0;stride>>=1)" in observer_kernel
))
check("deterministic observer has no floating reduction atomic", (
    "atomicAdd" not in observer_kernel
))
check("legacy regional histogram remains atomic and separate", (
    "__global__ void regional_profile_kernel" in cuda
    and "atomicAdd(histogram" in cuda and "atomicAdd(bins" in cuda
))

check("unit uses duplicate periodic faces", "{{13,4,4},0,0.25}" in unit)
check("unit requires exact CPU/device parity", "cpu_difference==0.0" in unit)
check("unit requires exact independent replay", "repeat_difference==0.0" in unit)
check("unit tests order sensitivity", "order_sensitivity>0.0" in unit)
check("unit requires bit-identical observer replay", "observer_repeat" in unit)
check("unit target registered", "test_cuda_ordered_current_observer" in cmake)
check("campaign target registered", "campaign_ordered_current_observer_cuda" in cmake)

for token in (
    "L=321", "ticks `0..312`", "contact tick 313", "`dt=1/4`",
    "solve tolerance `2e-14`", "maximum 384 iterations",
    "deadline 300", "post-arrival `301..312`", "D0 strict replay identity",
    "exactly once", "without tolerance relaxation",
):
    check(f"protocol contains {token}", token in protocol)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0750 static conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
