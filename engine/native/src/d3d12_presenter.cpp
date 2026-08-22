#include "native/d3d12_presenter.h"

#include "ftd/interop_particle_record.h"

#include <d3d12.h>
#include <d3d12sdklayers.h>
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

namespace ftd::native {
namespace {

constexpr std::uint32_t kFrameCount = 2;
static_assert(kFrameCount == D3D12Presenter::kFrameCount, "frame-in-flight pin");

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

// Rubber-sheet surface shader (Tranche 5): a triangle mesh with a per-vertex
// RGBA colour, transformed by the same view/proj the rest of the scene uses.
// The app's first non-billboard surface — drawn double-sided (cull NONE),
// alpha-blended, and depth-tested (no depth write) so the translucent sheets
// sit correctly among the particles and lines. `ps_main` shades the solid
// surface; `ps_wire` shades the wireframe pass (a brighter, lower-alpha scaffold
// that follows the ramp), drawn over the solid surface for the web mesh+wire look.
constexpr char kSheetShader[] = R"(
#pragma pack_matrix(row_major)
cbuffer Camera : register(b0) {
    float4x4 viewProj;
};
struct VSIn {
    float3 position : POSITION;
    float4 color : COLOR;
};
struct VSOut {
    float4 position : SV_Position;
    float4 color : COLOR;
};
VSOut vs_main(VSIn input) {
    VSOut o;
    o.position = mul(float4(input.position, 1.0), viewProj);
    o.color = input.color;
    return o;
}
float4 ps_main(VSOut input) : SV_Target {
    return input.color;
}
float4 ps_wire(VSOut input) : SV_Target {
    float3 c = saturate(input.color.rgb * 1.3 + 0.12);
    return float4(c, 0.22);
}
)";

// Force-Heatmap shader: the sprite VS (camera-facing quad, from CPU-expanded
// GpuVertex) paired with a gaussian-falloff PS. Additive glow: the PS returns a
// pre-multiplied colour weighted by exp(-r²·16) (port of updateForceHeatmap's
// heatFrag), and the heat PSO uses pure additive blending (SrcBlend ONE / DstBlend
// ONE) so overlapping blobs sum into a soft density field.
constexpr char kHeatShader[] = R"(
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
    if (r2 > 1.0) discard;
    float gauss = exp(-r2 * 16.0);
    const float opacity = 0.85;
    return float4(input.color * gauss * opacity, 1.0);  // additive (pre-multiplied)
}
)";

// Force-Glyph shader: world-space cone triangles (CPU-tessellated into GpuVertex,
// positions already in world space, uv unused). Flat-shaded — the PS reconstructs
// each facet's normal from screen-space derivatives of the interpolated world
// position (cross(ddx, ddy)) and applies simple hemispheric + head-lamp lighting,
// so the cones read as solid 3-D arrowheads without per-vertex normals. Opaque,
// depth test + write.
constexpr char kGlyphShader[] = R"(
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
    float3 world : TEXCOORD0;
};
VSOut vs_main(VSIn input) {
    VSOut o;
    o.position = mul(float4(input.position, 1.0), viewProj);
    o.color = input.color;
    o.world = input.position;
    return o;
}
float4 ps_main(VSOut input) : SV_Target {
    float3 n = normalize(cross(ddx(input.world), ddy(input.world)));
    float3 lightDir = normalize(float3(0.4, 0.8, 0.35));
    float diff = abs(dot(n, lightDir));            // two-sided (cones are open-ish)
    float shade = 0.42 + 0.58 * diff;
    return float4(input.color * shade, 1.0);
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
    // Tranche-5 rubber-sheet PSOs: a triangle-mesh vertex-colour surface
    // (double-sided, alpha-blended, depth-tested no-write) plus a wireframe
    // variant (FillMode WIREFRAME on the same mesh) for the mesh+wire look.
    ComPtr<ID3D12PipelineState> pso_sheet;
    ComPtr<ID3D12PipelineState> pso_sheet_wire;
    // Force render-style PSOs (share the sprite GpuVertex layout + camera CBV):
    // pso_heat = additive gaussian-falloff glow sprites (Force Heatmap style);
    // pso_glyph = opaque flat-shaded world-space cone triangles (Force Glyphs
    // style, the new instanced-cone triangle pipeline). Both are drawn as
    // sub-ranges of the same per-frame sprite vertex buffer.
    ComPtr<ID3D12PipelineState> pso_heat;
    ComPtr<ID3D12PipelineState> pso_glyph;
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
    // Per-frame-slot rubber-sheet vertex + index buffers (same double-buffering
    // discipline as vb[]/cb[]): grown on demand, Map()-written each render() for
    // impl_->frame, protected by that slot's frame_fence_values wait.
    ComPtr<ID3D12Resource> sheet_vb[kFrameCount];
    ComPtr<ID3D12Resource> sheet_ib[kFrameCount];
    std::size_t sheet_vb_capacity[kFrameCount] = {};
    std::size_t sheet_ib_capacity[kFrameCount] = {};
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
    ComPtr<ID3D12InfoQueue> info_queue;
    UINT srv_size = 0;
    std::vector<UINT> srv_free;
    ComPtr<ID3D12Resource> capture_readback;
    UINT capture_row_pitch = 0;
    UINT capture_width = 0;
    UINT capture_height = 0;
    UINT64 capture_fence_value = 0;
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
                                std::uint32_t height,
                                const D3D12PresenterOptions& options) {
    UINT factory_flags = 0;
    if (options.enable_debug_layer) {
        ComPtr<ID3D12Debug> debug;
        throw_if_failed(D3D12GetDebugInterface(IID_PPV_ARGS(&debug)),
                        "D3D12GetDebugInterface");
        debug->EnableDebugLayer();
        factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
    }
    throw_if_failed(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&impl_->factory)),
                    "CreateDXGIFactory2");
    ComPtr<IDXGIAdapter1> adapter =
        pick_hardware_adapter(*impl_->factory.Get(), &adapter_luid_);
    throw_if_failed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                      IID_PPV_ARGS(&impl_->device)),
                    "D3D12CreateDevice");
    has_adapter_luid_ = static_cast<bool>(adapter);
    if (options.enable_debug_layer) {
        throw_if_failed(impl_->device.As(&impl_->info_queue),
                        "Query ID3D12InfoQueue");
        impl_->info_queue->ClearStoredMessages();
    }

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

    // ── Rubber-sheet surface PSOs (Tranche 5) ──────────────────────────────────
    // A triangle-mesh vertex-colour surface: reuse the sprite PSO's alpha-blend +
    // RT/DSV formats, but draw double-sided (CullMode NONE, already set), keep
    // depth TEST but disable depth WRITE (translucent surfaces must not occlude
    // later transparent draws), and take a {POSITION rgb, COLOR rgba} input
    // layout. The wireframe variant is the same PSO with FillMode WIREFRAME and
    // the ps_wire pixel shader.
    ComPtr<ID3DBlob> svs, sps, swps, serr2;
    throw_if_failed(D3DCompile(kSheetShader, sizeof(kSheetShader) - 1, "sheet", nullptr,
                               nullptr, "vs_main", "vs_5_1", 0, 0, &svs, &serr2),
                    "D3DCompile sheet vs");
    throw_if_failed(D3DCompile(kSheetShader, sizeof(kSheetShader) - 1, "sheet", nullptr,
                               nullptr, "ps_main", "ps_5_1", 0, 0, &sps, &serr2),
                    "D3DCompile sheet ps");
    throw_if_failed(D3DCompile(kSheetShader, sizeof(kSheetShader) - 1, "sheet", nullptr,
                               nullptr, "ps_wire", "ps_5_1", 0, 0, &swps, &serr2),
                    "D3DCompile sheet ps_wire");
    D3D12_INPUT_ELEMENT_DESC sheet_layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC spso = pso;  // reuse blend/RT/DSV formats
    spso.VS = {svs->GetBufferPointer(), svs->GetBufferSize()};
    spso.PS = {sps->GetBufferPointer(), sps->GetBufferSize()};
    spso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    spso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // double-sided
    spso.DepthStencilState.DepthEnable = TRUE;
    spso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // test, no write
    spso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    spso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    spso.InputLayout = {sheet_layout, 2};
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&spso, IID_PPV_ARGS(&impl_->pso_sheet)),
        "CreateGraphicsPipelineState sheet");
    D3D12_GRAPHICS_PIPELINE_STATE_DESC swpso = spso;
    swpso.PS = {swps->GetBufferPointer(), swps->GetBufferSize()};
    swpso.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&swpso, IID_PPV_ARGS(&impl_->pso_sheet_wire)),
        "CreateGraphicsPipelineState sheet wire");

    // ── Force render-style PSOs (Heatmap glow + Glyph cones) ────────────────────
    // Both reuse the sprite GpuVertex input layout (POSITION/COLOR/TEXCOORD) and
    // the camera CBV, and draw as sub-ranges of the same per-frame sprite buffer.
    ComPtr<ID3DBlob> hvs, hps, herr;
    throw_if_failed(D3DCompile(kHeatShader, sizeof(kHeatShader) - 1, "heat", nullptr,
                               nullptr, "vs_main", "vs_5_1", 0, 0, &hvs, &herr),
                    "D3DCompile heat vs");
    throw_if_failed(D3DCompile(kHeatShader, sizeof(kHeatShader) - 1, "heat", nullptr,
                               nullptr, "ps_main", "ps_5_1", 0, 0, &hps, &herr),
                    "D3DCompile heat ps");
    D3D12_GRAPHICS_PIPELINE_STATE_DESC hpso = pso;  // sprite layout/RT/DSV
    hpso.VS = {hvs->GetBufferPointer(), hvs->GetBufferSize()};
    hpso.PS = {hps->GetBufferPointer(), hps->GetBufferSize()};
    hpso.BlendState.RenderTarget[0].BlendEnable = TRUE;   // pure additive glow
    hpso.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    hpso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    hpso.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    hpso.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    hpso.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
    hpso.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    hpso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;  // glow must not occlude
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&hpso, IID_PPV_ARGS(&impl_->pso_heat)),
        "CreateGraphicsPipelineState heat");

    ComPtr<ID3DBlob> gvs, gps, gerr;
    throw_if_failed(D3DCompile(kGlyphShader, sizeof(kGlyphShader) - 1, "glyph", nullptr,
                               nullptr, "vs_main", "vs_5_1", 0, 0, &gvs, &gerr),
                    "D3DCompile glyph vs");
    throw_if_failed(D3DCompile(kGlyphShader, sizeof(kGlyphShader) - 1, "glyph", nullptr,
                               nullptr, "ps_main", "ps_5_1", 0, 0, &gps, &gerr),
                    "D3DCompile glyph ps");
    D3D12_GRAPHICS_PIPELINE_STATE_DESC gpso = pso;  // sprite layout/RT/DSV
    gpso.VS = {gvs->GetBufferPointer(), gvs->GetBufferSize()};
    gpso.PS = {gps->GetBufferPointer(), gps->GetBufferSize()};
    gpso.BlendState.RenderTarget[0].BlendEnable = FALSE;  // opaque cones
    gpso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;  // draw both facets
    gpso.DepthStencilState.DepthEnable = TRUE;
    gpso.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    gpso.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    throw_if_failed(
        impl_->device->CreateGraphicsPipelineState(&gpso, IID_PPV_ARGS(&impl_->pso_glyph)),
        "CreateGraphicsPipelineState glyph");

    D3D12_DESCRIPTOR_HEAP_DESC srv_heap_desc{};
    srv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srv_heap_desc.NumDescriptors = D3D12Presenter::kSrvHeapSize;
    srv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    throw_if_failed(impl_->device->CreateDescriptorHeap(
                        &srv_heap_desc, IID_PPV_ARGS(&impl_->srv_heap)),
                    "CreateDescriptorHeap SRV");
    impl_->srv_size = impl_->device->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    impl_->srv_free.clear();
    impl_->srv_free.reserve(D3D12Presenter::kSrvHeapSize - 1);
    // Slot 0 is the interop StructuredBuffer SRV and is never handed out.
    // Slot 1 is popped first (reserved overlay font SRV); 2..N follow.
    for (UINT i = D3D12Presenter::kSrvHeapSize; i-- > 1;) {
        impl_->srv_free.push_back(i);
    }

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

void D3D12Presenter::set_scene_rect(SceneRect rect) {
    scene_ = rect;
}

void D3D12Presenter::set_overlay_recorder(OverlayRecorder* recorder) {
    overlay_recorder_ = recorder;
}

PresenterUiContext D3D12Presenter::ui_backend_context() {
    PresenterUiContext ctx;
    if (!impl_ || !impl_->device || !impl_->queue || !impl_->srv_heap) {
        return ctx;
    }
    ctx.device = impl_->device.Get();
    ctx.queue = impl_->queue.Get();
    ctx.srv_heap = impl_->srv_heap.Get();
    ctx.rtv_format = DXGI_FORMAT_R8G8B8A8_UNORM;
    ctx.dsv_format = DXGI_FORMAT_UNKNOWN;
    ctx.num_frames_in_flight = static_cast<int>(kFrameCount);
    ctx.user = this;
    ctx.alloc_srv = [](PresenterUiContext* c, D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
                       D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
        if (c && c->user) {
            static_cast<D3D12Presenter*>(c->user)->alloc_srv_descriptor(cpu, gpu);
        }
    };
    ctx.free_srv = [](PresenterUiContext* c, D3D12_CPU_DESCRIPTOR_HANDLE cpu,
                      D3D12_GPU_DESCRIPTOR_HANDLE gpu) {
        if (c && c->user) {
            static_cast<D3D12Presenter*>(c->user)->free_srv_descriptor(cpu, gpu);
        }
    };
    return ctx;
}

void D3D12Presenter::alloc_srv_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpu,
                                          D3D12_GPU_DESCRIPTOR_HANDLE* gpu) {
    if (!cpu || !gpu) return;
    *cpu = {};
    *gpu = {};
    if (!impl_ || !impl_->srv_heap || impl_->srv_size == 0 || impl_->srv_free.empty()) {
        throw std::runtime_error("SRV descriptor heap exhausted");
    }
    const UINT index = impl_->srv_free.back();
    impl_->srv_free.pop_back();
    *cpu = impl_->srv_heap->GetCPUDescriptorHandleForHeapStart();
    cpu->ptr += static_cast<SIZE_T>(index) * impl_->srv_size;
    *gpu = impl_->srv_heap->GetGPUDescriptorHandleForHeapStart();
    gpu->ptr += static_cast<UINT64>(index) * impl_->srv_size;
}

void D3D12Presenter::free_srv_descriptor(D3D12_CPU_DESCRIPTOR_HANDLE cpu,
                                         D3D12_GPU_DESCRIPTOR_HANDLE) {
    if (!impl_ || !impl_->srv_heap || impl_->srv_size == 0 || cpu.ptr == 0) {
        return;
    }
    const SIZE_T start = impl_->srv_heap->GetCPUDescriptorHandleForHeapStart().ptr;
    if (cpu.ptr < start) return;
    const UINT index = static_cast<UINT>((cpu.ptr - start) / impl_->srv_size);
    if (index == kSrvSlotInterop || index >= kSrvHeapSize) {
        return;
    }
    impl_->srv_free.push_back(index);
}

CaptureToken D3D12Presenter::request_capture(CaptureRegion region) {
    CaptureToken token;
    token.id = ++next_capture_.id;
    if (!impl_ || !impl_->device || width_ == 0 || height_ == 0) {
        last_capture_ = {};
        last_capture_.status = CaptureStatus::Failed;
        last_capture_.error = "presenter not initialized";
        ready_capture_ = token;
        pending_capture_ = {};
        return token;
    }
    if (pending_capture_.id != 0) {
        return token;
    }
    pending_capture_ = token;
    pending_region_ = region;
    last_capture_ = {};
    last_capture_.status = CaptureStatus::Pending;
    impl_->capture_fence_value = 0;
    impl_->capture_width = 0;
    impl_->capture_height = 0;
    return token;
}

CaptureResult D3D12Presenter::poll_capture(CaptureToken token) {
    CaptureResult failed;
    failed.status = CaptureStatus::Failed;
    failed.error = "unknown capture token";
    if (token.id == 0) {
        CaptureResult idle;
        idle.status = CaptureStatus::Idle;
        return idle;
    }
    if (token.id == pending_capture_.id) {
        if (!impl_ || !impl_->fence || impl_->capture_fence_value == 0
            || impl_->fence->GetCompletedValue() < impl_->capture_fence_value) {
            CaptureResult pending;
            pending.status = CaptureStatus::Pending;
            return pending;
        }
        if (!impl_->capture_readback || impl_->capture_width == 0
            || impl_->capture_height == 0) {
            last_capture_ = {};
            last_capture_.status = CaptureStatus::Failed;
            last_capture_.error = "capture readback missing";
            ready_capture_ = token;
            pending_capture_ = {};
            return last_capture_;
        }
        const UINT64 bytes = static_cast<UINT64>(impl_->capture_row_pitch)
                             * impl_->capture_height;
        D3D12_RANGE range{0, static_cast<SIZE_T>(bytes)};
        void* mapped = nullptr;
        throw_if_failed(impl_->capture_readback->Map(0, &range, &mapped),
                        "Map capture readback");
        last_capture_ = {};
        last_capture_.status = CaptureStatus::Ready;
        last_capture_.width = impl_->capture_width;
        last_capture_.height = impl_->capture_height;
        last_capture_.row_pitch = impl_->capture_row_pitch;
        last_capture_.bytes.assign(static_cast<const std::uint8_t*>(mapped),
                                   static_cast<const std::uint8_t*>(mapped) + bytes);
        D3D12_RANGE written{0, 0};
        impl_->capture_readback->Unmap(0, &written);
        ready_capture_ = token;
        pending_capture_ = {};
        return last_capture_;
    }
    if (token.id == ready_capture_.id) {
        return last_capture_;
    }
    return failed;
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

std::vector<std::string> D3D12Presenter::debug_messages() const {
    std::vector<std::string> out;
    if (!impl_->info_queue) return out;
    const UINT64 count =
        impl_->info_queue->GetNumStoredMessagesAllowedByRetrievalFilter();
    for (UINT64 i = 0; i < count; ++i) {
        SIZE_T bytes = 0;
        if (FAILED(impl_->info_queue->GetMessage(i, nullptr, &bytes))) continue;
        std::vector<std::uint8_t> storage(bytes);
        auto* message = reinterpret_cast<D3D12_MESSAGE*>(storage.data());
        if (FAILED(impl_->info_queue->GetMessage(i, message, &bytes))) continue;
        if (message->Severity == D3D12_MESSAGE_SEVERITY_CORRUPTION
            || message->Severity == D3D12_MESSAGE_SEVERITY_ERROR
            || message->Severity == D3D12_MESSAGE_SEVERITY_WARNING) {
            out.emplace_back(message->pDescription ? message->pDescription : "");
        }
    }
    return out;
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

    const SceneRect scene = scene_rect_clamped_to(scene_, width_, height_);
    const float aspect = scene.height > 0
        ? static_cast<float>(scene.width) / static_cast<float>(scene.height)
        : 1.0f;
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
    // Line vertices for the LINE PSO: the boundary wireframe first (its verts at
    // a fixed offset 0, gated by opts.lattice_box), then the field-overlay vector
    // segments (2 verts each). The boundary is host-generated in
    // frame.boundary_lines (cube by default, or sphere/platonic/cylinder/torus/
    // none — build_boundary_lines), replacing the presenter's legacy in-line cube.
    // All share the LineVertex layout + LINELIST topology through impl_->pso_lines.
    std::vector<LineVertex> line_verts;
    line_verts.reserve((frame.boundary_lines.size() + frame.field_lines.size()
                        + frame.field_lines_top.size() + frame.background_lines.size()) * 2);
    for (const auto& bl : frame.boundary_lines) {
        line_verts.push_back(LineVertex{bl.x0, bl.y0, bl.z0, bl.r0, bl.g0, bl.b0});
        line_verts.push_back(LineVertex{bl.x1, bl.y1, bl.z1, bl.r1, bl.g1, bl.b1});
    }
    const UINT box_line_verts = static_cast<UINT>(line_verts.size());
    for (const auto& fl : frame.field_lines) {
        line_verts.push_back(LineVertex{fl.x0, fl.y0, fl.z0, fl.r0, fl.g0, fl.b0});
        line_verts.push_back(LineVertex{fl.x1, fl.y1, fl.z1, fl.r1, fl.g1, fl.b1});
    }
    const UINT field_line_verts = static_cast<UINT>(line_verts.size()) - box_line_verts;
    // On-top overlay lines (Force-Flow): appended after the normal field lines so
    // they can be drawn in a later pass (after the opaque particles).
    const UINT top_line_start = static_cast<UINT>(line_verts.size());
    for (const auto& fl : frame.field_lines_top) {
        line_verts.push_back(LineVertex{fl.x0, fl.y0, fl.z0, fl.r0, fl.g0, fl.b0});
        line_verts.push_back(LineVertex{fl.x1, fl.y1, fl.z1, fl.r1, fl.g1, fl.b1});
    }
    const UINT top_line_verts = static_cast<UINT>(line_verts.size()) - top_line_start;
    // Environment background lines (Beyond grid) — appended last, DRAWN FIRST
    // (behind the scene) through the LINE PSO. Range starts after the top lines.
    const UINT bg_line_start = static_cast<UINT>(line_verts.size());
    for (const auto& bl : frame.background_lines) {
        line_verts.push_back(LineVertex{bl.x0, bl.y0, bl.z0, bl.r0, bl.g0, bl.b0});
        line_verts.push_back(LineVertex{bl.x1, bl.y1, bl.z1, bl.r1, bl.g1, bl.b1});
    }
    const UINT bg_line_verts = static_cast<UINT>(line_verts.size()) - bg_line_start;

    // Tessellate one Force-Glyph cone (world-space, oriented to its direction)
    // into `dst` as GpuVertex triangles: a 6-segment base ring + apex (side
    // fan) + base fan. Flat-shaded by the glyph PSO (facet normal from screen
    // derivatives), so per-vertex colour is the glyph colour and uv is unused.
    auto append_glyphs = [&](const std::vector<NativeGlyph>& src,
                             std::vector<GpuVertex>& dst) {
        constexpr int kSeg = 6;
        constexpr float kTwoPi = 6.28318530718f;
        for (const NativeGlyph& gph : src) {
            float dx = gph.dx, dy = gph.dy, dz = gph.dz;
            const float dm = std::sqrt(dx * dx + dy * dy + dz * dz);
            if (dm < 1e-8f) continue;
            dx /= dm; dy /= dm; dz /= dm;
            // Orthonormal basis (dir, u, v).
            float ux, uy, uz;
            if (std::fabs(dx) < 0.9f) { ux = 1.0f; uy = 0.0f; uz = 0.0f; }
            else { ux = 0.0f; uy = 1.0f; uz = 0.0f; }
            const float dot = ux * dx + uy * dy + uz * dz;
            ux -= dot * dx; uy -= dot * dy; uz -= dot * dz;
            const float ul = std::sqrt(ux * ux + uy * uy + uz * uz);
            ux /= ul; uy /= ul; uz /= ul;
            const float vx = dy * uz - dz * uy;
            const float vy = dz * ux - dx * uz;
            const float vz = dx * uy - dy * ux;
            const float len = gph.scale;
            const float rad = 0.30f * len;
            // Root the cone at the charge, apex pointing along the force — so it
            // sticks out past the (opaque) particle it sits on rather than being
            // buried inside it.
            const float bx = gph.x;                     // base centre = charge site
            const float by = gph.y;
            const float bz = gph.z;
            const float ax = gph.x + dx * len;          // apex
            const float ay = gph.y + dy * len;
            const float az = gph.z + dz * len;
            auto vtx = [&](float px, float py, float pz) {
                GpuVertex v;
                v.x = px; v.y = py; v.z = pz;
                v.r = gph.r; v.g = gph.g; v.b = gph.b;
                v.u = 0.0f; v.v = 0.0f;
                dst.push_back(v);
            };
            float rx[kSeg], ry[kSeg], rz[kSeg];
            for (int k = 0; k < kSeg; ++k) {
                const float th = kTwoPi * static_cast<float>(k) / static_cast<float>(kSeg);
                const float c = std::cos(th), s = std::sin(th);
                rx[k] = bx + rad * (c * ux + s * vx);
                ry[k] = by + rad * (c * uy + s * vy);
                rz[k] = bz + rad * (c * uz + s * vz);
            }
            for (int k = 0; k < kSeg; ++k) {
                const int n = (k + 1) % kSeg;
                vtx(ax, ay, az); vtx(rx[k], ry[k], rz[k]); vtx(rx[n], ry[n], rz[n]);  // side
                vtx(bx, by, bz); vtx(rx[n], ry[n], rz[n]); vtx(rx[k], ry[k], rz[k]);  // base
            }
        }
    };

    std::vector<GpuVertex> verts;
    verts.reserve((frame.flux.size() + frame.particles.size() + frame.flux_heat.size()
                   + frame.background_points.size()) * 6
                  + frame.field_glyphs.size() * 36);
    if (opts.flux) append_sprites(frame.flux, verts);
    const UINT flux_verts = static_cast<UINT>(verts.size());
    // When interop_particle_count > 0, particles are drawn from the imported
    // D3D12 buffer via the interop PSO below instead of the CPU-expanded
    // vertex buffer -- do not double-draw. frame.particles stays populated
    // either way (the CPU backend has no interop path and always needs it).
    if (opts.particles && interop_particle_count == 0) append_sprites(frame.particles, verts);
    const UINT particle_end = static_cast<UINT>(verts.size());
    // Force-Heatmap glow sprites (additive gaussian PSO) and Force-Glyph cones
    // (opaque flat-shaded PSO) — separate ranges of the same sprite buffer, only
    // populated when a Force overlay is in that style. Drawn unconditionally when
    // present (a force overlay's visibility, like its arrows, is not gated on the
    // ambient-flux view toggle).
    append_sprites(frame.flux_heat, verts);
    const UINT heat_end = static_cast<UINT>(verts.size());
    append_glyphs(frame.field_glyphs, verts);
    const UINT glyph_end = static_cast<UINT>(verts.size());
    const UINT heat_verts = heat_end - particle_end;
    const UINT glyph_verts = glyph_end - heat_end;
    // Environment background points — appended last but DRAWN FIRST (behind the
    // scene) through the additive depth-off heat PSO. Their range starts at
    // glyph_end in the shared sprite buffer.
    append_sprites(frame.background_points, verts);
    const UINT bg_verts = static_cast<UINT>(verts.size()) - glyph_end;

    const std::size_t sprite_bytes = verts.size() * sizeof(GpuVertex);
    const std::size_t line_bytes = line_verts.size() * sizeof(LineVertex);
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
        if (line_bytes != 0) {
            std::memcpy(bytes_out, line_verts.data(), line_bytes);
        }
        if (sprite_bytes != 0) {
            std::memcpy(bytes_out + line_bytes, verts.data(), sprite_bytes);
        }
        impl_->vb[impl_->frame]->Unmap(0, nullptr);
    }

    // ── Rubber-sheet mesh upload (Tranche 5) ────────────────────────────────────
    // Concatenate every active sheet's vertices into one buffer and its indices
    // into one index buffer (each sheet's indices rebased by the running vertex
    // count), so all sheets draw in a single DrawIndexedInstanced per pass
    // (solid + wireframe) through impl_->pso_sheet / pso_sheet_wire.
    std::vector<NativeSheetVertex> sheet_verts;
    std::vector<std::uint32_t> sheet_indices;
    // Order-independent-ish translucency for the sheets: the volume/heatmap passes
    // are additive (already order-independent), but the sheets are alpha-blended
    // (SRC_ALPHA/INV_SRC_ALPHA, depth-test no-write), so overlapping sheets composite
    // correctly only when drawn back-to-front. They batch into one draw, so we
    // concatenate them farthest-first: sort by centroid distance to the camera eye,
    // then build the buffer in that order. ≤6 sheets (non-intersecting height-field
    // planes), so the per-sheet centroid pass is negligible and a plane-sort is the
    // right-sized fix (no weighted-blended-OIT machinery needed).
    {
        const std::size_t ns = frame.field_sheets.size();
        std::vector<std::size_t> order;
        std::vector<float> dist2(ns, 0.0f);
        order.reserve(ns);
        for (std::size_t i = 0; i < ns; ++i) {
            const NativeSheet& s = frame.field_sheets[i];
            if (s.vertices.empty() || s.indices.empty()) continue;
            double cx = 0.0, cyc = 0.0, cz = 0.0;
            for (const NativeSheetVertex& v : s.vertices) { cx += v.x; cyc += v.y; cz += v.z; }
            const double inv = 1.0 / static_cast<double>(s.vertices.size());
            const float dx = static_cast<float>(cx * inv) - eye_x;
            const float dy = static_cast<float>(cyc * inv) - eye_y;
            const float dz = static_cast<float>(cz * inv) - eye_z;
            dist2[i] = dx * dx + dy * dy + dz * dz;
            order.push_back(i);
        }
        std::sort(order.begin(), order.end(),
                  [&](std::size_t a, std::size_t b) { return dist2[a] > dist2[b]; });
        for (std::size_t i : order) {
            const NativeSheet& s = frame.field_sheets[i];
            const std::uint32_t base = static_cast<std::uint32_t>(sheet_verts.size());
            sheet_verts.insert(sheet_verts.end(), s.vertices.begin(), s.vertices.end());
            for (std::uint32_t idx : s.indices) sheet_indices.push_back(base + idx);
        }
    }
    const UINT sheet_index_count = static_cast<UINT>(sheet_indices.size());
    const std::size_t sheet_vb_bytes = sheet_verts.size() * sizeof(NativeSheetVertex);
    const std::size_t sheet_ib_bytes = sheet_indices.size() * sizeof(std::uint32_t);
    if (sheet_index_count != 0) {
        auto ensure_upload = [&](ComPtr<ID3D12Resource>& res, std::size_t& cap,
                                 std::size_t need, const void* src, const char* what) {
            if (need > cap) {
                res.Reset();
                D3D12_HEAP_PROPERTIES up{};
                up.Type = D3D12_HEAP_TYPE_UPLOAD;
                D3D12_RESOURCE_DESC desc{};
                desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                desc.Width = need < 256 ? 256 : need;
                desc.Height = 1;
                desc.DepthOrArraySize = 1;
                desc.MipLevels = 1;
                desc.SampleDesc.Count = 1;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                throw_if_failed(impl_->device->CreateCommittedResource(
                                    &up, D3D12_HEAP_FLAG_NONE, &desc,
                                    D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                    IID_PPV_ARGS(&res)),
                                what);
                cap = static_cast<std::size_t>(desc.Width);
            }
            void* mapped = nullptr;
            throw_if_failed(res->Map(0, nullptr, &mapped), "Map sheet buffer");
            std::memcpy(mapped, src, need);
            res->Unmap(0, nullptr);
        };
        ensure_upload(impl_->sheet_vb[impl_->frame], impl_->sheet_vb_capacity[impl_->frame],
                      sheet_vb_bytes, sheet_verts.data(), "CreateCommittedResource sheet vb");
        ensure_upload(impl_->sheet_ib[impl_->frame], impl_->sheet_ib_capacity[impl_->frame],
                      sheet_ib_bytes, sheet_indices.data(), "CreateCommittedResource sheet ib");
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

    D3D12_VIEWPORT vp_desc{static_cast<float>(scene.x), static_cast<float>(scene.y),
                           static_cast<float>(scene.width),
                           static_cast<float>(scene.height), 0.0f, 1.0f};
    D3D12_RECT scissor{scene.x, scene.y,
                       scene.x + static_cast<LONG>(scene.width),
                       scene.y + static_cast<LONG>(scene.height)};
    impl_->list->RSSetViewports(1, &vp_desc);
    impl_->list->RSSetScissorRects(1, &scissor);
    impl_->list->SetGraphicsRootSignature(impl_->root.Get());
    impl_->list->SetGraphicsRootConstantBufferView(
        0, impl_->cb[impl_->frame]->GetGPUVirtualAddress());

    // Environment background grid (Beyond theme) — drawn FIRST through the LINE
    // PSO so the fading lattice sits behind the scene (the nearer scene occludes
    // it via depth). Range starts at bg_line_start in the shared line buffer.
    if (bg_line_verts != 0 && line_bytes != 0 && impl_->vb[impl_->frame]) {
        D3D12_VERTEX_BUFFER_VIEW blv{};
        blv.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress();
        blv.SizeInBytes = static_cast<UINT>(line_bytes);
        blv.StrideInBytes = sizeof(LineVertex);
        impl_->list->SetPipelineState(impl_->pso_lines.Get());
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        impl_->list->IASetVertexBuffers(0, 1, &blv);
        impl_->list->DrawInstanced(bg_line_verts, 1, bg_line_start, 0);
    }

    // Environment background — drawn FIRST (behind everything) through the additive,
    // depth-write-off heat PSO so a large procedural point cloud sits behind the
    // lattice without occluding it. Reuses the sprite buffer (range starts at
    // glyph_end == particle/heat/glyph end).
    if (bg_verts != 0 && sprite_bytes != 0 && impl_->pso_heat) {
        D3D12_VERTEX_BUFFER_VIEW bgv{};
        bgv.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress() + line_bytes;
        bgv.SizeInBytes = static_cast<UINT>(sprite_bytes);
        bgv.StrideInBytes = sizeof(GpuVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &bgv);
        impl_->list->SetPipelineState(impl_->pso_heat.Get());
        impl_->list->DrawInstanced(bg_verts, 1, glyph_end, 0);
    }

    // Boundary wireframe (conditional) + field-overlay vectors (whenever present)
    // — both through the LINE PSO. The boundary verts occupy buffer offset 0; the
    // field verts follow at box_line_verts. SetPipelineState explicitly since the
    // background pass above may have changed it off pso_lines.
    if (impl_->vb[impl_->frame] && line_bytes != 0
        && ((opts.lattice_box && box_line_verts != 0) || field_line_verts != 0)) {
        D3D12_VERTEX_BUFFER_VIEW line_view{};
        line_view.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress();
        line_view.SizeInBytes = static_cast<UINT>(line_bytes);
        line_view.StrideInBytes = sizeof(LineVertex);
        impl_->list->SetPipelineState(impl_->pso_lines.Get());
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        impl_->list->IASetVertexBuffers(0, 1, &line_view);
        if (opts.lattice_box && box_line_verts != 0) {
            impl_->list->DrawInstanced(box_line_verts, 1, 0, 0);
        }
        if (field_line_verts != 0) {
            impl_->list->DrawInstanced(field_line_verts, 1, box_line_verts, 0);
        }
    }

    if (sprite_bytes != 0) {
        D3D12_VERTEX_BUFFER_VIEW sprite_view{};
        sprite_view.BufferLocation =
            impl_->vb[impl_->frame]->GetGPUVirtualAddress() + line_bytes;
        sprite_view.SizeInBytes = static_cast<UINT>(sprite_bytes);
        sprite_view.StrideInBytes = sizeof(GpuVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &sprite_view);
        // Alpha-blended sprites (flux cloud + particle billboards) through pso.
        impl_->list->SetPipelineState(impl_->pso.Get());
        if (flux_verts != 0) {
            impl_->list->DrawInstanced(flux_verts, 1, 0, 0);
        }
        const UINT particle_verts = particle_end - flux_verts;
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

    // Force-Glyph cones drawn AFTER the particles (the force field lives at the
    // charge sites, so the cones — rooted there, pointing outward — must sit on
    // top of the opaque particles to be seen). Opaque, depth test + write. Re-bind
    // the sprite buffer (the interop pass rebinds topology / drops the VB).
    if (glyph_verts != 0 && sprite_bytes != 0 && impl_->pso_glyph) {
        D3D12_VERTEX_BUFFER_VIEW gview{};
        gview.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress() + line_bytes;
        gview.SizeInBytes = static_cast<UINT>(sprite_bytes);
        gview.StrideInBytes = sizeof(GpuVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &gview);
        impl_->list->SetPipelineState(impl_->pso_glyph.Get());
        impl_->list->DrawInstanced(glyph_verts, 1, heat_end, 0);
    }

    // On-top overlay lines (Force-Flow dashed streamlines) drawn after the
    // particles for the same reason. Re-bind the LINE buffer + LINELIST topology.
    if (top_line_verts != 0 && line_bytes != 0 && impl_->vb[impl_->frame]) {
        D3D12_VERTEX_BUFFER_VIEW tview{};
        tview.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress();
        tview.SizeInBytes = static_cast<UINT>(line_bytes);
        tview.StrideInBytes = sizeof(LineVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        impl_->list->IASetVertexBuffers(0, 1, &tview);
        impl_->list->SetPipelineState(impl_->pso_lines.Get());
        impl_->list->DrawInstanced(top_line_verts, 1, top_line_start, 0);
    }

    // Force-Heatmap glow drawn LAST (after the opaque particles, interop or CPU):
    // the force field is populated only at charge sites, so the additive,
    // depth-test-no-write glow must sit ON TOP of the particles to read as a
    // heatmap rather than being occluded by them. Re-bind the sprite buffer since
    // the interop pass rebinds topology / drops the vertex buffer.
    if (heat_verts != 0 && sprite_bytes != 0 && impl_->pso_heat) {
        D3D12_VERTEX_BUFFER_VIEW hview{};
        hview.BufferLocation = impl_->vb[impl_->frame]->GetGPUVirtualAddress() + line_bytes;
        hview.SizeInBytes = static_cast<UINT>(sprite_bytes);
        hview.StrideInBytes = sizeof(GpuVertex);
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &hview);
        impl_->list->SetPipelineState(impl_->pso_heat.Get());
        impl_->list->DrawInstanced(heat_verts, 1, particle_end, 0);
    }

    // Rubber-sheet surfaces (Tranche 5): drawn last among the 3D geometry so the
    // translucent, depth-tested-but-not-depth-writing sheets blend over the
    // particles/lines behind them. Solid pass first, then the wireframe overlay
    // over the same indexed mesh. Root sig + camera CBV are already bound above.
    if (sheet_index_count != 0 && impl_->sheet_vb[impl_->frame] && impl_->sheet_ib[impl_->frame]
        && impl_->pso_sheet && impl_->pso_sheet_wire) {
        D3D12_VERTEX_BUFFER_VIEW sheet_view{};
        sheet_view.BufferLocation = impl_->sheet_vb[impl_->frame]->GetGPUVirtualAddress();
        sheet_view.SizeInBytes = static_cast<UINT>(sheet_vb_bytes);
        sheet_view.StrideInBytes = sizeof(NativeSheetVertex);
        D3D12_INDEX_BUFFER_VIEW sheet_ibv{};
        sheet_ibv.BufferLocation = impl_->sheet_ib[impl_->frame]->GetGPUVirtualAddress();
        sheet_ibv.SizeInBytes = static_cast<UINT>(sheet_ib_bytes);
        sheet_ibv.Format = DXGI_FORMAT_R32_UINT;
        impl_->list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        impl_->list->IASetVertexBuffers(0, 1, &sheet_view);
        impl_->list->IASetIndexBuffer(&sheet_ibv);
        impl_->list->SetPipelineState(impl_->pso_sheet.Get());
        impl_->list->DrawIndexedInstanced(sheet_index_count, 1, 0, 0, 0);
        impl_->list->SetPipelineState(impl_->pso_sheet_wire.Get());
        impl_->list->DrawIndexedInstanced(sheet_index_count, 1, 0, 0, 0);
    }

    if (overlay_recorder_) {
        if (impl_->srv_heap) {
            ID3D12DescriptorHeap* heaps[] = {impl_->srv_heap.Get()};
            impl_->list->SetDescriptorHeaps(1, heaps);
        }
        impl_->list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
        RenderTargetInfo rt{};
        rt.rtv = rtv;
        rt.width = width_;
        rt.height = height_;
        overlay_recorder_->record(impl_->list.Get(), rt);
    }

    if (pending_capture_.id != 0) {
        UINT copy_x = 0;
        UINT copy_y = 0;
        UINT copy_w = width_;
        UINT copy_h = height_;
        if (pending_region_ == CaptureRegion::Scene) {
            copy_x = static_cast<UINT>(std::max(0, scene.x));
            copy_y = static_cast<UINT>(std::max(0, scene.y));
            copy_w = scene.width;
            copy_h = scene.height;
        }
        if (copy_w == 0 || copy_h == 0) {
            last_capture_ = {};
            last_capture_.status = CaptureStatus::Failed;
            last_capture_.error = "empty capture region";
            ready_capture_ = pending_capture_;
            pending_capture_ = {};
        } else {
            const UINT row_pitch =
                (copy_w * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1)
                & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
            const UINT64 readback_bytes =
                static_cast<UINT64>(row_pitch) * copy_h;
            D3D12_RESOURCE_DESC existing{};
            if (impl_->capture_readback) {
                existing = impl_->capture_readback->GetDesc();
            }
            if (!impl_->capture_readback || existing.Width < readback_bytes) {
                impl_->capture_readback.Reset();
                D3D12_HEAP_PROPERTIES readback{};
                readback.Type = D3D12_HEAP_TYPE_READBACK;
                D3D12_RESOURCE_DESC desc{};
                desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
                desc.Width = readback_bytes;
                desc.Height = 1;
                desc.DepthOrArraySize = 1;
                desc.MipLevels = 1;
                desc.SampleDesc.Count = 1;
                desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
                throw_if_failed(impl_->device->CreateCommittedResource(
                                    &readback, D3D12_HEAP_FLAG_NONE, &desc,
                                    D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                    IID_PPV_ARGS(&impl_->capture_readback)),
                                "CreateCommittedResource capture readback");
            }
            impl_->capture_row_pitch = row_pitch;
            impl_->capture_width = copy_w;
            impl_->capture_height = copy_h;

            D3D12_RESOURCE_BARRIER copy_bar{};
            copy_bar.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            copy_bar.Transition.pResource = impl_->targets[impl_->frame].Get();
            copy_bar.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
            copy_bar.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
            copy_bar.Transition.Subresource =
                D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            impl_->list->ResourceBarrier(1, &copy_bar);

            D3D12_TEXTURE_COPY_LOCATION src{};
            src.pResource = impl_->targets[impl_->frame].Get();
            src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            src.SubresourceIndex = 0;
            D3D12_TEXTURE_COPY_LOCATION dst{};
            dst.pResource = impl_->capture_readback.Get();
            dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            dst.PlacedFootprint.Offset = 0;
            dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            dst.PlacedFootprint.Footprint.Width = copy_w;
            dst.PlacedFootprint.Footprint.Height = copy_h;
            dst.PlacedFootprint.Footprint.Depth = 1;
            dst.PlacedFootprint.Footprint.RowPitch = row_pitch;
            D3D12_BOX box{};
            box.left = copy_x;
            box.top = copy_y;
            box.right = copy_x + copy_w;
            box.bottom = copy_y + copy_h;
            box.front = 0;
            box.back = 1;
            impl_->list->CopyTextureRegion(&dst, 0, 0, 0, &src, &box);

            copy_bar.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
            copy_bar.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
            impl_->list->ResourceBarrier(1, &copy_bar);
            impl_->capture_fence_value = impl_->fence_value + 1;
        }
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

}  // namespace ftd::native
