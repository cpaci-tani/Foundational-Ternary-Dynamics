#pragma once

#include <cstdint>
#include <vector>

#include "ftd/voxel.h"

namespace ftd {

class RenderBridge;

// FTD-0406: selected local representation of the direct colour-pair energy.
// `energy_density` is T00 in lattice energy-density units.  The six stress
// components are the symmetric Irving-Kirkwood central-force allocation.
struct StrongStressCell {
    double energy_density = 0.0;
    double stress_xx = 0.0;
    double stress_yy = 0.0;
    double stress_zz = 0.0;
    double stress_xy = 0.0;
    double stress_xz = 0.0;
    double stress_yz = 0.0;
};

struct StrongEnergyStepDiagnostics {
    double h_before = 0.0;
    double h_after = 0.0;
    double residual = 0.0;
    double lambda = 1.0;
    Vec3 momentum_before;
    Vec3 momentum_after;
    int projection_events = 0;
    int projection_failures = 0;
    int topology_failures = 0;
    int projected_particles = 0;
};

// Existing three-regime radial profile without the colour factor.
double strong_radial_profile(double r);

// Owner-selected vacuum convention: U_pair(1)=0 and no pair means zero.
double strong_pair_potential(double r, int8_t color_a, int8_t color_b);

// Sum over unordered coloured pairs using continuous lattice-coordinate plus
// signed remainder positions and the force path's periodic minimum image.
double compute_strong_potential_energy(const RenderBridge& rb);

// Recompute the selected straight-string CIC T00/stress allocation.  The
// output is resized to lattice.total_sites() and overwritten deterministically.
void compute_strong_stress_cells(const RenderBridge& rb,
                                 std::vector<StrongStressCell>& out);

// Tick hooks for the collision-free CPU projection contract.
void begin_strong_energy_step(RenderBridge& rb);
void complete_strong_energy_step(RenderBridge& rb);

}  // namespace ftd
