/**
 * History-journal drain: encoding validity + null-orientation reconstruction.
 *
 * The theory reads a manifestation transaction as a four-phase oriented cycle
 * through the null: +1 -> 0-down -> -1 -> 0-up -> +1 (FTD's ternary readout
 * state in {-1,0,+1} has only one zero -- voxel.h stores no birth tick and no
 * null-orientation field; P3's manifestation quotient is many-to-one). The
 * orientation of a given null ("0-down" if it was entered from +1, "0-up" if
 * entered from -1) is therefore not stored anywhere in the Voxel record and
 * must be reconstructed from the event journal's transition history.
 *
 * This test does two things natively (no WASM/emscripten dependency, since
 * the drainHistoryEvents() Embind binding cannot be exercised outside a wasm
 * build):
 *
 *   Part 1 -- structural validity of the drained stream: every event kind is
 *   one of the six declared HistoryEventKind values, ticks/site indices are
 *   sane, and a controlled evaporation-then-genesis pair at the same site
 *   lets a reconstructed null orientation be checked against ground truth.
 *   The row-flattening performed here (one row per touched site, carrying
 *   {tick, site, stateBefore, stateAfter}) mirrors the encoding emitted by
 *   the WASM binding drainHistoryEvents() (engine/wasm/bindings_render_bridge.cpp)
 *   and consumed by the JS transaction tracker
 *   (engine/web/js/scales/scale0/runtime/transaction-tracker.js) -- this test
 *   is the native cross-check that the reconstruction algorithm those two
 *   layers implement is sound.
 *
 *   Part 2 -- the measurement: with the engine's DEFAULT toggle set (no
 *   toggle touched) and a broad multi-patch flux seed comfortably above
 *   K_GENESIS, tick many times, accumulate every drained event, and report
 *   the minimum/median observed null dwell time ("transaction latency"),
 *   the minimum/median birth-to-expiry lifetime, and whether any complete
 *   four-phase cycle (+1 -> 0 -> -1 -> 0 -> +1, or the mirror) was observed
 *   at a single site. This is a measurement, not a preregistered pass/fail
 *   claim: no injection parameter here was tuned to manufacture a cycle.
 */

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

bool valid_kind(ftd::eft::HistoryEventKind k) {
    switch (k) {
        case ftd::eft::HistoryEventKind::Movement:
        case ftd::eft::HistoryEventKind::Genesis:
        case ftd::eft::HistoryEventKind::Evaporation:
        case ftd::eft::HistoryEventKind::PairProduction:
        case ftd::eft::HistoryEventKind::Annihilation:
        case ftd::eft::HistoryEventKind::WeakTransmutation:
            return true;
    }
    return false;
}

// One touched-site row flattened out of a HistoryEvent -- the same shape the
// WASM drainHistoryEvents() binding emits as parallel typed arrays.
struct Row {
    int tick;
    int site;
    std::int8_t stateBefore;
    std::int8_t stateAfter;
};

// A reconstructed null: the site's readout was 0 across [fromTick, toTick).
// Orientation is derived purely from the nonzero state observed immediately
// before/after -- never from a stored field, since none exists.
struct NullInterval {
    int site = -1;
    int fromTick = 0, toTick = 0;
    std::int8_t fromState = 0, toState = 0;
    char orientation = '?';   // 'D' = 0-down (entered from +1), 'U' = 0-up (entered from -1)
    bool cycleConsistent = false;  // toState == -fromState: the canonical continuation
};

struct SiteDeath { int tick; std::int8_t fromState; };

// Site-level reconstruction: a lattice site's own readout history, independent
// of which particle_id (if any) occupied it. Movement's vacated source and
// filled target are each a genuine site-level death/birth in their own right,
// so no special-casing by event kind is needed here -- only the before/after
// state pair at each touched site matters.
void reconstruct(const std::vector<Row>& rows,
                  std::vector<NullInterval>& intervals,
                  std::vector<int>& lifetimes,
                  std::map<int, std::vector<std::pair<int, std::int8_t>>>& chains) {
    std::map<int, SiteDeath> last_death;
    std::map<int, int> open_birth;
    for (const auto& r : rows) {
        const bool was_null = (r.stateBefore == 0);
        const bool is_null = (r.stateAfter == 0);
        if (!was_null && is_null) {
            last_death[r.site] = {r.tick, r.stateBefore};
            auto ob = open_birth.find(r.site);
            if (ob != open_birth.end()) {
                lifetimes.push_back(r.tick - ob->second);
                open_birth.erase(ob);
            }
        } else if (was_null && !is_null) {
            open_birth[r.site] = r.tick;
            chains[r.site].emplace_back(r.tick, r.stateAfter);
            auto d = last_death.find(r.site);
            if (d != last_death.end()) {
                NullInterval iv;
                iv.site = r.site;
                iv.fromTick = d->second.tick;
                iv.toTick = r.tick;
                iv.fromState = d->second.fromState;
                iv.toState = r.stateAfter;
                iv.orientation = (d->second.fromState > 0) ? 'D' : 'U';
                iv.cycleConsistent = (r.stateAfter == static_cast<std::int8_t>(-d->second.fromState));
                intervals.push_back(iv);
                last_death.erase(d);
            }
        }
        // was_null && is_null cannot occur (a row always records an actual
        // transition). !was_null && !is_null is WeakTransmutation's in-place
        // sign flip -- it never touches the null bookkeeping, correctly,
        // since the site's readout never visited 0 in that row.
    }
}

double median_of(std::vector<int> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const std::size_t n = v.size();
    return (n % 2 == 1) ? static_cast<double>(v[n / 2])
                         : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

void append_event_rows(const ftd::eft::HistoryEvent& e, std::vector<Row>& rows) {
    for (int s = 0; s < e.site_count && s < 2; ++s) {
        rows.push_back({e.tick, e.before[s].index,
                         e.before[s].voxel.state, e.after[s].voxel.state});
    }
}

// ---------------------------------------------------------------------
// Part 1: structural validity + a controlled null-orientation check.
// ---------------------------------------------------------------------
void run_structural_validity() {
    constexpr int L = 8;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.disable_all();
    rb.toggles.evaporation = true;
    rb.toggles.langevin_seed = 37;
    check("journal enables on CPU backend", rb.enable_history_journal());

    const int watched_site = rb.lattice().index(3, 3, 3);
    rb.inject_particle(3, 3, 3, +1, {0, 0, 0});

    std::vector<Row> rows;
    int death_tick = -1;
    for (int t = 0; t < 128 && death_tick < 0; ++t) {
        rb.tick();
        for (const auto& e : rb.history_events()) {
            check("event kind is one of the six declared HistoryEventKind values",
                  valid_kind(e.kind));
            check("event tick matches the tick it was recorded on",
                  e.tick == rb.current_tick() - 1);
            check("event site_count is 1 or 2",
                  e.site_count == 1 || e.site_count == 2);
            for (int s = 0; s < e.site_count; ++s) {
                check("touched site index is within the lattice",
                      e.before[s].index >= 0 && e.before[s].index < L * L * L);
                check("before/after capture agree on which site they describe",
                      e.before[s].index == e.after[s].index);
                check("legacy top-level state matches the embedded voxel snapshot",
                      e.before[s].state == e.before[s].voxel.state &&
                      e.after[s].state == e.after[s].voxel.state);
            }
            append_event_rows(e, rows);
            if (e.kind == ftd::eft::HistoryEventKind::Evaporation) death_tick = e.tick;
        }
    }
    check("evaporation event observed", death_tick >= 0);

    // Refill the exact same site via genesis and confirm the reconstructed
    // null orientation matches ground truth: it can only be '0-down', since
    // the only particle ever placed there was +1.
    rb.toggles.genesis = true;
    rb.inject_flux(3, 3, 3, {1000.0 * ftd::K_GENESIS, 0, 0});
    int rebirth_tick = -1;
    for (int t = 0; t < 128 && rebirth_tick < 0; ++t) {
        rb.tick();
        for (const auto& e : rb.history_events()) {
            append_event_rows(e, rows);
            for (int s = 0; s < e.site_count; ++s) {
                if (e.before[s].index == watched_site &&
                    e.before[s].voxel.state == 0 && e.after[s].voxel.state != 0) {
                    rebirth_tick = e.tick;
                }
            }
        }
    }
    check("genesis re-occupies the evaporated site", rebirth_tick >= 0);

    std::vector<NullInterval> intervals;
    std::vector<int> lifetimes;
    std::map<int, std::vector<std::pair<int, std::int8_t>>> chains;
    reconstruct(rows, intervals, lifetimes, chains);

    const auto it = std::find_if(intervals.begin(), intervals.end(),
        [&](const NullInterval& iv) { return iv.site == watched_site; });
    check("a null interval was reconstructed at the evaporated/reborn site",
          it != intervals.end());
    if (it != intervals.end()) {
        check("reconstructed orientation is 0-down (the site could only have died from +1)",
              it->orientation == 'D' && it->fromState == 1);
        std::cout << "  -- reconstructed null: site=" << it->site
                  << " [" << it->fromTick << "," << it->toTick << ") "
                  << "orientation=" << it->orientation
                  << " latency=" << (it->toTick - it->fromTick) << " ticks\n";
    }
}

// ---------------------------------------------------------------------
// Part 2: the measurement, under the engine's default toggle set.
// ---------------------------------------------------------------------
void run_default_toggle_measurement() {
    constexpr int L = 16;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    // No toggle is touched: this exercises whatever the shipped defaults are
    // (genesis=true, movement=true, weak_transmutation=true, forces=true,
    // gravity=true, poisson_coulomb=true, lorentz_force=true; pair_production
    // and evaporation stay OFF, per term_toggles.h).
    rb.toggles.langevin_seed = 424242;  // reproducibility only, not a physics toggle
    rb.seed_rng(424242);
    check("journal enables on CPU backend under default toggles",
          rb.enable_history_journal());

    // Broad, deterministic multi-patch flux seed -- comfortably above
    // K_GENESIS, alternating sign, spread across the lattice interior. Not
    // tuned toward any particular outcome; the same grid pattern would be a
    // reasonable choice for any genesis-response measurement on this engine.
    int patch_index = 0;
    for (int gx = 3; gx < L - 2; gx += 5) {
        for (int gy = 3; gy < L - 2; gy += 5) {
            for (int gz = 3; gz < L - 2; gz += 5) {
                const double sign = (patch_index % 2 == 0) ? 1.0 : -1.0;
                const double peak = sign * 2.2 * ftd::K_GENESIS;
                for (int dz = -1; dz <= 1; ++dz)
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx) {
                            const double falloff =
                                std::exp(-(dx * dx + dy * dy + dz * dz) / 2.0);
                            rb.inject_flux(gx + dx, gy + dy, gz + dz,
                                           {peak * falloff, 0, 0});
                        }
                ++patch_index;
            }
        }
    }

    constexpr int kTicks = 4000;
    std::vector<Row> rows;
    std::map<ftd::eft::HistoryEventKind, long long> kind_counts;
    for (int t = 0; t < kTicks; ++t) {
        rb.tick();
        for (const auto& e : rb.history_events()) {
            ++kind_counts[e.kind];
            append_event_rows(e, rows);
        }
    }

    std::vector<NullInterval> intervals;
    std::vector<int> lifetimes;
    std::map<int, std::vector<std::pair<int, std::int8_t>>> chains;
    reconstruct(rows, intervals, lifetimes, chains);

    auto kind_name = [](ftd::eft::HistoryEventKind k) -> const char* {
        switch (k) {
            case ftd::eft::HistoryEventKind::Movement: return "Movement";
            case ftd::eft::HistoryEventKind::Genesis: return "Genesis";
            case ftd::eft::HistoryEventKind::Evaporation: return "Evaporation";
            case ftd::eft::HistoryEventKind::PairProduction: return "PairProduction";
            case ftd::eft::HistoryEventKind::Annihilation: return "Annihilation";
            case ftd::eft::HistoryEventKind::WeakTransmutation: return "WeakTransmutation";
        }
        return "?";
    };

    std::cout << "\n  -- event counts over " << kTicks << " ticks at L=" << L << " --\n";
    for (const auto& [k, n] : kind_counts) {
        std::cout << "     " << kind_name(k) << ": " << n << '\n';
    }

    check("at least one genesis event fired under default toggles",
          kind_counts.count(ftd::eft::HistoryEventKind::Genesis) &&
          kind_counts[ftd::eft::HistoryEventKind::Genesis] > 0);

    // Minimum/median transaction latency: ticks a site spent in the null
    // state between a death and the next birth at that same site. A dwell of
    // 0 is a real, legitimate outcome, not a reconstruction bug: phase_movement
    // sweeps sites within a single tick, so a site can be vacated by one
    // HistoryEvent and re-occupied by an unrelated HistoryEvent later in the
    // SAME tick's sweep -- the null is real (the site's readout passed
    // through 0) but does not persist across a tick boundary.
    if (!intervals.empty()) {
        std::vector<int> dwell;
        dwell.reserve(intervals.size());
        for (const auto& iv : intervals) dwell.push_back(iv.toTick - iv.fromTick);
        const int min_latency = *std::min_element(dwell.begin(), dwell.end());
        const double med_latency = median_of(dwell);
        const long long same_tick =
            std::count(dwell.begin(), dwell.end(), 0);
        std::vector<int> positive_dwell;
        std::copy_if(dwell.begin(), dwell.end(), std::back_inserter(positive_dwell),
                     [](int d) { return d > 0; });
        std::cout << "\n  -- transaction latency (null dwell time, ticks) --\n"
                  << "     count=" << intervals.size()
                  << " min=" << min_latency
                  << " median=" << med_latency
                  << " same-tick(dwell=0)=" << same_tick
                  << " min-positive(crosses>=1 tick)="
                  << (positive_dwell.empty()
                          ? -1
                          : *std::min_element(positive_dwell.begin(), positive_dwell.end()))
                  << '\n';
        check("every reconstructed null dwell time is non-negative (the null "
              "cannot be filled before it was vacated)",
              min_latency >= 0);
    } else {
        std::cout << "\n  -- no site was ever reborn after dying: "
                     "no transaction latency observed in this run --\n";
    }

    if (!lifetimes.empty()) {
        const int min_life = *std::min_element(lifetimes.begin(), lifetimes.end());
        const double med_life = median_of(lifetimes);
        std::cout << "\n  -- birth-to-expiry lifetime (ticks) --\n"
                  << "     count=" << lifetimes.size()
                  << " min=" << min_life
                  << " median=" << med_life << '\n';
    } else {
        std::cout << "\n  -- no record completed a birth-to-expiry lifetime in this run --\n";
    }

    // Complete four-phase cycle: three consecutive births at the same site
    // with alternating sign (s, -s, s), each pair of which is separated by a
    // cycle-consistent null. Cycle length is measured birth-to-birth.
    std::vector<int> cycle_lengths;
    for (auto& [site, chain] : chains) {
        std::sort(chain.begin(), chain.end());
        for (std::size_t i = 0; i + 2 < chain.size(); ++i) {
            const auto& a = chain[i];
            const auto& b = chain[i + 1];
            const auto& c = chain[i + 2];
            if (a.second != 0 && b.second == static_cast<std::int8_t>(-a.second) &&
                c.second == a.second) {
                cycle_lengths.push_back(c.first - a.first);
            }
        }
    }
    if (!cycle_lengths.empty()) {
        const int min_cycle = *std::min_element(cycle_lengths.begin(), cycle_lengths.end());
        const double med_cycle = median_of(cycle_lengths);
        std::cout << "\n  -- complete +1<->0<->-1 cycles (birth-to-birth, ticks) --\n"
                  << "     count=" << cycle_lengths.size()
                  << " min=" << min_cycle
                  << " median=" << med_cycle << '\n';
    } else {
        std::cout << "\n  -- NO complete +1->0->-1->0->+1 cycle observed at any single site "
                     "under default dynamics in " << kTicks << " ticks --\n";
    }

    // Sanity: no interval's orientation contradicts the state it recorded.
    bool orientation_consistent = true;
    for (const auto& iv : intervals) {
        if ((iv.fromState > 0 && iv.orientation != 'D') ||
            (iv.fromState < 0 && iv.orientation != 'U')) {
            orientation_consistent = false;
        }
    }
    check("every reconstructed orientation matches the sign it was derived from",
          orientation_consistent);
}

}  // namespace

int main() {
    std::cout << "FTD history-journal drain: encoding validity + null-orientation reconstruction\n\n";
    run_structural_validity();
    std::cout << '\n';
    run_default_toggle_measurement();
    std::cout << "\nhistory_journal_drain: " << (failures == 0 ? "PASS" : "FAIL")
              << " (" << failures << " failing checks)\n";
    return failures;
}
