#pragma once
/**
 * Spectral Analysis — FFT-based dispersion relation measurement.
 *
 * Physics justification: The dispersion relation ω(k) is the fundamental
 * diagnostic for whether the lattice wave equation converges to Maxwell.
 * In the continuum limit, we need ω = c|k| (linear, isotropic).
 * Deviations from linearity are discretization artifacts that must be
 * characterized to validate the continuum limit.
 *
 * Also provides power spectral density for identifying dominant modes
 * and mode decomposition (transverse vs longitudinal).
 *
 * Implementation: Radix-2 Cooley-Tukey FFT (no external dependencies).
 * Works with lattice sizes that are powers of 2 (16, 32, 64, 128, 256).
 */

#include <vector>
#include <complex>
#include <cmath>
#include <algorithm>
#include "render_bridge.h"

namespace ftd {

// 1D radix-2 FFT (in-place, Cooley-Tukey)
// Input/output: complex array of length N (must be power of 2)
// inverse=true for IFFT
inline void fft_1d(std::vector<std::complex<double>>& data, bool inverse = false) {
    int N = static_cast<int>(data.size());
    if (N <= 1) return;

    // Bit-reversal permutation
    int j = 0;
    for (int i = 1; i < N; ++i) {
        int bit = N >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) std::swap(data[i], data[j]);
    }

    // Butterfly stages
    for (int len = 2; len <= N; len <<= 1) {
        double angle = (inverse ? 1.0 : -1.0) * 2.0 * PI / len;
        std::complex<double> wn(std::cos(angle), std::sin(angle));

        for (int i = 0; i < N; i += len) {
            std::complex<double> w(1.0, 0.0);
            for (int k = 0; k < len / 2; ++k) {
                std::complex<double> u = data[i + k];
                std::complex<double> v = data[i + k + len / 2] * w;
                data[i + k] = u + v;
                data[i + k + len / 2] = u - v;
                w *= wn;
            }
        }
    }

    if (inverse) {
        for (auto& x : data) x /= N;
    }
}

// 1D power spectral density of a real time series
// Returns |FFT(x)|² / N for frequencies 0 to N/2
inline std::vector<double> power_spectrum(const std::vector<double>& series) {
    // Pad to next power of 2
    int N = 1;
    while (N < static_cast<int>(series.size())) N <<= 1;

    std::vector<std::complex<double>> data(N, {0.0, 0.0});
    for (size_t i = 0; i < series.size(); ++i) {
        data[i] = {series[i], 0.0};
    }

    fft_1d(data);

    // Power = |X(k)|² / N
    int half = N / 2 + 1;
    std::vector<double> psd(half);
    for (int k = 0; k < half; ++k) {
        psd[k] = std::norm(data[k]) / N;
    }
    return psd;
}

// Measure spatial power spectrum of flux field along one axis.
// Extracts J_component(x, y0, z0) for x = 0..L-1, computes |FFT|².
// component: 0=Jx, 1=Jy, 2=Jz
// Returns PSD[k] for k = 0..L/2
inline std::vector<double> spatial_power_spectrum(
    const RenderBridge& rb, int y0, int z0, int component = 0)
{
    const auto& lat = rb.lattice();
    const auto& vox = rb.voxels();
    int L = lat.size();

    std::vector<double> line(L);
    for (int x = 0; x < L; ++x) {
        int idx = lat.index(x, y0, z0);
        switch (component) {
            case 0: line[x] = vox[idx].flux.x; break;
            case 1: line[x] = vox[idx].flux.y; break;
            case 2: line[x] = vox[idx].flux.z; break;
        }
    }
    return power_spectrum(line);
}

// Dispersion relation measurement:
// Place a monochromatic standing wave with known k, evolve, measure ω.
// Returns (k, omega) pair.
// k = 2π·n/L (wavevector), ω measured from temporal oscillation frequency.
struct DispersionPoint {
    double k;      // wavevector magnitude
    double omega;  // angular frequency
    double c_eff;  // effective phase velocity = omega/k
};

// Measure dispersion by initializing a plane wave with mode number n
// along the x-axis, evolving for T ticks, and extracting the frequency.
// The wave has J_y(x) = A·sin(2πnx/L), which excites a single Fourier mode.
inline DispersionPoint measure_dispersion(
    int lattice_size, int mode_number, int num_ticks = 500, double amplitude = 0.1)
{
    RenderBridge rb(lattice_size);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    // No coupling, no genesis, no forces — pure wave equation

    int L = lattice_size;
    double k = 2.0 * PI * mode_number / L;
    int mid = L / 2;

    // Initialize plane wave: J_y(x, y, z) = A·sin(kx) for ALL y,z
    // Must be uniform in y,z so Laplacian only picks up x-variation
    for (int x = 0; x < L; ++x) {
        double val = amplitude * std::sin(k * x);
        for (int y = 0; y < L; ++y)
            for (int z = 0; z < L; ++z)
                rb.inject_flux(x, y, z, {0.0, val, 0.0});
    }

    // Record J_y at a fixed probe point.
    // Use x=1 to avoid nodes: sin(2πn/L) ≠ 0 for reasonable mode numbers.
    // (L/4 is a node for even modes: sin(2π·2n·L/4/L) = sin(nπ) = 0)
    int probe_x = 1;
    int probe_idx = rb.lattice().index(probe_x, mid, mid);
    std::vector<double> time_series;
    time_series.reserve(num_ticks);

    for (int t = 0; t < num_ticks; ++t) {
        time_series.push_back(rb.voxels()[probe_idx].flux.y);
        rb.tick();
    }

    // Extract frequency from PSD peak
    auto psd = power_spectrum(time_series);
    int peak_bin = 1;  // skip DC
    double max_power = 0.0;
    for (int i = 1; i < static_cast<int>(psd.size()); ++i) {
        if (psd[i] > max_power) {
            max_power = psd[i];
            peak_bin = i;
        }
    }

    // Pad size (next power of 2 of num_ticks)
    int N_fft = 1;
    while (N_fft < num_ticks) N_fft <<= 1;

    double omega = 2.0 * PI * peak_bin / N_fft;  // radians per tick

    DispersionPoint dp;
    dp.k = k;
    dp.omega = omega;
    dp.c_eff = (k > 1e-10) ? omega / k : 0.0;
    return dp;
}

// Measure dispersion relation across multiple modes.
// Returns vector of (k, omega, c_eff) for mode numbers 1..max_mode.
inline std::vector<DispersionPoint> dispersion_relation(
    int lattice_size, int max_mode = -1, int num_ticks = 500)
{
    if (max_mode < 0) max_mode = lattice_size / 4;

    std::vector<DispersionPoint> points;
    for (int n = 1; n <= max_mode; ++n) {
        points.push_back(measure_dispersion(lattice_size, n, num_ticks));
    }
    return points;
}

}  // namespace ftd
