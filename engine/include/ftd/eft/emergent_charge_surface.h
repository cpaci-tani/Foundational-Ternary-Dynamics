#pragma once

/**
 * @file emergent_charge_surface.h
 * @brief Read-only closed-surface charge observer for the native flux field.
 *
 * The boundary sum is the exact discrete divergence theorem for the central
 * difference operator used by RenderBridge::divergence_flux and the production
 * Gauss projector.  This header owns no state and performs no engine writes.
 */

#include "ftd/render_bridge.h"

#include <cmath>

namespace ftd::eft {

struct SurfaceChargeSample {
    int radius = 0;
    int enclosed_sites = 0;
    long long enclosed_polarity = 0;
    double mean_polarity = 0.0;
    double boundary_flux = 0.0;
    double divergence_sum = 0.0;
    double gauss_target = 0.0;
    double telescope_residual = 0.0;
    double gauss_residual = 0.0;
};

inline SurfaceChargeSample measure_central_cube_charge(
    const RenderBridge& rb, int cx, int cy, int cz, int radius) {
    SurfaceChargeSample out;
    out.radius = radius;
    if (radius < 0) return out;

    const Lattice& lattice = rb.lattice();
    const auto& voxels = rb.voxels();
    const int a_x = cx - radius;
    const int b_x = cx + radius;
    const int a_y = cy - radius;
    const int b_y = cy + radius;
    const int a_z = cz - radius;
    const int b_z = cz + radius;

    const auto flux = [&](int x, int y, int z) -> const Vec3& {
        return voxels[static_cast<std::size_t>(lattice.index(x, y, z))].flux;
    };

    // Boundary form of sum_R [J_x(x+1)-J_x(x-1)]/2, plus y and z.
    for (int y = a_y; y <= b_y; ++y) {
        for (int z = a_z; z <= b_z; ++z) {
            out.boundary_flux += 0.5 * (
                flux(b_x + 1, y, z).x + flux(b_x, y, z).x
                - flux(a_x, y, z).x - flux(a_x - 1, y, z).x);
        }
    }
    for (int x = a_x; x <= b_x; ++x) {
        for (int z = a_z; z <= b_z; ++z) {
            out.boundary_flux += 0.5 * (
                flux(x, b_y + 1, z).y + flux(x, b_y, z).y
                - flux(x, a_y, z).y - flux(x, a_y - 1, z).y);
        }
    }
    for (int x = a_x; x <= b_x; ++x) {
        for (int y = a_y; y <= b_y; ++y) {
            out.boundary_flux += 0.5 * (
                flux(x, y, b_z + 1).z + flux(x, y, b_z).z
                - flux(x, y, a_z).z - flux(x, y, a_z - 1).z);
        }
    }

    for (int x = a_x; x <= b_x; ++x) {
        for (int y = a_y; y <= b_y; ++y) {
            for (int z = a_z; z <= b_z; ++z) {
                const int i = lattice.index(x, y, z);
                const auto& n = lattice.neighbors_6(i);
                out.divergence_sum +=
                    0.5 * (voxels[static_cast<std::size_t>(n[0])].flux.x
                           - voxels[static_cast<std::size_t>(n[1])].flux.x)
                    + 0.5 * (voxels[static_cast<std::size_t>(n[2])].flux.y
                             - voxels[static_cast<std::size_t>(n[3])].flux.y)
                    + 0.5 * (voxels[static_cast<std::size_t>(n[4])].flux.z
                             - voxels[static_cast<std::size_t>(n[5])].flux.z);
                out.enclosed_polarity += voxels[static_cast<std::size_t>(i)].state;
                ++out.enclosed_sites;
            }
        }
    }

    out.mean_polarity = static_cast<double>(rb.charge_sum())
        / static_cast<double>(lattice.total_sites());
    out.gauss_target = rb.toggles.coulomb_charge_coupling
        * (static_cast<double>(out.enclosed_polarity)
           - out.mean_polarity * static_cast<double>(out.enclosed_sites));
    out.telescope_residual = out.boundary_flux - out.divergence_sum;
    out.gauss_residual = out.boundary_flux - out.gauss_target;
    return out;
}

inline bool central_cube_telescope_closes(const SurfaceChargeSample& sample,
                                          double relative_tolerance = 1e-12) {
    return std::abs(sample.telescope_residual)
        <= relative_tolerance * (1.0 + std::abs(sample.divergence_sum));
}

}  // namespace ftd::eft
