/**
 * ParticleEngine: Scale 1 simulation
 *
 * Phase 7: Lattice-free engine with continuous positions and analytical forces.
 * Velocity Verlet integration (symplectic → energy-conserving).
 *
 * Force convention (matches Scale 0 Poisson solver ∇²φ = -s):
 *   F_EM   = alpha * q_i * q_j * r_hat / (4*pi * (r² + soft²))
 *   F_grav = G_N * m_i * m_j * r_hat / (r² + soft²)
 *
 * Gravity is always attractive (negative sign).
 * EM: like signs repel (positive), opposite attract (negative).
 */

#include "ftd/particle_engine.h"
#include <algorithm>
#include <cmath>

namespace ftd {

ParticleEngine::ParticleEngine() = default;

int ParticleEngine::add_particle(int8_t charge, Vec3 position, Vec3 velocity,
                                  double mass, double r_eff) {
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = r_eff;
    p.position = position;
    p.velocity = velocity;
    particles_.push_back(p);
    forces_.push_back({});
    return p.id;
}

int ParticleEngine::add_locked_particle(int8_t charge, Vec3 position, double mass) {
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = 2.48;
    p.position = position;
    p.locked = true;
    particles_.push_back(p);
    forces_.push_back({});
    return p.id;
}

Vec3 ParticleEngine::compute_force(int i) const {
    Vec3 f;
    const auto& pi = particles_[i];

    for (int j = 0; j < static_cast<int>(particles_.size()); ++j) {
        if (j == i) continue;
        const auto& pj = particles_[j];

        Vec3 r_vec = pj.position - pi.position;
        double r2 = r_vec.mag2() + soft_ * soft_;  // softened
        double r = std::sqrt(r2);

        if (r < 1e-30) continue;  // degenerate

        Vec3 r_hat = r_vec * (1.0 / r);

        // Coulomb: F = -alpha * qi * qj / (4*pi*r²) * r_hat
        // r_hat points from i toward j.
        // Same signs: qi*qj > 0, force = -positive * r_hat → AWAY from j → repulsion
        // Opposite:   qi*qj < 0, force = +positive * r_hat → TOWARD j → attraction
        double f_em = -ALPHA * pi.charge * pj.charge / (4.0 * PI * r2);
        f += r_hat * f_em;

        // Gravity: F = +G_N * mi * mj / r² * r_hat  (always toward j → attractive)
        if (gravity_enabled_) {
            double f_grav = G_N * pi.mass * pj.mass / r2;
            f += r_hat * f_grav;
        }
    }

    return f;
}

void ParticleEngine::compute_all_forces() {
    forces_.resize(particles_.size());
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        forces_[i] = compute_force(i);
    }
}

void ParticleEngine::half_kick() {
    double half_dt = dt_ * 0.5;
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        if (particles_[i].locked) continue;
        double inv_m = 1.0 / particles_[i].mass;
        particles_[i].velocity += forces_[i] * (half_dt * inv_m);
    }
}

void ParticleEngine::drift() {
    for (auto& p : particles_) {
        if (p.locked) continue;
        p.position += p.velocity * dt_;
    }
}

void ParticleEngine::check_annihilation() {
    // Mark pairs within contact distance (r_eff_i + r_eff_j) with opposite charges
    std::vector<bool> remove(particles_.size(), false);

    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        if (remove[i]) continue;
        for (int j = i + 1; j < static_cast<int>(particles_.size()); ++j) {
            if (remove[j]) continue;

            // Only opposite charges annihilate
            if (particles_[i].charge * particles_[j].charge >= 0) continue;

            Vec3 r_vec = particles_[j].position - particles_[i].position;
            double r = r_vec.mag();
            double contact = particles_[i].r_eff + particles_[j].r_eff;

            if (r < contact) {
                remove[i] = true;
                remove[j] = true;
                break;
            }
        }
    }

    // Remove annihilated particles (reverse order to preserve indices)
    for (int i = static_cast<int>(particles_.size()) - 1; i >= 0; --i) {
        if (remove[i]) {
            particles_.erase(particles_.begin() + i);
            forces_.erase(forces_.begin() + i);
        }
    }
}

void ParticleEngine::enforce_speed_limit() {
    for (auto& p : particles_) {
        if (p.locked) continue;
        double v = p.velocity.mag();
        if (v > C_SPEED) {
            p.velocity *= (C_SPEED / v);
        }
    }
}

void ParticleEngine::apply_damping() {
    if (!damping_enabled_) return;
    double factor = 1.0 - DAMPING * dt_;
    if (factor < 0.0) factor = 0.0;
    for (auto& p : particles_) {
        if (p.locked) continue;
        p.velocity *= factor;
    }
}

void ParticleEngine::tick() {
    // Velocity Verlet:
    // 1. Compute forces at current positions
    compute_all_forces();
    // 2. Half-kick: v += (dt/2) * F/m
    half_kick();
    // 3. Drift: r += dt * v
    drift();
    // 4. Recompute forces at new positions
    compute_all_forces();
    // 5. Half-kick: v += (dt/2) * F_new/m
    half_kick();
    // 6. Store acceleration for diagnostics
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        if (!particles_[i].locked) {
            particles_[i].acceleration = forces_[i] * (1.0 / particles_[i].mass);
        }
    }
    // 7. Annihilation check
    check_annihilation();
    // 8. Speed limit
    enforce_speed_limit();
    // 9. Damping (optional)
    apply_damping();

    ++tick_;
}

void ParticleEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        tick();
    }
}

ParticleDiagnostics ParticleEngine::diagnostics() const {
    ParticleDiagnostics d;
    d.tick = tick_;
    d.particle_count = static_cast<int>(particles_.size());

    // Kinetic energy and momentum
    for (const auto& p : particles_) {
        double v2 = p.velocity.mag2();
        d.total_ke += 0.5 * p.mass * v2;
        d.total_momentum += p.velocity * p.mass;

        // Angular momentum: L = r x (m*v)
        Vec3 mv = p.velocity * p.mass;
        Vec3 L;
        L.x = p.position.y * mv.z - p.position.z * mv.y;
        L.y = p.position.z * mv.x - p.position.x * mv.z;
        L.z = p.position.x * mv.y - p.position.y * mv.x;
        d.total_angular_momentum += L;
    }

    // Potential energy (pairwise)
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(particles_.size()); ++j) {
            Vec3 r_vec = particles_[j].position - particles_[i].position;
            double r = std::sqrt(r_vec.mag2() + soft_ * soft_);

            // Coulomb PE: alpha * qi * qj / (4*pi*r)
            d.total_pe += ALPHA * particles_[i].charge * particles_[j].charge
                        / (4.0 * PI * r);

            // Gravitational PE: -G_N * mi * mj / r
            if (gravity_enabled_) {
                d.total_pe -= G_N * particles_[i].mass * particles_[j].mass / r;
            }
        }
    }

    d.total_energy = d.total_ke + d.total_pe;
    return d;
}

}  // namespace ftd
