/**
 * FTD-0290: Thomson radiation shell meter.
 *
 * Follow-up to FTD-0289. The flux-excess discriminator found an above-gate
 * residual field/wave response in the native emergent flux-gradient channel.
 * This fixed campaign asks whether that residual carries outward Poynting
 * flux through spherical shells around the charge.
 *
 * Scope:
 *   [MEASUREMENT] Baseline-subtracted residual-field Poynting flux on the
 *   same fixed L=33, mode_n=4, amp=0.05 protocol.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a QED scattering amplitude. It is an instrument discriminator for
 *   outward shell power in the already frozen FTD-0288/0289 setup.
 */

#include "ftd/constants.h"
#include "ftd/field_operators.h"
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
constexpr double SHELL_HALF_WIDTH = 0.5;
constexpr double MACHINE_GATE = 1e-12;
constexpr double POWER_GATE = 1e-8;
constexpr double ANGULAR_GATE = 1e-3;
constexpr std::array<int, 6> SHELL_RADII = {5, 7, 9, 11, 13, 15};

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

struct Snapshot {
    std::string mode;
    std::string label;
    std::vector<ftd::Vec3> flux;
    std::vector<ftd::Vec3> wave;
    int tick = 0;
    bool finite = true;
    bool electron_present = false;
    double field_wave_l2 = 0.0;
    double field_energy = 0.0;
    double wave_energy = 0.0;
    double particle_ke = 0.0;
    double total_energy = 0.0;
    double poynting_x = 0.0;
    double poynting_y = 0.0;
    double poynting_z = 0.0;
    double disp_x = 0.0;
    double disp_y = 0.0;
    double disp_z = 0.0;
    double vx = 0.0;
    double vy = 0.0;
    double vz = 0.0;
    double speed = 0.0;
};

struct ShellMetric {
    int radius = 0;
    int samples = 0;
    double net_power = 0.0;
    double outward_power = 0.0;
    double inward_power = 0.0;
    double abs_power = 0.0;
    double dipole_x = 0.0;
    double dipole_y = 0.0;
    double dipole_z = 0.0;
    double dipole_mag = 0.0;
    double quad_xx = 0.0;
    double quad_yy = 0.0;
    double quad_zz = 0.0;
    double quad_xy = 0.0;
    double quad_xz = 0.0;
    double quad_yz = 0.0;
    double quad_mag = 0.0;
    double momentum_x = 0.0;
    double momentum_y = 0.0;
    double momentum_z = 0.0;
    double momentum_mag = 0.0;
    bool finite = true;
};

struct ShellSet {
    std::string label;
    std::vector<ShellMetric> shells;
    double max_abs_power = 0.0;
    double max_outward_power = 0.0;
    double max_net_abs_power = 0.0;
    double strongest_quadrupole = 0.0;
    double strongest_dipole = 0.0;
    int strongest_radius = 0;
    bool finite = true;
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

void configure(ftd::RenderBridge& rb, Mode mode) {
    rb.force_cpu();
    rb.seed_rng(2810);
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

Snapshot run_arm(Mode mode, const char* label, bool electron, bool beam) {
    ftd::RenderBridge rb(L);
    configure(rb, mode);
    if (electron) inject_electron(rb, mode == Mode::LockedLinear);
    if (beam) inject_plane_wave(rb);

    const int mc = L / 2;
    const int initial_idx = rb.lattice().index(mc, mc, mc);
    const int particle_id = electron ? rb.voxels()[initial_idx].particle_id : -1;
    double unwrapped_x = 0.0;
    double unwrapped_y = 0.0;
    double unwrapped_z = 0.0;
    int prev_x = mc, prev_y = mc, prev_z = mc;
    bool missing_after_start = false;

    for (int tick = 0; tick < TICKS; ++tick) {
        rb.tick();
        Track t = find_electron(rb, particle_id);
        if (electron && !t.present) {
            missing_after_start = true;
            continue;
        }
        if (t.present) {
            unwrapped_x += wrap_delta(t.x - prev_x);
            unwrapped_y += wrap_delta(t.y - prev_y);
            unwrapped_z += wrap_delta(t.z - prev_z);
            prev_x = t.x;
            prev_y = t.y;
            prev_z = t.z;
        }
    }

    Snapshot out;
    out.mode = mode_name(mode);
    out.label = label;
    out.tick = rb.current_tick();
    out.flux.reserve(static_cast<std::size_t>(L) * L * L);
    out.wave.reserve(static_cast<std::size_t>(L) * L * L);

    Track final = find_electron(rb, particle_id);
    out.electron_present = final.present;
    if (final.present) {
        out.disp_x = unwrapped_x + final.voxel->remainder.x;
        out.disp_y = unwrapped_y + final.voxel->remainder.y;
        out.disp_z = unwrapped_z + final.voxel->remainder.z;
        out.vx = final.voxel->velocity.x;
        out.vy = final.voxel->velocity.y;
        out.vz = final.voxel->velocity.z;
        out.speed = final.voxel->speed();
    }
    if (electron) out.finite = out.finite && !missing_after_start && out.electron_present;

    auto audit = rb.energy_audit();
    out.field_energy = audit.field_energy;
    out.wave_energy = audit.wave_energy;
    out.particle_ke = audit.particle_ke;
    out.total_energy = audit.total_energy;
    out.poynting_x = audit.total_poynting.x;
    out.poynting_y = audit.total_poynting.y;
    out.poynting_z = audit.total_poynting.z;

    double norm2 = 0.0;
    for (const auto& v : rb.voxels()) {
        out.flux.push_back(v.flux);
        out.wave.push_back(v.wave_vel);
        norm2 += v.flux.mag2() + v.wave_vel.mag2();
        const double vals[] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
        };
        for (double val : vals) out.finite = out.finite && finite_value(val);
    }
    out.field_wave_l2 = std::sqrt(norm2);

    const double vals[] = {
        out.field_wave_l2, out.field_energy, out.wave_energy, out.particle_ke,
        out.total_energy, out.poynting_x, out.poynting_y, out.poynting_z,
        out.disp_x, out.disp_y, out.disp_z, out.vx, out.vy, out.vz, out.speed,
    };
    for (double val : vals) out.finite = out.finite && finite_value(val);
    return out;
}

Snapshot zero_like(const Snapshot& s) {
    Snapshot z = s;
    std::fill(z.flux.begin(), z.flux.end(), ftd::Vec3{});
    std::fill(z.wave.begin(), z.wave.end(), ftd::Vec3{});
    z.field_wave_l2 = 0.0;
    return z;
}

ShellSet shell_meter(const Snapshot& plus,
                     const Snapshot& beam,
                     const Snapshot& charge,
                     const char* label) {
    ShellSet out;
    out.label = label;
    const std::size_t n = plus.flux.size();
    std::vector<ftd::Vec3> rflux(n);
    std::vector<ftd::Vec3> rwave(n);
    for (std::size_t i = 0; i < n; ++i) {
        rflux[i] = plus.flux[i] - beam.flux[i] - charge.flux[i];
        rwave[i] = plus.wave[i] - beam.wave[i] - charge.wave[i];
        const double vals[] = {
            rflux[i].x, rflux[i].y, rflux[i].z,
            rwave[i].x, rwave[i].y, rwave[i].z,
        };
        for (double val : vals) out.finite = out.finite && finite_value(val);
    }

    const ftd::Lattice lattice(L);
    const int mc = L / 2;
    for (int radius : SHELL_RADII) {
        ShellMetric m;
        m.radius = radius;
        double dipole_x = 0.0, dipole_y = 0.0, dipole_z = 0.0;
        double qxx = 0.0, qyy = 0.0, qzz = 0.0;
        double qxy = 0.0, qxz = 0.0, qyz = 0.0;
        for (int i = 0; i < static_cast<int>(n); ++i) {
            const auto c = lattice.coord(i);
            const int dx_i = wrap_delta(c.x - mc);
            const int dy_i = wrap_delta(c.y - mc);
            const int dz_i = wrap_delta(c.z - mc);
            const double dx = static_cast<double>(dx_i);
            const double dy = static_cast<double>(dy_i);
            const double dz = static_cast<double>(dz_i);
            const double r = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (r <= 0.0 || std::abs(r - static_cast<double>(radius)) > SHELL_HALF_WIDTH) {
                continue;
            }
            const ftd::Vec3 normal{dx / r, dy / r, dz / r};
            const ftd::Vec3 electric = rwave[i] * -1.0;
            const ftd::Vec3 magnetic = ftd::curl_from_flux_array(rflux, lattice, i);
            const ftd::Vec3 poynting = ftd::Vec3::cross(electric, magnetic);
            const double radial = poynting.dot(normal);
            const double w = std::abs(radial);

            ++m.samples;
            m.net_power += radial;
            m.outward_power += std::max(0.0, radial);
            m.inward_power += std::max(0.0, -radial);
            m.abs_power += w;
            m.momentum_x += poynting.x;
            m.momentum_y += poynting.y;
            m.momentum_z += poynting.z;
            dipole_x += w * normal.x;
            dipole_y += w * normal.y;
            dipole_z += w * normal.z;
            qxx += w * (normal.x * normal.x - 1.0 / 3.0);
            qyy += w * (normal.y * normal.y - 1.0 / 3.0);
            qzz += w * (normal.z * normal.z - 1.0 / 3.0);
            qxy += w * normal.x * normal.y;
            qxz += w * normal.x * normal.z;
            qyz += w * normal.y * normal.z;

            const double vals[] = {
                electric.x, electric.y, electric.z,
                magnetic.x, magnetic.y, magnetic.z,
                poynting.x, poynting.y, poynting.z, radial,
            };
            for (double val : vals) m.finite = m.finite && finite_value(val);
        }

        if (m.abs_power > 0.0) {
            m.dipole_x = dipole_x / m.abs_power;
            m.dipole_y = dipole_y / m.abs_power;
            m.dipole_z = dipole_z / m.abs_power;
            m.quad_xx = qxx / m.abs_power;
            m.quad_yy = qyy / m.abs_power;
            m.quad_zz = qzz / m.abs_power;
            m.quad_xy = qxy / m.abs_power;
            m.quad_xz = qxz / m.abs_power;
            m.quad_yz = qyz / m.abs_power;
        }
        m.dipole_mag = std::sqrt(m.dipole_x * m.dipole_x +
                                 m.dipole_y * m.dipole_y +
                                 m.dipole_z * m.dipole_z);
        m.quad_mag = std::sqrt(m.quad_xx * m.quad_xx +
                               m.quad_yy * m.quad_yy +
                               m.quad_zz * m.quad_zz +
                               2.0 * (m.quad_xy * m.quad_xy +
                                      m.quad_xz * m.quad_xz +
                                      m.quad_yz * m.quad_yz));
        m.momentum_mag = std::sqrt(m.momentum_x * m.momentum_x +
                                   m.momentum_y * m.momentum_y +
                                   m.momentum_z * m.momentum_z);

        const double vals[] = {
            m.net_power, m.outward_power, m.inward_power, m.abs_power,
            m.dipole_x, m.dipole_y, m.dipole_z, m.dipole_mag,
            m.quad_xx, m.quad_yy, m.quad_zz, m.quad_xy, m.quad_xz, m.quad_yz,
            m.quad_mag, m.momentum_x, m.momentum_y, m.momentum_z, m.momentum_mag,
        };
        for (double val : vals) m.finite = m.finite && finite_value(val);

        out.max_abs_power = std::max(out.max_abs_power, m.abs_power);
        out.max_outward_power = std::max(out.max_outward_power, m.outward_power);
        out.max_net_abs_power = std::max(out.max_net_abs_power, std::abs(m.net_power));
        if (m.abs_power >= POWER_GATE && m.quad_mag > out.strongest_quadrupole) {
            out.strongest_quadrupole = m.quad_mag;
            out.strongest_dipole = m.dipole_mag;
            out.strongest_radius = m.radius;
        }
        out.finite = out.finite && m.finite;
        out.shells.push_back(m);
    }
    return out;
}

ShellSet repeat_meter(const Snapshot& a, const Snapshot& b, const char* label) {
    const Snapshot zero = zero_like(a);
    return shell_meter(a, b, zero, label);
}

void print_snapshot(const Snapshot& s) {
    std::printf("arm,mode,%s,label,%s,tick,%d,electron_present,%s,disp_x,%.17g,disp_y,%.17g,disp_z,%.17g,vx,%.17g,vy,%.17g,vz,%.17g,speed,%.17g,finite,%s\n",
                s.mode.c_str(), s.label.c_str(), s.tick,
                s.electron_present ? "true" : "false",
                s.disp_x, s.disp_y, s.disp_z, s.vx, s.vy, s.vz, s.speed,
                s.finite ? "true" : "false");
    std::printf("energy,mode,%s,label,%s,field,%.17g,wave,%.17g,particle_ke,%.17g,total,%.17g,poynting_x,%.17g,poynting_y,%.17g,poynting_z,%.17g,field_wave_l2,%.17g\n",
                s.mode.c_str(), s.label.c_str(), s.field_energy, s.wave_energy,
                s.particle_ke, s.total_energy, s.poynting_x, s.poynting_y,
                s.poynting_z, s.field_wave_l2);
}

void print_shell_set(const ShellSet& set) {
    for (const auto& m : set.shells) {
        std::printf("shell,%s,radius,%d,samples,%d,net_power,%.17g,outward_power,%.17g,inward_power,%.17g,abs_power,%.17g,dipole_x,%.17g,dipole_y,%.17g,dipole_z,%.17g,dipole_mag,%.17g,quad_xx,%.17g,quad_yy,%.17g,quad_zz,%.17g,quad_xy,%.17g,quad_xz,%.17g,quad_yz,%.17g,quad_mag,%.17g,momentum_x,%.17g,momentum_y,%.17g,momentum_z,%.17g,momentum_mag,%.17g,finite,%s\n",
                    set.label.c_str(), m.radius, m.samples,
                    m.net_power, m.outward_power, m.inward_power, m.abs_power,
                    m.dipole_x, m.dipole_y, m.dipole_z, m.dipole_mag,
                    m.quad_xx, m.quad_yy, m.quad_zz, m.quad_xy, m.quad_xz,
                    m.quad_yz, m.quad_mag,
                    m.momentum_x, m.momentum_y, m.momentum_z, m.momentum_mag,
                    m.finite ? "true" : "false");
    }
    std::printf("shell_summary,%s,max_abs_power,%.17g,max_outward_power,%.17g,max_net_abs_power,%.17g,strongest_radius,%d,strongest_dipole,%.17g,strongest_quadrupole,%.17g,finite,%s\n",
                set.label.c_str(), set.max_abs_power, set.max_outward_power,
                set.max_net_abs_power, set.strongest_radius, set.strongest_dipole,
                set.strongest_quadrupole, set.finite ? "true" : "false");
}

bool detected(const ShellSet& set) {
    return set.finite && (set.max_outward_power > POWER_GATE ||
                          set.max_abs_power > POWER_GATE);
}

}  // namespace

int main() {
    std::printf("FTD-0290 Thomson radiation shell meter v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,shell_half_width,%.17g,c_speed,%.17g,alpha,%.17g,machine_gate,%.17g,power_gate,%.17g,angular_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, SHELL_HALF_WIDTH, ftd::C_SPEED,
                ftd::ALPHA, MACHINE_GATE, POWER_GATE, ANGULAR_GATE);
    std::printf("shell_radii");
    for (int r : SHELL_RADII) std::printf(",%d", r);
    std::printf("\n");
    std::printf("scope,residual_field_poynting_shell_meter_not_alpha_or_cross_section\n");
    std::printf("observable,S_res=E_res_cross_B_res_from_residual_field_charge_plus_beam-minus-beam_only-minus-charge_only\n");

    const Snapshot locked_beam = run_arm(Mode::LockedLinear, "beam_only", false, true);
    const Snapshot locked_charge = run_arm(Mode::LockedLinear, "charge_only", true, false);
    const Snapshot locked_plus = run_arm(Mode::LockedLinear, "charge_plus_beam", true, true);
    const Snapshot locked_repeat = run_arm(Mode::LockedLinear, "charge_plus_beam_repeat", true, true);

    const Snapshot legacy_beam = run_arm(Mode::NativeLegacy, "beam_only", false, true);
    const Snapshot legacy_charge = run_arm(Mode::NativeLegacy, "charge_only", true, false);
    const Snapshot legacy_plus = run_arm(Mode::NativeLegacy, "charge_plus_beam", true, true);
    const Snapshot legacy_repeat = run_arm(Mode::NativeLegacy, "charge_plus_beam_repeat", true, true);

    const Snapshot emergent_beam = run_arm(Mode::NativeEmergent, "beam_only", false, true);
    const Snapshot emergent_charge = run_arm(Mode::NativeEmergent, "charge_only", true, false);
    const Snapshot emergent_plus = run_arm(Mode::NativeEmergent, "charge_plus_beam", true, true);
    const Snapshot emergent_repeat = run_arm(Mode::NativeEmergent, "charge_plus_beam_repeat", true, true);

    print_snapshot(locked_beam);
    print_snapshot(locked_charge);
    print_snapshot(locked_plus);
    print_snapshot(locked_repeat);
    print_snapshot(legacy_beam);
    print_snapshot(legacy_charge);
    print_snapshot(legacy_plus);
    print_snapshot(legacy_repeat);
    print_snapshot(emergent_beam);
    print_snapshot(emergent_charge);
    print_snapshot(emergent_plus);
    print_snapshot(emergent_repeat);

    const ShellSet locked_shell = shell_meter(locked_plus, locked_beam, locked_charge, "locked_residual_shells");
    const ShellSet locked_replay = repeat_meter(locked_plus, locked_repeat, "locked_repeat_shells");
    const ShellSet legacy_shell = shell_meter(legacy_plus, legacy_beam, legacy_charge, "legacy_residual_shells");
    const ShellSet legacy_replay = repeat_meter(legacy_plus, legacy_repeat, "legacy_repeat_shells");
    const ShellSet emergent_shell = shell_meter(emergent_plus, emergent_beam, emergent_charge, "emergent_residual_shells");
    const ShellSet emergent_replay = repeat_meter(emergent_plus, emergent_repeat, "emergent_repeat_shells");

    print_shell_set(locked_shell);
    print_shell_set(locked_replay);
    print_shell_set(legacy_shell);
    print_shell_set(legacy_replay);
    print_shell_set(emergent_shell);
    print_shell_set(emergent_replay);

    const bool finite = locked_beam.finite && locked_charge.finite &&
                        locked_plus.finite && locked_repeat.finite &&
                        legacy_beam.finite && legacy_charge.finite &&
                        legacy_plus.finite && legacy_repeat.finite &&
                        emergent_beam.finite && emergent_charge.finite &&
                        emergent_plus.finite && emergent_repeat.finite &&
                        locked_shell.finite && locked_replay.finite &&
                        legacy_shell.finite && legacy_replay.finite &&
                        emergent_shell.finite && emergent_replay.finite;
    const bool deterministic = locked_replay.max_abs_power <= MACHINE_GATE &&
                               legacy_replay.max_abs_power <= MACHINE_GATE &&
                               emergent_replay.max_abs_power <= MACHINE_GATE;
    const bool locked_linear = locked_shell.max_abs_power <= MACHINE_GATE &&
                               locked_shell.max_outward_power <= MACHINE_GATE;
    const bool legacy_radiation = detected(legacy_shell);
    const bool emergent_radiation = detected(emergent_shell);
    const bool emergent_structured = emergent_radiation &&
                                     emergent_shell.strongest_quadrupole > ANGULAR_GATE;

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (!locked_linear) {
        verdict = "BASELINE_RADIATION_METER_INVALIDATED";
    } else if (legacy_radiation) {
        verdict = "NATIVE_LEGACY_RESIDUAL_RADIATION_DETECTED";
    } else if (emergent_structured) {
        verdict = "NATIVE_EMERGENT_STRUCTURED_RESIDUAL_RADIATION_DETECTED";
    } else if (emergent_radiation) {
        verdict = "NATIVE_EMERGENT_OUTWARD_RESIDUAL_POWER_DETECTED";
    } else {
        verdict = "NO_BASELINE_SUBTRACTED_OUTWARD_POWER";
    }

    std::printf("gates,finite,%s,deterministic,%s,locked_linear,%s,legacy_radiation,%s,emergent_radiation,%s,emergent_structured,%s\n",
                finite ? "true" : "false",
                deterministic ? "true" : "false",
                locked_linear ? "true" : "false",
                legacy_radiation ? "true" : "false",
                emergent_radiation ? "true" : "false",
                emergent_structured ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,residual_field_shell_power_only_no_alpha_cross_section_or_qed_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL" ||
                   std::string(verdict) == "BASELINE_RADIATION_METER_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
