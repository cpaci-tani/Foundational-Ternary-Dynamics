# ============================================================================
# FtdAddTest.cmake — unified test target + CTest registration macro
# ============================================================================
#
# Purpose:
#   Replace the add_executable / target_link_libraries / add_test triple that
#   repeats ~200 times in engine/CMakeLists.txt with a single call. Also
#   classifies tests as CPU or GPU so the FTD test runner (engine/tools/
#   test_runner/) can apply a smart dispatcher: serial GPU queue, parallel
#   CPU queue.
#
# Usage:
#
#   # Simple unit test (CPU). ctest name is derived by stripping "test_".
#   ftd_add_test(test_gauss tests/test_gauss.cpp)
#   #   → add_executable(test_gauss tests/test_gauss.cpp)
#   #   → target_link_libraries(test_gauss PRIVATE ftd_core)
#   #   → add_test(NAME gauss COMMAND test_gauss)
#
#   # Campaign test. ctest name is derived by replacing "ftd_" with "campaign_".
#   ftd_add_test(ftd_dispersion tests/campaign_dispersion.cpp)
#   #   → add_test(NAME campaign_dispersion COMMAND ftd_dispersion)
#
#   # GPU-heavy test. Links ftd_cuda when CUDA is enabled, applies the "gpu"
#   # CTest label so SmartDispatcher serializes it.
#   ftd_add_test(test_gpu_physics tests/test_gpu_physics.cpp GPU_HEAVY)
#
#   # Explicit ctest name override (when the derivation rule doesn't fit).
#   ftd_add_test(test_confinement tests/test_confinement.cpp
#                CTEST_NAME confinement_test)
#
#   # No ftd_core link (header-only tests like the trit_* library).
#   ftd_add_test(test_trit_packing tests/test_trit_packing.cpp NO_CORE)
#
# Options:
#   GPU_HEAVY    — needs dedicated CUDA device; links ftd_cuda, gets "gpu"
#                  CTest label, runner dispatches serially.
#   NO_CORE      — does not link ftd_core (header-only libraries).
#   CTEST_NAME   — explicit CTest registration name (default: derived from target)
#   TIMEOUT      — CTest timeout in seconds (default: 300; runner honors this)
#   LABELS       — additional CTest labels (beyond automatic "gpu" / "unit" /
#                  "campaign"; space-separated list)
#
# Side effects:
#   Registers a global CMake property FTD_ALL_TESTS that accumulates every
#   test name, so the runner can read it via CMake's file(READ) on
#   CMakeCache.txt / CTestTestfile.cmake for its test tree. This is a
#   convenience — the primary discovery mechanism is still
#   "ctest --show-only=json-v1".

if(NOT COMMAND cmake_parse_arguments)
    include(CMakeParseArguments)
endif()

function(ftd_add_test target source)
    set(options GPU_HEAVY NO_CORE)
    set(one_value_args CTEST_NAME TIMEOUT)
    set(multi_value_args LABELS)
    cmake_parse_arguments(FAT "${options}" "${one_value_args}" "${multi_value_args}" ${ARGN})

    # Derive CTest name if not explicitly provided.
    #   test_foo → foo
    #   ftd_foo  → campaign_foo
    if(NOT FAT_CTEST_NAME)
        set(FAT_CTEST_NAME "${target}")
        if(FAT_CTEST_NAME MATCHES "^test_")
            string(REGEX REPLACE "^test_" "" FAT_CTEST_NAME "${FAT_CTEST_NAME}")
        elseif(FAT_CTEST_NAME MATCHES "^ftd_")
            string(REGEX REPLACE "^ftd_" "campaign_" FAT_CTEST_NAME "${FAT_CTEST_NAME}")
        endif()
    endif()

    # Build the executable.
    add_executable(${target} ${source})
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/include)

    # Link libraries.
    if(NOT FAT_NO_CORE)
        if(FAT_GPU_HEAVY AND FTD_ENABLE_CUDA)
            target_link_libraries(${target} PRIVATE ftd_core ftd_cuda)
        else()
            target_link_libraries(${target} PRIVATE ftd_core)
        endif()
    endif()

    # Phase 7 (2026-04-27): every test target links ftd_test_support so the
    # test_telemetry impl (extracted from the header) is available. NO_CORE
    # targets still need this — the test_telemetry symbols live here, not
    # in ftd_core. The directory-scope link_libraries(ftd_test_support) call
    # in CMakeLists.txt covers tests defined via add_executable, but the
    # ftd_add_test macro must link it explicitly because the call to this
    # function may precede the link_libraries() statement (e.g. from a
    # subdirectory) — defensive belt-and-suspenders.
    if(TARGET ftd_test_support)
        target_link_libraries(${target} PRIVATE ftd_test_support)
    endif()

    # Register with CTest.
    add_test(NAME ${FAT_CTEST_NAME} COMMAND ${target})

    # Automatic label: "gpu" for GPU_HEAVY tests.
    if(FAT_GPU_HEAVY)
        set_property(TEST ${FAT_CTEST_NAME} APPEND PROPERTY LABELS "gpu")
    endif()

    # Automatic label: "unit" or "campaign" based on source filename.
    get_filename_component(source_name "${source}" NAME)
    if(source_name MATCHES "^test_")
        set_property(TEST ${FAT_CTEST_NAME} APPEND PROPERTY LABELS "unit")
    elseif(source_name MATCHES "^campaign_")
        set_property(TEST ${FAT_CTEST_NAME} APPEND PROPERTY LABELS "campaign")
    elseif(source_name MATCHES "^benchmark_")
        set_property(TEST ${FAT_CTEST_NAME} APPEND PROPERTY LABELS "benchmark")
    endif()

    # User-supplied labels.
    foreach(label ${FAT_LABELS})
        set_property(TEST ${FAT_CTEST_NAME} APPEND PROPERTY LABELS "${label}")
    endforeach()

    # Timeout.
    if(FAT_TIMEOUT)
        set_tests_properties(${FAT_CTEST_NAME} PROPERTIES TIMEOUT ${FAT_TIMEOUT})
    endif()

    # Accumulate test name for runner consumption.
    set_property(GLOBAL APPEND PROPERTY FTD_ALL_TESTS "${FAT_CTEST_NAME}")
endfunction()

# ----------------------------------------------------------------------------
# Helper: apply gpu label to a list of existing tests (for incremental migration
# before all tests are moved to ftd_add_test).
# ----------------------------------------------------------------------------
function(ftd_mark_gpu_tests)
    foreach(test_name ${ARGN})
        if(TEST ${test_name})
            set_property(TEST ${test_name} APPEND PROPERTY LABELS "gpu")
        endif()
    endforeach()
endfunction()
