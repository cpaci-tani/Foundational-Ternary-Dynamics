// test_spectrum.cpp — unit check for native/spectrum.h: Parseval + a known peak.

#include "native/spectrum.h"

#include <cmath>
#include <cstdio>
#include <vector>

int main() {
    const int L = 32;
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    std::vector<float> jx(N, 0.0f), jy(N, 0.0f), jz(N, 0.0f);

    // A pure plane wave in jx along x with n = 4 cycles across the box.
    const int n = 4;
    for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
            for (int x = 0; x < L; ++x) {
                const std::size_t i = static_cast<std::size_t>(x)
                                    + static_cast<std::size_t>(L) * (y + static_cast<std::size_t>(L) * z);
                jx[i] = static_cast<float>(std::sin(2.0 * 3.14159265358979 * n * x / L));
            }

    const ftd::native::SpectrumResult r =
        ftd::native::compute_flux_spectrum(jx, jy, jz, L, 32);
    if (!r.ok) { std::printf("FAIL: result not ok\n"); return 1; }

    float sum_ek = 0.0f;
    for (float e : r.ek) sum_ek += e;
    const float ratio = r.total_power > 0.0f ? sum_ek / r.total_power : 0.0f;
    std::printf("total_power=%.3f  sum_ek=%.3f  parseval_ratio=%.4f\n",
                r.total_power, sum_ek, ratio);
    std::printf("peak_k=%.3f (expect ~%d)  grid=%d  slope=%.3f\n",
                r.peak_k, n, r.grid, r.slope);

    const bool parseval_ok = std::fabs(ratio - 1.0f) < 0.02f;
    const bool peak_ok = std::fabs(r.peak_k - static_cast<float>(n)) < 2.0f;
    std::printf("parseval_ok=%d peak_ok=%d -> %s\n",
                parseval_ok, peak_ok, (parseval_ok && peak_ok) ? "PASS" : "FAIL");
    return (parseval_ok && peak_ok) ? 0 : 1;
}
