/**
 * @file benchmark_alpha_window_lean_gpu.cu
 * @brief Lean fixed-window Coulomb geometry benchmark.
 *
 * Computes the finite periodic lattice Green's-function normalization
 *
 *   alpha_G(r, L) = 2 r G_L(r)
 *
 * for fixed r={5,7,9} directly on the GPU, without constructing the full
 * RenderBridge/GpuEngine state. This is a geometric certification benchmark:
 * it tests the unit-flux Coulomb baseline and its L -> infinity approach to
 * 1/(2*pi). It is not a derivation of physical alpha=1/137.
 */

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CUDA_CHECK(call) do { \
    cudaError_t err__ = (call); \
    if (err__ != cudaSuccess) { \
        std::fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                     __FILE__, __LINE__, cudaGetErrorString(err__)); \
        std::exit(1); \
    } \
} while (0)

namespace {

constexpr int kNumR = 3;
constexpr int kThreads = 256;

struct Result {
    int L = 0;
    int radii[kNumR] = {};
    double alpha[kNumR] = {};
    double mean = 0.0;
    double wall_ms = 0.0;
};

__global__ void green_window_kernel(long long total_modes,
                                    int L,
                                    int r0,
                                    int r1,
                                    int r2,
                                    double* partial0,
                                    double* partial1,
                                    double* partial2) {
    __shared__ double s0[kThreads];
    __shared__ double s1[kThreads];
    __shared__ double s2[kThreads];

    const int tid = threadIdx.x;
    const long long stride = static_cast<long long>(blockDim.x) * gridDim.x;
    long long idx = static_cast<long long>(blockIdx.x) * blockDim.x + tid;

    const double twopi_over_L = 2.0 * M_PI / static_cast<double>(L);
    double sum0 = 0.0;
    double sum1 = 0.0;
    double sum2 = 0.0;

    for (; idx < total_modes; idx += stride) {
        const int kz = static_cast<int>(idx % L);
        const long long q = idx / L;
        const int ky = static_cast<int>(q % L);
        const int kx = static_cast<int>(q / L);

        if (kx == 0 && ky == 0 && kz == 0) continue;

        const double sx = sin(0.5 * twopi_over_L * kx);
        const double sy = sin(0.5 * twopi_over_L * ky);
        const double sz = sin(0.5 * twopi_over_L * kz);
        const double lambda = 4.0 * (sx * sx + sy * sy + sz * sz);
        const double inv_lambda = 1.0 / lambda;
        const double phase_base = twopi_over_L * kx;

        sum0 += cos(phase_base * static_cast<double>(r0)) * inv_lambda;
        sum1 += cos(phase_base * static_cast<double>(r1)) * inv_lambda;
        sum2 += cos(phase_base * static_cast<double>(r2)) * inv_lambda;
    }

    s0[tid] = sum0;
    s1[tid] = sum1;
    s2[tid] = sum2;
    __syncthreads();

    for (int offset = blockDim.x / 2; offset > 0; offset >>= 1) {
        if (tid < offset) {
            s0[tid] += s0[tid + offset];
            s1[tid] += s1[tid + offset];
            s2[tid] += s2[tid + offset];
        }
        __syncthreads();
    }

    if (tid == 0) {
        partial0[blockIdx.x] = s0[0];
        partial1[blockIdx.x] = s1[0];
        partial2[blockIdx.x] = s2[0];
    }
}

std::vector<int> parse_lattice_sizes(int argc, char** argv,
                                     bool& sqrt_window,
                                     int& fraction_denominator,
                                     double& power_exponent) {
    std::vector<int> out;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::printf("Usage: %s [--sqrt-window|--fraction-window N|--power-window P] [L...]\n", argv[0]);
            std::printf("Default L ladder: 256 512 768 1024 1152\n");
            std::printf("Default window: fixed r={5,7,9}\n");
            std::printf("--sqrt-window: use r near sqrt(L), so r->infinity and r/L->0\n");
            std::printf("--fraction-window N: use r near L/N, e.g. N=32\n");
            std::printf("--power-window P: use r near L^P, with 0<P<1\n");
            std::exit(0);
        }
        if (arg == "--sqrt-window") {
            sqrt_window = true;
            continue;
        }
        if (arg == "--fraction-window" && i + 1 < argc) {
            fraction_denominator = std::atoi(argv[++i]);
            sqrt_window = false;
            power_exponent = 0.0;
            continue;
        }
        if (arg == "--power-window" && i + 1 < argc) {
            power_exponent = std::atof(argv[++i]);
            sqrt_window = false;
            fraction_denominator = 0;
            continue;
        }
        const int L = std::atoi(arg.c_str());
        if (L > 0) out.push_back(L);
    }
    if (out.empty()) out = {256, 512, 768, 1024, 1152};
    return out;
}

int nearest_odd(int value) {
    if (value < 5) return 5;
    return (value % 2 == 0) ? value + 1 : value;
}

void choose_radii(int L, bool sqrt_window, int fraction_denominator,
                  double power_exponent,
                  int r[kNumR]) {
    if (!sqrt_window) {
        if (fraction_denominator > 0) {
            const int center = nearest_odd(std::max(5, L / fraction_denominator));
            r[0] = std::max(5, center - 2);
            r[1] = center;
            r[2] = center + 2;
            return;
        }
        if (power_exponent > 0.0 && power_exponent < 1.0) {
            const int center = nearest_odd(static_cast<int>(std::round(
                std::pow(static_cast<double>(L), power_exponent))));
            r[0] = std::max(5, center - 2);
            r[1] = center;
            r[2] = center + 2;
            return;
        }
        r[0] = 5;
        r[1] = 7;
        r[2] = 9;
        return;
    }
    const int center = nearest_odd(static_cast<int>(std::round(std::sqrt(static_cast<double>(L)))));
    r[0] = std::max(5, center - 2);
    r[1] = center;
    r[2] = center + 2;
}

Result run_L(int L, const int radii[kNumR]) {
    Result out;
    out.L = L;
    for (int i = 0; i < kNumR; ++i) {
        out.radii[i] = radii[i];
    }

    const long long total_modes = static_cast<long long>(L) * L * L;
    const int max_blocks = 65535;
    const int blocks = std::min<long long>(
        max_blocks, (total_modes + kThreads - 1) / kThreads);

    double* d_partial0 = nullptr;
    double* d_partial1 = nullptr;
    double* d_partial2 = nullptr;
    CUDA_CHECK(cudaMalloc(&d_partial0, static_cast<size_t>(blocks) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_partial1, static_cast<size_t>(blocks) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_partial2, static_cast<size_t>(blocks) * sizeof(double)));

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));

    green_window_kernel<<<blocks, kThreads>>>(total_modes, L,
                                              radii[0], radii[1], radii[2],
                                              d_partial0, d_partial1, d_partial2);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    out.wall_ms = static_cast<double>(ms);

    std::vector<double> h0(blocks), h1(blocks), h2(blocks);
    CUDA_CHECK(cudaMemcpy(h0.data(), d_partial0, h0.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h1.data(), d_partial1, h1.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h2.data(), d_partial2, h2.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    CUDA_CHECK(cudaFree(d_partial0));
    CUDA_CHECK(cudaFree(d_partial1));
    CUDA_CHECK(cudaFree(d_partial2));

    double sums[kNumR] = {};
    for (int i = 0; i < blocks; ++i) {
        sums[0] += h0[i];
        sums[1] += h1[i];
        sums[2] += h2[i];
    }

    const double inv_volume = 1.0 / static_cast<double>(total_modes);
    for (int i = 0; i < kNumR; ++i) {
        const double G = sums[i] * inv_volume;
        out.alpha[i] = 2.0 * radii[i] * G;
        out.mean += out.alpha[i];
    }
    out.mean /= static_cast<double>(kNumR);
    return out;
}

void print_extrapolation(const std::vector<Result>& results, int min_L,
                         const char* label, double alpha_geom) {
    std::vector<Result> tail;
    for (const auto& r : results) {
        if (r.L >= min_L) tail.push_back(r);
    }
    if (tail.size() < 3) return;

    const int n = static_cast<int>(tail.size());
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    for (const auto& r : tail) {
        const double x = 1.0 / static_cast<double>(r.L);
        const double y = r.mean;
        sx += x;
        sy += y;
        sxx += x * x;
        sxy += x * y;
    }
    const double denom = n * sxx - sx * sx;
    if (std::abs(denom) <= 1e-30) return;
    const double slope = (n * sxy - sx * sy) / denom;
    const double alpha_inf = (sy - slope * sx) / n;
    const double err = 100.0 * std::abs(alpha_inf - alpha_geom) / alpha_geom;
    const double L_cross = slope / (alpha_geom - alpha_inf);

    std::printf("  %-16s alpha_inf=%.10f slope=%+.6f err=%.5f%%",
                label, alpha_inf, slope, err);
    if (L_cross > 0.0 && std::isfinite(L_cross)) {
        std::printf(" L_cross=%.1f", L_cross);
    }
    std::printf("\n");
}

bool solve_3x3(double a[3][4], double out[3]) {
    for (int col = 0; col < 3; ++col) {
        int pivot = col;
        double best = std::abs(a[col][col]);
        for (int row = col + 1; row < 3; ++row) {
            const double candidate = std::abs(a[row][col]);
            if (candidate > best) {
                best = candidate;
                pivot = row;
            }
        }
        if (best <= 1e-30) return false;
        if (pivot != col) {
            for (int j = col; j < 4; ++j) {
                std::swap(a[col][j], a[pivot][j]);
            }
        }

        const double inv_pivot = 1.0 / a[col][col];
        for (int j = col; j < 4; ++j) {
            a[col][j] *= inv_pivot;
        }
        for (int row = 0; row < 3; ++row) {
            if (row == col) continue;
            const double factor = a[row][col];
            for (int j = col; j < 4; ++j) {
                a[row][j] -= factor * a[col][j];
            }
        }
    }

    for (int i = 0; i < 3; ++i) {
        out[i] = a[i][3];
    }
    return true;
}

void print_two_scale_fit(const std::vector<Result>& results,
                         double alpha_geom,
                         double power_exponent) {
    if (results.size() < 3) {
        std::printf("  Need at least three L points for the two-scale fit.\n");
        return;
    }

    double normal[3][4] = {};
    for (const auto& row : results) {
        const double r_center = static_cast<double>(row.radii[1]);
        const double x[3] = {
            1.0,
            1.0 / (r_center * r_center),
            r_center / static_cast<double>(row.L),
        };
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                normal[i][j] += x[i] * x[j];
            }
            normal[i][3] += x[i] * row.mean;
        }
    }

    double coeff[3] = {};
    if (!solve_3x3(normal, coeff)) return;

    const double alpha_inf = coeff[0];
    const double err = 100.0 * std::abs(alpha_inf - alpha_geom) / alpha_geom;
    std::printf("  alpha ~= A + B/r^2 + C*(r/L)\n");
    std::printf("  two-scale fit   A=%.10f B=%+.6f C=%+.6f err(A)=%.5f%%\n",
                alpha_inf, coeff[1], coeff[2], err);

    if (power_exponent > 0.0 && power_exponent < 1.0) {
        for (int L_pred : {12288, 16384, 24576}) {
            const double r_pred = std::pow(static_cast<double>(L_pred), power_exponent);
            const double y = alpha_inf
                           + coeff[1] / (r_pred * r_pred)
                           + coeff[2] * r_pred / static_cast<double>(L_pred);
            const double y_err = 100.0 * std::abs(y - alpha_geom) / alpha_geom;
            std::printf("  model L=%-6d r~%-7.2f alpha=%.10f err=%.5f%%\n",
                        L_pred, r_pred, y, y_err);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    const double alpha_geom = 1.0 / (2.0 * M_PI);
    bool sqrt_window = false;
    int fraction_denominator = 0;
    double power_exponent = 0.0;
    const auto Ls = parse_lattice_sizes(argc, argv, sqrt_window,
                                        fraction_denominator,
                                        power_exponent);

    cudaDeviceProp prop {};
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));
    std::printf("================================================================\n");
    std::printf("  Lean GPU fixed-window Coulomb geometry benchmark\n");
    std::printf("  GPU: %s\n", prop.name);
    std::printf("  alpha_geom = 1/(2*pi) = %.10f\n", alpha_geom);
    if (fraction_denominator > 0) {
        std::printf("  window mode: L/%d\n", fraction_denominator);
    } else if (power_exponent > 0.0) {
        std::printf("  window mode: L^%.3f\n", power_exponent);
    } else {
        std::printf("  window mode: %s\n", sqrt_window ? "sqrt(L)" : "fixed");
    }
    std::printf("================================================================\n\n");

    std::printf("%-6s  %11s  %12s  %12s  %12s  %12s  %10s  %10s\n",
                "L", "r", "alpha_r0", "alpha_r1", "alpha_r2",
                "mean", "err(%)", "kernel_ms");
    std::printf("------  -----------  ------------  ------------  ------------  ------------  ----------  ----------\n");

    std::vector<Result> results;
    for (int L : Ls) {
        int radii[kNumR] = {};
        choose_radii(L, sqrt_window, fraction_denominator, power_exponent, radii);
        Result result = run_L(L, radii);
        results.push_back(result);
        const double err = 100.0 * std::abs(result.mean - alpha_geom) / alpha_geom;
        std::printf("%-6d  {%3d,%3d,%3d}  %12.8f  %12.8f  %12.8f  %12.8f  %10.5f  %10.2f\n",
                    L, radii[0], radii[1], radii[2],
                    result.alpha[0], result.alpha[1], result.alpha[2],
                    result.mean, err, result.wall_ms);
    }

    std::printf("\n================================================================\n");
    std::printf("  1/L extrapolation of fixed-window mean\n");
    std::printf("================================================================\n");
    print_extrapolation(results, 0, "all", alpha_geom);
    print_extrapolation(results, 512, "tail>=512", alpha_geom);
    print_extrapolation(results, 768, "tail>=768", alpha_geom);
    std::printf("\n================================================================\n");
    std::printf("  Two-scale fit for valid growing-window interpretation\n");
    std::printf("================================================================\n");
    print_two_scale_fit(results, alpha_geom, power_exponent);

    std::printf("\n================================================================\n");
    std::printf("  Done. Geometric benchmark only; physical alpha is out of scope.\n");
    std::printf("================================================================\n");
    return 0;
}
