#pragma once
/**
 * @file volumetric_measure.h
 * @brief Explicit spatial measure for the three-dimensional unit lattice.
 *
 * Local field norms remain quadratic. Three-dimensionality enters a spatial
 * integral through the cubic cell measure V_cell = a_lat^3. The production
 * engine fixes a_lat=1; keeping the factor explicit prevents density, cell
 * energy, and point-particle energy from being conflated when NCEMC is built.
 *
 * This header does not license arbitrary non-unit lattice spacing. Such a
 * change also rescales difference operators and couplings and requires a
 * separate contract.
 */

#include "ftd/constants.h"

#if defined(__CUDACC__)
#define FTD_VOLUME_HD __host__ __device__
#else
#define FTD_VOLUME_HD
#endif

namespace ftd {

static_assert(D_SPATIAL == 3,
              "The current volumetric measure is the D=3 cubic-cell contract");

FTD_VOLUME_HD constexpr double square_face_area(double edge) {
    return edge * edge;
}

FTD_VOLUME_HD constexpr double cubic_cell_volume(double edge) {
    return edge * edge * edge;
}

inline constexpr double VOXEL_EDGE_LENGTH = 1.0;
inline constexpr double VOXEL_FACE_AREA = square_face_area(VOXEL_EDGE_LENGTH);
inline constexpr double VOXEL_VOLUME = cubic_cell_volume(VOXEL_EDGE_LENGTH);

FTD_VOLUME_HD constexpr double quadratic_field_energy_density(double magnitude_squared) {
    return 0.5 * magnitude_squared;
}

FTD_VOLUME_HD constexpr double local_field_wave_energy_density(
    double flux_magnitude_squared,
    double wave_magnitude_squared) {
    return quadratic_field_energy_density(flux_magnitude_squared)
         + quadratic_field_energy_density(wave_magnitude_squared);
}

FTD_VOLUME_HD constexpr double integrate_voxel_density(
    double density,
    double cell_volume = VOXEL_VOLUME) {
    return density * cell_volume;
}

static_assert(VOXEL_FACE_AREA == 1.0, "Unit lattice face area must remain one");
static_assert(VOXEL_VOLUME == 1.0, "Unit lattice cell volume must remain one");

}  // namespace ftd

#undef FTD_VOLUME_HD
