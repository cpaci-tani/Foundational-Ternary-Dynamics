/**
 * @file test_watson_integrals.cpp
 * @brief Numerical Watson integrals for SC, BCC, FCC, and Moore-18 stencils.
 *
 * W_L = (1/(2π)³) ∫ d³k / (1 - p_L(k)) over the Brillouin zone [-π, π]³.
 *
 * Reference values (classical):
 *   W_SC   = 1.51639...  (Watson 1939)
 *   W_BCC  = Γ(1/4)^4 / (4π³) = G*² / (2π) = 1.39320...
 *   W_FCC  = 1.34466...  (Joyce 1994)
 *   W_M18  = ??           (not in standard references; needed for phenomenal/noumenal bridge)
 *
 * Method: direct quadrature over [0, π]³ by symmetry (reduces domain to 1/8),
 * using Simpson-like rule on an N×N×N grid, excluding k=0 via a small offset.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdio>
#include <cstdlib>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {

inline double p_SC(double k1, double k2, double k3) {
    return (std::cos(k1) + std::cos(k2) + std::cos(k3)) / 3.0;
}

inline double p_BCC(double k1, double k2, double k3) {
    return std::cos(k1) * std::cos(k2) * std::cos(k3);
}

inline double p_FCC(double k1, double k2, double k3) {
    const double c1 = std::cos(k1), c2 = std::cos(k2), c3 = std::cos(k3);
    return (c1 * c2 + c2 * c3 + c1 * c3) / 3.0;
}

inline double p_Moore18(double k1, double k2, double k3) {
    return 0.5 * (p_SC(k1, k2, k3) + p_FCC(k1, k2, k3));
}

// Numerical integration of 1/(1 - p(k)) over BZ [−π, π]³, using cubic symmetry
// to reduce to [0, π]³ and Simpson-like rule. Excludes k=0 region where the
// integrand diverges as 1/k², handling the IR piece analytically.
double watson_integral(double (*p_fn)(double, double, double), double c_L, int N) {
    // IR analytic piece: ∫_{|k|<k_cut} d³k/(2π)³ · 1/(c_L k²)
    // In spherical coords: (4π/(2π)³) · (1/c_L) · ∫_0^k_cut k²/(k²) dk = (4π/(2π)³) · k_cut/c_L
    //                    = k_cut / (2π² c_L)
    // Use k_cut = π/N (cell-wide cutoff at lattice resolution).
    const double k_cut = M_PI / N;
    const double W_ir = k_cut / (2.0 * M_PI * M_PI * c_L);

    // Bulk piece: |k_i| > k_cut in at least one direction.
    // We integrate over the cube [0, π]³ and subtract the IR box [0, k_cut]³.
    const double dk = M_PI / N;
    double W_bulk = 0.0;
    for (int i = 0; i <= N; ++i)
    for (int j = 0; j <= N; ++j)
    for (int l = 0; l <= N; ++l) {
        const double k1 = i * dk;
        const double k2 = j * dk;
        const double k3 = l * dk;
        // Skip the single point at origin (IR handled analytically)
        if (i == 0 && j == 0 && l == 0) continue;
        const double p = p_fn(k1, k2, k3);
        const double denom = 1.0 - p;
        if (denom <= 1e-14) continue;  // skip singular points (shouldn't happen past origin)
        const double f = 1.0 / denom;
        // Simpson weights: 1 at corners, 2/3 at face, 1/2 at edge... use trapezoidal for simplicity.
        // Trapezoidal 3D weight: 1/8 at corners, 1/4 at face edges, 1/2 at face centers, 1 inside.
        int face_count = (i == 0 || i == N ? 1 : 0)
                       + (j == 0 || j == N ? 1 : 0)
                       + (l == 0 || l == N ? 1 : 0);
        const double w = std::pow(0.5, face_count);
        W_bulk += w * f;
    }
    // Scale by the trapezoidal element and symmetry.
    // Element volume = (dk)³ per cube in [0,π]³.
    // Symmetry: multiply by 8 to cover full BZ [−π,π]³.
    // Divide by (2π)³ for the overall 1/(2π)³ factor.
    W_bulk *= std::pow(dk, 3);
    W_bulk *= 8.0;  // cubic symmetry
    W_bulk /= std::pow(2.0 * M_PI, 3);

    return W_bulk + W_ir;
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("================================================================\n");
    std::printf("  Numerical Watson integrals for phenomenal/noumenal bridge\n");
    std::printf("================================================================\n\n");

    const int N = 200;  // grid resolution per axis; N=200 gives ~8e6 sample points

    std::printf("  Method: trapezoidal quadrature on N=%d grid in [0,π]³\n", N);
    std::printf("          + analytic IR piece for |k| < π/N\n\n");

    const double W_SC   = watson_integral(p_SC,   1.0/6.0, N);
    const double W_BCC  = watson_integral(p_BCC,  1.0/2.0, N);
    const double W_FCC  = watson_integral(p_FCC,  1.0/3.0, N);
    const double W_M18  = watson_integral(p_Moore18, 1.0/4.0, N);

    // Reference values (exact / high-precision published)
    const double W_SC_ref  = 1.51639;
    const double W_BCC_ref = 1.39320;
    const double W_FCC_ref = 1.34466;

    std::printf("  W_SC   = %.5f    (ref: %.5f, err %.3f%%)\n",
                W_SC, W_SC_ref, 100.0 * std::abs(W_SC - W_SC_ref)/W_SC_ref);
    std::printf("  W_BCC  = %.5f    (ref: %.5f, err %.3f%%)\n",
                W_BCC, W_BCC_ref, 100.0 * std::abs(W_BCC - W_BCC_ref)/W_BCC_ref);
    std::printf("  W_FCC  = %.5f    (ref: %.5f, err %.3f%%)\n",
                W_FCC, W_FCC_ref, 100.0 * std::abs(W_FCC - W_FCC_ref)/W_FCC_ref);
    std::printf("  W_M18  = %.5f    (no published reference — THIS IS THE NEW CALCULATION)\n",
                W_M18);

    std::printf("\n--- Bridge factor candidates ---\n");
    std::printf("  W_M18 / W_BCC       = %.5f   (my phenomenal/noumenal bridge hypothesis)\n",
                W_M18 / W_BCC);
    std::printf("  W_M18 / W_FCC       = %.5f\n", W_M18 / W_FCC);
    std::printf("  W_M18 / W_SC        = %.5f\n", W_M18 / W_SC);

    std::printf("\n--- Target values for comparison ---\n");
    std::printf("  27/8                = 3.37500   (block volume ratio)\n");
    std::printf("  2π/√3               = 3.62760   (BZ/wave-speed)\n");
    std::printf("  Phase-F measured    = 3.60      (engine α_∞/α_ref, category error per audit)\n");
    std::printf("  26/18               = 1.44444   (shell volume ratio)\n");
    std::printf("  (G*/ϖ)³ = 8/π^(3/2) = 1.43716   (cube of Gaussian normalization)\n");
    std::printf("  G*/ϖ = 2/√π         = 1.12838   (linear Gaussian normalization)\n");

    std::printf("\n--- Verdict ---\n");
    const double ratio_m18_bcc = W_M18 / W_BCC;
    if (ratio_m18_bcc > 3.0) {
        std::printf("  W_M18/W_BCC ≈ %.2f — consistent with block/shell volume hypothesis\n",
                    ratio_m18_bcc);
    } else if (ratio_m18_bcc > 1.3 && ratio_m18_bcc < 1.6) {
        std::printf("  W_M18/W_BCC ≈ %.2f — consistent with shell volume (26/18) or (2/√π)³\n",
                    ratio_m18_bcc);
    } else if (ratio_m18_bcc > 0.9 && ratio_m18_bcc < 1.2) {
        std::printf("  W_M18/W_BCC ≈ %.2f — consistent with bulk-Watson-insensitivity:\n",
                    ratio_m18_bcc);
        std::printf("    the Green's-function at origin is mostly stencil-independent,\n");
        std::printf("    so the bridge factor is NOT a Watson-integral ratio.\n");
    } else {
        std::printf("  W_M18/W_BCC = %.3f — unexpected value; investigate.\n",
                    ratio_m18_bcc);
    }

    std::printf("\n================================================================\n");
    std::printf("  Bridge calculation complete. See stdout above for numerical result.\n");
    std::printf("================================================================\n");
    return 0;
}
