#pragma once
//
// native/spectrum.h — the spatial energy spectrum E(k) of the Scale-0 flux field.
//
// A native port of engine/web/js/scales/scale0/analysis/lattice-spectrum.js: sample
// the flux vector field onto a power-of-2 grid M, take a 3D radix-2 FFT of each
// component, sum the per-component power, and radial-bin by |k|. Parseval-normalized
// so Σ_bins E(k) ≈ Σ_x |J(x)|². Pure math (no engine/RmlUi deps) so it is unit-
// testable and callable from the Scale-0 adapter's snapshot builder.
//
#include <vector>

namespace ftd::native {

struct SpectrumResult {
    std::vector<float> k;      // bin-centre |k| (cycles per lattice length)
    std::vector<float> ek;     // radial power E(k)  (Σ ek ≈ Σ|J|², Parseval)
    float total_power = 0.0f;  // Σ_x |J(x)|²  (real-space, the Parseval target)
    float peak_k = 0.0f;       // |k| of the largest-E(k) non-DC bin
    float slope = 0.0f;        // log-log least-squares slope of the tail (spectral index)
    int   grid = 0;            // FFT grid size M = nextPow2(L)
    bool  ok = false;
};

// E(k) of the flux vector field. jx/jy/jz are the L³ field components in lattice
// order (idx = x + L*(y + L*z)); L need not be a power of 2 (the field is sampled
// onto M = nextPow2(L) by nearest-cell). `nbins` radial bins over |k| ∈ [0, M/2].
SpectrumResult compute_flux_spectrum(const std::vector<float>& jx,
                                     const std::vector<float>& jy,
                                     const std::vector<float>& jz,
                                     int L, int nbins = 32);

}  // namespace ftd::native
