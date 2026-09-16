// ============================================================================
// test_ws_idle_policy.cpp — pure decision-table coverage for
// ftd::IdleShutdownPolicy (idle auto-stop, 2026-09-16 spec).
//
// Scope: the header-only policy in <ftd/ws_idle_policy.h> only. No sockets,
// no RenderBridge, no real clock — every timestamp is an explicit
// std::chrono::steady_clock::time_point so the boundary arithmetic is exact
// and deterministic. This is the "testability requirement" the spec calls
// out: ws_server_runtime.cpp consults this same type with `now` read from
// the real clock, but that wiring is out of scope here.
// ============================================================================

#include "ftd/ws_idle_policy.h"
#include "ftd/test_telemetry.h"

#include <chrono>
#include <string>

namespace ftd { namespace test {

using Clock = std::chrono::steady_clock;
using std::chrono::minutes;
using std::chrono::seconds;

void test_disabled_policy_never_stops() {
    section("Disabled policy (timeout=0) never stops either clock");
    IdleShutdownPolicy policy;
    policy.timeout = minutes(0);
    check("enabled() is false when timeout is zero", !policy.enabled(), "");

    const auto epoch = Clock::time_point{};
    const auto far_future = epoch + minutes(1'000'000);
    check("no client attached, arbitrarily far past any timeout: no stop",
          policy.should_stop(far_future, epoch, /*client_attached=*/false, epoch) == nullptr,
          "disabled policy must never fire the disconnected clock");
    check("client attached and silent, arbitrarily far past any timeout: no stop",
          policy.should_stop(far_future, epoch, /*client_attached=*/true, epoch) == nullptr,
          "disabled policy must never fire the silent-client clock");
}

void test_disconnected_clock_boundary() {
    section("Disconnected clock fires at the boundary and not before");
    IdleShutdownPolicy policy;
    policy.timeout = minutes(30);

    const auto disconnected_since = Clock::time_point{};
    const auto just_before = disconnected_since + minutes(30) - seconds(1);
    const auto at_boundary = disconnected_since + minutes(30);
    const auto after = disconnected_since + minutes(31);

    check("one second short of the timeout: no stop",
          policy.should_stop(just_before, disconnected_since, false, disconnected_since) == nullptr,
          "fired before the boundary");

    const char* at = policy.should_stop(at_boundary, disconnected_since, false, disconnected_since);
    check("exactly at the timeout: stops", at != nullptr, "did not fire at the boundary");
    check("boundary reason names the disconnected clock",
          at != nullptr && std::string(at) == std::string(kIdleReasonNoClientAttached), "");

    check("past the timeout: still stops",
          policy.should_stop(after, disconnected_since, false, disconnected_since) != nullptr, "");
}

void test_silent_client_clock_fires_independently() {
    section("Silent-client clock fires independently of the disconnected clock");
    IdleShutdownPolicy policy;
    policy.timeout = minutes(30);

    // disconnected_since is ancient — it would have fired long ago if the
    // policy consulted it — but a client IS attached, so should_stop must
    // ignore it and look only at last_client_activity.
    const auto disconnected_since = Clock::time_point{};
    const auto last_activity = disconnected_since + minutes(1000);
    const auto just_before = last_activity + minutes(30) - seconds(1);
    const auto at_boundary = last_activity + minutes(30);

    check("attached client, ancient disconnected_since, not yet silent long enough: no stop",
          policy.should_stop(just_before, disconnected_since, true, last_activity) == nullptr,
          "the disconnected clock leaked into the attached-client branch");

    const char* reason = policy.should_stop(at_boundary, disconnected_since, true, last_activity);
    check("attached client silent past the timeout: stops", reason != nullptr, "");
    check("silent-client reason is distinct from the disconnected reason",
          reason != nullptr && std::string(reason) == std::string(kIdleReasonClientSilent), "");
}

void test_activity_resets_the_clock() {
    section("Fresh client activity resets the silent-client clock");
    IdleShutdownPolicy policy;
    policy.timeout = minutes(30);

    const auto disconnected_since = Clock::time_point{};
    const auto stale_activity = disconnected_since;
    const auto now = stale_activity + minutes(45);
    const auto fresh_activity = now - minutes(1);  // a message just arrived

    check("stale activity alone would already have fired (sanity check)",
          policy.should_stop(now, disconnected_since, true, stale_activity) != nullptr,
          "test setup is not actually past the timeout");
    check("fresh activity resets the clock: no stop",
          policy.should_stop(now, disconnected_since, true, fresh_activity) == nullptr,
          "activity did not reset the silent-client clock");
}

void test_reasons_are_distinguishable() {
    section("The two stop reasons are distinct, non-null strings");
    check("reason constants are non-null",
          kIdleReasonNoClientAttached != nullptr && kIdleReasonClientSilent != nullptr, "");
    check("reason constants differ",
          std::string(kIdleReasonNoClientAttached) != std::string(kIdleReasonClientSilent),
          "both clocks report the same reason string");
}

}}  // namespace ftd::test

int main() {
    ftd::test::init("test_ws_idle_policy");
    ftd::test::test_disabled_policy_never_stops();
    ftd::test::test_disconnected_clock_boundary();
    ftd::test::test_silent_client_clock_fires_independently();
    ftd::test::test_activity_resets_the_clock();
    ftd::test::test_reasons_are_distinguishable();
    return ftd::test::finalize();
}
