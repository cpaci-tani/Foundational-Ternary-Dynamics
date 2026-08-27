#!/usr/bin/env bash
# Canonical Linux/WSL2 CUDA build helper. Paths are derived from this script,
# so the checkout may live anywhere and the CUDA installation may be upgraded
# without editing the repository.
set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
repo_root="$(cd -- "${script_dir}/../.." && pwd)"
engine_dir="${repo_root}/engine"
build_dir="${FTD_WSL_BUILD_DIR:-${engine_dir}/build_wsl}"
cuda_architectures="${FTD_CUDA_ARCHITECTURES:-89;120}"
build_jobs="${FTD_BUILD_JOBS:-$(nproc)}"

cuda_root="${FTD_CUDA_ROOT:-}"
if [[ -z "${cuda_root}" ]]; then
    if command -v nvcc >/dev/null 2>&1; then
        cuda_root="$(cd -- "$(dirname -- "$(command -v nvcc)")/.." && pwd)"
    elif [[ -x /usr/local/cuda/bin/nvcc ]]; then
        cuda_root="$(cd -- /usr/local/cuda && pwd)"
    else
        echo "error: nvcc not found; set FTD_CUDA_ROOT to the CUDA toolkit root" >&2
        exit 1
    fi
fi

if [[ ! -x "${cuda_root}/bin/nvcc" ]]; then
    echo "error: ${cuda_root}/bin/nvcc is not executable" >&2
    exit 1
fi

export PATH="${cuda_root}/bin:${PATH}"
export LD_LIBRARY_PATH="${cuda_root}/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

for required_command in cmake ninja nvidia-smi; do
    if ! command -v "${required_command}" >/dev/null 2>&1; then
        echo "error: required command not found: ${required_command}" >&2
        exit 1
    fi
done

echo "FTD repository: ${repo_root}"
echo "Build directory: ${build_dir}"
echo "CUDA toolkit: ${cuda_root}"
echo "CUDA architectures: ${cuda_architectures}"
echo "Parallel jobs: ${build_jobs}"
nvcc --version | tail -3
nvidia-smi --query-gpu=name,driver_version --format=csv,noheader
cmake --version | head -1

if [[ "${1:-}" == "--preflight" ]]; then
    exit 0
fi

if [[ "$#" -gt 0 ]]; then
    build_targets=("$@")
else
    build_targets=(ftd_core ftd_cuda)
fi

cmake -S "${engine_dir}" -B "${build_dir}" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DFTD_ENABLE_CUDA=ON \
    -DCMAKE_CUDA_ARCHITECTURES="${cuda_architectures}"
cmake --build "${build_dir}" --target "${build_targets[@]}" \
    --parallel "${build_jobs}"
