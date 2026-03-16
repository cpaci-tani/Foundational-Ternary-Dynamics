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
                                  double mass, double r_eff,
                                  int8_t spin, int8_t color) {
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = r_eff;
    p.position = position;
    p.velocity = velocity;
    p.spin = spin;
    p.color = color;
    // Auto-initialize spin_axis for fermions (z-axis quantization)
    if (spin != 0 && p.spin_axis.mag2() < 1e-30) {
        p.spin_axis = {0.0, 0.0, static_cast<double>(spin)};
    }
    particles_.push_back(p);
    forces_.push_back({});
    return p.id;
}

int ParticleEngine::add_locked_particle(int8_t charge, Vec3 position, double mass,
                                         int8_t spin, int8_t color) {
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = 2.48;
    p.position = position;
    p.locked = true;
    p.spin = spin;
    p.color = color;
    if (spin != 0 && p.spin_axis.mag2() < 1e-30) {
        p.spin_axis = {0.0, 0.0, static_cast<double>(spin)};
    }
    particles_.push_back(p);
    forces_.push_back({});
    return p.id;
}

Vec3 ParticleEngine::compute_force(int i) const {
    Vec3 f;
    const auto& pi = particles_[i];

    // Accumulate per-force diagnostics if buffer is available
    ParticleForceDiag* diag = nullptr;
    if (i < static_cast<int>(force_diag_.size())) {
        diag = &force_diag_[i];  // force_diag_ is mutable, no const_cast needed
        *diag = {};  // zero all components
    }

    for (int j = 0; j < static_cast<int>(particles_.size()); ++j) {
        if (j == i) continue;
        const auto& pj = particles_[j];

        Vec3 r_vec = pj.position - pi.position;
        double r2 = r_vec.mag2() + soft_ * soft_;  // softened
        double r = std::sqrt(r2);

        if (r < 1e-30) continue;  // degenerate

        Vec3 r_hat = r_vec * (1.0 / r);

        // 1. Coulomb: F = -alpha * qi * qj / (4*pi*r²) * r_hat
        if (toggles.coulomb) {
            double f_em = -ALPHA * pi.charge * pj.charge / (4.0 * PI * r2);
            Vec3 fc = r_hat * f_em;
            f += fc;
            if (diag) diag->f_coulomb += fc;
        }

        // 2. Gravity: F = +G_N * mi * mj / r² * r_hat  (always toward j → attractive)
        if (toggles.gravity) {
            double f_grav = G_N * pi.mass * pj.mass / r2;
            Vec3 fg = r_hat * f_grav;
            f += fg;
            if (diag) diag->f_gravity += fg;
        }

        // 3. Exchange (Pauli): same-spin, same-charge repulsion
        if (toggles.exchange && pi.spin != 0 && pj.spin == pi.spin
            && pi.charge == pj.charge) {
            double f_mag = ALPHA_EXCHANGE * std::exp(-r2 / EXCHANGE_RANGE_SQ) / r2;
            Vec3 fe = r_hat * (-f_mag);  // repulsive (away from j)
            f += fe;
            if (diag) diag->f_exchange += fe;
        }

        // 4. Strong: Yukawa + confinement for colored particles
        if (toggles.strong && pi.color != 0 && pj.color != 0) {
            double cf = (pi.color == pj.color) ? 0.5 : -1.0;
            double raw_r = std::sqrt(r_vec.mag2());  // unsoftened
            double F_strong_mag;
            if (raw_r > 1e-30 && raw_r < R_CONFINEMENT) {
                double as = alpha_s_lattice(raw_r);
                F_strong_mag = as * cf / r2;
            } else {
                F_strong_mag = SIGMA_STRING * cf;
            }
            Vec3 fs = r_hat * (-F_strong_mag);
            f += fs;
            if (diag) diag->f_strong += fs;
        }

        // 5. Magnetic dipole-dipole interaction
        if (toggles.magnetic_dipole
            && pi.spin_axis.mag2() > 1e-30 && pj.spin_axis.mag2() > 1e-30) {
            // Magnetic moments: mu = charge/mass * spin_axis (g=2, hbar=1)
            Vec3 mi_mu = pi.spin_axis * (static_cast<double>(pi.charge) / pi.mass);
            Vec3 mj_mu = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);

            double r3 = r * r2;
            double r5 = r3 * r2;
            double mi_dot_r = mi_mu.dot(r_vec);
            double mj_dot_r = mj_mu.dot(r_vec);
            double mi_dot_mj = mi_mu.dot(mj_mu);

            double coeff = 3.0 * ALPHA / (4.0 * PI * r5);
            Vec3 fdd = (r_vec * (5.0 * mi_dot_r * mj_dot_r / r2)
                        - mj_mu * mi_dot_r - mi_mu * mj_dot_r
                        - r_vec * mi_dot_mj) * coeff;
            f += fdd;
            if (diag) diag->f_magnetic_dipole += fdd;
        }

        // 6. Spin-orbit coupling
        if (toggles.spin_orbit && pi.spin_axis.mag2() > 1e-30) {
            Vec3 p_rel = pi.velocity * pi.mass;
            Vec3 L_orb = Vec3::cross(r_vec, p_rel);
            double L_dot_S = L_orb.dot(pi.spin_axis);

            double raw_r = std::sqrt(r_vec.mag2());
            if (raw_r > 1e-15) {
                double r3 = raw_r * raw_r * raw_r;
                double m2c2 = pi.mass * pi.mass * C_SPEED * C_SPEED;
                double coeff_so = ALPHA / (2.0 * m2c2 * r3);
                Vec3 fso = r_hat * (coeff_so * L_dot_S);
                f += fso;
                if (diag) diag->f_spin_orbit += fso;
            }
        }
    }

    // 7. Lorentz force: F = alpha * charge * (v x B_total)
    //    B_total accumulated from magnetic dipole fields of all other particles
    if (toggles.lorentz && pi.velocity.mag2() > 1e-30) {
        Vec3 B_total;
        for (int j = 0; j < static_cast<int>(particles_.size()); ++j) {
            if (j == i) continue;
            const auto& pj = particles_[j];
            if (pj.spin_axis.mag2() < 1e-30) continue;

            Vec3 rv = pj.position - pi.position;
            double rd2 = rv.mag2() + soft_ * soft_;
            double rd = std::sqrt(rd2);
            if (rd < 1e-30) continue;
            Vec3 rh = rv * (1.0 / rd);

            // Magnetic moment of j
            Vec3 mj = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);

            // Dipole B-field: B = (1/(4*pi)) * [3(m.r_hat)r_hat - m] / r^3
            double r3 = rd * rd2;
            double m_dot_rh = mj.dot(rh);
            Vec3 B_j = (rh * (3.0 * m_dot_rh) - mj) * (1.0 / (4.0 * PI * r3));
            B_total += B_j;
        }

        Vec3 fl = Vec3::cross(pi.velocity, B_total) * (ALPHA * pi.charge);
        f += fl;
        if (diag) diag->f_lorentz += fl;
    }

    // 8. Radiation reaction: F_rad = -(2/3) * alpha * q² / (m*c³) * |a_prev|² * v_hat
    if (toggles.radiation && pi.prev_acceleration.mag2() > 1e-30
        && pi.velocity.mag2() > 1e-30) {
        double a2 = pi.prev_acceleration.mag2();
        double q2 = static_cast<double>(pi.charge) * pi.charge;
        double c3 = C_SPEED * C_SPEED * C_SPEED;
        double coeff_rad = -(2.0 / 3.0) * ALPHA * q2 / (pi.mass * c3);
        double v_mag = pi.velocity.mag();
        Vec3 v_hat = pi.velocity * (1.0 / v_mag);
        Vec3 frad = v_hat * (coeff_rad * a2);
        f += frad;
        if (diag) diag->f_radiation += frad;
    }

    // 9. Relativistic correction: F_rel = -(gamma - 1) * F_total  (MUST BE LAST)
    if (toggles.relativistic) {
        double v2 = pi.velocity.mag2();
        double c2 = C_SPEED * C_SPEED;
        double beta2 = v2 / c2;
        if (beta2 > 1e-10 && beta2 < 1.0) {
            double gamma = 1.0 / std::sqrt(1.0 - beta2);
            Vec3 frel = f * (1.0 / gamma - 1.0);
            if (diag) diag->f_relativistic += frel;
            f += frel;  // effectively f = f / gamma
        }
    }

    return f;
}

void ParticleEngine::compute_all_forces() {
    forces_.resize(particles_.size());
    force_diag_.resize(particles_.size());
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
    if (!toggles.damping) return;
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
    // 6. Store previous acceleration (for radiation reaction), then update
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        if (!particles_[i].locked) {
            particles_[i].prev_acceleration = particles_[i].acceleration;
            particles_[i].acceleration = forces_[i] * (1.0 / particles_[i].mass);
        }
    }
    // 7. Annihilation check
    check_annihilation();
    // 8. Speed limit (hard clamp to C_SPEED)
    // NOTE: This breaks the symplectic property of Velocity Verlet — the hard clamp
    // is a non-smooth projection that destroys time-reversibility. This is intentional:
    // C_SPEED is a fundamental lattice constraint (nothing outruns light), and we accept
    // the energy non-conservation at ultra-relativistic speeds. In practice, particles
    // rarely approach C_SPEED so the symplectic break has negligible effect.
    enforce_speed_limit();
    // 9. Damping (optional)
    // NOTE: Post-Verlet damping also breaks symplecticity. When enabled, the integrator
    // is no longer energy-conserving by design — damping models dissipation into the
    // underlying flux substrate. Energy drift is expected and monitored via diagnostics.
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
            if (toggles.gravity) {
                d.total_pe -= G_N * particles_[i].mass * particles_[j].mass / r;
            }
        }
    }

    d.total_energy = d.total_ke + d.total_pe;
    return d;
}

}  // namespace ftd
