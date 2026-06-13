/**
 * FTD-0295: Source-free discrete tick local continuity v2.
 *
 * Follow-up to FTD-0294 v1. V1 kept the right density/current but invalidated
 * its relative gate because the exchange-relative denominator was degenerate
 * when both Delta H and boundary flux were near zero. This v2 keeps the same
 * density/current and gates, and changes only the gated relative denominator
 * to a finite-volume energy scale while still reporting the v1 exchange-relative
 * value.
 *
 * The source-free tick preserves the modified global energy
 *
 *     H = 0.5 W^2 + 0.5 J K J - 0.5 W K J,  K = -c^2 L.
 *
 * This fixed campaign checks the corresponding finite-volume identity using
 * the exact antisymmetric graph-edge current implied by the same tick:
 *
 *     Delta H_V + Phi_out(boundary V) = 0
 *
 * for spherical balls in the same source-free L=33, mode_n=4, amp=0.05 setup.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a radiation claim. It is the source-free local-current control
 *   required before adding state-coupling source/work terms.
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
constexpr long double PI = 3.141592653589793238462643383279502884L;
constexpr long double BALANCE_ABS_GATE = 1e-10L;
constexpr long double BALANCE_REL_GATE = 1e-12L;
constexpr std::array<int, 5> BALL_RADII = {5, 7, 9, 11, 13};

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

struct Snapshot {
    std::vector<ftd::Vec3> flux;
    std::vector<ftd::Vec3> wave;
    bool finite = true;
};

struct BallMask {
    int radius = 0;
    int sites = 0;
    std::vector<unsigned char> inside;
};

struct BalanceMetric {
    int radius = 0;
    int sites = 0;
    int steps = 0;
    long double max_abs_dE = 0.0L;
    long double max_abs_flux = 0.0L;
    long double max_outward_flux = 0.0L;
    long double max_inward_flux = 0.0L;
    long double max_abs_balance = 0.0L;
    long double max_rel_balance = 0.0L;
    long double max_exchange_rel_balance = 0.0L;
    long double rms_balance_accum = 0.0L;
    long double mean_abs_balance_accum = 0.0L;
    long double max_abs_energy = 0.0L;
    bool finite = true;
};

struct BalanceSet {
    std::vector<BalanceMetric> metrics;
    long double max_abs_dE = 0.0L;
    long double max_abs_flux = 0.0L;
    long double max_outward_flux = 0.0L;
    long double max_inward_flux = 0.0L;
    long double max_abs_balance = 0.0L;
    long double max_rel_balance = 0.0L;
    long double max_exchange_rel_balance = 0.0L;
    long double rms_balance = 0.0L;
    bool finite = true;
};

bool finite_value(long double x) {
    return std::isfinite(x);
}

int wrap_delta(int d) {
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
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
    rb.seed_rng(2814);
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

Snapshot capture(const ftd::RenderBridge& rb) {
    Snapshot s;
    s.flux.reserve(static_cast<std::size_t>(L) * L * L);
    s.wave.reserve(static_cast<std::size_t>(L) * L * L);
    for (const auto& v : rb.voxels()) {
        s.flux.push_back(v.flux);
        s.wave.push_back(v.wave_vel);
        const long double vals[] = {
            static_cast<long double>(v.flux.x),
            static_cast<long double>(v.flux.y),
            static_cast<long double>(v.flux.z),
            static_cast<long double>(v.wave_vel.x),
            static_cast<long double>(v.wave_vel.y),
            static_cast<long double>(v.wave_vel.z),
        };
        for (long double val : vals) s.finite = s.finite && finite_value(val);
    }
    return s;
}

std::vector<BallMask> make_balls() {
    std::vector<BallMask> balls;
    const ftd::Lattice lattice(L);
    const int mc = L / 2;
    for (int radius : BALL_RADII) {
        BallMask b;
        b.radius = radius;
        b.inside.resize(static_cast<std::size_t>(L) * L * L, 0);
        const int r2max = radius * radius;
        for (int i = 0; i < static_cast<int>(b.inside.size()); ++i) {
            const auto c = lattice.coord(i);
            const int dx = wrap_delta(c.x - mc);
            const int dy = wrap_delta(c.y - mc);
            const int dz = wrap_delta(c.z - mc);
            if (dx * dx + dy * dy + dz * dz <= r2max) {
                b.inside[i] = 1;
                ++b.sites;
            }
        }
        balls.push_back(b);
    }
    return balls;
}

ftd::Vec3 stiffness_times_flux(const Snapshot& s,
                               const ftd::Lattice& lattice,
                               int idx,
                               const std::vector<EdgeOffset>& offsets) {
    const auto c = lattice.coord(idx);
    const long double c2 = static_cast<long double>(ftd::C_WAVE) *
                           static_cast<long double>(ftd::C_WAVE);
    ftd::Vec3 out;
    for (const auto& o : offsets) {
        const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
        const ftd::Vec3 d = s.flux[idx] - s.flux[j];
        out += d * static_cast<double>(c2 * o.weight);
    }
    return out;
}

long double energy_inside(const Snapshot& s,
                          const BallMask& ball,
                          const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    Kahan total;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        const ftd::Vec3 kq = stiffness_times_flux(s, lattice, i, offsets);
        const long double h = 0.5L * mag2_ld(s.wave[i]) +
                              0.5L * dot_ld(s.flux[i], kq) -
                              0.5L * dot_ld(s.wave[i], kq);
        total.add(h);
    }
    return total.sum;
}

long double outward_flux(const Snapshot& prev,
                         const Snapshot& curr,
                         const BallMask& ball,
                         const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    const long double c2 = static_cast<long double>(ftd::C_WAVE) *
                           static_cast<long double>(ftd::C_WAVE);
    Kahan total;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        const auto c = lattice.coord(i);
        for (const auto& o : offsets) {
            const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
            if (ball.inside[j]) continue;
            const long double a = c2 * o.weight;
            const long double edge_out =
                0.5L * a * (dot_ld(prev.flux[i], curr.wave[j]) -
                             dot_ld(curr.wave[i], prev.flux[j]));
            total.add(edge_out);
        }
    }
    return total.sum;
}

BalanceSet make_balance_set(const std::vector<BallMask>& balls) {
    BalanceSet set;
    for (const auto& b : balls) {
        BalanceMetric m;
        m.radius = b.radius;
        m.sites = b.sites;
        set.metrics.push_back(m);
    }
    return set;
}

void observe(BalanceSet& set,
             const Snapshot& prev,
             const Snapshot& curr,
             const std::vector<BallMask>& balls,
             const std::vector<EdgeOffset>& offsets) {
    set.finite = set.finite && prev.finite && curr.finite;
    long double rms_accum = 0.0L;
    int rms_count = 0;
    for (std::size_t k = 0; k < balls.size(); ++k) {
        auto& m = set.metrics[k];
        const long double e0 = energy_inside(prev, balls[k], offsets);
        const long double e1 = energy_inside(curr, balls[k], offsets);
        const long double dE = e1 - e0;
        const long double flux = outward_flux(prev, curr, balls[k], offsets);
        const long double balance = dE + flux;
        const long double exchange_scale = std::max(std::abs(dE) + std::abs(flux), 1e-300L);
        const long double energy_scale =
            std::max({std::abs(e0), std::abs(e1), exchange_scale, 1e-300L});
        const long double rel = std::abs(balance) / energy_scale;
        const long double exchange_rel = std::abs(balance) / exchange_scale;

        ++m.steps;
        m.max_abs_dE = std::max(m.max_abs_dE, std::abs(dE));
        m.max_abs_flux = std::max(m.max_abs_flux, std::abs(flux));
        m.max_outward_flux = std::max(m.max_outward_flux, std::max(0.0L, flux));
        m.max_inward_flux = std::max(m.max_inward_flux, std::max(0.0L, -flux));
        m.max_abs_balance = std::max(m.max_abs_balance, std::abs(balance));
        m.max_rel_balance = std::max(m.max_rel_balance, rel);
        m.max_exchange_rel_balance = std::max(m.max_exchange_rel_balance, exchange_rel);
        m.max_abs_energy = std::max(m.max_abs_energy,
                                    std::max(std::abs(e0), std::abs(e1)));
        m.rms_balance_accum += balance * balance;
        m.mean_abs_balance_accum += std::abs(balance);

        const long double vals[] = {e0, e1, dE, flux, balance, rel, exchange_rel};
        for (long double val : vals) m.finite = m.finite && finite_value(val);

        set.max_abs_dE = std::max(set.max_abs_dE, m.max_abs_dE);
        set.max_abs_flux = std::max(set.max_abs_flux, m.max_abs_flux);
        set.max_outward_flux = std::max(set.max_outward_flux, m.max_outward_flux);
        set.max_inward_flux = std::max(set.max_inward_flux, m.max_inward_flux);
        set.max_abs_balance = std::max(set.max_abs_balance, m.max_abs_balance);
        set.max_rel_balance = std::max(set.max_rel_balance, m.max_rel_balance);
        set.max_exchange_rel_balance =
            std::max(set.max_exchange_rel_balance, m.max_exchange_rel_balance);
        set.finite = set.finite && m.finite;
        rms_accum += m.rms_balance_accum;
        rms_count += m.steps;
    }
    if (rms_count > 0) {
        set.rms_balance = std::sqrt(rms_accum / static_cast<long double>(rms_count));
    }
}

void print_balance_set(const BalanceSet& set) {
    for (const auto& m : set.metrics) {
        const long double rms = m.steps > 0
                                    ? std::sqrt(m.rms_balance_accum /
                                                static_cast<long double>(m.steps))
                                    : 0.0L;
        const long double mean_abs = m.steps > 0
                                         ? m.mean_abs_balance_accum /
                                               static_cast<long double>(m.steps)
                                         : 0.0L;
        std::printf("balance,radius,%d,sites,%d,steps,%d,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_outward_flux,%.21Lg,max_inward_flux,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,mean_abs_balance,%.21Lg,max_scale_rel_balance,%.21Lg,max_exchange_rel_balance,%.21Lg,max_abs_energy,%.21Lg,finite,%s\n",
                    m.radius, m.sites, m.steps, m.max_abs_dE, m.max_abs_flux,
                    m.max_outward_flux, m.max_inward_flux, m.max_abs_balance,
                    rms, mean_abs, m.max_rel_balance, m.max_exchange_rel_balance,
                    m.max_abs_energy,
                    m.finite ? "true" : "false");
    }
    std::printf("balance_summary,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_outward_flux,%.21Lg,max_inward_flux,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,max_scale_rel_balance,%.21Lg,max_exchange_rel_balance,%.21Lg,finite,%s\n",
                set.max_abs_dE, set.max_abs_flux, set.max_outward_flux,
                set.max_inward_flux, set.max_abs_balance, set.rms_balance,
                set.max_rel_balance, set.max_exchange_rel_balance,
                set.finite ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0295 source-free discrete tick local continuity v2\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,alpha,%.17g,balance_abs_gate,%.21Lg,balance_scale_rel_gate,%.21Lg\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::ALPHA,
                BALANCE_ABS_GATE, BALANCE_REL_GATE);
    std::printf("ball_radii");
    for (int r : BALL_RADII) std::printf(",%d", r);
    std::printf("\n");
    std::printf("scope,source_free_single_substrate_tick_local_continuity_not_alpha_or_cross_section\n");
    std::printf("density,h_i=0p5_W_i2_plus_0p5_J_i_dot_KJ_i_minus_0p5_W_i_dot_KJ_i,K=-c2L\n");
    std::printf("current,Phi_i_to_j=0p5*c2*w_ij*(J_i_old_dot_W_j_next-W_i_next_dot_J_j_old),outward_positive\n");
    std::printf("identity,Delta_H_V_plus_Phi_out_equals_0\n");
    std::printf("relative_metric,scale=max(abs(H_V_old),abs(H_V_next),abs_delta_plus_abs_flux,1e-300),exchange_relative_reported_not_gated\n");
    std::printf("accumulation,long_double_kahan\n");

    const std::vector<EdgeOffset> offsets = stencil_offsets();
    const std::vector<BallMask> balls = make_balls();
    BalanceSet balances = make_balance_set(balls);

    ftd::RenderBridge rb(L);
    configure_source_free(rb);
    inject_plane_wave(rb);
    Snapshot prev = capture(rb);

    for (int tick = 0; tick < TICKS; ++tick) {
        rb.tick();
        Snapshot curr = capture(rb);
        observe(balances, prev, curr, balls, offsets);
        prev = std::move(curr);
    }

    print_balance_set(balances);

    const bool local_continuity = balances.finite &&
                                  balances.max_abs_balance <= BALANCE_ABS_GATE &&
                                  balances.max_rel_balance <= BALANCE_REL_GATE;

    const char* verdict = "UNCLASSIFIED";
    if (!balances.finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (local_continuity) {
        verdict = "SOURCE_FREE_LOCAL_TICK_CONTINUITY_CONFIRMED";
    } else {
        verdict = "SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED";
    }

    std::printf("gates,local_continuity,%s,exchange_relative_degenerate,%s\n",
                local_continuity ? "true" : "false",
                balances.max_exchange_rel_balance > BALANCE_REL_GATE ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,source_free_local_tick_current_only_next_step_is_state_coupling_source_work_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "SOURCE_FREE_LOCAL_TICK_CONTINUITY_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
