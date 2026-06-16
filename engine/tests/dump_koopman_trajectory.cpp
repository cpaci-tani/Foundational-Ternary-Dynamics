#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <cmath>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/scale_context.h"

// FTD Koopman Observable Dumper
// Injects the A=14 canonical cloud and runs the Langevin bath.
// Exports observables to CSV for Koopman estimator processing.
//
// Each row is annotated with the read-only scale-context readout admissibility
// diagnostics (sc_* columns) so the downstream Python estimator can refuse a
// trajectory whose cloud is not scale-separated / self-confined / stationary.
// The gate runs in OBSERVE-ONLY mode here (it never blocks the dump); the hard
// refusal lives in scripts/proofs/proof_alpha_stochastic_koopman.py.
// See docs/theory/01_reference/SPEC_SCALE_CONTEXT_READOUT.md.

int main(int argc, char** argv) {
    std::cout << "================================================================\n";
    std::cout << "  FTD: Dump Koopman Stochastic Observables\n";
    std::cout << "================================================================\n\n";

    int L = 32;
    int n_ticks = 100000;
    double A = 14.0;

    if (argc > 1) {
        n_ticks = std::atoi(argv[1]);
    }
    // Optional second arg: lattice size L (default 32). Larger L is the
    // scale-separation knob — the same physics at a bigger box probes whether
    // the cloud's R_eff is intrinsic (admissible) or box-driven (percolating).
    if (argc > 2) {
        L = std::atoi(argv[2]);
        if (L < 4) L = 4;
    }

    if (argc > 3) {
        A = std::atof(argv[3]);
    }

    std::cout << "Usage: dump_koopman_trajectory [n_ticks] [L] [A]\n";
    std::cout << "Configuration: L=" << L << ", N_TICKS=" << n_ticks << ", A=" << A << "\n";
    std::cout << "Bath: T=0.005, gamma=0.02 (Canonical Persistence Baseline)\n";
    
    ftd::RenderBridge rb(L);
    
    // Apply Canonical Bath Toggles
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis = true;
    rb.toggles.langevin = true;
    rb.toggles.langevin_T = 0.005;
    rb.toggles.langevin_gamma = 0.02;

    const int cx = L / 2, cy = L / 2, cz = L / 2;
    // [IMPOSED] amplitude convention (2026-06-15): A is in genesis-threshold
    // units, so the injected flux magnitude is A*K_GENESIS. This unifies the
    // dumper with campaign_alpha_readout_scattering.cpp (same convention) and
    // RE-PINS the canonical "A=14" cloud relative to the pre-2026-06 raw-A
    // injection (physical amplitude rises by K_GENESIS ~= 1.533).
    const double amp = A * ftd::K_GENESIS;
    rb.inject_flux(cx, cy, cz, {amp, 0.0, 0.0});
    std::cout << "Injection: A=" << A << " (genesis units) -> |J|=" << amp
              << " = A*K_GENESIS  (K_GENESIS=" << ftd::K_GENESIS << ")\n";

    // Observe-only scale-context tracker (gate_active=false by default).
    ftd::ScaleContextConfig sc_cfg;          // [IMPOSED] defaults
    ftd::ScaleContextTracker sc(sc_cfg);
    ftd::ScaleContextDiagnostics scd{};

    std::string out_file = "traj_koopman_A14.csv";
    if (argc > 4) {
        out_file = argv[4];
    }
    std::ofstream out;
    if (out_file != "none") {
        out.open(out_file);
        out << "tick,q_x,p_x,rho,j2,rho_core,rho_shell,j2_core,j2_shell,"
               "Qxx,Qyy,Qzz,Qxy,Qxz,Qyz,R2,flips,flux_leak,"
               "sc_R_eff,sc_kappa,sc_zeta,sc_beta,sc_factive,sc_phi_out,sc_phi_ret,"
               "sc_dPhidR,sc_dRdt,sc_dJ2dt,sc_tau,sc_Theta,sc_regime,sc_status\n";
    }
    
    std::cout << "Running thermalization transient (2000 ticks)...\n";
    for (int t = 0; t < 2000; ++t) {
        rb.tick();
    }
    
    std::cout << "Recording Koopman observables...\n";
    
    const int N_vox = L * L * L;
    std::vector<int> last_state(N_vox, 0);
    const double R_c = 4.0;

    for (int t = 0; t < n_ticks; ++t) {
        rb.tick();
        
        double q = 0.0, p = 0.0, rho = 0.0, j2 = 0.0;
        double rho_core = 0.0, rho_shell = 0.0;
        double j2_core = 0.0, j2_shell = 0.0;
        double Qxx = 0.0, Qyy = 0.0, Qzz = 0.0;
        double Qxy = 0.0, Qxz = 0.0, Qyz = 0.0;
        double R2 = 0.0, flux_leak = 0.0;
        int flips = 0;
        
        const auto& voxels = rb.voxels();
        for (int i = 0; i < N_vox; ++i) {
            const auto& v = voxels[i];
            
            if (v.state != last_state[i]) {
                flips++;
                last_state[i] = v.state;
            }
            
            if (v.state != 0) {
                int z = i / (L * L);
                int y = (i / L) % L;
                int x = i % L;
                
                double dx = x - cx;
                double dy = y - cy;
                double dz = z - cz;
                double r2 = dx*dx + dy*dy + dz*dz;
                double r = std::sqrt(r2);
                
                q += v.flux.x;
                p += v.wave_vel.x;
                rho += 1.0;
                double v_j2 = v.flux.mag2();
                j2 += v_j2;
                
                Qxx += dx*dx;
                Qyy += dy*dy;
                Qzz += dz*dz;
                Qxy += dx*dy;
                Qxz += dx*dz;
                Qyz += dy*dz;
                R2 += r2;
                
                if (r < R_c) {
                    rho_core += 1.0;
                    j2_core += v_j2;
                } else {
                    rho_shell += 1.0;
                    j2_shell += v_j2;
                    if (r > 0) {
                        flux_leak += (v.flux.x * dx + v.flux.y * dy + v.flux.z * dz) / r;
                    }
                }
            }
        }
        
        // Read-only scale-context annotation for this tick (observe-only).
        scd = sc.ingest(rb);

        if (out_file != "none") {
            out << t << "," << q << "," << p << "," << rho << "," << j2 << ","
                << rho_core << "," << rho_shell << "," << j2_core << "," << j2_shell << ","
                << Qxx << "," << Qyy << "," << Qzz << "," << Qxy << "," << Qxz << "," << Qyz << ","
                << R2 << "," << flips << "," << flux_leak << ","
                << scd.R_eff << "," << scd.kappa << "," << scd.zeta << "," << scd.beta << ","
                << scd.active_fraction << "," << scd.phi_outward << "," << scd.phi_return << ","
                << scd.dPhi_dR << "," << scd.dR_dt << "," << scd.dJ2_dt << ","
                << scd.tau_cloud << "," << scd.Theta << ","
                << static_cast<int>(scd.regime) << "," << static_cast<int>(scd.status) << "\n";
        }

        if ((t + 1) % 10000 == 0) {
            std::cout << "  Recorded " << (t + 1) << " / " << n_ticks << " ticks...\n";
        }
    }

    if (out_file != "none") {
        out.close();
        std::cout << "Saved observables to " << out_file << "\n";
    }

    // Final windowed readout-status verdict, derived from the last tick's
    // regime + rolling self-confinement / stationarity fields. (The tracker
    // ran observe-only, so we report the verdict the armed gate would give.)
    {
        const bool stationary_ok = scd.stationary;
        const bool confined_ok   = scd.confinement_fixed_point;
        const char* regime_name[] = {"Indeterminate", "Evaporating", "UVLocked",
                                     "BoundedAdmissible", "ShellDominated", "Percolating"};
        const int ri = static_cast<int>(scd.regime);
        std::cout << "READOUT_STATUS regime="
                  << (ri >= 0 && ri <= 5 ? regime_name[ri] : "?")
                  << " R_eff=" << scd.R_eff
                  << " kappa=" << scd.kappa
                  << " zeta=" << scd.zeta
                  << " beta=" << scd.beta
                  << " confined=" << (confined_ok ? "true" : "false")
                  << " stationary=" << (stationary_ok ? "true" : "false") << "\n";
        if (scd.regime == ftd::ScaleRegime::BoundedAdmissible && confined_ok && stationary_ok) {
            std::cout << "READOUT_STATUS = ADMISSIBLE\n";
        } else if (scd.regime == ftd::ScaleRegime::BoundedAdmissible && !confined_ok) {
            std::cout << "READOUT_STATUS = REJECTED_SELF_CONFINEMENT\n";
        } else if (scd.regime == ftd::ScaleRegime::BoundedAdmissible && !stationary_ok) {
            std::cout << "READOUT_STATUS = REJECTED_NON_STATIONARY\n";
        } else {
            std::cout << "READOUT_STATUS = REJECTED_SCALE_CONTEXT\n";
        }
    }
    return 0;
}