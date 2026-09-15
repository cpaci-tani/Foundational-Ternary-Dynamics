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

// T4: integrators that change the wave step report Unavailable with the reason.
void unavailable() {
    for (const std::string toggle : {"symplectic_leapfrog", "verlet_wave_integrator"}) {
        ftd::RenderBridge rb(17); rb.force_cpu();
        ftd::dispatch_scenario(rb, "flux-pulse");
        if (toggle == "symplectic_leapfrog") rb.toggles.symplectic_leapfrog = true;
        else rb.toggles.verlet_wave_integrator = true;
        rb.set_link_energy_observation(true);
        rb.tick(); rb.tick();
        const auto& obs = rb.link_energy_observer();
        check(toggle + ": status Unavailable", obs.status() == LinkEnergyStatus::Unavailable);
        check(toggle + ": reason names the toggle", obs.reason().find(toggle) != std::string::npos);
    }
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
}  // namespace

int main() {
    std::printf("link_energy_observer\n");
    pure_wave("flux-pulse");
    pure_wave("flux-dipole");
    independent_current();
    unavailable();
    state_neutral();
    std::printf("%s (%d failures)\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
