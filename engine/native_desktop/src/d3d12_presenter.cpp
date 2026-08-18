#include "native_desktop/d3d12_presenter.h"

#include "ftd/interop_particle_record.h"

#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace ftd::native_desktop {
namespace {

constexpr std::uint32_t kFrameCount = 2;

void throw_if_failed(HRESULT hr, const char* what) {
    if (FAILED(hr)) {
        throw std::runtime_error(std::string(what) + " HRESULT=" +
                                 std::to_string(static_cast<unsigned long>(hr)));
    }
}

// Enumerates adapters via IDXGIFactory6's GPU-preference ordering when
// available (puts the discrete/high-performance GPU first on hybrid-graphics
// systems), falling back to plain EnumAdapters1 on older DXGI. Returns the
// first adapter that is not the WARP software rasterizer.
//
// Callers (select_hardware_adapter() and initialize()) each pass their own
// independently-created IDXGIFactory4, built with different creation flags
// (0 vs. possibly DXGI_CREATE_FACTORY_DEBUG). This relies on DXGI adapter
// enumeration order being a system-level property, not a property of the
// enumerating factory instance or its creation flags — so two independently
// created factories are expected to enumerate adapters identically, and
// select_hardware_adapter()'s standalone report is expected to match what
// initialize() actually selects, without the two call sites sharing a
// factory. This is an assumption about DXGI/OS behavior, not something this
// code verifies at runtime.
ComPtr<IDXGIAdapter1> pick_hardware_adapter(IDXGIFactory4& factory, LUID* out_luid) {
    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(factory.QueryInterface(IID_PPV_ARGS(&factory6)))) {
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                 i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
             ++i) {
            DXGI_ADAPTER_DESC1 desc{};
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
            if (out_luid) *out_luid = desc.AdapterLuid;
            return adapter;
        }
        return nullptr;
    }
    // Fallback for pre-Windows-10-1803 DXGI (no IDXGIFactory6): plain
    // EnumAdapters1, first non-software hit.
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; SUCCEEDED(factory.EnumAdapters1(i, &adapter)); ++i) {
        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (out_luid) *out_luid = desc.AdapterLuid;
        return adapter;
    }
    return nullptr;
}

struct GpuVertex {
    float x, y, z;
    float r, g, b;
    float u, v;
};

struct CameraConstants {
    float view_proj[16];
    float camera_right[3];
    float _pad0;
    float camera_up[3];
    float _pad1;
};
static_assert(sizeof(CameraConstants) == 96,
              "CameraConstants must match kInteropParticleShader's cbuffer Camera layout exactly");

void perspective(float* out, float fov_y, float aspect, float zn, float zf) {
    const float y = 1.0f / std::tan(fov_y * 0.5f);
    const float x = y / aspect;
    const float q = zf / (zf - zn);
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[0] = x;
    out[5] = y;
    out[10] = q;
    out[11] = 1.0f;
    out[14] = -zn * q;
}

void look_at(float* out, float ex, float ey, float ez, float tx, float ty,
             float tz, float ux, float uy, float uz) {
    float fx = tx - ex, fy = ty - ey, fz = tz - ez;
    float fl = std::sqrt(fx * fx + fy * fy + fz * fz);
    fx /= fl;
    fy /= fl;
    fz /= fl;
    float sx = fy * uz - fz * uy;
    float sy = fz * ux - fx * uz;
    float sz = fx * uy - fy * ux;
    float sl = std::sqrt(sx * sx + sy * sy + sz * sz);
    sx /= sl;
    sy /= sl;
    sz /= sl;
    float ux2 = sy * fz - sz * fy;
    float uy2 = sz * fx - sx * fz;
    float uz2 = sx * fy - sy * fx;
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[0] = sx;
    out[1] = ux2;
    out[2] = fx;
    out[4] = sy;
    out[5] = uy2;
    out[6] = fy;
    out[8] = sz;
    out[9] = uz2;
    out[10] = fz;
    out[12] = -(sx * ex + sy * ey + sz * ez);
    out[13] = -(ux2 * ex + uy2 * ey + uz2 * ez);
    out[14] = -(fx * ex + fy * ey + fz * ez);
    out[15] = 1.0f;
}

void mul4(float* out, const float* a, const float* b) {
    float t[16];
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            t[r * 4 + c] = a[r * 4 + 0] * b[0 * 4 + c] + a[r * 4 + 1] * b[1 * 4 + c] +
                           a[r * 4 + 2] * b[2 * 4 + c] + a[r * 4 + 3] * b[3 * 4 + c];
        }
    }
    for (int i = 0; i < 16; ++i) out[i] = t[i];
}

constexpr char kShader[] = R"(
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) {
    float4x4 viewProj;
};

struct VSIn {
    float3 position : POSITION;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VSOut {
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

VSOut vs_main(VSIn input) {
    VSOut o;
    o.position = mul(float4(input.position, 1.0), viewProj);
    o.color = input.color;
    o.uv = input.uv;
    return o;
}

float4 ps_main(VSOut input) : SV_Target {
    float2 d = input.uv * 2.0 - 1.0;
    float r2 = dot(d, d);
    clip(1.0 - r2);
    float edge = smoothstep(1.0, 0.65, r2);
    return float4(input.color * edge, edge);
}
)";

constexpr char kLineShader[] = R"(
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) {
    float4x4 viewProj;
};
struct VSIn {
    float3 position : POSITION;
    float3 color : COLOR;
};
struct VSOut {
    float4 position : SV_Position;
    float3 color : COLOR;
};
VSOut vs_main(VSIn input) {
    VSOut o;
    o.position = mul(float4(input.position, 1.0), viewProj);
    o.color = input.color;
    return o;
}
float4 ps_main(VSOut input) : SV_Target {
    return float4(input.color, 1.0);
}
)";

constexpr char kInteropParticleShader[] = R"(
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) {
    float4x4 viewProj;
    float3 cameraRight;
    float _pad0;
    float3 cameraUp;
    float _pad1;
};

struct ParticleRecord {
    float3 position;
    float size;
    float3 color;
    float _pad;
};
StructuredBuffer<ParticleRecord> Particles : register(t0);

struct VSOut {
    float4 position : SV_Position;
    float3 color : COLOR;
    float2 uv : TEXCOORD;
};

// One instance per particle, 6 vertices per instance (two triangles making
// a camera-facing quad) -- mirrors the CPU append_sprites() corner/uv tables
// in the pre-interop path exactly (d3d12_presenter.cpp's existing `ox`/`oy`/
// `uv` arrays), so DrawInstanced(6, particle_count, 0, 0) with no vertex or
// index buffer at all reproduces the same geometry the old CPU path built.
static const float2 kCornerOffsets[6] = {
    float2(-1, -1), float2(1, -1), float2(1, 1),
    float2(-1, -1), float2(1, 1),  float2(-1, 1),
};
static const float2 kCornerUVs[6] = {
    float2(0, 0), float2(1, 0), float2(1, 1),
    float2(0, 0), float2(1, 1), float2(0, 1),
};

VSOut vs_main(uint vertex_id : SV_VertexID, uint instance_id : SV_InstanceID) {
    ParticleRecord p = Particles[instance_id];
    float2 corner = kCornerOffsets[vertex_id];
    float3 world = p.position
        + (cameraRight * corner.x + cameraUp * corner.y) * p.size;

    VSOut o;
    o.position = mul(float4(world, 1.0), viewProj);
    o.color = p.color;
    o.uv = kCornerUVs[vertex_id];
    return o;
}

float4 ps_main(VSOut input) : SV_Target {
    float2 d = input.uv * 2.0 - 1.0;
    float r2 = dot(d, d);
    clip(1.0 - r2);
    float edge = smoothstep(1.0, 0.65, r2);
    return float4(input.color * edge, edge);
}
)";

}  // namespace

struct D3D12Presenter::Impl {
    ComPtr<IDXGIFactory4> factory;
    ComPtr<ID3D12Device> device;
    ComPtr<ID3D12CommandQueue> queue;
    ComPtr<IDXGISwapChain3> swapchain;
    ComPtr<ID3D12DescriptorHeap> rtv_heap;
    ComPtr<ID3D12Resource> targets[kFrameCount];
    ComPtr<ID3D12CommandAllocator> allocators[kFrameCount];
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12RootSignature> root;
    ComPtr<ID3D12PipelineState> pso;
    ComPtr<ID3D12PipelineState> pso_lines;
    ComPtr<ID3D12DescriptorHeap> srv_heap;
    ComPtr<ID3D12PipelineState> pso_interop;
    ComPtr<ID3D12DescriptorHeap> dsv_heap;
    ComPtr<ID3D12Resource> depth;
    // Task 11 correction: cb/vb must be genuinely per-frame-slot resources
    // (like targets[]/allocators[] below), not single shared ones -- see the
    // frame_fence_values comment. A single non-slotted resource here would be
    // Map()-overwritten by every render() call regardless of impl_->frame,
    // while the per-slot wait below only guarantees the slot from
    // kFrameCount renders ago has finished, not the immediately-preceding
    // render() call -- i.e. it would not actually protect a shared resource.
    ComPtr<ID3D12Resource> cb[kFrameCount];
    ComPtr<ID3D12Resource> vb[kFrameCount];
    ComPtr<ID3D12Resource> shared_particle_buffer;
    ComPtr<ID3D12Fence> fence;
    // Distinct from `fence` above (the presenter's own present-sync fence,
    // unchanged by this task). This is the cross-API GPU-timeline fence:
    // CUDA signals it after the interop gather kernel; wait_shared_fence()
    // makes the render queue wait on it before the draw that reads the
    // interop buffer.
    ComPtr<ID3D12Fence> shared_fence;
    HANDLE fence_event = nullptr;
    UINT64 fence_value = 0;
    // Per-frame-slot fence values for real double buffering (Task 11): the
    // fence value that was signalled after the most recent submission that
    // used `vb[i]`/`cb[i]` for slot i (index by impl_->frame, mirroring
    // targets[kFrameCount]/allocators[kFrameCount]). 0 means "this slot has
    // never been submitted" -- skip the wait. render() waits on
    // frame_fence_values[frame] at its top, before Map()-writing vb[frame]/
    // cb[frame] for that slot, instead of the old per-frame wait_idle()
    // full-pipeline stall.
    UINT64 frame_fence_values[kFrameCount] = {};
    UINT frame = 0;
    UINT rtv_size = 0;
    std::size_t vb_capacity[kFrameCount] = {};
    void* cb_mapped[kFrameCount] = {};
};

D3D12Presenter::D3D12Presenter() : impl_(std::make_unique<Impl>()) {}

D3D12Presenter::~D3D12Presenter() {
    try {
        wait_idle();
    } catch (...) {
    }
    if (impl_ && impl_->fence_event) CloseHandle(impl_->fence_event);
}

bool D3D12Presenter::select_hardware_adapter(LUID* out_luid, bool* out_is_hardware) {
    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
        if (out_is_hardware) *out_is_hardware = false;
        return false;
    }
    LUID luid{};
    ComPtr<IDXGIAdapter1> adapter = pick_hardware_adapter(*factory.Get(), &luid);
    if (!adapter) {
        if (out_is_hardware) *out_is_hardware = false;
        return false;
    }
    if (out_luid) *out_luid = luid;
    if (out_is_hardware) *out_is_hardware = true;
    return true;
}

void D3D12Presenter::initialize(HWND hwnd, std::uint32_t width,
                                std::uint32_t height) {
    UINT factory_flags = 0;
#if defined(_DEBUG)
    ComPtr<ID3D12Debug> debug;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)))) {
        debug->EnableDebugLayer();
        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif
    throw_if_failed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&impl_->factory)),
                    "CreateDXGIFactory2");
    ComPtr<IDXGIAdapter1> adapter =
        pick_hardware_adapter(*impl_->factory.Get(), &adapter_luid_);
    throw_if_failed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                      IID_PPV_ARGS(&impl_->device)),
                    "D3D12CreateDevice");
    has_adapter_luid_ = static_cast<bool>(adapter);

    D3D12_COMMAND_QUEUE_DESC q{};
    q.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    throw_if_failed(impl_->device->CreateCommandQueue(&q, IID_PPV_ARGS(&impl_->queue)),
                    "CreateCommandQueue");

    DXGI_SWAP_CHAIN_DESC1 sd{};
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = kFrameCount;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    ComPtr<IDXGISwapChain1> sc1;
    throw_if_failed(impl_->factory->CreateSwapChainForHwnd(
                        impl_->queue.Get(), hwnd, &sd, nullptr, nullptr, &sc1),
                    "CreateSwapChainForHwnd");
    throw_if_failed(sc1.As(&impl_->swapchain), "Query IDXGISwapChain3");
    impl_->factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    D3D12_DESCRIPTOR_HEAP_DESC heap{};
    heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    heap.NumDescriptors = kFrameCount;
    throw_if_failed(
        impl_->device->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&impl_->rtv_heap)),
        "CreateDescriptorHeap");
    impl_->rtv_size =
        impl_->device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    for (UINT i = 0; i < kFrameCount; ++i) {
        throw_if_failed(
            impl_->device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                  IID_PPV_ARGS(&impl_->allocators[i])),
            "CreateCommandAllocator");
    }
    throw_if_failed(impl_->device->CreateCommandList(
                        0, D3D12_COMMAND_LIST_TYPE_DIRECT, impl_->allocators[0].Get(),
                        nullptr, IID_PPV_ARGS(&impl_->list)),
                    "CreateCommandList");
    impl_->list->Close();

    D3D12_DESCRIPTOR_RANGE srv_range{};
    srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srv_range.NumDescriptors = 1;
    srv_range.BaseShaderRegister = 0;
    srv_range.RegisterSpace = 0;
    srv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER params[2]{};
    params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[0].Descriptor.ShaderRegister = 0;
    params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[1].DescriptorTable.NumDescriptorRanges = 1;
    params[1].DescriptorTable.pDescriptorRanges = &srv_range;

    D3D12_ROOT_SIGNATURE_DESC rs{};
    rs.NumParameters = 2;
    rs.pParameters = params;
    rs.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    ComPtr<ID3DBlob> blob, err;
    throw_if_failed(
        D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &err),
        "D3D12SerializeRootSignature");
    throw_if_failed(impl_->device->CreateRootSignature(
                        0, blob->GetBufferPointer(), blob->GetBufferSize(),
                        IID_PPV_ARGS(&impl_->root)),
                    "CreateRootSignature");

    ComPtr<ID3DBlob> vs, ps, serr;
    throw_if_failed(D3DCompile(kShader, sizeof(kShader) - 1, "particle", nullptr,
                               nullptr, "vs_main", "vs_5_1", 0, 0, &vs, &serr),
                    "D3DCompile vs");
    throw_if_failed(D3DCompile(kShader, sizeof(kShader) - 1, "particle", nullptr,
                               nullptr, "ps_main", "ps_5_1", 0, 0, &ps, &serr),
                    "D3DCompile ps");

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
    pso.pRootSignature = impl_->root.Get();
    pso.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
    pso.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
    pso.BlendState.RenderTarget[0].RenderTargetWriteMask = 0x0F;
    pso.BlendState.RenderTarget[0].BlendEnable = TRUE;
    pso.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    pso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    pso.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    pso.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    pso.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    pso.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable = TRUE;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    pso.SampleDesc.Count = 1;
    pso.InputLayout = {layout, 3};
    pso.DepthStencilState.DepthEnable = TRUE;
    pso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    pso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&impl_->pso)),
        "CreateGraphicsPipelineState");

    ComPtr<ID3DBlob> lvs, lps, lerr;
    throw_if_failed(D3DCompile(kLineShader, sizeof(kLineShader) - 1, "line", nullptr,
                               nullptr, "vs_main", "vs_5_1", 0, 0, &lvs, &lerr),
                    "D3DCompile line vs");
    throw_if_failed(D3DCompile(kLineShader, sizeof(kLineShader) - 1, "line", nullptr,
                               nullptr, "ps_main", "ps_5_1", 0, 0, &lps, &lerr),
                    "D3DCompile line ps");
    D3D12_INPUT_ELEMENT_DESC line_layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC lpso = pso;
    lpso.VS = {lvs->GetBufferPointer(), lvs->GetBufferSize()};
    lpso.PS = {lps->GetBufferPointer(), lps->GetBufferSize()};
    lpso.BlendState.RenderTarget[0].BlendEnable = FALSE;
    lpso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    lpso.InputLayout = {line_layout, 2};
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&lpso, IID_PPV_ARGS(&impl_->pso_lines)),
        "CreateGraphicsPipelineState lines");

    ComPtr<ID3DBlob> ivs, ips, ierr;
    throw_if_failed(D3DCompile(kInteropParticleShader, sizeof(kInteropParticleShader) - 1,
                               "interop_particle", nullptr, nullptr, "vs_main",
                               "vs_5_1", 0, 0, &ivs, &ierr),
                    "D3DCompile interop vs");
    throw_if_failed(D3DCompile(kInteropParticleShader, sizeof(kInteropParticleShader) - 1,
                               "interop_particle", nullptr, nullptr, "ps_main",
                               "ps_5_1", 0, 0, &ips, &ierr),
                    "D3DCompile interop ps");
    D3D12_GRAPHICS_PIPELINE_STATE_DESC ipso = pso;  // reuse blend/raster/depth state
    ipso.VS = {ivs->GetBufferPointer(), ivs->GetBufferSize()};
    ipso.PS = {ips->GetBufferPointer(), ips->GetBufferSize()};
    ipso.InputLayout = {nullptr, 0};  // no input assembler -- SV_VertexID/SV_InstanceID only
    throw_if_failed(impl_->device->CreateGraphicsPipelineState(
                        &ipso, IID_PPV_ARGS(&impl_->pso_interop)),
                    "CreateGraphicsPipelineState interop");

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.NumDescriptors = 1;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    throw_if_failed(impl_->device->CreateDescriptorHeap(
                        &srv_heap_desc, IID_PPV_ARGS(&impl_->srv_heap)),
                    "CreateDescriptorHeap SRV");

    D3D12_DESCRIPTOR_HEAP_DESC dsv_desc{};
    dsv_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsv_desc.NumDescriptors = 1;
    throw_if_failed(
        impl_->device->CreateDescriptorHeap(&dsv_desc, IID_PPV_ARGS(&impl_->dsv_heap)),
        "CreateDescriptorHeap DSV");

    D3D12_HEAP_PROPERTIES upload{};
    upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC cb_desc{};
    cb_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    cb_desc.Width = 256;
    cb_desc.Height = 1;
    cb_desc.DepthOrArraySize = 1;
    cb_desc.MipLevels = 1;
    cb_desc.SampleDesc.Count = 1;
    cb_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    // One constant buffer per frame slot (Task 11 correction): a single
    // shared cb would be Map()-overwritten every render() call regardless of
    // which slot's prior GPU submission is still in flight -- see the Impl
    // struct comment on cb/vb above.
    for (UINT i = 0; i < kFrameCount; ++i) {
        throw_if_failed(impl_->device->CreateCommittedResource(
                            &upload, D3D12_HEAP_FLAG_NONE, &cb_desc,
                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                            IID_PPV_ARGS(&impl_->cb[i])),
                        "CreateCommittedResource cb");
        throw_if_failed(impl_->cb[i]->Map(0, nullptr, &impl_->cb_mapped[i]), "Map cb");
    }

    throw_if_failed(
        impl_->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&impl_->fence)),
        "CreateFence");
    impl_->fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!impl_->fence_event) throw std::runtime_error("CreateEventW failed");

    resize(width, height);
}

void D3D12Presenter::resize(std::uint32_t width, std::uint32_t height) {
    if (!impl_->swapchain) return;
    wait_idle();
    width_ = width < 1 ? 1 : width;
    height_ = height < 1 ? 1 : height;
    for (UINT i = 0; i < kFrameCount; ++i) impl_->targets[i].Reset();
    throw_if_failed(impl_->swapchain->ResizeBuffers(kFrameCount, width_, height_,
                                                    DXGI_FORMAT_R8G8B8A8_UNORM, 0),
                    "ResizeBuffers");
    D3D12_CPU_DESCRIPTOR_HANDLE handle =
        impl_->rtv_heap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < kFrameCount; ++i) {
        throw_if_failed(
            impl_->swapchain->GetBuffer(i, IID_PPV_ARGS(&impl_->targets[i])),
            "GetBuffer");
        impl_->device->CreateRenderTargetView(impl_->targets[i].Get(), nullptr, handle);
        handle.ptr += impl_->rtv_size;
    }

    impl_->depth.Reset();
    D3D12_HEAP_PROPERTIES heap_default{};
    heap_default.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC depth_desc{};
    depth_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depth_desc.Width = width_;
    depth_desc.Height = height_;
    depth_desc.DepthOrArraySize = 1;
    depth_desc.MipLevels = 1;
    depth_desc.Format = DXGI_FORMAT_D32_FLOAT;
    depth_desc.SampleDesc.Count = 1;
    depth_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    D3D12_CLEAR_VALUE depth_clear{};
    depth_clear.Format = DXGI_FORMAT_D32_FLOAT;
    depth_clear.DepthStencil.Depth = 1.0f;
    throw_if_failed(impl_->device->CreateCommittedResource(
                        &heap_default, D3D12_HEAP_FLAG_NONE, &depth_desc,
                        D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear,
                        IID_PPV_ARGS(&impl_->depth)),
                    "CreateCommittedResource depth");
    impl_->device->CreateDepthStencilView(
        impl_->depth.Get(), nullptr,
        impl_->dsv_heap->GetCPUDescriptorHandleForHeapStart());

    impl_->frame = impl_->swapchain->GetCurrentBackBufferIndex();
}

void D3D12Presenter::wait_idle() {
    if (!impl_->queue) return;
    ++impl_->fence_value;
    throw_if_failed(impl_->queue->Signal(impl_->fence.Get(), impl_->fence_value),
                    "Signal");
    if (impl_->fence->GetCompletedValue() < impl_->fence_value) {
        throw_if_failed(
            impl_->fence->SetEventOnCompletion(impl_->fence_value, impl_->fence_event),
            "SetEventOnCompletion");
        WaitForSingleObject(impl_->fence_event, INFINITE);
    }
}

HANDLE D3D12Presenter::create_shared_particle_buffer(std::uint32_t max_particles) {
    if (!impl_->device) return nullptr;

    // Reset up front so every failure path below (including the two
    // CreateCommittedResource/CreateSharedHandle branches) leaves this at 0
    // rather than reporting a stale size for a buffer that no longer
    // exists -- IID_PPV_ARGS(&impl_->shared_particle_buffer) below releases
    // any previously-held resource via ReleaseAndGetAddressOf() regardless
    // of whether the new CreateCommittedResource call succeeds.
    shared_particle_buffer_bytes_ = 0;

    // Wait for outstanding GPU work before releasing/replacing any
    // previous shared buffer -- mirrors resize()'s wait_idle() before it
    // recreates impl_->depth. Harmless (cheap) on a queue with nothing in
    // flight, e.g. the very first call.
    wait_idle();

    const UINT64 bytes = static_cast<UINT64>(max_particles) *
                         sizeof(ftd::InteropParticleRecord);
    D3D12_HEAP_PROPERTIES heap_default{};
    heap_default.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    // ALLOW_UNORDERED_ACCESS: CUDA writes it. D3D12_RESOURCE_FLAG_ALLOW_
    // SIMULTANEOUS_ACCESS was considered here too (cross-API access from
    // CUDA bypasses D3D12's resource-state/barrier tracking) but the D3D12
    // validation layer rejects it outright on a buffer resource: "MiscFlag
    // cannot have D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS set when
    // Dimension is D3D12_RESOURCE_DIMENSION_BUFFER" (confirmed against a
    // live device -- CreateCommittedResource returns E_INVALIDARG). The
    // flag is texture-only; D3D12 buffers have no compression/tiling
    // metadata for a fence-based cross-API handoff to invalidate, so no
    // buffer-side equivalent is needed for this resource.
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    // max_particles == 0 (not-yet-populated caller state, not an error) is
    // intentionally unguarded here: Width == 0 makes CreateCommittedResource
    // fail on its own, which the existing failure path below already
    // handles correctly.
    if (FAILED(impl_->device->CreateCommittedResource(
            &heap_default, D3D12_HEAP_FLAG_SHARED, &desc,
            D3D12_RESOURCE_STATE_COMMON, nullptr,
            IID_PPV_ARGS(&impl_->shared_particle_buffer)))) {
        return nullptr;
    }
    HANDLE handle = nullptr;
    if (FAILED(impl_->device->CreateSharedHandle(
            impl_->shared_particle_buffer.Get(), nullptr, GENERIC_ALL, nullptr,
            &handle))) {
        impl_->shared_particle_buffer.Reset();
        return nullptr;
    }
    shared_particle_buffer_bytes_ = bytes;
    return handle;
}

void D3D12Presenter::bind_interop_particle_srv() {
    if (!impl_->shared_particle_buffer) return;
    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Format = DXGI_FORMAT_UNKNOWN;
    srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Buffer.NumElements = static_cast<UINT>(
        shared_particle_buffer_bytes_ / sizeof(ftd::InteropParticleRecord));
    srv.Buffer.StructureByteStride = sizeof(ftd::InteropParticleRecord);
    impl_->device->CreateShaderResourceView(
        impl_->shared_particle_buffer.Get(), &srv,
        impl_->srv_heap->GetCPUDescriptorHandleForHeapStart());
}

HANDLE D3D12Presenter::create_shared_fence() {
    if (!impl_->device) return nullptr;

    // Wait for outstanding GPU work before releasing/replacing any previous
    // shared fence -- mirrors create_shared_particle_buffer()'s wait_idle()
    // before it recreates/replaces impl_->shared_particle_buffer, for the
    // identical reason: IID_PPV_ARGS(&impl_->shared_fence) below releases any
    // previously-held fence via ReleaseAndGetAddressOf() regardless of
    // whether the new CreateFence call succeeds, and an in-flight
    // queue->Wait() against the OLD fence object must not be torn out from
    // under outstanding GPU work. Harmless (cheap) on a queue with nothing in
    // flight, e.g. the very first call.
    wait_idle();

    if (FAILED(impl_->device->CreateFence(0, D3D12_FENCE_FLAG_SHARED,
                                          IID_PPV_ARGS(&impl_->shared_fence)))) {
        return nullptr;
    }
    HANDLE handle = nullptr;
    if (FAILED(impl_->device->CreateSharedHandle(impl_->shared_fence.Get(), nullptr,
                                                 GENERIC_ALL, nullptr, &handle))) {
        impl_->shared_fence.Reset();
        return nullptr;
    }
    return handle;
}

void D3D12Presenter::wait_shared_fence(std::uint64_t value) {
    // Mirrors wait_idle()'s identical `if (!impl_->queue) return;` guard for
    // the same precondition (called before initialize()). impl_->shared_fence
    // can't be set without impl_->device/impl_->queue already existing
    // (create_shared_fence() requires impl_->device), so this is defensive
    // rather than reachable in practice today.
    if (!impl_->queue || !impl_->shared_fence) return;
    // Every other consequential D3D12 call in this file (including the
    // structurally identical impl_->queue->Signal() in wait_idle()) is
    // wrapped in throw_if_failed -- Wait() is the actual GPU-timeline
    // synchronization primitive this task exists to add, so a silently
    // discarded failure here (device-removed/TDR, stale fence object, wrong
    // queue type) would leave the caller with zero signal that the
    // subsequent draw call is about to proceed unsynchronized.
    throw_if_failed(impl_->queue->Wait(impl_->shared_fence.Get(), value),
                    "Wait shared_fence");
}

void* D3D12Presenter::debug_device() const {
    return impl_->device.Get();
}

void D3D12Presenter::render(const NativeFrame& frame, const Camera& camera,
                            const NativeViewOptions& opts,
                            std::uint32_t interop_particle_count) {
    // Wait for THIS frame slot's own last submission (kFrameCount frames
    // ago) to finish being read by the GPU before this call's Map()s below
    // overwrite vb[frame]/cb[frame] for it. Skipped on a slot's first-ever
    // use (fence value 0 means "never submitted"). This is only correct
    // because vb/cb are themselves per-slot arrays (Task 11 correction) --
    // a shared, non-slotted vb/cb would not be protected by a per-slot wait.
    const UINT64 needed = impl_->frame_fence_values[impl_->frame];
    if (needed != 0 && impl_->fence->GetCompletedValue() < needed) {
        throw_if_failed(
            impl_->fence->SetEventOnCompletion(needed, impl_->fence_event),
            "SetEventOnCompletion");
        WaitForSingleObject(impl_->fence_event, INFINITE);
    }

    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    const float cy = std::cos(camera.pitch);
    const float eye_x = camera.target_x + camera.distance * cy * std::sin(camera.yaw);
    const float eye_y = camera.target_y + camera.distance * std::sin(camera.pitch);
    const float eye_z = camera.target_z + camera.distance * cy * std::cos(camera.yaw);

    float view[16], proj[16], vp[16];
    look_at(view, eye_x, eye_y, eye_z, camera.target_x, camera.target_y,
            camera.target_z, 0.0f, 1.0f, 0.0f);
    perspective(proj, camera.fov_y, aspect, 0.1f, 2048.0f);
    mul4(vp, view, proj);

    const float rx = view[0], ry = view[4], rz = view[8];
    const float ux = view[1], uy = view[5], uz = view[9];

    CameraConstants cbuf{};
    std::memcpy(cbuf.view_proj, vp, sizeof(vp));
    cbuf.camera_right[0] = rx; cbuf.camera_right[1] = ry; cbuf.camera_right[2] = rz;
    cbuf.camera_up[0] = ux; cbuf.camera_up[1] = uy; cbuf.camera_up[2] = uz;
    std::memcpy(impl_->cb_mapped[impl_->frame], &cbuf, sizeof(cbuf));

    const float uv[6][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 0}, {1, 1}, {0, 1}};
    const float ox[6] = {-1, 1, 1, -1, 1, -1};
    const float oy[6] = {-1, -1, 1, -1, 1, 1};

    auto append_sprites = [&](const std::vector<NativeParticle>& src,
                              std::vector<GpuVertex>& dst) {
        for (const NativeParticle& p : src) {
            const float size = p.size > 0.0f ? p.size : 0.45f;
            for (int i = 0; i < 6; ++i) {
                GpuVertex v;
                v.x = p.x + (rx * ox[i] + ux * oy[i]) * size;
                v.y = p.y + (ry * ox[i] + uy * oy[i]) * size;
                v.z = p.z + (rz * ox[i] + uz * oy[i]) * size;
                v.r = p.r;
                v.g = p.g;
                v.b = p.b;
                v.u = uv[i][0];
                v.v = uv[i][1];
                dst.push_back(v);
            }
        }
    };

    struct LineVertex {
        float x, y, z, r, g, b;
    };
    const float L = static_cast<float>(std::max(1, frame.lattice_size));
    const float cr = 0.35f, cg = 0.42f, cb = 0.52f;
    const float corners[8][3] = {
        {0, 0, 0}, {L, 0, 0}, {L, L, 0}, {0, L, 0},
        {0, 0, L}, {L, 0, L}, {L, L, L}, {0, L, L},
    };
    const int edges[12][2] = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    };
    LineVertex lines[24];
    for (int e = 0; e < 12; ++e) {
        for (int k = 0; k < 2; ++k) {
            const int ci = edges[e][k];
            LineVertex v{corners[ci][0], corners[ci][1], corners[ci][2], cr, cg, cb};
            lines[e * 2 + k] = v;
        }
    }

    std::vector<GpuVertex> verts;
    verts.reserve((frame.flux.size() + frame.particles.size()) * 6);
    if (opts.flux) append_sprites(frame.flux, verts);
    const UINT flux_verts = static_cast<UINT>(verts.size());
    // When interop_particle_count > 0, particles are drawn from the imported
    // D3D12 buffer via the interop PSO below instead of the CPU-expanded
    // vertex buffer -- do not double-draw. frame.particles stays populated
    // either way (the CPU backend has no interop path and always needs it).
    if (opts.particles && interop_particle_count == 0) append_sprites(frame.particles, verts);

    const std::size_t sprite_bytes = verts.size() * sizeof(GpuVertex);
    const std::size_t line_bytes = sizeof(lines);
    const std::size_t bytes = sprite_bytes + line_bytes;
    if (bytes > impl_->vb_capacity[impl_->frame]) {
        impl_->vb[impl_->frame].Reset();
        D3D12_HEAP_PROPERTIES upload{};
        upload.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Width = bytes < 256 ? 256 : bytes;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.SampleDesc.Count = 1;
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        throw_if_failed(impl_->device->CreateCommittedResource(
                            &upload, D3D12_HEAP_FLAG_NONE, &desc,
                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                            IID_PPV_ARGS(&impl_->vb[impl_->frame])),
                        "CreateCommittedResource vb");
        impl_->vb_capacity[impl_->frame] = static_cast<std::size_t>(desc.Width);
    }
    if (impl_->vb[impl_->frame]) {
        void* mapped = nullptr;
        throw_if_failed(impl_->vb[impl_->frame]->Map(0, nullptr, &mapped), "Map vb");
        auto* bytes_out = static_cast<std::uint8_t*>(mapped);
        std::memcpy(bytes_out, lines, line_bytes);
        if (sprite_bytes != 0) {
            std::memcpy(bytes_out + line_bytes, verts.data(), sprite_bytes);
        }
        impl_->vb[impl_->frame]->Unmap(0, nullptr);
    }

    throw_if_failed(impl_->allocators[impl_->frame]->Reset(), "allocator Reset");
    throw_if_failed(
        impl_->list->Reset(impl_->allocators[impl_->frame].Get(), impl_->pso_lines.Get()),
        "list Reset");

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = impl_->targets[impl_->frame].Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    impl_->list->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtv =
        impl_->rtv_heap->GetCPUDescriptorHandleForHeapStart();
    rtv.ptr += impl_->frame * impl_->rtv_size;
    D3D12_CPU_DESCRIPTOR_HANDLE dsv =
        impl_->dsv_heap->GetCPUDescriptorHandleForHeapStart();
    impl_->list->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
    const float clear[4] = {0.04f, 0.05f, 0.07f, 1.0f};
    impl_->list->ClearRenderTargetView(rtv, clear, 0, nullptr);
    impl_->list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    D3D12_VIEWPORT vp_desc{0.0f, 0.0f, static_cast<float>(width_),
                           static_cast<float>(height_), 0.0f, 1.0f};
    D3D12_RECT scissor{0, 0, static_cast<LONG>(width_), static_cast<LONG>(height_)};
    impl_->list->RSSetViewports(1, &vp_desc);
    impl_->list->RSSetScissorRects(1, &scissor);
    impl_->list->SetGraphicsRootSignature(impl_->root.Get());
    impl_->list->SetGraphicsRootConstantBufferView(
        0, impl_->cb[impl_->frame]->GetGPUVirtualAddress());

    if (opts.lattice_box && impl_->vb[impl_->frame]) {
        D3D12_VERTEX_BUFFER_VIEW line_view{};
        line_view.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress();
        line_view.SizeInBytes = static_cast<UINT>(line_bytes);
        line_view.StrideInBytes = sizeof(LineVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        impl_->list->IASetVertexBuffers(0, 1, &line_view);
        impl_->list->DrawInstanced(24, 1, 0, 0);
    }

    if (sprite_bytes != 0) {
        impl_->list->SetPipelineState(impl_->pso.Get());
        D3D12_VERTEX_BUFFER_VIEW sprite_view{};
        sprite_view.BufferLocation =
            impl_->vb[impl_->frame]->GetGPUVirtualAddress() + line_bytes;
        sprite_view.SizeInBytes = static_cast<UINT>(sprite_bytes);
        sprite_view.StrideInBytes = sizeof(GpuVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &sprite_view);
        if (flux_verts != 0) {
            impl_->list->DrawInstanced(flux_verts, 1, 0, 0);
        }
        const UINT particle_verts = static_cast<UINT>(verts.size()) - flux_verts;
        if (particle_verts != 0) {
            impl_->list->DrawInstanced(particle_verts, 1, flux_verts, 0);
        }
    }

    if (opts.particles && interop_particle_count != 0 && impl_->pso_interop && impl_->srv_heap) {
        impl_->list->SetPipelineState(impl_->pso_interop.Get());
        ID3D12DescriptorHeap* heaps[] = {impl_->srv_heap.Get()};
        impl_->list->SetDescriptorHeaps(1, heaps);
        impl_->list->SetGraphicsRootDescriptorTable(
            1, impl_->srv_heap->GetGPUDescriptorHandleForHeapStart());
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->DrawInstanced(6, interop_particle_count, 0, 0);
    }

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    impl_->list->ResourceBarrier(1, &barrier);
    throw_if_failed(impl_->list->Close(), "Close");
    ID3D12CommandList* lists[] = {impl_->list.Get()};
    impl_->queue->ExecuteCommandLists(1, lists);
    throw_if_failed(impl_->swapchain->Present(1, 0), "Present");

    ++impl_->fence_value;
    throw_if_failed(impl_->queue->Signal(impl_->fence.Get(), impl_->fence_value),
                    "Signal");
    impl_->frame_fence_values[impl_->frame] = impl_->fence_value;
    impl_->frame = impl_->swapchain->GetCurrentBackBufferIndex();
}

}  // namespace ftd::native_desktop
