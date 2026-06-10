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

static ftd::ParticleForceDiag compute_pe_force_diag_snapshot(
    const ftd::ParticleEngine& pe, int i) {
    ftd::ParticleForceDiag diag;
    const auto& particles = pe.particles();
    if (i < 0 || i >= static_cast<int>(particles.size())) return diag;

    const auto& pi = particles[i];
    const auto& toggles = pe.toggles;
    const double soft = pe.softening();

    for (int j = 0; j < static_cast<int>(particles.size()); ++j) {
        if (i == j) continue;
        const auto& pj = particles[j];

        ftd::Vec3 r_vec = pj.position - pi.position;
        double raw_r2 = r_vec.mag2();
        double r2 = raw_r2 + soft * soft;
        double r = std::sqrt(r2);
        if (r < 1e-30) continue;

        ftd::Vec3 r_hat = r_vec * (1.0 / r);

        if (toggles.coulomb) {
            double f_em = -ftd::ALPHA_EFT * pi.charge * pj.charge / (4.0 * ftd::PI * r2);
            diag.f_coulomb += r_hat * f_em;
        }

        if (toggles.gravity) {
            double f_grav = ftd::G_N * pi.mass * pj.mass / r2;
            diag.f_gravity += r_hat * f_grav;
        }

        if (toggles.exchange && pi.spin != 0 && pj.spin == pi.spin
            && pi.charge == pj.charge) {
            double f_mag = ftd::ALPHA_EXCHANGE * std::exp(-r2 / ftd::EXCHANGE_RANGE_SQ) / r2;
            diag.f_exchange += r_hat * (-f_mag);
        }

        if (toggles.strong && pi.color != 0 && pj.color != 0) {
            double cf = (pi.color == pj.color) ? 0.5 : -1.0;
            double raw_r = std::sqrt(raw_r2);
            if (raw_r < 1.0) raw_r = 1.0;
            double raw_force;
            if (raw_r < 3.0) {
                double as = ftd::alpha_s_lattice(raw_r);
                raw_force = as * cf / (raw_r * raw_r);
            } else if (raw_r < 8.0) {
                double as = ftd::alpha_s_lattice(raw_r);
                raw_force = as * cf / (3.0 * raw_r);
            } else {
                raw_force = ftd::SIGMA_STRING * cf;
            }
            diag.f_strong += r_hat * (-raw_force);
        }

        if (toggles.magnetic_dipole
            && pi.spin_axis.mag2() > 1e-30 && pj.spin_axis.mag2() > 1e-30) {
            ftd::Vec3 mi_mu = pi.spin_axis * (static_cast<double>(pi.charge) / pi.mass);
            ftd::Vec3 mj_mu = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);

            double r3 = r * r2;
            double r5 = r3 * r2;
            double mi_dot_r = mi_mu.dot(r_vec);
            double mj_dot_r = mj_mu.dot(r_vec);
            double mi_dot_mj = mi_mu.dot(mj_mu);

            double coeff = 3.0 * ftd::ALPHA_EFT / (4.0 * ftd::PI * r5);
            ftd::Vec3 fdd = (r_vec * (5.0 * mi_dot_r * mj_dot_r / r2)
                             - mj_mu * mi_dot_r - mi_mu * mj_dot_r
                             - r_vec * mi_dot_mj) * coeff;
            diag.f_magnetic_dipole += fdd;
        }

        if (toggles.spin_orbit && pi.spin_axis.mag2() > 1e-30) {
            ftd::Vec3 p_rel = pi.velocity * pi.mass;
            ftd::Vec3 L_orb = ftd::Vec3::cross(r_vec, p_rel);
            double L_dot_S = L_orb.dot(pi.spin_axis);
            double raw_r = std::sqrt(raw_r2);
            if (raw_r > 1e-15) {
                double r3 = raw_r * raw_r * raw_r;
                double m2c2 = pi.mass * pi.mass * ftd::C_SPEED * ftd::C_SPEED;
                double coeff_so = ftd::ALPHA / (2.0 * m2c2 * r3);
                diag.f_spin_orbit += r_hat * (coeff_so * L_dot_S);
            }
        }

        if (toggles.lorentz && pi.velocity.mag2() > 1e-30
            && pj.spin_axis.mag2() > 1e-30) {
            ftd::Vec3 mj = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);
            double r3 = r * r2;
            double m_dot_rh = mj.dot(r_hat);
            ftd::Vec3 B_j = (r_hat * (3.0 * m_dot_rh) - mj)
                           * (1.0 / (4.0 * ftd::PI * r3));
            diag.f_lorentz += ftd::Vec3::cross(pi.velocity, B_j)
                            * (ftd::ALPHA * pi.charge);
        }
    }

    ftd::Vec3 total = diag.total();

    if (toggles.radiation && pi.prev_acceleration.mag2() > 1e-30
        && pi.velocity.mag2() > 1e-30) {
        double a2 = pi.prev_acceleration.mag2();
        double q2 = static_cast<double>(pi.charge) * pi.charge;
        double c3 = ftd::C_SPEED * ftd::C_SPEED * ftd::C_SPEED;
        double coeff_rad = -(2.0 / 3.0) * ftd::ALPHA * q2 / (pi.mass * c3);
        double v_mag = pi.velocity.mag();
        ftd::Vec3 v_hat = pi.velocity * (1.0 / v_mag);
        ftd::Vec3 frad = v_hat * (coeff_rad * a2);
        diag.f_radiation += frad;
        total += frad;
    }

    if (toggles.relativistic) {
        double v2 = pi.velocity.mag2();
        double c2 = ftd::C_SPEED * ftd::C_SPEED;
        double beta2 = v2 / c2;
        if (beta2 > 1e-10 && beta2 < 1.0) {
            double gamma = 1.0 / std::sqrt(1.0 - beta2);
            diag.f_relativistic += total * (1.0 / gamma - 1.0);
        }
    }

    return diag;
}

// ── PE Particle Data Extraction ─────────────────────────────────────
// Returns positions + charge-based colors + mass-based sizes for Three.js
static val get_pe_particle_data(ftd::ParticleEngine& pe) {
    const auto& particles = pe.particles();
    int count = static_cast<int>(particles.size());

    val positions = val::global("Float32Array").new_(count * 3);
    val velocities = val::global("Float32Array").new_(count * 3);
    val colors    = val::global("Float32Array").new_(count * 3);
    val sizes     = val::global("Float32Array").new_(count);
    val masses    = val::global("Float64Array").new_(count);
    val r_eff     = val::global("Float32Array").new_(count);
    val charges   = val::global("Int8Array").new_(count);
    val ids       = val::global("Int32Array").new_(count);
    val locked    = val::global("Uint8Array").new_(count);
    val spins     = val::global("Int8Array").new_(count);
    val color_ids = val::global("Int8Array").new_(count);

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];

        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));
        velocities.set(i * 3,     static_cast<float>(p.velocity.x));
        velocities.set(i * 3 + 1, static_cast<float>(p.velocity.y));
        velocities.set(i * 3 + 2, static_cast<float>(p.velocity.z));

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
        masses.set(i, p.mass);
        r_eff.set(i, static_cast<float>(p.r_eff));
        locked.set(i, p.locked ? 1 : 0);
        spins.set(i, static_cast<int>(p.spin));
        color_ids.set(i, static_cast<int>(p.color));
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("velocities", velocities);
    result.set("colors", colors);
    result.set("sizes", sizes);
    result.set("charges", charges);
    result.set("ids", ids);
    result.set("masses", masses);
    result.set("rEff", r_eff);
    result.set("locked", locked);
    result.set("spins", spins);
    result.set("colorIds", color_ids);
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
    result.set("coulombPE",      d.coulomb_pe);
    result.set("gravityPE",      d.gravity_pe);
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
    pe.clear();
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
        {"relativistic_verlet", &ftd::ParticleToggles::relativistic_verlet},
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
    result.set("radiation_x", d.f_radiation.x); result.set("radiation_y", d.f_radiation.y); result.set("radiation_z", d.f_radiation.z);
    result.set("spin_orbit_x", d.f_spin_orbit.x); result.set("spin_orbit_y", d.f_spin_orbit.y); result.set("spin_orbit_z", d.f_spin_orbit.z);
    result.set("relativistic_x", d.f_relativistic.x); result.set("relativistic_y", d.f_relativistic.y); result.set("relativistic_z", d.f_relativistic.z);
    result.set("magnetic_dipole_x", d.f_magnetic_dipole.x); result.set("magnetic_dipole_y", d.f_magnetic_dipole.y); result.set("magnetic_dipole_z", d.f_magnetic_dipole.z);
    auto tot = d.total();
    result.set("total_x", tot.x); result.set("total_y", tot.y); result.set("total_z", tot.z);
    return result;
}

static val get_pe_forces(ftd::ParticleEngine& pe) {
    const auto& particles = pe.particles();
    int count = static_cast<int>(particles.size());
    val positions = val::global("Float32Array").new_(count * 3);
    val forces = val::global("Float32Array").new_(count * 3);
    double max_force = 0.0;

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        auto fd = compute_pe_force_diag_snapshot(pe, i);
        auto f = fd.total();
        positions.set(i * 3,     static_cast<float>(p.position.x));
        positions.set(i * 3 + 1, static_cast<float>(p.position.y));
        positions.set(i * 3 + 2, static_cast<float>(p.position.z));
        forces.set(i * 3,     static_cast<float>(f.x));
        forces.set(i * 3 + 1, static_cast<float>(f.y));
        forces.set(i * 3 + 2, static_cast<float>(f.z));
        double mag = f.mag();
        if (mag > max_force) max_force = mag;
    }

    val result = val::object();
    result.set("positions", positions);
    result.set("forces", forces);
    result.set("count", count);
    result.set("maxForce", max_force);
    return result;
}

static val get_pe_extended_data(ftd::ParticleEngine& pe) {
    const auto& particles = pe.particles();
    int count = static_cast<int>(particles.size());
    val ids = val::global("Int32Array").new_(count);
    val charges = val::global("Int8Array").new_(count);
    val masses = val::global("Float64Array").new_(count);
    val positions = val::global("Float64Array").new_(count * 3);
    val velocities = val::global("Float64Array").new_(count * 3);
    val forces = val::global("Float64Array").new_(count * 3);
    val accelerations = val::global("Float64Array").new_(count * 3);
    val locked = val::global("Uint8Array").new_(count);
    val r_eff = val::global("Float64Array").new_(count);
    val spins = val::global("Int8Array").new_(count);
    val color_ids = val::global("Int8Array").new_(count);

    for (int i = 0; i < count; ++i) {
        const auto& p = particles[i];
        auto fd = compute_pe_force_diag_snapshot(pe, i);
        auto f = fd.total();
        ids.set(i, p.id);
        charges.set(i, static_cast<int>(p.charge));
        masses.set(i, p.mass);
        locked.set(i, p.locked ? 1 : 0);
        r_eff.set(i, p.r_eff);
        spins.set(i, static_cast<int>(p.spin));
        color_ids.set(i, static_cast<int>(p.color));
        positions.set(i * 3,     p.position.x);
        positions.set(i * 3 + 1, p.position.y);
        positions.set(i * 3 + 2, p.position.z);
        velocities.set(i * 3,     p.velocity.x);
        velocities.set(i * 3 + 1, p.velocity.y);
        velocities.set(i * 3 + 2, p.velocity.z);
        forces.set(i * 3,     f.x);
        forces.set(i * 3 + 1, f.y);
        forces.set(i * 3 + 2, f.z);
        if (p.mass > 1e-30 && !p.locked) {
            accelerations.set(i * 3,     f.x / p.mass);
            accelerations.set(i * 3 + 1, f.y / p.mass);
            accelerations.set(i * 3 + 2, f.z / p.mass);
        } else {
            accelerations.set(i * 3, 0.0);
            accelerations.set(i * 3 + 1, 0.0);
            accelerations.set(i * 3 + 2, 0.0);
        }
    }

    val result = val::object();
    result.set("count", count);
    result.set("ids", ids);
    result.set("charges", charges);
    result.set("masses", masses);
    result.set("positions", positions);
    result.set("velocities", velocities);
    result.set("forces", forces);
    result.set("accelerations", accelerations);
    result.set("locked", locked);
    result.set("rEff", r_eff);
    result.set("spins", spins);
    result.set("colorIds", color_ids);
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
    function("getPEExtendedData",   &get_pe_extended_data);
    function("getPEForces",         &get_pe_forces);
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
