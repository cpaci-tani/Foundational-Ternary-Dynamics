#include "ftd/lagrangian.h"
#include "ftd/parallel.h"
#include "ftd/volumetric_measure.h"
#include <cmath>
#include <algorithm>

namespace ftd {

namespace {

void merge_lagrangian(LagrangianDiag& into, const LagrangianDiag& from) {
    into.field_kinetic_sum += from.field_kinetic_sum;
    into.field_gradient_sum += from.field_gradient_sum;
    into.born_infeld_sum += from.born_infeld_sum;
    into.coupling_sum += from.coupling_sum;
    into.velocity_coupling_sum += from.velocity_coupling_sum;
    into.gauss_sum += from.gauss_sum;
    into.dissipation_sum += from.dissipation_sum;
    into.total_lagrangian += from.total_lagrangian;
    into.total_hamiltonian += from.total_hamiltonian;
    into.gauss_violation += from.gauss_violation;
    into.max_gauss_error = std::max(into.max_gauss_error, from.max_gauss_error);
    into.total_flux_mag += from.total_flux_mag;
    into.total_wave_energy += from.total_wave_energy;
    into.manifested_count += from.manifested_count;
    into.locked_count += from.locked_count;
}

}  // namespace

LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb) {
    LagrangianDiag d;
    if (rb.copy_compact_lagrangian(d)) return d;
    const int N = static_cast<int>(rb.lattice().total_sites());
    const auto& voxels = rb.voxels();

    // Fixed chunk identities make the floating-point reduction order
    // independent of the active parallel backend and thread count.  Serial
    // WASM, the pthread WASM build, and native OpenMP therefore merge the same
    // partial sums in the same order.
    constexpr int CHUNK_SIZE = 2048;
    const int chunk_count = (N + CHUNK_SIZE - 1) / CHUNK_SIZE;
    std::vector<LagrangianDiag> chunks(static_cast<std::size_t>(chunk_count));
    parallel_for(0, chunk_count, [&](int chunk_lo, int chunk_hi) {
      for (int chunk = chunk_lo; chunk < chunk_hi; ++chunk) {
        LagrangianDiag& local = chunks[static_cast<std::size_t>(chunk)];
        const int begin = chunk * CHUNK_SIZE;
        const int end = std::min(N, begin + CHUNK_SIZE);
        for (int i = begin; i < end; ++i) {
          const auto& v = voxels[i];

        // Compute field quantities at this site
        double divJ = rb.divergence_flux(i);
        double rho = static_cast<double>(v.state);

        // --- Field-sector terms (the wave equation's energy) ---
        double fk = field_kinetic_term(v.wave_vel);
        // Global pairs-once gradient: visit one orientation from every
        // periodic reversal pair (3 faces + 6 plane diagonals).  The -1/2
        // coefficient gives the same action as the public per-site
        // half-share's directed 18-neighbor sum with coefficient -1/4.
        // Degenerate L=1/L=2 quotients retain their oriented multiplicities.
        double grad_sq = 0.0;
        const auto face = rb.lattice().neighbors_6(i);
        const auto edge = rb.lattice().neighbors_12(i);
        for (int slot : {0, 2, 4}) {
            const Vec3 delta = voxels[face[slot]].flux - v.flux;
            grad_sq += (1.0 / 3.0) * delta.mag2();
        }
        for (int slot : {0, 1, 4, 5, 8, 9}) {
            const Vec3 delta = voxels[edge[slot]].flux - v.flux;
            grad_sq += (1.0 / 6.0) * delta.mag2();
        }
        const double fg = -0.5 * (C_WAVE * C_WAVE) * grad_sq;

        // --- Kinematic/interaction diagnostic terms (4 terms) ---
        // The Born core is currently state-independent and evaluated at every
        // voxel; this is not a genesis energy payment (FTD-0567).
        double bi       = born_infeld_term(v);
        double coup     = coupling_term(v, divJ);
        double vel_coup = velocity_coupling_term(v);
        double gauss    = gauss_term(divJ, rho);
        double dissip   = rayleigh_dissipation(v);

        // Accumulate per-term sums
        local.field_kinetic_sum     += integrate_voxel_density(fk);
        local.field_gradient_sum    += integrate_voxel_density(fg);
        local.born_infeld_sum       += integrate_voxel_density(bi);
        local.coupling_sum          += integrate_voxel_density(coup);
        local.velocity_coupling_sum += integrate_voxel_density(vel_coup);
        local.gauss_sum             += integrate_voxel_density(gauss);
        local.dissipation_sum       += integrate_voxel_density(dissip);

        // Complete Lagrangian = field sector + interaction sector
        double L_site = fk + fg + bi + coup + vel_coup + gauss;
        local.total_lagrangian += integrate_voxel_density(L_site);
        local.total_hamiltonian += integrate_voxel_density(
            hamiltonian_density(v, divJ, rho));

        // Gauss constraint violation
        double gauss_v = divJ - rho;
        local.gauss_violation += gauss_v * gauss_v;
        local.max_gauss_error = std::max(local.max_gauss_error, std::abs(gauss_v));

        // Conservation checks
        local.total_flux_mag += v.density();
        local.total_wave_energy += integrate_voxel_density(
            quadratic_field_energy_density(v.wave_vel.mag2()));

        // Counters
        if (v.state != 0) {
            local.manifested_count++;
            if (v.locked) local.locked_count++;
        }
        }
      }
    });

    for (const auto& chunk : chunks) {
        merge_lagrangian(d, chunk);
    }

    // Discrete action = volume-integrated Lagrangian (one time slice).
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
        // Independently replay what phase_read() should produce. FTD-0574
        // derives this from the free discrete action plus the prescribed-
        // source functional +G_C<s,div J>+G_C<curl J,s v>. It is not the
        // J-variation of lagrangian_density()'s onsite velocity term.
        //   delta_j = c²∇²J − g_c·∇(s) + g_c·∇×(s·v)
        // (electric source sign per lagrangian.h Term 2, amended 2026-07-18)
        Vec3 expected = rb.laplacian_flux(i) * (C_WAVE * C_WAVE);
        expected -= rb.gradient_state(i) * G_C;
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

        // Independently replay the selected production EM formula. FTD-0467
        // proves that neither branch below is the matter-side variation of the
        // written +G_C*s*div(J) interaction.
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
        // Selected matter-side onsite velocity interaction replay. FTD-0574
        // proves this is not the common-action partner of phase_read's
        // +G_C*curl(s*v) source; the α normalization is selected separately.
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
