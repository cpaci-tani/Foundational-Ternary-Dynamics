/**
 * @file ftd_wasm.cpp
 * @brief Emscripten Embind bindings for the FTD engine — shared helpers.
 *
 * [EXTENDED] Exposes RenderBridge to JavaScript with typed-array extraction
 * for zero-copy GPU upload of particle data, plus full diagnostic access
 * (energy audit, Lagrangian constraints, voxel inspection, force
 * decomposition, spin/color statistics).
 *
 * Split layout (W1-W3 extraction, 2026-04-18):
 *   - ftd_wasm.cpp              — this file: typed-array helpers, constants,
 *                                  RenderBridge class_<> binding for tick/run
 *   - bindings_render_bridge.cpp — RenderBridge toggles, injection, scenarios
 *   - bindings_particle.cpp      — ParticleEngine (Scale 1)
 *   - bindings_atom.cpp          — AtomEngine (Scale 2)
 *
 * All four TUs register into the single Emscripten module `ftd_module`
 * via distinct `EMSCRIPTEN_BINDINGS(ftd_module_*)` blocks. Emscripten
 * links them into one .wasm regardless of the block name — the block
 * identifier is an internal token, not the JS module name.
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <algorithm>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/lagrangian.h"
#include "ftd/constants.h"
#include "bindings_internal.h"

using namespace emscripten;

namespace ftd_wasm_internal {

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
        // Voxel-center convention: particles render at world (x+0.5, y+0.5, z+0.5)
        // so they align with the wireframe crosshair (which draws at raw+0.5 —
        // see viewport/boundary-geometry.js buildCubeBoundary). Without this
        // offset, single-particle seeds like `s0-seed-quark` appeared half a
        // voxel low-and-right of the cube crosshair at even N (matches the
        // MockBridge JS path at wasm-bridge-dag.js:656-658 which already
        // applies the same +0.5f offset).
        pos_cache[o3]     = static_cast<float>(c.x) + 0.5f;
        pos_cache[o3 + 1] = static_cast<float>(c.y) + 0.5f;
        pos_cache[o3 + 2] = static_cast<float>(c.z) + 0.5f;

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

// ── Lattice info ────────────────────────────────────────────────────
int get_lattice_size(ftd::RenderBridge& rb) {
    return rb.lattice().size();
}

} // namespace ftd_wasm_internal

// ── Embind Registration ──────────────────────────────────────────────
// The RenderBridge class_<> itself registers here so the core tick/run
// surface ships alongside its constructor. All other RB helpers (toggles,
// injection, scenarios, diagnostics) register from the sibling binding
// TUs — see bindings_render_bridge.cpp, bindings_particle.cpp, and
// bindings_atom.cpp. Emscripten unions every EMSCRIPTEN_BINDINGS block
// into the final module.
//
// DagEngine binding intentionally removed (2026 consolidation sweep).
// The web engine always uses RenderBridge — see the binding above and
// the comment in web/js/wasm-bridge-dag.js explaining why. DagEngine
// is now an experimental C++-only data-structure prototype; exposing
// it through WASM would invite callers into an unfinished code path
// whose gauss_project / phase_forces / phase_movement are stubs.
//
// If you're reading this because you hit a missing-binding error:
// use RenderBridge. The API surface is identical for tick/run/etc.
EMSCRIPTEN_BINDINGS(ftd_module_core) {
    class_<ftd::RenderBridge>("RenderBridge")
        .constructor<int>()
        .function("tick", &ftd::RenderBridge::tick)
        .function("run",  &ftd::RenderBridge::run)
        .function("currentTick", &ftd::RenderBridge::current_tick)
        ;
}
