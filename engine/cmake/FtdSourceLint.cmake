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

# Latency horizon clamp single-source guard (revision 3.8).
# The value is defined in include/ftd/constants.h. Runtime implementations
# must name it instead of copying the numeric literal, otherwise CPU, CUDA,
# WASM, and the browser proxy can silently diverge.
set(_latency_impls
    "${ENGINE_DIR}/src/visual_field_sample.cpp"
    "${ENGINE_DIR}/cuda/visual_field_sample.cu"
    "${ENGINE_DIR}/wasm/ftd_wasm.cpp"
    "${ENGINE_DIR}/web/js/scales/scale0/analysis/gravity-analysis.js"
)
set(_latency_violations "")
foreach(_f ${_latency_impls})
    file(READ "${_f}" _src)
    string(FIND "${_src}" "0.998" _bare_literal)
    string(FIND "${_src}" "LATENCY_HORIZON_CLAMP" _named_constant)
    if(NOT _bare_literal EQUAL -1)
        list(APPEND _latency_violations "${_f}: copies the bare 0.998 literal")
    endif()
    if(_named_constant EQUAL -1)
        list(APPEND _latency_violations "${_f}: does not reference LATENCY_HORIZON_CLAMP")
    endif()
endforeach()
if(_latency_violations)
    list(JOIN _latency_violations "\n  " _latency_msg)
    message(FATAL_ERROR
        "FtdSourceLint: latency horizon clamp drift risk:\n  ${_latency_msg}")
endif()
message(STATUS "FtdSourceLint: OK — latency horizon clamp is named across runtime paths")

# Epistemic-copy guard: these umbrella/binding comments are user- and
# maintainer-facing metadata. Keep imposed/conjectural physical mappings from
# drifting back into theorem-grade language; this check changes no runtime.
file(READ "${ENGINE_DIR}/include/ftd/ontic.h" _ontic_header)
file(READ "${ENGINE_DIR}/include/ftd/ontic/consciousness.h" _reference_frame_header)
file(READ "${ENGINE_DIR}/wasm/ftd_wasm.cpp" _wasm_binding)
set(_epistemic_violations "")
foreach(_phrase
        "Everything from nothing"
        "each derived from the one above"
        "Every physical constant in the engine traces back")
    string(FIND "${_ontic_header}" "${_phrase}" _hit)
    if(NOT _hit EQUAL -1)
        list(APPEND _epistemic_violations "ontic.h: forbidden overclaim '${_phrase}'")
    endif()
endforeach()
foreach(_phrase
        "irreducibly subjective"
        "This proves φ is not an imposed constant"
        "[THEOREM] PT-unbroken")
    string(FIND "${_reference_frame_header}" "${_phrase}" _hit)
    if(NOT _hit EQUAL -1)
        list(APPEND _epistemic_violations "ontic/consciousness.h: forbidden overclaim '${_phrase}'")
    endif()
endforeach()
string(FIND "${_reference_frame_header}" "[IMPOSED]" _has_imposed)
string(FIND "${_reference_frame_header}" "[CONJECTURE]" _has_conjecture)
string(FIND "${_wasm_binding}" "EFT-derived:" _wasm_overclaim)
if(_has_imposed EQUAL -1 OR _has_conjecture EQUAL -1)
    list(APPEND _epistemic_violations
        "ontic/consciousness.h: construction must retain [IMPOSED] and [CONJECTURE] tags")
endif()
if(NOT _wasm_overclaim EQUAL -1)
    list(APPEND _epistemic_violations "ftd_wasm.cpp: ALPHA_EFT mislabeled as EFT-derived")
endif()
if(_epistemic_violations)
    list(JOIN _epistemic_violations "\n  " _epistemic_msg)
    message(FATAL_ERROR "FtdSourceLint: epistemic copy drift:\n  ${_epistemic_msg}")
endif()
message(STATUS "FtdSourceLint: OK — audited engine epistemic copy retains status tags")

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
