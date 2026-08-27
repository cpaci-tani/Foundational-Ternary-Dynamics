/**
 * CPU-only SimEngine selection regression.
 *
 * SimEngine's explicit prefer_gpu=false path keeps the fallback compiled and
 * exercised on GPU-capable developer machines without changing the class
 * layout macros used by the linked engine library.
 */
#include <stdexcept>

#include "ftd/engine_select.h"
#include "ftd/test_telemetry.h"

using ftd::test::check;

int main() {
    ftd::test::init("test_engine_select_cpu");

    ftd::SimEngine engine(4, /*prefer_gpu=*/false);
    check("CPU fallback reports using_gpu=false", !engine.using_gpu());
    check("CPU fallback site count is initialized", engine.total_sites() == 64);
    check("CPU fallback starts at tick zero", engine.current_tick() == 0);
    engine.tick();
    check("CPU fallback can tick", engine.current_tick() == 1);

    bool invalid_rejected = false;
    try {
        ftd::SimEngine invalid(0, /*prefer_gpu=*/false);
    } catch (const std::invalid_argument&) {
        invalid_rejected = true;
    }
    check("CPU fallback rejects invalid lattice size", invalid_rejected);

    return ftd::test::finalize();
}
