// constructors_bulk_matter.cpp
// Covers source lines 736-1078 of the pre-split constructors.cpp:
//   Level 4 composites   — pion, proton, neutron
//   Level 5 atoms/mol    — hydrogen, helium, h2_molecule
//   Level 6 gauge/topo   — wilson_loop, flux_tube, monopole, instanton
//
// NOTE: the ticket label "bulk_matter" is a bucket name for the banner-based
// split. This TU groups the mid-tier composite, atomic, and gauge
// constructors; no true bulk-crystal/plasma/gas constructors exist yet.

#include "ftd/constructors.h"
#include "./_common.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace ftd {
namespace ctor {

using detail::safe_normalize;
using detail::merge_sites;

// ============================================================================
// Level 4 — composite particles
// ============================================================================

StampResult pion(RenderBridge& rb, Coord center, int separation) {
    int half = separation / 2;

    // Quark at center + (half, 0, 0)
    Coord q_pos{center.x + half, center.y, center.z};
    auto rq = quark(rb, q_pos, /*charge=*/+1, /*color=*/1, /*spin=*/+1);

    // Antiquark at center - (half, 0, 0)
    Coord aq_pos{center.x - half, center.y, center.z};
    auto raq = antiquark(rb, aq_pos, /*charge=*/+1, /*color=*/1, /*spin=*/-1);

    StampResult r{"pion", 4, center, {}};
    merge_sites(r.sites, rq.sites);
    merge_sites(r.sites, raq.sites);
    return r;
}

StampResult proton(RenderBridge& rb, Coord center, int radius) {
    // Three quarks on equilateral triangle in xy-plane:
    // u(+1, red) at 0deg, u(+1, green) at 120deg, d(-1, blue) at 240deg
    const double r = static_cast<double>(radius);

    // Angle offsets: 0, 2pi/3, 4pi/3
    int ux1 = center.x + static_cast<int>(std::round(r * std::cos(0.0)));
    int uy1 = center.y + static_cast<int>(std::round(r * std::sin(0.0)));
    auto r1 = quark(rb, {ux1, uy1, center.z}, /*charge=*/+1, /*color=*/1, /*spin=*/+1);

    int ux2 = center.x + static_cast<int>(std::round(r * std::cos(2.0 * PI / 3.0)));
    int uy2 = center.y + static_cast<int>(std::round(r * std::sin(2.0 * PI / 3.0)));
    auto r2 = quark(rb, {ux2, uy2, center.z}, /*charge=*/+1, /*color=*/2, /*spin=*/-1);

    int dx3 = center.x + static_cast<int>(std::round(r * std::cos(4.0 * PI / 3.0)));
    int dy3 = center.y + static_cast<int>(std::round(r * std::sin(4.0 * PI / 3.0)));
    auto r3 = quark(rb, {dx3, dy3, center.z}, /*charge=*/-1, /*color=*/3, /*spin=*/+1);

    StampResult result{"proton", 4, center, {}};
    merge_sites(result.sites, r1.sites);
    merge_sites(result.sites, r2.sites);
    merge_sites(result.sites, r3.sites);
    return result;
}

StampResult neutron(RenderBridge& rb, Coord center, int radius) {
    // Three quarks on equilateral triangle in xy-plane:
    // u(+1, red) at 0deg, d(-1, green) at 120deg, d(-1, blue) at 240deg
    const double r = static_cast<double>(radius);

    int ux1 = center.x + static_cast<int>(std::round(r * std::cos(0.0)));
    int uy1 = center.y + static_cast<int>(std::round(r * std::sin(0.0)));
    auto r1 = quark(rb, {ux1, uy1, center.z}, /*charge=*/+1, /*color=*/1, /*spin=*/+1);

    int dx2 = center.x + static_cast<int>(std::round(r * std::cos(2.0 * PI / 3.0)));
    int dy2 = center.y + static_cast<int>(std::round(r * std::sin(2.0 * PI / 3.0)));
    auto r2 = quark(rb, {dx2, dy2, center.z}, /*charge=*/-1, /*color=*/2, /*spin=*/-1);

    int dx3 = center.x + static_cast<int>(std::round(r * std::cos(4.0 * PI / 3.0)));
    int dy3 = center.y + static_cast<int>(std::round(r * std::sin(4.0 * PI / 3.0)));
    auto r3 = quark(rb, {dx3, dy3, center.z}, /*charge=*/-1, /*color=*/3, /*spin=*/+1);

    StampResult result{"neutron", 4, center, {}};
    merge_sites(result.sites, r1.sites);
    merge_sites(result.sites, r2.sites);
    merge_sites(result.sites, r3.sites);
    return result;
}

// ============================================================================
// Level 5 — atoms & molecules
// ============================================================================

StampResult hydrogen(RenderBridge& rb, Coord center, int orbital_radius) {
    // Proton at center
    auto rp = proton(rb, center);

    // Electron at center + (0, 0, orbital_radius)
    Coord e_pos{center.x, center.y, center.z + orbital_radius};
    auto re = electron(rb, e_pos, /*spin=*/-1);

    StampResult result{"hydrogen", 5, center, {}};
    merge_sites(result.sites, rp.sites);
    merge_sites(result.sites, re.sites);
    return result;
}

StampResult helium(RenderBridge& rb, Coord center, int orbital_radius) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();

    StampResult result{"helium", 5, center, {}};

    // Nucleus: single site with state=+1, flux amplitude 2*K_B (charge +2)
    rb.inject_particle(center.x, center.y, center.z,
                       /*state=*/+1, Vec3{0, 0, 2.0 * K_B},
                       /*spin=*/0, /*color=*/0);
    int nuc_idx = lat.index(center.x, center.y, center.z);
    result.sites.push_back(nuc_idx);

    // Electron 1 at +orbital_radius along z, spin up
    Coord e1_pos{center.x, center.y, center.z + orbital_radius};
    auto re1 = electron(rb, e1_pos, /*spin=*/+1);

    // Electron 2 at -orbital_radius along z, spin down
    Coord e2_pos{center.x, center.y, center.z - orbital_radius};
    auto re2 = electron(rb, e2_pos, /*spin=*/-1);

    merge_sites(result.sites, re1.sites);
    merge_sites(result.sites, re2.sites);
    return result;
}

StampResult h2_molecule(RenderBridge& rb, Coord center,
                        int bond_length, int orbital_radius) {
    int half = bond_length / 2;

    // Hydrogen atom 1 at center - (half, 0, 0)
    Coord h1_pos{center.x - half, center.y, center.z};
    auto rh1 = hydrogen(rb, h1_pos, orbital_radius);

    // Hydrogen atom 2 at center + (half, 0, 0)
    Coord h2_pos{center.x + half, center.y, center.z};
    auto rh2 = hydrogen(rb, h2_pos, orbital_radius);

    StampResult result{"h2_molecule", 5, center, {}};
    merge_sites(result.sites, rh1.sites);
    merge_sites(result.sites, rh2.sites);
    return result;
}

// ============================================================================
// Level 6 — gauge/topological objects
// ============================================================================

StampResult wilson_loop(RenderBridge& rb, Coord center,
                        int radius, double flux_strength) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int R = radius;
    const int z = center.z;

    StampResult result{"wilson_loop", 6, center, {}};

    // Four corners: place +1 particles
    std::array<Coord, 4> corners = {{
        {center.x - R, center.y - R, z},
        {center.x + R, center.y - R, z},
        {center.x + R, center.y + R, z},
        {center.x - R, center.y + R, z},
    }};
    for (const auto& c : corners) {
        int idx = lat.index(c.x, c.y, c.z);
        vox[idx].state = +1;
        result.sites.push_back(idx);
    }

    // Bottom edge: flux in +x direction
    for (int x = center.x - R; x <= center.x + R; ++x) {
        int idx = lat.index(x, center.y - R, z);
        vox[idx].flux += Vec3{flux_strength, 0.0, 0.0};
        result.sites.push_back(idx);
    }
    // Right edge: flux in +y direction
    for (int y = center.y - R; y <= center.y + R; ++y) {
        int idx = lat.index(center.x + R, y, z);
        vox[idx].flux += Vec3{0.0, flux_strength, 0.0};
        result.sites.push_back(idx);
    }
    // Top edge: flux in -x direction
    for (int x = center.x + R; x >= center.x - R; --x) {
        int idx = lat.index(x, center.y + R, z);
        vox[idx].flux += Vec3{-flux_strength, 0.0, 0.0};
        result.sites.push_back(idx);
    }
    // Left edge: flux in -y direction
    for (int y = center.y + R; y >= center.y - R; --y) {
        int idx = lat.index(center.x - R, y, z);
        vox[idx].flux += Vec3{0.0, -flux_strength, 0.0};
        result.sites.push_back(idx);
    }

    std::sort(result.sites.begin(), result.sites.end());
    result.sites.erase(std::unique(result.sites.begin(), result.sites.end()),
                       result.sites.end());
    return result;
}

StampResult flux_tube(RenderBridge& rb, Coord end_a, Coord end_b,
                      double strength) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    Coord center{(end_a.x + end_b.x) / 2,
                 (end_a.y + end_b.y) / 2,
                 (end_a.z + end_b.z) / 2};
    StampResult result{"flux_tube", 6, center, {}};

    // Place quark at end_a (state=+1, color=1)
    int idx_a = lat.index(end_a.x, end_a.y, end_a.z);
    vox[idx_a].state = +1;
    vox[idx_a].color = 1;
    result.sites.push_back(idx_a);

    // Place antiquark at end_b (state=-1, color=1)
    int idx_b = lat.index(end_b.x, end_b.y, end_b.z);
    vox[idx_b].state = -1;
    vox[idx_b].color = 1;
    result.sites.push_back(idx_b);

    // Direction vector from a to b
    Vec3 ab{static_cast<double>(end_b.x - end_a.x),
            static_cast<double>(end_b.y - end_a.y),
            static_cast<double>(end_b.z - end_a.z)};
    double ab_len = ab.mag();
    if (ab_len < 1e-10) return result;
    Vec3 ab_hat = safe_normalize(ab);

    const double sigma = 1.5;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const double cutoff_perp = 3.0 * sigma; // 3-sigma cutoff

    // Iterate all sites, stamp flux along tube
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{static_cast<double>(x - end_a.x),
               static_cast<double>(y - end_a.y),
               static_cast<double>(z - end_a.z)};
        double t = r.dot(ab_hat);  // projection along tube axis
        if (t < -0.5 || t > ab_len + 0.5) continue;

        // Perpendicular distance
        Vec3 proj = ab_hat * t;
        Vec3 perp = r - proj;
        double d_perp = perp.mag();
        if (d_perp > cutoff_perp) continue;

        double gauss = std::exp(-d_perp * d_perp * inv_2sig2);
        Vec3 flux_val = ab_hat * (strength * gauss);
        if (flux_val.mag2() > 1e-30) {
            int idx = lat.index(x, y, z);
            vox[idx].flux += flux_val;
            result.sites.push_back(idx);
        }
    }

    std::sort(result.sites.begin(), result.sites.end());
    result.sites.erase(std::unique(result.sites.begin(), result.sites.end()),
                       result.sites.end());
    return result;
}

StampResult monopole(RenderBridge& rb, Coord center, double charge) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    StampResult result{"monopole", 6, center, {}};

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        double dx = x - static_cast<double>(center.x);
        double dy = y - static_cast<double>(center.y);
        double dz = z - static_cast<double>(center.z);
        double r = std::sqrt(dx*dx + dy*dy + dz*dz);
        double r_eff = std::max(r, 1.0); // softening

        if (r < 1e-10) continue; // skip center

        // Tangential flux: theta_hat-like in spherical coords
        // For a monopole, we want J such that curl(J) ~ r_hat/r^2.
        // Use J = charge * theta_hat / (4*pi*r) with 1/r falloff.
        // theta_hat = (cos(theta)cos(phi), cos(theta)sin(phi), -sin(theta))
        double rho = std::sqrt(dx*dx + dy*dy);
        double cos_theta = dz / r_eff;
        double sin_theta = rho / r_eff;

        Vec3 flux_val;
        if (rho > 1e-10) {
            double cos_phi = dx / rho;
            double sin_phi = dy / rho;
            // theta_hat direction
            Vec3 theta_hat{cos_theta * cos_phi,
                           cos_theta * sin_phi,
                           -sin_theta};
            flux_val = theta_hat * (charge / (4.0 * PI * r_eff));
        } else {
            // On the z-axis: theta_hat is ill-defined, skip
            continue;
        }

        if (flux_val.mag2() > 1e-30) {
            int idx = lat.index(x, y, z);
            vox[idx].flux += flux_val;
            result.sites.push_back(idx);
        }
    }

    std::sort(result.sites.begin(), result.sites.end());
    result.sites.erase(std::unique(result.sites.begin(), result.sites.end()),
                       result.sites.end());
    return result;
}

StampResult instanton(RenderBridge& rb, Coord center, double size) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();
    const double rho2 = size * size;

    StampResult result{"instanton", 6, center, {}};

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        double dx = x - static_cast<double>(center.x);
        double dy = y - static_cast<double>(center.y);
        double dz = z - static_cast<double>(center.z);
        double r2 = dx*dx + dy*dy + dz*dz;
        double r = std::sqrt(r2);

        if (r < 1e-10) continue; // skip center

        // BPST-like profile: size / (r^2 + size^2) * r_hat
        double profile = size / (r2 + rho2);
        Vec3 r_hat{dx / r, dy / r, dz / r};
        Vec3 flux_val = r_hat * profile;

        if (flux_val.mag2() > 1e-30) {
            int idx = lat.index(x, y, z);
            vox[idx].flux += flux_val;
            result.sites.push_back(idx);
        }
    }

    std::sort(result.sites.begin(), result.sites.end());
    result.sites.erase(std::unique(result.sites.begin(), result.sites.end()),
                       result.sites.end());
    return result;
}

}  // namespace ctor
}  // namespace ftd
