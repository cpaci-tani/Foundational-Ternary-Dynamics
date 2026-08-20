#pragma once

#include "native/command_queue.h"
#include "native/parameter_journal.h"
#include "native/snapshot_publisher.h"
#include "native/ui_result.h"
#include "native/ui_snapshot.h"

#include <cstdint>
#include <optional>
#include <string>

namespace ftd {
class NativeTelemetryScheduler;
class RenderBridge;
}

namespace ftd::native {

class NativeEngineSession;

struct UiBoundaryState {
    SnapshotPublisher* publisher = nullptr;
    ParameterJournal* journal = nullptr;
    ftd::NativeTelemetryScheduler* scheduler = nullptr;
    LoopControl loop;
    DataNeeds demand;
    std::optional<UiCommand> deferred_continuity;
    ReloadResult last_reload;
    TickResult last_tick;
    std::uint64_t snapshot_seq = 0;
    std::uint64_t last_applied_seq = 0;
    int staged_lattice_size = 0;
    int apply_tick = 0;
    bool did_tick = false;
    bool host_upload_this_boundary = false;
    bool applied_reload = false;
    bool applied_host_write = false;
};

ApplyResult apply_mutation(NativeEngineSession& session, const UiCommand& command,
                           ParameterJournal& journal);

ApplyResult apply_mutation_on_bridge(ftd::RenderBridge& bridge,
                                     NativeEngineSession* session,
                                     const QueuedCommand& item,
                                     ParameterJournal& journal, int tick_applied,
                                     LoopControl& loop);

ObservationResult observe_on_bridge(ftd::RenderBridge& bridge, const UiCommand& command,
                                    UiSnapshot& snapshot, UiBoundaryState& state);

void process_ui_boundary(ftd::RenderBridge& bridge, NativeEngineSession* session,
                         CommandQueue& queue, UiBoundaryState& state);

std::string journal_key_for(const UiCommand& command);

}  // namespace ftd::native
