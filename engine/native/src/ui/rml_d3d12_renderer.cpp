#include "ui/rml_d3d12_renderer.h"

#include <d3dcompiler.h>
#include <wincodec.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace ftd::native::ui {
namespace {

void throw_if_failed(HRESULT hr, const char* what) {
    if (FAILED(hr)) {
        throw std::runtime_error(std::string("RmlD3D12Renderer: ") + what +
                                 " HRESULT=0x" + [hr] {
                                     char buf[16];
                                     std::snprintf(buf, sizeof(buf), "%08lX",
                                                   static_cast<unsigned long>(hr));
                                     return std::string(buf);
                                 }());
    }
}

// The Rml::Vertex memory layout the UI input layout below assumes.
static_assert(sizeof(Rml::Vertex) == 20, "Rml::Vertex must be 20 bytes (pos8 + colour4 + uv8)");
static_assert(offsetof(Rml::Vertex, position) == 0, "position at offset 0");
static_assert(offsetof(Rml::Vertex, colour) == 8, "colour at offset 8");
static_assert(offsetof(Rml::Vertex, tex_coord) == 12, "tex_coord at offset 12");

std::string read_text_file(const wchar_t* path) {
    FILE* f = nullptr;
    if (_wfopen_s(&f, path, L"rb") != 0 || !f) {
        throw std::runtime_error("RmlD3D12Renderer: cannot open HLSL source");
    }
    std::fseek(f, 0, SEEK_END);
    long n = std::ftell(f);
    std::fseek(f, 0, SEEK_SET);
    std::string s(static_cast<size_t>(n < 0 ? 0 : n), '\0');
    if (n > 0) {
        size_t got = std::fread(s.data(), 1, static_cast<size_t>(n), f);
        s.resize(got);
    }
    std::fclose(f);
    return s;
}

// Column-major ortho for the shader's `mul(uProj, float4(p,0,1))` (HLSL's
// default column-major matrix packing). Maps screen space (x in [0,w], y in
// [0,h], origin top-left, y down) to clip space (x' in [-1,1], y' in [1,-1]):
//   x' = 2x/w - 1,   y' = 1 - 2y/h,   z' = 0,   w' = 1
// so screen (0,0) -> NDC (-1,+1) = RT top-left, screen (w,h) -> (+1,-1).
void build_ortho(float* m, float w, float h) {
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0] = 2.0f / w;    // col0.x
    m[5] = -2.0f / h;   // col1.y
    m[12] = -1.0f;      // col3.x  (translation)
    m[13] = 1.0f;       // col3.y
    m[15] = 1.0f;       // col3.w
}

}  // namespace

// ── SystemInterface ─────────────────────────────────────────────────────────
RmlD3D12System::RmlD3D12System() : start_(std::chrono::steady_clock::now()) {}

double RmlD3D12System::GetElapsedTime() {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - start_).count();
}

bool RmlD3D12System::LogMessage(Rml::Log::Type type, const Rml::String& message) {
    const char* tag = "RmlUi";
    switch (type) {
        case Rml::Log::LT_ERROR:
        case Rml::Log::LT_ASSERT: tag = "RmlUi ERROR"; break;
        case Rml::Log::LT_WARNING: tag = "RmlUi WARN"; break;
        case Rml::Log::LT_INFO: tag = "RmlUi info"; break;
        case Rml::Log::LT_DEBUG: tag = "RmlUi debug"; break;
        default: tag = "RmlUi"; break;
    }
    std::fprintf(stderr, "[%s] %s\n", tag, message.c_str());
    return true;  // continue execution (never break into the debugger)
}

// ── Renderer ────────────────────────────────────────────────────────────────
RmlD3D12Renderer::RmlD3D12Renderer() = default;

RmlD3D12Renderer::~RmlD3D12Renderer() {
    if (upload_event_) CloseHandle(upload_event_);
}

void RmlD3D12Renderer::initialize(ID3D12Device* device, ID3D12CommandQueue* queue,
                                  const wchar_t* hlsl_path) {
    device_ = device;
    queue_ = queue;

    // Root signature: b0 = 20 32-bit constants (uProj 4x4 + uTranslate float2 +
    // pad float2), t0 = one SRV in a descriptor table, s0 = static linear-clamp
    // sampler.
    D3D12_DESCRIPTOR_RANGE srv_range{};
    srv_range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    srv_range.NumDescriptors = 1;
    srv_range.BaseShaderRegister = 0;  // t0
    srv_range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER params[2]{};
    params[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    params[0].Constants.ShaderRegister = 0;  // b0
    params[0].Constants.RegisterSpace = 0;
    params[0].Constants.Num32BitValues = 20;
    params[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    params[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    params[1].DescriptorTable.NumDescriptorRanges = 1;
    params[1].DescriptorTable.pDescriptorRanges = &srv_range;
    params[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_STATIC_SAMPLER_DESC sampler{};
    sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
    sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    sampler.MaxLOD = D3D12_FLOAT32_MAX;
    sampler.ShaderRegister = 0;  // s0
    sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    D3D12_ROOT_SIGNATURE_DESC rs{};
    rs.NumParameters = 2;
    rs.pParameters = params;
    rs.NumStaticSamplers = 1;
    rs.pStaticSamplers = &sampler;
    rs.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ComPtr<ID3DBlob> rs_blob, rs_err;
    HRESULT hr = D3D12SerializeRootSignature(&rs, D3D_ROOT_SIGNATURE_VERSION_1,
                                             &rs_blob, &rs_err);
    if (FAILED(hr)) {
        std::string msg = "D3D12SerializeRootSignature";
        if (rs_err) msg += std::string(": ") + static_cast<const char*>(rs_err->GetBufferPointer());
        throw_if_failed(hr, msg.c_str());
    }
    throw_if_failed(device_->CreateRootSignature(0, rs_blob->GetBufferPointer(),
                                                 rs_blob->GetBufferSize(),
                                                 IID_PPV_ARGS(&root_)),
                    "CreateRootSignature");

    // Compile the UI shader (rmlui.hlsl) at runtime.
    const std::string hlsl = read_text_file(hlsl_path);
    UINT flags = 0;
#ifndef NDEBUG
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ComPtr<ID3DBlob> vs, ps, err;
    hr = D3DCompile(hlsl.data(), hlsl.size(), "rmlui.hlsl", nullptr, nullptr,
                    "VSMain", "vs_5_0", flags, 0, &vs, &err);
    if (FAILED(hr)) {
        std::string msg = "D3DCompile VSMain";
        if (err) msg += std::string(": ") + static_cast<const char*>(err->GetBufferPointer());
        throw_if_failed(hr, msg.c_str());
    }
    hr = D3DCompile(hlsl.data(), hlsl.size(), "rmlui.hlsl", nullptr, nullptr,
                    "PSMain", "ps_5_0", flags, 0, &ps, &err);
    if (FAILED(hr)) {
        std::string msg = "D3DCompile PSMain";
        if (err) msg += std::string(": ") + static_cast<const char*>(err->GetBufferPointer());
        throw_if_failed(hr, msg.c_str());
    }

    D3D12_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso{};
    pso.pRootSignature = root_.Get();
    pso.VS = {vs->GetBufferPointer(), vs->GetBufferSize()};
    pso.PS = {ps->GetBufferPointer(), ps->GetBufferSize()};
    // Premultiplied-alpha blend: RmlUi 6.x emits premultiplied vertex colours
    // AND premultiplied texture data (Vertex.colour is ColourbPremultiplied,
    // GenerateTexture pixels are premultiplied), so src factor is ONE, not
    // SRC_ALPHA. (The rmlui.hlsl header comment names SRC_ALPHA; that is the
    // straight-alpha convention and would double-darken text/edges here.)
    pso.BlendState.RenderTarget[0].BlendEnable = TRUE;
    pso.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    pso.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    pso.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    pso.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    pso.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    pso.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    pso.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    pso.SampleMask = UINT_MAX;
    pso.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    pso.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
    pso.RasterizerState.DepthClipEnable = TRUE;
    // No depth: UI draws over the 3D scene. Scissor is always applied via
    // RSSetScissorRects (D3D12 has no rasterizer scissor-enable toggle).
    pso.DepthStencilState.DepthEnable = FALSE;
    pso.DepthStencilState.StencilEnable = FALSE;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_UNKNOWN;
    pso.SampleDesc.Count = 1;
    pso.InputLayout = {layout, 3};
    throw_if_failed(device_->CreateGraphicsPipelineState(&pso, IID_PPV_ARGS(&pso_)),
                    "CreateGraphicsPipelineState");

    // Shader-visible SRV heap: slot 0 = white default, 1.. = RmlUi textures.
    D3D12_DESCRIPTOR_HEAP_DESC heap{};
    heap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    heap.NumDescriptors = kSrvHeapSize;
    heap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    throw_if_failed(device_->CreateDescriptorHeap(&heap, IID_PPV_ARGS(&srv_heap_)),
                    "CreateDescriptorHeap");
    srv_inc_ = device_->GetDescriptorHandleIncrementSize(
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
    srv_free_.clear();
    srv_free_.reserve(kSrvHeapSize - 1);
    for (UINT i = kSrvHeapSize; i-- > 1;) srv_free_.push_back(i);

    // Private synchronous-upload command infrastructure.
    throw_if_failed(device_->CreateCommandAllocator(
                        D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&upload_alloc_)),
                    "CreateCommandAllocator upload");
    throw_if_failed(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               upload_alloc_.Get(), nullptr,
                                               IID_PPV_ARGS(&upload_list_)),
                    "CreateCommandList upload");
    throw_if_failed(upload_list_->Close(), "Close upload list");
    throw_if_failed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE,
                                         IID_PPV_ARGS(&upload_fence_)),
                    "CreateFence upload");
    upload_event_ = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!upload_event_) throw std::runtime_error("RmlD3D12Renderer: CreateEventW failed");

    // 1x1 opaque white default for untextured geometry (texture handle 0).
    const std::uint8_t white_px[4] = {255, 255, 255, 255};
    white_.resource = upload_texture_sync(1, 1, white_px, 4);
    white_.slot = kSlotWhite;
    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Texture2D.MipLevels = 1;
    device_->CreateShaderResourceView(white_.resource.Get(), &srv, srv_cpu(kSlotWhite));
    white_.gpu = srv_gpu(kSlotWhite);
}

RmlD3D12Renderer::ComPtrRes RmlD3D12Renderer::create_upload_buffer(UINT64 bytes,
                                                                   const void* data) {
    if (bytes == 0) return nullptr;
    D3D12_HEAP_PROPERTIES upload{};
    upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC desc{};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    desc.Width = bytes;
    desc.Height = 1;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.SampleDesc.Count = 1;
    desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtrRes res;
    throw_if_failed(device_->CreateCommittedResource(
                        &upload, D3D12_HEAP_FLAG_NONE, &desc,
                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&res)),
                    "CreateCommittedResource upload buffer");
    void* mapped = nullptr;
    D3D12_RANGE none{0, 0};
    throw_if_failed(res->Map(0, &none, &mapped), "Map upload buffer");
    std::memcpy(mapped, data, static_cast<size_t>(bytes));
    res->Unmap(0, nullptr);
    return res;
}

RmlD3D12Renderer::ComPtrRes RmlD3D12Renderer::upload_texture_sync(UINT width, UINT height,
                                                                 const void* rgba,
                                                                 UINT src_row_pitch) {
    D3D12_RESOURCE_DESC tex{};
    tex.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    tex.Width = width;
    tex.Height = height;
    tex.DepthOrArraySize = 1;
    tex.MipLevels = 1;
    tex.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    tex.SampleDesc.Count = 1;
    tex.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

    D3D12_HEAP_PROPERTIES def{};
    def.Type = D3D12_HEAP_TYPE_DEFAULT;
    ComPtrRes resource;
    throw_if_failed(device_->CreateCommittedResource(
                        &def, D3D12_HEAP_FLAG_NONE, &tex,
                        D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&resource)),
                    "CreateCommittedResource texture");

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint{};
    UINT num_rows = 0;
    UINT64 row_bytes = 0, total_bytes = 0;
    device_->GetCopyableFootprints(&tex, 0, 1, 0, &footprint, &num_rows, &row_bytes,
                                   &total_bytes);

    ComPtrRes staging;
    D3D12_HEAP_PROPERTIES upload{};
    upload.Type = D3D12_HEAP_TYPE_UPLOAD;
    D3D12_RESOURCE_DESC sdesc{};
    sdesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    sdesc.Width = total_bytes;
    sdesc.Height = 1;
    sdesc.DepthOrArraySize = 1;
    sdesc.MipLevels = 1;
    sdesc.SampleDesc.Count = 1;
    sdesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    throw_if_failed(device_->CreateCommittedResource(
                        &upload, D3D12_HEAP_FLAG_NONE, &sdesc,
                        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&staging)),
                    "CreateCommittedResource texture staging");

    std::uint8_t* dst = nullptr;
    D3D12_RANGE none{0, 0};
    throw_if_failed(staging->Map(0, &none, reinterpret_cast<void**>(&dst)),
                    "Map texture staging");
    const auto* src = static_cast<const std::uint8_t*>(rgba);
    const UINT copy_bytes = std::min<UINT>(src_row_pitch, static_cast<UINT>(row_bytes));
    for (UINT y = 0; y < height; ++y) {
        std::memcpy(dst + footprint.Offset + static_cast<UINT64>(y) * footprint.Footprint.RowPitch,
                    src + static_cast<UINT64>(y) * src_row_pitch, copy_bytes);
    }
    staging->Unmap(0, nullptr);

    throw_if_failed(upload_alloc_->Reset(), "Reset upload allocator");
    throw_if_failed(upload_list_->Reset(upload_alloc_.Get(), nullptr), "Reset upload list");

    D3D12_TEXTURE_COPY_LOCATION dst_loc{};
    dst_loc.pResource = resource.Get();
    dst_loc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    dst_loc.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION src_loc{};
    src_loc.pResource = staging.Get();
    src_loc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    src_loc.PlacedFootprint = footprint;
    upload_list_->CopyTextureRegion(&dst_loc, 0, 0, 0, &src_loc, nullptr);

    D3D12_RESOURCE_BARRIER barrier{};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = resource.Get();
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    upload_list_->ResourceBarrier(1, &barrier);

    throw_if_failed(upload_list_->Close(), "Close upload list");
    ID3D12CommandList* lists[] = {upload_list_.Get()};
    queue_->ExecuteCommandLists(1, lists);
    ++upload_fence_value_;
    throw_if_failed(queue_->Signal(upload_fence_.Get(), upload_fence_value_),
                    "Signal upload fence");
    if (upload_fence_->GetCompletedValue() < upload_fence_value_) {
        throw_if_failed(upload_fence_->SetEventOnCompletion(upload_fence_value_, upload_event_),
                        "SetEventOnCompletion upload");
        WaitForSingleObject(upload_event_, INFINITE);
    }
    return resource;  // staging released here (safe: GPU copy has completed).
}

Rml::TextureHandle RmlD3D12Renderer::register_texture(ComPtrRes resource, UINT /*w*/,
                                                      UINT /*h*/) {
    const UINT slot = alloc_srv_slot();
    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srv.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Texture2D.MipLevels = 1;
    device_->CreateShaderResourceView(resource.Get(), &srv, srv_cpu(slot));

    Texture t;
    t.resource = std::move(resource);
    t.slot = slot;
    t.gpu = srv_gpu(slot);
    const Rml::TextureHandle handle = next_texture_++;
    textures_.emplace(handle, std::move(t));
    return handle;
}

UINT RmlD3D12Renderer::alloc_srv_slot() {
    if (srv_free_.empty()) throw std::runtime_error("RmlD3D12Renderer: SRV heap exhausted");
    const UINT slot = srv_free_.back();
    srv_free_.pop_back();
    return slot;
}

void RmlD3D12Renderer::free_srv_slot(UINT slot) {
    if (slot != kSlotWhite && slot < kSrvHeapSize) srv_free_.push_back(slot);
}

D3D12_CPU_DESCRIPTOR_HANDLE RmlD3D12Renderer::srv_cpu(UINT slot) const {
    D3D12_CPU_DESCRIPTOR_HANDLE h = srv_heap_->GetCPUDescriptorHandleForHeapStart();
    h.ptr += static_cast<SIZE_T>(slot) * srv_inc_;
    return h;
}

D3D12_GPU_DESCRIPTOR_HANDLE RmlD3D12Renderer::srv_gpu(UINT slot) const {
    D3D12_GPU_DESCRIPTOR_HANDLE h = srv_heap_->GetGPUDescriptorHandleForHeapStart();
    h.ptr += static_cast<UINT64>(slot) * srv_inc_;
    return h;
}

void RmlD3D12Renderer::collect_garbage() {
    auto keep = retired_.end();
    keep = std::remove_if(retired_.begin(), retired_.end(), [&](Retired& r) {
        if (r.frame + kFramesInFlight <= frame_index_) {
            if (r.has_slot) free_srv_slot(r.slot);
            r.a.Reset();
            r.b.Reset();
            return true;
        }
        return false;
    });
    retired_.erase(keep, retired_.end());
}

void RmlD3D12Renderer::begin_frame(ID3D12GraphicsCommandList* cmd, std::uint32_t width,
                                   std::uint32_t height) {
    ++frame_index_;
    collect_garbage();
    cmd_ = cmd;
    viewport_w_ = width;
    viewport_h_ = height;
    build_ortho(proj_, static_cast<float>(width), static_cast<float>(height));
    full_rect_ = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    scissor_enabled_ = false;
    scissor_rect_ = full_rect_;

    ID3D12DescriptorHeap* heaps[] = {srv_heap_.Get()};
    cmd_->SetDescriptorHeaps(1, heaps);
    cmd_->SetGraphicsRootSignature(root_.Get());
    cmd_->SetPipelineState(pso_.Get());
    cmd_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D3D12_VIEWPORT vp{0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height),
                      0.0f, 1.0f};
    cmd_->RSSetViewports(1, &vp);
    cmd_->RSSetScissorRects(1, &full_rect_);
}

void RmlD3D12Renderer::end_frame() { cmd_ = nullptr; }

Rml::CompiledGeometryHandle RmlD3D12Renderer::CompileGeometry(
    Rml::Span<const Rml::Vertex> vertices, Rml::Span<const int> indices) {
    Geometry g;
    g.index_count = static_cast<UINT>(indices.size());
    g.vb_bytes = static_cast<UINT>(vertices.size() * sizeof(Rml::Vertex));
    g.ib_bytes = static_cast<UINT>(indices.size() * sizeof(int));
    g.vb = create_upload_buffer(g.vb_bytes, vertices.data());
    g.ib = create_upload_buffer(g.ib_bytes, indices.data());
    const Rml::CompiledGeometryHandle handle = next_geometry_++;
    geometry_.emplace(handle, std::move(g));
    return handle;
}

void RmlD3D12Renderer::RenderGeometry(Rml::CompiledGeometryHandle geometry,
                                      Rml::Vector2f translation,
                                      Rml::TextureHandle texture) {
    if (!cmd_) return;
    auto it = geometry_.find(geometry);
    if (it == geometry_.end()) return;
    const Geometry& g = it->second;
    if (g.index_count == 0 || !g.vb || !g.ib) return;

    // b0: uProj (16) + uTranslate (2) + pad (2) = 20 DWORDs.
    float constants[20];
    std::memcpy(constants, proj_, sizeof(proj_));
    constants[16] = translation.x;
    constants[17] = translation.y;
    constants[18] = 0.0f;
    constants[19] = 0.0f;
    cmd_->SetGraphicsRoot32BitConstants(0, 20, constants, 0);

    D3D12_GPU_DESCRIPTOR_HANDLE srv = white_.gpu;
    if (texture != 0) {
        auto t = textures_.find(texture);
        if (t != textures_.end()) srv = t->second.gpu;
    }
    cmd_->SetGraphicsRootDescriptorTable(1, srv);

    const D3D12_RECT& rect = scissor_enabled_ ? scissor_rect_ : full_rect_;
    cmd_->RSSetScissorRects(1, &rect);

    D3D12_VERTEX_BUFFER_VIEW vbv{};
    vbv.BufferLocation = g.vb->GetGPUVirtualAddress();
    vbv.SizeInBytes = g.vb_bytes;
    vbv.StrideInBytes = sizeof(Rml::Vertex);
    D3D12_INDEX_BUFFER_VIEW ibv{};
    ibv.BufferLocation = g.ib->GetGPUVirtualAddress();
    ibv.SizeInBytes = g.ib_bytes;
    ibv.Format = DXGI_FORMAT_R32_UINT;
    cmd_->IASetVertexBuffers(0, 1, &vbv);
    cmd_->IASetIndexBuffer(&ibv);
    cmd_->DrawIndexedInstanced(g.index_count, 1, 0, 0, 0);
}

void RmlD3D12Renderer::ReleaseGeometry(Rml::CompiledGeometryHandle geometry) {
    auto it = geometry_.find(geometry);
    if (it == geometry_.end()) return;
    Retired r;
    r.frame = frame_index_;
    r.a = std::move(it->second.vb);
    r.b = std::move(it->second.ib);
    retired_.push_back(std::move(r));
    geometry_.erase(it);
}

Rml::TextureHandle RmlD3D12Renderer::LoadTexture(Rml::Vector2i& texture_dimensions,
                                                 const Rml::String& source) {
    texture_dimensions = Rml::Vector2i(0, 0);
    // WIC decode -> premultiplied RGBA8 (matches GenerateTexture's convention).
    // Requires COM initialized on the calling thread. Not exercised by the
    // M-UI-1 shell (its RCSS references no images) but implemented per SPEC §2.1.
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory)))) {
        return 0;
    }
    const std::wstring wpath(source.begin(), source.end());
    ComPtr<IWICBitmapDecoder> decoder;
    if (FAILED(factory->CreateDecoderFromFilename(wpath.c_str(), nullptr, GENERIC_READ,
                                                  WICDecodeMetadataCacheOnDemand, &decoder))) {
        return 0;
    }
    ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, &frame))) return 0;
    ComPtr<IWICFormatConverter> conv;
    if (FAILED(factory->CreateFormatConverter(&conv))) return 0;
    if (FAILED(conv->Initialize(frame.Get(), GUID_WICPixelFormat32bppPRGBA,
                                WICBitmapDitherTypeNone, nullptr, 0.0,
                                WICBitmapPaletteTypeCustom))) {
        return 0;
    }
    UINT w = 0, h = 0;
    conv->GetSize(&w, &h);
    if (w == 0 || h == 0) return 0;
    const UINT stride = w * 4;
    std::vector<std::uint8_t> pixels(static_cast<size_t>(stride) * h);
    if (FAILED(conv->CopyPixels(nullptr, stride, static_cast<UINT>(pixels.size()),
                                pixels.data()))) {
        return 0;
    }
    ComPtrRes res = upload_texture_sync(w, h, pixels.data(), stride);
    if (!res) return 0;
    texture_dimensions = Rml::Vector2i(static_cast<int>(w), static_cast<int>(h));
    return register_texture(std::move(res), w, h);
}

Rml::TextureHandle RmlD3D12Renderer::GenerateTexture(Rml::Span<const Rml::byte> source,
                                                     Rml::Vector2i source_dimensions) {
    const UINT w = static_cast<UINT>(source_dimensions.x);
    const UINT h = static_cast<UINT>(source_dimensions.y);
    if (w == 0 || h == 0) return 0;
    // RmlUi supplies premultiplied RGBA8, tightly packed (row pitch = w*4).
    ComPtrRes res = upload_texture_sync(w, h, source.data(), w * 4);
    if (!res) return 0;
    return register_texture(std::move(res), w, h);
}

void RmlD3D12Renderer::ReleaseTexture(Rml::TextureHandle texture) {
    auto it = textures_.find(texture);
    if (it == textures_.end()) return;
    Retired r;
    r.frame = frame_index_;
    r.a = std::move(it->second.resource);
    r.has_slot = true;
    r.slot = it->second.slot;
    retired_.push_back(std::move(r));
    textures_.erase(it);
}

void RmlD3D12Renderer::EnableScissorRegion(bool enable) { scissor_enabled_ = enable; }

void RmlD3D12Renderer::SetScissorRegion(Rml::Rectanglei region) {
    scissor_rect_ = {static_cast<LONG>(region.Left()), static_cast<LONG>(region.Top()),
                     static_cast<LONG>(region.Right()), static_cast<LONG>(region.Bottom())};
}

}  // namespace ftd::native::ui
