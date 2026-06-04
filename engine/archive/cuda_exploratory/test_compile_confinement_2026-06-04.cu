#include <cuda_runtime.h>
#include <curand_kernel.h>
#include <iostream>
#include <cmath>

#define L 256
#define MAX_R 32
#define THETA 0.771239

__global__ void init_lattice_kernel(int* lattice, unsigned long long seed) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < L && y < L) {
        curandState state;
        curand_init(seed, y * L + x, 0, &state);
        
        float r = curand_uniform(&state);
        int val = 0;
        if (r < 0.333333f) val = -1;
        else if (r < 0.666667f) val = 0;
        else val = 1;
        
        lattice[y * L + x] = val;
    }
}

__global__ void compute_all_wilson_kernel(const int* lattice, double* results) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    
    if (x < L && y < L) {
        int sum = 0;
        for (int R = 1; R <= MAX_R; R++) {
            if (x < L - R + 1 && y < L - R + 1) {
                if (R == 1) {
                    sum = lattice[y * L + x];
                } else {
                    for (int dy = 0; dy < R - 1; dy++) {
                        sum += lattice[(y + dy) * L + (x + R - 1)];
                    }
                    for (int dx = 0; dx < R; dx++) {
                        sum += lattice[(y + R - 1) * L + (x + dx)];
                    }
                }
                
                double loop_val = cos(THETA * sum);
                atomicAdd(&results[R - 1], loop_val);
            }
        }
    }
}

int main() {
    int* d_lattice;
    double* d_results;
    double h_results[MAX_R];
    
    cudaMalloc(&d_lattice, L * L * sizeof(int));
    cudaMalloc(&d_results, MAX_R * sizeof(double));
    cudaMemset(d_results, 0, MAX_R * sizeof(double));
    
    dim3 blockSize(16, 16);
    dim3 gridSize((L + 15) / 16, (L + 15) / 16);
    
    init_lattice_kernel<<<gridSize, blockSize>>>(d_lattice, 12345ULL);
    cudaDeviceSynchronize();
    
    compute_all_wilson_kernel<<<gridSize, blockSize>>>(d_lattice, d_results);
    cudaDeviceSynchronize();
    
    cudaMemcpy(h_results, d_results, MAX_R * sizeof(double), cudaMemcpyDeviceToHost);
    
    std::cout << "String Tension Sigma Scaling (Area Law vs Perimeter Law):\\n" << std::endl;
    for(int R = 1; R <= 10; ++R) {
        int valid_origins = (L - R + 1) * (L - R + 1);
        double avg = h_results[R-1] / valid_origins;
        std::cout << "  Wilson Loop W(R=" << R << ", T=" << R << ") = " << avg << std::endl;
    }
    
    cudaFree(d_lattice);
    cudaFree(d_results);
    return 0;
}
