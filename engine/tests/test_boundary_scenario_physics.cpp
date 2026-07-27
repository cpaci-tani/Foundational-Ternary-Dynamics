/**
 * Quantitative certification for the public Scale-0 finite-box boundary probe.
 *
 * The engine has three computational flux boundary laws:
 *   Periodic   — toroidal neighbor table; closed translation-invariant system.
 *   Reflective — one ghost-cell Neumann shell copied from the first interior layer.
 *   Dispersal  — outer-shell multiplicative sink with keep=1-C_SPEED.
 *
 * These names describe algorithms. A finite simulation face is not interpreted
 * as an ontological edge of space, and the shell sink is not called a derived
 * Sommerfeld/radiation condition.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

ftd::Vec3 periodic_laplacian(const ftd::RenderBridge& rb, int x, int y, int z) {
    return rb.laplacian_flux(rb.lattice().index(x, y, z));
}

ftd::Vec3 reflective_laplacian(const ftd::RenderBridge& rb, int x, int y, int z) {
    const int L = rb.lattice().size();
    const auto sample = [&](int sx, int sy, int sz) -> const ftd::Vec3& {
        sx = std::clamp(sx, 1, L - 2);
        sy = std::clamp(sy, 1, L - 2);
        sz = std::clamp(sz, 1, L - 2);
        return rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(sx, sy, sz))].flux;
    };
    ftd::Vec3 face;
    ftd::Vec3 edge;
    const int faces[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };
    const int edges[12][3] = {
        {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
        {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
        {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
    };
    for (const auto& o : faces) face += sample(x + o[0], y + o[1], z + o[2]);
    for (const auto& o : edges) edge += sample(x + o[0], y + o[1], z + o[2]);
    return face * (1.0 / 3.0) + edge * (1.0 / 6.0)
         - sample(x, y, z) * 4.0;
}

double modified_hamiltonian(const ftd::RenderBridge& rb, bool reflective) {
    const int L = rb.lattice().size();
    const int lo = reflective ? 1 : 0;
    const int hi = reflective ? L - 1 : L;
    double kinetic = 0.0;
    double cross = 0.0;
    double gradient = 0.0;
    for (int x = lo; x < hi; ++x)
    for (int y = lo; y < hi; ++y)
    for (int z = lo; z < hi; ++z) {
        const auto& v = rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(x, y, z))];
        const ftd::Vec3 lap = reflective
            ? reflective_laplacian(rb, x, y, z)
            : periodic_laplacian(rb, x, y, z);
        kinetic += v.wave_vel.mag2();
        cross += v.wave_vel.dot(lap);
        gradient -= v.flux.dot(lap);
    }
    const double c2 = ftd::C_SPEED * ftd::C_SPEED;
    return 0.5 * kinetic + 0.5 * c2 * cross + 0.5 * c2 * gradient;
}

double field_norm(const ftd::RenderBridge& rb) {
    double out = 0.0;
    for (const auto& v : rb.voxels()) out += v.flux.mag2() + v.wave_vel.mag2();
    return out;
}

std::vector<unsigned char> causal_reach(const ftd::RenderBridge& rb, int ticks) {
    const int L = rb.lattice().size();
    std::vector<unsigned char> reach(rb.lattice().total_sites(), 0);
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int i = rb.lattice().index(x, y, z);
        const auto& v = rb.voxels()[static_cast<std::size_t>(i)];
        if (v.flux.mag2() != 0.0 || v.wave_vel.mag2() != 0.0) reach[i] = 1;
    }
    const int offsets[18][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1},
        {1,1,0},{1,-1,0},{-1,1,0},{-1,-1,0},
        {1,0,1},{1,0,-1},{-1,0,1},{-1,0,-1},
        {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}
    };
    for (int t = 0; t < ticks; ++t) {
        auto next = reach;
        for (int x = 0; x < L; ++x)
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            if (!reach[static_cast<std::size_t>(rb.lattice().index(x, y, z))]) continue;
            for (const auto& o : offsets) {
                next[static_cast<std::size_t>(
                    rb.lattice().index(x + o[0], y + o[1], z + o[2]))] = 1;
            }
        }
        reach.swap(next);
    }
    return reach;
}

bool support_is_contained(const ftd::RenderBridge& rb,
                          const std::vector<unsigned char>& reach) {
    for (std::size_t i = 0; i < rb.lattice().total_sites(); ++i) {
        const auto& v = rb.voxels()[i];
        if ((v.flux.mag2() != 0.0 || v.wave_vel.mag2() != 0.0) && !reach[i]) {
            return false;
        }
    }
    return true;
}

double normalized_divergence(const ftd::RenderBridge& rb) {
    double div2 = 0.0;
    double flux2 = 0.0;
    for (std::size_t i = 0; i < rb.lattice().total_sites(); ++i) {
        const double div = rb.divergence_flux(static_cast<int>(i));
        div2 += div * div;
        flux2 += rb.voxels()[i].flux.mag2();
    }
    return std::sqrt(div2 / std::max(1e-30, flux2));
}

double x_flux_momentum(const ftd::RenderBridge& rb, bool reflective) {
    const int L = rb.lattice().size();
    const int lo = reflective ? 1 : 0;
    const int hi = reflective ? L - 1 : L;
    double out = 0.0;
    for (int x = lo; x < hi; ++x)
    for (int y = lo; y < hi; ++y)
    for (int z = lo; z < hi; ++z) {
        const auto sample = [&](int sx) -> const ftd::Vec3& {
            if (reflective) sx = std::clamp(sx, 1, L - 2);
            return rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(sx, y, z))].flux;
        };
        const auto& v = rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(x, y, z))];
        const ftd::Vec3 dx = (sample(x + 1) - sample(x - 1)) * 0.5;
        out -= v.wave_vel.dot(dx);
    }
    return out;
}

int manifested_count(const ftd::RenderBridge& rb) {
    int count = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++count;
    return count;
}

bool only_native_wave_term_enabled(const ftd::TermToggles& toggles) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const bool enabled = toggles.*(spec.field);
        if (std::string(spec.name) == "wave_propagation") {
            if (!enabled) return false;
        } else if (std::string(spec.name) == "strict_validation") {
            continue;  // diagnostic guard, not a physics term
        } else if (enabled) {
            return false;
        }
    }
    return true;
}

void tick_n(ftd::RenderBridge& rb, int n) {
    for (int i = 0; i < n; ++i) rb.tick();
}

struct SixFields {
    ftd::Vec3 flux, wave_vel, flux_L, flux_R, wave_vel_L, wave_vel_R;
};

SixFields snapshot(const ftd::Voxel& v) {
    return {v.flux, v.wave_vel, v.flux_L, v.flux_R,
            v.wave_vel_L, v.wave_vel_R};
}

void seed_six_fields(ftd::Voxel& v, double a) {
    v.flux       = ftd::Vec3(a, 2*a, 3*a);
    v.wave_vel   = ftd::Vec3(4*a, 5*a, 6*a);
    v.flux_L     = ftd::Vec3(7*a, 8*a, 9*a);
    v.flux_R     = ftd::Vec3(10*a, 11*a, 12*a);
    v.wave_vel_L = ftd::Vec3(13*a, 14*a, 15*a);
    v.wave_vel_R = ftd::Vec3(16*a, 17*a, 18*a);
}

bool vec_scaled(const ftd::Vec3& actual, const ftd::Vec3& original,
                double scale) {
    return (actual - original * scale).mag2() < 1e-28;
}

bool six_scaled(const ftd::Voxel& v, const SixFields& original, double scale) {
    return vec_scaled(v.flux, original.flux, scale)
        && vec_scaled(v.wave_vel, original.wave_vel, scale)
        && vec_scaled(v.flux_L, original.flux_L, scale)
        && vec_scaled(v.flux_R, original.flux_R, scale)
        && vec_scaled(v.wave_vel_L, original.wave_vel_L, scale)
        && vec_scaled(v.wave_vel_R, original.wave_vel_R, scale);
}

void test_boundary_operator_definitions() {
    constexpr int L = 12;
    constexpr int y = 6;
    constexpr int z = 6;

    ftd::RenderBridge reflective(L);
    reflective.force_cpu();
    seed_six_fields(reflective.voxel_at(1, y, z), 0.25);
    seed_six_fields(reflective.voxel_at(0, y, z), 9.0);
    const SixFields reflected_source = snapshot(reflective.voxel_at(1, y, z));
    ftd::apply_reflective_flux_boundary(reflective);
    check("reflective operator copies all six fields from the clamped interior cell",
          six_scaled(reflective.voxel_at(0, y, z), reflected_source, 1.0));

    ftd::RenderBridge sink(L);
    sink.force_cpu();
    seed_six_fields(sink.voxel_at(0, y, z), 0.5);
    seed_six_fields(sink.voxel_at(1, y, z), 0.75);
    const SixFields sink_shell = snapshot(sink.voxel_at(0, y, z));
    const SixFields sink_interior = snapshot(sink.voxel_at(1, y, z));
    ftd::apply_dispersal_flux_boundary(sink);
    check("dispersal operator is exactly the declared one-shell multiplier",
          six_scaled(sink.voxel_at(0, y, z), sink_shell, 1.0 - ftd::C_SPEED)
          && six_scaled(sink.voxel_at(1, y, z), sink_interior, 1.0));

    ftd::RenderBridge sponge(L);
    sponge.force_cpu();
    SixFields before[4];
    for (int x = 0; x < 4; ++x) {
        seed_six_fields(sponge.voxel_at(x, y, z), 0.2 * (x + 1));
        before[x] = snapshot(sponge.voxel_at(x, y, z));
    }
    ftd::apply_absorbing_boundary(sponge);
    // L=12 gives D=min(6,max(2,L/4))=3 and f(d)=(d/3)^2.
    check("absorbing operator is exactly the declared D-deep quadratic sponge",
          six_scaled(sponge.voxel_at(0, y, z), before[0], 0.0)
          && six_scaled(sponge.voxel_at(1, y, z), before[1], 1.0 / 9.0)
          && six_scaled(sponge.voxel_at(2, y, z), before[2], 4.0 / 9.0)
          && six_scaled(sponge.voxel_at(3, y, z), before[3], 1.0));
}

void test_boundary_probe() {
    // One fixed geometry, no scan: L=48, observation at tick 90. The first run
    // retained 52.9% of field norm in the one-shell sink, falsifying the prior
    // <25% absorption gate. The surviving qualification is deliberately weaker:
    // exact operator identity plus attenuation relative to the periodic control.
    constexpr int L = 48;
    ftd::RenderBridge periodic(L);
    ftd::RenderBridge reflective(L);
    ftd::RenderBridge sink(L);
    periodic.force_cpu();
    reflective.force_cpu();
    sink.force_cpu();

    check("boundary probe dispatches on all three arms",
          ftd::dispatch_scenario(periodic, "flux-pulse")
          && ftd::dispatch_scenario(reflective, "flux-pulse")
          && ftd::dispatch_scenario(sink, "flux-pulse"));
    periodic.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    reflective.toggles.flux_boundary = ftd::FluxBoundaryMode::Reflective;
    sink.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
    periodic.toggles.strict_validation = true;
    reflective.toggles.strict_validation = true;
    sink.toggles.strict_validation = true;

    check("boundary probe is transverse and uses only the native wave map",
          only_native_wave_term_enabled(periodic.toggles)
          && normalized_divergence(periodic) < 1e-12);

    const double h_periodic_0 = modified_hamiltonian(periodic, false);
    const double h_reflective_0 = modified_hamiltonian(reflective, true);
    const double norm_sink_0 = field_norm(sink);
    const double p_reflective_0 = x_flux_momentum(reflective, true);
    const double p_periodic_0 = x_flux_momentum(periodic, false);

    // Expand the actual initialized support by the exact 18-neighbor stencil.
    // This tests the native causal graph without making the false assumption
    // that this particular truncated packet is 12 cells from every face.
    const auto reachable_at_12 = causal_reach(periodic, 12);
    tick_n(periodic, 12);
    check("periodic packet remains inside the exact 18-neighbor causal cone",
          support_is_contained(periodic, reachable_at_12));

    tick_n(periodic, 78);  // total 90
    tick_n(reflective, 90);
    tick_n(sink, 90);

    const double h_periodic_90 = modified_hamiltonian(periodic, false);
    const double h_reflective_90 = modified_hamiltonian(reflective, true);
    const double periodic_drift = std::fabs(h_periodic_90 - h_periodic_0)
                                / std::max(1e-30, std::fabs(h_periodic_0));
    const double reflective_drift = std::fabs(h_reflective_90 - h_reflective_0)
                                  / std::max(1e-30, std::fabs(h_reflective_0));
    const double p_reflective_90 = x_flux_momentum(reflective, true);
    const double p_periodic_90 = x_flux_momentum(periodic, false);
    const double sink_ratio = field_norm(sink) / std::max(1e-30, norm_sink_0);

    std::cout << "    H_periodic_drift=" << periodic_drift
              << " H_reflective_drift=" << reflective_drift
              << " P0=" << p_reflective_0
              << " P_reflective_90=" << p_reflective_90
              << " P_periodic_90=" << p_periodic_90
              << " sink_norm_ratio=" << sink_ratio << '\n';

    check("periodic arm conserves the exact kick-drift Hamiltonian",
          periodic_drift < 1e-10);
    check("Neumann ghost-shell arm conserves its interior modified Hamiltonian",
          reflective_drift < 1e-9);
    check("Neumann ghost-shell arm reverses x-directed flux momentum",
          p_reflective_0 > 0.0 && p_reflective_90 < -0.1 * p_reflective_0);
    check("periodic control retains the original momentum sign",
          p_periodic_0 > 0.0 && p_periodic_90 > 0.1 * p_periodic_0);
    check("single-shell sink attenuates the packet relative to the periodic arm",
          sink_ratio < 1.0);
    check("rejected 75% absorption claim remains closed negative",
          sink_ratio >= 0.25);
    check("all boundary arms remain unmanifested",
          manifested_count(periodic) == 0
          && manifested_count(reflective) == 0
          && manifested_count(sink) == 0);
}

}  // namespace

int main() {
    std::cout << "=== Scale-0 finite-box boundary scenario certification ===\n";
    test_boundary_operator_definitions();
    test_boundary_probe();
    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
