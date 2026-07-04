#pragma once
// ============================================================================
// ftd/parallel.h — unified parallel primitives with THREE compile-time backends
// ============================================================================
//
//   FTD_WASM_THREADS defined  -> persistent std::thread pool (Emscripten
//                                pthreads; validated by the Phase-0 stencil
//                                spike at 5.5-7x on 8 threads).
//   _OPENMP defined           -> OpenMP (native CPU/CUDA build, unchanged
//                                physics — the golden gate stays bit-exact).
//   neither                   -> serial (the shipped serial WASM fallback,
//                                and any non-OpenMP native build).
//
// WHY THIS IS DETERMINISM-SAFE (golden hash preserved across all three):
//   The engine's parallel loops are PARTITION-INDEPENDENT — each iteration
//   writes only its own voxel output, there are NO float reductions inside a
//   parallel region (Poisson phi-sums are kept SEQUENTIAL elsewhere), and the
//   few shared-state writes (set_state) go through with_critical(). Therefore
//   ANY static partition of [begin,end) yields bit-identical results, so the
//   pthread-pool and OpenMP and serial backends all match the pinned golden
//   hash (current pin: GOLDEN_HASH in test_render_bridge_golden.cpp).
//   (Verified natively by test_render_bridge_golden
//   and campaign_determinism_gate's omp1==pool check.)
//
// USAGE — convert
//     #pragma omp parallel for schedule(static)
//     for (int ix = 0; ix < L; ++ix) { <body using ix> }
// to
//     ftd::parallel_for(0, L, [&](int _lo, int _hi){
//       for (int ix = _lo; ix < _hi; ++ix) { <body using ix> }
//     });
//
// Convert  #pragma omp critical { <body> }  to  ftd::with_critical([&]{ <body> });
// Convert  #pragma omp atomic; ++x;          to a std::atomic<int> member + fetch_add.
// ============================================================================

#include <functional>
#include <algorithm>

#if defined(_OPENMP)
#include <omp.h>
#endif

#if defined(FTD_WASM_THREADS)
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>
#include <cstdlib>
#endif

namespace ftd {

// ----------------------------------------------------------------------------
//  Thread-count hint (informational; the partition is what matters, not the
//  exact count). Native: OpenMP max threads. Pool: the pool size. Serial: 1.
// ----------------------------------------------------------------------------
int parallel_max_threads();

// Override the pool thread count (call ONCE before the first parallel_for).
// No-op on the OpenMP/serial backends. Lets a host A/B pool sizes (the GV2
// determinism/speed harness, and the browser's hardwareConcurrency choice).
void set_pool_threads(int n);

// ----------------------------------------------------------------------------
//  parallel_for: split [begin,end) into contiguous chunks and run body(lo,hi)
//  on each, concurrently. Static, contiguous partition (deterministic result).
// ----------------------------------------------------------------------------
void parallel_for(int begin, int end, const std::function<void(int, int)>& body);

// ----------------------------------------------------------------------------
//  with_critical: run f() under mutual exclusion (replacement for the
//  #pragma omp critical regions that the compiler silently DROPS when there is
//  no -fopenmp — without this the pool backend would race on set_state).
// ----------------------------------------------------------------------------
template <class F>
inline void with_critical(F&& f);

// ----------------------------------------------------------------------------
//  atomic_inc: ++x atomically (replacement for the #pragma omp atomic the
//  compiler drops without -fopenmp). Used for the FTD-0267 genesis/evaporation
//  telemetry counters (observation-only; not hashed) so the pool backend does
//  not data-race a plain long long (UB) even though the value is non-physical.
// ----------------------------------------------------------------------------
inline void atomic_inc(long long& x);

// ============================================================================
//  Backend: FTD_WASM_THREADS (persistent std::thread pool)
// ============================================================================
#if defined(FTD_WASM_THREADS)

namespace detail {

// Optional pool-size override (0 = auto). Set via ftd::set_pool_threads()
// before the first parallel_for constructs the pool.
inline int& pool_override() { static int v = 0; return v; }

// One process-wide pool. Function-local static => single shared instance across
// all TUs (C++17 inline-statics ODR). Threads are spawned once and reused.
class ThreadPool {
public:
    static ThreadPool& instance() { static ThreadPool p; return p; }

    int size() const { return T_; }

    // Run body over [begin,end) split into T_ contiguous chunks.
    void run(int begin, int end, const std::function<void(int, int)>& body) {
        const int n = end - begin;
        if (n <= 0) return;
        if (T_ <= 1 || n == 1) { body(begin, end); return; }
        const int per = (n + T_ - 1) / T_;
        {
            std::unique_lock<std::mutex> lk(m_);
            body_ = &body; begin_ = begin; end_ = end; per_ = per;
            remaining_ = T_;          // worker threads + this (master) thread
            ++gen_;
        }
        cv_go_.notify_all();
        // Master runs chunk 0 itself (no idle core).
        run_chunk(0);
        std::unique_lock<std::mutex> lk(m_);
        cv_done_.wait(lk, [&] { return remaining_ == 0; });
        body_ = nullptr;
    }

private:
    ThreadPool() {
        int t = pool_override();   // ftd::set_pool_threads() takes precedence
        // FTD_POOL_THREADS env fallback (note: Emscripten getenv does NOT see
        // the host process env, so the embind setter is the portable path).
        if (t <= 0) { if (const char* e = std::getenv("FTD_POOL_THREADS")) t = std::atoi(e); }
        if (t <= 0) {
            unsigned hw = std::thread::hardware_concurrency();
            t = static_cast<int>(hw == 0 ? 4u : std::min(hw, 8u));
        }
        T_ = t < 1 ? 1 : t;
        for (int i = 1; i < T_; ++i) workers_.emplace_back([this, i] { worker_loop(i); });
    }
    ~ThreadPool() {
        { std::unique_lock<std::mutex> lk(m_); stop_ = true; }
        cv_go_.notify_all();
        for (auto& t : workers_) if (t.joinable()) t.join();
    }

    void run_chunk(int id) {
        const int lo = begin_ + id * per_;
        const int hi = std::min(end_, lo + per_);
        if (lo < hi) (*body_)(lo, hi);
        if (--remaining_ == 0) { std::unique_lock<std::mutex> lk(m_); cv_done_.notify_one(); }
    }

    void worker_loop(int id) {
        int seen = 0;
        for (;;) {
            std::unique_lock<std::mutex> lk(m_);
            cv_go_.wait(lk, [&] { return stop_ || gen_ != seen; });
            if (stop_) return;
            seen = gen_;
            lk.unlock();
            run_chunk(id);
        }
    }

    int T_ = 1;
    std::vector<std::thread> workers_;
    std::mutex m_;
    std::condition_variable cv_go_, cv_done_;
    int gen_ = 0;
    std::atomic<int> remaining_{0};
    bool stop_ = false;
    // Current job:
    const std::function<void(int, int)>* body_ = nullptr;
    int begin_ = 0, end_ = 0, per_ = 0;
};

// Recursive so a critical body that transitively re-enters another
// with_critical on the same thread cannot self-deadlock (the engine's named
// OpenMP criticals are independent; one global lock is correct, just less
// concurrent — and these sites are not the hot inner loop).
inline std::recursive_mutex& critical_mutex() { static std::recursive_mutex m; return m; }

} // namespace detail

inline int parallel_max_threads() { return detail::ThreadPool::instance().size(); }

inline void set_pool_threads(int n) { detail::pool_override() = n; }

inline void parallel_for(int begin, int end, const std::function<void(int, int)>& body) {
    detail::ThreadPool::instance().run(begin, end, body);
}

template <class F>
inline void with_critical(F&& f) {
    std::lock_guard<std::recursive_mutex> g(detail::critical_mutex());
    f();
}

inline void atomic_inc(long long& x) {
    std::lock_guard<std::recursive_mutex> g(detail::critical_mutex());
    ++x;
}

// ============================================================================
//  Backend: _OPENMP (native — physics unchanged, golden-preserving)
// ============================================================================
#elif defined(_OPENMP)

inline int parallel_max_threads() { return omp_get_max_threads(); }

inline void set_pool_threads(int) {}

inline void parallel_for(int begin, int end, const std::function<void(int, int)>& body) {
    const int n = end - begin;
    if (n <= 0) return;
    #pragma omp parallel
    {
        const int nt = omp_get_num_threads();
        const int tid = omp_get_thread_num();
        const int per = (n + nt - 1) / nt;
        const int lo = begin + tid * per;
        const int hi = std::min(end, lo + per);
        if (lo < hi) body(lo, hi);
    }
}

namespace detail {
// A runtime OpenMP lock gives critical-section semantics from a plain function
// (so with_critical can be a template, unlike a lexical #pragma omp critical).
inline omp_lock_t& critical_lock() {
    static omp_lock_t l = [] { omp_lock_t x; omp_init_lock(&x); return x; }();
    return l;
}
} // namespace detail

template <class F>
inline void with_critical(F&& f) {
    omp_set_lock(&detail::critical_lock());
    f();
    omp_unset_lock(&detail::critical_lock());
}

inline void atomic_inc(long long& x) {
    #pragma omp atomic
    ++x;
}

// ============================================================================
//  Backend: serial
// ============================================================================
#else

inline int parallel_max_threads() { return 1; }

inline void set_pool_threads(int) {}

inline void parallel_for(int begin, int end, const std::function<void(int, int)>& body) {
    if (end > begin) body(begin, end);
}

template <class F>
inline void with_critical(F&& f) { f(); }

inline void atomic_inc(long long& x) { ++x; }

#endif

} // namespace ftd
