/**
 * FTD-0289: Thomson flux-excess discriminator.
 *
 * Follow-up to FTD-0288. The dashboard showed that the visually dominant
 * recoil is flux-field motion. This fixed campaign subtracts the free-wave
 * and charge-only baselines:
 *
 *     residual = charge_plus_beam - beam_only - charge_only
 *
 * Scope:
 *   [MEASUREMENT] Machine-precision field/wave residual on the same fixed
 *   L=33, mode_n=4, amp=0.05 protocol.
 *
 * Non-scope:
 *   This is not an alpha derivation, not a Thomson cross-section derivation,
 *   and not a QED scattering amplitude. It is an instrument discriminator for
 *   excess lattice flux response after baseline subtraction.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <string>
#include <vector>

namespace {

constexpr int L = 33;
constexpr int TICKS = 200;
constexpr int MODE_N = 4;
constexpr int LOCAL_RADIUS = 8;
constexpr double AMP = 0.05;
constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double MACHINE_GATE = 1e-12;
constexpr double EXCESS_GATE = 1e-8;

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
    std::vector<double> values;
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

struct Residual {
    std::string label;
    double l2 = 0.0;
    double rel_l2 = 0.0;
    double max_abs = 0.0;
    double mean_abs = 0.0;
    std::size_t max_index = 0;
    double energy = 0.0;
    double local_energy = 0.0;
    double local_cx = 0.0;
    double local_cy = 0.0;
    double local_cz = 0.0;
    double local_cmag = 0.0;
    double transverse_centroid = 0.0;
    double comp_x_l2 = 0.0;
    double comp_y_l2 = 0.0;
    double comp_z_l2 = 0.0;
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

void configure(ftd::RenderBridge& rb, Mode mode) {
    rb.force_cpu();
    rb.seed_rng(2809);
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

int wrapped_step(int next, int prev) {
    int d = next - prev;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

Snapshot run_arm(Mode mode, const char* label, bool electron, bool beam) {
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
    bool missing_after_start = false;

    for (int tick = 0; tick < TICKS; ++tick) {
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
        }
    }

    Snapshot out;
    out.mode = mode_name(mode);
    out.label = label;
    out.tick = rb.current_tick();
    out.values.reserve(static_cast<std::size_t>(L) * L * L * 6);

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
    if (electron) {
        out.finite = out.finite && !missing_after_start && out.electron_present;
    }

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
        const double components[6] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
        };
        for (double c : components) {
            out.values.push_back(c);
            norm2 += c * c;
            out.finite = out.finite && std::isfinite(c);
        }
    }
    out.field_wave_l2 = std::sqrt(norm2);

    const double values[] = {
        out.field_wave_l2, out.field_energy, out.wave_energy, out.particle_ke,
        out.total_energy, out.poynting_x, out.poynting_y, out.poynting_z,
        out.disp_x, out.disp_y, out.disp_z, out.vx, out.vy, out.vz, out.speed,
    };
    for (double v : values) {
        out.finite = out.finite && std::isfinite(v);
    }
    return out;
}

Residual residual_field(const Snapshot& plus,
                        const Snapshot& beam,
                        const Snapshot& charge,
                        const char* label) {
    Residual out;
    out.label = label;
    const std::size_t n = plus.values.size();
    const int mc = L / 2;
    double l2 = 0.0;
    double comp_x2 = 0.0;
    double comp_y2 = 0.0;
    double comp_z2 = 0.0;
    double sum_abs = 0.0;
    double local_sx = 0.0;
    double local_sy = 0.0;
    double local_sz = 0.0;
    const int r2_max = LOCAL_RADIUS * LOCAL_RADIUS;
    for (std::size_t i = 0; i < n; ++i) {
        const double diff = plus.values[i] - beam.values[i] - charge.values[i];
        out.finite = out.finite && std::isfinite(diff);
        const double ad = std::abs(diff);
        l2 += diff * diff;
        sum_abs += ad;
        if (ad > out.max_abs) {
            out.max_abs = ad;
            out.max_index = i;
        }
        const int component = static_cast<int>(i % 6);
        if (component == 0 || component == 3) comp_x2 += diff * diff;
        if (component == 1 || component == 4) comp_y2 += diff * diff;
        if (component == 2 || component == 5) comp_z2 += diff * diff;
    }

    const std::size_t voxels = n / 6;
    for (std::size_t vi = 0; vi < voxels; ++vi) {
        const double dfx = plus.values[vi * 6 + 0] - beam.values[vi * 6 + 0] - charge.values[vi * 6 + 0];
        const double dfy = plus.values[vi * 6 + 1] - beam.values[vi * 6 + 1] - charge.values[vi * 6 + 1];
        const double dfz = plus.values[vi * 6 + 2] - beam.values[vi * 6 + 2] - charge.values[vi * 6 + 2];
        const double dwx = plus.values[vi * 6 + 3] - beam.values[vi * 6 + 3] - charge.values[vi * 6 + 3];
        const double dwy = plus.values[vi * 6 + 4] - beam.values[vi * 6 + 4] - charge.values[vi * 6 + 4];
        const double dwz = plus.values[vi * 6 + 5] - beam.values[vi * 6 + 5] - charge.values[vi * 6 + 5];
        const double e = 0.5 * (dfx * dfx + dfy * dfy + dfz * dfz +
                                dwx * dwx + dwy * dwy + dwz * dwz);
        out.energy += e;
        const int x = static_cast<int>(vi % L);
        const int y = static_cast<int>((vi / L) % L);
        const int z = static_cast<int>(vi / (L * L));
        int dx = x - mc;
        int dy = y - mc;
        int dz = z - mc;
        if (dx > L / 2) dx -= L;
        if (dx < -L / 2) dx += L;
        if (dy > L / 2) dy -= L;
        if (dy < -L / 2) dy += L;
        if (dz > L / 2) dz -= L;
        if (dz < -L / 2) dz += L;
        if (dx * dx + dy * dy + dz * dz <= r2_max) {
            out.local_energy += e;
            local_sx += static_cast<double>(dx) * e;
            local_sy += static_cast<double>(dy) * e;
            local_sz += static_cast<double>(dz) * e;
        }
    }

    out.l2 = std::sqrt(l2);
    out.mean_abs = (n > 0) ? sum_abs / static_cast<double>(n) : 0.0;
    out.rel_l2 = out.l2 / std::max(plus.field_wave_l2, 1e-300);
    out.comp_x_l2 = std::sqrt(comp_x2);
    out.comp_y_l2 = std::sqrt(comp_y2);
    out.comp_z_l2 = std::sqrt(comp_z2);
    if (out.local_energy > 0.0) {
        out.local_cx = local_sx / out.local_energy;
        out.local_cy = local_sy / out.local_energy;
        out.local_cz = local_sz / out.local_energy;
        out.local_cmag = std::sqrt(out.local_cx * out.local_cx +
                                   out.local_cy * out.local_cy +
                                   out.local_cz * out.local_cz);
        out.transverse_centroid = std::sqrt(out.local_cy * out.local_cy +
                                            out.local_cz * out.local_cz);
    }

    const double values[] = {
        out.l2, out.rel_l2, out.max_abs, out.mean_abs, out.energy,
        out.local_energy, out.local_cx, out.local_cy, out.local_cz,
        out.local_cmag, out.transverse_centroid, out.comp_x_l2,
        out.comp_y_l2, out.comp_z_l2,
    };
    for (double v : values) {
        out.finite = out.finite && std::isfinite(v);
    }
    return out;
}

Residual repeat_residual(const Snapshot& a, const Snapshot& b, const char* label) {
    Snapshot zero = a;
    std::fill(zero.values.begin(), zero.values.end(), 0.0);
    Residual r = residual_field(a, b, zero, label);
    return r;
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

void print_residual(const Residual& r) {
    std::printf("residual,%s,l2,%.17g,rel_l2,%.17g,max_abs,%.17g,mean_abs,%.17g,max_index,%zu,energy,%.17g,local_energy,%.17g,local_cx,%.17g,local_cy,%.17g,local_cz,%.17g,local_cmag,%.17g,transverse_centroid,%.17g,comp_x_l2,%.17g,comp_y_l2,%.17g,comp_z_l2,%.17g,finite,%s\n",
                r.label.c_str(), r.l2, r.rel_l2, r.max_abs, r.mean_abs,
                r.max_index, r.energy, r.local_energy, r.local_cx, r.local_cy,
                r.local_cz, r.local_cmag, r.transverse_centroid, r.comp_x_l2,
                r.comp_y_l2, r.comp_z_l2, r.finite ? "true" : "false");
}

bool detected(const Residual& r) {
    return r.finite && (r.max_abs > EXCESS_GATE || r.rel_l2 > EXCESS_GATE ||
                        r.local_energy > EXCESS_GATE * EXCESS_GATE);
}

}  // namespace

int main() {
    std::printf("FTD-0289 Thomson flux-excess discriminator v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,local_radius,%d,c_speed,%.17g,alpha,%.17g,machine_gate,%.17g,excess_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, LOCAL_RADIUS, ftd::C_SPEED, ftd::ALPHA,
                MACHINE_GATE, EXCESS_GATE);
    std::printf("scope,baseline_subtracted_flux_excess_not_alpha_derivation\n");
    std::printf("observable,residual=charge_plus_beam-minus-beam_only-minus-charge_only\n");

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

    const Residual locked_residual = residual_field(locked_plus, locked_beam, locked_charge, "locked_plus_minus_beam_minus_charge");
    const Residual locked_replay = repeat_residual(locked_plus, locked_repeat, "locked_repeat");
    const Residual legacy_residual = residual_field(legacy_plus, legacy_beam, legacy_charge, "legacy_plus_minus_beam_minus_charge");
    const Residual legacy_replay = repeat_residual(legacy_plus, legacy_repeat, "legacy_repeat");
    const Residual emergent_residual = residual_field(emergent_plus, emergent_beam, emergent_charge, "emergent_plus_minus_beam_minus_charge");
    const Residual emergent_replay = repeat_residual(emergent_plus, emergent_repeat, "emergent_repeat");

    print_residual(locked_residual);
    print_residual(locked_replay);
    print_residual(legacy_residual);
    print_residual(legacy_replay);
    print_residual(emergent_residual);
    print_residual(emergent_replay);

    const bool finite = locked_beam.finite && locked_charge.finite &&
                        locked_plus.finite && locked_repeat.finite &&
                        legacy_beam.finite && legacy_charge.finite &&
                        legacy_plus.finite && legacy_repeat.finite &&
                        emergent_beam.finite && emergent_charge.finite &&
                        emergent_plus.finite && emergent_repeat.finite &&
                        locked_residual.finite && locked_replay.finite &&
                        legacy_residual.finite && legacy_replay.finite &&
                        emergent_residual.finite && emergent_replay.finite;
    const bool deterministic = locked_replay.max_abs <= MACHINE_GATE &&
                               legacy_replay.max_abs <= MACHINE_GATE &&
                               emergent_replay.max_abs <= MACHINE_GATE;
    const bool locked_linear = locked_residual.max_abs <= MACHINE_GATE &&
                               locked_residual.rel_l2 <= MACHINE_GATE;
    const bool legacy_excess = detected(legacy_residual);
    const bool emergent_excess = detected(emergent_residual);
    const bool emergent_transverse = emergent_excess &&
        std::abs(emergent_residual.local_cy) > 10.0 * std::max(std::abs(emergent_residual.local_cz), EXCESS_GATE);

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (!locked_linear) {
        verdict = "BASELINE_SUBTRACTION_INVALIDATED";
    } else if (legacy_excess) {
        verdict = "NATIVE_LEGACY_EXCESS_FLUX_DEFLECTION_DETECTED";
    } else if (emergent_excess) {
        verdict = emergent_transverse
            ? "NATIVE_EMERGENT_EXCESS_TRANSVERSE_FLUX_DEFLECTION_DETECTED"
            : "NATIVE_EMERGENT_EXCESS_FLUX_DEFLECTION_DETECTED";
    } else {
        verdict = "NO_BASELINE_SUBTRACTED_EXCESS_FLUX_DEFLECTION";
    }

    std::printf("gates,finite,%s,deterministic,%s,locked_linear,%s,legacy_excess,%s,emergent_excess,%s,emergent_transverse,%s\n",
                finite ? "true" : "false",
                deterministic ? "true" : "false",
                locked_linear ? "true" : "false",
                legacy_excess ? "true" : "false",
                emergent_excess ? "true" : "false",
                emergent_transverse ? "true" : "false");
    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,field_residual_observable_only_no_alpha_or_cross_section_claim\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL" ||
                   std::string(verdict) == "BASELINE_SUBTRACTION_INVALIDATED"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
