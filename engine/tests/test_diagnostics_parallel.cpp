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

double signed_unit(std::uint64_t& state) {
    return 2.0 * static_cast<double>(next_bits(state) & 0xfffffU) / 1048575.0 - 1.0;
}

void seed(ftd::RenderBridge& rb, std::uint64_t bits) {
    auto& voxels = rb.voxels();
    for (auto& v : voxels) {
        v.flux = {signed_unit(bits), signed_unit(bits), signed_unit(bits)};
        v.velocity = {0.1 * signed_unit(bits), 0.1 * signed_unit(bits),
                      0.1 * signed_unit(bits)};
        v.latency = 0.1 * std::abs(signed_unit(bits));
        const unsigned code = static_cast<unsigned>(next_bits(bits) % 7U);
        v.state = code == 0 ? -1 : (code == 1 ? 1 : 0);
        v.spin = v.state == 0 ? 0 : ((next_bits(bits) & 1U) ? 1 : -1);
        v.color = v.state == 0 ? 0 : static_cast<int8_t>(next_bits(bits) % 4U);
    }
}

ftd::Diagnostics reference(const ftd::RenderBridge& rb) {
    ftd::Diagnostics d;
    d.tick = rb.current_tick();
    const auto& voxels = rb.voxels();
    const auto& lattice = rb.lattice();
    const auto& ternary = rb.ternary_field();
    const auto& active = ternary.ordered_active_indices();
    double total_mag2 = 0.0;
    for (const auto& v : voxels) {
        d.total_flux += v.density();
        d.total_energy += std::abs(v.born_infeld_core());
        d.max_bandwidth = std::max(d.max_bandwidth, v.bandwidth_used());
        d.max_causal_budget = std::max(d.max_causal_budget, v.causal_budget());
        total_mag2 += v.flux.mag2();
    }
    if (total_mag2 >= ftd::EPSILON_FLUX_SQ) {
        for (const auto& v : voxels) {
            const double p = v.flux.mag2() / total_mag2;
            if (p > ftd::EPSILON_FLUX_SQ) d.total_entropy -= p * std::log(p);
        }
    }
    d.manifested_count = ternary.manifested_count();
    d.positive_count = ternary.positive_count();
    d.negative_count = ternary.negative_count();
    ftd::Vec3 center;
    for (int i : active) {
        const auto& v = voxels[i];
        if (v.spin > 0) ++d.spin_up_count; else if (v.spin < 0) ++d.spin_down_count;
        if (v.color >= 0 && v.color <= 3) ++d.color_count[v.color];
        const auto c = lattice.coord(i);
        center += ftd::Vec3{static_cast<double>(c.x), static_cast<double>(c.y),
                            static_cast<double>(c.z)};
    }
    if (!active.empty()) {
        center *= 1.0 / static_cast<double>(active.size());
        for (int i : active) {
            const auto c = lattice.coord(i);
            const ftd::Vec3 r{c.x - center.x, c.y - center.y, c.z - center.z};
            d.total_angular_momentum += ftd::Vec3::cross(r, voxels[i].velocity);
        }
    }
    d.causal_projection_events = rb.causal_projection_events_this_tick();
    return d;
}

void close(const char* name, double a, double b) {
    ftd::test::check_close(name, a, b,
        3e-12 * std::max({1.0, std::abs(a), std::abs(b)}));
}

void compare(const ftd::Diagnostics& a, const ftd::Diagnostics& b) {
    ftd::test::check("tick", a.tick == b.tick);
    close("total flux", a.total_flux, b.total_flux);
    close("total energy", a.total_energy, b.total_energy);
    close("average drag", a.avg_drag, b.avg_drag);
    close("maximum bandwidth", a.max_bandwidth, b.max_bandwidth);
    close("maximum causal budget", a.max_causal_budget, b.max_causal_budget);
    close("entropy", a.total_entropy, b.total_entropy);
    ftd::test::check("manifested", a.manifested_count == b.manifested_count);
    ftd::test::check("positive", a.positive_count == b.positive_count);
    ftd::test::check("negative", a.negative_count == b.negative_count);
    ftd::test::check("spin up", a.spin_up_count == b.spin_up_count);
    ftd::test::check("spin down", a.spin_down_count == b.spin_down_count);
    for (int i = 0; i < 4; ++i)
        ftd::test::check("color count", a.color_count[i] == b.color_count[i]);
    close("angular x", a.total_angular_momentum.x, b.total_angular_momentum.x);
    close("angular y", a.total_angular_momentum.y, b.total_angular_momentum.y);
    close("angular z", a.total_angular_momentum.z, b.total_angular_momentum.z);
    ftd::test::check("causal events",
        a.causal_projection_events == b.causal_projection_events);
}

}  // namespace

int main() {
    ftd::test::init("diagnostics_parallel");
    for (int size : {1, 17, 33}) {
        ftd::RenderBridge rb(size);
        rb.force_cpu();
        seed(rb, 0xd1b54a32d192ed03ULL ^ static_cast<std::uint64_t>(size));
        compare(rb.diagnostics(), reference(rb));
    }
    return ftd::test::finalize();
}
