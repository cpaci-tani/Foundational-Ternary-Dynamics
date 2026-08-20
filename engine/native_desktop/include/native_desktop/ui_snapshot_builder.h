#pragma once

#include "native_desktop/ui_demand.h"
#include "native_desktop/ui_snapshot.h"

#include "ftd/native_telemetry_scheduler.h"

namespace ftd {
class RenderBridge;
}

namespace ftd::native_desktop {

void build_snapshot(ftd::RenderBridge& bridge,
                    const ftd::NativeTelemetryScheduler::CachedView* cached,
                    const DataNeeds& needs, UiSnapshot& out);

}  // namespace ftd::native_desktop
