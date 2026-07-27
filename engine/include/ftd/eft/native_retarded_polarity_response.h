#pragma once

/**
 * @file native_retarded_polarity_response.h
 * @brief Batched read-only moving-source observer for FTD-0430.
 *
 * The observer compares two settled RenderBridge snapshots.  It performs no
 * writes and owns no engine state.  Fourier divergence uses the same central
 * difference symbol as the production field operator.
 */

#include "ftd/eft/native_dynamic_polarity_response.h"
#include "ftd/field_operators.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {

struct NativeRetardedMode {
    int n = 0;
    std::array<int, 3> direction{};
    std::array<double, 3> k{};
    std::complex<double> moving_source{};
    std::complex<double> stationary_source{};
    std::complex<double> delta_source{};
    std::array<std::complex<double>, 3> delta_flux{};
    std::complex<double> delta_divergence{};
    std::complex<double> response{};
};

struct NativeCausalSupport {
    int tau = 0;
    int allowed_radius = 0;
    int support_radius = -1;
    int support_sites = 0;
    double max_abs = 0.0;
    double max_outside = 0.0;
};

inline std::vector<NativeRetardedMode> native_retarded_mode_basis(int L) {
    const std::array<std::array<int, 3>, 3> directions{{
        {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}};
    std::vector<NativeRetardedMode> modes;
    modes.reserve(9);
    for (const auto& direction : directions) {
        for (int n : {1, 2, 3}) {
            NativeRetardedMode mode;
            mode.n = n;
            mode.direction = direction;
            const double unit = 2.0 * PI * static_cast<double>(n)
                / static_cast<double>(L);
            for (int axis = 0; axis < 3; ++axis)
                mode.k[axis] = unit * static_cast<double>(direction[axis]);
            modes.push_back(mode);
        }
    }
    return modes;
}

inline std::vector<NativeRetardedMode> measure_native_retarded_modes(
    const RenderBridge& moving, const RenderBridge& stationary) {
    const int L = moving.lattice().size();
    if (stationary.lattice().size() != L) return {};

    auto modes = native_retarded_mode_basis(L);
    std::vector<std::vector<std::complex<double>>> phases(
        modes.size(), std::vector<std::complex<double>>(static_cast<std::size_t>(L)));
    for (std::size_t mode_index = 0; mode_index < modes.size(); ++mode_index) {
        const double unit = 2.0 * PI * static_cast<double>(modes[mode_index].n)
            / static_cast<double>(L);
        for (int m = 0; m < L; ++m) {
            const double angle = unit * static_cast<double>(m);
            phases[mode_index][static_cast<std::size_t>(m)] =
                {std::cos(angle), -std::sin(angle)};
        }
    }

    const auto& lattice = moving.lattice();
    const auto& moving_voxels = moving.voxels();
    const auto& stationary_voxels = stationary.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                const std::size_t index = static_cast<std::size_t>(
                    lattice.index(x, y, z));
                const Voxel& mv = moving_voxels[index];
                const Voxel& sv = stationary_voxels[index];
                for (std::size_t mode_index = 0;
                     mode_index < modes.size(); ++mode_index) {
                    auto& mode = modes[mode_index];
                    int m = mode.direction[0] * x
                        + mode.direction[1] * y
                        + mode.direction[2] * z;
                    m %= L;
                    if (m < 0) m += L;
                    const auto phase = phases[mode_index][static_cast<std::size_t>(m)];
                    mode.moving_source += static_cast<double>(mv.state) * phase;
                    mode.stationary_source += static_cast<double>(sv.state) * phase;
                    const Vec3 delta = mv.flux - sv.flux;
                    mode.delta_flux[0] += delta.x * phase;
                    mode.delta_flux[1] += delta.y * phase;
                    mode.delta_flux[2] += delta.z * phase;
                }
            }
        }
    }

    const double inv_volume = 1.0 / static_cast<double>(lattice.total_sites());
    for (auto& mode : modes) {
        mode.moving_source *= inv_volume;
        mode.stationary_source *= inv_volume;
        mode.delta_source = mode.moving_source - mode.stationary_source;
        for (auto& component : mode.delta_flux) component *= inv_volume;
        std::complex<double> longitudinal{};
        for (int axis = 0; axis < 3; ++axis)
            longitudinal += std::sin(mode.k[axis]) * mode.delta_flux[axis];
        mode.delta_divergence = std::complex<double>(0.0, 1.0) * longitudinal;
        if (std::abs(mode.delta_source) > 0.0)
            mode.response = mode.delta_divergence / mode.delta_source;
    }
    return modes;
}

inline int periodic_axis_distance(int a, int b, int L) {
    const int direct = std::abs(a - b);
    return std::min(direct, L - direct);
}

inline int periodic_chebyshev_distance(
    const Coord& point, const Coord& source, int L) {
    return std::max({
        periodic_axis_distance(point.x, source.x, L),
        periodic_axis_distance(point.y, source.y, L),
        periodic_axis_distance(point.z, source.z, L)});
}

inline NativeCausalSupport measure_native_causal_support(
    const RenderBridge& moving,
    const RenderBridge& stationary,
    const std::array<int, 4>& changed_sites,
    int tau,
    double support_threshold = 1e-13) {
    NativeCausalSupport out;
    out.tau = tau;
    out.allowed_radius = tau > 0 ? tau + 1 : 0;
    const int L = moving.lattice().size();
    if (stationary.lattice().size() != L) {
        out.max_abs = std::numeric_limits<double>::infinity();
        out.max_outside = out.max_abs;
        return out;
    }

    const auto& lattice = moving.lattice();
    const auto& mv = moving.voxels();
    const auto& sv = stationary.voxels();
    std::array<Coord, 4> changed{};
    for (std::size_t i = 0; i < changed.size(); ++i)
        changed[i] = lattice.coord(changed_sites[i]);

    for (std::size_t index = 0; index < lattice.total_sites(); ++index) {
        const auto& neighbors = lattice.neighbors_6(static_cast<int>(index));
        const auto component_difference = [&](int neighbor, int axis) {
            const Vec3 delta = mv[static_cast<std::size_t>(neighbor)].flux
                - sv[static_cast<std::size_t>(neighbor)].flux;
            return axis == 0 ? delta.x : (axis == 1 ? delta.y : delta.z);
        };
        const double divergence = 0.5 * (
            component_difference(neighbors[0], 0)
            - component_difference(neighbors[1], 0)
            + component_difference(neighbors[2], 1)
            - component_difference(neighbors[3], 1)
            + component_difference(neighbors[4], 2)
            - component_difference(neighbors[5], 2));
        const double magnitude = std::abs(divergence);
        out.max_abs = std::max(out.max_abs, magnitude);

        const auto point = lattice.coord(static_cast<int>(index));
        int distance = L;
        for (const auto& source : changed)
            distance = std::min(distance,
                                periodic_chebyshev_distance(point, source, L));
        if (magnitude > support_threshold) {
            ++out.support_sites;
            out.support_radius = std::max(out.support_radius, distance);
        }
        if (distance > out.allowed_radius)
            out.max_outside = std::max(out.max_outside, magnitude);
    }
    return out;
}

inline double native_step_residue_ratio(const NativeResponseFit& fit) {
    const double intercept = std::abs(fit.intercept);
    if (!(intercept > 0.0)) return std::numeric_limits<double>::infinity();
    return std::sqrt(std::norm(fit.cosine) + std::norm(fit.sine)) / intercept;
}

inline double native_exact_step_residue_ratio(double omega) {
    return 1.0 / std::cos(0.5 * omega);
}

}  // namespace ftd::eft
