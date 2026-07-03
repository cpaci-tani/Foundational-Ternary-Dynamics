# ============================================================================
# FtdSourceLint.cmake — source-text lint gate (revision 2.6).
#
# Asserts that the tree-level reference constants X_PLUS / X_MINUS never
# appear in a runtime force/physics path: those paths must use the
# precision-corrected ALPHA chain (ALPHA = 1/X_PLUS_PRECISION). Banner,
# audit, CLI-display, and test usages are legitimate and NOT scanned.
#
# Run as a CTest: cmake -DENGINE_DIR=<engine> -P FtdSourceLint.cmake
# Extend PHYSICS_PATHS/FORBIDDEN when new force paths or reference-only
# constants are added.
# ============================================================================

if(NOT ENGINE_DIR)
    message(FATAL_ERROR "FtdSourceLint: ENGINE_DIR is required")
endif()

set(PHYSICS_PATHS
    "${ENGINE_DIR}/src/render_bridge.cpp"
    "${ENGINE_DIR}/src/poisson_solvers.cpp"
    "${ENGINE_DIR}/src/transmutation_phases.cpp"
    "${ENGINE_DIR}/src/injection.cpp"
    "${ENGINE_DIR}/src/energy_ledger_compute.cpp"
)
file(GLOB _phase_files "${ENGINE_DIR}/src/render_bridge_phases/*.cpp")
file(GLOB _kernel_files "${ENGINE_DIR}/cuda/kernels_*.cu" "${ENGINE_DIR}/cuda/gpu_engine.cu")
list(APPEND PHYSICS_PATHS ${_phase_files} ${_kernel_files})

# Forbidden identifiers in physics paths. Word-boundary matching: X_PLUS
# must not match X_PLUS_PRECISION (which is allowed via the ALPHA chain).
set(FORBIDDEN "X_PLUS" "X_MINUS")

set(_violations "")
foreach(_f ${PHYSICS_PATHS})
    if(NOT EXISTS "${_f}")
        continue()
    endif()
    file(READ "${_f}" _src)
    foreach(_ident ${FORBIDDEN})
        # Match _ident NOT followed by an identifier character (so
        # X_PLUS_PRECISION does not trip the X_PLUS rule) and not preceded
        # by one (so MY_X_PLUS wouldn't either).
        string(REGEX MATCH "[^A-Za-z0-9_]${_ident}[^A-Za-z0-9_]" _hit "${_src}")
        if(_hit)
            list(APPEND _violations "${_f}: uses ${_ident}")
        endif()
    endforeach()
endforeach()

if(_violations)
    list(JOIN _violations "\n  " _msg)
    message(FATAL_ERROR
        "FtdSourceLint: tree-level reference constants leaked into runtime "
        "physics paths (use the ALPHA / X_PLUS_PRECISION chain instead — see "
        "master_quadratic.h guardrail note):\n  ${_msg}")
endif()

message(STATUS "FtdSourceLint: OK — no tree-level constants in physics paths")

# ── Experimental-quarantine include guard (revision 3.7, ADR-0016) ──────────
# No production TU may include the quarantined headers; the experimental
# implementations and their gated tests are exempt.
file(GLOB_RECURSE _prod_srcs "${ENGINE_DIR}/src/*.cpp" "${ENGINE_DIR}/wasm/*.cpp")
list(FILTER _prod_srcs EXCLUDE REGEX "src/dag_engine\.cpp$")
list(FILTER _prod_srcs EXCLUDE REGEX "src/cognition/")
set(_quarantined "ftd/dag_engine.h" "ftd/dag_lattice.h" "cognition/cognitive_lattice.h")
set(_qviolations "")
foreach(_f ${_prod_srcs})
    file(READ "${_f}" _src)
    foreach(_h ${_quarantined})
        string(FIND "${_src}" "#include \"${_h}\"" _hit)
        if(NOT _hit EQUAL -1)
            list(APPEND _qviolations "${_f}: includes quarantined ${_h}")
        endif()
    endforeach()
endforeach()
if(_qviolations)
    list(JOIN _qviolations "\n  " _qmsg)
    message(FATAL_ERROR
        "FtdSourceLint: production TU includes an experimental/quarantined "
        "header (ADR-0016 boundary):\n  ${_qmsg}")
endif()
message(STATUS "FtdSourceLint: OK — quarantine boundary intact (ADR-0016)")
