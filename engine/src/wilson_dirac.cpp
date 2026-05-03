/**
 * Wilson-Dirac CPU implementation -- Phase II.2-A.
 *
 * Implements the operator D_W and its evolution per
 * docs/theory/10_eft_program/SPEC_WILSON_DIRAC_FTD.md.
 *
 * Conventions (committed pre-measurement):
 *   - Chiral (Weyl) basis: gamma^i = ((0, -sigma^i), (sigma^i, 0))
 *   - 3 spatial dimensions; periodic BC inherited from Lattice
 *   - Lattice spacing a = 1 (engine-internal)
 *   - Wilson parameter r = 1
 *   - Time evolution: i d/dt psi = D_W psi (Schrodinger-like)
 */

#include "ftd/wilson_dirac.h"

#include <cmath>

namespace ftd {
namespace wilson_dirac {

namespace {

// Pauli matrices.
// sigma^1 = [[0, 1], [1, 0]]
// sigma^2 = [[0, -i], [i, 0]]
// sigma^3 = [[1, 0], [0, -1]]
inline std::array<cdouble, 2> apply_pauli(int i, const std::array<cdouble, 2>& v) {
    switch (i) {
    case 0: {
        return {v[1], v[0]};
    }
    case 1: {
        const cdouble I{0, 1};
        return {-I * v[1], I * v[0]};
    }
    case 2:
    default:
        return {v[0], -v[1]};
    }
}

// In chiral basis, a Spinor (4 components) splits as upper = {psi[0], psi[1]} and
// lower = {psi[2], psi[3]}. Helper to extract / repack.
inline std::array<cdouble, 2> upper(const Spinor& psi) { return {psi[0], psi[1]}; }
inline std::array<cdouble, 2> lower(const Spinor& psi) { return {psi[2], psi[3]}; }
inline Spinor pack(const std::array<cdouble, 2>& u, const std::array<cdouble, 2>& l) {
    return {u[0], u[1], l[0], l[1]};
}

}  // namespace

// -------------------------------------------------------------
// Gamma-matrix-projector kernels.
//
// In chiral basis: gamma^i acts on a 4-spinor as
//   gamma^i psi = (-sigma^i * lower, sigma^i * upper)
// i.e., it swaps upper and lower with sigma^i applied (with a sign on lower).
//
// (r - gamma^i) psi = (r * upper + sigma^i * lower, r * lower - sigma^i * upper)
// (r + gamma^i) psi = (r * upper - sigma^i * lower, r * lower + sigma^i * upper)
// -------------------------------------------------------------
Spinor apply_r_minus_gamma_spatial(double r, int mu, const Spinor& psi) {
    const auto u = upper(psi);
    const auto l = lower(psi);
    const auto sig_l = apply_pauli(mu, l);
    const auto sig_u = apply_pauli(mu, u);
    std::array<cdouble, 2> new_u = {r * u[0] + sig_l[0], r * u[1] + sig_l[1]};
    std::array<cdouble, 2> new_l = {r * l[0] - sig_u[0], r * l[1] - sig_u[1]};
    return pack(new_u, new_l);
}

Spinor apply_r_plus_gamma_spatial(double r, int mu, const Spinor& psi) {
    const auto u = upper(psi);
    const auto l = lower(psi);
    const auto sig_l = apply_pauli(mu, l);
    const auto sig_u = apply_pauli(mu, u);
    std::array<cdouble, 2> new_u = {r * u[0] - sig_l[0], r * u[1] - sig_l[1]};
    std::array<cdouble, 2> new_l = {r * l[0] + sig_u[0], r * l[1] + sig_u[1]};
    return pack(new_u, new_l);
}

Spinor apply_sigma_spatial(int i, const Spinor& psi) {
    // Sigma^i = diag(sigma^i, sigma^i) in chiral basis.
    const auto u = upper(psi);
    const auto l = lower(psi);
    const auto su = apply_pauli(i, u);
    const auto sl = apply_pauli(i, l);
    return pack(su, sl);
}

// -------------------------------------------------------------
// SpinorField total norm-squared.
// -------------------------------------------------------------
double SpinorField::total_norm_squared() const {
    double total = 0.0;
    for (const auto& s : data) {
        total += spinor_norm_squared(s);
    }
    return total;
}

// -------------------------------------------------------------
// GaugeLinks helpers.
// -------------------------------------------------------------
void GaugeLinks::set_identity() {
    const std::size_t N = static_cast<std::size_t>(L) * L * L;
    for (int mu = 0; mu < 3; ++mu) {
        for (std::size_t i = 0; i < N; ++i) {
            U[mu][i] = cdouble{1, 0};
        }
    }
}

void GaugeLinks::set_uniform_B_z(double g_a_B0_a) {
    // Landau gauge: A_x(n) = -B0 * n_y * a, others zero.
    // Phase per link: phi_x(n) = a * g * A_x(n) = -g * a * B0 * n_y * a.
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const std::size_t idx = static_cast<std::size_t>(z) * L * L + y * L + x;
                const double phi_x = -g_a_B0_a * static_cast<double>(y);
                U[0][idx] = std::exp(cdouble{0, phi_x});
                U[1][idx] = cdouble{1, 0};
                U[2][idx] = cdouble{1, 0};
            }
        }
    }
}

// -------------------------------------------------------------
// Wilson-Dirac operator.
//
// (D_W psi)(n) = (m + 4r/a) psi(n)
//                - (1/2a) sum_{mu=0,1,2} [
//                     (r - gamma^mu) U_mu(n)         psi(n + mu_hat)
//                   + (r + gamma^mu) U_mu^dag(n - mu) psi(n - mu_hat)
//                  ]
// -------------------------------------------------------------
namespace {

// Periodic-coordinate shift along axis mu.
inline int shift_plus(int n, int L, int mu, int delta) {
    int v = (mu == 0) ? n : 0;
    (void)mu; (void)delta;
    return (n + delta + L) % L;
}

}  // namespace

void apply_wilson_dirac(SpinorField& out,
                        const SpinorField& psi,
                        const GaugeLinks& links,
                        const Lattice& lattice,
                        const WilsonDiracParams& params) {
    const int L = lattice.size();
    // 3D spatial Wilson-Dirac: dimension-consistent mass shift = D_spatial * r / a = 3r/a.
    // (For full 4D Euclidean Wilson-Dirac, this would be 4r/a; we evolve continuous time
    //  via Schrödinger-like i d/dt psi = D_W psi, so the spatial operator is what's needed.)
    const double diag = params.m + 3.0 * params.r / params.a;
    const double off = 1.0 / (2.0 * params.a);

    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int idx = lattice.index(x, y, z);
                Spinor result = scale(cdouble{diag, 0}, psi.at(idx));

                // mu = 0 (x direction)
                {
                    const int xp = (x + 1) % L;
                    const int xm = (x - 1 + L) % L;
                    const int idx_xp = lattice.index(xp, y, z);
                    const int idx_xm = lattice.index(xm, y, z);
                    const cdouble U_x_n     = links.U[0][static_cast<std::size_t>(idx)];
                    const cdouble U_x_nminus = links.U[0][static_cast<std::size_t>(idx_xm)];
                    const Spinor psi_xp = scale(U_x_n, psi.at(idx_xp));
                    const Spinor psi_xm = scale(std::conj(U_x_nminus), psi.at(idx_xm));
                    Spinor term = add(apply_r_minus_gamma_spatial(params.r, 0, psi_xp),
                                      apply_r_plus_gamma_spatial(params.r, 0, psi_xm));
                    result = add(result, scale(cdouble{-off, 0}, term));
                }
                // mu = 1 (y direction)
                {
                    const int yp = (y + 1) % L;
                    const int ym = (y - 1 + L) % L;
                    const int idx_yp = lattice.index(x, yp, z);
                    const int idx_ym = lattice.index(x, ym, z);
                    const cdouble U_y_n     = links.U[1][static_cast<std::size_t>(idx)];
                    const cdouble U_y_nminus = links.U[1][static_cast<std::size_t>(idx_ym)];
                    const Spinor psi_yp = scale(U_y_n, psi.at(idx_yp));
                    const Spinor psi_ym = scale(std::conj(U_y_nminus), psi.at(idx_ym));
                    Spinor term = add(apply_r_minus_gamma_spatial(params.r, 1, psi_yp),
                                      apply_r_plus_gamma_spatial(params.r, 1, psi_ym));
                    result = add(result, scale(cdouble{-off, 0}, term));
                }
                // mu = 2 (z direction)
                {
                    const int zp = (z + 1) % L;
                    const int zm = (z - 1 + L) % L;
                    const int idx_zp = lattice.index(x, y, zp);
                    const int idx_zm = lattice.index(x, y, zm);
                    const cdouble U_z_n     = links.U[2][static_cast<std::size_t>(idx)];
                    const cdouble U_z_nminus = links.U[2][static_cast<std::size_t>(idx_zm)];
                    const Spinor psi_zp = scale(U_z_n, psi.at(idx_zp));
                    const Spinor psi_zm = scale(std::conj(U_z_nminus), psi.at(idx_zm));
                    Spinor term = add(apply_r_minus_gamma_spatial(params.r, 2, psi_zp),
                                      apply_r_plus_gamma_spatial(params.r, 2, psi_zm));
                    result = add(result, scale(cdouble{-off, 0}, term));
                }

                out.at(idx) = result;
            }
        }
    }
}

void apply_wilson_dirac_dagger(SpinorField& out,
                               const SpinorField& psi,
                               const GaugeLinks& links,
                               const Lattice& lattice,
                               const WilsonDiracParams& params) {
    // D_W^dag in chiral basis: same as D_W with U <-> U^dag and gamma^mu sign flipped.
    // For gauge-invariance verification only; not on the critical path for II.2-A.
    const int L = lattice.size();
    const double diag = params.m + 3.0 * params.r / params.a;  // 3D spatial dimension shift
    const double off = 1.0 / (2.0 * params.a);

    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int idx = lattice.index(x, y, z);
                Spinor result = scale(cdouble{diag, 0}, psi.at(idx));

                // For dagger, we swap (r-gamma) and (r+gamma) and use U^dag in place of U.
                for (int mu = 0; mu < 3; ++mu) {
                    int dx = (mu == 0) ? 1 : 0;
                    int dy = (mu == 1) ? 1 : 0;
                    int dz = (mu == 2) ? 1 : 0;
                    const int xp = (x + dx + L) % L;
                    const int yp = (y + dy + L) % L;
                    const int zp = (z + dz + L) % L;
                    const int xm = (x - dx + L) % L;
                    const int ym = (y - dy + L) % L;
                    const int zm = (z - dz + L) % L;
                    const int idx_p = lattice.index(xp, yp, zp);
                    const int idx_m = lattice.index(xm, ym, zm);
                    const cdouble U_n     = links.U[mu][static_cast<std::size_t>(idx)];
                    const cdouble U_nminus = links.U[mu][static_cast<std::size_t>(idx_m)];
                    const Spinor psi_p = scale(std::conj(U_n), psi.at(idx_p));
                    const Spinor psi_m = scale(U_nminus, psi.at(idx_m));
                    Spinor term = add(apply_r_plus_gamma_spatial(params.r, mu, psi_p),
                                      apply_r_minus_gamma_spatial(params.r, mu, psi_m));
                    result = add(result, scale(cdouble{-off, 0}, term));
                }
                out.at(idx) = result;
            }
        }
    }
}

// -------------------------------------------------------------
// RK4 step for i d/dt psi = D_W psi
// -> d/dt psi = -i D_W psi
// -------------------------------------------------------------
namespace {

// out := psi + c * k
void axpy(SpinorField& out, const SpinorField& psi, double c, const SpinorField& k) {
    const std::size_t N = psi.data.size();
    for (std::size_t i = 0; i < N; ++i) {
        const Spinor& p = psi.data[i];
        const Spinor& kk = k.data[i];
        out.data[i] = {p[0] + c * kk[0], p[1] + c * kk[1], p[2] + c * kk[2], p[3] + c * kk[3]};
    }
}

// k <- -i * D_W * psi (the right-hand side of the Schrodinger equation).
void rhs(SpinorField& k,
         const SpinorField& psi,
         const GaugeLinks& links,
         const Lattice& lattice,
         const WilsonDiracParams& params) {
    apply_wilson_dirac(k, psi, links, lattice, params);
    const cdouble minus_i{0, -1};
    for (auto& s : k.data) {
        s = scale(minus_i, s);
    }
}

}  // namespace

void evolve_rk4_step(SpinorField& psi,
                     SpinorField& k1,
                     SpinorField& k_temp,
                     const GaugeLinks& links,
                     const Lattice& lattice,
                     const WilsonDiracParams& params,
                     double dt) {
    // RK4 with two scratch buffers k1, k_temp.
    // Compute k1 = f(psi); k2 = f(psi + dt/2 * k1); k3 = f(psi + dt/2 * k2);
    //         k4 = f(psi + dt * k3); psi += dt/6 * (k1 + 2 k2 + 2 k3 + k4).
    SpinorField k_accum(lattice.size());
    SpinorField k_stage(lattice.size());

    // k1
    rhs(k1, psi, links, lattice, params);
    // accumulator: dt/6 * k1
    for (std::size_t i = 0; i < k_accum.data.size(); ++i) {
        const Spinor& a = k1.data[i];
        k_accum.data[i] = {(dt / 6.0) * a[0], (dt / 6.0) * a[1],
                           (dt / 6.0) * a[2], (dt / 6.0) * a[3]};
    }

    // k2 = f(psi + dt/2 * k1)
    axpy(k_temp, psi, dt * 0.5, k1);
    rhs(k_stage, k_temp, links, lattice, params);
    for (std::size_t i = 0; i < k_accum.data.size(); ++i) {
        const Spinor& a = k_stage.data[i];
        k_accum.data[i] = {k_accum.data[i][0] + (dt / 3.0) * a[0],
                           k_accum.data[i][1] + (dt / 3.0) * a[1],
                           k_accum.data[i][2] + (dt / 3.0) * a[2],
                           k_accum.data[i][3] + (dt / 3.0) * a[3]};
    }

    // k3 = f(psi + dt/2 * k2)
    axpy(k_temp, psi, dt * 0.5, k_stage);
    rhs(k_stage, k_temp, links, lattice, params);
    for (std::size_t i = 0; i < k_accum.data.size(); ++i) {
        const Spinor& a = k_stage.data[i];
        k_accum.data[i] = {k_accum.data[i][0] + (dt / 3.0) * a[0],
                           k_accum.data[i][1] + (dt / 3.0) * a[1],
                           k_accum.data[i][2] + (dt / 3.0) * a[2],
                           k_accum.data[i][3] + (dt / 3.0) * a[3]};
    }

    // k4 = f(psi + dt * k3)
    axpy(k_temp, psi, dt, k_stage);
    rhs(k_stage, k_temp, links, lattice, params);
    for (std::size_t i = 0; i < k_accum.data.size(); ++i) {
        const Spinor& a = k_stage.data[i];
        k_accum.data[i] = {k_accum.data[i][0] + (dt / 6.0) * a[0],
                           k_accum.data[i][1] + (dt / 6.0) * a[1],
                           k_accum.data[i][2] + (dt / 6.0) * a[2],
                           k_accum.data[i][3] + (dt / 6.0) * a[3]};
    }

    // psi += k_accum
    for (std::size_t i = 0; i < psi.data.size(); ++i) {
        Spinor& p = psi.data[i];
        const Spinor& d = k_accum.data[i];
        p = {p[0] + d[0], p[1] + d[1], p[2] + d[2], p[3] + d[3]};
    }
}

// -------------------------------------------------------------
// Plane-wave initial state.
// =============================================================================
void initialize_plane_wave(SpinorField& psi,
                           const Lattice& lattice,
                           const std::array<double, 3>& momentum,
                           int spin_index) {
    const int L = lattice.size();
    // Tree-level positive-energy spinor at small momentum, chosen for definiteness:
    //   spin_index = 0: u(p, +z) ~ (1, 0, ?, ?)
    //   spin_index = 1: u(p, -z) ~ (0, 1, ?, ?)
    // For the smoke test we only check norm conservation and dispersion, so the
    // exact lower-component structure doesn't matter as long as it's consistent.
    Spinor u_template;
    if (spin_index == 0) {
        u_template = {cdouble{1, 0}, cdouble{0, 0}, cdouble{0, 0}, cdouble{0, 0}};
    } else {
        u_template = {cdouble{0, 0}, cdouble{1, 0}, cdouble{0, 0}, cdouble{0, 0}};
    }
    for (int z = 0; z < L; ++z) {
        for (int y = 0; y < L; ++y) {
            for (int x = 0; x < L; ++x) {
                const int idx = lattice.index(x, y, z);
                const double phase = momentum[0] * x + momentum[1] * y + momentum[2] * z;
                const cdouble e_iphase = std::exp(cdouble{0, phase});
                psi.at(idx) = scale(e_iphase, u_template);
            }
        }
    }
}

}  // namespace wilson_dirac
}  // namespace ftd
