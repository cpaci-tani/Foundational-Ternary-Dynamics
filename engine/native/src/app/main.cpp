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
#include "native/scale0_overlays.h"
#include "native/scenario_catalog.h"  // ftd::native scenario catalog (Setup picker)
#include "native/scene_rect.h"

#include "ftd/term_toggles.h"
#include "ftd/visual_field_sample.h"   // ftd::VisualFieldKind (FIELDS overlay menu)
#include "ftd/visual_snapshot.h"   // ftd::kMaxVisualParticleCapture (interop buffer sizing)

#include "ui/rml_d3d12_renderer.h"
#include "ui/ftd_chart_element.h"

#include "app/app_options.h"   // AppOptions / parse_app_options / parse_force_style
#include "app/app_util.h"      // split_csv / to_lower / upper / fmt / fmt3
#include "app/ui_model.h"      // ShellData + row types + toggle/config/overlay/scenario builders
#include "app/app_context.h"   // AppContext + command / request / nudge helpers
#include "app/app_pick.h"      // camera framing + click-to-inspect ray picking
#include "app/app_win32.h"     // widen / save_png / utf8_args / console attach
#include "app/app_input.h"     // wnd_proc + RmlOverlay recorder

#include <RmlUi/Core.h>
#include <RmlUi/Core/Factory.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

using Microsoft::WRL::ComPtr;
using ftd::native::ui::RmlD3D12Renderer;
using ftd::native::ui::RmlD3D12System;

// The app's helpers are being split out of this file (behavior-neutral) into
// namespace ftd::native::app under src/app/. This directive lets the remaining
// run_app / wWinMain code below reference them unqualified, as before the split.
using namespace ftd::native::app;

namespace {

// Win32 window procedure + RmlOverlay recorder + input helpers live in
// app/app_input.{h,cpp}.

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

    // Resolve the initial active-overlay set (Scale-0 only) from --overlays
    // (comma-separated) and the legacy --field alias. Unknown names warn + skip.
    // The resolved names drive BOTH the first-boundary stamp (so frame 0 already
    // composites them) AND the panel's initial lit LEDs.
    std::vector<std::string> initial_overlays;
    if (app_opts.scale == 0) {
        auto add_overlay = [&](const std::string& name) {
            if (const auto* d = ftd::native::overlay_by_name(name)) {
                if (std::find(initial_overlays.begin(), initial_overlays.end(), name)
                    == initial_overlays.end())
                    initial_overlays.push_back(d->name);
            } else {
                std::cerr << "native_app: unknown overlay '" << name << "' (ignored)\n"
                          << std::flush;
            }
        };
        for (const std::string& n : split_csv(app_opts.overlays)) add_overlay(n);
        if (!app_opts.field.empty()) add_overlay(app_opts.field);
    }

    // Prime the loop control + publish one snapshot before the sim thread starts.
    // The initial overlays (Scale-0) are stamped into this first boundary so the
    // very first captured frame already composites them.
    {
        host.set_loop_control({app_opts.start_paused, !app_opts.start_paused, 0});
        ftd::native::CommandBus stamp;
        for (const std::string& name : initial_overlays) {
            if (const auto* d = ftd::native::overlay_by_name(name)) {
                stamp.push(ftd::native::scale0_command(ftd::native::SetOverlay{
                    static_cast<std::uint32_t>(d->id), true}));
            }
        }
        // Initial sheet slice heights (--sheet-height), stamped AFTER SetOverlay
        // so they override the y_frac seed the toggle-on installs.
        if (app_opts.scale == 0) {
            for (const auto& [nm, frac] : app_opts.sheet_heights) {
                const auto* d = ftd::native::overlay_by_name(nm);
                if (d && d->render == ftd::native::OverlayRender::Sheet) {
                    stamp.push(ftd::native::scale0_command(ftd::native::SetSheetHeight{
                        static_cast<std::uint32_t>(d->id), frac}));
                } else {
                    std::cerr << "native_app: --sheet-height '" << nm
                              << "' is not a rubber-sheet overlay (ignored)\n" << std::flush;
                }
            }
            // Global force render-style (--force-style), stamped so the first
            // captured frame renders the Force overlays in that style.
            if (!app_opts.force_style.empty()) {
                stamp.push(ftd::native::scale0_command(ftd::native::SetForceStyle{
                    static_cast<std::uint32_t>(parse_force_style(app_opts.force_style))}));
            }
        }
        host.process_ui_boundary(stamp);
    }
    // Prime-tick-on-load (default ON): run exactly ONE tick before the sim thread
    // starts so overlays have field data to render even while the app is paused
    // (mirrors the web `primeTickOnLoad`). tick_once() advances one tick
    // regardless of the pause state set above; the sim thread then honors pause.
    if (app_opts.prime_tick) {
        const auto primed = host.tick_once();
        if (!primed.ok)
            std::cerr << "native_app: prime tick failed: " << primed.message << "\n"
                      << std::flush;
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
    // Physics control surface: COLLAPSED on boot (data.phys_open / data.cfg_open
    // both false), so the bound toggle_groups / config_rows arrays start EMPTY —
    // the 44 toggle rows + config knobs are instantiated lazily the first time the
    // user opens a section (see the toggle_physics / toggle_config callbacks),
    // keeping the boot/normal-use DOM tiny. The --open-physics / --open-config CLI
    // flags pre-open them for headless captures (handled after `app` is built).
    // FIELDS overlay panel (7 columns, multi-select) + initial lit LEDs (mirrors
    // the overlay stamp above so the panel matches the geometry from frame 0).
    data.overlay_columns = build_overlay_columns();
    for (const std::string& name : initial_overlays)
        if (OverlayRow* r = find_overlay_row(&data, Rml::String(name.c_str())))
            r->on = true;
    // Reflect the initial --force-style in the selector (mirrors the SetForceStyle
    // stamped above) so the lit button matches the geometry from frame 0.
    data.force_style = static_cast<int>(parse_force_style(app_opts.force_style));
    // Setup scenario picker: starts COLLAPSED (data.scn_open == false), so the
    // bound scenario_groups array is left EMPTY on boot — the ~130 scenario <div>s
    // are instantiated lazily the first time the user opens the picker (see the
    // toggle_scn_picker callback). This keeps the boot/normal-use DOM tiny.
    // Reflect any --sheet-height overrides in the panel rows (mirrors the
    // SetSheetHeight commands stamped above) so the shown value starts correct.
    for (const auto& [nm, frac] : app_opts.sheet_heights) {
        if (OverlayRow* r = find_overlay_row(&data, Rml::String(nm.c_str()))) {
            const float h = std::clamp(frac, 0.0f, 0.999f);
            r->height = h;
            r->hstr = sheet_hstr(h);
        }
    }

    // ── Telemetry ring buffers + the <ftd-chart> registry/instancer ────────────
    // The app owns every series (GUI thread), pushing one scalar per published
    // snapshot below; the custom elements read them read-only. Declared here so
    // all outlive Rml::Shutdown() (which releases the instanced elements back
    // through the instancer). The registry maps each chart's `id` → its coloured
    // series set; the trace colours mirror the RCSS legend-chip classes (.s0..s4)
    // so the legend and the traces agree. Registered after Rml::Initialise(),
    // before LoadDocument parses <ftd-chart>.
    //
    // Sources (all from the published Scale0Snapshot):
    //   chart-diag  ← telemetry.diagnostics (scheduler DIAGNOSTICS group) + the
    //                 always-live energy_ledger; the base "what is the field doing"
    //                 view (energy · manifested · entropy · net charge).
    //   chart-audit ← telemetry.audit (scheduler AUDIT group) + energy_ledger.dE_dt;
    //                 the "is energy conserved" view (accounted E · drift · Gauss).
    //   chart-lagr  ← telemetry.lagrangian (scheduler LAGRANGIAN group); ℒ and ℋ.
    using ftd::native::ui::ChartSeries;
    ChartSeries diag_energy(240), diag_manif(240), diag_entropy(240), diag_charge(240);
    ChartSeries aud_energy(240), aud_drift(240), aud_gauss(240);
    ChartSeries lag_lag(240), lag_ham(240);
    ftd::native::ui::ChartRegistry chart_registry;
    {
        using C = Rml::Colourb;
        const C kBlue(106, 168, 224);    // .s0
        const C kGreen(78, 203, 138);    // .s1
        const C kAmber(224, 166, 58);    // .s2
        const C kRed(224, 106, 106);     // .s3
        const C kViolet(169, 138, 224);  // .s4
        chart_registry.binding("chart-diag").series = {
            {&diag_energy, kBlue}, {&diag_manif, kGreen},
            {&diag_entropy, kAmber}, {&diag_charge, kRed}};
        chart_registry.binding("chart-audit").series = {
            {&aud_energy, kBlue}, {&aud_drift, kAmber}, {&aud_gauss, kRed}};
        chart_registry.binding("chart-lagr").series = {
            {&lag_lag, kBlue}, {&lag_ham, kViolet}};
    }
    ftd::native::ui::FtdChartInstancer chart_instancer(&chart_registry);
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
    // Seed the lattice-reboot run config from the launch options, so a lattice
    // nudge re-applies the same backend/boundary the app booted with (the reboot
    // reloads the CURRENT scenario at the new L).
    app.run_config.lattice_size = host.lattice_size();
    app.run_config.force_cpu = engine_opts.force_cpu;
    app.run_config.flux_boundary = engine_opts.flux_boundary;
    app.backend_cpu = engine_opts.force_cpu;

    // Seed the scroll-wheel height target to the last active sheet (if any), so
    // Shift+wheel over the scene is tactile from frame 0 in headless captures.
    for (const std::string& name : initial_overlays) {
        const auto* d = ftd::native::overlay_by_name(name);
        if (d && d->render == ftd::native::OverlayRender::Sheet) {
            app.last_sheet_id = static_cast<std::uint32_t>(d->id);
            app.has_last_sheet = true;
        }
    }

    // Simulated initial pick (headless captures — interactive picking cannot run
    // under --capture-frames). Honored only when it matches the initial scale
    // (voxel ↔ Scale 0, particle ↔ Scale 1). This seeds the SAME selection state
    // a real click sets, so the GUI loop's live re-inspection populates the
    // inspector before the capture fires.
    if (initial_scale == 0 && !app_opts.inspect_voxel.empty()) {
        int ix = 0, iy = 0, iz = 0;
        if (parse_ijk(app_opts.inspect_voxel, ix, iy, iz)) {
            app.inspect_kind = 1;
            app.inspect_vx = ix;
            app.inspect_vy = iy;
            app.inspect_vz = iz;
        } else {
            std::cerr << "native_app: bad --inspect-voxel '" << app_opts.inspect_voxel
                      << "' (want i,j,k; ignored)\n" << std::flush;
        }
    } else if (initial_scale == 1 && app_opts.have_inspect_particle) {
        app.inspect_kind = 2;
        app.inspect_pidx = app_opts.inspect_particle;
    }
    // Simulated neighbour-walk (headless proof of the click-to-walk feature): step
    // the just-selected inspection cursor by the given Moore offset.
    if (app.inspect_kind == 1 && !app_opts.walk_neigh.empty()) {
        int wx = 0, wy = 0, wz = 0;
        if (parse_ijk(app_opts.walk_neigh, wx, wy, wz))
            walk_inspection(&app, wx, wy, wz);
        else
            std::cerr << "native_app: bad --walk-neigh '" << app_opts.walk_neigh
                      << "' (want dx,dy,dz; ignored)\n" << std::flush;
    }

    // Simulated Setup-picker selection (headless captures — interactive clicking
    // cannot run under --capture-frames). --pick-scenario <id> drives the SAME
    // select_scenario → LoadScenario path a real click uses: it records the id as
    // the Reset target, moves the highlight, and enqueues the reload the sim
    // thread applies at its next boundary, so the captured frame shows a scenario
    // loaded via the picker. Scale-0 only (the catalog is Scale-0 scenarios).
    if (initial_scale == 0 && !app_opts.pick_scenario.empty()) {
        const std::string& pick = app_opts.pick_scenario;
        if (ftd::native::find_scenario_meta(pick)) {
            // Open the picker for the headless capture and demonstrate the intended
            // fast-browse path: type-to-filter narrows the list to the matches (a
            // small DOM), then select loads. Seeding the filter with the target id
            // force-opens its group with only the matching row(s), so the captured
            // frame shows a searched list with the target highlighted — the exact
            // select_scenario → LoadScenario path a real picker click drives.
            data.scn_open = true;
            app.scenario_filter = pick;
            rebuild_scenario_view(&data, pick, app.scenario_filter,
                                  app.scn_expanded_groups);
            app.scenario_id = pick;
            set_current_scenario(&data, pick);
            data.scenario = pick;
            push_core(&app, ftd::native::LoadScenario{pick});
            std::cout << "native_app: picker select_scenario -> " << pick << "\n"
                      << std::flush;
        } else {
            std::cerr << "native_app: --pick-scenario '" << pick
                      << "' not in catalog; ignored\n" << std::flush;
        }
    }

    // UI-profiling spike: open the picker and expand a full category (no filter),
    // so the profiler measures the reflow cost of a LARGE list. Deliberately
    // distinct from --pick-scenario (which seeds a filter to SHRINK the list).
    if (app_opts.scn_open || app_opts.scn_expand_cat > 0) {
        data.scn_open = true;
        app.scenario_filter.clear();
        const int ncat =
            static_cast<int>(sizeof(kScenarioCategories) / sizeof(kScenarioCategories[0]));
        if (app_opts.scn_expand_cat >= 1 && app_opts.scn_expand_cat <= ncat) {
            app.scn_expanded_groups.assign(
                1, kScenarioCategories[app_opts.scn_expand_cat - 1]);
        }
        rebuild_scenario_view(&data, app.scenario_id, app.scenario_filter,
                              app.scn_expanded_groups);
        std::cout << "native_app: profile picker open, expand_cat="
                  << app_opts.scn_expand_cat << " ("
                  << data.scenario_groups.size() << " groups)\n"
                  << std::flush;
    }

    Rml::DataModelConstructor ctor = context->CreateDataModel("shell");
    if (!ctor) throw std::runtime_error("CreateDataModel(shell) failed");
    // Physics control surface: the 44-toggle rows (categorized) + config knobs.
    if (auto trow = ctor.RegisterStruct<FullToggleRow>()) {
        trow.RegisterMember("name", &FullToggleRow::name);
        trow.RegisterMember("desc", &FullToggleRow::desc);
        trow.RegisterMember("req", &FullToggleRow::req);
        trow.RegisterMember("on", &FullToggleRow::on);
        trow.RegisterMember("gated", &FullToggleRow::gated);
        trow.RegisterMember("has_req", &FullToggleRow::has_req);
    }
    ctor.RegisterArray<Rml::Vector<FullToggleRow>>();
    if (auto tgrp = ctor.RegisterStruct<ToggleGroupRow>()) {
        tgrp.RegisterMember("title", &ToggleGroupRow::title);
        tgrp.RegisterMember("expanded", &ToggleGroupRow::expanded);
        tgrp.RegisterMember("count", &ToggleGroupRow::count);
        tgrp.RegisterMember("items", &ToggleGroupRow::items);
    }
    ctor.RegisterArray<Rml::Vector<ToggleGroupRow>>();
    if (auto crow = ctor.RegisterStruct<ConfigRow>()) {
        crow.RegisterMember("key", &ConfigRow::key);
        crow.RegisterMember("label", &ConfigRow::label);
        crow.RegisterMember("vstr", &ConfigRow::vstr);
        crow.RegisterMember("hint", &ConfigRow::hint);
    }
    ctor.RegisterArray<Rml::Vector<ConfigRow>>();
    if (auto orow = ctor.RegisterStruct<OverlayRow>()) {
        orow.RegisterMember("name", &OverlayRow::name);
        orow.RegisterMember("label", &OverlayRow::label);
        orow.RegisterMember("on", &OverlayRow::on);
        orow.RegisterMember("is_sheet", &OverlayRow::is_sheet);
        orow.RegisterMember("hstr", &OverlayRow::hstr);
    }
    ctor.RegisterArray<Rml::Vector<OverlayRow>>();
    if (auto ocol = ctor.RegisterStruct<OverlayColumnRow>()) {
        ocol.RegisterMember("title", &OverlayColumnRow::title);
        ocol.RegisterMember("expanded", &OverlayColumnRow::expanded);
        ocol.RegisterMember("count", &OverlayColumnRow::count);
        ocol.RegisterMember("items", &OverlayColumnRow::items);
    }
    ctor.RegisterArray<Rml::Vector<OverlayColumnRow>>();
    if (auto irow = ctor.RegisterStruct<InspLine>()) {
        irow.RegisterMember("k", &InspLine::k);
        irow.RegisterMember("v", &InspLine::v);
        irow.RegisterMember("header", &InspLine::header);
    }
    ctor.RegisterArray<Rml::Vector<InspLine>>();
    if (auto ncell = ctor.RegisterStruct<InspNeighCell>()) {
        ncell.RegisterMember("dir", &InspNeighCell::dir);
        ncell.RegisterMember("val", &InspNeighCell::val);
        ncell.RegisterMember("state", &InspNeighCell::state);
        ncell.RegisterMember("dx", &InspNeighCell::dx);
        ncell.RegisterMember("dy", &InspNeighCell::dy);
        ncell.RegisterMember("dz", &InspNeighCell::dz);
    }
    ctor.RegisterArray<Rml::Vector<InspNeighCell>>();
    if (auto srow = ctor.RegisterStruct<ScenarioRow>()) {
        srow.RegisterMember("id", &ScenarioRow::id);
        srow.RegisterMember("title", &ScenarioRow::title);
        srow.RegisterMember("current", &ScenarioRow::current);
        srow.RegisterMember("visible", &ScenarioRow::visible);
        srow.RegisterMember("tag", &ScenarioRow::tag);
        srow.RegisterMember("tag_cls", &ScenarioRow::tag_cls);
    }
    ctor.RegisterArray<Rml::Vector<ScenarioRow>>();
    if (auto sgrp = ctor.RegisterStruct<ScenarioGroupRow>()) {
        sgrp.RegisterMember("title", &ScenarioGroupRow::title);
        sgrp.RegisterMember("expanded", &ScenarioGroupRow::expanded);
        sgrp.RegisterMember("has_visible", &ScenarioGroupRow::has_visible);
        sgrp.RegisterMember("show_items", &ScenarioGroupRow::show_items);
        sgrp.RegisterMember("count", &ScenarioGroupRow::count);
        sgrp.RegisterMember("items", &ScenarioGroupRow::items);
    }
    ctor.RegisterArray<Rml::Vector<ScenarioGroupRow>>();
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
    ctor.Bind("phys_open", &data.phys_open);
    ctor.Bind("cfg_open", &data.cfg_open);
    ctor.Bind("ov_open", &data.ov_open);
    ctor.Bind("toggle_groups", &data.toggle_groups);
    ctor.Bind("config_rows", &data.config_rows);
    ctor.Bind("has_validation", &data.has_validation);
    ctor.Bind("validation_msg", &data.validation_msg);
    ctor.Bind("overlay_columns", &data.overlay_columns);
    ctor.Bind("force_style", &data.force_style);
    ctor.Bind("scn_open", &data.scn_open);
    ctor.Bind("scenario_groups", &data.scenario_groups);
    ctor.Bind("insp_active", &data.insp_active);
    ctor.Bind("insp_title", &data.insp_title);
    ctor.Bind("insp_lines", &data.insp_lines);
    ctor.Bind("insp_neigh_show", &data.insp_neigh_show);
    ctor.Bind("insp_neigh_open", &data.insp_neigh_open);
    ctor.Bind("insp_faces", &data.insp_faces);
    ctor.Bind("insp_edges", &data.insp_edges);
    ctor.Bind("insp_corners", &data.insp_corners);
    // Telemetry section (collapsible; charts + legend live behind tel_open).
    ctor.Bind("tel_open", &data.tel_open);
    ctor.Bind("tel_d_energy", &data.tel_d_energy);
    ctor.Bind("tel_d_manif", &data.tel_d_manif);
    ctor.Bind("tel_d_entropy", &data.tel_d_entropy);
    ctor.Bind("tel_d_charge", &data.tel_d_charge);
    ctor.Bind("tel_diag_prov", &data.tel_diag_prov);
    ctor.Bind("tel_a_energy", &data.tel_a_energy);
    ctor.Bind("tel_a_drift", &data.tel_a_drift);
    ctor.Bind("tel_a_gauss", &data.tel_a_gauss);
    ctor.Bind("tel_audit_prov", &data.tel_audit_prov);
    ctor.Bind("tel_l_lag", &data.tel_l_lag);
    ctor.Bind("tel_l_ham", &data.tel_l_ham);
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
    // Physics-terms section — expand/collapse the whole toggle-category list. On
    // open, build the category headers (items are built per-category on demand);
    // on close, drop every row so the DOM holds ~0 toggle <div>s (the picker's
    // DOM-shrink discipline). This is the fps guard for the 44-toggle panel.
    ctor.BindEventCallback("toggle_physics", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                    const Rml::VariantList&) {
        if (!app.data) return;
        app.data->phys_open = !app.data->phys_open;
        if (app.data->phys_open)
            rebuild_toggle_groups(app.data, app.live_toggles, app.tog_expanded_groups);
        else
            app.data->toggle_groups.clear();
        h.DirtyVariable("phys_open");
        h.DirtyVariable("toggle_groups");
    });
    // Expand/collapse one toggle CATEGORY (its header is the affordance). Only an
    // expanded category builds its item rows, so the live DOM stays small.
    ctor.BindEventCallback("toggle_tog_group", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                      const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string title(v[0].Get<Rml::String>().c_str());
        auto& ex = app.tog_expanded_groups;
        auto it = std::find(ex.begin(), ex.end(), title);
        if (it == ex.end()) ex.push_back(title);
        else ex.erase(it);
        rebuild_toggle_groups(app.data, app.live_toggles, ex);
        h.DirtyVariable("toggle_groups");
    });
    // Config section — expand/collapse the config knobs + lattice + reset. On
    // open, build the knob rows from the live engine truth; on close, drop them.
    ctor.BindEventCallback("toggle_config", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                   const Rml::VariantList&) {
        if (!app.data) return;
        app.data->cfg_open = !app.data->cfg_open;
        if (app.data->cfg_open)
            build_config_rows(app.data, app.live_toggles, app.live_knobs);
        else
            app.data->config_rows.clear();
        h.DirtyVariable("cfg_open");
        h.DirtyVariable("config_rows");
    });
    // Field-overlays section collapse (matches Physics terms / Config): gates the
    // 7 overlay columns + the force-style row behind ov_open. Collapsed by default.
    ctor.BindEventCallback("toggle_overlays", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                     const Rml::VariantList&) {
        if (!app.data) return;
        app.data->ov_open = !app.data->ov_open;
        h.DirtyVariable("ov_open");
    });
    // Config knob nudge: v[0] = knob key, v[1] = "-"/"+". Steps/cycles the value
    // (reading current from the live mirror) and pushes the matching command.
    ctor.BindEventCallback("config_nudge", [&app](Rml::DataModelHandle, Rml::Event&,
                                                  const Rml::VariantList& v) {
        if (v.size() < 2) return;
        request_config_nudge(&app, std::string(v[0].Get<Rml::String>().c_str()),
                             std::string(v[1].Get<Rml::String>().c_str()));
    });
    // Reset every term toggle to its canonical default (ResetToDefaults).
    ctor.BindEventCallback("reset_toggles", [&app](Rml::DataModelHandle, Rml::Event&,
                                                   const Rml::VariantList&) {
        request_reset_toggles(&app);
    });
    // Telemetry section — expand/collapse the charts. Opening it flips the demand
    // mask to add the heavier AUDIT + LAGRANGIAN scheduler groups (diagnostics is
    // demanded regardless); closing it drops back to diagnostics-only so idle fps
    // stays high. The charts/legend are gated by data-if="tel_open", so a closed
    // section holds ~0 chart DOM (the physics/scenario-picker DOM-shrink pattern).
    ctor.BindEventCallback("toggle_telemetry", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                      const Rml::VariantList&) {
        if (!app.data) return;
        app.data->tel_open = !app.data->tel_open;
        request_telemetry_demand(&app, app.data->tel_open);
        h.DirtyVariable("tel_open");
    });
    ctor.BindEventCallback("scale_lattice", [&app](Rml::DataModelHandle, Rml::Event&,
                                                   const Rml::VariantList&) {
        request_switch_scale(&app, 0);
    });
    ctor.BindEventCallback("scale_particles", [&app](Rml::DataModelHandle, Rml::Event&,
                                                     const Rml::VariantList&) {
        request_switch_scale(&app, 1);
    });
    // Overlay toggle: flip one overlay's membership in the active set. Pushes a
    // SetOverlay Scale-0 command (multi-select — the adapter composites all
    // active overlays) and lights/clears the row's LED. The bound `on` state is
    // owned here (no snapshot round-trip carries it back), so dirty the array.
    ctor.BindEventCallback("set_overlay", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                 const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const Rml::String name = v[0].Get<Rml::String>();
        const ftd::native::OverlayDescriptor* d =
            ftd::native::overlay_by_name(name.c_str());
        OverlayRow* row = find_overlay_row(app.data, name);
        if (!d || !row) return;
        row->on = !row->on;
        push_scale0(&app, ftd::native::SetOverlay{static_cast<std::uint32_t>(d->id),
                                                  row->on});
        // Toggling a sheet ON makes it the scroll-wheel target and resets its
        // shown height to the registry default (matching the adapter's seed).
        if (row->is_sheet && row->on) {
            row->height = d->y_frac;
            row->hstr = sheet_hstr(d->y_frac);
            app.last_sheet_id = static_cast<std::uint32_t>(d->id);
            app.has_last_sheet = true;
        }
        h.DirtyVariable("overlay_columns");
    });
    // Force render-style selector (Forces column): v[0] = 0..3 (Arrows / Heatmap
    // / Flow / Glyphs). Updates the bound `force_style` (lighting the active
    // button) and pushes SetForceStyle so all four Force overlays re-render in the
    // chosen style — live even while paused (SetForceStyle is a frame-refresh write).
    ctor.BindEventCallback("set_force_style", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                     const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        int style = 0;
        v[0].GetInto(style);
        if (style < 0 || style >= static_cast<int>(ftd::native::ForceStyle::Count)) style = 0;
        app.data->force_style = style;
        push_scale0(&app, ftd::native::SetForceStyle{static_cast<std::uint32_t>(style)});
        h.DirtyVariable("force_style");
    });
    // Rubber-sheet height nudge: the panel's −/＋ affordance for one active sheet.
    // v[0] = overlay name, v[1] = "-" or "+". Steps the slice height by ±0.05 via
    // the shared nudge helper (clamps, updates the row, pushes SetSheetHeight).
    ctor.BindEventCallback("sheet_height_nudge", [&app](Rml::DataModelHandle, Rml::Event&,
                                                        const Rml::VariantList& v) {
        if (v.size() < 2 || !app.data) return;
        const Rml::String name = v[0].Get<Rml::String>();
        const Rml::String dir = v[1].Get<Rml::String>();
        OverlayRow* row = find_overlay_row(app.data, name);
        if (!row) return;
        nudge_sheet_height(&app, row, dir == "+" ? 0.05f : -0.05f);
    });
    // Collapse/expand one overlay column (its header is the affordance). Pure
    // view-state; flips `expanded` and re-lays the column list via data-if.
    ctor.BindEventCallback("toggle_overlay_col", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                        const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const Rml::String title = v[0].Get<Rml::String>();
        for (OverlayColumnRow& col : app.data->overlay_columns) {
            if (col.title == title) { col.expanded = !col.expanded; break; }
        }
        h.DirtyVariable("overlay_columns");
    });
    // Setup scenario picker — expand/collapse the whole list ("Scenarios ▾"
    // header). Collapsing DROPS every row (scenario_groups.clear()) so the DOM
    // holds ~0 scenario <div>s; expanding rebuilds the list from the catalog and
    // re-applies the active search filter + current-row highlight. This is the
    // DOM-shrink half of the framerate fix: the ~130-item list only exists while
    // the picker is open.
    ctor.BindEventCallback("toggle_scn_picker", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                       const Rml::VariantList&) {
        if (!app.data) return;
        app.data->scn_open = !app.data->scn_open;
        if (app.data->scn_open) {
            rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter,
                                  app.scn_expanded_groups);
        } else {
            app.data->scenario_groups.clear();  // collapse → 0 scenario rows in the DOM
        }
        h.DirtyVariable("scn_open");
        h.DirtyVariable("scenario_groups");
    });
    // Setup scenario picker — select one scenario. Issues the LoadScenario core
    // command (the exact path Reset uses) so the host reboots the engine into it
    // (a LIVE switch; overlays/scene refresh on the reload). Optimistically moves
    // the current-row highlight and records the id as the new Reset target; the
    // per-frame sync confirms it against the effective (possibly W9-corrected)
    // scenario the host actually loaded.
    ctor.BindEventCallback("select_scenario", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                     const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string id = std::string(v[0].Get<Rml::String>().c_str());
        set_current_scenario(app.data, id);
        app.scenario_id = id;
        push_core(&app, ftd::native::LoadScenario{id});
        h.DirtyVariable("scenario_groups");
    });
    // Collapse/expand one scenario category group (its header is the affordance).
    // Flips the group's membership in the expanded set and rebuilds the view, so
    // an expanded group instantiates its rows and a collapsed group drops them.
    ctor.BindEventCallback("toggle_scenario_group", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                           const Rml::VariantList& v) {
        if (v.empty() || !app.data) return;
        const std::string title(v[0].Get<Rml::String>().c_str());
        auto& ex = app.scn_expanded_groups;
        auto it = std::find(ex.begin(), ex.end(), title);
        if (it == ex.end()) ex.push_back(title);
        else ex.erase(it);
        rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter, ex);
        h.DirtyVariable("scenario_groups");
    });
    // Live search over the picker: the change event carries the input's new text.
    // Rebuilds the view keeping only matching rows (matching groups force-open),
    // so typing SHRINKS the live DOM to the matches rather than hiding 130 rows.
    ctor.BindEventCallback("filter_scenarios", [&app](Rml::DataModelHandle h, Rml::Event& ev,
                                                      const Rml::VariantList&) {
        if (!app.data) return;
        app.scenario_filter = std::string(ev.GetParameter<Rml::String>("value", "").c_str());
        rebuild_scenario_view(app.data, app.scenario_id, app.scenario_filter,
                              app.scn_expanded_groups);
        h.DirtyVariable("scenario_groups");
    });
    // Inspector close affordance (the × in the readout header): drop the current
    // selection so the GUI loop stops re-issuing the inspect command and hides
    // the section next frame. Clear the bound fields here too for immediacy.
    ctor.BindEventCallback("clear_inspect", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                   const Rml::VariantList&) {
        app.inspect_kind = 0;
        app.inspect_pidx = -1;
        app.insp_has_data = false;
        if (app.data && app.data->insp_active) {
            app.data->insp_active = false;
            app.data->insp_lines.clear();
            app.data->insp_neigh_show = false;
            app.data->insp_faces.clear();
            app.data->insp_edges.clear();
            app.data->insp_corners.clear();
            h.DirtyVariable("insp_active");
            h.DirtyVariable("insp_lines");
            h.DirtyVariable("insp_neigh_show");
            h.DirtyVariable("insp_faces");
            h.DirtyVariable("insp_edges");
            h.DirtyVariable("insp_corners");
        }
    });
    // Collapse/expand the 26-neighbour grid. Closing DROPS the three cell vectors
    // (0 DOM) and — because the app only issues InspectNeighbors while the grid is
    // open — also stops the 26-read gather; opening lets the next boundary refill
    // both. Same collapse-saves-work discipline as the physics/telemetry panels.
    ctor.BindEventCallback("toggle_insp_neigh", [&app](Rml::DataModelHandle h, Rml::Event&,
                                                       const Rml::VariantList&) {
        if (!app.data) return;
        app.data->insp_neigh_open = !app.data->insp_neigh_open;
        if (!app.data->insp_neigh_open) {
            app.data->insp_faces.clear();
            app.data->insp_edges.clear();
            app.data->insp_corners.clear();
            h.DirtyVariable("insp_faces");
            h.DirtyVariable("insp_edges");
            h.DirtyVariable("insp_corners");
        }
        h.DirtyVariable("insp_neigh_open");
    });
    // Neighbour walk: click a 26-neighbour cell to move the inspection cursor to
    // that voxel — the current inspected cell + the clicked cell's Moore offset
    // (v = dx, dy, dz). Clamped to the lattice; the loop's inspect_retarget path
    // then re-issues InspectVoxel/Force/Neighbors for the new target so the readout
    // + grid refresh immediately. Scale-0 voxel inspections only (kind == 1).
    ctor.BindEventCallback("walk_neigh", [&app](Rml::DataModelHandle, Rml::Event&,
                                                const Rml::VariantList& v) {
        if (v.size() < 3) return;
        int dx = 0, dy = 0, dz = 0;
        v[0].GetInto(dx);
        v[1].GetInto(dy);
        v[2].GetInto(dz);
        walk_inspection(&app, dx, dy, dz);
    });
    Rml::DataModelHandle model = ctor.GetModelHandle();
    // Publish the handle to the app so wnd_proc's scroll-wheel height nudge can
    // dirty the overlay panel (the RML event callbacks get their own handle).
    app.model = model;
    app.model_ready = true;

    // ── Status-bar data model (fps fix) ────────────────────────────────────────
    // The status readouts move to a SEPARATE document (statusbar.rml) with their
    // own data model, so their ~8/s dirties reflow only that 27dp strip — never
    // this document's 92-row scenario picker (measured: a status tick reflowed the
    // whole shell at 107ms layout + 62ms geometry → 5.7 fps with the picker open).
    // Bound to the SAME ShellData fields; the per-frame loop dirties this handle
    // for status-only vars, and both handles for the two vars a shell element also
    // binds (the Scale-1 readout) — but only while Scale 1 is active.
    Rml::DataModelConstructor sctor = context->CreateDataModel("status");
    if (!sctor) throw std::runtime_error("CreateDataModel(status) failed");
    sctor.Bind("running", &data.running);
    sctor.Bind("tick", &data.tick);
    sctor.Bind("particle_count", &data.particle_count);
    sctor.Bind("total_energy", &data.total_energy);
    sctor.Bind("physical_time", &data.physical_time);
    sctor.Bind("fps", &data.fps);
    sctor.Bind("backend", &data.backend);
    Rml::DataModelHandle status_model = sctor.GetModelHandle();

    // ── CLI-driven physics-control state (headless proof of the panel) ──────────
    // Seed the live caches from the host's boot snapshot so the pre-opened panel
    // shows real engine values from frame 0 (the per-frame sync keeps them live).
    if (app_opts.scale == 0) {
        if (auto boot = host.publisher().acquire()) {
            if (const ftd::native::Scale0Snapshot* s0 = boot->scale0()) {
                app.live_toggles = s0->term_toggles;
                app.live_knobs = s0->knobs;
                app.have_live = true;
                app.backend_cpu = (s0->env.backend == ftd::native::BackendKindUi::Cpu);
            }
        }
        // --open-physics: open the section (categories collapsed by default — 4
        // headers whose counts sum to 44). --expand-all-tog / --expand-tog-group
        // expand the requested categories so their toggle rows show.
        if (app_opts.open_physics) {
            data.phys_open = true;
            app.tog_expanded_groups.clear();
            if (app_opts.expand_all_tog) {
                for (const char* c : kToggleCategories)
                    app.tog_expanded_groups.emplace_back(c);
            } else {
                for (const std::string& g : app_opts.expand_tog_groups)
                    app.tog_expanded_groups.push_back(g);
            }
            rebuild_toggle_groups(&data, app.live_toggles, app.tog_expanded_groups);
        }
        if (app_opts.open_config) {
            data.cfg_open = true;
            build_config_rows(&data, app.live_toggles, app.live_knobs);
        }
        // Simulated control edits — the SAME commands the toggle clicks / config
        // nudges push, so the captured snapshot reflects them (control works).
        for (const std::string& n : app_opts.toggles_on) {
            if (ftd::term_toggles_detail::find_spec(n))
                push_scale0(&app, ftd::native::SetToggle{n, true});
            else std::cerr << "native_app: unknown toggle '" << n << "' (ignored)\n" << std::flush;
        }
        for (const std::string& n : app_opts.toggles_off) {
            if (ftd::term_toggles_detail::find_spec(n))
                push_scale0(&app, ftd::native::SetToggle{n, false});
            else std::cerr << "native_app: unknown toggle '" << n << "' (ignored)\n" << std::flush;
        }
        if (app_opts.set_dt) push_scale0(&app, ftd::native::SetDt{app_opts.dt_value});
        if (app_opts.set_sor) push_scale0(&app, ftd::native::SetSorIterations{app_opts.sor_value});
        if (app_opts.set_boundary) {
            const int b = std::clamp(app_opts.boundary_value, 0, 2);
            app.run_config.flux_boundary = b;
            push_scale0(&app, ftd::native::SetBoundary{static_cast<ftd::FluxBoundaryMode>(b)});
        }
        if (app_opts.set_lattice > 0) request_lattice_reboot(&app, app_opts.set_lattice);
    }

    // Telemetry: pre-open the section for a headless capture if requested, then
    // publish the initial demand. DIAGNOSTICS is demanded regardless of the section
    // state (cheap cadence-1 base charts); AUDIT + LAGRANGIAN follow when the
    // section is open. Issued unconditionally (any start scale) so a later switch
    // to Scale 0 already has a live demand on the host.
    if (app_opts.open_telemetry) data.tel_open = true;
    if (app_opts.open_overlays) data.ov_open = true;
    request_telemetry_demand(&app, data.tel_open);

    Rml::ElementDocument* doc = context->LoadDocument(FTD_RML_SHELL_PATH);
    if (!doc) throw std::runtime_error("LoadDocument(shell.rml) failed");
    doc->Show();

    // Load the status bar as a SECOND document, overlaying the shell's
    // #status-spacer. Its path sits beside shell.rml (same ui/rml dir), so it is
    // derived from FTD_RML_SHELL_PATH rather than needing its own CMake define. A
    // load failure is non-fatal — the app still runs, just without the status bar.
    {
        std::string sp = FTD_RML_SHELL_PATH;
        const auto pos = sp.rfind("shell.rml");
        if (pos != std::string::npos) sp.replace(pos, sizeof("shell.rml") - 1, "statusbar.rml");
        if (Rml::ElementDocument* sdoc = context->LoadDocument(sp)) {
            sdoc->Show();
        } else {
            std::cerr << "native_app: LoadDocument(statusbar.rml) failed at '" << sp
                      << "' - status bar disabled\n" << std::flush;
        }
    }

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
    // True once a device-resident interop gather has EVER landed this session.
    // Gates the 0-particle-field arm-gate fallback below: if interop is enabled
    // but no gather ever lands (a pure-field scenario — where the rubber sheets
    // shine), arm the capture a short grace past the warmup instead of hanging to
    // the 40 s deadline. Once a gather DOES land, this stays true and the arm
    // reverts to the exact prior behavior (wait for draw_interop_count > 0).
    bool interop_gathered_once = false;
    int frame_no = 0;
    // Capture output path: --png-out overrides the compiled default so a single
    // build can emit distinct captures (e.g. fields_vec.png vs fields_scalar.png).
    const std::string png_out =
        app_opts.png_out.empty() ? std::string(FTD_APP_PNG_OUT) : app_opts.png_out;

    // Telemetry ring-buffer feed bookkeeping (GUI thread). chart_series_scale
    // tracks the scale the buffer currently holds (reset the trace on a switch);
    // last_pushed_seq dedups pushes to one scalar per published snapshot.
    int chart_series_scale = initial_scale;
    std::uint64_t last_pushed_seq = 0;
    bool pushed_any = false;

    // Setup-picker highlight sync (GUI thread). Tracks the last scenario the
    // picker was highlighted for so the current-row highlight follows the
    // effective (host-authoritative) scenario after a Reset, initial boot, live
    // pick, or a W9 validation-reject that changed the loaded id.
    std::string synced_scenario = initial_scenario;

    // Click-to-inspect bookkeeping (GUI thread). last_inspect_seq dedups the
    // per-boundary re-issue to one inspect command per new snapshot;
    // last_inspect_scale drops the selection on a scale switch.
    std::uint64_t last_inspect_seq = 0;
    int last_inspect_scale = initial_scale;
    // The 26-neighbour gather is heavier (26 synchronous reads) than the single
    // voxel/force reads, and its data does not need full-rate refresh. Re-issue it
    // at a throttled cadence (~1 in NEIGH_STRIDE boundaries) while the voxel/force
    // readout stays live every boundary; the latch keeps the grid populated in
    // between. last_neigh_seq is reset to 0 on a new pick to force an immediate
    // first fill.
    std::uint64_t last_neigh_seq = 0;
    constexpr std::uint64_t NEIGH_STRIDE = 6;

    // The interop StructuredBuffer SRV (heap slot 0) only needs binding ONCE for
    // the lifetime of the never-recreated shared buffer. This catch-up covers both
    // the startup-active case and a later inactive→active reload transition; a
    // D3D12 presenter call must stay on this (GUI) thread, so it lives here rather
    // than in the sim-thread reload block above.
    bool interop_srv_bound = false;

    // ── Cosmetic status-bar throttle (GUI thread) ───────────────────────────
    // The continuously-changing status readouts (tick, particle count, fps,
    // physical time, total/S1 energy) are pushed into the data model on a ~120 ms
    // cadence rather than every frame. Each such push DirtyVariable()s a bound
    // field, and a single dirty status field forces RmlUi to re-lay-out the whole
    // document and regenerate geometry on the next Context::Update()/Render();
    // at 60..145 fps that full reflow ran up to ~145×/s. Throttling caps the
    // status-driven reflow near ~8/s. Everything that changes on USER ACTION or
    // structure (scenario, toggles, overlays, inspector, sheet heights,
    // active_scale, running) stays IMMEDIATE below — those writes are all
    // change-guarded, so a steady state costs nothing.
    constexpr auto kStatusPushInterval = std::chrono::milliseconds(120);
    auto last_status_push = std::chrono::steady_clock::now();
    bool status_first = true;

    // ── UI-profiling accumulators (--profile-ui) ──────────────────────────────
    // Per-frame Update()/render() wall-times, split by whether the status push
    // dirtied a bound field that frame. With a static scenario + telemetry closed,
    // the status push is the ONLY thing that dirties, so "status-push" vs "quiet"
    // frames isolate reflow-on-dirty from the steady-state (nothing-dirty) cost.
    std::vector<double> prof_upd_push, prof_upd_quiet, prof_rnd_push, prof_rnd_quiet;
    auto prof_stats = [](std::vector<double> v) {
        struct S { std::size_t n; double mean, p50, p95, mx; };
        if (v.empty()) return S{0, 0.0, 0.0, 0.0, 0.0};
        std::sort(v.begin(), v.end());
        double sum = 0.0;
        for (double x : v) sum += x;
        auto pct = [&](double p) {
            return v[std::min(v.size() - 1,
                              static_cast<std::size_t>(p * (v.size() - 1) + 0.5))];
        };
        return S{v.size(), sum / static_cast<double>(v.size()), pct(0.5), pct(0.95),
                 v.back()};
    };

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
        // Status-bar dirties target the SEPARATE status document's model, so they
        // reflow only that 27dp strip and never this document's 92-row picker.
        // sset_* = status-only vars. bset_* = a var a shell element ALSO binds (the
        // Scale-1 readout): dirty the shell handle too, but only while that binding
        // is live (Scale 1 active) — so a Scale-0 status tick never touches the
        // picker. `running` is shared with the always-present toolbar, so it dirties
        // both unconditionally (it flips rarely — not a per-frame reflow driver).
        auto sset_int = [&](const char* name, int& dst, int val) {
            if (dst != val) { dst = val; status_model.DirtyVariable(name); }
        };
        auto sset_str = [&](const char* name, Rml::String& dst, const std::string& val) {
            if (dst != val) { dst = val; status_model.DirtyVariable(name); }
        };
        auto bset_int = [&](const char* name, int& dst, int val, bool also_shell) {
            if (dst != val) {
                dst = val; status_model.DirtyVariable(name);
                if (also_shell) model.DirtyVariable(name);
            }
        };
        auto bset_str = [&](const char* name, Rml::String& dst, const std::string& val,
                            bool also_shell) {
            if (dst != val) {
                dst = val; status_model.DirtyVariable(name);
                if (also_shell) model.DirtyVariable(name);
            }
        };
        auto bset_bool = [&](const char* name, bool& dst, bool val) {
            if (dst != val) {
                dst = val; status_model.DirtyVariable(name); model.DirtyVariable(name);
            }
        };

        // Throttle gate for the cosmetic status readouts (see kStatusPushInterval).
        const auto now_status = std::chrono::steady_clock::now();
        bool push_status =
            status_first || (now_status - last_status_push) >= kStatusPushInterval;
        // Profiling control: force the ~8/s status push OFF so a large expanded list
        // stays STATIC (no DirtyVariable). If fps then recovers, the cost was
        // reflow-on-status-dirty (single-document coupling), not the DOM's presence.
        if (app_opts.profile_freeze_status) push_status = false;
        if (push_status) { last_status_push = now_status; status_first = false; }

        if (push_status) sset_int("tick", data.tick, frame.tick);
        if (snap) set_int("active_scale", data.active_scale, snap->active_scale);
        if (push_status)
            bset_int("particle_count", data.particle_count,
                     static_cast<int>(frame.total_manifested), data.active_scale == 1);
        bset_bool("running", data.running, !paused.load());
        if (push_status) sset_int("fps", data.fps, smoothed_fps);
        set_str("scenario", data.scenario, frame.scenario.empty() ? app.scenario_id
                                                                   : frame.scenario);
        // Keep the picker's current-row highlight on the actually-loaded scenario
        // (Reset / initial boot / live pick / W9-corrected id). Idempotent; dirties
        // only when a highlight flag actually flips.
        {
            const std::string eff =
                frame.scenario.empty() ? app.scenario_id : frame.scenario;
            if (eff != synced_scenario) {
                synced_scenario = eff;
                if (set_current_scenario(&data, eff))
                    model.DirtyVariable("scenario_groups");
            }
        }
        sset_str("backend", data.backend, upper(frame.backend.empty() ? host.backend_name()
                                                                       : frame.backend));
        set_str("lattice", data.lattice, std::to_string(frame.lattice_size));
        if (push_status)
            sset_str("physical_time", data.physical_time,
                     fmt("%.2e s", static_cast<double>(frame.tick) * kTPhysSeconds));
        // Reflect the adapter's authoritative sheet slice heights (frame-carried)
        // in the panel rows, so a CLI --sheet-height or an adapter clamp shows up
        // live. Only dirties when a value actually changed (steady state = free).
        {
            bool sheets_changed = false;
            for (const ftd::native::NativeSheetHeight& sh : frame.sheet_heights) {
                const auto* d = ftd::native::overlay_by_id(sh.overlay_id);
                if (!d) continue;
                OverlayRow* r = find_overlay_row(&data, Rml::String(d->name));
                if (!r) continue;
                if (std::abs(r->height - sh.height) > 1.0e-4f) {
                    r->height = sh.height;
                    r->hstr = sheet_hstr(sh.height);
                    sheets_changed = true;
                }
            }
            if (sheets_changed) model.DirtyVariable("overlay_columns");
        }
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
            if (push_status)
                bset_str("total_energy", data.total_energy,
                         fmt("%.1f", s0->energy_ledger.E_curr), data.active_scale == 1);

            // ── Physics-control live sync (authoritative engine truth) ──
            // Mirror the full TermToggles + published knobs into the app caches
            // (every control callback reads its "current" value from these), then
            // refresh whatever view rows are currently built. All writes are
            // change-guarded, so a steady state costs nothing and never dirties
            // the (reflow-heavy) bindings.
            app.live_toggles = s0->term_toggles;
            app.live_knobs = s0->knobs;
            app.have_live = true;
            app.backend_cpu = (s0->env.backend == ftd::native::BackendKindUi::Cpu);

            bool toggles_changed = false;
            for (ToggleGroupRow& g : data.toggle_groups)
                for (FullToggleRow& r : g.items)
                    if (refresh_toggle_row(r, s0->term_toggles)) toggles_changed = true;
            if (toggles_changed) model.DirtyVariable("toggle_groups");

            if (refresh_config_rows(&data, s0->term_toggles, s0->knobs))
                model.DirtyVariable("config_rows");

            // Live validation banner: the engine's own validate() (+ CPU runtime
            // warnings), so an invalid combo the user builds is surfaced rather
            // than silently stderr-warned. Change-guarded (rarely non-empty).
            std::string verr;
            s0->term_toggles.validate(&verr);
            if (app.backend_cpu) {
                const std::string warn = s0->term_toggles.cpu_runtime_warnings();
                if (!warn.empty()) {
                    if (!verr.empty()) verr += "\n";
                    verr += warn;
                }
            }
            // Surface just the first line (the banner is one row).
            std::string vfirst = verr.substr(0, verr.find('\n'));
            const bool has_v = !vfirst.empty();
            if (data.has_validation != has_v) {
                data.has_validation = has_v;
                model.DirtyVariable("has_validation");
            }
            set_str("validation_msg", data.validation_msg, vfirst);

            // ── Telemetry legend / current-value + freshness (Scale-0) ──
            // Throttled with the cosmetic status readouts and only while the
            // section is open (the values live behind data-if="tel_open", and a
            // dirtied bound field forces a document reflow — the same fps guard the
            // status bar uses). The values come from the published Scale0Snapshot
            // telemetry channels the scheduler now fills; the "t=" provenance is the
            // per-group sampled tick, so a slow audit group reads its own freshness
            // rather than the fast diagnostics tick.
            if (data.tel_open && push_status) {
                const ftd::TelemetrySnapshot& tm = s0->telemetry;
                const ftd::Diagnostics& dg = tm.diagnostics;
                const ftd::EnergyAudit& au = tm.audit;
                const ftd::TelemetryLagrangian& lg = tm.lagrangian;
                set_str("tel_d_energy", data.tel_d_energy,
                        fmt("%.2f", s0->energy_ledger.E_curr));
                set_str("tel_d_manif", data.tel_d_manif,
                        std::to_string(dg.manifested_count));
                set_str("tel_d_entropy", data.tel_d_entropy,
                        fmt("%.3f", dg.total_entropy));
                set_str("tel_d_charge", data.tel_d_charge,
                        std::to_string(dg.positive_count - dg.negative_count));
                set_str("tel_diag_prov", data.tel_diag_prov,
                        "t=" + std::to_string(tm.diagnostics_meta.tick));
                set_str("tel_a_energy", data.tel_a_energy,
                        fmt("%.2f", au.total_energy));
                set_str("tel_a_drift", data.tel_a_drift,
                        fmt("%+.4f", s0->energy_ledger.dE_dt));
                set_str("tel_a_gauss", data.tel_a_gauss,
                        fmt("%.2e", au.gauss_violation));
                set_str("tel_audit_prov", data.tel_audit_prov,
                        "t=" + std::to_string(tm.audit_meta.tick));
                set_str("tel_l_lag", data.tel_l_lag,
                        fmt("%.2f", lg.total_lagrangian));
                set_str("tel_l_ham", data.tel_l_ham,
                        fmt("%.2f", lg.total_hamiltonian));
            }
        } else if (const ftd::native::Scale1Snapshot* s1 = snap ? snap->scale1() : nullptr) {
            chart_energy = s1->total_energy;
            chart_energy_valid = true;
            if (push_status) {
                // Scale-1 branch (active_scale == 1): total_energy shows in both the
                // status strip and the Scale-1 readout; s1_ke/s1_pe are shell-only.
                bset_str("total_energy", data.total_energy, fmt("%.3f", s1->total_energy),
                         true);
                set_str("s1_ke", data.s1_ke, fmt("%.3f", s1->total_ke));
                set_str("s1_pe", data.s1_pe, fmt("%.3f", s1->total_pe));
            }
        }

        // ── Feed the telemetry ring buffers (snapshot-only, GUI thread) ──
        // Reset every series on a scale switch so Scale-0 telemetry is never
        // plotted next to Scale-1 telemetry; push exactly one scalar per NEW
        // published snapshot (dedup by seq — the GUI loop runs faster than the sim
        // tick). The DIAGNOSTICS channels are fed on every Scale-0 boundary so the
        // base charts are already populated when the section is opened; the heavier
        // AUDIT + LAGRANGIAN channels are fed only while the section is open (their
        // scheduler groups are only demanded then).
        if (snap) {
            if (snap->active_scale != chart_series_scale) {
                diag_energy.clear(); diag_manif.clear(); diag_entropy.clear();
                diag_charge.clear();
                aud_energy.clear(); aud_drift.clear(); aud_gauss.clear();
                lag_lag.clear(); lag_ham.clear();
                chart_series_scale = snap->active_scale;
                last_pushed_seq = 0;
                pushed_any = false;
            }
            if (chart_energy_valid && (!pushed_any || snap->seq != last_pushed_seq)) {
                // Energy trace is dual-scale (Scale-0 accounted E / Scale-1 total).
                diag_energy.push(static_cast<float>(chart_energy));
                if (const ftd::native::Scale0Snapshot* s0 = snap->scale0()) {
                    const ftd::Diagnostics& dg = s0->telemetry.diagnostics;
                    diag_manif.push(static_cast<float>(dg.manifested_count));
                    diag_entropy.push(static_cast<float>(dg.total_entropy));
                    diag_charge.push(
                        static_cast<float>(dg.positive_count - dg.negative_count));
                    if (data.tel_open) {
                        const ftd::EnergyAudit& au = s0->telemetry.audit;
                        aud_energy.push(static_cast<float>(au.total_energy));
                        aud_drift.push(static_cast<float>(s0->energy_ledger.dE_dt));
                        aud_gauss.push(static_cast<float>(au.gauss_violation));
                        const ftd::TelemetryLagrangian& lg = s0->telemetry.lagrangian;
                        lag_lag.push(static_cast<float>(lg.total_lagrangian));
                        lag_ham.push(static_cast<float>(lg.total_hamiltonian));
                    }
                }
                last_pushed_seq = snap->seq;
                pushed_any = true;
            }
        }

        // ── Click-to-inspect (GUI thread) ────────────────────────────────────
        // (a) resolve a pending scene click into a selection (unproject + pick),
        // (b) re-issue the inspect command once per new boundary so the readout
        // stays LIVE, (c) mirror the published inspection into the data model.
        // Runs before Context::Update() so the readout dirties lay out this frame;
        // the pick reuses the previous frame's viewport_rect (stable frame-to-
        // frame, and always valid once the shell has laid out at least once).
        const int cur_scale = snap ? snap->active_scale : data.active_scale;
        // Capture-mode auto-scroll (no wheel available headlessly): reveal the
        // click-to-inspect readout + telemetry — now in the LEFT #setup panel —
        // and the overlays/config at the bottom of the RIGHT #physics panel, so a
        // composited shot shows the moved content. A live pick sets only
        // scroll_setup_bottom (reveal the inspector) so it does not yank the
        // properties panel out from under the user.
        bool scroll_setup_bottom = capture_mode && !app_opts.no_scroll;
        bool scroll_physics_bottom = capture_mode && !app_opts.no_scroll;
        if (cur_scale != last_inspect_scale) {
            // Scale switch: the old target index is meaningless on the new scale.
            app.inspect_kind = 0;
            app.inspect_pidx = -1;
            app.insp_has_data = false;
            last_inspect_scale = cur_scale;
            last_inspect_seq = 0;
            last_neigh_seq = 0;
        }
        if (app.pick_pending) {
            app.pick_pending = false;
            const PickRay ray =
                make_pick_ray(camera, app.viewport_rect, app.pick_x, app.pick_y);
            if (cur_scale == 0) {
                const int L = frame.lattice_size > 0 ? frame.lattice_size : camera_lattice;
                int vx = 0, vy = 0, vz = 0;
                if (pick_scale0(frame, ray, L, vx, vy, vz)) {
                    app.inspect_kind = 1;
                    app.inspect_vx = vx;
                    app.inspect_vy = vy;
                    app.inspect_vz = vz;
                    app.insp_has_data = false;      // fresh target — reset the latch
                    scroll_setup_bottom = true;     // reveal the readout (left panel)
                } else {
                    app.inspect_kind = 0;           // empty space → clear
                }
            } else if (cur_scale == 1) {
                int pidx = -1;
                if (pick_scale1(frame, ray, pidx)) {
                    app.inspect_kind = 2;
                    app.inspect_pidx = pidx;
                    app.insp_has_data = false;
                    scroll_setup_bottom = true;   // reveal the readout (left panel)
                } else {
                    app.inspect_kind = 0;
                }
            }
            last_inspect_seq = 0;   // force an immediate re-issue for the new target
            last_neigh_seq = 0;
        }
        // A neighbour-cell walk retargeted the inspection: force an immediate
        // re-issue for the new voxel (the callback already moved inspect_v{x,y,z}).
        if (app.inspect_retarget) {
            app.inspect_retarget = false;
            last_inspect_seq = 0;
            last_neigh_seq = 0;
        }
        // Re-issue the inspect command once per NEW published snapshot so the
        // adapter refreshes the inspection payload every boundary (live data).
        if (snap && app.inspect_kind != 0 && snap->seq != last_inspect_seq) {
            if (app.inspect_kind == 1) {
                // Voxel + force readouts stay live at full rate (one cheap read each).
                push_scale0(&app, ftd::native::InspectVoxel{app.inspect_vx, app.inspect_vy,
                                                            app.inspect_vz});
                push_scale0(&app, ftd::native::InspectForce{app.inspect_vx, app.inspect_vy,
                                                            app.inspect_vz});
                // The 26-neighbour gather (26 reads) is heavier and only issued
                // while the grid is open, at a throttled cadence — the latch keeps
                // the cells populated between refreshes.
                if (data.insp_neigh_open
                    && snap->seq - last_neigh_seq >= NEIGH_STRIDE) {
                    push_scale0(&app, ftd::native::InspectNeighbors{
                                          app.inspect_vx, app.inspect_vy, app.inspect_vz});
                    last_neigh_seq = snap->seq;
                }
            } else if (app.inspect_kind == 2) {
                push_scale1(&app, ftd::native::InspectParticle1{app.inspect_pidx});
            }
            last_inspect_seq = snap->seq;
        }
        // Mirror the published inspection into the bound readout fields.
        if (app.inspect_kind == 0) {
            if (data.insp_active) {
                data.insp_active = false;
                data.insp_lines.clear();
                data.insp_neigh_show = false;
                data.insp_faces.clear();
                data.insp_edges.clear();
                data.insp_corners.clear();
                model.DirtyVariable("insp_active");
                model.DirtyVariable("insp_lines");
                model.DirtyVariable("insp_neigh_show");
                model.DirtyVariable("insp_faces");
                model.DirtyVariable("insp_edges");
                model.DirtyVariable("insp_corners");
            }
        } else {
            Rml::String title;
            Rml::Vector<InspLine> lines;
            bool have_now = false;   // this snapshot carries fresh inspection data
            if (app.inspect_kind == 1) {
                title = fmt3("Voxel (%.0f, %.0f, %.0f)",
                             static_cast<double>(app.inspect_vx),
                             static_cast<double>(app.inspect_vy),
                             static_cast<double>(app.inspect_vz));
                const ftd::native::Scale0Snapshot* s0 = snap ? snap->scale0() : nullptr;
                if (s0 && s0->voxel_present) {
                    const ftd::VoxelInspection& vi = s0->voxel;
                    const ftd::Voxel& vx = vi.voxel;
                    const ftd::Vec3& J = vx.flux;
                    const double cmag = vi.curl.mag();
                    // ── formatting helpers ──
                    auto vec3s = [](const ftd::Vec3& a) {
                        return fmt3("%.3f, %.3f, %.3f", a.x, a.y, a.z);
                    };
                    auto hdr = [&lines](const char* t) {
                        lines.push_back(InspLine{t, "", true});
                    };
                    auto row = [&lines](const char* k, std::string v) {
                        lines.push_back(InspLine{k, std::move(v), false});
                    };
                    auto spin_lbl = [](std::int8_t s) -> std::string {
                        return s > 0 ? "up (+1)" : s < 0 ? "down (-1)" : "none (0)";
                    };
                    auto color_lbl = [](std::int8_t c) -> std::string {
                        switch (c) {
                            case 1: return "red (1)";
                            case 2: return "green (2)";
                            case 3: return "blue (3)";
                            default: return "colorless (0)";
                        }
                    };
                    auto flavor_lbl = [](std::int8_t f) -> std::string {
                        switch (f) {
                            case 1: return "e (1)";
                            case 2: return "mu (2)";
                            case 3: return "tau (3)";
                            default: return "none (0)";
                        }
                    };
                    // ── core (the original 5 rows, kept at the top) ──
                    row("State", std::to_string(static_cast<int>(vx.state)));
                    row("Flux J", vec3s(J));
                    row("|J|", fmt("%.4f", J.mag()));
                    row("Div J", fmt("%.4f", vi.divergence));
                    row("|Curl|", fmt("%.4f", cmag));
                    // ── field / kinematics ──
                    hdr("Field / kinematics");
                    row("Wave vel", vec3s(vx.wave_vel));
                    row("Velocity", vec3s(vx.velocity));
                    row("|v|", fmt("%.4f", vx.speed()));
                    row("Remainder", vec3s(vx.remainder));
                    // ── EM decomposition (E = -dJ/dt, B = curl J) ──
                    hdr("EM (E=-dJ/dt, B=curl J)");
                    row("E", vec3s(vi.em.E));
                    row("|E|", fmt("%.4f", vi.em.E_mag));
                    row("B", vec3s(vi.em.B));
                    row("|B|", fmt("%.4f", vi.em.B_mag));
                    row("Curl J", vec3s(vi.curl));
                    // ── gravity / clock ──
                    hdr("Gravity / clock");
                    row("Latency L", fmt("%.4f", vx.latency));
                    row("Tau", fmt("%.4f", vx.tau));
                    row("Phase (de Broglie)", fmt("%.4f", vx.phase));
                    row("Accel |a|", fmt("%.4f", vx.accel_mag));
                    // ── quantum numbers / identity ──
                    hdr("Quantum numbers / identity");
                    row("Spin", spin_lbl(vx.spin));
                    row("Color", color_lbl(vx.color));
                    row("Flavor", flavor_lbl(vx.flavor));
                    row("Particle id", std::to_string(vx.particle_id));
                    row("Pair id", std::to_string(vx.pair_id));
                    row("Locked", vx.locked ? "yes" : "no");
                    // ── dual substrate (J = J_L + J_R) ──
                    hdr("Dual substrate (J = J_L + J_R)");
                    row("Flux L", vec3s(vx.flux_L));
                    row("Flux R", vec3s(vx.flux_R));
                    row("Wave L", vec3s(vx.wave_vel_L));
                    row("Wave R", vec3s(vx.wave_vel_R));
                    row("Chirality", fmt("%.4f", vx.chirality_density()));
                    // ── strong / weak substrate ──
                    hdr("Strong / weak substrate");
                    row("Flux strong", vec3s(vx.flux_strong));
                    row("Wave strong", vec3s(vx.wave_vel_strong));
                    row("Flux weak", vec3s(vx.flux_weak));
                    row("Wave weak", vec3s(vx.wave_vel_weak));
                    // ── force channels (from InspectForce) ──
                    if (s0->force_present) {
                        auto force_row = [&lines](const char* name, const ftd::Vec3& f) {
                            lines.push_back(InspLine{
                                name,
                                fmt("%.4f", f.mag())
                                    + fmt3("  (%.3f, %.3f, %.3f)", f.x, f.y, f.z),
                                false});
                        };
                        hdr("Forces (|F| + direction)");
                        force_row("Coulomb", s0->force.f_coulomb);
                        force_row("Strong", s0->force.f_strong);
                        force_row("Magnetic", s0->force.f_magnetic);
                        force_row("Gravity", s0->force.f_gravity);
                        force_row("Exchange", s0->force.f_exchange);
                    }
                    have_now = true;
                }
            } else if (app.inspect_kind == 2) {
                title = "Particle #" + std::to_string(app.inspect_pidx);
                const ftd::native::Scale1Snapshot* s1 = snap ? snap->scale1() : nullptr;
                if (s1 && s1->insp_present) {
                    const double vmag = std::sqrt(s1->insp_vel[0] * s1->insp_vel[0]
                                                  + s1->insp_vel[1] * s1->insp_vel[1]
                                                  + s1->insp_vel[2] * s1->insp_vel[2]);
                    const std::string chg = (s1->insp_charge >= 0 ? "+" : "")
                                            + std::to_string(s1->insp_charge);
                    lines.push_back(InspLine{"Charge", chg});
                    lines.push_back(InspLine{"Pos", fmt3("%.2f, %.2f, %.2f", s1->insp_pos[0],
                                                         s1->insp_pos[1], s1->insp_pos[2])});
                    lines.push_back(InspLine{"Vel", fmt3("%.3f, %.3f, %.3f", s1->insp_vel[0],
                                                         s1->insp_vel[1], s1->insp_vel[2])});
                    lines.push_back(InspLine{"|v|", fmt("%.4f", vmag)});
                    lines.push_back(InspLine{"Locked", s1->insp_locked ? "yes" : "no"});
                    have_now = true;
                }
            }
            if (data.insp_title != title) {
                data.insp_title = title;
                model.DirtyVariable("insp_title");
            }
            // Refresh the lines only on a snapshot that actually carries the
            // inspection (they change every boundary — live data). Between the
            // sparse re-issues keep the last values; show "reading..." only until
            // the very first payload for this target arrives.
            if (have_now) {
                data.insp_lines = std::move(lines);
                model.DirtyVariable("insp_lines");
                app.insp_has_data = true;
            } else if (!app.insp_has_data) {
                data.insp_lines.clear();
                data.insp_lines.push_back(InspLine{"", "reading..."});
                model.DirtyVariable("insp_lines");
            }
            if (!data.insp_active) {
                data.insp_active = true;
                model.DirtyVariable("insp_active");
            }
            // ── 26-Moore-neighbour grid ──────────────────────────────────────
            // Only a Scale-0 voxel has a lattice neighbourhood; a Scale-1 particle
            // hides the whole sub-section. When the grid is open and this snapshot
            // carries a fresh gather (they land ~1/NEIGH_STRIDE boundaries), bucket
            // the 26 cells by shell (face/edge/corner) into the bound vectors; the
            // latch keeps the last cells between throttled refreshes.
            if (app.inspect_kind == 1) {
                if (!data.insp_neigh_show) {
                    data.insp_neigh_show = true;
                    model.DirtyVariable("insp_neigh_show");
                }
                const ftd::native::Scale0Snapshot* s0n = snap ? snap->scale0() : nullptr;
                if (data.insp_neigh_open && s0n && s0n->neighbors_present) {
                    Rml::Vector<InspNeighCell> faces, edges, corners;
                    for (const ftd::native::NeighborCell& nc : s0n->neighbors) {
                        if (!nc.present) continue;
                        std::string dir;
                        auto ax = [&dir](int d, char a) {
                            if (d > 0) { dir += '+'; dir += a; }
                            else if (d < 0) { dir += '-'; dir += a; }
                        };
                        ax(nc.dx, 'X');
                        ax(nc.dy, 'Y');
                        ax(nc.dz, 'Z');
                        const bool voidcell = nc.state == 0 && nc.flux_mag < 1e-6;
                        const char* glyph = nc.state > 0 ? "+" : nc.state < 0 ? "-" : "0";
                        std::string val = std::string(glyph) + "  "
                            + (voidcell ? std::string("\xE2\x80\x94")  // em dash
                                        : fmt("%.3f", nc.flux_mag));
                        InspNeighCell cell{dir, val, static_cast<int>(nc.state),
                                           nc.dx, nc.dy, nc.dz};
                        if (nc.shell == 1) faces.push_back(std::move(cell));
                        else if (nc.shell == 2) edges.push_back(std::move(cell));
                        else corners.push_back(std::move(cell));
                    }
                    data.insp_faces = std::move(faces);
                    data.insp_edges = std::move(edges);
                    data.insp_corners = std::move(corners);
                    model.DirtyVariable("insp_faces");
                    model.DirtyVariable("insp_edges");
                    model.DirtyVariable("insp_corners");
                }
            } else {
                if (data.insp_neigh_show) {
                    data.insp_neigh_show = false;
                    model.DirtyVariable("insp_neigh_show");
                }
                if (!data.insp_faces.empty() || !data.insp_edges.empty()
                    || !data.insp_corners.empty()) {
                    data.insp_faces.clear();
                    data.insp_edges.clear();
                    data.insp_corners.clear();
                    model.DirtyVariable("insp_faces");
                    model.DirtyVariable("insp_edges");
                    model.DirtyVariable("insp_corners");
                }
            }
        }

        // Lay out, then map the #viewport hole rect for the scene + input.
        const auto t_upd0 = std::chrono::steady_clock::now();
        context->Update();
        const double upd_ms = std::chrono::duration<double, std::milli>(
                                  std::chrono::steady_clock::now() - t_upd0)
                                  .count();
        double render_ms = 0.0;   // set inside the render block below (profiling)
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

        // Both side panels can overflow the body-row height: the LEFT #setup
        // panel stacks the scenario picker + play controls + telemetry charts +
        // inspector; the RIGHT #physics panel stacks PHYSICS TERMS + CONFIG +
        // FIELD overlays. Interactively they scroll by wheel; in a headless
        // capture (no wheel) and on a fresh pick, scroll the relevant panel to the
        // bottom so the inspector (left) / overlays (right) come into view.
        // Post-Update so GetScrollHeight is valid; SetScrollTop dirties the child
        // offsets, which render() recomputes, so the same frame shows the scroll.
        // No-op when the panel fits.
        if (scroll_setup_bottom) {
            if (Rml::Element* setup = doc->GetElementById("setup"))
                setup->SetScrollTop(setup->GetScrollHeight());
        }
        if (scroll_physics_bottom) {
            if (Rml::Element* phys = doc->GetElementById("physics"))
                phys->SetScrollTop(phys->GetScrollHeight());
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
            // shows before the first gather completes.
            if (draw_interop_count > 0) interop_gathered_once = true;
            // Arm-gate fallback (pure-field scenarios): if interop is enabled but
            // NO gather has ever landed a short grace past the warmup, arm anyway
            // (the CPU particle path — empty here — still supplies the frame), so a
            // 0-particle field capture writes a PNG instead of hitting the 40 s
            // deadline. Disabled the instant a gather lands (interop_gathered_once),
            // so behavior is unchanged whenever particles DO gather.
            constexpr int kInteropGraceFrames = 60;
            const bool interop_zero_particle_grace =
                interop_active.load() && !interop_gathered_once
                && frame_no >= app_opts.capture_frames + kInteropGraceFrames;
            const bool interop_ready_or_na =
                !interop_active.load() || draw_interop_count > 0
                || interop_zero_particle_grace;
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
                              << ", fps=" << smoothed_fps
                              << ", lattice=" << frame.lattice_size
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
                const auto t_rnd0 = std::chrono::steady_clock::now();
                presenter.render(frame, camera, view_opts, draw_interop_count);
                render_ms = std::chrono::duration<double, std::milli>(
                                std::chrono::steady_clock::now() - t_rnd0)
                                .count();
            } catch (const std::exception& ex) {
                std::cerr << "render threw at frame " << frame_no << ": " << ex.what() << "\n"
                          << std::flush;
                running.store(false);
                sim.join();
                throw;
            }
        }
        if (app_opts.profile_ui_frames > 0) {
            (push_status ? prof_upd_push : prof_upd_quiet).push_back(upd_ms);
            (push_status ? prof_rnd_push : prof_rnd_quiet).push_back(render_ms);
        }
        ++frame_no;
        if (app_opts.profile_ui_frames > 0 && frame_no >= app_opts.profile_ui_frames) {
            const double wall = std::chrono::duration<double>(
                                    std::chrono::steady_clock::now() - loop_start)
                                    .count();
            auto line = [](const char* label, auto s) {
                char buf[160];
                std::snprintf(buf, sizeof(buf),
                              "  %s  n=%llu mean=%.3f p50=%.3f p95=%.3f max=%.3f ms\n",
                              label, static_cast<unsigned long long>(s.n), s.mean,
                              s.p50, s.p95, s.mx);
                std::cout << buf;
            };
            std::cout << "\n=== UI PROFILE  frames=" << frame_no
                      << "  wall=" << fmt("%.1f", wall) << "s"
                      << "  fps_avg=" << fmt("%.1f", frame_no / (wall > 0 ? wall : 1))
                      << (app_opts.profile_freeze_status ? "  [status FROZEN]" : "")
                      << "  scn_open=" << (data.scn_open ? 1 : 0)
                      << "  expand_cat=" << app_opts.scn_expand_cat << " ===\n";
            line("Update() push ", prof_stats(prof_upd_push));
            line("Update() quiet", prof_stats(prof_upd_quiet));
            line("render() push ", prof_stats(prof_rnd_push));
            line("render() quiet", prof_stats(prof_rnd_quiet));
            std::cout.flush();
            quit = true;
        }

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
