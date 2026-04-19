/**
 * Scratch probe for debugging the matched_poisson implementation.
 * NOT part of the regular test suite; can be deleted later.
 */
#include <cmath>
#include <cstdio>
#include "ftd/eft/matched_poisson.h"
#include "ftd/render_bridge.h"
#include "ftd/field_operators.h"

int main() {
    const int L = 16;
    ftd::RenderBridge rb(L);
    rb.toggles.gauss_projection = false;

    // Inject a single +1 with nonzero flux
    rb.inject_particle(L/2, L/2, L/2, +1, {0.1, 0.0, 0.0});

    // Sanity check: read the voxel at (L/2, L/2, L/2)
    int idx = rb.lattice().index(L/2, L/2, L/2);
    auto& v = rb.voxels()[idx];
    std::printf("Particle voxel: state=%d  flux=(%f, %f, %f)\n",
                (int)v.state, v.flux.x, v.flux.y, v.flux.z);

    // Compute divergence at several vacuum voxels using engine's operator
    int idx_next = rb.lattice().index(L/2 + 1, L/2, L/2);
    double div_eng = rb.divergence_flux(idx_next);
    double div_6pt = ftd::eft::detail::divergence_6pt(rb.voxels(), rb.lattice(), idx_next);
    std::printf("At (L/2+1, L/2, L/2) [state=%d, flux=(%f,%f,%f)]:\n",
                (int)rb.voxels()[idx_next].state,
                rb.voxels()[idx_next].flux.x,
                rb.voxels()[idx_next].flux.y,
                rb.voxels()[idx_next].flux.z);
    std::printf("  engine divergence_flux() = %e\n", div_eng);
    std::printf("  matched_poisson::divergence_6pt() = %e\n", div_6pt);

    // Also check at the particle voxel itself
    double div_p_eng = rb.divergence_flux(idx);
    double div_p_6pt = ftd::eft::detail::divergence_6pt(rb.voxels(), rb.lattice(), idx);
    std::printf("At particle voxel (L/2, L/2, L/2):\n");
    std::printf("  engine divergence_flux() = %e\n", div_p_eng);
    std::printf("  matched_poisson::divergence_6pt() = %e\n", div_p_6pt);

    // Sweep all vacuum voxels, find max
    const int N = rb.lattice().total_sites();
    double max_div_vac = 0.0;
    int argmax = -1;
    for (int i = 0; i < N; ++i) {
        if (rb.voxels()[i].state != 0) continue;
        double d = ftd::eft::detail::divergence_6pt(rb.voxels(), rb.lattice(), i);
        if (std::abs(d) > max_div_vac) { max_div_vac = std::abs(d); argmax = i; }
    }
    std::printf("Max vacuum |div|: %e at idx=%d\n", max_div_vac, argmax);

    // -----------------------------------------
    // Reproduce M2 exactly
    // -----------------------------------------
    std::puts("\n--- Replicating M2 configuration ---");
    ftd::RenderBridge rb2(L);
    rb2.toggles.gauss_projection = false;
    rb2.inject_particle(L/2 - 2, L/2, L/2, +1, {0.05, 0.03, -0.02});
    rb2.inject_particle(L/2 + 2, L/2, L/2, -1, {-0.05, -0.03, 0.02});
    rb2.inject_flux(L/2, L/2, L/2, {0.1, 0, 0});
    rb2.inject_flux(L/2, L/2 + 3, L/2, {0, 0.1, 0});

    // Raw vacuum divergence scan
    double m2_max = 0.0;
    int m2_argmax = -1;
    for (int i = 0; i < N; ++i) {
        if (rb2.voxels()[i].state != 0) continue;
        double d = ftd::eft::detail::divergence_6pt(rb2.voxels(), rb2.lattice(), i);
        if (std::abs(d) > m2_max) { m2_max = std::abs(d); m2_argmax = i; }
    }
    std::printf("M2 config: max vacuum |div| = %e at idx %d\n", m2_max, m2_argmax);

    // Now invoke matched_gauss_project and see what it reports
    auto rpt = ftd::eft::matched_gauss_project(rb2, 1e-10, 400);
    std::printf("After matched_gauss_project:\n");
    std::printf("  CG iter: %d  res: %e\n", rpt.iterations, rpt.final_residual_norm);
    std::printf("  vac_max_div:  before=%e  after=%e\n",
                rpt.vacuum_max_div_before, rpt.vacuum_max_div_after);

    return 0;
}
