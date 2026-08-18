#pragma once

#include "native_desktop/engine_session.h"

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstdint>
#include <memory>

namespace ftd::native_desktop {

struct Camera {
    float target_x = 0.0f;
    float target_y = 0.0f;
    float target_z = 0.0f;
    float yaw = 0.6f;
    float pitch = 0.4f;
    float distance = 48.0f;
    float fov_y = 0.9f;
};

struct NativeViewOptions {
    bool particles = true;
    bool flux = true;
    bool lattice_box = true;
};

class D3D12Presenter {
public:
    D3D12Presenter();
    ~D3D12Presenter();

    D3D12Presenter(const D3D12Presenter&) = delete;
    D3D12Presenter& operator=(const D3D12Presenter&) = delete;

    void initialize(HWND hwnd, std::uint32_t width, std::uint32_t height);
    void resize(std::uint32_t width, std::uint32_t height);
    void render(const NativeFrame& frame, const Camera& camera,
                const NativeViewOptions& opts = {});
    void wait_idle();

    std::uint32_t width() const { return width_; }
    std::uint32_t height() const { return height_; }

    // Enumerates DXGI adapters and picks the first non-software one (skips
    // WARP). Static + no side effects on `this` so it's testable without a
    // window or a live device. Returns false if only a software adapter is
    // available (rare on real hardware, common in some CI/VM environments).
    static bool select_hardware_adapter(LUID* out_luid, bool* out_is_hardware);

    LUID adapter_luid() const { return adapter_luid_; }
    bool has_adapter_luid() const { return has_adapter_luid_; }

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    std::uint32_t width_ = 0;
    std::uint32_t height_ = 0;
    LUID adapter_luid_{};
    bool has_adapter_luid_ = false;
};

}  // namespace ftd::native_desktop
