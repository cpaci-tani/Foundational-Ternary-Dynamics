# CUDA library for FTD GPU acceleration
# Requires: CUDA Toolkit 12.8+ (SM 120 for Blackwell/RTX 5090)

add_library(ftd_cuda STATIC
    gpu_buffers.cu
    gpu_engine.cu
    kernels_stencil.cu
    kernels_poisson.cu
    kernels_forces.cu
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

# Enable separable compilation for device-side linking
set_target_properties(ftd_cuda PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    POSITION_INDEPENDENT_CODE ON
)
