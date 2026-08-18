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
#include <commctrl.h>

#include "ftd/scenarios.h"
#include "ftd/visual_snapshot.h"
#include "native_desktop/d3d12_presenter.h"
#include "native_desktop/engine_session.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, \
                "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

constexpr int kPanelWidth = 332;
constexpr int kControlPad = 14;

enum ControlId {
    IDC_FILTER = 1001,
    IDC_SCENARIOS,
    IDC_LOAD,
    IDC_LATTICE,
    IDC_BOUNDARY,
    IDC_PLAY,
    IDC_STEP,
    IDC_RESET,
    IDC_SPEED,
    IDC_SHOW_PARTICLES,
    IDC_SHOW_FLUX,
    IDC_SHOW_BOX,
    IDC_RESET_CAM,
    IDC_STATUS,
};

struct PendingWork {
    std::mutex mu;
    std::optional<ftd::native_desktop::NativeEngineOptions> reload;
    std::optional<int> boundary;
    int steps = 0;
};

struct AppState {
    HWND hwnd = nullptr;
    HWND view = nullptr;
    HWND filter = nullptr;
    HWND list = nullptr;
    HWND lattice = nullptr;
    HWND boundary = nullptr;
    HWND play = nullptr;
    HWND speed = nullptr;
    HWND chk_particles = nullptr;
    HWND chk_flux = nullptr;
    HWND chk_box = nullptr;
    HWND status = nullptr;
    HFONT font = nullptr;
    HFONT title_font = nullptr;
    HBRUSH bg = nullptr;
    HBRUSH edit_bg = nullptr;

    ftd::native_desktop::D3D12Presenter* presenter = nullptr;
    ftd::native_desktop::Camera camera;
    ftd::native_desktop::NativeViewOptions view_opts;
    ftd::native_desktop::NativeEngineOptions live_opts;
    std::vector<std::string> scenario_ids;
    PendingWork* pending = nullptr;
    std::atomic<bool>* paused = nullptr;
    std::atomic<int>* tick_hz = nullptr;
    std::atomic<bool>* reloading = nullptr;
    // Set once during main()'s setup (running right after it's declared,
    // sim right after the sim thread is constructed) and never reassigned
    // afterward, so reading these pointers from view_proc -- which runs on
    // this same GUI/message-loop thread, never concurrently with the writes
    // -- is race-free. See stop_sim_and_rethrow()'s doc comment for why
    // view_proc needs them at all: any D3D12Presenter call reachable from a
    // window message (e.g. resize() from WM_SIZE) can throw, and that
    // exception unwinds straight past the still-joinable `sim` thread
    // object unless it is stopped and joined first.
    std::atomic<bool>* running = nullptr;
    std::thread* sim = nullptr;

    bool dragging = false;
    bool syncing = false;
    POINT last{};
};

AppState* app_from_hwnd(HWND hwnd) {
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

std::wstring utf8_to_wide(const std::string& text) {
    if (text.empty()) return {};
    const int count =
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    std::wstring wide(static_cast<std::size_t>(count > 0 ? count - 1 : 0), L'\0');
    if (count > 1) {
        MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), count);
    }
    return wide;
}

std::string wide_to_utf8(const std::wstring& text) {
    if (text.empty()) return {};
    const int count =
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string out(static_cast<std::size_t>(count > 0 ? count - 1 : 0), '\0');
    if (count > 1) {
        WideCharToMultiByte(CP_UTF8, 0, text.c_str(), -1, out.data(), count, nullptr,
                            nullptr);
    }
    return out;
}

bool contains_ci(const std::string& hay, const std::string& needle) {
    if (needle.empty()) return true;
    auto lower = [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    };
    std::string a = hay;
    std::string b = needle;
    std::transform(a.begin(), a.end(), a.begin(), lower);
    std::transform(b.begin(), b.end(), b.begin(), lower);
    return a.find(b) != std::string::npos;
}

HWND create_label(HWND parent, const wchar_t* text, int x, int y, int w, int h,
                  HFONT font) {
    HWND hwnd = CreateWindowExW(0, L"STATIC", text,
                                WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, w, h,
                                parent, nullptr, GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return hwnd;
}

HWND create_button(HWND parent, ControlId id, const wchar_t* text, int x, int y,
                   int w, int h, HFONT font) {
    HWND hwnd = CreateWindowExW(0, L"BUTTON", text,
                                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                x, y, w, h, parent, reinterpret_cast<HMENU>(id),
                                GetModuleHandleW(nullptr), nullptr);
    SendMessageW(hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    return hwnd;
}

void request_reload(AppState* app) {
    if (!app || !app->pending) return;
    std::lock_guard<std::mutex> lock(app->pending->mu);
    app->pending->reload = app->live_opts;
    if (app->reloading) app->reloading->store(true);
}

void refill_scenarios(AppState* app) {
    if (!app || !app->list) return;
    wchar_t filter_buf[256] = {};
    GetWindowTextW(app->filter, filter_buf, 256);
    const std::string filter = wide_to_utf8(filter_buf);

    app->syncing = true;
    SendMessageW(app->list, LB_RESETCONTENT, 0, 0);
    int select = 0;
    int visible = 0;
    for (const std::string& id : app->scenario_ids) {
        if (!contains_ci(id, filter)) continue;
        const std::wstring wide = utf8_to_wide(id);
        SendMessageW(app->list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(wide.c_str()));
        if (id == app->live_opts.scenario) select = visible;
        ++visible;
    }
    if (visible > 0) SendMessageW(app->list, LB_SETCURSEL, select, 0);
    app->syncing = false;
}

std::string selected_scenario(const AppState* app) {
    if (!app || !app->list) return {};
    const int index = static_cast<int>(SendMessageW(app->list, LB_GETCURSEL, 0, 0));
    if (index < 0) return {};
    const int len = static_cast<int>(SendMessageW(app->list, LB_GETTEXTLEN, index, 0));
    if (len < 0) return {};
    std::wstring wide(static_cast<std::size_t>(len + 1), L'\0');
    SendMessageW(app->list, LB_GETTEXT, index, reinterpret_cast<LPARAM>(wide.data()));
    wide.resize(static_cast<std::size_t>(len));
    return wide_to_utf8(wide);
}

void load_selected_scenario(AppState* app) {
    const std::string name = selected_scenario(app);
    if (name.empty()) return;
    app->live_opts.scenario = name;
    request_reload(app);
}

void sync_lattice_combo(AppState* app) {
    if (!app || !app->lattice) return;
    app->syncing = true;
    const int count = static_cast<int>(SendMessageW(app->lattice, CB_GETCOUNT, 0, 0));
    int found = -1;
    for (int i = 0; i < count; ++i) {
        if (static_cast<int>(SendMessageW(app->lattice, CB_GETITEMDATA, i, 0)) ==
            app->live_opts.lattice_size) {
            found = i;
            break;
        }
    }
    if (found < 0) {
        const std::wstring label = std::to_wstring(app->live_opts.lattice_size);
        found = static_cast<int>(SendMessageW(
            app->lattice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str())));
        SendMessageW(app->lattice, CB_SETITEMDATA, found, app->live_opts.lattice_size);
    }
    SendMessageW(app->lattice, CB_SETCURSEL, found, 0);
    app->syncing = false;
}

void layout_shell(AppState* app, int width, int height) {
    if (!app || !app->view) return;
    const int view_x = kPanelWidth;
    const int view_w = std::max(64, width - kPanelWidth);
    const int view_h = std::max(64, height);
    MoveWindow(app->view, view_x, 0, view_w, view_h, TRUE);
}

void set_playing_caption(AppState* app) {
    if (!app || !app->play || !app->paused) return;
    SetWindowTextW(app->play, app->paused->load() ? L"Play" : L"Pause");
}

void apply_camera_for_lattice(AppState* app, int lattice_size) {
    const float center = static_cast<float>(lattice_size) * 0.5f;
    app->camera.target_x = center;
    app->camera.target_y = center;
    app->camera.target_z = center;
    app->camera.distance = static_cast<float>(lattice_size) * 1.8f;
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

LRESULT CALLBACK view_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppState* app = app_from_hwnd(hwnd);
    switch (msg) {
        case WM_LBUTTONDOWN:
            if (app) {
                app->dragging = true;
                app->last = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
                SetCapture(hwnd);
            }
            return 0;
        case WM_LBUTTONUP:
            if (app) {
                app->dragging = false;
                ReleaseCapture();
            }
            return 0;
        case WM_MOUSEMOVE:
            if (app && app->dragging) {
                const int x = GET_X_LPARAM(lparam);
                const int y = GET_Y_LPARAM(lparam);
                app->camera.yaw += (x - app->last.x) * 0.01f;
                app->camera.pitch += (y - app->last.y) * 0.01f;
                if (app->camera.pitch > 1.4f) app->camera.pitch = 1.4f;
                if (app->camera.pitch < -1.4f) app->camera.pitch = -1.4f;
                app->last = {x, y};
            }
            return 0;
        case WM_MOUSEWHEEL:
            if (app) {
                const int delta = GET_WHEEL_DELTA_WPARAM(wparam);
                app->camera.distance *= (delta > 0) ? 0.9f : 1.1f;
                if (app->camera.distance < 4.0f) app->camera.distance = 4.0f;
                if (app->camera.distance > 256.0f) app->camera.distance = 256.0f;
            }
            return 0;
        case WM_SIZE:
            if (app && app->presenter && wparam != SIZE_MINIMIZED) {
                const UINT w = LOWORD(lparam);
                const UINT h = HIWORD(lparam);
                // Reachable synchronously from DispatchMessageW well after
                // the sim thread exists (e.g. dragging/maximizing the main
                // window -> wnd_proc's WM_SIZE -> layout_shell ->
                // MoveWindow on app->view -> this handler, all on this GUI
                // thread, all before MoveWindow returns). D3D12Presenter::
                // resize() funnels ResizeBuffers/GetBuffer/
                // CreateCommittedResource/its own wait_idle() through
                // throw_if_failed() -- see stop_sim_and_rethrow()'s doc
                // comment for why an uncaught throw here is a
                // std::terminate hazard, identical to the one already
                // guarded at the per-frame render() call site below.
                if (w > 0 && h > 0) {
                    try {
                        app->presenter->resize(w, h);
                    } catch (...) {
                        stop_sim_and_rethrow(app->running, app->sim);
                    }
                }
            }
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
}

void handle_command(AppState* app, WPARAM wparam) {
    if (!app) return;
    const int id = LOWORD(wparam);
    const int code = HIWORD(wparam);
    if (app->syncing) return;

    switch (id) {
        case IDC_FILTER:
            if (code == EN_CHANGE) refill_scenarios(app);
            break;
        case IDC_SCENARIOS:
            if (code == LBN_DBLCLK) load_selected_scenario(app);
            break;
        case IDC_LOAD:
            load_selected_scenario(app);
            break;
        case IDC_LATTICE:
            if (code == CBN_SELCHANGE) {
                const int index =
                    static_cast<int>(SendMessageW(app->lattice, CB_GETCURSEL, 0, 0));
                if (index < 0) break;
                const int size = static_cast<int>(
                    SendMessageW(app->lattice, CB_GETITEMDATA, index, 0));
                if (size >= 4) {
                    app->live_opts.lattice_size = size;
                    request_reload(app);
                }
            }
            break;
        case IDC_BOUNDARY:
            if (code == CBN_SELCHANGE && app->pending) {
                const int index =
                    static_cast<int>(SendMessageW(app->boundary, CB_GETCURSEL, 0, 0));
                if (index < 0) break;
                const int mode = static_cast<int>(
                    SendMessageW(app->boundary, CB_GETITEMDATA, index, 0));
                app->live_opts.flux_boundary = mode;
                std::lock_guard<std::mutex> lock(app->pending->mu);
                app->pending->boundary = mode;
            }
            break;
        case IDC_PLAY:
            if (app->paused) app->paused->store(!app->paused->load());
            set_playing_caption(app);
            break;
        case IDC_STEP:
            if (app->paused) app->paused->store(true);
            set_playing_caption(app);
            if (app->pending) {
                std::lock_guard<std::mutex> lock(app->pending->mu);
                ++app->pending->steps;
            }
            break;
        case IDC_RESET:
            request_reload(app);
            break;
        case IDC_SHOW_PARTICLES:
            app->view_opts.particles =
                SendMessageW(app->chk_particles, BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case IDC_SHOW_FLUX:
            app->view_opts.flux =
                SendMessageW(app->chk_flux, BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case IDC_SHOW_BOX:
            app->view_opts.lattice_box =
                SendMessageW(app->chk_box, BM_GETCHECK, 0, 0) == BST_CHECKED;
            break;
        case IDC_RESET_CAM:
            apply_camera_for_lattice(app, app->live_opts.lattice_size);
            break;
        default:
            break;
    }
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    AppState* app = app_from_hwnd(hwnd);
    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (app) {
                layout_shell(app, LOWORD(lparam), HIWORD(lparam));
            }
            return 0;
        case WM_COMMAND:
            handle_command(app, wparam);
            return 0;
        case WM_HSCROLL:
            if (app && app->speed && reinterpret_cast<HWND>(lparam) == app->speed &&
                app->tick_hz) {
                const int pos =
                    static_cast<int>(SendMessageW(app->speed, TBM_GETPOS, 0, 0));
                app->tick_hz->store(std::max(1, pos));
            }
            return 0;
        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLOREDIT: {
            if (!app) break;
            const HDC hdc = reinterpret_cast<HDC>(wparam);
            SetTextColor(hdc, RGB(220, 228, 236));
            SetBkColor(hdc, msg == WM_CTLCOLORSTATIC ? RGB(18, 22, 28) : RGB(28, 34, 44));
            return reinterpret_cast<LRESULT>(
                msg == WM_CTLCOLORSTATIC ? app->bg : app->edit_bg);
        }
        case WM_ERASEBKGND: {
            if (!app) break;
            RECT rc{};
            GetClientRect(hwnd, &rc);
            rc.right = kPanelWidth;
            FillRect(reinterpret_cast<HDC>(wparam), &rc, app->bg);
            return 1;
        }
        default:
            return DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return DefWindowProcW(hwnd, msg, wparam, lparam);
}

void create_controls(HWND parent, AppState* app) {
    const int x = kControlPad;
    const int w = kPanelWidth - kControlPad * 2;
    int y = 12;

    create_label(parent, L"FTD Native Desktop", x, y, w, 22, app->title_font);
    y += 24;
    create_label(parent, L"In-process D3D12  ·  not WebView2", x, y, w, 18, app->font);
    y += 28;

    create_label(parent, L"Filter scenarios", x, y, w, 16, app->font);
    y += 18;
    app->filter = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                  x, y, w, 24, parent,
                                  reinterpret_cast<HMENU>(IDC_FILTER),
                                  GetModuleHandleW(nullptr), nullptr);
    SendMessageW(app->filter, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    y += 32;

    create_label(parent, L"Scenarios", x, y, w, 16, app->font);
    y += 18;
    app->list = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_NOTIFY |
            LBS_NOINTEGRALHEIGHT,
        x, y, w, 250, parent, reinterpret_cast<HMENU>(IDC_SCENARIOS),
        GetModuleHandleW(nullptr), nullptr);
    SendMessageW(app->list, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    y += 258;

    create_button(parent, IDC_LOAD, L"Load scenario", x, y, w, 28, app->font);
    y += 40;

    create_label(parent, L"Lattice", x, y, 90, 16, app->font);
    create_label(parent, L"Boundary", x + 110, y, 120, 16, app->font);
    y += 18;
    app->lattice = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, x, y, 100,
        200, parent, reinterpret_cast<HMENU>(IDC_LATTICE), GetModuleHandleW(nullptr),
        nullptr);
    app->boundary = CreateWindowExW(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST, x + 110, y, w - 110,
        200, parent, reinterpret_cast<HMENU>(IDC_BOUNDARY), GetModuleHandleW(nullptr),
        nullptr);
    SendMessageW(app->lattice, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    SendMessageW(app->boundary, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    y += 36;

    app->play = create_button(parent, IDC_PLAY, L"Pause", x, y, 96, 28, app->font);
    create_button(parent, IDC_STEP, L"Step", x + 104, y, 88, 28, app->font);
    create_button(parent, IDC_RESET, L"Reset", x + 200, y, w - 200, 28, app->font);
    y += 40;

    create_label(parent, L"Ticks / second", x, y, w, 16, app->font);
    y += 18;
    app->speed = CreateWindowExW(
        0, TRACKBAR_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOOLTIPS, x, y, w, 32, parent,
        reinterpret_cast<HMENU>(IDC_SPEED), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(app->speed, TBM_SETRANGE, TRUE, MAKELPARAM(1, 60));
    SendMessageW(app->speed, TBM_SETPOS, TRUE, 20);
    y += 40;

    app->chk_particles = CreateWindowExW(
        0, L"BUTTON", L"Particles",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x, y, 100, 22, parent,
        reinterpret_cast<HMENU>(IDC_SHOW_PARTICLES), GetModuleHandleW(nullptr), nullptr);
    app->chk_flux = CreateWindowExW(
        0, L"BUTTON", L"Flux",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x + 104, y, 70, 22,
        parent, reinterpret_cast<HMENU>(IDC_SHOW_FLUX), GetModuleHandleW(nullptr),
        nullptr);
    app->chk_box = CreateWindowExW(
        0, L"BUTTON", L"Lattice box",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX, x + 178, y, 120, 22,
        parent, reinterpret_cast<HMENU>(IDC_SHOW_BOX), GetModuleHandleW(nullptr),
        nullptr);
    SendMessageW(app->chk_particles, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    SendMessageW(app->chk_flux, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    SendMessageW(app->chk_box, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);
    SendMessageW(app->chk_particles, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(app->chk_flux, BM_SETCHECK, BST_CHECKED, 0);
    SendMessageW(app->chk_box, BM_SETCHECK, BST_CHECKED, 0);
    y += 30;

    create_button(parent, IDC_RESET_CAM, L"Reset camera", x, y, w, 26, app->font);
    y += 36;

    create_label(parent, L"Status", x, y, w, 16, app->font);
    y += 18;
    app->status = CreateWindowExW(
        0, L"STATIC", L"Loading…",
        WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, w, 90, parent,
        reinterpret_cast<HMENU>(IDC_STATUS), GetModuleHandleW(nullptr), nullptr);
    SendMessageW(app->status, WM_SETFONT, reinterpret_cast<WPARAM>(app->font), TRUE);

    const int sizes[] = {9, 17, 25, 32, 33, 49};
    for (int size : sizes) {
        const std::wstring label = std::to_wstring(size);
        const int index = static_cast<int>(SendMessageW(
            app->lattice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str())));
        SendMessageW(app->lattice, CB_SETITEMDATA, index, size);
    }

    struct BoundaryItem {
        const wchar_t* label;
        int value;
    };
    const BoundaryItem boundaries[] = {
        {L"Periodic", 0},
        {L"Reflective", 1},
        {L"Dispersal", 2},
    };
    for (const BoundaryItem& item : boundaries) {
        const int index = static_cast<int>(SendMessageW(
            app->boundary, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.label)));
        SendMessageW(app->boundary, CB_SETITEMDATA, index, item.value);
        if (item.value == app->live_opts.flux_boundary) {
            SendMessageW(app->boundary, CB_SETCURSEL, index, 0);
        }
    }
}

bool is_edit_focus() {
    HWND focus = GetFocus();
    if (!focus) return false;
    wchar_t cls[32] = {};
    GetClassNameW(focus, cls, 32);
    return _wcsicmp(cls, L"Edit") == 0;
}

ftd::native_desktop::NativeEngineOptions parse_options(int argc, char** argv) {
    ftd::native_desktop::NativeEngineOptions options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--cpu") {
            options.force_cpu = true;
        } else if (arg == "--gpu") {
            options.force_cpu = false;
        } else if (arg == "--lattice" && i + 1 < argc) {
            options.lattice_size = std::stoi(argv[++i]);
        } else if (arg == "--scenario" && i + 1 < argc) {
            options.scenario = argv[++i];
        } else if (arg == "--help") {
            std::cout
                << "ftd_native_desktop [--cpu|--gpu] [--lattice N] [--scenario name]\n"
                << "  Defaults: --cpu --lattice 32 --scenario s0-seed-hydrogen\n"
                << "  Left panel: scenarios, lattice, play/pause/step/reset\n"
                << "  View: left-drag orbit, wheel zoom, Space pause, Esc quit\n";
            std::exit(0);
        }
    }
    return options;
}

}  // namespace

int main(int argc, char** argv) {
    try {
        INITCOMMONCONTROLSEX icc{};
        icc.dwSize = sizeof(icc);
        icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES;
        InitCommonControlsEx(&icc);

        auto options = parse_options(argc, argv);
        std::cout << "FTD native desktop (in-process, not WebView2)\n";
        std::cout << "Loading L=" << options.lattice_size
                  << " scenario=" << options.scenario
                  << (options.force_cpu ? " cpu" : " gpu-default") << "...\n"
                  << std::flush;

        ftd::native_desktop::NativeEngineSession session(options);
        std::cout << "backend=" << session.backend_name()
                  << " status=" << session.status() << "\n"
                  << std::flush;

        PendingWork pending;
        std::atomic<bool> running{true};
        std::atomic<bool> paused{false};
        std::atomic<int> tick_hz{20};
        std::atomic<bool> reloading{false};

        AppState app;
        app.live_opts = session.options();
        app.pending = &pending;
        app.paused = &paused;
        app.tick_hz = &tick_hz;
        app.reloading = &reloading;
        app.running = &running;
        app.bg = CreateSolidBrush(RGB(18, 22, 28));
        app.edit_bg = CreateSolidBrush(RGB(28, 34, 44));
        app.font = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                               L"Segoe UI");
        app.title_font = CreateFontW(-18, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                     CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                     DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        for (std::string_view id : ftd::scale0_scenario_ids()) {
            app.scenario_ids.emplace_back(id);
        }
        apply_camera_for_lattice(&app, session.lattice_size());

        WNDCLASSW wc{};
        wc.lpfnWndProc = wnd_proc;
        wc.hInstance = GetModuleHandleW(nullptr);
        wc.lpszClassName = L"FtdNativeDesktop";
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = app.bg;
        RegisterClassW(&wc);

        WNDCLASSW view_wc{};
        view_wc.lpfnWndProc = view_proc;
        view_wc.hInstance = wc.hInstance;
        view_wc.lpszClassName = L"FtdNativeView";
        view_wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        view_wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
        RegisterClassW(&view_wc);

        HWND hwnd = CreateWindowExW(
            0, wc.lpszClassName, L"FTD Native Desktop",
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE, CW_USEDEFAULT,
            CW_USEDEFAULT, 1480, 860, nullptr, nullptr, wc.hInstance, nullptr);
        if (!hwnd) throw std::runtime_error("CreateWindowExW failed");
        app.hwnd = hwnd;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));

        create_controls(hwnd, &app);
        refill_scenarios(&app);
        sync_lattice_combo(&app);
        set_playing_caption(&app);

        RECT client{};
        GetClientRect(hwnd, &client);
        app.view = CreateWindowExW(
            0, view_wc.lpszClassName, L"", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            kPanelWidth, 0, std::max(64L, client.right - kPanelWidth),
            std::max(64L, client.bottom), hwnd, nullptr, wc.hInstance, nullptr);
        SetWindowLongPtrW(app.view, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&app));
        layout_shell(&app, client.right, client.bottom);

        ftd::native_desktop::D3D12Presenter presenter;
        RECT view_rc{};
        GetClientRect(app.view, &view_rc);
        presenter.initialize(app.view, static_cast<std::uint32_t>(view_rc.right),
                             static_cast<std::uint32_t>(view_rc.bottom));
        app.presenter = &presenter;

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
        ftd::native_desktop::NativeFrame latest = session.capture();
        int camera_lattice = session.lattice_size();
        // Interop poll result, produced exclusively on the sim thread (the
        // only thread allowed to touch `session`/`bridge_` -- see the sim
        // lambda below) and consumed on the GUI/message-loop thread under
        // frame_mu, same pattern as `latest`. -1 means "not ready this
        // round" or "interop inactive", matching
        // NativeEngineSession::poll_interop_particle_count()'s own contract.
        int latest_interop_count = -1;
        std::uint64_t latest_interop_fence = 0;

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
                ftd::native_desktop::NativeEngineOptions reload_opts;
                bool do_reload = false;
                int boundary = -1;
                int steps = 0;
                {
                    std::lock_guard<std::mutex> lock(pending.mu);
                    if (pending.reload) {
                        reload_opts = *pending.reload;
                        pending.reload.reset();
                        do_reload = true;
                    }
                    if (pending.boundary) {
                        boundary = *pending.boundary;
                        pending.boundary.reset();
                    }
                    steps = pending.steps;
                    pending.steps = 0;
                }

                const auto start = std::chrono::steady_clock::now();
                try {
                    const bool need_work =
                    do_reload || boundary >= 0 || steps > 0 || !paused.load();
                    if (need_work) {
                        if (do_reload) {
                            const bool was_active = interop_active.load();
                            session.apply_options(reload_opts);
                            // boot() (invoked by apply_options()) always
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
                                ftd::native_desktop::reimport_interop_after_reload(
                                    session, interop_buf_handle,
                                    interop_buffer_bytes, interop_fence_handle,
                                    was_active);
                            interop_active.store(outcome.interop_active);
                            if (outcome.log_enabled) {
                                std::cout << "interop: enabled after reload\n"
                                          << std::flush;
                            } else if (outcome.log_lost) {
                                // Genuinely unexpected at this point (the same
                                // handles/buffer just worked before this
                                // reload), but not impossible -- e.g. a
                                // device-removed/TDR event on the shared
                                // resources during the reload. Log it the same
                                // way the pre-Task-12 code logged the
                                // then-permanent "no re-import path" fallback,
                                // so the degradation is visible somewhere beyond
                                // a debugger.
                                std::cout << "interop: reload could not "
                                             "re-establish the D3D12/CUDA path, "
                                             "falling back to the CPU particle "
                                             "path for this session\n"
                                          << std::flush;
                            }
                        } else if (boundary >= 0) {
                            session.set_flux_boundary(boundary);
                        }
                        if (steps > 0) {
                            for (int i = 0; i < steps; ++i) session.tick();
                        } else if (!paused.load() && !do_reload) {
                            session.tick();
                        }
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
                        ftd::native_desktop::NativeFrame next = session.capture();
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
        // view_proc can read this pointer race-free. No window message is
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
        while (!quit) {
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) quit = true;
                if (msg.message == WM_KEYDOWN) {
                    const bool typing = is_edit_focus();
                    if (msg.wParam == VK_ESCAPE) quit = true;
                    if (!typing && msg.wParam == VK_SPACE) {
                        paused.store(!paused.load());
                        set_playing_caption(&app);
                    }
                    if (!typing && msg.wParam == 'R') request_reload(&app);
                    if (!typing && msg.wParam == 'S') {
                        paused.store(true);
                        set_playing_caption(&app);
                        std::lock_guard<std::mutex> lock(pending.mu);
                        ++pending.steps;
                    }
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            if (quit) break;

            ftd::native_desktop::NativeFrame frame;
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

            wchar_t status[512];
            swprintf(status, 512,
                     L"%hs\nbackend %hs\nL=%d  tick=%d\nparticles %zu   flux %zu\n%hs",
                     frame.scenario.empty() ? app.live_opts.scenario.c_str()
                                            : frame.scenario.c_str(),
                     frame.backend.empty() ? "cpu" : frame.backend.c_str(),
                     frame.lattice_size, frame.tick, frame.particles.size(),
                     frame.flux.size(),
                     reloading.load() ? "Loading scenario..."
                                      : (frame.status.empty() ? "ready"
                                                              : frame.status.c_str()));
            SetWindowTextW(app.status, status);

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
            // view_proc's WM_SIZE handler guards resize() against above).
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
                presenter.render(frame, app.camera, app.view_opts, draw_interop_count);
            } catch (...) {
                stop_sim_and_rethrow(&running, &sim);
            }
        }

        running.store(false);
        sim.join();
        presenter.wait_idle();
        // Closed here, once, now that the sim thread (the only thread that
        // ever reads these after startup, via try_enable_interop() in the
        // do_reload branch above) is joined and done touching them -- see
        // their declaration above for why they were kept open this long
        // instead of being closed right after the startup import.
        if (interop_fence_handle) CloseHandle(interop_fence_handle);
        if (interop_buf_handle) CloseHandle(interop_buf_handle);
        DeleteObject(app.font);
        DeleteObject(app.title_font);
        DeleteObject(app.bg);
        DeleteObject(app.edit_bg);
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "ftd_native_desktop: " << ex.what() << "\n";
        MessageBoxA(nullptr, ex.what(), "FTD Native Desktop", MB_ICONERROR);
        return 1;
    }
}
