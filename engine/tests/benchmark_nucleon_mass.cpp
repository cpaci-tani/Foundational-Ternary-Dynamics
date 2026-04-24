/**
 * @file benchmark_nucleon_mass.cpp
 * @brief Dynamical Nucleon Mass Benchmark
 *
 * Tests the triad (nucleon analog) binding energy under physical 
 * fine-structure coupling limits rather than geometric limits.
 *
 * This benchmark demonstrates that by using the explicit physical 
 * coupling (coulomb_charge_coupling = sqrt(2 * PI * ALPHA)), the 
 * electrostatic repulsion is massively reduced, allowing the triad 
 * to express different binding characteristics.
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>

#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/eft/coupling_measurement.h"

int main() {
    std::cout << "================================================================\n";
    std::cout << "  DYNAMICAL NUCLEON MASS BENCHMARK\n";
    std::cout << "  Testing triad binding under physical α coupling\n";
    std::cout << "================================================================\n\n";

    const int L = 48;
    const int mid = L / 2;
    const int WARMUP = 600;

    // Physical coupling (engine convention)
    const double GC_PHYSICAL = std::sqrt(2.0 * M_PI * ftd::ALPHA);
    
    std::cout << "  Physical α = " << ftd::ALPHA << " (1/137.036)\n";
    std::cout << "  Engine GC = " << GC_PHYSICAL << "\n\n";

    // ----------------------------------------------------------------
    // Part 1: Single Particle (Electron Analog)
    // ----------------------------------------------------------------
    double E_single = 0;
    {
        ftd::RenderBridge rb(L);
        // Use EFT validation environment for clean measurements
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        
        // OVERRIDES for this specific test
        rb.toggles.genesis = false;         // Prevent evaporation/pair production
        rb.toggles.damping = true;          // Enable damping to find ground state
        rb.toggles.poisson_coulomb = false; // Use full wave equation
        rb.toggles.coulomb_charge_coupling = GC_PHYSICAL;
        
        // Inject single particle
        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;

        rb.run(WARMUP);

        auto audit = rb.energy_audit();
        E_single = audit.field_energy + audit.wave_energy;

        std::cout << "--- Single Particle (Electron Analog) ---\n";
        std::cout << "  Field energy: " << audit.field_energy << "\n";
        std::cout << "  Wave energy:  " << audit.wave_energy << "\n";
        std::cout << "  Total E_single = " << E_single << "\n\n";
    }

    // ----------------------------------------------------------------
    // Part 2: Triad (Nucleon Analog)
    // ----------------------------------------------------------------
    double E_triad = 0;
    {
        ftd::RenderBridge rb(L);
        ftd::eft::configure_bare_lattice_for_coupling(rb);
        
        // OVERRIDES
        rb.toggles.genesis = false;
        rb.toggles.damping = true;
        rb.toggles.poisson_coulomb = false;
        rb.toggles.coulomb_charge_coupling = GC_PHYSICAL;

        // Inject 3 particles in equilateral triangle at r=sqrt(2)
        rb.inject_particle(mid, mid, mid, +1, {ftd::K_B, 0, 0});
        rb.inject_particle(mid+1, mid+1, mid, +1, {ftd::K_B, 0, 0});
        rb.inject_particle(mid+1, mid, mid+1, +1, {ftd::K_B, 0, 0});

        rb.voxels()[rb.lattice().index(mid, mid, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid+1, mid+1, mid)].locked = true;
        rb.voxels()[rb.lattice().index(mid+1, mid, mid+1)].locked = true;

        rb.run(WARMUP);

        auto audit = rb.energy_audit();
        E_triad = audit.field_energy + audit.wave_energy;

        std::cout << "--- Triad (Nucleon Analog) ---\n";
        std::cout << "  Field energy: " << audit.field_energy << "\n";
        std::cout << "  Wave energy:  " << audit.wave_energy << "\n";
        std::cout << "  Total E_triad = " << E_triad << "\n\n";
    }

    // ----------------------------------------------------------------
    // Part 3: Analysis
    // ----------------------------------------------------------------
    double E_isolated = 3.0 * E_single;
    double E_bind = E_triad - E_isolated;
    double mass_ratio = E_triad / E_single;

    std::cout << "--- Emergent Mass Analysis ---\n";
    std::cout << "  3 × E_single   = " << E_isolated << "\n";
    std::cout << "  E_triad        = " << E_triad << "\n";
    std::cout << "  Binding Energy = " << E_bind << " (" 
              << (E_bind < 0 ? "NEGATIVE / BOUND" : "POSITIVE / UNBOUND") << ")\n\n";

    std::cout << "--- Mass Ratio ---\n";
    std::cout << "  Target m_p/m_e = 1836.15 (Physical)\n";
    std::cout << "  Target Constituent = 3520 (Topological/Raw)\n";
    std::cout << "  Measured Ratio = " << mass_ratio << "\n\n";

    std::cout << "================================================================\n";
    std::cout << "  Done.\n";
    std::cout << "================================================================\n";

    return 0;
}
