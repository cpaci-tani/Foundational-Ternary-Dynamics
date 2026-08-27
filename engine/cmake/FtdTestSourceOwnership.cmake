# Every standalone test translation unit must have an explicit owner. A source
# is either compiled by a production/CTest target or deliberately quarantined
# in the frontier-research manifest. This configure-time gate prevents new
# research programs from silently becoming untracked build-system debt.
include(${CMAKE_SOURCE_DIR}/cmake/FtdFrontierResearchSources.cmake)

file(GLOB _ftd_test_cpp_sources CONFIGURE_DEPENDS
     RELATIVE ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/tests/*.cpp)
# `_repro_*.cpp` is the gitignored local-diagnostic convention. These files
# never ship to CI and therefore cannot be valid manifest or target sources.
list(FILTER _ftd_test_cpp_sources EXCLUDE REGEX "^tests/_repro_.*\\.cpp$")
get_property(_ftd_build_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
set(_ftd_owned_test_sources)
foreach(_ftd_target IN LISTS _ftd_build_targets)
    get_target_property(_ftd_target_sources ${_ftd_target} SOURCES)
    if(NOT _ftd_target_sources)
        continue()
    endif()
    foreach(_ftd_source IN LISTS _ftd_target_sources)
        if(IS_ABSOLUTE "${_ftd_source}")
            file(RELATIVE_PATH _ftd_relative_source
                 ${CMAKE_SOURCE_DIR} "${_ftd_source}")
        else()
            set(_ftd_relative_source "${_ftd_source}")
        endif()
        string(REPLACE "\\" "/" _ftd_relative_source
                       "${_ftd_relative_source}")
        if(_ftd_relative_source MATCHES "^tests/[^/]+\\.cpp$")
            list(APPEND _ftd_owned_test_sources "${_ftd_relative_source}")
        endif()
    endforeach()
endforeach()
list(REMOVE_DUPLICATES _ftd_owned_test_sources)

# Targets behind build options are absent from BUILDSYSTEM_TARGETS when their
# option is disabled. Their sources are still owned, not frontier research.
get_property(_ftd_conditional_test_sources GLOBAL
             PROPERTY FTD_CONDITIONAL_TEST_SOURCES)
if(NOT _ftd_conditional_test_sources)
    set(_ftd_conditional_test_sources)
endif()
list(REMOVE_DUPLICATES _ftd_conditional_test_sources)
foreach(_ftd_conditional_source IN LISTS _ftd_conditional_test_sources)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${_ftd_conditional_source}")
        message(FATAL_ERROR
            "Stale conditional test source: ${_ftd_conditional_source}")
    endif()
    if("${_ftd_conditional_source}" IN_LIST FTD_FRONTIER_RESEARCH_SOURCES)
        message(FATAL_ERROR
            "Conditional target source is also quarantined: "
            "${_ftd_conditional_source}")
    endif()
    list(APPEND _ftd_owned_test_sources "${_ftd_conditional_source}")
endforeach()
list(REMOVE_DUPLICATES _ftd_owned_test_sources)

set(_ftd_frontier_unique ${FTD_FRONTIER_RESEARCH_SOURCES})
list(LENGTH FTD_FRONTIER_RESEARCH_SOURCES _ftd_frontier_count)
list(REMOVE_DUPLICATES _ftd_frontier_unique)
list(LENGTH _ftd_frontier_unique _ftd_frontier_unique_count)
if(NOT _ftd_frontier_count EQUAL _ftd_frontier_unique_count)
    message(FATAL_ERROR
        "FtdFrontierResearchSources.cmake contains duplicate entries")
endif()

foreach(_ftd_frontier_source IN LISTS FTD_FRONTIER_RESEARCH_SOURCES)
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${_ftd_frontier_source}")
        message(FATAL_ERROR
            "Stale frontier research source: ${_ftd_frontier_source}")
    endif()
    if("${_ftd_frontier_source}" IN_LIST _ftd_owned_test_sources)
        message(FATAL_ERROR
            "Promoted test source remains quarantined: ${_ftd_frontier_source}")
    endif()
endforeach()

foreach(_ftd_test_source IN LISTS _ftd_test_cpp_sources)
    if(NOT "${_ftd_test_source}" IN_LIST _ftd_owned_test_sources
       AND NOT "${_ftd_test_source}" IN_LIST FTD_FRONTIER_RESEARCH_SOURCES)
        message(FATAL_ERROR
            "Unclassified test source: ${_ftd_test_source}. Add it to a build "
            "target or to FtdFrontierResearchSources.cmake.")
    endif()
endforeach()

list(LENGTH _ftd_test_cpp_sources _ftd_test_source_count)
list(LENGTH _ftd_conditional_test_sources _ftd_conditional_source_count)
message(STATUS
    "Engine test-source ownership: ${_ftd_test_source_count} total, "
    "${_ftd_frontier_count} explicitly quarantined, "
    "${_ftd_conditional_source_count} conditionally targeted")
unset(_ftd_build_targets)
unset(_ftd_conditional_source)
unset(_ftd_conditional_source_count)
unset(_ftd_conditional_test_sources)
unset(_ftd_frontier_count)
unset(_ftd_frontier_source)
unset(_ftd_frontier_unique)
unset(_ftd_frontier_unique_count)
unset(_ftd_owned_test_sources)
unset(_ftd_relative_source)
unset(_ftd_source)
unset(_ftd_target)
unset(_ftd_target_sources)
unset(_ftd_test_cpp_sources)
unset(_ftd_test_source)
