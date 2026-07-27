#pragma once

/**
 * @file native_dynamic_polarity_response.h
 * @brief Read-only Fourier observer for FTD-0429.
 *
 * This observer projects the existing cell-centred ternary and flux fields.
 * It owns no engine state and performs no writes.  The central divergence is
 * evaluated from its exact Fourier symbol, i sin(k_a).
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <complex>
#include <cstddef>
#include <limits>
#include <vector>

namespace ftd::eft {

struct NativePolarityMode {
    int n = 0;
    std::array<int, 3> direction{};
    std::array<double, 3> k{};
    std::complex<double> source{};
    std::array<std::complex<double>, 3> flux{};
    std::complex<double> divergence{};
    std::complex<double> response{};
};

struct NativeResponseSample {
    int tick = 0;
    std::complex<double> response{};
};

struct NativeResponseFit {
    std::complex<double> intercept{};
    std::complex<double> cosine{};
    std::complex<double> sine{};
    double normalized_residual = std::numeric_limits<double>::infinity();
    bool valid = false;
};

inline double native_wave_symbol_M(const std::array<double, 3>& k) {
    const double cx = std::cos(k[0]);
    const double cy = std::cos(k[1]);
    const double cz = std::cos(k[2]);
    return 4.0
        - (2.0 / 3.0) * (cx + cy + cz)
        - (2.0 / 3.0) * (cx * cy + cx * cz + cy * cz);
}

inline double native_discrete_pole(const std::array<double, 3>& k) {
    const double argument = std::clamp(
        1.0 - C_WAVE * C_WAVE * native_wave_symbol_M(k) * 0.5,
        -1.0, 1.0);
    return std::acos(argument);
}

inline double native_exact_static_response(const std::array<double, 3>& k) {
    double gradient_norm = 0.0;
    for (double component : k) {
        const double s = std::sin(component);
        gradient_norm += s * s;
    }
    const double M = native_wave_symbol_M(k);
    return M > 0.0
        ? (G_C / (C_WAVE * C_WAVE)) * gradient_norm / M
        : 0.0;
}

inline NativePolarityMode measure_native_polarity_mode(
    const RenderBridge& bridge, int n, std::array<int, 3> direction) {
    NativePolarityMode out;
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

    const auto& lattice = bridge.lattice();
    const auto& voxels = bridge.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int m = direction[0] * x + direction[1] * y + direction[2] * z;
                m %= L;
                if (m < 0) m += L;
                const std::complex<double> p = phase[static_cast<std::size_t>(m)];
                const Voxel& voxel = voxels[static_cast<std::size_t>(
                    lattice.index(x, y, z))];
                out.source += static_cast<double>(voxel.state) * p;
                out.flux[0] += voxel.flux.x * p;
                out.flux[1] += voxel.flux.y * p;
                out.flux[2] += voxel.flux.z * p;
            }
        }
    }

    const double inv_volume = 1.0 / static_cast<double>(lattice.total_sites());
    out.source *= inv_volume;
    for (auto& component : out.flux) component *= inv_volume;

    std::complex<double> longitudinal{};
    for (int axis = 0; axis < 3; ++axis)
        longitudinal += std::sin(out.k[axis]) * out.flux[axis];
    out.divergence = std::complex<double>(0.0, 1.0) * longitudinal;
    if (std::abs(out.source) > 0.0)
        out.response = out.divergence / out.source;
    return out;
}

namespace detail {

inline bool solve_three_by_three(double matrix[3][3], double rhs[3],
                                 double solution[3]) {
    double augmented[3][4]{};
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col)
            augmented[row][col] = matrix[row][col];
        augmented[row][3] = rhs[row];
    }
    for (int pivot = 0; pivot < 3; ++pivot) {
        int best = pivot;
        for (int row = pivot + 1; row < 3; ++row)
            if (std::abs(augmented[row][pivot]) >
                std::abs(augmented[best][pivot])) best = row;
        if (std::abs(augmented[best][pivot]) < 1e-18) return false;
        if (best != pivot)
            for (int col = pivot; col < 4; ++col)
                std::swap(augmented[pivot][col], augmented[best][col]);
        const double scale = augmented[pivot][pivot];
        for (int col = pivot; col < 4; ++col) augmented[pivot][col] /= scale;
        for (int row = 0; row < 3; ++row) {
            if (row == pivot) continue;
            const double factor = augmented[row][pivot];
            for (int col = pivot; col < 4; ++col)
                augmented[row][col] -= factor * augmented[pivot][col];
        }
    }
    for (int row = 0; row < 3; ++row) solution[row] = augmented[row][3];
    return true;
}

}  // namespace detail

inline NativeResponseFit fit_native_response(
    const std::vector<NativeResponseSample>& samples, double omega) {
    NativeResponseFit out;
    if (samples.size() < 4 || !(omega > 0.0)) return out;

    double normal[3][3]{};
    double rhs_real[3]{};
    double rhs_imag[3]{};
    for (const auto& sample : samples) {
        const double basis[3]{
            1.0,
            std::cos(omega * static_cast<double>(sample.tick)),
            std::sin(omega * static_cast<double>(sample.tick))};
        for (int row = 0; row < 3; ++row) {
            rhs_real[row] += basis[row] * sample.response.real();
            rhs_imag[row] += basis[row] * sample.response.imag();
            for (int col = 0; col < 3; ++col)
                normal[row][col] += basis[row] * basis[col];
        }
    }

    double normal_real[3][3]{};
    double normal_imag[3][3]{};
    for (int row = 0; row < 3; ++row)
        for (int col = 0; col < 3; ++col)
            normal_real[row][col] = normal_imag[row][col] = normal[row][col];
    double real_coeff[3]{};
    double imag_coeff[3]{};
    if (!detail::solve_three_by_three(normal_real, rhs_real, real_coeff)
        || !detail::solve_three_by_three(normal_imag, rhs_imag, imag_coeff)) {
        return out;
    }

    out.intercept = {real_coeff[0], imag_coeff[0]};
    out.cosine = {real_coeff[1], imag_coeff[1]};
    out.sine = {real_coeff[2], imag_coeff[2]};
    double residual_sq = 0.0;
    double data_sq = 0.0;
    for (const auto& sample : samples) {
        const std::complex<double> model = out.intercept
            + out.cosine * std::cos(omega * static_cast<double>(sample.tick))
            + out.sine * std::sin(omega * static_cast<double>(sample.tick));
        residual_sq += std::norm(sample.response - model);
        data_sq += std::norm(sample.response);
    }
    out.normalized_residual = std::sqrt(
        residual_sq / std::max(1e-30, data_sq));
    out.valid = std::isfinite(out.normalized_residual)
        && std::isfinite(out.intercept.real())
        && std::isfinite(out.intercept.imag());
    return out;
}

}  // namespace ftd::eft
