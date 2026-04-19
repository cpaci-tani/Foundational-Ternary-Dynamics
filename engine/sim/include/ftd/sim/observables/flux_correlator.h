#pragma once
/**
 * @file ftd/sim/observables/flux_correlator.h
 * @brief FluxCorrelator — direction-averaged ⟨J(x)·J(x+r)⟩ over separations r.
 *
 * The standard two-point function used everywhere in the EFT program.
 * Reproduces the pattern from engine/include/ftd/correlations.h
 * (::ftd::spatial_flux_correlation) — averages over all lattice sites
 * and the three positive cubic axes, with periodic wrap.
 *
 * Result type: std::vector<double> of length max_r. Entry r is
 * ⟨J(x)·J(x+r·μ̂)⟩ averaged over x and μ ∈ {x̂, ŷ, ẑ}.
 *
 * Backend-agnostic implementation via state.voxels(). Complexity O(L³·R).
 * For L=64, R=32 this is ~8M dot products per measurement — ~10ms on
 * CPU, comparable (sync-dominated) on GPU Phase-C baseline. Phase-D
 * optimisation would fuse the triple loop into a cub reduction.
 */

#include <cstddef>
#include <utility>
#include <vector>

#include "ftd/sim/observable.h"

namespace ftd {
namespace sim {

template <typename Backend>
class FluxCorrelator : public Observable<Backend, std::vector<double>> {
public:
    /// @param max_r  maximum separation r (default L/2 via -1 sentinel)
    explicit FluxCorrelator(int max_r = -1) : max_r_request_(max_r) {}

    void measure(typename Backend::DeviceState& state) override {
        const auto& vox = state.voxels();
        const auto& lat = state.lattice();
        const int L = lat.size();
        const int max_r = (max_r_request_ < 0 || max_r_request_ > L / 2)
                          ? L / 2 : max_r_request_;

        std::vector<double> C(max_r, 0.0);
        std::vector<long long> counts(max_r, 0);

        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    const int i0 = lat.index(x, y, z);
                    const Vec3& J0 = vox[i0].flux;
                    for (int r = 0; r < max_r; ++r) {
                        const int ix = lat.index(lat.wrap(x + r), y, z);
                        const int iy = lat.index(x, lat.wrap(y + r), z);
                        const int iz = lat.index(x, y, lat.wrap(z + r));
                        C[r] += J0.dot(vox[ix].flux);
                        C[r] += J0.dot(vox[iy].flux);
                        C[r] += J0.dot(vox[iz].flux);
                        counts[r] += 3;
                    }
                }
            }
        }
        for (int r = 0; r < max_r; ++r)
            if (counts[r] > 0) C[r] /= static_cast<double>(counts[r]);

        last_value_ = std::move(C);
        history_.emplace_back(state.tick(), last_value_);
    }

    std::vector<double> result_host() const override { return last_value_; }

    /// The full history of measurements, paired with the tick at which
    /// each was taken.
    const std::vector<std::pair<int, std::vector<double>>>& history() const {
        return history_;
    }

    void reset() override {
        last_value_.clear();
        history_.clear();
    }

private:
    int max_r_request_ = -1;
    std::vector<double> last_value_;
    std::vector<std::pair<int, std::vector<double>>> history_;
};

}  // namespace sim
}  // namespace ftd
