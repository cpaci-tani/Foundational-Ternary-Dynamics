# engine/native — clean-slate native rebuild.
#
# The UI is a custom Direct3D 12 + Direct2D/DirectWrite layer built in the
# engine's own device (owner decision 2026-08-20 — no Dear ImGui). This tree
# therefore carries only the ImGui-free salvage spine: the D3D12 presenter +
# CUDA<->D3D12 interop, the transport (command queue / snapshot publisher /
# journal), and the engine session. The shell/panels/theme are rebuilt on the
# custom UI in a later phase. Distinct native_* target names let this tree
# coexist with native_desktop until that tree is retired.
#
# See native/docs/PLAN_NATIVE_REBUILD.md + SPEC_NATIVE_REBUILD_R0R1.md.

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

    # ── native_windowing: DPI ──
    add_library(native_windowing STATIC src/dpi_support.cpp)
    target_include_directories(native_windowing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

    # ── Device / interop test executables ──
    # Each compiles the presenter (ImGui-free — takes an opaque OverlayRecorder)
    # plus the self-contained test_telemetry.cpp TU directly (ftd_test_support is
    # defined in the parent AFTER this add_subdirectory runs). CTest add_test
    # registration lives in the parent engine/CMakeLists.txt.
    set(NATIVE_TT "${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp")

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

    # DPI awareness (windowing only).
    add_executable(native_test_dpi_awareness
        tests/test_native_desktop_dpi_awareness.cpp ${NATIVE_TT})
    target_include_directories(native_test_dpi_awareness PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(native_test_dpi_awareness PRIVATE native_windowing ftd_core)

    # CUDA<->D3D12 interop device tests.
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
