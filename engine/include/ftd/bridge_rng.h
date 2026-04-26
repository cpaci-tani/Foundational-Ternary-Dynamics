#pragma once
/**
 * @file bridge_rng.h
 * @brief PIMPL'd RNG state for RenderBridge.
 *
 * RF-9 (CHECKLIST_ENGINE.md): render_bridge.h previously pulled in
 * <random> directly via `std::mt19937 rng_`, `std::uniform_real_distribution
 * uniform_`, and `std::vector<std::mt19937> thread_rngs_` member fields.
 * That dragged the entire `<random>` header into ~140 translation units
 * even though the RNG is purely internal state.
 *
 * This header forward-declares `BridgeRng` and exposes a sample-by-value
 * API. The `<random>` header is included only inside src/bridge_rng.cpp,
 * so render_bridge.h (and every TU that includes it) stops pulling it in.
 *
 * RenderBridge holds a `std::unique_ptr<BridgeRng>`; its destructor lives
 * in render_bridge.cpp where BridgeRng is fully defined.
 */

#include <cstddef>
#include <cstdint>

namespace ftd {

class BridgeRng {
  public:
    BridgeRng();
    explicit BridgeRng(unsigned int seed);
    ~BridgeRng();

    BridgeRng(const BridgeRng&) = delete;
    BridgeRng& operator=(const BridgeRng&) = delete;

    /// Re-seed the bridge-level RNG (used for genesis sampling and as the
    /// parent generator for per-thread Langevin streams).
    void seed(unsigned int seed);

    /// Single uniform [0, 1) sample from the bridge-level RNG. Used by the
    /// transmutation phases (CPU path) and any single-threaded sampler.
    double sample_uniform();

    /// Resize the per-thread RNG pool. Called once in the RenderBridge
    /// ctor (sized to omp_get_max_threads()).
    void resize_thread_pool(std::size_t n);

    /// Re-seed the per-thread pool from the current bridge-level RNG state.
    /// `thread_seeds_out` is filled with the seeds used (size == pool size).
    void reseed_thread_pool(unsigned int* thread_seeds_out, std::size_t n);

    /// Sample uniform [0, 1) from the per-thread RNG at index `tid`.
    /// Thread-safe iff distinct `tid`s are used per OMP thread.
    double thread_uniform(std::size_t tid);

    /// Sample N(0, 1) from the per-thread RNG at index `tid`.
    double thread_normal(std::size_t tid);

  private:
    struct Impl;
    Impl* impl_;
};

}  // namespace ftd
