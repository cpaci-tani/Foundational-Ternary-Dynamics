// ============================================================================
// test_gauge_links.cpp — SU(2)/SU(3) gauge-link sector: golden profile +
// invariants (revision 0.9, option a — WIRED).
//
// SECTOR STATUS (2026-07-02, engine revision program 0.9 option a): the
// non-Abelian gauge sector is WIRED into the tick on both backends, gated on
// toggles.su2_gauge / toggles.su3_gauge (default OFF):
//   - CPU: RenderBridge::tick() Rule 7b calls relax_su2/su3_links_cpu
//     (engine/src/transmutation_phases.cpp) once per tick.
//   - GPU: GpuEngine::tick() Phase 7b launches the kernels_gauge.cu sweeps
//     via launch_relax_su2/su3_links; GpuBackend marshals the host arrays
//     (upload once on activation, download each gauge tick).
//   - Both sweeps are Jacobi double-buffered (read pre-sweep state, write
//     scratch, swap) — the original in-place parallel-for neighbor race is
//     fixed and G4 below is its regression tripwire.
//   - The 6 link buffers (528 B/site) are LAZY on both sides (revision
//     4.1b — ensure_gauge_links() / upload_gauge_links(); G0 pins this).
//
// EPISTEMIC STATUS: the relaxation is an [IMPOSED] dynamic — the standard
// Wilson-action staple update imported from lattice gauge theory, with
// [IMPOSED] rate calibrations GAUGE_RELAX_DT/GAUGE_RELAX_BETA (constants.h).
// The links are WRITE-ONLY w.r.t. the substrate: nothing downstream consumes
// them (color_forces uses color labels, not links), so the sector is
// measurement infrastructure, not a derivation of anything. No LEDGER claim
// rides on this wiring.
//
// Sections:
//   G0. Link buffers are lazily allocated; accessors materialize the
//       identity configuration on demand.
//   G1a. Write-only guarantee: a gauge-enabled run (perturbed links) folds
//        to the SAME substrate hash as a defaults run — the link sector
//        cannot alter voxel state, energy audit, or any pinned golden.
//   G1b. Gauge golden profile (ADR-0012): pinned link-fold hash for the
//        L=17 / seed-42 / 100-tick harness with su2_gauge+su3_gauge ON from
//        the standard perturbed configuration. Fails if the sector is
//        unwired, if the sweep semantics change, or if the perturbation
//        stream drifts.
//   G2. SU(2) relaxation preserves unitarity and stays finite.
//   G3. SU(3) relaxation stays finite; U†U−I deviation measured.
//   G4. Determinism + thread-count invariance (the race-fix tripwire).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/transmutation_phases.h"
#include "ftd/gauge_field.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"
#include "support/golden_hash.h"
#include "support/gauge_test_utils.h"

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

static void inject_standard_state(RenderBridge& rb) {
    rb.inject_particle( 3,  3,  3, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle(12, 12, 12, -1, Vec3{0.0, 0.0, 0.0});
    rb.inject_particle( 8,  3, 12, +1, Vec3{0.0, 0.0, 0.0});
    rb.inject_flux(8, 8, 8, Vec3{1.0, 0.0, 0.0});
}

// ---------------------------------------------------------------------------
// FROZEN GAUGE GOLDEN HASH (CPU backend; ADR-0012 multi-profile policy).
// Profile: L=17, seed 42, standard injection, su2_gauge + su3_gauge ON,
// links perturbed by perturb_links(rb, 0.05) BEFORE the run, 100 ticks,
// hash_all_links() fold (SU2 then SU3, x/y/z, fixed traversal order).
//   - 2026-07-02: initial capture (revision 0.9 option a wiring commit) —
//     stable across 3 consecutive runs and OMP_NUM_THREADS=1 vs full pool
//     (Jacobi thread-count invariance). Re-baseline policy: ADR-0012.
// ---------------------------------------------------------------------------
static constexpr std::uint64_t GAUGE_GOLDEN_HASH = 0xa4dec20d1dd94ec8ULL;  // captured 2026-07-02 (MSVC, /fp:precise), stable ×3 + OMP=1

struct GaugeRun {
    std::uint64_t substrate;  // compute_state_hash_ext fold (voxels + audit)
    std::uint64_t links;      // hash_all_links fold
};

static GaugeRun run_profile(bool gauge_on) {
    RenderBridge rb(17);
    rb.force_cpu();
    rb.seed_rng(42);
    if (gauge_on) {
        rb.toggles.su2_gauge = true;
        rb.toggles.su3_gauge = true;
        // Identity links are exactly stationary under the staple update, so
        // the golden profile starts from the standard perturbed configuration.
        perturb_links(rb, 0.05);
    }
    inject_standard_state(rb);
    rb.seed_rng(42);
    for (int t = 0; t < 100; ++t) rb.tick();
    return { compute_state_hash_ext(rb), hash_all_links(rb) };
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

    // G1a — write-only guarantee.
    section("G1a: gauge relaxation is write-only w.r.t. the substrate");
    const GaugeRun off = run_profile(false);
    const GaugeRun on  = run_profile(true);
    std::printf("[gauge] substrate: defaults=0x%016llx gauge-on=0x%016llx\n",
                (unsigned long long)off.substrate, (unsigned long long)on.substrate);
    check("gauge-enabled run folds to the identical substrate hash",
          off.substrate == on.substrate,
          "the link sector now FEEDS BACK into the substrate — that is a new "
          "physics coupling, not a wiring detail: it needs its own golden "
          "profile decision (ADR-0012), an epistemic-tag statement, and a "
          "LEDGER-facing rationale before it can land.");

    // G1b — gauge golden profile.
    section("G1b: gauge golden profile (ADR-0012)");
    std::printf("[gauge] links fold = 0x%016llx (pinned 0x%016llx)\n",
                (unsigned long long)on.links,
                (unsigned long long)GAUGE_GOLDEN_HASH);
    // Unwire tripwire: a freshly perturbed, UNRELAXED configuration must not
    // match the post-run fold — if it does, the toggles no longer drive the
    // relaxation and the sector has been silently disconnected again.
    {
        RenderBridge ref(17);
        ref.force_cpu();
        perturb_links(ref, 0.05);
        check("wired sector actually evolves the links",
              on.links != hash_all_links(ref),
              "su2_gauge/su3_gauge no longer relax the links — the sector has "
              "been unwired; restore Rule 7b or retire the toggles honestly");
    }
    check("links fold matches frozen GAUGE_GOLDEN_HASH",
          on.links == GAUGE_GOLDEN_HASH,
          "gauge-link relaxation output changed. If intentional (rate "
          "constants, sweep semantics, perturbation stream), state the "
          "rationale and re-pin per ADR-0012.");

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
