#include "ftd/render_bridge.h"
#include "ftd/eft/dual_cell_blocking.h"
#include "ftd/eft/dual_cell_continuity.h"
#include "ftd/eft/dual_cell_flow.h"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>

std::vector<int> state_snapshot(const ftd::RenderBridge& rb) {
  std::vector<int> out(static_cast<size_t>(rb.lattice().total_sites()), 0);
  const auto& voxels = rb.voxels();
  for (size_t i = 0; i < out.size(); ++i) {
    out[i] = static_cast<int>(voxels[i].state);
  }
  return out;
}

int main(int argc, char** argv) {
    int L = 32;
    int snapshots = 1000;
    int warmup = 1000;
    double T_langevin = 0.100;
    std::string out_path = "native_scale_flow_telemetry.csv";

    // Parse simple args
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--L" && i + 1 < argc) L = std::stoi(argv[++i]);
        else if (arg == "--snapshots" && i + 1 < argc) snapshots = std::stoi(argv[++i]);
        else if (arg == "--warmup" && i + 1 < argc) warmup = std::stoi(argv[++i]);
        else if (arg == "--T" && i + 1 < argc) T_langevin = std::stod(argv[++i]);
        else if (arg == "--out" && i + 1 < argc) out_path = argv[++i];
    }

    std::cout << "Starting Native Scale Flow Campaign (L=" << L << ")\n";

    ftd::RenderBridge rb(L);
    // Enable nonlinear ensemble physics
    rb.toggles.disable_all();
    rb.toggles.gauss_projection = true;
    rb.toggles.forces = true;
    rb.toggles.movement = true;
    rb.toggles.pair_production = true;
    rb.toggles.genesis = true;
    rb.toggles.evaporation = true;
    rb.toggles.langevin = true;
    rb.toggles.langevin_T = T_langevin;

    std::cout << "Warming up for " << warmup << " ticks...\n";
    for (int i = 0; i < warmup; i++) {
        rb.tick();
    }

    std::ofstream out(out_path);
    out << "tick,L,b,e_flux,v_coupling,total_q,total_sq,reaction_l1\n";

    std::cout << "Recording " << snapshots << " snapshots...\n";
    for (int i = 0; i < snapshots; i++) {
        auto before = state_snapshot(rb);
        rb.tick();
        auto after = state_snapshot(rb);

        ftd::eft::DualCellContinuity fine_current(L);
        auto hist = ftd::eft::extract_moore_history_from_snapshots(L, before, after, fine_current);
        auto fine_flux = ftd::eft::render_bridge_to_dual_cell_fields(rb);
        
        // Cascade blocking to b=2 and b=4
        auto b2_flux = ftd::eft::block_dual_cell_b2(fine_flux);
        auto b2_current = ftd::eft::block_dual_cell_continuity_b2(fine_current);

        auto b4_flux = ftd::eft::block_dual_cell_b2(b2_flux);
        auto b4_current = ftd::eft::block_dual_cell_continuity_b2(b2_current);

        // Compute observables (e_flux and v_coupling)
        auto log_scale = [&](int b, const ftd::eft::DualCellFields& flux, const ftd::eft::DualCellContinuity& current) {
            double cell_vol = b * b * b;
            double face_area = b * b;

            double e_flux = ftd::eft::canonical_flux_energy(flux, cell_vol, face_area);
            double v_coupling = ftd::eft::canonical_current_flux_vertex(current, flux, cell_vol, face_area);
            
            // source totals
            int total_q = ftd::eft::total_source(flux);
            
            // custom state squared sum
            int sq_sum = 0;
            for(int q : flux.rho_cell) sq_sum += q * q;

            int reaction_l1 = ftd::eft::total_reaction_l1(current);

            out << rb.current_tick() << "," << L << "," << b << ","
                << e_flux << "," << v_coupling << ","
                << total_q << "," << sq_sum << "," << reaction_l1 << "\n";
        };

        log_scale(1, fine_flux, fine_current);
        log_scale(2, b2_flux, b2_current);
        log_scale(4, b4_flux, b4_current);

        if ((i+1) % (snapshots / 10 == 0 ? 1 : snapshots / 10) == 0) {
            std::cout << "  progress: " << (i+1) << "/" << snapshots << "\n";
        }
    }

    std::cout << "Campaign complete. Telemetry saved to " << out_path << "\n";
    return 0;
}
