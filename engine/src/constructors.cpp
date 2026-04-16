#include "ftd/constructors.h"
#include "ftd/render_bridge.h"
#include "ftd/lattice.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

namespace ftd {
namespace ctor {

namespace {

using SnapshotEntry = std::pair<int, Voxel>;
using Snapshot      = std::vector<SnapshotEntry>;

inline bool voxel_changed(const Voxel& a, const Voxel& b) {
    return a.state != b.state
        || a.flux.x  != b.flux.x  || a.flux.y  != b.flux.y  || a.flux.z  != b.flux.z
        || a.flux_L.x != b.flux_L.x || a.flux_L.y != b.flux_L.y || a.flux_L.z != b.flux_L.z
        || a.flux_R.x != b.flux_R.x || a.flux_R.y != b.flux_R.y || a.flux_R.z != b.flux_R.z
        || a.spin  != b.spin
        || a.color != b.color;
}

Snapshot snapshot_box(const RenderBridge& rb, Coord center, int radius) {
    Snapshot out;
    const Lattice& lat = rb.lattice();
    const auto& vox = rb.voxels();
    const int side = 2 * radius + 1;
    out.reserve(static_cast<size_t>(side) * side * side);
    for (int dx = -radius; dx <= radius; ++dx)
        for (int dy = -radius; dy <= radius; ++dy)
            for (int dz = -radius; dz <= radius; ++dz) {
                int idx = lat.index(center.x + dx, center.y + dy, center.z + dz);
                out.push_back({idx, vox[idx]});
            }
    return out;
}

std::vector<int> diff_sites(const RenderBridge& rb, const Snapshot& before) {
    const auto& vox = rb.voxels();
    std::vector<int> changed;
    changed.reserve(before.size());
    for (const auto& entry : before) {
        if (voxel_changed(vox[entry.first], entry.second)) changed.push_back(entry.first);
    }
    std::sort(changed.begin(), changed.end());
    changed.erase(std::unique(changed.begin(), changed.end()), changed.end());
    return changed;
}

}  // anonymous namespace

// Level 0 implementations
StampResult flux(RenderBridge& rb, Coord at, Vec3 J) {
    auto before = snapshot_box(rb, at, 0);
    rb.inject_flux(at.x, at.y, at.z, J);
    return StampResult{"flux", 0, at, diff_sites(rb, before)};
}

StampResult particle(RenderBridge& rb, Coord at, int8_t state, Vec3 J,
                     int8_t spin, int8_t color) {
    auto before = snapshot_box(rb, at, 0);
    rb.inject_particle(at.x, at.y, at.z, state, J, spin, color);
    return StampResult{"particle", 0, at, diff_sites(rb, before)};
}

StampResult wavepacket(RenderBridge& rb, Coord at, int8_t state, double sigma, double amp) {
    const int radius = static_cast<int>(GAUSSIAN_CUTOFF_SIGMA * sigma) + 1;
    auto before = snapshot_box(rb, at, radius);
    rb.inject_wavepacket(at.x, at.y, at.z, state, sigma, amp);
    return StampResult{"wavepacket", 0, at, diff_sites(rb, before)};
}

StampResult entangled_pair(RenderBridge& rb, Coord at, Vec3 J) {
    auto before = snapshot_box(rb, at, 1);
    rb.create_entangled_pair(at.x, at.y, at.z, J);
    return StampResult{"entangled_pair", 0, at, diff_sites(rb, before)};
}

// Level 1A implementations
StampResult octahedron(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 6> OFFSETS = {{
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"octahedron", 1, center, {}};
    r.sites.reserve(6);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult cuboctahedron(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 12> OFFSETS = {{
        { 1, 1, 0}, { 1,-1, 0}, {-1, 1, 0}, {-1,-1, 0},
        { 1, 0, 1}, { 1, 0,-1}, {-1, 0, 1}, {-1, 0,-1},
        { 0, 1, 1}, { 0, 1,-1}, { 0,-1, 1}, { 0,-1,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"cuboctahedron", 1, center, {}};
    r.sites.reserve(12);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult stella_octangula(RenderBridge& rb, Coord center, int8_t state) {
    static constexpr std::array<std::array<int,3>, 8> OFFSETS = {{
        { 1, 1, 1}, { 1, 1,-1}, { 1,-1, 1}, { 1,-1,-1},
        {-1, 1, 1}, {-1, 1,-1}, {-1,-1, 1}, {-1,-1,-1},
    }};
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    StampResult r{"stella_octangula", 1, center, {}};
    r.sites.reserve(8);
    for (const auto& o : OFFSETS) {
        int idx = lat.index(center.x + o[0], center.y + o[1], center.z + o[2]);
        vox[idx].state = state;
        r.sites.push_back(idx);
    }
    return r;
}

StampResult moore_cell(RenderBridge& rb, Coord center, int8_t state) {
    auto r_oct  = octahedron(rb, center, state);
    auto r_cub  = cuboctahedron(rb, center, state);
    auto r_stel = stella_octangula(rb, center, state);

    StampResult r{"moore_cell", 1, center, {}};
    r.sites.reserve(r_oct.sites.size() + r_cub.sites.size() + r_stel.sites.size());
    r.sites.insert(r.sites.end(), r_oct.sites.begin(),  r_oct.sites.end());
    r.sites.insert(r.sites.end(), r_cub.sites.begin(),  r_cub.sites.end());
    r.sites.insert(r.sites.end(), r_stel.sites.begin(), r_stel.sites.end());
    std::sort(r.sites.begin(), r.sites.end());
    r.sites.erase(std::unique(r.sites.begin(), r.sites.end()), r.sites.end());
    return r;
}

// ============================================================================
// Level 2 — field configurations
// ============================================================================

namespace {

// Normalize a vector; returns zero vector if input is near-zero.
inline Vec3 safe_normalize(Vec3 v) {
    double m = v.mag();
    if (m < 1e-30) return {0.0, 0.0, 0.0};
    return v * (1.0 / m);
}

// Build an orthonormal basis where u1 = normalized(a).
// Returns two vectors perpendicular to a and to each other.
inline void ortho_basis(Vec3 a_norm, Vec3& e1, Vec3& e2) {
    // Pick a vector not parallel to a_norm
    Vec3 tmp = (std::abs(a_norm.x) < 0.9) ? Vec3{1,0,0} : Vec3{0,1,0};
    e1 = safe_normalize(Vec3::cross(a_norm, tmp));
    e2 = Vec3::cross(a_norm, e1);
}

}  // anonymous namespace

StampResult plane_wave(RenderBridge& rb,
                       Vec3 direction,
                       Vec3 polarization,
                       double wavelength,
                       double amplitude) {
    const int N = rb.lattice().size();
    Vec3 d_hat = safe_normalize(direction);
    Vec3 p_hat = safe_normalize(polarization);
    const double k = 2.0 * PI / wavelength;
    Coord center{N/2, N/2, N/2};
    StampResult result{"plane_wave", 2, center, {}};
    auto& vox = rb.voxels();

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)};
        double phase = k * d_hat.dot(r);
        double s = std::sin(phase);
        double c = std::cos(phase);
        Vec3 flux_val = p_hat * (amplitude * s);
        Vec3 wvel_val = p_hat * (amplitude * c * C_SPEED);
        double mag2 = flux_val.mag2() + wvel_val.mag2();
        if (mag2 > 1e-30) {
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = flux_val;
            vox[idx].wave_vel = wvel_val;
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult standing_wave(RenderBridge& rb,
                          Vec3 direction,
                          Vec3 polarization,
                          double wavelength,
                          double amplitude) {
    const int N = rb.lattice().size();
    Vec3 d_hat = safe_normalize(direction);
    Vec3 p_hat = safe_normalize(polarization);
    const double k = 2.0 * PI / wavelength;
    Coord center{N/2, N/2, N/2};
    StampResult result{"standing_wave", 2, center, {}};
    auto& vox = rb.voxels();

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{static_cast<double>(x), static_cast<double>(y), static_cast<double>(z)};
        double phase = k * d_hat.dot(r);
        double s = std::sin(phase);
        Vec3 flux_val = p_hat * (amplitude * s);
        if (flux_val.mag2() > 1e-30) {
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = flux_val;
            // wave_vel stays zero (standing wave)
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult uniform_e(RenderBridge& rb, Vec3 E) {
    const int N = rb.lattice().size();
    Vec3 neg_E = E * (-1.0); // wave_vel = -E
    Coord center{N/2, N/2, N/2};
    StampResult result{"uniform_e", 2, center, {}};

    if (E.mag2() < 1e-30) return result;

    auto& vox = rb.voxels();
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        int idx = rb.lattice().index(x, y, z);
        vox[idx].wave_vel = neg_E;
        result.sites.push_back(idx);
    }
    return result;
}

StampResult uniform_b(RenderBridge& rb, Vec3 B) {
    const int N = rb.lattice().size();
    const double mid = (N - 1) / 2.0;
    Vec3 B_hat = safe_normalize(B);
    double B_mag = B.mag();
    Coord center{N/2, N/2, N/2};
    StampResult result{"uniform_b", 2, center, {}};

    if (B_mag < 1e-30) return result;

    // Build orthonormal frame: B_hat is the "z" axis, e1 and e2 span the plane.
    Vec3 e1, e2;
    ortho_basis(B_hat, e1, e2);

    auto& vox = rb.voxels();
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{x - mid, y - mid, z - mid};
        // Project r onto the plane perpendicular to B
        double r1 = r.dot(e1);
        double r2 = r.dot(e2);
        // Vector potential A = (B/2) x r  =>  J = (-B_mag*r2/2, B_mag*r1/2, 0)
        // in the (e1, e2) plane
        Vec3 J_val = e1 * (-B_mag * r2 / 2.0) + e2 * (B_mag * r1 / 2.0);
        if (J_val.mag2() > 1e-30) {
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = J_val;
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult photon_pulse(RenderBridge& rb,
                         Coord center,
                         Vec3 direction,
                         Vec3 polarization,
                         double sigma,
                         double amplitude) {
    const int N = rb.lattice().size();
    Vec3 d_hat = safe_normalize(direction);
    Vec3 p_hat = safe_normalize(polarization);
    const double lambda_eff = 4.0 * sigma;
    const double k = 2.0 * PI / lambda_eff;
    const double inv_2sig2 = 1.0 / (2.0 * sigma * sigma);
    const double g_cutoff = 1e-6;
    const double r2_cutoff = -2.0 * sigma * sigma * std::log(g_cutoff);

    StampResult result{"photon_pulse", 2, center, {}};
    auto& vox = rb.voxels();

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        double dx = x - center.x;
        double dy = y - center.y;
        double dz = z - center.z;
        double r2 = dx*dx + dy*dy + dz*dz;
        if (r2 > r2_cutoff) continue;

        double g = std::exp(-r2 * inv_2sig2);
        Vec3 disp{dx, dy, dz};
        double phase = k * d_hat.dot(disp);
        double s = std::sin(phase);
        double c = std::cos(phase);
        Vec3 flux_val = p_hat * (amplitude * g * s);
        Vec3 wvel_val = p_hat * (amplitude * g * c * C_SPEED);
        double mag2 = flux_val.mag2() + wvel_val.mag2();
        if (mag2 > 1e-30) {
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = flux_val;
            vox[idx].wave_vel = wvel_val;
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult electric_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 axis,
                            int separation) {
    const int N = rb.lattice().size();
    Vec3 a_hat = safe_normalize(axis);
    double half_sep = separation / 2.0;

    // Positive charge position
    int px = center.x + static_cast<int>(std::round(a_hat.x * half_sep));
    int py = center.y + static_cast<int>(std::round(a_hat.y * half_sep));
    int pz = center.z + static_cast<int>(std::round(a_hat.z * half_sep));

    // Negative charge position
    int nx = center.x - static_cast<int>(std::round(a_hat.x * half_sep));
    int ny = center.y - static_cast<int>(std::round(a_hat.y * half_sep));
    int nz = center.z - static_cast<int>(std::round(a_hat.z * half_sep));

    StampResult result{"electric_dipole", 2, center, {}};
    auto& vox = rb.voxels();

    // Place the two charges
    int idx_pos = rb.lattice().index(px, py, pz);
    int idx_neg = rb.lattice().index(nx, ny, nz);
    vox[idx_pos].state = +1;
    vox[idx_pos].flux = a_hat * K_B;
    vox[idx_neg].state = -1;
    vox[idx_neg].flux = a_hat * (-K_B);

    // Coulomb dressing: superpose 1/r^2 field from both charges
    // Wrap-aware positions of the two charges
    int wp_x = rb.lattice().wrap(px);
    int wp_y = rb.lattice().wrap(py);
    int wp_z = rb.lattice().wrap(pz);
    int wn_x = rb.lattice().wrap(nx);
    int wn_y = rb.lattice().wrap(ny);
    int wn_z = rb.lattice().wrap(nz);

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        int idx = rb.lattice().index(x, y, z);
        if (idx == idx_pos || idx == idx_neg) continue;

        // Displacement from positive charge
        double dp_x = x - wp_x; double dp_y = y - wp_y; double dp_z = z - wp_z;
        double rp2 = dp_x*dp_x + dp_y*dp_y + dp_z*dp_z + 1.0; // softened
        double rp = std::sqrt(rp2);
        Vec3 rp_hat{dp_x/rp, dp_y/rp, dp_z/rp};

        // Displacement from negative charge
        double dn_x = x - wn_x; double dn_y = y - wn_y; double dn_z = z - wn_z;
        double rn2 = dn_x*dn_x + dn_y*dn_y + dn_z*dn_z + 1.0;
        double rn = std::sqrt(rn2);
        Vec3 rn_hat{dn_x/rn, dn_y/rn, dn_z/rn};

        // J += alpha * q / (4*pi*r^2) * r_hat
        double coeff = ALPHA / (4.0 * PI);
        Vec3 J_add = rp_hat * (coeff / rp2) + rn_hat * (-coeff / rn2);

        if (J_add.mag2() > 1e-12) {
            vox[idx].flux += J_add;
        }
    }

    // Collect all sites with nonzero flux
    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        int idx = rb.lattice().index(x, y, z);
        if (vox[idx].flux.mag2() > 1e-12 || vox[idx].state != 0) {
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult magnetic_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 moment,
                            int radius,
                            double amplitude) {
    Vec3 m_hat = safe_normalize(moment);
    Vec3 e1, e2;
    ortho_basis(m_hat, e1, e2);

    StampResult result{"magnetic_dipole", 2, center, {}};
    auto& vox = rb.voxels();
    const int N = rb.lattice().size();
    const double rad = static_cast<double>(radius);

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{x - static_cast<double>(center.x),
               y - static_cast<double>(center.y),
               z - static_cast<double>(center.z)};
        // Project onto the plane perpendicular to moment
        double r1 = r.dot(e1);
        double r2 = r.dot(e2);
        double along = r.dot(m_hat);
        double rplane = std::sqrt(r1*r1 + r2*r2);

        // Check if this site is close to the ideal circle
        double dist_from_circle = std::abs(rplane - rad);
        double dist_from_plane = std::abs(along);
        if (dist_from_circle <= 0.5 && dist_from_plane <= 0.5 && rplane > 0.01) {
            // Tangent direction (azimuthal): theta_hat = (-sin(theta), cos(theta))
            // in the (e1, e2) plane
            Vec3 theta_hat = e1 * (-r2 / rplane) + e2 * (r1 / rplane);
            Vec3 flux_val = theta_hat * amplitude;
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = flux_val;
            result.sites.push_back(idx);
        }
    }
    return result;
}

StampResult vortex_line(RenderBridge& rb,
                        Coord center,
                        Vec3 axis,
                        double circulation) {
    const int N = rb.lattice().size();
    Vec3 a_hat = safe_normalize(axis);
    Vec3 e1, e2;
    ortho_basis(a_hat, e1, e2);

    StampResult result{"vortex_line", 2, center, {}};
    auto& vox = rb.voxels();
    const double coeff = circulation / (2.0 * PI);

    for (int x = 0; x < N; ++x)
    for (int y = 0; y < N; ++y)
    for (int z = 0; z < N; ++z) {
        Vec3 r{x - static_cast<double>(center.x),
               y - static_cast<double>(center.y),
               z - static_cast<double>(center.z)};
        // Project onto the plane perpendicular to axis
        double r1 = r.dot(e1);
        double r2 = r.dot(e2);
        double rplane = std::sqrt(r1*r1 + r2*r2);
        double reff = std::max(rplane, 1.0); // core softening

        // Azimuthal direction
        if (rplane < 1e-10) continue; // on the axis itself, skip
        Vec3 theta_hat = e1 * (-r2 / rplane) + e2 * (r1 / rplane);
        Vec3 flux_val = theta_hat * (coeff / reff);

        if (flux_val.mag2() > 1e-12) {
            int idx = rb.lattice().index(x, y, z);
            vox[idx].flux = flux_val;
            result.sites.push_back(idx);
        }
    }
    return result;
}

// ============================================================================
// Level 3 — elementary particles
// ============================================================================

namespace {

/// Merge sites from a sub-result into a parent result (sort + dedup).
inline void merge_sites(std::vector<int>& dst, const std::vector<int>& src) {
    dst.insert(dst.end(), src.begin(), src.end());
    std::sort(dst.begin(), dst.end());
    dst.erase(std::unique(dst.begin(), dst.end()), dst.end());
}

}  // anonymous namespace

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

// ============================================================================
// Level 7 — gravity/cosmology
// ============================================================================

StampResult schwarzschild(RenderBridge& rb, Coord center, double r_s) {
    const Lattice& lat = rb.lattice();
    auto& vox = rb.voxels();
    const int N = lat.size();

    StampResult result{"schwarzschild", 7, center, {}};

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
// Level 8 — consciousness/observer
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
