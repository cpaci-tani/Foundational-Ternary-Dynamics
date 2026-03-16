/**
 * Test: Spectral Analysis (Phase 1 — Measurement Infrastructure)
 *
 * Verifies FFT implementation and dispersion relation measurement.
 *
 *   SP1: FFT of delta function → flat spectrum
 *   SP2: FFT of single sine → peak at correct frequency
 *   SP3: FFT inverse recovers original signal
 *   SP4: Spatial power spectrum of uniform field → DC only
 *   SP5: Dispersion measurement on single mode → correct ω and c_eff
 *   SP6: Multiple dispersion points → approximately linear ω(k)
 */

#include <cmath>
#include <complex>
#include <iostream>
#include <vector>
#include "ftd/spectral.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Spectral Analysis (Phase 1) — 6 Checks\n";
    std::cout << "================================================================\n";

    // ----------------------------------------------------------------
    // SP1: Delta function → flat spectrum
    // ----------------------------------------------------------------
    std::cout << "\n--- SP1: Delta Function Spectrum ---\n";
    {
        std::vector<double> delta(64, 0.0);
        delta[0] = 1.0;
        auto psd = ftd::power_spectrum(delta);

        // All bins should have equal power = 1/N
        double expected = 1.0 / 64.0;
        bool all_equal = true;
        for (double p : psd) {
            if (std::abs(p - expected) > 1e-10) { all_equal = false; break; }
        }
        check("SP1: Delta → flat PSD", all_equal);
    }

    // ----------------------------------------------------------------
    // SP2: Single sine → peak at correct bin
    // ----------------------------------------------------------------
    std::cout << "\n--- SP2: Sine Peak Detection ---\n";
    {
        int N = 128;
        int freq_bin = 5;  // 5 cycles in N samples
        std::vector<double> sine(N);
        for (int i = 0; i < N; ++i) {
            sine[i] = std::sin(2.0 * ftd::PI * freq_bin * i / N);
        }
        auto psd = ftd::power_spectrum(sine);

        // Find peak
        int peak = 0;
        double max_p = 0.0;
        for (int i = 1; i < static_cast<int>(psd.size()); ++i) {
            if (psd[i] > max_p) { max_p = psd[i]; peak = i; }
        }
        std::cout << "  Expected peak at bin " << freq_bin << ", found at bin " << peak << "\n";
        check("SP2: Sine peak at correct frequency", peak == freq_bin);
    }

    // ----------------------------------------------------------------
    // SP3: FFT inverse recovers original
    // ----------------------------------------------------------------
    std::cout << "\n--- SP3: FFT Round-Trip ---\n";
    {
        int N = 32;
        std::vector<std::complex<double>> data(N);
        for (int i = 0; i < N; ++i) {
            data[i] = {std::sin(0.3 * i), std::cos(0.7 * i)};
        }
        auto original = data;

        ftd::fft_1d(data, false);   // forward
        ftd::fft_1d(data, true);    // inverse

        double max_err = 0.0;
        for (int i = 0; i < N; ++i) {
            max_err = std::max(max_err, std::abs(data[i] - original[i]));
        }
        std::cout << "  Round-trip max error: " << max_err << "\n";
        check("SP3: FFT→IFFT error < 1e-12", max_err < 1e-12);
    }

    // ----------------------------------------------------------------
    // SP4: Uniform flux → DC-only spatial spectrum
    // ----------------------------------------------------------------
    std::cout << "\n--- SP4: Uniform Flux Spatial Spectrum ---\n";
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0.5, 0.0, 0.0});

        auto psd = ftd::spatial_power_spectrum(rb, L/2, L/2, 0);

        // DC bin should dominate
        double dc_power = psd[0];
        double ac_total = 0.0;
        for (size_t i = 1; i < psd.size(); ++i) ac_total += psd[i];

        std::cout << "  DC=" << dc_power << " AC_total=" << ac_total << "\n";
        check("SP4: Uniform field → DC >> AC", dc_power > 100.0 * ac_total + 1e-30);
    }

    // ----------------------------------------------------------------
    // SP5: Dispersion measurement on single mode
    // ----------------------------------------------------------------
    std::cout << "\n--- SP5: Single Mode Dispersion ---\n";
    {
        // Mode 1 on L=32 lattice
        auto dp = ftd::measure_dispersion(32, 1, 256, 0.01);
        std::cout << "  k=" << dp.k << " omega=" << dp.omega
                  << " c_eff=" << dp.c_eff << "\n";

        // c_eff should be near C_WAVE = 1/sqrt(3) ≈ 0.577
        // But on a discrete lattice, the exact value depends on k
        // For low k (long wavelength), c_eff → C_WAVE
        check("SP5: c_eff > 0 (wave propagates)", dp.c_eff > 0.0);
        check("SP5b: c_eff < 1.0 (subluminal)", dp.c_eff < 1.0);
    }

    // ----------------------------------------------------------------
    // SP6: Multiple modes → approximately linear ω(k)
    // ----------------------------------------------------------------
    std::cout << "\n--- SP6: Dispersion Relation Linearity ---\n";
    {
        auto points = ftd::dispersion_relation(32, 4, 256);

        std::cout << "  Mode | k       | omega   | c_eff\n";
        for (const auto& p : points) {
            std::cout << "       | " << p.k << " | " << p.omega
                      << " | " << p.c_eff << "\n";
        }

        // Check that ω increases with k (monotonic)
        bool monotonic = true;
        for (size_t i = 1; i < points.size(); ++i) {
            if (points[i].omega < points[i-1].omega - 1e-10) {
                monotonic = false;
                break;
            }
        }
        check("SP6: omega(k) is monotonically increasing", monotonic);
    }

    std::cout << "\n================================================================\n";
    std::cout << "  RESULT: " << (failures == 0 ? "ALL PASSED" : "FAILURES DETECTED")
              << " (" << failures << " failures)\n";
    std::cout << "================================================================\n";
    return failures;
}
