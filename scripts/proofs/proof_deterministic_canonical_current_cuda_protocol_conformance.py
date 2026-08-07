"""Static pre-execution conformance certificate for FTD-0749."""

from __future__ import annotations

import hashlib
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
PROTOCOL = ROOT / "docs/theory/10_eft_program/preregistrations/engine_infrastructure_rg/PREREG_DETERMINISTIC_CANONICAL_CURRENT_CUDA_v1.md"
PREEXEC = ROOT / "docs/theory/07_assessment/engine_infrastructure_rg/AUDIT_DETERMINISTIC_CANONICAL_CURRENT_CUDA_PREEXEC_v1.md"
RUNNER = ROOT / "engine/tests/campaign_deterministic_canonical_current_cuda.cpp"
CUDA_HEADER = ROOT / "engine/include/ftd/eft/cuda_matched_field_pipeline.h"
CUDA_SOURCE = ROOT / "engine/cuda/cuda_matched_field_pipeline.cu"
UNIT = ROOT / "engine/tests/test_cuda_canonical_current_deposition.cpp"
CMAKE = ROOT / "engine/CMakeLists.txt"
RESULTS = ROOT / "engine/results/ftd_0749"
EXPECTED_PROTOCOL = "6C0BE1E8109DBD17451FF3A21F426A75583120810EB8C0C9B9077056AE86BB83"

checks: list[tuple[str, bool]] = []


def check(label: str, condition: bool) -> None:
    checks.append((label, bool(condition)))
    print(f"{'PASS' if condition else 'FAIL'}  {label}")


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


protocol = PROTOCOL.read_text(encoding="utf-8")
runner = RUNNER.read_text(encoding="utf-8")
header = CUDA_HEADER.read_text(encoding="utf-8")
cuda = CUDA_SOURCE.read_text(encoding="utf-8")
unit = UNIT.read_text(encoding="utf-8")
cmake = CMAKE.read_text(encoding="utf-8")

check("protocol SHA-256 is frozen", sha256(PROTOCOL) == EXPECTED_PROTOCOL)
check("runner embeds protocol SHA-256", EXPECTED_PROTOCOL in runner)
check("held-out execution refuses UNLOCKED", '=="UNLOCKED"' in runner)
check("pre-execution result absence is current or immutably witnessed", (
    not RESULTS.exists()
    or "Immediately before held-out execution, `engine/results/ftd_0749/` did not\nexist."
       in PREEXEC.read_text(encoding="utf-8")
))
check("FTD-0749 identifier serialized", '"FTD-0749"' in runner)
check("unique-face backend serialized", "wsl2_cuda_unique_face_current" in runner)
check("replicates restricted to a/b", 'replicate!="a"&&replicate!="b"' in runner)
check("qualification capped at eight ticks", "ticks>8" in runner)
check("qualification does not call result writer", (
    runner.index("if(qualification)") < runner.index("write_horizon_records")
))
check("runner derives replicate-specific slug", 'slug+"_"+replicate' in runner)
check("runner transforms only inherited deposition call", (
    "#define apply_sparse_current apply_canonical_sparse_current" in runner
    and "#undef apply_sparse_current" in runner
))
check("inherited FTD-0748 run function retained", (
    "run_canonical_horizon_cuda_arm" in runner
))
check("inherited ordered verdict retained", "canonical_horizon_verdict" in runner)
check("main record horizon stem is FTD-0749", (
    "ftd_0749_deterministic_canonical_current_cuda_v1" in runner
))
check("support record horizon stem is FTD-0749", (
    '"ftd_0749_deterministic_canonical_current_cuda_v1_"' in runner
))

check("research-only canonical API declared", "apply_canonical_sparse_current" in header)
check("legacy API remains declared", "apply_sparse_current" in header)
check("canonical API documented as production-preserving", (
    "every production caller unchanged" in header
))
check("canonical path uses zero filtering tolerance", (
    "segments,polarity_scale,0.0" in cuda
))
check("canonical path rejects invalid aggregate", (
    "!current.valid||current.L!=impl_->L" in cuda
))
check("canonical path uploads aggregate entries", (
    "entries.reserve(current.entries.size())" in cuda
))
check("unique kernel exists", "apply_unique_current_kernel" in cuda)
unique_kernel = cuda[cuda.index("__global__ void apply_unique_current_kernel"):
                     cuda.index("__global__ void scatter_density_kernel")]
check("unique kernel uses direct component update", "component[entry.index]+=increment" in unique_kernel)
check("unique kernel has no floating atomic deposition", "atomicAdd" not in unique_kernel)
legacy_kernel = cuda[cuda.index("__global__ void apply_current_kernel"):
                     cuda.index("__global__ void apply_unique_current_kernel")]
check("legacy kernel remains unchanged and atomic", "atomicAdd" in legacy_kernel)
check("canonical launch uses already-scaled coefficient", (
    "entries.size(),-1.0" in cuda
))
check("canonical state flags match pipeline contract", (
    "impl_->current_applied=true" in cuda and "impl_->observed=false" in cuda
))

check("unit uses colliding periodic faces", "{{13,4,4},0,0.25}" in unit)
check("unit has six raw contributions", "canonical.raw_contributions!=6" in unit)
check("unit requires three net faces", "canonical.entries.size()!=3" in unit)
check("unit reverses segment order", "std::reverse(permuted.begin()" in unit)
check("unit reverses entry order", "std::reverse(item.sparse_current.begin()" in unit)
check("unit runs independent pipelines", "CudaMatchedFieldPipeline first(L),second(L)" in unit)
check("unit calls canonical API twice", unit.count("apply_canonical_sparse_current") == 2)
check("unit requires exact repeat identity", "repeat_difference==0.0" in unit)
check("unit requires exact host/device identity", "expected_difference==0.0" in unit)
check("unit requires exact magnetic identity", "magnetic_difference==0.0" in unit)
check("unit target registered", "test_cuda_canonical_current_deposition" in cmake)
check("campaign target registered", "campaign_deterministic_canonical_current_cuda" in cmake)

for token in (
    "L=321", "ticks `0..312`", "contact tick 313", "`dt=1/4`",
    "solve tolerance `2e-14`", "maximum 384 iterations",
    "deadline 300", "post-arrival `301..312`", "D0 replay identity",
    "exactly once", "without tolerance\nrelaxation",
):
    check(f"protocol contains {token}", token in protocol)

passed = sum(condition for _, condition in checks)
print(f"\nFTD-0749 static conformance: {passed}/{len(checks)} checks passed")
raise SystemExit(0 if passed == len(checks) else 1)
