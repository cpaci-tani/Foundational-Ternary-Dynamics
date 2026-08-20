#include "native_desktop/ui_snapshot_builder.h"

#include "ftd/render_bridge.h"

namespace ftd::native_desktop {

void build_snapshot(ftd::RenderBridge& bridge,
                    const ftd::NativeTelemetryScheduler::CachedView* cached,
                    const DataNeeds& needs, UiSnapshot& out) {
    out.frame.tick = bridge.current_tick();
    out.frame.lattice_size = bridge.lattice().size();
    out.frame.flux_boundary = static_cast<int>(bridge.toggles.flux_boundary);
    out.energy_ledger = bridge.energy_ledger();
    out.term_toggles = bridge.toggles;
    out.knobs.lattice_size = bridge.lattice().size();
    out.knobs.dt = bridge.dt();
    out.knobs.sor_iterations = bridge.sor_iterations();
    out.knobs.genesis_threshold_override = bridge.genesis_threshold_override;
    out.knobs.manifest_scale_override = bridge.manifest_scale_override;
    out.knobs.manifest_use_temperature = bridge.manifest_use_temperature;
    out.env.interactive_gpu_mode = bridge.interactive_gpu_mode();
    switch (bridge.backend().kind()) {
        case ftd::Backend::Kind::Cpu:
            out.env.backend = BackendKindUi::Cpu;
            break;
        case ftd::Backend::Kind::Gpu:
            out.env.backend = BackendKindUi::Gpu;
            break;
        default:
            out.env.backend = BackendKindUi::Unknown;
            break;
    }
    out.env.thread_count = 1;
    out.demand = needs;
    if (cached) out.telemetry = cached->snapshot;
    (void)needs;
}

}  // namespace ftd::native_desktop
