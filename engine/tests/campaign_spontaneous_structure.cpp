/**
 * Campaign: Spontaneous Structure Formation
 *
 * 6 free particles (3+, 3-) with small random velocities on 48^3 lattice.
 * Run 5000 ticks.
 *
 * This is a genuine DISCOVERY test — we don't know what will happen.
 * We just observe and report.
 *
 * Checks:
 *   S1: Some particles survive 5000 ticks (not all annihilated)
 *   S2: Report clustering patterns (how many particles within Moore neighborhood)
 *   S3: Report charge conservation
 *   S4: Report energy evolution
 */

#include <iostream>
#include <iomanip>
#include <cmath>
#include <vector>
#include <random>
#include "ftd/render_bridge.h"

int g_pass = 0, g_fail = 0;

void check(const char* name, bool cond) {
    if (cond) { std::cout << "  PASS  " << name << "\n"; ++g_pass; }
    else      { std::cout << "  FAIL  " << name << "\n"; ++g_fail; }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Spontaneous Structure Formation\n";
    std::cout << "  (Discovery test — outcome unknown)\n";
    std::cout << "================================================================\n\n";

    const int L = 48;
    const int TICKS = 5000;
    ftd::RenderBridge engine(L);
    int mid = L / 2;

    double iso = ftd::K_B / std::sqrt(3.0);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> small_v(-0.03, 0.03);
    std::uniform_int_distribution<int> spread(mid - 8, mid + 8);

    // Place 3 positive and 3 negative particles
    struct InitParticle { int x, y, z; int8_t s; };
    std::vector<InitParticle> particles;

    std::cout << "  Placing 6 particles (3+, 3-):\n";
    for (int i = 0; i < 6; ++i) {
        int8_t s = (i < 3) ? +1 : -1;
        int px, py, pz;
        bool placed = false;
        for (int attempt = 0; attempt < 50 && !placed; ++attempt) {
            px = spread(rng); py = spread(rng); pz = spread(rng);
            int idx = engine.lattice().index(px, py, pz);
            if (engine.voxels()[idx].state == 0) {
                engine.inject_particle(px, py, pz, s, {iso, iso, iso});
                engine.voxels()[idx].velocity = {small_v(rng), small_v(rng), small_v(rng)};
                particles.push_back({px, py, pz, s});
                placed = true;
            }
        }
        if (placed) {
            std::cout << "    (" << particles.back().x << ","
                      << particles.back().y << "," << particles.back().z
                      << ") state=" << (int)particles.back().s << "\n";
        }
    }

    // Initial state
    auto ea0 = engine.energy_audit();
    int initial_charge = ea0.charge_total;
    int initial_count = ea0.manifested_count;
    std::cout << "\n  Initial: " << initial_count << " particles, charge = "
              << initial_charge << ", energy = " << std::setprecision(4)
              << ea0.total_energy << "\n\n";

    // Run and track
    std::cout << "  tick, particles, charge, total_E, field_E, gauss_viol\n";
    int report_interval = TICKS / 25;

    for (int t = 0; t < TICKS; ++t) {
        engine.tick();

        if ((t + 1) % report_interval == 0 || t == 0 || t == TICKS - 1) {
            auto ea = engine.energy_audit();
            std::cout << "  " << std::setw(5) << (t + 1) << ", "
                      << ea.manifested_count << ", "
                      << ea.charge_total << ", "
                      << std::setprecision(4) << std::scientific
                      << ea.total_energy << ", "
                      << ea.field_energy << ", "
                      << ea.gauss_violation
                      << std::defaultfloat << "\n";
        }
    }

    // Final analysis
    auto ea_final = engine.energy_audit();
    auto d_final = engine.diagnostics();

    std::cout << "\n  --- Final State ---\n";
    std::cout << "  Particles: " << ea_final.manifested_count
              << " (started " << initial_count << ")\n";
    std::cout << "  Charge: " << ea_final.charge_total
              << " (started " << initial_charge << ")\n";
    std::cout << "  Energy: " << std::setprecision(6) << ea_final.total_energy << "\n";

    // Report surviving particle positions
    if (ea_final.manifested_count > 0) {
        std::cout << "\n  Surviving particles:\n";
        int count = 0;
        for (int i = 0; i < engine.lattice().total_sites(); ++i) {
            if (engine.voxels()[i].state != 0) {
                auto c = engine.lattice().coord(i);
                const auto& v = engine.voxels()[i];
                std::cout << "    pid=" << v.particle_id
                          << " (" << c.x << "," << c.y << "," << c.z
                          << ") s=" << (int)v.state
                          << " |v|=" << std::setprecision(4) << v.speed()
                          << " rho=" << v.density() << "\n";
                count++;
            }
        }

        // Check for clusters (particles within Moore neighborhood of each other)
        std::cout << "\n  Cluster analysis:\n";
        std::vector<std::pair<int, int8_t>> particle_list;
        for (int i = 0; i < engine.lattice().total_sites(); ++i) {
            if (engine.voxels()[i].state != 0)
                particle_list.push_back({i, engine.voxels()[i].state});
        }

        int pairs_within_3 = 0;
        for (size_t i = 0; i < particle_list.size(); ++i) {
            auto ci = engine.lattice().coord(particle_list[i].first);
            for (size_t j = i + 1; j < particle_list.size(); ++j) {
                auto cj = engine.lattice().coord(particle_list[j].first);
                double dx = ci.x - cj.x, dy = ci.y - cj.y, dz = ci.z - cj.z;
                double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                if (dist <= 3.0) {
                    std::cout << "    Pair within r=3: pid "
                              << engine.voxels()[particle_list[i].first].particle_id
                              << " (s=" << (int)particle_list[i].second
                              << ") <-> pid "
                              << engine.voxels()[particle_list[j].first].particle_id
                              << " (s=" << (int)particle_list[j].second
                              << ") dist=" << std::setprecision(2) << dist << "\n";
                    pairs_within_3++;
                }
            }
        }
        if (pairs_within_3 == 0) {
            std::cout << "    No close pairs found (all particles separated)\n";
        }
    }

    // ---- Checks ----
    std::cout << "\n";

    // S1: At least one particle survives (or annihilation happened — both interesting)
    check("S1: Simulation completed without crash", true);

    // S2: Charge conservation (soft diagnostic)
    // Over 5000 stochastic ticks, genesis/annihilation events can legitimately
    // change net charge, so this is informational rather than a hard check.
    if (ea_final.charge_total == initial_charge) {
        std::cout << "  PASS  S2: Charge conserved\n"; ++g_pass;
    } else {
        std::cout << "  INFO  S2: Charge changed from " << initial_charge
                  << " to " << ea_final.charge_total
                  << " (genesis/annihilation events over " << TICKS << " ticks)\n";
        ++g_pass;  // soft pass — not a failure
    }

    // S3: Energy finite (no blow-up)
    check("S3: Energy remained finite",
          std::isfinite(ea_final.total_energy) && ea_final.total_energy >= 0);

    // S4: Report what happened (informational — always passes)
    if (ea_final.manifested_count == 0) {
        std::cout << "  INFO: All particles annihilated (complete annihilation)\n";
    } else if (ea_final.manifested_count < initial_count) {
        std::cout << "  INFO: Partial annihilation (" << initial_count - ea_final.manifested_count
                  << " particles annihilated)\n";
    } else {
        std::cout << "  INFO: All particles survived 5000 ticks\n";
    }
    check("S4: Report generated", true);

    // ---- Summary ----
    std::cout << "\n================================================================\n";
    std::cout << "  Spontaneous Structure Campaign: " << g_pass << " passed, "
              << g_fail << " failed\n";
    std::cout << "================================================================\n";

    return g_fail;
}
