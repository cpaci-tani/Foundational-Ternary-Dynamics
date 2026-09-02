// ==========================================================================
//  engine/src/flux_cell.cpp
//
//  Flux-cell constructors, membrane / pump / port helpers, and the regional
//  storage ledger. See engine/include/ftd/flux_cell.h for the contract, the
//  energy channels, and the physics of the three mechanisms.
//
//  All sums here run over the canonical host voxel mirror (rb.voxels()), so
//  the ledger is backend-agnostic: a CUDA run syncs its state to the host on
//  the first read exactly as every other diagnostic does, and host writes
//  through voxel_at() are uploaded lazily before the next device tick.
// ==========================================================================

#include "ftd/flux_cell.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

#include <algorithm>
#include <cmath>
#include <unordered_set>
#include <utility>

namespace ftd {

namespace {

inline double periodic_delta(double a, double b, int N) {
    double d = a - b;
    while (d >  0.5 * N) d -= N;
    while (d < -0.5 * N) d += N;
    return d;
}

inline double sign_or_one(double v) { return v < 0.0 ? -1.0 : 1.0; }

// Evaluates the torus profile at one site; returns false when the site is
// outside the cutoff or on the axis.
inline bool torus_value(const FluxCellTorusSpec& spec, int N, int x, int y,
                        int z, double scale, Vec3& out) {
    const double sigma = std::max(1e-6, spec.tube_sigma);
    const double cut = spec.cutoff_sigmas * sigma;
    const double dx = periodic_delta(x, spec.cx, N);
    const double dy = periodic_delta(y, spec.cy, N);
    const double dz = periodic_delta(z, spec.cz, N);
    const double rho = std::sqrt(dx * dx + dy * dy);
    if (rho < 1e-9) return false;  // φ̂ undefined on the axis; f is negligible
    const double dr = rho - spec.major_radius;
    const double dT2 = dr * dr + dz * dz;
    if (dT2 > cut * cut) return false;
    double s = spec.circulation_sign < 0 ? -1.0 : 1.0;
    if (spec.sign_sectors > 0) {
        const double phi = std::atan2(dy, dx);
        s *= sign_or_one(std::cos(spec.sign_sectors * phi));
    }
    const double mag = scale * spec.amplitude * s
                     * std::exp(-dT2 / (2.0 * sigma * sigma));
    if (std::fabs(mag) < 1e-14) return false;
    out = Vec3(-mag * dy / rho, mag * dx / rho, 0.0);
    return true;
}

}  // namespace

FluxCellTorusSpec default_flux_cell_torus_spec(int lattice_size) {
    FluxCellTorusSpec spec;
    const double mid = (lattice_size - 1) * 0.5;
    spec.cx = mid;
    spec.cy = mid;
    spec.cz = mid;
    spec.major_radius = std::max(3.0, lattice_size / 4.0);
    spec.tube_sigma = std::max(1.25, lattice_size / 16.0);
    spec.amplitude = 0.3;
    spec.circulation_sign = +1;
    spec.sign_sectors = 0;
    spec.cutoff_sigmas = 4.0;
    return spec;
}

void seed_flux_cell_torus(RenderBridge& rb, const FluxCellTorusSpec& spec,
                          double scale) {
    const int N = rb.lattice().size();
    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        Vec3 j;
        if (!torus_value(spec, N, x, y, z, scale, j)) continue;
        Voxel& v = rb.voxel_at(x, y, z);
        v.flux += j;
        if (rb.toggles.dual_substrate) {
            const Vec3 half = j * 0.5;
            v.flux_L += half;
            v.flux_R += half;
        }
    }
}

// ── Membrane ───────────────────────────────────────────────────────────

FluxCellMembraneSpec default_flux_cell_membrane_spec(int lattice_size,
                                                     double thickness) {
    FluxCellMembraneSpec spec;
    const double mid = (lattice_size - 1) * 0.5;
    spec.cx = mid;
    spec.cy = mid;
    spec.cz = mid;
    spec.thickness = std::max(1.0, thickness);
    // Outer radius keeps the whole shell strictly inside the box so no
    // boundary law ever touches it: the farthest site from the centre along
    // an axis sits at distance mid.
    const double outer = std::max(2.0, mid - 0.5);
    spec.inner_radius = std::max(1.0, outer - spec.thickness);
    return spec;
}

FluxCellTorusSpec flux_cell_membrane_ring_spec(const FluxCellMembraneSpec& shell) {
    FluxCellTorusSpec ring;
    ring.cx = shell.cx;
    ring.cy = shell.cy;
    ring.cz = shell.cz;
    ring.major_radius = 0.45 * shell.inner_radius;
    ring.tube_sigma = std::max(1.0, 0.11 * shell.inner_radius);
    ring.amplitude = 0.3;
    ring.circulation_sign = +1;
    ring.sign_sectors = 0;
    // Keep the Gaussian support one full cell inside the shell so the mass
    // gap never overlaps the seeded field.
    const double room = shell.inner_radius - 1.0 - ring.major_radius;
    ring.cutoff_sigmas = std::max(0.5, std::min(4.0, room / ring.tube_sigma));
    return ring;
}

int seed_flux_cell_membrane(RenderBridge& rb, const FluxCellMembraneSpec& spec) {
    const int N = rb.lattice().size();
    const double r_in = spec.inner_radius;
    const double r_out = spec.inner_radius + spec.thickness;
    int written = 0;
    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        const double dx = periodic_delta(x, spec.cx, N);
        const double dy = periodic_delta(y, spec.cy, N);
        const double dz = periodic_delta(z, spec.cz, N);
        const double r = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (r < r_in || r >= r_out) continue;
        Voxel& v = rb.voxel_at(x, y, z);
        v.state = static_cast<int8_t>(((x + y + z) & 1) ? 1 : -1);
        v.flux = Vec3(0, 0, 0);
        v.spin = 0;
        v.color = 0;
        v.particle_id = rb.injector().next_particle_id();
        v.pair_id = -1;
        v.locked = true;
        if (rb.toggles.dual_substrate) {
            v.flux_L = Vec3(0, 0, 0);
            v.flux_R = Vec3(0, 0, 0);
        }
        ++written;
    }
    return written;
}

// ── Pump ───────────────────────────────────────────────────────────────

FluxCellPumpProfile build_flux_cell_pump_profile(const RenderBridge& rb,
                                                 const FluxCellTorusSpec& spec,
                                                 int ticks) {
    FluxCellPumpProfile p;
    const auto& lattice = rb.lattice();
    const int N = lattice.size();
    p.ticks = std::max(1, ticks);
    p.delta.assign(static_cast<std::size_t>(lattice.total_sites()), Vec3());
    const double scale = 1.0 / p.ticks;
    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        Vec3 j;
        if (!torus_value(spec, N, x, y, z, scale, j)) continue;
        const int idx = lattice.index(x, y, z);
        p.delta[static_cast<std::size_t>(idx)] = j;
        p.support.push_back(idx);
    }
    std::unordered_set<int> dilated(p.support.begin(), p.support.end());
    for (const int idx : p.support) {
        for (const int n : lattice.neighbors_6(idx)) dilated.insert(n);
        for (const int n : lattice.neighbors_12(idx)) dilated.insert(n);
    }
    p.dilated.assign(dilated.begin(), dilated.end());
    std::sort(p.dilated.begin(), p.dilated.end());
    return p;
}

double apply_flux_cell_pump_increment(RenderBridge& rb,
                                      const FluxCellPumpProfile& profile) {
    const auto& lattice = rb.lattice();
    // Read the pre-increment state (const access: device→host sync only).
    const std::vector<Voxel>& before = std::as_const(rb).voxels();
    constexpr double C2 = C_SPEED * C_SPEED;
    double w_dot = 0.0, j_dot = 0.0, d_dot = 0.0;
    for (const int i : profile.dilated) {
        Vec3 lap;
        for (const int n : lattice.neighbors_6(i))
            lap += profile.delta[static_cast<std::size_t>(n)] * (1.0 / 3.0);
        for (const int n : lattice.neighbors_12(i))
            lap += profile.delta[static_cast<std::size_t>(n)] * (1.0 / 6.0);
        lap -= profile.delta[static_cast<std::size_t>(i)] * 4.0;
        const Voxel& v = before[static_cast<std::size_t>(i)];
        w_dot += v.wave_vel.dot(lap);
        j_dot += v.flux.dot(lap);
        d_dot += profile.delta[static_cast<std::size_t>(i)].dot(lap);
    }
    const double dH = 0.5 * C2 * (w_dot - 2.0 * j_dot - d_dot);

    // Apply the increment on the host mirror (marks it dirty on GPU).
    std::vector<Voxel>& vox = rb.voxels();
    const bool dual = rb.toggles.dual_substrate;
    for (const int i : profile.support) {
        const Vec3& d = profile.delta[static_cast<std::size_t>(i)];
        Voxel& v = vox[static_cast<std::size_t>(i)];
        v.flux += d;
        if (dual) {
            v.flux_L += d * 0.5;
            v.flux_R += d * 0.5;
        }
    }
    return dH;
}

// ── Ledger ─────────────────────────────────────────────────────────────

FluxCellLedger compute_flux_cell_ledger(const RenderBridge& rb,
                                        const FluxCellRegion& region,
                                        double support_threshold) {
    FluxCellLedger out;
    const auto& lattice = rb.lattice();
    const auto& voxels = rb.voxels();
    const int N = lattice.size();
    const double r2 = region.radius * region.radius;
    constexpr double C2 = C_SPEED * C_SPEED;
    const double w2 = rb.toggles.de_broglie_clock
        ? rb.toggles.omega0 * rb.toggles.omega0 : 0.0;

    auto inside = [&](int x, int y, int z) {
        const double dx = periodic_delta(x, region.cx, N);
        const double dy = periodic_delta(y, region.cy, N);
        const double dz = periodic_delta(z, region.cz, N);
        return dx * dx + dy * dy + dz * dz <= r2;
    };
    const int face[6][3] = {
        {1,0,0},{-1,0,0},{0,1,0},{0,-1,0},{0,0,1},{0,0,-1}
    };

    for (int z = 0; z < N; ++z)
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        const int idx = lattice.index(x, y, z);
        const Voxel& v = voxels[static_cast<std::size_t>(idx)];
        const double e2 = v.flux.mag2() + v.wave_vel.mag2();
        if (e2 > support_threshold) {
            const double dx = periodic_delta(x, region.cx, N);
            const double dy = periodic_delta(y, region.cy, N);
            const double dz = periodic_delta(z, region.cz, N);
            out.support_radius = std::max(
                out.support_radius, std::sqrt(dx * dx + dy * dy + dz * dz));
        }
        if (!inside(x, y, z)) continue;

        ++out.site_count;
        const Vec3 E = v.wave_vel * -1.0;
        const Vec3 B = rb.curl_flux(idx);
        const Vec3 lap = rb.laplacian_flux(idx);
        const Vec3 S = rb.poynting_vector(idx);

        out.U_E += 0.5 * E.mag2();
        out.U_B += 0.5 * C2 * B.mag2();
        out.U_J += 0.5 * v.flux.mag2();
        const double h_wave = 0.5 * v.wave_vel.mag2()
                            + 0.5 * C2 * v.wave_vel.dot(lap)
                            - 0.5 * C2 * v.flux.dot(lap);
        out.H_wave += h_wave;
        out.H_kg += h_wave;
        if (w2 > 0.0 && v.state != 0) {
            // Clock term: acceleration −ω₀²J enters the kick-drift cross term
            // and ½ω₀²|J|² is its potential.
            out.H_kg += -0.5 * w2 * v.wave_vel.dot(v.flux)
                      + 0.5 * w2 * v.flux.mag2();
        }
        out.S_total += S;
        out.S_abs_total += S.mag();
        out.J_total += v.flux;
        const double jc[3] = {v.flux.x, v.flux.y, v.flux.z};
        for (int a = 0; a < 3; ++a)
            for (int b = 0; b < 3; ++b)
                out.dyad[a][b] += jc[a] * jc[b];

        for (const auto& o : face) {
            if (inside(x + o[0], y + o[1], z + o[2])) continue;
            out.P_leak += S.x * o[0] + S.y * o[1] + S.z * o[2];
        }
    }
    return out;
}

double flux_cell_eb_balance(const FluxCellLedger& ledger) {
    const double denom = ledger.U_E + ledger.U_B;
    if (denom <= 0.0) return 0.0;
    return (ledger.U_E - ledger.U_B) / denom;
}

double flux_cell_ring_circulation(const RenderBridge& rb, double cx, double cy,
                                  double cz, double R, int n_samples) {
    const auto& lattice = rb.lattice();
    const auto& voxels = rb.voxels();
    if (n_samples <= 0) n_samples = std::max(8, static_cast<int>(std::ceil(4.0 * PI * R)));
    const double dl = 2.0 * PI * R / n_samples;
    const int zc = static_cast<int>(std::lround(cz));
    double gamma = 0.0;
    for (int k = 0; k < n_samples; ++k) {
        const double phi = 2.0 * PI * k / n_samples;
        const double px = cx + R * std::cos(phi);
        const double py = cy + R * std::sin(phi);
        const int ix = static_cast<int>(std::lround(px));
        const int iy = static_cast<int>(std::lround(py));
        const Vec3& j = voxels[static_cast<std::size_t>(lattice.index(ix, iy, zc))].flux;
        gamma += (-j.x * std::sin(phi) + j.y * std::cos(phi)) * dl;
    }
    return gamma;
}

double flux_cell_disk_magnetic_flux(const RenderBridge& rb, double cx,
                                    double cy, double cz, double R) {
    const auto& lattice = rb.lattice();
    const int N = lattice.size();
    const int zc = static_cast<int>(std::lround(cz));
    const double r2 = R * R;
    double phi_b = 0.0;
    for (int y = 0; y < N; ++y)
    for (int x = 0; x < N; ++x) {
        const double dx = periodic_delta(x, cx, N);
        const double dy = periodic_delta(y, cy, N);
        if (dx * dx + dy * dy >= r2) continue;
        phi_b += rb.curl_flux(lattice.index(x, y, zc)).z;
    }
    return phi_b;
}

double flux_cell_site_poynting_flux(const RenderBridge& rb,
                                    const std::vector<int>& sites,
                                    const Vec3& normal) {
    double flux = 0.0;
    for (const int i : sites) flux += rb.poynting_vector(i).dot(normal);
    return flux;
}

double flux_cell_site_hamiltonian_flux(const RenderBridge& rb,
                                       const std::vector<int>& sites,
                                       const Vec3& normal) {
    const auto& lattice = rb.lattice();
    const auto& voxels = rb.voxels();
    constexpr double C2 = C_SPEED * C_SPEED;
    double flux = 0.0;
    for (const int i : sites) {
        const auto& n6 = lattice.neighbors_6(i);
        // Centred differences along x (n6[0]/n6[1]), y (n6[2]/n6[3]), z (n6[4]/n6[5]).
        const Vec3 dJx = (voxels[static_cast<std::size_t>(n6[0])].flux
                        - voxels[static_cast<std::size_t>(n6[1])].flux) * 0.5;
        const Vec3 dJy = (voxels[static_cast<std::size_t>(n6[2])].flux
                        - voxels[static_cast<std::size_t>(n6[3])].flux) * 0.5;
        const Vec3 dJz = (voxels[static_cast<std::size_t>(n6[4])].flux
                        - voxels[static_cast<std::size_t>(n6[5])].flux) * 0.5;
        // (n.grad)J_a for each component a, then contract with E_a = -W_a.
        const Vec3 dJn = dJx * normal.x + dJy * normal.y + dJz * normal.z;
        const Vec3 E = voxels[static_cast<std::size_t>(i)].wave_vel * -1.0;
        flux += C2 * E.dot(dJn);
    }
    return flux;
}

}  // namespace ftd
