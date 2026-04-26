#pragma once
/**
 * @file bridge_view.h
 * @brief Read-only view alias for RenderBridge — diagnostician contract.
 *
 * ARCH-1 Phase B (CHECKLIST_ENGINE.md): formalises the existing convention
 * that all diagnostic / inspection free functions in `diagnostics_compute.cpp`
 * and `energy_ledger_compute.cpp` take a `const RenderBridge&`.
 *
 * Using `RenderBridgeView` instead of `const RenderBridge&` at API boundaries
 * documents the intent ("this function will not mutate the bridge") and
 * provides a single hook if the underlying type is ever PIMPL'd or replaced
 * by a true value-type view.
 *
 * Usage:
 *   #include "ftd/bridge_view.h"
 *
 *   double my_diagnostic(ftd::RenderBridgeView v) {
 *       const auto& voxels = v.voxels();
 *       // ... read-only access only ...
 *   }
 *
 * This header forward-declares RenderBridge to keep the include footprint
 * minimal — callers that need full access should include `render_bridge.h`.
 */

namespace ftd {

class RenderBridge;

/// Read-only handle to a RenderBridge. Currently a type alias for
/// `const RenderBridge&`; reserved as a customisation point for future
/// PIMPL/value-type-view migrations.
using RenderBridgeView = const RenderBridge&;

} // namespace ftd
