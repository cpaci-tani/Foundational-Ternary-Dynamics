#pragma once
/**
 * Multi-Scale Physics: effective entity projections and scale definitions
 *
 * Phase 7: The compact display projection {state, energy, boundary} recurs at
 * each effective scale. It is a presentation interface, not a scale-navigation
 * handoff and not a claim that three scalars are the state-complete primitive
 * record of v3 FTD.
 *
 * | Component | Voxel (Scale 0) | Particle (Scale 1) | Atom (Scale 2) |
 * |-----------|-----------------|-------------------|-----------------|
 * | State     | s in {-1,0,+1}  | effective signed q | Z (atomic num)  |
 * | Energy    | |J| (flux)      | mass = K_B        | binding energy  |
 * | Boundary  | 1 voxel         | r_eff = 2.48      | orbital radius  |
 *
 * "Not many worlds, many scales."
 */

#include <cstdint>
namespace ftd {

// Scale levels in the ontic hierarchy
enum class ScaleLevel : int {
    VOXEL    = 0,   // Planck-scale lattice dynamics
    PARTICLE = 1,   // Effective point particles with analytical forces
    ATOM     = 2,   // Bound states (hydrogen, helium, ...)
    MOLECULE = 3,   // Chemical bonds
    BULK     = 4,   // Thermodynamic limit
    COSMIC   = 5    // N-body + SPH cosmic simulation
};

// Scale-local presentation summary: {state, energy, boundary}.
// The complete primitive record has additional finite fields and history; this
// compact triple exists for presentation interoperability.
struct OnticEntity {
    int state = 0;           // What it IS (charge, atomic number, ...)
    double energy = 0.0;     // What it CAN DO (mass, binding energy, ...)
    double boundary = 0.0;   // Where it ENDS (r_eff, orbital radius, ...)
};

}  // namespace ftd
