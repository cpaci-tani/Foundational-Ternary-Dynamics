/**
 * GPU ParticleEngine backend (Wave 5.4 Phase 1).
 *
 * Pair-force acceleration for ftd::ParticleEngine. Implements Coulomb
 * (using ALPHA_EFT = G_C²) and Newtonian gravity (using G_N) as an
 * O(N²) per-particle kernel. Matches the CPU reference in
 * engine/src/particle_engine.cpp compute_pairwise_force() for the
 * subset of toggles handled here.
 *
 * Phase 1 scope:
 *   - Upload/download SoA particle fields
 *   - O(N²) pair forces: coulomb + gravity
 *   - Per-force diagnostics (f_coulomb, f_gravity)
 *
 * Phase 2 (deferred): strong, exchange, lorentz, magnetic_dipole,
 *   spin_orbit.
 * Phase 3 (deferred): radiation (self-force), relativistic correction
 *   (post-processing), Barnes-Hut octree.
 */

#include "ftd/gpu_particle_engine.h"

#ifdef FTD_ENABLE_CUDA

#include "ftd/particle_engine.h"
#include "ftd/constants.h"
#include <cuda_runtime.h>
#include <cstdio>
#include <cstdlib>

#define CUDA_CHECK(call) do { \
    cudaError_t err = (call); \
    if (err != cudaSuccess) { \
        fprintf(stderr, "CUDA error at %s:%d: %s\n", \
                __FILE__, __LINE__, cudaGetErrorString(err)); \
        exit(1); \
    } \
} while(0)

namespace ftd {
namespace gpu {

// ============================================================================
// ParticleBuffers: allocate / free / upload / download
// ============================================================================

void ParticleBuffers::allocate(int capacity) {
    if (capacity <= N_alloc) return;
    free();
    N_alloc = capacity;

    CUDA_CHECK(cudaMalloc(&d_pos_x,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_pos_y,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_pos_z,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_vel_x,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_vel_y,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_vel_z,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_force_x, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_force_y, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_force_z, capacity * sizeof(double)));

    CUDA_CHECK(cudaMalloc(&d_mass,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_charge, capacity * sizeof(int8_t)));
    CUDA_CHECK(cudaMalloc(&d_locked, capacity * sizeof(uint8_t)));

    CUDA_CHECK(cudaMalloc(&d_f_coulomb_x, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_coulomb_y, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_coulomb_z, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_gravity_x, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_gravity_y, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_gravity_z, capacity * sizeof(double)));
}

void ParticleBuffers::free() {
    if (d_pos_x)    { cudaFree(d_pos_x);    d_pos_x    = nullptr; }
    if (d_pos_y)    { cudaFree(d_pos_y);    d_pos_y    = nullptr; }
    if (d_pos_z)    { cudaFree(d_pos_z);    d_pos_z    = nullptr; }
    if (d_vel_x)    { cudaFree(d_vel_x);    d_vel_x    = nullptr; }
    if (d_vel_y)    { cudaFree(d_vel_y);    d_vel_y    = nullptr; }
    if (d_vel_z)    { cudaFree(d_vel_z);    d_vel_z    = nullptr; }
    if (d_force_x)  { cudaFree(d_force_x);  d_force_x  = nullptr; }
    if (d_force_y)  { cudaFree(d_force_y);  d_force_y  = nullptr; }
    if (d_force_z)  { cudaFree(d_force_z);  d_force_z  = nullptr; }
    if (d_mass)     { cudaFree(d_mass);     d_mass     = nullptr; }
    if (d_charge)   { cudaFree(d_charge);   d_charge   = nullptr; }
    if (d_locked)   { cudaFree(d_locked);   d_locked   = nullptr; }
    if (d_f_coulomb_x) { cudaFree(d_f_coulomb_x); d_f_coulomb_x = nullptr; }
    if (d_f_coulomb_y) { cudaFree(d_f_coulomb_y); d_f_coulomb_y = nullptr; }
    if (d_f_coulomb_z) { cudaFree(d_f_coulomb_z); d_f_coulomb_z = nullptr; }
    if (d_f_gravity_x) { cudaFree(d_f_gravity_x); d_f_gravity_x = nullptr; }
    if (d_f_gravity_y) { cudaFree(d_f_gravity_y); d_f_gravity_y = nullptr; }
    if (d_f_gravity_z) { cudaFree(d_f_gravity_z); d_f_gravity_z = nullptr; }
    N_alloc = 0;
    N = 0;
}

void ParticleBuffers::ensure_capacity(int n) {
    if (n > N_alloc) {
        int new_cap = N_alloc > 0 ? N_alloc : 16;
        while (new_cap < n) new_cap *= 2;
        allocate(new_cap);
    }
}

void ParticleBuffers::upload_particles(const std::vector<Particle>& host_particles) {
    int n = static_cast<int>(host_particles.size());
    ensure_capacity(n);
    N = n;

    std::vector<double>  h_px(n), h_py(n), h_pz(n);
    std::vector<double>  h_vx(n), h_vy(n), h_vz(n);
    std::vector<double>  h_mass(n);
    std::vector<int8_t>  h_charge(n);
    std::vector<uint8_t> h_locked(n);

    for (int i = 0; i < n; ++i) {
        const auto& p = host_particles[i];
        h_px[i]     = p.position.x;
        h_py[i]     = p.position.y;
        h_pz[i]     = p.position.z;
        h_vx[i]     = p.velocity.x;
        h_vy[i]     = p.velocity.y;
        h_vz[i]     = p.velocity.z;
        h_mass[i]   = p.mass;
        h_charge[i] = p.charge;
        h_locked[i] = p.locked ? 1 : 0;
    }

    CUDA_CHECK(cudaMemcpy(d_pos_x,  h_px.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pos_y,  h_py.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pos_z,  h_pz.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_x,  h_vx.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_y,  h_vy.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_z,  h_vz.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_mass,   h_mass.data(),   n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_charge, h_charge.data(), n * sizeof(int8_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_locked, h_locked.data(), n * sizeof(uint8_t), cudaMemcpyHostToDevice));
}

void ParticleBuffers::download_forces(std::vector<Vec3>& out_forces,
                                      std::vector<ParticleForceDiag>& out_diag) const {
    int n = N;
    out_forces.resize(n);
    out_diag.resize(n);

    std::vector<double> h_fx(n), h_fy(n), h_fz(n);
    std::vector<double> h_cx(n), h_cy(n), h_cz(n);
    std::vector<double> h_gx(n), h_gy(n), h_gz(n);

    CUDA_CHECK(cudaMemcpy(h_fx.data(), d_force_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fy.data(), d_force_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fz.data(), d_force_z, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_cx.data(), d_f_coulomb_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_cy.data(), d_f_coulomb_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_cz.data(), d_f_coulomb_z, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_gx.data(), d_f_gravity_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_gy.data(), d_f_gravity_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_gz.data(), d_f_gravity_z, n * sizeof(double), cudaMemcpyDeviceToHost));

    for (int i = 0; i < n; ++i) {
        out_forces[i] = { h_fx[i], h_fy[i], h_fz[i] };
        out_diag[i].f_coulomb = { h_cx[i], h_cy[i], h_cz[i] };
        out_diag[i].f_gravity = { h_gx[i], h_gy[i], h_gz[i] };
        // Other components (f_lorentz, f_exchange, f_strong, f_radiation,
        // f_spin_orbit, f_relativistic, f_magnetic_dipole) stay zero at
        // the device level — caller initializes them on the host side
        // before this download and any CPU-side extras are added after.
    }
}

// ============================================================================
// Kernel: O(N²) pair forces (Coulomb + Gravity)
//
// Reference: particle_engine.cpp:102-134 compute_pairwise_force() for
// the toggles.coulomb and toggles.gravity branches.
//
// For each particle i, loop over all j ≠ i, sum Coulomb + gravity pair
// forces into (force_x[i], force_y[i], force_z[i]). Per-component
// diagnostics go into f_coulomb and f_gravity buffers.
// ============================================================================

__global__ void particle_pair_forces_kernel(
    const double* __restrict__ pos_x,
    const double* __restrict__ pos_y,
    const double* __restrict__ pos_z,
    const double* __restrict__ mass,
    const int8_t* __restrict__ charge,
    double* __restrict__ force_x,
    double* __restrict__ force_y,
    double* __restrict__ force_z,
    double* __restrict__ f_coulomb_x,
    double* __restrict__ f_coulomb_y,
    double* __restrict__ f_coulomb_z,
    double* __restrict__ f_gravity_x,
    double* __restrict__ f_gravity_y,
    double* __restrict__ f_gravity_z,
    int N,
    double soft,
    double alpha_eft,
    double pi,
    double g_n,
    int toggles_coulomb,
    int toggles_gravity
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    double px = pos_x[i];
    double py = pos_y[i];
    double pz = pos_z[i];
    double mi = mass[i];
    double qi = static_cast<double>(charge[i]);

    double fx = 0.0, fy = 0.0, fz = 0.0;
    double fcx = 0.0, fcy = 0.0, fcz = 0.0;   // Coulomb accumulators
    double fgx = 0.0, fgy = 0.0, fgz = 0.0;   // Gravity accumulators

    double soft2 = soft * soft;
    double inv_4pi = 1.0 / (4.0 * pi);

    for (int j = 0; j < N; ++j) {
        if (i == j) continue;

        double rx = pos_x[j] - px;
        double ry = pos_y[j] - py;
        double rz = pos_z[j] - pz;
        double r2 = rx * rx + ry * ry + rz * rz + soft2;
        double r  = sqrt(r2);
        if (r < 1e-30) continue;

        double inv_r = 1.0 / r;
        double rhx = rx * inv_r;
        double rhy = ry * inv_r;
        double rhz = rz * inv_r;

        // 1. Coulomb: F = -ALPHA_EFT * q_i * q_j / (4*pi*r²) * r_hat
        if (toggles_coulomb) {
            double qj = static_cast<double>(charge[j]);
            double f_em = -alpha_eft * qi * qj * inv_4pi / r2;
            double fc_x = rhx * f_em;
            double fc_y = rhy * f_em;
            double fc_z = rhz * f_em;
            fcx += fc_x; fcy += fc_y; fcz += fc_z;
            fx += fc_x; fy += fc_y; fz += fc_z;
        }

        // 2. Gravity: F = +G_N * m_i * m_j / r² * r_hat (always attractive)
        if (toggles_gravity) {
            double mj = mass[j];
            double f_grav = g_n * mi * mj / r2;
            double fg_x = rhx * f_grav;
            double fg_y = rhy * f_grav;
            double fg_z = rhz * f_grav;
            fgx += fg_x; fgy += fg_y; fgz += fg_z;
            fx += fg_x; fy += fg_y; fz += fg_z;
        }
    }

    force_x[i]     = fx;
    force_y[i]     = fy;
    force_z[i]     = fz;
    f_coulomb_x[i] = fcx;
    f_coulomb_y[i] = fcy;
    f_coulomb_z[i] = fcz;
    f_gravity_x[i] = fgx;
    f_gravity_y[i] = fgy;
    f_gravity_z[i] = fgz;
}

// ============================================================================
// ParticleEngineGpu host wrapper
// ============================================================================

ParticleEngineGpu::ParticleEngineGpu()
    : bufs_(std::make_unique<ParticleBuffers>()) {}

ParticleEngineGpu::~ParticleEngineGpu() {
    if (bufs_) bufs_->free();
}

void ParticleEngineGpu::compute_pair_forces(
    const std::vector<Particle>& host_particles,
    const ParticleToggles& toggles,
    double soft,
    std::vector<Vec3>& out_forces,
    std::vector<ParticleForceDiag>& out_diag)
{
    int n = static_cast<int>(host_particles.size());
    if (n == 0) {
        out_forces.clear();
        out_diag.clear();
        return;
    }

    bufs_->upload_particles(host_particles);

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    particle_pair_forces_kernel<<<blocks, threads>>>(
        bufs_->d_pos_x, bufs_->d_pos_y, bufs_->d_pos_z,
        bufs_->d_mass,  bufs_->d_charge,
        bufs_->d_force_x, bufs_->d_force_y, bufs_->d_force_z,
        bufs_->d_f_coulomb_x, bufs_->d_f_coulomb_y, bufs_->d_f_coulomb_z,
        bufs_->d_f_gravity_x, bufs_->d_f_gravity_y, bufs_->d_f_gravity_z,
        n,
        soft,
        ALPHA_EFT,
        PI,
        G_N,
        toggles.coulomb ? 1 : 0,
        toggles.gravity ? 1 : 0
    );
    CUDA_CHECK(cudaGetLastError());

    bufs_->download_forces(out_forces, out_diag);
}

}  // namespace gpu
}  // namespace ftd

#endif  // FTD_ENABLE_CUDA
