/**
 * C4 transaction census -- does the engine's transaction structure realise the
 * four-phase oriented cycle  +1 -> 0-down -> -1 -> 0-up -> +1 ?
 *
 * Executes PREREG_C4_TRANSACTION_CENSUS_v1.md (docs/theory/10_eft_program/
 * preregistrations/engine_transaction_census/). Read the lock first; this file
 * computes, it does not decide. Gates and thresholds are those declared there.
 *
 * Same run as test_history_journal_drain.cpp's default-toggle measurement
 * (L=16, 4000 ticks, deterministic multi-patch seed, nothing tuned), so the
 * "min complete cycle 36 ticks" run of FTD-1027 is the run under test. The
 * site-level reconstruction helpers are duplicated from that test verbatim so
 * this census neither modifies nor depends on its sibling.
 *
 * Claims tested (see the lock for the gates):
 *   (A) reversal passes through the null      -> T3 (null-skip fraction)
 *   (B) the null is a passage, not a bounce   -> T1 (reversal vs bounce)
 *   (C) the cycle has a characteristic period -> T2 (chi^2 on residues mod 4 / mod 8)
 */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "ftd/constants.h"
#include "ftd/eft/history_event_journal.h"
#include "ftd/render_bridge.h"

namespace {

int failures = 0;
void check(const std::string& name, bool condition) {
    std::cout << (condition ? "  PASS  " : "  FAIL  ") << name << '\n';
    if (!condition) ++failures;
}

// ---- reconstruction helpers (verbatim from test_history_journal_drain.cpp) ----
struct Row { int tick; int site; std::int8_t stateBefore; std::int8_t stateAfter; };

struct NullInterval {
    int site = -1;
    int fromTick = 0, toTick = 0;
    std::int8_t fromState = 0, toState = 0;
    char orientation = '?';
    bool cycleConsistent = false;
};

struct SiteDeath { int tick; std::int8_t fromState; };

void reconstruct(const std::vector<Row>& rows,
                 std::vector<NullInterval>& intervals,
                 std::map<int, std::vector<std::pair<int, std::int8_t>>>& chains) {
    std::map<int, SiteDeath> last_death;
    for (const auto& r : rows) {
        const bool was_null = (r.stateBefore == 0);
        const bool is_null = (r.stateAfter == 0);
        if (!was_null && is_null) {
            last_death[r.site] = {r.tick, r.stateBefore};
        } else if (was_null && !is_null) {
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
    }
}

void append_event_rows(const ftd::eft::HistoryEvent& e, std::vector<Row>& rows) {
    for (int s = 0; s < e.site_count && s < 2; ++s) {
        rows.push_back({e.tick, e.before[s].index,
                        e.before[s].voxel.state, e.after[s].voxel.state});
    }
}

double median_of(std::vector<int> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const std::size_t n = v.size();
    return (n % 2 == 1) ? static_cast<double>(v[n / 2]) : 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

// ---- chi^2 against uniform residues --------------------------------------
// Critical values at p = 0.01 (standard chi^2 table; these DEFINE the gate,
// they are not results): df=3 -> 11.345, df=7 -> 18.475.
constexpr double kChi2Crit_df3 = 11.345;
constexpr double kChi2Crit_df7 = 18.475;

struct ResidueTest { int k; double chi2; double crit; long long n; std::vector<long long> counts; };

ResidueTest residue_test(const std::vector<int>& xs, int k) {
    ResidueTest t{k, 0.0, (k == 4) ? kChi2Crit_df3 : kChi2Crit_df7, 0, std::vector<long long>(k, 0)};
    for (int x : xs) { ++t.counts[((x % k) + k) % k]; ++t.n; }
    if (t.n == 0) return t;
    const double expected = static_cast<double>(t.n) / k;
    for (int i = 0; i < k; ++i) {
        const double d = t.counts[i] - expected;
        t.chi2 += d * d / expected;
    }
    return t;
}

void print_residue(const char* label, const ResidueTest& t) {
    std::cout << "     " << label << " mod " << t.k << ": n=" << t.n << " counts=[";
    for (int i = 0; i < t.k; ++i) std::cout << (i ? "," : "") << t.counts[i];
    std::cout << "] chi2=" << t.chi2 << " crit(p<.01)=" << t.crit
              << (t.chi2 > t.crit ? "  -> STRUCTURE" : "  -> uniform") << '\n';
}

}  // namespace

int main() {
    std::cout << "FTD C4 transaction census (PREREG_C4_TRANSACTION_CENSUS_v1)\n\n";

    // ---- the run under test: identical to test_history_journal_drain's Part 2 ----
    constexpr int L = 16;
    ftd::RenderBridge rb(L);
    rb.force_cpu();
    rb.toggles.langevin_seed = 424242;
    rb.seed_rng(424242);
    check("journal enables on CPU backend under default toggles", rb.enable_history_journal());

    int patch_index = 0;
    for (int gx = 3; gx < L - 2; gx += 5)
        for (int gy = 3; gy < L - 2; gy += 5)
            for (int gz = 3; gz < L - 2; gz += 5) {
                const double sign = (patch_index % 2 == 0) ? 1.0 : -1.0;
                const double peak = sign * 2.2 * ftd::K_GENESIS;
                for (int dz = -1; dz <= 1; ++dz)
                    for (int dy = -1; dy <= 1; ++dy)
                        for (int dx = -1; dx <= 1; ++dx) {
                            const double falloff = std::exp(-(dx * dx + dy * dy + dz * dz) / 2.0);
                            rb.inject_flux(gx + dx, gy + dy, gz + dz, {peak * falloff, 0, 0});
                        }
                ++patch_index;
            }

    constexpr int kTicks = 4000;
    std::vector<Row> rows;
    for (int t = 0; t < kTicks; ++t) {
        rb.tick();
        for (const auto& e : rb.history_events()) append_event_rows(e, rows);
    }

    std::vector<NullInterval> intervals;
    std::map<int, std::vector<std::pair<int, std::int8_t>>> chains;
    reconstruct(rows, intervals, chains);

    // ---- D1: transition-type census ------------------------------------------
    long long p_to_0 = 0, m_to_0 = 0, z_to_p = 0, z_to_m = 0, p_to_m = 0, m_to_p = 0, other = 0;
    for (const auto& r : rows) {
        const int b = r.stateBefore, a = r.stateAfter;
        if (b > 0 && a == 0) ++p_to_0; else if (b < 0 && a == 0) ++m_to_0;
        else if (b == 0 && a > 0) ++z_to_p; else if (b == 0 && a < 0) ++z_to_m;
        else if (b > 0 && a < 0) ++p_to_m; else if (b < 0 && a > 0) ++m_to_p;
        else ++other;
    }
    std::cout << "\n  -- D1 transition-type census (site-level rows, " << kTicks << " ticks, L=" << L << ") --\n"
              << "     +->0: " << p_to_0 << "   -->0: " << m_to_0
              << "   0->+: " << z_to_p << "   0->-: " << z_to_m << '\n'
              << "     +->- (in-place, null skipped): " << p_to_m
              << "   -->+ (in-place, null skipped): " << m_to_p
              << "   other: " << other << '\n';

    // ---- T1: passage vs bounce at the null -----------------------------------
    long long reversal = 0, bounce = 0;
    for (const auto& iv : intervals) { if (iv.cycleConsistent) ++reversal; else ++bounce; }
    const double reversal_frac = intervals.empty() ? 0.0
        : static_cast<double>(reversal) / static_cast<double>(intervals.size());
    std::cout << "\n  -- T1 the null: passage or bounce? --\n"
              << "     reconstructed nulls=" << intervals.size()
              << "  exit to -from (C4 passage)=" << reversal
              << "  exit to +from (bounce)=" << bounce
              << "  reversal fraction=" << reversal_frac << '\n';
    const bool t1 = !intervals.empty() && reversal_frac >= 0.5;
    check("T1 (gate: reversal fraction >= 0.5) -- nulls are predominantly oriented passages", t1);

    // ---- T3: do reversals skip the null? ------------------------------------
    const long long skip = p_to_m + m_to_p;
    const long long all_reversals = skip + reversal;
    const double skip_frac = all_reversals ? static_cast<double>(skip) / all_reversals : 0.0;
    std::cout << "\n  -- T3 do polarity reversals pass through the null? --\n"
              << "     in-place +s->-s (skip)=" << skip
              << "  via null (passage)=" << reversal
              << "  skip fraction=" << skip_frac << '\n';
    const bool t3 = all_reversals > 0 && skip_frac < 0.1;
    check("T3 (gate: skip fraction < 0.1) -- reversal passes through the null", t3);

    // ---- T2: characteristic period? ------------------------------------------
    std::vector<int> half_cycles, positive_dwell;
    for (auto& [site, chain] : chains) {
        std::sort(chain.begin(), chain.end());
        for (std::size_t i = 0; i + 1 < chain.size(); ++i)
            if (chain[i].second != 0 && chain[i + 1].second == static_cast<std::int8_t>(-chain[i].second))
                half_cycles.push_back(chain[i + 1].first - chain[i].first);
    }
    for (const auto& iv : intervals) { const int d = iv.toTick - iv.fromTick; if (d > 0) positive_dwell.push_back(d); }

    std::cout << "\n  -- T2 period structure (chi^2 on residues vs uniform) --\n"
              << "     half-cycles (birth -> next opposite-sign birth, same site): n=" << half_cycles.size()
              << (half_cycles.empty() ? "" : "  min=" + std::to_string(*std::min_element(half_cycles.begin(), half_cycles.end()))
                                             + " median=" + std::to_string(median_of(half_cycles)))
              << '\n';
    const bool powered_hc = half_cycles.size() >= 30, powered_dw = positive_dwell.size() >= 100;
    bool t2 = false;
    if (powered_hc) {
        const auto a4 = residue_test(half_cycles, 4), a8 = residue_test(half_cycles, 8);
        print_residue("T2a half-cycle", a4); print_residue("T2a half-cycle", a8);
        t2 = t2 || a4.chi2 > a4.crit || a8.chi2 > a8.crit;
    } else std::cout << "     T2a: underpowered (<30 half-cycles) -- no verdict\n";
    if (powered_dw) {
        const auto b4 = residue_test(positive_dwell, 4), b8 = residue_test(positive_dwell, 8);
        print_residue("T2b dwell     ", b4); print_residue("T2b dwell     ", b8);
        t2 = t2 || b4.chi2 > b4.crit || b8.chi2 > b8.crit;
    } else std::cout << "     T2b: underpowered (<100 positive dwells) -- no verdict\n";
    if (powered_hc || powered_dw)
        check("T2 (gate: chi^2 above p<0.01 critical on any series) -- a characteristic period exists", t2);

    // ---- D2: dwell histogram ---------------------------------------------------
    {
        std::vector<int> all_dwell; all_dwell.reserve(intervals.size());
        for (const auto& iv : intervals) all_dwell.push_back(iv.toTick - iv.fromTick);
        const int edges[] = {0, 1, 2, 3, 4, 5, 9, 17};
        const char* labels[] = {"0", "1", "2", "3", "4", "5-8", "9-16", "17+"};
        long long bins[8] = {0};
        double sum = 0, sum2 = 0;
        for (int d : all_dwell) {
            sum += d; sum2 += static_cast<double>(d) * d;
            int b = 7; for (int i = 7; i >= 0; --i) if (d >= edges[i]) { b = i; break; }
            ++bins[b];
        }
        const double n = static_cast<double>(all_dwell.size());
        const double mean = n ? sum / n : 0, var = n ? sum2 / n - mean * mean : 0;
        std::cout << "\n  -- D2 null dwell histogram (ticks) --\n     ";
        for (int i = 0; i < 8; ++i) std::cout << labels[i] << ":" << bins[i] << "  ";
        std::cout << "\n     mean=" << mean << " CV=" << (mean > 0 ? std::sqrt(std::max(var, 0.0)) / mean : 0.0) << '\n';
    }

    // ---- verdict per the lock ---------------------------------------------------
    std::cout << "\n  -- VERDICT (PREREG_C4_TRANSACTION_CENSUS_v1 section 'Outcomes') --\n";
    if (t1 && t3 && t2)            std::cout << "     C4-REALISED\n";
    else if (!t1 || !t3)           std::cout << "     C4-NOT-REALISED (T1=" << t1 << " T3=" << t3 << ")\n";
    else if (!(powered_hc || powered_dw)) std::cout << "     UNDERPOWERED for T2; T1 and T3 pass\n";
    else                           std::cout << "     PERIOD-ONLY-FAILS: oriented passages without a characteristic period\n";

    // The CTest exit status reports whether the INSTRUMENT ran validly, not the
    // theory verdict: the lock says a negative is an expected, informative outcome.
    const bool instrument_ok = (failures == 0) || (!intervals.empty());
    std::cout << "\nc4_transaction_census: " << (instrument_ok ? "INSTRUMENT-OK" : "INSTRUMENT-FAIL")
              << " (gates failed: " << failures << ")\n";
    return instrument_ok ? 0 : 1;
}
