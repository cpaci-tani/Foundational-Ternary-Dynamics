#pragma once
/**
 * CSV Data Export Utility for FTD Simulations
 *
 * Utility for exporting simulation data to CSV files.
 * Designed for post-processing in Python/Julia/R with standard
 * data analysis tooling (pandas, matplotlib, etc.).
 *
 * Three core export functions:
 *   1. export_flux_field()      — Full 3D field dump (x,y,z,Jx,Jy,Jz,|J|,state)
 *   2. export_density_slice()   — 2D slice of |J| for visualization
 *   3. export_diagnostics_row() — Append per-tick stats to a timeseries file
 *
 * Usage:
 *   #include "ftd/csv_export.h"
 *   ftd::csv::export_flux_field(bridge, "output/flux_t100.csv");
 *   ftd::csv::export_density_slice(bridge, "output/slice_xy.csv", 'z', L/2);
 *   ftd::csv::export_diagnostics_row(bridge, "output/timeseries.csv");
 *
 * Implementation lives in src/csv_export.cpp.
 */

#include <string>
#include "render_bridge.h"

namespace ftd {
namespace csv {

/**
 * Export the full 3D flux field to CSV.
 *
 * Columns: x, y, z, Jx, Jy, Jz, density, state, latency, tau
 *
 * @param bridge    The RenderBridge containing the simulation state
 * @param filename  Output CSV file path
 * @return          true if export succeeded
 */
bool export_flux_field(const RenderBridge& bridge, const std::string& filename);

/**
 * Export a 2D density slice to CSV.
 *
 * Slices the 3D lattice along the specified axis at the given index,
 * producing a 2D grid of |J| values suitable for heatmap plotting.
 *
 * Columns: u, v, density, Jx, Jy, Jz, state
 * Where (u,v) are the two coordinates perpendicular to the slice axis.
 *
 * @param bridge    The RenderBridge containing the simulation state
 * @param filename  Output CSV file path
 * @param axis      Slice axis: 'x', 'y', or 'z'
 * @param index     Position along the slice axis
 * @return          true if export succeeded
 */
bool export_density_slice(const RenderBridge& bridge, const std::string& filename,
                          char axis, int index);

/**
 * Append a diagnostics row to a timeseries CSV file.
 *
 * If the file does not exist or is empty, writes the header first.
 * Each call appends one row with current-tick diagnostics.
 *
 * Columns: tick, manifested, positive, negative, total_flux,
 *          total_energy, avg_drag, max_bandwidth, total_entropy
 *
 * @param bridge    The RenderBridge containing the simulation state
 * @param filename  Output CSV file path (will be appended to)
 * @return          true if export succeeded
 */
bool export_diagnostics_row(const RenderBridge& bridge, const std::string& filename);

/**
 * Export a 1D profile along a line to CSV.
 *
 * Useful for detector-line measurements (e.g., double-slit experiment).
 *
 * Columns: position, density, Jx, Jy, Jz, state
 *
 * @param bridge    The RenderBridge containing the simulation state
 * @param filename  Output CSV file path
 * @param axis      The axis along which to sweep: 'x', 'y', or 'z'
 * @param fixed1    First fixed coordinate value
 * @param fixed2    Second fixed coordinate value
 * @return          true if export succeeded
 *
 * Example: export along z at (x=38, y=24):
 *   export_line_profile(bridge, "detector.csv", 'z', 38, 24);
 */
bool export_line_profile(const RenderBridge& bridge, const std::string& filename,
                         char axis, int fixed1, int fixed2);

/**
 * Export per-particle snapshot to CSV (append mode).
 *
 * Each call appends rows for all manifested particles at the current tick.
 * Writes header if the file is new or empty.
 *
 * Columns: tick, particle_id, x, y, z, state, vx, vy, vz,
 *          density, spin, color, pair_id, f_em_mag, f_grav_mag
 */
bool export_particle_snapshot(const RenderBridge& bridge, const std::string& filename);

/**
 * Export radial force profile from a center point along principal axes.
 *
 * Measures along +x, +y, +z, and +xyz diagonal from (cx,cy,cz).
 *
 * Columns: r, axis, grad_divJ_mag, density, div_J, state
 */
bool export_radial_profile(const RenderBridge& bridge, const std::string& filename,
                           int cx, int cy, int cz);

}  // namespace csv
}  // namespace ftd
