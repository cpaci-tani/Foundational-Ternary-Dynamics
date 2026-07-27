#pragma once
/**
 * @file ftd/eft/emergent_boundary_observer.h
 * @brief Read-only manifested-boundary and free-wave-stress observer (FTD-0474).
 *
 * This observer never mutates RenderBridge. The stress is the selected
 * canonical spatial stress of the written quadratic free-flux action; it is
 * not asserted to be a complete matter-field stress tensor.
 */

#include <cmath>
#include <cstdint>
#include <queue>
#include <vector>

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

namespace ftd::eft {

struct EmergentBoundaryObservation {
    bool valid = false;
    int occupancy = 0;
    int boundary_sites = 0;
    int interior_sites = 0;
    double centroid_x = 0.0;
    double centroid_y = 0.0;
    double centroid_z = 0.0;
    double centroid_displacement = 0.0;
    double rms_radius = 0.0;
    double volume_radius = 0.0;
    double area_coefficient = 0.0;
    double radial_traction_jump = 0.0;
    double laplace_coefficient = 0.0;
    double interface_gradient_fraction = 0.0;
    double wave_kinetic_energy = 0.0;
};

namespace boundary_detail {

inline int min_image(int value, int origin, int L) {
    int d = value - origin;
    if (d > L / 2) d -= L;
    if (d < -L / 2) d += L;
    return d;
}

inline Vec3 central_flux_gradient(const RenderBridge& rb, int index, int axis) {
    const auto c = rb.lattice().coord(index);
    int xp = c.x, yp = c.y, zp = c.z;
    int xm = c.x, ym = c.y, zm = c.z;
    if (axis == 0) { ++xp; --xm; }
    if (axis == 1) { ++yp; --ym; }
    if (axis == 2) { ++zp; --zm; }
    const Vec3& plus = rb.voxels()[rb.lattice().index(xp, yp, zp)].flux;
    const Vec3& minus = rb.voxels()[rb.lattice().index(xm, ym, zm)].flux;
    return (plus - minus) * 0.5;
}

inline double gradient_energy(const RenderBridge& rb, int index) {
    const Vec3 gx = central_flux_gradient(rb, index, 0);
    const Vec3 gy = central_flux_gradient(rb, index, 1);
    const Vec3 gz = central_flux_gradient(rb, index, 2);
    return 0.5 * C_WAVE * C_WAVE
        * (gx.mag2() + gy.mag2() + gz.mag2());
}

inline double radial_traction(const RenderBridge& rb, int index,
                              const Vec3& normal) {
    const Vec3 gx = central_flux_gradient(rb, index, 0);
    const Vec3 gy = central_flux_gradient(rb, index, 1);
    const Vec3 gz = central_flux_gradient(rb, index, 2);
    const Vec3 radial_gradient = gx * normal.x
                               + gy * normal.y
                               + gz * normal.z;
    const double grad2 = gx.mag2() + gy.mag2() + gz.mag2();
    const double lagrangian_density =
        0.5 * rb.voxels()[index].wave_vel.mag2()
        - 0.5 * C_WAVE * C_WAVE * grad2;
    return C_WAVE * C_WAVE * radial_gradient.mag2()
         + lagrangian_density;
}

}  // namespace boundary_detail

inline EmergentBoundaryObservation observe_emergent_boundary(
        const RenderBridge& rb, int origin_x, int origin_y, int origin_z) {
    EmergentBoundaryObservation out;
    const int L = rb.lattice().size();
    const int total = static_cast<int>(rb.lattice().total_sites());
    const auto& voxels = rb.voxels();

    std::vector<std::uint8_t> visited(static_cast<std::size_t>(total), 0);
    std::vector<int> largest;
    std::vector<int> component;
    std::queue<int> queue;

    for (int seed = 0; seed < total; ++seed) {
        if (visited[seed] || voxels[seed].state == 0) continue;
        component.clear();
        visited[seed] = 1;
        queue.push(seed);
        while (!queue.empty()) {
            const int current = queue.front();
            queue.pop();
            component.push_back(current);
            for (int neighbor : rb.lattice().neighbors_26(current)) {
                if (visited[neighbor] || voxels[neighbor].state == 0) continue;
                visited[neighbor] = 1;
                queue.push(neighbor);
            }
        }
        if (component.size() > largest.size()) largest = component;
    }

    if (largest.empty()) return out;
    out.valid = true;
    out.occupancy = static_cast<int>(largest.size());

    std::vector<std::uint8_t> member(static_cast<std::size_t>(total), 0);
    for (int index : largest) member[index] = 1;

    const auto reference = rb.lattice().coord(largest.front());
    std::vector<Vec3> unwrapped;
    unwrapped.reserve(largest.size());
    Vec3 centroid;
    for (int index : largest) {
        const auto c = rb.lattice().coord(index);
        const Vec3 p{
            static_cast<double>(reference.x
                + boundary_detail::min_image(c.x, reference.x, L)),
            static_cast<double>(reference.y
                + boundary_detail::min_image(c.y, reference.y, L)),
            static_cast<double>(reference.z
                + boundary_detail::min_image(c.z, reference.z, L))};
        unwrapped.push_back(p);
        centroid += p;
    }
    centroid *= 1.0 / static_cast<double>(largest.size());
    out.centroid_x = centroid.x;
    out.centroid_y = centroid.y;
    out.centroid_z = centroid.z;

    const Vec3 origin{
        static_cast<double>(reference.x
            + boundary_detail::min_image(origin_x, reference.x, L)),
        static_cast<double>(reference.y
            + boundary_detail::min_image(origin_y, reference.y, L)),
        static_cast<double>(reference.z
            + boundary_detail::min_image(origin_z, reference.z, L))};
    out.centroid_displacement = (centroid - origin).mag();

    double radius2 = 0.0;
    for (const Vec3& p : unwrapped) radius2 += (p - centroid).mag2();
    out.rms_radius = std::sqrt(radius2 / static_cast<double>(largest.size()));
    out.volume_radius = std::cbrt(3.0 * static_cast<double>(out.occupancy)
                                  / (4.0 * PI));

    std::vector<int> boundary;
    std::vector<std::uint8_t> exterior_mask(static_cast<std::size_t>(total), 0);
    std::vector<int> exterior;
    for (int index : largest) {
        bool is_boundary = false;
        for (int neighbor : rb.lattice().neighbors_6(index)) {
            if (member[neighbor]) continue;
            is_boundary = true;
            if (!exterior_mask[neighbor]) {
                exterior_mask[neighbor] = 1;
                exterior.push_back(neighbor);
            }
        }
        if (is_boundary) boundary.push_back(index);
    }
    out.boundary_sites = static_cast<int>(boundary.size());
    out.interior_sites = out.occupancy - out.boundary_sites;
    out.area_coefficient = out.occupancy > 0
        ? static_cast<double>(out.boundary_sites)
            / std::pow(static_cast<double>(out.occupancy), 2.0 / 3.0)
        : 0.0;

    auto normal_at = [&](int index) {
        const auto c = rb.lattice().coord(index);
        const Vec3 p{
            static_cast<double>(reference.x
                + boundary_detail::min_image(c.x, reference.x, L)),
            static_cast<double>(reference.y
                + boundary_detail::min_image(c.y, reference.y, L)),
            static_cast<double>(reference.z
                + boundary_detail::min_image(c.z, reference.z, L))};
        const Vec3 d = p - centroid;
        const double r = d.mag();
        return r > 1e-15 ? d * (1.0 / r) : Vec3{1.0, 0.0, 0.0};
    };

    double inside_traction = 0.0;
    for (int index : boundary)
        inside_traction += boundary_detail::radial_traction(
            rb, index, normal_at(index));
    if (!boundary.empty()) inside_traction /= boundary.size();

    double outside_traction = 0.0;
    for (int index : exterior)
        outside_traction += boundary_detail::radial_traction(
            rb, index, normal_at(index));
    if (!exterior.empty()) outside_traction /= exterior.size();
    out.radial_traction_jump = inside_traction - outside_traction;
    out.laplace_coefficient = out.radial_traction_jump * out.volume_radius;

    double gradient_total = 0.0;
    for (int index = 0; index < total; ++index) {
        gradient_total += boundary_detail::gradient_energy(rb, index);
        out.wave_kinetic_energy += 0.5 * voxels[index].wave_vel.mag2();
    }
    double interface_gradient = 0.0;
    for (int index : boundary)
        interface_gradient += boundary_detail::gradient_energy(rb, index);
    for (int index : exterior)
        interface_gradient += boundary_detail::gradient_energy(rb, index);
    out.interface_gradient_fraction = gradient_total > 0.0
        ? interface_gradient / gradient_total : 0.0;
    return out;
}

}  // namespace ftd::eft

