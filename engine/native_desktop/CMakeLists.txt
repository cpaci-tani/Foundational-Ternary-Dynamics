add_library(ftd_native_ui_model STATIC
    src/command_queue.cpp
    src/snapshot_publisher.cpp
)
target_include_directories(ftd_native_ui_model
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ftd_native_ui_model PUBLIC ftd_core)

add_library(ftd_native_session STATIC
    src/engine_session.cpp
    src/command_applier.cpp
    src/parameter_journal.cpp
    src/ui_snapshot_builder.cpp
)
target_include_directories(ftd_native_session
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ftd_native_session PUBLIC ftd_core ftd_native_ui_model)

if(FTD_ENABLE_CUDA)
    target_link_libraries(ftd_native_session PUBLIC ftd_cuda)
endif()

if(WIN32)
    add_library(ftd_native_windowing STATIC
        src/dpi_support.cpp
    )
    target_include_directories(ftd_native_windowing PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

    add_executable(ftd_native_desktop
        src/main.cpp
        src/d3d12_presenter.cpp
    )
    target_include_directories(ftd_native_desktop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(ftd_native_desktop PRIVATE
        ftd_native_session
        ftd_native_windowing
        d3d12
        dxgi
        d3dcompiler
        comctl32
    )

    # NOTE: the ftd_test_support static library (which owns the compiled
    # tests/support/test_telemetry.cpp — see engine/CMakeLists.txt "Phase 7"
    # comment) is defined in the *parent* CMakeLists.txt, and that
    # add_library() call happens after add_subdirectory(native_desktop). A
    # target_link_libraries(... ftd_test_support) reference from here would
    # therefore be a forward reference to a not-yet-defined target. Rather
    # than depend on cross-directory ordering, compile the (self-contained,
    # ftd_core-independent) test_telemetry.cpp translation unit directly into
    # this test binary.
    add_executable(test_d3d12_adapter_selection
        tests/test_d3d12_adapter_selection.cpp
        src/d3d12_presenter.cpp
        ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
    )
    target_include_directories(test_d3d12_adapter_selection PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_d3d12_adapter_selection PRIVATE
        ftd_native_session
        ftd_core
        d3d12
        dxgi
        d3dcompiler
    )
    # CTest registration (add_test/set_tests_properties) is done from the
    # parent engine/CMakeLists.txt, not here — see the comment there. This
    # directory's add_subdirectory(native_desktop) call runs before
    # enable_testing() in the parent, so a CTestTestfile.cmake is never
    # generated for this directory and add_test() calls placed here are
    # silently dropped (verified: engine/build/native_desktop/ has no
    # CTestTestfile.cmake even after a full configure).

    # Task 4: D3D12_HEAP_FLAG_SHARED particle buffer + NT handle export.
    # D3D12-only (no CUDA import on this side), so unlike
    # test_cuda_d3d12_adapter_match below it does not need to be gated by
    # FTD_ENABLE_CUDA -- it lives in this same unconditional if(WIN32) block
    # as test_d3d12_adapter_selection above. Same
    # cross-directory-ftd_test_support-ordering hazard as that test, so
    # test_telemetry.cpp is compiled directly into this binary too.
    add_executable(test_d3d12_shared_buffer
        tests/test_d3d12_shared_buffer.cpp
        src/d3d12_presenter.cpp
        ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
    )
    target_include_directories(test_d3d12_shared_buffer PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_d3d12_shared_buffer PRIVATE
        ftd_native_session
        ftd_core
        d3d12
        dxgi
        d3dcompiler
    )

    # Regression coverage for D3D12Presenter::render() itself (Task 11 frame-
    # in-flight fencing + the immediately-following vb/cb double-buffering
    # fix) -- see test_d3d12_render_frame_fencing.cpp's file-level comment.
    # render() is otherwise only called from main.cpp's message loop, so
    # without this target that logic had zero automated coverage. D3D12-only
    # (interop_particle_count stays 0, the CPU vertex-buffer path), so like
    # test_d3d12_shared_buffer above it does not need FTD_ENABLE_CUDA gating
    # and lives in this same unconditional if(WIN32) block. Same cross-
    # directory-ftd_test_support-ordering hazard as the other tests here, so
    # test_telemetry.cpp is compiled directly into this binary too.
    add_executable(test_d3d12_render_frame_fencing
        tests/test_d3d12_render_frame_fencing.cpp
        src/d3d12_presenter.cpp
        ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
    )
    target_include_directories(test_d3d12_render_frame_fencing PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_d3d12_render_frame_fencing PRIVATE
        ftd_native_session
        ftd_core
        d3d12
        dxgi
        d3dcompiler
    )

    # Phase 0A: Release-capable D3D12 debug-layer switch plus inspectable
    # ID3D12InfoQueue. Same HWND+presenter compilation pattern as the other
    # device tests; CTest registration is in the parent CMakeLists.txt.
    add_executable(test_d3d12_debug_observability
        tests/test_d3d12_debug_observability.cpp
        src/d3d12_presenter.cpp
        ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
    )
    target_include_directories(test_d3d12_debug_observability PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_d3d12_debug_observability PRIVATE
        ftd_native_session
        ftd_core
        d3d12
        dxgi
        d3dcompiler
    )

    add_executable(test_native_desktop_dpi_awareness
        tests/test_native_desktop_dpi_awareness.cpp
        ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
    )
    target_include_directories(test_native_desktop_dpi_awareness PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(test_native_desktop_dpi_awareness PRIVATE
        ftd_native_windowing
        ftd_core
    )

    if(FTD_ENABLE_CUDA)
        # Cross-checks the D3D12 hardware adapter (Task 1,
        # select_hardware_adapter()) against the CUDA device's LUID
        # (Task 2, GpuEngine::device_luid()) to confirm both APIs are
        # targeting the same physical GPU. Needs ftd_cuda + CUDA toolkit
        # headers reachable via GpuEngine, so it's gated the same way
        # ftd_native_session's ftd_cuda link above is.
        add_executable(test_cuda_d3d12_adapter_match
            tests/test_cuda_d3d12_adapter_match.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_cuda_d3d12_adapter_match PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_cuda_d3d12_adapter_match PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )
        # Same ftd_test_support cross-directory-ordering hazard as
        # test_d3d12_adapter_selection above: compile test_telemetry.cpp
        # directly into this binary instead of linking ftd_test_support
        # (which is defined in the parent CMakeLists.txt after
        # add_subdirectory(native_desktop) runs).

        # Task 5: round-trips a D3D12_HEAP_FLAG_SHARED buffer (Task 4) into a
        # CUDA-mapped device pointer via GpuEngine::import_d3d12_particle_buffer()
        # (this task). Needs ftd_cuda the same way test_cuda_d3d12_adapter_match
        # above does, so it lives in the same FTD_ENABLE_CUDA block.
        add_executable(test_cuda_import_shared_buffer
            tests/test_cuda_import_shared_buffer.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_cuda_import_shared_buffer PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_cuda_import_shared_buffer PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )
        # Same ftd_test_support cross-directory-ordering hazard as the other
        # native_desktop tests above: compile test_telemetry.cpp directly
        # into this binary instead of linking ftd_test_support.

        # Task 6: reuses the flags/prefix-scan/header selection pass from
        # launch_visual_particle_capture but gathers fully-decoded
        # InteropParticleRecords straight into the buffer imported by Task 5
        # (test_cuda_import_shared_buffer above), instead of raw
        # VisualParticleRecords for later CPU decode. Same FTD_ENABLE_CUDA
        # gating as that test, for the same reason (needs ftd_cuda).
        add_executable(test_interop_gather
            tests/test_interop_gather.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_interop_gather PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_interop_gather PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )

        # Task 7: cross-API GPU-timeline fence. Round-trips a
        # D3D12_FENCE_FLAG_SHARED fence (D3D12Presenter::create_shared_fence())
        # into a CUDA external semaphore (GpuEngine::import_d3d12_fence()),
        # signals it after the Task 6 gather kernel
        # (GpuEngine::interop_signal_fence(), via interop_gather_particles()'s
        # fence_value argument), and confirms D3D12's command queue actually
        # blocks on it (D3D12Presenter::wait_shared_fence()). Same
        # FTD_ENABLE_CUDA gating as test_interop_gather above (needs ftd_cuda).
        add_executable(test_interop_fence_roundtrip
            tests/test_interop_fence_roundtrip.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_interop_fence_roundtrip PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_interop_fence_roundtrip PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )

        # Task 9 round-2 code review: NativeEngineSession::boot() must clear
        # interop_enabled_ on every reload (apply_options() /
        # load_scenario() / set_lattice_size() / reset_current()), or a
        # 'stale interop_enabled_ after reload' regression silently reappears
        # -- see test_interop_reload_reset.cpp's own header comment. Drives
        # NativeEngineSession's public API directly (not GpuEngine) so it
        # needs ftd_native_session + a real D3D12Presenter, same as the other
        # interop tests in this FTD_ENABLE_CUDA block; no direct ftd_cuda
        # symbols are used, but it stays gated the same way since the
        # assertions it makes are meaningless without a CUDA-backed session.
        add_executable(test_interop_reload_reset
            tests/test_interop_reload_reset.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_interop_reload_reset PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_interop_reload_reset PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )

        # Interop Task 10: the design doc's own stated validation bar for
        # Component B -- compare rendered particle positions/colors between
        # the interop gather kernel (Task 6) and the pre-interop CPU capture
        # path (NativeEngineSession::capture()) for the same session/tick,
        # not just "looks right". Needs ftd_native_session (for
        # NativeEngineSession::debug_gpu_engine()) and ftd_cuda (for
        # GpuEngine::debug_read_interop_records()), same FTD_ENABLE_CUDA
        # gating as the rest of this block.
        add_executable(test_interop_visual_parity
            tests/test_interop_visual_parity.cpp
            src/d3d12_presenter.cpp
            ${CMAKE_SOURCE_DIR}/tests/support/test_telemetry.cpp
        )
        target_include_directories(test_interop_visual_parity PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
        )
        target_link_libraries(test_interop_visual_parity PRIVATE
            ftd_native_session
            ftd_cuda
            d3d12
            dxgi
            d3dcompiler
        )
    endif()
endif()
