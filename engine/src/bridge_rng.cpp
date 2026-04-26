/**
 * @file bridge_rng.cpp
 * @brief PIMPL'd RNG state implementation.
 *
 * RF-9: hides <random> from the public render_bridge.h surface. See
 * include/ftd/bridge_rng.h for the rationale.
 */

#include "ftd/bridge_rng.h"

#include <random>
#include <vector>

namespace ftd {

struct BridgeRng::Impl {
    std::mt19937 rng{42};
    std::uniform_real_distribution<double> uniform{0.0, 1.0};
    std::normal_distribution<double> normal{0.0, 1.0};
    std::vector<std::mt19937> thread_rngs;
};

BridgeRng::BridgeRng() : impl_(new Impl) {}

BridgeRng::BridgeRng(unsigned int seed) : impl_(new Impl) {
    impl_->rng.seed(seed);
}

BridgeRng::~BridgeRng() { delete impl_; }

void BridgeRng::seed(unsigned int s) { impl_->rng.seed(s); }

double BridgeRng::sample_uniform() { return impl_->uniform(impl_->rng); }

void BridgeRng::resize_thread_pool(std::size_t n) {
    impl_->thread_rngs.resize(n);
}

void BridgeRng::reseed_thread_pool(unsigned int* thread_seeds_out, std::size_t n) {
    if (impl_->thread_rngs.size() < n) impl_->thread_rngs.resize(n);
    for (std::size_t t = 0; t < n; ++t) {
        const unsigned int s = static_cast<unsigned int>(impl_->rng());
        thread_seeds_out[t] = s;
        impl_->thread_rngs[t].seed(s);
    }
}

double BridgeRng::thread_uniform(std::size_t tid) {
    // Distribution objects are stateless for uniform_real / normal so a
    // local instance is fine; reusing the impl_->uniform across threads
    // is NOT safe (mutable state inside the distribution).
    std::uniform_real_distribution<double> u(0.0, 1.0);
    return u(impl_->thread_rngs[tid]);
}

double BridgeRng::thread_normal(std::size_t tid) {
    std::normal_distribution<double> g(0.0, 1.0);
    return g(impl_->thread_rngs[tid]);
}

}  // namespace ftd
