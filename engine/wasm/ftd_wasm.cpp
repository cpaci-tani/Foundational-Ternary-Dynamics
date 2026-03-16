/**
 * Emscripten Embind bindings for the FTD engine.
 *
 * Exposes RenderBridge to JavaScript with typed-array extraction
 * for zero-copy GPU upload of particle data, plus full diagnostic
 * access (energy audit, Lagrangian constraints, voxel inspection,
 * force decomposition, spin/color statistics).
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/particle_engine.h"
#include "ftd/atom_engine.h"
#include "ftd/constants.h"

using namespace emscripten;

// ── Particle Data Extraction ─────────────────────────────────────────
// Returns a JS object with Float32Array views for direct BufferAttribute upload.
// Format: { positions: Float32Array, colors: Float32Array, sizes: Float32Array, count: int }

val get_particle_data(ftd::RenderBridge& rb) {
    const auto& voxels = rb.voxels();
    const int N = rb.lattice().size();
    const int total = N * N * N;

    // Count manifested + flux-carrying voxels for display
    int count = 0;
    for (int i = 0; i < total; i++) {
        if (voxels[i].state != 0 || voxels[i].density() > ftd::K_B * 0.05) {
            count++;
        }
    }

    // Allocate JS typed arrays
    val positions = val::global("Float32Array").new_(count * 3);
    val colors    = val::global("Float32Array").new_(count * 3);
    val sizes     = val::global("Float32Array").new_(count);

    int idx = 0;
    for (int i = 0; i < total; i++) {
        const auto& v = voxels[i];
        if (v.state == 0 && v.density() <= ftd::K_B * 0.05) continue;

        // Position from lattice index
        auto c = rb.lattice().coord(i);
        int x = c.x, y = c.y, z = c.z;

        positions.set(idx * 3,     static_cast<float>(x));
        positions.set(idx * 3 + 1, static_cast<float>(y));
        positions.set(idx * 3 + 2, static_cast<float>(z));

        // Color by state
        if (v.state == 1) {
            colors.set(idx * 3,     0.29f);  // green
            colors.set(idx * 3 + 1, 0.87f);
            colors.set(idx * 3 + 2, 0.50f);
        } else if (v.state == -1) {
            colors.set(idx * 3,     0.97f);  // red
            colors.set(idx * 3 + 1, 0.44f);
            colors.set(idx * 3 + 2, 0.44f);
        } else {
            // Void with flux: blue-gray, brightness proportional to density
            float brightness = static_cast<float>(v.density() / (ftd::K_B * 2.0));
            if (brightness > 1.0f) brightness = 1.0f;
            colors.set(idx * 3,     0.37f + brightness * 0.1f);
            colors.set(idx * 3 + 1, 0.45f + brightness * 0.1f);
            colors.set(idx * 3 + 2, 0.58f + brightness * 0.2f);
        }

        // Size: manifested particles larger, void proportional to density
        if (v.state != 0) {
            sizes.set(idx, 6.0f);
        } else {
            float s = 1.5f + static_cast<float>(v.density() / ftd::K_B) * 3.0f;
            if (s > 5.0f) s = 5.0f;
            sizes.set(idx, s);
        }

        idx++;
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("colors", colors);
    result.set("sizes", sizes);
    result.set("count", count);
    return result;
}

// ── Diagnostics Extraction (full) ───────────────────────────────────
val get_diagnostics(ftd::RenderBridge& rb) {
    auto d = rb.diagnostics();
    val result = val::object();
    result.set("tick",          d.tick);
    result.set("physicalTime",  rb.physical_time());
    result.set("dt",            rb.dt());
    result.set("manifested",    d.manifested_count);
    result.set("positive",      d.positive_count);
    result.set("negative",      d.negative_count);
    result.set("totalFlux",     d.total_flux);
    result.set("totalEnergy",   d.total_energy);
    result.set("maxBandwidth",  d.max_bandwidth);
    result.set("avgDrag",       d.avg_drag);
    result.set("entropy",       d.total_entropy);
    result.set("chargeBalance", d.positive_count - d.negative_count);
    // Spin statistics
    result.set("spinUp",        d.spin_up_count);
    result.set("spinDown",      d.spin_down_count);
    // Color statistics
    result.set("colorless",     d.color_count[0]);
    result.set("colorRed",      d.color_count[1]);
    result.set("colorGreen",    d.color_count[2]);
    result.set("colorBlue",     d.color_count[3]);
    // Angular momentum
    result.set("angMomX",       d.total_angular_momentum.x);
    result.set("angMomY",       d.total_angular_momentum.y);
    result.set("angMomZ",       d.total_angular_momentum.z);
    return result;
}

// ── Energy Audit ────────────────────────────────────────────────────
val get_energy_audit(ftd::RenderBridge& rb) {
    auto ea = rb.energy_audit();
    val result = val::object();
    result.set("fieldEnergy",       ea.field_energy);
    result.set("waveEnergy",        ea.wave_energy);
    result.set("particleKE",        ea.particle_ke);
    result.set("totalEnergy",       ea.total_energy);
    result.set("gaussViolation",    ea.gauss_violation);
    result.set("maxGaussError",     ea.max_gauss_error);
    result.set("selfFieldInjection", ea.self_field_injection);
    result.set("coulombPE",         ea.coulomb_pe);
    result.set("EFieldEnergy",      ea.E_field_energy);
    result.set("BFieldEnergy",      ea.B_field_energy);
    result.set("chargeTotal",       ea.charge_total);
    result.set("manifested",        ea.manifested_count);
    return result;
}

// ── Lagrangian Extraction (full with constraints) ───────────────────
val get_lagrangian(ftd::RenderBridge& rb) {
    auto lag = ftd::compute_lagrangian_diagnostics(rb);
    val result = val::object();
    // Field-sector terms
    result.set("fieldKinetic",  lag.field_kinetic_sum);
    result.set("fieldGradient", lag.field_gradient_sum);
    // Interaction-sector terms
    result.set("bornInfeld",  lag.born_infeld_sum);
    result.set("coupling",    lag.coupling_sum);
    result.set("velocity",    lag.velocity_coupling_sum);
    result.set("gauss",       lag.gauss_sum);
    result.set("dissipation", lag.dissipation_sum);
    // Totals
    result.set("total",       lag.total_lagrangian);
    result.set("hamiltonian", lag.total_hamiltonian);
    result.set("totalAction", lag.total_action);
    // Constraint violations
    result.set("gaussViolation",  lag.gauss_violation);
    result.set("maxGaussError",   lag.max_gauss_error);
    // Conservation checks
    result.set("totalFluxMag",    lag.total_flux_mag);
    result.set("totalWaveEnergy", lag.total_wave_energy);
    // Counters
    result.set("manifested",      lag.manifested_count);
    result.set("locked",          lag.locked_count);
    return result;
}

// ── Voxel Inspection ────────────────────────────────────────────────
// Inspect a single voxel by lattice coordinates
val inspect_voxel(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto& v = rb.voxel_at(x, y, z);
    int idx = rb.lattice().index(x, y, z);

    val result = val::object();
    // State
    result.set("state",      static_cast<int>(v.state));
    result.set("particleId", v.particle_id);
    result.set("pairId",     v.pair_id);
    result.set("locked",     v.locked);
    result.set("spin",       static_cast<int>(v.spin));
    result.set("color",      static_cast<int>(v.color));
    // Flux
    result.set("fluxX",      v.flux.x);
    result.set("fluxY",      v.flux.y);
    result.set("fluxZ",      v.flux.z);
    result.set("density",    v.density());
    // Wave velocity
    result.set("waveVelX",   v.wave_vel.x);
    result.set("waveVelY",   v.wave_vel.y);
    result.set("waveVelZ",   v.wave_vel.z);
    // Particle velocity
    result.set("velX",       v.velocity.x);
    result.set("velY",       v.velocity.y);
    result.set("velZ",       v.velocity.z);
    result.set("speed",      v.speed());
    // Acceleration
    result.set("accelMag",   v.accel_mag);
    // Discrete operators at this site
    double divJ = rb.divergence_flux(idx);
    auto curlJ = rb.curl_flux(idx);
    result.set("divJ",       divJ);
    result.set("curlX",      curlJ.x);
    result.set("curlY",      curlJ.y);
    result.set("curlZ",      curlJ.z);
    // EM field decomposition: E = -wave_vel, B = curl(J)
    auto em = rb.em_field_at(idx);
    result.set("Ex",         em.E.x);
    result.set("Ey",         em.E.y);
    result.set("Ez",         em.E.z);
    result.set("Emag",       em.E_mag);
    result.set("Bx",         em.B.x);
    result.set("By",         em.B.y);
    result.set("Bz",         em.B.z);
    result.set("Bmag",       em.B_mag);
    return result;
}

// ── Force Diagnostics at a voxel ────────────────────────────────────
val get_force_at(ftd::RenderBridge& rb, int x, int y, int z) {
    const auto& fd = rb.force_diag_at(x, y, z);
    val result = val::object();
    // Coulomb
    result.set("coulombX", fd.f_coulomb.x);
    result.set("coulombY", fd.f_coulomb.y);
    result.set("coulombZ", fd.f_coulomb.z);
    result.set("coulombMag", fd.f_coulomb.mag());
    // Strong
    result.set("strongX", fd.f_strong.x);
    result.set("strongY", fd.f_strong.y);
    result.set("strongZ", fd.f_strong.z);
    result.set("strongMag", fd.f_strong.mag());
    // Magnetic
    result.set("magneticX", fd.f_magnetic.x);
    result.set("magneticY", fd.f_magnetic.y);
    result.set("magneticZ", fd.f_magnetic.z);
    result.set("magneticMag", fd.f_magnetic.mag());
    // Gravity
    result.set("gravityX", fd.f_gravity.x);
    result.set("gravityY", fd.f_gravity.y);
    result.set("gravityZ", fd.f_gravity.z);
    result.set("gravityMag", fd.f_gravity.mag());
    // Exchange (Pauli)
    result.set("exchangeX", fd.f_exchange.x);
    result.set("exchangeY", fd.f_exchange.y);
    result.set("exchangeZ", fd.f_exchange.z);
    result.set("exchangeMag", fd.f_exchange.mag());
    return result;
}

// ── Engine Constants ────────────────────────────────────────────────
val get_constants() {
    val result = val::object();
    result.set("ALPHA",       ftd::ALPHA);
    result.set("ALPHA_INV",   1.0 / ftd::ALPHA);
    result.set("G_STAR",      ftd::G_STAR);
    result.set("K_B",         ftd::K_B);
    result.set("K_GENESIS",   ftd::K_GENESIS);
    result.set("G_C",         ftd::G_C);
    result.set("G_N",         ftd::G_N);
    result.set("DAMPING",     ftd::DAMPING);
    result.set("C_SPEED",     ftd::C_SPEED);
    result.set("N_C",         ftd::N_C);
    result.set("B3",          ftd::B_3);
    result.set("N_BASE",      ftd::N_BASE);
    result.set("N_EFF",       ftd::N_EFF);
    result.set("VARPI",       ftd::VARPI);
    return result;
}

// ── Toggle wrapper ───────────────────────────────────────────────────
void set_toggle(ftd::RenderBridge& rb, const std::string& name, bool value) {
    auto& t = rb.toggles;
    if      (name == "wave_propagation") t.wave_propagation = value;
    else if (name == "coupling")         t.coupling = value;
    else if (name == "damping")          t.damping = value;
    else if (name == "genesis")          t.genesis = value;
    else if (name == "gauss_projection") t.gauss_projection = value;
    else if (name == "forces")           t.forces = value;
    else if (name == "gravity")          t.gravity = value;
    else if (name == "poisson_coulomb")  t.poisson_coulomb = value;
    else if (name == "movement")         t.movement = value;
    else if (name == "lorentz_force")    t.lorentz_force = value;
    else if (name == "selective_damping") t.selective_damping = value;
    else if (name == "larmor_radiation") t.larmor_radiation = value;
    else if (name == "dual_substrate")   t.dual_substrate = value;
    else if (name == "color_forces")     t.color_forces = value;
    else if (name == "weak_transmutation") t.weak_transmutation = value;
    else if (name == "strong_force")     t.strong_force = value;
    else if (name == "triad_binding")    t.triad_binding = value;
    else if (name == "pair_production")  t.pair_production = value;
    else if (name == "exchange_force")   t.exchange_force = value;
    else if (name == "latency_field")    t.latency_field = value;
}

bool get_toggle(ftd::RenderBridge& rb, const std::string& name) {
    const auto& t = rb.toggles;
    if      (name == "wave_propagation") return t.wave_propagation;
    else if (name == "coupling")         return t.coupling;
    else if (name == "damping")          return t.damping;
    else if (name == "genesis")          return t.genesis;
    else if (name == "gauss_projection") return t.gauss_projection;
    else if (name == "forces")           return t.forces;
    else if (name == "gravity")          return t.gravity;
    else if (name == "poisson_coulomb")  return t.poisson_coulomb;
    else if (name == "movement")         return t.movement;
    else if (name == "lorentz_force")    return t.lorentz_force;
    else if (name == "selective_damping") return t.selective_damping;
    else if (name == "larmor_radiation") return t.larmor_radiation;
    else if (name == "dual_substrate")   return t.dual_substrate;
    else if (name == "color_forces")     return t.color_forces;
    else if (name == "weak_transmutation") return t.weak_transmutation;
    else if (name == "strong_force")     return t.strong_force;
    else if (name == "triad_binding")    return t.triad_binding;
    else if (name == "pair_production")  return t.pair_production;
    else if (name == "exchange_force")   return t.exchange_force;
    else if (name == "latency_field")    return t.latency_field;
    return false;
}

// ── Inject wrappers ──────────────────────────────────────────────────
void inject_particle_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), ftd::Vec3(0, 0, 0));
}

void inject_particle_full(ftd::RenderBridge& rb, int x, int y, int z,
                           int state, int spin, int color) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), ftd::Vec3(0, 0, 0),
                       static_cast<int8_t>(spin), static_cast<int8_t>(color));
}

void inject_wavepacket_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_wavepacket(x, y, z, static_cast<int8_t>(state));
}

void inject_wavepacket_full(ftd::RenderBridge& rb, int x, int y, int z,
                             int state, double sigma, double amplitude) {
    rb.inject_wavepacket(x, y, z, static_cast<int8_t>(state), sigma, amplitude);
}

void inject_flux(ftd::RenderBridge& rb, int x, int y, int z,
                  double fx, double fy, double fz) {
    rb.inject_flux(x, y, z, ftd::Vec3(fx, fy, fz));
}

void create_entangled_pair(ftd::RenderBridge& rb, int x, int y, int z,
                            double fx, double fy, double fz) {
    rb.create_entangled_pair(x, y, z, ftd::Vec3(fx, fy, fz));
}

// ── Time step control ────────────────────────────────────────────────
void set_dt(ftd::RenderBridge& rb, double dt) { rb.set_dt(dt); }
double get_dt(ftd::RenderBridge& rb) { return rb.dt(); }
double get_physical_time(ftd::RenderBridge& rb) { return rb.physical_time(); }

// ── Bulk flux extraction ─────────────────────────────────────────────
// getFluxSlice: returns Float64Array of flux magnitudes for a 2D slice
//   axis: 0=YZ (fixed X), 1=XZ (fixed Y), 2=XY (fixed Z)
//   index: which slice along that axis
val get_flux_slice(ftd::RenderBridge& rb, int axis, int index) {
    const int N = rb.lattice().size();
    const int sliceSize = N * N;
    const auto& voxels = rb.voxels();

    val data = val::global("Float64Array").new_(sliceSize);

    for (int a = 0; a < N; ++a) {
        for (int b = 0; b < N; ++b) {
            int x, y, z;
            if (axis == 0)      { x = index; y = a; z = b; }
            else if (axis == 1) { x = a; y = index; z = b; }
            else                { x = a; y = b; z = index; }
            int idx = rb.lattice().index(x, y, z);
            data.set(a * N + b, voxels[idx].density());
        }
    }

    return data;
}

// getFluxVolume: returns Float64Array of all voxel flux magnitudes (N^3 values)
val get_flux_volume(ftd::RenderBridge& rb) {
    const int N = rb.lattice().size();
    const int total = N * N * N;
    const auto& voxels = rb.voxels();

    val data = val::global("Float64Array").new_(total);

    for (int i = 0; i < total; ++i) {
        data.set(i, voxels[i].density());
    }

    return data;
}

// ── Bulk Sampled Vector Field Exports ────────────────────────────────
// Each returns { positions: Float32Array(N×3), vectors: Float32Array(N×3), count: int }
// stride controls spatial sampling density (stride=2 → every other voxel).

val get_e_field_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val vectors   = val::global("Float32Array").new_(maxPts * 3);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto em = rb.em_field_at(idx);
                double mag = em.E_mag;
                if (mag < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                vectors.set(count * 3,       static_cast<float>(em.E.x));
                vectors.set(count * 3 + 1,   static_cast<float>(em.E.y));
                vectors.set(count * 3 + 2,   static_cast<float>(em.E.z));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("vectors", vectors);
    result.set("count", count);
    return result;
}

val get_b_field_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val vectors   = val::global("Float32Array").new_(maxPts * 3);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto em = rb.em_field_at(idx);
                double mag = em.B_mag;
                if (mag < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                vectors.set(count * 3,       static_cast<float>(em.B.x));
                vectors.set(count * 3 + 1,   static_cast<float>(em.B.y));
                vectors.set(count * 3 + 2,   static_cast<float>(em.B.z));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("vectors", vectors);
    result.set("count", count);
    return result;
}

val get_poynting_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val vectors   = val::global("Float32Array").new_(maxPts * 3);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto S_vec = rb.poynting_vector(idx);
                double mag = S_vec.mag();
                if (mag < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                vectors.set(count * 3,       static_cast<float>(S_vec.x));
                vectors.set(count * 3 + 1,   static_cast<float>(S_vec.y));
                vectors.set(count * 3 + 2,   static_cast<float>(S_vec.z));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("vectors", vectors);
    result.set("count", count);
    return result;
}

val get_divj_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val values    = val::global("Float32Array").new_(maxPts);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                double div = rb.divergence_flux(idx);
                if (std::abs(div) < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                values.set(count, static_cast<float>(div));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("values", values);
    result.set("count", count);
    return result;
}

val get_flux_vector_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;
    const auto& voxels = rb.voxels();

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val vectors   = val::global("Float32Array").new_(maxPts * 3);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                const auto& v = voxels[idx];
                double mag = v.density();
                if (mag < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                vectors.set(count * 3,       static_cast<float>(v.flux.x));
                vectors.set(count * 3 + 1,   static_cast<float>(v.flux.y));
                vectors.set(count * 3 + 2,   static_cast<float>(v.flux.z));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("vectors", vectors);
    result.set("count", count);
    return result;
}

val get_force_field_sampled(ftd::RenderBridge& rb, int stride) {
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    val positions = val::global("Float32Array").new_(maxPts * 3);
    val vectors   = val::global("Float32Array").new_(maxPts * 3);

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                auto fd = rb.force_diag_at(x, y, z);
                auto f = fd.f_coulomb + fd.f_gravity + fd.f_magnetic;
                double mag = f.mag();
                if (mag < 1e-15) continue;
                positions.set(count * 3,     static_cast<float>(x));
                positions.set(count * 3 + 1, static_cast<float>(y));
                positions.set(count * 3 + 2, static_cast<float>(z));
                vectors.set(count * 3,       static_cast<float>(f.x));
                vectors.set(count * 3 + 1,   static_cast<float>(f.y));
                vectors.set(count * 3 + 2,   static_cast<float>(f.z));
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("vectors", vectors);
    result.set("count", count);
    return result;
}

// ── Scenario setup ───────────────────────────────────────────────────
void setup_scenario(ftd::RenderBridge& rb, const std::string& name) {
    const int N = rb.lattice().size();
    const int mid = N / 2;

    // ── Scale 0: Flux-only scenarios (pure substrate, no particles) ──

    if (name == "empty") {
        // Nothing to inject
    } else if (name == "flux-pulse") {
        // Gaussian flux pulse at center — watch spherical wave propagation
        double amp = ftd::K_B * 0.8;
        double sigma = 3.0;
        for (int dx = -5; dx <= 5; ++dx) {
            for (int dy = -5; dy <= 5; ++dy) {
                for (int dz = -5; dz <= 5; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                    }
                }
            }
        }
    } else if (name == "flux-dipole") {
        // Two opposite flux injections — watch interference pattern
        double amp = ftd::K_B * 0.6;
        double sigma = 2.5;
        int off = N / 6;
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                for (int dz = -4; dz <= 4; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid-off+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                        rb.inject_flux(mid+off+dx, mid+dy, mid+dz, ftd::Vec3(-g, 0, 0));
                    }
                }
            }
        }
    } else if (name == "flux-standing") {
        // Two counter-propagating wave packets creating standing wave
        double amp = ftd::K_B * 0.5;
        double sigma = 2.5;
        int off = N / 4;
        // Left-moving pulse (give it wave_vel to the right)
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dz = -2; dz <= 2; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid-off+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                        rb.inject_flux(mid+off+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                    }
                }
            }
        }
    } else if (name == "flux-dispersion") {
        // Sharp impulse at center — demonstrates lattice dispersion
        // Single-site delta function
        double amp = ftd::K_B * 2.0;
        rb.inject_flux(mid, mid, mid, ftd::Vec3(amp, 0, 0));
    } else if (name == "flux-soliton") {
        // Large amplitude pulse — explore nonlinear regime
        double amp = ftd::K_B * 3.0;
        double sigma = 2.0;
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                for (int dz = -4; dz <= 4; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid+dx, mid+dy, mid+dz, ftd::Vec3(g, g*0.5, 0));
                    }
                }
            }
        }
    } else if (name == "flux-cascade") {
        // Genesis threshold demo: inject above K_B, watch spontaneous manifestation
        // Enable genesis for this scenario
        double amp = ftd::K_GENESIS * 1.2;  // above genesis threshold
        double sigma = 3.0;
        for (int dx = -5; dx <= 5; ++dx) {
            for (int dy = -5; dy <= 5; ++dy) {
                for (int dz = -5; dz <= 5; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                    }
                }
            }
        }
    } else if (name == "flux-damping") {
        // Demonstrate damping: two pulses, one near particle (selective), one in vacuum
        double amp = ftd::K_B * 0.6;
        double sigma = 2.5;
        int off = N / 4;
        for (int dx = -4; dx <= 4; ++dx) {
            for (int dy = -4; dy <= 4; ++dy) {
                for (int dz = -4; dz <= 4; ++dz) {
                    double r2 = dx*dx + dy*dy + dz*dz;
                    double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                    if (g > amp * 0.01) {
                        rb.inject_flux(mid-off+dx, mid+dy, mid+dz, ftd::Vec3(g, 0, 0));
                        rb.inject_flux(mid+off+dx, mid+dy, mid+dz, ftd::Vec3(0, g, 0));
                    }
                }
            }
        }
    } else if (name == "flux-ring") {
        // Ring of flux injections creating circular wave pattern in XZ plane
        double amp = ftd::K_B * 0.5;
        int radius = N / 4;
        int nPoints = 16;
        for (int i = 0; i < nPoints; ++i) {
            double angle = 2.0 * 3.14159265358979 * i / nPoints;
            int rx = mid + static_cast<int>(radius * std::cos(angle));
            int rz = mid + static_cast<int>(radius * std::sin(angle));
            // Flux pointing radially inward
            double fx = -amp * std::cos(angle);
            double fz = -amp * std::sin(angle);
            rb.inject_flux(rx, mid, rz, ftd::Vec3(fx, 0, fz));
        }
    } else if (name == "flux-interference") {
        // 4 coherent sources → constructive/destructive pattern
        double amp = ftd::K_B * 2.0;
        int q = N / 4;
        int sources[][3] = {
            {mid - q, mid, mid - q},
            {mid + q, mid, mid - q},
            {mid - q, mid, mid + q},
            {mid + q, mid, mid + q},
        };
        for (int s = 0; s < 4; ++s) {
            for (int dz = -4; dz <= 4; ++dz)
            for (int dy = -4; dy <= 4; ++dy)
            for (int dx = -4; dx <= 4; ++dx) {
                double r2 = dx*dx + dy*dy + dz*dz;
                double val = amp * 1.5 * std::exp(-r2 / 12.0);
                if (val > 0.001)
                    rb.inject_flux(sources[s][0]+dx, sources[s][1]+dy, sources[s][2]+dz,
                                   ftd::Vec3(val, 0, 0));
            }
        }
    } else if (name == "flux-vortex") {
        // Circular-polarized flux ring → curl-dominated structure (spin origin)
        double amp = ftd::K_B * 2.0;
        int vRadius = N / 5;
        int nV = 24;
        double pi = 3.14159265358979;
        for (int i = 0; i < nV; ++i) {
            double angle = (2.0 * pi * i) / nV;
            int rx = mid + static_cast<int>(std::round(vRadius * std::cos(angle)));
            int rz = mid + static_cast<int>(std::round(vRadius * std::sin(angle)));
            // Tangential flux + upward component for helicity
            double tX = -std::sin(angle) * amp * 2.0;
            double tZ =  std::cos(angle) * amp * 2.0;
            double tY = amp * 0.5;
            rb.inject_flux(rx, mid,     rz, ftd::Vec3(tX, tY, tZ));
            rb.inject_flux(rx, mid + 1, rz, ftd::Vec3(tX*0.5, tY*0.5, tZ*0.5));
            rb.inject_flux(rx, mid - 1, rz, ftd::Vec3(tX*0.5, -tY*0.5, tZ*0.5));
        }
    } else if (name == "flux-collision") {
        // Two ±1 particles on collision course with flux dressing
        double amp = ftd::K_B * 2.0;
        int off = N / 3;
        rb.inject_wavepacket(mid - off, mid, mid, 1);
        rb.inject_wavepacket(mid + off, mid, mid, -1);
        // Give them flux push toward each other
        for (int d = -3; d <= 3; ++d)
        for (int dy = -3; dy <= 3; ++dy)
        for (int dx = -3; dx <= 3; ++dx) {
            double r2 = dx*dx + dy*dy + d*d;
            double val = amp * std::exp(-r2 / 8.0);
            if (val > 0.001) {
                rb.inject_flux(mid - off + dx, mid + dy, mid + d, ftd::Vec3( val, 0, 0));
                rb.inject_flux(mid + off + dx, mid + dy, mid + d, ftd::Vec3(-val, 0, 0));
            }
        }
    } else if (name == "flux-pair-production") {
        // Super-threshold flux burst → spontaneous ±1 pair genesis
        double bigAmp = ftd::K_GENESIS * 5.0;
        for (int dz = -4; dz <= 4; ++dz)
        for (int dy = -4; dy <= 4; ++dy)
        for (int dx = -4; dx <= 4; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            double val = bigAmp * std::exp(-r2 / 12.0);
            if (val > 0.001)
                rb.inject_flux(mid+dx, mid+dy, mid+dz,
                               ftd::Vec3(val, val*0.7, val*0.3));
        }
    } else if (name == "flux-hydrogen") {
        // Locked +1 proton at center + free -1 electron nearby
        double amp = ftd::K_B * 2.0;
        rb.inject_wavepacket(mid, mid, mid, 1);
        rb.inject_particle(mid + 6, mid, mid, -1, ftd::Vec3(0, 0, 0));
        // Seed flux as Coulomb-like dressing around proton
        for (int dz = -5; dz <= 5; ++dz)
        for (int dy = -5; dy <= 5; ++dy)
        for (int dx = -5; dx <= 5; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            if (r2 == 0 || r2 > 36) continue;
            double r = std::sqrt(r2);
            double val = amp * 0.5 / r;
            rb.inject_flux(mid+dx, mid+dy, mid+dz,
                           ftd::Vec3(val*dx/r, val*dy/r, val*dz/r));
        }
    } else if (name == "flux-gravity-cluster") {
        // Many same-sign particles for gravity clustering
        double amp = ftd::K_B * 2.0;
        int spread = N / 3;
        std::mt19937 rng(42);
        std::uniform_real_distribution<double> dist(-0.5, 0.5);
        for (int i = 0; i < 12; ++i) {
            int px = mid + static_cast<int>(std::round(dist(rng) * spread));
            int py = mid + static_cast<int>(std::round(dist(rng) * spread));
            int pz = mid + static_cast<int>(std::round(dist(rng) * spread));
            rb.inject_wavepacket(px, py, pz, 1);
        }
        // Background flux
        for (int dz = -4; dz <= 4; ++dz)
        for (int dy = -4; dy <= 4; ++dy)
        for (int dx = -4; dx <= 4; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            double val = amp * 0.5 * std::exp(-r2 / 18.0);
            if (val > 0.001)
                rb.inject_flux(mid+dx, mid+dy, mid+dz, ftd::Vec3(val, val*0.3, 0));
        }
    } else if (name == "flux-random-genesis") {
        // Random super-threshold flux patches → stochastic particle creation
        std::mt19937 rng(123);
        std::uniform_int_distribution<int> posDist(4, N - 5);
        std::uniform_real_distribution<double> ampDist(0.8, 1.6);
        std::uniform_real_distribution<double> dirDist(-0.5, 0.5);
        double threshold = ftd::K_GENESIS * 2.5;
        for (int p = 0; p < 8; ++p) {
            int cx = posDist(rng), cy = posDist(rng), cz = posDist(rng);
            double pAmp = threshold * ampDist(rng);
            for (int dz = -2; dz <= 2; ++dz)
            for (int dy = -2; dy <= 2; ++dy)
            for (int dx = -2; dx <= 2; ++dx) {
                double r2 = dx*dx + dy*dy + dz*dz;
                double val = pAmp * std::exp(-r2 / 6.0);
                if (val > 0.001)
                    rb.inject_flux(cx+dx, cy+dy, cz+dz,
                                   ftd::Vec3(dirDist(rng)*val, dirDist(rng)*val, dirDist(rng)*val));
            }
        }
    } else if (name == "flux-dual-substrate") {
        // L/R chirality demo — two offset pulses
        double amp = ftd::K_B * 2.0;
        int off = N / 4;
        for (int dz = -5; dz <= 5; ++dz)
        for (int dy = -5; dy <= 5; ++dy)
        for (int dx = -5; dx <= 5; ++dx) {
            double r2 = dx*dx + dy*dy + dz*dz;
            double val = amp * 1.5 * std::exp(-r2 / 16.0);
            if (val > 0.001) {
                rb.inject_flux(mid - off + dx, mid + dy, mid + dz,
                               ftd::Vec3(val, val*0.5, -val*0.3));
                rb.inject_flux(mid + off + dx, mid + dy, mid + dz,
                               ftd::Vec3(val, -val*0.5, val*0.3));
            }
        }
    }

    // ── Light & Color scenarios ──────────────────────────────────────

    else if (name == "light-rainbow") {
        // Three traveling waves at different frequencies = "colors"
        // Red (n=1, y-pol), Green (n=3, z-pol), Blue (n=6, x-pol)
        // Orthogonal polarizations so they don't interfere
        double amp = 0.15;
        double pi = 3.14159265358979;
        struct ColorWave { int n; int pol; };  // pol: 0=x, 1=y, 2=z
        ColorWave waves[] = {{1, 1}, {3, 2}, {6, 0}};  // red, green, blue
        for (int w = 0; w < 3; ++w) {
            double k = 2.0 * pi * waves[w].n / N;
            double omega = 2.0 * ftd::C_WAVE * std::sin(k / 2.0);
            for (int x = 0; x < N; ++x)
            for (int y = 0; y < N; ++y)
            for (int z = 0; z < N; ++z) {
                int idx = rb.lattice().index(x, y, z);
                double J_val = amp * std::sin(k * x);
                double wv_val = -omega * amp * std::cos(k * x);
                switch (waves[w].pol) {
                    case 0: rb.voxels()[idx].flux.x += J_val; rb.voxels()[idx].wave_vel.x += wv_val; break;
                    case 1: rb.voxels()[idx].flux.y += J_val; rb.voxels()[idx].wave_vel.y += wv_val; break;
                    case 2: rb.voxels()[idx].flux.z += J_val; rb.voxels()[idx].wave_vel.z += wv_val; break;
                }
            }
        }
    } else if (name == "light-prism") {
        // Delta-function pulse at center — contains ALL frequencies
        // Dispersive broadening separates them like a prism
        double amp = 0.4;
        for (int y = 0; y < N; ++y)
        for (int z = 0; z < N; ++z) {
            int idx = rb.lattice().index(mid, y, z);
            rb.voxels()[idx].flux.z = amp;
            rb.voxels()[idx].wave_vel.z = amp;  // outgoing in +x
        }
    } else if (name == "light-dipole") {
        // Gaussian z-directed pulse → classical sin²θ radiation pattern
        double amp = 0.5;
        double sigma = 3.0;
        for (int x = 0; x < N; ++x)
        for (int y = 0; y < N; ++y)
        for (int z = 0; z < N; ++z) {
            double dx = x - mid, dy = y - mid, dz = z - mid;
            double r2 = dx * dx + dy * dy + dz * dz;
            double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            int idx = rb.lattice().index(x, y, z);
            rb.voxels()[idx].flux.z += g;
            rb.voxels()[idx].wave_vel.z += g;
        }
    } else if (name == "light-two-slit") {
        // Two coherent line sources in z-direction, offset in y
        // Creates interference fringes in the far field
        double amp = 0.3;
        double sigma = 2.0;
        int slit_sep = N / 6;  // separation between slits
        int slit_x = N / 4;    // slits at x = L/4
        int slit_y[] = { mid - slit_sep, mid + slit_sep };
        for (int s = 0; s < 2; ++s) {
            for (int z = 0; z < N; ++z) {
                for (int dy = -4; dy <= 4; ++dy) {
                    for (int dx = -4; dx <= 4; ++dx) {
                        double r2 = dx * dx + dy * dy;
                        double g = amp * std::exp(-r2 / (2.0 * sigma * sigma));
                        if (g < 1e-6) continue;
                        int px = slit_x + dx;
                        int py = slit_y[s] + dy;
                        if (px < 0 || px >= N || py < 0 || py >= N) continue;
                        int idx = rb.lattice().index(px, py, z);
                        rb.voxels()[idx].flux.z += g;
                        rb.voxels()[idx].wave_vel.x += g;  // propagate in +x
                    }
                }
            }
        }
    } else if (name == "light-photon-race") {
        // Two Gaussian pulses: dim (A=0.05) and bright (A=0.5)
        // Both travel at exactly the same speed → linearity of wave equation
        double sigma = 3.0;
        int x_start = N / 4;
        double amps[] = {0.05, 0.5};
        int y_offsets[] = {mid - N / 6, mid + N / 6};
        for (int p = 0; p < 2; ++p) {
            for (int x = 0; x < N; ++x) {
                double dx = x - x_start;
                double g = amps[p] * std::exp(-dx * dx / (2.0 * sigma * sigma));
                if (g < 1e-8) continue;
                for (int y = y_offsets[p] - 2; y <= y_offsets[p] + 2; ++y)
                for (int z = mid - 2; z <= mid + 2; ++z) {
                    if (y < 0 || y >= N || z < 0 || z >= N) continue;
                    int idx = rb.lattice().index(x, y, z);
                    rb.voxels()[idx].flux.z += g;
                    rb.voxels()[idx].wave_vel.z += g;  // outgoing in +x
                }
            }
        }
    }

    // ── Legacy particle scenarios (kept for Scale 0 backward compat) ──

    else if (name == "pair") {
        rb.inject_wavepacket(mid, mid, mid, 1);
        rb.inject_particle(mid + 6, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "production") {
        for (int i = 0; i < 5; i++) {
            rb.inject_particle(4 + i, mid, mid, 1, ftd::Vec3(0, 0, 0));
            rb.inject_particle(N - 5 - i, mid, mid, -1, ftd::Vec3(0, 0, 0));
        }
    } else if (name == "interference") {
        int q = N / 4;
        rb.inject_wavepacket(q, q, mid, 1);
        rb.inject_wavepacket(N - q, q, mid, 1);
        rb.inject_wavepacket(q, N - q, mid, 1);
        rb.inject_wavepacket(N - q, N - q, mid, 1);
    } else if (name == "force") {
        rb.inject_wavepacket(mid, mid, mid, 1);
    } else if (name == "hydrogen") {
        rb.inject_particle(mid, mid, mid, 1, ftd::Vec3(0, 0, 0));
        rb.inject_particle(mid + 8, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "entangled") {
        rb.create_entangled_pair(mid, mid, mid, ftd::Vec3(ftd::K_B, 0, 0));
    } else if (name == "annihilation") {
        rb.inject_particle(mid - 3, mid, mid, 1, ftd::Vec3(0, 0, 0));
        rb.inject_particle(mid + 3, mid, mid, -1, ftd::Vec3(0, 0, 0));
    } else if (name == "triad") {
        rb.inject_wavepacket(mid, mid + 2, mid, 1);
        rb.inject_wavepacket(mid - 2, mid - 1, mid, 1);
        rb.inject_wavepacket(mid + 2, mid - 1, mid, 1);
    } else if (name == "dipole") {
        rb.inject_wavepacket(mid - 2, mid, mid, 1);
        rb.inject_wavepacket(mid + 2, mid, mid, -1);
        rb.voxel_at(mid - 2, mid, mid).locked = true;
        rb.voxel_at(mid + 2, mid, mid).locked = true;
    } else if (name == "scattering") {
        auto& v1 = rb.voxel_at(mid - 8, mid, mid);
        rb.inject_particle(mid - 8, mid, mid, 1, ftd::Vec3(0, 0, 0));
        v1.velocity = ftd::Vec3(0.3, 0.05, 0);
        auto& v2 = rb.voxel_at(mid + 8, mid, mid);
        rb.inject_particle(mid + 8, mid, mid, 1, ftd::Vec3(0, 0, 0));
        v2.velocity = ftd::Vec3(-0.3, -0.05, 0);
    } else if (name == "wave") {
        double amp = ftd::K_B * 0.8;
        rb.inject_flux(mid, mid, mid, ftd::Vec3(amp, 0, 0));
        rb.inject_flux(mid+1, mid, mid, ftd::Vec3(amp*0.6, 0, 0));
        rb.inject_flux(mid-1, mid, mid, ftd::Vec3(amp*0.6, 0, 0));
        rb.inject_flux(mid, mid+1, mid, ftd::Vec3(0, amp*0.6, 0));
        rb.inject_flux(mid, mid-1, mid, ftd::Vec3(0, amp*0.6, 0));
        rb.inject_flux(mid, mid, mid+1, ftd::Vec3(0, 0, amp*0.6));
        rb.inject_flux(mid, mid, mid-1, ftd::Vec3(0, 0, amp*0.6));
    } else if (name == "cluster") {
        int d = 3;
        for (int dx = -1; dx <= 1; dx += 2) {
            for (int dy = -1; dy <= 1; dy += 2) {
                for (int dz = -1; dz <= 1; dz += 2) {
                    int8_t st = ((dx + dy + dz) > 0) ? 1 : -1;
                    rb.inject_wavepacket(mid + dx * d, mid + dy * d, mid + dz * d, st);
                }
            }
        }
    } else if (name == "vacuum") {
        std::mt19937 rng(123);
        std::uniform_real_distribution<double> dist(-0.2, 0.2);
        double seed_amp = ftd::K_B * 0.3;
        for (int x = mid - 4; x <= mid + 4; ++x) {
            for (int y = mid - 4; y <= mid + 4; ++y) {
                for (int z = mid - 4; z <= mid + 4; ++z) {
                    rb.inject_flux(x, y, z, ftd::Vec3(
                        seed_amp * dist(rng),
                        seed_amp * dist(rng),
                        seed_amp * dist(rng)
                    ));
                }
            }
        }
    }
}

// ── Lattice info ────────────────────────────────────────────────────
int get_lattice_size(ftd::RenderBridge& rb) {
    return rb.lattice().size();
}

// ═══════════════════════════════════════════════════════════════════
// ParticleEngine (Scale 1) bindings
// ═══════════════════════════════════════════════════════════════════

// ── PE Particle Data Extraction ─────────────────────────────────────
// Returns positions + charge-based colors + mass-based sizes for Three.js
val get_pe_particle_data(ftd::ParticleEngine& pe) {
    const auto& particles = pe.particles();
    int count = static_cast<int>(particles.size());

    val positions = val::global("Float32Array").new_(count * 3);
    val colors    = val::global("Float32Array").new_(count * 3);
    val sizes     = val::global("Float32Array").new_(count);
    val charges   = val::global("Int8Array").new_(count);
    val ids       = val::global("Int32Array").new_(count);

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];

        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));

        // Default colors by charge (overridden by JS catalog lookup)
        if (p.charge > 0) {
            colors.set(i * 3,     0.29f);
            colors.set(i * 3 + 1, 0.87f);
            colors.set(i * 3 + 2, 0.50f);
        } else if (p.charge < 0) {
            colors.set(i * 3,     0.97f);
            colors.set(i * 3 + 1, 0.44f);
            colors.set(i * 3 + 2, 0.44f);
        } else {
            colors.set(i * 3,     0.60f);
            colors.set(i * 3 + 1, 0.60f);
            colors.set(i * 3 + 2, 0.70f);
        }

        // Size proportional to log(mass/m_e) + 1
        float s = 3.0f + 2.0f * static_cast<float>(std::log10(p.mass / ftd::K_B + 1.0));
        if (s > 12.0f) s = 12.0f;
        sizes.set(i, s);

        charges.set(i, static_cast<int>(p.charge));
        ids.set(i, p.id);
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("colors", colors);
    result.set("sizes", sizes);
    result.set("charges", charges);
    result.set("ids", ids);
    result.set("count", count);
    return result;
}

// ── PE Diagnostics ─────────────────────────────────────────────────
val get_pe_diagnostics(ftd::ParticleEngine& pe) {
    auto d = pe.diagnostics();
    val result = val::object();
    result.set("tick",           d.tick);
    result.set("particleCount",  d.particle_count);
    result.set("totalKE",        d.total_ke);
    result.set("totalPE",        d.total_pe);
    result.set("totalEnergy",    d.total_energy);
    result.set("momentumX",      d.total_momentum.x);
    result.set("momentumY",      d.total_momentum.y);
    result.set("momentumZ",      d.total_momentum.z);
    result.set("angMomX",        d.total_angular_momentum.x);
    result.set("angMomY",        d.total_angular_momentum.y);
    result.set("angMomZ",        d.total_angular_momentum.z);
    return result;
}

// ── PE Particle injection ──────────────────────────────────────────
int pe_add_particle(ftd::ParticleEngine& pe, int charge,
                    double x, double y, double z,
                    double vx, double vy, double vz,
                    double mass, double r_eff) {
    return pe.add_particle(static_cast<int8_t>(charge),
                           ftd::Vec3(x, y, z),
                           ftd::Vec3(vx, vy, vz),
                           mass, r_eff);
}

int pe_add_locked_particle(ftd::ParticleEngine& pe, int charge,
                            double x, double y, double z,
                            double mass, double r_eff) {
    int id = pe.add_locked_particle(static_cast<int8_t>(charge),
                                     ftd::Vec3(x, y, z), mass);
    // Override default r_eff (C++ default is 2.48, too large for atomic orbits)
    pe.particles().back().r_eff = r_eff;
    return id;
}

// ── PE Controls ────────────────────────────────────────────────────
void pe_set_dt(ftd::ParticleEngine& pe, double dt) { pe.set_dt(dt); }
double pe_get_dt(ftd::ParticleEngine& pe) { return pe.dt(); }
void pe_set_softening(ftd::ParticleEngine& pe, double s) { pe.set_softening(s); }
void pe_set_damping(ftd::ParticleEngine& pe, bool e) { pe.set_damping_enabled(e); }
void pe_set_gravity(ftd::ParticleEngine& pe, bool e) { pe.set_gravity_enabled(e); }
int pe_particle_count(ftd::ParticleEngine& pe) { return static_cast<int>(pe.particles().size()); }

void pe_clear(ftd::ParticleEngine& pe) {
    pe.particles().clear();
}

// ── PE Toggle getter/setter (generic, by name) ────────────────────
void pe_set_toggle(ftd::ParticleEngine& pe, const std::string& name, bool val) {
    auto& t = pe.toggles;
    if      (name == "coulomb")         t.coulomb = val;
    else if (name == "gravity")         t.gravity = val;
    else if (name == "damping")         t.damping = val;
    else if (name == "lorentz")         t.lorentz = val;
    else if (name == "exchange")        t.exchange = val;
    else if (name == "strong")          t.strong = val;
    else if (name == "radiation")       t.radiation = val;
    else if (name == "spin_orbit")      t.spin_orbit = val;
    else if (name == "relativistic")    t.relativistic = val;
    else if (name == "magnetic_dipole") t.magnetic_dipole = val;
}

bool pe_get_toggle(ftd::ParticleEngine& pe, const std::string& name) {
    const auto& t = pe.toggles;
    if      (name == "coulomb")         return t.coulomb;
    else if (name == "gravity")         return t.gravity;
    else if (name == "damping")         return t.damping;
    else if (name == "lorentz")         return t.lorentz;
    else if (name == "exchange")        return t.exchange;
    else if (name == "strong")          return t.strong;
    else if (name == "radiation")       return t.radiation;
    else if (name == "spin_orbit")      return t.spin_orbit;
    else if (name == "relativistic")    return t.relativistic;
    else if (name == "magnetic_dipole") return t.magnetic_dipole;
    return false;
}

// ── PE Force Diagnostic ───────────────────────────────────────────
val get_pe_force_diag(ftd::ParticleEngine& pe, int idx) {
    val result = val::object();
    const auto& fd = pe.force_diag();
    if (idx < 0 || idx >= static_cast<int>(fd.size())) return result;
    const auto& d = fd[idx];
    result.set("coulomb_x", d.f_coulomb.x); result.set("coulomb_y", d.f_coulomb.y); result.set("coulomb_z", d.f_coulomb.z);
    result.set("gravity_x", d.f_gravity.x); result.set("gravity_y", d.f_gravity.y); result.set("gravity_z", d.f_gravity.z);
    result.set("lorentz_x", d.f_lorentz.x); result.set("lorentz_y", d.f_lorentz.y); result.set("lorentz_z", d.f_lorentz.z);
    result.set("exchange_x", d.f_exchange.x); result.set("exchange_y", d.f_exchange.y); result.set("exchange_z", d.f_exchange.z);
    result.set("strong_x", d.f_strong.x); result.set("strong_y", d.f_strong.y); result.set("strong_z", d.f_strong.z);
    auto tot = d.total();
    result.set("total_x", tot.x); result.set("total_y", tot.y); result.set("total_z", tot.z);
    return result;
}

// ═══════════════════════════════════════════════════════════════════
// AtomEngine (Scale 2) bindings
// ═══════════════════════════════════════════════════════════════════

// CPK element colors: H=white, He=cyan, C=dark gray, N=blue, O=red, etc.
static void cpk_color(int Z, float& r, float& g, float& b) {
    switch (Z) {
        case  1: r=1.00f; g=1.00f; b=1.00f; break;  // H  — white
        case  2: r=0.85f; g=1.00f; b=1.00f; break;  // He — cyan
        case  3: r=0.80f; g=0.50f; b=1.00f; break;  // Li — violet
        case  4: r=0.76f; g=1.00f; b=0.00f; break;  // Be — dark yellow-green
        case  5: r=1.00f; g=0.71f; b=0.71f; break;  // B  — salmon
        case  6: r=0.56f; g=0.56f; b=0.56f; break;  // C  — dark gray
        case  7: r=0.19f; g=0.31f; b=0.97f; break;  // N  — blue
        case  8: r=1.00f; g=0.05f; b=0.05f; break;  // O  — red
        case  9: r=0.56f; g=0.88f; b=0.31f; break;  // F  — green
        case 10: r=0.70f; g=0.89f; b=0.96f; break;  // Ne — light cyan
        case 11: r=0.67f; g=0.36f; b=0.95f; break;  // Na — purple
        case 12: r=0.54f; g=1.00f; b=0.00f; break;  // Mg — green
        case 13: r=0.75f; g=0.65f; b=0.65f; break;  // Al — gray
        case 14: r=0.94f; g=0.78f; b=0.63f; break;  // Si — tan
        case 15: r=1.00f; g=0.50f; b=0.00f; break;  // P  — orange
        case 16: r=1.00f; g=1.00f; b=0.19f; break;  // S  — yellow
        case 17: r=0.12f; g=0.94f; b=0.12f; break;  // Cl — green
        case 18: r=0.50f; g=0.82f; b=0.89f; break;  // Ar — light blue
        default: r=0.70f; g=0.70f; b=0.70f; break;  // Unknown — light gray
    }
}

// ── AE Atom Data Extraction ─────────────────────────────────────────
val get_ae_atom_data(ftd::AtomEngine& ae) {
    const auto& atoms = ae.atoms();
    int count = static_cast<int>(atoms.size());

    val positions  = val::global("Float32Array").new_(count * 3);
    val colors     = val::global("Float32Array").new_(count * 3);
    val sizes      = val::global("Float32Array").new_(count);
    val atomicNums = val::global("Int32Array").new_(count);
    val charges    = val::global("Int32Array").new_(count);
    val ids        = val::global("Int32Array").new_(count);

    // Count bonds for bond index array
    int total_bonds = 0;
    for (const auto& a : atoms) {
        for (const auto& b : a.bonds) {
            if (b.partner_id > a.id) total_bonds++;  // avoid double-counting
        }
    }
    val bonds = val::global("Int32Array").new_(total_bonds * 2);

    for (int i = 0; i < count; ++i) {
        const auto& a = atoms[i];

        positions.set(i * 3,     static_cast<float>(a.position.x));
        positions.set(i * 3 + 1, static_cast<float>(a.position.y));
        positions.set(i * 3 + 2, static_cast<float>(a.position.z));

        float cr, cg, cb;
        cpk_color(a.Z, cr, cg, cb);
        colors.set(i * 3,     cr);
        colors.set(i * 3 + 1, cg);
        colors.set(i * 3 + 2, cb);

        // Size proportional to atomic radius (log scale for visibility)
        float s = 4.0f + 3.0f * static_cast<float>(std::log10(a.radius + 1.0));
        if (s > 15.0f) s = 15.0f;
        sizes.set(i, s);

        atomicNums.set(i, a.Z);
        charges.set(i, a.charge);
        ids.set(i, a.id);
    }

    // Fill bond index pairs
    int bi = 0;
    for (const auto& a : atoms) {
        for (const auto& b : a.bonds) {
            if (b.partner_id > a.id) {
                bonds.set(bi * 2,     a.id);
                bonds.set(bi * 2 + 1, b.partner_id);
                bi++;
            }
        }
    }

    val result = val::object();
    result.set("positions",  positions);
    result.set("colors",     colors);
    result.set("sizes",      sizes);
    result.set("atomicNums", atomicNums);
    result.set("charges",    charges);
    result.set("ids",        ids);
    result.set("bonds",      bonds);
    result.set("bondCount",  total_bonds);
    result.set("count",      count);
    return result;
}

// ── AE Diagnostics ─────────────────────────────────────────────────
val get_ae_diagnostics(ftd::AtomEngine& ae) {
    auto d = ae.diagnostics();
    val result = val::object();
    result.set("tick",          d.tick);
    result.set("atomCount",     d.atom_count);
    result.set("bondCount",     d.bond_count);
    result.set("totalKE",       d.total_ke);
    result.set("totalPEIonic",  d.total_pe_ionic);
    result.set("totalPEVdw",    d.total_pe_vdw);
    result.set("totalPEBond",   d.total_pe_bond);
    result.set("totalEnergy",   d.total_energy);
    result.set("momentumX",     d.total_momentum.x);
    result.set("momentumY",     d.total_momentum.y);
    result.set("momentumZ",     d.total_momentum.z);
    result.set("temperature",   d.temperature);
    return result;
}

// ── AE Atom injection ──────────────────────────────────────────────
int ae_add_atom(ftd::AtomEngine& ae, int Z,
                double x, double y, double z,
                double vx, double vy, double vz,
                int charge, int N) {
    return ae.add_atom(Z, ftd::Vec3(x, y, z), ftd::Vec3(vx, vy, vz), charge, N);
}

int ae_add_locked_atom(ftd::AtomEngine& ae, int Z,
                        double x, double y, double z,
                        int charge, int N) {
    return ae.add_locked_atom(Z, ftd::Vec3(x, y, z), charge, N);
}

int ae_create_bond(ftd::AtomEngine& ae, int id_a, int id_b, int order) {
    ae.create_bond(id_a, id_b, order);
    return 0;
}

// ── AE Controls ────────────────────────────────────────────────────
void ae_set_dt(ftd::AtomEngine& ae, double dt) { ae.set_dt(dt); }
double ae_get_dt(ftd::AtomEngine& ae) { return ae.dt(); }
void ae_set_softening(ftd::AtomEngine& ae, double s) { ae.set_softening(s); }
void ae_set_damping(ftd::AtomEngine& ae, bool e) { ae.set_damping_enabled(e); }
void ae_set_bonding(ftd::AtomEngine& ae, bool e) { ae.set_bonding_enabled(e); }
int ae_atom_count(ftd::AtomEngine& ae) { return static_cast<int>(ae.atoms().size()); }

void ae_clear(ftd::AtomEngine& ae) {
    ae.atoms().clear();
}

// ── AE Toggle getter/setter (generic, by name) ────────────────────
void ae_set_toggle(ftd::AtomEngine& ae, const std::string& name, bool val) {
    auto& t = ae.toggles;
    if      (name == "ionic")            t.ionic = val;
    else if (name == "van_der_waals")    t.van_der_waals = val;
    else if (name == "covalent_bonds")   t.covalent_bonds = val;
    else if (name == "auto_bonding")     t.auto_bonding = val;
    else if (name == "damping")          t.damping = val;
    else if (name == "h_bonds")          t.h_bonds = val;
    else if (name == "dipole_dipole")    t.dipole_dipole = val;
    else if (name == "angle_strain")     t.angle_strain = val;
    else if (name == "torsional")        t.torsional = val;
    else if (name == "thermostat")       t.thermostat = val;
    else if (name == "electronegativity") t.electronegativity = val;
}

bool ae_get_toggle(ftd::AtomEngine& ae, const std::string& name) {
    const auto& t = ae.toggles;
    if      (name == "ionic")            return t.ionic;
    else if (name == "van_der_waals")    return t.van_der_waals;
    else if (name == "covalent_bonds")   return t.covalent_bonds;
    else if (name == "auto_bonding")     return t.auto_bonding;
    else if (name == "damping")          return t.damping;
    else if (name == "h_bonds")          return t.h_bonds;
    else if (name == "dipole_dipole")    return t.dipole_dipole;
    else if (name == "angle_strain")     return t.angle_strain;
    else if (name == "torsional")        return t.torsional;
    else if (name == "thermostat")       return t.thermostat;
    else if (name == "electronegativity") return t.electronegativity;
    return false;
}

// ── AE Force Diagnostic ───────────────────────────────────────────
val get_ae_force_diag(ftd::AtomEngine& ae, int idx) {
    val result = val::object();
    const auto& fd = ae.force_diag();
    if (idx < 0 || idx >= static_cast<int>(fd.size())) return result;
    const auto& d = fd[idx];
    result.set("ionic_x", d.f_ionic.x); result.set("ionic_y", d.f_ionic.y); result.set("ionic_z", d.f_ionic.z);
    result.set("vdw_x", d.f_vdw.x); result.set("vdw_y", d.f_vdw.y); result.set("vdw_z", d.f_vdw.z);
    result.set("bond_x", d.f_bond.x); result.set("bond_y", d.f_bond.y); result.set("bond_z", d.f_bond.z);
    result.set("hbond_x", d.f_hbond.x); result.set("hbond_y", d.f_hbond.y); result.set("hbond_z", d.f_hbond.z);
    result.set("dipole_x", d.f_dipole.x); result.set("dipole_y", d.f_dipole.y); result.set("dipole_z", d.f_dipole.z);
    auto tot = d.total();
    result.set("total_x", tot.x); result.set("total_y", tot.y); result.set("total_z", tot.z);
    return result;
}

// ── Embind Registration ──────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(ftd_module) {
    class_<ftd::RenderBridge>("RenderBridge")
        .constructor<int>()
        .function("tick", &ftd::RenderBridge::tick)
        .function("run",  &ftd::RenderBridge::run)
        .function("currentTick", &ftd::RenderBridge::current_tick)
        ;

    // Data extraction
    function("getParticleData",    &get_particle_data);
    function("getDiagnostics",     &get_diagnostics);
    function("getEnergyAudit",     &get_energy_audit);
    function("getLagrangian",      &get_lagrangian);
    function("getConstants",       &get_constants);
    function("getLatticeSize",     &get_lattice_size);

    // Voxel inspection
    function("inspectVoxel",       &inspect_voxel);
    function("getForceAt",         &get_force_at);

    // Bulk flux extraction (for flux volume visualization)
    function("getFluxSlice",       &get_flux_slice);
    function("getFluxVolume",      &get_flux_volume);

    // Bulk sampled vector field exports (for field line / arrow visualization)
    function("getEFieldSampled",      &get_e_field_sampled);
    function("getBFieldSampled",      &get_b_field_sampled);
    function("getPoyntingSampled",    &get_poynting_sampled);
    function("getDivJSampled",        &get_divj_sampled);
    function("getFluxVectorSampled",  &get_flux_vector_sampled);
    function("getForceFieldSampled",  &get_force_field_sampled);

    // Controls
    function("setToggle",          &set_toggle);
    function("getToggle",          &get_toggle);

    // Injection
    function("injectParticle",     &inject_particle_simple);
    function("injectParticleFull", &inject_particle_full);
    function("injectWavepacket",   &inject_wavepacket_simple);
    function("injectWavepacketFull", &inject_wavepacket_full);
    function("injectFlux",         &inject_flux);
    function("createEntangledPair", &create_entangled_pair);

    // Time step control
    function("setDt",              &set_dt);
    function("getDt",              &get_dt);
    function("getPhysicalTime",    &get_physical_time);

    // Scenarios
    function("setupScenario",      &setup_scenario);

    // ── ParticleEngine (Scale 1) ─────────────────────────────────
    class_<ftd::ParticleEngine>("ParticleEngine")
        .constructor<>()
        .function("tick", &ftd::ParticleEngine::tick)
        .function("run",  &ftd::ParticleEngine::run)
        .function("currentTick", &ftd::ParticleEngine::current_tick)
        ;

    function("getPEParticleData",   &get_pe_particle_data);
    function("getPEDiagnostics",    &get_pe_diagnostics);
    function("peAddParticle",       &pe_add_particle);
    function("peAddLockedParticle", &pe_add_locked_particle);
    function("peSetDt",             &pe_set_dt);
    function("peGetDt",             &pe_get_dt);
    function("peSetSoftening",      &pe_set_softening);
    function("peSetDamping",        &pe_set_damping);
    function("peSetGravity",        &pe_set_gravity);
    function("peSetToggle",         &pe_set_toggle);
    function("peGetToggle",         &pe_get_toggle);
    function("peGetForceDiag",      &get_pe_force_diag);
    function("peParticleCount",     &pe_particle_count);
    function("peClear",             &pe_clear);

    // ── AtomEngine (Scale 2) ────────────────────────────────────────
    class_<ftd::AtomEngine>("AtomEngine")
        .constructor<>()
        .function("tick", &ftd::AtomEngine::tick)
        .function("run",  &ftd::AtomEngine::run)
        .function("currentTick", &ftd::AtomEngine::current_tick)
        ;

    function("getAEAtomData",      &get_ae_atom_data);
    function("getAEDiagnostics",   &get_ae_diagnostics);
    function("aeAddAtom",          &ae_add_atom);
    function("aeAddLockedAtom",    &ae_add_locked_atom);
    function("aeCreateBond",       &ae_create_bond);
    function("aeSetDt",            &ae_set_dt);
    function("aeGetDt",            &ae_get_dt);
    function("aeSetSoftening",     &ae_set_softening);
    function("aeSetDamping",       &ae_set_damping);
    function("aeSetBonding",       &ae_set_bonding);
    function("aeSetToggle",        &ae_set_toggle);
    function("aeGetToggle",        &ae_get_toggle);
    function("aeGetForceDiag",     &get_ae_force_diag);
    function("aeAtomCount",        &ae_atom_count);
    function("aeClear",            &ae_clear);
}
