#include "ftd/native_telemetry_scheduler.h"
#include "ftd/render_bridge.h"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
using Scheduler = ftd::NativeTelemetryScheduler;
using namespace std::chrono;
Scheduler::Clock::time_point now = Scheduler::Clock::time_point{} + seconds(100);
Scheduler::Clock::time_point fake_now() { return now; }
int checks = 0, failures = 0;
void check(bool ok, const char* message) {
    ++checks;
    if (!ok) { ++failures; std::cerr << "FAIL: " << message << '\n'; }
}
}

int main() {
#ifdef _WIN32
    _putenv_s("FTD_FORCE_CPU", "1");
#else
    setenv("FTD_FORCE_CPU", "1", 1);
#endif
    using namespace ftd;
    RenderBridge bridge(4);
    Scheduler scheduler(fake_now);
    scheduler.on_source_replaced(bridge);
    Scheduler::Demand demand;
    demand.enabled_mask = TELEMETRY_DIAGNOSTICS | TELEMETRY_AUDIT;
    scheduler.set_demand(demand);
    const auto start = now;
    check(!scheduler.pump(bridge) && scheduler.observation_in_flight(),
          "initial observation starts immediately");

    // A stale first result has never populated the cache. Retiring it must
    // still retain the producer deadline, even across a client reconnect.
    scheduler.on_client_disconnected();
    scheduler.set_demand(demand);
    now = start + milliseconds(1);
    check(!scheduler.pump(bridge), "retired snapshot is not published");
    check(!scheduler.observation_in_flight(), "empty cache cannot bypass attempt interval");
    check(scheduler.latest().available_mask == 0 && scheduler.snapshot_version() == 0,
          "retired result never enters cache");
    const auto diagnostic_deadline = start + milliseconds(scheduler.min_interval_ms()[0]);
    now = diagnostic_deadline - milliseconds(1);
    for (int i = 0; i < 100; ++i) scheduler.pump(bridge);
    check(!scheduler.observation_in_flight() && !scheduler.has_pending_or_due_observation(),
          "repeated idle polling before deadline launches no work");
    now = diagnostic_deadline;
    check(scheduler.has_pending_or_due_observation(), "exact deadline makes diagnostics due");
    scheduler.pump(bridge);
    check(scheduler.latest().pending_mask == TELEMETRY_DIAGNOSTICS,
          "per-group deadlines exclude the slower empty audit cache");
    check(scheduler.pump(bridge), "current diagnostic result publishes");
    auto publication = scheduler.take_publication();
    check(publication && publication->published_mask == TELEMETRY_DIAGNOSTICS,
          "publication contains only completed eligible group");
    check(scheduler.latest().available_mask == TELEMETRY_DIAGNOSTICS,
          "audit remains unavailable until its own producer deadline");
    now = start + milliseconds(scheduler.min_interval_ms()[1]);
    scheduler.pump(bridge);
    check(scheduler.latest().pending_mask == TELEMETRY_AUDIT,
          "audit starts at exact independent deadline");
    check(scheduler.pump(bridge), "audit eventually publishes while paused");
    scheduler.take_publication();

    // Disable/re-enable clears the cache but cannot create an earlier pass.
    const auto audit_attempt = now;
    scheduler.set_demand(Scheduler::Demand{});
    demand.enabled_mask = TELEMETRY_AUDIT;
    scheduler.set_demand(demand);
    scheduler.pump(bridge);
    check(!scheduler.observation_in_flight(), "demand churn preserves source QoS");
    now = audit_attempt + milliseconds(scheduler.min_interval_ms()[1]);
    scheduler.pump(bridge);
    check(scheduler.observation_in_flight(), "re-enabled group eventually starts");

    // A state edit invalidates an in-flight result and opens the quiet window.
    scheduler.on_state_mutated(bridge);
    check(!scheduler.pump(bridge), "edit retires old epoch without publication");
    now += milliseconds(16);
    scheduler.pump(bridge);
    check(!scheduler.observation_in_flight(), "quiet-window expiry does not bypass QoS");
    now = audit_attempt + milliseconds(2 * scheduler.min_interval_ms()[1]);
    scheduler.pump(bridge);
    check(scheduler.observation_in_flight(), "stale edit retries at next producer deadline");
    check(scheduler.pump(bridge), "settled edit eventually publishes");
    check(scheduler.latest().fresh_mask == TELEMETRY_AUDIT, "replacement has current epoch");

    // A genuinely new source has its own initial sampling entitlement.
    check(scheduler.safe_to_replace(), "source replacement happens with no active observation");
    scheduler.on_source_replaced(bridge);
    scheduler.pump(bridge);
    check(scheduler.observation_in_flight(), "new source resets initial producer deadline");
    scheduler.pump(bridge);
    check(bridge.current_tick() == 0 && bridge.physical_time() == 0,
          "all scheduling and retries preserve the physical clock");
    bool rejected_null = false;
    try { Scheduler invalid(nullptr); } catch (const std::invalid_argument&) { rejected_null = true; }
    check(rejected_null, "missing injected clock rejects");
    std::cout << checks - failures << '/' << checks << " checks passed\n";
    return failures ? 1 : 0;
}
