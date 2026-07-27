/**
 * Certification for the two native opposite-polarity event scenarios.
 *
 * This test deliberately distinguishes implemented transition logic from its
 * possible physical interpretation. Pair production is a selected stochastic
 * rule. Opposite-state collision removes both states and redistributes only
 * pre-existing flux; it does not convert rest mass into radiation.
 */

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

bool only_enabled(const ftd::TermToggles& toggles,
                  const std::string& expected) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const bool enabled = toggles.*(spec.field);
        if (std::string(spec.name) == expected) {
            if (!enabled) return false;
        } else if (std::string(spec.name) == "strict_validation") {
            continue;
        } else if (enabled) {
            return false;
        }
    }
    return !toggles.dual_substrate;
}

bool is_pair_source(int x, int y, int z, int L) {
    if (x < 2 || y < 2 || z < 2 || x + 1 >= L - 2
        || y >= L - 2 || z >= L - 2) return false;
    return (x - 2) % 3 == 0 && (y - 2) % 3 == 0 && (z - 2) % 3 == 0;
}

double field_norm(const ftd::RenderBridge& rb) {
    double sum = 0.0;
    for (const auto& v : rb.voxels()) sum += v.flux.mag2();
    return sum;
}

ftd::Vec3 field_sum(const ftd::RenderBridge& rb) {
    ftd::Vec3 sum;
    for (const auto& v : rb.voxels()) sum += v.flux;
    return sum;
}

void test_pair_production_cohort() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L);
    rb.force_cpu();

    rb.toggles.genesis = true;
    rb.toggles.wave_propagation = true;
    rb.toggles.matched_gauss_dynamics = true;
    rb.genesis_threshold_override = 999.0;
    rb.manifest_scale_override = 999.0;

    check("pair cohort scenario dispatches",
          ftd::dispatch_scenario(rb, "flux-pair-production"));
    check("pair cohort isolates the pair-production rule",
          only_enabled(rb.toggles, "pair_production")
          && rb.toggles.langevin_seed == 1
          && rb.genesis_threshold_override <= 0.0
          && rb.manifest_scale_override <= 0.0);

    const double pair_amp = ftd::K_GENESIS + ftd::K_MANIFEST * std::log(2.0);
    int sources = 0;
    bool exact_initial_data = true;
    for (int z = 0; z < L; ++z)
    for (int y = 0; y < L; ++y)
    for (int x = 0; x < L; ++x) {
        const auto& v = rb.voxel_at(x, y, z);
        if (is_pair_source(x, y, z, L)) {
            ++sources;
            exact_initial_data = exact_initial_data
                && std::fabs(v.flux.x - pair_amp) < 1e-15
                && v.flux.y == 0.0 && v.flux.z == 0.0
                && v.wave_vel.mag2() == 0.0 && v.state == 0
                && rb.voxel_at(x + 1, y, z).state == 0
                && rb.voxel_at(x + 1, y, z).flux.mag2() == 0.0;
        } else if (!is_pair_source(x - 1, y, z, L)) {
            exact_initial_data = exact_initial_data
                && v.flux.mag2() == 0.0 && v.wave_vel.mag2() == 0.0
                && v.state == 0;
        }
    }
    check("isolated p=1/2 source-partner cells are initialized exactly",
          exact_initial_data && sources == 343);

    check("read-only event journal enables", rb.enable_history_journal());
    rb.toggles.strict_validation = true;
    rb.tick();

    int pair_events = 0;
    int other_events = 0;
    bool exact_events = true;
    for (const auto& event : rb.history_events()) {
        if (event.kind != ftd::eft::HistoryEventKind::PairProduction) {
            ++other_events;
            continue;
        }
        ++pair_events;
        const auto a = rb.lattice().coord(event.after[0].index);
        const auto b = rb.lattice().coord(event.after[1].index);
        const auto& upstream = rb.voxels()[event.after[0].index];
        const auto& downstream = rb.voxels()[event.after[1].index];
        exact_events = exact_events
            && is_pair_source(a.x, a.y, a.z, L)
            && b.x == a.x + 1 && b.y == a.y && b.z == a.z
            && event.before[0].state == 0 && event.before[1].state == 0
            && event.after[0].state == -1 && event.after[1].state == +1
            && event.after[0].flux.x + event.after[1].flux.x == 0.0
            && event.after[0].flux.y + event.after[1].flux.y == 0.0
            && event.after[0].flux.z + event.after[1].flux.z == 0.0
            && upstream.pair_id >= 0 && upstream.pair_id == downstream.pair_id
            && upstream.particle_id >= 0 && downstream.particle_id >= 0
            && upstream.particle_id != downstream.particle_id;
    }

    int manifested = 0;
    int signed_sum = 0;
    for (const auto& v : rb.voxels()) {
        if (v.state != 0) ++manifested;
        signed_sum += v.state;
    }
    const double mean = 0.5 * sources;
    const double sigma = std::sqrt(0.25 * sources);
    std::cout << "    sources=" << sources << " events=" << pair_events
              << " expected=" << mean << " +/- " << sigma << '\n';
    check("accepted count satisfies the preregistered six-sigma Bernoulli gate",
          pair_events > 0 && std::fabs(pair_events - mean) <= 6.0 * sigma);
    check("every journaled event obeys the compiled polarity-pair map",
          exact_events && other_events == 0);
    check("pair creation preserves exact signed polarity globally",
          manifested == 2 * pair_events && signed_sum == 0);
}

void test_opposite_state_collision() {
    constexpr int L = 24;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.genesis = true;
    rb.toggles.wave_propagation = true;
    rb.toggles.lorentz_force = true;

    check("collision scenario dispatches",
          ftd::dispatch_scenario(rb, "flux-annihilation"));
    check("collision scenario isolates movement",
          only_enabled(rb.toggles, "movement"));

    const double norm_before = field_norm(rb);
    const auto sum_before = field_sum(rb);
    check("initial collision is polarity- and vector-flux-neutral",
          norm_before > 0.0 && sum_before.mag2() == 0.0);

    check("read-only event journal enables for collision", rb.enable_history_journal());
    rb.toggles.strict_validation = true;
    rb.tick();
    int early_annihilation = 0;
    for (const auto& event : rb.history_events())
        if (event.kind == ftd::eft::HistoryEventKind::Annihilation)
            ++early_annihilation;
    check("sub-cell remainder prevents a first-tick collision", early_annihilation == 0);

    rb.tick();
    int annihilations = 0;
    int other_events = 0;
    bool exact_event = true;
    for (const auto& event : rb.history_events()) {
        if (event.kind != ftd::eft::HistoryEventKind::Annihilation) {
            ++other_events;
            continue;
        }
        ++annihilations;
        exact_event = exact_event
            && event.site_count == 2
            && event.before[0].state * event.before[1].state == -1
            && event.after[0].state == 0 && event.after[1].state == 0;
    }

    int manifested = 0;
    bool zero_wave_velocity = true;
    for (const auto& v : rb.voxels()) {
        if (v.state != 0) ++manifested;
        zero_wave_velocity = zero_wave_velocity && v.wave_vel.mag2() == 0.0;
    }
    const double norm_after = field_norm(rb);
    const auto sum_after = field_sum(rb);
    std::cout << "    field_norm_ratio=" << norm_after / norm_before
              << " vector_sum2=" << sum_after.mag2() << '\n';
    check("exactly one opposite-state collision removes both states",
          annihilations == 1 && other_events == 0 && exact_event
          && manifested == 0);
    check("collision conserves the pre-existing vector-flux sum",
          sum_after.mag2() < 1e-28);
    check("six-face spreading gives the exact 1/6 field-norm ratio",
          std::fabs(norm_after / norm_before - 1.0 / 6.0) < 1e-14);
    check("collision creates no wave momentum or rest-mass radiation",
          zero_wave_velocity && !rb.toggles.wave_propagation);
}

}  // namespace

int main() {
    std::cout << "=== Scale-0 native reaction scenario certification ===\n";
    test_pair_production_cohort();
    test_opposite_state_collision();
    std::cout << "=== " << (failures == 0 ? "ALL PASS" : "FAILURES")
              << " (" << failures << ") ===\n";
    return failures == 0 ? 0 : 1;
}
