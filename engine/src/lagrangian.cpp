#include "ftd/lagrangian.h"
#include <cmath>
#include <algorithm>

namespace ftd {

LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb) {
    LagrangianDiag d;
    const int N = static_cast<int>(rb.lattice().total_sites());
    const auto& voxels = rb.voxels();

    for (int i = 0; i < N; ++i) {
        const auto& v = voxels[i];

        // Compute field quantities at this site
        double divJ = rb.divergence_flux(i);
        double rho = static_cast<double>(v.state);

        // --- Field-sector terms (the wave equation's energy) ---
        double fk = field_kinetic_term(v.wave_vel);
        double fg = field_gradient_term(v.flux,
                        rb.lattice().neighbors_6(i),
                        rb.lattice().neighbors_12(i),
                        voxels);

        // --- Interaction-sector terms (4 terms) ---
        double bi       = born_infeld_term(v);
        double coup     = coupling_term(v, divJ);
        double vel_coup = velocity_coupling_term(v);
        double gauss    = gauss_term(divJ, rho);
        double dissip   = rayleigh_dissipation(v);

        // Accumulate per-term sums
        d.field_kinetic_sum     += fk;
        d.field_gradient_sum    += fg;
        d.born_infeld_sum       += bi;
        d.coupling_sum          += coup;
        d.velocity_coupling_sum += vel_coup;
        d.gauss_sum             += gauss;
        d.dissipation_sum       += dissip;

        // Complete Lagrangian = field sector + interaction sector
        double L_site = fk + fg + bi + coup + vel_coup + gauss;
        d.total_lagrangian += L_site;
        d.total_hamiltonian += hamiltonian_density(v, divJ, rho);

        // Gauss constraint violation
        double gauss_v = divJ - rho;
        d.gauss_violation += gauss_v * gauss_v;
        d.max_gauss_error = std::max(d.max_gauss_error, std::abs(gauss_v));

        // Conservation checks
        d.total_flux_mag += v.density();
        d.total_wave_energy += v.wave_vel.mag2() * 0.5;

        // Counters
        if (v.state != 0) {
            d.manifested_count++;
            if (v.locked) d.locked_count++;
        }
    }

    // Discrete action = total Lagrangian (single time-slice contribution)
    d.total_action = d.total_lagrangian;

    return d;
}

ELResidual compute_el_residual(const RenderBridge& rb) {
    ELResidual res;
    const int N = static_cast<int>(rb.lattice().total_sites());
    const auto& stored = rb.delta_j();
    double sum_sq = 0.0;

    // If delta_j hasn't been populated yet, return zeros
    if (static_cast<int>(stored.size()) != N) return res;

    for (int i = 0; i < N; ++i) {
        // Independently recompute what phase_read() should produce:
        //   delta_j = c²∇²J + g_c·∇(s) + g_c·∇×(s·v)
        Vec3 expected = rb.laplacian_flux(i) * (C_WAVE * C_WAVE);
        expected += rb.gradient_state(i) * G_C;
        expected += rb.curl_state_velocity(i) * G_C;

        // Residual = stored - expected
        Vec3 diff = stored[i] - expected;
        double mag2 = diff.mag2();
        sum_sq += mag2;
        double mag = std::sqrt(mag2);
        if (mag > res.max_abs) res.max_abs = mag;
    }

    res.rms = std::sqrt(sum_sq / N);
    return res;
}

ParticleELResidual compute_particle_el_residual(const RenderBridge& rb) {
    ParticleELResidual res;
    const int N = static_cast<int>(rb.lattice().total_sites());
    const auto& voxels = rb.voxels();
    const auto& fd = rb.force_diag();
    double sum_sq = 0.0;

    // If force_diag hasn't been populated yet, return zeros
    if (static_cast<int>(fd.size()) != N) return res;

    for (int i = 0; i < N; ++i) {
        const auto& v = voxels[i];
        if (v.state == 0) continue;

        res.particle_count++;

        // ── Independently recompute EM force from Lagrangian ──
        // L_coupling = -g_c·s·(∇·J) → F_EM = -α·s·∇(φ_C) [Poisson]
        //                              or   = -α·s·∇(∇·J)  [legacy]
        Vec3 expected_em;
        if (rb.toggles.poisson_coulomb) {
            Vec3 grad_phi = rb.gradient_scalar(i, rb.phi_coulomb());
            expected_em = grad_phi * (-ALPHA * v.state);
        } else {
            Vec3 grad_divJ = rb.gradient_divergence(i);
            expected_em = grad_divJ * (-ALPHA * v.state);
        }

        // ── Independently recompute gravity force ──
        // L_grav contribution → F_grav = G_N·∇ρ (tier-2 stencil, r=2)
        Vec3 expected_grav;
        if (rb.toggles.gravity) {
            auto c = rb.lattice().coord(i);
            double dx = voxels[rb.lattice().index(c.x + 2, c.y, c.z)].density() -
                        voxels[rb.lattice().index(c.x - 2, c.y, c.z)].density();
            double dy = voxels[rb.lattice().index(c.x, c.y + 2, c.z)].density() -
                        voxels[rb.lattice().index(c.x, c.y - 2, c.z)].density();
            double dz = voxels[rb.lattice().index(c.x, c.y, c.z + 2)].density() -
                        voxels[rb.lattice().index(c.x, c.y, c.z - 2)].density();
            Vec3 grad_rho = {dx * GRAD_TIER2_SCALE, dy * GRAD_TIER2_SCALE,
                             dz * GRAD_TIER2_SCALE};
            expected_grav = grad_rho * G_N;
        }

        // ── Independently recompute Lorentz force ──
        // L_velocity = -g_c·s·(v·J) → F_mag = α·s·(v × ∇×J)
        Vec3 expected_lorentz;
        if (rb.toggles.lorentz_force && v.speed() > EPSILON_MAG) {
            Vec3 B = rb.curl_flux(i);
            expected_lorentz = Vec3::cross(v.velocity, B) * (ALPHA * v.state);
        }

        // ── Compare against stored force_diag ──
        Vec3 diff_em = fd[i].f_coulomb - expected_em;
        Vec3 diff_grav = fd[i].f_gravity - expected_grav;
        Vec3 diff_lorentz = fd[i].f_magnetic - expected_lorentz;

        double mag2 = diff_em.mag2() + diff_grav.mag2() + diff_lorentz.mag2();
        sum_sq += mag2;
        double mag = std::sqrt(mag2);
        if (mag > res.max_abs) res.max_abs = mag;
    }

    if (res.particle_count > 0) {
        res.rms = std::sqrt(sum_sq / res.particle_count);
    }
    return res;
}

}  // namespace ftd
