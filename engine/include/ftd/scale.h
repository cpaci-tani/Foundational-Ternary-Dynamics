#pragma once
/**
 * Multi-Scale Physics: OnticEntity and Scale definitions
 *
 * Phase 7: The universal ternary triple {state, energy, boundary}
 * recurs at every scale of reality.
 *
 * | Component | Voxel (Scale 0) | Particle (Scale 1) | Atom (Scale 2) |
 * |-----------|-----------------|-------------------|-----------------|
 * | State     | s in {-1,0,+1}  | charge +/-1       | Z (atomic num)  |
 * | Energy    | |J| (flux)      | mass = K_B        | binding energy  |
 * | Boundary  | 1 voxel         | r_eff = 2.48      | orbital radius  |
 *
 * "Not many worlds, many scales."
 */

#include <cstdint>
#include <vector>

namespace ftd {

// Forward declarations for bridge functions
class RenderBridge;
struct Particle;
class ParticleEngine;
struct Atom;
class AtomEngine;

// Scale levels in the ontic hierarchy
enum class ScaleLevel : int {
    VOXEL    = 0,   // Planck-scale lattice dynamics
    PARTICLE = 1,   // Effective point particles with analytical forces
    ATOM     = 2,   // Bound states (hydrogen, helium, ...)
    MOLECULE = 3,   // Chemical bonds
    BULK     = 4    // Thermodynamic limit
};

// The universal ternary triple: {state, energy, boundary}
// Every entity at every scale is fully characterized by these three numbers.
struct OnticEntity {
    int state = 0;           // What it IS (charge, atomic number, ...)
    double energy = 0.0;     // What it CAN DO (mass, binding energy, ...)
    double boundary = 0.0;   // Where it ENDS (r_eff, orbital radius, ...)
};

// ============================================================================
// Scale Bridge: coarsen/refine transitions between Scale 0 and Scale 1
// ============================================================================

// Scale 0 → Scale 1: extract particle descriptions from voxel simulation
std::vector<Particle> coarsen_to_particles(const RenderBridge& rb);

// Scale 1 → Scale 0: reconstruct voxel state from a particle
void refine_to_voxels(const Particle& p, RenderBridge& rb);

// ============================================================================
// Scale Bridge: coarsen/refine transitions between Scale 1 and Scale 2
// ============================================================================

// Scale 1 → Scale 2: group particles into atoms by proximity
// Locked proton clusters + nearby electrons → Atom with Z from proton count
std::vector<Atom> coarsen_to_atoms(const ParticleEngine& pe);

// Scale 2 → Scale 1: decompose atom into constituent particles
// Returns Z locked protons at center + (Z - charge) electrons at radius
std::vector<Particle> refine_to_particles(const Atom& a);

}  // namespace ftd
