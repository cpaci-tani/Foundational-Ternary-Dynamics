add_library(ftd_native_session STATIC
    src/engine_session.cpp
)
target_include_directories(ftd_native_session
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ftd_native_session PUBLIC ftd_core)

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
endif()
