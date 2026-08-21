// native/spectrum.cpp — E(k) of the flux field (see native/spectrum.h).

#include "native/spectrum.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace ftd::native {
namespace {

constexpr double kPi = 3.14159265358979323846;

int next_pow2(int n) {
    int m = 1;
    while (m < n) m <<= 1;
    return m;
}

// In-place iterative radix-2 Cooley–Tukey FFT (forward, twiddle e^{-2πi k/len}).
void fft1d(std::vector<double>& re, std::vector<double>& im) {
    const std::size_t n = re.size();
    // Bit-reversal permutation.
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) { std::swap(re[i], re[j]); std::swap(im[i], im[j]); }
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double ang = -2.0 * kPi / static_cast<double>(len);
        const double wr = std::cos(ang), wi = std::sin(ang);
        for (std::size_t i = 0; i < n; i += len) {
            double cwr = 1.0, cwi = 0.0;
            for (std::size_t k = 0; k < len / 2; ++k) {
                const std::size_t a = i + k, b = i + k + len / 2;
                const double vr = re[b] * cwr - im[b] * cwi;
                const double vi = re[b] * cwi + im[b] * cwr;
                re[b] = re[a] - vr; im[b] = im[a] - vi;
                re[a] += vr;        im[a] += vi;
                const double ncwr = cwr * wr - cwi * wi;
                cwi = cwr * wi + cwi * wr;
                cwr = ncwr;
            }
        }
    }
}

// 3D FFT of an M³ flat array (idx = x + M*(y + M*z)): 1D FFT along each axis.
void fft3d(std::vector<double>& re, std::vector<double>& im, int M) {
    std::vector<double> tr(static_cast<std::size_t>(M)), ti(static_cast<std::size_t>(M));
    // X axis: rows of length M with stride 1, for each (y,z).
    for (int z = 0; z < M; ++z)
        for (int y = 0; y < M; ++y) {
            const std::size_t base = (static_cast<std::size_t>(z) * M + y) * M;
            for (int x = 0; x < M; ++x) { tr[x] = re[base + x]; ti[x] = im[base + x]; }
            fft1d(tr, ti);
            for (int x = 0; x < M; ++x) { re[base + x] = tr[x]; im[base + x] = ti[x]; }
        }
    // Y axis: stride M, for each (x,z).
    for (int z = 0; z < M; ++z)
        for (int x = 0; x < M; ++x) {
            const std::size_t base = static_cast<std::size_t>(z) * M * M + x;
            for (int y = 0; y < M; ++y) {
                tr[y] = re[base + static_cast<std::size_t>(y) * M];
                ti[y] = im[base + static_cast<std::size_t>(y) * M];
            }
            fft1d(tr, ti);
            for (int y = 0; y < M; ++y) {
                re[base + static_cast<std::size_t>(y) * M] = tr[y];
                im[base + static_cast<std::size_t>(y) * M] = ti[y];
            }
        }
    // Z axis: stride M², for each (x,y).
    for (int y = 0; y < M; ++y)
        for (int x = 0; x < M; ++x) {
            const std::size_t base = static_cast<std::size_t>(y) * M + x;
            for (int z = 0; z < M; ++z) {
                tr[z] = re[base + static_cast<std::size_t>(z) * M * M];
                ti[z] = im[base + static_cast<std::size_t>(z) * M * M];
            }
            fft1d(tr, ti);
            for (int z = 0; z < M; ++z) {
                re[base + static_cast<std::size_t>(z) * M * M] = tr[z];
                im[base + static_cast<std::size_t>(z) * M * M] = ti[z];
            }
        }
}

// Nearest-cell resample of an L³ field into an M³ grid (identity when M == L).
void resample(const std::vector<float>& src, int L, std::vector<double>& dst, int M) {
    dst.assign(static_cast<std::size_t>(M) * M * M, 0.0);
    if (L <= 0) return;
    for (int z = 0; z < M; ++z) {
        const int sz = std::min(L - 1, z * L / M);
        for (int y = 0; y < M; ++y) {
            const int sy = std::min(L - 1, y * L / M);
            for (int x = 0; x < M; ++x) {
                const int sx = std::min(L - 1, x * L / M);
                const std::size_t si = static_cast<std::size_t>(sx)
                                     + static_cast<std::size_t>(L) * (sy + static_cast<std::size_t>(L) * sz);
                const std::size_t di = static_cast<std::size_t>(x)
                                     + static_cast<std::size_t>(M) * (y + static_cast<std::size_t>(M) * z);
                if (si < src.size()) dst[di] = static_cast<double>(src[si]);
            }
        }
    }
}

}  // namespace

SpectrumResult compute_flux_spectrum(const std::vector<float>& jx,
                                     const std::vector<float>& jy,
                                     const std::vector<float>& jz,
                                     int L, int nbins) {
    SpectrumResult r;
    if (L < 2 || nbins < 2) return r;
    const std::size_t need = static_cast<std::size_t>(L) * L * L;
    if (jx.size() < need || jy.size() < need || jz.size() < need) return r;

    const int M = next_pow2(L);
    r.grid = M;
    const std::size_t M3 = static_cast<std::size_t>(M) * M * M;

    // Real-space total power (the Parseval target).
    double p_real = 0.0;
    for (std::size_t i = 0; i < need; ++i)
        p_real += static_cast<double>(jx[i]) * jx[i] + static_cast<double>(jy[i]) * jy[i]
                + static_cast<double>(jz[i]) * jz[i];
    r.total_power = static_cast<float>(p_real);

    // Per-component FFT → accumulate power spectrum |F|² over the 3 components.
    std::vector<double> power(M3, 0.0);
    std::vector<double> re, im;
    const std::vector<float>* comps[3] = {&jx, &jy, &jz};
    for (const std::vector<float>* comp : comps) {
        resample(*comp, L, re, M);
        im.assign(M3, 0.0);
        fft3d(re, im, M);
        for (std::size_t i = 0; i < M3; ++i)
            power[i] += re[i] * re[i] + im[i] * im[i];
    }
    // Unitary normalization: Σ_k |F|²/M³ = Σ_x |J|²  (Parseval on the M³ grid).
    const double inv = 1.0 / static_cast<double>(M3);

    // Radial bins over |k| ∈ [0, M/2] (cycles per lattice length).
    const double kmax = 0.5 * M;
    r.k.assign(static_cast<std::size_t>(nbins), 0.0f);
    r.ek.assign(static_cast<std::size_t>(nbins), 0.0f);
    for (int b = 0; b < nbins; ++b)
        r.k[b] = static_cast<float>((b + 0.5) / nbins * kmax);
    for (int z = 0; z < M; ++z) {
        const int kz = (z <= M / 2) ? z : z - M;
        for (int y = 0; y < M; ++y) {
            const int ky = (y <= M / 2) ? y : y - M;
            for (int x = 0; x < M; ++x) {
                const int kx = (x <= M / 2) ? x : x - M;
                const double kmag = std::sqrt(static_cast<double>(kx * kx + ky * ky + kz * kz));
                if (kmag <= 0.0) continue;   // drop the DC bin
                int bin = static_cast<int>(kmag / kmax * nbins);
                if (bin >= nbins) bin = nbins - 1;
                const std::size_t i = static_cast<std::size_t>(x)
                                    + static_cast<std::size_t>(M) * (y + static_cast<std::size_t>(M) * z);
                r.ek[static_cast<std::size_t>(bin)] += static_cast<float>(power[i] * inv);
            }
        }
    }

    // Peak (non-DC) bin and a log-log tail slope (spectral index).
    float best = 0.0f;
    for (int b = 0; b < nbins; ++b)
        if (r.ek[b] > best) { best = r.ek[b]; r.peak_k = r.k[b]; }
    double sx = 0, sy = 0, sxx = 0, sxy = 0;
    int cnt = 0;
    for (int b = 1; b < nbins; ++b) {
        if (r.k[b] <= 0.0f || r.ek[b] <= 0.0f) continue;
        const double lx = std::log(r.k[b]), ly = std::log(r.ek[b]);
        sx += lx; sy += ly; sxx += lx * lx; sxy += lx * ly; ++cnt;
    }
    if (cnt >= 2) {
        const double denom = cnt * sxx - sx * sx;
        if (std::abs(denom) > 1e-12) r.slope = static_cast<float>((cnt * sxy - sx * sy) / denom);
    }
    r.ok = true;
    return r;
}

}  // namespace ftd::native
