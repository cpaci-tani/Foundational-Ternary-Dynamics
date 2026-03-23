#pragma once
// tritium/convolution.h — 1D/2D/3D ternary convolution with trit kernels
//
// Convolution with trit-valued kernels produces integer outputs.
// Each output = sum of trit products = sliding dot product of trit vectors.

#include "trit_vector.h"
#include "trit_matrix.h"
#include <vector>
#include <cassert>

namespace tritium {

// ============================================================================
// 1D convolution: sliding dot product of input with kernel
// ============================================================================
// output[i] = Σ_k input[i+k] * kernel[k]  for i = 0..input.size()-kernel.size()
// Output length: input.size() - kernel.size() + 1

inline std::vector<int> conv1d(const TritVector& input, const TritVector& kernel) {
    assert(input.size() >= kernel.size());
    size_t out_len = input.size() - kernel.size() + 1;
    std::vector<int> output(out_len, 0);

    size_t ksize = kernel.size();

    for (size_t i = 0; i < out_len; ++i) {
        int sum = 0;
        for (size_t k = 0; k < ksize; ++k) {
            int a = to_int(input[i + k]);
            int b = to_int(kernel[k]);
            sum += a * b;
        }
        output[i] = sum;
    }

    return output;
}

// ============================================================================
// 2D convolution: sliding 2D dot product
// ============================================================================
// input: H x W matrix of trits
// kernel: kH x kW matrix of trits
// output: (H-kH+1) x (W-kW+1) matrix of ints

inline std::vector<std::vector<int>> conv2d(const TritMatrix& input,
                                             const TritMatrix& kernel) {
    assert(input.rows() >= kernel.rows() && input.cols() >= kernel.cols());
    size_t out_h = input.rows() - kernel.rows() + 1;
    size_t out_w = input.cols() - kernel.cols() + 1;
    std::vector<std::vector<int>> output(out_h, std::vector<int>(out_w, 0));

    for (size_t i = 0; i < out_h; ++i) {
        for (size_t j = 0; j < out_w; ++j) {
            int sum = 0;
            for (size_t ki = 0; ki < kernel.rows(); ++ki)
                for (size_t kj = 0; kj < kernel.cols(); ++kj)
                    sum += to_int(input.get(i + ki, j + kj)) *
                           to_int(kernel.get(ki, kj));
            output[i][j] = sum;
        }
    }

    return output;
}

// ============================================================================
// 3D convolution (for volumetric lattice data)
// ============================================================================
// Uses flat arrays with explicit dimensions.
// input: D x H x W, kernel: kD x kH x kW
// output: (D-kD+1) x (H-kH+1) x (W-kW+1)

struct Volume3D {
    std::vector<Trit> data;
    size_t depth, height, width;

    Trit get(size_t d, size_t h, size_t w) const {
        return data[d * height * width + h * width + w];
    }
    void set(size_t d, size_t h, size_t w, Trit t) {
        data[d * height * width + h * width + w] = t;
    }
};

inline std::vector<int> conv3d(const Volume3D& input, const Volume3D& kernel) {
    assert(input.depth >= kernel.depth);
    assert(input.height >= kernel.height);
    assert(input.width >= kernel.width);

    size_t out_d = input.depth - kernel.depth + 1;
    size_t out_h = input.height - kernel.height + 1;
    size_t out_w = input.width - kernel.width + 1;
    std::vector<int> output(out_d * out_h * out_w, 0);

    for (size_t d = 0; d < out_d; ++d) {
        for (size_t h = 0; h < out_h; ++h) {
            for (size_t w = 0; w < out_w; ++w) {
                int sum = 0;
                for (size_t kd = 0; kd < kernel.depth; ++kd)
                    for (size_t kh = 0; kh < kernel.height; ++kh)
                        for (size_t kw = 0; kw < kernel.width; ++kw)
                            sum += to_int(input.get(d+kd, h+kh, w+kw)) *
                                   to_int(kernel.get(kd, kh, kw));
                output[d * out_h * out_w + h * out_w + w] = sum;
            }
        }
    }

    return output;
}

} // namespace tritium
