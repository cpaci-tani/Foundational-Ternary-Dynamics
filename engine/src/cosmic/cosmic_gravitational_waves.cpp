/**
 * CosmicEngine gravitational wave emission + propagation.
 *
 * Extracted from cosmic_engine.cpp (CE5). Detects BH/NS/quasar mergers
 * (bodies within a few Schwarzschild radii), emits a GravWaveEvent with
 * quadrupole-scaled strain, merges the bodies (momentum-conserving), and
 * advances existing events outward at c = 1/sqrt(3).
 */

#include "ftd/cosmic_engine.h"
#include <algorithm>
#include <cmath>

namespace ftd {

// ============================================================================
// Gravitational waves
// ============================================================================

void CosmicEngine::detect_gw_events() {
    if (!toggles.gravitational_waves) return;

    // Check for BH/NS mergers (bodies within Schwarzschild radius)
    for (int i = 0; i < (int)bodies_.size(); ++i) {
        if (!is_compact_object(bodies_[i].type)) continue;
        for (int j = i + 1; j < (int)bodies_.size(); ++j) {
            if (!is_compact_object(bodies_[j].type)) continue;

            Vec3 dr = {
                bodies_[i].position.x - bodies_[j].position.x,
                bodies_[i].position.y - bodies_[j].position.y,
                bodies_[i].position.z - bodies_[j].position.z
            };
            double r = dr.mag();
            double r_merge = bodies_[i].schwarzschild_radius() +
                            bodies_[j].schwarzschild_radius();

            if (r < r_merge * 3.0) {
                // Merger! Create GW event
                GravWaveEvent gw;
                gw.origin = {
                    0.5 * (bodies_[i].position.x + bodies_[j].position.x),
                    0.5 * (bodies_[i].position.y + bodies_[j].position.y),
                    0.5 * (bodies_[i].position.z + bodies_[j].position.z)
                };
                gw.emission_tick = tick_;
                gw.total_mass = bodies_[i].mass + bodies_[j].mass;
                // GW strain: h ~ 4*G*M*v^2 / (r*c^4) — distance factor is critical
                double v2 = (bodies_[i].velocity.mag2() + bodies_[j].velocity.mag2()) * 0.5;
                double r_source = std::max(dr.mag(), softening_);
                gw.strain = 4.0 * G_N * gw.total_mass * v2 / (r_source * C_SPEED * C_SPEED * C_SPEED * C_SPEED);
                gw.current_radius = 0.0;
                gw_events_.push_back(gw);

                // Merge bodies: j absorbed into i, ~5% mass radiated as GWs
                double m_total = bodies_[i].mass + bodies_[j].mass;
                double gw_mass_loss = 0.05; // ~5% of rest mass radiated (equal-mass limit)
                // Momentum-conserving velocity merge (before mass loss)
                bodies_[i].velocity.x = (bodies_[i].velocity.x * bodies_[i].mass +
                                         bodies_[j].velocity.x * bodies_[j].mass) / m_total;
                bodies_[i].velocity.y = (bodies_[i].velocity.y * bodies_[i].mass +
                                         bodies_[j].velocity.y * bodies_[j].mass) / m_total;
                bodies_[i].velocity.z = (bodies_[i].velocity.z * bodies_[i].mass +
                                         bodies_[j].velocity.z * bodies_[j].mass) / m_total;
                bodies_[i].mass = m_total * (1.0 - gw_mass_loss);
                // Mark j for removal
                bodies_[j].mass = 0.0;
                bodies_[j].type = CosmicBodyType::DARK_ENERGY; // Will be cleaned
            }
        }
    }

    // Clean up zero-mass bodies
    bodies_.erase(
        std::remove_if(bodies_.begin(), bodies_.end(),
                       [](const CosmicBody& b) { return b.mass <= 0.0; }),
        bodies_.end());
}

void CosmicEngine::propagate_gw() {
    if (!toggles.gravitational_waves) return;

    for (auto& gw : gw_events_) {
        // GW propagates at c = 1/sqrt(3)
        gw.current_radius += C_SPEED * dt_;
        // Strain falls as 1/r
        // (strain at source is stored; observers compute h(r) = h_source * r_source / r)
    }

    // Remove old events that have propagated beyond the box
    gw_events_.erase(
        std::remove_if(gw_events_.begin(), gw_events_.end(),
                       [this](const GravWaveEvent& gw) {
                           return gw.current_radius > box_size_ * 2.0;
                       }),
        gw_events_.end());
}

}  // namespace ftd
