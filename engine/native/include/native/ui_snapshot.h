#pragma once

#include "native/native_frame.h"
#include "native/ui_demand.h"
#include "native/ui_result.h"

#include "ftd/render_bridge_diagnostics.h"
#include "ftd/telemetry_snapshot.h"
#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"

#include <cstdint>
#include <string>

namespace ftd::native {

enum class BackendKindUi { Cpu = 0, Gpu = 1, Unknown = 2 };

struct EnvInfo {
    BackendKindUi backend = BackendKindUi::Unknown;
    bool interactive_gpu_mode = false;
    int thread_count = 1;
};

struct UiForceDiag {
    Vec3 f_coulomb;
    Vec3 f_strong;
    Vec3 f_magnetic;
    Vec3 f_gravity;
    Vec3 f_exchange;
};

struct ContinuitySnapshot {
    ObservationStatus status = ObservationStatus::Rejected;
    int L = 0;
    bool synchronized = false;
};

struct ChargeSumResult {
    bool present = false;
    long long value = 0;
    bool synchronization_cost = false;
};

struct BridgeKnobs {
    int lattice_size = 0;
    double dt = 1.0;
    int sor_iterations = 1;
    double genesis_threshold_override = -1.0;
    double manifest_scale_override = -1.0;
    bool manifest_use_temperature = false;
};

struct UiSnapshot {
    NativeFrame frame;
    ftd::TelemetrySnapshot telemetry;
    ftd::EnergyLedger energy_ledger;
    ftd::VoxelInspection voxel;
    UiForceDiag force;
    bool voxel_present = false;
    bool force_present = false;
    ContinuitySnapshot continuity;
    ChargeSumResult charge_sum;
    ftd::VisualFieldSample field_sample;
    ftd::TermToggles term_toggles;
    BridgeKnobs knobs;
    EnvInfo env;
    DataNeeds demand;
    std::uint64_t last_applied_seq = 0;
    std::uint64_t seq = 0;

    std::uint64_t checksum() const {
        std::uint64_t h = 14695981039346656037ull;
        const auto mix = [&](std::uint64_t v) {
            h ^= v;
            h *= 1099511628211ull;
        };
        mix(seq);
        mix(last_applied_seq);
        mix(static_cast<std::uint64_t>(frame.tick));
        mix(static_cast<std::uint64_t>(energy_ledger.updates));
        mix(static_cast<std::uint64_t>(knobs.sor_iterations));
        mix(static_cast<std::uint64_t>(term_toggles.larmor_radiation));
        return h;
    }
};

}  // namespace ftd::native
