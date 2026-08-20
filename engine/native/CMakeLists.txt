# engine/native — clean-slate native rebuild (R0 salvage transplant).
#
# Mirrors the proven engine/native_desktop target structure under distinct
# `native_*` target names so both trees coexist until native_desktop is retired
# (owner decision 2026-08-20). Vendored ImGui/ImPlot live at engine/thirdparty
# and are shared; each tree compiles its own static lib with its own config.
#
# See native_desktop/docs/SPEC_NATIVE_REBUILD_R0R1.md (salvage map + layout) and
# PLAN_NATIVE_REBUILD.md (architecture + phases). R0.1 transplants the working
# app verbatim (renamed ftd::native_desktop -> ftd::native); the Scale-0-shaped
# internals are rewritten scale-generic in R1.

set(NATIVE_IMGUI_DIR "${CMAKE_SOURCE_DIR}/thirdparty/imgui")
set(NATIVE_IMPLOT_DIR "${CMAKE_SOURCE_DIR}/thirdparty/implot")

# ── native_imgui: Dear ImGui + ImPlot, no backend (presenter-only tests stay
#    ImGui-free). Config via native/ftd_imconfig.h. ──
add_library(native_imgui STATIC
    ${NATIVE_IMGUI_DIR}/imgui.cpp
    ${NATIVE_IMGUI_DIR}/imgui_draw.cpp
    ${NATIVE_IMGUI_DIR}/imgui_tables.cpp
    ${NATIVE_IMGUI_DIR}/imgui_widgets.cpp
    ${NATIVE_IMPLOT_DIR}/implot.cpp
    ${NATIVE_IMPLOT_DIR}/implot_items.cpp
    src/imgui_assert.cpp
    src/imgui_font.cpp
)
target_include_directories(native_imgui
    PUBLIC
        ${NATIVE_IMGUI_DIR}
        ${NATIVE_IMPLOT_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
)
target_compile_definitions(native_imgui PUBLIC
    "IMGUI_USER_CONFIG=\"native/ftd_imconfig.h\""
)
if(MSVC)
    target_compile_options(native_imgui PRIVATE /W3)
endif()

# ── native_cli: command-line options ──
add_library(native_cli STATIC src/cli_options.cpp)
target_include_directories(native_cli PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_cli PUBLIC ftd_core)

# ── native_model: transport spine (command queue, snapshot publisher) ──
add_library(native_model STATIC
    src/command_queue.cpp
    src/snapshot_publisher.cpp
)
target_include_directories(native_model PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_model PUBLIC ftd_core)

# ── native_ui: shell + theme + workspace + palette + panels (no D3D12, no engine) ──
add_library(native_ui STATIC
    src/ui/theme.cpp
    src/ui/panel_registry.cpp
    src/ui/workspace.cpp
    src/ui/command_palette.cpp
    src/ui/ui_shell.cpp
    src/ui/panels/scenario_browser.cpp
    src/ui/panels/run_config.cpp
    src/ui/panels/play_bar.cpp
    src/ui/panels/substrate.cpp
    src/ui/panels/telemetry.cpp
    src/ui/panels/audit.cpp
    src/ui/panels/lagrangian.cpp
    src/ui/panels/inspector.cpp
    src/ui/panels/physics_terms.cpp
    src/ui/panels/fields.cpp
    src/ui/panels/log.cpp
)
target_include_directories(native_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(native_ui PUBLIC native_imgui native_model)
if(MSVC)
    target_compile_options(native_ui PRIVATE /W3)
endif()

# ── native_session: engine ownership + command applier + journal + snapshot builder ──
add_library(native_session STATIC
    src/engine_session.cpp
    src/command_applier.cpp
    src/parameter_journal.cpp
    src/ui_snapshot_builder.cpp
)
target_include_directories(native_session PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(native_session PUBLIC ftd_core native_model)
if(FTD_ENABLE_CUDA)
    target_link_libraries(native_session PUBLIC ftd_cuda)
endif()

if(WIN32)
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        message(FATAL_ERROR
            "native D3D12/CUDA interop is Win64-only (got pointer size ${CMAKE_SIZEOF_VOID_P})")
    endif()

    # ── native_imgui_dx12: Win32 + DX12 backends + overlay recorder ──
    add_library(native_imgui_dx12 STATIC
        ${NATIVE_IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${NATIVE_IMGUI_DIR}/backends/imgui_impl_dx12.cpp
        src/imgui_overlay.cpp
    )
    target_include_directories(native_imgui_dx12
        PUBLIC
            ${NATIVE_IMGUI_DIR}/backends
            ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(native_imgui_dx12 PUBLIC native_imgui d3d12 dxgi dwmapi)
    if(MSVC)
        target_compile_options(native_imgui_dx12 PRIVATE /W3)
    endif()

    # ── native_windowing: DPI ──
    add_library(native_windowing STATIC src/dpi_support.cpp)
    target_include_directories(native_windowing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # ── native_app: the desktop executable (presenter is compiled in, ImGui-free) ──
    add_executable(native_app WIN32
        src/main.cpp
        src/d3d12_presenter.cpp
    )
    target_include_directories(native_app PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    target_link_libraries(native_app PRIVATE
        native_session
        native_windowing
        native_cli
        native_ui
        native_imgui_dx12
        d3d12
        dxgi
        d3dcompiler
        shell32
    )

    # ── Device / interop test executables ──
    # Compile the presenter + the self-contained test_telemetry.cpp TU directly
    # (ftd_test_support is defined in the parent AFTER this add_subdirectory
    # runs — a target_link_libraries here would be a forward reference). CTest
    # add_test registration lives in the parent engine/CMakeLists.txt.
    set(NATIVE_TT "${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp")

    # D3D12-only device tests (no CUDA import on this side).
    foreach(t IN ITEMS
        test_d3d12_adapter_selection
        test_d3d12_shared_buffer
        test_d3d12_render_frame_fencing
        test_d3d12_debug_observability
        test_d3d12_overlay_once
        test_d3d12_capture_lifecycle
    )
        add_executable(native_${t} tests/${t}.cpp src/d3d12_presenter.cpp ${NATIVE_TT})
        target_include_directories(native_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
        target_link_libraries(native_${t} PRIVATE native_session ftd_core d3d12 dxgi d3dcompiler)
    endforeach()

    # ImGui-in-D3D12 device test additionally needs the dx12 backend.
    add_executable(native_test_ui_imgui_d3d12
        tests/test_ui_imgui_d3d12.cpp src/d3d12_presenter.cpp ${NATIVE_TT})
    target_include_directories(native_test_ui_imgui_d3d12 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(native_test_ui_imgui_d3d12 PRIVATE
        native_session ftd_core native_imgui_dx12 d3d12 dxgi d3dcompiler)

    # DPI awareness (windowing only).
    add_executable(native_test_dpi_awareness
        tests/test_native_desktop_dpi_awareness.cpp ${NATIVE_TT})
    target_include_directories(native_test_dpi_awareness PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(native_test_dpi_awareness PRIVATE native_windowing ftd_core)

    # CUDA↔D3D12 interop device tests.
    if(FTD_ENABLE_CUDA)
        foreach(t IN ITEMS
            test_cuda_d3d12_adapter_match
            test_cuda_import_shared_buffer
            test_interop_gather
            test_interop_fence_roundtrip
            test_interop_reload_reset
            test_interop_visual_parity
        )
            add_executable(native_${t} tests/${t}.cpp src/d3d12_presenter.cpp ${NATIVE_TT})
            target_include_directories(native_${t} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
            target_link_libraries(native_${t} PRIVATE native_session ftd_cuda d3d12 dxgi d3dcompiler)
        endforeach()
    endif()
endif()
