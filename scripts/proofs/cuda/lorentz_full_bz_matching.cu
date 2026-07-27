// Deterministic CUDA quadrature for the FTD-0419 full-Brillouin-zone match.
//
// This is not a parameter search.  It evaluates the one-loop integrands fixed
// by FTD-0418 on uniform N^4 lattices.  Photon loop momenta are periodic with
// the global zero mode removed; fermion loop momenta are antiperiodic in all
// four directions.  External polarization momenta are one bosonic grid unit,
// so the discrete Ward shift is exact at every N.

#include <cuda_runtime.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double C2 = 1.0 / 7.0;
constexpr double C = 0.3779644730092272272145165362341800608;
constexpr int NOUT = 6;
constexpr int BLOCK = 256;

__device__ inline double coeff(int mu) { return mu == 0 ? 1.0 : C; }
__device__ inline double photon_weight(int mu) { return mu == 0 ? C2 : 1.0; }

__device__ inline void fermion_fields(
    const double p[4], double mass, double& w, double k[4], double& den) {
  w = mass;
  den = 0.0;
  #pragma unroll
  for (int mu = 0; mu < 4; ++mu) {
    const double a = coeff(mu);
    w += a * (1.0 - cos(p[mu]));
    k[mu] = a * sin(p[mu]);
    den += k[mu] * k[mu];
  }
  den += w * w;
}

// tr[(W1-i gamma.K1) V_mu (W2-i gamma.K2) V_nu] / (D1 D2)
// with V_mu=a_mu+i b_mu gamma_mu.  Euclidean tr(1)=4.
__device__ inline double bubble_trace(
    const double p[4], const double q[4], int mu, int nu, double mass) {
  double p2[4];
  double average[4];
  #pragma unroll
  for (int rho = 0; rho < 4; ++rho) {
    p2[rho] = p[rho] + q[rho];
    average[rho] = p[rho] + 0.5 * q[rho];
  }

  double w1, w2, d1, d2, k1[4], k2[4];
  fermion_fields(p, mass, w1, k1, d1);
  fermion_fields(p2, mass, w2, k2, d2);

  const double am = coeff(mu) * sin(average[mu]);
  const double bm = coeff(mu) * cos(average[mu]);
  const double an = coeff(nu) * sin(average[nu]);
  const double bn = coeff(nu) * cos(average[nu]);

  double kdot = 0.0;
  #pragma unroll
  for (int rho = 0; rho < 4; ++rho) kdot += k1[rho] * k2[rho];

  // Vector representatives are v1=-K1, v2=b_mu e_mu,
  // v3=-K2, v4=b_nu e_nu for factors (s+i gamma.v).
  const double d12 = -k1[mu] * bm;
  const double d13 = kdot;
  const double d14 = -k1[nu] * bn;
  const double d23 = -bm * k2[mu];
  const double d24 = (mu == nu) ? bm * bn : 0.0;
  const double d34 = -k2[nu] * bn;

  double tr4 = w1 * am * w2 * an;
  tr4 -= d12 * w2 * an + d13 * am * an + d14 * am * w2
       + d23 * w1 * an + d24 * w1 * w2 + d34 * w1 * am;
  tr4 += d12 * d34 - d13 * d24 + d14 * d23;
  return 4.0 * tr4 / (d1 * d2);
}

__device__ inline double contact_trace(
    const double p[4], int mu, double mass) {
  double w, den, k[4];
  fermion_fields(p, mass, w, k, den);
  return 4.0 * (w * coeff(mu) * cos(p[mu]) - k[mu] * k[mu]) / den;
}

// Derivative at external p=0 of the gamma_ext coefficient of
// 1/2 V^(2)D - V^(1)S V^(1)D, divided by the free coefficient nu_ext.
__device__ inline double self_energy_relative_slope(
    const double loop[4], int ext, bool is_zero, double mass) {
  if (is_zero) return 0.0;  // finite-volume global photon zero-mode removal

  double delta = 4.0 * sin(0.5 * loop[0]) * sin(0.5 * loop[0]);
  #pragma unroll
  for (int i = 1; i < 4; ++i) {
    const double s = sin(0.5 * loop[i]);
    delta += 4.0 * C2 * s * s;
  }

  double w = mass;
  double k[4];
  double den = 0.0;
  #pragma unroll
  for (int rho = 0; rho < 4; ++rho) {
    const double a = coeff(rho);
    w += a * (1.0 - cos(loop[rho]));
    k[rho] = -a * sin(loop[rho]);  // internal fermion momentum is -loop
    den += k[rho] * k[rho];
  }
  den += w * w;

  const double re = coeff(ext);
  const double dw = -re * sin(loop[ext]);
  const double dk = re * cos(loop[ext]);
  const double dden = 2.0 * w * dw + 2.0 * k[ext] * dk;

  double exchange_derivative = 0.0;
  #pragma unroll
  for (int mu = 0; mu < 4; ++mu) {
    const double rm = coeff(mu);
    const double sh = sin(0.5 * loop[mu]);
    const double ch = cos(0.5 * loop[mu]);
    const double a = -rm * sh;
    const double b = rm * ch;
    double numer, dnumer;

    if (mu == ext) {
      const double da = rm * ch;
      const double db = rm * sh;
      numer = (b * b - a * a) * k[ext] + 2.0 * a * b * w;
      dnumer = (2.0 * b * db - 2.0 * a * da) * k[ext]
             + (b * b - a * a) * dk
             + 2.0 * ((da * b + a * db) * w + a * b * dw);
    } else {
      numer = -(a * a + b * b) * k[ext];
      dnumer = -(a * a + b * b) * dk;
    }

    const double derivative = (dnumer * den - numer * dden) / (den * den);
    exchange_derivative += photon_weight(mu) * derivative / delta;
  }

  const double seagull_derivative =
      -0.5 * re * photon_weight(ext) / delta;
  return (seagull_derivative - exchange_derivative) / re;
}

__global__ void integrate_kernel(
    unsigned long long n4, int n, double external_h, int twist_all,
    double bare_mass, double* sums) {
  extern __shared__ double shared[];
  double local[NOUT] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  const unsigned long long stride =
      static_cast<unsigned long long>(blockDim.x) * gridDim.x;
  const double h = 2.0 * PI / static_cast<double>(n);

  for (unsigned long long index =
           static_cast<unsigned long long>(blockIdx.x) * blockDim.x + threadIdx.x;
       index < n4; index += stride) {
    unsigned long long value = index;
    int digit[4];
    #pragma unroll
    for (int mu = 3; mu >= 0; --mu) {
      digit[mu] = static_cast<int>(value % static_cast<unsigned long long>(n));
      value /= static_cast<unsigned long long>(n);
    }

    double loop[4];
    double fermion_p[4];
    bool is_zero = true;
    #pragma unroll
    for (int mu = 0; mu < 4; ++mu) {
      loop[mu] = h * static_cast<double>(digit[mu]);
      const double twist = (twist_all != 0 || mu == 0) ? 0.5 : 0.0;
      fermion_p[mu] = -PI + h * (static_cast<double>(digit[mu]) + twist);
      is_zero = is_zero && (digit[mu] == 0);
    }

    local[0] += self_energy_relative_slope(loop, 0, is_zero, bare_mass);
    local[1] += self_energy_relative_slope(loop, 1, is_zero, bare_mass);

    const double qt[4] = {external_h, 0.0, 0.0, 0.0};
    const double qs[4] = {0.0, 0.0, external_h, 0.0};
    const double contact_transverse = contact_trace(fermion_p, 1, bare_mass);
    local[2] += bubble_trace(fermion_p, qt, 1, 1, bare_mass)
              - contact_transverse;
    local[3] += bubble_trace(fermion_p, qs, 1, 1, bare_mass)
              - contact_transverse;
    local[4] += bubble_trace(fermion_p, qt, 0, 0, bare_mass)
              - contact_trace(fermion_p, 0, bare_mass);
    local[5] += bubble_trace(fermion_p, qs, 2, 2, bare_mass)
              - contact_trace(fermion_p, 2, bare_mass);
  }

  #pragma unroll
  for (int output = 0; output < NOUT; ++output) {
    shared[output * blockDim.x + threadIdx.x] = local[output];
  }
  __syncthreads();

  for (int offset = blockDim.x / 2; offset > 0; offset >>= 1) {
    if (threadIdx.x < offset) {
      #pragma unroll
      for (int output = 0; output < NOUT; ++output) {
        shared[output * blockDim.x + threadIdx.x] +=
            shared[output * blockDim.x + threadIdx.x + offset];
      }
    }
    __syncthreads();
  }

  if (threadIdx.x == 0) {
    #pragma unroll
    for (int output = 0; output < NOUT; ++output) {
      atomicAdd(&sums[output], shared[output * blockDim.x]);
    }
  }
}

void check_cuda(const char* stage) {
  const cudaError_t status = cudaGetLastError();
  if (status != cudaSuccess) {
    std::fprintf(stderr, "CUDA failure at %s: %s\n", stage,
                 cudaGetErrorString(status));
    std::exit(2);
  }
}

}  // namespace

int main(int argc, char** argv) {
  std::vector<int> sizes;
  bool twist_all = true;
  int external_mode = 1;
  int fixed_ratio = 0;
  double bare_mass = 0.0;
  double fixed_external_q = -1.0;
  for (int i = 1; i < argc; ++i) {
    if (std::strcmp(argv[i], "--temporal-ap") == 0) {
      twist_all = false;
    } else if (std::strncmp(argv[i], "--mode=", 7) == 0) {
      external_mode = std::atoi(argv[i] + 7);
    } else if (std::strncmp(argv[i], "--ratio=", 8) == 0) {
      fixed_ratio = std::atoi(argv[i] + 8);
    } else if (std::strncmp(argv[i], "--mass=", 7) == 0) {
      bare_mass = std::strtod(argv[i] + 7, nullptr);
    } else if (std::strncmp(argv[i], "--q=", 4) == 0) {
      fixed_external_q = std::strtod(argv[i] + 4, nullptr);
    } else {
      sizes.push_back(std::atoi(argv[i]));
    }
  }
  if (sizes.empty()) sizes = {16, 24, 32, 48, 64};

  double* device_sums = nullptr;
  cudaMalloc(&device_sums, NOUT * sizeof(double));
  check_cuda("cudaMalloc");

  std::printf("N,Zt,Zs,Zs_minus_Zt,ZE,ZB,ZB_minus_ZE,delta_match,"
              "ward_t,ward_s\n");
  std::fprintf(stderr, "fermion spin structure: %s\n",
               twist_all ? "antiperiodic-all" : "antiperiodic-time-only");
  std::fprintf(stderr, "external bosonic mode: %d\n", external_mode);
  std::fprintf(stderr, "bare fermion mass: %.17g\n", bare_mass);
  if (external_mode < 1 || fixed_ratio < 0 || bare_mass < 0.0
      || fixed_external_q == 0.0) {
    std::fprintf(stderr, "external mode must be >= 1\n");
    return 3;
  }
  for (const int n : sizes) {
    if (n < 4 || (n % 2) != 0) {
      std::fprintf(stderr, "N must be an even integer >= 4 (received %d)\n", n);
      return 3;
    }
    int actual_mode = external_mode;
    if (fixed_ratio > 0) {
      if ((n % fixed_ratio) != 0) {
        std::fprintf(stderr, "N=%d is not divisible by fixed ratio %d\n",
                     n, fixed_ratio);
        return 3;
      }
      actual_mode = n / fixed_ratio;
    }
    const double actual_external_q = fixed_external_q > 0.0
        ? fixed_external_q
        : 2.0 * PI * static_cast<double>(actual_mode) / static_cast<double>(n);
    const unsigned long long n2 =
        static_cast<unsigned long long>(n) * static_cast<unsigned long long>(n);
    const unsigned long long n4 = n2 * n2;
    cudaMemset(device_sums, 0, NOUT * sizeof(double));
    check_cuda("cudaMemset");

    const unsigned long long wanted_blocks = (n4 + BLOCK - 1) / BLOCK;
    const int blocks = static_cast<int>(wanted_blocks < 65535ULL
                                            ? wanted_blocks : 65535ULL);
    integrate_kernel<<<blocks, BLOCK, NOUT * BLOCK * sizeof(double)>>>(
        n4, n, actual_external_q, twist_all ? 1 : 0, bare_mass, device_sums);
    check_cuda("kernel launch");
    cudaDeviceSynchronize();
    check_cuda("kernel synchronize");

    double sums[NOUT];
    cudaMemcpy(sums, device_sums, NOUT * sizeof(double), cudaMemcpyDeviceToHost);
    check_cuda("cudaMemcpy");

    const double inv_volume = 1.0 / static_cast<double>(n4);
    const double khat2 = 4.0 * sin(0.5 * actual_external_q)
                              * sin(0.5 * actual_external_q);
    const double zt = sums[0] * inv_volume;
    const double zs = sums[1] * inv_volume;
    const double ze = sums[2] * inv_volume / khat2;
    const double zb = sums[3] * inv_volume / (C2 * khat2);
    const double ward_t = sums[4] * inv_volume;
    const double ward_s = sums[5] * inv_volume;
    const double match = (zs - zt) - 0.5 * (zb - ze);

    std::printf("%d,%.15g,%.15g,%.15g,%.15g,%.15g,%.15g,%.15g,%.3e,%.3e\n",
                n, zt, zs, zs - zt, ze, zb, zb - ze, match,
                ward_t, ward_s);
    std::fflush(stdout);
  }

  cudaFree(device_sums);
  return 0;
}
