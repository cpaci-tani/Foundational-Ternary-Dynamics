add_library(ftd_native_session STATIC
    src/engine_session.cpp
)
target_include_directories(ftd_native_session
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ftd_native_session PUBLIC ftd_core)

if(FTD_ENABLE_CUDA)
    target_link_libraries(ftd_native_session PUBLIC ftd_cuda)
endif()

if(WIN32)
    add_executable(ftd_native_desktop
        src/main.cpp
        src/d3d12_presenter.cpp
    )
    target_include_directories(ftd_native_desktop PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
    target_link_libraries(ftd_native_desktop PRIVATE
        ftd_native_session
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
        # Same ftd_test_support cross-directory-ordering hazard as the other
        # native_desktop tests above: compile test_telemetry.cpp directly
        # into this binary instead of linking ftd_test_support.
    endif()
endif()
