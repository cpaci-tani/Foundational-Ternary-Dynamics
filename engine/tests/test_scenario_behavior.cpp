/**
 * Behavioral regression tests for the Scale-0 scenarios whose initial data
 * are intended to evolve, rather than merely render a recognizable picture.
 *
 * These checks deliberately avoid particle-name assertions. They test only
 * native observables: transversality, translation, common linear-wave speed,
 * pair bookkeeping, and whether candidate composites are actually unlocked.
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

struct Profile {
    double weight = 0.0;
    double centroid = 0.0;
    double width = 0.0;
};

struct HarmonicProjection {
    double sine = 0.0;
    double cosine = 0.0;
};

HarmonicProjection project_jz_harmonic(const ftd::RenderBridge& rb, int mode_n) {
    const int L = rb.lattice().size();
    HarmonicProjection out;
    for (int x = 0; x < L; ++x) {
        const double phase = 2.0 * ftd::PI * mode_n * x / L;
        double plane_sum = 0.0;
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            plane_sum += rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))].flux.z;
        }
        out.sine += plane_sum * std::sin(phase);
        out.cosine += plane_sum * std::cos(phase);
    }
    const double norm = 2.0 / static_cast<double>(L * L * L);
    out.sine *= norm;
    out.cosine *= norm;
    return out;
}

HarmonicProjection project_flux_harmonic(const ftd::RenderBridge& rb,
                                         int mode_n, int axis) {
    const int L = rb.lattice().size();
    HarmonicProjection out;
    for (int x = 0; x < L; ++x) {
        const double phase = 2.0 * ftd::PI * mode_n * x / L;
        double plane_sum = 0.0;
        for (int y = 0; y < L; ++y)
        for (int z = 0; z < L; ++z) {
            const auto& j = rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))].flux;
            plane_sum += axis == 0 ? j.x : (axis == 1 ? j.y : j.z);
        }
        out.sine += plane_sum * std::sin(phase);
        out.cosine += plane_sum * std::cos(phase);
    }
    const double norm = 2.0 / static_cast<double>(L * L * L);
    out.sine *= norm;
    out.cosine *= norm;
    return out;
}

Profile x_profile(const ftd::RenderBridge& rb,
                  const std::function<bool(int)>& include_y = {},
                  int component_axis = -1) {
    const int L = rb.lattice().size();
    std::vector<double> slice(static_cast<std::size_t>(L), 0.0);
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y) {
        if (include_y && !include_y(y)) continue;
        for (int z = 0; z < L; ++z) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))];
            if (component_axis < 0) {
                slice[static_cast<std::size_t>(x)] += v.flux.mag2() + v.wave_vel.mag2();
            } else {
                const double j = component_axis == 0 ? v.flux.x
                    : component_axis == 1 ? v.flux.y : v.flux.z;
                const double w = component_axis == 0 ? v.wave_vel.x
                    : component_axis == 1 ? v.wave_vel.y : v.wave_vel.z;
                slice[static_cast<std::size_t>(x)] += j*j + w*w;
            }
        }
    }

    Profile out;
    double cosine = 0.0;
    double sine = 0.0;
    for (int x = 0; x < L; ++x) {
        const double w = slice[static_cast<std::size_t>(x)];
        const double a = 2.0 * ftd::PI * static_cast<double>(x) / L;
        out.weight += w;
        cosine += w * std::cos(a);
        sine += w * std::sin(a);
    }
    if (out.weight == 0.0) return out;
    double angle = std::atan2(sine, cosine);
    if (angle < 0.0) angle += 2.0 * ftd::PI;
    out.centroid = angle * L / (2.0 * ftd::PI);

    for (int x = 0; x < L; ++x) {
        double dx = x - out.centroid;
        while (dx > 0.5 * L) dx -= L;
        while (dx < -0.5 * L) dx += L;
        out.width += slice[static_cast<std::size_t>(x)] * dx * dx;
    }
    out.width = std::sqrt(out.width / out.weight);
    return out;
}

double forward_delta(double end, double start, int L) {
    double dx = end - start;
    while (dx < -0.5 * L) dx += L;
    while (dx > 0.5 * L) dx -= L;
    return dx;
}

double normalized_divergence(const ftd::RenderBridge& rb) {
    double div2 = 0.0;
    double flux2 = 0.0;
    for (std::size_t i = 0; i < rb.lattice().total_sites(); ++i) {
        const double d = rb.divergence_flux(static_cast<int>(i));
        div2 += d * d;
        flux2 += rb.voxels()[i].flux.mag2();
    }
    return std::sqrt(div2 / std::max(1e-30, flux2));
}

double periodic_modified_hamiltonian(const ftd::RenderBridge& rb) {
    double kinetic = 0.0;
    double cross = 0.0;
    double gradient = 0.0;
    for (std::size_t i = 0; i < rb.lattice().total_sites(); ++i) {
        const auto& v = rb.voxels()[i];
        const ftd::Vec3 lap = rb.laplacian_flux(static_cast<int>(i));
        kinetic += v.wave_vel.mag2();
        cross += v.wave_vel.dot(lap);
        gradient -= v.flux.dot(lap);
    }
    const double c2 = ftd::C_SPEED * ftd::C_SPEED;
    return 0.5 * kinetic + 0.5 * c2 * cross + 0.5 * c2 * gradient;
}

double x_reflection_error(const ftd::RenderBridge& rb, double parity) {
    const int L = rb.lattice().size();
    double residual2 = 0.0;
    double norm2 = 0.0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& a = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(x, y, z))];
        const auto& b = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(L - 1 - x, y, z))];
        const ftd::Vec3 dj = a.flux - b.flux * parity;
        const ftd::Vec3 dw = a.wave_vel - b.wave_vel * parity;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += a.flux.mag2() + a.wave_vel.mag2();
    }
    return std::sqrt(residual2 / std::max(1e-30, norm2));
}

double z_reflection_error(const ftd::RenderBridge& rb, double parity) {
    const int L = rb.lattice().size();
    double residual2 = 0.0;
    double norm2 = 0.0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& a = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(x, y, z))];
        const auto& b = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(x, y, L - 1 - z))];
        const ftd::Vec3 dj = a.flux - b.flux * parity;
        const ftd::Vec3 dw = a.wave_vel - b.wave_vel * parity;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += a.flux.mag2() + a.wave_vel.mag2();
    }
    return std::sqrt(residual2 / std::max(1e-30, norm2));
}

double x_component_parity_error(const ftd::RenderBridge& rb,
                                const ftd::Vec3& signs) {
    const int L=rb.lattice().size();double residual2=0.0,norm2=0.0;
    for(int z=0;z<L;++z)for(int y=0;y<L;++y)for(int x=0;x<L;++x){
        const auto&a=rb.voxels()[static_cast<std::size_t>(rb.lattice().index(x,y,z))];
        const auto&b=rb.voxels()[static_cast<std::size_t>(rb.lattice().index(L-1-x,y,z))];
        const ftd::Vec3 sj(b.flux.x*signs.x,b.flux.y*signs.y,b.flux.z*signs.z);
        const ftd::Vec3 sw(b.wave_vel.x*signs.x,b.wave_vel.y*signs.y,b.wave_vel.z*signs.z);
        residual2+=(a.flux-sj).mag2()+(a.wave_vel-sw).mag2();
        norm2+=a.flux.mag2()+a.wave_vel.mag2();
    }
    return std::sqrt(residual2/std::max(1e-30,norm2));
}

int manifested_count(const ftd::RenderBridge& rb) {
    int count = 0;
    for (const auto& v : rb.voxels()) if (v.state != 0) ++count;
    return count;
}

bool only_terms_enabled(const ftd::RenderBridge& rb,
                        const std::vector<std::string>& expected) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const bool want = std::find(expected.begin(), expected.end(), spec.name)
                        != expected.end();
        if ((rb.toggles.*(spec.field)) != want) return false;
    }
    return true;
}

void tick_n(ftd::RenderBridge& rb, int n) {
    for (int i = 0; i < n; ++i) rb.tick();
}

struct ResearchSetupStats {
    int manifested = 0;
    int signed_state = 0;
    int locked = 0;
    int colored = 0;
    double field_norm = 0.0;
    double wave_norm = 0.0;
    double max_flux = 0.0;
    bool finite = true;
};

ResearchSetupStats research_setup_stats(const ftd::RenderBridge& rb) {
    ResearchSetupStats out;
    const auto finite_vec = [](const ftd::Vec3& v) {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    };
    for (const auto& v : rb.voxels()) {
        if (v.state != 0) {
            ++out.manifested;
            out.signed_state += v.state;
        }
        if (v.locked) ++out.locked;
        if (v.color != 0) ++out.colored;
        out.field_norm += v.flux.mag2();
        out.wave_norm += v.wave_vel.mag2();
        out.max_flux = std::max(out.max_flux, v.flux.mag());
        out.finite = out.finite && finite_vec(v.flux) && finite_vec(v.wave_vel)
            && finite_vec(v.velocity) && finite_vec(v.remainder)
            && std::isfinite(v.latency) && std::isfinite(v.tau)
            && std::isfinite(v.phase);
    }
    return out;
}

bool exact_research_setup_replay(const ftd::RenderBridge& a,
                                 const ftd::RenderBridge& b) {
    if (a.voxels().size() != b.voxels().size()) return false;
    const auto same_vec = [](const ftd::Vec3& x, const ftd::Vec3& y) {
        return x.x == y.x && x.y == y.y && x.z == y.z;
    };
    for (std::size_t i = 0; i < a.voxels().size(); ++i) {
        const auto& x = a.voxels()[i];
        const auto& y = b.voxels()[i];
        if (x.state != y.state || !same_vec(x.flux, y.flux)
            || !same_vec(x.wave_vel, y.wave_vel)
            || !same_vec(x.velocity, y.velocity)
            || !same_vec(x.remainder, y.remainder)
            || x.latency != y.latency || x.tau != y.tau || x.phase != y.phase
            || x.locked != y.locked || x.particle_id != y.particle_id
            || x.pair_id != y.pair_id || x.spin != y.spin
            || x.color != y.color || x.flavor != y.flavor) {
            return false;
        }
    }
    return true;
}

std::string enabled_term_list(const ftd::RenderBridge& rb) {
    std::string out;
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        if (!(rb.toggles.*(spec.field))) continue;
        if (!out.empty()) out += ',';
        out += spec.name;
    }
    return out.empty() ? "none" : out;
}

void test_remaining_research_setup_probe_matrix() {
    // This is a diagnostic floor, not the public admission gate.  Dispatch,
    // finiteness, and replay prove that a setup is mechanically usable; each
    // public scenario still needs a mechanism-specific assertion elsewhere.
    static const char* ids[] = {
        "s0-seed-ew-phase-transition",
        "s0-seed-up-quark", "s0-seed-down-quark",
        "s0-seed-strange-quark", "s0-seed-charm-quark",
        "s0-seed-bottom-quark", "s0-seed-top-quark",
        "s0-seed-higgs-field", "s0-seed-gluon",
        "s0-seed-beta-decay", "s0-seed-ee-annihilation",
        "s0-seed-quark-gluon-plasma",
        "s0-seed-hydrogen", "s0-seed-helium",
        "s0-seed-h2-bond-formation", "s0-seed-spark-of-life",
        "s0-seed-schwarzschild", "s0-seed-gravitational-lensing",
        "s0-seed-gravitational-wave", "s0-seed-time-gravity-well",
        "s0-seed-time-twin-clocks", "s0-seed-time-horizon",
        "s0-seed-cluster-law",
        "s0-vacuum-electron", "s0-vacuum-muon", "s0-vacuum-tau",
        "s0-vacuum-muon-neutrino", "s0-vacuum-tau-neutrino",
        "s0-vacuum-w-boson", "s0-vacuum-z-boson", "s0-vacuum-higgs",
        "s0-vacuum-proton", "s0-vacuum-neutron",
        "s0-vacuum-pion-charged", "s0-vacuum-pion-neutral",
        "s0-vacuum-kaon-charged", "s0-seed-de-broglie-clock",
    };

    constexpr int L = 24;
    constexpr int ticks = 8;
    for (const char* id : ids) {
        ftd::RenderBridge a(L), b(L);
        a.force_cpu();
        b.force_cpu();
        const bool da = ftd::dispatch_scenario(a, id);
        const bool db = ftd::dispatch_scenario(b, id);
        check(std::string(id) + " research setup dispatches twice", da && db);
        const auto initial = research_setup_stats(a);
        const std::string terms = enabled_term_list(a);
        tick_n(a, ticks);
        tick_n(b, ticks);
        const auto final = research_setup_stats(a);
        std::cout << "    PROBE " << id
                  << " terms=" << terms
                  << " N=" << initial.manifested << "->" << final.manifested
                  << " Q=" << initial.signed_state << "->" << final.signed_state
                  << " lock=" << initial.locked << "->" << final.locked
                  << " color=" << initial.colored << "->" << final.colored
                  << " |J|2=" << initial.field_norm << "->" << final.field_norm
                  << " |W|2=" << initial.wave_norm << "->" << final.wave_norm
                  << " maxJ=" << initial.max_flux << "->" << final.max_flux
                  << '\n';
        check(std::string(id) + " remains finite for eight native ticks",
              initial.finite && final.finite);
        check(std::string(id) + " replays bit-exactly for eight native ticks",
              exact_research_setup_replay(a, b));
    }
}

void test_unlocked_composite_candidate_outcomes() {
    struct Case {
        const char* id;
        int initial;
        int signed_state;
        int n8, n16, n32, n64;
    };
    const Case cases[] = {
        {"s0-vacuum-proton", 3, +1, 3, 1, 0, 0},
        {"s0-vacuum-neutron", 3, -1, 1, 1, 1, 0},
        {"s0-vacuum-pion-charged", 2, 0, 0, 0, 0, 0},
        {"s0-vacuum-pion-neutral", 2, 0, 0, 0, 0, 0},
        {"s0-vacuum-kaon-charged", 2, 0, 0, 0, 0, 0},
    };
    for (const auto& c : cases) {
        ftd::RenderBridge rb(24);
        rb.force_cpu();
        check(std::string(c.id) + " selected-color candidate dispatched",
              ftd::dispatch_scenario(rb, c.id));
        const auto initial = research_setup_stats(rb);
        check(std::string(c.id) + " isolates force, color force, and movement",
              only_terms_enabled(rb, {"forces", "movement", "color_forces"})
              && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
        check(std::string(c.id) + " has the declared unlocked initial cohort",
              initial.manifested == c.initial
              && initial.signed_state == c.signed_state
              && initial.locked == 0 && initial.colored == c.initial
              && initial.field_norm > 0.0 && initial.wave_norm == 0.0);
        const int expected[] = {c.n8, c.n16, c.n32, c.n64};
        int checkpoint = 0;
        for (int t = 1; t <= 64; ++t) {
            rb.tick();
            if (t == 8 || t == 16 || t == 32 || t == 64) {
                const auto stats = research_setup_stats(rb);
                std::cout << "    composite " << c.id << " t=" << t
                          << " N=" << stats.manifested
                          << " Q=" << stats.signed_state
                          << " color=" << stats.colored
                          << " |J|2=" << stats.field_norm
                          << " maxJ=" << stats.max_flux << '\n';
                check(std::string(c.id) + " remains finite through checkpoint "
                      + std::to_string(t), stats.finite);
                check(std::string(c.id) + " matches the selected finite-run survival count at "
                      + std::to_string(t), stats.manifested == expected[checkpoint]);
                ++checkpoint;
            }
        }

        ftd::RenderBridge replay(24);
        replay.force_cpu();
        ftd::dispatch_scenario(replay, c.id);
        tick_n(replay, 64);
        check(std::string(c.id) + " 64-tick history replays bit-exactly",
              exact_research_setup_replay(rb, replay));
        check(std::string(c.id) + " does not survive as a bound candidate",
              manifested_count(rb) == 0);
    }

    ftd::RenderBridge charged(24), neutral(24);
    charged.force_cpu();
    neutral.force_cpu();
    ftd::dispatch_scenario(charged, "s0-vacuum-pion-charged");
    ftd::dispatch_scenario(neutral, "s0-vacuum-pion-neutral");
    check("charged- and neutral-pion labels initialize bit-identical candidates",
          exact_research_setup_replay(charged, neutral));
    tick_n(charged, 16);
    tick_n(neutral, 16);
    check("charged- and neutral-pion labels remain bit-identical",
          exact_research_setup_replay(charged, neutral));
}

void test_long_baseline_opposite_polarity_collision() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L), replay(L);
    rb.force_cpu();
    replay.force_cpu();
    check("long-baseline opposite-polarity collision dispatched twice",
          ftd::dispatch_scenario(rb, "s0-seed-ee-annihilation")
          && ftd::dispatch_scenario(replay, "s0-seed-ee-annihilation"));
    check("long-baseline collision isolates native movement",
          only_terms_enabled(rb, {"movement"})
          && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    const auto initial = research_setup_stats(rb);
    check("long-baseline collision starts as a neutral unlocked pair",
          initial.manifested == 2 && initial.signed_state == 0
          && initial.locked == 0 && initial.field_norm > 0.0
          && initial.wave_norm == 0.0);
    int collision_tick = 0;
    for (int t = 1; t <= 64; ++t) {
        rb.tick();
        replay.tick();
        if (collision_tick == 0 && manifested_count(rb) == 0) collision_tick = t;
    }
    const auto final = research_setup_stats(rb);
    std::cout << "    long-baseline collision_tick=" << collision_tick
              << " |J|2=" << initial.field_norm << "->" << final.field_norm
              << " |W|2=" << final.wave_norm << '\n';
    check("long-baseline pair reaches the collision-removal branch",
          collision_tick == 24 && final.manifested == 0);
    check("long-baseline collision history replays bit-exactly",
          exact_research_setup_replay(rb, replay));
    check("movement-only collision creates no propagating wave momentum",
          final.wave_norm == 0.0 && final.field_norm > 0.0
          && final.field_norm < initial.field_norm);
}

void test_prepared_coulomb_candidate_outcomes() {
    struct Case {
        const char* id;
        int initial;
        int locked;
        int signed_state;
        int n8, n16, n32, n64;
    };
    const Case cases[] = {
        {"s0-seed-hydrogen", 4, 3, 0, 4, 4, 4, 4},
        {"s0-seed-helium", 14, 12, -2, 14, 14, 14, 14},
        {"s0-seed-h2-bond-formation", 8, 6, 0, 8, 8, 8, 6},
    };
    for (const auto& c : cases) {
        ftd::RenderBridge rb(24);
        rb.force_cpu();
        check(std::string(c.id) + " prepared Coulomb candidate dispatched",
              ftd::dispatch_scenario(rb, c.id));
        const auto initial = research_setup_stats(rb);
        check(std::string(c.id) + " isolates Poisson force and movement",
              only_terms_enabled(rb, {"forces", "poisson_coulomb", "movement"})
              && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
        check(std::string(c.id) + " has its declared locked/mobile cohort",
              initial.manifested == c.initial && initial.locked == c.locked
              && initial.signed_state == c.signed_state
              && initial.manifested - initial.locked == 2 - (c.initial == 4));
        const int expected[] = {c.n8, c.n16, c.n32, c.n64};
        int checkpoint = 0;
        for (int t = 1; t <= 64; ++t) {
            rb.tick();
            if (t == 8 || t == 16 || t == 32 || t == 64) {
                const auto stats = research_setup_stats(rb);
                std::cout << "    Coulomb candidate " << c.id << " t=" << t
                          << " N=" << stats.manifested
                          << " lock=" << stats.locked
                          << " mobile=" << stats.manifested - stats.locked
                          << " Q=" << stats.signed_state
                          << " |J|2=" << stats.field_norm << '\n';
                if (t == 64) {
                    for (int i = 0; i < static_cast<int>(rb.voxels().size()); ++i) {
                        const auto& v = rb.voxels()[static_cast<std::size_t>(i)];
                        if (v.state == 0 || v.locked) continue;
                        const auto p = rb.lattice().coord(i);
                        std::cout << "      mobile id=" << v.particle_id
                                  << " x=" << p.x + v.remainder.x
                                  << " y=" << p.y + v.remainder.y
                                  << " z=" << p.z + v.remainder.z
                                  << " |v|=" << v.velocity.mag() << '\n';
                    }
                }
                check(std::string(c.id) + " remains finite through Coulomb checkpoint "
                      + std::to_string(t), stats.finite && stats.locked == c.locked);
                check(std::string(c.id) + " matches its finite-run survival count at "
                      + std::to_string(t), stats.manifested == expected[checkpoint]);
                ++checkpoint;
            }
        }
        ftd::RenderBridge replay(24);
        replay.force_cpu();
        ftd::dispatch_scenario(replay, c.id);
        tick_n(replay, 64);
        check(std::string(c.id) + " prepared 64-tick history replays bit-exactly",
              exact_research_setup_replay(rb, replay));
    }
}

void test_uniform_additive_genesis_drive_response() {
    constexpr int L = 16;
    ftd::RenderBridge rb(L), replay(L);
    rb.force_cpu();
    replay.force_cpu();
    check("uniform additive genesis drive dispatched twice",
          ftd::dispatch_scenario(rb, "s0-seed-ew-phase-transition")
          && ftd::dispatch_scenario(replay, "s0-seed-ew-phase-transition"));
    check("uniform drive isolates wave, Gauss, genesis, and drive",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection",
                                  "genesis", "ew_background_sweep"})
          && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic
          && manifested_count(rb) == 0);
    rb.tick();
    replay.tick();
    bool first_step_uniform = manifested_count(rb) == 0;
    double first_max_error = 0.0;
    double first_max_wave = 0.0;
    for (const auto& v : rb.voxels()) {
        first_max_error = std::max(first_max_error, std::fabs(v.flux.x - 0.025));
        first_max_wave = std::max(first_max_wave, v.wave_vel.mag());
        first_step_uniform = first_step_uniform
            && std::fabs(v.flux.y) < 1e-14 && std::fabs(v.flux.z) < 1e-14;
    }
    std::cout << "    uniform drive first-step max_error=" << first_max_error
              << " max_wave=" << first_max_wave << '\n';
    check("first drive step is uniform D(0)=0.025 to projection precision",
          first_step_uniform && first_max_error < 1e-14 && first_max_wave < 1e-14);
    const int checkpoints[] = {16, 24, 32, 64};
    const int expected_n[] = {0, 0, 0, 2068};
    const int expected_q[] = {0, 0, 0, -36};
    int checkpoint = 0;
    for (int t = 2; t <= 64; ++t) {
        rb.tick();
        replay.tick();
        if (t == 16 || t == 24 || t == 32 || t == 64) {
            const auto stats = research_setup_stats(rb);
            std::cout << "    uniform drive t=" << t
                      << " N=" << stats.manifested
                      << " Q=" << stats.signed_state
                      << " |J|2=" << stats.field_norm
                      << " maxJ=" << stats.max_flux << '\n';
            check("uniform-drive response remains finite at " + std::to_string(t),
                  stats.finite);
            check("uniform-drive response matches the selected finite-run cohort at "
                  + std::to_string(checkpoints[checkpoint]),
                  stats.manifested == expected_n[checkpoint]
                  && stats.signed_state == expected_q[checkpoint]);
            ++checkpoint;
        }
    }
    check("uniform-drive 64-tick response replays bit-exactly",
          exact_research_setup_replay(rb, replay));
    bool nonnegative_drive = true;
    for (int t = 0; t < 1000; ++t) {
        const double d = (std::sin(0.01 * t) + 1.0) * 0.025;
        nonnegative_drive = nonnegative_drive && d >= 0.0 && d <= 0.05;
    }
    check("declared drive never executes a negative down-sweep",
          nonnegative_drive);
}

void test_prepared_weak_transmutation_cohort() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L), replay(L);
    rb.force_cpu();
    replay.force_cpu();
    check("prepared weak-transmutation cohort dispatched twice",
          ftd::dispatch_scenario(rb, "s0-seed-beta-decay")
          && ftd::dispatch_scenario(replay, "s0-seed-beta-decay"));
    check("prepared weak cohort isolates dual substrate and weak flip",
          only_terms_enabled(rb, {"dual_substrate", "weak_transmutation"}));
    const auto initial = research_setup_stats(rb);
    check("alleged beta-decay products are already present at tick zero",
          initial.manifested == 4 && initial.signed_state == -2
          && initial.field_norm > 0.0 && initial.wave_norm > 0.0);
    check("weak event journal enables", rb.enable_history_journal());
    int weak_events = 0;
    int first_event_tick = 0;
    for (int t = 1; t <= 64; ++t) {
        rb.tick();
        replay.tick();
        for (const auto& event : rb.history_events()) {
            if (event.kind == ftd::eft::HistoryEventKind::WeakTransmutation) {
                ++weak_events;
                if (first_event_tick == 0) first_event_tick = t;
            }
        }
    }
    const auto final = research_setup_stats(rb);
    std::cout << "    prepared weak cohort events=" << weak_events
              << " first=" << first_event_tick
              << " N=" << initial.manifested << "->" << final.manifested
              << " Q=" << initial.signed_state << "->" << final.signed_state
              << " |J|2=" << initial.field_norm << "->" << final.field_norm
              << '\n';
    check("prepared weak cohort remains finite", initial.finite && final.finite);
    check("prepared weak cohort matches the selected stress-ramp response",
          weak_events == 7 && first_event_tick == 54
          && final.manifested == 4 && final.signed_state == 0
          && final.field_norm > initial.field_norm);
    check("prepared weak cohort state evolution replays exactly",
          exact_research_setup_replay(rb, replay));
}

void test_fixed_seed_thermal_transport_cohort() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L), replay(L);
    rb.force_cpu();
    replay.force_cpu();
    check("fixed-seed thermal transport cohort dispatched twice",
          ftd::dispatch_scenario(rb, "s0-seed-quark-gluon-plasma")
          && ftd::dispatch_scenario(replay, "s0-seed-quark-gluon-plasma"));
    check("thermal transport isolates wave, Gauss, movement, and Langevin",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection",
                                  "movement", "langevin"})
          && rb.toggles.langevin_T == 0.02
          && rb.toggles.langevin_gamma == 0.05
          && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    const auto initial = research_setup_stats(rb);
    int exact_speed = 0;
    for (const auto& v : rb.voxels()) {
        if (v.state != 0
            && std::fabs(v.speed() - 0.5 * ftd::C_SPEED) < 1e-14) {
            ++exact_speed;
        }
    }
    check("thermal transport starts as eight neutral colored moving markers",
          initial.manifested == 8 && initial.signed_state == 0
          && initial.colored == 8 && initial.locked == 0
          && exact_speed == 8 && initial.field_norm > 0.0
          && initial.wave_norm > 0.0);
    check("thermal transport event journal enables", rb.enable_history_journal());
    const int expected_n[] = {8, 8, 8, 1};
    const int expected_q[] = {0, 0, 0, -1};
    int checkpoint = 0;
    int movement_events = 0;
    int annihilation_events = 0;
    for (int t = 1; t <= 64; ++t) {
        rb.tick();
        replay.tick();
        for (const auto& event : rb.history_events()) {
            if (event.kind == ftd::eft::HistoryEventKind::Movement) ++movement_events;
            if (event.kind == ftd::eft::HistoryEventKind::Annihilation) ++annihilation_events;
        }
        if (t == 8 || t == 16 || t == 32 || t == 64) {
            const auto stats = research_setup_stats(rb);
            std::cout << "    thermal transport t=" << t
                      << " N=" << stats.manifested
                      << " Q=" << stats.signed_state
                      << " color=" << stats.colored
                      << " |J|2=" << stats.field_norm
                      << " |W|2=" << stats.wave_norm << '\n';
            check("thermal transport remains finite at " + std::to_string(t),
                  stats.finite);
            check("thermal transport matches selected marker count at "
                  + std::to_string(t), stats.manifested == expected_n[checkpoint]
                  && stats.signed_state == expected_q[checkpoint]);
            ++checkpoint;
        }
    }
    std::cout << "    thermal transport movement_events=" << movement_events
              << " annihilation_events=" << annihilation_events << '\n';
    check("thermal transport depletion is open-boundary motion, not annihilation",
          movement_events == 145 && annihilation_events == 0);
    check("thermal transport 64-tick history replays bit-exactly",
          exact_research_setup_replay(rb, replay));
}

void test_patterned_genesis_response_cohort() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L), replay(L);
    rb.force_cpu();
    replay.force_cpu();
    check("patterned genesis-response cohort dispatched twice",
          ftd::dispatch_scenario(rb, "s0-seed-spark-of-life")
          && ftd::dispatch_scenario(replay, "s0-seed-spark-of-life"));
    check("patterned response uses only its declared production stack",
          only_terms_enabled(rb, {"wave_propagation", "coupling", "damping",
                                  "genesis", "gauss_projection", "forces",
                                  "movement"})
          && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    const auto initial = research_setup_stats(rb);
    check("patterned response starts as the declared 16+8+3 cohort",
          initial.manifested == 27 && initial.locked == 16
          && initial.colored == 11 && initial.signed_state == 1
          && initial.field_norm > 0.0);
    check("patterned response event journal enables", rb.enable_history_journal());
    int genesis_events = 0;
    int evaporation_events = 0;
    int movement_events = 0;
    int annihilation_events = 0;
    const int expected_n[] = {27, 33, 33, 33};
    const int expected_q[] = {1, 7, 7, 7};
    int checkpoint = 0;
    for (int t = 1; t <= 32; ++t) {
        rb.tick();
        replay.tick();
        for (const auto& event : rb.history_events()) {
            if (event.kind == ftd::eft::HistoryEventKind::Genesis) ++genesis_events;
            if (event.kind == ftd::eft::HistoryEventKind::Evaporation) ++evaporation_events;
            if (event.kind == ftd::eft::HistoryEventKind::Movement) ++movement_events;
            if (event.kind == ftd::eft::HistoryEventKind::Annihilation) ++annihilation_events;
        }
        if (t == 1 || t == 8 || t == 16 || t == 32) {
            const auto stats = research_setup_stats(rb);
            std::cout << "    patterned response t=" << t
                      << " N=" << stats.manifested
                      << " Q=" << stats.signed_state
                      << " lock=" << stats.locked
                      << " |J|2=" << stats.field_norm
                      << " maxJ=" << stats.max_flux << '\n';
            check("patterned response remains finite at " + std::to_string(t),
                  stats.finite && stats.locked == 16);
            check("patterned response matches the selected finite-run cohort at "
                  + std::to_string(t), stats.manifested == expected_n[checkpoint]
                  && stats.signed_state == expected_q[checkpoint]);
            ++checkpoint;
        }
    }
    std::cout << "    patterned events genesis=" << genesis_events
              << " evaporation=" << evaporation_events
              << " movement=" << movement_events
              << " annihilation=" << annihilation_events << '\n';
    check("patterned response produces one finite genesis burst without turnover",
          genesis_events == 6 && evaporation_events == 0
          && movement_events == 18 && annihilation_events == 0);
    check("patterned response 32-tick history replays bit-exactly",
          exact_research_setup_replay(rb, replay));
}

void test_empty_baseline_stays_empty() {
    ftd::RenderBridge rb(24);
    rb.force_cpu();
    check("empty baseline dispatched", ftd::dispatch_scenario(rb, "empty"));

    const auto exactly_empty = [&]() {
        for (const auto& v : rb.voxels()) {
            if (v.state != 0 || v.flux.mag2() != 0.0
                || v.wave_vel.mag2() != 0.0 || v.velocity.mag2() != 0.0) {
                return false;
            }
        }
        return true;
    };
    check("empty baseline starts exactly empty", exactly_empty());
    tick_n(rb, 16);
    check("empty baseline remains exactly empty across ticks", exactly_empty());
}

void test_vacuum_photon_translates() {
    constexpr int L = 33;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("vacuum photon dispatched", ftd::dispatch_scenario(rb, "s0-vacuum-photon"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    rb.toggles.strict_validation = true;

    const auto initial = x_profile(rb);
    check("vacuum photon has field energy", initial.weight > 0.0);
    check("vacuum photon is exactly transverse", normalized_divergence(rb) < 1e-12);
    check("vacuum photon starts without manifested sites", manifested_count(rb) == 0);
    check("vacuum photon uses isolated wave sector",
          rb.toggles.wave_propagation && !rb.toggles.coupling
          && !rb.toggles.damping && !rb.toggles.genesis
          && !rb.toggles.forces && !rb.toggles.movement);

    tick_n(rb, 20);
    const auto final = x_profile(rb);
    const double dx = forward_delta(final.centroid, initial.centroid, L);
    const double speed = dx / 20.0;
    std::cout << "    photon dx=" << dx << " speed=" << speed
              << " width_ratio=" << final.width / initial.width << '\n';
    check("vacuum photon translates in +x", dx > 9.0);
    check("vacuum photon speed resolves C_SPEED",
          std::fabs(speed - ftd::C_SPEED) < 0.08);
    check("vacuum photon remains a coherent pulse", final.width < 1.25 * initial.width);
    check("vacuum photon remains unmanifested", manifested_count(rb) == 0);
}

void test_photon_race_common_speed() {
    constexpr int L = 48;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("photon race dispatched", ftd::dispatch_scenario(rb, "light-photon-race"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    rb.toggles.strict_validation = true;

    const auto low0 = x_profile(rb, {}, 1);
    const auto high0 = x_profile(rb, {}, 2);
    tick_n(rb, 16);
    const auto low1 = x_profile(rb, {}, 1);
    const auto high1 = x_profile(rb, {}, 2);
    const double dx_low = forward_delta(low1.centroid, low0.centroid, L);
    const double dx_high = forward_delta(high1.centroid, high0.centroid, L);
    std::cout << "    race dx_low=" << dx_low << " dx_high=" << dx_high << '\n';
    // These sheets include transverse momenta, so their x-centroid speed is
    // below the plane-wave cone. The load-bearing race claim is equality
    // across a tenfold amplitude change, not equality to C_SPEED.
    check("both race packets translate in +x", dx_low > 4.5 && dx_high > 4.5);
    check("tenfold amplitude change leaves limiting speed unchanged",
          std::fabs(dx_low - dx_high) < 1e-6);
    check("photon race remains unmanifested", manifested_count(rb) == 0);
}

void test_rainbow_modes_are_transverse() {
    constexpr int L = 48;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("rainbow harmonics dispatched", ftd::dispatch_scenario(rb, "light-rainbow"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    rb.toggles.strict_validation = true;

    double longitudinal_energy = 0.0;
    double total_energy = 0.0;
    for (const auto& v : rb.voxels()) {
        longitudinal_energy += v.flux.x * v.flux.x
                             + v.wave_vel.x * v.wave_vel.x;
        total_energy += v.flux.mag2() + v.wave_vel.mag2();
    }
    check("rainbow has nonzero field energy", total_energy > 0.0);
    check("all rainbow modes are exactly transverse", longitudinal_energy < 1e-24);
    check("rainbow initial divergence is machine zero", normalized_divergence(rb) < 1e-12);
    tick_n(rb, 8);
    check("rainbow remains unmanifested", manifested_count(rb) == 0);
}

void test_exact_traveling_harmonic() {
    constexpr int L = 48;
    constexpr int mode_n = 4;
    constexpr int ticks = 11;
    const double amp = 2.0 * ftd::K_B;
    const double k = 2.0 * ftd::PI * mode_n / L;
    const double omega = 2.0 * std::asin(ftd::C_SPEED * std::sin(0.5 * k));

    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("traveling harmonic dispatched",
          ftd::dispatch_scenario(rb, "s0-field-plane-wave"));
    rb.toggles.strict_validation = true;

    const auto initial = project_jz_harmonic(rb, mode_n);
    check("traveling harmonic starts as the selected pure Fourier mode",
          std::fabs(initial.sine - amp) < 1e-11
          && std::fabs(initial.cosine) < 1e-11);
    check("traveling harmonic uses the isolated unprojected wave map",
          rb.toggles.wave_propagation && !rb.toggles.gauss_projection
          && !rb.toggles.coupling && !rb.toggles.damping
          && !rb.toggles.genesis && !rb.toggles.dual_substrate);
    check("traveling harmonic is exactly transverse",
          normalized_divergence(rb) < 1e-12);

    tick_n(rb, ticks);
    const auto measured = project_jz_harmonic(rb, mode_n);
    const double expected_sine = amp * std::cos(omega * ticks);
    const double expected_cosine = -amp * std::sin(omega * ticks);
    const double sine_error = std::fabs(measured.sine - expected_sine) / amp;
    const double cosine_error = std::fabs(measured.cosine - expected_cosine) / amp;
    const double amplitude_error = std::fabs(
        std::hypot(measured.sine, measured.cosine) - amp) / amp;
    std::cout << "    traveling omega=" << omega
              << " sine_error=" << sine_error
              << " cosine_error=" << cosine_error
              << " amplitude_error=" << amplitude_error << '\n';
    check("traveling harmonic follows the exact lattice phase",
          sine_error < 1e-10 && cosine_error < 1e-10);
    check("traveling harmonic preserves its Fourier amplitude",
          amplitude_error < 1e-10);
    check("traveling harmonic remains transverse and unmanifested",
          normalized_divergence(rb) < 1e-12 && manifested_count(rb) == 0);
}

void test_gravity_named_wave_aliases_are_plain_native_modes() {
    constexpr int L = 48;
    constexpr int mode_n = 4;
    constexpr int ticks = 11;
    constexpr double amp = 0.1;
    const double k = 2.0 * ftd::PI * mode_n / L;
    const double omega = 2.0 * std::asin(ftd::C_SPEED * std::sin(0.5 * k));
    ftd::RenderBridge wave(L), well_alias(L), twin_alias(L);
    wave.force_cpu(); well_alias.force_cpu(); twin_alias.force_cpu();
    check("three gravity/time-named wave profiles dispatch",
          ftd::dispatch_scenario(wave, "s0-seed-gravitational-wave")
          && ftd::dispatch_scenario(well_alias, "s0-seed-time-gravity-well")
          && ftd::dispatch_scenario(twin_alias, "s0-seed-time-twin-clocks"));
    check("gravity/time-named profiles isolate only the native wave operator",
          only_terms_enabled(wave, {"wave_propagation"})
          && only_terms_enabled(well_alias, {"wave_propagation"})
          && only_terms_enabled(twin_alias, {"wave_propagation"}));
    check("time-well and twin-clock entries are exact aliases, not new experiments",
          exact_research_setup_replay(wave, well_alias)
          && exact_research_setup_replay(wave, twin_alias));
    const auto initial = project_jz_harmonic(wave, mode_n);
    const double h0 = periodic_modified_hamiltonian(wave);
    check("gravity-named seed is the exact selected transverse harmonic",
          std::fabs(initial.sine - amp) < 1e-12
          && std::fabs(initial.cosine) < 1e-12
          && normalized_divergence(wave) < 1e-12
          && manifested_count(wave) == 0);

    tick_n(wave, ticks); tick_n(well_alias, ticks); tick_n(twin_alias, ticks);
    const auto measured = project_jz_harmonic(wave, mode_n);
    const double phase_error = std::hypot(
        measured.sine - amp * std::cos(omega * ticks),
        measured.cosine + amp * std::sin(omega * ticks)) / amp;
    const double h1 = periodic_modified_hamiltonian(wave);
    const double h_drift = std::fabs(h1 - h0) / std::max(1e-30, std::fabs(h0));
    std::cout << "    gravity-named plain-wave phase_error=" << phase_error
              << " modified-H drift=" << h_drift << '\n';
    check("plain-wave alias follows the exact native lattice pole",
          phase_error < 1e-10 && h_drift < 1e-12);
    check("all three aliases remain bit-identical after evolution",
          exact_research_setup_replay(wave, well_alias)
          && exact_research_setup_replay(wave, twin_alias));
    check("gravity/time mechanisms are absent from the qualified profiles",
          manifested_count(wave) == 0 && manifested_count(well_alias) == 0
          && manifested_count(twin_alias) == 0);
}

void test_gravity_named_radial_ansatz_and_optical_null() {
    constexpr double amplitude = ftd::G_N * ftd::K_B * 3.0;
    {
        constexpr int L = 25;
        constexpr int c = L / 2;
        ftd::RenderBridge radial(L), horizon_alias(L);
        radial.force_cpu(); horizon_alias.force_cpu();
        check("Schwarzschild/horizon-named radial profiles dispatch",
              ftd::dispatch_scenario(radial, "s0-seed-schwarzschild")
              && ftd::dispatch_scenario(horizon_alias, "s0-seed-time-horizon"));
        check("radial and horizon profiles are exact inert aliases",
              only_terms_enabled(radial, {})
              && only_terms_enabled(horizon_alias, {})
              && exact_research_setup_replay(radial, horizon_alias));
        int states = 0;
        double max_error = 0.0;
        for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
            const auto& v = radial.voxel_at(x, y, z);
            if (v.state != 0) ++states;
            const double dx = x - c, dy = y - c, dz = z - c;
            const double r2 = dx*dx + dy*dy + dz*dz;
            ftd::Vec3 expected;
            if (r2 > 0.0 && amplitude / r2 >= 1e-6) {
                const double r = std::sqrt(r2);
                expected = {-amplitude * dx / (r2 * r),
                            -amplitude * dy / (r2 * r),
                            -amplitude * dz / (r2 * r)};
            }
            max_error = std::max(max_error, (v.flux - expected).mag());
        }
        std::cout << "    inward-radial ansatz max_error=" << max_error << '\n';
        check("radial profile is exactly the declared inward inverse-square ansatz",
              states == 1 && radial.voxel_at(c, c, c).state == +1
              && max_error < 1e-15);
        tick_n(radial, 8); tick_n(horizon_alias, 8);
        check("inert radial aliases remain bit-identical without gravity dynamics",
              exact_research_setup_replay(radial, horizon_alias));
    }

    {
        constexpr int L = 48;
        constexpr int ticks = 16;
        ftd::RenderBridge full(L), radial(L), packet(L);
        full.force_cpu(); radial.force_cpu(); packet.force_cpu();
        check("gravity-optics null profile dispatches",
              ftd::dispatch_scenario(full, "s0-seed-gravitational-lensing")
              && ftd::dispatch_scenario(radial, "s0-seed-schwarzschild"));
        radial.toggles = full.toggles;
        packet.toggles = full.toggles;
        for (std::size_t i = 0; i < full.voxels().size(); ++i) {
            packet.voxels()[i].flux = full.voxels()[i].flux - radial.voxels()[i].flux;
            packet.voxels()[i].wave_vel = full.voxels()[i].wave_vel
                                        - radial.voxels()[i].wave_vel;
        }
        const auto y_centroid = [](const ftd::RenderBridge& rb) {
            double weighted = 0.0, norm = 0.0;
            const int n = rb.lattice().size();
            for (int z = 0; z < n; ++z)
            for (int y = 0; y < n; ++y)
            for (int x = 0; x < n; ++x) {
                const auto& v = rb.voxels()[static_cast<std::size_t>(
                    rb.lattice().index(x, y, z))];
                const double e = v.flux.mag2() + v.wave_vel.mag2();
                weighted += y * e;
                norm += e;
            }
            return weighted / std::max(1e-30, norm);
        };
        const auto decomposition_error = [](const ftd::RenderBridge& sum,
                                            const ftd::RenderBridge& a,
                                            const ftd::RenderBridge& b) {
            double residual2 = 0.0, norm2 = 0.0;
            for (std::size_t i = 0; i < sum.voxels().size(); ++i) {
                const auto& s = sum.voxels()[i];
                residual2 += (s.flux - a.voxels()[i].flux - b.voxels()[i].flux).mag2()
                           + (s.wave_vel - a.voxels()[i].wave_vel
                                              - b.voxels()[i].wave_vel).mag2();
                norm2 += s.flux.mag2() + s.wave_vel.mag2();
            }
            return std::sqrt(residual2 / std::max(1e-30, norm2));
        };
        check("optical null isolates the unprojected linear wave operator",
              only_terms_enabled(full, {"wave_propagation"})
              && decomposition_error(full, radial, packet) < 1e-15);
        const double y0 = y_centroid(packet);
        tick_n(full, ticks); tick_n(radial, ticks); tick_n(packet, ticks);
        const double residual = decomposition_error(full, radial, packet);
        const double y1 = y_centroid(packet);
        std::cout << "    gravity-optics residual=" << residual
                  << " packet_y=" << y0 << "->" << y1 << '\n';
        check("radial background and packet evolve by exact linear superposition",
              residual < 1e-12);
        check("radial background induces no packet deflection or interaction",
              residual < 1e-12 && std::isfinite(y1 - y0)
              && manifested_count(packet) == 0);
    }
}

void test_exact_standing_harmonic() {
    constexpr int L = 48;
    constexpr int mode_n = 4;
    constexpr int ticks = 11;
    const double amp = 2.0 * ftd::K_B;
    const double k = 2.0 * ftd::PI * mode_n / L;
    const double omega = 2.0 * std::asin(ftd::C_SPEED * std::sin(0.5 * k));

    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("standing harmonic dispatched",
          ftd::dispatch_scenario(rb, "s0-field-standing-wave"));
    rb.toggles.strict_validation = true;

    const auto initial = project_jz_harmonic(rb, mode_n);
    check("standing harmonic starts as the selected pure Fourier mode",
          std::fabs(initial.sine - amp) < 1e-11
          && std::fabs(initial.cosine) < 1e-11);
    check("standing harmonic uses the isolated unprojected wave map",
          rb.toggles.wave_propagation && !rb.toggles.gauss_projection
          && !rb.toggles.coupling && !rb.toggles.damping
          && !rb.toggles.genesis && !rb.toggles.dual_substrate);

    tick_n(rb, ticks);
    const auto measured = project_jz_harmonic(rb, mode_n);
    const double expected_sine = amp * std::cos(omega * ticks);
    const double temporal_error = std::fabs(measured.sine - expected_sine) / amp;
    const double traveling_leakage = std::fabs(measured.cosine) / amp;
    double node_error = 0.0;
    for (int x = 0; x < L; x += L / (2 * mode_n))
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        node_error = std::max(node_error, std::fabs(rb.voxels()[
            static_cast<std::size_t>(rb.lattice().index(x, y, z))].flux.z));
    }
    std::cout << "    standing omega=" << omega
              << " temporal_error=" << temporal_error
              << " traveling_leakage=" << traveling_leakage
              << " node_error=" << node_error << '\n';
    check("standing harmonic follows the exact lattice oscillation",
          temporal_error < 1e-10);
    check("standing harmonic keeps fixed nodes without a traveling branch",
          traveling_leakage < 1e-10 && node_error < 1e-10);
    check("standing harmonic remains transverse and unmanifested",
          normalized_divergence(rb) < 1e-12 && manifested_count(rb) == 0);
}

void test_two_coherent_source_qualification_gate() {
    // Predeclared observation geometry: the sources start at x=L/4 and the
    // screen is x=L/2 after 20 ticks, matching the C_SPEED flight time to
    // within one voxel.  Admission requires both signs of the interference
    // cross-term to exceed 5% of the incoherent peak; no scan over x or t.
    constexpr int L = 48;
    constexpr int ticks = 20;
    constexpr int screen_x = L / 2;
    constexpr int mid = L / 2;

    ftd::RenderBridge pair(L);
    ftd::RenderBridge lower(L);
    ftd::RenderBridge upper(L);
    pair.force_cpu();
    lower.force_cpu();
    upper.force_cpu();
    check("two-source field dispatched",
          ftd::dispatch_scenario(pair, "light-two-slit"));

    // The two production sources have disjoint support except on the symmetry
    // plane y=mid, where equal source contributions add.  This exact split
    // reconstructs the two independent initial solutions without calling the
    // scenario helper from the test.
    lower.toggles = pair.toggles;
    upper.toggles = pair.toggles;
    pair.toggles.strict_validation = true;
    lower.toggles.strict_validation = true;
    upper.toggles.strict_validation = true;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const auto& src = pair.voxels()[static_cast<std::size_t>(
            pair.lattice().index(x, y, z))];
        const double lower_share = y < mid ? 1.0 : (y == mid ? 0.5 : 0.0);
        const double upper_share = 1.0 - lower_share;
        if (lower_share > 0.0) {
            lower.inject_flux_add(x, y, z, src.flux * lower_share);
            lower.inject_wave_vel_add(x, y, z, src.wave_vel * lower_share);
        }
        if (upper_share > 0.0) {
            upper.inject_flux_add(x, y, z, src.flux * upper_share);
            upper.inject_wave_vel_add(x, y, z, src.wave_vel * upper_share);
        }
    }

    tick_n(pair, ticks);
    tick_n(lower, ticks);
    tick_n(upper, ticks);

    double residual2 = 0.0;
    double pair_norm2 = 0.0;
    for (std::size_t i = 0; i < pair.lattice().total_sites(); ++i) {
        const ftd::Vec3 dj = pair.voxels()[i].flux
                           - lower.voxels()[i].flux - upper.voxels()[i].flux;
        const ftd::Vec3 dw = pair.voxels()[i].wave_vel
                           - lower.voxels()[i].wave_vel - upper.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        pair_norm2 += pair.voxels()[i].flux.mag2()
                    + pair.voxels()[i].wave_vel.mag2();
    }
    const double superposition_residual = std::sqrt(
        residual2 / std::max(1e-30, pair_norm2));

    double cross_max = 0.0;
    double cross_min = 0.0;
    double incoherent_peak = 0.0;
    for (int y = 0; y < L; ++y) {
        double cross = 0.0;
        double incoherent = 0.0;
        for (int z = 0; z < L; ++z) {
            const std::size_t i = static_cast<std::size_t>(
                pair.lattice().index(screen_x, y, z));
            cross += 2.0 * lower.voxels()[i].flux.z * upper.voxels()[i].flux.z;
            incoherent += lower.voxels()[i].flux.z * lower.voxels()[i].flux.z
                        + upper.voxels()[i].flux.z * upper.voxels()[i].flux.z;
        }
        cross_max = std::max(cross_max, cross);
        cross_min = std::min(cross_min, cross);
        incoherent_peak = std::max(incoherent_peak, incoherent);
    }
    const double constructive_fraction = cross_max / std::max(1e-30, incoherent_peak);
    const double destructive_fraction = -cross_min / std::max(1e-30, incoherent_peak);
    std::cout << "    two-source superposition_residual=" << superposition_residual
              << " constructive_fraction=" << constructive_fraction
              << " destructive_fraction=" << destructive_fraction << '\n';
    check("two-source evolution obeys pointwise linear superposition",
          superposition_residual < 1e-12);
    check("two-source carrier produces both cross-term signs",
          constructive_fraction > 0.0 && destructive_fraction > 0.0);
    check("two-source candidate remains below the preregistered bidirectional contrast gate",
          constructive_fraction <= 0.05 || destructive_fraction <= 0.05);
    check("two-source field remains transverse and unmanifested",
          normalized_divergence(pair) < 1e-12 && manifested_count(pair) == 0);
}

void test_neutral_candidate_is_dynamic() {
    constexpr int L = 48;
    ftd::RenderBridge base(L), medium(L), high(L);
    base.force_cpu(); medium.force_cpu(); high.force_cpu();
    check("three amplitude-coded neutral candidates dispatch",
          ftd::dispatch_scenario(base, "s0-vacuum-electron-neutrino")
          && ftd::dispatch_scenario(medium, "s0-vacuum-muon-neutrino")
          && ftd::dispatch_scenario(high, "s0-vacuum-tau-neutrino"));
    for (auto* rb : {&base, &medium, &high}) {
        rb->toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    }

    const auto scaled_residual = [](const ftd::RenderBridge& a,
                                    const ftd::RenderBridge& b,
                                    double scale) {
        double residual2 = 0.0;
        double norm2 = 0.0;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            residual2 += (y.flux - x.flux * scale).mag2()
                       + (y.wave_vel - x.wave_vel * scale).mag2();
            norm2 += y.flux.mag2() + y.wave_vel.mag2();
        }
        return std::sqrt(residual2 / std::max(1e-30, norm2));
    };

    const Profile initial[3] = {
        x_profile(base), x_profile(medium), x_profile(high),
    };
    const double initial_divergence = std::max({normalized_divergence(base),
        normalized_divergence(medium), normalized_divergence(high)});
    check("all neutral candidate packets are divergence-free",
          initial_divergence < 1e-12);
    check("medium and high packets are only imposed 1.3x/1.6x amplitude copies",
          scaled_residual(base, medium, 1.3) < 1e-12
          && scaled_residual(base, high, 1.6) < 1e-12);

    tick_n(base, 12); tick_n(medium, 12); tick_n(high, 12);
    const Profile final[3] = {
        x_profile(base), x_profile(medium), x_profile(high),
    };
    const double dx[3] = {
        forward_delta(final[0].centroid, initial[0].centroid, L),
        forward_delta(final[1].centroid, initial[1].centroid, L),
        forward_delta(final[2].centroid, initial[2].centroid, L),
    };
    std::cout << "    neutral-candidate dx base/1.3x/1.6x="
              << dx[0] << '/' << dx[1] << '/' << dx[2] << '\n';
    check("all neutral candidates translate rather than remaining static icons",
          dx[0] > 4.0 && dx[1] > 4.0 && dx[2] > 4.0);
    check("amplitude coding creates no flavor-dependent propagation",
          std::fabs(dx[0] - dx[1]) < 1e-12
          && std::fabs(dx[0] - dx[2]) < 1e-12
          && scaled_residual(base, medium, 1.3) < 1e-12
          && scaled_residual(base, high, 1.6) < 1e-12);
    check("all neutral candidates remain unmanifested",
          manifested_count(base) == 0 && manifested_count(medium) == 0
          && manifested_count(high) == 0);
}

void test_imposed_kg_block_clock_integration() {
    constexpr int L = 24;
    constexpr int c = L / 2;
    constexpr double j0 = 0.08;
    constexpr double omega0 = 0.30;
    ftd::RenderBridge clock(L), control(L), replay(L);
    clock.force_cpu(); control.force_cpu(); replay.force_cpu();
    check("imposed KG block clock dispatches reproducibly",
          ftd::dispatch_scenario(clock, "s0-seed-de-broglie-clock")
          && ftd::dispatch_scenario(control, "s0-seed-de-broglie-clock")
          && ftd::dispatch_scenario(replay, "s0-seed-de-broglie-clock"));
    for (auto* rb : {&clock, &control, &replay}) {
        rb->toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    }
    check("KG block isolates only the wave and imposed clock operators",
          only_terms_enabled(clock, {"wave_propagation", "de_broglie_clock"})
          && clock.toggles.omega0 == omega0);
    check("KG block starts as exactly 7 cubed manifested sites",
          manifested_count(clock) == 343
          && clock.voxel_at(c, c, c).flux.x == j0);

    control.toggles.de_broglie_clock = false;
    clock.tick(); control.tick(); replay.tick();
    const auto& on = clock.voxel_at(c, c, c);
    const auto& off = control.voxel_at(c, c, c);
    const double expected_w = -omega0 * omega0 * j0;
    std::cout << "    KG-block center W_on/W_off="
              << on.wave_vel.x << '/' << off.wave_vel.x << '\n';
    check("clock term supplies the exact first-tick center acceleration",
          std::fabs(on.wave_vel.x - expected_w) < 1e-15
          && std::fabs(on.flux.x - (j0 + expected_w)) < 1e-15);
    check("clock-off control has no center oscillation kick",
          std::fabs(off.wave_vel.x) < 1e-15
          && std::fabs(off.flux.x - j0) < 1e-15);
    check("actual scenario first tick replays bit-exactly",
          exact_research_setup_replay(clock, replay));
    check("the imposed clock does not create or remove block sites on tick one",
          manifested_count(clock) == 343);
}

void test_particle_named_wave_template_cohorts() {
    {
        struct Case { const char* id; int state; int color; double boost; };
        const Case cases[] = {
            {"s0-seed-up-quark", +1, 1, 0.5},
            {"s0-seed-down-quark", -1, 2, 0.5},
            {"s0-seed-strange-quark", -1, 3, 0.7},
            {"s0-seed-charm-quark", +1, 1, 1.0},
            {"s0-seed-bottom-quark", -1, 2, 1.4},
            {"s0-seed-top-quark", +1, 3, 2.5},
        };
        double base_norm = 0.0;
        for (const auto& c : cases) {
            ftd::RenderBridge rb(24);
            rb.force_cpu();
            check(std::string(c.id) + " selected wave template dispatches",
                  ftd::dispatch_scenario(rb, c.id));
            check(std::string(c.id) + " isolates only the native wave operator",
                  only_terms_enabled(rb, {"wave_propagation"}));
            int count = 0, state = 0, color = 0;
            for (const auto& v : rb.voxels()) if (v.state != 0) {
                ++count; state = v.state; color = v.color;
            }
            const auto s0 = research_setup_stats(rb);
            if (base_norm == 0.0) base_norm = s0.field_norm;
            const double expected_ratio = (c.boost / 0.5) * (c.boost / 0.5);
            check(std::string(c.id) + " has only its imposed polarity/color marker",
                  count == 1 && state == c.state && color == c.color);
            check(std::string(c.id) + " field norm follows the imposed amplitude code",
                  std::fabs(s0.field_norm / base_norm - expected_ratio) < 1e-12);
            const double h0 = periodic_modified_hamiltonian(rb);
            tick_n(rb, 12);
            const double h1 = periodic_modified_hamiltonian(rb);
            check(std::string(c.id) + " is a stable source-free wave evolution",
                  std::fabs(h1 - h0) / std::max(1e-30, std::fabs(h0)) < 1e-12
                  && manifested_count(rb) == 1);
        }
    }

    {
        constexpr int L = 24;
        ftd::RenderBridge base(L), medium(L), high(L);
        base.force_cpu(); medium.force_cpu(); high.force_cpu();
        check("three negative radial-dressing templates dispatch",
              ftd::dispatch_scenario(base, "s0-vacuum-electron")
              && ftd::dispatch_scenario(medium, "s0-vacuum-muon")
              && ftd::dispatch_scenario(high, "s0-vacuum-tau"));
        const auto scaled_error = [](const ftd::RenderBridge& a,
                                     const ftd::RenderBridge& b,
                                     double scale) {
            double r2 = 0.0, n2 = 0.0;
            for (std::size_t i = 0; i < a.voxels().size(); ++i) {
                r2 += (b.voxels()[i].flux - a.voxels()[i].flux * scale).mag2()
                    + (b.voxels()[i].wave_vel
                       - a.voxels()[i].wave_vel * scale).mag2();
                n2 += b.voxels()[i].flux.mag2() + b.voxels()[i].wave_vel.mag2();
            }
            return std::sqrt(r2 / std::max(1e-30, n2));
        };
        check("negative templates differ only by imposed 1.2x/1.5x field amplitude",
              scaled_error(base, medium, 1.2) < 1e-12
              && scaled_error(base, high, 1.5) < 1e-12);
        check("negative templates contain one inert negative marker each",
              manifested_count(base) == 1 && manifested_count(medium) == 1
              && manifested_count(high) == 1
              && only_terms_enabled(base, {"wave_propagation"})
              && only_terms_enabled(medium, {"wave_propagation"})
              && only_terms_enabled(high, {"wave_propagation"}));
        tick_n(base, 12); tick_n(medium, 12); tick_n(high, 12);
        check("amplitude copies remain exact under the linear wave map",
              scaled_error(base, medium, 1.2) < 1e-12
              && scaled_error(base, high, 1.5) < 1e-12
              && manifested_count(base) == 1 && manifested_count(medium) == 1
              && manifested_count(high) == 1);
    }

    {
        struct Case { const char* id; int expected_states; };
        const Case cases[] = {
            {"s0-seed-higgs-field", 0},
            {"s0-seed-gluon", 0},
            {"s0-vacuum-w-boson", 1},
            {"s0-vacuum-z-boson", 0},
            {"s0-vacuum-higgs", 0},
        };
        for (const auto& c : cases) {
            ftd::RenderBridge rb(24);
            rb.force_cpu();
            check(std::string(c.id) + " vector-field template dispatches",
                  ftd::dispatch_scenario(rb, c.id));
            const auto s0 = research_setup_stats(rb);
            check(std::string(c.id) + " is nonempty selected initial data",
                  s0.field_norm > 0.0 && s0.finite
                  && s0.manifested == c.expected_states
                  && only_terms_enabled(rb, {"wave_propagation"}));
            const double h0 = periodic_modified_hamiltonian(rb);
            tick_n(rb, 12);
            const double h1 = periodic_modified_hamiltonian(rb);
            const auto s1 = research_setup_stats(rb);
            const double drift = std::fabs(h1 - h0)
                               / std::max(1e-30, std::fabs(h0));
            std::cout << "    " << c.id << " modified-H drift=" << drift << '\n';
            check(std::string(c.id) + " follows finite source-free wave dynamics",
                  drift < 1e-12 && s1.finite
                  && s1.manifested == c.expected_states);
        }
    }
}

void test_tagged_pair_bookkeeping() {
    ftd::RenderBridge rb(24);
    rb.force_cpu();
    check("tagged pair dispatched", ftd::dispatch_scenario(rb, "quantum-entangle"));
    int count = 0;
    int signed_sum = 0;
    int pair_id = -1;
    ftd::Vec3 flux_sum;
    bool same_pair = true;
    for (const auto& v : rb.voxels()) {
        if (v.state == 0) continue;
        ++count;
        signed_sum += v.state;
        flux_sum += v.flux;
        if (pair_id < 0) pair_id = v.pair_id;
        else same_pair = same_pair && v.pair_id == pair_id;
    }
    check("tagged pair contains exactly two sites", count == 2);
    check("tagged pair polarities cancel", signed_sum == 0);
    check("tagged pair shares one nonnegative pair id", same_pair && pair_id >= 0);
    check("tagged pair flux cancels", flux_sum.mag2() < 1e-24);
}

void test_composite_candidates_are_unlocked() {
    ftd::RenderBridge rb(33);
    rb.force_cpu();
    check("proton candidate dispatched", ftd::dispatch_scenario(rb, "s0-vacuum-proton"));
    int count = 0;
    bool any_locked = false;
    bool colors[4] = {false, false, false, false};
    for (const auto& v : rb.voxels()) {
        if (v.state == 0) continue;
        ++count;
        any_locked = any_locked || v.locked;
        if (v.color >= 1 && v.color <= 3) colors[v.color] = true;
    }
    check("proton candidate seeds three manifested constituents", count == 3);
    check("proton candidate does not impose stability with locks", !any_locked);
    check("proton candidate carries three color labels", colors[1] && colors[2] && colors[3]);
    check("proton candidate uses implemented color dynamics",
          rb.toggles.color_forces && rb.toggles.forces && rb.toggles.movement
          && !rb.toggles.strong_force && !rb.toggles.confinement);
}

void test_moore_geometry_seed_contracts() {
    struct Contract {
        const char* id;
        int total;
        int shell1;
        int shell2;
        int shell3;
        bool alternating;
    };
    const Contract contracts[] = {
        {"s0-seed-octahedron",          7,  6,  0, 0, false},
        {"s0-seed-cuboctahedron",      13,  0, 12, 0, false},
        {"s0-seed-stella-octangula",   9,  0,  0, 8, false},
        {"s0-seed-moore-cell",         27,  6, 12, 8, false},
        {"s0-seed-moore-decomposition",27,  6, 12, 8, true},
    };

    for (const auto& c : contracts) {
        constexpr int L = 25;
        constexpr int m = L / 2;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check(std::string(c.id) + " dispatched", ftd::dispatch_scenario(rb, c.id));

        int total = 0;
        int shells[4] = {0, 0, 0, 0};
        bool exact_support = true;
        bool exact_states = true;
        bool zero_fields = true;
        for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(rb.lattice().index(x, y, z))];
            zero_fields = zero_fields && v.flux.mag2() == 0.0
                         && v.wave_vel.mag2() == 0.0 && v.velocity.mag2() == 0.0;
            if (v.state == 0) continue;
            ++total;
            const int dx = x - m, dy = y - m, dz = z - m;
            const int r2 = dx*dx + dy*dy + dz*dz;
            if (r2 < 0 || r2 > 3) exact_support = false;
            else ++shells[r2];
            if (r2 == 0) exact_states = exact_states && v.state == -1;
            else if (c.alternating) {
                exact_states = exact_states && v.state == (r2 == 2 ? -1 : +1);
            } else {
                exact_states = exact_states && v.state == +1;
            }
        }
        const bool exact_geometry = total == c.total && shells[0] == 1
            && shells[1] == c.shell1 && shells[2] == c.shell2
            && shells[3] == c.shell3 && exact_support && exact_states;
        check(std::string(c.id) + " has exact shell orbit and ternary labels",
              exact_geometry);
        check(std::string(c.id) + " has no field or motion dressing", zero_fields);
        check(std::string(c.id) + " uses an inert all-terms-off profile",
              only_terms_enabled(rb, {}));

        tick_n(rb, 8);
        check(std::string(c.id) + " remains the same inert construction",
              manifested_count(rb) == c.total && only_terms_enabled(rb, {}));
    }
}

void test_cluster_amplitude_ordering() {
    const char* ids[] = {
        "s0-seed-cluster-law-subknee",
        "s0-seed-cluster-law-knee",
        "s0-seed-cluster-law-superknee",
    };
    int counts[3] = {0, 0, 0};
    for (int i = 0; i < 3; ++i) {
        ftd::RenderBridge rb(33);
        rb.force_cpu();
        check(std::string(ids[i]) + " dispatched", ftd::dispatch_scenario(rb, ids[i]));
        check(std::string(ids[i]) + " uses isolated genesis-response terms",
              only_terms_enabled(rb, {"wave_propagation", "genesis",
                                      "gauss_projection", "langevin"}));
        tick_n(rb, 200);
        counts[i] = manifested_count(rb);
        const int at_200 = counts[i];
        tick_n(rb, 20);
        check(std::string(ids[i]) + " response is stable from tick 200 to 220",
              manifested_count(rb) == at_200);
    }
    std::cout << "    cluster counts A=12/16/40: "
              << counts[0] << '/' << counts[1] << '/' << counts[2] << '\n';
    check("selected genesis response is ordered across A=12,16,40",
          counts[0] > 0 && counts[0] < counts[1] && counts[1] < counts[2]);
}

void test_locked_mass_latency_probe() {
    constexpr int L = 33;
    constexpr int m = L / 2;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("locked mass latency probe dispatched",
          ftd::dispatch_scenario(rb, "s0-seed-massive-body"));
    check("locked mass probe isolates only the latency solver",
          only_terms_enabled(rb, {"gravity", "latency_field"}));

    int count = 0;
    bool all_locked = true;
    for (const auto& v : rb.voxels()) {
        if (v.state == 0) continue;
        ++count;
        all_locked = all_locked && v.locked;
    }
    check("locked mass probe seeds the exact compact 33-site ball", count == 33);
    check("every mass-source site is locked", all_locked);

    tick_n(rb, 4);
    const double center = rb.voxel_at(m, m, m).latency;
    const double near = rb.voxel_at(m + 4, m, m).latency;
    const double far = rb.voxel_at(0, m, m).latency;
    std::cout << "    mass latency center/near/far="
              << center << '/' << near << '/' << far << '\n';
    check("native latency solution is positive and radially decreasing",
          center > near && near > far && far >= 0.0);
    check("locked source remains static without genesis or motion",
          manifested_count(rb) == 33);
}

void test_periodic_random_wave_bath() {
    ftd::RenderBridge rb(24);
    rb.force_cpu();
    check("periodic random wave bath dispatched",
          ftd::dispatch_scenario(rb, "flux-zero-point"));
    check("random bath isolates the bare unprojected wave map",
          only_terms_enabled(rb, {"wave_propagation"})
          && rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    check("random bath starts unmanifested", manifested_count(rb) == 0);
    const double h0 = periodic_modified_hamiltonian(rb);
    double min_h = h0;
    double max_h = h0;
    for (int t = 0; t < 200; ++t) {
        rb.tick();
        const double h = periodic_modified_hamiltonian(rb);
        min_h = std::min(min_h, h);
        max_h = std::max(max_h, h);
    }
    const double relative_span = (max_h - min_h) / std::max(1e-30, std::fabs(h0));
    std::cout << "    random-bath H0=" << h0
              << " relative_span=" << relative_span << '\n';
    check("random bath conserves the exact kick-drift modified Hamiltonian",
          h0 > 0.0 && relative_span < 1e-11);
    check("random bath remains unmanifested", manifested_count(rb) == 0);
}

void test_high_amplitude_packet_dispersion() {
    constexpr int L = 48;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("high-amplitude packet dispatched",
          ftd::dispatch_scenario(rb, "flux-soliton"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    check("high-amplitude packet uses isolated wave terms",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection"}));
    check("high-amplitude packet is transverse and unmanifested",
          normalized_divergence(rb) < 1e-12 && manifested_count(rb) == 0);
    const auto before = x_profile(rb);
    tick_n(rb, 20);
    const auto after = x_profile(rb);
    const double dx = forward_delta(after.centroid, before.centroid, L);
    const double width_ratio = after.width / before.width;
    std::cout << "    high-amplitude dx=" << dx
              << " width_ratio=" << width_ratio << '\n';
    check("high-amplitude packet centroid moves in +x while dispersing", dx > 3.0);
    check("high-amplitude packet exhibits measurable lattice dispersion",
          width_ratio > 2.0);
    check("high-amplitude packet remains unmanifested",
          manifested_count(rb) == 0);
}

void test_field_photon_packet_qualification_gate() {
    constexpr int L = 48;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("field photon packet dispatched",
          ftd::dispatch_scenario(rb, "s0-field-photon-pulse"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    check("field photon packet uses isolated wave terms",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection"}));
    check("field photon packet is transverse",
          normalized_divergence(rb) < 1e-12);
    const auto before = x_profile(rb);
    tick_n(rb, 20);
    const auto after = x_profile(rb);
    const double dx = forward_delta(after.centroid, before.centroid, L);
    const double speed = dx / 20.0;
    const double width_ratio = after.width / before.width;
    std::cout << "    field-photon dx=" << dx << " speed=" << speed
              << " width_ratio=" << width_ratio << '\n';
    check("field photon candidate fails the near-C_SPEED centroid gate",
          std::fabs(speed - ftd::C_SPEED) >= 0.08);
    check("field photon candidate fails the coherence gate without manifesting",
          width_ratio >= 1.25 && manifested_count(rb) == 0);
}

void test_bidirectional_transverse_lobes() {
    constexpr int L = 48;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    check("bidirectional lobe proxy dispatched",
          ftd::dispatch_scenario(rb, "light-dipole"));
    rb.toggles.flux_boundary = ftd::FluxBoundaryMode::Periodic;
    check("bidirectional lobes use isolated wave terms",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection"}));
    check("bidirectional lobes are transverse and unmanifested",
          normalized_divergence(rb) < 1e-12 && manifested_count(rb) == 0);
    const auto before = x_profile(rb);
    tick_n(rb, 12);
    const auto after = x_profile(rb);
    double left = 0.0;
    double right = 0.0;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const auto& v = rb.voxel_at(x, y, z);
        const double e = v.flux.mag2() + v.wave_vel.mag2();
        if (x < L / 2) left += e;
        else if (x > L / 2) right += e;
    }
    const double balance = std::fabs(left - right) / std::max(1e-30, left + right);
    const double width_ratio = after.width / before.width;
    std::cout << "    bidirectional width_ratio=" << width_ratio
              << " half_balance=" << balance << '\n';
    check("two transverse lobes separate symmetrically",
          width_ratio > 1.3 && balance < 1e-10);
    check("bidirectional lobe proxy remains unmanifested",
          manifested_count(rb) == 0);
}

void test_uniform_field_initial_data() {
    constexpr int L = 25;
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("uniform electric proxy dispatched",
              ftd::dispatch_scenario(rb, "s0-field-uniform-e"));
        bool exact = only_terms_enabled(rb, {});
        for (const auto& v : rb.voxels()) {
            exact = exact && v.state == 0 && v.flux.mag2() == 0.0
                && std::fabs(v.wave_vel.x + 0.1) < 1e-15
                && v.wave_vel.y == 0.0 && v.wave_vel.z == 0.0;
        }
        check("uniform electric proxy is exact inert canonical momentum", exact);
        tick_n(rb, 8);
        check("uniform electric proxy remains unchanged with all terms off",
              std::fabs(rb.voxel_at(L/2, L/2, L/2).wave_vel.x + 0.1) < 1e-15);
    }
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("uniform magnetic proxy dispatched",
              ftd::dispatch_scenario(rb, "s0-field-uniform-b"));
        double max_error = 0.0;
        double max_other = 0.0;
        for (int z = 1; z < L - 1; ++z)
        for (int y = 1; y < L - 1; ++y)
        for (int x = 1; x < L - 1; ++x) {
            const auto b = rb.curl_flux(rb.lattice().index(x, y, z));
            max_error = std::max(max_error, std::fabs(b.z - 0.05));
            max_other = std::max(max_other, std::hypot(b.x, b.y));
        }
        std::cout << "    uniform-B interior curl error=" << max_error
                  << " transverse_curl=" << max_other << '\n';
        check("uniform magnetic proxy has exact interior z curl",
              only_terms_enabled(rb, {}) && max_error < 1e-12 && max_other < 1e-12);
        tick_n(rb, 8);
        const auto b = rb.curl_flux(rb.lattice().index(L/2, L/2, L/2));
        check("uniform magnetic proxy remains unchanged with all terms off",
              std::fabs(b.z - 0.05) < 1e-12);
    }
}

void test_reference_geometry_ansatze() {
    constexpr int L = 33;
    constexpr int m = L / 2;
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("tangential ring ansatz dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-sloop"));
        int count = 0;
        bool exact = only_terms_enabled(rb, {});
        ftd::Vec3 flux_sum;
        for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
            const auto& v = rb.voxel_at(x, y, z);
            if (v.state == 0) continue;
            ++count;
            exact = exact && z == m && v.state == +1
                && std::fabs(v.flux.mag() - ftd::K_B) < 1e-12
                && v.wave_vel.mag2() == 0.0;
            flux_sum += v.flux;
        }
        check("tangential ring has 12 equal-flux sites in one plane",
              count == 12 && exact && flux_sum.mag2() < 1e-24);
        tick_n(rb, 8);
        check("tangential ring remains an inert ansatz", manifested_count(rb) == 12);
    }
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("alternating Moore-shell cell dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-observer-cell"));
        int shells[4] = {0, 0, 0, 0};
        bool exact = only_terms_enabled(rb, {});
        for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
            const auto& v = rb.voxel_at(x, y, z);
            if (v.state == 0) continue;
            const int dx=x-m, dy=y-m, dz=z-m;
            const int r2=dx*dx+dy*dy+dz*dz;
            if (r2 < 0 || r2 > 3) { exact = false; continue; }
            ++shells[r2];
            const int expected = (r2 % 2 == 0) ? +1 : -1;
            exact = exact && v.state == expected && v.flux.mag2() == 0.0;
        }
        check("alternating cell is the exact imposed 1+6+12+8 pattern",
              exact && shells[0]==1 && shells[1]==6 && shells[2]==12 && shells[3]==8);
        tick_n(rb, 8);
        check("alternating Moore-shell cell remains inert", manifested_count(rb) == 27);
    }
}

void test_gauge_named_initial_data_without_gauge_claims() {
    constexpr int L = 33;
    constexpr int m = L / 2;
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("oriented square path dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-wilson-loop"));
        const int R = std::max(3, L / 8);
        bool exact = only_terms_enabled(rb, {});
        int support = 0;
        ftd::Vec3 sum;
        for (int z=0;z<L;++z) for (int y=0;y<L;++y) for (int x=0;x<L;++x) {
            ftd::Vec3 expected;
            if (z == m && y == m-R && x >= m-R && x <= m+R) expected.x += ftd::K_B;
            if (z == m && x == m+R && y >= m-R && y <= m+R) expected.y += ftd::K_B;
            if (z == m && y == m+R && x >= m-R && x <= m+R) expected.x -= ftd::K_B;
            if (z == m && x == m-R && y >= m-R && y <= m+R) expected.y -= ftd::K_B;
            const auto& v = rb.voxel_at(x,y,z);
            exact = exact && (v.flux-expected).mag2() < 1e-28
                && v.wave_vel.mag2() == 0.0 && v.state == 0;
            if (v.flux.mag2() > 0.0) ++support;
            sum += v.flux;
        }
        check("square path is the exact oriented 8R-site vector ansatz",
              exact && support == 8*R && sum.mag2() < 1e-24);
        tick_n(rb, 4);
        check("square path remains inert and computes no Wilson observable",
              manifested_count(rb) == 0 && only_terms_enabled(rb, {}));
    }
    {
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("Gaussian axial tube dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-flux-tube"));
        const int half = std::max(6, L/4) / 2;
        bool exact = only_terms_enabled(rb, {});
        int states = 0;
        int signed_sum = 0;
        for (int z=0;z<L;++z) for (int y=0;y<L;++y) for (int x=0;x<L;++x) {
            const double dy=y-m, dz=z-m;
            const double g=ftd::K_B*std::exp(-(dy*dy+dz*dz)/(2.0*1.5*1.5));
            const double expected=(x>=m-half && x<=m+half && g>0.001)?g:0.0;
            const auto& v=rb.voxel_at(x,y,z);
            exact=exact && std::fabs(v.flux.x-expected)<1e-14
                && v.flux.y==0.0 && v.flux.z==0.0 && v.wave_vel.mag2()==0.0;
            if(v.state!=0){++states;signed_sum+=v.state;}
        }
        check("tube is an exact Gaussian axial profile with neutral endpoints",
              exact && states==2 && signed_sum==0);
    }
    {
        ftd::RenderBridge rb(25);
        rb.force_cpu();
        check("radial inverse-square profile dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-monopole"));
        const int c=12;
        bool exact=only_terms_enabled(rb, {});
        for(int z=0;z<25;++z) for(int y=0;y<25;++y) for(int x=0;x<25;++x){
            const auto& v=rb.voxel_at(x,y,z);
            const ftd::Vec3 r(x-c,y-c,z-c);
            const double radius=r.mag();
            if(radius==0.0){exact=exact&&v.flux.mag2()==0.0;continue;}
            const double expected=1.0/(4.0*ftd::PI*radius*radius);
            exact=exact && v.state==0
                && ftd::Vec3::cross(v.flux,r).mag()<1e-13
                && std::fabs(v.flux.mag()-expected)<1e-13;
        }
        check("monopole-named seed is now exactly radial inverse-square initial data", exact);
    }
    {
        ftd::RenderBridge rb(25);
        rb.force_cpu();
        check("localized radial profile dispatched",
              ftd::dispatch_scenario(rb, "s0-seed-instanton"));
        const int c=12;
        bool exact=only_terms_enabled(rb, {});
        for(int z=0;z<25;++z) for(int y=0;y<25;++y) for(int x=0;x<25;++x){
            const auto& v=rb.voxel_at(x,y,z);
            const ftd::Vec3 r(x-c,y-c,z-c);
            const double radius=r.mag();
            if(radius<0.5){exact=exact&&v.flux.mag2()==0.0;continue;}
            const double expected=3.0/(radius*radius+9.0);
            exact=exact && ftd::Vec3::cross(v.flux,r).mag()<1e-12
                && std::fabs(v.flux.mag()-expected)<1e-13;
        }
        check("instanton-named seed is only the declared radial 3-vector profile", exact);
    }
}

void test_selected_field_ansatze() {
    constexpr int L=25;
    constexpr int c=L/2;
    {
        ftd::RenderBridge rb(L); rb.force_cpu();
        check("softened opposite-source profile dispatched",
              ftd::dispatch_scenario(rb,"s0-field-electric-dipole"));
        const int half=std::max(2,L/8)/2, px=c+half, nx=c-half;
        const double amp=ftd::ALPHA/(4.0*ftd::PI);
        bool exact=only_terms_enabled(rb, {}); int count=0,sum=0;
        for(int z=0;z<L;++z) for(int y=0;y<L;++y) for(int x=0;x<L;++x){
            const ftd::Vec3 rp(x-px,y-c,z-c), rn(x-nx,y-c,z-c);
            const double dp=std::pow(rp.mag2()+1.0,1.5);
            const double dn=std::pow(rn.mag2()+1.0,1.5);
            ftd::Vec3 expected=rp*(amp/dp)-rn*(amp/dn);
            if(expected.mag()<=1e-6) expected=ftd::Vec3();
            const auto& v=rb.voxel_at(x,y,z);
            exact=exact&&(v.flux-expected).mag()<1e-13&&v.wave_vel.mag2()==0.0;
            if(v.state!=0){++count;sum+=v.state;}
        }
        check("opposite-source field equals the declared softened Coulomb ansatz",
              exact&&count==2&&sum==0);
    }
    {
        ftd::RenderBridge rb(L); rb.force_cpu();
        check("softened dipole vector potential dispatched",
              ftd::dispatch_scenario(rb,"s0-field-magnetic-dipole"));
        const double mu=ftd::K_B/(4.0*ftd::PI);
        bool exact=only_terms_enabled(rb, {});
        for(int z=0;z<L;++z) for(int y=0;y<L;++y) for(int x=0;x<L;++x){
            const double rx=x-c,ry=y-c,rz=z-c;
            const double d=std::pow(rx*rx+ry*ry+rz*rz+1.0,1.5);
            ftd::Vec3 expected(-mu*ry/d,mu*rx/d,0.0);
            if(expected.mag()<=1e-8) expected=ftd::Vec3();
            const auto& v=rb.voxel_at(x,y,z);
            exact=exact&&(v.flux-expected).mag()<1e-13&&v.state==0;
        }
        check("dipole-potential field equals the exact selected smooth ansatz",exact);
    }
    {
        ftd::RenderBridge rb(L); rb.force_cpu();
        check("azimuthal inverse-radius profile dispatched",
              ftd::dispatch_scenario(rb,"s0-field-vortex-line"));
        const double target=ftd::K_B*4.0/(2.0*ftd::PI);
        bool exact=only_terms_enabled(rb, {});
        for(int z=0;z<L;++z) for(int y=0;y<L;++y) for(int x=0;x<L;++x){
            const double rx=x-c,ry=y-c,r=std::hypot(rx,ry);
            const auto& v=rb.voxel_at(x,y,z);
            if(r==0.0){exact=exact&&v.flux.mag2()==0.0;continue;}
            exact=exact&&std::fabs(v.flux.x*rx+v.flux.y*ry)<1e-13
                &&v.flux.z==0.0&&std::fabs(r*v.flux.mag()-target)<1e-13;
        }
        check("vortex-named field is exactly tangential with r times magnitude constant",exact);
    }
}

void test_symmetric_wave_pair_profiles() {
    {
        ftd::RenderBridge rb(48); rb.force_cpu();
        check("antisymmetric wave pair dispatched",ftd::dispatch_scenario(rb,"flux-dipole"));
        check("antisymmetric pair isolates the periodic wave map",
              only_terms_enabled(rb,{"wave_propagation"})
              &&rb.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
        check("antisymmetric pair starts at exact odd x parity",x_reflection_error(rb,-1.0)<1e-13);
        tick_n(rb,12);
        check("native wave evolution preserves odd x parity without manifestation",
              x_reflection_error(rb,-1.0)<1e-12&&manifested_count(rb)==0);
    }
    {
        ftd::RenderBridge rb(48); rb.force_cpu();
        check("reflection-even broadband pair dispatched",ftd::dispatch_scenario(rb,"flux-standing"));
        double w2=0.0;for(const auto&v:rb.voxels())w2+=v.wave_vel.mag2();
        check("broadband pair starts even with zero canonical momentum",
              only_terms_enabled(rb,{"wave_propagation"})&&w2==0.0
              &&x_reflection_error(rb,+1.0)<1e-13);
        tick_n(rb,12);
        check("native wave evolution preserves even x parity without manifestation",
              x_reflection_error(rb,+1.0)<1e-12&&manifested_count(rb)==0);
    }
    {
        ftd::RenderBridge rb(33); rb.force_cpu();
        check("helical ring ansatz dispatched",ftd::dispatch_scenario(rb,"flux-vortex"));
        const int m=16;double circulation=0.0,axial=0.0;bool support=only_terms_enabled(rb,{});
        for(int z=0;z<33;++z)for(int y=0;y<33;++y)for(int x=0;x<33;++x){
            const auto&v=rb.voxel_at(x,y,z);if(v.flux.mag2()==0.0)continue;
            support=support&&std::abs(y-m)<=1&&v.state==0&&v.wave_vel.mag2()==0.0;
            circulation+=(x-m)*v.flux.z-(z-m)*v.flux.x;axial+=v.flux.y;
        }
        std::cout << "    helical-ring support=" << support
                  << " circulation=" << circulation << " axial=" << axial << '\n';
        check("helical ring has positive imposed circulation and axial bias",
              support&&circulation>0.0&&axial>0.0);
    }
}

void test_wave_family_native_dispersion() {
    constexpr int L=48;
    struct NativeLane { const char* id; int mode; int axis; };
    const NativeLane native_lanes[] = {
        {"s0-field-rf-lattice-wave",1,1},
        {"s0-field-light-lattice-wave",6,1},
    };
    for(const auto& lane:native_lanes){
        ftd::RenderBridge rb(L);rb.force_cpu();
        check(std::string(lane.id)+" dispatched",ftd::dispatch_scenario(rb,lane.id));
        check(std::string(lane.id)+" isolates the periodic wave map",
              only_terms_enabled(rb,{"wave_propagation"})
              &&rb.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
        const double k=2.0*ftd::PI*lane.mode/L;
        const double omega=2.0*std::asin(ftd::C_SPEED*std::sin(k/2.0));
        const auto a0=project_flux_harmonic(rb,lane.mode,lane.axis);
        tick_n(rb,1);
        const auto a1=project_flux_harmonic(rb,lane.mode,lane.axis);
        const double expected_s=a0.sine*std::cos(omega)+a0.cosine*std::sin(omega);
        const double expected_c=a0.cosine*std::cos(omega)-a0.sine*std::sin(omega);
        const double scale=std::max(1e-30,std::hypot(a0.sine,a0.cosine));
        const double error=std::hypot(a1.sine-expected_s,a1.cosine-expected_c)/scale;
        std::cout<<"    "<<lane.id<<" omega="<<omega<<" one_tick_error="<<error<<'\n';
        check(std::string(lane.id)+" plane-average follows the exact kick-drift pole",
              scale>0.0&&error<1e-11&&manifested_count(rb)==0);
    }

    ftd::RenderBridge sound(L);sound.force_cpu();
    check("longitudinal sound-proxy seed dispatched",
          ftd::dispatch_scenario(sound,"s0-field-sound-lattice-wave"));
    check("sound-proxy seed still uses only the periodic native wave operator",
          only_terms_enabled(sound,{"wave_propagation"})
          &&sound.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
    constexpr int mode=4;
    const double k=2.0*ftd::PI*mode/L;
    const double native_omega=2.0*std::asin(ftd::C_SPEED*std::sin(k/2.0));
    const double seed_omega=2.0*(ftd::C_SPEED/8.0)*std::sin(k/2.0);
    const auto a0=project_flux_harmonic(sound,mode,0);
    tick_n(sound,1);const auto a1=project_flux_harmonic(sound,mode,0);
    tick_n(sound,1);const auto a2=project_flux_harmonic(sound,mode,0);
    const double scale=std::max(1e-30,std::hypot(a0.sine,a0.cosine));
    const auto recurrence_error=[&](double omega){
        const double s=a2.sine-2.0*std::cos(omega)*a1.sine+a0.sine;
        const double c=a2.cosine-2.0*std::cos(omega)*a1.cosine+a0.cosine;
        return std::hypot(s,c)/scale;
    };
    const double native_error=recurrence_error(native_omega);
    const double slow_error=recurrence_error(seed_omega);
    std::cout<<"    sound-proxy native_omega="<<native_omega
             <<" seed_omega="<<seed_omega<<" native_error="<<native_error
             <<" slow_error="<<slow_error<<'\n';
    check("longitudinal mode follows the native pole to numerical precision",
          native_error<1e-11&&manifested_count(sound)==0);
    check("c-over-8 sound-speed interpretation is rejected by the recurrence",
          slow_error>1e-3);
}

void test_native_point_response_cone() {
    constexpr int L=33;
    constexpr int c=L/2;
    ftd::RenderBridge rb(L);rb.force_cpu();
    check("native point-response probe dispatched",
          ftd::dispatch_scenario(rb,"s0-field-spacetime-forcing-boundary"));
    check("point response isolates the periodic production wave map",
          only_terms_enabled(rb,{"wave_propagation"})
          &&rb.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
    const double h0=periodic_modified_hamiltonian(rb);
    bool inside=true;
    int final_front=0;
    for(int t=1;t<=8;++t){
        rb.tick();
        int front=0;
        for(int z=0;z<L;++z)for(int y=0;y<L;++y)for(int x=0;x<L;++x){
            const auto&v=rb.voxel_at(x,y,z);
            if(v.flux.mag2()+v.wave_vel.mag2()==0.0)continue;
            int dx=std::abs(x-c),dy=std::abs(y-c),dz=std::abs(z-c);
            dx=std::min(dx,L-dx);dy=std::min(dy,L-dy);dz=std::min(dz,L-dz);
            const int cheb=std::max(dx,std::max(dy,dz));
            front=std::max(front,cheb);
            inside=inside&&cheb<=t;
        }
        final_front=front;
    }
    const double h8=periodic_modified_hamiltonian(rb);
    const double drift=std::fabs(h8-h0)/std::max(1e-30,std::fabs(h0));
    std::cout<<"    point-response front@8="<<final_front
             <<" modified-H drift="<<drift<<'\n';
    check("point response never outruns one lattice neighborhood per tick",
          inside&&final_front==8);
    check("point response conserves the exact periodic kick-drift invariant",
          drift<1e-12&&manifested_count(rb)==0);
}

void test_multilobe_wave_symmetries() {
    const char* ids[]={"flux-interference","flux-nested-standing"};
    for(const char* id:ids){
        ftd::RenderBridge rb(48);rb.force_cpu();
        check(std::string(id)+" dispatched",ftd::dispatch_scenario(rb,id));
        check(std::string(id)+" isolates the periodic wave map",
              only_terms_enabled(rb,{"wave_propagation"})
              &&rb.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
        check(std::string(id)+" starts even under x and z reflection",
              x_reflection_error(rb,+1.0)<1e-13&&z_reflection_error(rb,+1.0)<1e-13);
        tick_n(rb,12);
        check(std::string(id)+" preserves both reflection symmetries",
              x_reflection_error(rb,+1.0)<1e-12&&z_reflection_error(rb,+1.0)<1e-12
              &&manifested_count(rb)==0);
    }
    {
        ftd::RenderBridge rb(48);rb.force_cpu();
        check("mirror-polarized wave pair dispatched",
              ftd::dispatch_scenario(rb,"flux-dual-substrate"));
        check("mirror-polarized pair leaves dual-substrate dynamics off",
              only_terms_enabled(rb,{"wave_propagation"})
              &&rb.toggles.flux_boundary==ftd::FluxBoundaryMode::Periodic);
        const ftd::Vec3 signs(+1.0,-1.0,-1.0);
        check("mirror pair starts with exact even/odd component parity",
              x_component_parity_error(rb,signs)<1e-13);
        tick_n(rb,12);
        check("native wave map preserves the mixed component parity",
              x_component_parity_error(rb,signs)<1e-12&&manifested_count(rb)==0);
    }
}

void test_imposed_uniform_b_native_curvature() {
    constexpr int L = 48;
    constexpr int ticks = 80;
    const int mc = L / 2;

    struct Endpoint {
        bool found = false;
        double x = 0.0, y = 0.0, z = 0.0;
        ftd::Vec3 velocity;
    };
    auto endpoint = [](const ftd::RenderBridge& rb, int particle_id) {
        Endpoint out;
        const auto& voxels = rb.voxels();
        for (int i = 0; i < static_cast<int>(voxels.size()); ++i) {
            const auto& v = voxels[static_cast<std::size_t>(i)];
            if (v.state == 0 || v.particle_id != particle_id) continue;
            const auto c = rb.lattice().coord(i);
            out.found = true;
            out.x = c.x + v.remainder.x;
            out.y = c.y + v.remainder.y;
            out.z = c.z + v.remainder.z;
            out.velocity = v.velocity;
            break;
        }
        return out;
    };

    ftd::RenderBridge magnetic(L);
    magnetic.force_cpu();
    check("imposed-B curvature probe dispatched",
          ftd::dispatch_scenario(magnetic, "flux-cyclotron"));
    check("curvature probe isolates Lorentz force and movement",
          only_terms_enabled(magnetic, {"forces", "poisson_coulomb", "movement", "lorentz_force"})
          && magnetic.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    const int initial_idx = magnetic.lattice().index(mc, mc, mc);
    const int particle_id = magnetic.voxels()[static_cast<std::size_t>(initial_idx)].particle_id;
    const ftd::Vec3 b0 = magnetic.curl_flux(initial_idx);
    check("imposed vector potential has Bz=1 at the probe",
          std::fabs(b0.x) < 1e-12 && std::fabs(b0.y) < 1e-12
          && std::fabs(b0.z - 1.0) < 1e-12);

    ftd::RenderBridge straight(L);
    straight.force_cpu();
    check("straight-line control dispatched",
          ftd::dispatch_scenario(straight, "flux-cyclotron"));
    straight.toggles.lorentz_force = false;
    const int control_id = straight.voxels()[static_cast<std::size_t>(initial_idx)].particle_id;

    tick_n(magnetic, ticks);
    tick_n(straight, ticks);
    const Endpoint curved = endpoint(magnetic, particle_id);
    const Endpoint control = endpoint(straight, control_id);
    check("magnetic and control markers survive", curved.found && control.found);
    const double initial_speed = 0.12;
    const double speed_drift = std::fabs(curved.velocity.mag() - initial_speed) / initial_speed;
    std::cout << "    imposed-B endpoint=(" << curved.x << ',' << curved.y << ',' << curved.z
              << ") v=(" << curved.velocity.x << ',' << curved.velocity.y << ','
              << curved.velocity.z << ") speed_drift=" << speed_drift
              << " control_y=" << control.y << '\n';
    check("Lorentz branch bends velocity toward minus y",
          curved.velocity.y < -0.05 && std::fabs(curved.y - control.y) > 2.0);
    check("no-Lorentz control remains straight",
          std::fabs(control.y - mc) < 1e-12
          && std::fabs(control.velocity.y) < 1e-12);
    check("magnetic response is approximately work-free at this step size",
          speed_drift < 0.03);
}

void test_prepared_polarity_geometry_seeds() {
    auto primitive_equal = [](const std::vector<ftd::Voxel>& a,
                              const std::vector<ftd::Voxel>& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (a[i].state != b[i].state || a[i].locked != b[i].locked ||
                a[i].flux.x != b[i].flux.x || a[i].flux.y != b[i].flux.y ||
                a[i].flux.z != b[i].flux.z ||
                a[i].wave_vel.x != b[i].wave_vel.x ||
                a[i].wave_vel.y != b[i].wave_vel.y ||
                a[i].wave_vel.z != b[i].wave_vel.z ||
                a[i].velocity.x != b[i].velocity.x ||
                a[i].velocity.y != b[i].velocity.y ||
                a[i].velocity.z != b[i].velocity.z) return false;
        }
        return true;
    };

    {
        constexpr int L = 40;
        constexpr int mc = L / 2;
        constexpr int r = L / 5;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("octahedral polarity-shell seed dispatched",
              ftd::dispatch_scenario(rb, "flux-screening"));
        check("octahedral polarity-shell seed is inert",
              only_terms_enabled(rb, {}));
        int count = 0, signed_sum = 0;
        for (const auto& v : rb.voxels()) {
            if (v.state != 0) { ++count; signed_sum += v.state; }
        }
        const int offsets[6][3] = {
            {r,0,0},{-r,0,0},{0,r,0},{0,-r,0},{0,0,r},{0,0,-r}
        };
        bool shell_exact = rb.voxel_at(mc, mc, mc).state == +1;
        for (const auto& o : offsets) {
            shell_exact = shell_exact &&
                rb.voxel_at(mc + o[0], mc + o[1], mc + o[2]).state == -1;
        }
        check("polarity shell is exactly 1+6 with net state -5",
              count == 7 && signed_sum == -5 && shell_exact);
        double field2 = 0.0;
        for (const auto& v : rb.voxels()) field2 += v.flux.mag2();
        check("polarity shell carries a nonzero imposed radial dressing", field2 > 0.0);
        const auto before = rb.voxels();
        tick_n(rb, 8);
        check("octahedral polarity-shell primitives remain exact",
              primitive_equal(before, rb.voxels()));
    }

    {
        constexpr int L = 48;
        constexpr int mc = L / 2;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("threefold inward-flux seed dispatched",
              ftd::dispatch_scenario(rb, "flux-triad"));
        check("threefold inward-flux seed is inert",
              only_terms_enabled(rb, {}));
        int count = 0, signed_sum = 0, locked = 0;
        double field2 = 0.0;
        for (const auto& v : rb.voxels()) {
            if (v.state != 0) { ++count; signed_sum += v.state; }
            if (v.locked) ++locked;
            field2 += v.flux.mag2();
        }
        const bool positions = rb.voxel_at(mc + 8, mc, mc).state == +1 &&
                               rb.voxel_at(mc - 4, mc, mc + 7).state == +1 &&
                               rb.voxel_at(mc - 4, mc, mc - 7).state == +1;
        check("threefold seed has exactly three unlocked positive markers",
              count == 3 && signed_sum == 3 && locked == 0 && positions);
        check("threefold seed carries nonzero imposed inward dressing", field2 > 0.0);
        const auto before = rb.voxels();
        tick_n(rb, 8);
        check("threefold seed primitives remain exact without binding",
              primitive_equal(before, rb.voxels()));
    }
}

void test_deterministic_random_wave_profiles() {
    auto field_equal = [](const ftd::RenderBridge& a, const ftd::RenderBridge& b) {
        if (a.voxels().size() != b.voxels().size()) return false;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            if (x.state != y.state || x.flux.x != y.flux.x ||
                x.flux.y != y.flux.y || x.flux.z != y.flux.z ||
                x.wave_vel.x != y.wave_vel.x ||
                x.wave_vel.y != y.wave_vel.y ||
                x.wave_vel.z != y.wave_vel.z) return false;
        }
        return true;
    };
    auto outside_energy = [](const ftd::RenderBridge& rb, int cx, int cy, int cz,
                             int radius) {
        const int L = rb.lattice().size();
        double outside = 0.0, total = 0.0;
        for (int z = 0; z < L; ++z)
        for (int y = 0; y < L; ++y)
        for (int x = 0; x < L; ++x) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))];
            const double e = v.flux.mag2() + v.wave_vel.mag2();
            int dx = std::abs(x - cx), dy = std::abs(y - cy), dz = std::abs(z - cz);
            dx = std::min(dx, L - dx); dy = std::min(dy, L - dy); dz = std::min(dz, L - dz);
            total += e;
            if (std::max(dx, std::max(dy, dz)) > radius) outside += e;
        }
        return std::pair<double, double>{outside, total};
    };

    {
        constexpr int L = 32;
        constexpr int corner = L / 4;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("localized random-wave profile dispatched",
              ftd::dispatch_scenario(rb, "flux-thermalization"));
        check("localized random-wave profile isolates the periodic wave map",
              only_terms_enabled(rb, {"wave_propagation"}) &&
              rb.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
        const auto e0 = outside_energy(rb, corner, corner, corner, 6);
        const double h0 = periodic_modified_hamiltonian(rb);
        check("localized random-wave profile starts inside its declared support",
              e0.first == 0.0 && e0.second > 0.0 && manifested_count(rb) == 0);
        tick_n(rb, 12);
        const auto e12 = outside_energy(rb, corner, corner, corner, 6);
        const double h12 = periodic_modified_hamiltonian(rb);
        const double outside_fraction = e12.first / std::max(1e-30, e12.second);
        const double h_drift = std::fabs(h12 - h0) / std::max(1e-30, std::fabs(h0));
        std::cout << "    random-wave outside_fraction=" << outside_fraction
                  << " modified-H drift=" << h_drift << '\n';
        check("native wave map spreads energy beyond the initial patch",
              outside_fraction > 0.01);
        check("random-wave spreading preserves the exact modified Hamiltonian",
              h_drift < 1e-12 && manifested_count(rb) == 0);
    }

    {
        constexpr int L = 24;
        ftd::RenderBridge a(L), b(L);
        a.force_cpu(); b.force_cpu();
        check("finite random-wave ball dispatched twice",
              ftd::dispatch_scenario(a, "flux-vacuum-foam") &&
              ftd::dispatch_scenario(b, "flux-vacuum-foam"));
        check("random-wave ball uses only the periodic source-free wave map",
              only_terms_enabled(a, {"wave_propagation"}) &&
              a.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
        check("random-wave ball initial data is exact fixed-seed replay",
              field_equal(a, b) && manifested_count(a) == 0);
        const double h0 = periodic_modified_hamiltonian(a);
        tick_n(a, 12); tick_n(b, 12);
        const double h12 = periodic_modified_hamiltonian(a);
        const double h_drift = std::fabs(h12 - h0) / std::max(1e-30, std::fabs(h0));
        check("random-wave ball remains exact replay without ongoing noise",
              field_equal(a, b));
        check("random-wave ball conserves modified H and remains unmanifested",
              h_drift < 1e-12 && manifested_count(a) == 0);
    }
}

void test_fixed_seed_genesis_response_profiles() {
    auto exact_state_and_field = [](const ftd::RenderBridge& a,
                                    const ftd::RenderBridge& b) {
        if (a.voxels().size() != b.voxels().size()) return false;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            if (x.state != y.state || x.particle_id != y.particle_id ||
                x.pair_id != y.pair_id || x.flux.x != y.flux.x ||
                x.flux.y != y.flux.y || x.flux.z != y.flux.z ||
                x.wave_vel.x != y.wave_vel.x ||
                x.wave_vel.y != y.wave_vel.y ||
                x.wave_vel.z != y.wave_vel.z) return false;
        }
        return true;
    };

    const char* ids[] = {"flux-cascade", "flux-random-genesis"};
    for (const char* id : ids) {
        ftd::RenderBridge a(32), b(32);
        a.force_cpu(); b.force_cpu();
        check(std::string(id) + " dispatched twice",
              ftd::dispatch_scenario(a, id) && ftd::dispatch_scenario(b, id));
        check(std::string(id) + " isolates the selected genesis rule",
              only_terms_enabled(a, {"genesis"}) && manifested_count(a) == 0);
        check(std::string(id) + " initial data is fixed-seed replay",
              exact_state_and_field(a, b));
        a.tick(); b.tick();
        int positive = 0, negative = 0, paired = 0;
        for (const auto& v : a.voxels()) {
            if (v.state > 0) ++positive;
            if (v.state < 0) ++negative;
            if (v.state != 0 && v.pair_id >= 0) ++paired;
        }
        std::cout << "    " << id << " one-tick +/-=" << positive
                  << '/' << negative << " paired=" << paired << '\n';
        check(std::string(id) + " produces a nonempty single-site cohort",
              positive + negative > 0 && paired == 0);
        check(std::string(id) + " one-tick outcomes replay exactly",
              exact_state_and_field(a, b));
    }
}

void test_multistate_free_transport_controls() {
    struct Site {
        bool found = false;
        int x = 0, y = 0, z = 0;
        ftd::Vec3 velocity;
        ftd::Vec3 remainder;
    };
    auto find_id = [](const ftd::RenderBridge& rb, int id) {
        Site out;
        for (int i = 0; i < static_cast<int>(rb.voxels().size()); ++i) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(i)];
            if (v.state == 0 || v.particle_id != id) continue;
            const auto c = rb.lattice().coord(i);
            out = {true, c.x, c.y, c.z, v.velocity, v.remainder};
            break;
        }
        return out;
    };

    {
        constexpr int L = 40;
        constexpr int mc = L / 2;
        constexpr int left_x = 15;
        constexpr int right_x = 24;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("outward opposite-polarity control dispatched",
              ftd::dispatch_scenario(rb, "flux-string-breaking"));
        check("outward polarity control uses movement only",
              only_terms_enabled(rb, {"movement"}));
        const int left_id = rb.voxel_at(left_x, mc, mc).particle_id;
        const int right_id = rb.voxel_at(right_x, mc, mc).particle_id;
        tick_n(rb, 4);
        const Site left = find_id(rb, left_id);
        const Site right = find_id(rb, right_id);
        check("outward states translate one lattice face after four ticks",
              left.found && right.found && left.x == left_x - 1 &&
              right.x == right_x + 1 && left.y == mc && right.y == mc);
        check("outward velocities and signed remainders remain exact",
              std::fabs(left.velocity.x + 0.3) < 1e-12 &&
              std::fabs(right.velocity.x - 0.3) < 1e-12 &&
              std::fabs(left.remainder.x + 0.2) < 1e-12 &&
              std::fabs(right.remainder.x - 0.2) < 1e-12);
        check("outward transport creates no string-breaking pair",
              manifested_count(rb) == 2 && left_id >= 0 && right_id >= 0);
    }

    {
        constexpr int L = 48;
        constexpr int mc = L / 2;
        ftd::RenderBridge rb(L);
        rb.force_cpu();
        check("threefold tangential transport control dispatched",
              ftd::dispatch_scenario(rb, "flux-baryon"));
        check("threefold transport control uses movement only",
              only_terms_enabled(rb, {"movement"}));
        const int id0 = rb.voxel_at(32, mc, mc).particle_id;
        const int id1 = rb.voxel_at(20, mc, 30).particle_id;
        const int id2 = rb.voxel_at(19, mc, 17).particle_id;
        const int sea_id = rb.voxel_at(28, 28, mc).particle_id;
        std::cout << "    threefold ids=" << id0 << '/' << id1 << '/' << id2
                  << " opposite=" << sea_id << '\n';
        tick_n(rb, 30);
        const Site p0 = find_id(rb, id0);
        const Site p1 = find_id(rb, id1);
        const Site p2 = find_id(rb, id2);
        const Site sea = find_id(rb, sea_id);
        std::cout << "    threefold sites=(" << p0.x << ',' << p0.z << ")/("
                  << p1.x << ',' << p1.z << ")/(" << p2.x << ',' << p2.z
                  << ") opposite=(" << sea.x << ',' << sea.y << ',' << sea.z << ")\n";
        check("threefold seeded velocities produce exact face translations",
              p0.found && p1.found && p2.found &&
              p0.x == 32 && p0.z == 25 &&
              p1.x == 19 && p1.z == 30 &&
              p2.x == 20 && p2.z == 17);
        check("stationary opposite marker remains fixed and no binding occurs",
              sea.found && sea.x == 28 && sea.y == 28 && sea.z == mc &&
              manifested_count(rb) == 4);
        bool any_locked = false;
        for (const auto& v : rb.voxels()) any_locked = any_locked || v.locked;
        check("threefold transport leaves every marker unlocked", !any_locked);
    }
}

void test_fixed_temperature_langevin_genesis_probe() {
    auto exact_state_and_field = [](const ftd::RenderBridge& a,
                                    const ftd::RenderBridge& b) {
        if (a.voxels().size() != b.voxels().size()) return false;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            if (x.state != y.state || x.particle_id != y.particle_id ||
                x.pair_id != y.pair_id || x.flux.x != y.flux.x ||
                x.flux.y != y.flux.y || x.flux.z != y.flux.z ||
                x.wave_vel.x != y.wave_vel.x ||
                x.wave_vel.y != y.wave_vel.y ||
                x.wave_vel.z != y.wave_vel.z) return false;
        }
        return true;
    };
    auto excitation = [](const ftd::RenderBridge& rb) {
        double total = 0.0;
        for (const auto& v : rb.voxels())
            total += v.flux.mag2() + v.wave_vel.mag2();
        return total;
    };

    constexpr int L = 16;
    ftd::RenderBridge a(L), b(L);
    a.force_cpu(); b.force_cpu();
    check("fixed-temperature bath probe dispatched twice",
          ftd::dispatch_scenario(a, "s0-seed-thermal-ignition") &&
          ftd::dispatch_scenario(b, "s0-seed-thermal-ignition"));
    check("bath probe uses only wave, Gauss, genesis, and Langevin",
          only_terms_enabled(a, {"wave_propagation", "gauss_projection",
                                 "genesis", "langevin"}) &&
          a.toggles.langevin_T == 0.03 &&
          a.toggles.langevin_gamma == 0.02 &&
          a.toggles.langevin_seed == 1);
    check("bath probe starts from the exact empty lattice",
          excitation(a) == 0.0 && manifested_count(a) == 0 &&
          exact_state_and_field(a, b));

    tick_n(a, 100); tick_n(b, 100);
    const double e100 = excitation(a);
    const int n100 = manifested_count(a);
    std::cout << "    T=0.03 bath t=100 excitation=" << e100
              << " manifested=" << n100 << '/' << (L * L * L) << '\n';
    check("fixed-seed Langevin/genesis response replays bit-exactly",
          exact_state_and_field(a, b));
    check("the imposed bath produces a finite nonzero native response",
          std::isfinite(e100) && e100 > 0.0 &&
          n100 >= 0 && n100 <= L * L * L);
}

void test_longitudinal_packet_overlap_is_linear() {
    constexpr int L = 48;
    constexpr int ticks = 20;
    constexpr double mid_f = (L - 1) * 0.5;
    constexpr int mc = L / 2;

    ftd::RenderBridge pair(L), left(L), right(L);
    pair.force_cpu(); left.force_cpu(); right.force_cpu();
    check("longitudinal two-packet profile dispatched",
          ftd::dispatch_scenario(pair, "s0-field-sound-collision"));
    check("longitudinal packet pair isolates the periodic wave map",
          only_terms_enabled(pair, {"wave_propagation"}) &&
          pair.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    left.toggles = pair.toggles;
    right.toggles = pair.toggles;

    // Reconstruct the two declared lanes independently. This is an exact
    // source decomposition of the frozen initial condition, not a fitted
    // separation of the evolved field.
    const double proxy_speed = ftd::C_SPEED / 8.0;
    const double sigma = std::max(1.15, L * 0.11);
    const double pulse_sigma = std::max(1.5, L * 0.15 * 0.5);
    const int mode_n = 4;
    const double k = 2.0 * ftd::PI * mode_n / L;
    const double seed_omega = 2.0 * proxy_speed * std::abs(std::sin(k / 2.0));
    const double cut = sigma * 2.4;
    const double cut2 = cut * cut;
    struct Lane { double offset; double direction; ftd::RenderBridge* bridge; };
    const Lane lanes[] = {{-0.25, +1.0, &left}, {+0.25, -1.0, &right}};
    for (const auto& lane : lanes) {
        const double center_x = mid_f + lane.offset * L;
        const int zlo = std::max(0, static_cast<int>(std::floor(mc - cut)));
        const int zhi = std::min(L - 1, static_cast<int>(std::ceil(mc + cut)));
        const int ylo = zlo, yhi = zhi;
        for (int z = zlo; z <= zhi; ++z)
        for (int y = ylo; y <= yhi; ++y)
        for (int x = 0; x < L; ++x) {
            const double dy = y - mc, dz = z - mc;
            const double r2 = dy*dy + dz*dz;
            if (r2 > cut2) continue;
            const double dx = x - center_x;
            const double gx = std::exp(-(dx*dx) / (2.0*pulse_sigma*pulse_sigma));
            const double g = gx * std::exp(-r2 / (2.0*sigma*sigma));
            if (g < 1e-4) continue;
            const double phase = k * x;
            const double j = 0.030 * g * std::sin(phase);
            const double w = lane.direction *
                (-seed_omega * 0.030 * g * std::cos(phase));
            if (std::fabs(j) > 1e-12)
                lane.bridge->inject_flux_add(x, y, z, {j, 0, 0});
            if (std::fabs(w) > 1e-12)
                lane.bridge->inject_wave_vel_add(x, y, z, {w, 0, 0});
        }
    }

    auto relative_superposition_error = [](const ftd::RenderBridge& sum,
                                            const ftd::RenderBridge& a,
                                            const ftd::RenderBridge& b) {
        double residual2 = 0.0, norm2 = 0.0;
        for (std::size_t i = 0; i < sum.voxels().size(); ++i) {
            const ftd::Vec3 dj = sum.voxels()[i].flux
                               - a.voxels()[i].flux - b.voxels()[i].flux;
            const ftd::Vec3 dw = sum.voxels()[i].wave_vel
                               - a.voxels()[i].wave_vel - b.voxels()[i].wave_vel;
            residual2 += dj.mag2() + dw.mag2();
            norm2 += sum.voxels()[i].flux.mag2() + sum.voxels()[i].wave_vel.mag2();
        }
        return std::sqrt(residual2 / std::max(1e-30, norm2));
    };
    check("declared lane decomposition reconstructs the initial field",
          relative_superposition_error(pair, left, right) < 1e-12);

    tick_n(pair, ticks); tick_n(left, ticks); tick_n(right, ticks);
    const double residual = relative_superposition_error(pair, left, right);
    double overlap = 0.0, left_norm = 0.0, right_norm = 0.0;
    for (std::size_t i = 0; i < pair.voxels().size(); ++i) {
        const double el = left.voxels()[i].flux.mag2() + left.voxels()[i].wave_vel.mag2();
        const double er = right.voxels()[i].flux.mag2() + right.voxels()[i].wave_vel.mag2();
        overlap += std::sqrt(el * er);
        left_norm += el;
        right_norm += er;
    }
    const double normalized_overlap = overlap /
        std::sqrt(std::max(1e-30, left_norm * right_norm));
    std::cout << "    longitudinal overlap@20=" << normalized_overlap
              << " superposition_residual=" << residual << '\n';
    check("counter-seeded packets have substantial spatial overlap",
          normalized_overlap > 0.10);
    check("packet overlap creates no nonlinear collision residual",
          residual < 1e-12 && manifested_count(pair) == 0);
}

void test_quantum_named_two_source_field_is_classical() {
    constexpr int L = 48;
    constexpr int ticks = 20;
    constexpr int mid = L / 2;
    constexpr int screen_x = L / 2;
    ftd::RenderBridge pair(L), lower(L), upper(L);
    pair.force_cpu(); lower.force_cpu(); upper.force_cpu();
    check("quantum-named two-source field dispatched",
          ftd::dispatch_scenario(pair, "quantum-double-slit"));
    check("quantum-named two-source field isolates the wave map",
          only_terms_enabled(pair, {"wave_propagation", "gauss_projection"}));
    lower.toggles = pair.toggles;
    upper.toggles = pair.toggles;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const auto& src = pair.voxels()[static_cast<std::size_t>(
            pair.lattice().index(x, y, z))];
        const double lower_share = y < mid ? 1.0 : (y == mid ? 0.5 : 0.0);
        const double upper_share = 1.0 - lower_share;
        if (lower_share > 0.0) {
            lower.inject_flux_add(x, y, z, src.flux * lower_share);
            lower.inject_wave_vel_add(x, y, z, src.wave_vel * lower_share);
        }
        if (upper_share > 0.0) {
            upper.inject_flux_add(x, y, z, src.flux * upper_share);
            upper.inject_wave_vel_add(x, y, z, src.wave_vel * upper_share);
        }
    }
    tick_n(pair, ticks); tick_n(lower, ticks); tick_n(upper, ticks);
    double residual2 = 0.0, norm2 = 0.0;
    for (std::size_t i = 0; i < pair.voxels().size(); ++i) {
        const ftd::Vec3 dj = pair.voxels()[i].flux
                           - lower.voxels()[i].flux - upper.voxels()[i].flux;
        const ftd::Vec3 dw = pair.voxels()[i].wave_vel
                           - lower.voxels()[i].wave_vel - upper.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += pair.voxels()[i].flux.mag2() + pair.voxels()[i].wave_vel.mag2();
    }
    const double residual = std::sqrt(residual2 / std::max(1e-30, norm2));
    double cross_max = 0.0, cross_min = 0.0, incoherent_peak = 0.0;
    for (int y = 0; y < L; ++y) {
        double cross = 0.0, incoherent = 0.0;
        for (int z = 0; z < L; ++z) {
            const std::size_t i = static_cast<std::size_t>(
                pair.lattice().index(screen_x, y, z));
            const double a = lower.voxels()[i].flux.z;
            const double b = upper.voxels()[i].flux.z;
            cross += 2.0 * a * b;
            incoherent += a*a + b*b;
        }
        cross_max = std::max(cross_max, cross);
        cross_min = std::min(cross_min, cross);
        incoherent_peak = std::max(incoherent_peak, incoherent);
    }
    const double constructive = cross_max / std::max(1e-30, incoherent_peak);
    const double destructive = -cross_min / std::max(1e-30, incoherent_peak);
    std::cout << "    quantum-two-source residual=" << residual
              << " constructive=" << constructive
              << " destructive=" << destructive << '\n';
    check("quantum-named field is exact classical linear superposition",
          residual < 1e-12);
    check("quantum-named field has constructive overlap but no destructive screen band",
          constructive > 0.10 && destructive < 1e-12 && manifested_count(pair) == 0);
}

void test_quantum_named_genesis_controls() {
    auto exact_replay = [](const ftd::RenderBridge& a, const ftd::RenderBridge& b) {
        if (a.voxels().size() != b.voxels().size()) return false;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            if (x.state != y.state || x.pair_id != y.pair_id ||
                x.flux.x != y.flux.x || x.flux.y != y.flux.y ||
                x.flux.z != y.flux.z || x.wave_vel.x != y.wave_vel.x ||
                x.wave_vel.y != y.wave_vel.y || x.wave_vel.z != y.wave_vel.z)
                return false;
        }
        return true;
    };
    struct Case { const char* id; int expected_count; };
    const Case cases[] = {
        {"quantum-born-rule", 36},
        {"quantum-zeno", 491},
    };
    for (const auto& c : cases) {
        ftd::RenderBridge a(32), b(32);
        a.force_cpu(); b.force_cpu();
        check(std::string(c.id) + " dispatched twice",
              ftd::dispatch_scenario(a, c.id) && ftd::dispatch_scenario(b, c.id));
        check(std::string(c.id) + " isolates genesis only",
              only_terms_enabled(a, {"genesis"}) && manifested_count(a) == 0);
        check(std::string(c.id) + " fixed envelope replays exactly",
              exact_replay(a, b));
        a.tick(); b.tick();
        const int count = manifested_count(a);
        int paired = 0;
        for (const auto& v : a.voxels())
            if (v.state != 0 && v.pair_id >= 0) ++paired;
        std::cout << "    " << c.id << " one-tick manifested=" << count
                  << " paired=" << paired << '\n';
        check(std::string(c.id) + " one-tick outcome replays exactly",
              exact_replay(a, b));
        check(std::string(c.id) + " has the declared threshold response",
              count == c.expected_count && paired == 0);
    }
}

void test_quantum_well_markers_do_not_confine() {
    constexpr int L = 32;
    constexpr int wall_a = L / 4;
    constexpr int wall_b = 3 * L / 4;
    ftd::RenderBridge marked(L), no_markers(L);
    marked.force_cpu(); no_markers.force_cpu();
    check("quantum-well marker profile dispatched",
          ftd::dispatch_scenario(marked, "quantum-well"));
    check("well profile isolates the unprojected wave map",
          only_terms_enabled(marked, {"wave_propagation"}) &&
          marked.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    no_markers.toggles = marked.toggles;
    int state_count = 0, locked_count = 0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& v = marked.voxel_at(x, y, z);
        if (v.state != 0) ++state_count;
        if (v.locked) ++locked_count;
        if (v.flux.mag2() > 0.0) no_markers.inject_flux_add(x, y, z, v.flux);
        if (v.wave_vel.mag2() > 0.0)
            no_markers.inject_wave_vel_add(x, y, z, v.wave_vel);
    }
    check("well contains exactly two locked marker planes",
          state_count == 2 * L * L && locked_count == state_count);

    tick_n(marked, 8); tick_n(no_markers, 8);
    double residual2 = 0.0, norm2 = 0.0, outside = 0.0, total = 0.0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const std::size_t i = static_cast<std::size_t>(marked.lattice().index(x, y, z));
        const ftd::Vec3 dj = marked.voxels()[i].flux - no_markers.voxels()[i].flux;
        const ftd::Vec3 dw = marked.voxels()[i].wave_vel - no_markers.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        const double e = marked.voxels()[i].flux.mag2() + marked.voxels()[i].wave_vel.mag2();
        total += e;
        if (x <= wall_a || x >= wall_b) outside += e;
    }
    const double marker_effect = std::sqrt(residual2 / std::max(1e-30, norm2 + total));
    const double outside_fraction = outside / std::max(1e-30, total);
    std::cout << "    well marker_effect=" << marker_effect
              << " outside_fraction@8=" << outside_fraction << '\n';
    check("removing marker planes leaves wave evolution bit-identical",
          marker_effect == 0.0);
    check("imposed harmonics propagate outside the marker interval",
          outside_fraction > 0.0 && manifested_count(marked) == 2 * L * L);
}

void test_aharonov_bohm_named_geometry_has_no_phase_mechanism() {
    constexpr int L = 48;
    constexpr int center = L / 2;
    constexpr int radius = L / 8;
    ftd::RenderBridge rb(L), tube(L), paths(L);
    rb.force_cpu(); tube.force_cpu(); paths.force_cpu();
    check("Aharonov-Bohm-named geometry dispatched",
          ftd::dispatch_scenario(rb, "quantum-aharonov-bohm"));
    check("Aharonov-Bohm-named geometry isolates the projected wave map",
          only_terms_enabled(rb, {"wave_propagation", "gauss_projection"}) &&
          manifested_count(rb) == 0);
    tube.toggles = rb.toggles;
    paths.toggles = rb.toggles;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        ftd::Vec3 tube_j;
        const int dx = x - center, dy = y - center;
        if (dx*dx + dy*dy <= radius*radius)
            tube_j.z = ftd::K_B * 0.5;
        if (tube_j.mag2() > 0.0) tube.inject_flux_add(x, y, z, tube_j);
        const auto& full = rb.voxels()[static_cast<std::size_t>(
            rb.lattice().index(x, y, z))];
        const ftd::Vec3 path_j = full.flux - tube_j;
        if (path_j.mag2() > 0.0) paths.inject_flux_add(x, y, z, path_j);
        if (full.wave_vel.mag2() > 0.0)
            paths.inject_wave_vel_add(x, y, z, full.wave_vel);
    }
    auto superposition_error = [](const ftd::RenderBridge& full,
                                  const ftd::RenderBridge& a,
                                  const ftd::RenderBridge& b) {
        double residual2 = 0.0, norm2 = 0.0;
        for (std::size_t i = 0; i < full.voxels().size(); ++i) {
            const ftd::Vec3 dj = full.voxels()[i].flux
                               - a.voxels()[i].flux - b.voxels()[i].flux;
            const ftd::Vec3 dw = full.voxels()[i].wave_vel
                               - a.voxels()[i].wave_vel - b.voxels()[i].wave_vel;
            residual2 += dj.mag2() + dw.mag2();
            norm2 += full.voxels()[i].flux.mag2() + full.voxels()[i].wave_vel.mag2();
        }
        return std::sqrt(residual2 / std::max(1e-30, norm2));
    };
    const double initial_residual = superposition_error(rb, tube, paths);
    tick_n(rb, 12); tick_n(tube, 12); tick_n(paths, 12);
    const double evolved_residual = superposition_error(rb, tube, paths);
    std::cout << "    AB-geometry initial_residual=" << initial_residual
              << " evolved_residual=" << evolved_residual
              << " divergence=" << normalized_divergence(rb) << '\n';
    check("central tube plus path packets reconstruct the initial field",
          initial_residual < 1e-12);
    check("tube and path fields only superpose without an interaction residual",
          evolved_residual < 1e-12 && normalized_divergence(rb) < 1e-12 &&
          manifested_count(rb) == 0);
}

void test_casimir_named_plates_are_wave_transparent() {
    constexpr int L = 32;
    ftd::RenderBridge plates(L), no_plates(L);
    plates.force_cpu(); no_plates.force_cpu();
    check("Casimir-named plate control dispatched",
          ftd::dispatch_scenario(plates, "quantum-casimir"));
    check("plate control isolates the unprojected periodic wave map",
          only_terms_enabled(plates, {"wave_propagation"}) &&
          plates.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    no_plates.toggles = plates.toggles;
    int states = 0, locked = 0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& v = plates.voxel_at(x, y, z);
        if (v.state != 0) ++states;
        if (v.locked) ++locked;
        if (v.flux.mag2() > 0.0) no_plates.inject_flux_add(x, y, z, v.flux);
        if (v.wave_vel.mag2() > 0.0)
            no_plates.inject_wave_vel_add(x, y, z, v.wave_vel);
    }
    check("plate control contains exactly two locked marker planes",
          states == 2 * L * L && locked == states);
    tick_n(plates, 12); tick_n(no_plates, 12);
    double residual2 = 0.0, norm2 = 0.0;
    for (std::size_t i = 0; i < plates.voxels().size(); ++i) {
        const ftd::Vec3 dj = plates.voxels()[i].flux - no_plates.voxels()[i].flux;
        const ftd::Vec3 dw = plates.voxels()[i].wave_vel - no_plates.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += plates.voxels()[i].flux.mag2() + plates.voxels()[i].wave_vel.mag2();
    }
    const double plate_effect = std::sqrt(residual2 / std::max(1e-30, norm2));
    std::cout << "    Casimir plate_effect@12=" << plate_effect << '\n';
    check("removing both plates leaves the wave bit-identical",
          plate_effect == 0.0);
    check("locked plates remain static without acquiring dynamics",
          manifested_count(plates) == 2 * L * L);
}

void test_locked_state_wall_native_transmission() {
    constexpr int L = 32;
    constexpr int wall_x = L / 2;
    constexpr int wall_width = 3;
    constexpr int ticks = 28;
    ftd::RenderBridge wall(L), control(L);
    wall.force_cpu(); control.force_cpu();
    check("locked state-wall transmission profile dispatched",
          ftd::dispatch_scenario(wall, "quantum-tunnel"));
    check("state-wall profile isolates wave, Gauss, and coupling",
          only_terms_enabled(wall, {"wave_propagation", "gauss_projection", "coupling"}) &&
          wall.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    control.toggles = wall.toggles;
    int states = 0, locked = 0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& v = wall.voxel_at(x, y, z);
        if (v.state != 0) ++states;
        if (v.locked) ++locked;
        if (v.flux.mag2() > 0.0) control.inject_flux_add(x, y, z, v.flux);
        if (v.wave_vel.mag2() > 0.0)
            control.inject_wave_vel_add(x, y, z, v.wave_vel);
    }
    check("state wall is exactly three locked full planes",
          states == wall_width * L * L && locked == states);
    tick_n(wall, ticks); tick_n(control, ticks);
    auto right_energy = [](const ftd::RenderBridge& rb) {
        const int n = rb.lattice().size();
        double e = 0.0;
        for (int z = 0; z < n; ++z)
        for (int y = 0; y < n; ++y)
        for (int x = n / 2 + 3; x < n; ++x) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))];
            e += v.flux.mag2() + v.wave_vel.mag2();
        }
        return e;
    };
    const double e_wall = right_energy(wall);
    const double e_control = right_energy(control);
    const double ratio = e_wall / std::max(1e-30, e_control);
    double residual2 = 0.0, norm2 = 0.0;
    for (std::size_t i = 0; i < wall.voxels().size(); ++i) {
        const ftd::Vec3 dj = wall.voxels()[i].flux - control.voxels()[i].flux;
        const ftd::Vec3 dw = wall.voxels()[i].wave_vel - control.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += control.voxels()[i].flux.mag2() + control.voxels()[i].wave_vel.mag2();
    }
    const double field_difference = std::sqrt(residual2 / std::max(1e-30, norm2));
    std::cout << "    state-wall transmitted_ratio@28=" << ratio
              << " field_difference=" << field_difference
              << " Ewall=" << e_wall << " Econtrol=" << e_control << '\n';
    check("locked wall acts as a large native coupling source rather than a barrier",
          std::isfinite(ratio) && std::isfinite(field_difference) &&
          ratio > 1000.0 && field_difference > 1.0);
    check("state-wall and control retain finite transmitted field",
          e_wall > 0.0 && e_control > 0.0 && manifested_count(wall) == states);
}

void test_eraser_named_grid_is_a_coupling_source() {
    constexpr int L = 32;
    constexpr int ticks = 28;
    ftd::RenderBridge grid(L), control(L);
    grid.force_cpu(); control.force_cpu();
    check("eraser-named checkerboard profile dispatched",
          ftd::dispatch_scenario(grid, "quantum-eraser"));
    check("checkerboard profile isolates wave, Gauss, and coupling",
          only_terms_enabled(grid, {"wave_propagation", "gauss_projection", "coupling"}) &&
          grid.toggles.flux_boundary == ftd::FluxBoundaryMode::Periodic);
    control.toggles = grid.toggles;
    int states = 0, locked = 0;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& v = grid.voxel_at(x, y, z);
        if (v.state != 0) ++states;
        if (v.locked) ++locked;
        if (v.flux.mag2() > 0.0) control.inject_flux_add(x, y, z, v.flux);
        if (v.wave_vel.mag2() > 0.0)
            control.inject_wave_vel_add(x, y, z, v.wave_vel);
    }
    check("eraser-named grid is exactly one locked checkerboard plane",
          states == L * L / 2 && locked == states);
    tick_n(grid, ticks); tick_n(control, ticks);
    auto downstream_energy = [](const ftd::RenderBridge& rb) {
        const int n = rb.lattice().size();
        double e = 0.0;
        for (int z = 0; z < n; ++z)
        for (int y = 0; y < n; ++y)
        for (int x = n / 2 + 1; x < n; ++x) {
            const auto& v = rb.voxels()[static_cast<std::size_t>(
                rb.lattice().index(x, y, z))];
            e += v.flux.mag2() + v.wave_vel.mag2();
        }
        return e;
    };
    const double e_grid = downstream_energy(grid);
    const double e_control = downstream_energy(control);
    const double ratio = e_grid / std::max(1e-30, e_control);
    double residual2 = 0.0, norm2 = 0.0;
    for (std::size_t i = 0; i < grid.voxels().size(); ++i) {
        const ftd::Vec3 dj = grid.voxels()[i].flux - control.voxels()[i].flux;
        const ftd::Vec3 dw = grid.voxels()[i].wave_vel - control.voxels()[i].wave_vel;
        residual2 += dj.mag2() + dw.mag2();
        norm2 += control.voxels()[i].flux.mag2() + control.voxels()[i].wave_vel.mag2();
    }
    const double difference = std::sqrt(residual2 / std::max(1e-30, norm2));
    std::cout << "    eraser-grid downstream_ratio@28=" << ratio
              << " field_difference=" << difference
              << " Egrid=" << e_grid << " Econtrol=" << e_control << '\n';
    check("checkerboard states act as a strong native coupling source",
          std::isfinite(ratio) && std::isfinite(difference) &&
          ratio > 10.0 && difference > 4.0);
    check("checkerboard response is not a measurement or erasure operation",
          e_grid > 0.0 && e_control > 0.0 && manifested_count(grid) == states);
}

void test_emergent_genesis_profile_matrix() {
    auto exact_replay = [](const ftd::RenderBridge& a, const ftd::RenderBridge& b) {
        if (a.voxels().size() != b.voxels().size()) return false;
        for (std::size_t i = 0; i < a.voxels().size(); ++i) {
            const auto& x = a.voxels()[i];
            const auto& y = b.voxels()[i];
            if (x.state != y.state || x.particle_id != y.particle_id ||
                x.pair_id != y.pair_id || x.flux.x != y.flux.x ||
                x.flux.y != y.flux.y || x.flux.z != y.flux.z ||
                x.wave_vel.x != y.wave_vel.x ||
                x.wave_vel.y != y.wave_vel.y ||
                x.wave_vel.z != y.wave_vel.z) return false;
        }
        return true;
    };
    struct Case {
        const char* id;
        double temperature;
        int expected_100;
        int expected_120;
    };
    const Case cases[] = {
        {"s0-seed-emergent-ic1", 0.005, 3, 3},
        {"s0-seed-emergent-ic3-collision", 0.005, 2, 2},
        {"s0-seed-emergent-ic4-subthreshold", 0.005, 0, 0},
        {"s0-seed-emergent-ic2-thermal-runaway", 0.05, 0, 0},
        {"s0-seed-emergent-ic1-diagonal", 0.005, 1, 1},
        {"s0-seed-emergent-ic1-isotropic", 0.005, 8, 8},
        {"s0-seed-emergent-ic1-viz", 0.0, 22, 20},
        {"s0-seed-emergent-ic1-diagonal-viz", 0.0, 22, 20},
        {"s0-seed-emergent-ic1-isotropic-viz", 0.0, 20, 18},
        {"s0-seed-cluster-law", 0.005, 3, 3},
    };
    for (const auto& c : cases) {
        ftd::RenderBridge a(24), b(24);
        a.force_cpu(); b.force_cpu();
        check(std::string(c.id) + " dispatched twice",
              ftd::dispatch_scenario(a, c.id) && ftd::dispatch_scenario(b, c.id));
        check(std::string(c.id) + " uses the isolated genesis-response stack",
              only_terms_enabled(a, {"wave_propagation", "gauss_projection",
                                     "genesis", "langevin"}) &&
              a.toggles.langevin_T == c.temperature &&
              a.toggles.langevin_gamma == 0.02);
        check(std::string(c.id) + " initial data replays exactly", exact_replay(a, b));
        tick_n(a, 100); tick_n(b, 100);
        const int n100 = manifested_count(a);
        tick_n(a, 20); tick_n(b, 20);
        const int n120 = manifested_count(a);
        std::cout << "    " << c.id << " count@100/120="
                  << n100 << '/' << n120 << '\n';
        check(std::string(c.id) + " 120-tick history replays exactly",
              exact_replay(a, b));
        check(std::string(c.id) + " matches the qualified finite-volume response",
              n100 == c.expected_100 && n120 == c.expected_120);
    }
}

}  // namespace

int main() {
    std::cout << "=== Scale-0 scenario behavior regression ===\n";
    test_empty_baseline_stays_empty();
    test_vacuum_photon_translates();
    test_photon_race_common_speed();
    test_rainbow_modes_are_transverse();
    test_exact_traveling_harmonic();
    test_gravity_named_wave_aliases_are_plain_native_modes();
    test_gravity_named_radial_ansatz_and_optical_null();
    test_exact_standing_harmonic();
    test_two_coherent_source_qualification_gate();
    test_neutral_candidate_is_dynamic();
    test_imposed_kg_block_clock_integration();
    test_particle_named_wave_template_cohorts();
    test_tagged_pair_bookkeeping();
    test_composite_candidates_are_unlocked();
    test_unlocked_composite_candidate_outcomes();
    test_long_baseline_opposite_polarity_collision();
    test_prepared_coulomb_candidate_outcomes();
    test_uniform_additive_genesis_drive_response();
    test_prepared_weak_transmutation_cohort();
    test_fixed_seed_thermal_transport_cohort();
    test_patterned_genesis_response_cohort();
    test_moore_geometry_seed_contracts();
    test_cluster_amplitude_ordering();
    test_locked_mass_latency_probe();
    test_periodic_random_wave_bath();
    test_high_amplitude_packet_dispersion();
    test_field_photon_packet_qualification_gate();
    test_bidirectional_transverse_lobes();
    test_uniform_field_initial_data();
    test_reference_geometry_ansatze();
    test_gauge_named_initial_data_without_gauge_claims();
    test_selected_field_ansatze();
    test_symmetric_wave_pair_profiles();
    test_wave_family_native_dispersion();
    test_native_point_response_cone();
    test_multilobe_wave_symmetries();
    test_imposed_uniform_b_native_curvature();
    test_prepared_polarity_geometry_seeds();
    test_deterministic_random_wave_profiles();
    test_fixed_seed_genesis_response_profiles();
    test_multistate_free_transport_controls();
    test_fixed_temperature_langevin_genesis_probe();
    test_longitudinal_packet_overlap_is_linear();
    test_quantum_named_two_source_field_is_classical();
    test_quantum_named_genesis_controls();
    test_quantum_well_markers_do_not_confine();
    test_aharonov_bohm_named_geometry_has_no_phase_mechanism();
    test_casimir_named_plates_are_wave_transparent();
    test_locked_state_wall_native_transmission();
    test_eraser_named_grid_is_a_coupling_source();
    test_emergent_genesis_profile_matrix();
    test_remaining_research_setup_probe_matrix();
    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
