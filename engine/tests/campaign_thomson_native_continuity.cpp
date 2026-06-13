/**
 * FTD-0291: Thomson native finite-volume continuity meter.
 *
 * Follow-up to FTD-0290. The borrowed residual-field Poynting shell meter did
 * not produce above-gate outward power. This fixed campaign asks whether the
 * same setup has a native graph-energy continuity law on finite balls.
 *
 * Scope:
 *   [MEASUREMENT] Baseline-subtracted finite-volume balance using the engine's
 *   18-neighbor wave-Laplacian graph. The candidate outward current is
 *
 *       F_i->j = -c^2 w_ij 0.5 (W_i + W_j) dot (J_j - J_i)
 *
 *   on edges that cross a finite-volume boundary.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a QED scattering amplitude. It is a native accounting-law
 *   discriminator for the already frozen FTD-0288/0289/0290 setup.
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
constexpr double MACHINE_GATE = 1e-12;
constexpr double BALANCE_ABS_GATE = 1e-8;
constexpr double BALANCE_REL_GATE = 1e-6;
constexpr double GRAPH_FLUX_GATE = 1e-8;
constexpr std::array<int, 5> BALL_RADII = {5, 7, 9, 11, 13};

enum class Mode {
    LockedLinear,
    NativeLegacy,
    NativeEmergent,
};

struct Track {
    bool present = false;
    int idx = -1;
    int x = 0;
    int y = 0;
    int z = 0;
    const ftd::Voxel* voxel = nullptr;
};

struct FieldSnapshot {
    std::vector<ftd::Vec3> flux;
    std::vector<ftd::Vec3> wave;
    bool finite = true;
};

struct BallMask {
    int radius = 0;
    std::vector<unsigned char> inside;
    int sites = 0;
};

struct EdgeOffset {
    int dx = 0;
    int dy = 0;
    int dz = 0;
    double weight = 0.0;
};

struct BalanceMetric {
    int radius = 0;
    int sites = 0;
    int steps = 0;
    double max_abs_dE = 0.0;
    double max_abs_flux = 0.0;
    double max_outward_flux = 0.0;
    double max_inward_flux = 0.0;
    double max_abs_balance = 0.0;
    double rms_balance_accum = 0.0;
    double mean_abs_balance_accum = 0.0;
    double max_rel_balance = 0.0;
    double max_energy = 0.0;
    bool finite = true;
};

struct BalanceSet {
    std::string label;
    std::vector<BalanceMetric> metrics;
    double max_abs_dE = 0.0;
    double max_abs_flux = 0.0;
    double max_outward_flux = 0.0;
    double max_inward_flux = 0.0;
    double max_abs_balance = 0.0;
    double max_rel_balance = 0.0;
    double rms_balance = 0.0;
    bool finite = true;
};

struct ModeReport {
    std::string mode;
    BalanceSet beam_balance;
    BalanceSet residual_balance;
    BalanceSet repeat_balance;
    bool arms_finite = true;
};

const char* mode_name(Mode mode) {
    switch (mode) {
        case Mode::LockedLinear: return "locked_linear";
        case Mode::NativeLegacy: return "native_legacy";
        case Mode::NativeEmergent: return "native_emergent";
    }
    return "unknown";
}

bool finite_value(double x) {
    return std::isfinite(x);
}

int wrap_delta(int d) {
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
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

void configure(ftd::RenderBridge& rb, Mode mode) {
    rb.force_cpu();
    rb.seed_rng(2811);
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
    rb.toggles.symmetric_movement_order = false;

    if (mode == Mode::LockedLinear) {
        rb.toggles.forces = false;
        rb.toggles.movement = false;
        rb.toggles.emergent_forces = false;
    } else if (mode == Mode::NativeLegacy) {
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = false;
    } else if (mode == Mode::NativeEmergent) {
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = true;
    }
}

void inject_electron(ftd::RenderBridge& rb, bool locked) {
    const int mc = L / 2;
    rb.inject_particle(mc, mc, mc, static_cast<int8_t>(-1), {0.0, 0.0, 0.0},
                       static_cast<int8_t>(-1), static_cast<int8_t>(0));
    rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = locked;
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

int initialize_arm(ftd::RenderBridge& rb, Mode mode, bool electron, bool beam) {
    configure(rb, mode);
    if (electron) inject_electron(rb, mode == Mode::LockedLinear);
    if (beam) inject_plane_wave(rb);
    if (!electron) return -1;
    const int mc = L / 2;
    return rb.voxels()[rb.lattice().index(mc, mc, mc)].particle_id;
}

Track find_electron(const ftd::RenderBridge& rb, int particle_id) {
    const auto& voxels = rb.voxels();
    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& v = voxels[i];
        if (v.state < 0 && (particle_id < 0 || v.particle_id == particle_id)) {
            const auto c = rb.lattice().coord(i);
            return {true, i, c.x, c.y, c.z, &v};
        }
    }
    for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
        const auto& v = voxels[i];
        if (v.state < 0) {
            const auto c = rb.lattice().coord(i);
            return {true, i, c.x, c.y, c.z, &v};
        }
    }
    return {};
}

bool arm_finite(const ftd::RenderBridge& rb, int particle_id, bool electron) {
    bool ok = true;
    if (electron) {
        ok = ok && find_electron(rb, particle_id).present;
    }
    for (const auto& v : rb.voxels()) {
        const double vals[] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
            v.velocity.x, v.velocity.y, v.velocity.z,
            v.remainder.x, v.remainder.y, v.remainder.z,
        };
        for (double val : vals) ok = ok && finite_value(val);
    }
    return ok;
}

FieldSnapshot capture(const ftd::RenderBridge& rb) {
    FieldSnapshot out;
    out.flux.reserve(static_cast<std::size_t>(L) * L * L);
    out.wave.reserve(static_cast<std::size_t>(L) * L * L);
    for (const auto& v : rb.voxels()) {
        out.flux.push_back(v.flux);
        out.wave.push_back(v.wave_vel);
        const double vals[] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
        };
        for (double val : vals) out.finite = out.finite && finite_value(val);
    }
    return out;
}

FieldSnapshot residual(const FieldSnapshot& plus,
                       const FieldSnapshot& beam,
                       const FieldSnapshot& charge) {
    FieldSnapshot out;
    const std::size_t n = plus.flux.size();
    out.flux.resize(n);
    out.wave.resize(n);
    out.finite = plus.finite && beam.finite && charge.finite &&
                 beam.flux.size() == n && charge.flux.size() == n;
    for (std::size_t i = 0; i < n; ++i) {
        out.flux[i] = plus.flux[i] - beam.flux[i] - charge.flux[i];
        out.wave[i] = plus.wave[i] - beam.wave[i] - charge.wave[i];
        const double vals[] = {
            out.flux[i].x, out.flux[i].y, out.flux[i].z,
            out.wave[i].x, out.wave[i].y, out.wave[i].z,
        };
        for (double val : vals) out.finite = out.finite && finite_value(val);
    }
    return out;
}

FieldSnapshot difference(const FieldSnapshot& a, const FieldSnapshot& b) {
    FieldSnapshot out;
    const std::size_t n = a.flux.size();
    out.flux.resize(n);
    out.wave.resize(n);
    out.finite = a.finite && b.finite && b.flux.size() == n;
    for (std::size_t i = 0; i < n; ++i) {
        out.flux[i] = a.flux[i] - b.flux[i];
        out.wave[i] = a.wave[i] - b.wave[i];
        const double vals[] = {
            out.flux[i].x, out.flux[i].y, out.flux[i].z,
            out.wave[i].x, out.wave[i].y, out.wave[i].z,
        };
        for (double val : vals) out.finite = out.finite && finite_value(val);
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
        const double r2max = static_cast<double>(radius * radius);
        for (int i = 0; i < static_cast<int>(b.inside.size()); ++i) {
            const auto c = lattice.coord(i);
            const int dx = wrap_delta(c.x - mc);
            const int dy = wrap_delta(c.y - mc);
            const int dz = wrap_delta(c.z - mc);
            const double r2 = static_cast<double>(dx * dx + dy * dy + dz * dz);
            if (r2 <= r2max) {
                b.inside[i] = 1;
                ++b.sites;
            }
        }
        balls.push_back(b);
    }
    return balls;
}

double graph_energy_inside(const FieldSnapshot& s,
                           const BallMask& ball,
                           const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    const double cw2 = ftd::C_WAVE * ftd::C_WAVE;
    double energy = 0.0;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        energy += 0.5 * s.wave[i].mag2();
        const auto c = lattice.coord(i);
        for (const auto& o : offsets) {
            const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
            const ftd::Vec3 dJ = s.flux[j] - s.flux[i];
            energy += 0.25 * cw2 * o.weight * dJ.mag2();
        }
    }
    return energy;
}

double graph_outward_flux(const FieldSnapshot& prev,
                          const FieldSnapshot& curr,
                          const BallMask& ball,
                          const std::vector<EdgeOffset>& offsets) {
    const ftd::Lattice lattice(L);
    const double cw2 = ftd::C_WAVE * ftd::C_WAVE;
    double flux = 0.0;
    for (int i = 0; i < static_cast<int>(ball.inside.size()); ++i) {
        if (!ball.inside[i]) continue;
        const auto c = lattice.coord(i);
        for (const auto& o : offsets) {
            const int j = lattice.index(c.x + o.dx, c.y + o.dy, c.z + o.dz);
            if (ball.inside[j]) continue;

            const ftd::Vec3 Ji = (prev.flux[i] + curr.flux[i]) * 0.5;
            const ftd::Vec3 Jj = (prev.flux[j] + curr.flux[j]) * 0.5;
            const ftd::Vec3 Wi = (prev.wave[i] + curr.wave[i]) * 0.5;
            const ftd::Vec3 Wj = (prev.wave[j] + curr.wave[j]) * 0.5;
            const ftd::Vec3 dJ = Jj - Ji;
            const ftd::Vec3 Wavg = (Wi + Wj) * 0.5;
            flux += -cw2 * o.weight * Wavg.dot(dJ);
        }
    }
    return flux;
}

BalanceSet make_balance_set(const char* label, const std::vector<BallMask>& balls) {
    BalanceSet set;
    set.label = label;
    for (const auto& b : balls) {
        BalanceMetric m;
        m.radius = b.radius;
        m.sites = b.sites;
        set.metrics.push_back(m);
    }
    return set;
}

void observe_balance(BalanceSet& set,
                     const FieldSnapshot& prev,
                     const FieldSnapshot& curr,
                     const std::vector<BallMask>& balls,
                     const std::vector<EdgeOffset>& offsets) {
    set.finite = set.finite && prev.finite && curr.finite;
    double rms_accum = 0.0;
    int rms_count = 0;
    for (std::size_t k = 0; k < balls.size(); ++k) {
        auto& m = set.metrics[k];
        const double e0 = graph_energy_inside(prev, balls[k], offsets);
        const double e1 = graph_energy_inside(curr, balls[k], offsets);
        const double dE = e1 - e0;
        const double flux = graph_outward_flux(prev, curr, balls[k], offsets);
        const double balance = dE + flux;
        const double denom = std::max(std::abs(dE) + std::abs(flux), 1e-300);
        const double rel = std::abs(balance) / denom;

        ++m.steps;
        m.max_abs_dE = std::max(m.max_abs_dE, std::abs(dE));
        m.max_abs_flux = std::max(m.max_abs_flux, std::abs(flux));
        m.max_outward_flux = std::max(m.max_outward_flux, std::max(0.0, flux));
        m.max_inward_flux = std::max(m.max_inward_flux, std::max(0.0, -flux));
        m.max_abs_balance = std::max(m.max_abs_balance, std::abs(balance));
        m.max_rel_balance = std::max(m.max_rel_balance, rel);
        m.max_energy = std::max(m.max_energy, std::max(std::abs(e0), std::abs(e1)));
        m.rms_balance_accum += balance * balance;
        m.mean_abs_balance_accum += std::abs(balance);

        const double vals[] = {e0, e1, dE, flux, balance, rel};
        for (double val : vals) m.finite = m.finite && finite_value(val);

        set.max_abs_dE = std::max(set.max_abs_dE, m.max_abs_dE);
        set.max_abs_flux = std::max(set.max_abs_flux, m.max_abs_flux);
        set.max_outward_flux = std::max(set.max_outward_flux, m.max_outward_flux);
        set.max_inward_flux = std::max(set.max_inward_flux, m.max_inward_flux);
        set.max_abs_balance = std::max(set.max_abs_balance, m.max_abs_balance);
        set.max_rel_balance = std::max(set.max_rel_balance, m.max_rel_balance);
        set.finite = set.finite && m.finite;
        rms_accum += m.rms_balance_accum;
        rms_count += m.steps;
    }
    if (rms_count > 0) {
        set.rms_balance = std::sqrt(rms_accum / static_cast<double>(rms_count));
    }
}

void print_balance_set(const BalanceSet& set) {
    for (const auto& m : set.metrics) {
        const double rms = m.steps > 0
                               ? std::sqrt(m.rms_balance_accum / static_cast<double>(m.steps))
                               : 0.0;
        const double mean_abs = m.steps > 0
                                    ? m.mean_abs_balance_accum / static_cast<double>(m.steps)
                                    : 0.0;
        std::printf("balance,%s,radius,%d,sites,%d,steps,%d,max_abs_dE,%.17g,max_abs_flux,%.17g,max_outward_flux,%.17g,max_inward_flux,%.17g,max_abs_balance,%.17g,rms_balance,%.17g,mean_abs_balance,%.17g,max_rel_balance,%.17g,max_energy,%.17g,finite,%s\n",
                    set.label.c_str(), m.radius, m.sites, m.steps,
                    m.max_abs_dE, m.max_abs_flux, m.max_outward_flux,
                    m.max_inward_flux, m.max_abs_balance, rms, mean_abs,
                    m.max_rel_balance, m.max_energy,
                    m.finite ? "true" : "false");
    }
    std::printf("balance_summary,%s,max_abs_dE,%.17g,max_abs_flux,%.17g,max_outward_flux,%.17g,max_inward_flux,%.17g,max_abs_balance,%.17g,rms_balance,%.17g,max_rel_balance,%.17g,finite,%s\n",
                set.label.c_str(), set.max_abs_dE, set.max_abs_flux,
                set.max_outward_flux, set.max_inward_flux,
                set.max_abs_balance, set.rms_balance, set.max_rel_balance,
                set.finite ? "true" : "false");
}

ModeReport run_mode(Mode mode,
                    const std::vector<BallMask>& balls,
                    const std::vector<EdgeOffset>& offsets) {
    ModeReport report;
    report.mode = mode_name(mode);
    report.beam_balance =
        make_balance_set((report.mode + "_beam_only_graph_balance").c_str(), balls);
    report.residual_balance =
        make_balance_set((report.mode + "_residual_graph_balance").c_str(), balls);
    report.repeat_balance =
        make_balance_set((report.mode + "_repeat_graph_balance").c_str(), balls);

    ftd::RenderBridge beam(L);
    ftd::RenderBridge charge(L);
    ftd::RenderBridge plus(L);
    ftd::RenderBridge repeat(L);

    const int beam_particle = initialize_arm(beam, mode, false, true);
    const int charge_particle = initialize_arm(charge, mode, true, false);
    const int plus_particle = initialize_arm(plus, mode, true, true);
    const int repeat_particle = initialize_arm(repeat, mode, true, true);

    FieldSnapshot beam_prev = capture(beam);
    FieldSnapshot charge_prev = capture(charge);
    FieldSnapshot plus_prev = capture(plus);
    FieldSnapshot repeat_prev = capture(repeat);
    FieldSnapshot residual_prev = residual(plus_prev, beam_prev, charge_prev);
    FieldSnapshot repeat_diff_prev = difference(plus_prev, repeat_prev);

    for (int tick = 0; tick < TICKS; ++tick) {
        beam.tick();
        charge.tick();
        plus.tick();
        repeat.tick();

        report.arms_finite = report.arms_finite &&
                             arm_finite(beam, beam_particle, false) &&
                             arm_finite(charge, charge_particle, true) &&
                             arm_finite(plus, plus_particle, true) &&
                             arm_finite(repeat, repeat_particle, true);

        FieldSnapshot beam_curr = capture(beam);
        FieldSnapshot charge_curr = capture(charge);
        FieldSnapshot plus_curr = capture(plus);
        FieldSnapshot repeat_curr = capture(repeat);
        FieldSnapshot residual_curr = residual(plus_curr, beam_curr, charge_curr);
        FieldSnapshot repeat_diff_curr = difference(plus_curr, repeat_curr);

        observe_balance(report.beam_balance, beam_prev, beam_curr, balls, offsets);
        observe_balance(report.residual_balance, residual_prev, residual_curr, balls, offsets);
        observe_balance(report.repeat_balance, repeat_diff_prev, repeat_diff_curr, balls, offsets);

        beam_prev = std::move(beam_curr);
        charge_prev = std::move(charge_curr);
        plus_prev = std::move(plus_curr);
        repeat_prev = std::move(repeat_curr);
        residual_prev = std::move(residual_curr);
        repeat_diff_prev = std::move(repeat_diff_curr);
    }

    report.arms_finite = report.arms_finite &&
                         report.beam_balance.finite &&
                         report.residual_balance.finite &&
                         report.repeat_balance.finite;
    return report;
}

bool deterministic(const BalanceSet& set) {
    return set.finite &&
           set.max_abs_balance <= MACHINE_GATE &&
           set.max_abs_flux <= MACHINE_GATE &&
           set.max_abs_dE <= MACHINE_GATE;
}

bool free_wave_continuity_valid(const BalanceSet& set) {
    return set.finite &&
           set.max_abs_balance <= BALANCE_ABS_GATE &&
           set.max_rel_balance <= BALANCE_REL_GATE;
}

bool quiet_residual(const BalanceSet& set) {
    return set.finite &&
           set.max_abs_balance <= MACHINE_GATE &&
           set.max_abs_flux <= MACHINE_GATE &&
           set.max_abs_dE <= MACHINE_GATE;
}

bool flux_detected(const BalanceSet& set) {
    return set.finite && set.max_outward_flux > GRAPH_FLUX_GATE;
}

bool source_like_detected(const BalanceSet& set) {
    return set.finite && set.max_abs_balance > BALANCE_ABS_GATE;
}

}  // namespace

int main() {
    std::printf("FTD-0291 Thomson native finite-volume continuity meter v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,alpha,%.17g,machine_gate,%.17g,balance_abs_gate,%.17g,balance_rel_gate,%.17g,graph_flux_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::ALPHA, MACHINE_GATE,
                BALANCE_ABS_GATE, BALANCE_REL_GATE, GRAPH_FLUX_GATE);
    std::printf("ball_radii");
    for (int r : BALL_RADII) std::printf(",%d", r);
    std::printf("\n");
    std::printf("scope,native_graph_finite_volume_continuity_not_alpha_or_cross_section\n");
    std::printf("energy,graph_hamiltonian,E_V=sum_inside_0p5_W2_plus_endpoint_attributed_0p5_c2_gradJ2\n");
    std::printf("current,F_i_to_j=-c2*w_ij*0p5*(W_i+W_j)_dot_(J_j-J_i),stencil,18_neighbor_laplacian\n");

    const std::vector<BallMask> balls = make_balls();
    const std::vector<EdgeOffset> offsets = stencil_offsets();

    const ModeReport locked = run_mode(Mode::LockedLinear, balls, offsets);
    const ModeReport legacy = run_mode(Mode::NativeLegacy, balls, offsets);
    const ModeReport emergent = run_mode(Mode::NativeEmergent, balls, offsets);

    print_balance_set(locked.beam_balance);
    print_balance_set(locked.residual_balance);
    print_balance_set(locked.repeat_balance);
    print_balance_set(legacy.beam_balance);
    print_balance_set(legacy.residual_balance);
    print_balance_set(legacy.repeat_balance);
    print_balance_set(emergent.beam_balance);
    print_balance_set(emergent.residual_balance);
    print_balance_set(emergent.repeat_balance);

    const bool finite = locked.arms_finite && legacy.arms_finite && emergent.arms_finite;
    const bool repeats_deterministic = deterministic(locked.repeat_balance) &&
                                       deterministic(legacy.repeat_balance) &&
                                       deterministic(emergent.repeat_balance);
    const bool locked_linear = quiet_residual(locked.residual_balance);
    const bool beam_continuity = free_wave_continuity_valid(locked.beam_balance) &&
                                 free_wave_continuity_valid(legacy.beam_balance) &&
                                 free_wave_continuity_valid(emergent.beam_balance);
    const bool legacy_flux = flux_detected(legacy.residual_balance);
    const bool legacy_source = source_like_detected(legacy.residual_balance);
    const bool emergent_flux = flux_detected(emergent.residual_balance);
    const bool emergent_source = source_like_detected(emergent.residual_balance);

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!repeats_deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (!locked_linear) {
        verdict = "LOCKED_LINEAR_SUPERPOSITION_FAILED";
    } else if (!beam_continuity) {
        verdict = "NATIVE_GRAPH_CONTINUITY_CANDIDATE_INVALIDATED";
    } else if (legacy_flux || legacy_source) {
        verdict = "NATIVE_LEGACY_GRAPH_FLUX_OR_SOURCE_DETECTED";
    } else if (emergent_flux) {
        verdict = "NATIVE_EMERGENT_OUTWARD_GRAPH_FLUX_DETECTED";
    } else if (emergent_source) {
        verdict = "NATIVE_EMERGENT_LOCAL_SOURCE_WITHOUT_OUTWARD_FLUX";
    } else {
        verdict = "NO_NATIVE_GRAPH_RADIATION_OR_SOURCE_ABOVE_GATE";
    }

    std::printf("gates,finite,%s,repeats_deterministic,%s,locked_linear,%s,beam_continuity,%s,legacy_flux,%s,legacy_source,%s,emergent_flux,%s,emergent_source,%s\n",
                finite ? "true" : "false",
                repeats_deterministic ? "true" : "false",
                locked_linear ? "true" : "false",
                beam_continuity ? "true" : "false",
                legacy_flux ? "true" : "false",
                legacy_source ? "true" : "false",
                emergent_flux ? "true" : "false",
                emergent_source ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,native_finite_volume_accounting_only_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL" ||
                   std::string(verdict) == "LOCKED_LINEAR_SUPERPOSITION_FAILED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
