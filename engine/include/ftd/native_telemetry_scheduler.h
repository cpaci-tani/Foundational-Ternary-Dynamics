#pragma once
/**
 * @file native_telemetry_scheduler.h
 * @brief Tick-boundary publisher for native interactive telemetry.
 *
 * The native dashboard is not a collection of synchronous inspection RPCs.
 * It is a consumer of immutable observation snapshots produced by the engine
 * after a settled state boundary.  This scheduler owns that distinction:
 *
 *   * `on_tick_complete()` advances the server epoch and starts at most one
 *     non-blocking backend snapshot request.
 *   * `pump()` only polls that request and promotes a completed result into a
 *     cache.  It never asks a panel request to run a reduction.
 *   * while a CUDA request is in flight, newer ticks are coalesced; the next
 *     request observes the latest settled epoch rather than building a FIFO
 *     of stale whole-lattice reductions.
 *   * every group retains its own source metadata.  Fast diagnostics and slow
 *     action/energy diagnostics therefore cannot be misrepresented as one
 *     common-time observation.
 *
 * This is deliberately header-only: it is linked by `ws_server` but has no
 * dependency on the engine core target or its CMake source list.  RenderBridge
 * exposes the non-blocking snapshot API through Backend on every build; the
 * CUDA implementation queues work and the CPU implementation makes a
 * immediately-pollable snapshot for functional parity.
 */

#include "ftd/render_bridge.h"
#include "ftd/telemetry_snapshot.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <stdexcept>
#include <string>

namespace ftd {

class NativeTelemetryScheduler final {
public:
    static constexpr std::uint32_t kMaxCadenceTicks = 65535u;

    /// Subscription state controlled by one explicit protocol command.  A
    /// cadence is expressed in settled simulation ticks, not browser frames
    /// or wall-clock time, so physics observation stays tied to engine time.
    struct Demand {
        std::uint32_t enabled_mask = 0;
        // diagnostics, audit, gravity, lagrangian
        std::array<std::uint32_t, 4> every_ticks{{1u, 8u, 4u, 12u}};
    };

    /// A cache view has top-level publication metadata plus the immutable
    /// per-group source provenance retained in `snapshot.*_meta`.
    struct CachedView {
        std::uint64_t snapshot_version = 0;
        // Identifies the RenderBridge/source generation only. Unlike epoch,
        // this does not advance for ordinary ticks or direct state edits.
        std::uint64_t source_epoch = 0;
        std::uint64_t epoch = 0;
        int tick = 0;
        std::uint32_t enabled_mask = 0;
        std::uint32_t available_mask = 0;
        std::uint32_t fresh_mask = 0;
        std::uint32_t pending_mask = 0;
        TelemetrySnapshot snapshot;
        // Cache publication that last refreshed each group.  A zero value
        // means that group has never been observed by this scheduler.
        std::array<std::uint64_t, 4> group_snapshot_versions{{0, 0, 0, 0}};
        std::array<std::uint32_t, 4> min_interval_ms{{0, 0, 0, 0}};
    };

    /// One server-push event. `snapshot.groups` is a delta: only the groups
    /// named by `published_mask` are new. Consumers merge this into their own
    /// group cache using the associated per-group metadata/version.
    struct Publication {
        std::uint64_t snapshot_version = 0;
        std::uint64_t source_epoch = 0;
        std::uint64_t epoch = 0;
        int tick = 0;
        std::uint32_t published_mask = 0;
        std::uint32_t available_mask = 0;
        std::uint32_t fresh_mask = 0;
        std::uint32_t pending_mask = 0;
        TelemetrySnapshot snapshot;
        std::array<std::uint64_t, 4> group_snapshot_versions{{0, 0, 0, 0}};
        std::array<std::uint32_t, 4> min_interval_ms{{0, 0, 0, 0}};
    };

    /// A state boundary is observable before its next full-grid reduction is
    /// ready. This tiny control-plane delta tells a client that retained
    /// group values belong to an older epoch; it never carries reductions or
    /// forces a producer pass.
    struct Invalidation {
        std::uint64_t source_epoch = 0;
        std::uint64_t epoch = 0;
        std::uint64_t snapshot_version = 0;
        int tick = 0;
        std::uint32_t available_mask = 0;
        std::uint32_t pending_mask = 0;
        std::string reason;
    };

    NativeTelemetryScheduler() = default;

    const Demand& demand() const { return demand_; }
    const std::array<std::uint32_t, 4>& min_interval_ms() const {
        return min_interval_ms_;
    }
    std::uint64_t epoch() const { return epoch_; }
    std::uint64_t source_epoch() const { return source_epoch_; }
    std::uint64_t snapshot_version() const { return snapshot_version_; }
    bool suspended() const { return suspended_; }
    bool restart_required() const { return suspended_; }
    const std::string& suspension_reason() const { return suspension_reason_; }

    /// Replacing RenderBridge destroys the old backend. That destruction must
    /// never race an active CUDA observation event because backend teardown
    /// may synchronize it. The server calls pump() first, then replaces only
    /// when this non-blocking predicate becomes true.
    bool safe_to_replace() const { return !suspended_ && !in_flight_; }
    bool observation_in_flight() const { return in_flight_; }

    /// True when a scalar observation has priority over a bulk visual frame.
    /// Callers use this only to defer optional visual transport; it never
    /// launches or waits for a reduction itself.
    bool has_pending_or_due_observation() const {
        if (suspended_ || demand_.enabled_mask == 0) return false;
        if (in_flight_) return true;
        if (direct_mutation_due_ != Clock::time_point::min()) return true;
        if (!schedule_armed_) return false;
        bool throttled = false;
        return due_mask(Clock::now(), throttled) != 0;
    }

    /// A CUDA event/query failure means the observation stream is no longer
    /// trustworthy. There is no safe generic cancellation primitive for a
    /// failed device event, and destroying the bridge can itself synchronize
    /// that event. Retire scheduler state and suspend producer work; the
    /// native server must fail closed and require a process restart rather
    /// than attempting an in-process scenario/reset replacement.
    void abort_and_suspend(const std::string& reason = {}) {
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

    /// A WebSocket client owns a subscription, not the engine. On disconnect
    /// drop queued publications and retire its demand so a later client cannot
    /// receive a stale delta before it has established its own scenario and
    /// subscription. A device snapshot already in flight is left for poll()
    /// to drain, but its echoed epoch is made stale and therefore discarded.
    void on_client_disconnected() {
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

    /// Changes the desired observation product.  This is a control-plane
    /// operation: it only arms the next non-blocking producer pass; it does
    /// not compute telemetry itself.
    void set_demand(Demand demand) {
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

    /// Call once after a new RenderBridge has become authoritative. Existing
    /// cached values belong to a destroyed/replaced lattice and must never be
    /// served under the new scenario generation.
    void on_source_replaced(const RenderBridge& bridge) {
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

    /// Marks a direct host/device mutation.  Such commands invalidate cache
    /// freshness, but intentionally do not start a measurement midway through
    /// a multi-command injection batch; the following tick is the normal
    /// settled observation boundary.  Scenario replacement uses the explicit
    /// method above because it is itself an atomic transaction.
    void on_state_mutated(const RenderBridge& bridge) {
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

    /// Called immediately after `RenderBridge::tick()` or `run()` returns.
    /// This queues a CUDA snapshot after the finished simulation work, but
    /// never waits for CUDA completion on the simulation command path.
    void on_tick_complete(RenderBridge& bridge) {
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

    /// Non-blocking publisher service. Call from the native server event loop
    /// while idle and after a control command. It first promotes any completed
    /// backend request, then (if no request is active) begins one coalesced
    /// latest-epoch request. No call in this method waits on a CUDA event.
    bool pump(RenderBridge& bridge) {
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

    CachedView latest() const {
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

    std::optional<Publication> take_publication() {
        if (publications_.empty()) return std::nullopt;
        Publication out = std::move(publications_.front());
        publications_.pop_front();
        return out;
    }

    std::optional<Invalidation> take_invalidation() {
        if (invalidations_.empty()) return std::nullopt;
        Invalidation out = std::move(invalidations_.front());
        invalidations_.pop_front();
        return out;
    }

private:
    using Clock = std::chrono::steady_clock;
    static constexpr auto kDirectMutationDebounce = std::chrono::milliseconds(16);
    static constexpr std::uint64_t kNoEpoch = ~std::uint64_t{0};

    static constexpr std::array<std::uint32_t, 4> kGroupBits{{
        TELEMETRY_DIAGNOSTICS,
        TELEMETRY_AUDIT,
        TELEMETRY_GRAVITY,
        TELEMETRY_LAGRANGIAN,
    }};

    template <typename F>
    static void for_each_group(F&& fn) {
        for (std::size_t i = 0; i < kGroupBits.size(); ++i) fn(kGroupBits[i], i);
    }

    static TelemetryGroupMeta& group_meta(TelemetrySnapshot& snapshot,
                                           std::size_t index) {
        switch (index) {
        case 0: return snapshot.diagnostics_meta;
        case 1: return snapshot.audit_meta;
        case 2: return snapshot.gravity_meta;
        default: return snapshot.lagrangian_meta;
        }
    }

    static const TelemetryGroupMeta& group_meta(const TelemetrySnapshot& snapshot,
                                                 std::size_t index) {
        switch (index) {
        case 0: return snapshot.diagnostics_meta;
        case 1: return snapshot.audit_meta;
        case 2: return snapshot.gravity_meta;
        default: return snapshot.lagrangian_meta;
        }
    }

    void clear_versions(std::uint32_t mask) {
        for_each_group([&](std::uint32_t bit, std::size_t index) {
            if ((mask & bit) != 0) group_snapshot_versions_[index] = 0;
        });
    }

    void publish_invalidation(const char* reason) {
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

    std::uint32_t due_mask(Clock::time_point now, bool& throttled) const {
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

    void promote(const TelemetrySnapshot& completed, std::uint32_t mask) {
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

    Demand demand_{};
    TelemetrySnapshot cache_{};
    std::array<std::uint64_t, 4> group_snapshot_versions_{{0, 0, 0, 0}};
    std::deque<Publication> publications_;
    std::deque<Invalidation> invalidations_;

    // `source_epoch_` is a source-generation guard for transport consumers;
    // `epoch_` is a fine-grained state/freshness clock and advances on normal
    // ticks and direct host mutations.
    std::uint64_t source_epoch_ = 0;
    std::uint64_t epoch_ = 0;
    std::uint64_t snapshot_version_ = 0;
    int current_tick_ = 0;
    bool schedule_armed_ = false;
    bool in_flight_ = false;
    std::uint32_t in_flight_mask_ = 0;
    std::uint64_t in_flight_epoch_ = 0;
    Clock::time_point in_flight_deadline_ = Clock::time_point::min();
    std::uint64_t last_failed_attempt_epoch_ = kNoEpoch;
    std::uint32_t last_failed_attempt_mask_ = 0;
    bool suspended_ = false;
    bool retired_in_flight_ = false;
    std::string suspension_reason_;
    std::array<std::uint32_t, 4> min_interval_ms_{{33u, 250u, 125u, 500u}};
    std::array<Clock::time_point, 4> next_allowed_{{
        Clock::time_point::min(), Clock::time_point::min(),
        Clock::time_point::min(), Clock::time_point::min(),
    }};
    std::uint32_t forced_mask_ = 0;
    Clock::time_point direct_mutation_due_ = Clock::time_point::min();

    void apply_native_qos_policy(int lattice_size) {
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

    static Clock::duration snapshot_timeout(int lattice_size) {
        // A fence is expected to complete in milliseconds on the native GPU.
        // Keep failure detection deliberately generous for large/all-group
        // reductions, but never let an error-not-ready event stall transport
        // priority forever.
        if (lattice_size >= 113) return std::chrono::seconds(15);
        if (lattice_size >= 65) return std::chrono::seconds(6);
        return std::chrono::seconds(2);
    }
};

}  // namespace ftd
