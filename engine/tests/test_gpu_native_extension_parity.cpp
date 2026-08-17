/**
 * Focused CPU/CUDA parity gates for native Scale-0 extension phases that sit
 * outside the historical six-phase CUDA core:
 *   EP-1  EW background drive executes before phase_read and enters L/R.
 *   EP-2  absorbing/reflective/dispersal field boundaries run post-movement.
 *   EP-3  field_energy_gravity contributes the CPU T00 source to latency.
 *   EP-4  exact_dual_gauss corrects manifested sites and preserves J=L+R.
 *   EP-5  pairwise/triad capacity overflow fails closed (never truncates).
 */

#include "ftd/constants.h"
#include "ftd/gpu_engine.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

using ftd::FluxBoundaryMode;
using ftd::RenderBridge;
using ftd::TermToggles;
using ftd::Vec3;
using ftd::Voxel;

int passed = 0;
int failed = 0;

void check(const char* name, bool ok) {
    std::printf("  %s  %s\n", ok ? "PASS" : "FAIL", name);
    ok ? ++passed : ++failed;
}

void check_close(const char* name, double a, double b, double tol) {
    const double diff = std::abs(a - b);
    std::printf("  %s  %s (cpu=%+.9e gpu=%+.9e diff=%.3e tol=%.3e)\n",
                diff <= tol ? "PASS" : "FAIL", name, a, b, diff, tol);
    diff <= tol ? ++passed : ++failed;
}

int index_of(int x, int y, int z, int L) {
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

double max_field_error(const std::vector<Voxel>& a,
                       const std::vector<Voxel>& b) {
    double result = 0.0;
    for (std::size_t i = 0; i < a.size(); ++i)
        result = std::max(result, field_error(a[i], b[i]));
    return result;
}

void scale_fields(Voxel& v, double scale) {
    v.flux *= scale;
    v.wave_vel *= scale;
    v.flux_L *= scale;
    v.flux_R *= scale;
    v.wave_vel_L *= scale;
    v.wave_vel_R *= scale;
}

struct Snapshot {
    std::vector<Voxel> voxels;
    std::vector<double> phi_latency;
};

using Configure = std::function<void(TermToggles&)>;

Snapshot run_cpu(int L, const std::vector<Voxel>& seed,
                 const Configure& configure, int sor_iterations = 6) {
    RenderBridge bridge(L);
    bridge.force_cpu();
    bridge.toggles.disable_all();
    configure(bridge.toggles);
    bridge.set_sor_iterations(sor_iterations);
    bridge.voxels() = seed;
    bridge.tick();
    Snapshot result;
    result.voxels = static_cast<const RenderBridge&>(bridge).voxels();
    result.phi_latency = bridge.phi_latency();
    return result;
}

Snapshot run_gpu(int L, const std::vector<Voxel>& seed,
                 const Configure& configure) {
    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    configure(engine.toggles);
    engine.upload_from_host(seed);
    engine.tick();
    Snapshot result;
    engine.sync_to_host(result.voxels);
    result.phi_latency = engine.phi_latency();
    return result;
}

void test_ew_pre_read_drive() {
    std::printf("\nEP-1: EW drive ordering and dual-register parity\n");
    constexpr int L = 8;
    constexpr int c = L / 2;
    std::vector<Voxel> seed(L * L * L);
    Voxel& center_seed = seed[index_of(c, c, c, L)];
    center_seed.state = 1;
    center_seed.particle_id = 7;

    const Configure configure = [](TermToggles& t) {
        t.ew_background_sweep = true;
        t.de_broglie_clock = true;
        t.omega0 = 0.5;
        t.dual_substrate = true;
    };
    const Snapshot cpu = run_cpu(L, seed, configure);
    const Snapshot gpu = run_gpu(L, seed, configure);
    const int center = index_of(c, c, c, L);
    const int vacuum = index_of(1, 1, 1, L);

    // At tick zero D=0.025. The manifested site's same-tick KG kick is
    // -omega0^2*D, proving the drive preceded phase_read (0.01875 vs 0.025).
    check_close("EW vacuum receives D(0)", cpu.voxels[vacuum].flux.x,
                0.025, 1e-14);
    check_close("EW manifested site sees same-tick KG response",
                cpu.voxels[center].flux.x, 0.01875, 1e-14);
    check("EW drive precedes phase_read",
          cpu.voxels[center].flux.x < cpu.voxels[vacuum].flux.x);
    check("EW CPU dual split is symmetric",
          std::abs(cpu.voxels[vacuum].flux_L.x
                 - cpu.voxels[vacuum].flux_R.x) < 1e-15);
    check("EW GPU dual split is symmetric",
          std::abs(gpu.voxels[vacuum].flux_L.x
                 - gpu.voxels[vacuum].flux_R.x) < 1e-15);
    check("EW full field CPU/GPU parity",
          max_field_error(cpu.voxels, gpu.voxels) < 1e-13);
}

std::vector<Voxel> boundary_seed(int L) {
    std::vector<Voxel> seed(L * L * L);
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = index_of(x, y, z, L);
        const double q = 1e-3 * (1.0 + 3.0 * x + 5.0 * y + 7.0 * z);
        seed[i].flux = Vec3(q, -2.0 * q, 0.5 * q);
        seed[i].wave_vel = Vec3(-0.3 * q, 0.7 * q, -0.9 * q);
        seed[i].flux_L = Vec3(1.1 * q, -1.2 * q, 1.3 * q);
        seed[i].flux_R = Vec3(-1.4 * q, 1.5 * q, -1.6 * q);
        seed[i].wave_vel_L = Vec3(1.7 * q, -1.8 * q, 1.9 * q);
        seed[i].wave_vel_R = Vec3(-2.0 * q, 2.1 * q, -2.2 * q);
    }
    return seed;
}

void test_field_boundaries() {
    std::printf("\nEP-2: post-movement field-boundary parity\n");
    constexpr int L = 8;
    const auto seed = boundary_seed(L);
    const Configure periodic = [](TermToggles& t) {
        t.wave_propagation = true;
    };
    const Snapshot baseline = run_cpu(L, seed, periodic);

    struct Mode {
        const char* name;
        Configure configure;
        int kind;  // 0 absorbing, 1 reflective, 2 dispersal
    };
    const Mode modes[] = {
        {"absorbing", [](TermToggles& t) {
             t.wave_propagation = true;
             t.absorbing_boundary = true;
         }, 0},
        {"reflective", [](TermToggles& t) {
             t.wave_propagation = true;
             t.flux_boundary = FluxBoundaryMode::Reflective;
         }, 1},
        {"dispersal", [](TermToggles& t) {
             t.wave_propagation = true;
             t.flux_boundary = FluxBoundaryMode::Dispersal;
         }, 2},
    };

    for (const auto& mode : modes) {
        const Snapshot cpu = run_cpu(L, seed, mode.configure);
        const Snapshot gpu = run_gpu(L, seed, mode.configure);
        std::vector<Voxel> expected = baseline.voxels;
        const int Nm1 = L - 1;
        const int depth = std::min(6, std::max(2, L / 4));
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            const int i = index_of(x, y, z, L);
            const bool shell = x == 0 || x == Nm1 || y == 0 || y == Nm1
                            || z == 0 || z == Nm1;
            if (mode.kind == 0) {
                const int d = std::min({x, Nm1 - x, y, Nm1 - y, z, Nm1 - z});
                if (d < depth) {
                    const double r = static_cast<double>(d) / depth;
                    scale_fields(expected[i], r * r);
                }
            } else if (mode.kind == 1 && shell) {
                const int sx = x == 0 ? 1 : (x == Nm1 ? Nm1 - 1 : x);
                const int sy = y == 0 ? 1 : (y == Nm1 ? Nm1 - 1 : y);
                const int sz = z == 0 ? 1 : (z == Nm1 ? Nm1 - 1 : z);
                expected[i] = baseline.voxels[index_of(sx, sy, sz, L)];
            } else if (mode.kind == 2 && shell) {
                scale_fields(expected[i], 1.0 - ftd::C_SPEED);
            }
        }

        const double cpu_law_error = max_field_error(cpu.voxels, expected);
        const double gpu_law_error = max_field_error(gpu.voxels, expected);
        const double parity_error = max_field_error(cpu.voxels, gpu.voxels);
        std::printf("    %-10s cpu-law=%.3e gpu-law=%.3e parity=%.3e\n",
                    mode.name, cpu_law_error, gpu_law_error, parity_error);
        check((std::string(mode.name) + " CPU boundary law").c_str(),
              cpu_law_error < 1e-13);
        check((std::string(mode.name) + " GPU boundary law").c_str(),
              gpu_law_error < 1e-13);
        check((std::string(mode.name) + " CPU/GPU parity").c_str(),
              parity_error < 1e-13);
    }
}

void test_field_energy_gravity() {
    std::printf("\nEP-3: field-energy latency source parity\n");
    constexpr int L = 16;
    constexpr int c = L / 2;
    std::vector<Voxel> seed(L * L * L);
    seed[index_of(c, c, c, L)].flux = Vec3(0.8, -0.3, 0.2);
    seed[index_of(c + 2, c - 1, c, L)].wave_vel = Vec3(-0.4, 0.6, 0.1);

    const Configure off = [](TermToggles& t) {
        t.gravity = true;
        t.latency_field = true;
    };
    const Configure on = [](TermToggles& t) {
        t.gravity = true;
        t.latency_field = true;
        t.field_energy_gravity = true;
    };
    const Snapshot gpu_off = run_gpu(L, seed, off);
    const Snapshot cpu = run_cpu(L, seed, on, 2000);
    const Snapshot gpu = run_gpu(L, seed, on);

    double off_max = 0.0;
    double cpu_max = 0.0;
    double gpu_max = 0.0;
    double max_phi_diff = 0.0;
    double max_latency_diff = 0.0;
    for (std::size_t i = 0; i < cpu.voxels.size(); ++i) {
        off_max = std::max(off_max, std::abs(gpu_off.phi_latency[i]));
        cpu_max = std::max(cpu_max, std::abs(cpu.phi_latency[i]));
        gpu_max = std::max(gpu_max, std::abs(gpu.phi_latency[i]));
        max_phi_diff = std::max(max_phi_diff,
                                std::abs(cpu.phi_latency[i] - gpu.phi_latency[i]));
        max_latency_diff = std::max(max_latency_diff,
            std::abs(cpu.voxels[i].latency - gpu.voxels[i].latency));
    }
    std::printf("    phi max cpu=%.6e gpu=%.6e off=%.3e; max diff=%.3e\n",
                cpu_max, gpu_max, off_max, max_phi_diff);
    check("flux-only latency source is inert when field-energy term is off",
          off_max < 1e-15);
    check("flux-only field energy produces a GPU gravity well", gpu_max > 1e-5);
    check("field-energy latency potential CPU/GPU family parity",
          max_phi_diff < cpu_max * 0.02 + 1e-8);
    check("field-energy voxel latency CPU/GPU family parity",
          max_latency_diff < 2e-3);
}

void test_exact_dual_gauss() {
    std::printf("\nEP-4: exact_dual_gauss manifested-site semantics\n");
    constexpr int L = 16;
    constexpr int c = L / 2;
    std::vector<Voxel> seed(L * L * L);
    seed[index_of(c, c, c, L)].state = 1;
    seed[index_of(c, c, c, L)].particle_id = 1;
    seed[index_of(c + 3, c, c, L)].state = -1;
    seed[index_of(c + 3, c, c, L)].particle_id = 2;

    const Configure ordinary = [](TermToggles& t) {
        t.gauss_projection = true;
        t.dual_substrate = true;
    };
    const Configure exact = [](TermToggles& t) {
        t.gauss_projection = true;
        t.dual_substrate = true;
        t.exact_dual_gauss = true;
    };
    const Snapshot gpu_ordinary = run_gpu(L, seed, ordinary);
    const Snapshot cpu_exact = run_cpu(L, seed, exact, 2000);
    const Snapshot gpu_exact = run_gpu(L, seed, exact);
    const int positive = index_of(c, c, c, L);

    const double ordinary_self = gpu_ordinary.voxels[positive].flux.mag();
    const double cpu_self = cpu_exact.voxels[positive].flux.mag();
    const double gpu_self = gpu_exact.voxels[positive].flux.mag();
    std::printf("    manifested |J| ordinary=%.6e cpu-exact=%.6e gpu-exact=%.6e\n",
                ordinary_self, cpu_self, gpu_self);
    check("ordinary GPU Gauss preserves manifested-site flux",
          ordinary_self < 1e-15);
    check("exact GPU Gauss corrects manifested-site flux", gpu_self > 1e-4);
    check_close("exact manifested correction CPU/GPU family parity",
                cpu_self, gpu_self, cpu_self * 0.02 + 1e-7);
    check("exact correction direction matches CPU",
          cpu_exact.voxels[positive].flux.x
        * gpu_exact.voxels[positive].flux.x > 0.0);

    double invariant_error = 0.0;
    for (const auto& v : gpu_exact.voxels) {
        invariant_error = std::max(invariant_error,
            vec_error(v.flux, v.flux_L + v.flux_R));
    }
    check("exact GPU Gauss preserves observable J = J_L + J_R",
          invariant_error < 1e-14);
}

void test_particle_capacity_fails_closed() {
    std::printf("\nEP-5: pairwise/triad particle-list capacity\n");
    constexpr int L = 21;  // 9,261 sites > fixed 8,192 pairwise capacity
    std::vector<Voxel> seed(static_cast<std::size_t>(L) * L * L);
    for (std::size_t i = 0; i < seed.size(); ++i) {
        seed[i].state = 1;
        seed[i].particle_id = static_cast<int>(i);
    }

    ftd::gpu::GpuEngine engine(L);
    engine.toggles.disable_all();
    engine.toggles.color_forces = true;
    engine.upload_from_host(seed);

    bool threw = false;
    std::string message;
    try {
        engine.tick();
    } catch (const std::runtime_error& ex) {
        threw = true;
        message = ex.what();
    }
    check("GPU refuses partial pairwise/triad physics above capacity",
          threw && message.find("refusing partial") != std::string::npos);
}

}  // namespace

int main() {
    std::printf("=== Native CUDA extension parity ===\n");
    test_ew_pre_read_drive();
    test_field_boundaries();
    test_field_energy_gravity();
    test_exact_dual_gauss();
    test_particle_capacity_fails_closed();
    std::printf("\n=== %d passed, %d failed ===\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
