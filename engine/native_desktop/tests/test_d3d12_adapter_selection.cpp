#include "native_desktop/d3d12_presenter.h"
#include "ftd/test_telemetry.h"

int main() {
    ftd::test::init("test_d3d12_adapter_selection");

    ftd::native_desktop::D3D12Presenter presenter;
    // initialize() needs a real HWND to build a swapchain; adapter selection
    // itself does not, so it's exposed as a static helper testable without a
    // window.
    LUID luid{};
    bool is_hardware = false;
    const bool found = ftd::native_desktop::D3D12Presenter::select_hardware_adapter(
        &luid, &is_hardware);

    ftd::test::check("an adapter was found", found);
    ftd::test::check("the selected adapter is not the software (WARP) adapter",
                      is_hardware);
    ftd::test::check("the LUID is non-zero",
                      luid.LowPart != 0 || luid.HighPart != 0);

    return ftd::test::finalize();
}
