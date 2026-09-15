// engine/tests/test_link_energy_observer.cpp
// Exact link energy current of the reference engine's wave step (spec 2026-09-15
// native transport overlays, section 9 T1, T3, T4, T5). Measurements of the
// engine's own v1 wave map; no physics identification is made here.
#include "ftd/constants.h"
#include "ftd/link_energy_observer.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

namespace {
int g_fail = 0;
void check(const std::string& n, bool ok) { std::printf("  [%s] %s\n", ok ? "PASS" : "FAIL", n.c_str()); if (!ok) ++g_fail; }
using ftd::LinkEnergyStatus;

std::vector<double> flux_of(ftd::RenderBridge& rb) {
    const int L = rb.lattice().size();
    std::vector<double> u(3 * static_cast<std::size_t>(L) * L * L);
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) {
        const std::size_t i = static_cast<std::size_t>((x * L + y) * L + z);
        const auto f = rb.voxel_at(x, y, z).flux;
        u[3 * i] = f.x; u[3 * i + 1] = f.y; u[3 * i + 2] = f.z;
    }
    return u;
}

// T1: pure wave step closes locally to round-off and conserves the invariant.
void pure_wave(const char* scen) {
    ftd::RenderBridge rb(33); rb.force_cpu();
    check(std::string(scen) + ": scenario loads", ftd::dispatch_scenario(rb, scen));
    rb.set_link_energy_observation(true);
    const auto& obs = rb.link_energy_observer();
    double H0 = 0, worst_closure = 0, worst_drift = 0; bool all_ok = true;
    for (int t = 1; t <= 200; ++t) {
        rb.tick();
        if (t == 1) { check(std::string(scen) + ": warming after one tick", obs.status() == LinkEnergyStatus::Warming); continue; }
        if (obs.status() != LinkEnergyStatus::Ok) { all_ok = false; break; }
        if (t == 2) H0 = obs.invariant();
        worst_closure = std::max(worst_closure, obs.closure());
        worst_drift = std::max(worst_drift, std::abs(obs.invariant() - H0) / std::abs(H0));
    }
    std::printf("    %s: worst closure %.3e, invariant drift %.3e\n", scen, worst_closure, worst_drift);
    check(std::string(scen) + ": status Ok on every tick after warming", all_ok);
    check(std::string(scen) + ": closure <= 1e-12", all_ok && worst_closure <= 1e-12);
    check(std::string(scen) + ": invariant drift <= 1e-10", all_ok && worst_drift <= 1e-10);
}

// T5: stored links equal an independent evaluation, and F(j->i) = -F(i->j).
void independent_current() {
    const int L = 17;
    ftd::RenderBridge rb(L); rb.force_cpu();
    ftd::dispatch_scenario(rb, "flux-pulse");
    rb.set_link_energy_observation(true);
    rb.tick(); const auto u0 = flux_of(rb);
    rb.tick(); const auto u1 = flux_of(rb);
    rb.tick(); const auto u2 = flux_of(rb);
    const auto& obs = rb.link_energy_observer();
    check("observer Ok after three ticks", obs.status() == LinkEnergyStatus::Ok);
    const double c2 = ftd::C_WAVE * ftd::C_WAVE;
    auto w = [L](int v) { return (v % L + L) % L; };
    double worst = 0, scale = 0;
    for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) for (int k = 0; k < 9; ++k) {
        const auto& d = ftd::LINK_DISPLACEMENT[k];
        const std::size_t i = static_cast<std::size_t>((x * L + y) * L + z);
        const std::size_t j = static_cast<std::size_t>((w(x + d[0]) * L + w(y + d[1])) * L + w(z + d[2]));
        double fij = 0, fji = 0;
        for (int c = 0; c < 3; ++c) {
            fij += (u1[3 * i + c] - u1[3 * j + c]) * ((u2[3 * i + c] - u0[3 * i + c]) + (u2[3 * j + c] - u0[3 * j + c]));
            fji += (u1[3 * j + c] - u1[3 * i + c]) * ((u2[3 * j + c] - u0[3 * j + c]) + (u2[3 * i + c] - u0[3 * i + c]));
        }
        fij *= 0.25 * c2 * ftd::LINK_WEIGHT[k]; fji *= 0.25 * c2 * ftd::LINK_WEIGHT[k];
        const double got = obs.links_exact()[9 * i + k];
        worst = std::max({worst, std::abs(got - fij), std::abs(got + fji)});
        scale = std::max(scale, std::abs(fij));
    }
    std::printf("    independent evaluation: worst difference %.3e of scale %.3e\n", worst, scale);
    check("links match an independent evaluation and F(j->i) = -F(i->j)", scale > 0 && worst <= 1e-14 * scale);
}

// T4: configurations that change or replace the wave step report Unavailable with the reason.
void unavailable() {
    struct Case { const char* toggle; void (*configure)(ftd::RenderBridge&); };
    const Case cases[] = {
        {"symplectic_leapfrog", [](ftd::RenderBridge& rb) { rb.toggles.symplectic_leapfrog = true; }},
        {"verlet_wave_integrator", [](ftd::RenderBridge& rb) { rb.toggles.verlet_wave_integrator = true; }},
        {"lorentz_period2_floquet", [](ftd::RenderBridge& rb) { rb.toggles.lorentz_period2_floquet = true; }},
        {"lorentz_bcc_time_floquet", [](ftd::RenderBridge& rb) { rb.toggles.lorentz_bcc_time_floquet = true; }},
        {"wave_propagation", [](ftd::RenderBridge& rb) { rb.toggles.wave_propagation = false; }},
    };
    auto report = [](const std::string& toggle, const ftd::LinkEnergyObserver& obs) {
        check(toggle + ": status Unavailable", obs.status() == LinkEnergyStatus::Unavailable);
        check(toggle + ": reason names the toggle", obs.reason().find(toggle) != std::string::npos);
    };
    for (const auto& cs : cases) {
        ftd::RenderBridge rb(17); rb.force_cpu();
        ftd::dispatch_scenario(rb, "flux-pulse");
        cs.configure(rb);
        rb.set_link_energy_observation(true);
        rb.tick(); rb.tick();
        report(cs.toggle, rb.link_energy_observer());
    }
    // matched_gauss_dynamics fails toggle validation, and tick() throws before the
    // observer runs, unless the isolated sector is configured and explicitly
    // initialized (FTD-0428); this uses test_matched_maxwell_integration's setup.
    ftd::RenderBridge rb(16); rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.movement = false;
    rb.toggles.matched_gauss_dynamics = true;
    rb.toggles.strict_validation = true;
    check("matched_gauss_dynamics: isolated sector initializes", rb.initialize_matched_gauss_dynamics().valid);
    rb.set_link_energy_observation(true);
    rb.tick(); rb.tick();
    report("matched_gauss_dynamics", rb.link_energy_observer());
}

// T3 (state half): engine state is bit-identical with the observer on or off.
void state_neutral() {
    auto run = [](bool observe) {
        ftd::RenderBridge rb(17); rb.force_cpu();
        ftd::dispatch_scenario(rb, "s0-seed-emergent-ic1");
        rb.set_link_energy_observation(observe);
        for (int t = 0; t < 60; ++t) rb.tick();
        std::vector<double> out;
        for (const auto& v : rb.voxels()) {
            out.push_back(v.flux.x); out.push_back(v.flux.y); out.push_back(v.flux.z);
            out.push_back(v.wave_vel.x); out.push_back(v.wave_vel.y); out.push_back(v.wave_vel.z);
            out.push_back(static_cast<double>(v.state));
        }
        return out;
    };
    const auto a = run(false), b = run(true);
    check("engine state bit-identical with observer on vs off (ic1, 60 ticks)",
          a.size() == b.size() && std::memcmp(a.data(), b.data(), a.size() * sizeof(double)) == 0);
}
// T2: in the genesis scenario the balance fails to close exactly when a term
// that exchanges energy off the links is on, and closes when all are off.
void exchange_decomposition() {
    struct Case { const char* label; bool langevin, gauss, damping, genesis; };
    const Case cases[] = {
        {"langevin only", true, false, false, false},
        {"gauss_projection only", false, true, false, false},
        {"damping only", false, false, true, false},
        {"langevin, gauss_projection, damping and genesis off", false, false, false, false},
    };
    for (const auto& cs : cases) {
        ftd::RenderBridge rb(33); rb.force_cpu();
        ftd::dispatch_scenario(rb, "s0-seed-emergent-ic1");
        rb.toggles.langevin = cs.langevin;
        rb.toggles.gauss_projection = cs.gauss;
        rb.toggles.damping = cs.damping;
        rb.toggles.genesis = cs.genesis;
        rb.set_link_energy_observation(true);
        for (int t = 0; t < 100; ++t) rb.tick();
        const auto& obs = rb.link_energy_observer();
        std::printf("    %s: closure %.3e, terms 0x%x\n", cs.label, obs.closure(), obs.active_exchange_terms());
        check(std::string(cs.label) + ": status Ok", obs.status() == LinkEnergyStatus::Ok);
        if (cs.langevin || cs.gauss || cs.damping)
            check(std::string(cs.label) + ": balance does not close (closure > 1e-9)", obs.closure() > 1e-9);
        else
            check(std::string(cs.label) + ": balance closes (closure <= 1e-12)", obs.closure() <= 1e-12);
        const auto bits = obs.active_exchange_terms();
        check(std::string(cs.label) + ": exchange-term bits match the toggles",
              ((bits & ftd::LinkExchangeTerm::Langevin) != 0) == cs.langevin
              && ((bits & ftd::LinkExchangeTerm::GaussProjection) != 0) == cs.gauss
              && ((bits & ftd::LinkExchangeTerm::Damping) != 0) == cs.damping
              && ((bits & ftd::LinkExchangeTerm::Genesis) != 0) == cs.genesis);
    }
    ftd::RenderBridge rb(33); rb.force_cpu();
    ftd::dispatch_scenario(rb, "s0-seed-emergent-ic1");
    rb.set_link_energy_observation(true);
    for (int t = 0; t < 100; ++t) rb.tick();
    const auto& obs = rb.link_energy_observer();
    const auto& r = obs.residual_exact();
    std::size_t above = 0;
    for (double v : r) if (std::abs(v) > 0.01 * obs.max_local_change()) ++above;
    const double frac = r.empty() ? 0.0 : static_cast<double>(above) / static_cast<double>(r.size());
    std::printf("    scenario profile: %.1f%% of sites above 1%% of the largest local change\n", 100.0 * frac);
    check("scenario profile with the thermostat: more than half the sites exchange energy off the links", frac > 0.5);
}

// Non-periodic flux boundaries: the law does not join opposite faces, so every
// link whose neighbour leaves [0, L-1] stays exactly zero.
void nonperiodic_boundary() {
    const int L = 17;
    struct Mode { const char* label; ftd::FluxBoundaryMode mode; };
    const Mode modes[] = {
        {"Reflective", ftd::FluxBoundaryMode::Reflective},
        {"Dispersal", ftd::FluxBoundaryMode::Dispersal},
    };
    for (const auto& m : modes) {
        const std::string label = std::string(m.label) + " boundary";
        ftd::RenderBridge rb(L); rb.force_cpu();
        ftd::dispatch_scenario(rb, "flux-pulse");
        rb.toggles.flux_boundary = m.mode;
        rb.set_link_energy_observation(true);
        for (int t = 0; t < 20; ++t) rb.tick();
        const auto& obs = rb.link_energy_observer();
        check(label + ": status Ok", obs.status() == LinkEnergyStatus::Ok);
        check(label + ": NonPeriodicBoundary bit set",
              (obs.active_exchange_terms() & ftd::LinkExchangeTerm::NonPeriodicBoundary) != 0);
        const auto& links = obs.links_exact();
        const bool sized = links.size() == 9 * static_cast<std::size_t>(L) * L * L;
        bool wrap_zero = sized;
        std::size_t wrap_links = 0, inner_nonzero = 0;
        if (sized) for (int x = 0; x < L; ++x) for (int y = 0; y < L; ++y) for (int z = 0; z < L; ++z) for (int k = 0; k < 9; ++k) {
            const auto& d = ftd::LINK_DISPLACEMENT[k];
            const int nx = x + d[0], ny = y + d[1], nz = z + d[2];
            const bool wraps = nx < 0 || nx >= L || ny < 0 || ny >= L || nz < 0 || nz >= L;
            const double v = links[9 * static_cast<std::size_t>((x * L + y) * L + z) + k];
            if (wraps) { ++wrap_links; if (v != 0.0) wrap_zero = false; }
            else if (v != 0.0) ++inner_nonzero;
        }
        std::printf("    %s: %zu wrap links, %zu non-zero non-wrapping links\n", label.c_str(), wrap_links, inner_nonzero);
        check(label + ": every wrap link is exactly 0", sized && wrap_links > 0 && wrap_zero);
        check(label + ": at least one non-wrapping link is non-zero", inner_nonzero > 0);
    }
}

// The de Broglie clock's on-site Klein-Gordon term (phase_read) acts only at
// manifested sites (state != 0) unless db_clock_coulomb is on, and flux-pulse
// manifests none, so the site of largest |flux| is manifested before
// observation starts. omega0 keeps its TermToggles default 1.0; no setter needed.
void clock_term_named() {
    ftd::RenderBridge rb(17); rb.force_cpu();
    ftd::dispatch_scenario(rb, "flux-pulse");
    const auto& vox = rb.voxels();
    std::size_t site = 0;
    for (std::size_t i = 1; i < vox.size(); ++i) if (vox[i].flux.mag2() > vox[site].flux.mag2()) site = i;
    rb.set_state(static_cast<int>(site), +1);
    rb.toggles.de_broglie_clock = true;
    rb.set_link_energy_observation(true);
    for (int t = 0; t < 10; ++t) rb.tick();
    const auto& obs = rb.link_energy_observer();
    std::printf("    de_broglie_clock: closure %.3e, terms 0x%x\n", obs.closure(), obs.active_exchange_terms());
    check("de_broglie_clock: status Ok", obs.status() == LinkEnergyStatus::Ok);
    check("de_broglie_clock: balance does not close (closure > 1e-9)", obs.closure() > 1e-9);
    check("de_broglie_clock: DeBroglieClock bit set",
          (obs.active_exchange_terms() & ftd::LinkExchangeTerm::DeBroglieClock) != 0);
}

// Switching the observer off releases every buffer.
void release_on_disable() {
    ftd::RenderBridge rb(17); rb.force_cpu();
    ftd::dispatch_scenario(rb, "flux-pulse");
    rb.set_link_energy_observation(true);
    for (int t = 0; t < 3; ++t) rb.tick();
    const auto& obs = rb.link_energy_observer();
    check("release: status Ok after three ticks", obs.status() == LinkEnergyStatus::Ok);
    check("release: links non-empty while Ok", !obs.links().empty());
    rb.set_link_energy_observation(false);
    check("release: status Off after disable", obs.status() == LinkEnergyStatus::Off);
    check("release: links capacity 0", obs.links().capacity() == 0);
    check("release: residual capacity 0", obs.residual().capacity() == 0);
    check("release: links_exact capacity 0", obs.links_exact().capacity() == 0);
    check("release: residual_exact capacity 0", obs.residual_exact().capacity() == 0);
}
}  // namespace

int main() {
    std::printf("link_energy_observer\n");
    pure_wave("flux-pulse");
    pure_wave("flux-dipole");
    independent_current();
    unavailable();
    state_neutral();
    exchange_decomposition();
    nonperiodic_boundary();
    clock_term_named();
    release_on_disable();
    std::printf("%s (%d failures)\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
