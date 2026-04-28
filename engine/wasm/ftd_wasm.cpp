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
        // offset, single-particle seeds like `s0-vacuum-electron` appeared half a
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

    // Poynting vector Σ S(v) = Σ E(v) × B(v) — exposed as
    // { x, y, z } so the JS `totalPoynting?.x` accessor in
    // telemetry-hub.js + the diagnostics-panel descriptor light up.
    val poynt = val::object();
    poynt.set("x", ea.total_poynting.x);
    poynt.set("y", ea.total_poynting.y);
    poynt.set("z", ea.total_poynting.z);
    result.set("totalPoynting",     poynt);

    // Dual-substrate diagnostics — only meaningful when the
    // `dual_substrate` toggle is on (C++ `compute_energy_audit`
    // gates the per-voxel accumulation on `rb.toggles.dual_substrate`,
    // so all four are 0 with the toggle off; emit the keys
    // unconditionally so the dashboard's descriptor lookup succeeds).
    result.set("ELTotal",           ea.E_L_total);
    result.set("ERTotal",           ea.E_R_total);
    result.set("wvLTotal",          ea.wv_L_total);
    result.set("wvRTotal",          ea.wv_R_total);
    result.set("chiralityTotal",    ea.chirality_total);

    // Strong / weak field energies (sub-channel sums).
    result.set("strongEnergy",      ea.strong_energy);
    result.set("weakEnergy",        ea.weak_energy);
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

// ── sample_v_at_ray ────────────────────────────────────────────────
// Trilinear interpolation of phi_coulomb_ along a 3D ray. Returns
// `{ positions:Float32Array(3N), V:Float32Array(N), count:N }` via
// zero-copy typed_memory_view, mirroring get_e_field_sampled. count=0
// when phi_coulomb_ is empty (Poisson toggle off, fresh sim before
// first solve). Replaces the JS-side getEFieldSampled + interp path
// in p1-observables-panel.js (~200μs marshalling) with a direct read
// (~3μs).
val sample_v_at_ray(
    ftd::RenderBridge& rb,
    double x1, double y1, double z1,
    double x2, double y2, double z2,
    int n)
{
    static std::vector<float> pos_cache, v_cache;
    if (n < 2) n = 2;
    if (n > 4096) n = 4096;
    const auto& phi = rb.phi_coulomb();
    val result = val::object();
    if (phi.empty()) {
        result.set("count", 0);
        return result;
    }
    if (static_cast<int>(pos_cache.size()) < n * 3) {
        pos_cache.resize(n * 3);
        v_cache.resize(n);
    }
    const int N = rb.lattice().size();
    auto wrap = [N](int v) {
        // Periodic wrap matching the lattice's index() convention
        return ((v % N) + N) % N;
    };
    auto sample_phi = [&](double x, double y, double z) -> double {
        // Trilinear interpolation with periodic wrap (matches engine's
        // periodic boundary conventions).
        const int x0 = static_cast<int>(std::floor(x));
        const int y0 = static_cast<int>(std::floor(y));
        const int z0 = static_cast<int>(std::floor(z));
        const double fx = x - x0;
        const double fy = y - y0;
        const double fz = z - z0;
        const int wx0 = wrap(x0), wx1 = wrap(x0 + 1);
        const int wy0 = wrap(y0), wy1 = wrap(y0 + 1);
        const int wz0 = wrap(z0), wz1 = wrap(z0 + 1);
        const auto idx = [&](int x, int y, int z) {
            return rb.lattice().index(x, y, z);
        };
        const double c000 = phi[idx(wx0, wy0, wz0)];
        const double c100 = phi[idx(wx1, wy0, wz0)];
        const double c010 = phi[idx(wx0, wy1, wz0)];
        const double c110 = phi[idx(wx1, wy1, wz0)];
        const double c001 = phi[idx(wx0, wy0, wz1)];
        const double c101 = phi[idx(wx1, wy0, wz1)];
        const double c011 = phi[idx(wx0, wy1, wz1)];
        const double c111 = phi[idx(wx1, wy1, wz1)];
        const double c00 = c000 * (1 - fx) + c100 * fx;
        const double c10 = c010 * (1 - fx) + c110 * fx;
        const double c01 = c001 * (1 - fx) + c101 * fx;
        const double c11 = c011 * (1 - fx) + c111 * fx;
        const double c0 = c00 * (1 - fy) + c10 * fy;
        const double c1 = c01 * (1 - fy) + c11 * fy;
        return c0 * (1 - fz) + c1 * fz;
    };
    for (int i = 0; i < n; ++i) {
        const double t = (n == 1) ? 0.0 : static_cast<double>(i) / (n - 1);
        const double x = x1 + (x2 - x1) * t;
        const double y = y1 + (y2 - y1) * t;
        const double z = z1 + (z2 - z1) * t;
        const int o3 = i * 3;
        pos_cache[o3]     = static_cast<float>(x);
        pos_cache[o3 + 1] = static_cast<float>(y);
        pos_cache[o3 + 2] = static_cast<float>(z);
        v_cache[i]        = static_cast<float>(sample_phi(x, y, z));
    }
    result.set("positions", val(typed_memory_view(n * 3, pos_cache.data())));
    result.set("V",         val(typed_memory_view(n,     v_cache.data())));
    result.set("count", n);
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

// ══════════════════════════════════════════════════════════════════════
// Force-field decomposition samplers (2026-04-19)
// ══════════════════════════════════════════════════════════════════════
// Each of the three samplers below returns a per-voxel force vector field
// for ONE physical interaction. The viewport renders these as force-arrow
// overlays so users can see the EM, gravity, and strong-force landscapes
// separately.
//
// All three mirror the MockBridge JS implementations in
// engine/web/js/bridge/mock-lattice-samplers.js so WASM and fallback paths
// produce the same visuals. Positions are written at voxel CENTERS
// (x + 0.5f) to match the particle-render convention — see the April-19
// fix to get_particle_data() for the rationale.

// Gravity-force-field sampler: gradient of flux-density magnitude, scaled
// by G_N. F_grav ≈ G_N · ∇|J|  — pulls material toward high-density regions,
// reproducing the lattice analogue of Newtonian gravity (see FTD paper
// §Gravity). Periodic-wrap at the boundaries matches the engine's own
// `lattice().index()` convention.
val get_gravity_field_sampled(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    const auto& voxels = rb.voxels();
    auto density = [&](int x, int y, int z) -> double {
        const auto& v = voxels[rb.lattice().index(x, y, z)];
        return std::sqrt(v.flux.x * v.flux.x + v.flux.y * v.flux.y + v.flux.z * v.flux.z);
    };

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                // Central difference of |J| — periodic wrap via lattice().index().
                const double gradX = (density(x + 1, y, z) - density(x - 1, y, z)) * 0.5;
                const double gradY = (density(x, y + 1, z) - density(x, y - 1, z)) * 0.5;
                const double gradZ = (density(x, y, z + 1) - density(x, y, z - 1)) * 0.5;
                const double mag = std::sqrt(gradX * gradX + gradY * gradY + gradZ * gradZ);
                if (mag < 1e-10) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x) + 0.5f;
                pos_cache[o3 + 1] = static_cast<float>(y) + 0.5f;
                pos_cache[o3 + 2] = static_cast<float>(z) + 0.5f;
                vec_cache[o3]     = static_cast<float>(ftd::G_N * gradX);
                vec_cache[o3 + 1] = static_cast<float>(ftd::G_N * gradY);
                vec_cache[o3 + 2] = static_cast<float>(ftd::G_N * gradZ);
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

// EM force-field sampler: Coulomb force on a unit test charge at each
// voxel, summed over all manifested particles (state != 0). Periodic
// nearest-image to match the lattice's PBC convention.
//
//     F_EM(r) = -Σ_p [α/(4π)] · s_p · (r - r_p) / |r - r_p|³
//
// With softening (soft = 1.0 lattice units) to prevent singularities at
// particle centers.
val get_em_force_field(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    // Collect manifested particles (voxels with state != 0). Empty result
    // if no charges — no force field to sample.
    struct ParticleRef { double x, y, z; int state; };
    std::vector<ParticleRef> particles;
    const auto& voxels = rb.voxels();
    const int total = N * N * N;
    for (int i = 0; i < total; i++) {
        const auto& v = voxels[i];
        if (v.state == 0) continue;
        auto c = rb.lattice().coord(i);
        particles.push_back({double(c.x), double(c.y), double(c.z), v.state});
    }

    val result = val::object();
    if (particles.empty()) {
        result.set("positions", val(typed_memory_view(0, pos_cache.data())));
        result.set("vectors",   val(typed_memory_view(0, vec_cache.data())));
        result.set("count", 0);
        return result;
    }

    const double halfN = N / 2.0;
    const double alpha4pi = ftd::ALPHA / (4.0 * 3.14159265358979323846);
    const double soft = 1.0;  // softening² — matches MockBridge

    int count = 0;
    for (int z = 0; z < N; z += stride) {
        for (int y = 0; y < N; y += stride) {
            for (int x = 0; x < N; x += stride) {
                double fx = 0.0, fy = 0.0, fz = 0.0;
                for (const auto& p : particles) {
                    double dx = p.x - x, dy = p.y - y, dz = p.z - z;
                    // Periodic nearest-image
                    if (dx >  halfN) dx -= N; else if (dx < -halfN) dx += N;
                    if (dy >  halfN) dy -= N; else if (dy < -halfN) dy += N;
                    if (dz >  halfN) dz -= N; else if (dz < -halfN) dz += N;
                    const double r2 = dx * dx + dy * dy + dz * dz + soft;
                    const double invR  = 1.0 / std::sqrt(r2);
                    const double invR3 = invR / r2;
                    // F = -(α/4π) · s · r̂ / r²  (signed by particle state)
                    const double c = -alpha4pi * p.state * invR3;
                    fx += c * dx;
                    fy += c * dy;
                    fz += c * dz;
                }
                const double mag = std::sqrt(fx * fx + fy * fy + fz * fz);
                if (mag < 1e-12) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x) + 0.5f;
                pos_cache[o3 + 1] = static_cast<float>(y) + 0.5f;
                pos_cache[o3 + 2] = static_cast<float>(z) + 0.5f;
                vec_cache[o3]     = static_cast<float>(fx);
                vec_cache[o3 + 1] = static_cast<float>(fy);
                vec_cache[o3 + 2] = static_cast<float>(fz);
                count++;
            }
        }
    }

    result.set("positions", val(typed_memory_view(count * 3, pos_cache.data())));
    result.set("vectors",   val(typed_memory_view(count * 3, vec_cache.data())));
    result.set("count", count);
    return result;
}

// Strong-force field sampler: 3-regime color force (Coulomb → transition →
// linear confinement) along flux tubes between all particle pairs, plus a
// short-range nuclear attraction at each particle. Mirrors the mockBridge
// implementation to keep WASM and JS visuals identical.
//
//   r < 3       → F = α_s(r) / r²     (Coulomb at short range)
//   3 ≤ r < 8   → F = α_s(r) / (3r)   (transition)
//   r ≥ 8       → F = α_s(r) · r / 64 (linear confinement)
//
// where α_s(r) = 1 / (1 + 0.1·ln(1 + r)) encodes running asymptotic freedom.
// The tube envelope (Gaussian perpendicular to the pair axis) localizes the
// force to the confinement string connecting each quark pair.
val get_strong_force_field(ftd::RenderBridge& rb, int stride) {
    static std::vector<float> pos_cache, vec_cache;
    const int N = rb.lattice().size();
    if (stride < 1) stride = 1;
    const int S = (N + stride - 1) / stride;
    const int maxPts = S * S * S;

    if (static_cast<int>(pos_cache.size()) < maxPts * 3) {
        pos_cache.resize(maxPts * 3);
        vec_cache.resize(maxPts * 3);
    }

    // Collect manifested particles
    struct ParticleRef { double x, y, z; };
    std::vector<ParticleRef> particles;
    const auto& voxels = rb.voxels();
    const int total = N * N * N;
    for (int i = 0; i < total; i++) {
        if (voxels[i].state == 0) continue;
        auto c = rb.lattice().coord(i);
        particles.push_back({double(c.x), double(c.y), double(c.z)});
    }

    val result = val::object();
    if (particles.size() < 2) {
        // Need ≥2 particles to form a flux tube
        result.set("positions", val(typed_memory_view(0, pos_cache.data())));
        result.set("vectors",   val(typed_memory_view(0, vec_cache.data())));
        result.set("count", 0);
        return result;
    }

    const double halfN = N / 2.0;
    const double ALPHA_S = 1.0;
    const double TUBE_W  = 1.5;        // flux tube Gaussian width (lattice units)

    // Precompute pair geometries (unit tangents, separations)
    struct PairRef { double ax, ay, az; double tx, ty, tz; double sep; };
    std::vector<PairRef> pairs;
    pairs.reserve(particles.size() * (particles.size() - 1) / 2);
    for (size_t i = 0; i < particles.size(); i++) {
        for (size_t j = i + 1; j < particles.size(); j++) {
            double dx = particles[j].x - particles[i].x;
            double dy = particles[j].y - particles[i].y;
            double dz = particles[j].z - particles[i].z;
            if (dx >  halfN) dx -= N; else if (dx < -halfN) dx += N;
            if (dy >  halfN) dy -= N; else if (dy < -halfN) dy += N;
            if (dz >  halfN) dz -= N; else if (dz < -halfN) dz += N;
            const double sep = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (sep < 0.5) continue;
            const double inv = 1.0 / sep;
            pairs.push_back({particles[i].x, particles[i].y, particles[i].z,
                             dx * inv, dy * inv, dz * inv, sep});
        }
    }
    if (pairs.empty()) {
        result.set("positions", val(typed_memory_view(0, pos_cache.data())));
        result.set("vectors",   val(typed_memory_view(0, vec_cache.data())));
        result.set("count", 0);
        return result;
    }

    int count = 0;
    for (int z = 0; z < N && count < maxPts; z += stride) {
        for (int y = 0; y < N && count < maxPts; y += stride) {
            for (int x = 0; x < N && count < maxPts; x += stride) {
                double fx = 0.0, fy = 0.0, fz = 0.0;

                // 1. Flux-tube contribution: force along each tube, pointing INWARD
                //    toward the tube midpoint from both ends (confinement geometry).
                for (const auto& pair : pairs) {
                    double rx = x - pair.ax, ry = y - pair.ay, rz = z - pair.az;
                    if (rx >  halfN) rx -= N; else if (rx < -halfN) rx += N;
                    if (ry >  halfN) ry -= N; else if (ry < -halfN) ry += N;
                    if (rz >  halfN) rz -= N; else if (rz < -halfN) rz += N;

                    // Project onto tube axis
                    const double t = rx * pair.tx + ry * pair.ty + rz * pair.tz;
                    if (t < -1.0 || t > pair.sep + 1.0) continue;

                    // Perpendicular distance² from tube axis (Gaussian envelope)
                    const double projX = t * pair.tx, projY = t * pair.ty, projZ = t * pair.tz;
                    const double perpX = rx - projX, perpY = ry - projY, perpZ = rz - projZ;
                    const double perp2 = perpX * perpX + perpY * perpY + perpZ * perpZ;
                    const double tubeEnv = std::exp(-perp2 / (2.0 * TUBE_W * TUBE_W));
                    if (tubeEnv < 0.01) continue;

                    // 3-regime magnitude
                    double r = std::sqrt(rx * rx + ry * ry + rz * rz);
                    if (r < 0.5) r = 0.5;
                    const double alpha_s_r = ALPHA_S / (1.0 + 0.1 * std::log(1.0 + r));
                    double fMag;
                    if (r < 3.0) {
                        fMag = alpha_s_r / (r * r);       // Coulomb
                    } else if (r < 8.0) {
                        fMag = alpha_s_r / (3.0 * r);     // Transition
                    } else {
                        fMag = alpha_s_r * r / 64.0;      // Linear confinement
                    }
                    fMag *= tubeEnv;

                    // Point inward: near-A half (t < sep/2) pushes toward B (+tangent);
                    // near-B half (t > sep/2) pushes toward A (−tangent).
                    const double sign = (t < pair.sep * 0.5) ? 1.0 : -1.0;
                    fx += fMag * pair.tx * sign;
                    fy += fMag * pair.ty * sign;
                    fz += fMag * pair.tz * sign;
                }

                // 2. Short-range nuclear attraction at each particle (r < 5)
                for (const auto& p : particles) {
                    double rx = x - p.x, ry = y - p.y, rz = z - p.z;
                    if (rx >  halfN) rx -= N; else if (rx < -halfN) rx += N;
                    if (ry >  halfN) ry -= N; else if (ry < -halfN) ry += N;
                    if (rz >  halfN) rz -= N; else if (rz < -halfN) rz += N;
                    const double r = std::sqrt(rx * rx + ry * ry + rz * rz + 0.5);
                    if (r > 5.0) continue;
                    const double alpha_s_r = ALPHA_S / (1.0 + 0.1 * std::log(1.0 + r));
                    const double fNuc = alpha_s_r / (r * r);
                    if (fNuc < 1e-4) continue;
                    // Attractive: toward the quark (so subtract outward r̂)
                    fx -= fNuc * rx / r;
                    fy -= fNuc * ry / r;
                    fz -= fNuc * rz / r;
                }

                const double mag = std::sqrt(fx * fx + fy * fy + fz * fz);
                if (mag < 1e-4) continue;
                const int o3 = count * 3;
                pos_cache[o3]     = static_cast<float>(x) + 0.5f;
                pos_cache[o3 + 1] = static_cast<float>(y) + 0.5f;
                pos_cache[o3 + 2] = static_cast<float>(z) + 0.5f;
                vec_cache[o3]     = static_cast<float>(fx);
                vec_cache[o3 + 1] = static_cast<float>(fy);
                vec_cache[o3 + 2] = static_cast<float>(fz);
                count++;
            }
        }
    }

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
