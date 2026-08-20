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

# ── CPU tick phase-order pin (revision 3.2) ─────────────────────────────────
# The CPU tick ladder's ordering constraints (CALLSTACKS.md §3.1, ADR-0008)
# are enforced structurally (private methods, sequential code) and validated
# causally by test_tick_phase_order PO-2/PO-3. This lint additionally pins
# the SOURCE ORDER of the phase dispatch sites as data, so any reorder —
# accidental or intentional — fails a test until this expected sequence is
# updated in the same commit (the "single source of truth to diff
# CALLSTACKS.md against" from revision ticket 3.2, implemented as a lint
# instead of a runtime table to avoid churning the engine's most
# load-bearing function).
set(_tick_sequence
    "toggles.validate"
    "toggles.ew_background_sweep"
    "toggles.db_clock_coulomb"
    "phase_read()"
    "phase_write()"
    "pair_production_cpu()"
    "gauss_project()"
    "solve_latency_poisson()"
    "phase_forces()"
    "phase_movement()"
    "apply_absorbing_boundary"
    "apply_reflective_flux_boundary"
    "apply_dispersal_flux_boundary"
    "weak_transmutation_cpu()"
    "triad_binding_cpu()"
    "relax_su2_links_cpu"
    "relax_su3_links_cpu"
    "accumulate_proper_time()"
    "knot_tracker_->record"
    "update_energy_ledger()"
)
file(READ "${ENGINE_DIR}/src/render_bridge.cpp" _rb_src)
# Scope to the tick() body: from the definition to the transmutation-wrapper
# banner that follows it.
string(FIND "${_rb_src}" "void RenderBridge::tick()" _tick_begin)
string(FIND "${_rb_src}" "Transmutation phase bodies extracted" _tick_end)
if(_tick_begin EQUAL -1 OR _tick_end EQUAL -1)
    message(FATAL_ERROR "FtdSourceLint: could not locate the tick() body markers in render_bridge.cpp — update the phase-order lint scope")
endif()
math(EXPR _tick_len "${_tick_end} - ${_tick_begin}")
string(SUBSTRING "${_rb_src}" ${_tick_begin} ${_tick_len} _tick_body)
set(_cursor 0)
foreach(_marker ${_tick_sequence})
    string(SUBSTRING "${_tick_body}" ${_cursor} -1 _rest)
    string(FIND "${_rest}" "${_marker}" _pos)
    if(_pos EQUAL -1)
        message(FATAL_ERROR
            "FtdSourceLint: tick() phase-order pin broken — '${_marker}' not found "
            "after the previous phase. If the tick ladder changed INTENTIONALLY, "
            "update _tick_sequence here and CALLSTACKS.md in the same commit.")
    endif()
    math(EXPR _cursor "${_cursor} + ${_pos} + 1")
endforeach()
message(STATUS "FtdSourceLint: OK — tick() phase order matches the pinned sequence (revision 3.2)")

# ---------------------------------------------------------------------------
# 9.3d UI-model include lint (SPEC_UI_V2 Phase 0B)
# ---------------------------------------------------------------------------
set(_ui_model_files
    "${ENGINE_DIR}/native_desktop/include/native_desktop/ui_demand.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/ui_command.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/ui_journal.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/ui_result.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/ui_snapshot.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/command_queue.h"
    "${ENGINE_DIR}/native_desktop/include/native_desktop/snapshot_publisher.h"
    "${ENGINE_DIR}/native_desktop/src/command_queue.cpp"
    "${ENGINE_DIR}/native_desktop/src/snapshot_publisher.cpp"
)
set(_ui_model_allow
    "ftd/term_toggles.h"
    "ftd/telemetry_snapshot.h"
    "ftd/visual_field_sample.h"
    "ftd/render_bridge_diagnostics.h"
    "ftd/scenario_meta.h"
    "ftd/lattice.h"
    "ftd/constants.h"
)
foreach(_f ${_ui_model_files})
    if(NOT EXISTS "${_f}")
        message(FATAL_ERROR "FtdSourceLint: UI-model source missing: ${_f}")
    endif()
    file(READ "${_f}" _src)
    if(_src MATCHES "RenderBridge" OR _src MATCHES "[Ii]mGui" OR _src MATCHES "d3d12" OR _src MATCHES "D3D12" OR _src MATCHES "cuda_runtime" OR _src MATCHES "ftd/gpu")
        message(FATAL_ERROR "FtdSourceLint: UI-model file ${_f} contains a forbidden RenderBridge/ImGui/D3D12/CUDA token")
    endif()
    string(REGEX MATCHALL "#include[ \t]+\"ftd/[^\"]+\"" _incs "${_src}")
    foreach(_inc ${_incs})
        string(REGEX REPLACE "#include[ \t]+\"" "" _inc "${_inc}")
        string(REGEX REPLACE "\"" "" _inc "${_inc}")
        list(FIND _ui_model_allow "${_inc}" _ok)
        if(_ok EQUAL -1)
            message(FATAL_ERROR "FtdSourceLint: UI-model file ${_f} includes non-allowlisted '${_inc}'")
        endif()
    endforeach()
endforeach()
list(LENGTH _ui_model_files _ui_model_n)
if(_ui_model_n LESS 9)
    message(FATAL_ERROR "FtdSourceLint: UI-model include lint is vacuous")
endif()
message(STATUS "FtdSourceLint: OK — ftd_native_ui_model include surface matches SPEC_UI_V2 §3.1")

# ---------------------------------------------------------------------------
# 9.3b boundary lint: Win32 app / presenter must not grow engine observers
# ---------------------------------------------------------------------------
set(_ui_boundary_main "${ENGINE_DIR}/native_desktop/src/main.cpp")
set(_ui_boundary_presenter "${ENGINE_DIR}/native_desktop/src/d3d12_presenter.cpp")
if(NOT EXISTS "${_ui_boundary_main}" OR NOT EXISTS "${_ui_boundary_presenter}")
    message(FATAL_ERROR "FtdSourceLint: UI boundary sources missing")
endif()
file(READ "${_ui_boundary_presenter}" _presenter_src)
if(_presenter_src MATCHES "[Ii]mGui")
    message(FATAL_ERROR "FtdSourceLint: d3d12_presenter.cpp must not include ImGui")
endif()
file(READ "${_ui_boundary_main}" _main_src)
if(_main_src MATCHES "RenderBridge")
    message(FATAL_ERROR "FtdSourceLint: main.cpp must not name RenderBridge (session owns the bridge)")
endif()
if(_main_src MATCHES "inspect_voxel\\(" OR _main_src MATCHES "energy_audit\\(" OR _main_src MATCHES "continuity_step\\(" OR _main_src MATCHES "energy_ledger\\(")
    message(FATAL_ERROR "FtdSourceLint: main.cpp contains a RenderBridge observer call")
endif()
message(STATUS "FtdSourceLint: OK — native-desktop UI boundary keeps observers off the presenter/main")
