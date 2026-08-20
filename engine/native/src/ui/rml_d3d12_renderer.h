// RmlD3D12Renderer — RmlUi 6.2 Rml::RenderInterface implemented on the engine's
// own Direct3D 12 device (M-UI-1, see native/docs/SPEC_NATIVE_UI_RMLUI.md §2).
//
// RmlUi Core is renderer-agnostic: it lays out RML/RCSS and emits geometry
// (Rml::Vertex + int indices), textures (font atlas via GenerateTexture, images
// via LoadTexture) and scissor rectangles through this interface. We turn that
// into D3D12 draw calls in a caller-supplied command list.
//
// Ownership: the caller owns the ID3D12Device + a DIRECT command queue and
// hands both to initialize(). The renderer owns its UI PSO, root signature, a
// shader-visible CBV_SRV_UAV heap (slot 0 = a 1x1 white default for untextured
// geometry, slots 1.. = RmlUi textures), a 1x1 white texture, and a private
// command allocator/list/fence used for synchronous texture uploads.
//
// Frame flow: the caller records the 3D scene + RT clear into a command list,
// calls begin_frame(cmd, w, h) (binds heap/root-sig/PSO/ortho/viewport), then
// context->Render() which drives RenderGeometry() into that same list, then
// end_frame(). RenderGeometry never allocates or executes on the queue — only
// CompileGeometry/GenerateTexture/LoadTexture touch the queue (uploads, done
// synchronously so a texture is resident before any draw samples it).
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d12.h>
#include <wrl/client.h>

#include <chrono>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include <RmlUi/Core/Log.h>
#include <RmlUi/Core/RenderInterface.h>
#include <RmlUi/Core/SystemInterface.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/Vertex.h>

namespace ftd::native::ui {

// Minimal SystemInterface: monotonic clock for CSS transitions/animations and
// log routing to stderr (the native Log-panel/status-bar contract lands later).
class RmlD3D12System : public Rml::SystemInterface {
public:
    RmlD3D12System();
    double GetElapsedTime() override;
    bool LogMessage(Rml::Log::Type type, const Rml::String& message) override;

private:
    std::chrono::steady_clock::time_point start_;
};

class RmlD3D12Renderer : public Rml::RenderInterface {
public:
    RmlD3D12Renderer();
    ~RmlD3D12Renderer() override;

    RmlD3D12Renderer(const RmlD3D12Renderer&) = delete;
    RmlD3D12Renderer& operator=(const RmlD3D12Renderer&) = delete;

    // Build PSO/root-signature/descriptor-heap/white-texture from the given
    // device + DIRECT queue. `hlsl_path` is the rmlui.hlsl source compiled at
    // runtime via D3DCompile. Throws std::runtime_error on any failure.
    void initialize(ID3D12Device* device, ID3D12CommandQueue* queue,
                    const wchar_t* hlsl_path);

    // Begin recording UI into `cmd` for a `width`x`height` context. Binds the
    // SRV heap, root signature, PSO, triangle-list topology, the ortho matrix
    // for this size, and a full-target viewport/scissor. Must be called before
    // the enclosing Context::Render().
    void begin_frame(ID3D12GraphicsCommandList* cmd, std::uint32_t width,
                     std::uint32_t height);
    // Stop routing RenderGeometry into the current command list.
    void end_frame();

    // Rml::RenderInterface — required functions (SPEC §2.1).
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices,
                                                Rml::Span<const int> indices) override;
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f translation,
                        Rml::TextureHandle texture) override;
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                   const Rml::String& source) override;
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source,
                                       Rml::Vector2i source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;

    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(Rml::Rectanglei region) override;

private:
    using ComPtrRes = Microsoft::WRL::ComPtr<ID3D12Resource>;

    struct Geometry {
        ComPtrRes vb;  // UPLOAD-heap, written once, immutable thereafter.
        ComPtrRes ib;
        UINT vb_bytes = 0;
        UINT ib_bytes = 0;
        UINT index_count = 0;
    };

    struct Texture {
        ComPtrRes resource;
        UINT slot = 0;  // index into srv_heap_
        D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
    };

    // Deferred release: a resource + descriptor slot retired at frame N is
    // freed only once N is kFramesInFlight frames in the past (mirrors the
    // presenter's deferred-release pattern so a resource is never destroyed
    // while a still-in-flight command list references it).
    struct Retired {
        std::uint64_t frame = 0;
        ComPtrRes a;
        ComPtrRes b;
        bool has_slot = false;
        UINT slot = 0;
    };

    ComPtrRes create_upload_buffer(UINT64 bytes, const void* data);
    // Uploads RGBA8 pixels into a fresh DEFAULT-heap texture, synchronously
    // (execute + CPU-wait), leaving it in PIXEL_SHADER_RESOURCE state.
    ComPtrRes upload_texture_sync(UINT width, UINT height, const void* rgba,
                                  UINT src_row_pitch);
    Rml::TextureHandle register_texture(ComPtrRes resource, UINT width, UINT height);
    UINT alloc_srv_slot();
    void free_srv_slot(UINT slot);
    D3D12_CPU_DESCRIPTOR_HANDLE srv_cpu(UINT slot) const;
    D3D12_GPU_DESCRIPTOR_HANDLE srv_gpu(UINT slot) const;
    void collect_garbage();

    static constexpr UINT kSrvHeapSize = 512;
    static constexpr UINT kSlotWhite = 0;
    static constexpr std::uint64_t kFramesInFlight = 3;

    Microsoft::WRL::ComPtr<ID3D12Device> device_;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> queue_;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> root_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso_;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srv_heap_;
    UINT srv_inc_ = 0;
    std::vector<UINT> srv_free_;  // free slots (1.. ; slot 0 is the white default)

    // Private upload path (texture staging copies).
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> upload_alloc_;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> upload_list_;
    Microsoft::WRL::ComPtr<ID3D12Fence> upload_fence_;
    UINT64 upload_fence_value_ = 0;
    HANDLE upload_event_ = nullptr;

    Texture white_;  // 1x1 opaque white, bound for untextured (handle 0) geometry.

    std::unordered_map<Rml::CompiledGeometryHandle, Geometry> geometry_;
    std::unordered_map<Rml::TextureHandle, Texture> textures_;
    Rml::CompiledGeometryHandle next_geometry_ = 1;
    Rml::TextureHandle next_texture_ = 1;
    std::vector<Retired> retired_;

    // Per-frame state.
    ID3D12GraphicsCommandList* cmd_ = nullptr;
    std::uint64_t frame_index_ = 0;
    UINT viewport_w_ = 0;
    UINT viewport_h_ = 0;
    float proj_[16] = {};  // column-major ortho for the shader's mul(uProj, v)
    bool scissor_enabled_ = false;
    D3D12_RECT scissor_rect_{};
    D3D12_RECT full_rect_{};
};

}  // namespace ftd::native::ui
