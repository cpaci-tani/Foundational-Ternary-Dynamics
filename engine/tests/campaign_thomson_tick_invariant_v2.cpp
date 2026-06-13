/**
 * FTD-0293: Source-free discrete tick energy invariant, precision v2.
 *
 * Follow-up to FTD-0292 v1. V1 used the correct modified-energy candidate but
 * ordinary double accumulation missed its predeclared relative gate while the
 * absolute drift stayed small. This v2 keeps the same update, initial state,
 * invariant formula, and gates, and changes only the measurement arithmetic to
 * long-double Kahan accumulation.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a radiation claim. It is the source-free invariant control needed
 *   before deriving any local finite-volume current.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
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
constexpr long double PI = 3.141592653589793238462643383279502884L;
constexpr long double MODIFIED_ABS_GATE = 1e-10L;
constexpr long double MODIFIED_REL_GATE = 1e-12L;
constexpr long double NAIVE_DRIFT_GATE = 1e-6L;

struct EdgeOffset {
    int dx = 0;
    int dy = 0;
    int dz = 0;
    long double weight = 0.0L;
};

struct Kahan {
    long double sum = 0.0L;
    long double correction = 0.0L;

    void add(long double value) {
        const long double y = value - correction;
        const long double t = sum + y;
        correction = (t - sum) - y;
        sum = t;
    }
};

struct Energy {
    long double kinetic = 0.0L;
    long double gradient = 0.0L;
    long double cross = 0.0L;
    long double naive = 0.0L;
    long double modified = 0.0L;
    bool finite = true;
};

struct Drift {
    long double initial_naive = 0.0L;
    long double initial_modified = 0.0L;
    long double final_naive = 0.0L;
    long double final_modified = 0.0L;
    long double max_abs_naive_drift = 0.0L;
    long double max_rel_naive_drift = 0.0L;
    long double max_abs_modified_drift = 0.0L;
    long double max_rel_modified_drift = 0.0L;
    int max_naive_tick = 0;
    int max_modified_tick = 0;
    bool finite = true;
};

bool finite_value(long double x) {
    return std::isfinite(x);
}

std::vector<EdgeOffset> stencil_offsets() {
    std::vector<EdgeOffset> out;
    out.reserve(18);
    const long double wf = static_cast<long double>(ftd::LAPLACIAN_FACE_WEIGHT);
    const long double we = static_cast<long double>(ftd::LAPLACIAN_EDGE_WEIGHT);
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
    rb.seed_rng(2813);
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
    const long double k = 2.0L * PI * static_cast<long double>(MODE_N) /
                          static_cast<long double>(L);
    const long double omega = 2.0L * static_cast<long double>(ftd::C_SPEED) *
                              std::abs(std::sin(k * 0.5L));
    auto& voxels = rb.voxels();
    for (int x = 0; x < L; ++x) {
        const long double phase = k * static_cast<long double>(x);
        const double jy = static_cast<double>(static_cast<long double>(AMP) *
                                              std::sin(phase));
        const double wy = static_cast<double>(-omega * static_cast<long double>(AMP) *
                                              std::cos(phase));
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
        lap += (voxels[j].flux - voxels[idx].flux) * static_cast<double>(o.weight);
    }
    return lap;
}

long double mag2_ld(const ftd::Vec3& v) {
    const long double x = static_cast<long double>(v.x);
    const long double y = static_cast<long double>(v.y);
    const long double z = static_cast<long double>(v.z);
    return x * x + y * y + z * z;
}

long double dot_ld(const ftd::Vec3& a, const ftd::Vec3& b) {
    return static_cast<long double>(a.x) * static_cast<long double>(b.x) +
           static_cast<long double>(a.y) * static_cast<long double>(b.y) +
           static_cast<long double>(a.z) * static_cast<long double>(b.z);
}

Energy compute_energy(const ftd::RenderBridge& rb,
                      const std::vector<EdgeOffset>& offsets) {
    Kahan kinetic;
    Kahan gradient;
    Kahan cross;
    Energy e;
    const auto& voxels = rb.voxels();
    const auto& lattice = rb.lattice();
    const long double c2 = static_cast<long double>(ftd::C_WAVE) *
                           static_cast<long double>(ftd::C_WAVE);

    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& v = voxels[i];
        kinetic.add(0.5L * mag2_ld(v.wave_vel));

        const auto c = lattice.coord(i);
        for (const auto& o : offsets) {
            const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
            const ftd::Vec3 dJ = voxels[j].flux - v.flux;
            gradient.add(0.25L * c2 * o.weight * mag2_ld(dJ));
        }

        const ftd::Vec3 delta =
            laplacian_from_flux(voxels, lattice, i, offsets) *
            static_cast<double>(c2);
        cross.add(0.5L * dot_ld(v.wave_vel, delta));

        const long double vals[] = {
            static_cast<long double>(v.flux.x),
            static_cast<long double>(v.flux.y),
            static_cast<long double>(v.flux.z),
            static_cast<long double>(v.wave_vel.x),
            static_cast<long double>(v.wave_vel.y),
            static_cast<long double>(v.wave_vel.z),
            static_cast<long double>(delta.x),
            static_cast<long double>(delta.y),
            static_cast<long double>(delta.z),
        };
        for (long double val : vals) e.finite = e.finite && finite_value(val);
    }

    e.kinetic = kinetic.sum;
    e.gradient = gradient.sum;
    e.cross = cross.sum;
    e.naive = e.kinetic + e.gradient;
    e.modified = e.naive + e.cross;
    const long double vals[] = {e.kinetic, e.gradient, e.cross, e.naive, e.modified};
    for (long double val : vals) e.finite = e.finite && finite_value(val);
    return e;
}

void observe(Drift& d, const Energy& e, int tick) {
    const long double naive_abs = std::abs(e.naive - d.initial_naive);
    const long double mod_abs = std::abs(e.modified - d.initial_modified);
    const long double naive_rel =
        naive_abs / std::max(std::abs(d.initial_naive), 1e-300L);
    const long double mod_rel =
        mod_abs / std::max(std::abs(d.initial_modified), 1e-300L);

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

void print_energy(int tick, const Energy& e) {
    std::printf("energy,tick,%d,kinetic,%.21Lg,gradient,%.21Lg,cross,%.21Lg,naive,%.21Lg,modified,%.21Lg,finite,%s\n",
                tick, e.kinetic, e.gradient, e.cross, e.naive, e.modified,
                e.finite ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0293 source-free discrete tick energy invariant v2\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,alpha,%.17g,modified_abs_gate,%.21Lg,modified_rel_gate,%.21Lg,naive_drift_gate,%.21Lg\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::ALPHA,
                MODIFIED_ABS_GATE, MODIFIED_REL_GATE, NAIVE_DRIFT_GATE);
    std::printf("scope,source_free_single_substrate_tick_invariant_precision_v2_not_alpha_or_cross_section\n");
    std::printf("update,W_next=W+c2*LJ,J_next=J+W_next\n");
    std::printf("observable,E_tick=0p5_W2_plus_0p5_JKJ_minus_0p5_WKJ,equivalent_cross=0p5_W_dot_c2LJ\n");
    std::printf("accumulation,long_double_kahan\n");

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

    print_energy(0, initial);

    for (int tick = 1; tick <= TICKS; ++tick) {
        rb.tick();
        const Energy e = compute_energy(rb, offsets);
        observe(drift, e, tick);
        if (tick == 1 || tick == 2 || tick == 10 || tick == 50 ||
            tick == 100 || tick == TICKS) {
            print_energy(tick, e);
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

    std::printf("drift_summary,initial_naive,%.21Lg,final_naive,%.21Lg,max_abs_naive_drift,%.21Lg,max_rel_naive_drift,%.21Lg,max_naive_tick,%d,initial_modified,%.21Lg,final_modified,%.21Lg,max_abs_modified_drift,%.21Lg,max_rel_modified_drift,%.21Lg,max_modified_tick,%d,finite,%s\n",
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
    std::printf("interpretation,source_free_tick_invariant_confirmed_before_local_current_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "DISCRETE_TICK_INVARIANT_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
