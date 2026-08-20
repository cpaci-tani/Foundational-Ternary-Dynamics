#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>

#ifndef _WIN64
#error "ftd_native is a Win64 (x86-64) target"
#endif

#include "ftd/scenarios.h"
#include "ftd/visual_snapshot.h"
#include "native/cli_options.h"
#include "native/d3d12_presenter.h"
#include "native/dpi_support.h"
#include "native/engine_session.h"
#include "native/command_queue.h"
#include "native/imgui_overlay.h"
#include "native/scene_rect.h"
#include "native/ui_command.h"
#include "native/ui_result.h"
#include "ui/ui_shell.h"
#include "ui/theme.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

struct AppState {
    HWND hwnd = nullptr;

    ftd::native::D3D12Presenter* presenter = nullptr;
    ftd::native::ImGuiOverlay* overlay = nullptr;
    ftd::native::UiShell* shell = nullptr;
    ftd::native::Camera camera;
    ftd::native::NativeViewOptions view_opts;
    ftd::native::NativeEngineOptions live_opts;
    ftd::native::CommandQueue* commands = nullptr;
    std::atomic<bool>* paused = nullptr;
    std::atomic<int>* tick_hz = nullptr;
    std::atomic<bool>* reloading = nullptr;
    // Set once during main()'s setup (running right after it's declared,
    // sim right after the sim thread is constructed) and never reassigned
    // afterward, so reading these pointers from wnd_proc -- which runs on
    // this same GUI/message-loop thread, never concurrently with the writes
    // -- is race-free. See stop_sim_and_rethrow()'s doc comment for why
    // wnd_proc needs them at all: any D3D12Presenter call reachable from a
    // window message (e.g. resize() from WM_SIZE) can throw, and that
    // exception unwinds straight past the still-joinable `sim` thread
    // object unless it is stopped and joined first.
    std::atomic<bool>* running = nullptr;
    std::thread* sim = nullptr;

    bool dragging = false;
    POINT last{};
};

AppState* app_from_hwnd(HWND hwnd) {
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

void push_loop_command(AppState* app, ftd::native::UiCommand command) {
    if (!app || !app->commands) return;
    app->commands->push(std::move(command));
}

void request_play_toggle(AppState* app) {
    if (!app || !app->paused) return;
    if (app->paused->load()) {
        push_loop_command(app, ftd::native::Run{});
        app->paused->store(false);
    } else {
        push_loop_command(app, ftd::native::Pause{});
        app->paused->store(true);
    }
}

void request_step(AppState* app) {
    push_loop_command(app, ftd::native::Pause{});
    push_loop_command(app, ftd::native::Step{1});
    if (app->paused) app->paused->store(true);
}

void request_reset(AppState* app) {
    if (!app) return;
    const std::string id = app->live_opts.scenario;
    if (!id.empty()) push_loop_command(app, ftd::native::LoadScenario{id});
}

void apply_camera_for_lattice(AppState* app, int lattice_size) {
    const float center = static_cast<float>(lattice_size) * 0.5f;
    app->camera.target_x = center;
    app->camera.target_y = center;
    app->camera.target_z = center;
    app->camera.distance = static_cast<float>(lattice_size) * 1.8f;
}

void layout_full_client(AppState* app, int width, int height) {
    if (!app || !app->presenter) return;
    if (app->shell) return;
    app->presenter->set_scene_rect(
        {0, 0, static_cast<std::uint32_t>(std::max(1, width)),
         static_cast<std::uint32_t>(std::max(1, height))});
}

// Must be called from directly inside a `catch (...)` block (it ends with a
// bare `throw;`, which rethrows "the currently handled exception" and is
// only well-defined within the dynamic extent of a handler). Stops the sim
// thread and joins it -- if it exists and is still joinable -- before
// rethrowing, so that whatever unwinds past the now-defunct `sim`
// std::thread object finds it already joined. std::thread's destructor
// calls std::terminate() on a still-joinable thread, and every D3D12 call
// reachable from this GUI/message-loop thread (resize() from WM_SIZE,
// wait_shared_fence()/render() from the per-frame draw in main()) funnels
// failures through throw_if_failed() as std::runtime_error for realistic
// GPU-app failure modes: device-removed/TDR, adapter loss on sleep-resume
// or a monitor/DPI change, CreateCommittedResource running out of memory,
// etc. Centralizing the join-before-rethrow dance here keeps every
// GUI-thread D3D12 call site consistent instead of re-deriving it per call
// site (and forgetting one, as WM_SIZE's resize() call once did).
//
// `sim` may be null (main() hasn't constructed the sim thread yet, e.g.
// during initial window/view creation) or non-null but already joined by
// an earlier call to this same function further down the same unwind --
// both are handled by the joinable() check, so calling this more than once
// per exception is safe. `running` may also be null defensively, though in
// practice both call sites always pass a valid pointer.
//
// Note: std::thread::join() can itself throw std::system_error (e.g.
// resource_deadlock_would_occur) if `sim` is not in a joinable state that
// join() accepts. That's not expected to happen at either call site --
// `sim` is never joined anywhere else before this runs -- but if this
// pattern is ever copy-pasted to a call site where that invariant doesn't
// hold, a join() failure here would replace the original exception's
// message with a generic system_error before it reaches main()'s
// MessageBoxA, silently losing the diagnostic this whole mechanism exists
// to preserve.
[[noreturn]] void stop_sim_and_rethrow(std::atomic<bool>* running, std::thread* sim) {
    if (running) running->store(false);
    if (sim && sim->joinable()) sim->join();
    throw;
}

ftd::native::SceneRect live_scene_rect(AppState* app) {
    if (!app || !app->presenter || !app->hwnd) {
        return {};
    }
    RECT client{};
    GetClientRect(app->hwnd, &client);
    return ftd::native::scene_rect_clamped_to(
        app->presenter->scene_rect(),
        static_cast<std::uint32_t>(std::max(0L, client.right)),
        static_cast<std::uint32_t>(std::max(0L, client.bottom)));
}

void handle_scene_mouse(AppState* app, HWND hwnd, UINT msg, WPARAM wparam,
                        LPARAM lparam) {
    if (!app) return;
    const bool imgui_mouse = app->overlay && app->overlay->want_capture_mouse();
    const auto scene = live_scene_rect(app);

    auto client_point = [&]() -> POINT {
        POINT pt{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        if (msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL) {
            ScreenToClient(hwnd, &pt);
        }
        return pt;
    };

    switch (msg) {
        case WM_LBUTTONDOWN: {
            const POINT pt = client_point();
            if (!ftd::native::scene_accepts_pointer(scene, pt.x, pt.y,
                                                            imgui_mouse)) {
                break;
            }
            app->dragging = true;
            app->last = pt;
            SetCapture(hwnd);
            break;
        }
        case WM_LBUTTONUP:
            app->dragging = false;
            if (GetCapture() == hwnd) ReleaseCapture();
            break;
        case WM_CAPTURECHANGED:
        case WM_KILLFOCUS:
            app->dragging = false;
            break;
        case WM_MOUSEMOVE: {
            if (!app->dragging) break;
            if (imgui_mouse) {
                app->dragging = false;
                if (GetCapture() == hwnd) ReleaseCapture();
                break;
            }
            const POINT pt = client_point();
            app->camera.yaw += (pt.x - app->last.x) * 0.01f;
            app->camera.pitch += (pt.y - app->last.y) * 0.01f;
            if (app->camera.pitch > 1.4f) app->camera.pitch = 1.4f;
            if (app->camera.pitch < -1.4f) app->camera.pitch = -1.4f;
            app->last = pt;
            break;
        }
        case WM_MOUSEWHEEL: {
            const POINT pt = client_point();
            if (!ftd::native::scene_accepts_pointer(scene, pt.x, pt.y,
                                                            imgui_mouse)) {
                break;
            }
            const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            app->camera.distance *= (delta > 0) ? 0.9f : 1.1f;
            if (app->camera.distance < 4.0f) app->camera.distance = 4.0f;
            if (app->camera.distance > 256.0f) app->camera.distance = 256.0f;
            break;
        }
        default:
            break;
    }
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppState* app = app_from_hwnd(hwnd);
    if (app && app->overlay) {
        if (ftd::native::ImGuiOverlay::wnd_proc_handler(
                hwnd, msg, wparam, lparam)) {
            return 0;
        }
    }
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            ftd::native::apply_dpi_suggested_rect(hwnd, lparam);
            if (app && app->presenter && app->overlay) {
                try {
                    app->presenter->wait_idle();
                    const UINT dpi = GetDpiForWindow(hwnd);
                    const float scale = dpi > 0 ? static_cast<float>(dpi) / 96.0f : 1.0f;
                    app->overlay->rebuild_fonts(scale);
                    if (app->shell) app->shell->set_dpi_scale(scale);
                } catch (...) {
                    stop_sim_and_rethrow(app->running, app->sim);
                }
            }
            return 0;
        case WM_SIZE:
            if (app && app->presenter && wparam != SIZE_MINIMIZED) {
                const UINT w = LOWORD(lparam);
                const UINT h = HIWORD(lparam);
                if (w > 0 && h > 0) {
                    try {
                        app->presenter->resize(w, h);
                        layout_full_client(app, static_cast<int>(w),
                                           static_cast<int>(h));
                    } catch (...) {
                        stop_sim_and_rethrow(app->running, app->sim);
                    }
                }
            } else if (app) {
                layout_full_client(app, LOWORD(lparam), HIWORD(lparam));
            }
            return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
        case WM_MOUSEWHEEL:
        case WM_CAPTURECHANGED:
        case WM_KILLFOCUS:
            handle_scene_mouse(app, hwnd, msg, wparam, lparam);
            if (msg == WM_KILLFOCUS) {
                return DefWindowProcW(hwnd, msg, wparam, lparam);
            }
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

constexpr char kHelpText[] =
    "ftd_native [--cpu|--gpu] [--no-ui] [--lattice N] [--scenario name]\n"
    "  Defaults: GPU, paused, --lattice 32 --scenario s0-seed-hydrogen\n"
    "  --cpu forces the CPU backend; --gpu is the default\n"
    "  --no-ui skips ImGui and the dockspace shell (bisection tool; lattice still renders)\n"
    "  Docks: Setup (stacked: scenarios / run / substrate) · Instruments · Physics\n"
    "  View: left-drag orbit, wheel zoom, Space play/pause, Esc quit\n";

bool bind_crt_to_std_handle(DWORD std_id, int crt_fd) {
    HANDLE handle = GetStdHandle(std_id);
    if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    const DWORD type = GetFileType(handle);
    if (type != FILE_TYPE_DISK && type != FILE_TYPE_PIPE && type != FILE_TYPE_CHAR) {
        return false;
    }
    const int fd = _open_osfhandle(reinterpret_cast<intptr_t>(handle), _O_TEXT);
    if (fd < 0) {
        return false;
    }
    _dup2(fd, crt_fd);
    return true;
}

void attach_parent_console_if_any() {
    const bool out_bound = bind_crt_to_std_handle(STD_OUTPUT_HANDLE, 1);
    bind_crt_to_std_handle(STD_ERROR_HANDLE, 2);
    if (out_bound) {
        std::cout.clear();
        std::cerr.clear();
        return;
    }
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        return;
    }
    FILE* stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stdout);
    stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stderr);
    std::cout.clear();
    std::cerr.clear();
}

std::string wide_to_utf8(const wchar_t* wide) {
    if (wide == nullptr || wide[0] == L'\0') {
        return {};
    }
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (bytes <= 1) {
        return {};
    }
    std::string out(static_cast<size_t>(bytes - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, out.data(), bytes, nullptr, nullptr);
    return out;
}

std::vector<std::string> utf8_args_from_command_line() {
    int argc = 0;
    LPWSTR* wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (wargv == nullptr) {
        return {"ftd_native"};
    }
    std::vector<std::string> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.push_back(wide_to_utf8(wargv[i]));
    }
    LocalFree(wargv);
    if (args.empty()) {
        args.emplace_back("ftd_native");
    }
    return args;
}

void show_help() {
    std::cout << kHelpText << std::flush;
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    const DWORD type = (out && out != INVALID_HANDLE_VALUE) ? GetFileType(out) : FILE_TYPE_UNKNOWN;
    const bool writable = type == FILE_TYPE_DISK || type == FILE_TYPE_PIPE || type == FILE_TYPE_CHAR;
    if (!writable && GetConsoleWindow() == nullptr) {
        MessageBoxA(nullptr, kHelpText, "FTD Native Desktop", MB_OK | MB_ICONINFORMATION);
    }
}

}  // namespace

int run_native(int argc, char** argv) {
    try {
        if (!ftd::native::enable_per_monitor_v2_dpi()) {
            throw std::runtime_error("Per-monitor-V2 DPI awareness unavailable");
        }

        const auto parsed = ftd::native::parse_native_cli(argc, argv);
        if (parsed.help) {
            show_help();
            return 0;
        }
        auto options = parsed.options;
        std::cout << "FTD native desktop (in-process, not WebView2)\n";
        if (options.no_ui) {
            std::cout << "no-ui: ImGui init skipped\n";
        }
        std::cout << "Loading L=" << options.lattice_size
                  << " scenario=" << options.scenario
                  << (options.force_cpu ? " cpu" : " gpu-default") << "...\n"
                  << std::flush;

        ftd::native::NativeEngineSession session(options);
        std::cout << "backend=" << session.backend_name()
                  << " status=" << session.status() << "\n"
                  << std::flush;

        ftd::native::CommandQueue ui_commands;
        std::atomic<bool> running{true};
        std::atomic<bool> paused{true};
        std::atomic<int> tick_hz{20};
        std::atomic<bool> reloading{false};

        AppState app;
        app.live_opts = session.options();
        app.commands = &ui_commands;
        app.paused = &paused;
        app.tick_hz = &tick_hz;
        app.reloading = &reloading;
        app.running = &running;
        apply_camera_for_lattice(&app, session.lattice_size());

        WNDCLASSW wc{};
        wc.lpfnWndProc = wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"FtdNativeDesktop";
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        RegisterClassW(&wc);

        HWND hwnd = CreateWindowExW(
            0, wc.lpszClassName, L"FTD Native Desktop",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
            CW_USEDEFAULT, 1600, 900, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hwnd) throw std::runtime_error("CreateWindowExW failed");
        app.hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));

        RECT client{};
        GetClientRect(hwnd, &client);

        ftd::native::D3D12Presenter presenter;
        presenter.initialize(hwnd, static_cast<std::uint32_t>(client.right),
                             static_cast<std::uint32_t>(client.bottom));
        app.presenter = &presenter;
        layout_full_client(&app, client.right, client.bottom);

        ftd::native::ImGuiOverlay overlay;
        std::string ui_dir;
        if (const char* appdata = std::getenv("APPDATA")) {
            ui_dir = std::string(appdata) + "\\FTD\\native";
        }
        std::unique_ptr<ftd::native::UiShell> shell;
        if (!options.no_ui) {
            if (!overlay.initialize(hwnd, presenter.ui_backend_context())) {
                throw std::runtime_error("ImGui overlay initialize failed");
            }
            presenter.set_overlay_recorder(&overlay);
            app.overlay = &overlay;
            const UINT dpi = GetDpiForWindow(hwnd);
            const float scale = dpi > 0 ? static_cast<float>(dpi) / 96.0f : 1.0f;
            shell = std::make_unique<ftd::native::UiShell>(ui_dir);
            shell->set_dpi_scale(scale);
            app.shell = shell.get();
        }

        // std::atomic, not a plain bool: try_enable_interop() below runs
        // once here on the main thread before the sim thread exists, but
        // NativeEngineSession::boot() clears interop_enabled_ on every
        // reload (see engine_session.cpp), and the sim thread mirrors that
        // back into this flag after any reload -- so after startup this is
        // written only from the sim thread (see the reload branch inside the
        // sim lambda below). The GUI/message-loop thread never reads this
        // flag directly (draw_interop_count gating instead comes out of the
        // frame_mu-protected latest_interop_count/latest_interop_fence
        // snapshot below, produced by the sim thread's own read of this
        // flag) -- it stays atomic purely because the main thread's one-time
        // write above and the sim thread's later writes are two different
        // OS threads touching the same variable, not because of any
        // ongoing cross-thread read here.
        std::atomic<bool> interop_active{false};
        // Interop Task 12: kept open for the whole process lifetime (NOT
        // CloseHandle'd right after the startup import below) so every later
        // reload can re-import the SAME underlying D3D12 buffer/fence into
        // the freshly constructed GpuEngine boot() produces -- see the
        // do_reload branch inside the sim lambda below.
        // D3D12Presenter/`presenter` -- and with it these shared resources
        // and their SRV binding (bind_interop_particle_srv(), called once
        // below) -- is never destroyed or recreated across a reload; only
        // NativeEngineSession's internal bridge_/GpuEngine is. Reusing the
        // same NT handles across multiple
        // sequential imports is within contract: neither
        // import_d3d12_particle_buffer() nor import_d3d12_fence() takes
        // ownership of the handle it's given (see gpu_engine.h), so nothing
        // about closing them "once done" was ever load-bearing beyond
        // freeing the handle slot -- and since the old GpuEngine each
        // reload replaces is fully destroyed first (boot() resets bridge_
        // before reconstructing it), each re-import targets a completely
        // fresh CUDA-side external-memory/-semaphore object with no
        // dangling state from the previous one to worry about. Closed once,
        // together, near the very end of main() after the sim thread has
        // been joined.
        HANDLE interop_buf_handle = nullptr;
        HANDLE interop_fence_handle = nullptr;
        std::uint64_t interop_buffer_bytes = 0;
        if (!options.force_cpu) {
            interop_buf_handle =
                presenter.create_shared_particle_buffer(ftd::kMaxVisualParticleCapture);
            interop_fence_handle =
                interop_buf_handle ? presenter.create_shared_fence() : nullptr;
            if (interop_buf_handle && interop_fence_handle) {
                interop_buffer_bytes = presenter.shared_particle_buffer_bytes();
                const bool enabled = session.try_enable_interop(
                    interop_buf_handle, interop_buffer_bytes, interop_fence_handle);
                interop_active.store(enabled);
                if (enabled) presenter.bind_interop_particle_srv();
            }
            std::cout << "interop: "
                      << (interop_active.load() ? "enabled" : "unavailable, using CPU path")
                      << "\n" << std::flush;
        }

        std::mutex frame_mu;
        ftd::native::NativeFrame latest = session.capture();
        {
            ftd::native::CommandQueue stamp;
            session.process_ui_boundary(stamp);
        }
        int camera_lattice = session.lattice_size();
        // Interop poll result, produced exclusively on the sim thread (the
        // only thread allowed to touch `session`/`bridge_` -- see the sim
        // lambda below) and consumed on the GUI/message-loop thread under
        // frame_mu, same pattern as `latest`. -1 means "not ready this
        // round" or "interop inactive", matching
        // NativeEngineSession::poll_interop_particle_count()'s own contract.
        int latest_interop_count = -1;
        std::uint64_t latest_interop_fence = 0;

        if (app.overlay && app.shell) {
            app.overlay->begin_frame(1.0f / 60.0f);
            ftd::native::ViewChrome chrome;
            chrome.particles = &app.view_opts.particles;
            chrome.flux = &app.view_opts.flux;
            chrome.lattice_box = &app.view_opts.lattice_box;
            chrome.tick_hz = app.tick_hz;
            chrome.paused = app.paused;
            chrome.interop_active = interop_active.load();
            auto snap = session.snapshot_publisher().acquire();
            ftd::native::UiSnapshot fallback;
            fallback.frame = latest;
            const ftd::native::UiSnapshot& ui_snap = snap ? *snap : fallback;
            app.shell->draw(ui_snap, ui_commands, chrome);
            presenter.set_scene_rect(app.shell->scene_rect());
            app.overlay->end_frame();
        }

        std::thread sim([&] {
            // Both sim-thread-local only -- nothing else reads or writes
            // either counter, so a plain (non-atomic) std::uint64_t is
            // correct here. interop_fence_counter is the strictly
            // increasing value handed to request_interop_gather()/
            // interop_signal_fence() each time a gather is requested;
            // pending_interop_fence remembers which of those values the
            // most recently REQUESTED (possibly still in-flight) gather
            // used, so it can pair a polled particle count with the exact
            // fence value that gather was signaled under -- the snapshot
            // handed to the GUI thread below must never mix one gather's
            // count with another gather's fence value.
            //
            // Interop Task 12: deliberately NEVER reset on a reload, even
            // though interop_active does flip false then (possibly) true
            // again across the do_reload branch below. The D3D12-side
            // shared fence these values are eventually signaled against
            // (interop_fence_handle, imported via import_d3d12_fence() each
            // time try_enable_interop() runs) is the SAME ID3D12Fence
            // object before and after a reload -- D3D12Presenter and the
            // shared resources it owns are never destroyed/recreated by a
            // reload, only NativeEngineSession's internal bridge_/GpuEngine
            // is -- so its completed value keeps whatever it reached before
            // the reload. Resetting interop_fence_counter to 0 here would
            // make the first post-reload request_interop_gather() try to
            // signal a value the fence has already passed:
            // cudaSignalExternalSemaphoresAsync() documents that failing
            // outright for a non-monotonic value (see
            // GpuEngine::interop_signal_fence()'s doc comment in
            // gpu_engine.h), and even if it didn't, a D3D12
            // queue->Wait(value) for an already-passed value returns
            // immediately without actually waiting -- silently defeating
            // the cross-API synchronization wait_shared_fence() exists to
            // provide, and reintroducing exactly the "read the buffer
            // before the gather that fills it has finished" race that
            // synchronization is there to prevent. So both counters simply
            // keep counting up across arbitrarily many reloads instead.
            std::uint64_t interop_fence_counter = 0;
            std::uint64_t pending_interop_fence = 0;
            while (running.load()) {
                const auto start = std::chrono::steady_clock::now();
                try {
                    const auto loop = session.loop_control();
                    bool need_work = loop.pending_steps > 0 || !loop.pause;
                    if (need_work) {
                        if (session.loop_control().pending_steps > 0) {
                            const auto tick_result = session.tick_once();
                            session.consume_pending_step();
                            if (!tick_result.ok) throw std::runtime_error(tick_result.message);
                        } else if (!session.loop_control().pause) {
                            const auto tick_result = session.tick_once();
                            if (!tick_result.ok) throw std::runtime_error(tick_result.message);
                        }
                    }
                    session.process_ui_boundary(ui_commands);
                    paused.store(session.loop_control().pause);
                    if (session.last_reload_result().status
                        == ftd::native::ReloadStatus::InteropReimportRequired) {
                        const bool was_active = interop_active.load();
                        // boot() (invoked by LoadScenario / ApplyReboot) always
                        // clears the session's interop_enabled_ -- it
                        // tears down bridge_/GpuEngine and constructs a
                        // fresh one, and nothing has imported into that
                        // fresh GpuEngine yet. reimport_interop_after_
                        // reload() (engine_session.h) is the Interop
                        // Task 12 fix: re-establish it right here, on
                        // this thread, before mirroring the result into
                        // the flag the GUI thread reads -- see that
                        // function's doc comment for the full contract
                        // (why this thread, why the same handles, why a
                        // null handle must not reach
                        // try_enable_interop()) and
                        // test_interop_reload_orchestration.cpp /
                        // test_interop_reload_reset.cpp for its ctest
                        // coverage. interop_buf_handle/
                        // interop_fence_handle/interop_buffer_bytes are
                        // the SAME values used for the startup import:
                        // set once before this sim thread was
                        // constructed and never written again by any
                        // thread afterward (see their declaration
                        // above), so reading them here needs no extra
                        // synchronization -- same published-before-
                        // thread-start pattern `options`/`presenter`
                        // already rely on elsewhere in this lambda. The
                        // presenter-side D3D12 resources these handles
                        // name (the shared buffer, its SRV binding, and
                        // the shared fence) are untouched by a reload --
                        // but that only means nothing on the GUI-thread/
                        // D3D12-presenter side needs to be redone when
                        // interop was ALREADY active before this reload
                        // (the SRV was already bound then). It does NOT
                        // cover an inactive->active transition on this
                        // reload (e.g. interop failed at startup but
                        // this reload's reimport succeeds): in that case
                        // bind_interop_particle_srv() has never run for
                        // this process, and the GUI thread's message loop
                        // below separately covers that case with its own
                        // interop_srv_bound catch-up check.
                        const auto outcome =
                            ftd::native::reimport_interop_after_reload(
                                session, interop_buf_handle,
                                interop_buffer_bytes, interop_fence_handle,
                                was_active);
                        interop_active.store(outcome.interop_active);
                        if (outcome.log_enabled) {
                            std::cout << "interop: enabled after reload\n"
                                      << std::flush;
                        } else if (outcome.log_lost) {
                            std::cout << "interop: reload could not "
                                         "re-establish the D3D12/CUDA path, "
                                         "falling back to the CPU particle "
                                         "path for this session\n"
                                      << std::flush;
                        }
                        session.set_last_reload({});
                    }
                    if (session.applied_reload()) {
                        reloading.store(true);
                        need_work = true;
                    }
                    if (session.applied_host_write()) {
                        need_work = true;
                    }
                    if (need_work) {
                        // Poll and request interop work exclusively on this
                        // thread. `session`/`bridge_` must never be touched from
                        // the GUI/message-loop thread: boot() above can reset
                        // bridge_ to null mid-reconstruction with zero locking,
                        // so a render-thread call racing a reload is a
                        // null-pointer dereference waiting to happen. Staying on
                        // this thread also means a thrown GPU error (e.g.
                        // GpuEngine::interop_gather_ready()'s cudaEventQuery
                        // failure path) lands in the catch block below like every
                        // other session call here, instead of unwinding past the
                        // still-joinable `sim` thread object and calling
                        // std::terminate().
                        int polled_interop_count = -1;
                        std::uint64_t polled_interop_fence = 0;
                        if (interop_active.load()) {
                            polled_interop_count = session.poll_interop_particle_count();
                            polled_interop_fence = pending_interop_fence;
                            const std::uint64_t fv = ++interop_fence_counter;
                            // request_interop_gather() -> GpuEngine::interop_signal_fence()
                            // is documented (gpu_engine.h) as needing "the same OS
                            // thread that owns this GpuEngine's CUDA context", but
                            // try_enable_interop()'s imports ran on the main thread
                            // above while this call runs on this sim thread -- a
                            // different OS thread. What actually makes that safe is
                            // the CUDA Runtime API's per-device primary-context
                            // sharing: every host thread that touches device 0
                            // implicitly attaches to that same device-0 primary
                            // context (no cudaSetDevice() call needed to select it,
                            // since 0 is the default), so "the thread that owns the
                            // CUDA context" is really "any thread", and the main
                            // thread's imports and this sim thread's gather/signal
                            // calls end up sharing one context regardless of which
                            // OS thread issues them. That degenerates on a
                            // multi-GPU machine -- this codebase never calls
                            // cudaSetDevice(), so every thread defaults to device 0,
                            // the same assumption device_luid() relies on by reading
                            // device 0 directly -- and would need an explicit
                            // cudaSetDevice(0) per thread (or a real multi-GPU
                            // device selection story) to keep holding.
                            if (session.request_interop_gather(fv)) {
                                pending_interop_fence = fv;
                            } else {
                                // A real interop_signal_fence() failure: the
                                // gather itself may have succeeded, but the
                                // cross-API handoff that makes the buffer
                                // safely consumable by D3D12 did not, so the
                                // GUI thread's wait_shared_fence() would
                                // otherwise block forever on a fence value
                                // that is never signaled. Fall back to the
                                // CPU particle path for the rest of this
                                // session, the same way a failed post-reload
                                // re-import does above.
                                std::cout << "interop: gather/fence-signal "
                                             "failed mid-session, falling "
                                             "back to the CPU particle path "
                                             "for this session\n"
                                          << std::flush;
                                interop_active.store(false);
                            }
                        }
                        ftd::native::NativeFrame next = session.capture();
                        {
                            std::lock_guard<std::mutex> lock(frame_mu);
                            latest = std::move(next);
                            latest_interop_count = polled_interop_count;
                            latest_interop_fence = polled_interop_fence;
                        }
                    }
                } catch (const std::exception& ex) {
                    std::lock_guard<std::mutex> lock(frame_mu);
                    latest.status = ex.what();
                }
                reloading.store(false);

                const int hz = std::max(1, tick_hz.load());
                const auto budget = std::chrono::milliseconds(1000 / hz);
                const auto elapsed = std::chrono::steady_clock::now() - start;
                if (elapsed < budget) {
                    std::this_thread::sleep_for(budget - elapsed);
                }
            }
        });
        // Published only after the thread is fully constructed (and thus
        // already joinable); see AppState::sim's doc comment for why
        // wnd_proc can read this pointer race-free. No window message is
        // dispatched between this point and the sim thread's construction
        // above (the message loop hasn't started pumping yet), so there is
        // no window in which a WM_SIZE could observe app.sim as a stale
        // non-null pointer to a not-yet-started thread.
        app.sim = &sim;

        MSG msg{};
        bool quit = false;
        // GUI-thread-local catch-up for D3D12Presenter::bind_interop_
        // particle_srv(). The startup path above (interop_active.store(enabled)
        // followed by bind_interop_particle_srv()) only binds the SRV when
        // interop is already active at startup. reimport_interop_after_
        // reload() (engine_session.h/.cpp) is explicitly designed to support
        // an inactive->active transition on ANY later reload -- independent
        // of whether interop was active at startup -- and that path never
        // calls bind_interop_particle_srv(). Without this flag, a later
        // reload flipping interop_active from false to true would leave the
        // render loop issuing DrawInstanced against an srv_heap slot that was
        // never populated with a valid CreateShaderResourceView descriptor.
        // bind_interop_particle_srv() only needs to run once for the
        // lifetime of the never-recreated shared_particle_buffer resource,
        // and D3D12Presenter calls must stay off the sim thread (established
        // rule, commits be7eef14/1b80fb53), so this single GUI-thread check
        // is the correct and sufficient place for it.
        bool interop_srv_bound = false;
        bool request_quit = false;
        bool reset_camera = false;
        LARGE_INTEGER qpc_freq{};
        LARGE_INTEGER qpc_last{};
        QueryPerformanceFrequency(&qpc_freq);
        QueryPerformanceCounter(&qpc_last);
        while (!quit) {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) quit = true;
                if (msg.message == WM_KEYDOWN) {
                    const bool imgui_keys =
                        app.overlay && app.overlay->want_capture_keyboard();
                    if (msg.wParam == VK_ESCAPE) quit = true;
                    if (ftd::native::scene_accepts_keyboard(imgui_keys, false)) {
                        if (msg.wParam == VK_SPACE) request_play_toggle(&app);
                        if (msg.wParam == 'R') request_reset(&app);
                        if (msg.wParam == 'S') request_step(&app);
                    }
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (quit) break;

            ftd::native::NativeFrame frame;
            int this_frame_interop_count = -1;
            std::uint64_t this_frame_fence_value = 0;
            {
                std::lock_guard<std::mutex> lock(frame_mu);
                frame = latest;
                this_frame_interop_count = latest_interop_count;
                this_frame_fence_value = latest_interop_fence;
            }
            if (frame.lattice_size > 0 && frame.lattice_size != camera_lattice) {
                apply_camera_for_lattice(&app, frame.lattice_size);
                camera_lattice = frame.lattice_size;
            }
            if (!frame.scenario.empty()) app.live_opts.scenario = frame.scenario;
            if (frame.lattice_size > 0) app.live_opts.lattice_size = frame.lattice_size;

            wchar_t title[256];
            swprintf(title, 256,
                     L"FTD Native Desktop  %hs  L=%d  tick=%d  %hs",
                     frame.scenario.empty() ? app.live_opts.scenario.c_str()
                                            : frame.scenario.c_str(),
                     frame.lattice_size != 0 ? frame.lattice_size
                                             : app.live_opts.lattice_size,
                     frame.tick,
                     reloading.load() ? "loading"
                                      : (paused.load() ? "paused" : "run"));
            SetWindowTextW(hwnd, title);
            reloading.store(false);

            // this_frame_interop_count/this_frame_fence_value came straight out
            // of the frame_mu-protected snapshot above -- populated by the sim
            // thread, which is the only thread that ever calls
            // session.poll_interop_particle_count()/request_interop_gather()
            // (see the sim lambda's comment for why: touching `session`/
            // `bridge_` from this GUI/message-loop thread is unsafe). The
            // fence value is exactly the one request_interop_gather() was
            // called with for the gather this count was polled from, so
            // wait_shared_fence() below always waits on the fence value that
            // actually produced the buffer contents being drawn.
            const std::uint32_t draw_interop_count =
                this_frame_interop_count > 0
                    ? static_cast<std::uint32_t>(this_frame_interop_count)
                    : 0u;
            // wait_shared_fence() and render() both funnel D3D12 failures
            // through throw_if_failed() (device-removed/TDR, adapter loss on
            // sleep-resume or a monitor change, CreateCommittedResource
            // running out of memory, etc. -- realistic GPU-app failure
            // modes on this GUI/message-loop thread, not exotic ones). See
            // stop_sim_and_rethrow()'s doc comment for why an uncaught throw
            // from either call here is a std::terminate hazard (the same one
            // wnd_proc's WM_SIZE handler guards resize() against above).
            try {
                // Catch-up SRV bind for an inactive->active transition that
                // happened on a later reload rather than at startup -- see
                // interop_srv_bound's declaration above for the full
                // rationale. Idempotent and additive: does not replace the
                // startup-time bind_interop_particle_srv() call above, which
                // stays in place for the common case where interop is
                // already active at startup.
                if (interop_active.load() && !interop_srv_bound) {
                    presenter.bind_interop_particle_srv();
                    interop_srv_bound = true;
                }
                if (draw_interop_count != 0) {
                    presenter.wait_shared_fence(this_frame_fence_value);
                }
                if (app.overlay) {
                    LARGE_INTEGER qpc_now{};
                    QueryPerformanceCounter(&qpc_now);
                    float dt = 1.0f / 60.0f;
                    if (qpc_freq.QuadPart > 0) {
                        dt = static_cast<float>(qpc_now.QuadPart - qpc_last.QuadPart)
                             / static_cast<float>(qpc_freq.QuadPart);
                    }
                    qpc_last = qpc_now;
                    if (dt <= 0.0f || dt > 0.25f) dt = 1.0f / 60.0f;
                    app.overlay->begin_frame(dt);
                    if (app.shell) {
                        ftd::native::ViewChrome chrome;
                        chrome.particles = &app.view_opts.particles;
                        chrome.flux = &app.view_opts.flux;
                        chrome.lattice_box = &app.view_opts.lattice_box;
                        chrome.tick_hz = app.tick_hz;
                        chrome.paused = app.paused;
                        chrome.reset_camera = &reset_camera;
                        chrome.request_quit = &request_quit;
                        chrome.interop_active = interop_active.load();
                        auto snap = session.snapshot_publisher().acquire();
                        ftd::native::UiSnapshot fallback;
                        fallback.frame = frame;
                        const ftd::native::UiSnapshot& ui_snap =
                            snap ? *snap : fallback;
                        app.shell->draw(ui_snap, ui_commands, chrome);
                        presenter.set_scene_rect(app.shell->scene_rect());
                    }
                    app.overlay->end_frame();
                }
                if (reset_camera) {
                    apply_camera_for_lattice(&app, app.live_opts.lattice_size);
                    reset_camera = false;
                }
                if (request_quit) quit = true;
                presenter.render(frame, app.camera, app.view_opts, draw_interop_count);
            } catch (...) {
                stop_sim_and_rethrow(&running, &sim);
            }
        }

        running.store(false);
        sim.join();
        if (shell) shell->persist();
        presenter.set_overlay_recorder(nullptr);
        app.overlay = nullptr;
        app.shell = nullptr;
        presenter.wait_idle();
        overlay.shutdown();
        // Closed here, once, now that the sim thread (the only thread that
        // ever reads these after startup, via try_enable_interop() in the
        // do_reload branch above) is joined and done touching them -- see
        // their declaration above for why they were kept open this long
        // instead of being closed right after the startup import.
        if (interop_fence_handle) CloseHandle(interop_fence_handle);
        if (interop_buf_handle) CloseHandle(interop_buf_handle);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "ftd_native: " << ex.what() << "\n";
        MessageBoxA(nullptr, ex.what(), "FTD Native Desktop", MB_ICONERROR);
        return 1;
    }
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
    attach_parent_console_if_any();
    std::vector<std::string> args = utf8_args_from_command_line();
    std::vector<char*> argv;
    argv.reserve(args.size() + 1);
    for (std::string& arg : args) {
        argv.push_back(arg.data());
    }
    argv.push_back(nullptr);
    return run_native(static_cast<int>(args.size()), argv.data());
}
