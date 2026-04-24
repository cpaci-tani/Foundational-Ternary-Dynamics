/**
 * @file benchmark_alpha_relaxation_lean_gpu.cu
 * @brief Lean dynamical Coulomb benchmark using GPU Poisson relaxation.
 *
 * This is the bridge between the full RenderBridge Coulomb benchmark and the
 * direct spectral Green's-function benchmark. It keeps the dynamical ingredient
 * -- iterative field relaxation -- but removes RenderBridge, voxel staging,
 * substrates, Langevin buffers, and force/movement state.
 *
 * Epistemic status: [MEASUREMENT]. This benchmark certifies that a relaxed
 * lattice field reproduces the same unit-flux Green's geometry. It is not a
 * derivation of physical alpha=1/137.
 */

#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

struct Options {
    std::vector<int> Ls = {64};
    int iters = 4000;
    int check_every = 500;
    double omega = 1.92;
    bool sqrt_window = false;
    double power_exponent = 0.0;
};

struct SolveResult {
    double energy = 0.0;
    double max_delta = 0.0;
    double wall_ms = 0.0;
};

struct LResult {
    int L = 0;
    int radii[kNumR] = {};
    double self_energy = 0.0;
    double alpha_dyn[kNumR] = {};
    double alpha_ref[kNumR] = {};
    double mean_dyn = 0.0;
    double mean_ref = 0.0;
    double max_delta = 0.0;
    double wall_ms = 0.0;
};

__global__ void init_source_kernel(long long total,
                                   int L,
                                   int mode,
                                   int r,
                                   double* rho,
                                   double* phi) {
    const int mid = L / 2;
    const long long center = (static_cast<long long>(mid) * L + mid) * L + mid;
    const long long partner = (static_cast<long long>(mid + r) * L + mid) * L + mid;
    const long long stride = static_cast<long long>(blockDim.x) * gridDim.x;

    for (long long idx = static_cast<long long>(blockIdx.x) * blockDim.x + threadIdx.x;
         idx < total;
         idx += stride) {
        phi[idx] = 0.0;
        rho[idx] = 0.0;

        if (mode == 0) {
            rho[idx] = -1.0 / static_cast<double>(total);
            if (idx == center) rho[idx] += 1.0;
        } else {
            if (idx == center) rho[idx] = 1.0;
            if (idx == partner) rho[idx] = -1.0;
        }
    }
}

__global__ void rb_sor_kernel(long long total,
                              int L,
                              int color,
                              double omega,
                              const double* rho,
                              double* phi,
                              double* partial_delta) {
    __shared__ double smax[kThreads];

    const int tid = threadIdx.x;
    const long long stride = static_cast<long long>(blockDim.x) * gridDim.x;
    double local_max = 0.0;

    for (long long idx = static_cast<long long>(blockIdx.x) * blockDim.x + tid;
         idx < total;
         idx += stride) {
        const int z = static_cast<int>(idx % L);
        const long long q = idx / L;
        const int y = static_cast<int>(q % L);
        const int x = static_cast<int>(q / L);

        if (((x + y + z) & 1) == color) {
            const int xp = (x + 1 == L) ? 0 : x + 1;
            const int xm = (x == 0) ? L - 1 : x - 1;
            const int yp = (y + 1 == L) ? 0 : y + 1;
            const int ym = (y == 0) ? L - 1 : y - 1;
            const int zp = (z + 1 == L) ? 0 : z + 1;
            const int zm = (z == 0) ? L - 1 : z - 1;

            const double sum_neighbors =
                phi[(static_cast<long long>(xp) * L + y) * L + z] +
                phi[(static_cast<long long>(xm) * L + y) * L + z] +
                phi[static_cast<long long>(x) * L * L + static_cast<long long>(yp) * L + z] +
                phi[static_cast<long long>(x) * L * L + static_cast<long long>(ym) * L + z] +
                phi[static_cast<long long>(x) * L * L + static_cast<long long>(y) * L + zp] +
                phi[static_cast<long long>(x) * L * L + static_cast<long long>(y) * L + zm];

            const double old_phi = phi[idx];
            const double relaxed = (sum_neighbors + rho[idx]) / 6.0;
            const double new_phi = old_phi + omega * (relaxed - old_phi);
            phi[idx] = new_phi;
            local_max = fmax(local_max, fabs(new_phi - old_phi));
        }
    }

    smax[tid] = local_max;
    __syncthreads();
    for (int offset = blockDim.x / 2; offset > 0; offset >>= 1) {
        if (tid < offset) {
            smax[tid] = fmax(smax[tid], smax[tid + offset]);
        }
        __syncthreads();
    }
    if (tid == 0) partial_delta[blockIdx.x] = smax[0];
}

__global__ void energy_kernel(long long total,
                              const double* rho,
                              const double* phi,
                              double* partial) {
    __shared__ double ssum[kThreads];

    const int tid = threadIdx.x;
    const long long stride = static_cast<long long>(blockDim.x) * gridDim.x;
    long long idx = static_cast<long long>(blockIdx.x) * blockDim.x + tid;
    double sum = 0.0;

    for (; idx < total; idx += stride) {
        sum += rho[idx] * phi[idx];
    }

    ssum[tid] = sum;
    __syncthreads();
    for (int offset = blockDim.x / 2; offset > 0; offset >>= 1) {
        if (tid < offset) ssum[tid] += ssum[tid + offset];
        __syncthreads();
    }
    if (tid == 0) partial[blockIdx.x] = ssum[0];
}

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

int nearest_odd(int value) {
    if (value < 5) return 5;
    return (value % 2 == 0) ? value + 1 : value;
}

void choose_radii(int L, const Options& opts, int r[kNumR]) {
    if (opts.power_exponent > 0.0 && opts.power_exponent < 1.0) {
        const int center = nearest_odd(static_cast<int>(
            std::round(std::pow(static_cast<double>(L), opts.power_exponent))));
        r[0] = std::max(5, center - 2);
        r[1] = center;
        r[2] = center + 2;
        return;
    }
    if (opts.sqrt_window) {
        const int center = nearest_odd(static_cast<int>(
            std::round(std::sqrt(static_cast<double>(L)))));
        r[0] = std::max(5, center - 2);
        r[1] = center;
        r[2] = center + 2;
        return;
    }
    r[0] = 5;
    r[1] = 7;
    r[2] = 9;
}

Options parse_options(int argc, char** argv) {
    Options opts;
    opts.Ls.clear();
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            std::printf("Usage: %s [--iters N] [--omega W] [--check-every N] "
                        "[--sqrt-window|--power-window P] [L...]\n", argv[0]);
            std::printf("Default: L=64, fixed r={5,7,9}, iters=4000, omega=1.92\n");
            std::exit(0);
        }
        if (arg == "--iters" && i + 1 < argc) {
            opts.iters = std::atoi(argv[++i]);
            continue;
        }
        if (arg == "--omega" && i + 1 < argc) {
            opts.omega = std::atof(argv[++i]);
            continue;
        }
        if (arg == "--check-every" && i + 1 < argc) {
            opts.check_every = std::max(1, std::atoi(argv[++i]));
            continue;
        }
        if (arg == "--sqrt-window") {
            opts.sqrt_window = true;
            opts.power_exponent = 0.0;
            continue;
        }
        if (arg == "--power-window" && i + 1 < argc) {
            opts.power_exponent = std::atof(argv[++i]);
            opts.sqrt_window = false;
            continue;
        }
        const int L = std::atoi(arg.c_str());
        if (L > 0) opts.Ls.push_back(L);
    }
    if (opts.Ls.empty()) opts.Ls = {64};
    return opts;
}

double host_sum(const std::vector<double>& values) {
    double sum = 0.0;
    for (double v : values) sum += v;
    return sum;
}

double host_max(const std::vector<double>& values) {
    double out = 0.0;
    for (double v : values) out = std::max(out, v);
    return out;
}

SolveResult solve_energy(int L,
                         int mode,
                         int r,
                         const Options& opts,
                         double* d_rho,
                         double* d_phi,
                         double* d_partial,
                         double* d_delta,
                         int blocks,
                         int reduce_blocks) {
    const long long total = static_cast<long long>(L) * L * L;

    init_source_kernel<<<blocks, kThreads>>>(total, L, mode, r, d_rho, d_phi);
    CUDA_CHECK(cudaGetLastError());
    CUDA_CHECK(cudaDeviceSynchronize());

    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));
    CUDA_CHECK(cudaEventRecord(start));

    std::vector<double> h_delta(static_cast<size_t>(2) * blocks);
    double max_delta = 0.0;
    for (int iter = 1; iter <= opts.iters; ++iter) {
        rb_sor_kernel<<<blocks, kThreads>>>(total, L, 0, opts.omega,
                                            d_rho, d_phi, d_delta);
        rb_sor_kernel<<<blocks, kThreads>>>(total, L, 1, opts.omega,
                                            d_rho, d_phi, d_delta + blocks);
        CUDA_CHECK(cudaGetLastError());

        if (iter == opts.iters || (iter % opts.check_every) == 0) {
            CUDA_CHECK(cudaMemcpy(h_delta.data(), d_delta,
                                  h_delta.size() * sizeof(double),
                                  cudaMemcpyDeviceToHost));
            max_delta = host_max(h_delta);
        }
    }
    CUDA_CHECK(cudaDeviceSynchronize());

    energy_kernel<<<reduce_blocks, kThreads>>>(total, d_rho, d_phi, d_partial);
    CUDA_CHECK(cudaGetLastError());
    std::vector<double> h_partial(reduce_blocks);
    CUDA_CHECK(cudaMemcpy(h_partial.data(), d_partial,
                          h_partial.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));

    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));
    float ms = 0.0f;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));

    SolveResult out;
    out.energy = host_sum(h_partial);
    out.max_delta = max_delta;
    out.wall_ms = static_cast<double>(ms);
    return out;
}

void compute_spectral_alpha(int L, const int radii[kNumR], double alpha[kNumR]) {
    const long long total = static_cast<long long>(L) * L * L;
    const int blocks = std::min<long long>(65535, (total + kThreads - 1) / kThreads);

    double* d0 = nullptr;
    double* d1 = nullptr;
    double* d2 = nullptr;
    CUDA_CHECK(cudaMalloc(&d0, static_cast<size_t>(blocks) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d1, static_cast<size_t>(blocks) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d2, static_cast<size_t>(blocks) * sizeof(double)));

    green_window_kernel<<<blocks, kThreads>>>(total, L,
                                              radii[0], radii[1], radii[2],
                                              d0, d1, d2);
    CUDA_CHECK(cudaGetLastError());

    std::vector<double> h0(blocks), h1(blocks), h2(blocks);
    CUDA_CHECK(cudaMemcpy(h0.data(), d0, h0.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h1.data(), d1, h1.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h2.data(), d2, h2.size() * sizeof(double),
                          cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d0));
    CUDA_CHECK(cudaFree(d1));
    CUDA_CHECK(cudaFree(d2));

    const double inv_volume = 1.0 / static_cast<double>(total);
    const double sums[kNumR] = {host_sum(h0), host_sum(h1), host_sum(h2)};
    for (int i = 0; i < kNumR; ++i) {
        alpha[i] = 2.0 * static_cast<double>(radii[i]) * sums[i] * inv_volume;
    }
}

LResult run_L(int L, const Options& opts) {
    if ((L % 2) != 0) {
        std::fprintf(stderr, "L must be even for periodic red-black SOR; got L=%d\n", L);
        std::exit(1);
    }

    LResult out;
    out.L = L;
    choose_radii(L, opts, out.radii);

    const long long total = static_cast<long long>(L) * L * L;
    const int blocks = std::min<long long>(65535, (total + kThreads - 1) / kThreads);
    const int reduce_blocks = blocks;

    double* d_rho = nullptr;
    double* d_phi = nullptr;
    double* d_partial = nullptr;
    double* d_delta = nullptr;
    CUDA_CHECK(cudaMalloc(&d_rho, static_cast<size_t>(total) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_phi, static_cast<size_t>(total) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_partial, static_cast<size_t>(reduce_blocks) * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_delta, static_cast<size_t>(2) * blocks * sizeof(double)));

    compute_spectral_alpha(L, out.radii, out.alpha_ref);
    for (int i = 0; i < kNumR; ++i) out.mean_ref += out.alpha_ref[i];
    out.mean_ref /= static_cast<double>(kNumR);

    const SolveResult self = solve_energy(L, 0, 0, opts, d_rho, d_phi,
                                          d_partial, d_delta,
                                          blocks, reduce_blocks);
    out.self_energy = self.energy;
    out.max_delta = std::max(out.max_delta, self.max_delta);
    out.wall_ms += self.wall_ms;

    for (int i = 0; i < kNumR; ++i) {
        const SolveResult pair = solve_energy(L, 1, out.radii[i], opts,
                                              d_rho, d_phi, d_partial, d_delta,
                                              blocks, reduce_blocks);
        const double V = pair.energy - 2.0 * out.self_energy;
        out.alpha_dyn[i] = -V * static_cast<double>(out.radii[i]);
        out.mean_dyn += out.alpha_dyn[i];
        out.max_delta = std::max(out.max_delta, pair.max_delta);
        out.wall_ms += pair.wall_ms;
    }
    out.mean_dyn /= static_cast<double>(kNumR);

    CUDA_CHECK(cudaFree(d_rho));
    CUDA_CHECK(cudaFree(d_phi));
    CUDA_CHECK(cudaFree(d_partial));
    CUDA_CHECK(cudaFree(d_delta));
    return out;
}

}  // namespace

int main(int argc, char** argv) {
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    const Options opts = parse_options(argc, argv);
    const double alpha_geom = 1.0 / (2.0 * M_PI);

    cudaDeviceProp prop {};
    CUDA_CHECK(cudaGetDeviceProperties(&prop, 0));

    std::printf("================================================================\n");
    std::printf("  Lean GPU dynamical Coulomb relaxation benchmark\n");
    std::printf("  GPU: %s\n", prop.name);
    std::printf("  alpha_geom = 1/(2*pi) = %.10f\n", alpha_geom);
    std::printf("  iters=%d omega=%.4f check_every=%d\n",
                opts.iters, opts.omega, opts.check_every);
    if (opts.power_exponent > 0.0) {
        std::printf("  window mode: L^%.3f\n", opts.power_exponent);
    } else {
        std::printf("  window mode: %s\n", opts.sqrt_window ? "sqrt(L)" : "fixed");
    }
    std::printf("================================================================\n\n");

    std::printf("%-6s  %11s  %12s  %12s  %10s  %10s  %10s\n",
                "L", "r", "dyn_mean", "ref_mean", "err_ref%", "err_geom%", "wall_s");
    std::printf("------  -----------  ------------  ------------  ----------  ----------  ----------\n");

    for (int L : opts.Ls) {
        LResult row = run_L(L, opts);
        const double err_ref = 100.0 * std::abs(row.mean_dyn - row.mean_ref)
                             / std::abs(row.mean_ref);
        const double err_geom = 100.0 * std::abs(row.mean_dyn - alpha_geom)
                              / alpha_geom;
        std::printf("%-6d  {%3d,%3d,%3d}  %12.8f  %12.8f  %10.5f  %10.5f  %10.2f\n",
                    row.L, row.radii[0], row.radii[1], row.radii[2],
                    row.mean_dyn, row.mean_ref, err_ref, err_geom,
                    row.wall_ms / 1000.0);
        std::printf("  self_energy=%.10f max_delta=%.3e\n",
                    row.self_energy, row.max_delta);
        for (int i = 0; i < kNumR; ++i) {
            const double rerr = 100.0 * std::abs(row.alpha_dyn[i] - row.alpha_ref[i])
                              / std::abs(row.alpha_ref[i]);
            std::printf("  r=%-3d dyn_alpha=%.8f ref_alpha=%.8f err_ref=%8.5f%%\n",
                        row.radii[i], row.alpha_dyn[i], row.alpha_ref[i], rerr);
        }
    }

    std::printf("\n================================================================\n");
    std::printf("  Done. Dynamical relaxation bridge; physical alpha is out of scope.\n");
    std::printf("================================================================\n");
    return 0;
}
