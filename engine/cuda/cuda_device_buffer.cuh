#pragma once

#include "cuda_error.cuh"

#include <cuda_runtime_api.h>

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ftd::gpu {

// Move-only owner for transient cudaMalloc storage. Long-lived engine fields
// remain grouped under GpuBuffers because their teardown has stream/event
// quarantine rules; short-lived algorithm scratch should use this type so
// every CUDA_CHECK exception releases the successfully allocated prefix.
template <typename T>
class CudaDeviceBuffer final {
public:
    CudaDeviceBuffer() = default;
    explicit CudaDeviceBuffer(std::size_t count) { allocate(count); }
    ~CudaDeviceBuffer() { reset(); }

    CudaDeviceBuffer(const CudaDeviceBuffer&) = delete;
    CudaDeviceBuffer& operator=(const CudaDeviceBuffer&) = delete;

    CudaDeviceBuffer(CudaDeviceBuffer&& other) noexcept
        : ptr_(std::exchange(other.ptr_, nullptr)),
          count_(std::exchange(other.count_, 0)) {}

    CudaDeviceBuffer& operator=(CudaDeviceBuffer&& other) noexcept {
        if (this == &other) return *this;
        reset();
        ptr_ = std::exchange(other.ptr_, nullptr);
        count_ = std::exchange(other.count_, 0);
        return *this;
    }

    void allocate(std::size_t count) {
        if (count > (std::numeric_limits<std::size_t>::max)() / sizeof(T)) {
            throw std::overflow_error("CUDA device allocation size overflow");
        }

        T* fresh = nullptr;
        if (count != 0) {
            CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&fresh),
                                  count * sizeof(T)));
        }
        reset();
        ptr_ = fresh;
        count_ = count;
    }

    void reset() noexcept {
        if (ptr_) cudaFree(ptr_);
        ptr_ = nullptr;
        count_ = 0;
    }

    [[nodiscard]] T* get() noexcept { return ptr_; }
    [[nodiscard]] const T* get() const noexcept { return ptr_; }
    [[nodiscard]] std::size_t size() const noexcept { return count_; }
    [[nodiscard]] explicit operator bool() const noexcept { return ptr_ != nullptr; }

private:
    T* ptr_ = nullptr;
    std::size_t count_ = 0;
};

}  // namespace ftd::gpu
