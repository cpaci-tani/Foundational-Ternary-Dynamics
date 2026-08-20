#pragma once

/**
 * @file ftd/eft/matched_gauss_transport.h
 * @brief Projection-free oriented-face Gauss transport sidecar (FTD-0427).
 *
 * This is an experimental selected mechanism. It does not replace or mutate
 * RenderBridge's cell-centred Voxel::flux field. The discrete operators share
 * one backward-difference complex, so div(curl(.)) vanishes algebraically.
 */

#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/voxel.h"

#include <vector>

namespace ftd::eft {

struct MatchedEdgeField {
    int L = 0;
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;

    explicit MatchedEdgeField(int size = 0);
    int index(int x_coord, int y_coord, int z_coord) const;
};

struct MatchedFaceFlux {
    int L = 0;
    // Component at site i is the flux through i's positive-axis face.
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;

    explicit MatchedFaceFlux(int size = 0);
    int index(int x_coord, int y_coord, int z_coord) const;
};

struct MatchedTransportUpdate {
    bool valid = false;
    int reaction_l1 = 0;
    double transport_residual = 0.0;
    double current_l1 = 0.0;
};

struct MatchedSurfaceCharge {
    int radius = 0;
    int enclosed_sites = 0;
    double boundary_flux = 0.0;
    double divergence_sum = 0.0;
    double telescope_residual = 0.0;
};

struct MatchedMinimumEnergyResult {
    bool valid = false;
    bool neutral = false;
    bool converged = false;
    int iterations = 0;
    double solver_residual = 0.0;
    double gauss_residual = 0.0;
    double curl_adjoint_residual = 0.0;
    double electric_energy = 0.0;
};

struct MatchedWaveStep {
    bool valid = false;
    MatchedTransportUpdate transport;
    double gauss_residual = 0.0;
    double energy_before = 0.0;
    double energy_after = 0.0;
    double electric_l1 = 0.0;
    double magnetic_l1 = 0.0;
};

/// Seed an exact dipole: div(J)=+amount at source and -amount at sink.
/// The route is the deterministic shortest periodic x/y/z path.
bool seed_dipole_path(MatchedFaceFlux& field,
                      int source_index,
                      int sink_index,
                      double amount);

/// Apply J <- J-K for a source-free finite-volume transport history.
/// Histories with any reaction term are rejected without changing J.
MatchedTransportUpdate apply_conservative_current(
    MatchedFaceFlux& field,
    const DualCellContinuity& history,
    double tolerance = 1e-12);

/// Return C B using the matched backward-difference curl.
MatchedFaceFlux matched_curl(const MatchedEdgeField& edge);

/// Return C^T E, the exact periodic transpose of matched_curl().
MatchedEdgeField matched_curl_adjoint(const MatchedFaceFlux& face);

/// Apply J <- J + scale*C B. Returns ||scale*C B||_1.
double apply_transverse_curl(MatchedFaceFlux& field,
                             const MatchedEdgeField& edge,
                             double scale = 1.0);

double divergence_at(const MatchedFaceFlux& field,
                     int x_coord,
                     int y_coord,
                     int z_coord);
double max_divergence(const MatchedFaceFlux& field);
double max_curl_adjoint(const MatchedFaceFlux& field);
double max_gauss_residual(const MatchedFaceFlux& field,
                          const std::vector<int>& site_source);
double l1_norm(const MatchedFaceFlux& field);
double l1_norm(const MatchedEdgeField& field);
double quadratic_energy(const MatchedFaceFlux& field);
double quadratic_energy(const MatchedEdgeField& field);

MatchedSurfaceCharge measure_face_cube_charge(const MatchedFaceFlux& field,
                                              int cx,
                                              int cy,
                                              int cz,
                                              int radius);

/// Deterministic nonzero periodic edge field used only as a transverse gate.
MatchedEdgeField make_transverse_challenge(int L, double amplitude = 1e-3);

/**
 * Default-disconnected staggered Maxwell/Gauss state used by FTD-0428.
 * B is stored at the half tick. No global solve occurs in advance().
 */
class MatchedGaussDynamics {
  public:
    explicit MatchedGaussDynamics(int size = 0);

    int size() const { return electric_.L; }
    bool initialized() const { return initialized_; }
    const MatchedFaceFlux& electric() const { return electric_; }
    const MatchedEdgeField& magnetic_half() const { return magnetic_half_; }
    const MatchedMinimumEnergyResult& initialization_result() const {
        return initialization_result_;
    }
    const MatchedWaveStep& last_step() const { return last_step_; }

    void reset(int size);

    MatchedMinimumEnergyResult initialize_minimum_energy(
        const std::vector<int>& site_source,
        double tolerance = 1e-12,
        int max_iterations = 0);

    MatchedWaveStep advance(const DualCellContinuity& history,
                            double wave_speed,
                            double dt = 1.0,
                            double tolerance = 1e-12);

    bool inject_transverse_edge_potential(int x, int y, int z,
                                          int axis, double amplitude);

    Vec3 centered_electric_at(int x, int y, int z) const;
    double modified_energy(double wave_speed, double dt = 1.0) const;

    /// Replace the live face/edge state after a device advance (GPU mirror).
    void adopt_state(MatchedFaceFlux electric,
                     MatchedEdgeField magnetic,
                     const MatchedWaveStep& step);

  private:
    MatchedFaceFlux electric_;
    MatchedEdgeField magnetic_half_;
    bool initialized_ = false;
    MatchedMinimumEnergyResult initialization_result_;
    MatchedWaveStep last_step_;
};

}  // namespace ftd::eft
