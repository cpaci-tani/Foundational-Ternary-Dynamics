// native_app — the live windowed FTD native application (M-UI-1..M-UI-3 fused).
//
// A real Win32 window whose swapchain is owned by D3D12Presenter. Two threads,
// the same split the native_desktop reference (native_desktop/src/main.cpp) uses:
//   • sim thread   — owns a ScaleHost (Scale 0 behind the ScaleHost/ScaleAdapter
//                    seam), ticks it, drains the CommandBus at the tick boundary,
//                    publishes a HostSnapshot + a NativeFrame.
//   • GUI thread   — owns RmlUi (RmlD3D12Renderer + Rml::Context) and the
//                    presenter. Each frame it acquires the published snapshot,
//                    pushes it into the RmlUi data model, lays out (Context::
//                    Update), then calls presenter.render(...). The RmlUi shell
//                    is drawn INSIDE the presenter's OverlayRecorder seam so the
//                    CSS chrome composites over the live 3D scene in one frame.
//
// The shell's transparent `#viewport` element marks the 3D hole: the presenter's
// scene_rect is set to that element's laid-out rectangle each frame, so the
// interop-free CPU-gathered lattice/particle/flux scene renders exactly there
// while RmlUi draws the toolbar / Setup panel / Physics-terms panel / status bar
// around it.
//
// On the GPU backend at Scale 0 this app wires the CUDA<->D3D12 zero-copy
// interop path (mirroring the native_desktop reference): the presenter exposes
// a shared particle buffer + a shared cross-API fence, CUDA imports both, and
// the interop gather writes device-resident particles straight into that
// buffer. The sim thread requests a gather + advances the shared fence each
// round; the GUI thread waits on that fence and draws the device-resident
// particles (presenter.render(..., interop_particle_count > 0)). The CPU sprite
// path (interop_particle_count = 0) remains the fallback for the CPU backend,
// for Scale 1 (ParticleEngine has no device buffer), and for any frame whose
// gather has not yet landed — host.capture() still supplies frame.particles +
// flux + metadata every round, so a fallback frame is never blank.
//
// --capture-frames N: boot, run the sim while rendering, request a composited
// back-buffer readback, save it as a PNG, and exit 0 — the headless proof that
// chrome + live scene + real data all land in one frame.

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
// NOTE: deliberately NOT <windowsx.h> — its message-cracker macros
// (GetNextSibling / GetFirstChild / GetLastChild / …) collide with RmlUi's
// Rml::Element methods of the same name and break <RmlUi/Core.h>. The only
// windowsx.h helpers this file needs are the signed LPARAM coordinate
// extractors, defined locally below.
#include <shellapi.h>
#include <wincodec.h>
#include <wrl/client.h>

#ifndef _WIN64
#error "native_app is a Win64 (x86-64) target"
#endif

#include "native/cli_options.h"
#include "native/d3d12_presenter.h"
#include "native/dpi_support.h"
#include "native/host/command_bus.h"
#include "native/host/scale_host.h"
#include "native/model/commands.h"
#include "native/model/snapshot.h"
#include "native/native_frame.h"
#include "native/scene_rect.h"

#include "ftd/term_toggles.h"
#include "ftd/visual_snapshot.h"   // ftd::kMaxVisualParticleCapture (interop buffer sizing)

#include "ui/rml_d3d12_renderer.h"
#include "ui/ftd_chart_element.h"

#include <RmlUi/Core.h>
#include <RmlUi/Core/Factory.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;
using ftd::native::ui::RmlD3D12Renderer;
using ftd::native::ui::RmlD3D12System;

namespace {

// Signed LPARAM coordinate extractors (would come from <windowsx.h>, which we
// cannot include — see the include block above).
inline int lparam_x(LPARAM lp) { return static_cast<int>(static_cast<short>(LOWORD(lp))); }
inline int lparam_y(LPARAM lp) { return static_cast<int>(static_cast<short>(HIWORD(lp))); }

// ── The physics-terms panel: which toggles it shows, top to bottom. Names are
//    the canonical TermToggles field names (term_toggles.h) so a click maps to
//    a SetToggle command 1:1 and the on-state reads straight from the snapshot.
constexpr const char* kPanelToggles[] = {
    "wave_propagation", "coupling",       "damping",         "genesis",
    "gauss_projection", "forces",         "movement",        "poisson_coulomb",
    "selective_damping","gravity",        "lorentz_force",   "dual_substrate",
    "color_forces",     "strong_force",   "weak_transmutation",
    "de_broglie_clock",
};

// One tick in physical seconds (electron-primary gauge: t_phys = t_P/√3, see
// CLAUDE.md). Used only to render a human-facing "physical time" in the status
// bar; nothing physical depends on it.
constexpr double kTPhysSeconds = 3.11e-44;

// ── RmlUi data-model mirror of UiSnapshot (the bound C++ side of the shell) ──
struct ToggleRow {
    Rml::String name;
    bool on = false;
};

struct ShellData {
    int tick = 0;
    int active_scale = 0;   // drives the toolbar scale-switcher highlight
    int particle_count = 0;
    Rml::String physical_time = "0 s";
    Rml::String total_energy = "0.0";
    Rml::String s1_ke = "0.0";   // Scale-1 kinetic energy (particle readout)
    Rml::String s1_pe = "0.0";   // Scale-1 potential energy (particle readout)
    Rml::String scenario;
    Rml::String backend = "CPU";
    Rml::String lattice = "0";
    int fps = 0;
    bool running = false;
    Rml::Vector<ToggleRow> toggles;
};

bool toggle_on(const ftd::TermToggles& tt, const char* name) {
    const ftd::ToggleSpec* spec = ftd::term_toggles_detail::find_spec(name);
    return spec ? (tt.*(spec->field)) : false;
}

// ── Everything the Win32 wnd_proc + RmlUi event callbacks need to reach. Set
//    once on the GUI thread during setup and read only on the GUI thread
//    (wnd_proc runs during DispatchMessageW on this same thread), so no locking.
struct AppContext {
    HWND hwnd = nullptr;
    Rml::Context* context = nullptr;
    ftd::native::D3D12Presenter* presenter = nullptr;
    ftd::native::Camera* camera = nullptr;
    ftd::native::CommandBus* commands = nullptr;
    ShellData* data = nullptr;
    std::atomic<bool>* paused = nullptr;
    std::atomic<bool>* quit = nullptr;
    std::string scenario_id;

    // The laid-out #viewport hole in client pixels (updated each frame after
    // Context::Update). Pointer arbitration + the presenter scene_rect use it.
    ftd::native::SceneRect viewport_rect{};
    bool dragging = false;
    POINT last{};
};

// ── Command helpers (run on the GUI thread; drained by the sim thread) ──
// The bus carries scale-generic ScaleCommands: loop control / reload are core
// commands the host handles; toggles are a Scale-0 payload the adapter handles.
void push_core(AppContext* app, ftd::native::CoreCommand cmd) {
    if (app && app->commands)
        app->commands->push(ftd::native::core_command(std::move(cmd)));
}
void push_scale0(AppContext* app, ftd::native::Scale0Cmd cmd) {
    if (app && app->commands)
        app->commands->push(ftd::native::scale0_command(std::move(cmd)));
}

void request_play(AppContext* app) {
    push_core(app, ftd::native::Run{});
    if (app->paused) app->paused->store(false);
}
void request_pause(AppContext* app) {
    push_core(app, ftd::native::Pause{});
    if (app->paused) app->paused->store(true);
}
void request_play_toggle(AppContext* app) {
    if (app->paused && app->paused->load()) request_play(app);
    else request_pause(app);
}
void request_step(AppContext* app) {
    push_core(app, ftd::native::Pause{});
    push_core(app, ftd::native::Step{1});
    if (app->paused) app->paused->store(true);
}
void request_reset(AppContext* app) {
    if (!app->scenario_id.empty())
        push_core(app, ftd::native::LoadScenario{app->scenario_id});
}
// Push a core SwitchScale: the host tears down the active adapter, rebuilds via
// make_scale_adapter(level), and reboots into the given per-scale seed id. The
// app tracks the active scenario so Reset targets the newly active scale.
void request_switch_scale(AppContext* app, int level) {
    const char* scenario = (level == 1) ? "s1-hydrogen-cloud" : "s0-seed-hydrogen";
    push_core(app, ftd::native::SwitchScale{level, scenario});
    app->scenario_id = scenario;
}
void request_toggle(AppContext* app, const std::string& name) {
    bool cur = false;
    if (app->data) {
        for (const ToggleRow& r : app->data->toggles) {
            if (r.name == name) { cur = r.on; break; }
        }
    }
    push_scale0(app, ftd::native::SetToggle{name, !cur});
}

// ── Win32 → RmlUi input plumbing ──
int rml_key_modifiers() {
    int m = 0;
    if (GetKeyState(VK_CONTROL) & 0x8000) m |= Rml::Input::KM_CTRL;
    if (GetKeyState(VK_SHIFT) & 0x8000) m |= Rml::Input::KM_SHIFT;
    if (GetKeyState(VK_MENU) & 0x8000) m |= Rml::Input::KM_ALT;
    return m;
}

// True when the given client point is inside the laid-out #viewport hole, i.e.
// the pointer is over the 3D scene and should drive the camera rather than the
// (transparent) RML element that marks the hole.
bool over_viewport(const AppContext* app, int x, int y) {
    return ftd::native::scene_contains_client(app->viewport_rect, x, y);
}

AppContext* app_from_hwnd(HWND hwnd) {
    return reinterpret_cast<AppContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppContext* app = app_from_hwnd(hwnd);
    Rml::Context* ctx = app ? app->context : nullptr;
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            ftd::native::apply_dpi_suggested_rect(hwnd, lparam);
            return 0;
        case WM_MOUSEMOVE: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseMove(x, y, rml_key_modifiers());
            if (app->dragging) {
                app->camera->yaw += (x - app->last.x) * 0.01f;
                app->camera->pitch += (y - app->last.y) * 0.01f;
                app->camera->pitch = std::max(-1.4f, std::min(1.4f, app->camera->pitch));
                app->last = {x, y};
            }
            return 0;
        }
        case WM_LBUTTONDOWN: {
            const int x = lparam_x(lparam), y = lparam_y(lparam);
            if (ctx) ctx->ProcessMouseButtonDown(0, rml_key_modifiers());
            if (over_viewport(app, x, y)) {
                app->dragging = true;
                app->last = {x, y};
                SetCapture(hwnd);
            }
            return 0;
        }
        case WM_LBUTTONUP:
            if (ctx) ctx->ProcessMouseButtonUp(0, rml_key_modifiers());
            app->dragging = false;
            if (GetCapture() == hwnd) ReleaseCapture();
            return 0;
        case WM_RBUTTONDOWN:
            if (ctx) ctx->ProcessMouseButtonDown(1, rml_key_modifiers());
            return 0;
        case WM_RBUTTONUP:
            if (ctx) ctx->ProcessMouseButtonUp(1, rml_key_modifiers());
            return 0;
        case WM_CAPTURECHANGED:
        case WM_KILLFOCUS:
            if (app) app->dragging = false;
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        case WM_MOUSEWHEEL: {
            POINT pt{lparam_x(lparam), lparam_y(lparam)};
            ScreenToClient(hwnd, &pt);
            const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            if (over_viewport(app, pt.x, pt.y)) {
                app->camera->distance *= (delta > 0) ? 0.9f : 1.1f;
                app->camera->distance = std::max(4.0f, std::min(512.0f, app->camera->distance));
            } else if (ctx) {
                ctx->ProcessMouseWheel(Rml::Vector2f(0.0f, delta > 0 ? -1.0f : 1.0f),
                                       rml_key_modifiers());
            }
            return 0;
        }
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

// ── OverlayRecorder: draw the RmlUi shell into the presenter's command list ──
// Called from inside D3D12Presenter::render(), after the 3D scene is recorded
// and with the full-window RTV bound. begin_frame rebinds the UI heap / PSO /
// ortho / full viewport+scissor, Context::Render() emits geometry through the
// RenderInterface into `list`, end_frame() stops routing.
class RmlOverlay : public ftd::native::OverlayRecorder {
public:
    RmlOverlay(RmlD3D12Renderer* renderer, Rml::Context** context)
        : renderer_(renderer), context_(context) {}
    void record(ID3D12GraphicsCommandList* list,
                const ftd::native::RenderTargetInfo& rt) override {
        if (!renderer_ || !*context_) return;
        renderer_->begin_frame(list, rt.width, rt.height);
        (*context_)->Render();
        renderer_->end_frame();
    }

private:
    RmlD3D12Renderer* renderer_;
    Rml::Context** context_;
};

// ── PNG writer (WIC, RGBA8 rows) — mirrors test_ui_rml_smoke.cpp's save_png ──
std::wstring widen(const std::string& s) {
    if (s.empty()) return std::wstring();
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), nullptr, 0);
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), static_cast<int>(s.size()), w.data(), n);
    return w;
}

bool save_png(const std::wstring& path, const std::uint8_t* rgba, UINT w, UINT h,
              UINT row_pitch) {
    ComPtr<IWICImagingFactory> factory;
    if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                IID_PPV_ARGS(&factory))))
        return false;
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
    WICPixelFormatGUID fmt = GUID_WICPixelFormat32bppBGRA;
    if (FAILED(frame->SetPixelFormat(&fmt))) return false;
    ComPtr<IWICBitmap> source;
    if (FAILED(factory->CreateBitmapFromMemory(w, h, GUID_WICPixelFormat32bppRGBA, row_pitch,
                                               row_pitch * h, const_cast<BYTE*>(rgba), &source)))
        return false;
    if (FAILED(frame->WriteSource(source.Get(), nullptr))) return false;
    if (FAILED(frame->Commit())) return false;
    if (FAILED(encoder->Commit())) return false;
    return true;
}

// Bind stdout/stderr to the launching console so --capture-frames logging is
// visible for a WIN32-subsystem exe (copied from the native_desktop reference).
bool bind_crt_to_std_handle(DWORD std_id, int crt_fd) {
    HANDLE handle = GetStdHandle(std_id);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) return false;
    const DWORD type = GetFileType(handle);
    if (type != FILE_TYPE_DISK && type != FILE_TYPE_PIPE && type != FILE_TYPE_CHAR) return false;
    const int fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_TEXT);
    if (fd < 0) return false;
    _dup2(fd, crt_fd);
    return true;
}
void attach_parent_console_if_any() {
    const bool out_bound = bind_crt_to_std_handle(STD_OUTPUT_HANDLE, 1);
    bind_crt_to_std_handle(STD_ERROR_HANDLE, 2);
    if (out_bound) { std::cout.clear(); std::cerr.clear(); return; }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) return;
    FILE* s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stdout);
    s = nullptr;
    freopen_s(&s, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

std::string wide_to_utf8(const wchar_t* wide) {
    if (!wide || wide[0] == L'\0') return {};
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (bytes <= 1) return {};
    std::string out(static_cast<size_t>(bytes - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), bytes, nullptr, nullptr);
    return out;
}
std::vector<std::string> utf8_args() {
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;
    if (!wargv) return {"native_app"};
    for (int i = 0; i < argc; ++i) args.push_back(wide_to_utf8(wargv[i]));
    LocalFree(wargv);
    if (args.empty()) args.emplace_back("native_app");
    return args;
}

std::string upper(std::string s) {
    for (char& c : s) c = static_cast<char>(::toupper(static_cast<unsigned char>(c)));
    return s;
}
std::string fmt(const char* f, double v) {
    char buf[64];
    std::snprintf(buf, sizeof(buf), f, v);
    return buf;
}

void apply_camera_for_lattice(ftd::native::Camera& cam, int lattice) {
    const float c = static_cast<float>(lattice) * 0.5f;
    cam.target_x = cam.target_y = cam.target_z = c;
    cam.distance = static_cast<float>(lattice) * 1.8f;
}

struct AppOptions {
    int capture_frames = -1;   // -1 = interactive; >=0 = capture then exit
    bool start_paused = false; // default: live on launch
    int scale = 0;             // initial ScaleHost scale level (0 lattice, 1 particles)
};

AppOptions parse_app_options(const std::vector<std::string>& args) {
    AppOptions o;
    for (size_t i = 1; i < args.size(); ++i) {
        if (args[i] == "--capture-frames" && i + 1 < args.size()) {
            o.capture_frames = std::max(1, std::atoi(args[++i].c_str()));
        } else if (args[i] == "--paused") {
            o.start_paused = true;
        } else if (args[i] == "--run") {
            o.start_paused = false;
        } else if (args[i] == "--scale" && i + 1 < args.size()) {
            o.scale = std::max(0, std::atoi(args[++i].c_str()));
        }
    }
    return o;
}

int run_app(const std::vector<std::string>& args) {
    // ── CLI ──────────────────────────────────────────────────────────────────
    std::vector<const char*> argv;
    for (const std::string& a : args) argv.push_back(a.c_str());
    const auto parsed = ftd::native::parse_native_cli(static_cast<int>(argv.size()), argv.data());
    const AppOptions app_opts = parse_app_options(args);
    const bool capture_mode = app_opts.capture_frames >= 0;

    if (!ftd::native::enable_per_monitor_v2_dpi())
        throw std::runtime_error("Per-monitor-V2 DPI awareness unavailable");

    ftd::native::NativeEngineOptions engine_opts = parsed.options;
    std::cout << "native_app: L=" << engine_opts.lattice_size
              << " scenario=" << engine_opts.scenario
              << (engine_opts.force_cpu ? " cpu" : " gpu-default")
              << (capture_mode ? " [capture]" : "") << "\n" << std::flush;

    // ── Scale host (Scale 0/1 behind the ScaleHost/ScaleAdapter seam) ───────────
    ftd::native::HostOptions host_opts;
    host_opts.scale_level = app_opts.scale;
    host_opts.scenario = engine_opts.scenario;
    // Scale 1 has its own seed vocabulary; the Scale-0 default scenario would boot
    // the ParticleEngine into its fallback cloud anyway, but naming a Scale-1 id
    // keeps the status/scenario readout honest.
    if (app_opts.scale == 1 && host_opts.scenario.rfind("s1-", 0) != 0)
        host_opts.scenario = "s1-hydrogen-cloud";
    host_opts.run.lattice_size = engine_opts.lattice_size;
    host_opts.run.force_cpu = engine_opts.force_cpu;
    host_opts.run.flux_boundary = engine_opts.flux_boundary;
    const std::string initial_scenario = host_opts.scenario;
    const int initial_scale = host_opts.scale_level;
    ftd::native::ScaleHost host(std::move(host_opts));
    std::cout << "backend=" << host.backend_name() << " status=" << host.status()
              << "\n" << std::flush;

    ftd::native::CommandBus commands;
    std::atomic<bool> running{true};
    std::atomic<bool> paused{app_opts.start_paused};
    std::atomic<bool> quit_flag{false};
    std::atomic<int> tick_hz{capture_mode ? 240 : 60};

    ftd::native::Camera camera;
    ftd::native::NativeViewOptions view_opts;
    apply_camera_for_lattice(camera, host.lattice_size());

    // Prime the loop control + publish one snapshot before the sim thread starts.
    {
        host.set_loop_control({app_opts.start_paused, !app_opts.start_paused, 0});
        ftd::native::CommandBus stamp;
        host.process_ui_boundary(stamp);
    }
    ftd::native::NativeFrame latest = host.capture();

    // ── Window ─────────────────────────────────────────────────────────────────
    WNDCLASSW wc{};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = L"FtdNativeApp";
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
    RegisterClassW(&wc);

    const DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"FTD Native", style, CW_USEDEFAULT,
                                CW_USEDEFAULT, 1600, 900, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) throw std::runtime_error("CreateWindowExW failed");

    RECT client{};
    GetClientRect(hwnd, &client);
    std::uint32_t win_w = static_cast<std::uint32_t>(std::max<LONG>(1, client.right));
    std::uint32_t win_h = static_cast<std::uint32_t>(std::max<LONG>(1, client.bottom));

    // ── Presenter (owns the swapchain on this HWND) ─────────────────────────────
    ftd::native::D3D12Presenter presenter;
    presenter.initialize(hwnd, win_w, win_h);

    // ── RmlUi renderer + context (declared AFTER presenter so it destructs
    //    FIRST — its D3D12 resources must be released while the device lives) ──
    RmlD3D12System rml_system;
    RmlD3D12Renderer rml_renderer;
    ftd::native::PresenterUiContext ui_ctx = presenter.ui_backend_context();
    if (!ui_ctx.device || !ui_ctx.queue)
        throw std::runtime_error("presenter did not expose a UI device/queue");
    rml_renderer.initialize(ui_ctx.device, ui_ctx.queue, widen(FTD_RMLUI_HLSL_PATH).c_str());

    Rml::SetSystemInterface(&rml_system);
    Rml::SetRenderInterface(&rml_renderer);
    if (!Rml::Initialise()) throw std::runtime_error("Rml::Initialise failed");

    // The shell RCSS styles some elements "Inter" and some "JetBrains Mono".
    // Load the vendored Inter face under both families (a JetBrains Mono face is
    // not vendored yet — same shim the smoke test uses).
    if (!Rml::LoadFontFace(FTD_RML_FONT_PATH))
        throw std::runtime_error("LoadFontFace(Inter-Regular.ttf) failed");
    {
        static std::vector<Rml::byte> mono;
        std::FILE* f = nullptr;
        if (_wfopen_s(&f, widen(FTD_RML_FONT_PATH).c_str(), L"rb") == 0 && f) {
            std::fseek(f, 0, SEEK_END);
            long n = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            if (n > 0) {
                mono.resize(static_cast<size_t>(n));
                mono.resize(std::fread(mono.data(), 1, mono.size(), f));
            }
            std::fclose(f);
        }
        if (!mono.empty())
            Rml::LoadFontFace(Rml::Span<const Rml::byte>(mono.data(), mono.size()),
                              "JetBrains Mono", Rml::Style::FontStyle::Normal,
                              Rml::Style::FontWeight::Auto, false);
    }

    const UINT dpi0 = GetDpiForWindow(hwnd);
    float dpi_scale = dpi0 > 0 ? static_cast<float>(dpi0) / 96.0f : 1.0f;
    Rml::Context* context =
        Rml::CreateContext("main", Rml::Vector2i(static_cast<int>(win_w), static_cast<int>(win_h)));
    if (!context) throw std::runtime_error("Rml::CreateContext failed");
    context->SetDensityIndependentPixelRatio(dpi_scale);

    // ── Data model (must exist before LoadDocument so data-model binds) ─────────
    ShellData data;
    data.scenario = initial_scenario;
    data.active_scale = initial_scale;
    data.toggles.reserve(std::size(kPanelToggles));
    for (const char* n : kPanelToggles) data.toggles.push_back(ToggleRow{n, false});

    // ── Telemetry ring buffer + the <ftd-chart> instancer ──────────────────────
    // The app owns the series (GUI thread), pushing one total-energy scalar per
    // published snapshot below; the custom element reads it read-only. Declared
    // here so both outlive Rml::Shutdown() (which releases the instanced elements
    // back through the instancer). Registered after Rml::Initialise(), before
    // LoadDocument parses <ftd-chart>. The engine telemetry scheduler is inert
    // (demand mask 0), so this app-side buffer is the only source.
    ftd::native::ui::ChartSeries energy_series(240);
    ftd::native::ui::FtdChartInstancer chart_instancer(&energy_series);
    Rml::Factory::RegisterElementInstancer("ftd-chart", &chart_instancer);

    AppContext app;
    app.hwnd = hwnd;
    app.context = nullptr;  // published after LoadDocument
    app.presenter = &presenter;
    app.camera = &camera;
    app.commands = &commands;
    app.data = &data;
    app.paused = &paused;
    app.quit = &quit_flag;
    app.scenario_id = initial_scenario;

    Rml::DataModelConstructor ctor = context->CreateDataModel("shell");
    if (!ctor) throw std::runtime_error("CreateDataModel(shell) failed");
    if (auto row = ctor.RegisterStruct<ToggleRow>()) {
        row.RegisterMember("name", &ToggleRow::name);
        row.RegisterMember("on", &ToggleRow::on);
    }
    ctor.RegisterArray<Rml::Vector<ToggleRow>>();
    ctor.Bind("tick", &data.tick);
    ctor.Bind("active_scale", &data.active_scale);
    ctor.Bind("particle_count", &data.particle_count);
    ctor.Bind("physical_time", &data.physical_time);
    ctor.Bind("total_energy", &data.total_energy);
    ctor.Bind("s1_ke", &data.s1_ke);
    ctor.Bind("s1_pe", &data.s1_pe);
    ctor.Bind("scenario", &data.scenario);
    ctor.Bind("backend", &data.backend);
    ctor.Bind("lattice", &data.lattice);
    ctor.Bind("fps", &data.fps);
    ctor.Bind("running", &data.running);
    ctor.Bind("toggles", &data.toggles);
    ctor.BindEventCallback("run", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_play_toggle(&app);
    });
    ctor.BindEventCallback("pause", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_pause(&app);
    });
    ctor.BindEventCallback("step", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_step(&app);
    });
    ctor.BindEventCallback("reset", [&app](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) {
        request_reset(&app);
    });
    ctor.BindEventCallback("toggle", [&app](Rml::DataModelHandle, Rml::Event&,
                                            const Rml::VariantList& v) {
        if (!v.empty()) request_toggle(&app, v[0].Get<Rml::String>());
    });
    ctor.BindEventCallback("scale_lattice", [&app](Rml::DataModelHandle, Rml::Event&,
                                                   const Rml::VariantList&) {
        request_switch_scale(&app, 0);
    });
    ctor.BindEventCallback("scale_particles", [&app](Rml::DataModelHandle, Rml::Event&,
                                                     const Rml::VariantList&) {
        request_switch_scale(&app, 1);
    });
    Rml::DataModelHandle model = ctor.GetModelHandle();

    Rml::ElementDocument* doc = context->LoadDocument(FTD_RML_SHELL_PATH);
    if (!doc) throw std::runtime_error("LoadDocument(shell.rml) failed");
    doc->Show();

    // Publish the RmlUi context + overlay into the presenter's frame path.
    app.context = context;
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
    RmlOverlay overlay(&rml_renderer, &context);
    presenter.set_overlay_recorder(&overlay);

    // ── CUDA↔D3D12 zero-copy interop (Scale-0 GPU particle path) ────────────────
    // The presenter owns a D3D12_HEAP_FLAG_SHARED particle buffer + a
    // D3D12_FENCE_FLAG_SHARED cross-API fence; CUDA imports both and the interop
    // gather writes device-resident particles straight into that buffer. Lifecycle
    // mirrors the native_desktop reference: create + import once and keep the NT
    // handles open for the whole process, so every post-reload re-import targets
    // the SAME D3D12 resources (the presenter and its shared resources are never
    // recreated by a reload — only the CUDA GpuEngine is). Interop is Scale-0-only:
    // Scale 1's ParticleEngine has no device buffer and its adapter's interop
    // methods are no-ops, so try_enable_interop() there returns false and the CPU
    // sprite path is used.
    //
    // Handles are created whenever the backend is GPU (regardless of the INITIAL
    // scale) so that a later switch INTO Scale 0 can re-import them; the CUDA
    // import (try_enable_interop) only actually succeeds while Scale 0 is active.
    std::atomic<bool> interop_active{false};
    HANDLE interop_buf_handle = nullptr;
    HANDLE interop_fence_handle = nullptr;
    std::uint64_t interop_buffer_bytes = 0;
    if (!engine_opts.force_cpu) {
        interop_buf_handle =
            presenter.create_shared_particle_buffer(ftd::kMaxVisualParticleCapture);
        interop_fence_handle =
            interop_buf_handle ? presenter.create_shared_fence() : nullptr;
        if (interop_buf_handle && interop_fence_handle) {
            interop_buffer_bytes = presenter.shared_particle_buffer_bytes();
            interop_active.store(host.try_enable_interop(
                interop_buf_handle, interop_buffer_bytes, interop_fence_handle));
        }
        std::cout << "interop: "
                  << (interop_active.load()
                          ? "enabled (Scale-0 device-resident particles)"
                          : "unavailable, using CPU particle path")
                  << "\n" << std::flush;
    }

    int camera_lattice = host.lattice_size();

    // ── Sim thread ──────────────────────────────────────────────────────────────
    std::mutex frame_mu;
    // Interop poll result produced EXCLUSIVELY on the sim thread (the only thread
    // allowed to touch host/bridge/GpuEngine — a reload can tear the GpuEngine down
    // mid-flight with no locking) and consumed on the GUI thread under frame_mu,
    // paired with `latest`. -1 = "no gather ready this round" / interop inactive.
    int latest_interop_count = -1;
    std::uint64_t latest_interop_fence = 0;
    std::thread sim([&] {
        // Sim-thread-local fence bookkeeping. interop_fence_counter is the strictly
        // increasing value each gather is signaled under; it is NEVER reset across
        // reloads, because the D3D12-side shared fence object survives every reload
        // (only the CUDA GpuEngine is rebuilt) and keeps its completed value — a
        // reset would make the first post-reload gather signal an already-passed
        // value, which cudaSignalExternalSemaphoresAsync rejects and a D3D12
        // queue->Wait would treat as already-satisfied (defeating the sync).
        // pending_interop_fence remembers which value the most recently REQUESTED
        // gather used, so a polled count is always paired with the exact fence value
        // that produced it.
        std::uint64_t interop_fence_counter = 0;
        std::uint64_t pending_interop_fence = 0;
        while (running.load()) {
            const auto start = std::chrono::steady_clock::now();
            try {
                const auto loop = host.loop_control();
                const bool need_work = loop.pending_steps > 0 || !loop.pause;
                if (need_work) {
                    const auto r = host.tick_once();
                    if (loop.pending_steps > 0) host.consume_pending_step();
                    if (!r.ok) throw std::runtime_error(r.message);
                }
                host.process_ui_boundary(commands);
                paused.store(host.loop_control().pause);

                // A reload / scale-switch rebuilt the active engine. Re-establish
                // interop iff Scale 0 is active and the shared handles exist. Two
                // paths converge here: a same-scale Scale-0 reload clears the
                // adapter's interop (fresh GpuEngine) and ScaleHost reports
                // ReloadStatus::InteropReimportRequired; a switch BACK to Scale 0
                // reports Success on a fresh adapter (was_interop == false). Both
                // need the identical re-import, and a switch to Scale 1 (no device
                // buffer) simply leaves interop off. import_d3d12_* are documented
                // safe to call more than once, so re-importing the same handles into
                // the fresh GpuEngine is within contract.
                if (host.applied_reload()) {
                    bool now_active = false;
                    if (interop_buf_handle && interop_fence_handle
                        && host.active_scale() == 0) {
                        now_active = host.try_enable_interop(
                            interop_buf_handle, interop_buffer_bytes,
                            interop_fence_handle);
                    }
                    interop_active.store(now_active);
                }

                if (need_work || host.applied_reload() || host.applied_host_write()) {
                    // Poll the PREVIOUS gather's count and request the NEXT gather —
                    // only ever on this thread. A thrown GPU error also lands in the
                    // catch below rather than unwinding past the still-joinable sim
                    // thread.
                    int polled_interop_count = -1;
                    std::uint64_t polled_interop_fence = 0;
                    if (interop_active.load()) {
                        polled_interop_count = host.poll_interop_particle_count();
                        polled_interop_fence = pending_interop_fence;
                        const std::uint64_t fv = ++interop_fence_counter;
                        if (host.request_interop_gather(fv)) {
                            pending_interop_fence = fv;
                        } else {
                            // The cross-API fence signal failed: the GUI thread's
                            // wait_shared_fence() would otherwise block forever on a
                            // value never signaled. Drop to the CPU particle path
                            // for the rest of the session (host.capture() below
                            // still supplies frame.particles).
                            interop_active.store(false);
                            std::cerr << "interop: gather/fence-signal failed "
                                         "mid-session, falling back to the CPU "
                                         "particle path\n" << std::flush;
                        }
                    }
                    ftd::native::NativeFrame next = host.capture();
                    std::lock_guard<std::mutex> lock(frame_mu);
                    latest = std::move(next);
                    latest_interop_count = polled_interop_count;
                    latest_interop_fence = polled_interop_fence;
                }
            } catch (const std::exception& ex) {
                std::lock_guard<std::mutex> lock(frame_mu);
                latest.status = ex.what();
            }
            const int hz = std::max(1, tick_hz.load());
            const auto budget = std::chrono::milliseconds(1000 / hz);
            const auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed < budget) std::this_thread::sleep_for(budget - elapsed);
        }
    });

    // ── GUI loop ──────────────────────────────────────────────────────────────
    LARGE_INTEGER qpc_freq{}, qpc_last{};
    QueryPerformanceFrequency(&qpc_freq);
    QueryPerformanceCounter(&qpc_last);
    double fps_accum = 0.0;
    int fps_frames = 0;
    int smoothed_fps = 0;

    ftd::native::CaptureToken capture_token{};
    bool capture_requested = false;
    bool capture_saved = false;
    bool capture_ok = false;
    int frame_no = 0;
    const std::string png_out = FTD_APP_PNG_OUT;

    // Telemetry ring-buffer feed bookkeeping (GUI thread). chart_series_scale
    // tracks the scale the buffer currently holds (reset the trace on a switch);
    // last_pushed_seq dedups pushes to one scalar per published snapshot.
    int chart_series_scale = initial_scale;
    std::uint64_t last_pushed_seq = 0;
    bool pushed_any = false;

    // The interop StructuredBuffer SRV (heap slot 0) only needs binding ONCE for
    // the lifetime of the never-recreated shared buffer. This catch-up covers both
    // the startup-active case and a later inactive→active reload transition; a
    // D3D12 presenter call must stay on this (GUI) thread, so it lives here rather
    // than in the sim-thread reload block above.
    bool interop_srv_bound = false;

    const auto loop_start = std::chrono::steady_clock::now();
    bool quit = false;
    MSG msg{};
    while (!quit) {
        if (capture_mode) {
            if (frame_no % 30 == 0)
                std::cerr << "[frame " << frame_no << "] tick=" << data.tick
                          << " particles=" << data.particle_count << "\n" << std::flush;
            if (std::chrono::steady_clock::now() - loop_start > std::chrono::seconds(40)) {
                std::cerr << "capture: deadline exceeded at frame " << frame_no << "\n" << std::flush;
                break;
            }
        }
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) quit = true;
            if (msg.message == WM_KEYDOWN) {
                if (msg.wParam == VK_ESCAPE) quit = true;
                else if (msg.wParam == VK_SPACE) request_play_toggle(&app);
                else if (msg.wParam == 'R') request_reset(&app);
                else if (msg.wParam == 'S') request_step(&app);
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (quit || quit_flag.load()) break;

        // Frame-polled resize (keeps swapchain resizes off the wnd_proc reentrancy
        // path). GetClientRect is physical pixels on a per-monitor-V2 window.
        GetClientRect(hwnd, &client);
        const std::uint32_t cw = static_cast<std::uint32_t>(std::max<LONG>(1, client.right));
        const std::uint32_t ch = static_cast<std::uint32_t>(std::max<LONG>(1, client.bottom));
        if (cw != presenter.width() || ch != presenter.height()) {
            presenter.wait_idle();
            presenter.resize(cw, ch);
            context->SetDimensions(Rml::Vector2i(static_cast<int>(cw), static_cast<int>(ch)));
        }
        const UINT dpi = GetDpiForWindow(hwnd);
        const float scale = dpi > 0 ? static_cast<float>(dpi) / 96.0f : 1.0f;
        if (scale != dpi_scale) {
            dpi_scale = scale;
            context->SetDensityIndependentPixelRatio(dpi_scale);
        }

        // Acquire published state. The interop count/fence come straight out of
        // the same frame_mu-protected snapshot as `frame` — populated by the sim
        // thread, so the fence value is exactly the one request_interop_gather()
        // used for the gather this count was polled from.
        ftd::native::NativeFrame frame;
        int this_frame_interop_count = -1;
        std::uint64_t this_frame_fence_value = 0;
        {
            std::lock_guard<std::mutex> lock(frame_mu);
            frame = latest;
            this_frame_interop_count = latest_interop_count;
            this_frame_fence_value = latest_interop_fence;
        }
        const std::uint32_t draw_interop_count =
            this_frame_interop_count > 0
                ? static_cast<std::uint32_t>(this_frame_interop_count)
                : 0u;
        if (capture_mode && frame_no % 30 == 0) {
            std::cerr << "[frame " << frame_no << "] interop="
                      << (interop_active.load() ? "on" : "off")
                      << " interop_count=" << this_frame_interop_count
                      << " draw_count=" << draw_interop_count << "\n" << std::flush;
        }
        std::shared_ptr<const ftd::native::HostSnapshot> snap = host.publisher().acquire();

        if (frame.lattice_size > 0 && frame.lattice_size != camera_lattice) {
            apply_camera_for_lattice(camera, frame.lattice_size);
            camera_lattice = frame.lattice_size;
        }

        // fps (smoothed over ~0.4 s).
        LARGE_INTEGER qpc_now{};
        QueryPerformanceCounter(&qpc_now);
        double dt = qpc_freq.QuadPart
                        ? double(qpc_now.QuadPart - qpc_last.QuadPart) / double(qpc_freq.QuadPart)
                        : 1.0 / 60.0;
        qpc_last = qpc_now;
        if (dt <= 0.0 || dt > 0.5) dt = 1.0 / 60.0;
        fps_accum += dt;
        ++fps_frames;
        if (fps_accum >= 0.4) {
            smoothed_fps = static_cast<int>(fps_frames / fps_accum + 0.5);
            fps_accum = 0.0;
            fps_frames = 0;
        }

        // ── Push snapshot → data model (dirty only what changed) ──
        auto set_int = [&](const char* name, int& dst, int val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };
        auto set_bool = [&](const char* name, bool& dst, bool val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };
        auto set_str = [&](const char* name, Rml::String& dst, const std::string& val) {
            if (dst != val) { dst = val; model.DirtyVariable(name); }
        };

        set_int("tick", data.tick, frame.tick);
        if (snap) set_int("active_scale", data.active_scale, snap->active_scale);
        set_int("particle_count", data.particle_count,
                static_cast<int>(frame.total_manifested));
        set_bool("running", data.running, !paused.load());
        set_int("fps", data.fps, smoothed_fps);
        set_str("scenario", data.scenario, frame.scenario.empty() ? app.scenario_id
                                                                   : frame.scenario);
        set_str("backend", data.backend, upper(frame.backend.empty() ? host.backend_name()
                                                                      : frame.backend));
        set_str("lattice", data.lattice, std::to_string(frame.lattice_size));
        set_str("physical_time", data.physical_time,
                fmt("%.2e s", static_cast<double>(frame.tick) * kTPhysSeconds));
        // Energy + toggles come from whichever ScaleSnapshot alternative is live.
        // Scale 0 carries the full energy ledger + term-toggle state; Scale 1
        // carries a small particle-diagnostics payload feeding the readout panel.
        // Every deref is guarded so the wrong variant is never read after a switch.
        // `chart_energy` is the scalar the telemetry ring buffer plots this frame.
        double chart_energy = 0.0;
        bool chart_energy_valid = false;
        if (const ftd::native::Scale0Snapshot* s0 = snap ? snap->scale0() : nullptr) {
            chart_energy = s0->energy_ledger.E_curr;
            chart_energy_valid = true;
            set_str("total_energy", data.total_energy, fmt("%.1f", s0->energy_ledger.E_curr));
            bool toggles_changed = false;
            for (ToggleRow& r : data.toggles) {
                const bool on = toggle_on(s0->term_toggles, r.name.c_str());
                if (r.on != on) { r.on = on; toggles_changed = true; }
            }
            if (toggles_changed) model.DirtyVariable("toggles");
        } else if (const ftd::native::Scale1Snapshot* s1 = snap ? snap->scale1() : nullptr) {
            chart_energy = s1->total_energy;
            chart_energy_valid = true;
            set_str("total_energy", data.total_energy, fmt("%.3f", s1->total_energy));
            set_str("s1_ke", data.s1_ke, fmt("%.3f", s1->total_ke));
            set_str("s1_pe", data.s1_pe, fmt("%.3f", s1->total_pe));
        }

        // ── Feed the telemetry ring buffer (snapshot-only, GUI thread) ──
        // Reset on a scale switch so Scale-0 energy is never plotted next to
        // Scale-1 energy; push exactly one scalar per NEW published snapshot
        // (dedup by seq — the GUI loop runs faster than the sim tick).
        if (snap) {
            if (snap->active_scale != chart_series_scale) {
                energy_series.clear();
                chart_series_scale = snap->active_scale;
                last_pushed_seq = 0;
                pushed_any = false;
            }
            if (chart_energy_valid && (!pushed_any || snap->seq != last_pushed_seq)) {
                energy_series.push(static_cast<float>(chart_energy));
                last_pushed_seq = snap->seq;
                pushed_any = true;
            }
        }

        // Lay out, then map the #viewport hole rect for the scene + input.
        context->Update();
        if (Rml::Element* vp = doc->GetElementById("viewport")) {
            const Rml::Vector2f off = vp->GetAbsoluteOffset(Rml::BoxArea::Border);
            const int w = static_cast<int>(vp->GetOffsetWidth());
            const int h = static_cast<int>(vp->GetOffsetHeight());
            if (w > 0 && h > 0) {
                app.viewport_rect = {static_cast<int>(off.x), static_cast<int>(off.y),
                                     static_cast<std::uint32_t>(w),
                                     static_cast<std::uint32_t>(h)};
                presenter.set_scene_rect(app.viewport_rect);
            }
        }

        // ── Capture mode: after warmup, request + poll a composited readback ──
        // A capture is ARMED by exactly one render() after request_capture (that
        // frame records the copy into the readback buffer and signals the capture
        // fence). D3D12Presenter::render() re-arms the still-pending capture on
        // EVERY subsequent call — bumping the target fence value one ahead of what
        // the GPU has completed — so if we kept rendering while polling, poll would
        // never converge. Once armed, stop rendering and just poll until the fence
        // completes. (Pre-existing presenter behavior; the app owns the arm-once
        // discipline.)
        const bool capture_armed = capture_mode && capture_requested;
        if (capture_mode && !capture_saved) {
            // When interop is active, hold off arming until a device-resident
            // gather has actually landed (draw_interop_count > 0), so the captured
            // frame renders the interop particles rather than the CPU fallback that
            // shows before the first gather completes. The 40 s deadline above is
            // the safety net if a scenario never manifests any particles.
            const bool interop_ready_or_na =
                !interop_active.load() || draw_interop_count > 0;
            if (!capture_requested && frame_no >= app_opts.capture_frames
                && interop_ready_or_na) {
                capture_token = presenter.request_capture(ftd::native::CaptureRegion::FullWindow);
                capture_requested = true;
            }
            if (capture_requested) {
                ftd::native::CaptureResult res = presenter.poll_capture(capture_token);
                if (res.status == ftd::native::CaptureStatus::Ready) {
                    const bool ok = save_png(widen(png_out), res.bytes.data(), res.width,
                                             res.height, res.row_pitch);
                    std::cout << "capture: " << (ok ? "wrote " : "FAILED ") << png_out
                              << " (" << res.width << "x" << res.height
                              << ", tick=" << data.tick << ", particles=" << data.particle_count
                              << ", interop=" << (interop_active.load() ? "on" : "off")
                              << ", interop_count=" << this_frame_interop_count
                              << ", energy=" << data.total_energy.c_str() << ")\n";
                    // The readback already completed (poll returned Ready) and the
                    // PNG is committed to disk. Clean-exit attempt: mark the capture
                    // done and fall through to the normal run_app teardown (sim join,
                    // GPU idle, RmlUi shutdown, D3D12 release) + a normal return,
                    // rather than hard-terminating here.
                    std::cout.flush();
                    std::cerr.flush();
                    capture_ok = ok;
                    capture_saved = true;
                    running.store(false);
                    quit = true;
                    break;
                } else if (res.status == ftd::native::CaptureStatus::Failed) {
                    std::cerr << "capture: failed: " << res.error << "\n" << std::flush;
                    capture_saved = true;
                    quit = true;
                }
            }
        }

        // Skip render only AFTER the capture is armed (armed on a prior frame),
        // so the presenter does not re-arm the pending readback. The arming frame
        // itself (capture_armed still false here) renders normally.
        if (!capture_armed) {
            try {
                // Bind the interop SRV once (idempotent, GUI-thread-only D3D12
                // call), then make the render queue wait until CUDA has signaled
                // the shared fence for the gather whose count we are about to draw.
                if (interop_active.load() && !interop_srv_bound) {
                    presenter.bind_interop_particle_srv();
                    interop_srv_bound = true;
                }
                if (draw_interop_count != 0) {
                    presenter.wait_shared_fence(this_frame_fence_value);
                }
                presenter.render(frame, camera, view_opts, draw_interop_count);
            } catch (const std::exception& ex) {
                std::cerr << "render threw at frame " << frame_no << ": " << ex.what() << "\n"
                          << std::flush;
                running.store(false);
                sim.join();
                throw;
            }
        }
        ++frame_no;

        // Interactive: yield a little so the sim thread gets CPU; capture mode
        // spins to reach the readback fast.
        if (!capture_mode) std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // ── Teardown ────────────────────────────────────────────────────────────────
    running.store(false);
    sim.join();
    presenter.set_overlay_recorder(nullptr);
    app.context = nullptr;
    context = nullptr;  // stop the overlay from re-entering RmlUi during shutdown
    presenter.wait_idle();
    // The sim thread (the only thread that re-imports these after startup) is
    // joined and the GPU is idle, so nothing on either API still references the
    // shared buffer/fence. CUDA imported (did not take ownership of) these NT
    // handles, so closing them here frees the handle without touching the
    // still-live CUDA external objects. (In --capture-frames mode the process is
    // hard-terminated before reaching here; the OS reclaims the handles.)
    if (interop_fence_handle) CloseHandle(interop_fence_handle);
    if (interop_buf_handle) CloseHandle(interop_buf_handle);
    Rml::Shutdown();  // renderer (declared after presenter) still alive here

    if (capture_mode) return (capture_saved && capture_ok) ? 0 : 2;
    return 0;
}

}  // namespace

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    attach_parent_console_if_any();
    HRESULT co = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    int rc = 1;
    try {
        rc = run_app(utf8_args());
    } catch (const std::exception& ex) {
        std::cerr << "native_app: " << ex.what() << "\n";
        MessageBoxA(nullptr, ex.what(), "FTD Native App", MB_ICONERROR);
        rc = 1;
    }
    if (SUCCEEDED(co)) CoUninitialize();
    // Clean-exit attempt (see run_app teardown): all meaningful teardown has run
    // above (GPU idle, RmlUi shut down, the D3D12 device + swapchain released as
    // run_app's locals unwound). Return normally and let the CRT exit path run.
    std::cout.flush();
    std::cerr.flush();
    return rc;
}
