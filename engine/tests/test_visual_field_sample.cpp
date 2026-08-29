// Compact Scale-0 visual-field readback contract.
//
// These checks pin the CPU fallback used by the native WebSocket FTS2 path.
// CUDA equivalence is exercised by the live native-ws-smoke.mjs gate and the
// GPU extension parity suite; this file keeps the observable definitions from
// silently becoming blank when the render transport changes.

#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/visual_field_sample.h"
#include "ftd/constants.h"

#include <algorithm>
#include <array>
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

double max_abs(const ftd::VisualFieldSample& sample) {
    double result = 0.0;
    for (const float value : sample.data)
        result = std::max(result, std::abs(static_cast<double>(value)));
    return result;
}

double max_abs_component(const ftd::VisualFieldSample& sample, int component) {
    double result = 0.0;
    for (std::size_t i = static_cast<std::size_t>(component);
         i < sample.data.size(); i += sample.components)
        result = std::max(result, std::abs(static_cast<double>(sample.data[i])));
    return result;
}

bool vector_at(const ftd::VisualFieldSample& sample, float x, float y, float z,
               std::array<double, 3>& out) {
    for (std::size_t i = 0; i < sample.count(); ++i) {
        if (sample.positions[i * 3] == x
            && sample.positions[i * 3 + 1] == y
            && sample.positions[i * 3 + 2] == z) {
            out = {sample.data[i * 3], sample.data[i * 3 + 1], sample.data[i * 3 + 2]};
            return true;
        }
    }
    return false;
}

}  // namespace

int main() {
    std::cout << "Visual field sample contract\n";

    ftd::VisualFieldKind parsed{};
    check("parse E alias", ftd::parse_visual_field_kind("e", parsed)
          && parsed == ftd::VisualFieldKind::Electric);
    check("parse gravity alias", ftd::parse_visual_field_kind("gravity", parsed)
          && parsed == ftd::VisualFieldKind::GravityForce);
    check("parse real Poisson latency alias",
          ftd::parse_visual_field_kind("poissonLatency", parsed)
          && parsed == ftd::VisualFieldKind::PoissonLatency);
    check("wire kind 8 remains legacy flux-density latency proxy",
          static_cast<std::uint32_t>(ftd::VisualFieldKind::Latency) == 8u);
    check("real Poisson latency has stable next wire kind",
          static_cast<std::uint32_t>(ftd::VisualFieldKind::PoissonLatency) == 17u);
    check("reject unknown kind", !ftd::parse_visual_field_kind("not-a-field", parsed));

    {
        ftd::RenderBridge rb(8);
        rb.force_cpu();
        check("dispatch uniform E", ftd::dispatch_scenario(rb, "s0-field-uniform-e"));

        ftd::VisualFieldSample electric;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::Electric, 1, electric);
        check("sample uniform E", electric.count() != 0);
        check("uniform E vector layout", electric.components == 3
              && electric.positions.size() == electric.count() * 3
              && electric.data.size() == electric.count() * 3);
        check("uniform E has all voxels", electric.count() == 8u * 8u * 8u);
        check("uniform E is visible", max_abs_component(electric, 0) > 0.05);
        check("uniform E transverse components vanish",
              max_abs_component(electric, 1) < 1e-12
              && max_abs_component(electric, 2) < 1e-12);

        ftd::VisualFieldSample flux;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::FluxVector, 1, flux);
        check("zero canonical flux is represented sparsely", flux.count() == 0
              && flux.positions.empty() && flux.data.empty());
        check("uniform-E seed has zero canonical flux", max_abs(flux) < 1e-12);
    }


    // L=65 is the first lattice whose dense volume exceeds the 262144-point
    // transport ceiling, so a requested stride of one is automatically raised
    // to two.  Put the only flux on an odd/off-grid site: the old point sampler
    // returned a completely blank frame, while block-representative sampling
    // must preserve it in the regular output cell anchored at (30,32,34).
    {
        ftd::RenderBridge rb(65);
        rb.force_cpu();
        rb.inject_flux(31, 33, 35, {0.25, -0.5, 1.0});

        ftd::VisualFieldSample flux;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::FluxVector, 1, flux);
        check("large-L flux sampling raises stride to stay bounded",
              flux.effective_stride == 2);
        check("off-grid one-voxel seed survives bounded sampling", flux.count() == 1);
        check("block representative retains vector layout",
              flux.components == 3 && flux.positions.size() == 3u
              && flux.data.size() == 3u);
        if (flux.count() == 1) {
            check_close("representative x uses regular block anchor", flux.positions[0], 30.5);
            check_close("representative y uses regular block anchor", flux.positions[1], 32.5);
            check_close("representative z uses regular block anchor", flux.positions[2], 34.5);
            check_close("representative preserves Jx", flux.data[0], 0.25);
            check_close("representative preserves Jy", flux.data[1], -0.5);
            check_close("representative preserves Jz", flux.data[2], 1.0);
        }
    }

    // `latency` is the long-standing normalized |J|^2 visual proxy.  The new
    // `poissonLatency` kind reads the actual voxel latency written by the
    // gravitational Poisson solver; the two observables must remain distinct.
    {
        ftd::RenderBridge rb(9);
        rb.force_cpu();
        rb.voxel_at(4, 4, 4).latency = 0.375;

        ftd::VisualFieldSample real_latency;
        rb.copy_visual_field_sample(
            ftd::VisualFieldKind::PoissonLatency, 2, real_latency);
        check("real Poisson latency samples voxel.latency", real_latency.count() == 1);
        if (real_latency.count() == 1)
            check_close("real Poisson latency value", real_latency.data[0], 0.375);

        ftd::VisualFieldSample legacy_proxy;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::Latency, 2, legacy_proxy);
        check("legacy latency proxy stays empty without canonical flux",
              legacy_proxy.count() == 0);

        rb.inject_flux(2, 2, 2, {2.0, 0.0, 0.0});
        rb.copy_visual_field_sample(ftd::VisualFieldKind::Latency, 2, legacy_proxy);
        check("legacy latency proxy remains available from flux density",
              legacy_proxy.count() == 1 && max_abs(legacy_proxy) > 0.99);
        rb.copy_visual_field_sample(
            ftd::VisualFieldKind::PoissonLatency, 2, real_latency);
        check("real latency is not relabelled by flux-proxy data",
              real_latency.count() == 1);
        if (real_latency.count() == 1)
            check_close("real latency remains voxel-backed", real_latency.data[0], 0.375);
    }

    // Gravity is a selected effective lattice force, not a continuum 1/r
    // reconstruction. Its visual sampler must expose the exact radius-two
    // finite operator even when the tick force diagnostics are disabled and
    // even at an unmanifested site.
    {
        constexpr int L = 9;
        constexpr int C = 4;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.inject_flux(C + 2, C, C, {3.0, 0.0, 0.0});
        rb.inject_flux(C - 2, C, C, {1.0, 0.0, 0.0});

        ftd::VisualFieldSample gravity;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::GravityForce, 1, gravity);
        std::array<double, 3> center{};
        check("default gravity samples an unmanifested center site",
              vector_at(gravity, C + 0.5f, C + 0.5f, C + 0.5f, center));
        check_close("default gravity uses exact radius-two x operator", center[0],
                    ftd::G_N * ftd::GRAD_TIER2_SCALE * (3.0 - 1.0));
        check_close("default gravity y component vanishes", center[1], 0.0);
        check_close("default gravity z component vanishes", center[2], 0.0);
    }

    {
        constexpr int L = 9;
        constexpr int C = 4;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        rb.toggles.disable_all();
        rb.toggles.geometric_gravity = true;
        rb.voxel_at(C, C, C).latency = 0.4;
        rb.voxel_at(C + 2, C, C).latency = 0.6;
        rb.voxel_at(C - 2, C, C).latency = 0.2;

        ftd::VisualFieldSample gravity;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::GravityForce, 1, gravity);
        std::array<double, 3> center{};
        check("geometric gravity samples latency at the center",
              vector_at(gravity, C + 0.5f, C + 0.5f, C + 0.5f, center));
        const double expected = ftd::M_INERTIAL * ftd::C_SPEED * ftd::C_SPEED
                              * 0.4 * ftd::GRAD_TIER2_SCALE * (0.6 - 0.2);
        check_close("geometric gravity uses L times radius-two delta L",
                    center[0], expected);
        check_close("geometric gravity y component vanishes", center[1], 0.0);
        check_close("geometric gravity z component vanishes", center[2], 0.0);
    }

    {
        ftd::RenderBridge rb(10);
        rb.force_cpu();
        check("dispatch uniform B", ftd::dispatch_scenario(rb, "s0-field-uniform-b"));

        ftd::VisualFieldSample magnetic;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::Magnetic, 1, magnetic);
        check("sample uniform B", magnetic.count() != 0);
        check("B sampler matches established full-volume observable",
              magnetic.count() == 10u * 10u * 10u);
        check("uniform B is visible", max_abs_component(magnetic, 2) > 0.02);
        check("uniform B has finite data", std::all_of(
            magnetic.data.begin(), magnetic.data.end(),
            [](float value) { return std::isfinite(value); }));

        ftd::VisualFieldSample residual;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::GaussResidual, 2, residual);
        check("clean Gauss residual is represented sparsely", residual.count() == 0);
        check("scalar layout", residual.components == 1
              && residual.positions.size() == residual.count() * 3
              && residual.data.size() == residual.count());
        check("effective stride echoed", residual.effective_stride == 2);
        check("full-volume sample origin echoed", residual.origin == 0);

        ftd::VisualFieldSample vorticity;
        rb.copy_visual_field_sample(ftd::VisualFieldKind::Vorticity, 2, vorticity);
        // Interior grid is now center-anchored on the geometric center voxel
        // (was left-anchored at origin=1). For N=10, stride 2: center=(10-1)/2=4,
        // start = 4 - ((4-1)/2)*2 = 2, so the grid {2,4,6,8} includes the center.
        check("interior sample origin center-anchored", vorticity.origin == 2);
        check("interior effective stride echoed", vorticity.effective_stride == 2);
    }

    std::cout << (failures == 0 ? "ALL PASS\n" : "FAILURES: " + std::to_string(failures) + "\n");
    return failures == 0 ? 0 : 1;
}
