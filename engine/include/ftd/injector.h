#pragma once
/**
 * @file injector.h
 * @brief Injector — owns particle-ID and pair-ID counters.
 *
 * ARCH-1 Phase C (CHECKLIST_ENGINE.md): extracts the monotonic counters that
 * had been raw private members of RenderBridge. Centralising them in an
 * Injector class:
 *   1. Replaces 6 individual friend declarations on inject_*_cpu helpers
 *      with a single dependency through Injector's public API.
 *   2. Encapsulates the std::atomic semantics (BUG-001 fix from Round 1)
 *      behind named methods, so future migrations to a lock-free queue or
 *      sharded counter don't break call sites.
 *   3. Documents the contract: each call to next_particle_id() returns a
 *      unique value across all threads; the engine guarantees no duplicates
 *      for the lifetime of the bridge.
 *
 * Usage from injection / genesis paths:
 *   const int pid = bridge.injector().next_particle_id();
 *   v.particle_id = pid;
 */

#include <atomic>

namespace ftd {

class Injector {
public:
    Injector() = default;

    // Get the next monotonic particle ID. Lock-free; safe to call from
    // OpenMP parallel regions or CUDA host threads. Replaces the previous
    // pattern `omp critical(genesis_id) { pid = next_particle_id_++; }`.
    int next_particle_id() {
        return next_particle_id_.fetch_add(1, std::memory_order_relaxed);
    }

    // Get the next entangled-pair ID. NOT atomic — only called from
    // single-threaded test setup paths (create_entangled_pair_cpu); kept as
    // plain int to avoid pretending it's safe under parallelism.
    int next_pair_id() { return next_pair_id_++; }

    // Reset both counters. Used by tests that want determinism across
    // sequential RenderBridge constructions in the same process.
    void reset() {
        next_particle_id_.store(0, std::memory_order_relaxed);
        next_pair_id_ = 0;
    }

    // Read-only accessors (for diagnostics and parity assertions).
    int peek_next_particle_id() const {
        return next_particle_id_.load(std::memory_order_relaxed);
    }
    int peek_next_pair_id() const { return next_pair_id_; }

private:
    std::atomic<int> next_particle_id_{0};
    int              next_pair_id_     = 0;
};

}  // namespace ftd
