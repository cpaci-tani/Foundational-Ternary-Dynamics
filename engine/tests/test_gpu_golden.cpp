// ============================================================================
// test_gpu_golden.cpp — GPU-backend golden characterization (revision 0.7
// GPU half / CUDA audit ticket C4; ADR-0012 amendment).
//
// The CPU goldens pin the CPU backend only (rb.force_cpu()); the GPU tick
// path — including its FFT/spectral Poisson machinery, which is DOCUMENTED
// to diverge from CPU SOR (SPEC_ENGINE.md CPU/GPU parity model) — has no
// bit-exact gate of its own. This test runs the standard golden harness on
// the GPU backend and pins a SEPARATE constant.
//
// Platform policy (see engine/docs/DESIGN_RNG_PORTABILITY.md):
//   - The canonical GPU platform is the WSL2 build (engine/build_wsl,
//     RTX 5090). The pin below was captured there; on Linux the check is
//     STRICT.
//   - Windows-native CUDA is compile-check tier only (CLAUDE.md): the test
//     prints the computed hash for visibility but does not gate, because
//     the MSVC/nvcc toolchain pair is not the canonical campaign platform.
//   - CPU-only builds: the harness silently runs the CPU backend; the test
//     detects that (hash equals the CPU default-profile pin) and SKIPs.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#include <cstdint>
#include <cstdio>

namespace ftd { namespace test {

static void inject_initial_state(RenderBridge& rb) {
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// CPU default-profile pin (test_render_bridge_golden_default.cpp) — used
// only to detect that this build has no GPU backend at runtime.
static constexpr std::uint64_t CPU_DEFAULT_PIN = 0x54fe2f9ab5c0a255ULL;  // re-pinned 2026-07-18 in sync with GOLDEN_HASH_DEFAULT (Term-2 coupling sign amendment)

// ---------------------------------------------------------------------------
// FROZEN GPU GOLDEN HASH (canonical platform: WSL2 gcc + CUDA).
//   - 2026-07-02: initial capture (revision 0.7/C4) — see commit message for
//     the 10-run bit-stability record. Re-baseline policy: ADR-0012.
//   - 2026-07-16: BH-F5 completion (GPU evaporation deterministic threshold →
//     stochastic Boltzmann) verified hash-INVARIANT: in this scenario the
//     particles' FFT-exact self-fields reach E_local ≈ 0.8–1.4 within 20
//     ticks, and the specific SplitMix64 Evaporation draws fire zero times
//     over the run (whole-run survival probability ≈ 0.55 — measured, not
//     assumed; 10-run bit-stability re-verified post-port). The rule change
//     is guarded by test_gpu_evaporation_parity instead, which this scenario
//     cannot see.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GOLDEN_HASH_GPU = 0xf4c90122f1a37d00ULL;  // WSL2 RTX 5090; re-pinned 2026-07-18, Term-2 coupling sign amendment (see test_render_bridge_golden.cpp changelog; was 0xd6c0f7007f5a4f24, captured 2026-07-02); bit-stable across 3 runs

void test_gpu_golden() {
    section("100-tick byte-hash characterization (GPU backend, shipping defaults)");

    RenderBridge rb(17);     // NO force_cpu — GPU backend if this build has one
    rb.seed_rng(42);
    inject_initial_state(rb);
    rb.seed_rng(42);

    for (int t = 0; t < 100; ++t) {
        rb.tick();
    }

    const std::uint64_t hash = compute_state_hash_ext(rb);

    std::printf("[golden-gpu] computed hash = 0x%016llx\n",
                static_cast<unsigned long long>(hash));
    std::printf("[golden-gpu] expected hash = 0x%016llx (WSL2 pin)\n",
                static_cast<unsigned long long>(GOLDEN_HASH_GPU));

    if (hash == CPU_DEFAULT_PIN) {
        std::printf("[golden-gpu] SKIP: hash equals the CPU default-profile pin — "
                    "this build ran the CPU backend (no CUDA device/backend).\n");
        check("gpu golden skipped on CPU-only build", true, "");
        return;
    }

#if defined(__linux__)
    check("hash matches frozen GOLDEN_HASH_GPU (WSL2 canonical)",
          hash == GOLDEN_HASH_GPU,
          "GPU-backend physics changed on the canonical WSL2 platform. If "
          "intentional, state the rationale and update GOLDEN_HASH_GPU.");
#else
    // Windows-native CUDA: informational only (compile-check tier).
    std::printf("[golden-gpu] INFO: non-canonical platform (Windows CUDA) — "
                "hash printed for visibility, not gated. WSL2 pin: 0x%016llx\n",
                static_cast<unsigned long long>(GOLDEN_HASH_GPU));
    check("gpu golden informational on non-canonical platform", true, "");
#endif
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_gpu_golden");
    ftd::test::test_gpu_golden();
    return ftd::test::finalize();
}
