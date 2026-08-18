/**
 * CPU identity/provenance regression gates.
 *
 * Particle and pair identities are lifetime-monotonic namespaces.  A dead
 * particle must not make its identity reusable, while every non-pair birth
 * and every lifetime termination must clear stale pair provenance.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace {

int passed = 0;
int failed = 0;

void check(const char* name, bool condition) {
    std::printf("  %s  %s\n", condition ? "PASS" : "FAIL", name);
    condition ? ++passed : ++failed;
}

bool close(double a, double b, double tolerance = 1e-12) {
    return std::abs(a - b) <= tolerance;
}

std::vector<int> state_snapshot(const std::vector<ftd::Voxel>& voxels) {
    std::vector<int> states(voxels.size(), 0);
    for (std::size_t i = 0; i < voxels.size(); ++i)
        states[i] = static_cast<int>(voxels[i].state);
    return states;
}

void retire(ftd::RenderBridge& bridge, int index) {
    bridge.set_state(index, 0);
    auto& voxel = bridge.voxels()[static_cast<std::size_t>(index)];
    voxel.particle_id = -1;
    voxel.pair_id = -1;
    voxel.spin = 0;
    voxel.color = 0;
}

void test_pair_production_uses_independent_monotonic_ids() {
    std::printf("\nID-1: pair production identity and transaction contract\n");
    constexpr int L = 8;
    ftd::RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();

    bridge.create_entangled_pair(0, 0, 0, {});
    const int explicit_primary = bridge.lattice().index(0, 0, 0);
    const int explicit_partner = bridge.lattice().neighbors_6(explicit_primary)[0];
    const auto before = static_cast<const ftd::RenderBridge&>(bridge).voxels();
    const int retired_pair_id = before[explicit_primary].pair_id;
    const int retired_max_pid = std::max(before[explicit_primary].particle_id,
                                         before[explicit_partner].particle_id);
    retire(bridge, explicit_primary);
    retire(bridge, explicit_partner);

    const int source = bridge.lattice().index(3, 3, 3);
    const int partner = bridge.lattice().index(4, 3, 3);
    const double amplitude = ftd::K_GENESIS + 1000.0;
    auto& voxels = bridge.voxels();
    voxels[source].flux = {amplitude, 0.0, 0.0};
    voxels[source].wave_vel = {2.0, 4.0, 6.0};
    voxels[source].pair_id = 700;  // stale void provenance must be overwritten
    voxels[partner].wave_vel = {8.0, 10.0, 12.0};
    voxels[partner].pair_id = 701;

    const auto states_before = state_snapshot(
        static_cast<const ftd::RenderBridge&>(bridge).voxels());

    bridge.toggles.pair_production = true;
    bridge.tick();

    const auto& after = static_cast<const ftd::RenderBridge&>(bridge).voxels();
    const auto& upstream = after[source];
    const auto& downstream = after[partner];
    check("upstream/downstream state signs are -/+",
          upstream.state == -1 && downstream.state == +1);
    check("pair production consumes two new particle IDs",
          upstream.particle_id == retired_max_pid + 1
          && downstream.particle_id == retired_max_pid + 2);
    check("pair production consumes a dedicated new pair ID",
          upstream.pair_id == retired_pair_id + 1
          && downstream.pair_id == upstream.pair_id);
    check("source wave velocity is halved",
          close(upstream.wave_vel.x, 1.0)
          && close(upstream.wave_vel.y, 2.0)
          && close(upstream.wave_vel.z, 3.0));
    check("partner wave velocity is halved",
          close(downstream.wave_vel.x, 4.0)
          && close(downstream.wave_vel.y, 5.0)
          && close(downstream.wave_vel.z, 6.0));

    // phase_write precedes pair production and performs the canonical drift
    // even when phase_read is disabled.  The pair drain therefore acts on the
    // post-write field, not on the host-staged seed field.
    const ftd::Vec3 pre_pair_flux =
        ftd::Vec3{amplitude, 0.0, 0.0} + ftd::Vec3{2.0, 4.0, 6.0};
    const double drain = 1.0 - ftd::K_GENESIS / pre_pair_flux.mag();
    const ftd::Vec3 expected_source_flux = pre_pair_flux * drain;
    check("flux drain and opposite partner flux match the CPU contract",
          (upstream.flux - expected_source_flux).mag() < 1e-10
          && (downstream.flux + upstream.flux).mag() < 1e-10);

    const auto states_after = state_snapshot(after);
    ftd::eft::DualCellContinuity continuity;
    const auto extraction = ftd::eft::extract_moore_history_from_snapshots(
        L, states_before, states_after, continuity);
    check("pair birth is recorded as a balanced reaction pair",
          extraction.valid && continuity.reaction[source] == -1
          && continuity.reaction[partner] == +1);
}

void test_pair_provenance_lifetime_cleanup() {
    std::printf("\nID-2: non-pair birth and evaporation provenance cleanup\n");
    constexpr int L = 6;
    ftd::RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();

    const int site = bridge.lattice().index(2, 2, 2);
    bridge.voxels()[site].pair_id = 9123;
    bridge.inject_particle(2, 2, 2, +1, {});
    check("ordinary particle injection clears stale pair provenance",
          static_cast<const ftd::RenderBridge&>(bridge).voxels()[site].pair_id == -1);

    bridge.voxels()[site].pair_id = 37;
    bridge.toggles.evaporation = true;
    int ticks = 0;
    while (bridge.state_at(site) != 0 && ticks++ < 1024) bridge.tick();
    const auto& dead = static_cast<const ftd::RenderBridge&>(bridge).voxels()[site];
    check("zero-energy particle evaporates in bounded deterministic run",
          dead.state == 0 && ticks <= 1024);
    check("evaporation clears particle and pair identities",
          dead.particle_id == -1 && dead.pair_id == -1);
}

void test_retired_ids_are_never_reused() {
    std::printf("\nID-3: retired CPU identities remain reserved\n");
    constexpr int L = 6;
    ftd::RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.create_entangled_pair(2, 2, 2, {});

    const int primary = bridge.lattice().index(2, 2, 2);
    const int partner = bridge.lattice().neighbors_6(primary)[0];
    bridge.toggles.evaporation = true;
    int ticks = 0;
    while ((bridge.state_at(primary) != 0 || bridge.state_at(partner) != 0)
           && ticks++ < 1024) {
        bridge.tick();
    }
    check("both members of the first pair evaporated", ticks <= 1024);

    bridge.toggles.disable_all();
    bridge.inject_particle(0, 0, 0, +1, {});
    const int new_particle = bridge.lattice().index(0, 0, 0);
    bridge.create_entangled_pair(4, 4, 4, {});
    const int new_pair = bridge.lattice().index(4, 4, 4);
    const auto& voxels = static_cast<const ftd::RenderBridge&>(bridge).voxels();
    check("particle ID high-water survives complete retirement",
          voxels[new_particle].particle_id == 2);
    check("pair ID high-water survives complete retirement",
          voxels[new_pair].pair_id == 1);
}

void test_dual_pair_register_consistency() {
    std::printf("\nID-4: dual pair production keeps observable registers exact\n");
    constexpr int L = 8;
    ftd::RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.dual_substrate = true;
    bridge.toggles.pair_production = true;

    const int source = bridge.lattice().index(3, 3, 3);
    const int partner = bridge.lattice().index(4, 3, 3);
    const double amplitude = ftd::K_GENESIS + 1000.0;
    auto& voxels = bridge.voxels();
    voxels[source].flux_L = {0.7 * amplitude, 0.0, 0.0};
    voxels[source].flux_R = {0.3 * amplitude, 0.0, 0.0};
    voxels[source].flux = voxels[source].flux_L + voxels[source].flux_R;
    voxels[source].wave_vel_L = {2.0, 0.0, 0.0};
    voxels[source].wave_vel_R = {4.0, 0.0, 0.0};
    voxels[source].wave_vel =
        voxels[source].wave_vel_L + voxels[source].wave_vel_R;
    voxels[partner].wave_vel_L = {6.0, 0.0, 0.0};
    voxels[partner].wave_vel_R = {8.0, 0.0, 0.0};
    voxels[partner].wave_vel =
        voxels[partner].wave_vel_L + voxels[partner].wave_vel_R;

    bridge.tick();
    const auto& after = static_cast<const ftd::RenderBridge&>(bridge).voxels();
    const auto& a = after[source];
    const auto& b = after[partner];
    check("dual source observable equals L+R after drain",
          (a.flux - (a.flux_L + a.flux_R)).mag() < 1e-12
          && (a.wave_vel - (a.wave_vel_L + a.wave_vel_R)).mag() < 1e-12);
    check("dual partner observable equals L+R and opposite source flux",
          (b.flux - (b.flux_L + b.flux_R)).mag() < 1e-12
          && (b.wave_vel - (b.wave_vel_L + b.wave_vel_R)).mag() < 1e-12
          && (b.flux + a.flux).mag() < 1e-12);
}

}  // namespace

int main() {
    std::printf("CPU identity/lifecycle regression\n");
    test_pair_production_uses_independent_monotonic_ids();
    test_pair_provenance_lifetime_cleanup();
    test_retired_ids_are_never_reused();
    test_dual_pair_register_consistency();
    std::printf("\nResult: %d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
