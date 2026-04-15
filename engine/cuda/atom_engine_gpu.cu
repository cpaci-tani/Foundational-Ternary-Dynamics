/**
 * GPU AtomEngine backend (Wave 5.3 Phase 1).
 *
 * Pair-force acceleration for ftd::AtomEngine. Implements ionic (Coulomb)
 * and van der Waals (Lennard-Jones 12-6) forces as an O(N²) per-atom
 * kernel. Matches the CPU reference in engine/src/atom_engine.cpp
 * compute_pairwise_force() for the subset of toggles handled here.
 *
 * Phase 1 scope:
 *   - Upload/download SoA atom fields
 *   - O(N²) pair forces: ionic + vdW
 *   - Per-force diagnostics (f_ionic, f_vdw)
 *
 * Phase 2 (deferred): h_bond, bond forces, angle strain, dipole-dipole,
 * thermostat reduction.
 */

#include "ftd/gpu_atom_engine.h"

#ifdef FTD_ENABLE_CUDA

#include "ftd/atom_engine.h"
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
// AtomBuffers: allocate / free / upload / download
// ============================================================================

void AtomBuffers::allocate(int capacity) {
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

    CUDA_CHECK(cudaMalloc(&d_mass,    capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_charge,  capacity * sizeof(int32_t)));
    CUDA_CHECK(cudaMalloc(&d_radius,  capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_vdw_eps, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_vdw_sig, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_locked,  capacity * sizeof(uint8_t)));

    CUDA_CHECK(cudaMalloc(&d_f_ionic_x, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_ionic_y, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_ionic_z, capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_vdw_x,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_vdw_y,   capacity * sizeof(double)));
    CUDA_CHECK(cudaMalloc(&d_f_vdw_z,   capacity * sizeof(double)));
}

void AtomBuffers::free() {
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
    if (d_radius)   { cudaFree(d_radius);   d_radius   = nullptr; }
    if (d_vdw_eps)  { cudaFree(d_vdw_eps);  d_vdw_eps  = nullptr; }
    if (d_vdw_sig)  { cudaFree(d_vdw_sig);  d_vdw_sig  = nullptr; }
    if (d_locked)   { cudaFree(d_locked);   d_locked   = nullptr; }
    if (d_f_ionic_x){ cudaFree(d_f_ionic_x);d_f_ionic_x= nullptr; }
    if (d_f_ionic_y){ cudaFree(d_f_ionic_y);d_f_ionic_y= nullptr; }
    if (d_f_ionic_z){ cudaFree(d_f_ionic_z);d_f_ionic_z= nullptr; }
    if (d_f_vdw_x)  { cudaFree(d_f_vdw_x);  d_f_vdw_x  = nullptr; }
    if (d_f_vdw_y)  { cudaFree(d_f_vdw_y);  d_f_vdw_y  = nullptr; }
    if (d_f_vdw_z)  { cudaFree(d_f_vdw_z);  d_f_vdw_z  = nullptr; }
    N_alloc = 0;
    N = 0;
}

void AtomBuffers::ensure_capacity(int n) {
    if (n > N_alloc) {
        // Grow to next power of two to amortize reallocation cost
        int new_cap = N_alloc > 0 ? N_alloc : 16;
        while (new_cap < n) new_cap *= 2;
        allocate(new_cap);
    }
}

void AtomBuffers::upload_atoms(const std::vector<Atom>& host_atoms) {
    int n = static_cast<int>(host_atoms.size());
    ensure_capacity(n);
    N = n;

    std::vector<double>  h_px(n), h_py(n), h_pz(n);
    std::vector<double>  h_vx(n), h_vy(n), h_vz(n);
    std::vector<double>  h_mass(n), h_radius(n), h_eps(n), h_sig(n);
    std::vector<int32_t> h_charge(n);
    std::vector<uint8_t> h_locked(n);

    for (int i = 0; i < n; ++i) {
        const auto& a = host_atoms[i];
        h_px[i]     = a.position.x;
        h_py[i]     = a.position.y;
        h_pz[i]     = a.position.z;
        h_vx[i]     = a.velocity.x;
        h_vy[i]     = a.velocity.y;
        h_vz[i]     = a.velocity.z;
        h_mass[i]   = a.mass;
        h_charge[i] = a.charge;
        h_radius[i] = a.radius;
        h_eps[i]    = a.vdw_epsilon;
        h_sig[i]    = a.vdw_sigma;
        h_locked[i] = a.locked ? 1 : 0;
    }

    CUDA_CHECK(cudaMemcpy(d_pos_x,   h_px.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pos_y,   h_py.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_pos_z,   h_pz.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_x,   h_vx.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_y,   h_vy.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vel_z,   h_vz.data(),     n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_mass,    h_mass.data(),   n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_charge,  h_charge.data(), n * sizeof(int32_t), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_radius,  h_radius.data(), n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vdw_eps, h_eps.data(),    n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_vdw_sig, h_sig.data(),    n * sizeof(double), cudaMemcpyHostToDevice));
    CUDA_CHECK(cudaMemcpy(d_locked,  h_locked.data(), n * sizeof(uint8_t), cudaMemcpyHostToDevice));
}

void AtomBuffers::download_forces(std::vector<Vec3>& out_forces,
                                  std::vector<AtomForceDiag>& out_diag) const {
    int n = N;
    out_forces.resize(n);
    out_diag.resize(n);

    std::vector<double> h_fx(n), h_fy(n), h_fz(n);
    std::vector<double> h_ix(n), h_iy(n), h_iz(n);
    std::vector<double> h_vx(n), h_vy(n), h_vz(n);

    CUDA_CHECK(cudaMemcpy(h_fx.data(), d_force_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fy.data(), d_force_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_fz.data(), d_force_z, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_ix.data(), d_f_ionic_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_iy.data(), d_f_ionic_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_iz.data(), d_f_ionic_z, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vx.data(), d_f_vdw_x,   n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vy.data(), d_f_vdw_y,   n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vz.data(), d_f_vdw_z,   n * sizeof(double), cudaMemcpyDeviceToHost));

    for (int i = 0; i < n; ++i) {
        out_forces[i] = { h_fx[i], h_fy[i], h_fz[i] };
        out_diag[i].f_ionic = { h_ix[i], h_iy[i], h_iz[i] };
        out_diag[i].f_vdw   = { h_vx[i], h_vy[i], h_vz[i] };
        // Other components (f_bond, f_hbond, f_dipole, f_angle, f_torsion,
        // f_improper) are populated by the CPU path that calls us.
    }
}

void AtomBuffers::download_kinematics(std::vector<Atom>& host_atoms) const {
    int n = N;
    std::vector<double> h_px(n), h_py(n), h_pz(n);
    std::vector<double> h_vx(n), h_vy(n), h_vz(n);

    CUDA_CHECK(cudaMemcpy(h_px.data(), d_pos_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_py.data(), d_pos_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_pz.data(), d_pos_z, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vx.data(), d_vel_x, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vy.data(), d_vel_y, n * sizeof(double), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaMemcpy(h_vz.data(), d_vel_z, n * sizeof(double), cudaMemcpyDeviceToHost));

    for (int i = 0; i < n; ++i) {
        if (i < static_cast<int>(host_atoms.size())) {
            host_atoms[i].position = { h_px[i], h_py[i], h_pz[i] };
            host_atoms[i].velocity = { h_vx[i], h_vy[i], h_vz[i] };
        }
    }
}

// ============================================================================
// Kernel: O(N²) pair forces (ionic + van der Waals)
//
// Reference: atom_engine.cpp:168-304 compute_pairwise_force()
//
// For each atom i, loop over all j ≠ i, sum ionic + vdW pair forces into
// (force_x[i], force_y[i], force_z[i]). Per-component diagnostics go
// into f_ionic and f_vdw buffers.
// ============================================================================

__global__ void atom_pair_forces_kernel(
    const double* __restrict__ pos_x,
    const double* __restrict__ pos_y,
    const double* __restrict__ pos_z,
    const int32_t* __restrict__ charge,
    const double* __restrict__ vdw_eps,
    const double* __restrict__ vdw_sig,
    double* __restrict__ force_x,
    double* __restrict__ force_y,
    double* __restrict__ force_z,
    double* __restrict__ f_ionic_x,
    double* __restrict__ f_ionic_y,
    double* __restrict__ f_ionic_z,
    double* __restrict__ f_vdw_x,
    double* __restrict__ f_vdw_y,
    double* __restrict__ f_vdw_z,
    int N,
    double soft,
    double alpha,
    double pi,
    int toggles_ionic,
    int toggles_vdw
) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i >= N) return;

    double px = pos_x[i];
    double py = pos_y[i];
    double pz = pos_z[i];
    double qi = static_cast<double>(charge[i]);
    double eps_i = vdw_eps[i];
    double sig_i = vdw_sig[i];

    double fx = 0.0, fy = 0.0, fz = 0.0;
    double fix = 0.0, fiy = 0.0, fiz = 0.0;   // ionic accumulators
    double fvx = 0.0, fvy = 0.0, fvz = 0.0;   // vdW accumulators

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

        // 1. Ionic: F = -ALPHA * q_i * q_j / (4*pi*r²) * r_hat
        if (toggles_ionic) {
            double qj = static_cast<double>(charge[j]);
            if (qi != 0.0 && qj != 0.0) {
                double f_ionic = -alpha * qi * qj * inv_4pi / r2;
                double fi_x = rhx * f_ionic;
                double fi_y = rhy * f_ionic;
                double fi_z = rhz * f_ionic;
                fix += fi_x; fiy += fi_y; fiz += fi_z;
                fx += fi_x; fy += fi_y; fz += fi_z;
            }
        }

        // 2. Van der Waals (Lennard-Jones 12-6)
        //    F = -24 * eps * (2*sr^12 - sr^6) / r * r_hat
        if (toggles_vdw) {
            double eps_j = vdw_eps[j];
            double sig_j = vdw_sig[j];
            double eps_mix = sqrt(eps_i * eps_j);
            double sig_mix = 0.5 * (sig_i + sig_j);
            if (eps_mix > 0.0 && sig_mix > 0.0) {
                double sr   = sig_mix * inv_r;
                double sr2  = sr * sr;
                double sr6  = sr2 * sr2 * sr2;
                double sr12 = sr6 * sr6;
                double f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) * inv_r;
                double fv_x = rhx * f_vdw;
                double fv_y = rhy * f_vdw;
                double fv_z = rhz * f_vdw;
                fvx += fv_x; fvy += fv_y; fvz += fv_z;
                fx += fv_x; fy += fv_y; fz += fv_z;
            }
        }
    }

    force_x[i]   = fx;
    force_y[i]   = fy;
    force_z[i]   = fz;
    f_ionic_x[i] = fix;
    f_ionic_y[i] = fiy;
    f_ionic_z[i] = fiz;
    f_vdw_x[i]   = fvx;
    f_vdw_y[i]   = fvy;
    f_vdw_z[i]   = fvz;
}

// ============================================================================
// AtomEngineGpu host wrapper
// ============================================================================

AtomEngineGpu::AtomEngineGpu()
    : bufs_(std::make_unique<AtomBuffers>()) {}

AtomEngineGpu::~AtomEngineGpu() {
    if (bufs_) bufs_->free();
}

void AtomEngineGpu::compute_pair_forces(
    const std::vector<Atom>& host_atoms,
    const AtomToggles& toggles,
    double soft,
    std::vector<Vec3>& out_forces,
    std::vector<AtomForceDiag>& out_diag)
{
    int n = static_cast<int>(host_atoms.size());
    if (n == 0) {
        out_forces.clear();
        out_diag.clear();
        return;
    }

    bufs_->upload_atoms(host_atoms);

    int threads = 256;
    int blocks = (n + threads - 1) / threads;

    atom_pair_forces_kernel<<<blocks, threads>>>(
        bufs_->d_pos_x, bufs_->d_pos_y, bufs_->d_pos_z,
        bufs_->d_charge, bufs_->d_vdw_eps, bufs_->d_vdw_sig,
        bufs_->d_force_x, bufs_->d_force_y, bufs_->d_force_z,
        bufs_->d_f_ionic_x, bufs_->d_f_ionic_y, bufs_->d_f_ionic_z,
        bufs_->d_f_vdw_x,   bufs_->d_f_vdw_y,   bufs_->d_f_vdw_z,
        n,
        soft,
        ALPHA,
        PI,
        toggles.ionic        ? 1 : 0,
        toggles.van_der_waals ? 1 : 0
    );
    CUDA_CHECK(cudaGetLastError());

    bufs_->download_forces(out_forces, out_diag);
}

}  // namespace gpu
}  // namespace ftd

#endif  // FTD_ENABLE_CUDA
