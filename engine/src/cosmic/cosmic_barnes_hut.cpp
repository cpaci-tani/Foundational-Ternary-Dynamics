/**
 * CosmicEngine Barnes-Hut octree + gravity.
 *
 * Extracted from cosmic_engine.cpp (CE2). Contains:
 *   - build_octree()        : delegates to BarnesHutTree::build with lambdas
 *   - tree_force()          : recursive O(log N) force traversal
 *   - compute_gravity()     : top-level per-body dispatch, writes forces_ + diag
 */

#include "ftd/cosmic_engine.h"
#include <cmath>

namespace ftd {

// ============================================================================
// Barnes-Hut Octree
// ============================================================================
void CosmicEngine::build_octree() {
    octree_.build(bodies_,
        [](const CosmicBody& b) { return b.position; },
        [](const CosmicBody& b) { return b.mass; },
        [](const CosmicBody& b) { return 0.0; }
    );
}

Vec3 CosmicEngine::tree_force(int body_idx, int node_idx) const {
    const BarnesHutNode& node = octree_.nodes[node_idx];
    const CosmicBody& body = bodies_[body_idx];

    if (node.total_mass <= 0.0) return {};

    Vec3 dr = {
        node.center_of_mass.x - body.position.x,
        node.center_of_mass.y - body.position.y,
        node.center_of_mass.z - body.position.z
    };
    double r2 = dr.mag2() + softening_ * softening_;
    double r = std::sqrt(r2);

    if (node.is_leaf) {
        if (node.body_indices.empty()) return {};
        Vec3 lf;
        for (int b_idx : node.body_indices) {
            if (b_idx == body_idx) continue;

            Vec3 l_dr = {
                bodies_[b_idx].position.x - body.position.x,
                bodies_[b_idx].position.y - body.position.y,
                bodies_[b_idx].position.z - body.position.z
            };
            double lr2 = l_dr.mag2() + softening_ * softening_;
            double lr = std::sqrt(lr2);
            double f_mag = G_N * body.mass * bodies_[b_idx].mass / lr2;
            lf.x += f_mag * l_dr.x / lr;
            lf.y += f_mag * l_dr.y / lr;
            lf.z += f_mag * l_dr.z / lr;
        }
        return lf;
    }

    // Barnes-Hut opening angle test
    double s = node.width();
    if (s / r < cosmic::THETA_BH) {
        // Treat as single mass
        double f_mag = G_N * body.mass * node.total_mass / r2;
        return {f_mag * dr.x / r, f_mag * dr.y / r, f_mag * dr.z / r};
    }

    // Recurse into children
    Vec3 force = {};
    for (int c = 0; c < 8; ++c) {
        if (node.children[c] >= 0) {
            Vec3 cf = tree_force(body_idx, node.children[c]);
            force.x += cf.x;
            force.y += cf.y;
            force.z += cf.z;
        }
    }
    return force;
}

// ============================================================================
// Gravity computation
// ============================================================================

void CosmicEngine::compute_gravity() {
    if (!toggles.gravity || octree_.nodes.empty()) return;

    for (int i = 0; i < (int)bodies_.size(); ++i) {
        Vec3 fg = tree_force(i, octree_.root);
        // Acceleration = force / mass (but force already includes body mass)
        // Actually tree_force returns F = G*m_i*M*rhat/r^2, so a = F/m_i
        double m = bodies_[i].mass;
        if (m > 0.0) {
            forces_[i].x += fg.x / m;
            forces_[i].y += fg.y / m;
            forces_[i].z += fg.z / m;
            force_diag_[i].f_gravity = {fg.x / m, fg.y / m, fg.z / m};
        }
    }
}

}  // namespace ftd
