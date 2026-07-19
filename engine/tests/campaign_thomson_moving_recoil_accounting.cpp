/**
 * FTD-0297: Thomson moving-recoil source/work accounting.
 *
 * Follow-up to FTD-0296. The fixed-charge source/work identity closes the
 * phase_read/phase_write part of the tick. This campaign unlocks the charge
 * and measures whether the same finite-volume balance still closes after the
 * full tick, including phase_forces and phase_movement.
 *
 * Scope:
 *   [MEASUREMENT] Source/work accounting residual for locked, native-legacy,
 *   and native-emergent recoil modes under the fixed Thomson setup.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a radiation claim. A nonzero residual would mean post-write
 *   movement/transport work remains to be modeled.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int L = 33;
constexpr int TICKS = 200;
constexpr int MODE_N = 4;
constexpr double AMP = 0.05;
constexpr long double PI = 3.141592653589793238462643383279502884L;
constexpr long double BALANCE_ABS_GATE = 1e-10L;
constexpr long double BALANCE_REL_GATE = 1e-12L;
constexpr double MACHINE_GATE = 1e-12;
constexpr double RECOIL_GATE = 1e-8;
constexpr std::array<int, 5> BALL_RADII = {5, 7, 9, 11, 13};

enum class Mode {
    LockedFixed,
    NativeLegacy,
    NativeEmergent,
};

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

struct Track {
    bool present = false;
    int idx = -1;
    int x = 0;
    int y = 0;
    int z = 0;
    const ftd::Voxel* voxel = nullptr;
};

struct RunResult {
    std::string mode;
    std::string label;
    int tick = 0;
    BalanceSet balances;
    bool electron_present = false;
    bool finite = true;
    int x = 0;
    int y = 0;
    int z = 0;
    int hop_x = 0;
    int hop_y = 0;
    int hop_z = 0;
    int transport_events = 0;
    double disp_x = 0.0;
    double disp_y = 0.0;
    double disp_z = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double vz = 0.0;
    double speed = 0.0;
    double max_speed = 0.0;
    double max_accel = 0.0;
};

struct Delta {
    double disp_x = 0.0;
    double disp_y = 0.0;
    double disp_z = 0.0;
    double vel_x = 0.0;
    double vel_y = 0.0;
    double vel_z = 0.0;
    double disp_mag = 0.0;
    double vel_mag = 0.0;
    double max_abs = 0.0;
    bool finite = true;
};

const char* mode_name(Mode mode) {
    switch (mode) {
        case Mode::LockedFixed: return "locked_fixed_source";
        case Mode::NativeLegacy: return "native_legacy_unlocked";
        case Mode::NativeEmergent: return "native_emergent_unlocked";
    }
    return "unknown";
}

bool finite_value(long double x) {
    return std::isfinite(x);
}

int wrap_delta(int d) {
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

int wrapped_step(int next, int prev) {
    return wrap_delta(next - prev);
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

void configure(ftd::RenderBridge& rb, Mode mode) {
    rb.force_cpu();
    rb.seed_rng(2817);
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
    rb.toggles.symplectic_leapfrog = false;

    if (mode == Mode::LockedFixed) {
        rb.toggles.forces = false;
        rb.toggles.movement = false;
        rb.toggles.emergent_forces = false;
    } else if (mode == Mode::NativeLegacy) {
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = false;
    } else {
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
        // mirror phase_read.cpp for the recoil accounting to balance).
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

RunResult run_arm(Mode mode, const char* label, bool beam) {
    const std::vector<EdgeOffset> offsets = stencil_offsets();
    const std::vector<BallMask> balls = make_balls();

    ftd::RenderBridge rb(L);
    configure(rb, mode);
    inject_electron(rb, mode == Mode::LockedFixed);
    if (beam) inject_plane_wave(rb);

    const int mc = L / 2;
    const int initial_idx = rb.lattice().index(mc, mc, mc);
    const int particle_id = rb.voxels()[initial_idx].particle_id;

    RunResult out;
    out.mode = mode_name(mode);
    out.label = label;
    out.balances = make_balance_set(balls);

    double unwrapped_x = 0.0;
    double unwrapped_y = 0.0;
    double unwrapped_z = 0.0;
    int prev_x = mc;
    int prev_y = mc;
    int prev_z = mc;
    bool missing_after_start = false;
    Snapshot prev = capture(rb);

    for (int tick = 0; tick < TICKS; ++tick) {
        const SourceSnapshot src = capture_source(rb);
        rb.tick();
        Snapshot curr = capture(rb);
        observe(out.balances, prev, curr, src, balls, offsets);
        prev = std::move(curr);

        Track t = find_electron(rb, particle_id);
        if (!t.present) {
            missing_after_start = true;
            continue;
        }

        const int sx = wrapped_step(t.x, prev_x);
        const int sy = wrapped_step(t.y, prev_y);
        const int sz = wrapped_step(t.z, prev_z);
        unwrapped_x += sx;
        unwrapped_y += sy;
        unwrapped_z += sz;
        out.hop_x += sx;
        out.hop_y += sy;
        out.hop_z += sz;
        if (sx != 0 || sy != 0 || sz != 0) ++out.transport_events;
        prev_x = t.x;
        prev_y = t.y;
        prev_z = t.z;
        out.max_speed = std::max(out.max_speed, t.voxel->speed());
        out.max_accel = std::max(out.max_accel, t.voxel->accel_mag);
    }

    Track final = find_electron(rb, particle_id);
    out.tick = rb.current_tick();
    out.electron_present = final.present;
    if (final.present) {
        out.x = final.x;
        out.y = final.y;
        out.z = final.z;
        out.disp_x = unwrapped_x + final.voxel->remainder.x;
        out.disp_y = unwrapped_y + final.voxel->remainder.y;
        out.disp_z = unwrapped_z + final.voxel->remainder.z;
        out.vx = final.voxel->velocity.x;
        out.vy = final.voxel->velocity.y;
        out.vz = final.voxel->velocity.z;
        out.speed = final.voxel->speed();
    }

    const double values[] = {
        out.disp_x, out.disp_y, out.disp_z, out.vx, out.vy, out.vz,
        out.speed, out.max_speed, out.max_accel,
        static_cast<double>(out.balances.max_abs_balance),
        static_cast<double>(out.balances.max_scale_rel_balance),
    };
    for (double v : values) out.finite = out.finite && std::isfinite(v);
    out.finite = out.finite && out.balances.finite && !missing_after_start &&
                 out.electron_present;
    return out;
}

Delta motion_delta(const RunResult& plus, const RunResult& charge) {
    Delta d;
    d.disp_x = plus.disp_x - charge.disp_x;
    d.disp_y = plus.disp_y - charge.disp_y;
    d.disp_z = plus.disp_z - charge.disp_z;
    d.vel_x = plus.vx - charge.vx;
    d.vel_y = plus.vy - charge.vy;
    d.vel_z = plus.vz - charge.vz;
    d.disp_mag = std::sqrt(d.disp_x * d.disp_x + d.disp_y * d.disp_y +
                           d.disp_z * d.disp_z);
    d.vel_mag = std::sqrt(d.vel_x * d.vel_x + d.vel_y * d.vel_y +
                          d.vel_z * d.vel_z);
    const double values[] = {
        d.disp_x, d.disp_y, d.disp_z, d.vel_x, d.vel_y, d.vel_z,
        d.disp_mag, d.vel_mag,
    };
    for (double v : values) {
        d.max_abs = std::max(d.max_abs, std::abs(v));
        d.finite = d.finite && std::isfinite(v);
    }
    return d;
}

Delta repeat_delta(const RunResult& a, const RunResult& b) {
    Delta d;
    d.disp_x = a.disp_x - b.disp_x;
    d.disp_y = a.disp_y - b.disp_y;
    d.disp_z = a.disp_z - b.disp_z;
    d.vel_x = a.vx - b.vx;
    d.vel_y = a.vy - b.vy;
    d.vel_z = a.vz - b.vz;
    d.disp_mag = std::sqrt(d.disp_x * d.disp_x + d.disp_y * d.disp_y +
                           d.disp_z * d.disp_z);
    d.vel_mag = std::sqrt(d.vel_x * d.vel_x + d.vel_y * d.vel_y +
                          d.vel_z * d.vel_z);
    const long double balance_values[] = {
        a.balances.max_abs_dE - b.balances.max_abs_dE,
        a.balances.max_abs_flux - b.balances.max_abs_flux,
        a.balances.max_abs_work - b.balances.max_abs_work,
        a.balances.max_abs_balance - b.balances.max_abs_balance,
        a.balances.max_scale_rel_balance - b.balances.max_scale_rel_balance,
        a.balances.rms_balance - b.balances.rms_balance,
    };
    for (long double v : balance_values) {
        d.max_abs = std::max(d.max_abs, static_cast<double>(std::abs(v)));
        d.finite = d.finite && finite_value(v);
    }
    const double values[] = {
        d.disp_x, d.disp_y, d.disp_z, d.vel_x, d.vel_y, d.vel_z,
        d.disp_mag, d.vel_mag,
        static_cast<double>(a.transport_events - b.transport_events),
        static_cast<double>(a.hop_x - b.hop_x),
        static_cast<double>(a.hop_y - b.hop_y),
        static_cast<double>(a.hop_z - b.hop_z),
    };
    for (double v : values) {
        d.max_abs = std::max(d.max_abs, std::abs(v));
        d.finite = d.finite && std::isfinite(v);
    }
    return d;
}

bool accounting_closes(const RunResult& r) {
    return r.finite && r.balances.max_abs_balance <= BALANCE_ABS_GATE &&
           r.balances.max_scale_rel_balance <= BALANCE_REL_GATE;
}

bool recoil_detected(const Delta& d) {
    return d.finite && (d.disp_mag > RECOIL_GATE || d.vel_mag > RECOIL_GATE);
}

void print_balance_set(const RunResult& r) {
    for (const auto& m : r.balances.metrics) {
        const long double rms = m.steps > 0
                                    ? std::sqrt(m.rms_balance_accum /
                                                static_cast<long double>(m.steps))
                                    : 0.0L;
        const long double mean_abs = m.steps > 0
                                         ? m.mean_abs_balance_accum /
                                               static_cast<long double>(m.steps)
                                         : 0.0L;
        std::printf("balance,mode,%s,label,%s,radius,%d,sites,%d,steps,%d,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_abs_work,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,mean_abs_balance,%.21Lg,max_scale_rel_balance,%.21Lg,max_abs_energy,%.21Lg,finite,%s\n",
                    r.mode.c_str(), r.label.c_str(), m.radius, m.sites,
                    m.steps, m.max_abs_dE, m.max_abs_flux, m.max_abs_work,
                    m.max_abs_balance, rms, mean_abs,
                    m.max_scale_rel_balance, m.max_abs_energy,
                    m.finite ? "true" : "false");
    }
    std::printf("balance_summary,mode,%s,label,%s,max_abs_dE,%.21Lg,max_abs_flux,%.21Lg,max_abs_work,%.21Lg,max_abs_balance,%.21Lg,rms_balance,%.21Lg,max_scale_rel_balance,%.21Lg,finite,%s\n",
                r.mode.c_str(), r.label.c_str(), r.balances.max_abs_dE,
                r.balances.max_abs_flux, r.balances.max_abs_work,
                r.balances.max_abs_balance, r.balances.rms_balance,
                r.balances.max_scale_rel_balance,
                r.balances.finite ? "true" : "false");
}

void print_run(const RunResult& r) {
    print_balance_set(r);
    std::printf("motion,mode,%s,label,%s,tick,%d,electron_present,%s,x,%d,y,%d,z,%d,hop_x,%d,hop_y,%d,hop_z,%d,transport_events,%d,disp_x,%.17g,disp_y,%.17g,disp_z,%.17g,vx,%.17g,vy,%.17g,vz,%.17g,speed,%.17g,max_speed,%.17g,max_accel,%.17g,finite,%s\n",
                r.mode.c_str(), r.label.c_str(), r.tick,
                r.electron_present ? "true" : "false", r.x, r.y, r.z,
                r.hop_x, r.hop_y, r.hop_z, r.transport_events,
                r.disp_x, r.disp_y, r.disp_z, r.vx, r.vy, r.vz,
                r.speed, r.max_speed, r.max_accel,
                r.finite ? "true" : "false");
}

void print_delta(const char* label, const Delta& d) {
    std::printf("delta,%s,disp_x,%.17g,disp_y,%.17g,disp_z,%.17g,vel_x,%.17g,vel_y,%.17g,vel_z,%.17g,disp_mag,%.17g,vel_mag,%.17g,max_abs,%.17g,finite,%s\n",
                label, d.disp_x, d.disp_y, d.disp_z, d.vel_x, d.vel_y,
                d.vel_z, d.disp_mag, d.vel_mag, d.max_abs,
                d.finite ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0297 Thomson moving-recoil source/work accounting v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_wave,%.17g,g_c,%.17g,alpha,%.17g,balance_abs_gate,%.21Lg,balance_scale_rel_gate,%.21Lg,machine_gate,%.17g,recoil_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, ftd::C_WAVE, ftd::G_C, ftd::ALPHA,
                BALANCE_ABS_GATE, BALANCE_REL_GATE, MACHINE_GATE, RECOIL_GATE);
    std::printf("ball_radii");
    for (int r : BALL_RADII) std::printf(",%d", r);
    std::printf("\n");
    std::printf("scope,moving_recoil_accounting_not_alpha_or_cross_section\n");
    std::printf("identity_tested,Delta_H_V_plus_Phi_source_free_out_minus_Work_source_equals_post_write_transport_residual\n");
    std::printf("modes,locked_fixed_source,native_legacy_unlocked,native_emergent_unlocked\n");
    std::printf("source,S=G_C*(grad_state+curl_state_velocity),work,additive_phase_read_write_source_work\n");
    std::printf("transport_note,subvoxel_velocity_remainder_has_no_field_transport_integer_hops_carry_flux\n");
    std::printf("accumulation,long_double_kahan\n");

    const RunResult locked_charge = run_arm(Mode::LockedFixed, "charge_only", false);
    const RunResult locked_plus = run_arm(Mode::LockedFixed, "charge_plus_beam", true);
    const RunResult locked_repeat = run_arm(Mode::LockedFixed, "charge_plus_beam_repeat", true);

    const RunResult legacy_charge = run_arm(Mode::NativeLegacy, "charge_only", false);
    const RunResult legacy_plus = run_arm(Mode::NativeLegacy, "charge_plus_beam", true);
    const RunResult legacy_repeat = run_arm(Mode::NativeLegacy, "charge_plus_beam_repeat", true);

    const RunResult emergent_charge = run_arm(Mode::NativeEmergent, "charge_only", false);
    const RunResult emergent_plus = run_arm(Mode::NativeEmergent, "charge_plus_beam", true);
    const RunResult emergent_repeat = run_arm(Mode::NativeEmergent, "charge_plus_beam_repeat", true);

    print_run(locked_charge);
    print_run(locked_plus);
    print_run(locked_repeat);
    print_run(legacy_charge);
    print_run(legacy_plus);
    print_run(legacy_repeat);
    print_run(emergent_charge);
    print_run(emergent_plus);
    print_run(emergent_repeat);

    const Delta locked_replay = repeat_delta(locked_plus, locked_repeat);
    const Delta legacy_replay = repeat_delta(legacy_plus, legacy_repeat);
    const Delta emergent_replay = repeat_delta(emergent_plus, emergent_repeat);
    const Delta locked_extra = motion_delta(locked_plus, locked_charge);
    const Delta legacy_extra = motion_delta(legacy_plus, legacy_charge);
    const Delta emergent_extra = motion_delta(emergent_plus, emergent_charge);

    print_delta("locked_fixed_repeat", locked_replay);
    print_delta("native_legacy_repeat", legacy_replay);
    print_delta("native_emergent_repeat", emergent_replay);
    print_delta("locked_fixed_extra_plus_minus_charge", locked_extra);
    print_delta("native_legacy_extra_plus_minus_charge", legacy_extra);
    print_delta("native_emergent_extra_plus_minus_charge", emergent_extra);

    const bool finite = locked_charge.finite && locked_plus.finite &&
                        locked_repeat.finite && legacy_charge.finite &&
                        legacy_plus.finite && legacy_repeat.finite &&
                        emergent_charge.finite && emergent_plus.finite &&
                        emergent_repeat.finite && locked_replay.finite &&
                        legacy_replay.finite && emergent_replay.finite &&
                        locked_extra.finite && legacy_extra.finite &&
                        emergent_extra.finite;
    const bool deterministic = locked_replay.max_abs <= MACHINE_GATE &&
                               legacy_replay.max_abs <= MACHINE_GATE &&
                               emergent_replay.max_abs <= MACHINE_GATE;
    const bool locked_accounting = accounting_closes(locked_charge) &&
                                   accounting_closes(locked_plus);
    const bool legacy_accounting = accounting_closes(legacy_charge) &&
                                   accounting_closes(legacy_plus);
    const bool emergent_accounting = accounting_closes(emergent_charge) &&
                                     accounting_closes(emergent_plus);
    const bool legacy_recoil = recoil_detected(legacy_extra);
    const bool emergent_recoil = recoil_detected(emergent_extra);
    const bool any_transport = legacy_plus.transport_events > 0 ||
                               emergent_plus.transport_events > 0 ||
                               legacy_charge.transport_events > 0 ||
                               emergent_charge.transport_events > 0;

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (!locked_accounting) {
        verdict = "LOCKED_FIXED_SOURCE_ACCOUNTING_FAILED";
    } else if (!legacy_accounting || !emergent_accounting) {
        verdict = "MOVING_TRANSPORT_RESIDUAL_DETECTED";
    } else if ((legacy_recoil || emergent_recoil) && !any_transport) {
        verdict = "SUBVOXEL_RECOIL_ACCOUNTED_BY_ADDITIVE_SOURCE_WORK";
    } else if ((legacy_recoil || emergent_recoil) && any_transport) {
        verdict = "MOVING_RECOIL_ACCOUNTED_THROUGH_TRANSPORT_EVENT";
    } else {
        verdict = "MOVING_SOURCE_ACCOUNTING_CONFIRMED_NO_EXTRA_RECOIL";
    }

    std::printf("gates,finite,%s,deterministic,%s,locked_accounting,%s,legacy_accounting,%s,emergent_accounting,%s,legacy_recoil,%s,emergent_recoil,%s,any_transport,%s\n",
                finite ? "true" : "false", deterministic ? "true" : "false",
                locked_accounting ? "true" : "false",
                legacy_accounting ? "true" : "false",
                emergent_accounting ? "true" : "false",
                legacy_recoil ? "true" : "false",
                emergent_recoil ? "true" : "false",
                any_transport ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,source_work_accounting_for_moving_charge_no_alpha_cross_section_or_radiation_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL" ||
                   std::string(verdict) == "LOCKED_FIXED_SOURCE_ACCOUNTING_FAILED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
