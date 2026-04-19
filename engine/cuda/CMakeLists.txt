# CUDA library for FTD GPU acceleration
# Requires: CUDA Toolkit 12.8+ (SM 120 for Blackwell/GPU)

add_library(ftd_cuda STATIC
    gpu_buffers.cu
    gpu_engine.cu
    kernels_stencil.cu
    kernels_poisson.cu
    kernels_forces.cu
    atom_engine_gpu.cu      # Wave 5.3 Phase 1: gpu::AtomEngineGpu pair-force kernel
    particle_engine_gpu.cu  # Wave 5.4 Phase 1: gpu::ParticleEngineGpu pair-force kernel
)

target_include_directories(ftd_cuda PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# Link CUDA runtime libraries
target_link_libraries(ftd_cuda PUBLIC
    CUDA::cudart
    CUDA::cufft
    CUDA::curand
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

# Belt-and-braces: explicitly strip the Windows /Fd flag from CUDA
# compile commands. CMake may still try to inject it via generator
# expressions on target properties; this undoes that for our target.
if(MSVC)
    target_compile_options(ftd_cuda PRIVATE
        $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/Z7>
    )
endif()
