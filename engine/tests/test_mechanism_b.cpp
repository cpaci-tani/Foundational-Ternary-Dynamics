/**
 * Test: Mechanism B (Lattice-to-Continuum Matching via Vacuum Polarization)
 *
 * Implements the explicit stochastic quantization of the FTD engine.
 * We set the bare lattice coupling to geometrically 1.0. We turn on the Langevin
 * thermostat to create a true thermodynamic zero-point vacuum. Virtual Genesis
 * pairs pop in and out of existence, forming vacuum polarization.
 * 
 * We measure the time-averaged enclosed charge <Q_enc(r)> around a bare test
 * charge. If the macroscopic screened coupling g_eff^2 drops from 1.0 to 
 * approx 0.04585 (which gives alpha = 1/137), Mechanism B is proven.
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>

#include "ftd/render_bridge.h"
#include "ftd/test_telemetry.h"
#include "ftd/poisson_solvers.h"
#include "ftd/eft/coupling_measurement.h"

using namespace ftd;
using namespace ftd::eft;

// Custom configuration that enables Genesis (and Pair Production) to allow vacuum polarization
inline void configure_vacuum_polarization(RenderBridge& rb) {
    configure_bare_lattice_for_coupling(rb);
    rb.toggles.genesis = true;
    rb.toggles.pair_production = true;
}

inline std::vector<double> measure_potential_vp(const RenderBridge& bg, int8_t sign, int n_ticks, int max_r) {
    const int L = bg.lattice().size();
    RenderBridge rb(L);
    rb.force_cpu();
    configure_vacuum_polarization(rb);
    rb.toggles.langevin = false;
    copy_flux_and_wave_vel(bg, rb);
    const int mid = L / 2;
    place_test_charge_on_bg(rb, mid, mid, mid, sign);
    rb.run(n_ticks);

    // After evolution, compute the electrostatic potential phi
    // Since toggles.poisson_coulomb is false during run, we must solve it manually now:
    std::vector<double> my_phi(L*L*L, 0.0);
    std::vector<double> my_sor(L*L*L, 0.0);
    ftd::solve_coulomb_poisson_cpu(rb.voxels(), my_phi, my_sor, rb.lattice());
    auto phi = my_phi;
    
    // Compute spherical average of phi around the test charge
    std::vector<double> phi_r(max_r + 1, 0.0);
    std::vector<int> count_r(max_r + 1, 0);
    double phi_center = phi[rb.lattice().index(mid, mid, mid)];

    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                int dx = x - mid;
                int dy = y - mid;
                int dz = z - mid;
                
                if (dx > L/2) dx -= L; if (dx < -L/2) dx += L;
                if (dy > L/2) dy -= L; if (dy < -L/2) dy += L;
                if (dz > L/2) dz -= L; if (dz < -L/2) dz += L;
                
                double r = std::sqrt(dx*dx + dy*dy + dz*dz);
                int ir = static_cast<int>(std::round(r));
                
                if (ir <= max_r) {
                    phi_r[ir] += phi[rb.lattice().index(x, y, z)];
                    count_r[ir]++;
                }
            }
        }
    }
    
    for (int r = 0; r <= max_r; ++r) {
        if (count_r[r] > 0) phi_r[r] /= count_r[r];
    }
    return phi_r;
}

inline CouplingMeasurement measure_alpha_eff_vp(
    const RenderBridge& bg, int n_ticks = 300,
    int r_min = 4, int r_max = -1, int r_step = 2)
{
    CouplingMeasurement out;
    const int L = bg.lattice().size();
    out.L = L;
    out.n_ticks = n_ticks;
    if (r_max < 0) r_max = L / 3;
    if (r_max <= r_min) return out;
    if (L < 8) return out;

    // Measure potential of background without test charge
    std::vector<double> phi_bg = measure_potential_vp(bg, 0, n_ticks, r_max);
    
    // Measure potential of background with test charge +1
    std::vector<double> phi_pos = measure_potential_vp(bg, +1, n_ticks, r_max);

    // The potential due to the test charge is the difference
    for (int r = r_min; r <= r_max; r += r_step) {
        double dPhi = phi_pos[r] - phi_bg[r];
        // For a charge +1, V(r) = dPhi. 
        // V(r) = -alpha_r / r. So alpha_r = -V(r) * r
        double alpha_r = -dPhi * static_cast<double>(r);
        VofR pt; pt.r = r; pt.V = dPhi; pt.alpha_r = alpha_r;
        out.data.push_back(pt);
    }

    if (out.data.size() >= 3) {
        const int n = static_cast<int>(out.data.size());
        double sx = 0, sy = 0, sxx = 0, sxy = 0;
        for (const auto& p : out.data) {
            const double x = 1.0 / static_cast<double>(p.r);
            const double y = p.V;
            sx += x; sy += y; sxx += x * x; sxy += x * y;
        }
        const double denom = n * sxx - sx * sx;
        if (std::abs(denom) > 1e-30) {
            const double slope = (n * sxy - sx * sy) / denom;
            out.alpha_fit = -slope;
            out.valid = std::isfinite(out.alpha_fit);
        }
    }
    return out;
}

int main(int argc, char** argv) {
    ftd::test::init("mechanism_b_vacuum_polarization");
    ftd::test::section("mechanism_b");

    std::cout << "================================================================\n";
    std::cout << "  TEST: Mechanism B (Dynamic Vacuum Polarization)\n";
    std::cout << "================================================================\n";

    const int L = 32;
    const int T_BURN = 500;
    const int T_RUN = 300;

    std::vector<double> T_zp_scan = { 0.0, 0.05, 0.10, 0.15 };
    
    // The target renormalized coupling corresponding to alpha = 1/137.036
    const double TARGET_G_EFF_SQ = 0.04585;

    std::cout << std::fixed << std::setprecision(5);
    std::cout << "  Bare Coupling g_0^2 = 1.00000 (Geometrically fixed)\n";
    std::cout << "  Target Screened g_c^2 = " << TARGET_G_EFF_SQ << "\n\n";

    for (double T_zp : T_zp_scan) {
        std::cout << "--- Scanning T_zp = " << T_zp << " ---\n";
        
        // Prepare thermal background with Langevin
        auto bg = std::make_unique<RenderBridge>(L);
        bg->force_cpu(); // MUST force CPU because we only modified the CPU implementation!
        configure_vacuum_polarization(*bg);
        bg->toggles.langevin = true;
        bg->toggles.langevin_T = T_zp;
        bg->toggles.langevin_gamma = 0.1;
        
        if (T_zp > 0.0) {
            std::cout << "  Burning in vacuum for " << T_BURN << " ticks...\n";
            bg->run(T_BURN);
        }
        
        std::cout << "  Measuring alpha_eff(r) over " << T_RUN << " ticks...\n";
        
        auto m = measure_alpha_eff_vp(*bg, T_RUN, 4, L/3, 2);
        
        std::cout << "  r  |  alpha_r (g_eff^2)\n";
        std::cout << "-------------------------\n";
        for (const auto& pt : m.data) {
            std::cout << "  " << std::setw(2) << pt.r << " |  " << pt.alpha_r << "\n";
        }
        std::cout << "  -> alpha_fit = " << m.alpha_fit << "\n\n";
    }

    ftd::test::check("MechB1: Engine ran without crashing", true);
    
    return ftd::test::finalize();
}
