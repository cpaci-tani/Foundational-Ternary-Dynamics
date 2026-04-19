#!/usr/bin/env bash
# Linux + WSL2 CUDA build — bypasses the Windows CMake 4 + NVCC 13 escape bug.
# Runs from inside Ubuntu 22.04 WSL2 with CUDA 13.0 installed.
set -e
cd /mnt/c/Users/cpaci/Desktop/ftd
export PATH=/usr/local/cuda-13.0/bin:/usr/bin:/bin
export LD_LIBRARY_PATH=/usr/local/cuda-13.0/lib64
echo "=== nvcc ==="
nvcc --version | tail -3
echo "=== nvidia-smi ==="
nvidia-smi | head -5
echo "=== cmake ==="
cmake --version | head -1
echo "=== Configuring ==="
cmake -S engine -B engine/build_wsl \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DFTD_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_ARCHITECTURES="89;120" 2>&1 | tail -10
echo "=== Build ==="
cmake --build engine/build_wsl --target ftd_core ftd_cuda -j 4 2>&1 | tail -20
