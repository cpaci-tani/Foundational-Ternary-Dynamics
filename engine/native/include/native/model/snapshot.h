#pragma once
//
// model/snapshot.h — the scale-generic published snapshot.
//
// A HostSnapshot has a scale-common core (never hidden: tick, loop, backend,
// identity, sequence bookkeeping) plus one scale-namespaced observation payload.
// The Scale-0 payload REUSES the existing rich native/ui_snapshot.h::UiSnapshot
// verbatim (telemetry + energy ledger + toggles + inspection + field sample), so
// the Scale-0 adapter fills it through the existing build_snapshot()/
// observe_on_bridge() logic with no behavioral change.
//
#include "native/native_frame.h"
#include "native/ui_result.h"     // LoopControl
#include "native/ui_snapshot.h"   // the rich Scale-0 observation payload
#include "ftd/scale1/domain.h"    // shared Scale-1 scientific contract

#include <cstdint>
#include <string>
#include <variant>

namespace ftd::native {

// The Scale-0 observation payload is exactly today's snapshot content.
using Scale0Snapshot = UiSnapshot;

// Native and WASM consume the same versioned Scale-1 payload. Compatibility
// fields used by the current RML surface are mirrors inside this shared type.
using Scale1Snapshot = ftd::Scale1Snapshot;

// Scale2Snapshot, Scale5Snapshot, … arrive as further alternatives below.
using ScaleSnapshot = std::variant<std::monostate, Scale0Snapshot, Scale1Snapshot>;

struct HostSnapshot {
    // ── scale-common core ──
    int           active_scale = 0;
    int           tick = 0;
    LoopControl   loop;
    std::string   backend;
    std::string   scenario;
    std::string   status;
    int           lattice_size = 0;
    std::uint32_t total_manifested = 0;
    std::uint64_t seq = 0;
    std::uint64_t last_applied_seq = 0;

    // ── per-scale observation payload ──
    ScaleSnapshot scale{std::monostate{}};

    const Scale0Snapshot* scale0() const {
        return std::get_if<Scale0Snapshot>(&scale);
    }
    Scale0Snapshot* scale0() {
        return std::get_if<Scale0Snapshot>(&scale);
    }
    const Scale1Snapshot* scale1() const {
        return std::get_if<Scale1Snapshot>(&scale);
    }
    Scale1Snapshot* scale1() {
        return std::get_if<Scale1Snapshot>(&scale);
    }
};

}  // namespace ftd::native
