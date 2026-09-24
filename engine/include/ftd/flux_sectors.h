#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace ftd {
// Observation only: every canonical site contributes, with no display threshold.
struct FluxSector {
    std::uint64_t total_sites = 0;
    std::uint64_t nonzero_sites = 0;
    double squared_norm = 0.0;
    double max_abs_component = 0.0;
};
struct FluxSectors {
    FluxSector sectors[2];
    std::uint64_t nonfinite_value_count = 0;
    std::uint64_t state_version = 0;
    int lattice_size = 0;
    std::int64_t tick = 0;
};
inline void accumulate_flux_sector(FluxSectors& out, int index,
                                   double x, double y, double z) {
    const int l = out.lattice_size;
    const int parity = ((index % l) + ((index / l) % l) + index / (l * l)) & 1;
    auto& sector = out.sectors[parity];
    ++sector.total_sites;
    if (x != 0.0 || y != 0.0 || z != 0.0) ++sector.nonzero_sites;
    const auto invalid = static_cast<unsigned>(!std::isfinite(x))
        + static_cast<unsigned>(!std::isfinite(y)) + static_cast<unsigned>(!std::isfinite(z));
    out.nonfinite_value_count += invalid;
    if (invalid) return;
    const double norm = x*x + y*y + z*z;
    if (!std::isfinite(norm)) { ++out.nonfinite_value_count; return; }
    sector.squared_norm += norm;
    sector.max_abs_component = std::max({sector.max_abs_component,
                                       std::abs(x), std::abs(y), std::abs(z)});
}
inline void validate_flux_sector_sums(FluxSectors& out) {
    for (auto& sector : out.sectors) {
        if (!std::isfinite(sector.squared_norm)) {
            ++out.nonfinite_value_count;
            sector.squared_norm = 0.0; // Invalid flag prevents interpreting this value.
        }
    }
}
} // namespace ftd
