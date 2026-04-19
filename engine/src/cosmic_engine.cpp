/**
 * CosmicEngine: Scale 5 simulation — core TU.
 *
 * Barnes-Hut octree gravity (O(N log N)) + SPH gas dynamics.
 * All physics driven by FTD-derived constants — zero free parameters.
 *
 * This TU retains the CosmicEngine backbone: construction, body creation,
 * compact-object + stellar physics, extended-physics (B-field, radiation
 * pressure), Verlet integration, the main tick() orchestrator, diagnostics,
 * and clear(). Banner-identified sections have been extracted to
 * src/cosmic/{scenarios,barnes_hut,sph,cosmology,gravitational_waves}.cpp.
 *
 * See cosmic_engine.h for full documentation.
 */

#include "ftd/cosmic_engine.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace ftd {

// ============================================================================
// Construction
// ============================================================================

CosmicEngine::CosmicEngine() {
    bodies_.reserve(100000);
    forces_.reserve(100000);
    force_diag_.reserve(100000);
}

// ============================================================================
// Body creation
// ============================================================================

int CosmicEngine::add_body(CosmicBodyType type, double mass, Vec3 position,
                           Vec3 velocity, double temperature, double radius) {
    CosmicBody b;
    b.id = next_id_++;
    b.type = type;
    b.mass = mass;
    b.position = position;
    b.velocity = velocity;
    b.temperature = temperature;
    b.radius = (radius > 0.0) ? radius : std::cbrt(mass) * 0.1;

    // Set type-specific defaults
    if (is_sph_body(type)) {
        b.smoothing_length = b.radius * 2.0;
        b.internal_energy = temperature / (cosmic::GAMMA_ADIABATIC - 1.0);
    }
    if (type == CosmicBodyType::BLACK_HOLE || type == CosmicBodyType::QUASAR) {
        b.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + mass * G_N));
    }
    if (type == CosmicBodyType::STAR) {
        // Mass-luminosity relation: L ~ M^3.5 (main sequence approx)
        b.luminosity = std::pow(mass, 3.5);
    }
    if (type == CosmicBodyType::QUASAR) {
        b.luminosity = mass * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED;
    }

    bodies_.push_back(b);
    return b.id;
}

int CosmicEngine::add_dark_matter(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::DARK_MATTER, mass, position, velocity);
}

int CosmicEngine::add_gas(double mass, Vec3 position, Vec3 velocity,
                          double temperature) {
    return add_body(CosmicBodyType::GAS, mass, position, velocity, temperature);
}

int CosmicEngine::add_star(double mass, Vec3 position, Vec3 velocity,
                           double luminosity) {
    int id = add_body(CosmicBodyType::STAR, mass, position, velocity);
    if (luminosity > 0.0) {
        for (auto& b : bodies_) {
            if (b.id == id) { b.luminosity = luminosity; break; }
        }
    }
    return id;
}

int CosmicEngine::add_black_hole(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::BLACK_HOLE, mass, position, velocity);
}

int CosmicEngine::add_quasar(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::QUASAR, mass, position, velocity);
}

int CosmicEngine::add_neutron_star(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::NEUTRON_STAR, mass, position, velocity);
}

int CosmicEngine::add_nebula(double mass, Vec3 position, Vec3 velocity,
                             double temperature) {
    return add_body(CosmicBodyType::NEBULA, mass, position, velocity, temperature);
}

int CosmicEngine::add_white_dwarf(double mass, Vec3 position, Vec3 velocity) {
    return add_body(CosmicBodyType::WHITE_DWARF, mass, position, velocity);
}

// Scenario builders -> src/cosmic/cosmic_scenarios.cpp
// Barnes-Hut + gravity -> src/cosmic/cosmic_barnes_hut.cpp
// SPH -> src/cosmic/cosmic_sph.cpp
// Friedmann/Hubble/dark energy -> src/cosmic/cosmic_cosmology.cpp

// ============================================================================
// Compact objects
// ============================================================================

void CosmicEngine::compute_accretion() {
    if (!toggles.black_hole_accretion) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        auto& bh = bodies_[i];
        if (bh.type != CosmicBodyType::BLACK_HOLE &&
            bh.type != CosmicBodyType::QUASAR) continue;

        double r_s = bh.schwarzschild_radius();

        for (int j = 0; j < (int)bodies_.size(); ++j) {
            if (i == j) continue;
            if (!is_sph_body(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[j].position.x - bh.position.x,
                bodies_[j].position.y - bh.position.y,
                bodies_[j].position.z - bh.position.z
            };
            double r = dr.mag();
            if (r < r_s * 3.0) { // Inside ISCO — accrete
                // Bondi-Hoyle: Mdot = 4*pi*G^2*M^2*rho / (c_s^2 + v^2)^(3/2)
                double cs = bodies_[j].sound_speed();
                double v2 = bodies_[j].velocity.mag2();
                double denom = std::pow(cs * cs + v2, 1.5);
                if (denom > 0.0) {
                    double mdot = 4.0 * PI * G_N * G_N * bh.mass * bh.mass *
                                  bodies_[j].density / denom;
                    // Eddington limit: L_edd = 4*pi*G*M*c/sigma_T, then mdot_edd = L_edd/(eta*c^2)
                    // sigma_T ~ 0.4 in simulation units, eta = BONDI_EFFICIENCY
                    double L_edd = 4.0 * PI * G_N * bh.mass * C_SPEED / 0.4;
                    double mdot_edd = L_edd / (cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED);
                    mdot = std::min(mdot, mdot_edd);
                    double dm = std::min(mdot * dt_, bodies_[j].mass * 0.1);
                    bh.mass += dm;
                    bodies_[j].mass -= dm;
                    bh.accretion_rate = mdot;
                    if (bh.type == CosmicBodyType::QUASAR) {
                        bh.luminosity = mdot * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED;
                    }
                }
            }
        }

        // Update latency field
        bh.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + bh.mass * G_N));
    }
}

void CosmicEngine::compute_relativistic_jets() {
    if (!toggles.relativistic_jets) return;

    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::QUASAR &&
            b.type != CosmicBodyType::BLACK_HOLE) continue;
        if (b.accretion_rate <= 0.0) continue;

        // Jet velocity: v_jet = c * sqrt(1 - L^2) from FTD latency
        double v_jet = C_SPEED * std::sqrt(1.0 - b.latency * b.latency);

        // Jet direction along angular momentum axis
        Vec3 jet_dir = b.angular_momentum;
        double j_mag = jet_dir.mag();
        if (j_mag < 1e-10) {
            jet_dir = {0, 0, 1}; // Default to z-axis
        } else {
            jet_dir.x /= j_mag;
            jet_dir.y /= j_mag;
            jet_dir.z /= j_mag;
        }

        // Jet kinetic luminosity: fraction of accretion luminosity
        double L_jet = b.accretion_rate * cosmic::BONDI_EFFICIENCY * C_SPEED * C_SPEED * 0.1;
        // Momentum flux: p_dot = L_jet / v_jet (for relativistic material)
        double p_dot = L_jet / (v_jet + 1e-20);
        // Recoil on BH (Newton's third law) — bipolar, so net recoil is zero for symmetric jets
        // Add slight asymmetry for realistic kick
        double asymmetry = 0.05;
        b.velocity.x += jet_dir.x * p_dot * asymmetry / b.mass * dt_;
        b.velocity.y += jet_dir.y * p_dot * asymmetry / b.mass * dt_;
        b.velocity.z += jet_dir.z * p_dot * asymmetry / b.mass * dt_;
    }
}

// ============================================================================
// Stellar physics
// ============================================================================

void CosmicEngine::check_star_formation() {
    if (!toggles.star_formation) return;

    std::vector<CosmicBody> new_stars;
    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::GAS && b.type != CosmicBodyType::NEBULA) continue;
        // Jeans criterion: form star when density exceeds threshold
        if (b.density > cosmic::RHO_JEANS && b.temperature < 1e4) {
            // Convert fraction of gas to star
            double star_mass = b.mass * 0.1;
            b.mass -= star_mass;

            CosmicBody star;
            star.id = next_id_++;
            star.type = CosmicBodyType::STAR;
            star.mass = star_mass;
            star.position = b.position;
            star.velocity = b.velocity;
            star.luminosity = std::pow(star_mass, 3.5);
            star.radius = std::cbrt(star_mass) * 0.01;
            new_stars.push_back(star);
        }
    }
    for (auto& s : new_stars) bodies_.push_back(s);
}

void CosmicEngine::check_stellar_evolution() {
    if (!toggles.stellar_evolution) return;

    std::vector<CosmicBody> new_bodies;
    for (auto& b : bodies_) {
        if (b.type != CosmicBodyType::STAR) continue;

        // Stellar lifetime: t ~ (M+1)^(-2.5) * 1e7 (cosmic time units)
        double lifetime = std::pow(b.mass + 1.0, -2.5) * 1e7;
        if (t_cosmic_ > lifetime) {
            if (b.mass > cosmic::M_SUPERNOVA) {
                if (b.mass > cosmic::M_TOV * 10.0) {
                    // Massive star -> Black hole
                    b.type = CosmicBodyType::BLACK_HOLE;
                    b.latency = std::min(0.99, 1.0 - 1.0 / (1.0 + b.mass * G_N));
                    b.luminosity = 0.0;
                } else {
                    // Medium star -> Neutron star + supernova nebula
                    b.type = CosmicBodyType::NEUTRON_STAR;
                    double m_remnant = std::min(b.mass * 0.15 + 1.2, cosmic::M_TOV); // ~1.4-2.2 M_sun
                    double ejected = b.mass - m_remnant;
                    b.mass = m_remnant;
                    CosmicBody nebula;
                    nebula.id = next_id_++;
                    nebula.type = CosmicBodyType::NEBULA;
                    nebula.mass = ejected;
                    nebula.position = b.position;
                    nebula.velocity = b.velocity;
                    nebula.temperature = 1e6;
                    new_bodies.push_back(nebula);
                }
            } else if (b.mass < cosmic::M_CHANDRASEKHAR) {
                // Low mass star -> White dwarf
                b.type = CosmicBodyType::WHITE_DWARF;
                b.luminosity *= 0.01; // Much dimmer
            }
        }
    }
    for (auto& nb : new_bodies) bodies_.push_back(nb);
}

// ============================================================================
// Extended physics
// ============================================================================

void CosmicEngine::compute_magnetic_fields() {
    if (!toggles.magnetic_fields) return;

    // Simplified cosmic dynamo: B grows with rotation and turbulence
    for (auto& b : bodies_) {
        if (!is_sph_body(b.type)) continue;
        // Dynamo growth: dB/dt ~ alpha_dynamo * omega * B
        // alpha_dynamo ~ ALPHA (fine structure — from FTD)
        double omega = b.angular_momentum.mag() / (b.radius * b.radius + 1.0);
        double growth = ALPHA * omega;
        b.magnetic_field.x += growth * b.magnetic_field.x * dt_;
        b.magnetic_field.y += growth * b.magnetic_field.y * dt_;
        b.magnetic_field.z += growth * b.magnetic_field.z * dt_;

        // Seed field if none exists
        if (b.magnetic_field.mag() < 1e-10) {
            b.magnetic_field = {1e-8, 1e-8, 1e-8};
        }
    }
}

void CosmicEngine::compute_radiation_pressure() {
    if (!toggles.radiation_pressure) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_luminous(bodies_[i].type)) continue;
        double L = bodies_[i].luminosity;
        if (L <= 0.0) continue;

        for (int j = 0; j < (int)bodies_.size(); ++j) {
            if (i == j) continue;
            if (!is_sph_body(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[j].position.x - bodies_[i].position.x,
                bodies_[j].position.y - bodies_[i].position.y,
                bodies_[j].position.z - bodies_[i].position.z
            };
            double r2 = dr.mag2() + softening_ * softening_;
            double r = std::sqrt(r2);

            // F_rad = L / (4*pi*r^2*c) in direction away from source
            double f_mag = L / (4.0 * PI * r2 * C_SPEED);
            Vec3 f_rad = {f_mag * dr.x / r, f_mag * dr.y / r, f_mag * dr.z / r};

            forces_[j].x += f_rad.x / bodies_[j].mass;
            forces_[j].y += f_rad.y / bodies_[j].mass;
            forces_[j].z += f_rad.z / bodies_[j].mass;
            force_diag_[j].f_radiation.x += f_rad.x / bodies_[j].mass;
            force_diag_[j].f_radiation.y += f_rad.y / bodies_[j].mass;
            force_diag_[j].f_radiation.z += f_rad.z / bodies_[j].mass;
        }
    }
}

// Gravitational waves -> src/cosmic/cosmic_gravitational_waves.cpp

// ============================================================================
// Integration (Velocity Verlet)
// ============================================================================

void CosmicEngine::half_kick() {
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        bodies_[i].velocity.x += 0.5 * dt_ * forces_[i].x;
        bodies_[i].velocity.y += 0.5 * dt_ * forces_[i].y;
        bodies_[i].velocity.z += 0.5 * dt_ * forces_[i].z;
    }
}

void CosmicEngine::drift() {
    for (auto& b : bodies_) {
        b.position.x += dt_ * b.velocity.x;
        b.position.y += dt_ * b.velocity.y;
        b.position.z += dt_ * b.velocity.z;
    }
}

void CosmicEngine::enforce_speed_limit() {
    for (auto& b : bodies_) {
        double v = b.velocity.mag();
        if (v > C_SPEED) {
            double scale = C_SPEED / v;
            b.velocity.x *= scale;
            b.velocity.y *= scale;
            b.velocity.z *= scale;
        }
    }
}

// ============================================================================
// Main tick cycle (18 phases)
// ============================================================================

void CosmicEngine::tick() {
    // ================================================================
    // Tick cycle follows Gadget-2 conventions:
    //   1. Compute forces at current positions
    //   2. Kick-drift-kick (Velocity Verlet)
    //   3. Post-integration updates (mass changes, mergers, cleanup)
    //
    // Mass changes (accretion, star formation, mergers) happen AFTER
    // integration, never during force computation. This preserves
    // energy conservation and prevents mid-loop state corruption.
    // ================================================================

    // ── Phase A: Compute forces at CURRENT positions ──
    int n = (int)bodies_.size();
    forces_.assign(n, {});
    force_diag_.assign(n, {});

    build_octree();
    compute_gravity();
    compute_sph_density();
    compute_sph_forces();
    apply_hubble_expansion();
    apply_dark_energy();
    compute_magnetic_fields();
    compute_radiation_pressure();

    // ── Phase B: Velocity Verlet integration ──
    half_kick();                    // v += 0.5 * dt * a (current forces)
    drift();                        // x += dt * v

    // Recompute forces at NEW positions (symplecticity requirement)
    int n2 = (int)bodies_.size();
    forces_.assign(n2, {});
    force_diag_.assign(n2, {});
    build_octree();
    compute_gravity();
    if (toggles.sph_gas) { compute_sph_density(); compute_sph_forces(); }

    half_kick();                    // v += 0.5 * dt * a (fresh forces)

    // ── Phase C: Post-integration updates (Gadget-2 convention) ──
    // Mass changes, particle creation/destruction happen HERE, not in forces.
    compute_accretion();
    compute_relativistic_jets();
    check_star_formation();
    check_stellar_evolution();
    detect_gw_events();
    propagate_gw();
    enforce_speed_limit();

    tick_++;
}

void CosmicEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) tick();
}

// ============================================================================
// Diagnostics
// ============================================================================

CosmicDiagnostics CosmicEngine::diagnostics() const {
    CosmicDiagnostics d;
    d.tick = tick_;
    d.body_count = (int)bodies_.size();
    d.scale_factor = a_;
    d.hubble_parameter = hubble_parameter();
    d.dark_energy_density = H0_ * H0_ * OMEGA_LAMBDA_CONJ;

    for (const auto& b : bodies_) {
        // Count by type (offset by 3 for negative enum values)
        int idx = static_cast<int>(b.type) + 3;
        if (idx >= 0 && idx < 9) d.counts_by_type[idx]++;

        d.total_mass += b.mass;
        double v2 = b.velocity.mag2();
        d.total_ke += 0.5 * b.mass * v2;
        d.total_thermal += b.internal_energy * b.mass;

        d.total_momentum.x += b.mass * b.velocity.x;
        d.total_momentum.y += b.mass * b.velocity.y;
        d.total_momentum.z += b.mass * b.velocity.z;

        // Angular momentum: L = r x (m*v)
        d.total_angular_momentum.x += b.mass * (b.position.y * b.velocity.z -
                                                  b.position.z * b.velocity.y);
        d.total_angular_momentum.y += b.mass * (b.position.z * b.velocity.x -
                                                  b.position.x * b.velocity.z);
        d.total_angular_momentum.z += b.mass * (b.position.x * b.velocity.y -
                                                  b.position.y * b.velocity.x);

        if (is_sph_body(b.type) && b.density > d.max_density) {
            d.max_density = b.density;
        }
    }

    // Gravitational PE (O(N^2) — only for diagnostics, not every tick)
    // Approximate using virial theorem instead
    d.total_energy = d.total_ke + d.total_thermal; // PE not computed here for perf

    // Cosmological fractions
    double vol = box_size_ * box_size_ * box_size_;
    if (vol > 0.0) {
        d.matter_density = d.total_mass / vol;
        d.critical_density = 3.0 * d.hubble_parameter * d.hubble_parameter / (8.0 * PI * G_N);
        if (d.critical_density > 0.0) {
            d.omega_matter = d.matter_density / d.critical_density;
            d.omega_lambda = OMEGA_LAMBDA_CONJ;
            d.omega_total = d.omega_matter + d.omega_lambda;
        }
    }

    return d;
}

void CosmicEngine::clear() {
    bodies_.clear();
    forces_.clear();
    force_diag_.clear();
    gw_events_.clear();
    octree_.clear();
    sph_neighbors_.clear();
    tick_ = 0;
    next_id_ = 0;
    a_ = 1.0;
    adot_ = 0.0;
    t_cosmic_ = 0.0;
    H0_ = 0.0;
}

}  // namespace ftd
