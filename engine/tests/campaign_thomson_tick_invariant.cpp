/**
 * FTD-0292: Source-free discrete tick energy invariant.
 *
 * Follow-up to FTD-0291. The first native finite-volume current candidate
 * failed the free-wave control because it used a continuum-time balance on a
 * discrete symplectic-Euler tick. This fixed campaign measures the exact
 * modified quadratic invariant of the actual source-free update:
 *
 *     W' = W + c^2 L J
 *     J' = J + W'
 *
 * For K = -c^2 L, each mode preserves
 *
 *     E_tick = 0.5 W^2 + 0.5 J K J - 0.5 W K J
 *
 * equivalently, using delta = c^2 L J:
 *
 *     E_tick = 0.5 W^2 + E_grad + 0.5 W dot delta.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a radiation claim. It is the source-free invariant control needed
 *   before deriving any local finite-volume current.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

constexpr int L = 33;
constexpr int TICKS = 200;
constexpr int MODE_N = 4;
constexpr double AMP = 0.05;
constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double MODIFIED_ABS_GATE = 1e-10;
constexpr double MODIFIED_REL_GATE = 1e-12;
constexpr double NAIVE_DRIFT_GATE = 1e-6;

struct EdgeOffset {
    int dx = 0;
    int dy = 0;
    int dz = 0;
    double weight = 0.0;
};

struct Energy {
    double kinetic = 0.0;
    double gradient = 0.0;
    double cross = 0.0;
    double naive = 0.0;
    double modified = 0.0;
    bool finite = true;
};

struct Drift {
    double initial_naive = 0.0;
    double initial_modified = 0.0;
    double final_naive = 0.0;
    double final_modified = 0.0;
    double max_abs_naive_drift = 0.0;
    double max_rel_naive_drift = 0.0;
    double max_abs_modified_drift = 0.0;
    double max_rel_modified_drift = 0.0;
    int max_naive_tick = 0;
    int max_modified_tick = 0;
    bool finite = true;
};

bool finite_value(double x) {
    return std::isfinite(x);
}

std::vector<EdgeOffset> stencil_offsets() {
    std::vector<EdgeOffset> out;
    out.reserve(18);
    const double wf = ftd::LAPLACIAN_FACE_WEIGHT;
    const double we = ftd::LAPLACIAN_EDGE_WEIGHT;
    out.push_back({1, 0, 0, wf});
    out.push_back({-1, 0, 0, wf});
    out.push_back({0, 1, 0, wf});
    out.push_back({0, -1, 0, wf});
    out.push_back({0, 0, 1, wf});
    out.push_back({0, 0, -1, wf});
    out.push_back({1, 1, 0, we});
    out.push_back({1, -1, 0, we});
    out.push_back({-1, 1, 0, we});
    out.push_back({-1, -1, 0, we});
    out.push_back({1, 0, 1, we});
    out.push_back({1, 0, -1, we});
    out.push_back({-1, 0, 1, we});
    out.push_back({-1, 0, -1, we});
    out.push_back({0, 1, 1, we});
    out.push_back({0, 1, -1, we});
    out.push_back({0, -1, 1, we});
    out.push_back({0, -1, -1, we});
    return out;
}

void configure_source_free(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(2812);
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = false;
    rb.toggles.damping = false;
    rb.toggles.genesis = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.gravity = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.dual_substrate = false;
    rb.toggles.pair_production = false;
    rb.toggles.weak_transmutation = false;
    rb.toggles.forces = false;
    rb.toggles.movement = false;
    rb.toggles.emergent_forces = false;
    rb.toggles.symmetric_movement_order = false;
    rb.toggles.symplectic_leapfrog = false;
}

void inject_plane_wave(ftd::RenderBridge& rb) {
    const double k = 2.0 * PI * static_cast<double>(MODE_N) / static_cast<double>(L);
    const double omega = 2.0 * ftd::C_SPEED * std::abs(std::sin(k * 0.5));
    auto& voxels = rb.voxels();
    for (int x = 0; x < L; ++x) {
        const double jy = AMP * std::sin(k * static_cast<double>(x));
        const double wy = -omega * AMP * std::cos(k * static_cast<double>(x));
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                auto& v = voxels[rb.lattice().index(x, y, z)];
                v.flux.y += jy;
                v.wave_vel.y += wy;
            }
        }
    }
}

ftd::Vec3 laplacian_from_flux(const std::vector<ftd::Voxel>& voxels,
                              const ftd::Lattice& lattice,
                              int idx,
                              const std::vector<EdgeOffset>& offsets) {
    const auto c = lattice.coord(idx);
    ftd::Vec3 lap;
    for (const auto& o : offsets) {
        const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
        lap += (voxels[j].flux - voxels[idx].flux) * o.weight;
    }
    return lap;
}

Energy compute_energy(const ftd::RenderBridge& rb,
                      const std::vector<EdgeOffset>& offsets) {
    Energy e;
    const auto& voxels = rb.voxels();
    const auto& lattice = rb.lattice();
    const double c2 = ftd::C_WAVE * ftd::C_WAVE;

    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& v = voxels[i];
        e.kinetic += 0.5 * v.wave_vel.mag2();

        const auto c = lattice.coord(i);
        for (const auto& o : offsets) {
            const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
            const ftd::Vec3 dJ = voxels[j].flux - v.flux;
            e.gradient += 0.25 * c2 * o.weight * dJ.mag2();
        }

        const ftd::Vec3 delta = laplacian_from_flux(voxels, lattice, i, offsets) * c2;
        e.cross += 0.5 * v.wave_vel.dot(delta);

        const double vals[] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
            delta.x, delta.y, delta.z,
        };
        for (double val : vals) e.finite = e.finite && finite_value(val);
    }

    e.naive = e.kinetic + e.gradient;
    e.modified = e.naive + e.cross;
    const double vals[] = {e.kinetic, e.gradient, e.cross, e.naive, e.modified};
    for (double val : vals) e.finite = e.finite && finite_value(val);
    return e;
}

void observe(Drift& d, const Energy& e, int tick) {
    const double naive_abs = std::abs(e.naive - d.initial_naive);
    const double mod_abs = std::abs(e.modified - d.initial_modified);
    const double naive_rel = naive_abs / std::max(std::abs(d.initial_naive), 1e-300);
    const double mod_rel = mod_abs / std::max(std::abs(d.initial_modified), 1e-300);

    if (naive_abs > d.max_abs_naive_drift) {
        d.max_abs_naive_drift = naive_abs;
        d.max_rel_naive_drift = naive_rel;
        d.max_naive_tick = tick;
    }
    if (mod_abs > d.max_abs_modified_drift) {
        d.max_abs_modified_drift = mod_abs;
        d.max_rel_modified_drift = mod_rel;
        d.max_modified_tick = tick;
    }
    d.final_naive = e.naive;
    d.final_modified = e.modified;
    d.finite = d.finite && e.finite && finite_value(naive_abs) &&
               finite_value(mod_abs) && finite_value(naive_rel) &&
               finite_value(mod_rel);
}

}  // namespace

int main() {
    std::printf("FTD-0292 source-free discrete tick energy invariant v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,alpha,%.17g,modified_abs_gate,%.17g,modified_rel_gate,%.17g,naive_drift_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::ALPHA,
                MODIFIED_ABS_GATE, MODIFIED_REL_GATE, NAIVE_DRIFT_GATE);
    std::printf("scope,source_free_single_substrate_tick_invariant_not_alpha_or_cross_section\n");
    std::printf("update,W_next=W+c2*LJ,J_next=J+W_next\n");
    std::printf("observable,E_tick=0p5_W2_plus_0p5_JKJ_minus_0p5_WKJ,equivalent_cross=0p5_W_dot_c2LJ\n");

    const std::vector<EdgeOffset> offsets = stencil_offsets();
    ftd::RenderBridge rb(L);
    configure_source_free(rb);
    inject_plane_wave(rb);

    Drift drift;
    const Energy initial = compute_energy(rb, offsets);
    drift.initial_naive = initial.naive;
    drift.initial_modified = initial.modified;
    drift.final_naive = initial.naive;
    drift.final_modified = initial.modified;
    drift.finite = initial.finite;

    std::printf("energy,tick,0,kinetic,%.17g,gradient,%.17g,cross,%.17g,naive,%.17g,modified,%.17g,finite,%s\n",
                initial.kinetic, initial.gradient, initial.cross,
                initial.naive, initial.modified, initial.finite ? "true" : "false");

    for (int tick = 1; tick <= TICKS; ++tick) {
        rb.tick();
        const Energy e = compute_energy(rb, offsets);
        observe(drift, e, tick);
        if (tick == 1 || tick == 2 || tick == 10 || tick == 50 ||
            tick == 100 || tick == TICKS) {
            std::printf("energy,tick,%d,kinetic,%.17g,gradient,%.17g,cross,%.17g,naive,%.17g,modified,%.17g,finite,%s\n",
                        tick, e.kinetic, e.gradient, e.cross,
                        e.naive, e.modified, e.finite ? "true" : "false");
        }
    }

    const bool modified_invariant = drift.finite &&
                                    drift.max_abs_modified_drift <= MODIFIED_ABS_GATE &&
                                    drift.max_rel_modified_drift <= MODIFIED_REL_GATE;
    const bool naive_drift_seen = drift.max_abs_naive_drift > NAIVE_DRIFT_GATE;

    const char* verdict = "UNCLASSIFIED";
    if (!drift.finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (modified_invariant && naive_drift_seen) {
        verdict = "DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED";
    } else if (modified_invariant) {
        verdict = "DISCRETE_TICK_MODIFIED_ENERGY_CONFIRMED_NAIVE_QUIET";
    } else {
        verdict = "DISCRETE_TICK_INVARIANT_INVALIDATED";
    }

    std::printf("drift_summary,initial_naive,%.17g,final_naive,%.17g,max_abs_naive_drift,%.17g,max_rel_naive_drift,%.17g,max_naive_tick,%d,initial_modified,%.17g,final_modified,%.17g,max_abs_modified_drift,%.17g,max_rel_modified_drift,%.17g,max_modified_tick,%d,finite,%s\n",
                drift.initial_naive, drift.final_naive,
                drift.max_abs_naive_drift, drift.max_rel_naive_drift,
                drift.max_naive_tick, drift.initial_modified,
                drift.final_modified, drift.max_abs_modified_drift,
                drift.max_rel_modified_drift, drift.max_modified_tick,
                drift.finite ? "true" : "false");
    std::printf("gates,modified_invariant,%s,naive_drift_seen,%s\n",
                modified_invariant ? "true" : "false",
                naive_drift_seen ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,source_free_tick_invariant_only_next_step_is_local_current_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "DISCRETE_TICK_INVARIANT_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
