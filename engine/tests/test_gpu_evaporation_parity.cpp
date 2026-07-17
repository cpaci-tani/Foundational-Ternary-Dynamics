// ============================================================================
// test_gpu_evaporation_parity.cpp — CPU↔GPU stochastic-evaporation parity
// (BH-F5 completion, 2026-07-16).
//
// Guards the GPU port of the canonical stochastic Boltzmann evaporation rule
// (phase_write.cpp Loop 2, stochastic since 15882e98 2026-04-23):
//
//     evap_prob = exp(-E_7site / K_MANIFEST²) · K_EVAP_RATE
//     evaporate iff voxel_uniform(seed, i, tick, VoxelRng::Evaporation) < evap_prob
//
// Pre-fix, the GPU evaporation_kernel kept the pre-2026-04-23 deterministic
// threshold (E_7site < K_MANIFEST²·1e-6): a settled particle NEVER evaporated
// on GPU (isolated-particle lifetime ~8 ticks CPU vs infinite GPU — every
// lifetime/persistence measurement was backend-dependent).
//
// The draw is shared SplitMix64 (engine/include/ftd/voxel_rng.h), bit-exact
// across backends at identical (seed, voxel, tick), so the death TICK — not
// just the death rate — must agree CPU↔GPU.
//
// EP-1  zero-field particle: death tick identical CPU↔GPU, and > 1 (the
//       retired deterministic rule fires at tick 1 when E ≈ 0 — equality at
//       a later tick discriminates stochastic from threshold semantics).
// EP-2  locked exemption: a locked particle survives on both backends.
// EP-3  Boltzmann suppression: a high-field particle (E ≫ K_MANIFEST²)
//       survives the horizon on both backends (p_evap ≈ 2e-8/tick).
//
// CPU-only builds: the GPU bridge silently runs the CPU backend; the test
// detects that and SKIPs (the comparison would be CPU-vs-CPU, vacuous).
// ============================================================================

#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/test_telemetry.h"

#include <cstdio>

namespace ftd { namespace test {

namespace {

struct LifeResult {
    int  death_tick;   // -1 = survived the horizon
    bool ran_gpu;
};

// Single particle at the lattice center; all field dynamics off so E_7site
// stays at its injected value; genesis gates the evaporation pass.
LifeResult run_lifetime(bool force_cpu, const Vec3& flux, bool locked,
                        int horizon) {
    RenderBridge rb(9);
    if (force_cpu) rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.genesis     = true;   // (do_genesis || do_evaporation) gate
    rb.toggles.evaporation = true;
    rb.inject_particle(4, 4, 4, +1, flux);

    const int idx = rb.lattice().index(4, 4, 4);
    if (locked) {
        rb.voxels()[idx].locked = true;
        rb.backend().mark_host_dirty();   // re-upload before the next GPU tick
    }

    LifeResult r{-1, rb.backend().kind() == Backend::Kind::Gpu};
    for (int t = 1; t <= horizon; ++t) {
        rb.tick();
        rb.sync_from_gpu();
        if (rb.voxels()[idx].state == 0) { r.death_tick = t; break; }
    }
    return r;
}

}  // namespace

void test_gpu_evaporation_parity() {
    section("EP-0: backend availability");
    LifeResult gpu_probe = run_lifetime(/*force_cpu=*/false, Vec3{0, 0, 0},
                                        /*locked=*/false, /*horizon=*/1);
    if (!gpu_probe.ran_gpu) {
        std::printf("[evap-parity] SKIP: no GPU backend in this build — "
                    "CPU-vs-CPU comparison is vacuous.\n");
        check("evaporation parity skipped on CPU-only build", true, "");
        return;
    }

    section("EP-1: zero-field death tick agrees CPU<->GPU (stochastic rule)");
    LifeResult g1 = run_lifetime(false, Vec3{0, 0, 0}, false, 200);
    LifeResult c1 = run_lifetime(true,  Vec3{0, 0, 0}, false, 200);
    std::printf("[evap-parity] EP-1 death tick: GPU=%d CPU=%d\n",
                g1.death_tick, c1.death_tick);
    check("EP-1a: particle evaporates on GPU", g1.death_tick > 0,
          "GPU particle never evaporated — deterministic-threshold fossil "
          "behavior (pre-BH-F5-completion) has returned.");
    check("EP-1b: death tick identical CPU<->GPU", g1.death_tick == c1.death_tick,
          "Shared SplitMix64 Evaporation draw diverged between backends "
          "(seed/index/tick misalignment or rule mismatch).");
    check("EP-1c: death is stochastic, not the retired tick-1 threshold",
          g1.death_tick > 1,
          "Death at tick 1 with E=0 matches the retired deterministic "
          "threshold semantics, not the Boltzmann draw.");

    section("EP-2: locked voxels exempt on both backends");
    LifeResult g2 = run_lifetime(false, Vec3{0, 0, 0}, true, 200);
    LifeResult c2 = run_lifetime(true,  Vec3{0, 0, 0}, true, 200);
    std::printf("[evap-parity] EP-2 death tick: GPU=%d CPU=%d (-1 = survived)\n",
                g2.death_tick, c2.death_tick);
    check("EP-2a: locked particle survives on GPU", g2.death_tick == -1,
          "GPU evaporated a locked voxel — locked exemption lost.");
    check("EP-2b: locked particle survives on CPU", c2.death_tick == -1,
          "CPU evaporated a locked voxel — locked exemption lost.");

    section("EP-3: Boltzmann suppression at high field energy");
    // |J|² = 4 ≫ K_MANIFEST² ≈ 0.261 ⇒ p_evap ≈ 0.1·exp(-15.3) ≈ 2e-8/tick.
    LifeResult g3 = run_lifetime(false, Vec3{2.0, 0.0, 0.0}, false, 200);
    LifeResult c3 = run_lifetime(true,  Vec3{2.0, 0.0, 0.0}, false, 200);
    std::printf("[evap-parity] EP-3 death tick: GPU=%d CPU=%d (-1 = survived)\n",
                g3.death_tick, c3.death_tick);
    check("EP-3a: high-energy particle survives on GPU", g3.death_tick == -1,
          "GPU killed a Boltzmann-suppressed particle (p ~ 2e-8/tick).");
    check("EP-3b: high-energy particle survives on CPU", c3.death_tick == -1,
          "CPU killed a Boltzmann-suppressed particle (p ~ 2e-8/tick).");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_gpu_evaporation_parity");
    ftd::test::test_gpu_evaporation_parity();
    return ftd::test::finalize();
}
