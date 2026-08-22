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
#include <limits>
#include <stdexcept>
#if !(defined(__cpp_exceptions) || defined(__EXCEPTIONS))
#include <cstdio>
#include <cstdlib>
#endif

namespace ftd {

// Report a catastrophic id-counter overflow. The particle/pair id namespaces
// are 31-bit; exhausting them cannot occur in any real run. Native builds
// (exceptions enabled) throw std::overflow_error exactly as before. The WASM
// core is compiled -fno-exceptions, where a bare `throw` does not compile, so
// there it prints a diagnostic and aborts. Native behavior is unchanged.
[[noreturn]] inline void fatal_identity_overflow(const char* what) {
#if defined(__cpp_exceptions) || defined(__EXCEPTIONS)
    throw std::overflow_error(what);
#else
    std::fprintf(stderr, "FTD fatal: %s\n", what);
    std::abort();
#endif
}

class Injector {
public:
    Injector() = default;

    // Get the next monotonic particle ID. Lock-free; safe to call from
    // OpenMP parallel regions or CUDA host threads. Replaces the previous
    // pattern `omp critical(genesis_id) { pid = next_particle_id_++; }`.
    int next_particle_id() {
        int current = next_particle_id_.load(std::memory_order_relaxed);
        for (;;) {
            if (current >= std::numeric_limits<int>::max()) {
                fatal_identity_overflow("particle identity namespace exhausted");
            }
            if (next_particle_id_.compare_exchange_weak(
                    current, current + 1,
                    std::memory_order_relaxed, std::memory_order_relaxed)) {
                return current;
            }
        }
    }

    // Get the next entangled-pair ID. NOT atomic — only called from
    // single-threaded test setup paths (create_entangled_pair_cpu); kept as
    // plain int to avoid pretending it's safe under parallelism.
    int next_pair_id() {
        if (next_pair_id_ >= std::numeric_limits<int>::max()) {
            fatal_identity_overflow("pair identity namespace exhausted");
        }
        return next_pair_id_++;
    }

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

    // Raise-only reconciliation for a device backend.  A GPU may have issued
    // identities to particles that later evaporated, so scanning the live host
    // voxels cannot reconstruct the lifetime high-water mark.  Never lower a
    // counter: gaps are allowed; reuse is not.
    void raise_identity_counters(int next_particle_id, int next_pair_id) {
        int current = next_particle_id_.load(std::memory_order_relaxed);
        while (current < next_particle_id
               && !next_particle_id_.compare_exchange_weak(
                   current, next_particle_id,
                   std::memory_order_relaxed, std::memory_order_relaxed)) {}
        if (next_pair_id_ < next_pair_id) next_pair_id_ = next_pair_id;
    }

private:
    std::atomic<int> next_particle_id_{0};
    int              next_pair_id_     = 0;
};

}  // namespace ftd
