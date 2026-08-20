// M-UI-1 headless smoke test: render the FTD native shell (shell.rml + ftd.rcss)
// through RmlD3D12Renderer into an offscreen 1280x800 RGBA8 render target, read
// it back, assert a meaningful fraction of pixels differ from the clear colour
// (proving RML/RCSS content — panels, toggles, text — actually rasterized), and
// save a PNG so the result is visually verifiable.
//
// No window, no swapchain: a plain DEFAULT-heap render-target texture. Uses the
// same high-performance-adapter selection as D3D12Presenter, with a WARP
// fallback so the test still runs on machines/VMs without a hardware GPU.

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wincodec.h>
#include <wrl/client.h>

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include <RmlUi/Core.h>
#include <RmlUi/Core/Factory.h>

#include "ui/rml_d3d12_renderer.h"
#include "ui/ftd_chart_element.h"

using Microsoft::WRL::ComPtr;
using ftd::native::ui::RmlD3D12Renderer;
using ftd::native::ui::RmlD3D12System;
using ftd::native::ui::FtdChartInstancer;

namespace {

constexpr UINT kWidth = 1280;
constexpr UINT kHeight = 800;

// The shell body background (#0d1420) doubles as the clear colour, so only the
// chrome (toolbar, side panels, toggles, status bar, text) differs from it.
constexpr std::uint8_t kClearR = 0x0d;  // 13
constexpr std::uint8_t kClearG = 0x14;  // 20
constexpr std::uint8_t kClearB = 0x20;  // 32

void die(const char* msg) {
    std::fprintf(stderr, "FAIL: %s\n", msg);
}

std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()),
                                nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

// Mirrors D3D12Presenter::pick_hardware_adapter: high-performance ordering,
// first non-software adapter.
ComPtr<IDXGIAdapter1> pick_hardware_adapter(IDXGIFactory4* factory) {
    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&factory6)))) {
        ComPtr<IDXGIAdapter1> adapter;
        for (UINT i = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                 i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
             ++i) {
            DXGI_ADAPTER_DESC1 desc{};
            adapter->GetDesc1(&desc);
            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
            return adapter;
        }
    }
    ComPtr<IDXGIAdapter1> adapter;
    for (UINT i = 0; SUCCEEDED(factory->EnumAdapters1(i, &adapter)); ++i) {
        DXGI_ADAPTER_DESC1 desc{};
        adapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        return adapter;
    }
    return nullptr;
}

bool save_png(const std::wstring& path, const std::uint8_t* rgba, UINT w, UINT h,
              UINT row_pitch) {
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory)))) {
        return false;
    }
    ComPtr<IWICStream> stream;
    if (FAILED(factory->CreateStream(&stream))) return false;
    if (FAILED(stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE))) return false;
    ComPtr<IWICBitmapEncoder> encoder;
    if (FAILED(factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &encoder))) return false;
    if (FAILED(encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache))) return false;
    ComPtr<IWICBitmapFrameEncode> frame;
    ComPtr<IPropertyBag2> props;
    if (FAILED(encoder->CreateNewFrame(&frame, &props))) return false;
    if (FAILED(frame->Initialize(props.Get()))) return false;
    if (FAILED(frame->SetSize(w, h))) return false;
    // Request BGRA (universally supported by the PNG encoder). WriteSource maps
    // channels from the source's declared format, so tagging the source bitmap
    // 32bppRGBA is what makes the channels correct regardless of what format the
    // encoder settles on. (Feeding RGBA bytes straight to WritePixels while the
    // encoder silently downgrades RGBA->BGRA is exactly what swaps R and B.)
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(frame->SetPixelFormat(&fmt))) return false;
    ComPtr<IWICBitmap> source;
    if (FAILED(factory->CreateBitmapFromMemory(w, h, GUID_WICPixelFormat32bppRGBA,
                                               row_pitch, row_pitch * h,
                                               const_cast<BYTE*>(rgba), &source))) {
        return false;
    }
    if (FAILED(frame->WriteSource(source.Get(), nullptr))) return false;
    if (FAILED(frame->Commit())) return false;
    if (FAILED(encoder->Commit())) return false;
    return true;
}

int run() {
    // Resolve asset/output paths from compile-time definitions.
    const std::string shell_path = FTD_RML_SHELL_PATH;
    const std::string font_path = FTD_RML_FONT_PATH;
    const std::string hlsl_path = FTD_RMLUI_HLSL_PATH;
    const std::string png_path = FTD_RML_PNG_OUT;

    // ── Device + DIRECT queue (hardware, WARP fallback) ──────────────────────
    ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) {
        die("CreateDXGIFactory2");
        return 2;
    }
    ComPtr<ID3D12Device> device;
    ComPtr<IDXGIAdapter1> adapter = pick_hardware_adapter(factory.Get());
    bool is_warp = false;
    if (!adapter ||
        FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                                 IID_PPV_ARGS(&device)))) {
        ComPtr<IDXGIAdapter> warp;
        if (FAILED(factory->EnumWarpAdapter(IID_PPV_ARGS(&warp))) ||
            FAILED(D3D12CreateDevice(warp.Get(), D3D_FEATURE_LEVEL_11_0,
                                     IID_PPV_ARGS(&device)))) {
            die("D3D12CreateDevice (hardware and WARP both failed)");
            return 2;
        }
        is_warp = true;
    }
    std::fprintf(stderr, "device created (%s)\n", is_warp ? "WARP" : "hardware");

    D3D12_COMMAND_QUEUE_DESC qd{};
    qd.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    ComPtr<ID3D12CommandQueue> queue;
    if (FAILED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&queue)))) {
        die("CreateCommandQueue");
        return 2;
    }

    // ── Offscreen render target (1280x800 RGBA8) ─────────────────────────────
    D3D12_HEAP_PROPERTIES def{};
    def.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_RESOURCE_DESC rt_desc{};
    rt_desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    rt_desc.Width = kWidth;
    rt_desc.Height = kHeight;
    rt_desc.DepthOrArraySize = 1;
    rt_desc.MipLevels = 1;
    rt_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    rt_desc.SampleDesc.Count = 1;
    rt_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    D3D12_CLEAR_VALUE clear_val{};
    clear_val.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clear_val.Color[0] = kClearR / 255.0f;
    clear_val.Color[1] = kClearG / 255.0f;
    clear_val.Color[2] = kClearB / 255.0f;
    clear_val.Color[3] = 1.0f;
    ComPtr<ID3D12Resource> rt;
    if (FAILED(device->CreateCommittedResource(&def, D3D12_HEAP_FLAG_NONE, &rt_desc,
                                               D3D12_RESOURCE_STATE_RENDER_TARGET,
                                               &clear_val, IID_PPV_ARGS(&rt)))) {
        die("CreateCommittedResource render target");
        return 2;
    }
    D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc{};
    rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtv_heap_desc.NumDescriptors = 1;
    ComPtr<ID3D12DescriptorHeap> rtv_heap;
    if (FAILED(device->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)))) {
        die("CreateDescriptorHeap RTV");
        return 2;
    }
    const D3D12_CPU_DESCRIPTOR_HANDLE rtv = rtv_heap->GetCPUDescriptorHandleForHeapStart();
    device->CreateRenderTargetView(rt.Get(), nullptr, rtv);

    // ── Frame command infrastructure ─────────────────────────────────────────
    ComPtr<ID3D12CommandAllocator> alloc;
    ComPtr<ID3D12GraphicsCommandList> list;
    ComPtr<ID3D12Fence> fence;
    if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                              IID_PPV_ARGS(&alloc))) ||
        FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, alloc.Get(),
                                         nullptr, IID_PPV_ARGS(&list))) ||
        FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
        die("frame command infra");
        return 2;
    }
    list->Close();
    HANDLE fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if (!fence_event) {
        die("CreateEventW");
        return 2;
    }

    // Readback buffer sized to the 256-aligned copy footprint.
    const UINT row_pitch =
        (kWidth * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1) &
        ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1);
    const UINT64 readback_bytes = static_cast<UINT64>(row_pitch) * kHeight;
    D3D12_HEAP_PROPERTIES readback_heap{};
    readback_heap.Type = D3D12_HEAP_TYPE_READBACK;
    D3D12_RESOURCE_DESC rb_desc{};
    rb_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    rb_desc.Width = readback_bytes;
    rb_desc.Height = 1;
    rb_desc.DepthOrArraySize = 1;
    rb_desc.MipLevels = 1;
    rb_desc.SampleDesc.Count = 1;
    rb_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ComPtr<ID3D12Resource> readback;
    if (FAILED(device->CreateCommittedResource(&readback_heap, D3D12_HEAP_FLAG_NONE,
                                               &rb_desc, D3D12_RESOURCE_STATE_COPY_DEST,
                                               nullptr, IID_PPV_ARGS(&readback)))) {
        die("CreateCommittedResource readback");
        return 2;
    }

    // ── RmlUi setup ──────────────────────────────────────────────────────────
    RmlD3D12System system;
    RmlD3D12Renderer renderer;
    renderer.initialize(device.Get(), queue.Get(), widen(hlsl_path).c_str());

    Rml::SetSystemInterface(&system);
    Rml::SetRenderInterface(&renderer);
    if (!Rml::Initialise()) {
        die("Rml::Initialise");
        return 2;
    }

    // Register the <ftd-chart> instancer with a null series: the shell now
    // contains an <ftd-chart>, and this both proves the custom element instances
    // + renders headlessly and keeps the tag from falling back to a plain element.
    // A null series draws only the chart baseline (no trace), which is harmless.
    // Declared here so it outlives every Rml::Shutdown() path below.
    ftd::native::ui::FtdChartInstancer chart_instancer(nullptr);
    Rml::Factory::RegisterElementInstancer("ftd-chart", &chart_instancer);

    if (!Rml::LoadFontFace(font_path)) {
        die("LoadFontFace (Inter-Regular.ttf)");
        Rml::Shutdown();
        return 2;
    }
    // The shell RCSS also styles elements with "JetBrains Mono"; register the
    // Inter bytes under that family so those elements render text too (test-only
    // shim — a JetBrains Mono face is not vendored yet). Data must outlive
    // Rml::Shutdown(), so keep it in a static buffer.
    static std::vector<Rml::byte> mono_bytes;
    {
        std::FILE* f = nullptr;
        if (_wfopen_s(&f, widen(font_path).c_str(), L"rb") == 0 && f) {
            std::fseek(f, 0, SEEK_END);
            long n = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            if (n > 0) {
                mono_bytes.resize(static_cast<size_t>(n));
                size_t got = std::fread(mono_bytes.data(), 1, mono_bytes.size(), f);
                mono_bytes.resize(got);
            }
            std::fclose(f);
        }
        if (!mono_bytes.empty()) {
            Rml::LoadFontFace(Rml::Span<const Rml::byte>(mono_bytes.data(), mono_bytes.size()),
                              "JetBrains Mono", Rml::Style::FontStyle::Normal,
                              Rml::Style::FontWeight::Auto, false);
        }
    }

    Rml::Context* context =
        Rml::CreateContext("main", Rml::Vector2i(kWidth, kHeight));
    if (!context) {
        die("CreateContext");
        Rml::Shutdown();
        return 2;
    }

    Rml::ElementDocument* doc = context->LoadDocument(shell_path);
    if (!doc) {
        die("LoadDocument (shell.rml)");
        Rml::Shutdown();
        return 2;
    }
    doc->Show();
    context->Update();

    // ── Record + render one frame ────────────────────────────────────────────
    if (FAILED(alloc->Reset()) || FAILED(list->Reset(alloc.Get(), nullptr))) {
        die("reset frame list");
        Rml::Shutdown();
        return 2;
    }
    list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    const float clear[4] = {kClearR / 255.0f, kClearG / 255.0f, kClearB / 255.0f, 1.0f};
    list->ClearRenderTargetView(rtv, clear, 0, nullptr);

    renderer.begin_frame(list.Get(), kWidth, kHeight);
    context->Render();
    renderer.end_frame();

    // RT -> COPY_SOURCE, copy into readback.
    D3D12_RESOURCE_BARRIER to_copy{};
    to_copy.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    to_copy.Transition.pResource = rt.Get();
    to_copy.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    to_copy.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
    to_copy.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    list->ResourceBarrier(1, &to_copy);

    D3D12_TEXTURE_COPY_LOCATION src{};
    src.pResource = rt.Get();
    src.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
    src.SubresourceIndex = 0;
    D3D12_TEXTURE_COPY_LOCATION dst{};
    dst.pResource = readback.Get();
    dst.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
    dst.PlacedFootprint.Offset = 0;
    dst.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    dst.PlacedFootprint.Footprint.Width = kWidth;
    dst.PlacedFootprint.Footprint.Height = kHeight;
    dst.PlacedFootprint.Footprint.Depth = 1;
    dst.PlacedFootprint.Footprint.RowPitch = row_pitch;
    list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

    if (FAILED(list->Close())) {
        die("Close frame list");
        Rml::Shutdown();
        return 2;
    }
    ID3D12CommandList* lists[] = {list.Get()};
    queue->ExecuteCommandLists(1, lists);
    const UINT64 frame_fence = 1;
    if (FAILED(queue->Signal(fence.Get(), frame_fence))) {
        die("Signal frame fence");
        Rml::Shutdown();
        return 2;
    }
    if (fence->GetCompletedValue() < frame_fence) {
        fence->SetEventOnCompletion(frame_fence, fence_event);
        WaitForSingleObject(fence_event, INFINITE);
    }

    // ── Read back + analyze ──────────────────────────────────────────────────
    std::uint8_t* mapped = nullptr;
    D3D12_RANGE full{0, static_cast<SIZE_T>(readback_bytes)};
    if (FAILED(readback->Map(0, &full, reinterpret_cast<void**>(&mapped)))) {
        die("Map readback");
        Rml::Shutdown();
        return 2;
    }
    std::vector<std::uint8_t> pixels(static_cast<size_t>(readback_bytes));
    std::memcpy(pixels.data(), mapped, static_cast<size_t>(readback_bytes));
    D3D12_RANGE nowrite{0, 0};
    readback->Unmap(0, &nowrite);

    // The shell is a blue-accented dark theme with no red/orange content, so a
    // correct RGBA channel order yields far more "strongly blue" than "strongly
    // red" pixels; an R<->B swap anywhere in the pipeline inverts that. This is a
    // layout-independent guard on channel order (the readback is the RT the
    // native app would present, so it also proves the render — not just the PNG).
    std::uint64_t differing = 0, blue_px = 0, red_px = 0;
    for (UINT y = 0; y < kHeight; ++y) {
        const std::uint8_t* row = pixels.data() + static_cast<UINT64>(y) * row_pitch;
        for (UINT x = 0; x < kWidth; ++x) {
            const std::uint8_t* px = row + static_cast<size_t>(x) * 4;
            const int r = px[0], g = px[1], b = px[2];
            if (std::abs(r - int(kClearR)) + std::abs(g - int(kClearG)) +
                    std::abs(b - int(kClearB)) > 12) {
                ++differing;
            }
            if (b > 100 && b - r > 40) ++blue_px;
            if (r > 100 && r - b > 40) ++red_px;
        }
    }
    const double total = double(kWidth) * double(kHeight);
    const double frac = double(differing) / total;
    std::fprintf(stderr, "differing pixels: %llu / %.0f = %.2f%%\n",
                 static_cast<unsigned long long>(differing), total, frac * 100.0);
    std::fprintf(stderr, "channel order: blue=%llu red=%llu (blue should dominate)\n",
                 static_cast<unsigned long long>(blue_px),
                 static_cast<unsigned long long>(red_px));

    // ── Save PNG (before asserting, so a failure still leaves an artifact) ────
    const std::wstring wpng = widen(png_path);
    const bool png_ok = save_png(wpng, pixels.data(), kWidth, kHeight, row_pitch);
    if (png_ok) {
        std::fprintf(stderr, "wrote PNG: %s\n", png_path.c_str());
    } else {
        std::fprintf(stderr, "WARN: failed to write PNG: %s\n", png_path.c_str());
    }

    // ── Teardown (renderer must outlive Shutdown; it releases resources) ──────
    context->UnloadAllDocuments();
    Rml::Shutdown();
    CloseHandle(fence_event);

    // ── Verdict ──────────────────────────────────────────────────────────────
    // >5% of pixels changed proves RML/RCSS content rendered. Expected ~40%+
    // (two 240px side panels + toolbar + status bar over the shared bg colour).
    if (frac < 0.05) {
        die("too few pixels differ from the clear colour — RML content did not render");
        return 1;
    }
    if (blue_px < 1000 || blue_px <= red_px) {
        die("channel order looks wrong — blue-accented theme did not render blue-dominant");
        return 1;
    }
    if (!png_ok) {
        die("PNG was not written");
        return 1;
    }
    std::fprintf(stderr, "PASS\n");
    return 0;
}

}  // namespace

int main() {
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    int rc = 2;
    try {
        rc = run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "FAIL: exception: %s\n", e.what());
        rc = 2;
    }
    if (SUCCEEDED(co)) CoUninitialize();
    return rc;
}
