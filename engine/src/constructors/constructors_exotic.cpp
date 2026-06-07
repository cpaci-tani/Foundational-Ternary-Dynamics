// constructors_exotic.cpp
// Covers source lines 1081-1245 of the pre-split constructors.cpp:
//   Level 7 gravity/cosmology — schwarzschild, frw_patch, gravitational_wave
//   Level 8 reference frame context     — sloop, observer_cell

#include "ftd/constructors.h"
#include "./_common.h"

#include <cmath>
#include <vector>

namespace ftd {
namespace ctor {

using detail::safe_normalize;
using detail::merge_sites;

// ============================================================================
// Level 7 — gravity/cosmology
// ============================================================================

StampResult schwarzschild(RenderBridge& rb, Coord center, double r_s) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    StampResult result{"schwarzschild", 7, center, {}};

    // Outgoing waves should disperse into the void at the lattice faces, not
    // wrap/reflect. Enable the absorbing sponge here so it lands on whatever
    // bridge actually runs this scenario (the JS toggle path can't reliably
    // reach the WASM physics bridge under the dual-bridge routing).
    rb.toggles.absorbing_boundary = true;

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        double dx = x - static_cast<double>(center.x);
        double dy = y - static_cast<double>(center.y);
        double dz = z - static_cast<double>(center.z);
        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
        double r_eff = std::max(r, 0.5); // softening

        // latency = sqrt(r_s / r), clamped < 0.999
        double L = std::sqrt(r_s / r_eff);
        L = std::min(L, 0.999);

        int idx = lat.index(x, y, z);
        vox[idx].latency = L;
        result.sites.push_back(idx);
    }

    return result;
}

StampResult frw_patch(RenderBridge& rb, double density) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    Coord center{N / 2, N / 2, N / 2};
    StampResult result{"frw_patch", 7, center, {}};

    // Distribute particles at given density (fraction of sites)
    // Use deterministic pattern: place every stride-th site in flat index order,
    // alternating polarity for matter-antimatter balance.
    int total = N * N * N;
    int stride = static_cast<int>(1.0 / std::max(density, 1e-10));
    stride = std::max(stride, 1);

    int count = 0;
    for (int flat = 0; flat < total; flat += stride) {
        int z = flat % N;
        int y = (flat / N) % N;
        int x = flat / (N * N);
        int idx = lat.index(x, y, z);
        // Alternate sign: even-numbered particles +1, odd -1
        int8_t sign = (count % 2 == 0) ? static_cast<int8_t>(+1)
                                       : static_cast<int8_t>(-1);
        vox[idx].state = sign;
        result.sites.push_back(idx);
        ++count;
    }

    return result;
}

StampResult gravitational_wave(RenderBridge& rb, Vec3 direction,
                               double wavelength, double amplitude) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    Vec3 d_hat = safe_normalize(direction);
    const double k = 2.0 * PI / wavelength;

    Coord center{N / 2, N / 2, N / 2};
    StampResult result{"gravitational_wave", 7, center, {}};

    // Absorbing edges so the propagating wave disperses into the void instead of
    // wrapping (see schwarzschild() above for the dual-bridge rationale).
    rb.toggles.absorbing_boundary = true;

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{static_cast<double>(x), static_cast<double>(y),
               static_cast<double>(z)};
        double phase = k * d_hat.dot(r);
        double L = amplitude * std::sin(phase);

        int idx = lat.index(x, y, z);
        vox[idx].latency = L;
        result.sites.push_back(idx);
    }

    return result;
}

// ============================================================================
// Level 8 — reference frame context/observer
// ============================================================================

StampResult sloop(RenderBridge& rb, Coord center, int radius) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N_PARTICLES = 12;
    const double rad = static_cast<double>(radius);

    StampResult result{"sloop", 8, center, {}};

    // Place ~12 particles evenly spaced on a circle in the xy-plane
    std::vector<Coord> positions;
    positions.reserve(N_PARTICLES);
    for (int i = 0; i < N_PARTICLES; ++i) {
        double theta = 2.0 * PI * i / N_PARTICLES;
        int px = center.x + static_cast<int>(std::round(rad * std::cos(theta)));
        int py = center.y + static_cast<int>(std::round(rad * std::sin(theta)));
        positions.push_back({px, py, center.z});
    }

    // Stamp each particle with state=+1, flux pointing toward the NEXT particle
    for (int i = 0; i < N_PARTICLES; ++i) {
        const Coord& pos = positions[i];
        const Coord& next = positions[(i + 1) % N_PARTICLES];

        int idx = lat.index(pos.x, pos.y, pos.z);
        vox[idx].state = +1;

        // Flux direction: tangent to the ring (toward next particle)
        Vec3 dir{static_cast<double>(next.x - pos.x),
                 static_cast<double>(next.y - pos.y),
                 static_cast<double>(next.z - pos.z)};
        double d = dir.mag();
        if (d > 1e-10) {
            vox[idx].flux = dir * (K_B / d);
        }

        result.sites.push_back(idx);
    }

    std::sort(result.sites.begin(), result.sites.end());
    result.sites.erase(std::unique(result.sites.begin(), result.sites.end()),
                       result.sites.end());
    return result;
}

StampResult observer_cell(RenderBridge& rb, Coord center) {
    StampResult result{"observer_cell", 8, center, {}};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();

    // Center: state = +1 (the "self" -- the observer)
    int center_idx = lat.index(center.x, center.y, center.z);
    vox[center_idx].state = +1;
    result.sites.push_back(center_idx);

    // Shell 1 (6 face neighbors): state = -1 (the "mirror" -- sensory input)
    auto r_oct = octahedron(rb, center, -1);

    // Shell 2 (12 edge neighbors): state = +1 (the "frame" -- reference)
    auto r_cub = cuboctahedron(rb, center, +1);

    // Shell 3 (8 corner neighbors): state = -1 (the "context" -- environment)
    auto r_stel = stella_octangula(rb, center, -1);

    merge_sites(result.sites, r_oct.sites);
    merge_sites(result.sites, r_cub.sites);
    merge_sites(result.sites, r_stel.sites);
    return result;
}

}  // namespace ctor
}  // namespace ftd
