// constructors_molecules.cpp
// Covers source lines 499-733 of the pre-split constructors.cpp:
// Level 3 elementary particles (electron, positron, neutrino, quark, antiquark).
//
// NOTE: the ticket label "molecules" is a bucket name for the banner-based
// split — this TU actually holds the Level 3 elementary-particle constructors.
// (Actual molecule-scale constructors live in constructors_bulk_matter.cpp.)

#include "ftd/constructors.h"
#include "./_common.h"

#include <algorithm>
#include <cmath>

namespace ftd {
namespace ctor {

StampResult electron(RenderBridge& rb, Coord center, int8_t spin) {
    const Lattice& lat = rb.lattice();
    const int N = lat.size();
    const double sigma = std::max(3.0, N / 10.0);
    const double amplitude = K_B * 1.5;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const double cutoff_r2 = (GAUSSIAN_CUTOFF_SIGMA * sigma) *
                             (GAUSSIAN_CUTOFF_SIGMA * sigma);

    // Inject center particle
    rb.inject_particle(center.x, center.y, center.z,
                       /*state=*/-1, /*J=*/{0, 0, 0}, spin, /*color=*/0);

    StampResult r{"electron", 3, center, {}};
    auto& vox = rb.voxels();

    int center_idx = lat.index(center.x, center.y, center.z);
    r.sites.push_back(center_idx);

    // Stamp radial-inward flux envelope
    int range = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    for (int dx = -range; dx <= range; ++dx)
    for (int dy = -range; dy <= range; ++dy)
    for (int dz = -range; dz <= range; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        if (r2 > cutoff_r2) continue;
        double dist = std::sqrt(r2);
        double g = amplitude * std::exp(-r2 * inv_2sig2);
        // Flux pointing INWARD (toward center)
        Vec3 dir{static_cast<double>(-dx) / dist,
                 static_cast<double>(-dy) / dist,
                 static_cast<double>(-dz) / dist};
        Vec3 flux_val = dir * g;
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        vox[idx].flux += flux_val;
        r.sites.push_back(idx);
    }

    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

StampResult positron(RenderBridge& rb, Coord center, int8_t spin) {
    const Lattice& lat = rb.lattice();
    const int N = lat.size();
    const double sigma = std::max(3.0, N / 10.0);
    const double amplitude = K_B * 1.5;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const double cutoff_r2 = (GAUSSIAN_CUTOFF_SIGMA * sigma) *
                             (GAUSSIAN_CUTOFF_SIGMA * sigma);

    // Inject center particle
    rb.inject_particle(center.x, center.y, center.z,
                       /*state=*/+1, /*J=*/{0, 0, 0}, spin, /*color=*/0);

    StampResult r{"positron", 3, center, {}};
    auto& vox = rb.voxels();

    int center_idx = lat.index(center.x, center.y, center.z);
    r.sites.push_back(center_idx);

    // Stamp radial-outward flux envelope
    int range = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    for (int dx = -range; dx <= range; ++dx)
    for (int dy = -range; dy <= range; ++dy)
    for (int dz = -range; dz <= range; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        if (r2 > cutoff_r2) continue;
        double dist = std::sqrt(r2);
        double g = amplitude * std::exp(-r2 * inv_2sig2);
        // Flux pointing OUTWARD (away from center)
        Vec3 dir{static_cast<double>(dx) / dist,
                 static_cast<double>(dy) / dist,
                 static_cast<double>(dz) / dist};
        Vec3 flux_val = dir * g;
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        vox[idx].flux += flux_val;
        r.sites.push_back(idx);
    }

    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

StampResult neutrino(RenderBridge& rb, Coord center, int8_t chirality) {
    const Lattice& lat = rb.lattice();
    const double sigma = 2.0;
    const double amp = K_B * 0.3;
    const double delta = 0.1;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const int range = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;

    // State = 0 (no charge), spin = chirality
    rb.inject_particle(center.x, center.y, center.z,
                       /*state=*/0, /*J=*/{0, 0, 0},
                       /*spin=*/chirality, /*color=*/0);

    StampResult r{"neutrino", 3, center, {}};
    auto& vox = rb.voxels();

    int center_idx = lat.index(center.x, center.y, center.z);
    r.sites.push_back(center_idx);

    // Chirality seed: flux_L / flux_R asymmetry
    double d = (chirality == -1) ? delta : -delta;
    for (int dx = -range; dx <= range; ++dx)
    for (int dy = -range; dy <= range; ++dy)
    for (int dz = -range; dz <= range; ++dz) {
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        double cutoff_r2 = (GAUSSIAN_CUTOFF_SIGMA * sigma) *
                           (GAUSSIAN_CUTOFF_SIGMA * sigma);
        if (r2 > cutoff_r2) continue;
        double g = std::exp(-r2 * inv_2sig2);
        double base = amp * g;
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        // flux_L dominant for left-handed, flux_R dominant for right-handed
        double fl = base * (1.0 + d) / 2.0;
        double fr = base * (1.0 - d) / 2.0;
        vox[idx].flux_L = Vec3{fl, 0, 0};
        vox[idx].flux_R = Vec3{fr, 0, 0};
        r.sites.push_back(idx);
    }

    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

StampResult quark(RenderBridge& rb, Coord center,
                  int8_t charge, int8_t color, int8_t spin) {
    const Lattice& lat = rb.lattice();
    const double sigma = 2.0;
    const double amplitude = K_B * 0.5;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const int range = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    const double cutoff_r2 = (GAUSSIAN_CUTOFF_SIGMA * sigma) *
                             (GAUSSIAN_CUTOFF_SIGMA * sigma);

    // Inject center particle with state = charge, given spin and color
    rb.inject_particle(center.x, center.y, center.z,
                       /*state=*/charge, /*J=*/{0, 0, 0}, spin, color);

    StampResult r{"quark", 3, center, {}};
    auto& vox = rb.voxels();

    int center_idx = lat.index(center.x, center.y, center.z);
    r.sites.push_back(center_idx);

    // Small flux envelope: inward for negative charge, outward for positive
    double sign = (charge >= 0) ? -1.0 : 1.0;  // inward = toward center
    for (int dx = -range; dx <= range; ++dx)
    for (int dy = -range; dy <= range; ++dy)
    for (int dz = -range; dz <= range; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        if (r2 > cutoff_r2) continue;
        double dist = std::sqrt(r2);
        double g = amplitude * std::exp(-r2 * inv_2sig2);
        Vec3 dir{sign * static_cast<double>(dx) / dist,
                 sign * static_cast<double>(dy) / dist,
                 sign * static_cast<double>(dz) / dist};
        Vec3 flux_val = dir * g;
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        vox[idx].flux += flux_val;
        r.sites.push_back(idx);
    }

    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

StampResult antiquark(RenderBridge& rb, Coord center,
                      int8_t charge, int8_t color, int8_t spin) {
    const Lattice& lat = rb.lattice();
    const double sigma = 2.0;
    const double amplitude = K_B * 0.5;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const int range = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    const double cutoff_r2 = (GAUSSIAN_CUTOFF_SIGMA * sigma) *
                             (GAUSSIAN_CUTOFF_SIGMA * sigma);

    // Antimatter: state = -charge, flux direction reversed vs quark
    int8_t anti_state = static_cast<int8_t>(-charge);
    rb.inject_particle(center.x, center.y, center.z,
                       anti_state, /*J=*/{0, 0, 0}, spin, color);

    StampResult r{"antiquark", 3, center, {}};
    auto& vox = rb.voxels();

    int center_idx = lat.index(center.x, center.y, center.z);
    r.sites.push_back(center_idx);

    // Flux direction reversed relative to quark:
    // quark with charge>=0 has inward flux, so antiquark has outward, and vice versa
    double sign = (charge >= 0) ? 1.0 : -1.0;
    for (int dx = -range; dx <= range; ++dx)
    for (int dy = -range; dy <= range; ++dy)
    for (int dz = -range; dz <= range; ++dz) {
        if (dx == 0 && dy == 0 && dz == 0) continue;
        double r2 = static_cast<double>(dx*dx + dy*dy + dz*dz);
        if (r2 > cutoff_r2) continue;
        double dist = std::sqrt(r2);
        double g = amplitude * std::exp(-r2 * inv_2sig2);
        Vec3 dir{sign * static_cast<double>(dx) / dist,
                 sign * static_cast<double>(dy) / dist,
                 sign * static_cast<double>(dz) / dist};
        Vec3 flux_val = dir * g;
        int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
        vox[idx].flux += flux_val;
        r.sites.push_back(idx);
    }

    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

}  // namespace ctor
}  // namespace ftd
