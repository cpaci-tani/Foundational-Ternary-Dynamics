/**
 * FTD-0406 owner-authorized strong stress-energy contract.
 *
 * This is a selected numerical architecture, not a substrate derivation.
 * It is default-off and scoped to the collision-free CPU colour sector.
 */

#include "ftd/strong_stress_energy.h"

#include "ftd/causal_kinematics.h"
#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ftd {
namespace {

struct StrongParticle {
    int idx = -1;
    int particle_id = -1;
    int8_t color = 0;
    Vec3 position;
    Vec3 momentum;
};

double color_factor(int8_t a, int8_t b) {
    return a == b ? 0.5 : -1.0;
}

double minimum_image_component(double d, double L) {
    if (d > L * 0.5) d -= L;
    if (d < -L * 0.5) d += L;
    return d;
}

Vec3 minimum_image_displacement(const Vec3& a, const Vec3& b, double L) {
    return {
        minimum_image_component(b.x - a.x, L),
        minimum_image_component(b.y - a.y, L),
        minimum_image_component(b.z - a.z, L)
    };
}

Vec3 effective_position(const RenderBridge& rb, int idx) {
    const auto c = rb.lattice().coord(idx);
    const auto& v = rb.voxels()[idx];
    return {static_cast<double>(c.x) + v.remainder.x,
            static_cast<double>(c.y) + v.remainder.y,
            static_cast<double>(c.z) + v.remainder.z};
}

Vec3 physical_momentum(const Voxel& v) {
    return v.velocity * (flat_gamma(v.velocity.mag2()) * M_INERTIAL);
}

double kinetic_from_momentum(const Vec3& p) {
    const double c2 = C_SPEED * C_SPEED;
    return std::sqrt(E_REST * E_REST + c2 * p.mag2()) - E_REST;
}

Vec3 velocity_from_momentum(const Vec3& p) {
    const double c2 = C_SPEED * C_SPEED;
    const double energy = std::sqrt(E_REST * E_REST + c2 * p.mag2());
    if (!(energy > 0.0) || !std::isfinite(energy)) return {};
    return p * (c2 / energy);
}

std::vector<StrongParticle> gather_particles(const RenderBridge& rb) {
    std::vector<StrongParticle> particles;
    particles.reserve(rb.ordered_active_indices().size());
    for (int idx : rb.ordered_active_indices()) {
        const auto& v = rb.voxels()[idx];
        if (v.state == 0 || v.color == 0) continue;
        particles.push_back({idx, v.particle_id, v.color,
                             effective_position(rb, idx), physical_momentum(v)});
    }
    return particles;
}

double kinetic_sum(const std::vector<StrongParticle>& particles) {
    double out = 0.0;
    for (const auto& p : particles) out += kinetic_from_momentum(p.momentum);
    return out;
}

Vec3 momentum_sum(const std::vector<StrongParticle>& particles) {
    Vec3 out;
    for (const auto& p : particles) out += p.momentum;
    return out;
}

double potential_sum(const std::vector<StrongParticle>& particles, double L) {
    double out = 0.0;
    for (std::size_t i = 0; i < particles.size(); ++i) {
        for (std::size_t j = i + 1; j < particles.size(); ++j) {
            const Vec3 d = minimum_image_displacement(
                particles[i].position, particles[j].position, L);
            const double r = std::max(1.0, d.mag());
            out += strong_pair_potential(r, particles[i].color, particles[j].color);
        }
    }
    return out;
}

// Fixed 16-point Gauss-Legendre quadrature.  Regime boundaries are split by
// the caller, so this never integrates across a piecewise-force jump.
double integrate_fixed(double a, double b) {
    if (!(b > a)) return 0.0;
    static constexpr std::array<double, 8> nodes = {
        0.095012509837637440185319335424958,
        0.281603550779258913230460501460496,
        0.458016777657227386342419442983577,
        0.617876244402643748446671764048791,
        0.755404408355003033895101194847442,
        0.865631202387831743880467897712393,
        0.944575023073232576077988415534608,
        0.989400934991649932596154173450333
    };
    static constexpr std::array<double, 8> weights = {
        0.189450610455068496285396723208283,
        0.182603415044923588866763667969220,
        0.169156519395002538189312079030359,
        0.149595988816576732081501730547479,
        0.124628971255533872052476282192017,
        0.095158511682492784809925107602246,
        0.062253523938647892862843836994378,
        0.027152459411754094851780572456018
    };
    const double mid = 0.5 * (a + b);
    const double half = 0.5 * (b - a);
    double sum = 0.0;
    for (std::size_t k = 0; k < nodes.size(); ++k) {
        const double dx = half * nodes[k];
        sum += weights[k] * (strong_radial_profile(mid - dx)
                           + strong_radial_profile(mid + dx));
    }
    return half * sum;
}

double integral_from_one(double r) {
    if (!(r > 1.0)) return 0.0;

    const double r3 = std::min(r, COLOR_COULOMB_RADIUS);
    double value = integrate_fixed(1.0, r3);
    if (r <= COLOR_COULOMB_RADIUS) return value;

    const double r8 = std::min(r, COLOR_TRANSITION_RADIUS);
    value += integrate_fixed(COLOR_COULOMB_RADIUS, r8);
    if (r <= COLOR_TRANSITION_RADIUS) return value;

    // alpha_s_lattice is capped at one throughout the frozen harmonic arm.
    value += (r * r - COLOR_TRANSITION_RADIUS * COLOR_TRANSITION_RADIUS)
           / (2.0 * COLOR_LINEAR_DENOM);
    return value;
}

double wrap_real(double x, double L) {
    x = std::fmod(x, L);
    if (x < 0.0) x += L;
    if (x >= L) x -= L;
    return x;
}

void deposit_sample(const RenderBridge& rb,
                    const Vec3& position,
                    double energy,
                    const std::array<double, 6>& stress,
                    std::vector<StrongStressCell>& out) {
    const int L = rb.lattice().size();
    const double px = wrap_real(position.x, static_cast<double>(L));
    const double py = wrap_real(position.y, static_cast<double>(L));
    const double pz = wrap_real(position.z, static_cast<double>(L));
    const int x0 = static_cast<int>(std::floor(px));
    const int y0 = static_cast<int>(std::floor(py));
    const int z0 = static_cast<int>(std::floor(pz));
    const double fx = px - x0;
    const double fy = py - y0;
    const double fz = pz - z0;

    struct WeightedIndex { int idx; double weight; };
    std::array<WeightedIndex, 8> cells{};
    int cursor = 0;
    double weight_sum = 0.0;
    for (int ox = 0; ox <= 1; ++ox) {
        const double wx = ox ? fx : 1.0 - fx;
        for (int oy = 0; oy <= 1; ++oy) {
            const double wy = oy ? fy : 1.0 - fy;
            for (int oz = 0; oz <= 1; ++oz) {
                const double wz = oz ? fz : 1.0 - fz;
                const double w = wx * wy * wz;
                cells[cursor++] = {rb.lattice().index(x0 + ox, y0 + oy, z0 + oz), w};
                weight_sum += w;
            }
        }
    }
    if (!(weight_sum > 0.0)) return;

    for (const auto& cell : cells) {
        const double w = cell.weight / weight_sum;
        auto& dst = out[cell.idx];
        dst.energy_density += energy * w;
        dst.stress_xx += stress[0] * w;
        dst.stress_yy += stress[1] * w;
        dst.stress_zz += stress[2] * w;
        dst.stress_xy += stress[3] * w;
        dst.stress_xz += stress[4] * w;
        dst.stress_yz += stress[5] * w;
    }
}

bool same_topology(const std::vector<int>& before,
                   const std::vector<StrongParticle>& after) {
    std::vector<int> ids;
    ids.reserve(after.size());
    for (const auto& p : after) ids.push_back(p.particle_id);
    std::sort(ids.begin(), ids.end());
    if (std::adjacent_find(ids.begin(), ids.end()) != ids.end()) return false;
    return ids == before;
}

double kinetic_at_lambda(const std::vector<StrongParticle>& particles,
                         const Vec3& mean_p,
                         double lambda) {
    double out = 0.0;
    for (const auto& particle : particles) {
        const Vec3 p = mean_p + (particle.momentum - mean_p) * lambda;
        out += kinetic_from_momentum(p);
    }
    return out;
}

bool projection_configuration_valid(const TermToggles& t) {
    return t.strong_stress_energy && t.color_forces && t.forces && t.movement
        && !t.damping && !t.genesis && !t.evaporation && !t.pair_production
        && !t.poisson_coulomb && !t.emergent_forces && !t.gravity
        && !t.latency_field && !t.lorentz_force && !t.strong_force
        && !t.exchange_force && !t.weak_transmutation && !t.triad_binding
        && !t.absorbing_boundary && !t.reflective_boundary;
}

void surface_failure(StrongEnergyStepDiagnostics& d,
                     bool& active,
                     const TermToggles& toggles,
                     bool topology) {
    if (topology) ++d.topology_failures;
    else ++d.projection_failures;
    active = false;
    if (toggles.strict_validation) {
        throw std::runtime_error(topology
            ? "strong_stress_energy topology changed during projected tick"
            : "strong_stress_energy could not reach the frozen energy surface");
    }
}

}  // namespace

double strong_radial_profile(double r) {
    r = std::max(1.0, r);
    const double as = alpha_s_lattice(r);
    if (r < COLOR_COULOMB_RADIUS) return as / (r * r);
    if (r < COLOR_TRANSITION_RADIUS) return as / (COLOR_TRANSITION_DENOM * r);
    return as * r / COLOR_LINEAR_DENOM;
}

double strong_pair_potential(double r, int8_t color_a, int8_t color_b) {
    if (color_a == 0 || color_b == 0) return 0.0;
    r = std::max(1.0, r);
    return -color_factor(color_a, color_b) * integral_from_one(r);
}

double compute_strong_potential_energy(const RenderBridge& rb) {
    return potential_sum(gather_particles(rb), static_cast<double>(rb.lattice_.size()));
}

void compute_strong_stress_cells(const RenderBridge& rb,
                                 std::vector<StrongStressCell>& out) {
    const int N = static_cast<int>(rb.lattice_.total_sites());
    out.assign(N, {});
    if (!rb.toggles.strong_stress_energy) return;

    const auto particles = gather_particles(rb);
    const double L = static_cast<double>(rb.lattice_.size());
    for (std::size_t i = 0; i < particles.size(); ++i) {
        for (std::size_t j = i + 1; j < particles.size(); ++j) {
            const Vec3 d = minimum_image_displacement(
                particles[i].position, particles[j].position, L);
            const double raw_r = d.mag();
            const double r = std::max(1.0, raw_r);
            const double cf = color_factor(particles[i].color, particles[j].color);
            const double U = strong_pair_potential(
                r, particles[i].color, particles[j].color);
            const Vec3 force = r > 0.0 ? d * (-cf * strong_radial_profile(r) / r) : Vec3{};
            const std::array<double, 6> pair_stress = {
                -d.x * force.x, -d.y * force.y, -d.z * force.z,
                -d.x * force.y, -d.x * force.z, -d.y * force.z
            };

            const int samples = std::max(1, static_cast<int>(std::ceil(r)));
            for (int s = 0; s < samples; ++s) {
                const double t = (static_cast<double>(s) + 0.5) / samples;
                const Vec3 point = particles[i].position + d * t;
                std::array<double, 6> stress{};
                for (int k = 0; k < 6; ++k) stress[k] = pair_stress[k] / samples;
                deposit_sample(rb, point, U / samples, stress, out);
            }
        }
    }
}

void begin_strong_energy_step(RenderBridge& rb) {
    rb.strong_energy_step_diag_ = {};
    rb.strong_step_particle_ids_.clear();
    rb.strong_step_active_ = false;
    if (!rb.toggles.strong_stress_energy || !rb.toggles.movement) return;
    if (!projection_configuration_valid(rb.toggles)) {
        surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                        rb.toggles, false);
        return;
    }

    const auto particles = gather_particles(rb);
    if (particles.size() < 2) return;
    for (const auto& p : particles) rb.strong_step_particle_ids_.push_back(p.particle_id);
    std::sort(rb.strong_step_particle_ids_.begin(), rb.strong_step_particle_ids_.end());
    if (std::adjacent_find(rb.strong_step_particle_ids_.begin(),
                           rb.strong_step_particle_ids_.end())
        != rb.strong_step_particle_ids_.end()) {
        surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                        rb.toggles, true);
        return;
    }

    const double K = kinetic_sum(particles);
    const double U = potential_sum(particles, static_cast<double>(rb.lattice_.size()));
    rb.strong_step_h_before_ = K + U;
    rb.strong_step_momentum_before_ = momentum_sum(particles);
    rb.strong_energy_step_diag_.h_before = rb.strong_step_h_before_;
    rb.strong_energy_step_diag_.momentum_before = rb.strong_step_momentum_before_;
    rb.strong_step_active_ = std::isfinite(rb.strong_step_h_before_);
    if (!rb.strong_step_active_)
        surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                        rb.toggles, false);
}

void complete_strong_energy_step(RenderBridge& rb) {
    if (!rb.strong_step_active_) return;
    auto particles = gather_particles(rb);
    if (!same_topology(rb.strong_step_particle_ids_, particles)) {
        surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                        rb.toggles, true);
        return;
    }

    const double U_after = potential_sum(particles, static_cast<double>(rb.lattice_.size()));
    const double target_K = rb.strong_step_h_before_ - U_after;
    const Vec3 total_p = momentum_sum(particles);
    const Vec3 mean_p = total_p * (1.0 / static_cast<double>(particles.size()));
    const double min_K = kinetic_at_lambda(particles, mean_p, 0.0);
    const double tolerance = 1e-13 * std::max(1.0, std::abs(target_K));
    if (!std::isfinite(target_K) || target_K < min_K - tolerance) {
        surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                        rb.toggles, false);
        return;
    }

    double lambda = 0.0;
    if (target_K > min_K + tolerance) {
        double lo = 0.0;
        double hi = 1.0;
        int expansions = 0;
        while (kinetic_at_lambda(particles, mean_p, hi) < target_K
               && expansions < 64) {
            hi *= 2.0;
            ++expansions;
        }
        if (!std::isfinite(hi)
            || kinetic_at_lambda(particles, mean_p, hi) < target_K) {
            surface_failure(rb.strong_energy_step_diag_, rb.strong_step_active_,
                            rb.toggles, false);
            return;
        }
        for (int iter = 0; iter < 96; ++iter) {
            const double mid = 0.5 * (lo + hi);
            if (kinetic_at_lambda(particles, mean_p, mid) < target_K) lo = mid;
            else hi = mid;
        }
        lambda = 0.5 * (lo + hi);
    }

    for (auto& particle : particles) {
        particle.momentum = mean_p + (particle.momentum - mean_p) * lambda;
        rb.voxels_[particle.idx].velocity = velocity_from_momentum(particle.momentum);
    }

    const auto projected = gather_particles(rb);
    const double H_after = kinetic_sum(projected)
                         + potential_sum(projected, static_cast<double>(rb.lattice_.size()));
    auto& d = rb.strong_energy_step_diag_;
    d.h_after = H_after;
    d.residual = H_after - rb.strong_step_h_before_;
    d.lambda = lambda;
    d.momentum_after = momentum_sum(projected);
    d.projected_particles = static_cast<int>(projected.size());
    ++d.projection_events;
    if (!std::isfinite(d.residual) || std::abs(d.residual) > 1e-12) {
        ++d.projection_failures;
        if (rb.toggles.strict_validation)
            throw std::runtime_error("strong_stress_energy projection residual exceeds 1e-12");
    }
    rb.strong_step_active_ = false;
}

}  // namespace ftd
