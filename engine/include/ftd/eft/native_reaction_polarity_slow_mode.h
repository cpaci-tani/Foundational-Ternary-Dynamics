#pragma once

/**
 * @file native_reaction_polarity_slow_mode.h
 * @brief Read-only reaction/source mode observer for FTD-0431.
 */

#include "ftd/eft/native_dynamic_polarity_response.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {

struct NativeReactionModeMeasurement {
    int tick = 0;
    int n = 0;
    std::array<int, 3> direction{};
    std::array<double, 3> k{};
    std::complex<double> source{};
    std::complex<double> divergence{};
    long long occupancy = 0;
    long long signed_state = 0;
};

struct NativeDecayFit {
    double intercept = 0.0;
    double gamma = std::numeric_limits<double>::infinity();
    double normalized_rms = std::numeric_limits<double>::infinity();
    bool valid = false;
};

inline NativeReactionModeMeasurement measure_native_reaction_mode(
    const RenderBridge& bridge,
    int tick,
    int n,
    std::array<int, 3> direction) {
    NativeReactionModeMeasurement out;
    out.tick = tick;
    out.n = n;
    out.direction = direction;
    const int L = bridge.lattice().size();
    const double unit = 2.0 * PI * static_cast<double>(n)
        / static_cast<double>(L);
    for (int axis = 0; axis < 3; ++axis)
        out.k[axis] = unit * static_cast<double>(direction[axis]);

    std::vector<std::complex<double>> phase(static_cast<std::size_t>(L));
    for (int m = 0; m < L; ++m) {
        const double angle = unit * static_cast<double>(m);
        phase[static_cast<std::size_t>(m)] = {std::cos(angle), -std::sin(angle)};
    }

    std::array<std::complex<double>, 3> flux{};
    const auto& lattice = bridge.lattice();
    const auto& voxels = bridge.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int m = direction[0] * x + direction[1] * y + direction[2] * z;
                m %= L;
                if (m < 0) m += L;
                const auto p = phase[static_cast<std::size_t>(m)];
                const auto& voxel = voxels[static_cast<std::size_t>(
                    lattice.index(x, y, z))];
                out.source += static_cast<double>(voxel.state) * p;
                flux[0] += voxel.flux.x * p;
                flux[1] += voxel.flux.y * p;
                flux[2] += voxel.flux.z * p;
                if (voxel.state != 0) ++out.occupancy;
                out.signed_state += static_cast<long long>(voxel.state);
            }
        }
    }

    const double inv_volume = 1.0 / static_cast<double>(lattice.total_sites());
    out.source *= inv_volume;
    for (auto& component : flux) component *= inv_volume;
    std::complex<double> longitudinal{};
    for (int axis = 0; axis < 3; ++axis)
        longitudinal += std::sin(out.k[axis]) * flux[axis];
    out.divergence = std::complex<double>(0.0, 1.0) * longitudinal;
    return out;
}

inline double native_phase_referenced_amplitude(
    const std::complex<double>& source,
    const std::complex<double>& initial_source) {
    const double norm = std::norm(initial_source);
    if (!(norm > 0.0)) return std::numeric_limits<double>::quiet_NaN();
    return (source * std::conj(initial_source)).real() / norm;
}

inline NativeDecayFit fit_native_source_decay(
    const std::vector<NativeReactionModeMeasurement>& samples,
    int max_tick = 6) {
    NativeDecayFit out;
    if (samples.size() < 3 || max_tick < 2) return out;
    const auto initial = samples.front().source;
    double sx = 0.0;
    double sy = 0.0;
    double sxx = 0.0;
    double sxy = 0.0;
    int count = 0;
    std::vector<std::array<double, 2>> points;
    for (const auto& sample : samples) {
        if (sample.tick < 0 || sample.tick > max_tick) continue;
        const double amplitude = native_phase_referenced_amplitude(
            sample.source, initial);
        if (!(amplitude > 0.0) || !std::isfinite(amplitude)) return out;
        const double x = static_cast<double>(sample.tick);
        const double y = std::log(amplitude);
        points.push_back({x, y});
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
        ++count;
    }
    if (count < 3) return out;
    const double denominator = count * sxx - sx * sx;
    if (std::abs(denominator) < 1e-18) return out;
    const double slope = (count * sxy - sx * sy) / denominator;
    out.intercept = (sy - slope * sx) / count;
    out.gamma = -slope;
    double residual_sq = 0.0;
    double data_sq = 0.0;
    for (const auto& point : points) {
        const double model = out.intercept - out.gamma * point[0];
        residual_sq += (point[1] - model) * (point[1] - model);
        data_sq += point[1] * point[1];
    }
    out.normalized_rms = std::sqrt(
        residual_sq / std::max(1e-30, data_sq));
    out.valid = std::isfinite(out.gamma) && std::isfinite(out.normalized_rms);
    return out;
}

inline std::complex<double> native_reaction_field_residual(
    const NativeReactionModeMeasurement& previous,
    const NativeReactionModeMeasurement& current,
    const NativeReactionModeMeasurement& next) {
    const double omega_sq = C_WAVE * C_WAVE * native_wave_symbol_M(current.k);
    double gradient_norm = 0.0;
    for (double component : current.k) {
        const double s = std::sin(component);
        gradient_norm += s * s;
    }
    const std::complex<double> predicted =
        (2.0 - omega_sq) * current.divergence
        - previous.divergence + G_C * gradient_norm * current.source;
    return next.divergence - predicted;
}

}  // namespace ftd::eft
