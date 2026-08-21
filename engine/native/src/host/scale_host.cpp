// host/scale_host.cpp — the scale-generic session host.
//
// BOUNDARY: this file names NO concrete scale type (no Scale-0 engine, toggle
// struct, or Scale-0 command/snapshot type). It orchestrates through
// ScaleAdapter + the model types only. The active adapter is built by
// make_scale_adapter() (the one factory that knows concrete types) and owns its
// concrete engine.

#include "native/host/scale_host.h"

#include "native/scenario_catalog.h"  // ftd::native::ScenarioMeta + find_scenario_meta
                                       // (self-contained native catalog; the engine's
                                       // ftd/scenario_meta.h is an untracked concurrent-
                                       // session file this rebuild must not depend on).

#include "native/ui_journal.h"

#include <exception>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ftd::native {

ScaleHost::ScaleHost(HostOptions options) : options_(std::move(options)) {
    active_scale_ = options_.scale_level;
    adapter_ = make_scale_adapter(active_scale_);

    ReloadResult r;
    reload_to(options_.scenario, options_.run, r);
    last_reload_ = r;

    // Prime paused and publish one boundary before the sim thread starts
    // (mirrors the old NativeEngineSession constructor).
    loop_.pause = true;
    loop_.run = false;
    CommandBus boot_bus;
    process_ui_boundary(boot_bus);
}

ScaleHost::~ScaleHost() = default;

void ScaleHost::reload_to(const std::string& scenario_id, const RunConfig& cfg,
                          ReloadResult& out) {
    // Resolve a ScenarioMeta by id. Unknown ids are given a minimal synthesized
    // descriptor whose id points at the caller's still-live string, so the
    // adapter's W9 unknown-id path can fire without fabricating metadata.
    const ScenarioMeta* found = find_scenario_meta(scenario_id);
    ScenarioMeta synth{};
    if (!found) {
        synth.id = scenario_id.c_str();
        synth.scale = active_scale_;
    }
    const ScenarioMeta& meta = found ? *found : synth;

    const bool was_interop = adapter_->interop_enabled();
    BootReport report;
    try {
        adapter_->boot(meta, cfg, report);
    } catch (const std::exception& ex) {
        out.status = ReloadStatus::BackendRecreationFailed;
        out.message = ex.what();
        return;
    }

    options_.scenario = report.scenario;  // W9: effective scenario may differ
    options_.run = cfg;

    // Journal the reload as a scale-agnostic entry.
    JournalEntry entry;
    entry.tick_applied = adapter_->current_tick();
    entry.key = "run.scenario";
    entry.requested.kind = JKind::ScenarioId;
    entry.requested.s = scenario_id;
    entry.applied = entry.requested;
    journal_.append(std::move(entry));

    // Preserve the interop-reimport signal (the caller re-imports handles after a
    // reload rebuilt the engine). A boot that already failed keeps its status.
    if (report.status == ReloadStatus::Success && was_interop) {
        out.status = ReloadStatus::InteropReimportRequired;
    } else {
        out.status = report.status;
    }
    out.message = report.message;
}

ReloadResult ScaleHost::switch_scale(int scale_level, std::string scenario,
                                     const RunConfig& cfg) {
    // Rebuilding the adapter tears down any interop the old scale held.
    active_scale_ = scale_level;
    options_.scale_level = scale_level;
    options_.scenario = std::move(scenario);
    options_.run = cfg;
    adapter_ = make_scale_adapter(scale_level);

    ReloadResult rr;
    reload_to(options_.scenario, cfg, rr);
    last_reload_ = rr;
    return rr;
}

TickResult ScaleHost::tick_once() {
    TickResult result;
    try {
        adapter_->tick();
        did_tick_ = true;
        last_tick_ = result;
    } catch (const std::exception& ex) {
        result.ok = false;
        result.message = ex.what();
        last_tick_ = result;
        did_tick_ = false;
    }
    return result;
}

void ScaleHost::consume_pending_step() {
    if (loop_.pending_steps > 0) --loop_.pending_steps;
}

TickResult ScaleHost::process_ui_boundary(CommandBus& bus) {
    adapter_->bind_sim_thread();
    const int apply_tick = adapter_->current_tick();

    applied_reload_ = false;
    applied_host_write_ = false;
    adapter_->begin_boundary();

    auto items = bus.drain();
    std::vector<std::pair<std::uint64_t, ScalePayload>> observations;
    observations.reserve(items.size());

    for (const BusCommand& item : items) {
        if (item.command.is_core()) {
            std::visit(
                [&](const auto& c) {
                    using T = std::decay_t<decltype(c)>;
                    if constexpr (std::is_same_v<T, Pause>) {
                        loop_.pause = true;
                        loop_.run = false;
                        last_applied_seq_ = item.seq;
                    } else if constexpr (std::is_same_v<T, Run>) {
                        loop_.pause = false;
                        loop_.run = true;
                        last_applied_seq_ = item.seq;
                    } else if constexpr (std::is_same_v<T, Step>) {
                        loop_.pending_steps += c.ticks > 0 ? c.ticks : 1;
                        last_applied_seq_ = item.seq;
                    } else if constexpr (std::is_same_v<T, LoadScenario>) {
                        applied_reload_ = true;
                        ReloadResult rr;
                        reload_to(c.id, options_.run, rr);
                        last_reload_ = rr;
                        if (rr.status == ReloadStatus::Success
                            || rr.status == ReloadStatus::InteropReimportRequired) {
                            last_applied_seq_ = item.seq;
                        }
                    } else if constexpr (std::is_same_v<T, SetRunConfig>) {
                        applied_reload_ = true;
                        ReloadResult rr;
                        reload_to(options_.scenario, c.cfg, rr);
                        last_reload_ = rr;
                        last_applied_seq_ = item.seq;
                    } else if constexpr (std::is_same_v<T, SwitchScale>) {
                        last_reload_ = switch_scale(c.scale_level, c.scenario, options_.run);
                        applied_reload_ = true;
                        last_applied_seq_ = item.seq;
                    } else if constexpr (std::is_same_v<T, SetTelemetryDemand>) {
                        demand_ = c.needs;
                        last_applied_seq_ = item.seq;
                    }
                },
                item.command.core);
            // A reload rebuilt the engine; re-bind the sim thread on the new one.
            if (applied_reload_) adapter_->bind_sim_thread();
            continue;
        }

        const ScalePayload& payload = item.command.scale;
        if (adapter_->is_observation(payload)) {
            observations.emplace_back(item.seq, payload);
            continue;
        }
        if (adapter_->is_host_write(payload)) applied_host_write_ = true;
        const ApplyResult r = adapter_->apply(payload, journal_, apply_tick, loop_);
        if (r.ok) last_applied_seq_ = item.seq;
    }

    adapter_->flush_writes();

    for (const auto& [seq, payload] : observations) {
        if (adapter_->observe(payload)) {
            if (seq > last_applied_seq_) last_applied_seq_ = seq;
        }
    }

    if (did_tick_) adapter_->on_tick_complete();
    adapter_->build_snapshot(demand_);

    publish_boundary();
    did_tick_ = false;
    return last_tick_;
}

void ScaleHost::publish_boundary() {
    HostSnapshot snap;
    snap.active_scale = active_scale_;
    snap.tick = adapter_->current_tick();
    snap.loop = loop_;
    snap.backend = adapter_->backend_name();
    snap.scenario = adapter_->scenario_id();
    snap.status = adapter_->status();
    snap.lattice_size = adapter_->lattice_size();
    snap.total_manifested = adapter_->last_total_manifested();
    snap.last_applied_seq = last_applied_seq_;
    snap.seq = ++snapshot_seq_;
    snap.scale = adapter_->take_scale_snapshot();  // opaque per-scale payload
    publisher_.publish(std::move(snap));
}

NativeFrame ScaleHost::capture() { return adapter_->capture(); }

const char* ScaleHost::backend_name() const { return adapter_->backend_name(); }
std::string ScaleHost::status() const { return adapter_->status(); }
std::string ScaleHost::scenario() const { return adapter_->scenario_id(); }
int ScaleHost::lattice_size() const { return adapter_->lattice_size(); }

bool ScaleHost::try_enable_interop(void* buf, std::uint64_t bytes, void* fence) {
    const bool ok = adapter_->try_enable_interop(buf, bytes, fence);
    interop_was_active_ = ok;
    return ok;
}
bool ScaleHost::interop_enabled() const { return adapter_->interop_enabled(); }
bool ScaleHost::request_interop_gather(std::uint64_t fence_value) {
    return adapter_->request_interop_gather(fence_value);
}
int ScaleHost::poll_interop_particle_count() {
    return adapter_->poll_interop_particle_count();
}

}  // namespace ftd::native
