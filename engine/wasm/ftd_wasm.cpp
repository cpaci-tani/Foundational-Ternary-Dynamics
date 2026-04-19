/**
 * @file ftd_wasm.cpp
 * @brief Emscripten Embind bindings for the FTD engine.
 *
 * [EXTENDED] Exposes RenderBridge to JavaScript with typed-array extraction
 * for zero-copy GPU upload of particle data, plus full diagnostic
 * access (energy audit, Lagrangian constraints, voxel inspection,
 * force decomposition, spin/color statistics).
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <algorithm>
#include <unordered_map>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/particle_engine.h"
#include "ftd/atom_engine.h"
#include "ftd/constants.h"
#include "ftd/scenarios.h"  // ftd::dispatch_scenario — ported JS scenario library

using namespace emscripten;

// ── Particle Data Extraction ─────────────────────────────────────────
// Returns a JS object with Float32Array views for direct BufferAttribute upload.
// Format: { positions: Float32Array, colors: Float32Array, sizes: Float32Array, count: int }

val get_particle_data(ftd::RenderBridge& rb) {
    // PERF: zero-copy via typed_memory_view. Pre-fix this called val::set
    // 7x per particle. Now positions/colors/sizes are heap views.
    static std::vector<float> pos_cache, col_cache, size_cache;
    const auto& voxels = rb.voxels();
    const int N = rb.lattice().size();
    const int total = N * N * N;

    // First pass: count visible voxels
    int count = 0;
    for (int i = 0; i < total; i++) {
        const auto& v = voxels[i];
        if (v.state != 0 || v.density() > ftd::K_B * 0.3) count++;
    }

    if (static_cast<int>(pos_cache.size()) < count * 3) {
        pos_cache.resize(count * 3);
        col_cache.resize(count * 3);
        size_cache.resize(count);
    }

    int idx = 0;
    for (int i = 0; i < total; i++) {
        const auto& v = voxels[i];
        if (v.state == 0 && v.density() <= ftd::K_B * 0.3) continue;

        auto c = rb.lattice().coord(i);
        const int o3 = idx * 3;
        pos_cache[o3]     = static_cast<float>(c.x);
        pos_cache[o3 + 1] = static_cast<float>(c.y);
        pos_cache[o3 + 2] = static_cast<float>(c.z);

        if (v.state == 1) {
            col_cache[o3]     = 0.29f;
            col_cache[o3 + 1] = 0.87f;
            col_cache[o3 + 2] = 0.50f;
        } else if (v.state == -1) {
            col_cache[o3]     = 0.97f;
            col_cache[o3 + 1] = 0.44f;
            col_cache[o3 + 2] = 0.44f;
        } else {
            float brightness = static_cast<float>(v.density() / (ftd::K_B * 2.0));
            if (brightness > 1.0f) brightness = 1.0f;
            col_cache[o3]     = 0.37f + brightness * 0.1f;
            col_cache[o3 + 1] = 0.45f + brightness * 0.1f;
            col_cache[o3 + 2] = 0.58f + brightness * 0.2f;
        }

        if (v.state != 0) {
            size_cache[idx] = 6.0f;
        } else {
            float s = 1.5f + static_cast<float>(v.density() / ftd::K_B) * 3.0f;
            if (s > 5.0f) s = 5.0f;
            size_cache[idx] = s;
        }
        idx++;
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("colors",    val(typed_memory_view(count * 3, col_cache.data())));
    result.set("sizes",     val(typed_memory_view(count,     size_cache.data())));
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
    result.set("ALPHA_EFT",   ftd::ALPHA_EFT);  // EFT-derived: = G_C * G_C (compile-time assert in constants.h)
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
// Pointer-to-member map for TermToggles (RenderBridge).
using RbBoolPTM = bool ftd::TermToggles::*;
static const std::unordered_map<std::string, RbBoolPTM>& rb_toggle_map() {
    static const std::unordered_map<std::string, RbBoolPTM> kMap = {
        {"wave_propagation",  &ftd::TermToggles::wave_propagation},
        {"coupling",          &ftd::TermToggles::coupling},
        {"damping",           &ftd::TermToggles::damping},
        {"genesis",           &ftd::TermToggles::genesis},
        {"gauss_projection",  &ftd::TermToggles::gauss_projection},
        {"forces",            &ftd::TermToggles::forces},
        {"gravity",           &ftd::TermToggles::gravity},
        {"poisson_coulomb",   &ftd::TermToggles::poisson_coulomb},
        {"movement",          &ftd::TermToggles::movement},
        {"lorentz_force",     &ftd::TermToggles::lorentz_force},
        {"selective_damping", &ftd::TermToggles::selective_damping},
        {"larmor_radiation",  &ftd::TermToggles::larmor_radiation},
        {"dual_substrate",    &ftd::TermToggles::dual_substrate},
        {"color_forces",      &ftd::TermToggles::color_forces},
        {"weak_transmutation",&ftd::TermToggles::weak_transmutation},
        {"strong_force",      &ftd::TermToggles::strong_force},
        {"triad_binding",     &ftd::TermToggles::triad_binding},
        {"pair_production",   &ftd::TermToggles::pair_production},
        {"exchange_force",    &ftd::TermToggles::exchange_force},
        {"latency_field",     &ftd::TermToggles::latency_field},
        {"emergent_forces",   &ftd::TermToggles::emergent_forces},
    };
    return kMap;
}

void set_toggle(ftd::RenderBridge& rb, const std::string& name, bool value) {
    auto it = rb_toggle_map().find(name);
    if (it != rb_toggle_map().end()) rb.toggles.*(it->second) = value;
}

bool get_toggle(ftd::RenderBridge& rb, const std::string& name) {
    auto it = rb_toggle_map().find(name);
    if (it != rb_toggle_map().end()) return rb.toggles.*(it->second);
    return false;
}

// ── Inject wrappers ──────────────────────────────────────────────────
void inject_particle_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_particle(x, y, z, static_cast<int8_t>(state), ftd::Vec3(0, 0, 0));
}

void inject_wavepacket_simple(ftd::RenderBridge& rb, int x, int y, int z, int state) {
    rb.inject_wavepacket(x, y, z, static_cast<int8_t>(state));
}

void inject_flux(ftd::RenderBridge& rb, int x, int y, int z,
                  double fx, double fy, double fz) {
    rb.inject_flux(x, y, z, ftd::Vec3(fx, fy, fz));
}

// Additive variant (+=) — used by the JS-ported scenarios that accumulate
// overlapping Gaussian kernels. Not exported to JS binding (only called
// internally from setup_scenario below via dispatch_scenario).
void inject_flux_add(ftd::RenderBridge& rb, int x, int y, int z,
                      double fx, double fy, double fz) {
    rb.inject_flux_add(x, y, z, ftd::Vec3(fx, fy, fz));
}

// Additive wave-velocity injection — used by s0-field-* and light-* ports
// that seed traveling waves via wave_vel directly. Not exported to JS.
void inject_wave_vel_add(ftd::RenderBridge& rb, int x, int y, int z,
                          double wx, double wy, double wz) {
    rb.inject_wave_vel_add(x, y, z, ftd::Vec3(wx, wy, wz));
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
    // PERF: zero-copy via typed_memory_view (was N^2 per-element crossings).
    static std::vector<double> cache;
    const int N = rb.lattice().size();
    const int sliceSize = N * N;
    const auto& voxels = rb.voxels();

    if (static_cast<int>(cache.size()) != sliceSize) cache.resize(sliceSize);

    for (int a = 0; a < N; ++a) {
        for (int b = 0; b < N; ++b) {
            int x, y, z;
            if (axis == 0)      { x = index; y = a; z = b; }
            else if (axis == 1) { x = a; y = index; z = b; }
            else                { x = a; y = b; z = index; }
            int idx = rb.lattice().index(x, y, z);
            cache[a * N + b] = voxels[idx].density();
        }
    }
    return val(typed_memory_view(sliceSize, cache.data()));
}

// getFluxVolume: returns Float64Array of all voxel flux magnitudes (N^3 values)
//
// PERF: zero-copy via typed_memory_view. Pre-fix this called val::set N^3 times
// per call (262K crossings at L=64). Now it's a single boundary crossing — the
// JS side gets a Float64Array view directly into the WASM heap.
//
// Lifetime contract: the returned view is valid until the next call to this
// function. JS callers must consume (or copy) before the next getFluxVolume().
// Currently safe because animateLattice consumes synchronously each frame.
val get_flux_volume(ftd::RenderBridge& rb) {
    static std::vector<double> cache;
    const int N = rb.lattice().size();
    const int total = N * N * N;
    const auto& voxels = rb.voxels();

    if (static_cast<int>(cache.size()) != total) cache.resize(total);
    for (int i = 0; i < total; ++i) {
        cache[i] = voxels[i].density();
    }
    return val(typed_memory_view(total, cache.data()));
}

// ── Bulk Sampled Vector Field Exports ────────────────────────────────
// Each returns { positions: Float32Array(N×3), vectors: Float32Array(N×3), count: int }
// stride controls spatial sampling density (stride=2 → every other voxel).

val get_e_field_sampled(ftd::RenderBridge& rb, int stride) {
    // PERF: zero-copy via typed_memory_view.
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto em = rb.em_field_at(idx);
                if (em.E_mag < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                vec_cache[o3]     = static_cast<float>(em.E.x);
                vec_cache[o3 + 1] = static_cast<float>(em.E.y);
                vec_cache[o3 + 2] = static_cast<float>(em.E.z);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

val get_b_field_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto em = rb.em_field_at(idx);
                if (em.B_mag < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                vec_cache[o3]     = static_cast<float>(em.B.x);
                vec_cache[o3 + 1] = static_cast<float>(em.B.y);
                vec_cache[o3 + 2] = static_cast<float>(em.B.z);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

val get_poynting_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                auto S_vec = rb.poynting_vector(idx);
                if (S_vec.mag() < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                vec_cache[o3]     = static_cast<float>(S_vec.x);
                vec_cache[o3 + 1] = static_cast<float>(S_vec.y);
                vec_cache[o3 + 2] = static_cast<float>(S_vec.z);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

val get_divj_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, val_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        val_cache.resize(maxPts);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                double div = rb.divergence_flux(idx);
                if (std::abs(div) < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                val_cache[count]  = static_cast<float>(div);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("values",    val(typed_memory_view(count,     val_cache.data())));
    result.set("count", count);
    return result;
}

val get_flux_vector_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;
    const auto& voxels = rb.voxels();

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                int idx = rb.lattice().index(x, y, z);
                const auto& v = voxels[idx];
                if (v.density() < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                vec_cache[o3]     = static_cast<float>(v.flux.x);
                vec_cache[o3 + 1] = static_cast<float>(v.flux.y);
                vec_cache[o3 + 2] = static_cast<float>(v.flux.z);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

val get_force_field_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                // Compute idx once instead of letting force_diag_at(x,y,z) re-resolve
                int idx = rb.lattice().index(x, y, z);
                const auto& fd = rb.force_diag_at(idx);
                auto f = fd.f_coulomb + fd.f_gravity + fd.f_magnetic;
                if (f.mag() < 1e-15) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x);
                pos_cache[o3 + 1] = static_cast<float>(y);
                pos_cache[o3 + 2] = static_cast<float>(z);
                vec_cache[o3]     = static_cast<float>(f.x);
                vec_cache[o3 + 1] = static_cast<float>(f.y);
                vec_cache[o3 + 2] = static_cast<float>(f.z);
                count++;
            }
        }
    }

    val result = val::object();
    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

// ── Scenario setup ───────────────────────────────────────────────────
// NOTE: Primary scenario dispatch path is ftd::dispatch_scenario(rb, name)
// (src/scenarios.cpp + include/ftd/scenarios.h) which owns every flux-*,
// light-*, quantum-*, s0-seed-*, s0-field-* scenario in the UI registry.
// The legacy switch below handles ONLY prefixless backward-compat names
// (pair, cluster, wave, hydrogen, scattering, annihilation, dipole,
// entangled, force, interference, vacuum, triad, production, empty) that
// the dispatcher does not prefix-match. These remain for older tests and
// saved dashboards.
//
// Removed 2026-04-18: 21 dead flux-*/light-* branches that were either
// (a) dispatcher-duplicates (pulse/dipole/standing/soliton/cascade/
// interference/vortex/pair-production/random-genesis/dual-substrate,
// light-rainbow/dipole/two-slit/photon-race) or (b) unreachable orphans
// whose prefix the dispatcher swallows with an unconditional return-true
// (light-prism, flux-collision/damping/dispersion/gravity-cluster/
// hydrogen/ring). None had tests or JS callers. See git log for the
// deleted bodies if you need to restore any as proper dispatcher entries.
//
// JS↔WASM parity: any scenario name in the UI registry (engine/web/js/
// scales/scale0/scenario-registry.js) produces identical physics on both
// backends. Single-source authority: src/scenarios.cpp (C++) + engine/
// web/js/bridge/scenarios/*.js (JS); both are hand-maintained mirrors.
void setup_scenario(ftd::RenderBridge& rb, const std::string& name) {
    // Primary path: ported JS scenario library (83 scenarios, flux-/light-/
    // quantum-/s0-seed-/s0-field-). Dispatcher returns true on prefix match.
    if (ftd::dispatch_scenario(rb, name)) return;

    const int N = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mid  = static_cast<int>(std::round(midF));
    const double sigma_base = N / 10.0;
    const int halfWidth = std::max(4, N / 4);

    // ── Legacy backward-compat scenarios (not in JS UI registry;
    // preserved for older tests and saved dashboards) ────────────────

    if (name == "empty") {
        // Nothing to inject
    } else if (name == "pair") {
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
// Pointer-to-member map for ParticleToggles.
using PeBoolPTM = bool ftd::ParticleToggles::*;
static const std::unordered_map<std::string, PeBoolPTM>& pe_toggle_map() {
    static const std::unordered_map<std::string, PeBoolPTM> kMap = {
        {"coulomb",         &ftd::ParticleToggles::coulomb},
        {"gravity",         &ftd::ParticleToggles::gravity},
        {"damping",         &ftd::ParticleToggles::damping},
        {"lorentz",         &ftd::ParticleToggles::lorentz},
        {"exchange",        &ftd::ParticleToggles::exchange},
        {"strong",          &ftd::ParticleToggles::strong},
        {"radiation",       &ftd::ParticleToggles::radiation},
        {"spin_orbit",      &ftd::ParticleToggles::spin_orbit},
        {"relativistic",    &ftd::ParticleToggles::relativistic},
        {"magnetic_dipole", &ftd::ParticleToggles::magnetic_dipole},
    };
    return kMap;
}

void pe_set_toggle(ftd::ParticleEngine& pe, const std::string& name, bool val) {
    auto it = pe_toggle_map().find(name);
    if (it != pe_toggle_map().end()) pe.toggles.*(it->second) = val;
}

bool pe_get_toggle(ftd::ParticleEngine& pe, const std::string& name) {
    auto it = pe_toggle_map().find(name);
    if (it != pe_toggle_map().end()) return pe.toggles.*(it->second);
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
// Pointer-to-member map for AtomToggles.
using AeBoolPTM = bool ftd::AtomToggles::*;
static const std::unordered_map<std::string, AeBoolPTM>& ae_toggle_map() {
    static const std::unordered_map<std::string, AeBoolPTM> kMap = {
        {"ionic",               &ftd::AtomToggles::ionic},
        {"van_der_waals",       &ftd::AtomToggles::van_der_waals},
        {"covalent_bonds",      &ftd::AtomToggles::covalent_bonds},
        {"auto_bonding",        &ftd::AtomToggles::auto_bonding},
        {"damping",             &ftd::AtomToggles::damping},
        {"h_bonds",             &ftd::AtomToggles::h_bonds},
        {"dipole_dipole",       &ftd::AtomToggles::dipole_dipole},
        {"angle_strain",        &ftd::AtomToggles::angle_strain},
        {"torsional",           &ftd::AtomToggles::torsional},
        {"improper_torsional",  &ftd::AtomToggles::improper_torsional},
        {"thermostat",          &ftd::AtomToggles::thermostat},
        {"electronegativity",   &ftd::AtomToggles::electronegativity},
    };
    return kMap;
}

void ae_set_toggle(ftd::AtomEngine& ae, const std::string& name, bool val) {
    auto it = ae_toggle_map().find(name);
    if (it != ae_toggle_map().end()) ae.toggles.*(it->second) = val;
}

bool ae_get_toggle(ftd::AtomEngine& ae, const std::string& name) {
    auto it = ae_toggle_map().find(name);
    if (it != ae_toggle_map().end()) return ae.toggles.*(it->second);
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

    // DagEngine binding intentionally removed (2026 consolidation sweep).
    // The web engine always uses RenderBridge — see the binding above and
    // the comment in web/js/wasm-bridge-dag.js explaining why. DagEngine
    // is now an experimental C++-only data-structure prototype; exposing
    // it through WASM would invite callers into an unfinished code path
    // whose gauss_project / phase_forces / phase_movement are stubs.
    //
    // If you're reading this because you hit a missing-binding error:
    // use RenderBridge. The API surface is identical for tick/run/etc.


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
    function("injectWavepacket",   &inject_wavepacket_simple);
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
