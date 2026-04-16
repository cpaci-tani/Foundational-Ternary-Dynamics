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

}  // namespace ctor
}  // namespace ftd
