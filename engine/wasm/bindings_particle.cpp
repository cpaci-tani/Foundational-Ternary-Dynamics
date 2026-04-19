/**
 * @file bindings_particle.cpp
 * @brief Embind bindings for ParticleEngine (Scale 1).
 *
 * Extracted from ftd_wasm.cpp as part of W1-W3. Contains:
 *   - pe_toggle_map (pointer-to-member ParticleToggles lookup)
 *   - Data extraction (get_pe_particle_data, get_pe_diagnostics)
 *   - Injection helpers (pe_add_particle, pe_add_locked_particle)
 *   - Controls (dt, softening, damping, gravity, clear)
 *   - Force diagnostics
 *   - The ParticleEngine class_<> binding
 */

#include <emscripten/bind.h>
#include <emscripten/val.h>
#include <cmath>
#include <string>
#include <unordered_map>
#include "ftd/particle_engine.h"
#include "ftd/constants.h"

using namespace emscripten;

// ── PE Particle Data Extraction ─────────────────────────────────────
// Returns positions + charge-based colors + mass-based sizes for Three.js
static val get_pe_particle_data(ftd::ParticleEngine& pe) {
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
static val get_pe_diagnostics(ftd::ParticleEngine& pe) {
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
static int pe_add_particle(ftd::ParticleEngine& pe, int charge,
                           double x, double y, double z,
                           double vx, double vy, double vz,
                           double mass, double r_eff) {
    return pe.add_particle(static_cast<int8_t>(charge),
                           ftd::Vec3(x, y, z),
                           ftd::Vec3(vx, vy, vz),
                           mass, r_eff);
}

static int pe_add_locked_particle(ftd::ParticleEngine& pe, int charge,
                                   double x, double y, double z,
                                   double mass, double r_eff) {
    int id = pe.add_locked_particle(static_cast<int8_t>(charge),
                                     ftd::Vec3(x, y, z), mass);
    // Override default r_eff (C++ default is 2.48, too large for atomic orbits)
    pe.particles().back().r_eff = r_eff;
    return id;
}

// ── PE Controls ────────────────────────────────────────────────────
static void pe_set_dt(ftd::ParticleEngine& pe, double dt) { pe.set_dt(dt); }
static double pe_get_dt(ftd::ParticleEngine& pe) { return pe.dt(); }
static void pe_set_softening(ftd::ParticleEngine& pe, double s) { pe.set_softening(s); }
static void pe_set_damping(ftd::ParticleEngine& pe, bool e) { pe.set_damping_enabled(e); }
static void pe_set_gravity(ftd::ParticleEngine& pe, bool e) { pe.set_gravity_enabled(e); }
static int pe_particle_count(ftd::ParticleEngine& pe) { return static_cast<int>(pe.particles().size()); }

static void pe_clear(ftd::ParticleEngine& pe) {
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

static void pe_set_toggle(ftd::ParticleEngine& pe, const std::string& name, bool val) {
    auto it = pe_toggle_map().find(name);
    if (it != pe_toggle_map().end()) pe.toggles.*(it->second) = val;
}

static bool pe_get_toggle(ftd::ParticleEngine& pe, const std::string& name) {
    auto it = pe_toggle_map().find(name);
    if (it != pe_toggle_map().end()) return pe.toggles.*(it->second);
    return false;
}

// ── PE Force Diagnostic ───────────────────────────────────────────
static val get_pe_force_diag(ftd::ParticleEngine& pe, int idx) {
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

// ── Embind Registration ──────────────────────────────────────────────
EMSCRIPTEN_BINDINGS(ftd_module_particle) {
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
}
