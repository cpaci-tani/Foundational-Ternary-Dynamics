/**
 * Phase 7 — Stage 3: Scale Bridge unit tests (8 checks)
 *
 * SB1: Coarsen charge matches voxel state
 * SB2: Coarsen position matches coord + remainder
 * SB3: Coarsen velocity preserved exactly
 * SB4: Refine placement at correct lattice site
 * SB5: Refine energy — wavepacket energy ~ K_B^2
 * SB6: Round-trip position — coarsen(refine(p)) within 1 voxel
 * SB7: Round-trip velocity preserved exactly
 * SB8: Multi-particle — coarsen finds all particles
 */

#include "ftd/scale.h"
#include "ftd/particle_engine.h"
#include "ftd/render_bridge.h"
#include <iostream>
#include <cmath>

static int pass_count = 0;
static int fail_count = 0;

static void check(const char* name, bool ok) {
    if (ok) { ++pass_count; std::cout << "  PASS  " << name << "\n"; }
    else    { ++fail_count; std::cout << "  FAIL  " << name << "\n"; }
}

int main() {
    using namespace ftd;

    std::cout << "============================================================\n";
    std::cout << "  Phase 7 Stage 3: Scale Bridge Unit Tests\n";
    std::cout << "============================================================\n\n";

    const int N = 32;

    // ---- SB1: Coarsen charge ----
    {
        std::cout << "--- SB1: Coarsen charge ---\n";
        RenderBridge rb(N);
        rb.inject_particle(10, 10, 10, +1, {0.5, 0, 0});
        rb.inject_particle(20, 20, 20, -1, {-0.5, 0, 0});

        auto particles = coarsen_to_particles(rb);
        // Find particles by position
        bool found_pos = false, found_neg = false;
        for (auto& p : particles) {
            if (p.charge == +1) found_pos = true;
            if (p.charge == -1) found_neg = true;
        }
        check("SB1a: found +1 charge", found_pos);
        check("SB1b: found -1 charge", found_neg);
    }

    // ---- SB2: Coarsen position ----
    {
        std::cout << "\n--- SB2: Coarsen position ---\n";
        RenderBridge rb(N);
        rb.inject_particle(15, 10, 5, +1, {0.5, 0, 0});
        auto& v = rb.voxel_at(15, 10, 5);
        v.remainder = {0.3, 0.7, 0.1};

        auto particles = coarsen_to_particles(rb);
        // Find the particle we placed
        bool found = false;
        for (auto& p : particles) {
            if (std::abs(p.position.x - 15.3) < 0.01 &&
                std::abs(p.position.y - 10.7) < 0.01 &&
                std::abs(p.position.z - 5.1) < 0.01) {
                found = true;
            }
        }
        check("SB2: position = coord + remainder", found);
    }

    // ---- SB3: Coarsen velocity ----
    {
        std::cout << "\n--- SB3: Coarsen velocity ---\n";
        RenderBridge rb(N);
        rb.inject_particle(10, 10, 10, +1, {0.5, 0, 0});
        rb.voxel_at(10, 10, 10).velocity = {0.05, -0.03, 0.01};

        auto particles = coarsen_to_particles(rb);
        bool found = false;
        for (auto& p : particles) {
            if (p.charge == +1) {
                found = (std::abs(p.velocity.x - 0.05) < 1e-15 &&
                         std::abs(p.velocity.y + 0.03) < 1e-15 &&
                         std::abs(p.velocity.z - 0.01) < 1e-15);
            }
        }
        check("SB3: velocity preserved exactly", found);
    }

    // ---- SB4: Refine placement ----
    {
        std::cout << "\n--- SB4: Refine placement ---\n";
        RenderBridge rb(N);
        Particle p;
        p.charge = +1;
        p.position = {12.4, 8.7, 3.2};
        p.velocity = {0.01, 0, 0};

        refine_to_voxels(p, rb);

        // Should be placed at integer coords (12, 8, 3)
        auto& v = rb.voxel_at(12, 8, 3);
        check("SB4: particle at correct lattice site", v.state == +1);
    }

    // ---- SB5: Refine energy ----
    {
        std::cout << "\n--- SB5: Refine energy ---\n";
        RenderBridge rb(N);
        Particle p;
        p.charge = +1;
        p.position = {16, 16, 16};

        refine_to_voxels(p, rb);

        // Compute total field energy (sum |J|^2)
        double total_e = 0.0;
        for (auto& v : rb.voxels()) {
            total_e += v.flux.mag2();
        }
        double target = K_B * K_B;
        double err = std::abs(total_e - target) / target;
        std::cout << "    Total field energy: " << total_e << " (target: " << target << ")\n";
        std::cout << "    Relative error: " << err * 100.0 << "%\n";
        check("SB5: wavepacket energy within 5% of K_B^2", err < 0.05);
    }

    // ---- SB6: Round-trip position ----
    {
        std::cout << "\n--- SB6: Round-trip position ---\n";
        RenderBridge rb(N);
        Particle p_in;
        p_in.charge = +1;
        p_in.position = {14.6, 11.3, 7.8};
        p_in.velocity = {0.02, -0.01, 0.005};

        // Refine: particle → voxels
        refine_to_voxels(p_in, rb);

        // Coarsen: voxels → particles
        auto particles = coarsen_to_particles(rb);

        // Find our particle (should be at roughly the same position)
        bool found = false;
        for (auto& p_out : particles) {
            if (p_out.charge == +1) {
                double dx = p_out.position.x - p_in.position.x;
                double dy = p_out.position.y - p_in.position.y;
                double dz = p_out.position.z - p_in.position.z;
                double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
                std::cout << "    Round-trip distance: " << dist << " voxels\n";
                found = (dist < 1.0);
            }
        }
        check("SB6: round-trip position within 1 voxel", found);
    }

    // ---- SB7: Round-trip velocity ----
    {
        std::cout << "\n--- SB7: Round-trip velocity ---\n";
        RenderBridge rb(N);
        Particle p_in;
        p_in.charge = -1;
        p_in.position = {16, 16, 16};
        p_in.velocity = {0.03, -0.02, 0.01};

        refine_to_voxels(p_in, rb);
        auto particles = coarsen_to_particles(rb);

        bool found = false;
        for (auto& p_out : particles) {
            if (p_out.charge == -1) {
                double dv = (p_out.velocity - p_in.velocity).mag();
                std::cout << "    Velocity difference: " << dv << "\n";
                found = (dv < 1e-15);
            }
        }
        check("SB7: round-trip velocity preserved exactly", found);
    }

    // ---- SB8: Multi-particle coarsen ----
    {
        std::cout << "\n--- SB8: Multi-particle ---\n";
        RenderBridge rb(N);
        rb.inject_particle(5, 5, 5, +1, {0.5, 0, 0});
        rb.inject_particle(10, 10, 10, -1, {-0.5, 0, 0});
        rb.inject_particle(20, 20, 20, +1, {0, 0.5, 0});

        auto particles = coarsen_to_particles(rb);
        std::cout << "    Found " << particles.size() << " particles (expected 3)\n";
        check("SB8: coarsen finds all 3 particles", particles.size() == 3);
    }

    // ---- Summary ----
    std::cout << "\n============================================================\n";
    std::cout << "  Scale Bridge: " << pass_count << " passed, " << fail_count << " failed\n";
    std::cout << "============================================================\n";

    return fail_count;
}
