/**
 * Exact greedy CUDA movement transaction regression.
 *
 * CPU phase_movement is an ascending X-major live-state transaction. These
 * cases pin the CUDA implementation to that ordering at the places where the
 * former thread-per-site/CAS kernel was schedule-dependent: competing moves,
 * arrival reprocessing, metadata transport, annihilation cleanup/scatter,
 * dual-register transport, and repeatability.
 */

#include "ftd/constants.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

namespace {

using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const std::string& name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name.c_str());
    ok ? ++passed : ++failed;
}

int index_of(int L, int x, int y, int z) {
    return x * L * L + y * L + z;
}

bool close(double a, double b, double tol = 2e-13) {
    return std::abs(a - b) <= tol;
}

bool close(const Vec3& a, const Vec3& b, double tol = 2e-13) {
    return close(a.x, b.x, tol) && close(a.y, b.y, tol)
        && close(a.z, b.z, tol);
}

bool same_movement_voxel(const Voxel& a, const Voxel& b,
                         double tol = 2e-13) {
    return a.state == b.state
        && close(a.velocity, b.velocity, tol)
        && close(a.remainder, b.remainder, tol)
        && close(a.flux, b.flux, tol)
        && close(a.wave_vel, b.wave_vel, tol)
        && close(a.flux_L, b.flux_L, tol)
        && close(a.flux_R, b.flux_R, tol)
        && close(a.wave_vel_L, b.wave_vel_L, tol)
        && close(a.wave_vel_R, b.wave_vel_R, tol)
        && close(a.flux_strong, b.flux_strong, tol)
        && close(a.wave_vel_strong, b.wave_vel_strong, tol)
        && close(a.flux_weak, b.flux_weak, tol)
        && close(a.wave_vel_weak, b.wave_vel_weak, tol)
        && a.locked == b.locked
        && a.particle_id == b.particle_id
        && a.pair_id == b.pair_id
        && a.spin == b.spin
        && a.color == b.color
        && close(a.accel_mag, b.accel_mag, tol);
}

bool same_movement_image(const std::vector<Voxel>& a,
                         const std::vector<Voxel>& b,
                         double tol = 2e-13) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (!same_movement_voxel(a[i], b[i], tol)) return false;
    }
    return true;
}

bool exactly_same_movement_image(const std::vector<Voxel>& a,
                                 const std::vector<Voxel>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const auto& x = a[i];
        const auto& y = b[i];
        if (x.state != y.state || x.locked != y.locked
            || x.particle_id != y.particle_id || x.pair_id != y.pair_id
            || x.spin != y.spin || x.color != y.color
            || x.accel_mag != y.accel_mag
            || x.velocity.x != y.velocity.x
            || x.velocity.y != y.velocity.y
            || x.velocity.z != y.velocity.z
            || x.remainder.x != y.remainder.x
            || x.remainder.y != y.remainder.y
            || x.remainder.z != y.remainder.z
            || x.flux.x != y.flux.x || x.flux.y != y.flux.y
            || x.flux.z != y.flux.z
            || x.flux_L.x != y.flux_L.x || x.flux_L.y != y.flux_L.y
            || x.flux_L.z != y.flux_L.z
             || x.flux_R.x != y.flux_R.x || x.flux_R.y != y.flux_R.y
             || x.flux_R.z != y.flux_R.z
             || x.flux_strong.x != y.flux_strong.x
             || x.flux_strong.y != y.flux_strong.y
             || x.flux_strong.z != y.flux_strong.z
             || x.wave_vel_strong.x != y.wave_vel_strong.x
             || x.wave_vel_strong.y != y.wave_vel_strong.y
             || x.wave_vel_strong.z != y.wave_vel_strong.z
             || x.flux_weak.x != y.flux_weak.x
             || x.flux_weak.y != y.flux_weak.y
             || x.flux_weak.z != y.flux_weak.z
             || x.wave_vel_weak.x != y.wave_vel_weak.x
             || x.wave_vel_weak.y != y.wave_vel_weak.y
             || x.wave_vel_weak.z != y.wave_vel_weak.z) return false;
    }
    return true;
}

struct Snapshot {
    std::vector<Voxel> voxels;
    ftd::eft::DualCellContinuity continuity;
    unsigned long long projection_events = 0;
};

Snapshot run_cpu(int L, const std::vector<Voxel>& seed,
                  bool dual = false, bool reflective = false,
                  ftd::FluxBoundaryMode mode = ftd::FluxBoundaryMode::Dispersal,
                  ftd::PeriodicAxis axis = ftd::PeriodicAxis::All,
                  bool wave = false) {
    std::vector<int> before(seed.size(), 0);
    for (std::size_t i = 0; i < seed.size(); ++i)
        before[i] = static_cast<int>(seed[i].state);
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.movement = true;
    bridge.toggles.wave_propagation = wave;
    bridge.toggles.dual_substrate = dual;
    bridge.toggles.reflective_boundary = reflective;
    bridge.toggles.flux_boundary = mode;
    bridge.toggles.periodic_axis = axis;
    bridge.voxels() = seed;
    bridge.tick();
    Snapshot out;
    out.voxels = static_cast<const RenderBridge&>(bridge).voxels();
    std::vector<int> after(out.voxels.size(), 0);
    for (std::size_t i = 0; i < out.voxels.size(); ++i)
        after[i] = static_cast<int>(out.voxels[i].state);
    ftd::eft::extract_moore_history_from_snapshots(
        L, before, after, out.continuity);
    out.projection_events = static_cast<unsigned long long>(
        bridge.causal_projection_events_this_tick());
    return out;
}

Snapshot run_gpu(int L, const std::vector<Voxel>& seed,
                  bool dual = false, bool reflective = false,
                  ftd::FluxBoundaryMode mode = ftd::FluxBoundaryMode::Dispersal,
                  ftd::PeriodicAxis axis = ftd::PeriodicAxis::All,
                  bool wave = false) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    engine.toggles.movement = true;
    engine.toggles.wave_propagation = wave;
    engine.toggles.dual_substrate = dual;
    engine.toggles.reflective_boundary = reflective;
    engine.toggles.flux_boundary = mode;
    engine.toggles.periodic_axis = axis;
    engine.upload_from_host(seed);
    engine.tick();
    Snapshot out;
    engine.sync_to_host(out.voxels);
    out.continuity = engine.continuity_step();
    out.projection_events = engine.causal_projection_events();
    return out;
}

bool same_ledger(const ftd::eft::DualCellContinuity& a,
                 const ftd::eft::DualCellContinuity& b,
                 double tol = 1e-13) {
    if (a.L != b.L || a.rho_before != b.rho_before
        || a.rho_after != b.rho_after || a.reaction != b.reaction
        || a.current_x.size() != b.current_x.size()
        || a.current_y.size() != b.current_y.size()
        || a.current_z.size() != b.current_z.size()) return false;
    for (std::size_t i = 0; i < a.current_x.size(); ++i) {
        if (!close(a.current_x[i], b.current_x[i], tol)
            || !close(a.current_y[i], b.current_y[i], tol)
            || !close(a.current_z[i], b.current_z[i], tol)) return false;
    }
    return true;
}

void seed_particle(Voxel& v, int8_t state, int pid, int pair,
                   const Vec3& velocity, const Vec3& remainder) {
    v.state = state;
    v.particle_id = pid;
    v.pair_id = pair;
    v.velocity = velocity;
    v.remainder = remainder;
}

void test_same_target_contention() {
    std::printf("\nGMT-1: ascending same-target contention\n");
    constexpr int L = 8;
    std::vector<Voxel> seed(L * L * L);
    const int lower = index_of(L, 2, 2, 1);
    const int target = index_of(L, 2, 2, 2);
    const int upper = index_of(L, 2, 2, 3);
    seed_particle(seed[lower], +1, 101, 11, {0, 0, 0.30}, {0, 0, 0.80});
    seed_particle(seed[upper], +1, 202, 22, {0, 0, -0.30}, {0, 0, -0.80});

    const Snapshot cpu = run_cpu(L, seed);
    const Snapshot gpu = run_gpu(L, seed);
    check("same-target full CPU/GPU movement parity",
          same_movement_image(cpu.voxels, gpu.voxels));
    check("lower X-major source wins the void target",
          gpu.voxels[target].particle_id == 101
          && gpu.voxels[lower].state == 0);
    check("later same-sign contender observes winner and bounces",
          gpu.voxels[upper].particle_id == 202
          && close(gpu.voxels[upper].velocity.z, 0.30)
          && close(gpu.voxels[upper].remainder, Vec3{}));
    check("same-target continuity ledger parity",
          same_ledger(cpu.continuity, gpu.continuity));
}

void test_arrival_not_reprocessed() {
    std::printf("\nGMT-2: arrival executes at most once per tick\n");
    constexpr int L = 8;
    std::vector<Voxel> seed(L * L * L);
    const int source = index_of(L, 3, 3, 1);
    const int first_target = index_of(L, 3, 3, 2);
    const int forbidden_second = index_of(L, 3, 3, 3);
    // One move subtracts exactly one unit, leaving a still-superthreshold
    // remainder. A live flat scan without moved[] would move it again.
    seed_particle(seed[source], +1, 303, 33,
                  {0, 0, 0.30}, {0, 0, 1.90});

    const Snapshot cpu = run_cpu(L, seed);
    const Snapshot gpu = run_gpu(L, seed);
    check("arrival-reprocessing CPU/GPU parity",
          same_movement_image(cpu.voxels, gpu.voxels));
    check("arrival stops at its first target",
          gpu.voxels[first_target].particle_id == 303
          && gpu.voxels[forbidden_second].state == 0);
    check("arrival retains the one-step remainder",
          close(gpu.voxels[first_target].remainder.z, 1.20));
}

void test_metadata_and_dual_flux_transport() {
    std::printf("\nGMT-3: metadata identity and portable dual flux\n");
    constexpr int L = 8;
    std::vector<Voxel> seed(L * L * L);
    const int source = index_of(L, 3, 3, 2);
    const int target = index_of(L, 3, 3, 3);
    Voxel& v = seed[source];
    seed_particle(v, -1, 4242, 1717, {0, 0, 0.25}, {0, 0, 0.80});
    v.spin = -1;
    v.color = 3;
    v.accel_mag = 0.125;
    v.flux_L = {0.50, 0.20, 0.10};
    v.flux_R = {0.30, 0.40, -0.10};
    v.flux = v.flux_L + v.flux_R;
    seed[target].flux_L = {0.01, -0.02, 0.03};
    seed[target].flux_R = {-0.04, 0.05, -0.06};
    seed[target].flux = seed[target].flux_L + seed[target].flux_R;

    const Snapshot cpu = run_cpu(L, seed, true);
    const Snapshot gpu = run_gpu(L, seed, true);
    check("metadata/dual transport full CPU/GPU parity",
          same_movement_image(cpu.voxels, gpu.voxels));
    const Voxel& t = gpu.voxels[target];
    check("identity metadata stays attached to the moving particle",
          t.state == -1 && t.particle_id == 4242 && t.pair_id == 1717
          && t.spin == -1 && t.color == 3 && close(t.accel_mag, 0.125));
    check("portable observable flux is carried",
          t.flux.mag() > seed[target].flux.mag());
    check("dual registers carry the same observable fraction",
          close(t.flux, t.flux_L + t.flux_R));
    check("source identity metadata is retired",
          gpu.voxels[source].state == 0
          && gpu.voxels[source].particle_id == -1
          && gpu.voxels[source].pair_id == -1
          && gpu.voxels[source].spin == 0
          && gpu.voxels[source].color == 0);
}

void test_annihilation_cleanup_and_scatter() {
    std::printf("\nGMT-4: opposite-sign cleanup and periodic scatter\n");
    constexpr int L = 8;
    std::vector<Voxel> seed(L * L * L);
    const int source = index_of(L, 4, 4, 3);
    const int target = index_of(L, 4, 4, 4);
    Voxel& a = seed[source];
    Voxel& b = seed[target];
    seed_particle(a, +1, 501, 77, {0, 0, 0.30}, {0, 0, 0.80});
    seed_particle(b, -1, 502, 77, {}, {});
    a.spin = +1; a.color = 1; a.accel_mag = 0.2;
    b.spin = -1; b.color = 2; b.accel_mag = 0.3;
    a.flux_L = {0.18, 0.06, -0.03};
    a.flux_R = {0.12, -0.02, 0.05};
    a.flux = a.flux_L + a.flux_R;
    b.flux_L = {-0.09, 0.03, 0.06};
    b.flux_R = {-0.11, -0.04, 0.02};
    b.flux = b.flux_L + b.flux_R;

    const Snapshot cpu = run_cpu(L, seed, true);
    const Snapshot gpu = run_gpu(L, seed, true);
    check("annihilation/scatter full CPU/GPU parity",
          same_movement_image(cpu.voxels, gpu.voxels));
    const auto cleaned = [](const Voxel& v) {
        return v.state == 0 && v.particle_id == -1 && v.pair_id == -1
            && v.spin == 0 && v.color == 0 && close(v.velocity, Vec3{})
            && close(v.remainder, Vec3{}) && close(v.accel_mag, 0.0);
    };
    check("annihilation clears both particle identities and dynamics",
          cleaned(gpu.voxels[source]) && cleaned(gpu.voxels[target]));
    check("annihilation continuity reaction/ledger matches CPU",
          same_ledger(cpu.continuity, gpu.continuity)
          && ftd::eft::max_continuity_residual(gpu.continuity) < 1e-13);

    bool dual_scatter_present = false;
    for (const auto& v : gpu.voxels) {
        if (v.flux_L.mag2() + v.flux_R.mag2() > 1e-12) {
            dual_scatter_present = true;
            break;
        }
    }
    check("annihilation redistributes both dual registers",
          dual_scatter_present);
}

void test_projection_and_particle_boundaries() {
    std::printf("\nGMT-5: projection and reflective/open boundaries\n");
    constexpr int L = 8;
    std::vector<Voxel> projected(L * L * L);
    const int p = index_of(L, 2, 2, 2);
    seed_particle(projected[p], +1, 601, -1,
                  {3.0, 0.0, 0.0}, {0.80, 0.0, 0.0});
    projected[p].latency = 0.2;
    const Snapshot cpu_projected = run_cpu(L, projected);
    const Snapshot gpu_projected = run_gpu(L, projected);
    check("causal movement projection CPU/GPU parity",
          same_movement_image(cpu_projected.voxels, gpu_projected.voxels,
                              2e-12)
          && cpu_projected.projection_events == 1
          && gpu_projected.projection_events == 1);

    // A later non-crossing target is tentatively projected by the parallel
    // fast path, but CPU live order annihilates it before its turn. Projection
    // telemetry must therefore remain zero after ordered reconciliation.
    std::vector<Voxel> later_target(L * L * L);
    const int early_source = index_of(L, 3, 3, 2);
    const int late_target = index_of(L, 3, 3, 3);
    seed_particle(later_target[early_source], +1, 611, -1,
                  {0, 0, 0.30}, {0, 0, 0.80});
    seed_particle(later_target[late_target], -1, 612, -1,
                  {3.0, 0, 0}, {});
    const Snapshot cpu_late = run_cpu(L, later_target);
    const Snapshot gpu_late = run_gpu(L, later_target);
    check("annihilated later non-crosser does not count projection",
          same_movement_image(cpu_late.voxels, gpu_late.voxels, 2e-12)
          && cpu_late.projection_events == 0
          && gpu_late.projection_events == 0);

    // Reversing the indices means CPU projects the non-crosser before the
    // later source annihilates it, so the same physical final image carries
    // one legitimate projection event.
    std::vector<Voxel> earlier_target(L * L * L);
    const int early_target = index_of(L, 3, 3, 2);
    const int late_source = index_of(L, 3, 3, 3);
    seed_particle(earlier_target[early_target], -1, 621, -1,
                  {3.0, 0, 0}, {});
    seed_particle(earlier_target[late_source], +1, 622, -1,
                  {0, 0, -0.30}, {0, 0, -0.80});
    const Snapshot cpu_early = run_cpu(L, earlier_target);
    const Snapshot gpu_early = run_gpu(L, earlier_target);
    check("projected earlier non-crosser retains projection event",
          same_movement_image(cpu_early.voxels, gpu_early.voxels, 2e-12)
          && cpu_early.projection_events == 1
          && gpu_early.projection_events == 1);

    std::vector<Voxel> edge(L * L * L);
    const int e = index_of(L, 0, 3, 3);
    seed_particle(edge[e], -1, 701, 7,
                  {-0.30, 0.0, 0.0}, {-0.80, 0.0, 0.0});
    edge[e].flux = {0.2, 0.1, 0.0};
    const Snapshot cpu_reflect = run_cpu(
        L, edge, false, false, ftd::FluxBoundaryMode::Reflective);
    const Snapshot gpu_reflect = run_gpu(
        L, edge, false, false, ftd::FluxBoundaryMode::Reflective);
    check("reflective face handling CPU/GPU parity",
          same_movement_image(cpu_reflect.voxels, gpu_reflect.voxels)
          && gpu_reflect.voxels[e].state == -1
          && close(gpu_reflect.voxels[e].velocity.x, 0.30)
          && close(gpu_reflect.voxels[e].remainder, Vec3{}));

    const Snapshot cpu_open = run_cpu(L, edge);
    const Snapshot gpu_open = run_gpu(L, edge);
    check("open face exhaustion CPU/GPU parity",
          same_movement_image(cpu_open.voxels, gpu_open.voxels)
          && gpu_open.voxels[e].state == 0
          && gpu_open.voxels[e].particle_id == -1
          && close(gpu_open.voxels[e].flux, Vec3{}));

    std::vector<Voxel> face_hit(L * L * L);
    const int face_hit_source = index_of(L, 1, 3, 3);
    const int face_shell = index_of(L, 0, 3, 3);
    seed_particle(face_hit[face_hit_source], -1, 702, 8,
                  {-0.30, 0.0, 0.0}, {-0.80, 0.0, 0.0});
    face_hit[face_hit_source].flux = {0.2, 0.1, 0.0};
    face_hit[face_hit_source].accel_mag = 0.75;
    const Snapshot cpu_face_hit = run_cpu(L, face_hit);
    const Snapshot gpu_face_hit = run_gpu(L, face_hit);
    check("dispersal first-contact excision CPU/GPU parity",
          same_movement_image(cpu_face_hit.voxels, gpu_face_hit.voxels)
          && gpu_face_hit.voxels[face_hit_source].state == 0
          && gpu_face_hit.voxels[face_hit_source].particle_id == -1
          && close(gpu_face_hit.voxels[face_hit_source].flux, Vec3{})
          && close(gpu_face_hit.voxels[face_hit_source].accel_mag, 0.0)
          && gpu_face_hit.voxels[face_shell].state == 0);

    std::vector<Voxel> forward(L * L * L);
    const int forward_source = index_of(L, 3, 3, L - 1);
    const int aft_target = index_of(L, 3, 3, 0);
    seed_particle(forward[forward_source], +1, 711, 17,
                  {0.0, 0.0, 0.30}, {0.0, 0.0, 0.80});
    const Snapshot cpu_periodic_z = run_cpu(
        L, forward, false, false, ftd::FluxBoundaryMode::Periodic,
        ftd::PeriodicAxis::Z);
    const Snapshot gpu_periodic_z = run_gpu(
        L, forward, false, false, ftd::FluxBoundaryMode::Periodic,
        ftd::PeriodicAxis::Z);
    check("periodic forward crossing wraps with CPU/GPU parity",
          same_movement_image(cpu_periodic_z.voxels, gpu_periodic_z.voxels)
          && gpu_periodic_z.voxels[forward_source].state == 0
          && gpu_periodic_z.voxels[aft_target].particle_id == 711);

    std::vector<Voxel> lateral(L * L * L);
    const int lateral_source = index_of(L, L - 1, 3, 3);
    seed_particle(lateral[lateral_source], +1, 712, 18,
                  {0.30, 0.0, 0.0}, {0.80, 0.0, 0.0});
    const Snapshot cpu_periodic_z_lateral = run_cpu(
        L, lateral, false, false, ftd::FluxBoundaryMode::Periodic,
        ftd::PeriodicAxis::Z);
    const Snapshot gpu_periodic_z_lateral = run_gpu(
        L, lateral, false, false, ftd::FluxBoundaryMode::Periodic,
        ftd::PeriodicAxis::Z);
    check("periodic lateral face wraps despite Z orientation metadata",
          same_movement_image(cpu_periodic_z_lateral.voxels,
                              gpu_periodic_z_lateral.voxels)
          && gpu_periodic_z_lateral.voxels[lateral_source].state == 0
          && gpu_periodic_z_lateral.voxels[index_of(L, 0, 3, 3)].particle_id == 712);
}

void test_flux_boundary_cpu_gpu_parity() {
    std::printf("\nGMT-6: reflective, dispersal, and full-domain periodic fields\n");
    constexpr int L = 8;
    constexpr int c = 4;

    const auto compare = [&](const char* name, std::vector<Voxel> seed,
                             ftd::FluxBoundaryMode mode,
                             ftd::PeriodicAxis axis) {
        const Snapshot cpu = run_cpu(L, seed, false, false, mode, axis, true);
        const Snapshot gpu = run_gpu(L, seed, false, false, mode, axis, true);
        check(name, same_movement_image(cpu.voxels, gpu.voxels, 3e-12));
    };

    std::vector<Voxel> reflective(L * L * L);
    reflective[index_of(L, 1, c, c)].flux = {0.5, -0.2, 0.1};
    reflective[index_of(L, 1, c, c)].wave_vel = {0.1, 0.05, -0.03};
    compare("reflective field shell CPU/GPU parity", reflective,
            ftd::FluxBoundaryMode::Reflective, ftd::PeriodicAxis::All);

    std::vector<Voxel> dispersal(L * L * L);
    dispersal[index_of(L, L - 1, c, c)].flux = {0.7, 0.1, -0.2};
    dispersal[index_of(L, L - 1, c, c)].flux_strong = {0.4, -0.3, 0.2};
    dispersal[index_of(L, L - 1, c, c)].wave_vel_strong = {-0.2, 0.1, 0.3};
    dispersal[index_of(L, L - 1, c, c)].flux_weak = {0.25, 0.15, -0.35};
    dispersal[index_of(L, L - 1, c, c)].wave_vel_weak = {-0.1, -0.2, 0.4};
    dispersal[index_of(L, L - 2, c, c)].wave_vel = {-0.1, 0.2, 0.05};
    compare("dispersal outflow-trace CPU/GPU parity", dispersal,
            ftd::FluxBoundaryMode::Dispersal, ftd::PeriodicAxis::All);

    std::vector<Voxel> periodic_z(L * L * L);
    periodic_z[index_of(L, c, c, L - 1)].flux = {0.6, -0.1, 0.2};
    periodic_z[index_of(L, L - 1, c, c)].flux = {0.9, 0.0, 0.0};
    compare("full-domain periodic field CPU/GPU parity with Z orientation", periodic_z,
            ftd::FluxBoundaryMode::Periodic, ftd::PeriodicAxis::Z);
}

void test_deterministic_repeatability() {
    std::printf("\nGMT-7: deterministic repeatability under contention\n");
    constexpr int L = 8;
    std::vector<Voxel> seed(L * L * L);
    int pid = 800;
    for (int x = 1; x <= 6; ++x) {
        for (int y = 1; y <= 6; ++y) {
            // Both original candidates target z=3. Half the columns bounce
            // on equal signs; half annihilate on opposite signs.
            for (int side = 0; side < 2; ++side) {
                const int z = side == 0 ? 2 : 4;
                const double dir = side == 0 ? 1.0 : -1.0;
                const int8_t sign = ((x + y + side * (x & 1)) & 1)
                    ? +1 : -1;
                Voxel& v = seed[index_of(L, x, y, z)];
                seed_particle(v, sign, pid, pid + 1000,
                              {0, 0, dir * 0.30},
                              {0, 0, dir * 0.80});
                v.spin = ((x + y + side) & 1) ? +1 : -1;
                v.color = static_cast<int8_t>(1 + (pid % 3));
                v.accel_mag = pid * 1e-5;
                v.flux_L = {pid * 1e-5, x * 1e-3, -y * 1e-3};
                v.flux_R = {-pid * 3e-6, y * 2e-3, x * 1e-3};
                v.flux = v.flux_L + v.flux_R;
                ++pid;
            }
        }
    }

    const Snapshot reference = run_gpu(L, seed, true);
    bool exact = true;
    for (int repeat = 0; repeat < 7; ++repeat) {
        const Snapshot trial = run_gpu(L, seed, true);
        if (!exactly_same_movement_image(reference.voxels, trial.voxels)
            || !same_ledger(reference.continuity, trial.continuity, 0.0)
            || reference.projection_events != trial.projection_events) {
            exact = false;
            break;
        }
    }
    check("repeated CUDA transactions are bit-exact", exact);

    const Snapshot cpu = run_cpu(L, seed, true);
    check("contention stress image retains CPU live-order parity",
          same_movement_image(cpu.voxels, reference.voxels, 3e-13));
    check("contention stress device continuity remains closed",
          ftd::eft::max_continuity_residual(reference.continuity) < 1e-13);
}

}  // namespace

int main() {
    std::printf("=== Exact CUDA movement transaction ===\n");
    test_same_target_contention();
    test_arrival_not_reprocessed();
    test_metadata_and_dual_flux_transport();
    test_annihilation_cleanup_and_scatter();
    test_projection_and_particle_boundaries();
    test_flux_boundary_cpu_gpu_parity();
    test_deterministic_repeatability();
    std::printf("\n=== %d passed, %d failed ===\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
