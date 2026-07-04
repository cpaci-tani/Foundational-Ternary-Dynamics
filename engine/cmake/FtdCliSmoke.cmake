# ============================================================================
# FtdCliSmoke.cmake — ftd_sim CLI smoke-test runner (revision 1.5).
#
# The ftd_sim CLI (engine/src/main.cpp + cli_demos) is a live research
# data-product path (CSV/VTK export bundles) that had zero automated
# coverage. This script runs one scenario route at tiny L/ticks and asserts
# (a) exit code 0 and (b) expected non-empty output files.
#
# Usage (from an add_test COMMAND):
#   cmake -DFTD_SIM=<exe> -DSCENARIO=H -DL=9 -DTICKS=5 -DOUTDIR=<dir>
#         [-DEXTRA_ARGS=2;1] [-DEXPECT_GLOB=timeseries.csv] -P FtdCliSmoke.cmake
# ============================================================================

if(NOT FTD_SIM OR NOT SCENARIO OR NOT L OR NOT TICKS)
    message(FATAL_ERROR "FtdCliSmoke: FTD_SIM, SCENARIO, L, TICKS are required")
endif()

# Fresh output dir per run so stale artifacts can't mask a broken exporter.
if(OUTDIR)
    file(REMOVE_RECURSE "${OUTDIR}")
    file(MAKE_DIRECTORY "${OUTDIR}")
endif()

set(_args "${SCENARIO}" "${L}" "${TICKS}")
if(OUTDIR)
    list(APPEND _args "${OUTDIR}")
endif()
if(EXTRA_ARGS)
    list(APPEND _args ${EXTRA_ARGS})
endif()

execute_process(
    COMMAND "${FTD_SIM}" ${_args}
    RESULT_VARIABLE _rc
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    TIMEOUT 240
)

if(NOT _rc EQUAL 0)
    message(FATAL_ERROR "FtdCliSmoke: ftd_sim ${SCENARIO} exited with '${_rc}'\nstdout:\n${_out}\nstderr:\n${_err}")
endif()

if(EXPECT_GLOB)
    file(GLOB _found "${OUTDIR}/${EXPECT_GLOB}")
    list(LENGTH _found _n)
    if(_n EQUAL 0)
        file(GLOB _all "${OUTDIR}/*")
        message(FATAL_ERROR "FtdCliSmoke: scenario ${SCENARIO} produced no files matching '${EXPECT_GLOB}' in ${OUTDIR}. Dir contents: ${_all}")
    endif()
    foreach(_f ${_found})
        if(IS_DIRECTORY "${_f}")
            continue()  # exporters create per-kind subdirs (e.g. clusters/)
        endif()
        file(SIZE "${_f}" _sz)
        if(_sz EQUAL 0)
            message(FATAL_ERROR "FtdCliSmoke: scenario ${SCENARIO} output ${_f} is EMPTY")
        endif()
    endforeach()
    message(STATUS "FtdCliSmoke: scenario ${SCENARIO} OK — ${_n} file(s) matching ${EXPECT_GLOB}, all non-empty")
else()
    message(STATUS "FtdCliSmoke: scenario ${SCENARIO} OK — exit 0")
endif()
