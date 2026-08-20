#include "native/command_applier.h"
#include "native/engine_session.h"
#include "native/ui_snapshot_builder.h"

#include "ftd/native_telemetry_scheduler.h"
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/term_toggles.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ftd::native {
namespace {

void record_change(ParameterJournal& journal, int tick, std::string key,
                   JValue requested, const RenderBridge& bridge,
                   const JValue& old_value) {
    JournalEntry entry;
    entry.tick_applied = tick;
    entry.key = std::move(key);
    entry.old_value = old_value;
    entry.requested = std::move(requested);
    entry.applied = read_journal_key(bridge, entry.key);
    journal.append(std::move(entry));
}

bool scenario_known(const std::string& id) {
    for (const auto name : ftd::scale0_scenario_ids()) {
        if (name == id) return true;
    }
    return false;
}

void journal_toggle_diff(ParameterJournal& journal, int tick,
                         const ftd::TermToggles& before,
                         const RenderBridge& after) {
    for (const auto& spec : ftd::TOGGLE_SPECS) {
        const bool old_v = before.*(spec.field);
        const bool new_v = after.toggles.*(spec.field);
        if (old_v == new_v) continue;
        JValue requested;
        requested.kind = JKind::Bool;
        requested.b = new_v;
        JValue old_value;
        old_value.kind = JKind::Bool;
        old_value.b = old_v;
        record_change(journal, tick, std::string("toggles.") + spec.name, requested,
                      after, old_value);
    }
}

int clamp_index(const RenderBridge& bridge, int x, int y, int z) {
    const int n = bridge.lattice().size();
    if (x < 0 || y < 0 || z < 0 || x >= n || y >= n || z >= n) return -1;
    return 0;
}

int wrap_coord(int v, int n) {
    if (n <= 0) return 0;
    int r = v % n;
    if (r < 0) r += n;
    return r;
}

void wrap_xyz(const RenderBridge& bridge, int& x, int& y, int& z) {
    const int n = bridge.lattice().size();
    x = wrap_coord(x, n);
    y = wrap_coord(y, n);
    z = wrap_coord(z, n);
}

void journal_harness(ParameterJournal& journal, int tick, std::string key,
                     std::string detail) {
    JournalEntry entry;
    entry.tick_applied = tick;
    entry.key = std::move(key);
    entry.requested.kind = JKind::ScenarioId;
    entry.requested.s = std::move(detail);
    entry.applied = entry.requested;
    journal.append(std::move(entry));
}

}  // namespace

std::string journal_key_for(const UiCommand& command) {
    return std::visit(
        [](const auto& cmd) -> std::string {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, SetToggle>) {
                return std::string("toggles.") + cmd.name;
            } else if constexpr (std::is_same_v<T, SetDt>) {
                return "bridge.dt";
            } else if constexpr (std::is_same_v<T, SetSorIterations>) {
                return "bridge.sor_iterations";
            } else if constexpr (std::is_same_v<T, SetLatticeSize>) {
                return "run.staged_lattice_size";
            } else if constexpr (std::is_same_v<T, LoadScenario>) {
                return "run.scenario";
            } else if constexpr (std::is_same_v<T, SetBoundary>) {
                return "toggles.flux_boundary";
            } else if constexpr (std::is_same_v<T, SetBoolConfig>) {
                return "bridge.manifest_use_temperature";
            } else if constexpr (std::is_same_v<T, InjectWavepacket>) {
                return "harness.inject_wavepacket";
            } else if constexpr (std::is_same_v<T, InjectFluxAdd>) {
                return "harness.inject_flux_add";
            } else if constexpr (std::is_same_v<T, CreateEntangledPair>) {
                return "harness.create_entangled_pair";
            } else if constexpr (std::is_same_v<T, ClearField>) {
                return "harness.clear_field";
            } else if constexpr (std::is_same_v<T, SeedRandomFlux>) {
                return "harness.seed_random_flux";
            } else {
                return {};
            }
        },
        command);
}

ApplyResult apply_mutation(NativeEngineSession& session, const UiCommand& command,
                           ParameterJournal& journal) {
    QueuedCommand item;
    item.command = command;
    LoopControl loop;
    return apply_mutation_on_bridge(session.debug_bridge(), &session, item, journal,
                                    session.debug_bridge().current_tick(), loop);
}

ApplyResult apply_mutation_on_bridge(ftd::RenderBridge& bridge,
                                     NativeEngineSession* session,
                                     const QueuedCommand& item,
                                     ParameterJournal& journal, int tick_applied,
                                     LoopControl& loop) {
    ApplyResult result;
    result.sequence = item.seq;
    result.ok = true;

    const auto fail = [&](int code, std::string message) {
        result.ok = false;
        result.error_code = code;
        result.message = std::move(message);
        return result;
    };

    std::visit(
        [&](const auto& cmd) {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, Pause>) {
                loop.pause = true;
                loop.run = false;
            } else if constexpr (std::is_same_v<T, Run>) {
                loop.pause = false;
                loop.run = true;
            } else if constexpr (std::is_same_v<T, Step>) {
                loop.pending_steps += cmd.ticks > 0 ? cmd.ticks : 1;
            } else if constexpr (std::is_same_v<T, SetToggle>) {
                const auto* spec = ftd::term_toggles_detail::find_spec(cmd.name);
                const std::string key = std::string("toggles.") + cmd.name;
                JValue requested;
                requested.kind = JKind::Bool;
                requested.b = cmd.value;
                if (!spec) {
                    record_change(journal, tick_applied, key, requested, bridge,
                                  read_journal_key(bridge, key));
                    result = fail(1, "unresolved toggle name: " + cmd.name);
                    return;
                }
                const auto old = read_journal_key(bridge, key);
                bridge.toggles.*(spec->field) = cmd.value;
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetToggleProfile>) {
                const auto before = bridge.toggles;
                bridge.toggles = cmd.profile;
                journal_toggle_diff(journal, tick_applied, before, bridge);
            } else if constexpr (std::is_same_v<T, SetDouble>) {
                std::string key;
                JValue requested;
                requested.kind = JKind::Double;
                requested.d = cmd.value;
                switch (cmd.key) {
                    case DoubleKey::langevin_T:
                        key = "toggles.langevin_T";
                        break;
                    case DoubleKey::langevin_gamma:
                        key = "toggles.langevin_gamma";
                        break;
                    case DoubleKey::coulomb_charge_coupling:
                        key = "toggles.coulomb_charge_coupling";
                        break;
                    case DoubleKey::coulomb_source_scale:
                        key = "toggles.coulomb_source_scale";
                        break;
                    case DoubleKey::omega0:
                        key = "toggles.omega0";
                        break;
                    case DoubleKey::kinetic_drain:
                        key = "toggles.kinetic_drain";
                        break;
                    case DoubleKey::genesis_threshold_override:
                        key = "bridge.genesis_threshold_override";
                        break;
                    case DoubleKey::manifest_scale_override:
                        key = "bridge.manifest_scale_override";
                        break;
                }
                const auto old = read_journal_key(bridge, key);
                switch (cmd.key) {
                    case DoubleKey::langevin_T:
                        bridge.toggles.langevin_T = cmd.value;
                        break;
                    case DoubleKey::langevin_gamma:
                        bridge.toggles.langevin_gamma = cmd.value;
                        break;
                    case DoubleKey::coulomb_charge_coupling:
                        bridge.toggles.coulomb_charge_coupling = cmd.value;
                        break;
                    case DoubleKey::coulomb_source_scale:
                        bridge.toggles.coulomb_source_scale = cmd.value;
                        break;
                    case DoubleKey::omega0:
                        bridge.toggles.omega0 = cmd.value;
                        break;
                    case DoubleKey::kinetic_drain:
                        bridge.toggles.kinetic_drain = cmd.value;
                        break;
                    case DoubleKey::genesis_threshold_override:
                        bridge.genesis_threshold_override = cmd.value;
                        break;
                    case DoubleKey::manifest_scale_override:
                        bridge.manifest_scale_override = cmd.value;
                        break;
                }
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetEnum>) {
                const bool bcc = cmd.key == EnumKey::bcc_stencil;
                const std::string key =
                    bcc ? "toggles.bcc_stencil" : "toggles.langevin_site_filter";
                JValue requested;
                requested.kind = JKind::Enum;
                requested.e = cmd.value;
                const auto old = read_journal_key(bridge, key);
                if (bcc) {
                    bridge.toggles.bcc_stencil = static_cast<ftd::BccStencilMode>(cmd.value);
                } else {
                    bridge.toggles.langevin_site_filter =
                        static_cast<ftd::SiteClass>(cmd.value);
                }
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetUInt>) {
                const std::string key = "toggles.langevin_seed";
                JValue requested;
                requested.kind = JKind::UInt;
                requested.u = cmd.value;
                const auto old = read_journal_key(bridge, key);
                bridge.toggles.langevin_seed = cmd.value;
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetBoolConfig>) {
                const std::string key = "bridge.manifest_use_temperature";
                JValue requested;
                requested.kind = JKind::Bool;
                requested.b = cmd.value;
                const auto old = read_journal_key(bridge, key);
                bridge.manifest_use_temperature = cmd.value;
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetBoundary>) {
                const std::string key = "toggles.flux_boundary";
                JValue requested;
                requested.kind = JKind::Boundary;
                requested.e = static_cast<int>(cmd.mode);
                const auto old = read_journal_key(bridge, key);
                bridge.toggles.flux_boundary = cmd.mode;
                if (session) session->set_flux_boundary(static_cast<int>(cmd.mode));
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetDt>) {
                const std::string key = "bridge.dt";
                JValue requested;
                requested.kind = JKind::Double;
                requested.d = cmd.dt;
                const auto old = read_journal_key(bridge, key);
                bridge.set_dt(cmd.dt);
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetSorIterations>) {
                const std::string key = "bridge.sor_iterations";
                JValue requested;
                requested.kind = JKind::Enum;
                requested.e = cmd.n;
                const auto old = read_journal_key(bridge, key);
                bridge.set_sor_iterations(cmd.n);
                record_change(journal, tick_applied, key, requested, bridge, old);
            } else if constexpr (std::is_same_v<T, SetLatticeSize>) {
                JValue requested;
                requested.kind = JKind::Enum;
                requested.e = cmd.n;
                if (session) session->stage_lattice_size(cmd.n);
                JournalEntry entry;
                entry.tick_applied = tick_applied;
                entry.key = "run.staged_lattice_size";
                entry.requested = requested;
                entry.applied = requested;
                journal.append(std::move(entry));
            } else if constexpr (std::is_same_v<T, ApplyReboot>) {
                if (!session) {
                    result = fail(static_cast<int>(ReloadStatus::BackendRecreationFailed),
                                  "ApplyReboot requires a session");
                    return;
                }
                const bool was_interop = session->interop_enabled();
                try {
                    session->set_lattice_size(session->staged_lattice_size());
                    ReloadResult reload;
                    reload.status = was_interop ? ReloadStatus::InteropReimportRequired
                                                : ReloadStatus::Success;
                    session->set_last_reload(reload);
                } catch (const std::exception& ex) {
                    ReloadResult reload;
                    reload.status = ReloadStatus::BackendRecreationFailed;
                    reload.message = ex.what();
                    session->set_last_reload(reload);
                    result = fail(static_cast<int>(reload.status), ex.what());
                }
            } else if constexpr (std::is_same_v<T, LoadScenario>) {
                if (!session) {
                    result = fail(static_cast<int>(ReloadStatus::UnknownScenario),
                                  "LoadScenario requires a session");
                    return;
                }
                if (!scenario_known(cmd.id)) {
                    ReloadResult reload;
                    reload.status = ReloadStatus::UnknownScenario;
                    reload.message = "unknown scenario: " + cmd.id;
                    session->set_last_reload(reload);
                    result = fail(static_cast<int>(reload.status), reload.message);
                    return;
                }
                const auto before = bridge.toggles;
                JValue requested;
                requested.kind = JKind::ScenarioId;
                requested.s = cmd.id;
                const bool was_interop = session->interop_enabled();
                try {
                    session->load_scenario(cmd.id);
                    record_change(journal, tick_applied, "run.scenario", requested,
                                  session->debug_bridge(), requested);
                    journal_toggle_diff(journal, tick_applied, before,
                                        session->debug_bridge());
                    ReloadResult reload;
                    reload.status = was_interop ? ReloadStatus::InteropReimportRequired
                                                : ReloadStatus::Success;
                    session->set_last_reload(reload);
                } catch (const std::exception& ex) {
                    ReloadResult reload;
                    reload.status = ReloadStatus::ValidationRejected;
                    reload.message = ex.what();
                    session->set_last_reload(reload);
                    result = fail(static_cast<int>(reload.status), ex.what());
                }
            } else if constexpr (std::is_same_v<T, ResetToDefaults>) {
                const auto before = bridge.toggles;
                bridge.toggles = ftd::TermToggles{};
                journal_toggle_diff(journal, tick_applied, before, bridge);
            } else if constexpr (std::is_same_v<T, InjectWavepacket>) {
                int x = cmd.x;
                int y = cmd.y;
                int z = cmd.z;
                wrap_xyz(bridge, x, y, z);
                const int8_t state = cmd.state >= 0 ? int8_t{1} : int8_t{-1};
                bridge.inject_wavepacket(x, y, z, state);
                journal_harness(journal, tick_applied, "harness.inject_wavepacket",
                                std::to_string(x) + "," + std::to_string(y) + ","
                                    + std::to_string(z) + "," + std::to_string(state));
            } else if constexpr (std::is_same_v<T, InjectFluxAdd>) {
                int x = cmd.x;
                int y = cmd.y;
                int z = cmd.z;
                wrap_xyz(bridge, x, y, z);
                bridge.inject_flux_add(x, y, z, Vec3(cmd.fx, cmd.fy, cmd.fz));
                journal_harness(journal, tick_applied, "harness.inject_flux_add",
                                std::to_string(x) + "," + std::to_string(y) + ","
                                    + std::to_string(z));
            } else if constexpr (std::is_same_v<T, CreateEntangledPair>) {
                int x = cmd.x;
                int y = cmd.y;
                int z = cmd.z;
                wrap_xyz(bridge, x, y, z);
                bridge.create_entangled_pair(x, y, z, Vec3(cmd.fx, cmd.fy, cmd.fz));
                journal_harness(journal, tick_applied, "harness.create_entangled_pair",
                                std::to_string(x) + "," + std::to_string(y) + ","
                                    + std::to_string(z));
            } else if constexpr (std::is_same_v<T, ClearField>) {
                bridge.clearField();
                journal_harness(journal, tick_applied, "harness.clear_field", "clear");
            } else if constexpr (std::is_same_v<T, SeedRandomFlux>) {
                bridge.seedRandomFlux();
                journal_harness(journal, tick_applied, "harness.seed_random_flux",
                                "non-replayable");
            }
        },
        item.command);
    return result;
}

ObservationResult observe_on_bridge(ftd::RenderBridge& bridge, const UiCommand& command,
                                    UiSnapshot& snapshot, UiBoundaryState& state) {
    ObservationResult result;
    result.status = ObservationStatus::Ready;

    std::visit(
        [&](const auto& cmd) {
            using T = std::decay_t<decltype(cmd)>;
            if constexpr (std::is_same_v<T, InspectVoxel>) {
                if (clamp_index(bridge, cmd.x, cmd.y, cmd.z) < 0) {
                    result.status = ObservationStatus::Rejected;
                    result.message = "voxel out of range";
                    return;
                }
                snapshot.voxel = bridge.inspect_voxel(cmd.x, cmd.y, cmd.z);
                snapshot.voxel_present = true;
            } else if constexpr (std::is_same_v<T, InspectForce>) {
                if (clamp_index(bridge, cmd.x, cmd.y, cmd.z) < 0) {
                    result.status = ObservationStatus::Rejected;
                    result.message = "voxel out of range";
                    return;
                }
                const auto force = bridge.inspect_force(cmd.x, cmd.y, cmd.z);
                snapshot.force.f_coulomb = force.f_coulomb;
                snapshot.force.f_strong = force.f_strong;
                snapshot.force.f_magnetic = force.f_magnetic;
                snapshot.force.f_gravity = force.f_gravity;
                snapshot.force.f_exchange = force.f_exchange;
                snapshot.force_present = true;
            } else if constexpr (std::is_same_v<T, RequestField>) {
                bridge.copy_visual_field_sample(cmd.kind, cmd.stride, snapshot.field_sample);
            } else if constexpr (std::is_same_v<T, RequestChargeSum>) {
                snapshot.charge_sum.present = true;
                snapshot.charge_sum.value = bridge.charge_sum();
                snapshot.charge_sum.synchronization_cost =
                    bridge.backend().kind() == ftd::Backend::Kind::Gpu
                    && bridge.interactive_gpu_mode();
            } else if constexpr (std::is_same_v<T, RequestContinuity>) {
                const auto continuity = bridge.continuity_step();
                if (continuity.L == 0) {
                    snapshot.continuity.status = ObservationStatus::PendingAfterHostUpload;
                    snapshot.continuity.L = 0;
                    snapshot.continuity.synchronized = false;
                    result.status = ObservationStatus::PendingAfterHostUpload;
                    state.deferred_continuity = command;
                    return;
                }
                snapshot.continuity.status = ObservationStatus::Ready;
                snapshot.continuity.L = continuity.L;
                snapshot.continuity.synchronized = true;
                state.deferred_continuity.reset();
            } else if constexpr (std::is_same_v<T, SetTelemetryDemand>) {
                state.demand = cmd.needs;
                // Phase 0B–5: keep NativeTelemetryScheduler::Demand::enabled_mask at 0.
            }
        },
        command);
    return result;
}

void process_ui_boundary(ftd::RenderBridge& bridge, NativeEngineSession* session,
                         CommandQueue& queue, UiBoundaryState& state) {
    // LoadScenario / ApplyReboot call NativeEngineSession::boot(), which
    // destroys the RenderBridge this function was originally given. Always
    // re-resolve through the session so flush / observe / snapshot never
    // touch a dangling reference.
    auto live = [&]() -> ftd::RenderBridge& {
        return session ? session->debug_bridge() : bridge;
    };

    live().bind_sim_thread();
    const int apply_tick = live().current_tick();
    state.apply_tick = apply_tick;
    if (session) {
        state.loop = session->loop_control();
        if (state.staged_lattice_size == 0) {
            state.staged_lattice_size = session->lattice_size();
        }
    }

    auto items = queue.drain();
    std::vector<QueuedCommand> observations;
    observations.reserve(items.size());
    ParameterJournal* journal = state.journal;
    ParameterJournal local_journal;
    if (!journal) journal = &local_journal;
    state.applied_reload = false;
    state.applied_host_write = false;

    for (const auto& item : items) {
        if (is_observation_command(item.command)) {
            observations.push_back(item);
            continue;
        }
        const bool reload = std::holds_alternative<LoadScenario>(item.command)
            || std::holds_alternative<ApplyReboot>(item.command);
        if (reload) state.applied_reload = true;
        if (is_harness_command(item.command)) {
            state.applied_host_write = true;
        }
        const auto applied =
            apply_mutation_on_bridge(live(), session, item, *journal, apply_tick,
                                     state.loop);
        if (applied.ok) {
            state.last_applied_seq = item.seq;
            if (reload) live().bind_sim_thread();
        }
    }
    if (session) session->set_loop_control(state.loop);

    live().backend().flush_host_mutations();

    UiSnapshot snapshot;
    if (state.deferred_continuity) {
        bool already = false;
        for (const auto& item : observations) {
            if (std::holds_alternative<RequestContinuity>(item.command)) {
                already = true;
                break;
            }
        }
        if (!already) {
            observations.insert(observations.begin(),
                                QueuedCommand{0, *state.deferred_continuity});
        }
    }

    for (const auto& item : observations) {
        const auto observed = observe_on_bridge(live(), item.command, snapshot, state);
        if (observed.status == ObservationStatus::Ready) {
            state.last_applied_seq = std::max(state.last_applied_seq, item.seq);
        }
    }

    if (state.did_tick && state.scheduler) {
        state.scheduler->on_tick_complete(live());
        (void)state.scheduler->pump(live());
    }

    const ftd::NativeTelemetryScheduler::CachedView cached =
        state.scheduler ? state.scheduler->latest()
                        : ftd::NativeTelemetryScheduler::CachedView{};
    build_snapshot(live(), state.scheduler ? &cached : nullptr, state.demand,
                   snapshot);
    snapshot.last_applied_seq = state.last_applied_seq;
    snapshot.seq = ++state.snapshot_seq;
    if (session) {
        snapshot.frame.scenario = session->scenario();
        snapshot.frame.backend = session->backend_name();
        snapshot.frame.status = session->status();
        snapshot.frame.tick = session->current_tick();
        snapshot.frame.lattice_size = session->lattice_size();
        snapshot.frame.total_manifested = session->last_total_manifested();
    }
    if (state.publisher) state.publisher->publish(std::move(snapshot));
}

}  // namespace ftd::native
