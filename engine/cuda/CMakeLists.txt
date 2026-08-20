# CUDA library for FTD GPU acceleration
# Requires: CUDA Toolkit 12.8+ (SM 120 for Blackwell/GPU)

add_library(ftd_cuda STATIC
    gpu_buffers.cu
    gpu_engine.cu
    diagnostic_reductions.cu
    kernels_proper_time.cu
    kernels_injection.cu
    visual_field_sample.cu
    # Phase 5 split (2026-04-27): kernels_stencil.cu (1530 LOC, 14 kernels) was
    # decomposed into single-substrate, dual-substrate, and auxiliary TUs to
    # halve partial CUDA rebuild time. Shared damping/index helpers live in
    # kernels_stencil_common.cuh (mirrors cuda_index.cuh ADR-0007 pattern).
    kernels_stencil_single.cu
    kernels_stencil_dual.cu
    kernels_aux.cu
    kernels_poisson.cu
    kernels_forces.cu
    kernels_matched_gauss.cu
    atom_engine_gpu.cu      # Wave 5.3 Phase 1: gpu::AtomEngineGpu pair-force kernel
    particle_engine_gpu.cu  # Wave 5.4 Phase 1: gpu::ParticleEngineGpu pair-force kernel
    wilson_dirac_gpu.cu     # Phase II.2-E: Wilson-Dirac CPU/GPU parity kernel
    cuda_invariants.cu      # ADR-0014: __constant__ pattern for invariant matrices
    kernels_eft.cu          # [NEW] Pure GPU-native EFT snapshot & operator calculations
    cuda_matched_field_pipeline.cu # FTD-0746 successor: matched field/observer accelerator
    kernels_gauge.cu        # [NEW] GPU kernels for non-Abelian link variable relaxation
)

target_include_directories(ftd_cuda PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link CUDA runtime libraries.
# ftd_eft provides DualCellContinuity (called from gpu_engine.cu's
# continuity_step()). PUBLIC dep so libftd_eft.a appears in the link
# line after libftd_cuda.a (CMake topological sort), satisfying
# gpu_engine's reference. See engine/CMakeLists.txt ftd_eft block.
target_link_libraries(ftd_cuda PUBLIC
    ftd_eft
    CUDA::cudart
    CUDA::cufft
)

# NVCC needs relaxed constexpr for inline constexpr constants in device code
# --allow-unsupported-compiler: CUDA 13.0 officially supports MSVC 2019-2022;
# VS 2026 (MSVC 19.50) works fine but needs this override flag.
target_compile_options(ftd_cuda PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
    $<$<COMPILE_LANGUAGE:CUDA>:--allow-unsupported-compiler>
)

# Enable separable compilation for device-side linking.
# CUDA_RESOLVE_DEVICE_SYMBOLS ON ensures the device-link step runs
# when this static library is consumed by executables, resolving
# __cudaRegisterLinkedBinary symbols that would otherwise be unresolved.
set_target_properties(ftd_cuda PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    POSITION_INDEPENDENT_CODE ON
    # Workaround CMake 4.x + NVCC 13: empty-string PDB name suppresses the
    # `-Xcompiler=-Fd<path>,-FS` flag that nvcc misinterprets as multiple
    # input files (the comma between the .pdb path and -FS triggers
    # `nvcc fatal: A single input file is required`). CUDA .obj files
    # don't need a PDB; only the host CXX files do.
    COMPILE_PDB_NAME ""
    COMPILE_PDB_OUTPUT_DIRECTORY ""
)

# FTD-0752 research-only arithmetic-equivalence library.  It compiles only the
# matched-field pipeline from the unchanged source with contraction disabled,
# matching the host target's explicitly separate binary64 multiply/add map.
# The established ftd_cuda library and all production callers are unchanged.
add_library(ftd_cuda_explicit_rounding STATIC
    cuda_matched_field_pipeline.cu
)
target_include_directories(ftd_cuda_explicit_rounding PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(ftd_cuda_explicit_rounding PUBLIC
    ftd_eft
    CUDA::cudart
)
target_compile_options(ftd_cuda_explicit_rounding PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
    $<$<COMPILE_LANGUAGE:CUDA>:--allow-unsupported-compiler>
    $<$<COMPILE_LANGUAGE:CUDA>:--fmad=false>
)
set_target_properties(ftd_cuda_explicit_rounding PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ""
    COMPILE_PDB_OUTPUT_DIRECTORY ""
)
if(MSVC)
    target_compile_options(ftd_cuda_explicit_rounding PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/Z7>
    )
endif()

# FTD-0759 research-only device reduction for the state-only support ladder.
# Kept separate from ftd_cuda and the FTD-0758 explicit-rounding binary so the
# already-registered held-out executable remains byte-for-byte unchanged.
add_library(ftd_cuda_m3_observer STATIC
    cuda_state_only_support_ladder.cu
    cuda_quadratic_coat_orbit_gather.cu
    cuda_transported_chart_morphology.cu
    cuda_paired_field_response.cu
    # Arc 2 fused per-tick masked momentum-ledger reduction.  Scalar-only
    # output; no complete field is downloaded.
    cuda_momentum_transport_current.cu
)
target_include_directories(ftd_cuda_m3_observer PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(ftd_cuda_m3_observer PUBLIC
    ftd_eft
    CUDA::cudart
)
target_compile_options(ftd_cuda_m3_observer PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:--expt-relaxed-constexpr>
    $<$<COMPILE_LANGUAGE:CUDA>:--allow-unsupported-compiler>
    $<$<COMPILE_LANGUAGE:CUDA>:--fmad=false>
)
set_target_properties(ftd_cuda_m3_observer PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    CUDA_RESOLVE_DEVICE_SYMBOLS ON
    POSITION_INDEPENDENT_CODE ON
    COMPILE_PDB_NAME ""
    COMPILE_PDB_OUTPUT_DIRECTORY ""
)
if(MSVC)
    target_compile_options(ftd_cuda_m3_observer PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/Z7>
    )
endif()

# Belt-and-braces: explicitly strip the Windows /Fd flag from CUDA
# compile commands. CMake may still try to inject it via generator
# expressions on target properties; this undoes that for our target.
if(MSVC)
    target_compile_options(ftd_cuda PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/Z7>
    )
endif()
