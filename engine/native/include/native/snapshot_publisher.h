#pragma once

#include "native/ui_snapshot.h"

#include <memory>
#include <mutex>

namespace ftd::native {

class SnapshotPublisher {
public:
    void publish(UiSnapshot snapshot);
    std::shared_ptr<const UiSnapshot> acquire() const;

private:
    mutable std::mutex mu_;
    std::shared_ptr<const UiSnapshot> latest_;
};

}  // namespace ftd::native
