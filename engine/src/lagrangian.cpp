#include "ftd/lagrangian.h"
#include <cmath>
#include <algorithm>

namespace ftd {

LagrangianDiag compute_lagrangian_diagnostics(const RenderBridge& rb) {
    LagrangianDiag d;
    const int N = rb.lattice().total_sites();
    const auto& voxels = rb.voxels();

    for (int i = 0; i < N; ++i) {
        const auto& v = voxels[i];

        // Compute field quantities at this site
        double divJ = rb.divergence_flux(i);
        double rho = static_cast<double>(v.state);

        // Per-term Lagrangian computation (4 active terms only)
        double bi       = born_infeld_term(v);
        double coup     = coupling_term(v, divJ);
        double vel_coup = velocity_coupling_term(v);
        double gauss    = gauss_term(divJ, rho);
        double dissip   = rayleigh_dissipation(v);

        // Accumulate per-term sums
        d.born_infeld_sum       += bi;
        d.coupling_sum          += coup;
        d.velocity_coupling_sum += vel_coup;
        d.gauss_sum             += gauss;
        d.dissipation_sum       += dissip;

        // Total Lagrangian = sum of 4 active terms
        d.total_lagrangian += bi + coup + vel_coup + gauss;
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

    return d;
}

}  // namespace ftd
