// ============================================================================
// test_gauge_gpu_parity.cpp — SU(2)/SU(3) gauge-link relaxation: CPU/GPU
// parity + GPU determinism (revision 0.9 option a; pattern:
// test_langevin_gpu_cpu_parity / test_gpu_golden).
//
// What is asserted:
//   P1. CPU vs GPU link parity: the same L=9 harness (default toggles +
//       su2_gauge + su3_gauge, standard perturbed links, 20 ticks) produces
//       element-wise link agreement within TOL. Bit-exactness is NOT
//       expected: nvcc contracts FMA by default and the CPU staple products
//       associate A*(B*C) vs the GPU's (A*B)*C — both are documented,
//       value-level-equivalent differences. TOL is the parity criterion.
//   P2. GPU run-to-run determinism: two identical GPU runs produce
//       byte-identical link arrays. This is the GPU-side race tripwire —
//       the Jacobi double-buffered kernels (kernels_gauge.cu) have no
//       cross-thread neighbor hazard, so any nondeterminism here means the
//       src/dst separation regressed.
//
// The links never read the substrate (write-only sector), so CPU-SOR vs
// GPU-FFT voxel divergence — which is documented and expected — cannot leak
// into the link comparison.
//
// Platform policy (mirrors test_gpu_golden): canonical GPU platform is the
// WSL2 build (engine/build_wsl); strict there. On CPU-only builds the GPU
// bridge silently runs the CPU backend — detected via backend_kind() and
// reported as SKIP.
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/backend.h"
#include "ftd/gauge_field.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"
#include "support/gauge_test_utils.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace ftd { namespace test {

static constexpr int    L_PARITY   = 9;
static constexpr int    N_TICKS    = 20;
static constexpr double PARITY_TOL = 1e-10;  // max element-wise |CPU - GPU|

static void inject_standard_state_l9(RenderBridge& rb) {
    rb.inject_particle(2, 2, 2, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(6, 6, 6, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(4, 2, 6, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(4, 4, 4, Vec3{1.0, 0.0, 0.0});
}

// Run the gauge harness on the requested backend; returns the bridge so the
// caller can read the (downloaded) link arrays.
static void run_gauge_harness(RenderBridge& rb, bool force_cpu) {
    if (force_cpu) rb.force_cpu();
    rb.seed_rng(42);
    rb.toggles.su2_gauge = true;
    rb.toggles.su3_gauge = true;
    perturb_links(rb, 0.05);        // BEFORE the first tick — the GPU path
                                    // uploads the host links on activation
    inject_standard_state_l9(rb);
    rb.seed_rng(42);
    for (int t = 0; t < N_TICKS; ++t) rb.tick();
}

static double max_link_delta(const RenderBridge& a, const RenderBridge& b) {
    double d = 0.0;
    auto upd = [&d](const std::complex<double>& x, const std::complex<double>& y) {
        d = std::max(d, std::abs(x.real() - y.real()));
        d = std::max(d, std::abs(x.imag() - y.imag()));
    };
    const std::vector<SU2Link>* a2[3] = {&a.su2_links_x(), &a.su2_links_y(), &a.su2_links_z()};
    const std::vector<SU2Link>* b2[3] = {&b.su2_links_x(), &b.su2_links_y(), &b.su2_links_z()};
    for (int k = 0; k < 3; ++k) {
        for (std::size_t i = 0; i < a2[k]->size(); ++i) {
            upd((*a2[k])[i].a, (*b2[k])[i].a);
            upd((*a2[k])[i].b, (*b2[k])[i].b);
        }
    }
    const std::vector<SU3Link>* a3[3] = {&a.su3_links_x(), &a.su3_links_y(), &a.su3_links_z()};
    const std::vector<SU3Link>* b3[3] = {&b.su3_links_x(), &b.su3_links_y(), &b.su3_links_z()};
    for (int k = 0; k < 3; ++k) {
        for (std::size_t i = 0; i < a3[k]->size(); ++i) {
            for (int r = 0; r < 3; ++r)
                for (int c = 0; c < 3; ++c)
                    upd((*a3[k])[i].m[r][c], (*b3[k])[i].m[r][c]);
        }
    }
    return d;
}

void test_gauge_gpu_parity() {
    section("P1: CPU vs GPU link parity");

    RenderBridge rb_cpu(L_PARITY);
    run_gauge_harness(rb_cpu, /*force_cpu=*/true);

    RenderBridge rb_gpu(L_PARITY);   // GPU backend if this build has one
    run_gauge_harness(rb_gpu, /*force_cpu=*/false);

    if (rb_gpu.backend_kind() != Backend::Kind::Gpu) {
        std::printf("[gauge-parity] SKIP: no GPU backend in this build — the "
                    "\"GPU\" bridge ran the CPU path.\n");
        check("gauge parity skipped on CPU-only build", true, "");
        return;
    }

    const double delta = max_link_delta(rb_cpu, rb_gpu);
    std::printf("[gauge-parity] max element-wise |CPU - GPU| = %.3e (tol %.1e)\n",
                delta, PARITY_TOL);
    check("CPU and GPU link relaxation agree element-wise",
          delta < PARITY_TOL,
          "CPU/GPU gauge sweeps diverged beyond FP-contraction scale — check "
          "GAUGE_RELAX_DT/BETA plumbing, sweep order, or upload/download "
          "marshalling (GpuBackend::tick / sync_to_host)");

    section("P2: GPU run-to-run determinism (Jacobi race tripwire)");
    RenderBridge rb_gpu2(L_PARITY);
    run_gauge_harness(rb_gpu2, /*force_cpu=*/false);
    const std::uint64_t h1 = hash_all_links(rb_gpu);
    const std::uint64_t h2 = hash_all_links(rb_gpu2);
    std::printf("[gauge-parity] GPU run 1 = 0x%016llx run 2 = 0x%016llx\n",
                (unsigned long long)h1, (unsigned long long)h2);
    check("two identical GPU runs produce byte-identical links",
          h1 == h2,
          "GPU gauge relaxation is nondeterministic — the double-buffered "
          "src/dst separation in kernels_gauge.cu has regressed (in-place "
          "neighbor race)");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_gauge_gpu_parity");
    ftd::test::test_gauge_gpu_parity();
    return ftd::test::finalize();
}
