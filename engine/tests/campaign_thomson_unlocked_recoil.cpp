/**
 * FTD-0288: Thomson unlocked recoil campaign.
 *
 * Follow-up to FTD-0287. The locked dashboard observatory showed field-level
 * linear superposition, not mechanical recoil. This campaign unlocks the
 * electron-like charge and asks what the production force paths do.
 *
 * Scope:
 *   [MEASUREMENT] Native engine force/movement response of one negative charge
 *   to a fixed y-polarized plane wave.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a claim that the diagnostic qE arm is native physics. The qE arm is
 *   an explicitly imposed instrument for locating the missing electric hook.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>

namespace {

constexpr int L = 33;
constexpr int TICKS = 200;
constexpr int MODE_N = 4;
constexpr double AMP = 0.05;
constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double MACHINE_GATE = 1e-12;
constexpr double RECOIL_GATE = 1e-8;

enum class Mode {
    LockedLinear,
    NativeLegacy,
    NativeEmergent,
    DiagnosticQE,
};

struct Track {
    bool present = false;
    int idx = -1;
    int x = 0;
    int y = 0;
    int z = 0;
    const ftd::Voxel* voxel = nullptr;
};

struct Metrics {
    std::string mode;
    std::string label;
    int tick = 0;
    bool electron_present = false;
    bool finite = true;
    int x = 0;
    int y = 0;
    int z = 0;
    double disp_x = 0.0;
    double disp_y = 0.0;
    double disp_z = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double vz = 0.0;
    double speed = 0.0;
    double max_speed = 0.0;
    double max_accel = 0.0;
    double max_diagnostic_qe_force = 0.0;
    double field_energy = 0.0;
    double wave_energy = 0.0;
    double particle_ke = 0.0;
    double total_energy = 0.0;
    double gauss_violation = 0.0;
    double max_gauss_error = 0.0;
    double poynting_x = 0.0;
    double poynting_y = 0.0;
    double poynting_z = 0.0;
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
        case Mode::LockedLinear: return "locked_linear_control";
        case Mode::NativeLegacy: return "native_legacy_unlocked";
        case Mode::NativeEmergent: return "native_emergent_unlocked";
        case Mode::DiagnosticQE: return "diagnostic_qE_unlocked";
    }
    return "unknown";
}

void configure(ftd::RenderBridge& rb, Mode mode) {
    rb.force_cpu();
    rb.seed_rng(2808);
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
    } else if (mode == Mode::NativeLegacy) {
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = false;
    } else if (mode == Mode::NativeEmergent) {
        rb.toggles.forces = true;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = true;
    } else if (mode == Mode::DiagnosticQE) {
        rb.toggles.forces = false;
        rb.toggles.movement = true;
        rb.toggles.emergent_forces = false;
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

int wrapped_step(int next, int prev) {
    int d = next - prev;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

void integrate_force_like_phase_forces(ftd::Voxel& v, const ftd::Vec3& force,
                                       double dt) {
    const double C = ftd::C_SPEED;
    const double C2 = C * C;
    const double latency = v.latency;
    const double L2 = latency * latency;
    const double one_L2 = std::max(1.0 - L2, ftd::BANDWIDTH_FLOOR);

    const double v2 = v.velocity.mag2();
    double budget = v2 / C2 + L2;
    if (budget > 1.0 - ftd::BANDWIDTH_FLOOR) {
        budget = 1.0 - ftd::BANDWIDTH_FLOOR;
    }
    const double gamma_in = 1.0 / std::sqrt(1.0 - budget);

    ftd::Vec3 p = v.velocity * gamma_in;
    p = p + force * dt;
    const double p2 = p.mag2();
    const double scale = C * std::sqrt(one_L2 / (C2 + p2));
    v.velocity = p * scale;
    v.accel_mag = force.mag();
}

double apply_diagnostic_qe(ftd::RenderBridge& rb, int particle_id) {
    Track t = find_electron(rb, particle_id);
    if (!t.present) {
        return 0.0;
    }
    auto& v = rb.voxels()[t.idx];
    if (v.locked) {
        return 0.0;
    }
    const ftd::Vec3 electric = v.wave_vel * -1.0;
    const ftd::Vec3 force = electric * (ftd::ALPHA * static_cast<double>(v.state));
    integrate_force_like_phase_forces(v, force, rb.dt());
    return force.mag();
}

bool finite_value(double x) {
    return std::isfinite(x);
}

Metrics run_arm(Mode mode, const char* label, bool electron, bool beam) {
    ftd::RenderBridge rb(L);
    configure(rb, mode);
    if (electron) {
        inject_electron(rb, mode == Mode::LockedLinear);
    }
    if (beam) {
        inject_plane_wave(rb);
    }

    const int mc = L / 2;
    const int initial_idx = rb.lattice().index(mc, mc, mc);
    const int particle_id = electron ? rb.voxels()[initial_idx].particle_id : -1;

    double unwrapped_x = 0.0;
    double unwrapped_y = 0.0;
    double unwrapped_z = 0.0;
    int prev_x = mc;
    int prev_y = mc;
    int prev_z = mc;
    double max_speed = 0.0;
    double max_accel = 0.0;
    double max_qe_force = 0.0;
    bool missing_after_start = false;

    for (int tick = 0; tick < TICKS; ++tick) {
        if (mode == Mode::DiagnosticQE && electron) {
            max_qe_force = std::max(max_qe_force, apply_diagnostic_qe(rb, particle_id));
        }
        rb.tick();
        Track t = find_electron(rb, particle_id);
        if (electron && !t.present) {
            missing_after_start = true;
            continue;
        }
        if (t.present) {
            unwrapped_x += wrapped_step(t.x, prev_x);
            unwrapped_y += wrapped_step(t.y, prev_y);
            unwrapped_z += wrapped_step(t.z, prev_z);
            prev_x = t.x;
            prev_y = t.y;
            prev_z = t.z;
            max_speed = std::max(max_speed, t.voxel->speed());
            max_accel = std::max(max_accel, t.voxel->accel_mag);
        }
    }

    Metrics out;
    out.mode = mode_name(mode);
    out.label = label;
    out.tick = rb.current_tick();

    Track final = find_electron(rb, particle_id);
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
    out.max_speed = max_speed;
    out.max_accel = max_accel;
    out.max_diagnostic_qe_force = max_qe_force;

    auto audit = rb.energy_audit();
    out.field_energy = audit.field_energy;
    out.wave_energy = audit.wave_energy;
    out.particle_ke = audit.particle_ke;
    out.total_energy = audit.total_energy;
    out.gauss_violation = audit.gauss_violation;
    out.max_gauss_error = audit.max_gauss_error;
    out.poynting_x = audit.total_poynting.x;
    out.poynting_y = audit.total_poynting.y;
    out.poynting_z = audit.total_poynting.z;

    const double values[] = {
        out.disp_x, out.disp_y, out.disp_z, out.vx, out.vy, out.vz,
        out.speed, out.max_speed, out.max_accel, out.max_diagnostic_qe_force,
        out.field_energy, out.wave_energy, out.particle_ke, out.total_energy,
        out.gauss_violation, out.max_gauss_error,
        out.poynting_x, out.poynting_y, out.poynting_z,
    };
    for (double v : values) {
        out.finite = out.finite && finite_value(v);
    }
    if (electron) {
        out.finite = out.finite && !missing_after_start && out.electron_present;
    }
    return out;
}

Delta delta_motion(const Metrics& plus, const Metrics& electron) {
    Delta d;
    d.disp_x = plus.disp_x - electron.disp_x;
    d.disp_y = plus.disp_y - electron.disp_y;
    d.disp_z = plus.disp_z - electron.disp_z;
    d.vel_x = plus.vx - electron.vx;
    d.vel_y = plus.vy - electron.vy;
    d.vel_z = plus.vz - electron.vz;
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
        d.finite = d.finite && finite_value(v);
    }
    return d;
}

Delta repeat_delta(const Metrics& a, const Metrics& b) {
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
    const double values[] = {
        d.disp_x, d.disp_y, d.disp_z, d.vel_x, d.vel_y, d.vel_z,
        d.disp_mag, d.vel_mag,
        a.total_energy - b.total_energy,
        a.field_energy - b.field_energy,
        a.wave_energy - b.wave_energy,
        a.particle_ke - b.particle_ke,
    };
    for (double v : values) {
        d.max_abs = std::max(d.max_abs, std::abs(v));
        d.finite = d.finite && finite_value(v);
    }
    return d;
}

void print_metrics(const Metrics& m) {
    std::printf("arm,mode,%s,label,%s,tick,%d,electron_present,%s,x,%d,y,%d,z,%d,disp_x,%.17g,disp_y,%.17g,disp_z,%.17g,vx,%.17g,vy,%.17g,vz,%.17g,speed,%.17g,max_speed,%.17g,max_accel,%.17g,max_diagnostic_qe_force,%.17g,finite,%s\n",
                m.mode.c_str(), m.label.c_str(), m.tick,
                m.electron_present ? "true" : "false", m.x, m.y, m.z,
                m.disp_x, m.disp_y, m.disp_z, m.vx, m.vy, m.vz,
                m.speed, m.max_speed, m.max_accel, m.max_diagnostic_qe_force,
                m.finite ? "true" : "false");
    std::printf("energy,mode,%s,label,%s,field,%.17g,wave,%.17g,particle_ke,%.17g,total,%.17g,gauss_violation,%.17g,max_gauss_error,%.17g,poynting_x,%.17g,poynting_y,%.17g,poynting_z,%.17g\n",
                m.mode.c_str(), m.label.c_str(), m.field_energy, m.wave_energy,
                m.particle_ke, m.total_energy, m.gauss_violation,
                m.max_gauss_error, m.poynting_x, m.poynting_y, m.poynting_z);
}

void print_delta(const char* label, const Delta& d) {
    std::printf("delta,%s,disp_x,%.17g,disp_y,%.17g,disp_z,%.17g,vel_x,%.17g,vel_y,%.17g,vel_z,%.17g,disp_mag,%.17g,vel_mag,%.17g,max_abs,%.17g,finite,%s\n",
                label, d.disp_x, d.disp_y, d.disp_z, d.vel_x, d.vel_y,
                d.vel_z, d.disp_mag, d.vel_mag, d.max_abs,
                d.finite ? "true" : "false");
}

bool detected(const Delta& d) {
    return d.finite && (d.disp_mag > RECOIL_GATE || d.vel_mag > RECOIL_GATE);
}

bool diagnostic_transverse_detected(const Delta& d) {
    const double lateral_competition = std::max(std::abs(d.disp_x), std::abs(d.disp_z));
    return detected(d) && std::abs(d.disp_y) > 10.0 * std::max(lateral_competition, RECOIL_GATE);
}

}  // namespace

int main() {
    std::printf("FTD-0288 Thomson unlocked recoil campaign v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_speed,%.17g,alpha,%.17g,machine_gate,%.17g,recoil_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, ftd::C_SPEED, ftd::ALPHA,
                MACHINE_GATE, RECOIL_GATE);
    std::printf("scope,native_recoil_measurement_not_alpha_derivation\n");
    std::printf("ingredients,wave_propagation,true,coupling,true,damping,false,genesis,false,gauss_projection,false,gravity,false,poisson_coulomb,false,lorentz_force,false,diagnostic_qE,imposed_not_native\n");

    const Metrics locked_beam = run_arm(Mode::LockedLinear, "beam_only", false, true);
    const Metrics locked_electron = run_arm(Mode::LockedLinear, "electron_only", true, false);
    const Metrics locked_plus = run_arm(Mode::LockedLinear, "electron_plus_beam", true, true);
    const Metrics locked_repeat = run_arm(Mode::LockedLinear, "electron_plus_beam_repeat", true, true);

    const Metrics legacy_electron = run_arm(Mode::NativeLegacy, "electron_only", true, false);
    const Metrics legacy_plus = run_arm(Mode::NativeLegacy, "electron_plus_beam", true, true);
    const Metrics legacy_repeat = run_arm(Mode::NativeLegacy, "electron_plus_beam_repeat", true, true);

    const Metrics emergent_electron = run_arm(Mode::NativeEmergent, "electron_only", true, false);
    const Metrics emergent_plus = run_arm(Mode::NativeEmergent, "electron_plus_beam", true, true);
    const Metrics emergent_repeat = run_arm(Mode::NativeEmergent, "electron_plus_beam_repeat", true, true);

    const Metrics diagnostic_electron = run_arm(Mode::DiagnosticQE, "electron_only", true, false);
    const Metrics diagnostic_plus = run_arm(Mode::DiagnosticQE, "electron_plus_beam", true, true);
    const Metrics diagnostic_repeat = run_arm(Mode::DiagnosticQE, "electron_plus_beam_repeat", true, true);

    print_metrics(locked_beam);
    print_metrics(locked_electron);
    print_metrics(locked_plus);
    print_metrics(locked_repeat);
    print_metrics(legacy_electron);
    print_metrics(legacy_plus);
    print_metrics(legacy_repeat);
    print_metrics(emergent_electron);
    print_metrics(emergent_plus);
    print_metrics(emergent_repeat);
    print_metrics(diagnostic_electron);
    print_metrics(diagnostic_plus);
    print_metrics(diagnostic_repeat);

    const Delta locked_replay = repeat_delta(locked_plus, locked_repeat);
    const Delta legacy_extra = delta_motion(legacy_plus, legacy_electron);
    const Delta legacy_replay = repeat_delta(legacy_plus, legacy_repeat);
    const Delta emergent_extra = delta_motion(emergent_plus, emergent_electron);
    const Delta emergent_replay = repeat_delta(emergent_plus, emergent_repeat);
    const Delta diagnostic_extra = delta_motion(diagnostic_plus, diagnostic_electron);
    const Delta diagnostic_replay = repeat_delta(diagnostic_plus, diagnostic_repeat);

    print_delta("locked_linear_repeat", locked_replay);
    print_delta("native_legacy_extra_plus_minus_electron", legacy_extra);
    print_delta("native_legacy_repeat", legacy_replay);
    print_delta("native_emergent_extra_plus_minus_electron", emergent_extra);
    print_delta("native_emergent_repeat", emergent_replay);
    print_delta("diagnostic_qE_extra_plus_minus_electron", diagnostic_extra);
    print_delta("diagnostic_qE_repeat", diagnostic_replay);

    const bool finite = locked_beam.finite && locked_electron.finite &&
                        locked_plus.finite && locked_repeat.finite &&
                        legacy_electron.finite && legacy_plus.finite &&
                        legacy_repeat.finite && emergent_electron.finite &&
                        emergent_plus.finite && emergent_repeat.finite &&
                        diagnostic_electron.finite && diagnostic_plus.finite &&
                        diagnostic_repeat.finite && legacy_extra.finite &&
                        emergent_extra.finite && diagnostic_extra.finite;
    const bool deterministic = locked_replay.max_abs <= MACHINE_GATE &&
                               legacy_replay.max_abs <= MACHINE_GATE &&
                               emergent_replay.max_abs <= MACHINE_GATE &&
                               diagnostic_replay.max_abs <= MACHINE_GATE;
    const bool legacy_recoil = detected(legacy_extra);
    const bool emergent_recoil = detected(emergent_extra);
    const bool diagnostic_recoil = diagnostic_transverse_detected(diagnostic_extra);

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (legacy_recoil) {
        verdict = "NATIVE_LEGACY_RECOIL_DETECTED";
    } else if (emergent_recoil) {
        verdict = "NATIVE_EMERGENT_FLUX_GRADIENT_RECOIL_DETECTED";
    } else if (diagnostic_recoil) {
        verdict = "NATIVE_NO_RECOIL_ELECTRIC_HOOK_REQUIRED_DIAGNOSTIC_QE_RESPONDS";
    } else {
        verdict = "NO_RECOIL_NATIVE_OR_DIAGNOSTIC";
    }

    std::printf("gates,finite,%s,deterministic,%s,legacy_recoil,%s,emergent_recoil,%s,diagnostic_qE_transverse_recoil,%s\n",
                finite ? "true" : "false", deterministic ? "true" : "false",
                legacy_recoil ? "true" : "false",
                emergent_recoil ? "true" : "false",
                diagnostic_recoil ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,native_force_paths_measured_diagnostic_qE_is_imposed_not_a_derivation\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
