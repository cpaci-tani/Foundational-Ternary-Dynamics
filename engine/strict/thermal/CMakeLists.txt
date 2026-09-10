cmake_minimum_required(VERSION 3.20)
project(ftd_finite_thermal_candidate LANGUAGES CXX)

# Standalone selected research target. No legacy core, UI, CUDA runtime, or
# alternate dashboard owner is linked or enabled by this build.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()
add_library(ftd_finite_thermal_candidate STATIC finite_thermal.cpp)
target_compile_features(ftd_finite_thermal_candidate PUBLIC cxx_std_17)
target_include_directories(ftd_finite_thermal_candidate PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(ftd_finite_thermal_candidate PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_executable(ftd_finite_thermal_tests native_tests.cpp)
target_link_libraries(ftd_finite_thermal_tests PRIVATE ftd_finite_thermal_candidate)

# V2 is an isolated compile of the borrowed-state kernel. Both libraries use
# the same C++ namespace; never link them into the same program. The public
# definition keeps the consumer's LAW_ID consistent with its selected library.
add_library(ftd_finite_thermal_v2_candidate STATIC finite_thermal.cpp)
target_compile_features(ftd_finite_thermal_v2_candidate PUBLIC cxx_std_17)
target_include_directories(ftd_finite_thermal_v2_candidate PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(ftd_finite_thermal_v2_candidate PUBLIC FTD_THERMAL_EQUIVARIANT=1)
set_target_properties(ftd_finite_thermal_v2_candidate PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_executable(ftd_finite_thermal_v2_tests native_tests.cpp)
target_link_libraries(ftd_finite_thermal_v2_tests PRIVATE ftd_finite_thermal_v2_candidate)

foreach(target IN ITEMS ftd_finite_thermal_candidate ftd_finite_thermal_tests
                        ftd_finite_thermal_v2_candidate ftd_finite_thermal_v2_tests)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
enable_testing()
add_test(NAME finite_thermal_native_contracts COMMAND ftd_finite_thermal_tests)
add_test(NAME finite_thermal_v2_native_contracts COMMAND ftd_finite_thermal_v2_tests)
set_tests_properties(finite_thermal_native_contracts finite_thermal_v2_native_contracts PROPERTIES
    LABELS "candidate;finite;thermal;unit" TIMEOUT 120 PROCESSORS 1)
