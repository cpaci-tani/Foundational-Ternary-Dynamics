#include "ftd/native_telemetry_scheduler.h"

#include "ftd/render_bridge.h"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace ftd {

const NativeTelemetryScheduler::Demand& NativeTelemetryScheduler::demand() const { return demand_; }

const std::array<std::uint32_t, 4>& NativeTelemetryScheduler::min_interval_ms() const {
        return min_interval_ms_;
    }

std::uint64_t NativeTelemetryScheduler::epoch() const { return epoch_; }

std::uint64_t NativeTelemetryScheduler::source_epoch() const { return source_epoch_; }

std::uint64_t NativeTelemetryScheduler::snapshot_version() const { return snapshot_version_; }

bool NativeTelemetryScheduler::suspended() const { return suspended_; }

bool NativeTelemetryScheduler::restart_required() const { return suspended_; }

const std::string& NativeTelemetryScheduler::suspension_reason() const { return suspension_reason_; }

bool NativeTelemetryScheduler::safe_to_replace() const { return !suspended_ && !in_flight_; }

bool NativeTelemetryScheduler::observation_in_flight() const { return in_flight_; }

bool NativeTelemetryScheduler::has_pending_or_due_observation() const {
        if (suspended_ || demand_.enabled_mask == 0) return false;
        if (in_flight_) return true;
        if (direct_mutation_due_ != Clock::time_point::min()) return true;
        if (!schedule_armed_) return false;
        bool throttled = false;
        return due_mask(Clock::now(), throttled) != 0;
    }

void NativeTelemetryScheduler::abort_and_suspend(const std::string& reason) {
        ++epoch_;
        cache_ = TelemetrySnapshot{};
        group_snapshot_versions_.fill(0);
        publications_.clear();
        invalidations_.clear();
        in_flight_ = false;
        in_flight_mask_ = 0;
        in_flight_epoch_ = 0;
        in_flight_deadline_ = Clock::time_point::min();
        retired_in_flight_ = false;
        schedule_armed_ = false;
        forced_mask_ = 0;
        direct_mutation_due_ = Clock::time_point::min();
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
        suspension_reason_ = reason.empty()
            ? "native GPU telemetry observation failed"
            : reason;
        suspended_ = true;
    }

void NativeTelemetryScheduler::on_client_disconnected() {
        ++epoch_;
        demand_.enabled_mask = 0;
        cache_ = TelemetrySnapshot{};
        group_snapshot_versions_.fill(0);
        publications_.clear();
        invalidations_.clear();
        schedule_armed_ = false;
        forced_mask_ = 0;
        direct_mutation_due_ = Clock::time_point::min();
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
        retired_in_flight_ = in_flight_;
    }

void NativeTelemetryScheduler::set_demand(Demand demand) {
        demand.enabled_mask &= TELEMETRY_ALL;
        for (auto& cadence : demand.every_ticks) {
            cadence = std::clamp(cadence, 1u, kMaxCadenceTicks);
        }

        const std::uint32_t disabled = demand_.enabled_mask & ~demand.enabled_mask;
        if (disabled != 0) {
            cache_.groups &= ~disabled;
            clear_versions(disabled);
            forced_mask_ &= ~disabled;
        }

        demand_ = demand;
        // A newly enabled group (or an empty initial cache) should have a
        // current-state observation even when playback is paused.
        schedule_armed_ = !suspended_ && demand_.enabled_mask != 0;
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
    }

void NativeTelemetryScheduler::on_source_replaced(const RenderBridge& bridge) {
        ++source_epoch_;
        ++epoch_;
        current_tick_ = bridge.current_tick();
        apply_native_qos_policy(bridge.lattice().size());
        cache_ = TelemetrySnapshot{};
        group_snapshot_versions_.fill(0);
        publications_.clear();
        invalidations_.clear();
        in_flight_ = false;
        in_flight_mask_ = 0;
        in_flight_epoch_ = 0;
        in_flight_deadline_ = Clock::time_point::min();
        retired_in_flight_ = false;
        suspended_ = false;
        suspension_reason_.clear();
        schedule_armed_ = demand_.enabled_mask != 0;
        forced_mask_ = 0;
        direct_mutation_due_ = Clock::time_point::min();
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
    }

void NativeTelemetryScheduler::on_state_mutated(const RenderBridge& bridge) {
        if (suspended_) return;
        ++epoch_;
        current_tick_ = bridge.current_tick();
        // Injection/profile gestures commonly arrive as several independent
        // commands.  Treat the last mutation in a short quiet window as a
        // settled observation boundary, avoiding snapshots of half-built
        // wave packets while still serving a paused editor.
        direct_mutation_due_ = Clock::now() + kDirectMutationDebounce;
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
        publish_invalidation("state_mutated");
    }

void NativeTelemetryScheduler::on_tick_complete(RenderBridge& bridge) {
        if (suspended_) return;
        ++epoch_;
        current_tick_ = bridge.current_tick();
        schedule_armed_ = demand_.enabled_mask != 0;
        // A direct edit may be immediately followed by playback's next
        // tick, before its quiet-window callback can set forced_mask_. The
        // tick is a settled boundary, so carry that pending edit forward and
        // ensure every demanded group eventually refreshes (subject to the
        // producer QoS intervals) rather than leaving slow audit/action
        // groups stale for their nominal 8/12-tick cadence.
        if (direct_mutation_due_ != Clock::time_point::min()) {
            forced_mask_ |= demand_.enabled_mask;
        }
        direct_mutation_due_ = Clock::time_point::min();
        last_failed_attempt_epoch_ = kNoEpoch;
        last_failed_attempt_mask_ = 0;
        (void)pump(bridge);
    }

bool NativeTelemetryScheduler::pump(RenderBridge& bridge) {
        if (suspended_) return false;
        bool published = false;
        const auto now = Clock::now();

        if (direct_mutation_due_ != Clock::time_point::min()
            && now >= direct_mutation_due_) {
            forced_mask_ |= demand_.enabled_mask;
            schedule_armed_ = demand_.enabled_mask != 0;
            direct_mutation_due_ = Clock::time_point::min();
        }

        if (in_flight_) {
            if (now >= in_flight_deadline_) {
                const int lattice_size = bridge.lattice().size();
                const std::string message =
                    "native telemetry snapshot timed out at L="
                    + std::to_string(lattice_size)
                    + "; restart the native engine process to recover";
                abort_and_suspend(message);
                throw std::runtime_error(message);
            }
            TelemetrySnapshot completed;
            if (bridge.poll_telemetry_snapshot(completed)) {
                // The backend echoes the scheduler epoch. A mismatch means a
                // bridge mode/source transition raced a retired observation;
                // discard it rather than repaint the new world with old data.
                const std::uint32_t completed_mask =
                    completed.epoch == in_flight_epoch_
                    && completed.epoch == epoch_
                    ? (completed.groups & in_flight_mask_ & demand_.enabled_mask)
                    : 0u;
                if (completed_mask != 0) {
                    promote(completed, completed_mask);
                    published = true;
                }
                in_flight_ = false;
                in_flight_mask_ = 0;
                in_flight_epoch_ = 0;
                in_flight_deadline_ = Clock::time_point::min();
                retired_in_flight_ = false;
            }
        }

        // A direct edit may race an already-running GPU snapshot. Polling is
        // still allowed above so that obsolete work can retire, but never
        // begin a replacement while the edit batch's quiet window is open:
        // otherwise an empty/stale cache can make due_mask() launch before
        // the final mutation in the gesture arrives.
        const bool mutation_debounce_pending =
            direct_mutation_due_ != Clock::time_point::min()
            && now < direct_mutation_due_;
        if (!in_flight_ && schedule_armed_ && !mutation_debounce_pending) {
            bool throttled = false;
            const std::uint32_t due = due_mask(now, throttled);
            if (due == 0) {
                // Keep the producer armed if it is waiting only for the
                // native QoS interval. The idle event loop will retry at the
                // deadline; browser rAFs never own this policy.
                schedule_armed_ = throttled;
            } else if (last_failed_attempt_epoch_ != epoch_
                       || last_failed_attempt_mask_ != due) {
                TelemetrySnapshotRequest request;
                request.groups = due;
                request.epoch = epoch_;
                if (bridge.begin_telemetry_snapshot(request)) {
                    in_flight_ = true;
                    in_flight_mask_ = due;
                    in_flight_epoch_ = epoch_;
                    in_flight_deadline_ = now + snapshot_timeout(bridge.lattice().size());
                    for_each_group([&](std::uint32_t bit, std::size_t index) {
                        if ((due & bit) != 0) {
                            next_allowed_[index] = now
                                + std::chrono::milliseconds(min_interval_ms_[index]);
                        }
                    });
                    // A direct-mutation force is retired only when a group
                    // is actually promoted below. If a newer tick invalidates
                    // this in-flight request, retaining the bit ensures the
                    // replacement snapshot still refreshes slow groups.
                    // `promote()` ran before this begin when a previous
                    // request completed in the same pump. Make that emitted
                    // delta truthful about the next request already pending.
                    if (published && !publications_.empty()) {
                        publications_.back().pending_mask = in_flight_mask_;
                    }
                    // Keep the scheduler armed after a partial/coalesced
                    // request. A direct mutation or one tick can make fast
                    // diagnostics due before slower groups clear their QoS
                    // interval; after this fence completes, pump() must
                    // revisit those remaining forced/due groups even while
                    // playback is paused.
                    schedule_armed_ = true;
                    last_failed_attempt_epoch_ = kNoEpoch;
                    last_failed_attempt_mask_ = 0;
                } else {
                    // A backend can reject a producer while it is changing
                    // mode. Retry only after the next state/subscription
                    // boundary; never spin a polling loop on a failed begin.
                    last_failed_attempt_epoch_ = epoch_;
                    last_failed_attempt_mask_ = due;
                }
            }
        }

        return published;
    }

NativeTelemetryScheduler::CachedView NativeTelemetryScheduler::latest() const {
        CachedView out;
        out.snapshot_version = snapshot_version_;
        out.source_epoch = source_epoch_;
        out.epoch = epoch_;
        out.tick = current_tick_;
        out.enabled_mask = demand_.enabled_mask;
        out.available_mask = cache_.groups;
        out.pending_mask = in_flight_ ? in_flight_mask_ : 0;
        out.snapshot = cache_;
        out.group_snapshot_versions = group_snapshot_versions_;
        out.min_interval_ms = min_interval_ms_;

        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((out.available_mask & bit) == 0) return;
            const auto& meta = group_meta(cache_, index);
            if (meta.epoch == epoch_) out.fresh_mask |= bit;
        });
        return out;
    }

std::optional<NativeTelemetryScheduler::Publication> NativeTelemetryScheduler::take_publication() {
        if (publications_.empty()) return std::nullopt;
        Publication out = std::move(publications_.front());
        publications_.pop_front();
        return out;
    }

std::optional<NativeTelemetryScheduler::Invalidation> NativeTelemetryScheduler::take_invalidation() {
        if (invalidations_.empty()) return std::nullopt;
        Invalidation out = std::move(invalidations_.front());
        invalidations_.pop_front();
        return out;
    }

TelemetryGroupMeta& NativeTelemetryScheduler::group_meta(TelemetrySnapshot& snapshot,
                                                           std::size_t index) {
        switch (index) {
        case 0: return snapshot.diagnostics_meta;
        case 1: return snapshot.audit_meta;
        case 2: return snapshot.gravity_meta;
        default: return snapshot.lagrangian_meta;
        }
    }

const TelemetryGroupMeta& NativeTelemetryScheduler::group_meta(
    const TelemetrySnapshot& snapshot,
    std::size_t index) {
        switch (index) {
        case 0: return snapshot.diagnostics_meta;
        case 1: return snapshot.audit_meta;
        case 2: return snapshot.gravity_meta;
        default: return snapshot.lagrangian_meta;
        }
    }

void NativeTelemetryScheduler::clear_versions(std::uint32_t mask) {
        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((mask & bit) != 0) group_snapshot_versions_[index] = 0;
        });
    }

void NativeTelemetryScheduler::publish_invalidation(const char* reason) {
        Invalidation invalidation;
        invalidation.source_epoch = source_epoch_;
        invalidation.epoch = epoch_;
        invalidation.snapshot_version = snapshot_version_;
        invalidation.tick = current_tick_;
        invalidation.available_mask = cache_.groups;
        invalidation.pending_mask = in_flight_ ? in_flight_mask_ : 0;
        invalidation.reason = reason;
        invalidations_.push_back(std::move(invalidation));
    }

std::uint32_t NativeTelemetryScheduler::due_mask(Clock::time_point now,
                                                  bool& throttled) const {
        std::uint32_t due = 0;
        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((demand_.enabled_mask & bit) == 0) return;
            const bool force_sample = (forced_mask_ & bit) != 0;
            bool cadence_due = (cache_.groups & bit) == 0 || force_sample;
            if (!cadence_due) {
                const int sampled_tick = group_meta(cache_, index).tick;
                const std::uint32_t cadence = demand_.every_ticks[index];
                // Scenario/reset transitions can legitimately return the tick
                // counter to zero, so an apparent rewind means sample again.
                cadence_due = current_tick_ < sampled_tick
                    || static_cast<std::uint64_t>(current_tick_ - sampled_tick) >= cadence;
            }
            if (!cadence_due) return;
            // A never-seen group is allowed to establish an initial value
            // immediately. Existing values obey the source-owned GPU QoS.
            if ((cache_.groups & bit) != 0 && now < next_allowed_[index]) {
                throttled = true;
                return;
            }
            due |= bit;
        });
        return due;
    }

void NativeTelemetryScheduler::promote(const TelemetrySnapshot& completed,
                                         std::uint32_t mask) {
        const std::uint64_t version = ++snapshot_version_;
        TelemetrySnapshot delta{};
        delta.epoch = completed.epoch;
        delta.state_version = completed.state_version;
        delta.tick = completed.tick;
        delta.groups = mask;

        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((mask & bit) == 0) return;
            switch (index) {
            case 0:
                cache_.diagnostics = completed.diagnostics;
                delta.diagnostics = completed.diagnostics;
                break;
            case 1:
                cache_.audit = completed.audit;
                delta.audit = completed.audit;
                break;
            case 2:
                cache_.gravity = completed.gravity;
                delta.gravity = completed.gravity;
                break;
            default:
                cache_.lagrangian = completed.lagrangian;
                delta.lagrangian = completed.lagrangian;
                break;
            }
            group_meta(cache_, index) = group_meta(completed, index);
            group_meta(delta, index) = group_meta(completed, index);
            cache_.groups |= bit;
            group_snapshot_versions_[index] = version;
            forced_mask_ &= ~bit;
        });

        Publication publication;
        publication.snapshot_version = version;
        publication.source_epoch = source_epoch_;
        publication.epoch = epoch_;
        publication.tick = current_tick_;
        publication.published_mask = mask;
        publication.snapshot = std::move(delta);
        publication.available_mask = cache_.groups;
        publication.pending_mask = 0;  // refreshed after an optional new begin below
        publication.group_snapshot_versions = group_snapshot_versions_;
        publication.min_interval_ms = min_interval_ms_;
        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((publication.available_mask & bit) == 0) return;
            if (group_meta(cache_, index).epoch == epoch_) publication.fresh_mask |= bit;
        });
        publications_.push_back(std::move(publication));
    }

void NativeTelemetryScheduler::apply_native_qos_policy(int lattice_size) {
        // Policy belongs beside the producer because each sample scans the
        // lattice. This is deliberately independent of visibility/rAF: the
        // UI only declares desired products; the native engine protects its
        // own compute budget. Values are minimum spacing, not promises.
        if (lattice_size >= 113) {
            min_interval_ms_ = {{125u, 1250u, 750u, 2000u}};
        } else if (lattice_size >= 65) {
            min_interval_ms_ = {{66u, 500u, 250u, 1000u}};
        } else {
            min_interval_ms_ = {{33u, 250u, 125u, 500u}};
        }
        next_allowed_.fill(Clock::time_point::min());
    }

NativeTelemetryScheduler::Clock::duration
NativeTelemetryScheduler::snapshot_timeout(int lattice_size) {
        // A fence is expected to complete in milliseconds on the native GPU.
        // Keep failure detection deliberately generous for large/all-group
        // reductions, but never let an error-not-ready event stall transport
        // priority forever.
        if (lattice_size >= 113) return std::chrono::seconds(15);
        if (lattice_size >= 65) return std::chrono::seconds(6);
        return std::chrono::seconds(2);
    }

}  // namespace ftd
