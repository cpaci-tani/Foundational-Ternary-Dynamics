#pragma once

#include "native/field_slice.h"
#include "native/knot_snapshot.h"
#include "native/native_frame.h"
#include "native/spectrum.h"
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

// One Moore-neighbour readout cell (26 sites around a picked voxel). Filled
// synchronously by observe_on_bridge() when an InspectNeighbors observation is
// served — native reads are synchronous, so all 26 are gathered in one boundary
// (no async budget). `shell` = |dx|+|dy|+|dz| ∈ {1,2,3} classes the site as a
// face / edge / corner neighbour; dx/dy/dz ∈ {-1,0,+1}. `present` records that
// the read succeeded (always true on the periodic lattice); a genuinely void
// site is state==0 with flux_mag≈0 — the UI shows it blank, never fabricated.
struct NeighborCell {
    int     dx = 0, dy = 0, dz = 0;
    int     shell = 0;
    int8_t  state = 0;
    double  flux_mag = 0.0;
    bool    locked = false;
    int32_t particle_id = -1;
    bool    present = false;
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
    NeighborCell neighbors[26];
    bool voxel_present = false;
    bool force_present = false;
    bool neighbors_present = false;
    ContinuitySnapshot continuity;
    ChargeSumResult charge_sum;
    ftd::VisualFieldSample field_sample;
    ftd::TermToggles term_toggles;
    BridgeKnobs knobs;
    EnvInfo env;
    SpectrumResult spectrum;         // flux E(k) (filled when demand.spectrum)
    bool spectrum_present = false;
    FieldSliceResult slices[SLICE_PLANES];  // yz/xz/xy centre slices (demand.slice)
    bool slices_present = false;
    KnotSnapshot knots;              // engine knot-tracker telemetry (demand.knots)
    bool knots_present = false;
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
