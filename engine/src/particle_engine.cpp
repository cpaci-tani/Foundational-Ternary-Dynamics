/**
 * ParticleEngine: Scale 1 simulation
 *
 * Phase 7: Effective, lattice-free engine with continuous positions and
 * analytical forces, advanced once per discrete global tick. Velocity Verlet
 * is symplectic only when damping, radiation, selected contact removal, and the hard speed
 * projection do not intervene.
 *
 * Force convention (matches Scale 0 Poisson solver ∇²φ = -s):
 *   F_EM   = alpha * q_i * q_j * r_hat / (4*pi * (r² + soft²))
 *   F_grav = G_PE * m_i * m_j * r_hat / (r² + soft²)
 *
 * Gravity is always attractive (negative sign).
 * EM: like signs repel (positive), opposite attract (negative).
 */

#include "ftd/particle_engine.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_particle_engine.h"
#endif

namespace ftd {

#ifdef FTD_ENABLE_CUDA
// Opaque wrapper so particle_engine.h doesn't need to include <cuda_runtime.h>.
// Holds the gpu::ParticleEngineGpu and is lazily instantiated on first use.
struct ParticleEngine::GpuBackend {
    gpu::ParticleEngineGpu engine;
};
#endif

OrbitalElements compute_orbital_elements(const Particle& orbiter,
                                          const Particle& center,
                                          double alpha_eff) {
    OrbitalElements oe;

    Vec3 r_vec = orbiter.position - center.position;
    double r = r_vec.mag();
    double v2 = orbiter.velocity.mag2();

    // Specific energy: E/m = 0.5*v² - alpha_eff/r
    oe.specific_energy = 0.5 * v2 - alpha_eff / (r + 1e-30);
    oe.bound = (oe.specific_energy < 0);

    // Specific angular momentum: |L/m| = |r x v|
    Vec3 L = Vec3::cross(r_vec, orbiter.velocity);
    oe.specific_angular_momentum = L.mag();

    if (!oe.bound || alpha_eff < 1e-30) return oe;

    // Semi-major axis: a = -alpha_eff / (2 * E_specific)
    oe.semi_major_axis = -alpha_eff / (2.0 * oe.specific_energy);

    // Eccentricity: e² = 1 - L²/(m * alpha_eff * a)
    // (using specific quantities: e² = 1 - h²/(alpha_eff * a))
    double h2 = oe.specific_angular_momentum * oe.specific_angular_momentum;
    double e2 = 1.0 - h2 / (alpha_eff * oe.semi_major_axis);
    oe.eccentricity = (e2 > 0) ? std::sqrt(e2) : 0.0;

    // Periapsis and apoapsis
    oe.periapsis = oe.semi_major_axis * (1.0 - oe.eccentricity);
    oe.apoapsis  = oe.semi_major_axis * (1.0 + oe.eccentricity);

    // Kepler period: T = 2*pi*sqrt(a³ / alpha_eff)
    double a3 = oe.semi_major_axis * oe.semi_major_axis * oe.semi_major_axis;
    oe.period = 2.0 * PI * std::sqrt(a3 / alpha_eff);

    return oe;
}

ParticleEngine::ParticleEngine() {
    toggles.verified();
    last_relativistic_verlet_ = toggles.relativistic_verlet;
}

// Out-of-line destructor: required so unique_ptr<GpuBackend> can see the
// complete GpuBackend type when deleting. Without this, every translation
// unit that includes particle_engine.h and indirectly calls ~ParticleEngine
// would need to see GpuBackend's definition.
ParticleEngine::~ParticleEngine() = default;

std::uint32_t ParticleEngine::toggle_observation_mask() const {
    std::uint32_t mask = 0;
    std::uint32_t bit = 1;
    for (const auto& spec : PARTICLE_TOGGLE_SPECS) {
        if (toggles.*(spec.field)) mask |= bit;
        bit <<= 1;
    }
    return mask;
}

void ParticleEngine::invalidate_diagnostics_cache() {
    ++observation_revision_;
    diagnostics_cache_valid_ = false;
}

void ParticleEngine::invalidate_observation() {
    invalidate_diagnostics_cache();
    force_diag_ready_ = false;
}

namespace {
bool finite_vec3(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
}

void ParticleEngine::set_dt(double d) {
    if (!std::isfinite(d) || d <= 0.0) {
        throw std::invalid_argument("ParticleEngine dt must be finite and positive");
    }
    dt_ = d;
    invalidate_diagnostics_cache();
}

void ParticleEngine::set_softening(double s) {
    if (!std::isfinite(s) || s < 0.0) {
        throw std::invalid_argument(
            "ParticleEngine softening must be finite and nonnegative");
    }
    soft_ = s;
    invalidate_observation();
}

void ParticleEngine::configure_insulating_box(Vec3 center, Vec3 half_extents) {
    if (!finite_vec3(center) || !finite_vec3(half_extents)
        || half_extents.x <= 0.0 || half_extents.y <= 0.0
        || half_extents.z <= 0.0) {
        throw std::invalid_argument(
            "ParticleEngine insulating box requires finite positive half-extents");
    }
    insulating_box_.enabled = true;
    insulating_box_.center = center;
    insulating_box_.half_extents = half_extents;
    insulating_box_.ports.clear();
    insulator_collision_count_ = 0;
    insulator_port_crossing_count_ = 0;
    cumulative_insulator_impulse_ = {};
    invalidate_diagnostics_cache();
}

void ParticleEngine::add_insulating_port(int axis, int side,
                                          double center_u, double center_v,
                                          double half_u, double half_v,
                                          int required_charge_sign,
                                          int crossing_direction) {
    if (!insulating_box_.enabled) {
        throw std::logic_error(
            "ParticleEngine insulating box must be configured before its ports");
    }
    if (axis < 0 || axis > 2 || (side != -1 && side != 1)
        || !std::isfinite(center_u) || !std::isfinite(center_v)
        || !std::isfinite(half_u) || !std::isfinite(half_v)
        || half_u <= 0.0 || half_v <= 0.0
        || required_charge_sign < -1 || required_charge_sign > 1
        || crossing_direction < -1 || crossing_direction > 1) {
        throw std::invalid_argument("ParticleEngine insulating port is invalid");
    }

    const Vec3& h = insulating_box_.half_extents;
    const double face_half_u = axis == 0 ? h.y : h.x;
    const double face_half_v = axis == 2 ? h.y : h.z;
    if (std::abs(center_u) + half_u > face_half_u
        || std::abs(center_v) + half_v > face_half_v) {
        throw std::invalid_argument(
            "ParticleEngine insulating port must fit within its selected face");
    }
    insulating_box_.ports.push_back(
        {axis, side, center_u, center_v, half_u, half_v,
         required_charge_sign, crossing_direction});
    invalidate_diagnostics_cache();
}

void ParticleEngine::clear_insulating_box() {
    insulating_box_ = {};
    insulator_collision_count_ = 0;
    insulator_port_crossing_count_ = 0;
    cumulative_insulator_impulse_ = {};
    invalidate_diagnostics_cache();
}

bool ParticleEngine::validate_state(std::string* err) const {
    std::string message;
    if (!std::isfinite(dt_) || dt_ <= 0.0) {
        message = "dt must be finite and positive";
    } else if (!std::isfinite(soft_) || soft_ < 0.0) {
        message = "softening must be finite and nonnegative";
    } else if (insulating_box_.enabled
               && (!finite_vec3(insulating_box_.center)
                   || !finite_vec3(insulating_box_.half_extents)
                   || insulating_box_.half_extents.x <= 0.0
                   || insulating_box_.half_extents.y <= 0.0
                   || insulating_box_.half_extents.z <= 0.0)) {
        message = "insulating box must have finite positive half-extents";
    } else if (forces_.size() != particles_.size()) {
        message = "force buffer must match particle record count";
    } else {
        for (const auto& p : particles_) {
            if (!std::isfinite(p.mass) || p.mass <= 0.0) {
                message = "particle mass must be finite and positive";
            } else if (!std::isfinite(p.r_eff) || p.r_eff < 0.0) {
                message = "particle effective radius must be finite and nonnegative";
            } else if (!finite_vec3(p.position) || !finite_vec3(p.velocity)
                       || !finite_vec3(p.acceleration)
                       || !finite_vec3(p.prev_acceleration)
                       || !finite_vec3(p.spin_axis)
                       || !finite_vec3(p.momentum)) {
                message = "particle vector fields must be finite";
            }
            if (!message.empty()) break;
        }
    }
    if (err) *err = message;
    return message.empty();
}

Vec3 ParticleEngine::momentum_from_velocity(const Particle& p) {
    if (p.locked) return {};
    const double v2 = p.velocity.mag2();
    const double c2 = C_SPEED * C_SPEED;
    if (v2 <= 1e-30) return {};
    if (v2 >= c2) {
        throw std::domain_error(
            "ParticleEngine cannot construct finite momentum at or beyond C_SPEED");
    }
    const double gamma = 1.0 / std::sqrt(1.0 - v2 / c2);
    return p.velocity * (p.mass * gamma);
}

double ParticleEngine::kinetic_energy(const Particle& p, bool relativistic) {
    if (p.locked) return 0.0;
    const double v2 = p.velocity.mag2();
    if (!relativistic) return 0.5 * p.mass * v2;
    if (v2 <= 1e-30) return 0.0;
    const double c2 = C_SPEED * C_SPEED;
    if (v2 >= c2) return std::numeric_limits<double>::infinity();
    const double gamma = 1.0 / std::sqrt(1.0 - v2 / c2);
    return (gamma - 1.0) * p.mass * c2;
}

void ParticleEngine::synchronize_momentum_from_velocity() {
    for (auto& p : particles_) {
        p.momentum = momentum_from_velocity(p);
    }
}

int ParticleEngine::add_particle(int8_t charge, Vec3 position, Vec3 velocity,
                                  double mass, double r_eff,
                                  int8_t spin, int8_t color) {
    if (!finite_vec3(position) || !finite_vec3(velocity)) {
        throw std::invalid_argument(
            "ParticleEngine particle position and velocity must be finite");
    }
    if (!std::isfinite(mass) || mass <= 0.0) {
        throw std::invalid_argument("ParticleEngine particle mass must be finite and positive");
    }
    if (!std::isfinite(r_eff) || r_eff < 0.0) {
        throw std::invalid_argument(
            "ParticleEngine particle effective radius must be finite and nonnegative");
    }
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = r_eff;
    p.position = position;
    p.velocity = velocity;
    const double requested_speed = p.velocity.mag();
    if (requested_speed >= C_SPEED) {
        // The effective record has no admissible superluminal state. Project at
        // the API boundary and mark the run ineligible for a conservation-drift
        // claim instead of allowing an invalid record into a transaction.
        const double capped = std::nextafter(C_SPEED, 0.0);
        p.velocity *= capped / requested_speed;
        ++speed_projection_count_;
    }
    p.spin = spin;
    p.color = color;
    // Auto-initialize spin_axis for fermions (z-axis quantization)
    if (spin != 0 && p.spin_axis.mag2() < 1e-30) {
        p.spin_axis = {0.0, 0.0, static_cast<double>(spin)};
    }
    p.momentum = momentum_from_velocity(p);
    p.provenance.source_object_id = p.id;
    particles_.push_back(p);
    forces_.push_back({});
    invalidate_observation();
    return p.id;
}

int ParticleEngine::add_locked_particle(int8_t charge, Vec3 position, double mass,
                                         int8_t spin, int8_t color) {
    if (!finite_vec3(position)) {
        throw std::invalid_argument("ParticleEngine locked-particle position must be finite");
    }
    if (!std::isfinite(mass) || mass <= 0.0) {
        throw std::invalid_argument(
            "ParticleEngine locked-particle mass must be finite and positive");
    }
    Particle p;
    p.id = next_id_++;
    p.charge = charge;
    p.mass = mass;
    p.r_eff = R_EFF_DEFAULT;
    p.position = position;
    p.locked = true;
    p.spin = spin;
    p.color = color;
    if (spin != 0 && p.spin_axis.mag2() < 1e-30) {
        p.spin_axis = {0.0, 0.0, static_cast<double>(spin)};
    }
    p.momentum = {0.0, 0.0, 0.0};
    p.provenance.source_object_id = p.id;
    p.provenance.source_kind = "imposed_kinematic_anchor";
    particles_.push_back(p);
    forces_.push_back({});
    invalidate_observation();
    return p.id;
}

bool ParticleEngine::set_particle_velocity(int id, Vec3 velocity) {
    if (!finite_vec3(velocity)) return false;
    for (auto& p : particles_) {
        if (p.id != id) continue;
        const double speed = velocity.mag();
        if (speed >= C_SPEED) {
            const double capped = std::nextafter(C_SPEED, 0.0);
            velocity *= capped / speed;
            ++speed_projection_count_;
        }
        p.velocity = p.locked ? Vec3{} : velocity;
        p.momentum = momentum_from_velocity(p);
        invalidate_observation();
        return true;
    }
    return false;
}

namespace {
// Shared Coulomb + gravity accumulation (revision 2.4 dedup): the pairwise
// loop and the Barnes-Hut monopole branch computed identical expressions,
// differing only in the source terms (per-particle charge/mass vs the
// node's aggregated totals). Exact operation order preserved — int8 charges
// convert to double losslessly at the call site.
inline void accumulate_coulomb_gravity(const ParticleToggles& toggles,
                                       double pi_charge, double pi_mass,
                                       double src_charge, double src_mass,
                                       const Vec3& r_hat, double r2,
                                       Vec3& f, ParticleForceDiag* diag) {
    // Coulomb: F = -alpha * qi * qj / (4*pi*r²) * r_hat
    if (toggles.coulomb) {
        double f_em = -ALPHA_EFT * pi_charge * src_charge / (4.0 * PI * r2);  // EFT: G_C²
        Vec3 fc = r_hat * f_em;
        f += fc;
        if (diag) diag->f_coulomb += fc;
    }
    // Gravity: F = +G_PE * mi * mj / r² * r_hat  (FTD-0131 physical coupling)
    if (toggles.gravity) {
        double f_grav = G_PE * pi_mass * src_mass / r2;
        Vec3 fg = r_hat * f_grav;
        f += fg;
        if (diag) diag->f_gravity += fg;
    }
}

inline void apply_force_postprocessing(const ParticleToggles& toggles,
                                       const Particle& p, Vec3& force,
                                       ParticleForceDiag* diag) {
    // Radiation reaction (self-interaction, not pairwise).
    if (toggles.radiation && p.prev_acceleration.mag2() > 1e-30
        && p.velocity.mag2() > 1e-30) {
        const double a2 = p.prev_acceleration.mag2();
        const double q2 = static_cast<double>(p.charge) * p.charge;
        const double c3 = C_SPEED * C_SPEED * C_SPEED;
        const double coeff_rad = -(2.0 / 3.0) * ALPHA * q2 / (p.mass * c3);
        const Vec3 v_hat = p.velocity * (1.0 / p.velocity.mag());
        const Vec3 f_rad = v_hat * (coeff_rad * a2);
        force += f_rad;
        if (diag) diag->f_radiation += f_rad;
    }
}
}  // namespace

Vec3 ParticleEngine::compute_pairwise_force(int i, int j) const {
    Vec3 f;
    const auto& pi = particles_[i];
    const auto& pj = particles_[j];

    ParticleForceDiag* diag = nullptr;
    if (i < static_cast<int>(force_diag_.size())) {
        diag = &force_diag_[i];
    }

        Vec3 r_vec = pj.position - pi.position;
        double r2 = r_vec.mag2() + soft_ * soft_;  // softened
        double r = std::sqrt(r2);
        if (r < 1e-30) return f;  // degenerate

        Vec3 r_hat = r_vec * (1.0 / r);

        // 1+2. Coulomb + gravity (shared with the Barnes-Hut monopole branch).
        accumulate_coulomb_gravity(toggles, pi.charge, pi.mass,
                                   pj.charge, pj.mass, r_hat, r2, f, diag);

        // 3. Exchange (Pauli): same-spin, same-charge repulsion
        if (toggles.exchange && pi.spin != 0 && pj.spin == pi.spin
            && pi.charge == pj.charge) {
            double f_mag = ALPHA_EXCHANGE * std::exp(-r2 / EXCHANGE_RANGE_SQ) / r2;
            Vec3 fe = r_hat * (-f_mag);  // repulsive (away from j)
            f += fe;
            if (diag) diag->f_exchange += fe;
        }

        // 4. Strong: running alpha_s + confinement for colored particles
        // Uses unsoftened distance for both coupling AND force denominator
        // to maintain physical consistency (matches GPU kernel convention).
        if (toggles.strong && pi.color != 0 && pj.color != 0) {
            double cf = (pi.color == pj.color) ? 0.5 : -1.0;
            double raw_r = std::sqrt(r_vec.mag2());  // unsoftened
            if (raw_r < 1.0) raw_r = 1.0;            // clamp to lattice spacing
            double raw_r2 = raw_r * raw_r;
            double F_strong_mag;
            if (raw_r < 3.0) {
                double as = alpha_s_lattice(raw_r);
                F_strong_mag = as * cf / raw_r2;       // Coulomb regime
            } else if (raw_r < 8.0) {
                double as = alpha_s_lattice(raw_r);
                F_strong_mag = as * cf / (3.0 * raw_r); // Transition
            } else {
                F_strong_mag = SIGMA_STRING * cf;        // Linear confinement
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

            double coeff = 3.0 * ALPHA_EFT / (4.0 * PI * r5);  // EFT: G_C²
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

    // 7. Lorentz force
    if (toggles.lorentz && pi.velocity.mag2() > 1e-30 && pj.spin_axis.mag2() > 1e-30) {
        Vec3 rv = pj.position - pi.position;
        double rd2 = rv.mag2() + soft_ * soft_;
        double rd = std::sqrt(rd2);
        if (rd > 1e-30) {
            Vec3 rh = rv * (1.0 / rd);
            Vec3 mj = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);
            double r3 = rd * rd2;
            double m_dot_rh = mj.dot(rh);
            Vec3 B_j = (rh * (3.0 * m_dot_rh) - mj) * (1.0 / (4.0 * PI * r3));
            Vec3 fl = Vec3::cross(pi.velocity, B_j) * (ALPHA * pi.charge);
            f += fl;
            if (diag) diag->f_lorentz += fl;
        }
    }

    return f;
}

Vec3 ParticleEngine::tree_force(int i, int node_idx) const {
    const BarnesHutNode& node = octree_.nodes[node_idx];
    const auto& pi = particles_[i];
    
    // Skip empty nodes
    if (node.total_mass <= 0.0 && node.total_charge == 0.0) return {};

    Vec3 r_vec = node.center_of_mass - pi.position;
    double r2 = r_vec.mag2() + soft_ * soft_;
    double r = std::sqrt(r2);

    if (node.is_leaf) {
        if (node.body_indices.empty()) return {};
        Vec3 lf;
        for (int b_idx : node.body_indices) {
            if (b_idx == i) continue;
            lf += compute_pairwise_force(i, b_idx);
        }
        return lf;
    }

    // Barnes-Hut opening angle test (THETA_BH = 0.5)
    if (node.width() / r < 0.5) {
        // Far away: monopole approximation ONLY for 1/r^2 forces (Gravity, Coulomb)
        // Short-range forces (strong, exchange) and higher-order moments (dipole, spin-orbit)
        // are perfectly negligible at these macroscopic cutoff distances.
        Vec3 r_hat = r_vec * (1.0 / r);
        Vec3 f;
        ParticleForceDiag* diag = nullptr;
        if (i < static_cast<int>(force_diag_.size())) diag = &force_diag_[i];
        
        accumulate_coulomb_gravity(toggles, pi.charge, pi.mass,
                                   node.total_charge, node.total_mass,
                                   r_hat, r2, f, diag);
        return f;
    }

    // Recurse into children
    Vec3 force = {};
    for (int c = 0; c < 8; ++c) {
        if (node.children[c] >= 0) {
            Vec3 cf = tree_force(i, node.children[c]);
            force.x += cf.x;
            force.y += cf.y;
            force.z += cf.z;
        }
    }
    return force;
}

Vec3 ParticleEngine::compute_force(int i) const {
    Vec3 f;
    for (int j = 0; j < static_cast<int>(particles_.size()); ++j) {
        if (i == j) continue;
        f += compute_pairwise_force(i, j);
    }

    // Apply the same non-pairwise radiation term used by the transaction path
    // so direct force diagnostics and integrated forces agree. The former
    // isotropic "relativistic force" rescale was non-covariant and is retired;
    // relativistic behavior belongs exclusively to momentum-Verlet.
    const auto& pi = particles_[i];

    ParticleForceDiag* diag = i < static_cast<int>(force_diag_.size())
        ? &force_diag_[i] : nullptr;
    apply_force_postprocessing(toggles, pi, f, diag);

    return f;
}

ParticleForceDiag ParticleEngine::compute_force_diagnostic(int i) const {
    if (i < 0 || i >= static_cast<int>(particles_.size())) return {};
    if (force_diag_.size() < particles_.size()) {
        force_diag_.resize(particles_.size());
    }
    force_diag_[i] = {};
    (void)compute_force(i);
    const auto out = force_diag_[i];
    force_diag_ready_ = false;
    return out;
}

ParticleForceDiag ParticleEngine::compute_pair_force_diagnostic(int i, int j) const {
    if (i < 0 || j < 0 || i == j ||
        i >= static_cast<int>(particles_.size()) ||
        j >= static_cast<int>(particles_.size())) return {};
    if (force_diag_.size() < particles_.size()) {
        force_diag_.resize(particles_.size());
    }
    force_diag_[i] = {};
    (void)compute_pairwise_force(i, j);
    const auto out = force_diag_[i];
    force_diag_ready_ = false;
    return out;
}

void ParticleEngine::compute_all_forces() {
    forces_.resize(particles_.size());
    force_diag_.resize(particles_.size());

    // =========================================================================
    // Wave 5.4 Phase 1: GPU pair-force fast path
    // =========================================================================
    // When use_gpu_ is on AND CUDA is compiled in, upload particles to the
    // device and run an O(N²) CUDA kernel for Coulomb + Newtonian gravity.
    // Extended toggles (strong, exchange, lorentz, magnetic_dipole,
    // spin_orbit) and non-pairwise post-processing (radiation)
    // require state that isn't on the device yet, so we fall back to the
    // CPU Barnes-Hut path whenever any of them are on.
    //
    // Also fall back for tiny systems (< 8 particles) where upload/download
    // overhead dwarfs kernel work.
    bool gpu_pair_handled = false;
#ifdef FTD_ENABLE_CUDA
    const bool advanced_toggles_on =
        toggles.strong || toggles.exchange || toggles.lorentz ||
        toggles.magnetic_dipole || toggles.spin_orbit || toggles.radiation;
    if (use_gpu_ && !advanced_toggles_on && particles_.size() >= 8) {
        if (!gpu_backend_) {
            gpu_backend_ = std::make_unique<GpuBackend>();
        }
        gpu_backend_->engine.compute_pair_forces(
            particles_, toggles, soft_, forces_, force_diag_);
        gpu_pair_handled = true;
    }
#endif

    // Build O(N log N) spatial partition tree — still needed for the CPU
    // Barnes-Hut fallback path (radiation/strong/etc.) and
    // for tests that explicitly disable GPU.
    if (!gpu_pair_handled) {
        octree_.build(particles_,
            [](const Particle& p) { return p.position; },
            [](const Particle& p) { return p.mass; },
            [](const Particle& p) { return static_cast<double>(p.charge); }
        );
    }

    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        const auto& pi = particles_[i];

        Vec3 f;
        if (gpu_pair_handled) {
            // Pair forces already populated in forces_[i] by the GPU path.
            // force_diag_[i].f_coulomb / f_gravity already populated.
            f = forces_[i];
        } else {
            force_diag_[i] = {}; // Zero all diagnostics for the tick
            if (octree_.root >= 0) {
                f = tree_force(i, octree_.root);
            }
        }

        ParticleForceDiag* diag = &force_diag_[i];

        apply_force_postprocessing(toggles, pi, f, diag);

        forces_[i] = f;
    }
    force_diag_ready_ = true;
    force_diag_toggle_mask_ = toggle_observation_mask();
}

const std::vector<ParticleForceDiag>& ParticleEngine::observation_force_diag() {
    const bool profile_changed = force_diag_toggle_mask_ != toggle_observation_mask();
    if (!force_diag_ready_ || profile_changed
        || force_diag_.size() != particles_.size()) {
        compute_all_forces();
    }
    return force_diag_;
}

void ParticleEngine::half_kick() {
    double half_dt = dt_ * 0.5;
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        if (particles_[i].locked) continue;
        if (toggles.relativistic_verlet) {
            // Update relativistic momentum: p += F * dt/2
            particles_[i].momentum += forces_[i] * half_dt;
            // Update velocity from momentum: v = p / sqrt(m^2 + p^2/c^2)
            double p2 = particles_[i].momentum.mag2();
            double m2 = particles_[i].mass * particles_[i].mass;
            double c2 = C_SPEED * C_SPEED;
            particles_[i].velocity = particles_[i].momentum * (1.0 / std::sqrt(m2 + p2 / c2));
        } else {
            double inv_m = 1.0 / particles_[i].mass;
            particles_[i].velocity += forces_[i] * (half_dt * inv_m);
        }
    }
}

void ParticleEngine::drift() {
    for (auto& p : particles_) {
        if (p.locked) continue;
        if (insulating_box_.enabled) drift_with_insulating_box(p);
        else p.position += p.velocity * dt_;
    }
}

namespace {
double vec_component(const Vec3& v, int axis) {
    if (axis == 0) return v.x;
    if (axis == 1) return v.y;
    return v.z;
}

struct InsulatingBoxHit {
    bool hit = false;
    bool started_inside = false;
    double t = 0.0;
    int axis = -1;
    int side = 0;
    Vec3 normal;
};

InsulatingBoxHit first_box_surface_hit(const Vec3& start,
                                       const Vec3& displacement,
                                       const ParticleInsulatingBox& box,
                                       double epsilon) {
    const Vec3 local = start - box.center;
    const Vec3& h = box.half_extents;
    // Resolved encounters are nudged by epsilon to the retained side. Test
    // membership against the mathematical surface itself so an inside nudge
    // is not misclassified as exterior and reflected a second time at t=0.
    const bool inside = std::abs(local.x) <= h.x
        && std::abs(local.y) <= h.y
        && std::abs(local.z) <= h.z;

    InsulatingBoxHit result;
    result.started_inside = inside;
    if (inside) {
        double best_t = std::numeric_limits<double>::infinity();
        for (int axis = 0; axis < 3; ++axis) {
            const double p = vec_component(local, axis);
            const double d = vec_component(displacement, axis);
            const double extent = vec_component(h, axis);
            if (std::abs(d) <= epsilon) continue;
            const int side = d > 0.0 ? 1 : -1;
            const double t = (side * extent - p) / d;
            if (t >= -epsilon && t <= 1.0 + epsilon && t < best_t) {
                best_t = t;
                result.axis = axis;
                result.side = side;
            }
        }
        if (result.axis >= 0) {
            result.hit = true;
            result.t = std::clamp(best_t, 0.0, 1.0);
        }
    } else {
        double t_enter = -std::numeric_limits<double>::infinity();
        double t_exit = std::numeric_limits<double>::infinity();
        int enter_axis = -1;
        int enter_side = 0;
        for (int axis = 0; axis < 3; ++axis) {
            const double p = vec_component(local, axis);
            const double d = vec_component(displacement, axis);
            const double extent = vec_component(h, axis);
            if (std::abs(d) <= epsilon) {
                if (p < -extent || p > extent) return result;
                continue;
            }
            double near_t = (-extent - p) / d;
            double far_t = (extent - p) / d;
            int near_side = -1;
            if (near_t > far_t) {
                std::swap(near_t, far_t);
                near_side = 1;
            }
            if (near_t > t_enter) {
                t_enter = near_t;
                enter_axis = axis;
                enter_side = near_side;
            }
            t_exit = std::min(t_exit, far_t);
            if (t_enter > t_exit) return result;
        }
        if (enter_axis >= 0 && t_enter >= -epsilon && t_enter <= 1.0 + epsilon
            && t_exit >= -epsilon) {
            result.hit = true;
            result.t = std::clamp(t_enter, 0.0, 1.0);
            result.axis = enter_axis;
            result.side = enter_side;
        }
    }

    if (result.axis == 0) result.normal.x = static_cast<double>(result.side);
    else if (result.axis == 1) result.normal.y = static_cast<double>(result.side);
    else if (result.axis == 2) result.normal.z = static_cast<double>(result.side);
    return result;
}
}

bool ParticleEngine::port_allows_crossing(const Particle& p, int axis, int side,
                                           bool started_inside,
                                           const Vec3& hit_position) const {
    const Vec3 local = hit_position - insulating_box_.center;
    double u = 0.0;
    double v = 0.0;
    if (axis == 0) { u = local.y; v = local.z; }
    else if (axis == 1) { u = local.x; v = local.z; }
    else { u = local.x; v = local.y; }

    for (const auto& port : insulating_box_.ports) {
        if (port.axis != axis || port.side != side) continue;
        const int charge_sign = (p.charge > 0) - (p.charge < 0);
        if (port.required_charge_sign != 0
            && charge_sign != port.required_charge_sign) continue;
        const int crossing_direction = started_inside ? 1 : -1;
        if (port.crossing_direction != 0
            && crossing_direction != port.crossing_direction) continue;
        const double clear_u = port.half_u - p.r_eff;
        const double clear_v = port.half_v - p.r_eff;
        if (clear_u < 0.0 || clear_v < 0.0) continue;
        if (std::abs(u - port.center_u) <= clear_u
            && std::abs(v - port.center_v) <= clear_v) return true;
    }
    return false;
}

void ParticleEngine::drift_with_insulating_box(Particle& p) {
    Vec3 start = p.position;
    Vec3 displacement = p.velocity * dt_;
    const double scale = std::max({insulating_box_.half_extents.x,
                                   insulating_box_.half_extents.y,
                                   insulating_box_.half_extents.z, 1.0});
    const double epsilon = scale * 1.0e-10;

    // C_SPEED and ordinary Scale-1 time steps permit at most one encounter,
    // but keep the sweep iterative so unusually large test/setup steps cannot
    // tunnel through multiple faces in one transaction.
    constexpr int kMaxSurfaceEncounters = 16;
    for (int encounter = 0; encounter < kMaxSurfaceEncounters; ++encounter) {
        if (displacement.mag2() <= epsilon * epsilon) {
            p.position = start;
            return;
        }
        const auto hit = first_box_surface_hit(
            start, displacement, insulating_box_, epsilon);
        if (!hit.hit) {
            p.position = start + displacement;
            return;
        }

        const Vec3 hit_position = start + displacement * hit.t;
        Vec3 remaining = displacement * (1.0 - hit.t);
        const double crossing_direction = hit.started_inside ? 1.0 : -1.0;
        if (port_allows_crossing(
                p, hit.axis, hit.side, hit.started_inside, hit_position)) {
            ++insulator_port_crossing_count_;
            start = hit_position + hit.normal * (crossing_direction * epsilon);
            displacement = remaining;
            continue;
        }

        const Vec3 momentum_before = toggles.relativistic_verlet
            ? p.momentum : p.velocity * p.mass;
        p.velocity -= hit.normal * (2.0 * p.velocity.dot(hit.normal));
        p.momentum -= hit.normal * (2.0 * p.momentum.dot(hit.normal));
        const Vec3 momentum_after = toggles.relativistic_verlet
            ? p.momentum : p.velocity * p.mass;
        cumulative_insulator_impulse_ += momentum_after - momentum_before;
        ++insulator_collision_count_;

        remaining -= hit.normal * (2.0 * remaining.dot(hit.normal));
        const double retained_side = hit.started_inside ? -1.0 : 1.0;
        start = hit_position + hit.normal * (retained_side * epsilon);
        displacement = remaining;
    }

    // Fail closed if an extreme step exhausts the bounded sweep. The record
    // remains on the last resolved side of the insulating surface.
    p.position = start;
}

void ParticleEngine::append_event(Scale1EventRecord event) {
    event.sequence = next_event_sequence_++;
    events_.push_back(std::move(event));
    constexpr std::size_t kMaxEventHistory = 1024;
    if (events_.size() > kMaxEventHistory) {
        events_.erase(events_.begin(),
                      events_.begin() + (events_.size() - kMaxEventHistory));
    }
}

void ParticleEngine::process_contact_events() {
    if (!toggles.contact_events || particles_.size() < 2) return;

    struct Candidate {
        double distance = 0.0;
        int i = -1;
        int j = -1;
        int id_a = -1;
        int id_b = -1;
    };
    std::vector<Candidate> candidates;
    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(particles_.size()); ++j) {
            if (particles_[i].charge * particles_[j].charge >= 0) continue;
            const double r = (particles_[j].position - particles_[i].position).mag();
            if (r >= particles_[i].r_eff + particles_[j].r_eff) continue;
            candidates.push_back({r, i, j,
                                  std::min(particles_[i].id, particles_[j].id),
                                  std::max(particles_[i].id, particles_[j].id)});
        }
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) {
                  if (a.distance != b.distance) return a.distance < b.distance;
                  if (a.id_a != b.id_a) return a.id_a < b.id_a;
                  return a.id_b < b.id_b;
              });

    const ParticleDiagnostics before = diagnostics();
    std::vector<bool> remove(particles_.size(), false);
    std::vector<Candidate> selected;
    for (const auto& candidate : candidates) {
        if (remove[candidate.i] || remove[candidate.j]) continue;
        remove[candidate.i] = true;
        remove[candidate.j] = true;
        selected.push_back(candidate);
    }
    if (selected.empty()) return;

    for (int i = static_cast<int>(particles_.size()) - 1; i >= 0; --i) {
        if (!remove[i]) continue;
        particles_.erase(particles_.begin() + i);
        forces_.erase(forces_.begin() + i);
        if (i < static_cast<int>(force_diag_.size())) {
            force_diag_.erase(force_diag_.begin() + i);
        }
    }
    // Contact removal changes both the exact energy ledger and the force
    // population before this transaction completes.
    invalidate_observation();
    for (auto& p : particles_) {
        if (p.pair_id < 0) continue;
        const auto alive = std::find_if(particles_.begin(), particles_.end(),
            [&p](const Particle& candidate) { return candidate.id == p.pair_id; });
        if (alive == particles_.end()) p.pair_id = -1;
    }

    const ParticleDiagnostics after = diagnostics();
    const double batch_delta = after.total_energy - before.total_energy;
    cumulative_contact_delta_ += batch_delta;
    contact_event_count_ += selected.size();
    for (const auto& contact : selected) {
        Scale1EventRecord event;
        event.tick = tick_ + 1;
        event.type = Scale1EventType::ContactRemoval;
        event.participant_a = contact.id_a;
        event.participant_b = contact.id_b;
        event.state_energy_delta = selected.size() == 1 ? batch_delta : 0.0;
        event.accounting_complete = before.state_energy_complete
            && after.state_energy_complete && selected.size() == 1;
        event.status = Scale1EpistemicStatus::Selection;
        event.source_id = "contact_events";
        append_event(std::move(event));
    }
}

void ParticleEngine::enforce_speed_limit() {
    for (auto& p : particles_) {
        if (p.locked) continue;
        const double v = p.velocity.mag();
        if (v < C_SPEED) continue;
        const double before = kinetic_energy(p, toggles.relativistic_verlet);
        p.velocity *= std::nextafter(C_SPEED, 0.0) / v;
        const double after = kinetic_energy(p, toggles.relativistic_verlet);
        if (std::isfinite(before) && before > after) {
            cumulative_speed_projection_sink_ += before - after;
        }
        ++speed_projection_count_;
    }
}

void ParticleEngine::apply_damping() {
    if (!toggles.damping) return;
    double factor = 1.0 - DAMPING * dt_;
    if (factor < 0.0) factor = 0.0;
    for (auto& p : particles_) {
        if (p.locked) continue;
        const double before = kinetic_energy(p, toggles.relativistic_verlet);
        p.velocity *= factor;
        const double after = kinetic_energy(p, toggles.relativistic_verlet);
        if (std::isfinite(before) && before > after) {
            cumulative_damping_sink_ += before - after;
        }
    }
}

void ParticleEngine::evolve_spin_axes() {
    if (!toggles.magnetic_dipole && !toggles.lorentz) return;

    const int n = static_cast<int>(particles_.size());
    for (int i = 0; i < n; ++i) {
        auto& pi = particles_[i];
        if (pi.locked || pi.spin_axis.mag2() < 1e-30) continue;
        if (std::abs(pi.charge) < 1e-30 || pi.mass < 1e-30) continue;

        Vec3 B = {};
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            const auto& pj = particles_[j];
            if (pj.spin_axis.mag2() < 1e-30) continue;
            if (std::abs(pj.charge) < 1e-30 || pj.mass < 1e-30) continue;

            Vec3 mu_j = pj.spin_axis * (static_cast<double>(pj.charge) / pj.mass);
            Vec3 r_vec = pi.position - pj.position;
            double r2 = r_vec.mag2() + soft_ * soft_;
            double r = std::sqrt(r2);
            if (r < 1e-30) continue;
            double r3 = r * r2;
            Vec3 r_hat = r_vec * (1.0 / r);
            double mu_dot_r = mu_j.dot(r_hat);
            B += (r_hat * (3.0 * mu_dot_r) - mu_j) * (1.0 / (4.0 * PI * r3));
        }

        if (B.mag2() < 1e-60) continue;

        const double S_mag = pi.spin_axis.mag();
        const double gamma = static_cast<double>(pi.charge) / pi.mass;
        Vec3 dS = Vec3::cross(pi.spin_axis, B) * (gamma * dt_);
        pi.spin_axis += dS;
        const double new_mag = pi.spin_axis.mag();
        if (new_mag > 1e-30) pi.spin_axis *= (S_mag / new_mag);
    }
}

void ParticleEngine::tick() {
    std::string toggle_error;
    if (!toggles.validate(&toggle_error)) {
        throw std::logic_error("ParticleEngine invalid toggle profile: " + toggle_error);
    }
    std::string state_error;
    if (!validate_state(&state_error)) {
        throw std::logic_error("ParticleEngine invalid pre-tick state: " + state_error);
    }
    // Internal event accounting may request diagnostics after the drift. It
    // must never see the previous externally cached observation.
    invalidate_diagnostics_cache();

    // A tick is one state-complete transaction. Keep a bounded in-memory undo
    // image so an invalid post-state cannot leak half a schedule to a caller.
    const auto particles_before = particles_;
    const auto forces_before = forces_;
    const auto diag_before = force_diag_;
    const auto events_before = events_;
    const auto next_event_before = next_event_sequence_;
    const auto contact_count_before = contact_event_count_;
    const auto speed_count_before = speed_projection_count_;
    const auto insulator_collision_before = insulator_collision_count_;
    const auto insulator_crossing_before = insulator_port_crossing_count_;
    const Vec3 insulator_impulse_before = cumulative_insulator_impulse_;
    const double damping_before = cumulative_damping_sink_;
    const double radiation_before = cumulative_radiation_sink_;
    const double speed_sink_before = cumulative_speed_projection_sink_;
    const double contact_delta_before = cumulative_contact_delta_;
    const bool integrator_before = last_relativistic_verlet_;

    try {
        enforce_speed_limit();
        if (last_relativistic_verlet_ != toggles.relativistic_verlet) {
            synchronize_momentum_from_velocity();
            last_relativistic_verlet_ = toggles.relativistic_verlet;
        }

        compute_all_forces();
        half_kick();
        enforce_speed_limit();
        drift();
        compute_all_forces();
        half_kick();

        for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
            if (particles_[i].locked) continue;
            particles_[i].prev_acceleration = particles_[i].acceleration;
            particles_[i].acceleration = forces_[i] * (1.0 / particles_[i].mass);
        }

        process_contact_events();
        enforce_speed_limit();
        apply_damping();
        evolve_spin_axes();
        synchronize_momentum_from_velocity();

        if (!validate_state(&state_error)) {
            throw std::logic_error("ParticleEngine invalid post-tick state: " + state_error);
        }
        ++tick_;
        // The second force phase is already the integrator-aligned force
        // observation for this tick. Invalidate only the exact energy ledger.
        invalidate_diagnostics_cache();
    } catch (...) {
        particles_ = particles_before;
        forces_ = forces_before;
        force_diag_ = diag_before;
        events_ = events_before;
        next_event_sequence_ = next_event_before;
        contact_event_count_ = contact_count_before;
        speed_projection_count_ = speed_count_before;
        insulator_collision_count_ = insulator_collision_before;
        insulator_port_crossing_count_ = insulator_crossing_before;
        cumulative_insulator_impulse_ = insulator_impulse_before;
        cumulative_damping_sink_ = damping_before;
        cumulative_radiation_sink_ = radiation_before;
        cumulative_speed_projection_sink_ = speed_sink_before;
        cumulative_contact_delta_ = contact_delta_before;
        last_relativistic_verlet_ = integrator_before;
        force_diag_ready_ = false;
        diagnostics_cache_valid_ = false;
        throw;
    }
}

void ParticleEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        tick();
    }
}

ParticleDiagnostics ParticleEngine::diagnostics() const {
    const auto toggle_mask = toggle_observation_mask();
    if (diagnostics_cache_valid_
        && diagnostics_revision_ == observation_revision_
        && diagnostics_toggle_mask_ == toggle_mask) {
        return diagnostics_cache_;
    }
    ParticleDiagnostics d;
    d.tick = tick_;
    d.particle_count = static_cast<int>(particles_.size());
    d.insulator_collision_count = insulator_collision_count_;
    d.insulator_port_crossing_count = insulator_port_crossing_count_;
    d.cumulative_insulator_impulse = cumulative_insulator_impulse_;

    double total_mass = 0.0;
    for (const auto& p : particles_) {
        d.total_ke += kinetic_energy(p, toggles.relativistic_verlet);
        const Vec3 momentum = toggles.relativistic_verlet
            ? p.momentum : p.velocity * p.mass;
        d.total_momentum += momentum;
        d.total_angular_momentum += Vec3::cross(p.position, momentum);
        d.center_of_mass += p.position * p.mass;
        total_mass += p.mass;
    }
    if (total_mass > 0.0) d.center_of_mass *= 1.0 / total_mass;

    for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(particles_.size()); ++j) {
            Vec3 r_vec = particles_[j].position - particles_[i].position;
            double r = std::sqrt(r_vec.mag2() + soft_ * soft_);
            if (r < 1e-30) continue;

            if (toggles.coulomb) {
                d.coulomb_pe += ALPHA_EFT * particles_[i].charge * particles_[j].charge
                              / (4.0 * PI * r);
            }
            if (toggles.gravity) {
                d.gravity_pe -= G_PE * particles_[i].mass * particles_[j].mass / r;
            }
        }
    }

    d.total_pe = d.coulomb_pe + d.gravity_pe;
    d.total_energy = d.total_ke + d.total_pe;

    d.covered_mask = scale1_bit(Scale1Coverage::Kinetic);
    if (toggles.coulomb) {
        d.covered_mask |= scale1_bit(Scale1Coverage::CoulombPotential);
    }
    if (toggles.gravity) {
        d.covered_mask |= scale1_bit(Scale1Coverage::GravityPotential);
    }
    if (toggles.damping) {
        d.covered_mask |= scale1_bit(Scale1Coverage::DampingSink);
        d.nonconservative_mask |= scale1_bit(Scale1Coverage::DampingSink);
    }
    if (toggles.contact_events) {
        d.covered_mask |= scale1_bit(Scale1Coverage::ContactEvents);
        d.nonconservative_mask |= scale1_bit(Scale1Coverage::ContactEvents);
    }
    if (speed_projection_count_ > 0) {
        d.covered_mask |= scale1_bit(Scale1Coverage::SpeedProjectionSink);
        d.nonconservative_mask |= scale1_bit(Scale1Coverage::SpeedProjectionSink);
    }

    const auto mark_missing = [&d](bool enabled, Scale1Coverage coverage,
                                   bool nonconservative) {
        if (!enabled) return;
        d.missing_mask |= scale1_bit(coverage);
        if (nonconservative) d.nonconservative_mask |= scale1_bit(coverage);
    };
    mark_missing(toggles.lorentz, Scale1Coverage::LorentzFieldEnergy, false);
    mark_missing(toggles.exchange, Scale1Coverage::ExchangePotential, false);
    mark_missing(toggles.strong, Scale1Coverage::StrongPotential, false);
    mark_missing(toggles.radiation, Scale1Coverage::RadiationSink, true);
    mark_missing(toggles.spin_orbit, Scale1Coverage::SpinOrbitPotential, false);
    mark_missing(toggles.magnetic_dipole, Scale1Coverage::DipolePotential, false);

    d.state_energy_complete = d.missing_mask == 0;
    d.drift_eligible = d.state_energy_complete && d.nonconservative_mask == 0;
    d.cumulative_damping_sink = cumulative_damping_sink_;
    d.cumulative_radiation_sink = cumulative_radiation_sink_;
    d.cumulative_speed_projection_sink = cumulative_speed_projection_sink_;
    d.cumulative_contact_delta = cumulative_contact_delta_;
    d.contact_event_count = contact_event_count_;
    d.speed_projection_count = speed_projection_count_;
    diagnostics_cache_ = d;
    diagnostics_revision_ = observation_revision_;
    diagnostics_toggle_mask_ = toggle_mask;
    diagnostics_cache_valid_ = true;
    return diagnostics_cache_;
}

Scale1Snapshot ParticleEngine::snapshot(const std::string& scenario,
                                        const std::string& backend,
                                        bool include_forces) {
    Scale1Snapshot out;
    const auto* scenario_spec = find_scale1_scenario_spec(scenario);
    out.core.tick = tick_;
    out.core.effective_dt = dt_;
    out.core.mode = scenario_spec ? scenario_spec->mode : Scale1Mode::EffectiveLab;
    out.core.workspace = scenario_spec ? scenario_spec->workspace
        : Scale1Workspace::ReferenceLaboratory;
    out.core.scenario_class = scenario_spec ? scenario_spec->scenario_class
        : Scale1ScenarioClass::EffectiveReference;
    out.core.dynamics_owner = Scale1DynamicsOwner::ParticleEngine;
    out.core.backend = backend;
    out.core.scenario = scenario;
    out.core.source_revision = SCALE1_REGISTRY_REVISION;
    out.core.artifact_revision = scenario_spec ? scenario_spec->canonical_source : "";
    out.core.read_only = false;

    out.objects.reserve(particles_.size());
    for (const auto& p : particles_) {
        Scale1ObjectRecord object;
        object.id = p.id;
        object.effective_state = p.charge;
        object.mass = p.mass;
        object.mass_available = true;
        object.kinetic_energy = kinetic_energy(p, toggles.relativistic_verlet);
        object.kinetic_energy_available = true;
        object.effective_radius = p.r_eff;
        object.position = p.position;
        object.velocity = p.velocity;
        object.momentum = toggles.relativistic_verlet
            ? p.momentum : p.velocity * p.mass;
        object.locked = p.locked;
        object.identity_available = false;
        object.age_ticks = tick_;
        object.fractional_center = p.position;
        object.fractional_center_available = true;
        object.provenance = p.provenance;
        out.objects.push_back(std::move(object));
    }

    const auto add_force = [&out](int object_id, const char* id, const Vec3& force) {
        const auto* spec = find_scale1_physics_spec(id);
        if (!spec) return;
        Scale1ForceRecord record;
        record.object_id = object_id;
        record.term_id = spec->id;
        record.force = force;
        record.status = spec->status;
        record.conservative = spec->conservative;
        record.accounted = spec->potential_accounted;
        out.forces.push_back(std::move(record));
    };
    if (include_forces) {
        const auto& observed_forces = observation_force_diag();
        for (int i = 0; i < static_cast<int>(particles_.size()); ++i) {
            const auto& force = observed_forces[i];
            const int id = particles_[i].id;
            if (toggles.coulomb) add_force(id, "coulomb", force.f_coulomb);
            if (toggles.gravity) add_force(id, "newton_gravity", force.f_gravity);
            if (toggles.lorentz) add_force(id, "lorentz", force.f_lorentz);
            if (toggles.exchange) add_force(id, "exchange", force.f_exchange);
            if (toggles.strong) add_force(id, "strong", force.f_strong);
            if (toggles.radiation) add_force(id, "radiation", force.f_radiation);
            if (toggles.spin_orbit) add_force(id, "spin_orbit", force.f_spin_orbit);
            if (toggles.magnetic_dipole) {
                add_force(id, "magnetic_dipole", force.f_magnetic_dipole);
            }
        }
    }

    out.events = events_;
    const ParticleDiagnostics d = diagnostics();
    out.conservation.kinetic_energy = d.total_ke;
    out.conservation.potential_energy = d.total_pe;
    out.conservation.state_energy = d.total_energy;
    out.conservation.coulomb_potential = d.coulomb_pe;
    out.conservation.gravity_potential = d.gravity_pe;
    out.conservation.total_momentum = d.total_momentum;
    out.conservation.total_angular_momentum = d.total_angular_momentum;
    out.conservation.center_of_mass = d.center_of_mass;
    out.conservation.covered_mask = d.covered_mask;
    out.conservation.missing_mask = d.missing_mask;
    out.conservation.nonconservative_mask = d.nonconservative_mask;
    out.conservation.state_energy_complete = d.state_energy_complete;
    out.conservation.drift_eligible = d.drift_eligible;
    out.conservation.cumulative_damping_sink = d.cumulative_damping_sink;
    out.conservation.cumulative_radiation_sink = d.cumulative_radiation_sink;
    out.conservation.cumulative_speed_projection_sink =
        d.cumulative_speed_projection_sink;
    out.conservation.cumulative_contact_delta = d.cumulative_contact_delta;

    out.capability_ids.push_back("effective_lab");
    for (const auto& spec : scale1_physics_registry()) {
        if (!spec.toggle_name || !*spec.toggle_name) continue;
        if (!toggles.get_toggle(spec.toggle_name) || spec.potential_accounted) continue;
        if (spec.unavailable_reason && *spec.unavailable_reason) {
            out.unavailable_reasons.push_back(
                std::string(spec.label) + ": " + spec.unavailable_reason);
        }
    }

    out.particle_count = d.particle_count;
    out.total_energy = d.total_energy;
    out.total_ke = d.total_ke;
    out.total_pe = d.total_pe;
    out.status = d.state_energy_complete
        ? "Effective Particle Lab; state-energy ledger complete for active terms"
        : "Effective Particle Lab; active terms have incomplete state-energy coverage";
    return out;
}

ParticleEngineCheckpoint ParticleEngine::checkpoint() const {
    ParticleEngineCheckpoint out;
    out.tick = tick_;
    out.next_id = next_id_;
    out.next_event_sequence = next_event_sequence_;
    out.dt = dt_;
    out.softening = soft_;
    out.toggles = toggles;
    out.particles = particles_;
    out.events = events_;
    out.insulating_box = insulating_box_;
    out.cumulative_damping_sink = cumulative_damping_sink_;
    out.cumulative_radiation_sink = cumulative_radiation_sink_;
    out.cumulative_speed_projection_sink = cumulative_speed_projection_sink_;
    out.cumulative_contact_delta = cumulative_contact_delta_;
    out.contact_event_count = contact_event_count_;
    out.speed_projection_count = speed_projection_count_;
    out.insulator_collision_count = insulator_collision_count_;
    out.insulator_port_crossing_count = insulator_port_crossing_count_;
    out.cumulative_insulator_impulse = cumulative_insulator_impulse_;
    return out;
}

bool ParticleEngine::restore_checkpoint(
        const ParticleEngineCheckpoint& saved, std::string* err) {
    auto fail = [err](const std::string& message) {
        if (err) *err = message;
        return false;
    };
    if (saved.schema_version != ParticleEngineCheckpoint::SCHEMA_VERSION) {
        return fail("unsupported ParticleEngine checkpoint schema");
    }
    if (saved.tick < 0 || saved.next_id < 0
        || !std::isfinite(saved.dt) || saved.dt <= 0.0
        || !std::isfinite(saved.softening) || saved.softening < 0.0) {
        return fail("ParticleEngine checkpoint has invalid clock or solver parameters");
    }
    if (!std::isfinite(saved.cumulative_damping_sink)
        || !std::isfinite(saved.cumulative_radiation_sink)
        || !std::isfinite(saved.cumulative_speed_projection_sink)
        || !std::isfinite(saved.cumulative_contact_delta)
        || !finite_vec3(saved.cumulative_insulator_impulse)) {
        return fail("ParticleEngine checkpoint has nonfinite ledger values");
    }
    std::string toggle_error;
    if (!saved.toggles.validate(&toggle_error)) {
        return fail("ParticleEngine checkpoint toggle profile is invalid: " + toggle_error);
    }

    ParticleEngine staged;
    staged.tick_ = saved.tick;
    staged.next_id_ = saved.next_id;
    staged.next_event_sequence_ = saved.next_event_sequence;
    staged.dt_ = saved.dt;
    staged.soft_ = saved.softening;
    staged.toggles = saved.toggles;
    staged.particles_ = saved.particles;
    staged.events_ = saved.events;
    staged.insulating_box_ = saved.insulating_box;
    staged.cumulative_damping_sink_ = saved.cumulative_damping_sink;
    staged.cumulative_radiation_sink_ = saved.cumulative_radiation_sink;
    staged.cumulative_speed_projection_sink_ = saved.cumulative_speed_projection_sink;
    staged.cumulative_contact_delta_ = saved.cumulative_contact_delta;
    staged.contact_event_count_ = saved.contact_event_count;
    staged.speed_projection_count_ = saved.speed_projection_count;
    staged.insulator_collision_count_ = saved.insulator_collision_count;
    staged.insulator_port_crossing_count_ = saved.insulator_port_crossing_count;
    staged.cumulative_insulator_impulse_ = saved.cumulative_insulator_impulse;
    staged.last_relativistic_verlet_ = saved.toggles.relativistic_verlet;
    staged.forces_.assign(staged.particles_.size(), {});
    staged.force_diag_.assign(staged.particles_.size(), {});

    int max_id = -1;
    std::unordered_set<int> ids;
    for (const auto& particle : staged.particles_) {
        if (!ids.insert(particle.id).second) {
            return fail("ParticleEngine checkpoint contains duplicate particle ids");
        }
        max_id = std::max(max_id, particle.id);
    }
    if (staged.next_id_ <= max_id) {
        return fail("ParticleEngine checkpoint next particle id is not monotonic");
    }
    std::uint64_t max_event_sequence = 0;
    bool has_events = false;
    for (const auto& event : staged.events_) {
        if (!std::isfinite(event.state_energy_delta)) {
            return fail("ParticleEngine checkpoint contains a nonfinite event ledger");
        }
        has_events = true;
        max_event_sequence = std::max(max_event_sequence, event.sequence);
    }
    if (has_events && staged.next_event_sequence_ <= max_event_sequence) {
        return fail("ParticleEngine checkpoint next event sequence is not monotonic");
    }
    std::string state_error;
    if (!staged.validate_state(&state_error)) {
        return fail("ParticleEngine checkpoint state is invalid: " + state_error);
    }

    tick_ = staged.tick_;
    next_id_ = staged.next_id_;
    next_event_sequence_ = staged.next_event_sequence_;
    dt_ = staged.dt_;
    soft_ = staged.soft_;
    toggles = staged.toggles;
    particles_ = std::move(staged.particles_);
    events_ = std::move(staged.events_);
    insulating_box_ = std::move(staged.insulating_box_);
    cumulative_damping_sink_ = staged.cumulative_damping_sink_;
    cumulative_radiation_sink_ = staged.cumulative_radiation_sink_;
    cumulative_speed_projection_sink_ = staged.cumulative_speed_projection_sink_;
    cumulative_contact_delta_ = staged.cumulative_contact_delta_;
    contact_event_count_ = staged.contact_event_count_;
    speed_projection_count_ = staged.speed_projection_count_;
    insulator_collision_count_ = staged.insulator_collision_count_;
    insulator_port_crossing_count_ = staged.insulator_port_crossing_count_;
    cumulative_insulator_impulse_ = staged.cumulative_insulator_impulse_;
    last_relativistic_verlet_ = staged.last_relativistic_verlet_;
    forces_.assign(particles_.size(), {});
    force_diag_.assign(particles_.size(), {});
    invalidate_observation();
    if (err) err->clear();
    return true;
}

}  // namespace ftd
