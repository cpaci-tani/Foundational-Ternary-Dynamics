#include "ftd/eft/history_event_journal.h"

#include "ftd/voxel.h"

#include <cmath>
#include <mutex>

namespace ftd {
namespace eft {

namespace {

std::int8_t sign_with_floor(double value) {
    constexpr double floor = 1e-14;
    if (value > floor) return 1;
    if (value < -floor) return -1;
    return 0;
}

}  // namespace

HistorySiteState capture_history_site(int index, const Voxel& voxel) {
    HistorySiteState out;
    out.index = index;
    out.state = voxel.state;
    out.chirality_sign = sign_with_floor(voxel.chirality_density());
    out.flux = voxel.flux;
    out.flux_L = voxel.flux_L;
    out.flux_R = voxel.flux_R;
    out.voxel = voxel;
    return out;
}

struct HistoryEventJournal::Impl {
    mutable std::mutex mutex;
    bool enabled = false;
    std::vector<HistoryEvent> events;
};

HistoryEventJournal::HistoryEventJournal()
    : impl_(std::make_unique<Impl>()) {}

HistoryEventJournal::~HistoryEventJournal() = default;

void HistoryEventJournal::set_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->enabled = enabled;
    impl_->events.clear();
}

bool HistoryEventJournal::enabled() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->enabled;
}

void HistoryEventJournal::clear() {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->events.clear();
}

void HistoryEventJournal::record(const HistoryEvent& event) {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    if (impl_->enabled) impl_->events.push_back(event);
}

std::vector<HistoryEvent> HistoryEventJournal::snapshot() const {
    std::lock_guard<std::mutex> lock(impl_->mutex);
    return impl_->events;
}

}  // namespace eft
}  // namespace ftd
