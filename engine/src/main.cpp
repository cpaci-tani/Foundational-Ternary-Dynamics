/**
 * FTD Render-Bridge Simulation Engine
 *
 * Scenarios:
 *   A: Particle interaction (electron-proton attraction via flux)
 *   B: Pair production from high-energy flux pulse
 *   D: Locked particle stability test (default)
 *   E: Helium atom (2 locked protons + 2 orbiting electrons)
 *   F: Gravitational cluster (20 particles, pairwise gravity)
 *   G: Scale stress test (benchmark at specified lattice size)
 *   H: Helium atom with CSV export (density slices + timeseries)
 *   I: Interference pattern with CSV export (4-source, density heatmaps)
 *   J: Pair production with CSV export (counter-propagating beams)
 *
 * Usage: ftd_sim [scenario] [lattice_size] [num_ticks]
 */

#define _USE_MATH_DEFINES
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <chrono>
#include <random>
#include "ftd/constants.h"
#include "ftd/render_bridge.h"
#include "ftd/csv_export.h"

void print_header() {
    std::cout << "================================================================\n";
    std::cout << "  FTD RENDER-BRIDGE ENGINE\n";
    std::cout << "  G* = " << std::setprecision(10) << ftd::G_STAR
              << "  alpha^-1 = " << ftd::X_PLUS << "\n";
    std::cout << "================================================================\n\n";
}

// Helper: find a manifested particle near a position
struct ParticleInfo {
    int idx = -1;
    int x = 0, y = 0, z = 0;
    int8_t state = 0;
    double density = 0.0;
    ftd::Vec3 velocity;
};

ParticleInfo find_particle(const ftd::RenderBridge& engine, int near_x, int near_y, int near_z, int8_t sign, int search_radius = 5) {
    ParticleInfo best;
    double best_dist = 1e30;
    const auto& vox = engine.voxels();

    for (int dx = -search_radius; dx <= search_radius; ++dx)
    for (int dy = -search_radius; dy <= search_radius; ++dy)
    for (int dz = -search_radius; dz <= search_radius; ++dz) {
        int idx = engine.lattice().index(near_x + dx, near_y + dy, near_z + dz);
        if (vox[idx].state == sign) {
            double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (dist < best_dist) {
                best_dist = dist;
                best.idx = idx;
                auto c = engine.lattice().coord(idx);
                best.x = c.x; best.y = c.y; best.z = c.z;
                best.state = vox[idx].state;
                best.density = vox[idx].density();
                best.velocity = vox[idx].velocity;
            }
        }
    }
    return best;
}

// ============================================================================
// SCENARIO A: Electron-Proton Interaction
// ============================================================================
// Place a free electron (s=-1) and a free proton (s=+1) separated by a gap.
// The flux field mediates the interaction. If the force direction is correct,
// opposite-sign particles should attract each other.

void scenario_A(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO A: Electron-Proton Interaction\n";
    std::cout << "  Testing: Does the flux field mediate attraction between\n";
    std::cout << "  opposite-sign particles? (No explicit force law coded.)\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;
    int sep = 6;  // Initial separation

    // Place electron (s=-1) and proton (s=+1) separated by 'sep' voxels
    // Start LOCKED so the coupling term can build a self-sustaining flux halo
    // IMPORTANT: Use isotropic flux (equal on all axes) so the self-field
    // halo is spherically symmetric. Anisotropic initial flux creates a
    // directional self-field that produces spurious self-forces.
    int e_x = mid - sep/2, p_x = mid + sep/2;
    double iso = ftd::K_B / std::sqrt(3.0);  // K_B/sqrt(3) on each axis = K_B magnitude
    engine.inject_particle(e_x, mid, mid, -1, {iso, iso, iso});
    engine.voxel_at(e_x, mid, mid).locked = true;
    engine.inject_particle(p_x, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(p_x, mid, mid).locked = true;

    std::cout << "  Electron at (" << e_x << "," << mid << "," << mid << ") state=-1 [locked]\n";
    std::cout << "  Proton  at (" << p_x << "," << mid << "," << mid << ") state=+1 [locked]\n";
    std::cout << "  Separation = " << sep << " voxels\n\n";

    // Phase 1: Let the flux field equilibrate while particles are locked
    int equil_ticks = 500;
    std::cout << "  Phase 1: Flux equilibration (" << equil_ticks << " ticks, locked)...\n";
    engine.run(equil_ticks);

    // Report equilibrium state
    auto& ev = engine.voxel_at(e_x, mid, mid);
    auto& pv = engine.voxel_at(p_x, mid, mid);
    std::cout << "  After equilibration:\n";
    std::cout << "    Electron density = " << std::setprecision(4) << ev.density()
              << " (K_B = " << ftd::K_B << ")\n";
    std::cout << "    Proton  density = " << pv.density() << "\n";

    // Phase 2: Unlock and let them interact
    std::cout << "\n  Phase 2: UNLOCKING particles for interaction...\n";
    ev.locked = false;
    pv.locked = false;

    // Report force direction after first few ticks unlocked
    // Run 10 ticks to let force accumulate visibly
    engine.run(10);
    auto e = find_particle(engine, e_x, mid, mid, -1);
    auto p = find_particle(engine, p_x, mid, mid, +1);

    if (e.idx >= 0 && p.idx >= 0) {
        // Expected Coulomb force: F = alpha / r^2 = 0.00729 / 36 ~ 0.0002 per tick
        // After 10 ticks: v ~ 0.002
        double expected_F = ftd::ALPHA / (sep * sep);
        std::cout << "    Expected Coulomb force = alpha/r^2 = " << std::setprecision(6)
                  << expected_F << " per tick\n";
        std::cout << "    After 10 ticks:\n";
        std::cout << "    Electron at (" << e.x << "," << e.y << "," << e.z
                  << ") v=(" << std::setprecision(6)
                  << e.velocity.x << "," << e.velocity.y << "," << e.velocity.z << ")\n";
        std::cout << "    Proton  at (" << p.x << "," << p.y << "," << p.z
                  << ") v=(" << p.velocity.x << "," << p.velocity.y << "," << p.velocity.z << ")\n";

        bool e_toward_p = (e.velocity.x > 0);  // electron should move toward proton (+x)
        bool p_toward_e = (p.velocity.x < 0);  // proton should move toward electron (-x)
        std::cout << "    Electron moving toward proton? " << (e_toward_p ? "YES" : "no") << "\n";
        std::cout << "    Proton moving toward electron? " << (p_toward_e ? "YES" : "no") << "\n";
        std::cout << "    |v_x| >> |v_y|,|v_z|? (clean 1D signal) "
                  << (std::abs(e.velocity.x) > 3*std::abs(e.velocity.y) ? "YES" : "no") << "\n\n";
    } else {
        std::cout << "    WARNING: particle(s) missing after unlock!\n";
        if (e.idx < 0) std::cout << "      Electron evaporated (density < K_B)\n";
        if (p.idx < 0) std::cout << "      Proton evaporated (density < K_B)\n";
    }

    // Track interaction dynamics
    int remaining = num_ticks - equil_ticks - 10;
    std::cout << "  Phase 3: Tracking interaction (" << remaining << " ticks)\n";
    std::cout << "  tick, e_x, e_y, e_z, p_x, p_y, p_z, separation, manifested\n";

    int report_interval = std::max(1, remaining / 20);
    for (int t = 0; t < remaining; ++t) {
        engine.tick();

        if ((t + 1) % report_interval == 0 || t == remaining - 1) {
            auto e2 = find_particle(engine, mid, mid, mid, -1, lattice_size/2);
            auto p2 = find_particle(engine, mid, mid, mid, +1, lattice_size/2);
            auto d = engine.diagnostics();

            double sep_now = -1;
            if (e2.idx >= 0 && p2.idx >= 0) {
                double dx = e2.x - p2.x, dy = e2.y - p2.y, dz = e2.z - p2.z;
                sep_now = std::sqrt(dx*dx + dy*dy + dz*dz);
            }

            std::cout << "  " << std::setw(5) << (t+1) << ", ";
            if (e2.idx >= 0) std::cout << e2.x << "," << e2.y << "," << e2.z;
            else std::cout << "gone";
            std::cout << ", ";
            if (p2.idx >= 0) std::cout << p2.x << "," << p2.y << "," << p2.z;
            else std::cout << "gone";
            std::cout << ", " << std::setprecision(3);
            if (sep_now >= 0) std::cout << sep_now;
            else std::cout << "N/A";
            std::cout << ", " << d.manifested_count << "\n";
        }
    }

    // Final verdict
    std::cout << "\n";
    auto ef = find_particle(engine, mid, mid, mid, -1, lattice_size/2);
    auto pf = find_particle(engine, mid, mid, mid, +1, lattice_size/2);
    auto df = engine.diagnostics();

    if (ef.idx < 0 && pf.idx < 0) {
        std::cout << "  RESULT: Both particles annihilated!\n";
        std::cout << "  (They attracted, collided, and destroyed each other.)\n";
    } else if (ef.idx >= 0 && pf.idx >= 0) {
        double dx = ef.x - pf.x, dy = ef.y - pf.y, dz = ef.z - pf.z;
        double final_sep = std::sqrt(dx*dx + dy*dy + dz*dz);
        std::cout << "  RESULT: Final separation = " << std::setprecision(3) << final_sep
                  << " (initial = " << sep << ")\n";
        if (final_sep < sep) {
            std::cout << "  ATTRACTION CONFIRMED: Opposite-sign particles moved closer.\n";
        } else {
            std::cout << "  No clear attraction observed (may need more ticks or stronger coupling).\n";
        }
    } else {
        std::cout << "  RESULT: One particle evaporated (density dropped below K_B).\n";
    }
    std::cout << "  Total manifested: " << df.manifested_count
              << " (+" << df.positive_count << " / -" << df.negative_count << ")\n";
}

// ============================================================================
// SCENARIO B: Pair Production from Flux Pulse
// ============================================================================
// Inject a massive flux pulse (density >> K_B) and watch for spontaneous
// particle-antiparticle pair creation.

void scenario_B(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO B: Pair Production from Flux Pulse\n";
    std::cout << "  Testing: Can a pure flux injection (no particles) spontaneously\n";
    std::cout << "  create particle-antiparticle pairs via the manifestation threshold?\n";
    std::cout << "  K_B = " << ftd::K_B << " MeV,  K_GENESIS = " << ftd::K_GENESIS << " MeV\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Inject TWO counter-directed Gaussian flux beams that collide at center.
    // This creates a collision zone with high density AND alternating
    // divergence sign — the left beam has div > 0 on the right side of
    // its wavefront (convergent), and the right beam has div < 0 on the
    // left side. At the collision point, we get both positive and negative
    // divergence zones at high density -> charge-neutral pair production.
    double peak = 5.0;  // ~3.2x K_GENESIS = strong enough for genesis
    int beam_offset = 4;  // beams start 4 voxels from center
    int radius = 3;
    int injected = 0;

    std::cout << "  Injecting counter-propagating flux beams:\n";
    std::cout << "    Peak amplitude = " << peak << " (K_GENESIS = " << ftd::K_GENESIS << ")\n";
    std::cout << "    Beam offset = " << beam_offset << " voxels from center\n";
    std::cout << "    Beam radius = " << radius << " voxels\n";

    // Left beam: flux pointing +x (toward center)
    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        double r2 = dy*dy + dz*dz;  // transverse profile
        if (r2 > radius*radius) continue;
        double amp = peak * std::exp(-(dx*dx + r2) / 2.0);
        engine.inject_flux(mid - beam_offset + dx, mid + dy, mid + dz, {amp, 0, 0});
        ++injected;
    }

    // Right beam: flux pointing -x (toward center)
    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        double r2 = dy*dy + dz*dz;
        if (r2 > radius*radius) continue;
        double amp = peak * std::exp(-(dx*dx + r2) / 2.0);
        engine.inject_flux(mid + beam_offset + dx, mid + dy, mid + dz, {-amp, 0, 0});
        ++injected;
    }
    std::cout << "    Sites injected = " << injected << "\n\n";

    // Run and track particle creation
    std::cout << "  tick, total_flux, manifested, positive, negative, max_bw\n";

    auto d = engine.diagnostics();
    std::cout << "  " << std::setw(5) << d.tick << ", "
              << std::setprecision(4) << d.total_flux << ", "
              << d.manifested_count << ", " << d.positive_count << ", "
              << d.negative_count << ", " << d.max_bandwidth << "\n";

    int peak_manifested = 0;
    int peak_tick = 0;

    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();

        d = engine.diagnostics();
        if (d.manifested_count > peak_manifested) {
            peak_manifested = d.manifested_count;
            peak_tick = d.tick;
        }

        // Report every tick for first 20, then every 10, then every 50
        bool report = (t < 20) || ((t + 1) % 10 == 0 && t < 200) ||
                      ((t + 1) % 50 == 0);
        if (report || t == num_ticks - 1) {
            std::cout << "  " << std::setw(5) << d.tick << ", "
                      << std::setprecision(4) << d.total_flux << ", "
                      << d.manifested_count << ", " << d.positive_count << ", "
                      << d.negative_count << ", "
                      << std::setprecision(6) << d.max_bandwidth << "\n";
        }
    }

    std::cout << "\n";
    std::cout << "  RESULTS:\n";
    std::cout << "    Peak manifested = " << peak_manifested << " at tick " << peak_tick << "\n";
    std::cout << "    Final manifested = " << d.manifested_count
              << " (+" << d.positive_count << " / -" << d.negative_count << ")\n";
    std::cout << "    Final total flux = " << std::setprecision(6) << d.total_flux << "\n";

    if (peak_manifested > 0) {
        std::cout << "\n  PAIR PRODUCTION CONFIRMED: Pure flux created particles!\n";
        if (d.positive_count == d.negative_count) {
            std::cout << "  Charge conservation: equal +/- particles created.\n";
        }
    } else {
        std::cout << "\n  No pair production observed. May need higher energy or different geometry.\n";
    }

    // Report surviving particles
    if (d.manifested_count > 0) {
        std::cout << "\n  SURVIVING PARTICLES:\n";
        const auto& vox = engine.voxels();
        int count = 0;
        for (int i = 0; i < (int)vox.size() && count < 20; ++i) {
            if (vox[i].state != 0) {
                auto c = engine.lattice().coord(i);
                std::cout << "    (" << c.x << "," << c.y << "," << c.z
                          << ") state=" << (int)vox[i].state
                          << " density=" << std::setprecision(4) << vox[i].density()
                          << " v=(" << vox[i].velocity.x << "," << vox[i].velocity.y
                          << "," << vox[i].velocity.z << ")\n";
                ++count;
            }
        }
        if (count >= 20) std::cout << "    ... (more particles not shown)\n";
    }
}

// ============================================================================
// DEFAULT: Locked particle stability test (original scenario)
// ============================================================================

void scenario_default(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO D: Locked Particle Stability\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    engine.inject_particle(mid, mid, mid, -1, {0.0, 0.0, 1.0});
    engine.voxel_at(mid, mid, mid).locked = true;

    engine.inject_particle(mid+3, mid, mid, 1, {1.0, 0.0, 0.0});
    engine.voxel_at(mid+3, mid, mid).locked = true;
    engine.inject_particle(mid+3, mid+1, mid, 1, {0.0, 1.0, 0.0});
    engine.voxel_at(mid+3, mid+1, mid).locked = true;
    engine.inject_particle(mid+3, mid, mid+1, 1, {0.0, 0.0, 1.0});
    engine.voxel_at(mid+3, mid, mid+1).locked = true;

    std::cout << "tick,total_flux,manifested,positive,negative\n";
    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();
        if ((t + 1) % (num_ticks/10) == 0) {
            auto d = engine.diagnostics();
            std::cout << d.tick << "," << std::setprecision(6) << d.total_flux
                      << "," << d.manifested_count << "," << d.positive_count
                      << "," << d.negative_count << "\n";
        }
    }

    std::cout << "\n  PARTICLE REPORT:\n";
    const auto& vox = engine.voxels();
    for (int i = 0; i < (int)vox.size(); ++i) {
        if (vox[i].locked) {
            auto c = engine.lattice().coord(i);
            const auto& fd = engine.force_diag_at(i);
            double f_total = (fd.f_coulomb + fd.f_gravity).mag();
            std::cout << "    (" << c.x << "," << c.y << "," << c.z
                      << ") s=" << (int)vox[i].state
                      << " density=" << std::setprecision(6) << vox[i].density()
                      << " |v|=" << std::setprecision(4) << vox[i].speed()
                      << " |F|=" << std::setprecision(4) << f_total << "\n";
        }
    }
}

// ============================================================================
// SCENARIO E: Helium Atom (multi-electron)
// ============================================================================
// Locked 2-proton nucleus + 2 orbiting electrons. Demonstrates shell filling
// and electron correlation.

void scenario_E(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO E: Helium Atom\n";
    std::cout << "  2-proton nucleus (locked) + 2 electrons at r~5\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Locked proton pair at center
    double iso = ftd::K_B / std::sqrt(3.0);
    engine.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid, mid, mid).locked = true;
    engine.inject_particle(mid+1, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid+1, mid, mid).locked = true;

    // Let field develop
    engine.toggles.enable_all();
    engine.toggles.genesis = false;
    engine.toggles.gauss_projection = false;
    engine.run(30);

    // Inject 2 electrons at radius ~5 with tangential velocity
    engine.inject_particle(mid+5, mid, mid, -1, {0, 0, -ftd::K_B}, -1, 0);
    engine.voxels()[engine.lattice().index(mid+5, mid, mid)].velocity = {0, 0.06, 0};
    engine.inject_particle(mid-5, mid, mid, -1, {0, 0, -ftd::K_B}, 1, 0);
    engine.voxels()[engine.lattice().index(mid-5, mid, mid)].velocity = {0, -0.06, 0};

    // Enable full physics
    engine.toggles.enable_all();
    engine.toggles.genesis = false;

    // CSV header
    std::cout << "tick,e1_r,e2_r,total_energy,manifested\n";

    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();

        if ((t + 1) % (std::max(1, num_ticks / 100)) == 0) {
            // Find electrons
            double e1_r = -1, e2_r = -1;
            int e_count = 0;
            for (int i = 0; i < engine.lattice().total_sites(); ++i) {
                const auto& v = engine.voxels()[i];
                if (v.state < 0 && !v.locked) {
                    auto c = engine.lattice().coord(i);
                    double dx = c.x - mid, dy = c.y - mid, dz = c.z - mid;
                    double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (e_count == 0) e1_r = r;
                    else e2_r = r;
                    e_count++;
                }
            }
            auto d = engine.diagnostics();
            std::cout << d.tick << "," << std::setprecision(2)
                      << e1_r << "," << e2_r << ","
                      << std::setprecision(4) << d.total_flux << ","
                      << d.manifested_count << "\n";
        }
    }
}

// ============================================================================
// SCENARIO F: Gravitational Cluster
// ============================================================================
// 20 same-sign particles in random positions, field-mediated gravity.
// Tests density-gradient-mediated clustering from F_grav = G_N * grad(rho).

void scenario_F(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO F: Gravitational Cluster (field-mediated)\n";
    std::cout << "  20 same-sign particles, F_grav = G_N * grad(rho)\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Full physics — field-mediated forces include gravity via grad(rho)
    engine.toggles.enable_all();
    engine.toggles.genesis = false;  // No spontaneous pair creation

    // Random placement in central region
    std::mt19937 rng(42);
    int spread = lattice_size / 4;
    std::uniform_int_distribution<int> pos(mid - spread, mid + spread);
    double iso = ftd::K_B / std::sqrt(3.0);
    int placed = 0;

    for (int i = 0; i < 30 && placed < 20; ++i) {
        int px = pos(rng), py = pos(rng), pz = pos(rng);
        if (engine.voxels()[engine.lattice().index(px, py, pz)].state != 0) continue;
        engine.inject_particle(px, py, pz, +1, {iso, iso, iso});
        placed++;
    }
    std::cout << "  Placed " << placed << " particles\n\n";

    // CSV header
    std::cout << "tick,rms_radius,max_sep,kinetic_energy,particle_count\n";

    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();

        if ((t + 1) % (std::max(1, num_ticks / 200)) == 0) {
            // Compute RMS radius and max separation
            std::vector<std::array<int, 3>> positions;
            double KE = 0.0;
            for (int i = 0; i < engine.lattice().total_sites(); ++i) {
                const auto& v = engine.voxels()[i];
                if (v.state != 0) {
                    auto c = engine.lattice().coord(i);
                    positions.push_back({c.x, c.y, c.z});
                    KE += 0.5 * v.speed() * v.speed();
                }
            }

            double rms = 0.0;
            for (auto& p : positions) {
                double dx = p[0] - mid, dy = p[1] - mid, dz = p[2] - mid;
                rms += dx*dx + dy*dy + dz*dz;
            }
            rms = positions.empty() ? 0.0 : std::sqrt(rms / positions.size());

            double max_sep = 0.0;
            for (size_t i = 0; i < positions.size(); ++i)
                for (size_t j = i+1; j < positions.size(); ++j) {
                    double dx = positions[i][0] - positions[j][0];
                    double dy = positions[i][1] - positions[j][1];
                    double dz = positions[i][2] - positions[j][2];
                    double d = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (d > max_sep) max_sep = d;
                }

            std::cout << t+1 << "," << std::setprecision(2)
                      << rms << "," << max_sep << ","
                      << std::setprecision(6) << KE << ","
                      << positions.size() << "\n";
        }
    }
}

// ============================================================================
// SCENARIO G: Scale Stress Test
// ============================================================================
// Quick benchmark: empty lattice + 100-particle lattice at specified size.

void scenario_G(int lattice_size, int num_ticks) {
    std::cout << "SCENARIO G: Scale Stress Test\n";
    std::cout << "  Lattice: " << lattice_size << "^3\n\n";

    using Clock = std::chrono::high_resolution_clock;
    using Ms = std::chrono::duration<double, std::milli>;

    // Phase 1: Empty lattice
    {
        ftd::RenderBridge engine(lattice_size);
        engine.tick(); // warm up

        int ticks = std::min(num_ticks, 100);
        auto t0 = Clock::now();
        for (int i = 0; i < ticks; ++i) engine.tick();
        auto t1 = Clock::now();
        double ms = Ms(t1 - t0).count() / ticks;
        std::cout << std::fixed;
        std::cout << "  Empty lattice:   " << std::setprecision(2) << ms
                  << " ms/tick (" << std::setprecision(0) << 1000.0/ms << " Hz)\n";
    }

    // Phase 2: With 100 particles
    {
        ftd::RenderBridge engine(lattice_size);
        std::mt19937 rng(42);
        int L = engine.lattice().size();
        std::uniform_int_distribution<int> pos(1, L-2);
        std::uniform_int_distribution<int> sign(0, 1);
        double iso = ftd::K_B / std::sqrt(3.0);

        for (int i = 0; i < 100; ++i) {
            int8_t s = sign(rng) ? +1 : -1;
            engine.inject_particle(pos(rng), pos(rng), pos(rng), s, {iso, iso, iso});
        }

        engine.tick(); // warm up

        int ticks = std::min(num_ticks, 100);
        auto t0 = Clock::now();
        for (int i = 0; i < ticks; ++i) engine.tick();
        auto t1 = Clock::now();
        double ms = Ms(t1 - t0).count() / ticks;
        std::cout << "  100 particles:   " << std::setprecision(2) << ms
                  << " ms/tick (" << std::setprecision(0) << 1000.0/ms << " Hz)\n";
        std::cout << std::defaultfloat;
    }

    long long N = (long long)lattice_size * lattice_size * lattice_size;
    double mb = N * sizeof(ftd::Voxel) / (1024.0 * 1024.0);
    std::cout << std::fixed << std::setprecision(1);
    std::cout << "  Memory:          " << mb << " MB ("
              << N << " voxels, " << sizeof(ftd::Voxel) << " bytes each)\n";
    std::cout << std::defaultfloat;
}

// ============================================================================
// SCENARIO I: Interference Pattern (with CSV density-slice export)
// ============================================================================
// 4 coherent flux sources produce interference. Exports density slices for
// heatmap visualization.

void scenario_I(int lattice_size, int num_ticks, const std::string& outdir) {
    std::cout << "SCENARIO I: Interference Pattern Visualization\n";
    std::cout << "  4 sources, " << lattice_size << "^3 lattice, " << num_ticks << " ticks\n";
    std::cout << "  Output: " << outdir << "/\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Pure wave dynamics
    engine.toggles.disable_all();
    engine.toggles.wave_propagation = true;
    engine.toggles.damping = true;

    // 4 sources at corners of a square in the mid-plane
    int half = lattice_size / 5;
    double amp = 3.0 * ftd::K_B;

    engine.inject_flux(mid - half, mid - half, mid, {0, 0, amp});
    engine.inject_flux(mid + half, mid - half, mid, {0, 0, amp});
    engine.inject_flux(mid - half, mid + half, mid, {0, 0, amp});
    engine.inject_flux(mid + half, mid + half, mid, {0, 0, amp});

    // Also inject a ring of 8 secondary sources
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4.0;
        int sx = mid + static_cast<int>(half * 1.2 * std::cos(angle));
        int sy = mid + static_cast<int>(half * 1.2 * std::sin(angle));
        engine.inject_flux(sx, sy, mid, {0, 0, amp * 0.4});
    }

    // Export snapshots
    int snapshot_interval = std::max(1, num_ticks / 10);
    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();

        if ((t + 1) % snapshot_interval == 0 || t == 0 || t == num_ticks - 1) {
            std::string fname = outdir + "/slice_z_t" + std::to_string(t + 1) + ".csv";
            ftd::csv::export_density_slice(engine, fname, 'z', mid);
            std::cout << "  Exported: " << fname << "\n";
        }
    }

    // Also export a final full-field snapshot and xy/xz slices
    ftd::csv::export_density_slice(engine, outdir + "/slice_xy_final.csv", 'z', mid);
    ftd::csv::export_density_slice(engine, outdir + "/slice_xz_final.csv", 'y', mid);
    ftd::csv::export_diagnostics_row(engine, outdir + "/diagnostics.csv");

    std::cout << "\n  Done. " << num_ticks << " ticks completed.\n";
}

// ============================================================================
// SCENARIO H: Helium Atom with CSV export (extends Scenario E)
// ============================================================================
// Same physics as E but exports density slices and timeseries to files.

void scenario_H(int lattice_size, int num_ticks, const std::string& outdir) {
    std::cout << "SCENARIO H: Helium Atom (with CSV export)\n";
    std::cout << "  Output: " << outdir << "/\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Locked proton pair
    double iso = ftd::K_B / std::sqrt(3.0);
    engine.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid, mid, mid).locked = true;
    engine.inject_particle(mid+1, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid+1, mid, mid).locked = true;

    engine.toggles.enable_all();
    engine.toggles.genesis = false;
    engine.toggles.gauss_projection = false;
    engine.run(30);

    // Inject 2 electrons
    engine.inject_particle(mid+5, mid, mid, -1, {0, 0, -ftd::K_B}, -1, 0);
    engine.voxels()[engine.lattice().index(mid+5, mid, mid)].velocity = {0, 0.06, 0};
    engine.inject_particle(mid-5, mid, mid, -1, {0, 0, -ftd::K_B}, 1, 0);
    engine.voxels()[engine.lattice().index(mid-5, mid, mid)].velocity = {0, -0.06, 0};

    engine.toggles.enable_all();
    engine.toggles.genesis = false;

    // Remove old diagnostics file
    { std::ofstream(outdir + "/timeseries.csv", std::ios::trunc); }

    int snapshot_interval = std::max(1, num_ticks / 8);
    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();

        ftd::csv::export_diagnostics_row(engine, outdir + "/timeseries.csv");

        if ((t + 1) % snapshot_interval == 0 || t == num_ticks - 1) {
            std::string fname = outdir + "/he_slice_t" + std::to_string(t + 1) + ".csv";
            ftd::csv::export_density_slice(engine, fname, 'z', mid);
            std::cout << "  Exported: " << fname << "\n";
        }
    }
    std::cout << "\n  Done. " << num_ticks << " ticks completed.\n";
}

// ============================================================================
// SCENARIO J: Pair Production with CSV export (extends Scenario B)
// ============================================================================

void scenario_J(int lattice_size, int num_ticks, const std::string& outdir) {
    std::cout << "SCENARIO J: Pair Production (with CSV export)\n";
    std::cout << "  Output: " << outdir << "/\n\n";

    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Counter-propagating beams
    double peak = 5.0;
    int beam_offset = 4;
    int radius = 3;

    for (int dx = -radius; dx <= radius; ++dx)
    for (int dy = -radius; dy <= radius; ++dy)
    for (int dz = -radius; dz <= radius; ++dz) {
        double r2 = dy*dy + dz*dz;
        if (r2 > radius*radius) continue;
        double a = peak * std::exp(-(dx*dx + r2) / 2.0);
        engine.inject_flux(mid - beam_offset + dx, mid + dy, mid + dz, {a, 0, 0});
        engine.inject_flux(mid + beam_offset + dx, mid + dy, mid + dz, {-a, 0, 0});
    }

    { std::ofstream(outdir + "/timeseries.csv", std::ios::trunc); }

    int snapshot_interval = std::max(1, num_ticks / 10);
    for (int t = 0; t < num_ticks; ++t) {
        engine.tick();
        ftd::csv::export_diagnostics_row(engine, outdir + "/timeseries.csv");

        if (t < 20 || (t + 1) % snapshot_interval == 0 || t == num_ticks - 1) {
            // Export density slices at key moments
            if (t < 10 || (t + 1) % snapshot_interval == 0) {
                std::string fname = outdir + "/pair_slice_t" + std::to_string(t + 1) + ".csv";
                ftd::csv::export_density_slice(engine, fname, 'z', mid);
            }
        }
    }

    // Final full-field snapshot
    ftd::csv::export_density_slice(engine, outdir + "/pair_slice_final.csv", 'z', mid);
    ftd::csv::export_density_slice(engine, outdir + "/pair_slice_xz.csv", 'y', mid);
    std::cout << "\n  Done. " << num_ticks << " ticks completed.\n";

    auto d = engine.diagnostics();
    std::cout << "  Final: " << d.manifested_count << " particles ("
              << d.positive_count << "+ / " << d.negative_count << "-)\n";
}

// ============================================================================
// SCENARIO K: Force Law Profile
// ============================================================================
// The key experiment of Phase 2: does 1/r² emerge from ∇(∇·J)?
// Places a single locked +1 particle at center, equilibrates flux,
// then measures |gradient_divergence(r)| along axes.
// Also tests with a probe particle at varying distances.

void scenario_K(int lattice_size, int num_ticks, const std::string& outdir) {
    std::cout << "SCENARIO K: Force Law Profile\n";
    std::cout << "  KEY QUESTION: Does 1/r^2 emerge from grad(div(J))?\n\n";

    // ---- Part 1: Static field profile ----
    std::cout << "  Part 1: Single locked +1 at center, " << num_ticks << " ticks equilibration\n";
    ftd::RenderBridge engine(lattice_size);
    int mid = lattice_size / 2;

    // Isotropic K_B flux
    double iso = ftd::K_B / std::sqrt(3.0);
    engine.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
    engine.voxel_at(mid, mid, mid).locked = true;

    // Equilibrate
    std::cout << "  Equilibrating (" << num_ticks << " ticks)...\n";
    engine.run(num_ticks);

    // Measure field profile
    std::cout << "  Measuring radial profile...\n\n";
    ftd::csv::export_radial_profile(engine, outdir + "/force_profile.csv", mid, mid, mid);

    // Also export per-particle snapshot
    ftd::csv::export_particle_snapshot(engine, outdir + "/particles.csv");

    // Print summary table
    int max_r = std::min(lattice_size / 2 - 1, 30);
    std::cout << "  r, |grad(div(J))| along +x, density, div(J)\n";
    for (int r = 1; r <= max_r; ++r) {
        int px = mid + r;
        px = ((px % lattice_size) + lattice_size) % lattice_size;
        int idx = engine.lattice().index(px, mid, mid);
        ftd::Vec3 gdj = engine.gradient_divergence(idx);
        double div = engine.divergence_flux(idx);
        double rho = engine.voxels()[idx].density();
        std::cout << "  " << std::setw(3) << r << ", "
                  << std::setprecision(6) << std::scientific << gdj.mag() << ", "
                  << rho << ", " << div << "\n";
    }

    // Fit power law: log|F| = a + b*log(r)
    // Use r = 2..max_r/2 to avoid boundary effects
    int fit_start = 2, fit_end = std::min(max_r / 2, 15);
    double sum_logr = 0, sum_logF = 0, sum_logr2 = 0, sum_logr_logF = 0;
    int n_fit = 0;
    for (int r = fit_start; r <= fit_end; ++r) {
        int px = mid + r;
        px = ((px % lattice_size) + lattice_size) % lattice_size;
        int idx = engine.lattice().index(px, mid, mid);
        ftd::Vec3 gdj = engine.gradient_divergence(idx);
        double F = gdj.mag();
        if (F < 1e-30) continue;
        double lr = std::log(static_cast<double>(r));
        double lF = std::log(F);
        sum_logr += lr;
        sum_logF += lF;
        sum_logr2 += lr * lr;
        sum_logr_logF += lr * lF;
        n_fit++;
    }
    if (n_fit >= 3) {
        double exponent = (n_fit * sum_logr_logF - sum_logr * sum_logF) /
                          (n_fit * sum_logr2 - sum_logr * sum_logr);
        std::cout << std::defaultfloat << std::setprecision(4);
        std::cout << "\n  POWER LAW FIT (r=" << fit_start << ".." << fit_end << "):\n";
        std::cout << "    |F| ~ r^(" << exponent << ")\n";
        std::cout << "    Expected for 3D Coulomb: r^(-2.0)\n";
        std::cout << "    Measured exponent: " << exponent << "\n";
    }

    // ---- Part 2: Isotropy check ----
    std::cout << "\n  Part 2: Isotropy (comparing axes at r=5)\n";
    int r_iso = 5;
    auto measure_at = [&](int dx, int dy, int dz) {
        int px = ((mid + dx) % lattice_size + lattice_size) % lattice_size;
        int py = ((mid + dy) % lattice_size + lattice_size) % lattice_size;
        int pz = ((mid + dz) % lattice_size + lattice_size) % lattice_size;
        int idx = engine.lattice().index(px, py, pz);
        return engine.gradient_divergence(idx).mag();
    };
    double f_px = measure_at(r_iso, 0, 0);
    double f_py = measure_at(0, r_iso, 0);
    double f_pz = measure_at(0, 0, r_iso);
    double f_avg = (f_px + f_py + f_pz) / 3.0;
    double f_min = std::min({f_px, f_py, f_pz});
    double f_max = std::max({f_px, f_py, f_pz});
    double isotropy = (f_avg > 1e-30) ? f_min / f_max : 0.0;

    std::cout << std::setprecision(6) << std::scientific;
    std::cout << "    +x: " << f_px << "\n";
    std::cout << "    +y: " << f_py << "\n";
    std::cout << "    +z: " << f_pz << "\n";
    std::cout << std::defaultfloat << std::setprecision(4);
    std::cout << "    Isotropy ratio (min/max): " << isotropy
              << " (1.0 = perfect)\n";

    // ---- Part 3: Probe particle test ----
    std::cout << "\n  Part 3: Probe particle (free -1 at various r)\n";
    std::cout << "  r, force_on_probe\n";
    for (int r = 2; r <= std::min(15, max_r); ++r) {
        // Fresh engine for each distance
        ftd::RenderBridge eng2(lattice_size);
        eng2.inject_particle(mid, mid, mid, +1, {iso, iso, iso});
        eng2.voxel_at(mid, mid, mid).locked = true;

        // Equilibrate
        eng2.run(500);

        // Place probe
        eng2.inject_particle(mid + r, mid, mid, -1, {iso, iso, iso});
        eng2.voxel_at(mid + r, mid, mid).locked = true;

        // Run 1 tick to compute forces
        eng2.run(1);

        // Read force on probe
        int probe_idx = eng2.lattice().index(mid + r, mid, mid);
        const auto& fd = eng2.force_diag_at(probe_idx);
        double f_total = (fd.f_coulomb + fd.f_gravity).mag();
        std::cout << "  " << std::setw(3) << r << ", "
                  << std::setprecision(6) << std::scientific << f_total << "\n";
    }

    // Energy audit
    auto ea = engine.energy_audit();
    std::cout << std::defaultfloat << std::setprecision(6);
    std::cout << "\n  ENERGY AUDIT:\n";
    std::cout << "    Field energy:     " << ea.field_energy << "\n";
    std::cout << "    Wave energy:      " << ea.wave_energy << "\n";
    std::cout << "    Particle KE:      " << ea.particle_ke << "\n";
    std::cout << "    Total energy:     " << ea.total_energy << "\n";
    std::cout << "    Gauss violation:  " << ea.gauss_violation << "\n";
    std::cout << "    Max Gauss error:  " << ea.max_gauss_error << "\n";
    std::cout << "    Charge total:     " << ea.charge_total << "\n";

    std::cout << "\n  CSV output: " << outdir << "/force_profile.csv\n";
    std::cout << "  Done.\n";
}

int main(int argc, char* argv[]) {
    char scenario = 'D';
    int lattice_size = 32;
    int num_ticks = 2000;

    if (argc > 1) {
        char c = argv[1][0];
        if (c == 'A' || c == 'a') scenario = 'A';
        else if (c == 'B' || c == 'b') scenario = 'B';
        else if (c == 'E' || c == 'e') scenario = 'E';
        else if (c == 'F' || c == 'f') scenario = 'F';
        else if (c == 'G' || c == 'g') scenario = 'G';
        else if (c == 'H' || c == 'h') scenario = 'H';
        else if (c == 'I' || c == 'i') scenario = 'I';
        else if (c == 'J' || c == 'j') scenario = 'J';
        else if (c == 'K' || c == 'k') scenario = 'K';
        else scenario = 'D';
    }
    if (argc > 2) lattice_size = std::atoi(argv[2]);
    if (argc > 3) num_ticks = std::atoi(argv[3]);

    if (lattice_size < 4 || lattice_size > 256) {
        std::cerr << "Lattice size must be between 4 and 256\n";
        return 1;
    }

    print_header();

    // Output directory for CSV-exporting scenarios (4th arg or default)
    std::string outdir = "output";
    if (argc > 4) outdir = argv[4];

    switch (scenario) {
        case 'A': scenario_A(lattice_size, num_ticks); break;
        case 'B': scenario_B(lattice_size, num_ticks); break;
        case 'E': scenario_E(lattice_size, num_ticks); break;
        case 'F': scenario_F(lattice_size, num_ticks); break;
        case 'G': scenario_G(lattice_size, num_ticks); break;
        case 'H': scenario_H(lattice_size, num_ticks, outdir); break;
        case 'I': scenario_I(lattice_size, num_ticks, outdir); break;
        case 'J': scenario_J(lattice_size, num_ticks, outdir); break;
        case 'K': scenario_K(lattice_size, num_ticks, outdir); break;
        default:  scenario_default(lattice_size, num_ticks); break;
    }

    return 0;
}
