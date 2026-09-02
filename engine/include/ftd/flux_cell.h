#pragma once
// ==========================================================================
//  engine/include/ftd/flux_cell.h
//
//  Flux-cell (field reservoir) constructors, membrane / pump / port
//  mechanisms, and the regional storage ledger behind the s0-cell-* Scale-0
//  scenarios (src/scenarios/cell.cpp).
//
//  A flux cell is a localized finite field configuration whose energy stays
//  above the vacuum value after its external pump is disconnected. The
//  scenario bodies seed the initial data; this header exposes the reusable
//  pieces so tests and the dashboard can
//    (a) charge a cell dynamically (pump for N ticks, then disconnect),
//    (b) hold it behind a membrane and open a port on schedule, and
//    (c) measure the regional ledger that decides whether anything was
//        stored, held, or lost:
//
//     U_E    = ½ Σ |E|²,        E = -wave_vel     (electric channel)
//     U_B    = ½ c² Σ |B|²,     B = curl J        (magnetic channel)
//     U_J    = ½ Σ |J|²                           (flux-potential channel;
//                                                  NOT the electric energy)
//     H_wave = ½Σ|W|² + ½c²Σ W·L18(J) − ½c²Σ J·L18(J)
//                                                 (kick-drift conserved
//                                                  Hamiltonian, periodic
//                                                  18-point Laplacian)
//     P_leak = Σ_{region faces} S·n̂,  S = c²(E×B) (Poynting flux leaving
//                                                  the region)
//
//  ── Physics of the three mechanisms ─────────────────────────────────────
//
//  MEMBRANE.  The engine's de Broglie clock (FTD-0271, [IMPOSED]) adds the
//  Klein–Gordon mass term −ω₀²·J at every manifested site, so the field
//  obeys ω² = c²λ(k) + ω₀² there and ω² = c²λ(k) in the void.  A shell of
//  locked manifested sites is therefore a region of mass gap ω₀: every mode
//  with ω < ω₀ is evanescent inside it, decaying per voxel by
//  κ = acosh(1 + (ω₀² − ω²)/(2c²)) ≈ 1.6 at ω ≪ ω₀ = 1, exactly as a metal
//  reflects light below its plasma frequency or a waveguide rejects modes
//  below cutoff.  Nothing new is added to the dynamics: the membrane is the
//  existing clock term evaluated on an imposed shell, and its falsifiers are
//  quantitative — retention must fall toward the bare-map uniform-fill value
//  when ω₀ → 0 (transparent control) and leakage must fall with shell
//  thickness (evanescent scaling).  The shell is initial data, not a wall
//  primitive; locked marker sheets WITHOUT the clock term are already a
//  closed negative for confinement (quantum-well).
//
//  PUMP.  A time-gated source term in the flux wave equation,
//      J̈ = c²∇²J + s(x)·g(t),   g(t) = 1 for 0 ≤ t < N_pump, 0 afterwards,
//  applied before phase_read like the existing uniform drive
//  (ew_background_sweep).  The work delivered per tick is the exact change of
//  the kick-drift Hamiltonian under J → J + δ at fixed W, which by
//  bilinearity of H and symmetry of L18 on the periodic lattice is
//      ΔH = ½c²[ W·Lδ − 2 J·Lδ − δ·Lδ ],
//  an O(|support|) sum (Lδ vanishes outside the support dilated by one
//  Moore shell).  W_in = Σ ΔH is therefore booked exactly, with no O(N³)
//  pass, and disconnection (g = 0) is a hard switch: after the last
//  increment the engine injects nothing.  Increments may be spaced by a
//  period P (one every P ticks).  Because ΔH contains −2J·Lδ, an increment
//  delivers the most work when the cell's field is in phase with it, so
//  spacing the increments by the cell's breathing period is ordinary
//  resonant driving of an LC-like reservoir: the same increments then add
//  coherently (stored energy ∝ N²) instead of incoherently (∝ N).  The formula is exact for the periodic operator;
//  under Reflective or Dispersal boundary laws it is exact whenever the pump
//  support keeps two cells away from the faces (the stencil never touches
//  the shell), which every shipped profile does.
//
//  PORT.  A state-controlled opening: at a scheduled tick the shell sites
//  inside a small ball around a point of the membrane expire (state → 0,
//  identity cleared, flux left in place — the P5 non-injective expiry
//  bookkeeping used by evaporation).  The mass gap vanishes there, so the
//  hole is an aperture in an otherwise closed wall.  The energy leaving
//  through it is integrated every tick after opening over a one-layer
//  surface of the opened plug (the slab |d·n̂ − surface_offset| ≤ ½):
//      W_out = Σ_ticks Σ_{i ∈ port surface} S^H_i·n̂,
//      S^H   = c² Σ_a E_a ∇J_a,   E = −W.
//  S^H is the exact energy current of the component-wise vector wave
//  equation the kick-drift map integrates (three decoupled scalar wave
//  equations, each with density ½|∂_t φ|² + ½c²|∇φ|² and current
//  −c² ∂_t φ ∇φ), so ∂_t H_wave + ∇·S^H = 0 site by site up to the
//  centred-difference discretization.  The EM-like Poynting vector c²E×B
//  differs from S^H by the curl-type term c²(E·∇)J and is kept as a second,
//  observer-level integral.  The ledger the test closes is
//  ΔH_wave(inside) + W_out ≈ 0.
//
//  Every quantity is an observer-level diagnostic in lattice units.  Nothing
//  here asserts a capacitor, inductor, battery, or particle identity; the
//  scenario validation text carries the measured status.
// ==========================================================================

#include "ftd/voxel.h"  // Vec3

#include <vector>

namespace ftd {

class RenderBridge;

// ── Toroidal circulating-flux profile ───────────────────────────────────
//
//   rho   = sqrt(dx² + dy²)             (periodic deltas from the centre)
//   d_T   = sqrt((rho − R)² + dz²)      (distance to the ring centreline)
//   f     = exp(−d_T² / (2 σ²))
//   J     = A · f · s(φ) · φ̂,           φ̂ = (−dy, dx, 0) / rho
//
// s(φ) = circulation_sign when sign_sectors == 0 (coherent ring), otherwise
// circulation_sign · sign(cos(sign_sectors · φ)) — the phase-scrambled
// control with identical pointwise |J| and zero net circulation.
struct FluxCellTorusSpec {
    double cx = 0.0, cy = 0.0, cz = 0.0;  // ring centre (lattice units)
    double major_radius = 4.0;            // R
    double tube_sigma = 1.5;              // σ
    double amplitude = 0.3;               // A, peak |J| on the centreline
    int circulation_sign = +1;            // +1 counter-clockwise about +z
    int sign_sectors = 0;                 // 0 coherent; m>0 alternating sectors
    double cutoff_sigmas = 4.0;           // support cut at d_T > cutoff·σ
};

// Canonical ring for a lattice of the given size (centre at the geometric
// midpoint, R = max(3, N/4), σ = max(1.25, N/16), A = 0.3).
FluxCellTorusSpec default_flux_cell_torus_spec(int lattice_size);

// Additive seed: flux += scale · J_spec at every site inside the cutoff.
// wave_vel is untouched. scale < 1 lets a caller apply the profile as a
// per-tick pump; the scenario bodies call it once with scale = 1.
void seed_flux_cell_torus(RenderBridge& rb, const FluxCellTorusSpec& spec,
                          double scale = 1.0);

// ── Membrane: locked manifested shell (mass gap under the de Broglie clock) ──
//
// Sites with inner_radius ≤ r < inner_radius + thickness (periodic distance
// from the centre) become locked manifested cells of alternating polarity
// ((x+y+z) parity), so the shell is net-neutral. The shell only becomes a
// membrane when toggles.de_broglie_clock is ON (see the header comment).
struct FluxCellMembraneSpec {
    double cx = 0.0, cy = 0.0, cz = 0.0;
    double inner_radius = 8.0;
    double thickness = 3.0;
};

// Shell that fills the box (outer radius (N−1)/2 − 0.5) with the given
// thickness, and the ring that fits inside it with a one-cell margin.
FluxCellMembraneSpec default_flux_cell_membrane_spec(int lattice_size,
                                                     double thickness = 3.0);
FluxCellTorusSpec flux_cell_membrane_ring_spec(const FluxCellMembraneSpec& shell);

// Stages the shell on the host mirror (one lazy upload on GPU backends).
// Returns the number of shell sites written.
int seed_flux_cell_membrane(RenderBridge& rb, const FluxCellMembraneSpec& spec);

// ── Port: scheduled aperture in the membrane ───────────────────────────
struct FluxCellPortSpec {
    double cx = 0.0, cy = 0.0, cz = 0.0;  // hole centre (a point on the shell)
    double nx = 1.0, ny = 0.0, nz = 0.0;  // outward normal used for S·n̂
    double radius = 2.0;                  // hole radius (lattice units)
    double surface_offset = 0.0;          // signed distance along n from the hole
                                          // centre to the one-layer accounting
                                          // surface: every site (wall or void) of
                                          // the slab |d·n − offset| ≤ ½ inside the
                                          // hole (0 = through the centre; +t/2−0.5 =
                                          // a shell's outer face)
    int open_tick = -1;                   // completed-tick index at which the hole is
                                          // open (opens before that tick's dynamics); <0 never
};

// ── Pump profile: sparse increment plus its dilated support ─────────────
//
// delta holds the per-tick increment δ = J_spec / ticks on the support;
// dilated is the support grown by one Moore shell (where L18 δ ≠ 0).
struct FluxCellPumpProfile {
    std::vector<int> support;
    std::vector<int> dilated;
    std::vector<Vec3> delta;   // dense, size = total sites
    int ticks = 0;
};

FluxCellPumpProfile build_flux_cell_pump_profile(const RenderBridge& rb,
                                                 const FluxCellTorusSpec& spec,
                                                 int ticks);

// Applies one pump increment on the host mirror and returns the exact
// kick-drift Hamiltonian change ΔH = ½c²[W·Lδ − 2J·Lδ − δ·Lδ] evaluated on
// the state BEFORE the increment (periodic 18-point operator).
double apply_flux_cell_pump_increment(RenderBridge& rb,
                                      const FluxCellPumpProfile& profile);

// ── Regional storage ledger ────────────────────────────────────────────
//
// A spherical region (periodic distance) around a centre. A radius of at
// least lattice_size covers the whole box; radius ≤ 0 means "no region".
struct FluxCellRegion {
    double cx = 0.0, cy = 0.0, cz = 0.0;
    double radius = 0.0;
};

struct FluxCellLedger {
    double U_E = 0.0;            // ½ Σ |E|² over the region
    double U_B = 0.0;            // ½ c² Σ |B|² over the region
    double U_J = 0.0;            // ½ Σ |J|² over the region
    double H_wave = 0.0;         // kick-drift Hamiltonian restricted to the region
    double H_kg = 0.0;           // H_wave plus the de Broglie clock terms at manifested
                                 // sites (½ W·(−ω₀²J) + ½ ω₀²|J|²) when the clock is ON;
                                 // equal to H_wave otherwise. This is the region's share
                                 // of the Hamiltonian the membrane profile conserves.
    double P_leak = 0.0;         // Σ over region-boundary faces of S(inside)·n̂
    Vec3 S_total;                // Σ S over the region
    double S_abs_total = 0.0;    // Σ |S| over the region
    Vec3 J_total;                // Σ J over the region (net flux vector)
    double dyad[3][3] = {};      // Σ J_a J_b over the region (flux dyad)
    double support_radius = 0.0; // max periodic distance from the centre of a
                                 // lattice site (whole box) with
                                 // |J|² + |W|² > support_threshold
    int site_count = 0;          // sites inside the region
};

FluxCellLedger compute_flux_cell_ledger(const RenderBridge& rb,
                                        const FluxCellRegion& region,
                                        double support_threshold = 1e-10);

// Electric–magnetic balance coordinate (U_E − U_B) / (U_E + U_B); 0 when the
// denominator vanishes.
double flux_cell_eb_balance(const FluxCellLedger& ledger);

// Flux circulation Γ_J around a lattice circle of radius R in the plane
// z = cz about (cx, cy): Σ_k J(nearest site to point k) · φ̂_k · Δℓ over
// n_samples equally spaced points (n_samples <= 0 picks ceil(4πR)).
double flux_cell_ring_circulation(const RenderBridge& rb, double cx, double cy,
                                  double cz, double R, int n_samples = 0);

// Magnetic flux Φ_B through the spanning disk of that circle: Σ B_z over the
// sites of the nearest z-plane with periodic in-plane distance < R.
double flux_cell_disk_magnetic_flux(const RenderBridge& rb, double cx,
                                    double cy, double cz, double R);

// Poynting flux through a set of sites along a unit normal: Σ_i S_i·n̂.
double flux_cell_site_poynting_flux(const RenderBridge& rb,
                                    const std::vector<int>& sites,
                                    const Vec3& normal);

// Wave-Hamiltonian energy current through a set of sites along a unit
// normal: Σ_i c² Σ_a E_a (n̂·∇)J_a with centred differences (E = −W).
double flux_cell_site_hamiltonian_flux(const RenderBridge& rb,
                                       const std::vector<int>& sites,
                                       const Vec3& normal);

}  // namespace ftd
