#pragma once
/**
 * @file ws_idle_policy.h
 * @brief Pure, socket-free idle-shutdown decision for ws_server (2026-09-16
 *        idle auto-stop spec, section 1).
 *
 * Two independent activity clocks decide whether the server should stop:
 *   - `disconnected_since` tracks how long the accept loop has gone with no
 *     client attached at all.
 *   - `last_client_activity` tracks how long an attached client has gone
 *     silent (no message received, including the `ping` heartbeat).
 *
 * The server should exit when EITHER clock has run past `timeout`. Keeping
 * the decision here — with `now` always passed in rather than read from a
 * live clock — means it can be unit-tested with fabricated
 * steady_clock::time_point values and no sockets at all.
 */

#include <chrono>

namespace ftd {

// Distinguishable reasons `IdleShutdownPolicy::should_stop` can report, so a
// caller can log which clock fired without string-matching prose lines.
inline constexpr const char* kIdleReasonNoClientAttached = "no client attached";
inline constexpr const char* kIdleReasonClientSilent = "client attached but silent";

struct IdleShutdownPolicy {
    std::chrono::minutes timeout{30};  // zero disables auto-stop entirely

    bool enabled() const { return timeout.count() > 0; }

    // Returns one of the kIdleReason* constants above when the server should
    // stop, else nullptr. Disabled policies never stop. When a client is
    // attached, only the silent-client clock is consulted (the disconnected
    // clock is irrelevant while a client is attached); when no client is
    // attached, only the disconnected clock is consulted. Fires AT the
    // boundary (elapsed >= timeout), not only strictly after it.
    const char* should_stop(std::chrono::steady_clock::time_point now,
                            std::chrono::steady_clock::time_point disconnected_since,
                            bool client_attached,
                            std::chrono::steady_clock::time_point last_client_activity) const {
        if (!enabled()) return nullptr;
        if (client_attached) {
            if (now - last_client_activity >= timeout) return kIdleReasonClientSilent;
            return nullptr;
        }
        if (now - disconnected_since >= timeout) return kIdleReasonNoClientAttached;
        return nullptr;
    }
};

}  // namespace ftd
