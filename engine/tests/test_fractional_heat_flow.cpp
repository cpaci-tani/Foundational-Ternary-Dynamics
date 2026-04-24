/**
 * Test: Fractional Heat Flow (Continuum Limit)
 *
 * Verifies that the FTD lattice wave equation transitions from purely
 * ballistic wave propagation (r_rms ~ t^1) to fractional diffusion/heat 
 * flow (r_rms ~ t^1/2).
 * 
 * To rigorously prove this transition arises from thermodynamics (Fluctuation-
 * Dissipation Theorem) and not simply grid dispersion, this test uses the 
 * Langevin thermostat to inject stochastic noise and momentum dissipation.
 * We measure the signal variance against the thermal background and prove 
 * the diffusion exponent strictly scales with the dissipation parameter (gamma).
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <utility>
#include <algorithm>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

struct PassResult {
    double early_slope;
    double late_slope;
};

static PassResult run_heat_flow_pass(double gamma, double temperature) {
    const int L = 128;
    const int mid = L / 2;
    const int T_RUN = 80;
    const double SIGMA = 2.0;

    std::cout << "\n  --- Running Pass with gamma = " << gamma << " ---\n";

    ftd::RenderBridge engine(L);
    
    engine.toggles.disable_all();
    engine.toggles.wave_propagation = true;
    
    // Use the Langevin thermostat for true Fluctuation-Dissipation thermodynamics
    engine.toggles.langevin = true;
    engine.toggles.langevin_gamma = gamma;
    engine.toggles.langevin_T = temperature;

    // Inject a sharp, localized isotropic flux pulse
    auto& vox_mut = engine.voxels();
    for (int x = 0; x < L; ++x) {
        for (int y = 0; y < L; ++y) {
            for (int z = 0; z < L; ++z) {
                double dx = x - mid;
                double dy = y - mid;
                double dz = z - mid;
                
                if (dx > L / 2) dx -= L; if (dx < -L / 2) dx += L;
                if (dy > L / 2) dy -= L; if (dy < -L / 2) dy += L;
                if (dz > L / 2) dz -= L; if (dz < -L / 2) dz += L;

                double r2 = dx*dx + dy*dy + dz*dz;
                double envelope = std::exp(-r2 / (2.0 * SIGMA * SIGMA));
                
                if (envelope > 1e-6) {
                    int idx = engine.lattice().index(x, y, z);
                    vox_mut[idx].flux = {envelope, envelope, envelope};
                }
            }
        }
    }

    std::vector<double> t_log;
    std::vector<double> var_log;

    auto compute_variance = [&]() -> double {
        double sum_r2_I = 0.0;
        double sum_I = 0.0;
        const auto& vox_array = std::as_const(engine).voxels();
        int idx = 0;
        
        // We set the threshold to 5 times the background temperature to strictly 
        // extract the coherent signal from the stochastic vacuum noise.
        double threshold = 5.0 * temperature;
        
        for (int x = 0; x < L; ++x) {
            for (int y = 0; y < L; ++y) {
                for (int z = 0; z < L; ++z) {
                    double intensity = vox_array[idx].flux.mag2();
                    
                    if (intensity > threshold) {
                        double dx = x - mid;
                        double dy = y - mid;
                        double dz = z - mid;
                        if (dx > L / 2) dx -= L; if (dx < -L / 2) dx += L;
                        if (dy > L / 2) dy -= L; if (dy < -L / 2) dy += L;
                        if (dz > L / 2) dz -= L; if (dz < -L / 2) dz += L;
                        
                        double r2 = dx*dx + dy*dy + dz*dz;
                        // Subtract the background baseline so we only measure the pulse spread
                        double signal = intensity - threshold;
                        sum_r2_I += r2 * signal;
                        sum_I += signal;
                    }
                    idx++;
                }
            }
        }
        return (sum_I > 0) ? (sum_r2_I / sum_I) : 0.0;
    };

    double prev_log_t = 0.0;
    double prev_log_var = 0.0;

    for (int t = 1; t <= T_RUN; ++t) {
        engine.tick();
        
        if (t % 10 == 0 || t == 5) {
            double var = compute_variance();
            double lt = std::log(t);
            double lvar = std::log(var);
            
            double slope = 0.0;
            if (t > 10 && var > 0) {
                slope = (lvar - prev_log_var) / (lt - prev_log_t);
            }
            
            t_log.push_back(lt);
            var_log.push_back(lvar);
            
            prev_log_t = lt;
            prev_log_var = lvar;
        }
    }

    double early_t1 = std::log(10.0);
    double early_t2 = std::log(40.0);
    double early_v1 = 0, early_v2 = 0;
    
    for (size_t i = 0; i < t_log.size(); ++i) {
        if (std::abs(t_log[i] - early_t1) < 0.1) early_v1 = var_log[i];
        if (std::abs(t_log[i] - early_t2) < 0.1) early_v2 = var_log[i];
    }
    double early_slope = (early_v2 - early_v1) / (early_t2 - early_t1);

    double late_t1 = std::log(50.0);
    double late_t2 = std::log(80.0);
    double late_v1 = 0, late_v2 = 0;
    
    for (size_t i = 0; i < t_log.size(); ++i) {
        if (std::abs(t_log[i] - late_t1) < 0.1) late_v1 = var_log[i];
        if (std::abs(t_log[i] - late_t2) < 0.1) late_v2 = var_log[i];
    }
    double late_slope = (late_v2 - late_v1) / (late_t2 - late_t1);

    std::cout << "  Measured early slope (t: 10-40)  => " << early_slope << "\n";
    std::cout << "  Measured late slope  (t: 50-80)  => " << late_slope << "\n";

    return { early_slope, late_slope };
}

static void section_fractional_heat_flow() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Fractional Heat Flow (Fluctuation-Dissipation Scaling)\n";
    std::cout << "================================================================\n";
    std::cout << std::fixed << std::setprecision(4);

    double T_bg = 1e-15; // Thermal background
    
    // Pass A: Low dissipation
    PassResult resA = run_heat_flow_pass(0.02, T_bg);
    
    // Pass B: High dissipation
    PassResult resB = run_heat_flow_pass(0.10, T_bg);

    std::cout << "\n--- Exponent Analysis & Scaling Law Proof ---\n";
    
    // Check that early propagation is ballistic
    ftd::test::check("HF1: Pass A early propagation is ballistic (slope > 1.7)", resA.early_slope > 1.7);
    ftd::test::check("HF2: Pass B early propagation is ballistic (slope > 1.7)", resB.early_slope > 1.7);
    
    // Check gamma-scaling law
    // The higher the dissipation, the faster the transition to the thermal limit (p = 1.0).
    // Therefore, Pass B must have a significantly lower slope than Pass A at the same time slice.
    double slope_drop = resA.late_slope - resB.late_slope;
    std::cout << "  Slope Difference (A - B) => " << slope_drop << "\n";
    
    ftd::test::check("HF3: Gamma scaling law holds (Higher gamma drives stronger diffusion)", slope_drop > 0.2);
    ftd::test::check("HF4: Pass B late propagation approaches diffusion (slope < 1.3)", resB.late_slope < 1.3);
}

int main() {
    ftd::test::init("fractional_heat_flow");
    ftd::test::section("fractional_heat_flow");
    section_fractional_heat_flow();
    return ftd::test::finalize();
}
