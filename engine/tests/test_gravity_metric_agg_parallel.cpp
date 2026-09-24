#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {

std::uint64_t next_bits(std::uint64_t& state) {
    state ^= state << 13; state ^= state >> 7; state ^= state << 17;
    return state;
}

double unit(std::uint64_t& state) {
    return static_cast<double>(next_bits(state) & 0xfffffU) / 1048575.0;
}

ftd::GravityMetricAgg reference(const ftd::RenderBridge& rb) {
    ftd::GravityMetricAgg out;
    double sum = 0.0;
    for (const auto& voxel : rb.voxels()) {
        const double latency = voxel.latency;
        if (latency <= 0.0) continue;
        out.latency_max = std::max(out.latency_max, latency);
        out.gamma_max = std::max(out.gamma_max, voxel.gamma_ftd());
        sum += latency;
        ++out.voxel_count;
    }
    if (out.voxel_count > 0) {
        out.latency_mean = sum / out.voxel_count;
        out.f_min = 1.0 - out.latency_max * out.latency_max;
        out.dilation_max_pct =
            (1.0 - std::sqrt(std::max(0.0, out.f_min))) * 100.0;
    }
    out.requested = rb.toggles.latency_field || rb.toggles.field_energy_gravity;
    out.active = out.requested && out.voxel_count > 0;
    return out;
}

void compare(const ftd::GravityMetricAgg& actual,
             const ftd::GravityMetricAgg& expected) {
    const auto close = [](const char* name, double a, double b) {
        ftd::test::check_close(name, a, b,
            3e-14 * std::max({1.0, std::abs(a), std::abs(b)}));
    };
    close("latency max", actual.latency_max, expected.latency_max);
    close("latency mean", actual.latency_mean, expected.latency_mean);
    close("f min", actual.f_min, expected.f_min);
    close("gamma max", actual.gamma_max, expected.gamma_max);
    close("dilation max", actual.dilation_max_pct, expected.dilation_max_pct);
    ftd::test::check("voxel count", actual.voxel_count == expected.voxel_count);
    ftd::test::check("requested", actual.requested == expected.requested);
    ftd::test::check("active", actual.active == expected.active);
}

}  // namespace

int main() {
    ftd::test::init("gravity_metric_agg_parallel");
    for (int size : {1, 17, 33}) {
        ftd::RenderBridge rb(size);
        rb.force_cpu();
        std::uint64_t bits = 0x9e3779b97f4a7c15ULL ^ static_cast<std::uint64_t>(size);
        auto& voxels = rb.voxels();
        for (std::size_t i = 0; i < voxels.size(); ++i) {
            auto& voxel = voxels[i];
            voxel.latency = (i % 11 == 0) ? 0.0 : 0.8 * unit(bits);
            voxel.velocity = {0.02 * unit(bits), 0.02 * unit(bits), 0.02 * unit(bits)};
        }

        // Stamped latency remains observable even when its producer toggle is
        // disabled; requested/active must retain the existing distinction.
        rb.toggles.latency_field = false;
        rb.toggles.field_energy_gravity = false;
        compare(rb.gravity_metric_agg(), reference(rb));

        rb.toggles.latency_field = true;
        compare(rb.gravity_metric_agg(), reference(rb));
    }
    return ftd::test::finalize();
}
