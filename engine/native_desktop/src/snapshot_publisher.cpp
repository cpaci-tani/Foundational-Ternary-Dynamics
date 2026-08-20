#include "native_desktop/snapshot_publisher.h"

#include <utility>

namespace ftd::native_desktop {

void SnapshotPublisher::publish(UiSnapshot snapshot) {
    auto next = std::make_shared<const UiSnapshot>(std::move(snapshot));
    std::lock_guard<std::mutex> lock(mu_);
    latest_ = std::move(next);
}

std::shared_ptr<const UiSnapshot> SnapshotPublisher::acquire() const {
    std::lock_guard<std::mutex> lock(mu_);
    return latest_;
}

}  // namespace ftd::native_desktop
