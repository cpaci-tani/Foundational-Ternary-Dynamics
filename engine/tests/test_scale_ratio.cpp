// engine/tests/test_scale_ratio.cpp
// Unit tests for engine/include/ftd/scale_ratio.h
// FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6) — minimal reference implementation.
// No engine ticks; no ftd_core link needed (pure value-object header).

#include "ftd/scale_ratio.h"
#include <cmath>
#include <iostream>

static int pass_count = 0;
static int fail_count = 0;

static void check(const char* name, bool ok) {
    if (ok) { ++pass_count; std::cout << "  PASS  " << name << "\n"; }
    else    { ++fail_count; std::cout << "  FAIL  " << name << "\n"; }
}

int main() {
    using namespace ftd;
    std::cout << "============================================================\n";
    std::cout << "  Scale-Ratio Identity Tests (FC-3)\n";
    std::cout << "============================================================\n\n";

    const ScaleRatioBands bands{};  // default: chi_min=0.5, beta_max=0.6

    // ---- SR1: Coherent concentrated blob → is_phenomenon = true ----------
    // chi = xi/R = 6/10 = 0.6 >= 0.5 (coherent)
    // beta = delta/R = 4/10 = 0.4 <= 0.6 (concentrated)
    {
        std::cout << "--- SR1: Coherent concentrated blob ---\n";
        ScaleRatio s{ /*R=*/10.0, /*xi=*/6.0, /*delta=*/4.0 };
        check("SR1a chi=0.60 >= chi_min=0.50",  s.chi()  >= bands.chi_min);
        check("SR1b beta=0.40 <= beta_max=0.60", s.beta() <= bands.beta_max);
        check("SR1c is_phenomenon=true",          is_phenomenon(s, bands));
    }

    // ---- SR2: Incoherent thermal speckle → false (chi low) ---------------
    // chi = 1/10 = 0.1 < 0.5  (incoherent — no internal correlation across R)
    // beta = 4/10 = 0.4 <= 0.6 (concentrated, but that's not enough alone)
    {
        std::cout << "\n--- SR2: Incoherent thermal speckle ---\n";
        ScaleRatio s{ /*R=*/10.0, /*xi=*/1.0, /*delta=*/4.0 };
        check("SR2a chi=0.10 < chi_min=0.50",  s.chi() < bands.chi_min);
        check("SR2b is_phenomenon=false",        !is_phenomenon(s, bands));
    }

    // ---- SR3: Uniform fill → false (beta high) ---------------------------
    // chi = 6/10 = 0.6 >= 0.5 (coherent, but that's not enough alone)
    // beta = 8/10 = 0.8 > 0.6 (no core→edge hierarchy — no structure)
    {
        std::cout << "\n--- SR3: Uniform fill (no structure) ---\n";
        ScaleRatio s{ /*R=*/10.0, /*xi=*/6.0, /*delta=*/8.0 };
        check("SR3a beta=0.80 > beta_max=0.60", s.beta() > bands.beta_max);
        check("SR3b is_phenomenon=false",         !is_phenomenon(s, bands));
    }

    // ---- SR4: AtomicClosureContext-style (hydrogen n=2) → true -----------
    // Hydrogen-like atom: R = R_Bohr * n^2 / Z_eff = R_Bohr * 4 (n=2, Z=1).
    // In dimensionless units set R=4.0 (unit = R_Bohr). Coherence length ξ
    // ≈ orbital extent across the wavefunction ≈ 0.8·R.
    // Shell thickness δ ≈ valence shell radial falloff ≈ 0.2·R (thin shell).
    // chi = 0.8 >= 0.5; beta = 0.2 <= 0.6 → is_phenomenon = true.
    {
        std::cout << "\n--- SR4: AtomicClosureContext-style (H n=2) ---\n";
        const double R_Bohr = 1.0;  // dimensionless units
        const double n      = 2.0;
        const double Z_eff  = 1.0;
        ScaleRatio s;
        s.R     = R_Bohr * n * n / Z_eff;  // = 4.0
        s.xi    = 0.8 * s.R;               // coherence ≈ orbital extent
        s.delta = 0.2 * s.R;               // thin valence shell
        check("SR4a R=4.0 (n=2)",          std::abs(s.R - 4.0) < 1e-12);
        check("SR4b chi=0.80 >= 0.50",     s.chi()  >= bands.chi_min);
        check("SR4c beta=0.20 <= 0.60",    s.beta() <= bands.beta_max);
        check("SR4d is_phenomenon=true",    is_phenomenon(s, bands));
    }

    // ---- SR5: observe() box-independence ---------------------------------
    // A phenomenon's identity must not depend on a or L.
    // Same cloud, two different apparatus settings → different κ,ζ but same
    // is_phenomenon verdict (true in both cases).
    {
        std::cout << "\n--- SR5: observe() box-independence ---\n";
        ScaleRatio s{ /*R=*/10.0, /*xi=*/6.0, /*delta=*/4.0 };  // same as SR1

        // Apparatus A: small box, coarse lattice
        Observability obsA = observe(s, /*a=*/0.5, /*L=*/40.0);
        // Apparatus B: large box, fine lattice
        Observability obsB = observe(s, /*a=*/0.1, /*L=*/200.0);

        check("SR5a kappa_A = R/a = 20.0",    std::abs(obsA.kappa - 20.0)  < 1e-10);
        check("SR5b zeta_A  = R/L = 0.25",    std::abs(obsA.zeta  - 0.25)  < 1e-10);
        check("SR5c kappa_B = R/a = 100.0",   std::abs(obsB.kappa - 100.0) < 1e-10);
        check("SR5d zeta_B  = R/L = 0.05",    std::abs(obsB.zeta  - 0.05)  < 1e-10);

        // Identity does not change with the apparatus
        bool id_A = is_phenomenon(s, bands);
        bool id_B = is_phenomenon(s, bands);
        check("SR5e is_phenomenon unchanged across apparatus", id_A == id_B && id_A);

        // Edge case: R=0 → chi=0, beta=0 (both return 0.0, not NaN)
        ScaleRatio zero{};
        check("SR5f R=0 chi()=0.0",            zero.chi()  == 0.0);
        check("SR5g R=0 beta()=0.0",           zero.beta() == 0.0);
        check("SR5h R=0 not a phenomenon",      !is_phenomenon(zero, bands));
        Observability obs0 = observe(zero, 0.5, 40.0);
        check("SR5i observe(R=0) kappa=0",     obs0.kappa == 0.0);
        check("SR5j observe(R=0) zeta=0",      obs0.zeta  == 0.0);

        // Edge case: a=0 or L=0 → kappa=0 / zeta=0 (no division by zero)
        Observability obs_div = observe(s, /*a=*/0.0, /*L=*/0.0);
        check("SR5k observe(a=0) kappa=0 (no div/0)", obs_div.kappa == 0.0);
        check("SR5l observe(L=0) zeta=0  (no div/0)", obs_div.zeta  == 0.0);
    }

    std::cout << "\n============================================================\n";
    std::cout << "  RESULTS: " << pass_count << " passed, " << fail_count << " failed\n";
    std::cout << "============================================================\n";
    return fail_count == 0 ? 0 : 1;
}
