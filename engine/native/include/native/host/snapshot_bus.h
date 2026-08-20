#pragma once
//
// host/snapshot_bus.h — immutable publication of the scale-generic HostSnapshot.
//
// Mirrors the salvaged native/snapshot_publisher.h mechanism (mutex-guarded
// shared_ptr<const> hand-off between the sim thread and the GUI thread) but for
// the HostSnapshot (§4.4) instead of the legacy Scale-0 UiSnapshot. Header-only
// so it coexists with the legacy SnapshotPublisher (still used by
// NativeEngineSession + its tests) without retyping that shared library.
//
#include "native/model/snapshot.h"

#include <memory>
#include <mutex>
#include <utility>

namespace ftd::native {

class SnapshotBus {
public:
    void publish(HostSnapshot snapshot) {
        auto held = std::make_shared<const HostSnapshot>(std::move(snapshot));
        std::lock_guard<std::mutex> lock(mu_);
        latest_ = std::move(held);
    }

    std::shared_ptr<const HostSnapshot> acquire() const {
        std::lock_guard<std::mutex> lock(mu_);
        return latest_;
    }

private:
    mutable std::mutex mu_;
    std::shared_ptr<const HostSnapshot> latest_;
};

}  // namespace ftd::native
