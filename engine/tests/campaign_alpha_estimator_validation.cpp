// FTD-0286: alpha estimator validation after FTD-0285 invalidated.
//
// This campaign is not an alpha recovery attempt. It compares the production
// live-tick Gauss path with the matched-stencil projection tool already used by
// the EFT program, on the same fixed finite-cell window.

#include "ftd/eft/matched_poisson.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

constexpr int L = 32;
constexpr int TICKS = 300;
constexpr int SOR_ITERATIONS = 100;
constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double REL_TOL = 0.10;
constexpr std::array<int, 3> R_VALUES = {5, 7, 9};

struct Row {
    int r = 0;
    double alpha_r = 0.0;
    double alpha_phase_g = 0.0;
    double rel_err = 0.0;
};

struct ModeResult {
    const char* label = "";
    std::array<Row, R_VALUES.size()> rows{};
    double mean_alpha = 0.0;
    double mean_phase_g = 0.0;
    double mean_rel_err = 0.0;
    double max_rel_err = 0.0;
    double mean_ratio = 0.0;
    bool absolute_gate = false;
    bool projection_converged = true;
    double worst_deep_vacuum_after = 0.0;
};

double lattice_greens_function(int lattice_size, int r) {
    double G = 0.0;
    const double twopi_L = 2.0 * PI / static_cast<double>(lattice_size);
    for (int kx = 0; kx < lattice_size; ++kx) {
        for (int ky = 0; ky < lattice_size; ++ky) {
            for (int kz = 0; kz < lattice_size; ++kz) {
                if (kx == 0 && ky == 0 && kz == 0) {
                    continue;
                }
                const double sx = std::sin(twopi_L * kx * 0.5);
                const double sy = std::sin(twopi_L * ky * 0.5);
                const double sz = std::sin(twopi_L * kz * 0.5);
                const double lambda = 4.0 * (sx * sx + sy * sy + sz * sz);
                const double phase = twopi_L * kx * r;
                G += std::cos(phase) / lambda;
            }
        }
    }
    const double volume = static_cast<double>(lattice_size) *
                          static_cast<double>(lattice_size) *
                          static_cast<double>(lattice_size);
    return G / volume;
}

void configure_leak_guard(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.set_sor_iterations(SOR_ITERATIONS);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.coupling = false;
    rb.toggles.damping = false;
    rb.toggles.coulomb_charge_coupling = 1.0;
}

void place_charges(ftd::RenderBridge& rb, int sign_a, int sign_b, int sep,
                   double init_flux_scale) {
    const int mid = L / 2;
    if (sign_a != 0) {
        rb.inject_particle(mid, mid, mid, static_cast<int8_t>(sign_a),
                           {0.0, 0.0, static_cast<double>(sign_a) * init_flux_scale});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    }
    if (sign_b != 0) {
        rb.inject_particle(mid + sep, mid, mid, static_cast<int8_t>(sign_b),
                           {0.0, 0.0, static_cast<double>(sign_b) * init_flux_scale});
        rb.voxels()[rb.lattice().index(mid + sep, mid, mid)].locked = true;
    }
}

double production_energy_with(int sign_a, int sign_b, int sep) {
    ftd::RenderBridge rb(L);
    configure_leak_guard(rb);
    place_charges(rb, sign_a, sign_b, sep, 0.05);
    rb.run(TICKS);
    return rb.energy_audit().field_energy;
}

double matched_energy_with(int sign_a, int sign_b, int sep,
                           bool& converged,
                           double& deep_vacuum_after) {
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.disable_all();
    place_charges(rb, sign_a, sign_b, sep, 0.0);
    auto rpt = ftd::eft::matched_gauss_project(rb, 1e-10, 400);
    converged = rpt.converged;
    deep_vacuum_after = rpt.deep_vacuum_max_div_after;
    return rb.energy_audit().field_energy;
}

ModeResult run_mode(const char* label, bool matched) {
    ModeResult out;
    out.label = label;

    bool conv = true;
    double deep_after = 0.0;

    const double e_self_pos = matched
        ? matched_energy_with(+1, 0, 0, conv, deep_after)
        : production_energy_with(+1, 0, 0);
    out.projection_converged = out.projection_converged && conv;
    out.worst_deep_vacuum_after = std::max(out.worst_deep_vacuum_after, deep_after);

    const double e_self_neg = matched
        ? matched_energy_with(-1, 0, 0, conv, deep_after)
        : production_energy_with(-1, 0, 0);
    out.projection_converged = out.projection_converged && conv;
    out.worst_deep_vacuum_after = std::max(out.worst_deep_vacuum_after, deep_after);

    double sum_alpha = 0.0;
    double sum_phase = 0.0;
    double sum_err = 0.0;
    double sum_ratio = 0.0;
    double max_err = 0.0;

    for (std::size_t i = 0; i < R_VALUES.size(); ++i) {
        const int r = R_VALUES[i];
        const double e_pair = matched
            ? matched_energy_with(+1, -1, r, conv, deep_after)
            : production_energy_with(+1, -1, r);
        out.projection_converged = out.projection_converged && conv;
        out.worst_deep_vacuum_after = std::max(out.worst_deep_vacuum_after, deep_after);

        const double V = e_pair - e_self_pos - e_self_neg;
        const double alpha_r = -V * static_cast<double>(r);
        const double phase_g = 2.0 * static_cast<double>(r) *
                               lattice_greens_function(L, r);
        const double rel_err = std::abs(alpha_r - phase_g) /
                               std::max(std::abs(phase_g), 1e-30);
        out.rows[i] = {r, alpha_r, phase_g, rel_err};
        sum_alpha += alpha_r;
        sum_phase += phase_g;
        sum_err += rel_err;
        sum_ratio += alpha_r / phase_g;
        max_err = std::max(max_err, rel_err);
    }

    out.mean_alpha = sum_alpha / static_cast<double>(R_VALUES.size());
    out.mean_phase_g = sum_phase / static_cast<double>(R_VALUES.size());
    out.mean_rel_err = sum_err / static_cast<double>(R_VALUES.size());
    out.max_rel_err = max_err;
    out.mean_ratio = sum_ratio / static_cast<double>(R_VALUES.size());
    out.absolute_gate = out.projection_converged && out.max_rel_err < REL_TOL;
    return out;
}

void print_mode(const ModeResult& mode) {
    std::printf("mode,%s,projection_converged,%s,worst_deep_vacuum_after,%.15g\n",
                mode.label, mode.projection_converged ? "true" : "false",
                mode.worst_deep_vacuum_after);
    std::printf("row,%s,r,alpha_r,phase_g_expected,rel_err\n", mode.label);
    for (const Row& row : mode.rows) {
        std::printf("row,%s,%d,%.15g,%.15g,%.15g\n",
                    mode.label, row.r, row.alpha_r, row.alpha_phase_g,
                    row.rel_err);
    }
    std::printf("summary,%s,mean_alpha,%.15g,mean_phase_g,%.15g,mean_ratio,%.15g,mean_rel_err,%.15g,max_rel_err,%.15g,absolute_gate,%s\n",
                mode.label, mode.mean_alpha, mode.mean_phase_g,
                mode.mean_ratio, mode.mean_rel_err, mode.max_rel_err,
                mode.absolute_gate ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0286 alpha estimator validation v1\n");
    std::printf("protocol,L,%d,r_values,5|7|9,production_ticks,%d,production_sor,%d,matched_tol,1e-10,matched_max_iter,400,rel_tol,%.6g\n",
                L, TICKS, SOR_ITERATIONS, REL_TOL);
    std::printf("scope,estimator_validation_not_alpha_recovery\n");
    std::printf("leak_guard,production_coupling,false,production_charge_coupling,1.0\n");

    const ModeResult production = run_mode("production_live_tick", false);
    const ModeResult matched = run_mode("matched_static_projector", true);

    print_mode(production);
    print_mode(matched);

    const char* verdict = "UNCLASSIFIED";
    if (!production.absolute_gate && matched.absolute_gate) {
        verdict = "MATCHED_ESTIMATOR_CONFIRMED_PRODUCTION_GATE_INVALID";
    } else if (!production.absolute_gate && !matched.absolute_gate &&
               matched.projection_converged) {
        verdict = "ENERGY_FUNCTIONAL_MISMATCH";
    } else if (production.absolute_gate && matched.absolute_gate) {
        verdict = "PRODUCTION_AND_MATCHED_GATES_CONFIRMED";
    } else if (!matched.projection_converged) {
        verdict = "MATCHED_PROJECTOR_FAILED_TO_CONVERGE";
    }

    std::printf("verdict,%s\n", verdict);

    const bool valid = (std::string(verdict) != "UNCLASSIFIED");
    return valid ? EXIT_SUCCESS : EXIT_FAILURE;
}
