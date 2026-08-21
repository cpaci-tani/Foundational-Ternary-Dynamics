#include "native/command_queue.h"

#include <utility>

namespace ftd::native {
namespace {

template <typename T>
void coalesce_keep_last(std::vector<QueuedCommand>& items) {
    bool seen = false;
    for (int i = static_cast<int>(items.size()) - 1; i >= 0; --i) {
        if (!std::holds_alternative<T>(items[static_cast<std::size_t>(i)].command)) {
            continue;
        }
        if (seen) {
            items.erase(items.begin() + i);
        } else {
            seen = true;
        }
    }
}

}  // namespace

std::uint64_t CommandQueue::push(UiCommand command) {
    std::lock_guard<std::mutex> lock(mu_);
    const std::uint64_t seq = next_seq_++;
    pending_.push_back(QueuedCommand{seq, std::move(command)});
    return seq;
}

std::vector<QueuedCommand> CommandQueue::drain() {
    std::vector<QueuedCommand> items;
    {
        std::lock_guard<std::mutex> lock(mu_);
        items.swap(pending_);
    }
    coalesce_keep_last<RequestField>(items);
    coalesce_keep_last<SetTelemetryDemand>(items);
    coalesce_keep_last<InspectVoxel>(items);
    coalesce_keep_last<InspectForce>(items);
    coalesce_keep_last<InspectNeighbors>(items);
    return items;
}

}  // namespace ftd::native
