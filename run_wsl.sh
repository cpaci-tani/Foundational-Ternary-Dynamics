#!/bin/bash
export PATH="/usr/local/cuda/bin:$PATH"
cd /mnt/c/Users/cpaci/Desktop/ftd
rm -rf engine/build_wsl
cmake -G Ninja -S engine -B engine/build_wsl -DFTD_ENABLE_CUDA=ON -DCMAKE_BUILD_TYPE=Release
cmake --build engine/build_wsl -j$(nproc)
./engine/build_wsl/benchmark_alpha_convergence
