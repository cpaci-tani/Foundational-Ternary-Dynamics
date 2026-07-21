/**
 * FTD-0402 golden reconciliation instrument.
 *
 * Prints an independently folded hash for every persistent Voxel field group
 * plus every pre-FTD-0402 EnergyAudit channel. The same source can be linked
 * against the pre-lock engine to distinguish direct causal/mass deltas from
 * unintended field drift before golden pins are updated.
 */

#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

using namespace ftd;

namespace {

constexpr std::uint64_t OFFSET = 0xcbf29ce484222325ULL;
constexpr std::uint64_t PRIME = 0x100000001b3ULL;

std::uint64_t mix_u64(std::uint64_t h, std::uint64_t v) {
    h ^= v;
    return h * PRIME;
}

std::uint64_t mix_double(std::uint64_t h, double value) {
    if (std::isnan(value)) return mix_u64(h, 0x7ff8000000000000ULL);
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    return mix_u64(h, bits);
}

std::uint64_t mix_vec(std::uint64_t h, const Vec3& v) {
    h = mix_double(h, v.x);
    h = mix_double(h, v.y);
    return mix_double(h, v.z);
}

void inject_standard(RenderBridge& rb, int L) {
    if (L == 9) {
        rb.inject_particle(2, 2, 2, +1, {});
        rb.inject_particle(6, 6, 6, -1, {});
        rb.inject_particle(4, 2, 6, +1, {});
        rb.inject_flux(4, 4, 4, {1.0, 0.0, 0.0});
        return;
    }
    rb.inject_particle(3, 3, 3, +1, {});
    rb.inject_particle(12, 12, 12, -1, {});
    rb.inject_particle(8, 3, 12, +1, {});
    rb.inject_flux(8, 8, 8, {1.0, 0.0, 0.0});
}

void minimal_profile(RenderBridge& rb) {
    auto& t = rb.toggles;
    t.wave_propagation = true;
    t.coupling = true;
    t.gauss_projection = true;
    t.forces = true;
    t.movement = true;
    t.poisson_coulomb = true;
    t.damping = false;
    t.selective_damping = false;
    t.larmor_radiation = false;
    t.gravity = false;
    t.lorentz_force = false;
    t.color_forces = false;
    t.strong_force = false;
    t.exchange_force = false;
    t.confinement = false;
    t.dual_substrate = false;
    t.weak_transmutation = false;
    t.triad_binding = false;
    t.pair_production = false;
    t.latency_field = false;
    t.langevin = false;
    t.exact_dual_gauss = false;
    t.emergent_forces = false;
    t.genesis = true;
}

void configure(RenderBridge& rb, const std::string& profile) {
    if (profile == "minimal") minimal_profile(rb);
    else if (profile == "reflective_flux")
        rb.toggles.flux_boundary = FluxBoundaryMode::Reflective;
    else if (profile == "dispersal_flux")
        rb.toggles.flux_boundary = FluxBoundaryMode::Dispersal;
    else if (profile == "absorbing")
        rb.toggles.absorbing_boundary = true;
    else if (profile == "reflective_move") {
        rb.toggles.reflective_boundary = true;
        rb.toggles.genesis = false;
        rb.toggles.evaporation = false;
    }
}

void print_hash(const std::string& profile, const char* field,
                std::uint64_t hash) {
    std::printf("%s,%s,0x%016llx\n", profile.c_str(), field,
                static_cast<unsigned long long>(hash));
}

void print_value(const std::string& profile, const char* field, double value) {
    std::printf("%s,%s,%.17g\n", profile.c_str(), field, value);
}

void emit_manifest(const std::string& profile, const RenderBridge& rb) {
    std::uint64_t state = OFFSET, flux = OFFSET, wave = OFFSET;
    std::uint64_t velocity = OFFSET, remainder = OFFSET, latency = OFFSET;
    std::uint64_t tau = OFFSET, phase = OFFSET, identity = OFFSET;
    std::uint64_t dual = OFFSET, accel = OFFSET, strong = OFFSET, weak = OFFSET;
    const auto& voxels = rb.voxels();
    for (const auto& v : voxels) {
        state = mix_u64(state, static_cast<std::uint64_t>(static_cast<std::int64_t>(v.state)));
        flux = mix_vec(flux, v.flux);
        wave = mix_vec(wave, v.wave_vel);
        velocity = mix_vec(velocity, v.velocity);
        remainder = mix_vec(remainder, v.remainder);
        latency = mix_double(latency, v.latency);
        tau = mix_double(tau, v.tau);
        phase = mix_double(phase, v.phase);
        identity = mix_u64(identity, v.locked ? 1 : 0);
        identity = mix_u64(identity, static_cast<std::uint64_t>(v.particle_id));
        identity = mix_u64(identity, static_cast<std::uint64_t>(v.pair_id));
        identity = mix_u64(identity, static_cast<std::uint64_t>(static_cast<std::int64_t>(v.spin)));
        identity = mix_u64(identity, static_cast<std::uint64_t>(static_cast<std::int64_t>(v.color)));
        identity = mix_u64(identity, static_cast<std::uint64_t>(static_cast<std::int64_t>(v.flavor)));
        dual = mix_vec(dual, v.flux_L);
        dual = mix_vec(dual, v.flux_R);
        dual = mix_vec(dual, v.wave_vel_L);
        dual = mix_vec(dual, v.wave_vel_R);
        accel = mix_double(accel, v.accel_mag);
        strong = mix_vec(strong, v.flux_strong);
        strong = mix_vec(strong, v.wave_vel_strong);
        weak = mix_vec(weak, v.flux_weak);
        weak = mix_vec(weak, v.wave_vel_weak);
    }
    print_hash(profile, "voxel.state", state);
    print_hash(profile, "voxel.flux", flux);
    print_hash(profile, "voxel.wave_vel", wave);
    print_hash(profile, "voxel.velocity", velocity);
    print_hash(profile, "voxel.remainder", remainder);
    print_hash(profile, "voxel.latency", latency);
    print_hash(profile, "voxel.tau", tau);
    print_hash(profile, "voxel.phase", phase);
    print_hash(profile, "voxel.identity_labels", identity);
    print_hash(profile, "voxel.dual", dual);
    print_hash(profile, "voxel.accel_mag", accel);
    print_hash(profile, "voxel.strong", strong);
    print_hash(profile, "voxel.weak", weak);

    const auto a = rb.energy_audit();
    print_value(profile, "audit.field_energy", a.field_energy);
    print_value(profile, "audit.wave_energy", a.wave_energy);
    print_value(profile, "audit.particle_ke", a.particle_ke);
    print_value(profile, "audit.total_energy", a.total_energy);
    print_value(profile, "audit.gauss_violation", a.gauss_violation);
    print_value(profile, "audit.max_gauss_error", a.max_gauss_error);
    print_value(profile, "audit.self_field_injection", a.self_field_injection);
    print_value(profile, "audit.coulomb_pe", a.coulomb_pe);
    print_value(profile, "audit.E_field_energy", a.E_field_energy);
    print_value(profile, "audit.B_field_energy", a.B_field_energy);
    print_value(profile, "audit.charge_total", a.charge_total);
    print_value(profile, "audit.manifested_count", a.manifested_count);
    print_value(profile, "audit.poynting_x", a.total_poynting.x);
    print_value(profile, "audit.poynting_y", a.total_poynting.y);
    print_value(profile, "audit.poynting_z", a.total_poynting.z);
    print_value(profile, "audit.E_L_total", a.E_L_total);
    print_value(profile, "audit.E_R_total", a.E_R_total);
    print_value(profile, "audit.wv_L_total", a.wv_L_total);
    print_value(profile, "audit.wv_R_total", a.wv_R_total);
    print_value(profile, "audit.chirality_total", a.chirality_total);
    print_value(profile, "audit.strong_energy", a.strong_energy);
    print_value(profile, "audit.weak_energy", a.weak_energy);
}

} // namespace

int main(int argc, char** argv) {
    const std::string profile = argc > 1 ? argv[1] : "default";
    const int L = profile == "l9" ? 9 : 17;
    RenderBridge rb(L);
    if (profile != "gpu") rb.force_cpu();
    rb.seed_rng(42);
    configure(rb, profile);
    inject_standard(rb, L);
    if (profile == "reflective_move") {
        rb.inject_particle(1, 8, 8, +1, {});
        rb.voxel_at(1, 8, 8).velocity = {-0.5, 0.0, 0.0};
    }
    rb.seed_rng(42);
    for (int tick = 0; tick < 100; ++tick) rb.tick();
    emit_manifest(profile, rb);
    return 0;
}
