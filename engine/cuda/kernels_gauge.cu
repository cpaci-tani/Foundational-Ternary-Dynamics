/**
 * @file kernels_gauge.cu
 * @brief GPU kernels for Scale 0 Gauge Field non-Abelian plaquette relaxation.
 *
 * Implements SU(2) and SU(3) link variable relaxation on a 3D cubic lattice.
 */

#include <cuda_runtime.h>
#include <cmath>
#include "ftd/gauge_field.h"
#include "cuda_error.cuh"  // CUDA_CHECK (revision C2)

namespace ftd {
namespace gpu {

// Simple, device-compatible complex double representation
struct GpuComplex {
    double re;
    double im;

    __device__ __forceinline__ GpuComplex() : re(0.0), im(0.0) {}
    __device__ __forceinline__ GpuComplex(double r, double i) : re(r), im(i) {}

    __device__ __forceinline__ GpuComplex operator+(const GpuComplex& o) const {
        return {re + o.re, im + o.im};
    }
    __device__ __forceinline__ GpuComplex operator-(const GpuComplex& o) const {
        return {re - o.re, im - o.im};
    }
    __device__ __forceinline__ GpuComplex operator*(const GpuComplex& o) const {
        return {re * o.re - im * o.im, re * o.im + im * o.re};
    }
    __device__ __forceinline__ GpuComplex conj() const {
        return {re, -im};
    }
};

// Device-side SU(2) Link matrix
struct GpuSU2 {
    GpuComplex a;
    GpuComplex b;

    __device__ __forceinline__ GpuSU2() : a(1.0, 0.0), b(0.0, 0.0) {}
    __device__ __forceinline__ GpuSU2(GpuComplex val_a, GpuComplex val_b) : a(val_a), b(val_b) {}

    __device__ __forceinline__ GpuSU2 conj() const {
        return {a.conj(), GpuComplex(-b.re, -b.im)};
    }

    __device__ __forceinline__ GpuSU2 operator+(const GpuSU2& o) const {
        return {a + o.a, b + o.b};
    }

    __device__ __forceinline__ GpuSU2 operator*(const GpuSU2& o) const {
        // [[a1, b1], [-b1*, a1*]] * [[a2, b2], [-b2*, a2*]]
        GpuComplex new_a = a * o.a - b * o.b.conj();
        GpuComplex new_b = a * o.b + b * o.a.conj();
        return {new_a, new_b};
    }

    __device__ __forceinline__ void normalize() {
        double mag2 = a.re * a.re + a.im * a.im + b.re * b.re + b.im * b.im;
        double mag = std::sqrt(mag2);
        if (mag > 1e-12) {
            a.re /= mag;
            a.im /= mag;
            b.re /= mag;
            b.im /= mag;
        } else {
            a = GpuComplex(1.0, 0.0);
            b = GpuComplex(0.0, 0.0);
        }
    }

    // --- Element-wise primitives used by relax_links_kernel ------------------
    // These are the ONLY parts of the relaxation sweep that differ between
    // SU(2) and SU(3); everything else (staple geometry, accumulation order,
    // the local-minimization step) is shared by the template.

    // Loads link `s` in direction `dir` out of the three src buffers. The
    // direction branch is kept *outside* the element reads, exactly as in the
    // pre-consolidation kernel, so nvcc materializes the loads — and contracts
    // the downstream staple products — identically.
    static __device__ __forceinline__ GpuSU2 fetch(
        const SU2Link* src_x, const SU2Link* src_y, const SU2Link* src_z, int s, int dir)
    {
        GpuSU2 res;
        if (dir == 0) {
            res.a = GpuComplex(src_x[s].a.real(), src_x[s].a.imag());
            res.b = GpuComplex(src_x[s].b.real(), src_x[s].b.imag());
        } else if (dir == 1) {
            res.a = GpuComplex(src_y[s].a.real(), src_y[s].a.imag());
            res.b = GpuComplex(src_y[s].b.real(), src_y[s].b.imag());
        } else {
            res.a = GpuComplex(src_z[s].a.real(), src_z[s].a.imag());
            res.b = GpuComplex(src_z[s].b.real(), src_z[s].b.imag());
        }
        return res;
    }

    __device__ __forceinline__ void to_host(SU2Link& h) const {
        h.a = std::complex<double>(a.re, a.im);
        h.b = std::complex<double>(b.re, b.im);
    }

    // In place: *this = u_old + d * scale, element by element. Written as an
    // in-place assignment (rather than returning by value) so the emitted
    // expression tree — and therefore nvcc's FMA-contraction choices — match
    // the pre-consolidation kernel bit-for-bit.
    __device__ __forceinline__ void assign_scaled_add(const GpuSU2& u_old, const GpuSU2& d,
                                                      const GpuComplex& scale) {
        a = u_old.a + d.a * scale;
        b = u_old.b + d.b * scale;
    }
};

// Device-side SU(3) Link matrix
struct GpuSU3 {
    GpuComplex m[3][3];

    __device__ __forceinline__ GpuSU3() {
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                m[i][j] = (i == j) ? GpuComplex(1.0, 0.0) : GpuComplex(0.0, 0.0);
            }
        }
    }

    __device__ __forceinline__ GpuSU3 conj() const {
        GpuSU3 res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.m[i][j] = m[j][i].conj();
            }
        }
        return res;
    }

    __device__ __forceinline__ GpuSU3 operator+(const GpuSU3& o) const {
        GpuSU3 res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                res.m[i][j] = m[i][j] + o.m[i][j];
            }
        }
        return res;
    }

    __device__ __forceinline__ GpuSU3 operator*(const GpuSU3& o) const {
        GpuSU3 res;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                GpuComplex sum(0.0, 0.0);
                for (int k = 0; k < 3; ++k) {
                    sum = sum + m[i][k] * o.m[k][j];
                }
                res.m[i][j] = sum;
            }
        }
        return res;
    }

    __device__ __forceinline__ void normalize() {
        // Row-by-row Gram-Schmidt orthonormalization to project to SU(3)
        // Row 0 normalization
        double r0_mag = std::sqrt(m[0][0].re * m[0][0].re + m[0][0].im * m[0][0].im +
                                  m[0][1].re * m[0][1].re + m[0][1].im * m[0][1].im +
                                  m[0][2].re * m[0][2].re + m[0][2].im * m[0][2].im);
        if (r0_mag > 1e-12) {
            m[0][0].re /= r0_mag; m[0][0].im /= r0_mag;
            m[0][1].re /= r0_mag; m[0][1].im /= r0_mag;
            m[0][2].re /= r0_mag; m[0][2].im /= r0_mag;
        }

        // Row 1 orthogonalization against Row 0
        GpuComplex r1_dot_r0 = m[1][0] * m[0][0].conj() +
                               m[1][1] * m[0][1].conj() +
                               m[1][2] * m[0][2].conj();
        m[1][0] = m[1][0] - r1_dot_r0 * m[0][0];
        m[1][1] = m[1][1] - r1_dot_r0 * m[0][1];
        m[1][2] = m[1][2] - r1_dot_r0 * m[0][2];

        double r1_mag = std::sqrt(m[1][0].re * m[1][0].re + m[1][0].im * m[1][0].im +
                                  m[1][1].re * m[1][1].re + m[1][1].im * m[1][1].im +
                                  m[1][2].re * m[1][2].re + m[1][2].im * m[1][2].im);
        if (r1_mag > 1e-12) {
            m[1][0].re /= r1_mag; m[1][0].im /= r1_mag;
            m[1][1].re /= r1_mag; m[1][1].im /= r1_mag;
            m[1][2].re /= r1_mag; m[1][2].im /= r1_mag;
        }

        // Row 2 is cross product of Row 0 and Row 1 to guarantee det = 1
        // Row 2 = (Row 0 x Row 1)*
        m[2][0] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]).conj();
        m[2][1] = (m[0][2] * m[1][0] - m[0][0] * m[1][2]).conj();
        m[2][2] = (m[0][0] * m[1][1] - m[0][1] * m[1][0]).conj();
    }

    // --- Element-wise primitives used by relax_links_kernel (see GpuSU2) -----

    // Loads link `s` in direction `dir` (see GpuSU2::fetch). The direction
    // branch is kept *inside* the element loop, exactly as in the
    // pre-consolidation kernel.
    static __device__ __forceinline__ GpuSU3 fetch(
        const SU3Link* src_x, const SU3Link* src_y, const SU3Link* src_z, int s, int dir)
    {
        GpuSU3 res;
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (dir == 0) {
                    res.m[r][c] = GpuComplex(src_x[s].m[r][c].real(), src_x[s].m[r][c].imag());
                } else if (dir == 1) {
                    res.m[r][c] = GpuComplex(src_y[s].m[r][c].real(), src_y[s].m[r][c].imag());
                } else {
                    res.m[r][c] = GpuComplex(src_z[s].m[r][c].real(), src_z[s].m[r][c].imag());
                }
            }
        }
        return res;
    }

    __device__ __forceinline__ void to_host(SU3Link& h) const {
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                h.m[r][c] = std::complex<double>(m[r][c].re, m[r][c].im);
            }
        }
    }

    // In place: *this = u_old + d * scale, element by element (see GpuSU2).
    __device__ __forceinline__ void assign_scaled_add(const GpuSU3& u_old, const GpuSU3& d,
                                                      const GpuComplex& scale) {
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                m[r][c] = u_old.m[r][c] + d.m[r][c] * scale;
            }
        }
    }
};

// Coordinate helpers
__device__ __forceinline__ int wrap(int c, int L) {
    return (c + L) % L;
}

__device__ __forceinline__ int idx(int x, int y, int z, int L) {
    return wrap(x, L) * L * L + wrap(y, L) * L + wrap(z, L);
}

// -----------------------------------------------------------------------------
// Relaxation Kernel, shared by SU(2) and SU(3) — Jacobi double-buffered
// (revision 0.9 option a): every staple read comes from src_*, every write goes
// to dst_*, so threads never observe half-updated neighbor links. Matches the
// CPU sweep semantics in relax_su2_links_cpu (transmutation_phases.cpp). The
// caller swaps the src/dst pointers after each launch.
//
// One implementation serves both gauge groups. The staple geometry, the
// accumulation order and the local-minimization step are group-independent; the
// only group-dependent operations are element-wise, and are delegated to the
// GpuMat type: from_host / to_host (host<->device element load and store),
// scaled_add (U + dU * scale), conj, and normalize — the last being a 4-vector
// rescale for SU(2) and a Gram-Schmidt projection for SU(3).
// -----------------------------------------------------------------------------
template <typename GpuMat, typename HostLink>
__global__ void relax_links_kernel(
    const HostLink* src_x, const HostLink* src_y, const HostLink* src_z,
    HostLink* dst_x, HostLink* dst_y, HostLink* dst_z,
    int L, double dt, double beta)
{
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;

    if (x >= L || y >= L || z >= L) return;

    int site = idx(x, y, z, L);

    // Fetch links (from the pre-sweep src buffers — never dst)
    auto fetch_link = [&](int s, int dir) {
        return GpuMat::fetch(src_x, src_y, src_z, s, dir);
    };

    // Compute staples for each direction
    for (int mu = 0; mu < 3; ++mu) {
        GpuMat staple;
        bool first_staple = true;

        for (int nu = 0; nu < 3; ++nu) {
            if (mu == nu) continue;

            // Positive plaquette contribution:
            // U_nu(x + mu) * U_mu^\dagger(x + nu) * U_nu^\dagger(x)
            int site_xpmu = 0;
            int site_xpnu = 0;
            if (mu == 0) site_xpmu = idx(x + 1, y, z, L);
            else if (mu == 1) site_xpmu = idx(x, y + 1, z, L);
            else site_xpmu = idx(x, y, z + 1, L);

            if (nu == 0) site_xpnu = idx(x + 1, y, z, L);
            else if (nu == 1) site_xpnu = idx(x, y + 1, z, L);
            else site_xpnu = idx(x, y, z + 1, L);

            GpuMat u_nu_xpmu = fetch_link(site_xpmu, nu);
            GpuMat u_mu_xpnu = fetch_link(site_xpnu, mu);
            GpuMat u_nu_x = fetch_link(site, nu);

            GpuMat term1 = u_nu_xpmu * u_mu_xpnu.conj() * u_nu_x.conj();

            // Negative plaquette contribution:
            // U_nu^\dagger(x + mu - nu) * U_mu^\dagger(x - nu) * U_nu(x - nu)
            int site_xmnust = 0;
            int site_xpmu_mnu = 0;
            if (nu == 0) {
                site_xmnust = idx(x - 1, y, z, L);
                if (mu == 0) site_xpmu_mnu = idx(x, y, z, L);
                else if (mu == 1) site_xpmu_mnu = idx(x - 1, y + 1, z, L);
                else site_xpmu_mnu = idx(x - 1, y, z + 1, L);
            } else if (nu == 1) {
                site_xmnust = idx(x, y - 1, z, L);
                if (mu == 0) site_xpmu_mnu = idx(x + 1, y - 1, z, L);
                else if (mu == 1) site_xpmu_mnu = idx(x, y, z, L);
                else site_xpmu_mnu = idx(x, y - 1, z + 1, L);
            } else {
                site_xmnust = idx(x, y, z - 1, L);
                if (mu == 0) site_xpmu_mnu = idx(x + 1, y, z - 1, L);
                else if (mu == 1) site_xpmu_mnu = idx(x, y + 1, z - 1, L);
                else site_xpmu_mnu = idx(x, y, z, L);
            }

            GpuMat u_nu_xpmu_mnu = fetch_link(site_xpmu_mnu, nu);
            GpuMat u_mu_xmnu = fetch_link(site_xmnust, mu);
            GpuMat u_nu_xmnu = fetch_link(site_xmnust, nu);

            GpuMat term2 = u_nu_xpmu_mnu.conj() * u_mu_xmnu.conj() * u_nu_xmnu;

            if (first_staple) {
                staple = term1 + term2;
                first_staple = false;
            } else {
                staple = staple + term1 + term2;
            }
        }

        // Local minimization update: U_new = Proj[ U_old + dt * beta * staple^\dagger ]
        GpuMat u_old = fetch_link(site, mu);
        GpuMat staple_adj = staple.conj();
        GpuComplex scale(dt * beta, 0.0);

        GpuMat u_new;
        u_new.assign_scaled_add(u_old, staple_adj, scale);
        u_new.normalize();

        // Write back (to the dst buffers — Jacobi, never in place)
        HostLink* dst = (mu == 0) ? dst_x : ((mu == 1) ? dst_y : dst_z);
        u_new.to_host(dst[site]);
    }
}

// Host wrappers to launch the kernels from other source files (GpuEngine's
// gauge phase). Double-buffered (revision 0.9 option a): the kernels read
// src_* and write dst_*; callers swap the pointer sets after each launch.
// Block shape: 4x4x4 = 64 threads. The staple kernels are register-heavy
// (GpuSU3 is 18 doubles; several matrix temporaries live across the mu loop)
// — at 8x8x8 = 512 threads/block the SU(3) launch fails with "too many
// resources requested for launch" (register budget). 64 threads/block
// launches on any SM. Per-site independent (no shared memory, no
// reductions), so the block shape cannot affect results or determinism.
extern "C" void launch_relax_su2_links(
    const SU2Link* src_x, const SU2Link* src_y, const SU2Link* src_z,
    SU2Link* dst_x, SU2Link* dst_y, SU2Link* dst_z,
    int L, double dt, double beta, cudaStream_t stream)
{
    dim3 threads(4, 4, 4);
    dim3 blocks((L + 3) / 4, (L + 3) / 4, (L + 3) / 4);
    relax_links_kernel<GpuSU2, SU2Link><<<blocks, threads, 0, stream>>>(
        src_x, src_y, src_z, dst_x, dst_y, dst_z, L, dt, beta);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently
}

extern "C" void launch_relax_su3_links(
    const SU3Link* src_x, const SU3Link* src_y, const SU3Link* src_z,
    SU3Link* dst_x, SU3Link* dst_y, SU3Link* dst_z,
    int L, double dt, double beta, cudaStream_t stream)
{
    dim3 threads(4, 4, 4);
    dim3 blocks((L + 3) / 4, (L + 3) / 4, (L + 3) / 4);
    relax_links_kernel<GpuSU3, SU3Link><<<blocks, threads, 0, stream>>>(
        src_x, src_y, src_z, dst_x, dst_y, dst_z, L, dt, beta);
    CUDA_CHECK(cudaGetLastError());  // revision C2: launch-config errors must not propagate silently
}

} // namespace gpu
} // namespace ftd
