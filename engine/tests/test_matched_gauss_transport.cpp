/**
 * Exact operator and transport tests for FTD-0427.
 */

#include "ftd/eft/matched_gauss_transport.h"
#include "ftd/constants.h"

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
    std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!pass) ++failures;
}

std::vector<int> dipole_source(int L, int source, int sink, int q) {
    std::vector<int> rho(static_cast<std::size_t>(L * L * L), 0);
    rho[static_cast<std::size_t>(source)] = q;
    rho[static_cast<std::size_t>(sink)] = -q;
    return rho;
}

long double face_edge_pairing(const ftd::eft::MatchedFaceFlux& face,
                              const ftd::eft::MatchedFaceFlux& curl) {
    long double out = 0.0L;
    for (std::size_t i = 0; i < face.x.size(); ++i) {
        out += static_cast<long double>(face.x[i]) * curl.x[i];
        out += static_cast<long double>(face.y[i]) * curl.y[i];
        out += static_cast<long double>(face.z[i]) * curl.z[i];
    }
    return out;
}

long double edge_pairing(const ftd::eft::MatchedEdgeField& lhs,
                         const ftd::eft::MatchedEdgeField& rhs) {
    long double out = 0.0L;
    for (std::size_t i = 0; i < lhs.x.size(); ++i) {
        out += static_cast<long double>(lhs.x[i]) * rhs.x[i];
        out += static_cast<long double>(lhs.y[i]) * rhs.y[i];
        out += static_cast<long double>(lhs.z[i]) * rhs.z[i];
    }
    return out;
}

}  // namespace

int main() {
    constexpr int L = 16;
    ftd::eft::MatchedFaceFlux flux(L);
    const int source = flux.index(2, 5, 7);
    const int target = flux.index(3, 6, 7);
    const int sink = flux.index(12, 10, 9);

    check("dipole path seeds", ftd::eft::seed_dipole_path(
        flux, source, sink, +1.0));
    auto rho_before = dipole_source(L, source, sink, +1);
    check("initial exact Gauss relation",
          ftd::eft::max_gauss_residual(flux, rho_before) <= 1e-12);

    const auto edge = ftd::eft::make_transverse_challenge(L, 1e-3);
    const auto curl = ftd::eft::matched_curl(edge);
    check("transverse curl is nonzero", ftd::eft::l1_norm(curl) > 0.0);
    check("matched div curl identity",
          ftd::eft::max_divergence(curl) <= 1e-12);
    const auto adjoint = ftd::eft::matched_curl_adjoint(flux);
    check("matched curl transpose identity",
          std::abs(face_edge_pairing(flux, curl) -
                   edge_pairing(adjoint, edge)) <= 1e-12L);

    auto rho_after = dipole_source(L, target, sink, +1);
    ftd::eft::DualCellContinuity history;
    const auto extracted = ftd::eft::extract_moore_history_from_snapshots(
        L, rho_before, rho_after, history);
    check("synthetic Moore hop extracted", extracted.valid &&
          extracted.transported_events == 1 &&
          extracted.reaction_sites == 0);
    const auto update = ftd::eft::apply_conservative_current(flux, history);
    check("conservative current accepted", update.valid);
    check("transport continuity exact", update.transport_residual <= 1e-12);
    check("Gauss transported without projection",
          ftd::eft::max_gauss_residual(flux, rho_after) <= 1e-12);

    const double curl_l1 = ftd::eft::apply_transverse_curl(flux, edge);
    check("transverse challenge applied", curl_l1 > 0.0);
    check("transverse update preserves Gauss",
          ftd::eft::max_gauss_residual(flux, rho_after) <= 1e-12);

    const auto surface = ftd::eft::measure_face_cube_charge(flux, 3, 6, 7, 2);
    check("surface telescope exact",
          std::abs(surface.telescope_residual) <= 1e-12);
    check("surface reads transported charge",
          std::abs(surface.boundary_flux - 1.0) <= 1e-12);

    ftd::eft::DualCellContinuity reaction(L);
    reaction.rho_before = rho_after;
    reaction.rho_after = rho_after;
    reaction.rho_after[static_cast<std::size_t>(target)] = 0;
    reaction.reaction[static_cast<std::size_t>(target)] = -1;
    const double norm_before = ftd::eft::l1_norm(flux);
    const auto rejected = ftd::eft::apply_conservative_current(flux, reaction);
    check("reaction history rejected", !rejected.valid &&
          rejected.reaction_l1 == 1);
    check("rejected history leaves field unchanged",
          ftd::eft::l1_norm(flux) == norm_before);

    ftd::eft::MatchedGaussDynamics electrostatic(L);
    const auto minimum = electrostatic.initialize_minimum_energy(rho_before);
    check("minimum-energy dipole converges", minimum.valid &&
          minimum.converged && minimum.iterations <= 12 * L);
    check("minimum-energy field satisfies Gauss",
          minimum.gauss_residual <= 1e-10);
    check("minimum-energy field is longitudinal",
          minimum.curl_adjoint_residual <= 1e-10);
    ftd::eft::MatchedFaceFlux string_field(L);
    check("comparison string field seeds", ftd::eft::seed_dipole_path(
        string_field, source, sink, +1.0));
    check("minimum-energy dressing beats path string",
          minimum.electric_energy < ftd::eft::quadratic_energy(string_field));

    ftd::eft::MatchedGaussDynamics wave(L);
    const std::vector<int> vacuum(static_cast<std::size_t>(L * L * L), 0);
    check("vacuum initialization", wave.initialize_minimum_energy(vacuum).valid);
    check("transverse impulse injected",
          wave.inject_transverse_edge_potential(4, 5, 6, 2, 1e-3));
    ftd::eft::DualCellContinuity empty_history(L);
    empty_history.rho_before = vacuum;
    empty_history.rho_after = vacuum;
    const double invariant_initial = wave.modified_energy(ftd::C_SPEED);
    bool wave_valid = true;
    double max_drift = 0.0;
    for (int tick = 0; tick < 32; ++tick) {
        const auto step = wave.advance(empty_history, ftd::C_SPEED);
        wave_valid = wave_valid && step.valid;
        max_drift = std::max(max_drift,
            std::abs(step.energy_after - invariant_initial));
    }
    check("source-free staggered wave remains valid", wave_valid);
    check("source-free modified energy is conserved",
          max_drift <= 1e-12 * std::max(1.0, std::abs(invariant_initial)));
    check("source-free Gauss remains exact",
          ftd::eft::max_gauss_residual(wave.electric(), vacuum) <= 1e-12);

    std::cout << "matched_gauss_transport failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
