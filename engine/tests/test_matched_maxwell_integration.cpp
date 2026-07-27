/**
 * Production-tick integration gates for the FTD-0428 selected branch.
 */

#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int failures = 0;

void check(const std::string& label, bool pass) {
    std::cout << (pass ? "  PASS  " : "  FAIL  ") << label << '\n';
    if (!pass) ++failures;
}

void configure(ftd::RenderBridge& bridge, bool movement) {
    bridge.force_cpu();
    bridge.toggles.disable_all();
    bridge.toggles.movement = movement;
    bridge.toggles.matched_gauss_dynamics = true;
    bridge.toggles.strict_validation = true;
}

}  // namespace

int main() {
    {
        ftd::TermToggles toggles;
        check("matched branch defaults off", !toggles.matched_gauss_dynamics);
        toggles.matched_gauss_dynamics = true;
        std::string error;
        check("matched branch rejects legacy default writers",
              !toggles.validate(&error) &&
              error.find("isolated conservative movement sector") !=
                  std::string::npos);
    }

    {
        ftd::RenderBridge bridge(8);
        bridge.toggles.matched_gauss_dynamics = true;
        bool threw = false;
        try {
            bridge.tick();
        } catch (const std::logic_error&) {
            threw = true;
        }
        check("matched isolation fails closed without strict flag", threw);
    }

    constexpr int L = 16;
    {
        ftd::RenderBridge bridge(L);
        configure(bridge, false);
        bool threw = false;
        try {
            bridge.tick();
        } catch (const std::logic_error&) {
            threw = true;
        }
        check("tick fails closed before explicit initialization", threw);
    }

    {
        ftd::RenderBridge bridge(L);
        configure(bridge, false);
        check("unit-tick guard setup", bridge.initialize_matched_gauss_dynamics().valid);
        bridge.set_dt(2.0);
        bool threw = false;
        try {
            bridge.tick();
        } catch (const std::logic_error&) {
            threw = true;
        }
        check("matched branch rejects unlocked time step", threw);
    }

    {
        ftd::RenderBridge bridge(L);
        configure(bridge, false);
        bridge.set_state(3, 5, 7, +1);
        bridge.set_state(12, 10, 9, -1);
        const auto initialized = bridge.initialize_matched_gauss_dynamics();
        check("production bridge minimum-energy initialization",
              initialized.valid && initialized.converged &&
              initialized.iterations <= 12 * L);
        check("production bridge dressing is longitudinal",
              initialized.curl_adjoint_residual <= 1e-10);
        check("initial voxel mirror is exact",
              bridge.matched_gauss_voxel_sync_residual() <= 1e-15);

        const double energy = bridge.matched_gauss_state().modified_energy(
            ftd::C_SPEED, bridge.dt());
        double max_drift = 0.0;
        bool valid = true;
        for (int tick = 0; tick < 8; ++tick) {
            bridge.tick();
            const auto& step = bridge.matched_gauss_state().last_step();
            valid = valid && step.valid;
            max_drift = std::max(max_drift, std::abs(step.energy_after - energy));
        }
        check("static dressed pair survives production ticks", valid);
        check("static production energy is invariant",
              max_drift <= 1e-12 * std::max(1.0, std::abs(energy)));
        check("static production Gauss remains exact",
              bridge.matched_gauss_state().last_step().gauss_residual <= 1e-10);
        check("tick-end voxel mirror is exact",
              bridge.matched_gauss_voxel_sync_residual() <= 1e-15);
    }

    {
        ftd::RenderBridge bridge(L);
        configure(bridge, true);
        bridge.inject_particle(4, 5, 6, +1, {});
        bridge.inject_particle(12, 10, 9, -1, {});
        auto& mobile = bridge.voxel_at(4, 5, 6);
        const double speed = 0.99 * ftd::C_SPEED;
        mobile.velocity = {speed, 0.0, 0.0};
        mobile.remainder = {1.0 - speed, 0.0, 0.0};
        check("moving-pair initialization",
              bridge.initialize_matched_gauss_dynamics().valid);
        bridge.tick();
        const auto& step = bridge.matched_gauss_state().last_step();
        check("production movement transports polarity",
              bridge.state_at(5, 5, 6) == +1 && bridge.state_at(4, 5, 6) == 0);
        check("production movement yields routed face current",
              step.valid && step.transport.current_l1 > 0.0 &&
              step.transport.reaction_l1 == 0);
        check("production movement preserves Gauss without projection",
              step.gauss_residual <= 1e-10);
        check("moving tick voxel mirror is exact",
              bridge.matched_gauss_voxel_sync_residual() <= 1e-15);
    }

    {
        ftd::RenderBridge bridge(L);
        configure(bridge, false);
        check("vacuum branch initialization",
              bridge.initialize_matched_gauss_dynamics().valid);
        check("production transverse impulse injection",
              bridge.inject_matched_transverse_edge_potential(
                  4, 5, 6, 2, 1e-3));
        const double energy = bridge.matched_gauss_state().modified_energy(
            ftd::C_SPEED, bridge.dt());
        double max_drift = 0.0;
        bool valid = true;
        for (int tick = 0; tick < 32; ++tick) {
            bridge.tick();
            const auto& step = bridge.matched_gauss_state().last_step();
            valid = valid && step.valid;
            max_drift = std::max(max_drift, std::abs(step.energy_after - energy));
        }
        check("production transverse wave remains valid", valid);
        check("production transverse wave conserves modified energy",
              max_drift <= 1e-12 * std::max(1.0, std::abs(energy)));
        check("production transverse wave remains source-free",
              bridge.matched_gauss_state().last_step().gauss_residual <= 1e-12);
    }

    std::cout << "matched_maxwell_integration failures=" << failures << '\n';
    return failures == 0 ? 0 : 1;
}
