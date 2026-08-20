#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <d3d12.h>
#include <dxgiformat.h>

#include <cstdint>

namespace ftd::native {

struct RenderTargetInfo {
    D3D12_CPU_DESCRIPTOR_HANDLE rtv{};
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

class OverlayRecorder {
public:
    virtual ~OverlayRecorder() = default;
    virtual void record(ID3D12GraphicsCommandList* list,
                        const RenderTargetInfo& rt) = 0;
};

struct PresenterUiContext {
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* queue = nullptr;
    ID3D12DescriptorHeap* srv_heap = nullptr;
    DXGI_FORMAT rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    DXGI_FORMAT dsv_format = DXGI_FORMAT_UNKNOWN;
    int num_frames_in_flight = 2;
    void* user = nullptr;
    void (*alloc_srv)(PresenterUiContext* ctx,
                      D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
                      D3D12_GPU_DESCRIPTOR_HANDLE* gpu) = nullptr;
    void (*free_srv)(PresenterUiContext* ctx,
                     D3D12_CPU_DESCRIPTOR_HANDLE cpu,
                     D3D12_GPU_DESCRIPTOR_HANDLE gpu) = nullptr;
};

}  // namespace ftd::native
