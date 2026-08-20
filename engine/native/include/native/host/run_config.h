#pragma once
//
// host/run_config.h — scale-common run knobs (SPEC_NATIVE_REBUILD_R0R1 §4.1).
//
// A field is 0/ignored where a scale has no analogue. Scale-specific setup
// (initial conditions, catalog picks, …) rides the scale command payload, never
// this struct — so adding a scale never widens RunConfig.
//
namespace ftd::native {

struct RunConfig {
    int    lattice_size   = 0;   // Scale 0 (0 = n/a elsewhere)
    double dt             = 1.0;
    int    sor_iterations = 0;   // Scale 0 (0 = leave engine default)
    int    substeps       = 1;   // Scale 4/5 N-body substep multiplier
    bool   force_cpu      = false;
    int    flux_boundary  = 2;   // Scale 0 boundary mode (0/1/2); ignored elsewhere
};

}  // namespace ftd::native
