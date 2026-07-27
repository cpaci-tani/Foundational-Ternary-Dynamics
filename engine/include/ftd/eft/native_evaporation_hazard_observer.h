#pragma once

/**
 * @file native_evaporation_hazard_observer.h
 * @brief Exact pre-RNG conditional evaporation observer for FTD-0432.
 *
 * The observer reproduces the standard unit-tick single-substrate wave write
 * in scratch arrays, then evaluates the production evaporation probability.
 * It commits no voxel, RNG, toggle, backend, or tick state.  prepare_delta_j()
 * writes only the existing diagnostic acceleration buffer.
 */

#include "ftd/constants.h"
#include "ftd/proper_time_rate.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {

struct NativeEvaporationHazardObservation {
    int tick = 0;
    int n = 0;
    std::array<int, 3> direction{};
    std::array<double, 3> k{};
    std::complex<double> source{};
    std::complex<double> expected_loss_source{};
    std::complex<double> predicted_next_source{};
    double source_hazard = 0.0;
    double expected_removals = 0.0;
    double removal_variance = 0.0;
    double mean_site_probability = 0.0;
    double min_site_probability = 0.0;
    double max_site_probability = 0.0;
    double mean_local_energy = 0.0;
    double max_local_energy = 0.0;
    long long occupancy = 0;
    long long eligible_sites = 0;
};

inline NativeEvaporationHazardObservation
observe_native_evaporation_hazard(
    RenderBridge& bridge,
    int tick,
    int n,
    std::array<int, 3> direction) {
    NativeEvaporationHazardObservation out;
    out.tick = tick;
    out.n = n;
    out.direction = direction;

    bridge.prepare_delta_j();
    const auto& delta = bridge.delta_j();
    const auto& lattice = bridge.lattice();
    const auto& voxels = static_cast<const RenderBridge&>(bridge).voxels();
    const int L = lattice.size();
    const std::size_t total = lattice.total_sites();
    const double inverse_total = 1.0 / static_cast<double>(total);
    const double unit = 2.0 * PI * static_cast<double>(n)
        / static_cast<double>(L);
    for (int axis = 0; axis < 3; ++axis)
        out.k[axis] = unit * static_cast<double>(direction[axis]);

    std::vector<std::complex<double>> phase(static_cast<std::size_t>(L));
    for (int coordinate = 0; coordinate < L; ++coordinate) {
        const double angle = unit * static_cast<double>(coordinate);
        phase[static_cast<std::size_t>(coordinate)] = {
            std::cos(angle), -std::sin(angle)};
    }

    std::vector<Vec3> predicted_flux(total);
    std::vector<Vec3> predicted_velocity(total);
    for (std::size_t index = 0; index < total; ++index) {
        predicted_velocity[index] = voxels[index].wave_vel + delta[index];
        predicted_flux[index] = voxels[index].flux
            + predicted_velocity[index];
    }

    double energy_sum = 0.0;
    double probability_min = std::numeric_limits<double>::infinity();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const int index = lattice.index(x, y, z);
                const auto& voxel = voxels[static_cast<std::size_t>(index)];
                const auto mode_phase =
                    phase[static_cast<std::size_t>(x)]
                    * (direction[1] == 0
                        ? std::complex<double>(1.0, 0.0)
                        : phase[static_cast<std::size_t>(y)])
                    * (direction[2] == 0
                        ? std::complex<double>(1.0, 0.0)
                        : phase[static_cast<std::size_t>(z)]);
                const auto weighted_state =
                    static_cast<double>(voxel.state) * mode_phase;
                out.source += weighted_state * inverse_total;
                if (voxel.state == 0) continue;
                ++out.occupancy;
                if (voxel.locked) continue;

                double local_energy =
                    predicted_flux[static_cast<std::size_t>(index)].mag2()
                    + predicted_velocity[static_cast<std::size_t>(index)].mag2();
                for (int neighbor : lattice.neighbors_6(index)) {
                    local_energy +=
                        predicted_flux[static_cast<std::size_t>(neighbor)].mag2()
                        + predicted_velocity[
                            static_cast<std::size_t>(neighbor)].mag2();
                }
                const double dtau = proper_time_rate(
                    voxel.latency, voxel.speed() * voxel.speed());
                const double probability = std::clamp(
                    K_EVAP_RATE * dtau
                        * std::exp(-local_energy
                            / (K_MANIFEST * K_MANIFEST)),
                    0.0, K_EVAP_RATE);
                out.expected_removals += probability;
                out.removal_variance += probability * (1.0 - probability);
                out.expected_loss_source +=
                    weighted_state * probability * inverse_total;
                energy_sum += local_energy;
                out.max_local_energy = std::max(
                    out.max_local_energy, local_energy);
                probability_min = std::min(probability_min, probability);
                out.max_site_probability = std::max(
                    out.max_site_probability, probability);
                ++out.eligible_sites;
            }
        }
    }

    if (out.eligible_sites > 0) {
        out.mean_site_probability = out.expected_removals
            / static_cast<double>(out.eligible_sites);
        out.mean_local_energy = energy_sum
            / static_cast<double>(out.eligible_sites);
        out.min_site_probability = probability_min;
    }
    const double source_norm = std::norm(out.source);
    if (source_norm > 0.0) {
        out.source_hazard = (
            out.expected_loss_source * std::conj(out.source)).real()
            / source_norm;
    }
    out.predicted_next_source = out.source - out.expected_loss_source;
    return out;
}

}  // namespace ftd::eft
