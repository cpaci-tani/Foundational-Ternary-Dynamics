/**
 * One-tick certification for the selected native genesis hazard.
 *
 * Scope is intentionally narrow. The initial cohorts test the hard threshold
 * and p(J)=1-exp(-(J-K_GENESIS)/K_MANIFEST) before any accepted event drains
 * flux. Later ticks are not independent frozen cohorts: genesis changes J and
 * the master genesis toggle also runs evaporation.
 */

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

#include <array>
#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

bool only_genesis_enabled(const ftd::TermToggles& toggles) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const bool enabled = toggles.*(spec.field);
        if (std::string(spec.name) == "genesis") {
            if (!enabled) return false;
        } else if (std::string(spec.name) == "strict_validation") {
            continue;
        } else if (enabled) {
            return false;
        }
    }
    return !toggles.dual_substrate;
}

int band_for_x(int x, int L) {
    const int x1 = 1 + (L - 2) / 3;
    const int x2 = 1 + 2 * (L - 2) / 3;
    if (x <= 0 || x >= L - 1 || x == x1 || x == x2) return -1;
    return x < x1 ? 0 : x < x2 ? 1 : 2;
}

void test_one_tick_gate() {
    constexpr int L = 24;
    const std::array<double, 3> amp{{1.5160, 1.5250, 1.5340}};
    ftd::RenderBridge rb(L);
    rb.force_cpu();

    // Deliberately contaminate non-bulk controls and typed campaign overrides.
    // Scenario dispatch must remove this caller history.
    rb.toggles.matched_gauss_dynamics = true;
    rb.toggles.lorentz_period2_floquet = true;
    rb.toggles.db_clock_coulomb = true;
    rb.genesis_threshold_override = 999.0;
    rb.manifest_scale_override = 999.0;
    rb.manifest_use_temperature = true;

    check("genesis gate scenario dispatches",
          ftd::dispatch_scenario(rb, "flux-genesis-between-gates"));
    check("scenario clears every non-genesis physics toggle and campaign override",
          only_genesis_enabled(rb.toggles)
          && rb.genesis_threshold_override <= 0.0
          && rb.manifest_scale_override <= 0.0
          && !rb.manifest_use_temperature
          && rb.toggles.langevin_seed == 1);

    std::array<int, 3> cohort{{0, 0, 0}};
    bool exact_initial_data = true;
    for (int x = 0; x < L; ++x)
    for (int y = 0; y < L; ++y)
    for (int z = 0; z < L; ++z) {
        const int band = band_for_x(x, L);
        const auto& v = rb.voxel_at(x, y, z);
        const bool interior_yz = y > 0 && y < L - 1 && z > 0 && z < L - 1;
        if (band >= 0 && interior_yz) {
            ++cohort[band];
            exact_initial_data = exact_initial_data
                && v.flux.x == amp[band] && v.flux.y == 0.0 && v.flux.z == 0.0
                && v.wave_vel.mag2() == 0.0 && v.state == 0;
        } else {
            exact_initial_data = exact_initial_data
                && v.flux.mag2() == 0.0 && v.wave_vel.mag2() == 0.0
                && v.state == 0;
        }
    }
    check("three cohorts and separator planes are initialized exactly",
          exact_initial_data && cohort[0] > 0 && cohort[1] > 0 && cohort[2] > 0);
    check("cohorts straddle the compiled selected threshold",
          amp[0] < ftd::K_GENESIS && ftd::K_GENESIS < amp[1]
          && amp[1] < amp[2]);

    check("read-only event journal enables", rb.enable_history_journal());
    rb.toggles.strict_validation = true;
    rb.tick();

    std::array<int, 3> observed{{0, 0, 0}};
    int off_cohort = 0;
    for (const auto& event : rb.history_events()) {
        if (event.kind != ftd::eft::HistoryEventKind::Genesis) continue;
        const auto c = rb.lattice().coord(event.before[0].index);
        const int band = band_for_x(c.x, L);
        if (band < 0) ++off_cohort;
        else ++observed[band];
    }

    const auto probability = [](double j) {
        return j <= ftd::K_GENESIS
            ? 0.0
            : 1.0 - std::exp(-(j - ftd::K_GENESIS) / ftd::K_MANIFEST);
    };
    const double p_mid = probability(amp[1]);
    const double p_high = probability(amp[2]);
    const auto within_six_sigma = [](int count, int n, double p) {
        const double mean = n * p;
        const double sigma = std::sqrt(n * p * (1.0 - p));
        return std::fabs(count - mean) <= 6.0 * sigma;
    };

    std::cout << "    cohort=" << cohort[0] << '/' << cohort[1] << '/' << cohort[2]
              << " observed=" << observed[0] << '/' << observed[1] << '/' << observed[2]
              << " p=" << probability(amp[0]) << '/' << p_mid << '/' << p_high
              << " hazard_ratio=" << (p_high / p_mid) << '\n';

    check("below-threshold cohort has exactly zero genesis events",
          observed[0] == 0 && off_cohort == 0);
    check("middle cohort follows the declared one-tick Bernoulli hazard",
          observed[1] > 0 && within_six_sigma(observed[1], cohort[1], p_mid));
    check("upper cohort follows the declared one-tick Bernoulli hazard",
          observed[2] > observed[1]
          && within_six_sigma(observed[2], cohort[2], p_high));
    check("event counter agrees with the site-resolved journal",
          rb.genesis_events_this_tick() == observed[0] + observed[1] + observed[2]);
}

}  // namespace

int main() {
    std::cout << "=== Scale-0 genesis-gate scenario certification ===\n";
    test_one_tick_gate();
    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
