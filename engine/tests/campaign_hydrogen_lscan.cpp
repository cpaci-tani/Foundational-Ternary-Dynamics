/**
 * Campaign: Hydrogen L-scan — does s0-seed-hydrogen stay a stable atom, or
 *           does it flood/condense the periodic box, and is the onset
 *           L-dependent? (diagnostic for the 2026-06-16 L=49 flooding video)
 *
 * Setup: the CANONICAL s0-seed-hydrogen scenario (locked uud proton triad at
 *        center + dressed electron at amplitude K_B, oR=max(4,N/6) above it),
 *        run under DEFAULT toggles (genesis ON, the full-physics Scale-0 stack)
 *        — i.e. exactly what the web dashboard runs when you load the scenario
 *        and press play. CPU single-substrate genesis path (matches WASM).
 *
 * Question: a real H atom is STABLE. If FTD's hydrogen floods the box, that is
 *           a failure. Two readings to distinguish:
 *     (a) known L>33 scenario/numerical regression — flood onset tracks L=33,
 *         coulomb_pe wrong sign (positive), energy NOT conserved.
 *     (b) genuine vacuum-ignition — static nucleus pumps the box to the
 *         FTD-0272 condensation transition; onset ~L-independent in ticks,
 *         energy accounted for.
 *
 * Observables per checkpoint: manifested count + active fraction (manifested/L^3,
 *   the flooding order parameter), field_energy (½Σ|J|²; flux runaway),
 *   total_energy (conservation), coulomb_pe (the known-regression SIGN),
 *   charge_total (conservation), gauss_violation (projection health).
 *
 * Observation-only. No promotions. Golden gate untouched (new TU, default off-path).
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include "ftd/render_bridge.h"
#include "ftd/scenarios.h"
#include "ftd/constants.h"

using namespace ftd;

int main(int argc, char** argv) {
    // argv tokens (any order):
    //   nogenesis          → disable the genesis sector (control)
    //   dispersal/reflective/periodic → flux boundary mode (default periodic)
    bool nogenesis = false;
    FluxBoundaryMode bmode = FluxBoundaryMode::Periodic;
    const char* bname = "periodic";
    for (int a = 1; a < argc; ++a) {
        std::string t = argv[a];
        if (t == "nogenesis")  nogenesis = true;
        else if (t == "dispersal")  { bmode = FluxBoundaryMode::Dispersal;  bname = "dispersal"; }
        else if (t == "reflective") { bmode = FluxBoundaryMode::Reflective; bname = "reflective"; }
        else if (t == "periodic")   { bmode = FluxBoundaryMode::Periodic;   bname = "periodic"; }
    }
    const std::vector<int> Ls = {17, 25, 33, 41, 49};
    const int    MAX_TICKS  = 1500;
    const int    CHUNK      = 25;
    const double FLOOD_FRAC = 0.50;   // manifested/L^3 > 0.5 ⇒ box flooded
    const int    TAIL_TICKS = 100;    // run this far past onset, then stop

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "================================================================\n";
    std::cout << "  CAMPAIGN: Hydrogen L-scan (s0-seed-hydrogen, default toggles)\n";
    std::cout << "  genesis=" << (nogenesis ? "OFF" : "ON")
              << "  flux_boundary=" << bname << "\n";
    std::cout << "================================================================\n";

    std::cout << "\nL  flooded  onset_tick  frac_final  Emax/E0  fieldEmax/fieldE0  coulPE_init  Qok\n";
    std::vector<std::string> summary;

    for (int L : Ls) {
        RenderBridge rb(L);   // fresh => default toggles, empty lattice
        if (!dispatch_scenario(rb, "s0-seed-hydrogen")) {
            std::cout << "  DISPATCH FAILED for L=" << L << "\n";
            continue;
        }
        if (nogenesis) rb.toggles.genesis = false;   // control: disable manifestation sector
        rb.toggles.flux_boundary = bmode;            // flux boundary law under test
        const long long N3 = (long long)L * L * L;
        const int oR = std::max(4, L / 6);

        auto a0 = rb.energy_audit();
        const double E0     = a0.total_energy;
        const double fE0    = a0.field_energy;
        const int    q0     = a0.charge_total;

        std::cout << "\n=== L=" << L << "  (N^3=" << N3 << ", electron offset oR=" << oR << ") ===\n";
        std::cout << "  init: manif=" << a0.manifested_count
                  << "  frac=" << (double)a0.manifested_count / N3
                  << "  field_E=" << fE0
                  << "  total_E=" << E0
                  << "  coulombPE=" << a0.coulomb_pe
                  << "  Q=" << q0 << "\n";

        bool   flooded   = false;
        int    onset     = -1;
        double Emax      = E0;
        double fEmax     = fE0;
        double frac_fin  = (double)a0.manifested_count / N3;
        int    q_fin     = q0;
        double pe_min    = a0.coulomb_pe;   // most-bound (most negative) coulomb PE seen
        double pe_max    = a0.coulomb_pe;

        for (int t = CHUNK; t <= MAX_TICKS; t += CHUNK) {
            rb.run(CHUNK);
            auto a = rb.energy_audit();
            const double frac = (double)a.manifested_count / N3;
            Emax  = std::max(Emax,  a.total_energy);
            fEmax = std::max(fEmax, a.field_energy);
            pe_min = std::min(pe_min, a.coulomb_pe);
            pe_max = std::max(pe_max, a.coulomb_pe);
            frac_fin = frac;
            q_fin = a.charge_total;

            std::cout << "  t=" << t
                      << "  manif=" << a.manifested_count
                      << "  frac=" << frac
                      << "  field_E=" << a.field_energy
                      << "  total_E=" << a.total_energy
                      << "  coulPE=" << a.coulomb_pe
                      << "  Q=" << a.charge_total
                      << "  gauss=" << a.gauss_violation
                      << std::endl;   // flush for live streaming

            if (!flooded && frac > FLOOD_FRAC) {
                flooded = true;
                onset = t;
            }
            if (flooded && t >= onset + TAIL_TICKS) break;
        }

        std::cout << "  >>> L=" << L << "  " << (flooded ? "FLOODED" : "stable/contained")
                  << "  onset_tick=" << onset
                  << "  frac_final=" << frac_fin
                  << "  Emax/E0=" << (E0 != 0.0 ? Emax / E0 : 0.0)
                  << "  fieldEmax/fieldE0=" << (fE0 != 0.0 ? fEmax / fE0 : 0.0)
                  << "  coulPE[min,max]=[" << pe_min << "," << pe_max << "]"
                  << "  Q:" << q0 << "->" << q_fin
                  << "\n";

        char buf[256];
        std::snprintf(buf, sizeof(buf),
            "L=%-3d %-8s onset=%-6d frac=%.3f  Emax/E0=%.2f  fEmax/fE0=%.2f  coulPE0=%+.4f  Q%s",
            L, flooded ? "FLOOD" : "stable", onset, frac_fin,
            E0 != 0 ? Emax / E0 : 0.0, fE0 != 0 ? fEmax / fE0 : 0.0,
            a0.coulomb_pe, (q0 == q_fin) ? "ok" : "BROKEN");
        summary.emplace_back(buf);
    }

    std::cout << "\n================================================================\n";
    std::cout << "  SUMMARY\n";
    std::cout << "================================================================\n";
    for (auto& s : summary) std::cout << "  " << s << "\n";
    return 0;
}
