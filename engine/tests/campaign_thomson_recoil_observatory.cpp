/**
 * FTD-0287: Thomson recoil observatory.
 *
 * A machine-precision companion to the scale-0 dashboard scenario
 * `s0-field-thomson-scattering`.
 *
 * Scope:
 *   [INSTRUMENT] Fixed protocol for observing a y-polarized plane wave
 *   passing a locked electron-like charge on the discrete lattice.
 *
 * Non-scope:
 *   This is not an alpha derivation. It is not a mechanical recoil claim.
 *   The charge is locked and forces/movement are disabled to isolate the
 *   field response that the dashboard visualizes.
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
constexpr double AMP = 0.05;
constexpr double PI = 3.141592653589793238462643383279502884;
constexpr double MACHINE_GATE = 1e-12;

struct Metrics {
    int tick = 0;
    double center_flux_y = 0.0;
    double center_wv_x = 0.0;
    double center_wv_y = 0.0;
    double center_wv_z = 0.0;
    double electron_vx = 0.0;
    double electron_vy = 0.0;
    double electron_vz = 0.0;
    double energy_center_r3 = 0.0;
    double energy_lateral_y10_r3 = 0.0;
    double energy_forward_x10_r3 = 0.0;
    double field_energy = 0.0;
    double wave_energy = 0.0;
    double total_energy = 0.0;
    double gauss_violation = 0.0;
    double max_gauss_error = 0.0;
    double poynting_x = 0.0;
    double poynting_y = 0.0;
    double poynting_z = 0.0;
    double field_wave_l2 = 0.0;
    bool electron_present = false;
    bool finite = true;
};

struct Snapshot {
    std::string label;
    std::vector<double> values;
    Metrics metrics;
};

struct Residual {
    double l2 = 0.0;
    double rel_l2 = 0.0;
    double max_abs = 0.0;
    double mean_abs = 0.0;
    std::size_t max_index = 0;
    bool finite = true;
};

void configure_locked_field_probe(ftd::RenderBridge& rb) {
    rb.force_cpu();
    rb.seed_rng(2807);
    rb.toggles.disable_all();
    rb.toggles.strict_validation = true;
    rb.toggles.wave_propagation = true;
    rb.toggles.coupling = true;
    rb.toggles.damping = false;
    rb.toggles.genesis = false;
    rb.toggles.gauss_projection = false;
    rb.toggles.forces = false;
    rb.toggles.gravity = false;
    rb.toggles.poisson_coulomb = false;
    rb.toggles.movement = false;
    rb.toggles.lorentz_force = false;
    rb.toggles.dual_substrate = false;
}

double energy_in_sphere(const ftd::RenderBridge& rb, int cx, int cy, int cz,
                        int radius) {
    const int n = rb.lattice().size();
    const int r2_max = radius * radius;
    const auto& voxels = rb.voxels();
    double energy = 0.0;
    for (int dz = -radius; dz <= radius; ++dz) {
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx * dx + dy * dy + dz * dz > r2_max) {
                    continue;
                }
                const int x = rb.lattice().wrap(cx + dx);
                const int y = rb.lattice().wrap(cy + dy);
                const int z = rb.lattice().wrap(cz + dz);
                const auto& v = voxels[rb.lattice().index(x, y, z)];
                energy += 0.5 * (v.flux.mag2() + v.wave_vel.mag2());
            }
        }
    }
    return energy;
}

void inject_locked_electron(ftd::RenderBridge& rb) {
    const int mc = L / 2;
    rb.inject_particle(mc, mc, mc, static_cast<int8_t>(-1), {0.0, 0.0, 0.0},
                       static_cast<int8_t>(-1), static_cast<int8_t>(0));
    rb.voxels()[rb.lattice().index(mc, mc, mc)].locked = true;
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

Snapshot run_arm(const char* label, bool electron, bool beam) {
    ftd::RenderBridge rb(L);
    configure_locked_field_probe(rb);
    if (electron) {
        inject_locked_electron(rb);
    }
    if (beam) {
        inject_plane_wave(rb);
    }

    rb.run(TICKS);

    Snapshot out;
    out.label = label;
    out.values.reserve(static_cast<std::size_t>(L) * L * L * 6);

    const int mc = L / 2;
    const int center_idx = rb.lattice().index(mc, mc, mc);
    const auto& voxels = static_cast<const ftd::RenderBridge&>(rb).voxels();
    const auto& center = voxels[center_idx];
    auto audit = rb.energy_audit();

    out.metrics.tick = rb.current_tick();
    out.metrics.center_flux_y = center.flux.y;
    out.metrics.center_wv_x = center.wave_vel.x;
    out.metrics.center_wv_y = center.wave_vel.y;
    out.metrics.center_wv_z = center.wave_vel.z;
    out.metrics.electron_present = center.state < 0;
    out.metrics.electron_vx = center.velocity.x;
    out.metrics.electron_vy = center.velocity.y;
    out.metrics.electron_vz = center.velocity.z;
    out.metrics.energy_center_r3 = energy_in_sphere(rb, mc, mc, mc, 3);
    out.metrics.energy_lateral_y10_r3 = energy_in_sphere(rb, mc, mc + 10, mc, 3);
    out.metrics.energy_forward_x10_r3 = energy_in_sphere(rb, mc + 10, mc, mc, 3);
    out.metrics.field_energy = audit.field_energy;
    out.metrics.wave_energy = audit.wave_energy;
    out.metrics.total_energy = audit.total_energy;
    out.metrics.gauss_violation = audit.gauss_violation;
    out.metrics.max_gauss_error = audit.max_gauss_error;
    out.metrics.poynting_x = audit.total_poynting.x;
    out.metrics.poynting_y = audit.total_poynting.y;
    out.metrics.poynting_z = audit.total_poynting.z;

    double norm2 = 0.0;
    for (const auto& v : voxels) {
        const double components[6] = {
            v.flux.x, v.flux.y, v.flux.z,
            v.wave_vel.x, v.wave_vel.y, v.wave_vel.z,
        };
        for (double c : components) {
            out.values.push_back(c);
            norm2 += c * c;
            if (!std::isfinite(c)) {
                out.metrics.finite = false;
            }
        }
    }
    out.metrics.field_wave_l2 = std::sqrt(norm2);
    out.metrics.finite = out.metrics.finite &&
                          std::isfinite(out.metrics.field_wave_l2) &&
                          std::isfinite(out.metrics.total_energy);
    return out;
}

Residual superposition_residual(const Snapshot& plus,
                                const Snapshot& beam,
                                const Snapshot& electron) {
    Residual out;
    const std::size_t n = plus.values.size();
    double l2 = 0.0;
    double sum_abs = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double diff = plus.values[i] - beam.values[i] - electron.values[i];
        if (!std::isfinite(diff)) {
            out.finite = false;
        }
        const double ad = std::abs(diff);
        l2 += diff * diff;
        sum_abs += ad;
        if (ad > out.max_abs) {
            out.max_abs = ad;
            out.max_index = i;
        }
    }
    out.l2 = std::sqrt(l2);
    out.mean_abs = (n > 0) ? sum_abs / static_cast<double>(n) : 0.0;
    out.rel_l2 = out.l2 / std::max(plus.metrics.field_wave_l2, 1e-300);
    out.finite = out.finite && std::isfinite(out.l2) &&
                 std::isfinite(out.rel_l2) && std::isfinite(out.max_abs);
    return out;
}

Residual repeat_residual(const Snapshot& a, const Snapshot& b) {
    Residual out;
    const std::size_t n = a.values.size();
    double l2 = 0.0;
    double sum_abs = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
        const double diff = a.values[i] - b.values[i];
        if (!std::isfinite(diff)) {
            out.finite = false;
        }
        const double ad = std::abs(diff);
        l2 += diff * diff;
        sum_abs += ad;
        if (ad > out.max_abs) {
            out.max_abs = ad;
            out.max_index = i;
        }
    }
    out.l2 = std::sqrt(l2);
    out.mean_abs = (n > 0) ? sum_abs / static_cast<double>(n) : 0.0;
    out.rel_l2 = out.l2 / std::max(a.metrics.field_wave_l2, 1e-300);
    out.finite = out.finite && std::isfinite(out.l2) &&
                 std::isfinite(out.rel_l2) && std::isfinite(out.max_abs);
    return out;
}

void print_arm(const Snapshot& s) {
    const auto& m = s.metrics;
    std::printf("arm,%s,tick,%d,electron_present,%s,center_flux_y,%.17g,center_wv_x,%.17g,center_wv_y,%.17g,center_wv_z,%.17g,electron_vx,%.17g,electron_vy,%.17g,electron_vz,%.17g\n",
                s.label.c_str(), m.tick, m.electron_present ? "true" : "false",
                m.center_flux_y, m.center_wv_x, m.center_wv_y, m.center_wv_z,
                m.electron_vx, m.electron_vy, m.electron_vz);
    std::printf("energy,%s,center_r3,%.17g,lateral_y10_r3,%.17g,forward_x10_r3,%.17g,field,%.17g,wave,%.17g,total,%.17g,field_wave_l2,%.17g\n",
                s.label.c_str(), m.energy_center_r3, m.energy_lateral_y10_r3,
                m.energy_forward_x10_r3, m.field_energy, m.wave_energy,
                m.total_energy, m.field_wave_l2);
    std::printf("audit,%s,gauss_violation,%.17g,max_gauss_error,%.17g,poynting_x,%.17g,poynting_y,%.17g,poynting_z,%.17g,finite,%s\n",
                s.label.c_str(), m.gauss_violation, m.max_gauss_error,
                m.poynting_x, m.poynting_y, m.poynting_z,
                m.finite ? "true" : "false");
}

void print_residual(const char* label, const Residual& r) {
    std::printf("residual,%s,l2,%.17g,rel_l2,%.17g,max_abs,%.17g,mean_abs,%.17g,max_index,%zu,finite,%s\n",
                label, r.l2, r.rel_l2, r.max_abs, r.mean_abs, r.max_index,
                r.finite ? "true" : "false");
}

}  // namespace

int main() {
    std::printf("FTD-0287 Thomson recoil observatory v1\n");
    std::printf("protocol,L,%d,ticks,%d,mode_n,%d,amp,%.17g,c_speed,%.17g,double_epsilon,%.17g,machine_gate,%.17g\n",
                L, TICKS, MODE_N, AMP, ftd::C_SPEED,
                std::numeric_limits<double>::epsilon(), MACHINE_GATE);
    std::printf("scope,instrument_not_alpha_derivation\n");
    std::printf("ingredients,wave_propagation,true,coupling,true,damping,false,gauss_projection,false,forces,false,movement,false,poisson_coulomb,false,locked_charge,true\n");

    const Snapshot beam = run_arm("beam_only", false, true);
    const Snapshot electron = run_arm("electron_only", true, false);
    const Snapshot plus = run_arm("electron_plus_beam", true, true);
    const Snapshot plus_repeat = run_arm("electron_plus_beam_repeat", true, true);

    print_arm(beam);
    print_arm(electron);
    print_arm(plus);
    print_arm(plus_repeat);

    const Residual super = superposition_residual(plus, beam, electron);
    const Residual repeat = repeat_residual(plus, plus_repeat);
    print_residual("plus_minus_beam_minus_electron", super);
    print_residual("electron_plus_beam_repeat", repeat);

    const bool finite = beam.metrics.finite && electron.metrics.finite &&
                        plus.metrics.finite && plus_repeat.metrics.finite &&
                        super.finite && repeat.finite;
    const bool deterministic = repeat.max_abs <= MACHINE_GATE;
    const bool linear = super.max_abs <= MACHINE_GATE && super.rel_l2 <= MACHINE_GATE;

    const char* verdict = "UNCLASSIFIED";
    if (!finite) {
        verdict = "NONFINITE_PROTOCOL";
    } else if (!deterministic) {
        verdict = "NONDETERMINISTIC_PROTOCOL";
    } else if (linear) {
        verdict = "LINEAR_SUPERPOSITION_NO_RECOIL_OBSERVED";
    } else {
        verdict = "INTERACTION_RESIDUAL_DETECTED";
    }

    std::printf("verdict,%s\n", verdict);
    std::printf("interpretation,locked_charge_field_observatory_not_mechanical_recoil\n");

    return std::string(verdict) == "UNCLASSIFIED" ||
                   std::string(verdict) == "NONFINITE_PROTOCOL" ||
                   std::string(verdict) == "NONDETERMINISTIC_PROTOCOL"
               ? EXIT_FAILURE
               : EXIT_SUCCESS;
}
