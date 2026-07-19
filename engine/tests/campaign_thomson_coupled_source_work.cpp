/**
 * FTD-0296: Fixed-charge coupled tick source/work continuity.
 *
 * Follow-up to FTD-0295. The source-free tick now has a confirmed modified
 * energy and local boundary current. This campaign adds the state-flux source
 * term while keeping the charge fixed, so source/work is isolated from
 * particle motion:
 *
 *     W* = W + c^2 L J
 *     W' = W* + S
 *     J' = J + W'
 *
 * The exact source work density for the same modified energy is
 *
 *     Work_i = W*_i dot S_i + 0.5 |S_i|^2 + 0.5 J_i dot (K S)_i.
 *
 * The finite-volume identity tested here is
 *
 *     Delta H_V + Phi_out(source-free boundary current) - Work_V = 0.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a radiation claim. It is the fixed-source work control required
 *   before returning to unlocked recoil.
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

struct SourceSnapshot {
    std::vector<ftd::Vec3> source;
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
    long double max_abs_work = 0.0L;
    long double max_abs_balance = 0.0L;
    long double max_scale_rel_balance = 0.0L;
    long double rms_balance_accum = 0.0L;
    long double mean_abs_balance_accum = 0.0L;
    long double max_abs_energy = 0.0L;
    bool finite = true;
};

struct BalanceSet {
    std::vector<BalanceMetric> metrics;
    long double max_abs_dE = 0.0L;
    long double max_abs_flux = 0.0L;
    long double max_abs_work = 0.0L;
    long double max_abs_balance = 0.0L;
    long double max_scale_rel_balance = 0.0L;
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

void configure_fixed_coupled(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(2816);
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
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

void inject_electron(ftd::RenderBridge& rb) {
    const int mc = L / 2;
    rb.inject_particle(mc, mc, mc, static_cast<int8_t>(-1), {0.0, 0.0, 0.0},
                       static_cast<int8_t>(-1), static_cast<int8_t>(0));
    rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = true;
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

SourceSnapshot capture_source(const ftd::RenderBridge& rb) {
    SourceSnapshot out;
    const int n = static_cast<int>(rb.lattice().total_sites());
    out.source.reserve(n);
    for (int i = 0; i < n; ++i) {
        // Electric part −g_c·∇s (Term 2 sign amendment 2026-07-18; must
        // mirror phase_read.cpp for the work accounting to balance).
        const ftd::Vec3 source = (rb.curl_state_velocity(i) - rb.gradient_state(i)) * ftd::G_C;
        out.source.push_back(source);
        const long double vals[] = {
            static_cast<long double>(source.x),
            static_cast<long double>(source.y),
            static_cast<long double>(source.z),
        };
        for (long double val : vals) out.finite = out.finite && finite_value(val);
    }
    return out;
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

ftd::Vec3 stiffness_times_array(const std::vector<ftd::Vec3>& field,
                                const ftd::Lattice& lattice,
                                int idx,
                                const std::vector<EdgeOffset>& offsets) {
    const auto c = lattice.coord(idx);
    const long double c2 = static_cast<long double>(ftd::C_WAVE) *
                           static_cast<long double>(ftd::C_WAVE);
    ftd::Vec3 out;
    for (const auto& o : offsets) {
        const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
        const ftd::Vec3 d = field[idx] - field[j];
        out += d * static_cast<double>(c2 * o.weight);
    }
    return out;
}

std::vector<ftd::Vec3> source_free_wave_next(const Snapshot& s,
                                             const std::vector<EdgeOffset>& offsets) {
    std::vector<ftd::Vec3> wstar(s.wave.size());
    const ftd::Lattice lattice(L);
    for (int i = 0; i < static_cast<int>(s.wave.size()); ++i) {
        const ftd::Vec3 kq = stiffness_times_array(s.flux, lattice, i, offsets);
        wstar[i] = s.wave[i] - kq;
    }
    return wstar;
}

long double energy_inside(const Snapshot& s,
                          const BallMask& ball,
                          const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    Kahan total;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        const ftd::Vec3 kq = stiffness_times_array(s.flux, lattice, i, offsets);
        const long double h = 0.5L * mag2_ld(s.wave[i]) +
                              0.5L * dot_ld(s.flux[i], kq) -
                              0.5L * dot_ld(s.wave[i], kq);
        total.add(h);
    }
    return total.sum;
}

long double outward_flux_source_free(const Snapshot& prev,
                                     const std::vector<ftd::Vec3>& wstar,
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
                0.5L * a * (dot_ld(prev.flux[i], wstar[j]) -
                             dot_ld(wstar[i], prev.flux[j]));
            total.add(edge_out);
        }
    }
    return total.sum;
}

long double source_work_inside(const Snapshot& prev,
                               const std::vector<ftd::Vec3>& wstar,
                               const SourceSnapshot& src,
                               const BallMask& ball,
                               const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    Kahan total;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        const ftd::Vec3 ks = stiffness_times_array(src.source, lattice, i, offsets);
        const long double work_i = dot_ld(wstar[i], src.source[i]) +
                                   0.5L * mag2_ld(src.source[i]) +
                                   0.5L * dot_ld(prev.flux[i], ks);
        total.add(work_i);
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
             const SourceSnapshot& src,
             const std::vector<BallMask>& balls,
             const std::vector<EdgeOffset>& offsets) {
    set.finite = set.finite && prev.finite && curr.finite && src.finite;
    const std::vector<ftd::Vec3> wstar = source_free_wave_next(prev, offsets);
    long double rms_accum = 0.0L;
    int rms_count = 0;
    for (std::size_t k = 0; k < balls.size(); ++k) {
        auto& m = set.metrics[k];
        const long double e0 = energy_inside(prev, balls[k], offsets);
        const long double e1 = energy_inside(curr, balls[k], offsets);
        const long double dE = e1 - e0;
        const long double flux = outward_flux_source_free(prev, wstar, balls[k], offsets);
        const long double work = source_work_inside(prev, wstar, src, balls[k], offsets);
        const long double balance = dE + flux - work;
        const long double scale =
            std::max({std::abs(e0), std::abs(e1), std::abs(work),
                      std::abs(dE) + std::abs(flux), 1e-300L});
        const long double rel = std::abs(balance) / scale;

        ++m.steps;
        m.max_abs_dE = std::max(m.max_abs_dE, std::abs(dE));
        m.max_abs_flux = std::max(m.max_abs_flux, std::abs(flux));
        m.max_abs_work = std::max(m.max_abs_work, std::abs(work));
        m.max_abs_balance = std::max(m.max_abs_balance, std::abs(balance));
        m.max_scale_rel_balance = std::max(m.max_scale_rel_balance, rel);
        m.max_abs_energy = std::max(m.max_abs_energy,
                                    std::max(std::abs(e0), std::abs(e1)));
        m.rms_balance_accum += balance * balance;
        m.mean_abs_balance_accum += std::abs(balance);

        const long double vals[] = {e0, e1, dE, flux, work, balance, rel};
        for (long double val : vals) m.finite = m.finite && finite_value(val);

        set.max_abs_dE = std::max(set.max_abs_dE, m.max_abs_dE);
        set.max_abs_flux = std::max(set.max_abs_flux, m.max_abs_flux);
        set.max_abs_work = std::max(set.max_abs_work, m.max_abs_work);
        set.max_abs_balance = std::max(set.max_abs_balance, m.max_abs_balance);
        set.max_scale_rel_balance =
            std::max(set.max_scale_rel_balance, m.max_scale_rel_balance);
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
        std::printf("balance,radius,%d,sites,%d,steps,%d,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_abs_work,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,mean_abs_balance,%.21Lg,max_scale_rel_balance,%.21Lg,max_abs_energy,%.21Lg,finite,%s\n",
                    m.radius, m.sites, m.steps, m.max_abs_dE, m.max_abs_flux,
                    m.max_abs_work, m.max_abs_balance, rms, mean_abs,
                    m.max_scale_rel_balance, m.max_abs_energy,
                    m.finite ? "true" : "false");
    }
    std::printf("balance_summary,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_abs_work,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,max_scale_rel_balance,%.21Lg,finite,%s\n",
                set.max_abs_dE, set.max_abs_flux, set.max_abs_work,
                set.max_abs_balance, set.rms_balance, set.max_scale_rel_balance,
                set.finite ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0296 fixed-charge coupled tick source/work continuity v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,g_c,%.17g,alpha,%.17g,balance_abs_gate,%.21Lg,balance_scale_rel_gate,%.21Lg\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::G_C, ftd::ALPHA,
                BALANCE_ABS_GATE, BALANCE_REL_GATE);
    std::printf("ball_radii");
    for (int r : BALL_RADII) std::printf(",%d", r);
    std::printf("\n");
    std::printf("scope,fixed_charge_coupled_tick_source_work_not_alpha_or_cross_section\n");
    std::printf("source,S=G_C*(grad_state+curl_state_velocity),movement,false,charge_locked,true\n");
    std::printf("identity,Delta_H_V_plus_Phi_source_free_out_minus_Work_source_equals_0\n");
    std::printf("work,Work_i=Wstar_i_dot_S_i_plus_0p5_S_i2_plus_0p5_J_i_dot_KS_i\n");
    std::printf("accumulation,long_double_kahan\n");

    const std::vector<EdgeOffset> offsets = stencil_offsets();
    const std::vector<BallMask> balls = make_balls();
    BalanceSet balances = make_balance_set(balls);

    ftd::RenderBridge rb(L);
    configure_fixed_coupled(rb);
    inject_electron(rb);
    inject_plane_wave(rb);
    Snapshot prev = capture(rb);

    for (int tick = 0; tick < TICKS; ++tick) {
        const SourceSnapshot src = capture_source(rb);
        rb.tick();
        Snapshot curr = capture(rb);
        observe(balances, prev, curr, src, balls, offsets);
        prev = std::move(curr);
    }

    print_balance_set(balances);

    const bool coupled_source_work = balances.finite &&
                                     balances.max_abs_balance <= BALANCE_ABS_GATE &&
                                     balances.max_scale_rel_balance <= BALANCE_REL_GATE;

    const char* verdict = "UNCLASSIFIED";
    if (!balances.finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (coupled_source_work) {
        verdict = "FIXED_CHARGE_SOURCE_WORK_CONTINUITY_CONFIRMED";
    } else {
        verdict = "FIXED_CHARGE_SOURCE_WORK_CONTINUITY_INVALIDATED";
    }

    std::printf("gates,coupled_source_work,%s\n",
                coupled_source_work ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,fixed_source_work_accounting_only_next_step_unlocked_recoil_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "FIXED_CHARGE_SOURCE_WORK_CONTINUITY_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
