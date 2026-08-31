/**
 * Quantitative certification for the public Scale-0 finite-box boundary probe.
 *
 * The engine has three computational flux boundary laws:
 *   Periodic   — all three opposite-face pairs are identified; axis is orientation metadata.
 *   Reflective — one ghost-cell Neumann shell copied from the first interior layer.
 *   Dispersal  — exact-zero shell plus target-local one-way stencil samples.
 *
 * These names describe algorithms. A finite simulation face is not interpreted
 * as an ontological edge of space. The one-way closure suppresses incoming
 * storage modes; it is not called a derived exact radiation condition.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/render_bridge_phases.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <array>
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

double maximum_field_amplitude(const ftd::RenderBridge& rb) {
    double out = 0.0;
    for (const auto& v : rb.voxels()) {
        out = std::max(out, std::sqrt(v.flux.mag2()));
        out = std::max(out, std::sqrt(v.wave_vel.mag2()));
    }
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

struct TransportFields {
    ftd::Vec3 flux, wave_vel, flux_L, flux_R, wave_vel_L, wave_vel_R;
    ftd::Vec3 flux_strong, wave_vel_strong, flux_weak, wave_vel_weak;
};

TransportFields snapshot(const ftd::Voxel& v) {
    return {v.flux, v.wave_vel, v.flux_L, v.flux_R,
            v.wave_vel_L, v.wave_vel_R, v.flux_strong,
            v.wave_vel_strong, v.flux_weak, v.wave_vel_weak};
}

void seed_transport_fields(ftd::Voxel& v, double a) {
    v.flux       = ftd::Vec3(a, 2*a, 3*a);
    v.wave_vel   = ftd::Vec3(4*a, 5*a, 6*a);
    v.flux_L     = ftd::Vec3(7*a, 8*a, 9*a);
    v.flux_R     = ftd::Vec3(10*a, 11*a, 12*a);
    v.wave_vel_L = ftd::Vec3(13*a, 14*a, 15*a);
    v.wave_vel_R = ftd::Vec3(16*a, 17*a, 18*a);
    v.flux_strong = ftd::Vec3(19*a, 20*a, 21*a);
    v.wave_vel_strong = ftd::Vec3(22*a, 23*a, 24*a);
    v.flux_weak = ftd::Vec3(25*a, 26*a, 27*a);
    v.wave_vel_weak = ftd::Vec3(28*a, 29*a, 30*a);
}

bool vec_scaled(const ftd::Vec3& actual, const ftd::Vec3& original,
                double scale) {
    return (actual - original * scale).mag2() < 1e-28;
}

bool transport_scaled(const ftd::Voxel& v, const TransportFields& original,
                      double scale) {
    return vec_scaled(v.flux, original.flux, scale)
        && vec_scaled(v.wave_vel, original.wave_vel, scale)
        && vec_scaled(v.flux_L, original.flux_L, scale)
        && vec_scaled(v.flux_R, original.flux_R, scale)
        && vec_scaled(v.wave_vel_L, original.wave_vel_L, scale)
        && vec_scaled(v.wave_vel_R, original.wave_vel_R, scale)
        && vec_scaled(v.flux_strong, original.flux_strong, scale)
        && vec_scaled(v.wave_vel_strong, original.wave_vel_strong, scale)
        && vec_scaled(v.flux_weak, original.flux_weak, scale)
        && vec_scaled(v.wave_vel_weak, original.wave_vel_weak, scale);
}

bool has_void_non_transport_record(const ftd::Voxel& v) {
    return v.state == 0
        && v.velocity.mag2() == 0.0
        && v.remainder.mag2() == 0.0
        && v.latency == 0.0 && v.tau == 0.0 && v.phase == 0.0
        && !v.locked && v.particle_id == -1 && v.pair_id == -1
        && v.spin == 0 && v.color == 0 && v.flavor == 0
        && v.accel_mag == 0.0;
}

void test_boundary_operator_definitions() {
    constexpr int L = 12;
    constexpr int y = 6;
    constexpr int z = 6;
    const std::array<std::array<int, 3>, 6> shell{{
        {{0, y, z}}, {{L - 1, y, z}},
        {{y, 0, z}}, {{y, L - 1, z}},
        {{y, z, 0}}, {{y, z, L - 1}},
    }};
    const std::array<std::array<int, 3>, 6> interior{{
        {{1, y, z}}, {{L - 2, y, z}},
        {{y, 1, z}}, {{y, L - 2, z}},
        {{y, z, 1}}, {{y, z, L - 2}},
    }};

    ftd::RenderBridge reflective(L);
    reflective.force_cpu();
    std::array<TransportFields, 6> reflected_sources;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        seed_transport_fields(reflective.voxel_at(
            interior[i][0], interior[i][1], interior[i][2]), 0.25 + 0.1 * i);
        seed_transport_fields(reflective.voxel_at(
            shell[i][0], shell[i][1], shell[i][2]), 9.0 + i);
        reflected_sources[i] = snapshot(reflective.voxel_at(
            interior[i][0], interior[i][1], interior[i][2]));
    }
    ftd::apply_reflective_flux_boundary(reflective);
    bool reflected_all_faces = true;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        reflected_all_faces = reflected_all_faces && transport_scaled(
            reflective.voxel_at(shell[i][0], shell[i][1], shell[i][2]),
            reflected_sources[i], 1.0);
    }
    check("reflective operator mirrors every transported field on all six faces",
          reflected_all_faces);

    ftd::RenderBridge sink(L);
    sink.force_cpu();
    std::array<TransportFields, 6> sink_sources;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        seed_transport_fields(sink.voxel_at(
            interior[i][0], interior[i][1], interior[i][2]), 0.75 + 0.1 * i);
        seed_transport_fields(sink.voxel_at(
            shell[i][0], shell[i][1], shell[i][2]), 2.0 + i);
        sink_sources[i] = snapshot(sink.voxel_at(
            interior[i][0], interior[i][1], interior[i][2]));
    }
    seed_transport_fields(sink.voxel_at(2, y, z), 1.0);
    seed_transport_fields(sink.voxel_at(3, y, z), 1.25);
    for (const auto& face : shell) {
        auto& v = sink.voxel_at(face[0], face[1], face[2]);
        sink.set_state(face[0], face[1], face[2], +1);
        v.velocity = {0.1, -0.2, 0.3};
        v.remainder = {0.4, 0.5, -0.6};
        v.latency = 0.2; v.tau = 3.0; v.phase = 0.7;
        v.locked = true; v.particle_id = 91; v.pair_id = 92;
        v.spin = 1; v.color = 2; v.flavor = 3; v.accel_mag = 0.8;
    }
    const TransportFields sink_inner_shell = snapshot(sink.voxel_at(2, y, z));
    const TransportFields sink_core = snapshot(sink.voxel_at(3, y, z));
    ftd::apply_dispersal_flux_boundary(sink);
    bool dispersal_settled_faces = true;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        const auto& face = shell[i];
        const auto& v = sink.voxel_at(face[0], face[1], face[2]);
        dispersal_settled_faces = dispersal_settled_faces
            && has_void_non_transport_record(v)
            && transport_scaled(v, sink_sources[i], 0.0);
    }
    check("dispersal settled pass exact-zeroes every record on all six faces",
          dispersal_settled_faces
          && transport_scaled(sink.voxel_at(1, y, z), sink_sources[0], 1.0)
          && transport_scaled(sink.voxel_at(2, y, z), sink_inner_shell, 1.0)
          && transport_scaled(sink.voxel_at(3, y, z), sink_core, 1.0));

    sink.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
    ftd::prepare_flux_boundary(sink);
    bool dispersal_prepared_faces = true;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        const auto& face = shell[i];
        const auto& v = sink.voxel_at(face[0], face[1], face[2]);
        dispersal_prepared_faces = dispersal_prepared_faces
            && has_void_non_transport_record(v)
            && transport_scaled(v, sink_sources[i], 0.0);
    }
    check("dispersal keeps every face exact void during stencil preparation",
          dispersal_prepared_faces);
    ftd::apply_dispersal_flux_boundary(sink);
    bool dispersal_recleared_faces = true;
    for (std::size_t i = 0; i < shell.size(); ++i) {
        const auto& face = shell[i];
        dispersal_recleared_faces = dispersal_recleared_faces
            && transport_scaled(sink.voxel_at(face[0], face[1], face[2]),
                                sink_sources[i], 0.0);
    }
    check("dispersal reasserts the exact-zero shell after local writers",
          dispersal_recleared_faces);

    ftd::RenderBridge directional(L);
    directional.force_cpu();
    directional.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    directional.toggles.periodic_axis = ftd::PeriodicAxis::Z;
    seed_transport_fields(directional.voxel_at(0, y, z), 0.4);
    seed_transport_fields(directional.voxel_at(y, y, 0), 0.6);
    const TransportFields lateral_seam = snapshot(directional.voxel_at(0, y, z));
    const TransportFields selected_seam = snapshot(directional.voxel_at(y, y, 0));
    ftd::prepare_flux_boundary(directional);
    check("periodic preparation leaves all six faces intact regardless of orientation",
          transport_scaled(directional.voxel_at(0, y, z), lateral_seam, 1.0)
          && transport_scaled(directional.voxel_at(y, y, 0), selected_seam, 1.0));

    ftd::RenderBridge sponge(L);
    sponge.force_cpu();
    TransportFields before[4];
    for (int x = 0; x < 4; ++x) {
        seed_transport_fields(sponge.voxel_at(x, y, z), 0.2 * (x + 1));
        before[x] = snapshot(sponge.voxel_at(x, y, z));
    }
    ftd::apply_absorbing_boundary(sponge);
    // L=12 gives D=min(6,max(2,L/4))=3 and f(d)=(d/3)^2.
    check("absorbing operator is exactly the declared D-deep quadratic sponge",
          transport_scaled(sponge.voxel_at(0, y, z), before[0], 0.0)
          && transport_scaled(sponge.voxel_at(1, y, z), before[1], 1.0 / 9.0)
          && transport_scaled(sponge.voxel_at(2, y, z), before[2], 4.0 / 9.0)
          && transport_scaled(sponge.voxel_at(3, y, z), before[3], 1.0));
}

void configure_isolated_wave(ftd::RenderBridge& rb,
                             ftd::FluxBoundaryMode mode,
                             ftd::PeriodicAxis axis) {
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.flux_boundary = mode;
    rb.toggles.periodic_axis = axis;
    rb.force_cpu();
}

void test_storage_wrap_is_gated_by_boundary_contract() {
    constexpr int L = 12;
    constexpr int c = 6;

    ftd::RenderBridge periodic_z(L);
    configure_isolated_wave(periodic_z, ftd::FluxBoundaryMode::Periodic,
                            ftd::PeriodicAxis::Z);
    periodic_z.voxel_at(c, c, L - 1).flux = {1.0, 0.0, 0.0};
    periodic_z.tick();
    check("periodic Z field couples across the selected forward/aft seam",
          periodic_z.voxel_at(c, c, 0).flux.mag2() > 0.0
          || periodic_z.voxel_at(c, c, 0).wave_vel.mag2() > 0.0);

    ftd::RenderBridge periodic_x(L);
    configure_isolated_wave(periodic_x, ftd::FluxBoundaryMode::Periodic,
                            ftd::PeriodicAxis::Z);
    periodic_x.voxel_at(L - 1, c, c).flux = {1.0, 0.0, 0.0};
    periodic_x.tick();
    check("periodic field crosses the lateral seam despite Z orientation metadata",
          periodic_x.voxel_at(0, c, c).flux.mag2() > 0.0
          || periodic_x.voxel_at(0, c, c).wave_vel.mag2() > 0.0);

    ftd::RenderBridge dispersal(L);
    configure_isolated_wave(dispersal, ftd::FluxBoundaryMode::Dispersal,
                            ftd::PeriodicAxis::Z);
    dispersal.voxel_at(L - 1, c, c).flux = {1.0, 0.0, 0.0};
    dispersal.tick();
    check("dispersal field cannot contaminate the opposite storage face",
          dispersal.voxel_at(0, c, c).flux.mag2() == 0.0
          && dispersal.voxel_at(0, c, c).wave_vel.mag2() == 0.0);
}

void test_boundary_probe() {
    // One fixed geometry, no scan: L=48, observation at tick 90. The qualification
    // is deliberately limited to exact operator identity plus attenuation relative
    // to the periodic control; it does not assert a perfect radiation condition.
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
    const double p_sink_0 = x_flux_momentum(sink, false);

    // Expand the actual initialized support by the exact 18-neighbor stencil.
    // This tests the native causal graph without making the false assumption
    // that this particular truncated packet is 12 cells from every face.
    const auto reachable_at_12 = causal_reach(periodic, 12);
    tick_n(periodic, 12);
    check("periodic packet remains inside the exact 18-neighbor causal cone",
          support_is_contained(periodic, reachable_at_12));

    tick_n(periodic, 78);  // total 90
    tick_n(reflective, 90);
    double minimum_sink_momentum = p_sink_0;
    for (int tick = 0; tick < 90; ++tick) {
        sink.tick();
        minimum_sink_momentum = std::min(
            minimum_sink_momentum, x_flux_momentum(sink, false));
    }

    const double h_periodic_90 = modified_hamiltonian(periodic, false);
    const double h_reflective_90 = modified_hamiltonian(reflective, true);
    const double periodic_drift = std::fabs(h_periodic_90 - h_periodic_0)
                                / std::max(1e-30, std::fabs(h_periodic_0));
    const double reflective_drift = std::fabs(h_reflective_90 - h_reflective_0)
                                  / std::max(1e-30, std::fabs(h_reflective_0));
    const double p_reflective_90 = x_flux_momentum(reflective, true);
    const double p_periodic_90 = x_flux_momentum(periodic, false);
    const double p_sink_90 = x_flux_momentum(sink, false);
    const double sink_ratio = field_norm(sink) / std::max(1e-30, norm_sink_0);

    std::cout << "    H_periodic_drift=" << periodic_drift
              << " H_reflective_drift=" << reflective_drift
              << " P0=" << p_reflective_0
              << " P_reflective_90=" << p_reflective_90
              << " P_periodic_90=" << p_periodic_90
              << " P_dispersal_90=" << p_sink_90
              << " P_dispersal_min=" << minimum_sink_momentum
              << " sink_norm_ratio=" << sink_ratio << '\n';

    check("periodic arm conserves the exact kick-drift Hamiltonian",
          periodic_drift < 1e-10);
    check("Neumann ghost-shell arm conserves its interior modified Hamiltonian",
          reflective_drift < 1e-9);
    check("Neumann ghost-shell arm reverses x-directed flux momentum",
          p_reflective_0 > 0.0 && p_reflective_90 < -0.1 * p_reflective_0);
    check("periodic control retains the original momentum sign",
          p_periodic_0 > 0.0 && p_periodic_90 > 0.1 * p_periodic_0);
    // Post-implementation regression characterization, not a theorem: this
    // fixed packet catches the old zero-shell wall, whose reverse momentum was
    // 77% of the launch momentum. The one-way closure keeps the entire 90-tick
    // reverse excursion and residual norm below 1% for this probe.
    check("dispersal probe has no macroscopic reverse-momentum reverb",
          minimum_sink_momentum > -0.01 * p_sink_0
          && sink_ratio < 0.01);
    check("all boundary arms remain unmanifested",
          manifested_count(periodic) == 0
          && manifested_count(reflective) == 0
          && manifested_count(sink) == 0);
}

void test_dispersal_long_horizon_stability() {
    // Fixed browser-reproduction gate: the default L=33 packet was observed to
    // grow to ~1e91 at the first interior edges by tick 4071 even though the
    // settled outer shell was zero. This is a long-horizon stability test, not
    // a parameter scan or a claim of an exact transparent boundary.
    constexpr int L = 33;
    constexpr int ticks = 4096;
    ftd::RenderBridge sink(L);
    sink.force_cpu();
    check("long-horizon Dispersal probe dispatches",
          ftd::dispatch_scenario(sink, "flux-pulse"));
    sink.toggles.flux_boundary = ftd::FluxBoundaryMode::Dispersal;
    sink.toggles.strict_validation = true;

    const double norm_0 = field_norm(sink);
    const double amplitude_0 = maximum_field_amplitude(sink);
    double maximum_norm = norm_0;
    double maximum_amplitude = amplitude_0;
    for (int tick = 0; tick < ticks; ++tick) {
        sink.tick();
        maximum_norm = std::max(maximum_norm, field_norm(sink));
        maximum_amplitude = std::max(
            maximum_amplitude, maximum_field_amplitude(sink));
    }
    const double norm_final = field_norm(sink);
    std::cout << "    long_horizon_norm0=" << norm_0
              << " max_norm=" << maximum_norm
              << " final_norm=" << norm_final
              << " amplitude0=" << amplitude_0
              << " max_amplitude=" << maximum_amplitude << '\n';

    check("Dispersal remains finite and bounded through tick 4096",
          std::isfinite(maximum_norm)
          && std::isfinite(maximum_amplitude)
          && maximum_norm <= 4.0 * std::max(1e-30, norm_0)
          && maximum_amplitude <= 4.0 * std::max(1e-30, amplitude_0));
    check("Dispersal leaves no long-horizon residual field",
          norm_final <= 0.01 * std::max(1e-30, norm_0));
}

}  // namespace

int main() {
    std::cout << "=== Scale-0 finite-box boundary scenario certification ===\n";
    test_boundary_operator_definitions();
    test_storage_wrap_is_gated_by_boundary_contract();
    test_boundary_probe();
    test_dispersal_long_horizon_stability();
    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
