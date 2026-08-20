#pragma once
//
// model/snapshot.h — the scale-generic published snapshot (SPEC_NATIVE_REBUILD_R0R1 §4.4).
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

#include <cstdint>
#include <string>
#include <variant>

namespace ftd::native {

// The Scale-0 observation payload is exactly today's snapshot content.
using Scale0Snapshot = UiSnapshot;

// The Scale-1 (ParticleEngine) observation payload. Scale 1 needs no telemetry
// scheduler: the adapter fills this directly from ParticleEngine::diagnostics()
// each boundary. Deliberately small — it carries only what the status bar and a
// future Scale-1 panel read.
struct Scale1Snapshot {
    int          particle_count = 0;
    double       total_energy = 0.0;
    double       total_ke = 0.0;
    double       total_pe = 0.0;
    std::string  status;

    // Click-to-inspect readout (InspectParticle1). insp_present=false means
    // nothing is currently picked; the adapter's observe() fills these from the
    // selected particle each boundary the selection is re-issued (live data).
    bool         insp_present = false;
    int          insp_index = -1;
    int          insp_charge = 0;
    bool         insp_locked = false;
    double       insp_pos[3] = {0.0, 0.0, 0.0};
    double       insp_vel[3] = {0.0, 0.0, 0.0};
};

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
