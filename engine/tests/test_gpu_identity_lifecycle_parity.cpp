/**
 * Focused CPU/CUDA parity for sparse lifecycle transactions and identity
 * allocation.  These are release gates for native Scale-0 scenarios because
 * CUDA scheduling must not change pair matching, labels, or provenance.
 */

#include "ftd/constants.h"
#include "ftd/gpu_buffers.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const char* name, bool condition) {
    std::printf("  %s  %s\n", condition ? "PASS" : "FAIL", name);
    condition ? ++passed : ++failed;
}

int index_of(int L, int x, int y, int z) {
    return x * L * L + y * L + z;
}

double vec_error(const Vec3& a, const Vec3& b) {
    return std::max({std::abs(a.x - b.x), std::abs(a.y - b.y),
                     std::abs(a.z - b.z)});
}

double field_error(const Voxel& a, const Voxel& b) {
    return std::max({
        vec_error(a.flux, b.flux), vec_error(a.wave_vel, b.wave_vel),
        vec_error(a.flux_L, b.flux_L), vec_error(a.flux_R, b.flux_R),
        vec_error(a.wave_vel_L, b.wave_vel_L),
        vec_error(a.wave_vel_R, b.wave_vel_R),
    });
}

struct Snapshot {
    std::vector<Voxel> voxels;
    ftd::eft::DualCellContinuity continuity;
    bool continuity_valid = true;
    std::int32_t next_particle_id = 0;
    std::int32_t next_pair_id = 0;
};

std::vector<int> state_snapshot(const std::vector<Voxel>& voxels) {
    std::vector<int> states(voxels.size(), 0);
    for (std::size_t i = 0; i < voxels.size(); ++i)
        states[i] = static_cast<int>(voxels[i].state);
    return states;
}

void seed_host_identity_highwater(RenderBridge& bridge,
                                  const std::vector<Voxel>& seed) {
    int max_particle_id = -1;
    int max_pair_id = -1;
    for (const auto& voxel : seed) {
        max_particle_id = std::max(max_particle_id, voxel.particle_id);
        max_pair_id = std::max(max_pair_id, voxel.pair_id);
    }
    bridge.injector().raise_identity_counters(max_particle_id + 1,
                                              max_pair_id + 1);
}

Snapshot run_cpu(int L, const std::vector<Voxel>& seed,
                 const TermToggles& toggles,
                 double manifest_scale_override = -1.0) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles = toggles;
    bridge.manifest_scale_override = manifest_scale_override;
    bridge.voxels() = seed;
    seed_host_identity_highwater(bridge, seed);
    const auto states_before = state_snapshot(seed);
    bridge.tick();

    Snapshot out;
    out.voxels = static_cast<const RenderBridge&>(bridge).voxels();
    const auto states_after = state_snapshot(out.voxels);
    const auto extraction = ftd::eft::extract_moore_history_from_snapshots(
        L, states_before, states_after, out.continuity);
    out.continuity_valid = extraction.valid;
    out.next_particle_id = bridge.injector().peek_next_particle_id();
    out.next_pair_id = bridge.injector().peek_next_pair_id();
    return out;
}

Snapshot run_gpu(int L, const std::vector<Voxel>& seed,
                 const TermToggles& toggles,
                 double manifest_scale_override = -1.0) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles = toggles;
    engine.manifest_scale_override = manifest_scale_override;
    engine.upload_from_host(seed);
    engine.tick();

    Snapshot out;
    engine.sync_to_host(out.voxels);
    out.continuity = engine.continuity_step();
    engine.identity_counters(out.next_particle_id, out.next_pair_id);
    return out;
}

bool same_metadata(const Snapshot& a, const Snapshot& b) {
    if (a.voxels.size() != b.voxels.size()) return false;
    for (std::size_t i = 0; i < a.voxels.size(); ++i) {
        const auto& x = a.voxels[i];
        const auto& y = b.voxels[i];
        if (x.state != y.state || x.particle_id != y.particle_id
            || x.pair_id != y.pair_id || x.spin != y.spin
            || x.color != y.color || x.locked != y.locked) {
            return false;
        }
    }
    return a.next_particle_id == b.next_particle_id
        && a.next_pair_id == b.next_pair_id;
}

double max_field_error(const Snapshot& a, const Snapshot& b) {
    double error = 0.0;
    for (std::size_t i = 0; i < a.voxels.size(); ++i)
        error = std::max(error, field_error(a.voxels[i], b.voxels[i]));
    return error;
}

bool same_reaction(const Snapshot& a, const Snapshot& b) {
    return a.continuity_valid && b.continuity_valid
        && a.continuity.reaction == b.continuity.reaction;
}

std::vector<Voxel> pair_seed(int L, bool dual) {
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    const int source_a = index_of(L, 2, 3, 3);
    const int target = index_of(L, 3, 3, 3);
    const int source_b = index_of(L, 4, 3, 3);
    const int highwater = index_of(L, 7, 7, 7);
    const double amplitude = ftd::K_GENESIS + 1000.0;

    seed[highwater].state = +1;
    seed[highwater].locked = true;
    seed[highwater].particle_id = 41;
    seed[highwater].pair_id = 17;

    seed[source_a].flux = {amplitude, 0.0, 0.0};
    seed[source_a].wave_vel = {2.0, 4.0, 6.0};
    seed[target].wave_vel = {8.0, 10.0, 12.0};
    seed[source_b].flux = {-amplitude, 0.0, 0.0};
    if (dual) {
        seed[source_a].flux_L = seed[source_a].flux * 0.7;
        seed[source_a].flux_R = seed[source_a].flux * 0.3;
        seed[source_b].flux_L = seed[source_b].flux * 0.6;
        seed[source_b].flux_R = seed[source_b].flux * 0.4;
        seed[source_a].wave_vel_L = seed[source_a].wave_vel * 0.25;
        seed[source_a].wave_vel_R = seed[source_a].wave_vel * 0.75;
        seed[target].wave_vel_L = seed[target].wave_vel * 0.4;
        seed[target].wave_vel_R = seed[target].wave_vel * 0.6;
    }
    return seed;
}

void test_pair_transaction(bool dual) {
    std::printf("\nGID-1%s: deterministic pair transaction parity\n",
                dual ? "D" : "S");
    constexpr int L = 8;
    TermToggles toggles;
    toggles.disable_all();
    toggles.dual_substrate = dual;
    toggles.pair_production = true;
    const auto seed = pair_seed(L, dual);

    const Snapshot cpu = run_cpu(L, seed, toggles);
    const Snapshot gpu_a = run_gpu(L, seed, toggles);
    const Snapshot gpu_b = run_gpu(L, seed, toggles);
    check("CPU/GPU pair metadata and identity counters are exact",
          same_metadata(cpu, gpu_a));
    check("pair transaction fields match CPU",
          max_field_error(cpu, gpu_a) < 1e-10);
    check("pair transaction is repeatable across GPU launches",
          same_metadata(gpu_a, gpu_b)
          && max_field_error(gpu_a, gpu_b) == 0.0);
    check("pair reaction ledger matches CPU", same_reaction(cpu, gpu_a));

    const int source_a = index_of(L, 2, 3, 3);
    const int target = index_of(L, 3, 3, 3);
    const int source_b = index_of(L, 4, 3, 3);
    const auto& a = gpu_a.voxels[source_a];
    const auto& t = gpu_a.voxels[target];
    const auto& b = gpu_a.voxels[source_b];
    check("ascending greedy conflict selects only the first source",
          a.state == -1 && t.state == +1 && b.state == 0);
    check("stable device allocator emits canonical IDs",
          a.particle_id == 42 && t.particle_id == 43
          && a.pair_id == 18 && t.pair_id == 18
          && gpu_a.next_particle_id == 44 && gpu_a.next_pair_id == 19);
    check("pair ledger has exact balanced source terms",
          gpu_a.continuity.reaction[source_a] == -1
          && gpu_a.continuity.reaction[target] == +1
          && gpu_a.continuity.reaction[source_b] == 0);
    if (dual) {
        check("dual pair registers preserve both observable sums",
              vec_error(a.flux, a.flux_L + a.flux_R) < 1e-12
              && vec_error(t.flux, t.flux_L + t.flux_R) < 1e-12
              && vec_error(a.wave_vel,
                           a.wave_vel_L + a.wave_vel_R) < 1e-12
              && vec_error(t.wave_vel,
                           t.wave_vel_L + t.wave_vel_R) < 1e-12);
    }
}

void test_same_tick_genesis_evaporation_order() {
    std::printf("\nGID-2: same-tick genesis/evaporation live-neighbour parity\n");
    constexpr int L = 8;
    const int genesis = index_of(L, 2, 2, 2);
    const int neighbour = index_of(L, 3, 2, 2);
    std::vector<Voxel> seed(static_cast<std::size_t>(L * L * L));
    seed[genesis].flux = {ftd::K_GENESIS + 0.05, 0.0, 0.0};
    seed[genesis].pair_id = 800;  // non-pair genesis must clear this
    seed[neighbour].state = +1;
    seed[neighbour].particle_id = 31;
    seed[neighbour].pair_id = 9;

    TermToggles toggles;
    toggles.disable_all();
    toggles.genesis = true;  // implies the canonical sister evaporation pass
    constexpr double tiny_manifest_scale = 1e-12;  // accepted genesis, no RNG search
    const Snapshot cpu = run_cpu(L, seed, toggles, tiny_manifest_scale);
    const Snapshot gpu = run_gpu(L, seed, toggles, tiny_manifest_scale);

    check("ordered sparse lifecycle metadata matches CPU exactly",
          same_metadata(cpu, gpu));
    check("ordered sparse lifecycle fields match CPU",
          max_field_error(cpu, gpu) < 1e-12);
    check("ordered sparse lifecycle reaction ledger matches CPU",
          same_reaction(cpu, gpu));
    check("accepted genesis applied the canonical threshold drain",
          std::abs(gpu.voxels[genesis].flux.x - 0.05) < 1e-12);
    check("genesis never inherits stale pair provenance",
          gpu.voxels[genesis].pair_id == -1);
    check("later neighbour consumes the same live post-drain energy decision",
          gpu.voxels[neighbour].state == cpu.voxels[neighbour].state
          && gpu.voxels[neighbour].particle_id
                 == cpu.voxels[neighbour].particle_id
          && gpu.voxels[neighbour].pair_id == cpu.voxels[neighbour].pair_id);
}

void test_public_force_cpu_preserves_retired_highwater_and_state() {
    std::printf("\nGID-3: public GPU-to-CPU fallback synchronization\n");
    constexpr int L = 6;
    RenderBridge bridge(L);
    check("test starts on the CUDA backend",
          bridge.backend_kind() == ftd::Backend::Kind::Gpu);
    bridge.set_interactive_gpu_mode(true);
    bridge.toggles.disable_all();
    bridge.toggles.evaporation = true;

    const int primary = index_of(L, 2, 2, 2);
    const int partner = index_of(L, 3, 2, 2);  // first periodic face neighbour
    const int marker = index_of(L, 5, 5, 5);
    bridge.inject_flux(5, 5, 5, {1.25, -0.5, 0.75});
    bridge.create_entangled_pair(2, 2, 2, {});

    std::vector<std::int8_t> states;
    int ticks = 0;
    do {
        bridge.tick();
        bridge.copy_visual_states(states);
        ++ticks;
    } while ((states[primary] != 0 || states[partner] != 0) && ticks < 1024);
    check("all device-born pair members retired before fallback",
          states[primary] == 0 && states[partner] == 0);

    // No canonical voxel download has occurred in interactive mode.  The
    // public fallback itself must now synchronize both state and allocator
    // high-water marks before replacing the backend.
    bridge.force_cpu();
    check("force_cpu switched to the CPU backend",
          bridge.backend_kind() == ftd::Backend::Kind::Cpu);
    const auto& synced = static_cast<const RenderBridge&>(bridge).voxels();
    check("retired device provenance is cleared on the synchronized host image",
          synced[primary].state == 0 && synced[partner].state == 0
          && synced[primary].particle_id == -1
          && synced[partner].particle_id == -1
          && synced[primary].pair_id == -1 && synced[partner].pair_id == -1);
    check("unrelated device field state survives public force_cpu",
          vec_error(synced[marker].flux, {1.25, -0.5, 0.75}) < 1e-12);

    bridge.toggles.disable_all();
    bridge.inject_particle(0, 0, 0, +1, {});
    bridge.create_entangled_pair(4, 4, 4, {});
    const auto& after = static_cast<const RenderBridge&>(bridge).voxels();
    check("CPU fallback does not reuse retired device particle IDs",
          after[index_of(L, 0, 0, 0)].particle_id == 2
          && after[index_of(L, 4, 4, 4)].particle_id == 3);
    check("CPU fallback does not reuse retired device pair IDs",
          after[index_of(L, 4, 4, 4)].pair_id == 1);
}

void test_public_force_cpu_preserves_host_staged_highwater() {
    std::printf("\nGID-4: public fallback flushes host-staged identities\n");
    constexpr int L = 6;
    RenderBridge bridge(L);
    check("host-staged fallback test starts on CUDA",
          bridge.backend_kind() == ftd::Backend::Kind::Gpu);
    bridge.set_interactive_gpu_mode(true);
    bridge.toggles.disable_all();

    // Deliberately bypass Injector to model a caller/test restoring an
    // externally labelled host snapshot through the public mutable API.  The
    // pending host image is newer than the device, while both allocators still
    // begin at zero until force_cpu() performs its synchronization boundary.
    const int staged_index = index_of(L, 2, 2, 2);
    auto& staged = bridge.voxels()[staged_index];
    staged.state = +1;
    staged.particle_id = 41;
    staged.pair_id = 17;
    staged.locked = true;
    check("explicit host identity edit did not implicitly consume Injector IDs",
          bridge.injector().peek_next_particle_id() == 0
          && bridge.injector().peek_next_pair_id() == 0);

    bridge.force_cpu();
    check("host-staged fallback switched to CPU",
          bridge.backend_kind() == ftd::Backend::Kind::Cpu);
    const auto& synchronized =
        static_cast<const RenderBridge&>(bridge).voxels()[staged_index];
    check("host-staged identity survives the fallback boundary",
          synchronized.state == +1 && synchronized.particle_id == 41
          && synchronized.pair_id == 17);

    bridge.inject_particle(0, 0, 0, +1, {});
    bridge.create_entangled_pair(4, 4, 4, {});
    const auto& after = static_cast<const RenderBridge&>(bridge).voxels();
    check("CPU allocation does not reuse host-staged particle IDs",
          after[index_of(L, 0, 0, 0)].particle_id == 42
          && after[index_of(L, 4, 4, 4)].particle_id == 43);
    check("CPU allocation does not reuse host-staged pair IDs",
          after[index_of(L, 4, 4, 4)].pair_id == 18);
}

void test_clean_voxel_reads_do_not_repoll_identity_counters() {
    std::printf("\nGID-5: clean host reads avoid identity-counter D2H polls\n");
    constexpr int L = 6;
    RenderBridge bridge(L);
    check("identity-poll test starts on CUDA",
          bridge.backend_kind() == ftd::Backend::Kind::Gpu);
    bridge.toggles.disable_all();
    bridge.inject_particle(2, 2, 2, +1, {});

    ftd::gpu::g_gpu_identity_counter_download_bytes = 0;
    ftd::gpu::g_gpu_identity_counter_download_calls = 0;
    const auto& first = static_cast<const RenderBridge&>(bridge).voxels();
    check("first dirty read reconciles identity counters once",
          first[index_of(L, 2, 2, 2)].particle_id == 0
          && ftd::gpu::g_gpu_identity_counter_download_calls == 1
          && ftd::gpu::g_gpu_identity_counter_download_bytes
                 == 2 * sizeof(std::int32_t));

    ftd::gpu::g_gpu_identity_counter_download_bytes = 0;
    ftd::gpu::g_gpu_identity_counter_download_calls = 0;
    int observed_id_sum = 0;
    for (int repeat = 0; repeat < 128; ++repeat) {
        const auto& clean = static_cast<const RenderBridge&>(bridge).voxels();
        observed_id_sum += clean[index_of(L, 2, 2, 2)].particle_id;
    }
    check("repeated clean voxels reads issue no compact identity D2H",
          observed_id_sum == 0
          && ftd::gpu::g_gpu_identity_counter_download_calls == 0
          && ftd::gpu::g_gpu_identity_counter_download_bytes == 0);
}

}  // namespace

int main() {
    std::printf("GPU identity/lifecycle parity regression\n");
    test_pair_transaction(false);
    test_pair_transaction(true);
    test_same_tick_genesis_evaporation_order();
    test_public_force_cpu_preserves_retired_highwater_and_state();
    test_public_force_cpu_preserves_host_staged_highwater();
    test_clean_voxel_reads_do_not_repoll_identity_counters();
    std::printf("\nResult: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
