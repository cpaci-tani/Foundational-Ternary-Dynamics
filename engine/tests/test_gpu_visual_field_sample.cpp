// CUDA compact visual-field fidelity contract.
//
// Covers the two failure modes that are specific to the native large-lattice
// path: a thin seed located between regular sample points must not disappear,
// and reading the real Poisson latency field must stay on the selective device
// path rather than materializing the canonical host voxel mirror.

#include "ftd/gpu_buffers.h"
#include "ftd/render_bridge.h"
#include "ftd/visual_field_sample.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!condition) ++failures;
}

void check_close(const std::string& label, double actual, double expected,
                 double tolerance = 1e-6) {
    check(label, std::abs(actual - expected) <= tolerance);
}

void reset_full_mirror_counters() {
    ftd::gpu::g_gpu_full_voxel_download_bytes = 0;
    ftd::gpu::g_gpu_full_voxel_download_calls = 0;
}

void check_no_full_mirror(const std::string& label) {
    check(label, ftd::gpu::g_gpu_full_voxel_download_calls == 0
          && ftd::gpu::g_gpu_full_voxel_download_bytes == 0);
}

void check_sample_parity(const std::string& label,
                         const ftd::VisualFieldSample& cpu,
                         const ftd::VisualFieldSample& gpu) {
    check(label + " layout", gpu.components == cpu.components
          && gpu.effective_stride == cpu.effective_stride
          && gpu.origin == cpu.origin
          && gpu.positions == cpu.positions
          && gpu.data.size() == cpu.data.size());
    if (gpu.data.size() != cpu.data.size()) return;
    for (std::size_t i = 0; i < cpu.data.size(); ++i) {
        if (std::abs(static_cast<double>(gpu.data[i] - cpu.data[i])) > 1e-6) {
            check(label + " values", false);
            return;
        }
    }
    check(label + " values", true);
}

}  // namespace

int main() {
    std::cout << "CUDA visual field sample contract\n";

    // L=65 forces the bounded sampler from requested stride 1 to effective
    // stride 2.  The one-voxel seed lies strictly between the regular anchors.
    {
        ftd::RenderBridge rb(65);
        rb.set_interactive_gpu_mode(true);
        rb.toggles.disable_all();

        reset_full_mirror_counters();
        rb.inject_flux(31, 33, 35, {0.25, -0.5, 1.0});
        ftd::VisualFieldSample sample;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::FluxVector, 1, sample);

        check("GPU large-L sampler raises stride", sample.effective_stride == 2);
        check("GPU off-grid one-voxel seed survives", sample.count() == 1);
        check("GPU representative keeps vector layout",
              sample.components == 3 && sample.positions.size() == 3u
              && sample.data.size() == 3u);
        if (sample.count() == 1) {
            check_close("GPU representative x block anchor", sample.positions[0], 30.5);
            check_close("GPU representative y block anchor", sample.positions[1], 32.5);
            check_close("GPU representative z block anchor", sample.positions[2], 34.5);
            check_close("GPU representative preserves Jx", sample.data[0], 0.25);
            check_close("GPU representative preserves Jy", sample.data[1], -0.5);
            check_close("GPU representative preserves Jz", sample.data[2], 1.0);
        }
        check_no_full_mirror("GPU flux injection + compact sample avoid full mirror");
    }

    // Seed identical host latency values, then compare CPU and CUDA compact
    // samplers.  Reset after the deliberate host mutation: the assertion is
    // specifically that the sampler uploads the delta and reads d_latency
    // without a device->host lattice mirror.
    {
        constexpr int L = 9;
        constexpr int C = 4;
        constexpr double expected = 0.375;

        ftd::RenderBridge cpu(L);
        cpu.force_cpu();
        cpu.voxel_at(C, C, C).latency = expected;
        ftd::VisualFieldSample cpu_sample;
        cpu.copy_visual_field_sample(
            ftd::VisualFieldKind::PoissonLatency, 2, cpu_sample);

        ftd::RenderBridge gpu(L);
        gpu.set_interactive_gpu_mode(true);
        gpu.toggles.disable_all();
        gpu.voxel_at(C, C, C).latency = expected;
        reset_full_mirror_counters();
        ftd::VisualFieldSample gpu_sample;
        gpu.copy_visual_field_sample(
            ftd::VisualFieldKind::PoissonLatency, 2, gpu_sample);

        check("CPU/GPU real latency sparse count parity",
              cpu_sample.count() == 1 && gpu_sample.count() == cpu_sample.count());
        check("CPU/GPU real latency position parity",
              gpu_sample.positions == cpu_sample.positions);
        check("CPU/GPU real latency data parity", gpu_sample.data == cpu_sample.data);
        if (gpu_sample.count() == 1)
            check_close("GPU real latency reads d_latency", gpu_sample.data[0], expected);
        check_no_full_mirror("GPU real latency sample avoids full mirror");

        ftd::VisualFieldSample legacy_proxy;
        gpu.copy_visual_field_sample(ftd::VisualFieldKind::Latency, 2, legacy_proxy);
        check("GPU legacy |J|^2 latency proxy remains distinct",
              legacy_proxy.count() == 0);
        check_no_full_mirror("GPU legacy latency proxy also stays compact");
    }

    // Gravity-force visuals must evaluate the same exact radius-two operator
    // on CPU and CUDA. They must not depend on sparse manifestation-only force
    // diagnostics, and CUDA must stay on the compact selective-read path.
    {
        constexpr int L = 9;
        constexpr int C = 4;
        ftd::RenderBridge cpu(L);
        cpu.force_cpu();
        cpu.toggles.disable_all();
        cpu.inject_flux(C + 2, C, C, {3.0, 0.0, 0.0});
        cpu.inject_flux(C - 2, C, C, {1.0, 0.0, 0.0});
        ftd::VisualFieldSample cpu_gravity;
        cpu.copy_visual_field_sample(
            ftd::VisualFieldKind::GravityForce, 1, cpu_gravity);

        ftd::RenderBridge gpu(L);
        gpu.set_interactive_gpu_mode(true);
        gpu.toggles.disable_all();
        gpu.inject_flux(C + 2, C, C, {3.0, 0.0, 0.0});
        gpu.inject_flux(C - 2, C, C, {1.0, 0.0, 0.0});
        reset_full_mirror_counters();
        ftd::VisualFieldSample gpu_gravity;
        gpu.copy_visual_field_sample(
            ftd::VisualFieldKind::GravityForce, 1, gpu_gravity);

        check("CPU/CUDA default gravity produces visible vectors",
              cpu_gravity.count() != 0 && gpu_gravity.count() != 0);
        check_sample_parity("CPU/CUDA default radius-two gravity", cpu_gravity, gpu_gravity);
        check_no_full_mirror("GPU default gravity sample avoids full mirror");
    }

    {
        constexpr int L = 9;
        constexpr int C = 4;
        ftd::RenderBridge cpu(L);
        cpu.force_cpu();
        cpu.toggles.disable_all();
        cpu.toggles.geometric_gravity = true;
        cpu.voxel_at(C, C, C).latency = 0.4;
        cpu.voxel_at(C + 2, C, C).latency = 0.6;
        cpu.voxel_at(C - 2, C, C).latency = 0.2;
        ftd::VisualFieldSample cpu_gravity;
        cpu.copy_visual_field_sample(
            ftd::VisualFieldKind::GravityForce, 1, cpu_gravity);

        ftd::RenderBridge gpu(L);
        gpu.set_interactive_gpu_mode(true);
        gpu.toggles.disable_all();
        gpu.toggles.geometric_gravity = true;
        gpu.voxel_at(C, C, C).latency = 0.4;
        gpu.voxel_at(C + 2, C, C).latency = 0.6;
        gpu.voxel_at(C - 2, C, C).latency = 0.2;
        reset_full_mirror_counters();
        ftd::VisualFieldSample gpu_gravity;
        gpu.copy_visual_field_sample(
            ftd::VisualFieldKind::GravityForce, 1, gpu_gravity);

        check("CPU/CUDA geometric gravity produces visible vectors",
              cpu_gravity.count() != 0 && gpu_gravity.count() != 0);
        check_sample_parity("CPU/CUDA geometric latency gravity", cpu_gravity, gpu_gravity);
        check_no_full_mirror("GPU geometric gravity sample avoids full mirror");
    }

    // Finally exercise d_latency as produced by the real CUDA Poisson phase,
    // not merely by a host delta upload.
    {
        constexpr int L = 17;
        const int c = L / 2;
        ftd::RenderBridge rb(L);
        rb.set_interactive_gpu_mode(true);
        rb.toggles.disable_all();
        rb.toggles.gravity = true;
        rb.toggles.latency_field = true;
        rb.inject_particle(c, c, c, +1, {0.25, 0.0, 0.0});

        reset_full_mirror_counters();
        rb.tick();
        ftd::VisualFieldSample real_latency;
        rb.copy_visual_field_sample(
            ftd::VisualFieldKind::PoissonLatency, 1, real_latency);
        check("CUDA Poisson solve produces visible real latency",
              real_latency.count() != 0
              && std::any_of(real_latency.data.begin(), real_latency.data.end(),
                  [](float value) { return std::isfinite(value) && value > 0.0f; }));
        check_no_full_mirror("CUDA latency tick + visual sample avoid full mirror");
    }

    std::cout << (failures == 0 ? "ALL PASS\n"
                                : "FAILURES: " + std::to_string(failures) + "\n");
    return failures == 0 ? 0 : 1;
}
