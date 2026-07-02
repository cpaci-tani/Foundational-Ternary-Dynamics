// ============================================================================
// test_gauge_links.cpp — SU(2)/SU(3) gauge-link sector characterization
// (revision 0.9).
//
// AUDIT FINDING (2026-07-02, engine revision program): the non-Abelian
// gauge sector is DEFINED BUT DISCONNECTED on both backends:
//   - CPU: relax_su2_links_cpu / relax_su3_links_cpu
//     (engine/src/transmutation_phases.cpp:189/298) have ZERO call sites.
//   - GPU: launch_relax_su2_links / launch_relax_su3_links
//     (engine/cuda/kernels_gauge.cu:440/449) have ZERO call sites.
//   - The public toggles su2_gauge / su3_gauge (TOGGLE_SPECS, settable from
//     JS via rb_toggle_map) are read by NOTHING — setting them is a silent
//     no-op.
//   - The 6 link buffers (528 B/site = 3×32 B SU2 + 3×144 B SU3; ~132 MiB
//     at L=64, larger than the voxel array itself) were allocated
//     unconditionally by every RenderBridge; they are now LAZY
//     (revision 4.1b — ensure_gauge_links(), asserted by G0 below).
//   - relax_su2/su3_links_cpu previously relaxed IN PLACE under
//     `#pragma omp parallel for`, reading neighbor links other threads were
//     writing — a data race. FIXED (revision 0.9 option a, step 1): both
//     sweeps are Jacobi double-buffered (read pre-sweep state, write
//     scratch, swap), so the result is thread-count invariant (G4).
//
// Sections:
//   G0. Link buffers are lazily allocated; accessors materialize the
//       identity configuration on demand.
//   G1. Toggle tripwire: su2_gauge/su3_gauge ON produces a bit-identical
//       run to defaults. If this FAILS, the toggles have been wired into
//       the tick — write a gauge golden profile and update this test.
//   G2. SU(2) relaxation from a deterministically perturbed configuration
//       preserves unitarity (|a|²+|b|²=1, the projection normalize() runs
//       on every update) and produces finite values.
//   G3. SU(3) relaxation from a perturbed configuration stays finite;
//       U†U−I deviation is measured and reported (characterization).
//   G4. Determinism + thread-count invariance: repeat runs are byte-
//       identical, and a single-thread run matches a full-thread-pool run
//       bit-exactly (the Jacobi double-buffer guarantee — this is the
//       regression tripwire for the fixed race).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/transmutation_phases.h"
#include "ftd/gauge_field.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"

#ifdef _OPENMP
#include <omp.h>
#endif

#include <cstdint>
#include <cstdio>
#include <cmath>
#include <complex>
#include <cstring>
#include <vector>

namespace ftd { namespace test {

// Deterministic perturbation of the (identity-initialized) link fields.
// Test-only const_cast: the buffers have const accessors and the relax
// functions are RenderBridge friends; there is no public mutator because
// nothing in the engine writes them yet.
static void perturb_links(RenderBridge& rb, double eps) {
    auto& lx = const_cast<std::vector<SU2Link>&>(rb.su2_links_x());
    auto& ly = const_cast<std::vector<SU2Link>&>(rb.su2_links_y());
    auto& lz = const_cast<std::vector<SU2Link>&>(rb.su2_links_z());
    auto& mx = const_cast<std::vector<SU3Link>&>(rb.su3_links_x());
    std::uint64_t s = 0x9e3779b97f4a7c15ULL;
    auto next = [&s]() {
        s ^= s << 13; s ^= s >> 7; s ^= s << 17;
        return (double)(s >> 11) / (double)(1ULL << 53) - 0.5;
    };
    for (std::size_t i = 0; i < lx.size(); ++i) {
        for (auto* l : {&lx[i], &ly[i], &lz[i]}) {
            l->a += std::complex<double>(eps * next(), eps * next());
            l->b += std::complex<double>(eps * next(), eps * next());
            l->normalize();
        }
        // SU3: small off-diagonal perturbation, x-direction only (enough to
        // move the relaxation off the identity fixed point).
        mx[i].m[0][1] += std::complex<double>(eps * next(), eps * next());
        mx[i].m[1][0] -= std::conj(mx[i].m[0][1]);
    }
}

static std::uint64_t hash_su2_links(const RenderBridge& rb) {
    std::uint64_t h = GOLDEN_FNV_OFFSET;
    for (const auto* v : {&rb.su2_links_x(), &rb.su2_links_y(), &rb.su2_links_z()}) {
        for (const auto& l : *v) {
            h = mix_double(h, l.a.real()); h = mix_double(h, l.a.imag());
            h = mix_double(h, l.b.real()); h = mix_double(h, l.b.imag());
        }
    }
    return h;
}

static std::uint64_t hash_all_links(const RenderBridge& rb) {
    std::uint64_t h = hash_su2_links(rb);
    for (const auto* v : {&rb.su3_links_x(), &rb.su3_links_y(), &rb.su3_links_z()}) {
        for (const auto& l : *v) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    h = mix_double(h, l.m[i][j].real());
                    h = mix_double(h, l.m[i][j].imag());
                }
            }
        }
    }
    return h;
}

static void inject_standard_state(RenderBridge& rb) {
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

static std::uint64_t run_default_harness(bool gauge_toggles_on) {
    RenderBridge rb(17);
    rb.force_cpu();
    rb.seed_rng(42);
    if (gauge_toggles_on) {
        rb.toggles.su2_gauge = true;
        rb.toggles.su3_gauge = true;
    }
    inject_standard_state(rb);
    rb.seed_rng(42);
    for (int t = 0; t < 100; ++t) rb.tick();
    return compute_state_hash_ext(rb);
}

void test_gauge_sector() {
    // G0 — lazy allocation (revision 4.1b).
    section("G0: link buffers are lazily allocated");
    {
        RenderBridge rb0(9);
        check("fresh bridge allocates NO link buffers (528 B/site saved)",
              !rb0.gauge_links_allocated(),
              "link buffers are eagerly allocated again — revision 4.1b regressed");
        const auto& lx = rb0.su2_links_x();  // accessor materializes on demand
        check("accessor materializes total_sites() identity links",
              rb0.gauge_links_allocated()
                  && lx.size() == static_cast<std::size_t>(9 * 9 * 9)
                  && rb0.su3_links_z().size() == static_cast<std::size_t>(9 * 9 * 9),
              "ensure_gauge_links() did not produce full-size buffers");
        check("materialized SU(2) links are the identity",
              lx[0].a == std::complex<double>(1.0, 0.0)
                  && lx[0].b == std::complex<double>(0.0, 0.0),
              "lazy allocation no longer identity-initializes");
    }

    // G1 — toggle tripwire.
    section("G1: su2_gauge/su3_gauge toggles are currently no-ops");
    const std::uint64_t h_off = run_default_harness(false);
    const std::uint64_t h_on  = run_default_harness(true);
    std::printf("[gauge] default=0x%016llx gauge-toggles-on=0x%016llx\n",
                (unsigned long long)h_off, (unsigned long long)h_on);
    check("gauge toggles do not change the tick (disconnected sector)",
          h_off == h_on,
          "su2_gauge/su3_gauge now ALTER the tick — the gauge sector has "
          "been wired in. That is a feature change: add a gauge golden "
          "profile (ADR-0012 policy), fix the in-place parallel-for race in "
          "relax_su2_links_cpu first, and update this characterization.");

    // G2 — SU(2) unitarity + finiteness under direct relaxation.
    section("G2: SU(2) relaxation preserves unitarity");
    RenderBridge rb(9);
    rb.force_cpu();
    perturb_links(rb, 0.05);
    for (int it = 0; it < 5; ++it) relax_su2_links_cpu(rb, 0.1, 1.0);
    double max_unit_dev = 0.0;
    bool all_finite = true;
    for (const auto* v : {&rb.su2_links_x(), &rb.su2_links_y(), &rb.su2_links_z()}) {
        for (const auto& l : *v) {
            const double n = std::norm(l.a) + std::norm(l.b);
            max_unit_dev = std::max(max_unit_dev, std::fabs(n - 1.0));
            if (!std::isfinite(l.a.real()) || !std::isfinite(l.a.imag()) ||
                !std::isfinite(l.b.real()) || !std::isfinite(l.b.imag()))
                all_finite = false;
        }
    }
    std::printf("[gauge] SU(2) max | |a|^2+|b|^2 - 1 | = %.3e\n", max_unit_dev);
    check("SU(2) links stay unitary after relaxation", max_unit_dev < 1e-12,
          "normalize() projection no longer enforcing |a|^2+|b|^2=1");
    check("SU(2) links stay finite", all_finite, "NaN/Inf in relaxed links");

    // G3 — SU(3) finiteness + measured unitarity deviation.
    section("G3: SU(3) relaxation stays finite (unitarity measured)");
    for (int it = 0; it < 2; ++it) relax_su3_links_cpu(rb, 0.1, 1.0);
    double max_su3_dev = 0.0;
    bool su3_finite = true;
    for (const auto& l : rb.su3_links_x()) {
        // U†U - I, infinity norm over entries (spot: x-direction links).
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                std::complex<double> sum(0.0, 0.0);
                for (int k = 0; k < 3; ++k) sum += std::conj(l.m[k][i]) * l.m[k][j];
                const double target = (i == j) ? 1.0 : 0.0;
                max_su3_dev = std::max(max_su3_dev, std::abs(sum - target));
                if (!std::isfinite(sum.real()) || !std::isfinite(sum.imag())) su3_finite = false;
            }
        }
    }
    std::printf("[gauge] SU(3) max |U^dag U - I| = %.3e (characterization)\n", max_su3_dev);
    check("SU(3) links stay finite under relaxation", su3_finite,
          "NaN/Inf in relaxed SU(3) links");
    check("SU(3) unitarity deviation bounded", max_su3_dev < 0.5,
          "SU(3) relaxation diverging from the group manifold far faster "
          "than at characterization time");

    // G4 — determinism + thread-count invariance (race-fix tripwire).
    section("G4: determinism + thread-count invariance of the relaxation");
    RenderBridge ra(9), rc(9);
    ra.force_cpu(); rc.force_cpu();
    perturb_links(ra, 0.05);
    perturb_links(rc, 0.05);
#ifdef _OPENMP
    // Run A single-thread, run B with the full thread pool. The Jacobi
    // double-buffer (revision 0.9 option a) makes the sweep read only the
    // pre-sweep state, so the two MUST agree bit-exactly. A mismatch means
    // the in-place neighbor race has been reintroduced.
    const int max_threads = omp_get_max_threads();
    omp_set_num_threads(1);
#endif
    for (int it = 0; it < 5; ++it) relax_su2_links_cpu(ra, 0.1, 1.0);
    for (int it = 0; it < 2; ++it) relax_su3_links_cpu(ra, 0.1, 1.0);
#ifdef _OPENMP
    omp_set_num_threads(max_threads);
#endif
    for (int it = 0; it < 5; ++it) relax_su2_links_cpu(rc, 0.1, 1.0);
    for (int it = 0; it < 2; ++it) relax_su3_links_cpu(rc, 0.1, 1.0);
    const std::uint64_t ha = hash_all_links(ra);
    const std::uint64_t hc = hash_all_links(rc);
    std::printf("[gauge] run A (1 thread) = 0x%016llx run B (thread pool) = 0x%016llx\n",
                (unsigned long long)ha, (unsigned long long)hc);
    check("single-thread and full-thread-pool relaxations agree bit-exactly",
          ha == hc,
          "thread-count changes the result — the in-place neighbor race is back");
    // Repeat-run determinism at the same thread count.
    RenderBridge rd(9);
    rd.force_cpu();
    perturb_links(rd, 0.05);
    for (int it = 0; it < 5; ++it) relax_su2_links_cpu(rd, 0.1, 1.0);
    for (int it = 0; it < 2; ++it) relax_su3_links_cpu(rd, 0.1, 1.0);
    check("two identical relaxations agree bit-exactly",
          hash_all_links(rd) == hc, "relaxation is non-deterministic");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_gauge_links");
    ftd::test::test_gauge_sector();
    return ftd::test::finalize();
}
