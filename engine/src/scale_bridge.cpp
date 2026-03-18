/**
 * Scale Bridge: coarsen/refine between Scale 0 (voxels) and Scale 1 (particles)
 *
 * Phase 7 Stage 3.
 *
 * coarsen_to_particles: Scan voxels for state != 0, extract {charge, mass, r_eff,
 *     position (coord + remainder), velocity, spin, color, pair_id} → Particle.
 *
 * refine_to_voxels: Call inject_wavepacket() at particle position, then set
 *     velocity and remainder from the continuous Particle position.
 */

#include "ftd/scale.h"
#include "ftd/particle_engine.h"
#include "ftd/atom_engine.h"
#include "ftd/render_bridge.h"
#include <cmath>
#include <algorithm>

namespace ftd {

std::vector<Particle> coarsen_to_particles(const RenderBridge& rb) {
    std::vector<Particle> result;
    const auto& voxels = rb.voxels();
    const auto& lat = rb.lattice();
    int N = lat.size();

    for (int idx = 0; idx < lat.total_sites(); ++idx) {
        const Voxel& v = voxels[idx];
        if (v.state == 0) continue;

        Particle p;
        p.id = v.particle_id;
        p.charge = v.state;
        p.mass = K_B;
        p.r_eff = R_EFF_DEFAULT;

        // Position: integer coord + sub-lattice remainder
        Coord c = lat.coord(idx);
        p.position.x = c.x + v.remainder.x;
        p.position.y = c.y + v.remainder.y;
        p.position.z = c.z + v.remainder.z;

        // Velocity: direct transfer
        p.velocity = v.velocity;

        // Quantum numbers
        p.spin = v.spin;
        p.color = v.color;
        p.pair_id = v.pair_id;
        p.locked = v.locked;

        result.push_back(p);
    }

    return result;
}

void refine_to_voxels(const Particle& p, RenderBridge& rb) {
    const auto& lat = rb.lattice();
    int N = lat.size();

    // Integer lattice position (wrapped to [0, N))
    int ix = static_cast<int>(std::floor(p.position.x));
    int iy = static_cast<int>(std::floor(p.position.y));
    int iz = static_cast<int>(std::floor(p.position.z));
    ix = ((ix % N) + N) % N;
    iy = ((iy % N) + N) % N;
    iz = ((iz % N) + N) % N;

    // Inject wavepacket at lattice site (Phase 6 method)
    rb.inject_wavepacket(ix, iy, iz, p.charge, 3.0, K_B);

    // Restore sub-lattice remainder and velocity
    int idx = lat.index(ix, iy, iz);
    Voxel& v = rb.voxels()[idx];
    v.remainder.x = p.position.x - std::floor(p.position.x);
    v.remainder.y = p.position.y - std::floor(p.position.y);
    v.remainder.z = p.position.z - std::floor(p.position.z);
    v.velocity = p.velocity;
    v.spin = p.spin;
    v.color = p.color;
    v.pair_id = p.pair_id;
    v.locked = p.locked;
}

// ============================================================================
// Scale 1 → Scale 2: coarsen particles to atoms
// ============================================================================

std::vector<Atom> coarsen_to_atoms(const ParticleEngine& pe) {
    std::vector<Atom> result;
    const auto& particles = pe.particles();

    // Simple clustering: each locked positive particle = proton center.
    // Count protons within a cluster radius, nearby electrons reduce charge.
    // For now, each locked particle with charge +1 becomes an atom of Z=1.
    // Groups of locked +1 particles within 3 units form a nucleus with Z = count.

    std::vector<bool> used(particles.size(), false);
    constexpr double CLUSTER_RADIUS = 5.0;

    // First pass: identify proton clusters (locked, charge +1)
    for (int i = 0; i < static_cast<int>(particles.size()); ++i) {
        if (used[i] || !particles[i].locked || particles[i].charge != 1) continue;

        // Start a nucleus cluster with this proton
        std::vector<int> proton_indices = {i};
        used[i] = true;

        // Find nearby locked +1 particles
        for (int j = i + 1; j < static_cast<int>(particles.size()); ++j) {
            if (used[j] || !particles[j].locked || particles[j].charge != 1) continue;
            Vec3 dr = particles[j].position - particles[i].position;
            if (dr.mag() < CLUSTER_RADIUS) {
                proton_indices.push_back(j);
                used[j] = true;
            }
        }

        int Z = static_cast<int>(proton_indices.size());

        // Compute centroid
        Vec3 centroid;
        for (int pi : proton_indices) {
            centroid += particles[pi].position;
        }
        centroid *= (1.0 / Z);

        // Count nearby electrons (charge -1, not locked)
        int electron_count = 0;
        for (int j = 0; j < static_cast<int>(particles.size()); ++j) {
            if (used[j] || particles[j].charge != -1) continue;
            Vec3 dr = particles[j].position - centroid;
            if (dr.mag() < CLUSTER_RADIUS * 3.0) {
                used[j] = true;
                electron_count++;
                if (electron_count >= Z) break;  // neutral atom
            }
        }

        // Build atom
        AtomicProperties props = compute_atomic_properties(Z, Z);  // N ≈ Z
        Atom a;
        a.id = static_cast<int>(result.size());
        a.Z = Z;
        a.N = Z;
        a.charge = Z - electron_count;
        a.mass = props.mass;
        a.radius = props.radius;
        a.vdw_epsilon = props.vdw_epsilon;
        a.vdw_sigma = props.vdw_sigma;
        a.max_bonds = props.max_bonds;
        a.valence_electrons = props.max_bonds;
        a.position = centroid;
        a.locked = true;  // Preserve locked state from protons

        result.push_back(a);
    }

    return result;
}

// ============================================================================
// Scale 2 → Scale 1: refine atom to particles
// ============================================================================

std::vector<Particle> refine_to_particles(const Atom& a) {
    std::vector<Particle> result;

    // Z locked protons at center
    for (int i = 0; i < a.Z; ++i) {
        Particle p;
        p.id = i;
        p.charge = +1;
        p.mass = M_PROTON;
        p.r_eff = R_EFF_DEFAULT;
        p.position = a.position;
        p.locked = true;
        result.push_back(p);
    }

    // (Z - charge) electrons orbiting at atom radius
    int n_electrons = a.Z - a.charge;
    for (int i = 0; i < n_electrons; ++i) {
        double angle = 2.0 * PI * i / n_electrons;
        Particle p;
        p.id = a.Z + i;
        p.charge = -1;
        p.mass = K_B;
        p.r_eff = R_EFF_DEFAULT;
        p.position = a.position;
        p.position.x += a.radius * std::cos(angle);
        p.position.y += a.radius * std::sin(angle);
        p.locked = false;
        result.push_back(p);
    }

    return result;
}

}  // namespace ftd
