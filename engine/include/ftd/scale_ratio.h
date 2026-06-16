// engine/include/ftd/scale_ratio.h
#pragma once
// This module implements FC-3 (SPEC_SCALE_RATIO_ONTOLOGY.md §6).
// INVARIANT: must never reference ALPHA, Koopman, α, x_+, 1/137, or any
// QED observable — identity is determined by internal ratios alone.
namespace ftd {

// [DEFINITION] A phenomenon's three intrinsic scale features — all internal
// to the thing; no lattice spacing a or box size L.
struct ScaleRatio {
    double R     = 0.0;  // intrinsic extent (characteristic size)
    double xi    = 0.0;  // coherence length (internal correlation scale)
    double delta = 0.0;  // shell thickness (active-boundary / falloff width)

    // χ = ξ / R — coherence ratio (0 when R≤0; precondition: R≥0)
    double chi()  const { return R > 0.0 ? xi    / R : 0.0; }
    // β = δ / R — concentration ratio (0 when R≤0; precondition: R≥0)
    double beta() const { return R > 0.0 ? delta / R : 0.0; }
};

// [IMPOSED engineering defaults] — calibration deferred; must not be tuned
// to admit any specific trajectory or α candidate.
struct ScaleRatioBands {
    double chi_min  = 0.5;  // coherence floor
    double beta_max = 0.6;  // concentration ceiling
};

// Identity verdict: is this thing a phenomenon (has its own scale)?
// Box-independent by construction — a and L play no role here.
inline bool is_phenomenon(const ScaleRatio& s, const ScaleRatioBands& b) {
    return s.chi() >= b.chi_min && s.beta() <= b.beta_max;
}

// Apparatus-relative observability ratios (not identity — these are caveats
// on the measurement, never used to deny that something is a phenomenon).
struct Observability { double kappa = 0.0; double zeta = 0.0; };

// Precondition: a > 0, L > 0 (unphysical inputs silently return 0).
// kappa = R/a (grid resolution), zeta = R/L (box fraction).
inline Observability observe(const ScaleRatio& s, double a, double L) {
    return { a > 0.0 ? s.R / a : 0.0, L > 0.0 ? s.R / L : 0.0 };
}

}  // namespace ftd
