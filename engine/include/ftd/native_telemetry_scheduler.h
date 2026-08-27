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
 * The public contracts and state layout remain in this header. Implementation
 * lives in the engine core so consumers do not repeatedly parse the scheduler's
 * state machine. RenderBridge exposes the non-blocking snapshot API through
 * Backend on every build; CUDA queues work and CPU provides an immediately
 * pollable snapshot for functional parity.
 */

#include "ftd/telemetry_snapshot.h"

#include <array>
#include <chrono>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>

namespace ftd {

class RenderBridge;

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

    const Demand& demand() const;
    const std::array<std::uint32_t, 4>& min_interval_ms() const;
    std::uint64_t epoch() const;
    std::uint64_t source_epoch() const;
    std::uint64_t snapshot_version() const;
    bool suspended() const;
    bool restart_required() const;
    const std::string& suspension_reason() const;

    /// Replacing RenderBridge destroys the old backend. That destruction must
    /// never race an active CUDA observation event because backend teardown
    /// may synchronize it. The server calls pump() first, then replaces only
    /// when this non-blocking predicate becomes true.
    bool safe_to_replace() const;
    bool observation_in_flight() const;

    /// True when a scalar observation has priority over a bulk visual frame.
    /// Callers use this only to defer optional visual transport; it never
    /// launches or waits for a reduction itself.
    bool has_pending_or_due_observation() const;

    /// A CUDA event/query failure means the observation stream is no longer
    /// trustworthy. There is no safe generic cancellation primitive for a
    /// failed device event, and destroying the bridge can itself synchronize
    /// that event. Retire scheduler state and suspend producer work; the
    /// native server must fail closed and require a process restart rather
    /// than attempting an in-process scenario/reset replacement.
    void abort_and_suspend(const std::string& reason = {});

    /// A WebSocket client owns a subscription, not the engine. On disconnect
    /// drop queued publications and retire its demand so a later client cannot
    /// receive a stale delta before it has established its own scenario and
    /// subscription. A device snapshot already in flight is left for poll()
    /// to drain, but its echoed epoch is made stale and therefore discarded.
    void on_client_disconnected();

    /// Changes the desired observation product.  This is a control-plane
    /// operation: it only arms the next non-blocking producer pass; it does
    /// not compute telemetry itself.
    void set_demand(Demand demand);

    /// Call once after a new RenderBridge has become authoritative. Existing
    /// cached values belong to a destroyed/replaced lattice and must never be
    /// served under the new scenario generation.
    void on_source_replaced(const RenderBridge& bridge);

    /// Marks a direct host/device mutation.  Such commands invalidate cache
    /// freshness, but intentionally do not start a measurement midway through
    /// a multi-command injection batch; the following tick is the normal
    /// settled observation boundary.  Scenario replacement uses the explicit
    /// method above because it is itself an atomic transaction.
    void on_state_mutated(const RenderBridge& bridge);

    /// Called immediately after `RenderBridge::tick()` or `run()` returns.
    /// This queues a CUDA snapshot after the finished simulation work, but
    /// never waits for CUDA completion on the simulation command path.
    void on_tick_complete(RenderBridge& bridge);

    /// Non-blocking publisher service. Call from the native server event loop
    /// while idle and after a control command. It first promotes any completed
    /// backend request, then (if no request is active) begins one coalesced
    /// latest-epoch request. No call in this method waits on a CUDA event.
    bool pump(RenderBridge& bridge);

    CachedView latest() const;

    std::optional<Publication> take_publication();

    std::optional<Invalidation> take_invalidation();

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
                                           std::size_t index);

    static const TelemetryGroupMeta& group_meta(const TelemetrySnapshot& snapshot,
                                                 std::size_t index);

    void clear_versions(std::uint32_t mask);

    void publish_invalidation(const char* reason);

    std::uint32_t due_mask(Clock::time_point now, bool& throttled) const;

    void promote(const TelemetrySnapshot& completed, std::uint32_t mask);

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

    void apply_native_qos_policy(int lattice_size);

    static Clock::duration snapshot_timeout(int lattice_size);
};

}  // namespace ftd
