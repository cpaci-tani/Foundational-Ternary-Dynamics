#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"

#include <stdexcept>
#include <string>
#include <thread>

int main() {
    ftd::test::init("test_ui_thread_guard");

    ftd::RenderBridge rb(5);
    rb.force_cpu();
    rb.bind_sim_thread();
    (void)rb.energy_ledger();
    rb.tick();

#ifndef NDEBUG
    ftd::test::section("const observer from a second thread is rejected");
    bool threw = false;
    std::thread worker([&] {
        try {
            (void)rb.energy_ledger();
        } catch (const std::logic_error& ex) {
            threw = std::string(ex.what()).find("FTD_UI_DEBUG_THREAD_GUARD") != std::string::npos;
        }
    });
    worker.join();
    ftd::test::check("cross-thread energy_ledger throws the debug guard", threw);
#else
    ftd::test::check("Release thread guard is a no-op", true);
#endif

    return ftd::test::finalize();
}
