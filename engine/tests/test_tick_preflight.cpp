// Bounded production-RenderBridge preflight regression. This is neither a
// complete checkpoint schema nor a transaction/rollback test for later phases.
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <limits>
#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void check(bool ok, const std::string& label) {
    if (ok) ++passed;
    else { ++failed; std::printf("FAIL: %s\n", label.c_str()); }
}

// Store every named value without hashing or reading struct padding. Floating
// values retain their bit patterns, including the invalid NaN-clock witness.
using Words = std::vector<std::uint64_t>;
using Records = std::map<std::string, Words>;
template<class T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void append(Words& out, T value) { out.push_back(static_cast<std::uint64_t>(value)); }
void append(Words& out, double value) {
    std::uint64_t bits;
    static_assert(sizeof(bits) == sizeof(value));
    std::memcpy(&bits, &value, sizeof(bits));
    out.push_back(bits);
}
void append(Words& out, const ftd::Vec3& v) {
    append(out, v.x); append(out, v.y); append(out, v.z);
}
void append(Words& out, const ftd::Voxel& v) {
    append(out, v.state);
    append(out, v.flux); append(out, v.wave_vel);
    append(out, v.flux_L); append(out, v.flux_R);
    append(out, v.wave_vel_L); append(out, v.wave_vel_R);
    append(out, v.velocity); append(out, v.remainder);
    append(out, v.latency); append(out, v.tau); append(out, v.phase);
    append(out, v.locked); append(out, v.particle_id); append(out, v.pair_id);
    append(out, v.spin); append(out, v.color); append(out, v.flavor);
    append(out, v.accel_mag);
    append(out, v.flux_strong); append(out, v.wave_vel_strong);
    append(out, v.flux_weak); append(out, v.wave_vel_weak);
}
void append(Words& out, const ftd::eft::HistorySiteState& s) {
    append(out, s.index); append(out, s.state); append(out, s.chirality_sign);
    append(out, s.flux); append(out, s.flux_L); append(out, s.flux_R);
    append(out, s.voxel);
}
template<class T>
void append(Words& out, const std::vector<T>& values) {
    append(out, values.size());
    for (const auto& value : values) append(out, value);
}
void same_records(const Records& before, const Records& after,
                  const std::string& label) {
    check(before.size() == after.size(), label + ": record groups");
    for (const auto& entry : before) {
        const auto found = after.find(entry.first);
        check(found != after.end() && found->second == entry.second,
              label + ": " + entry.first);
    }
}
} // namespace

namespace ftd {
// Test-only access to otherwise inaccessible horizons and observation records.
// No mutable observer getter is called while capturing a rejected tick.
struct TickPreflightTestAccess {
    static Records capture(const RenderBridge& b) {
        Records r;
#define RECORD(member) append(r[#member], b.member)
        RECORD(tick_); RECORD(dt_); RECORD(physical_time_);
        RECORD(causal_projection_events_this_tick_);
        RECORD(genesis_events_this_tick_); RECORD(evaporation_events_this_tick_);
        RECORD(self_field_injection_);
        RECORD(voxels_); RECORD(delta_j_); RECORD(delta_j_L_); RECORD(delta_j_R_);
        RECORD(dJ_); RECORD(phi_); RECORD(phi_coulomb_); RECORD(phi_latency_);
        RECORD(moved_); RECORD(flux_pre_write_); RECORD(near_particle_);
        RECORD(near_accel_); RECORD(thread_seeds_); RECORD(sor_source_);
        RECORD(ternary_dirty_from_voxels_); RECORD(fields_dirty_from_voxels_);
        RECORD(flux_pump_configured_); RECORD(flux_pump_profile_built_);
        RECORD(flux_pump_ticks_); RECORD(flux_pump_period_);
        RECORD(flux_pump_next_tick_); RECORD(flux_pump_applied_);
        RECORD(flux_pump_work_);
        RECORD(flux_pump_profile_.support); RECORD(flux_pump_profile_.dilated);
        RECORD(flux_pump_profile_.delta); RECORD(flux_pump_profile_.ticks);
        RECORD(flux_cell_port_configured_); RECORD(flux_cell_port_open_);
        RECORD(flux_cell_port_work_out_); RECORD(flux_cell_port_poynting_out_);
        RECORD(flux_cell_port_sites_); RECORD(flux_cell_port_surface_);
        RECORD(energy_ledger_.updates); RECORD(energy_ledger_.tick_prev);
        RECORD(energy_ledger_.E_prev); RECORD(energy_ledger_.E_curr);
        RECORD(energy_ledger_.dE_dt); RECORD(energy_ledger_.drift_frac);
        RECORD(energy_ledger_.expected_rate); RECORD(energy_ledger_.residual);
        RECORD(energy_ledger_.cumulative_injection);
        RECORD(energy_ledger_.cumulative_dissipation);
        RECORD(energy_ledger_.max_residual_seen);
#ifdef FTD_ENABLE_CUDA
        RECORD(gpu_dirty_); RECORD(host_mutated_);
#endif
#undef RECORD
        for (const auto ch : b.last_validation_warn_)
            append(r["warning memo"], static_cast<unsigned char>(ch));
        append(r["history enabled"], b.history_event_journal_->enabled());
        const auto history = b.history_event_journal_->snapshot();
        auto& journal = r["complete history payload"];
        append(journal, history.size());
        for (const auto& event : history) {
            append(journal, static_cast<std::uint8_t>(event.kind));
            append(journal, event.tick); append(journal, event.site_count);
            for (const auto& s : event.before) append(journal, s);
            for (const auto& s : event.after) append(journal, s);
        }
        const auto& ternary = b.engine_state_.ternary;
        append(r["ternary states"], ternary.states());
        append(r["ternary positive bits"], ternary.pos_bits());
        append(r["ternary negative bits"], ternary.neg_bits());
        append(r["ternary occupied bits"], ternary.occupied_bits());
        append(r["ternary active order"], ternary.active_indices());
        append(r["ternary counts"], ternary.positive_count());
        append(r["ternary counts"], ternary.negative_count());
        append(r["ternary counts"], ternary.charge_sum());
        append(r["matched initialized"], b.matched_gauss_initialized());
        append(r["next particle identity"], b.injector_.peek_next_particle_id());
        append(r["next pair identity"], b.injector_.peek_next_pair_id());
        // Supplemental RNG diagnostic only; the comparisons above do not
        // depend on a dynamical digest or pretend this hash is a checkpoint.
        append(r["supplemental RNG hash"], b.rng_state_hash());
        return r;
    }
    static void prior_observation_sentinels(RenderBridge& b) {
        b.genesis_events_this_tick_ = 3;
        b.evaporation_events_this_tick_ = 5;
        b.last_validation_warn_ = "previous configuration warning";
    }
    static void tick(RenderBridge& b, int value) { b.tick_ = value; }
    static void time(RenderBridge& b, double value) { b.physical_time_ = value; }
    static void dt(RenderBridge& b, double value) { b.dt_ = value; }
    static void updates(RenderBridge& b, std::uint64_t value) {
        b.energy_ledger_.updates = value;
    }
    static void pump_next(RenderBridge& b, int value) { b.flux_pump_next_tick_ = value; }
    static void pump_period(RenderBridge& b, int value) { b.flux_pump_period_ = value; }
    static void pump_applied(RenderBridge& b, int value) { b.flux_pump_applied_ = value; }
    static int pump_next(const RenderBridge& b) { return b.flux_pump_next_tick_; }
    static const std::string& warning(const RenderBridge& b) { return b.last_validation_warn_; }
};
} // namespace ftd

namespace {
using ftd::RenderBridge;
using Access = ftd::TickPreflightTestAccess;

void prepare_prior_tick(RenderBridge& b) {
    b.force_cpu();
    b.toggles.disable_all();
    b.seed_rng(92817);
    check(b.enable_history_journal(), "CPU journal enabled");
    b.inject_particle(2, 2, 2, 1, {0.2, 0.1, 0.05}, 1, 2);
    auto& v = b.voxels()[b.lattice().index(2, 2, 2)];
    v.velocity = {0.9, 0.0, 0.0}; // actual causal projection, then one hop
    v.remainder = {0.8, 0.0, 0.0};
    v.tau = 0.75; v.phase = 1.25; v.pair_id = 17;
    b.toggles.movement = true;
    b.tick();
    check(!b.history_events().empty(), "prior tick produced a movement event");
    check(b.causal_projection_events_this_tick() > 0,
          "prior tick produced a causal projection count");
    check(b.energy_ledger().updates == 1, "prior tick populated the ledger");
    Access::prior_observation_sentinels(b);
}

void rejected_unchanged(RenderBridge& b, const std::string& expected,
                        const std::string& label) {
    const auto before = Access::capture(b);
    bool rejected = false;
    try { b.tick(); }
    catch (const std::exception& e) {
        rejected = true;
        check(std::string(e.what()).find(expected) != std::string::npos,
              label + ": specific preflight rejection");
    }
    check(rejected, label + ": rejected");
    same_records(before, Access::capture(b), label + ": unchanged");
}

void configuration_rejections() {
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.selective_damping = true;
        b.toggles.strict_validation = true;
        rejected_unchanged(b, "selective_damping requires damping", "strict invalid profile");
    }
    for (bool bcc : {false, true}) {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.symplectic_leapfrog = true;
        b.set_dt(0.25);
        b.toggles.wave_propagation = true;
        b.toggles.lorentz_period2_floquet = !bcc;
        b.toggles.lorentz_bcc_time_floquet = bcc;
        b.toggles.strict_validation = true;
        rejected_unchanged(b, "mutually exclusive", bcc ? "BCC stale dt" : "period-two stale dt");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.matched_gauss_dynamics = true;
        b.toggles.flux_pump = true;
        rejected_unchanged(b, "isolated conservative movement", "matched profile rejects without strict flag");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.matched_gauss_dynamics = true;
        b.set_dt(2.0);
        rejected_unchanged(b, "locked unit tick", "matched nonunit dt");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.matched_gauss_dynamics = true;
        rejected_unchanged(b, "explicit initialization", "matched uninitialized");
    }
}

void horizon_rejections() {
    for (int tick : {-1, std::numeric_limits<int>::max()}) {
        RenderBridge b(5); prepare_prior_tick(b); Access::tick(b, tick);
        rejected_unchanged(b, "tick counter", "tick horizon");
    }
    for (double t : {-1.0, std::numeric_limits<double>::infinity(),
                     std::numeric_limits<double>::quiet_NaN()}) {
        RenderBridge b(5); prepare_prior_tick(b); Access::time(b, t);
        rejected_unchanged(b, "physical time", "invalid physical clock");
    }
    for (double dt : {0.0, -1.0, std::numeric_limits<double>::infinity(),
                      std::numeric_limits<double>::quiet_NaN()}) {
        RenderBridge b(5); prepare_prior_tick(b); Access::dt(b, dt);
        rejected_unchanged(b, "physical time", "invalid stored step");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.set_dt(std::numeric_limits<double>::max());
        Access::time(b, std::numeric_limits<double>::max());
        rejected_unchanged(b, "physical time", "physical sum overflow");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        Access::updates(b, std::numeric_limits<std::uint64_t>::max());
        rejected_unchanged(b, "energy ledger", "ledger horizon");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.selective_damping = true; // warning-only profile
        Access::tick(b, std::numeric_limits<int>::max());
        rejected_unchanged(b, "tick counter", "warning memo retained on later preflight rejection");
    }
    for (bool first_opportunity : {false, true}) {
        RenderBridge b(5); prepare_prior_tick(b);
        b.set_flux_pump(ftd::default_flux_cell_torus_spec(5), 1,
                        std::numeric_limits<int>::max());
        b.toggles.flux_pump = true;
        if (!first_opportunity) Access::pump_next(b, b.current_tick());
        rejected_unchanged(b, "due flux-pump", first_opportunity
            ? "first pump opportunity overflow" : "scheduled pump opportunity overflow");
    }
    for (int invalid : {0, 1, 2}) {
        RenderBridge b(5); prepare_prior_tick(b);
        b.set_flux_pump(ftd::default_flux_cell_torus_spec(5), 1, 2);
        b.toggles.flux_pump = true;
        if (invalid == 0) Access::pump_period(b, 0);
        if (invalid == 1) Access::pump_next(b, -2);
        if (invalid == 2) Access::pump_applied(b, -1);
        rejected_unchanged(b, "invalid active flux-pump", "invalid active schedule");
    }
}

void accepted_boundaries() {
    {
        RenderBridge b(5); b.force_cpu(); b.toggles.disable_all();
        Access::tick(b, std::numeric_limits<int>::max() - 1);
        Access::updates(b, std::numeric_limits<std::uint64_t>::max() - 1);
        b.tick();
        check(b.current_tick() == std::numeric_limits<int>::max(), "last representable tick accepted");
        check(b.energy_ledger().updates == std::numeric_limits<std::uint64_t>::max(),
              "last representable ledger update accepted");
        rejected_unchanged(b, "tick counter", "next tick fails after exact boundary");
    }
    {
        RenderBridge b(5); b.force_cpu(); b.toggles.disable_all();
        b.set_flux_pump(ftd::default_flux_cell_torus_spec(5), 1,
                        std::numeric_limits<int>::max());
        b.toggles.flux_pump = true;
        b.tick(); // next=0 + INT_MAX is representable, even on final application
        check(b.flux_pump_ticks_applied() == 1, "boundary pump increment applied once");
        check(Access::pump_next(b) == std::numeric_limits<int>::max(),
              "exact pump horizon retained after final increment");
        b.tick();
        check(b.flux_pump_ticks_applied() == 1, "completed pump is inert at large period");
    }
    {
        RenderBridge b(5); b.force_cpu(); b.toggles.disable_all();
        b.set_flux_pump(ftd::default_flux_cell_torus_spec(5), 1,
                        std::numeric_limits<int>::max());
        Access::pump_next(b, std::numeric_limits<int>::max());
        b.toggles.flux_pump = true;
        b.tick();
        check(b.flux_pump_ticks_applied() == 0 && b.current_tick() == 1,
              "future pump is not prematurely rejected or applied");
    }
    {
        RenderBridge b(5); b.force_cpu(); b.toggles.disable_all();
        b.toggles.matched_gauss_dynamics = true;
        check(b.initialize_matched_gauss_dynamics().valid, "matched vacuum initialized");
        b.tick();
        check(b.current_tick() == 1 && b.physical_time() == 1.0,
              "initialized matched vacuum advances");
    }
    for (bool bcc : {false, true}) {
        RenderBridge stale(5), unit(5);
        for (auto* b : {&stale, &unit}) {
            prepare_prior_tick(*b);
            b->toggles.symplectic_leapfrog = true;
            b->set_dt(0.25);
            b->toggles.symplectic_leapfrog = false;
            b->toggles.wave_propagation = true;
            b->toggles.lorentz_period2_floquet = !bcc;
            b->toggles.lorentz_bcc_time_floquet = bcc;
            b->toggles.strict_validation = true;
        }
        unit.set_dt(1.0);
        stale.tick(); unit.tick();
        check(stale.physical_time() == 2.0, "Floquet accumulates normalized unit step");
        same_records(Access::capture(unit), Access::capture(stale),
                     bcc ? "valid BCC normalization parity" : "valid period-two normalization parity");
    }
    {
        RenderBridge b(5); prepare_prior_tick(b);
        b.toggles.selective_damping = true;
        b.tick();
        check(b.current_tick() == 2 && Access::warning(b).find("requires damping") != std::string::npos,
              "warning-only profile still advances and memoizes");
        b.toggles.selective_damping = false;
        b.tick();
        check(Access::warning(b).empty(), "repaired profile clears warning memo after preflight");
    }
}
} // namespace

int main() {
    try {
        configuration_rejections();
        horizon_rejections();
        accepted_boundaries();
    } catch (const std::exception& e) {
        ++failed;
        std::printf("FAIL: unexpected exception: %s\n", e.what());
    }
    std::printf("tick preflight: %d passed, %d failed\n", passed, failed);
    return failed ? 1 : 0;
}
