// FTD-0285: fixed no-alpha-input alpha probe.
//
// This campaign distinguishes native geometric Coulomb response from an
// explicit Postulate-W matching coupling. The native arm disables the separate
// state-flux coupling toggle that injects sqrt(alpha) through phase_read.

#include "ftd/ontic.h"
#include "ftd/render_bridge.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>

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
    double g_dyn_sq = 0.0;
    double rel_err = 0.0;
};

struct ArmResult {
    const char* label = "";
    const char* classification = "";
    double charge_coupling = 1.0;
    double expected_g_sq = 1.0;
    std::array<Row, R_VALUES.size()> rows{};
    double mean_g_dyn_sq = 0.0;
    double mean_rel_err = 0.0;
    double max_rel_err = 0.0;
    bool pass = false;
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

void configure_no_alpha_lattice(ftd::RenderBridge& rb, double charge_coupling) {
    rb.force_cpu();
    rb.set_sor_iterations(SOR_ITERATIONS);

    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = false;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = false;
    rb.toggles.damping = false;
    rb.toggles.evaporation = false;
    rb.toggles.forces = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.gravity = false;
    rb.toggles.movement = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.color_forces = false;
    rb.toggles.strong_force = false;
    rb.toggles.triad_binding = false;
    rb.toggles.pair_production = false;
    rb.toggles.exchange_force = false;
    rb.toggles.selective_damping = false;
    rb.toggles.larmor_radiation = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.latency_field = false;
    rb.toggles.exact_dual_gauss = false;
    rb.toggles.emergent_forces = false;
    rb.toggles.langevin = false;
    rb.toggles.symplectic_leapfrog = false;
    rb.toggles.absorbing_boundary = false;
    rb.toggles.field_energy_gravity = false;
    rb.toggles.cluster_inertia = false;
    rb.toggles.de_broglie_clock = false;
    rb.toggles.db_clock_coulomb = false;
    rb.toggles.confinement = false;
    rb.toggles.coulomb_charge_coupling = charge_coupling;
}

double energy_with(int sign_a, int sign_b, int sep, double charge_coupling) {
    ftd::RenderBridge rb(L);
    configure_no_alpha_lattice(rb, charge_coupling);

    const int mid = L / 2;
    const double init_flux = 0.05 * charge_coupling;

    if (sign_a != 0) {
        rb.inject_particle(mid, mid, mid, static_cast<int8_t>(sign_a),
                           {0.0, 0.0, static_cast<double>(sign_a) * init_flux});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
    }
    if (sign_b != 0) {
        rb.inject_particle(mid + sep, mid, mid, static_cast<int8_t>(sign_b),
                           {0.0, 0.0, static_cast<double>(sign_b) * init_flux});
        rb.voxels()[rb.lattice().index(mid + sep, mid, mid)].locked = true;
    }

    rb.run(TICKS);
    return rb.energy_audit().field_energy;
}

ArmResult run_arm(const char* label,
                  const char* classification,
                  double charge_coupling,
                  double expected_g_sq) {
    ArmResult out;
    out.label = label;
    out.classification = classification;
    out.charge_coupling = charge_coupling;
    out.expected_g_sq = expected_g_sq;

    const double e_self_pos = energy_with(+1, 0, 0, charge_coupling);
    const double e_self_neg = energy_with(-1, 0, 0, charge_coupling);

    double sum_g = 0.0;
    double sum_err = 0.0;
    double max_err = 0.0;
    for (std::size_t i = 0; i < R_VALUES.size(); ++i) {
        const int r = R_VALUES[i];
        const double e_pair = energy_with(+1, -1, r, charge_coupling);
        const double V = e_pair - e_self_pos - e_self_neg;
        const double alpha_r = -V * static_cast<double>(r);
        const double phase_g_unit = 2.0 * static_cast<double>(r) *
                                    lattice_greens_function(L, r);
        const double alpha_phase_g = expected_g_sq * phase_g_unit;
        const double g_dyn_sq = alpha_r / phase_g_unit;
        const double rel_err = std::abs(alpha_r - alpha_phase_g) /
                               std::max(std::abs(alpha_phase_g), 1e-30);

        out.rows[i] = {r, alpha_r, alpha_phase_g, g_dyn_sq, rel_err};
        sum_g += g_dyn_sq;
        sum_err += rel_err;
        max_err = std::max(max_err, rel_err);
    }

    out.mean_g_dyn_sq = sum_g / static_cast<double>(R_VALUES.size());
    out.mean_rel_err = sum_err / static_cast<double>(R_VALUES.size());
    out.max_rel_err = max_err;
    out.pass = out.max_rel_err < REL_TOL &&
               std::abs(out.mean_g_dyn_sq - expected_g_sq) /
                   std::max(std::abs(expected_g_sq), 1e-30) < REL_TOL;
    return out;
}

void print_arm(const ArmResult& arm) {
    std::printf("arm,%s,%s,charge_coupling,%.15g,expected_g_sq,%.15g\n",
                arm.label, arm.classification, arm.charge_coupling,
                arm.expected_g_sq);
    std::printf("row,%s,r,alpha_r,phase_g_expected,g_dyn_sq,rel_err\n",
                arm.label);
    for (const Row& row : arm.rows) {
        std::printf("row,%s,%d,%.15g,%.15g,%.15g,%.15g\n",
                    arm.label, row.r, row.alpha_r, row.alpha_phase_g,
                    row.g_dyn_sq, row.rel_err);
    }
    std::printf("summary,%s,mean_g_dyn_sq,%.15g,mean_rel_err,%.15g,max_rel_err,%.15g,pass,%s\n",
                arm.label, arm.mean_g_dyn_sq, arm.mean_rel_err,
                arm.max_rel_err, arm.pass ? "true" : "false");
}

}  // namespace

int main() {
    const double alpha_tree = ftd::ontic::ALPHA_TREE;
    const double postulate_w_g_sq = 2.0 * PI * alpha_tree;
    const double postulate_w_g = std::sqrt(postulate_w_g_sq);

    std::printf("FTD-0285 alpha no-alpha engine probe v1\n");
    std::printf("protocol,L,%d,ticks,%d,sor,%d,r_values,5|7|9,rel_tol,%.6g\n",
                L, TICKS, SOR_ITERATIONS, REL_TOL);
    std::printf("leak_guard,coupling,false,coulomb_charge_coupling_native,1.0,forces,false,poisson_coulomb,false,lorentz_force,false\n");
    std::printf("positive_control,postulate_w_g,%.15g,postulate_w_g_sq,%.15g,alpha_tree_input,%.15g\n",
                postulate_w_g, postulate_w_g_sq, alpha_tree);

    const ArmResult native = run_arm("native_unit", "NO_ALPHA_INPUT", 1.0, 1.0);
    const ArmResult control = run_arm("postulate_w_control",
                                      "EXPLICIT_MASTER_QUADRATIC_MATCHING_INPUT",
                                      postulate_w_g, postulate_w_g_sq);
    print_arm(native);
    print_arm(control);

    const double native_alpha_rel = std::abs(native.mean_g_dyn_sq - postulate_w_g_sq) /
                                    std::max(std::abs(postulate_w_g_sq), 1e-30);
    const bool native_postulate_w_match = native_alpha_rel < REL_TOL;

    std::printf("adjudication,native_phase_g_unit_match,%s,native_postulate_w_match,%s,native_alpha_rel,%.15g,control_match,%s\n",
                native.pass ? "true" : "false",
                native_postulate_w_match ? "true" : "false",
                native_alpha_rel,
                control.pass ? "true" : "false");

    const bool valid_verdict = control.pass &&
                               (native.pass || native_postulate_w_match);
    const char* verdict = "UNCLASSIFIED_NATIVE_RESPONSE";
    if (control.pass && native.pass) {
        verdict = "NATIVE_NULL_WITH_POSTULATE_W_CONTROL";
    } else if (control.pass && native_postulate_w_match) {
        verdict = "DYNAMICAL_FOUND_CANDIDATE_REQUIRES_LEAK_AUDIT";
    } else if (!control.pass) {
        verdict = "INVALIDATED_PROTOCOL_OR_ENGINE_DRIFT";
    }
    std::printf("verdict,%s\n", verdict);
    return valid_verdict ? EXIT_SUCCESS : EXIT_FAILURE;
}
