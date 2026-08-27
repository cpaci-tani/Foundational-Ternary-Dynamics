# CTest scheduling hygiene (2026-07-17): prevent the `ctest -j 32` timeout
# cascade. Every test binary is OpenMP-parallel and otherwise defaults to one
# thread per logical core. Keep the aggregate declared load bounded:
#
# - GPU tests and tests with TIMEOUT >= 1200 get an exclusive 32-slot budget.
# - Other tests use 8 threads/slots, allowing four-way concurrency on the
#   canonical 32-thread host.
#
# Hosted four-core runners invoke ctest with a sufficiently large job budget;
# PROCESSORS is resource accounting, not a literal hardware requirement.
get_property(_ftd_all_tests DIRECTORY PROPERTY TESTS)
foreach(_ftd_test IN LISTS _ftd_all_tests)
    get_test_property(${_ftd_test} TIMEOUT _ftd_timeout)
    if(NOT _ftd_timeout)
        set(_ftd_timeout 0)
    endif()
    if(_ftd_test MATCHES "gpu" OR _ftd_timeout GREATER_EQUAL 1200)
        set(_ftd_processors 32)
    else()
        set(_ftd_processors 8)
    endif()
    set_property(TEST ${_ftd_test} PROPERTY PROCESSORS ${_ftd_processors})
    set_property(TEST ${_ftd_test} APPEND PROPERTY
                 ENVIRONMENT "OMP_NUM_THREADS=${_ftd_processors}")
endforeach()
unset(_ftd_all_tests)
unset(_ftd_processors)
unset(_ftd_test)
unset(_ftd_timeout)
