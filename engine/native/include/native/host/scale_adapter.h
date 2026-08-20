#pragma once
//
// host/scale_adapter.h — the uniform contract every scale implements
// (SPEC_NATIVE_REBUILD_R0R1 §4.1).
//
// Scale 0's adapter is the old NativeEngineSession/command_applier/
// ui_snapshot_builder logic re-homed behind this interface; each further scale
// (ParticleEngine, AtomEngine, CosmicEngine — all `: ScaleEngine`) is one more
// file implementing the same methods. The host talks ONLY to this interface and
// the model types — never to a concrete engine or scale command/snapshot type.
//
// SEAM NOTE (RenderBridge ≠ ScaleEngine): Scale 0's engine, RenderBridge,
// predates ScaleEngine and does not derive from it, so engine() returns nullptr
// for Scale 0. The adapter therefore OWNS its concrete engine internally rather
// than handing a `unique_ptr<ScaleEngine>` up to the host. Scale-1+ engines are
// real ScaleEngines and their adapters can return a live engine() pointer.
//
#include "native/model/commands.h"     // ScalePayload
#include "native/model/draw_list.h"    // DrawList (defined; not yet the live render path)
#include "native/model/snapshot.h"     // ScaleSnapshot, LoopControl, NativeFrame
#include "native/ui_demand.h"          // DataNeeds
#include "native/ui_result.h"          // ApplyResult, ReloadStatus, LoopControl

#include <cstdint>
#include <memory>
#include <string>

namespace ftd {
class ScaleEngine;
struct ScenarioMeta;
}

namespace ftd::native {

class ParameterJournal;

// Outcome of one adapter boot(): distinguishes the two dispatch_scenario()==false
// meanings (W9). `scenario` is the id actually in effect afterward — it may
// differ from the requested id when a validation-reject forced a known-good
// re-boot.
struct BootReport {
    ReloadStatus status = ReloadStatus::Success;
    std::string  message;
    std::string  scenario;
    std::string  status_line;
};

class ScaleAdapter {
public:
    virtual ~ScaleAdapter() = default;

    // ── identity ──
    virtual int scale_level() const = 0;
    virtual const char* scale_name() const = 0;
    // Layer-0 engine handle IFF the engine derives from ScaleEngine. nullptr for
    // Scale 0 (RenderBridge). The host never calls this; it is the contract seam
    // for future ScaleEngine-based scales.
    virtual ftd::ScaleEngine* engine() { return nullptr; }

    // ── lifecycle ──
    // (Re)build the scale's engine for this scenario + config. Implements the W9
    // distinction internally and reports it via `out` (unknown id vs
    // validation-reject → known-good re-boot).
    virtual void boot(const ftd::ScenarioMeta& meta, const RunConfig& cfg,
                      BootReport& out) = 0;

    // ── sim thread ──
    virtual void bind_sim_thread() = 0;
    virtual void tick() = 0;
    virtual int  current_tick() const = 0;

    // ── scale-payload command handling (core commands never reach here) ──
    virtual bool is_observation(const ScalePayload& payload) const = 0;
    virtual bool is_host_write(const ScalePayload& payload) const = 0;   // harness → refresh frame
    virtual ApplyResult apply(const ScalePayload& payload, ParameterJournal& journal,
                              int apply_tick, LoopControl& loop) = 0;
    virtual void flush_writes() = 0;

    // ── boundary observation (scale snapshot accumulated in the adapter) ──
    virtual void begin_boundary() = 0;
    virtual bool observe(const ScalePayload& payload) = 0;   // returns "observation ready"
    virtual void on_tick_complete() = 0;
    virtual void build_snapshot(const DataNeeds& needs) = 0;
    virtual ScaleSnapshot take_scale_snapshot() = 0;

    // ── render frame (legacy NativeFrame path retained at R1 step 1) ──
    virtual NativeFrame capture() = 0;

    // ── common-core metadata for the published HostSnapshot ──
    virtual const char* backend_name() const = 0;
    virtual std::string status() const = 0;
    virtual std::string scenario_id() const = 0;
    virtual int lattice_size() const = 0;
    virtual std::uint32_t last_total_manifested() const = 0;

    // ── interop lifecycle (no-op for scales without a CUDA↔D3D12 device path) ──
    virtual bool try_enable_interop(void* /*buf*/, std::uint64_t /*bytes*/,
                                    void* /*fence*/) { return false; }
    virtual bool interop_enabled() const { return false; }
    virtual bool request_interop_gather(std::uint64_t /*fence_value*/) { return false; }
    virtual int  poll_interop_particle_count() { return -1; }
};

// The only place that knows the concrete engine/adapter types; everything else
// is generic. Defined in host/adapters/scale0_adapter.cpp for now.
std::unique_ptr<ScaleAdapter> make_scale_adapter(int scale_level);

}  // namespace ftd::native
