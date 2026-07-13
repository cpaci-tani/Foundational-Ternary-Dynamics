#pragma once
/**
 * ftd/constructors.h — lattice constructor library
 *
 * Named factory functions that stamp FTD theoretical entities onto a
 * RenderBridge's voxel grid. Every entry point returns a StampResult
 * describing exactly which voxels were modified, so tests and composite
 * constructors can validate and combine results uniformly.
 *
 * Catalog (Levels 0, 1A, and 2):
 *
 *   Constructor           Level  Sites  Theory reference
 *   --------------------  -----  -----  -----------------------------------------
 *   flux                  0      1      ontic.h (flux primitive)
 *   particle              0      1      DERIV_SPIN_STATISTICS_BRIDGE.md
 *   wavepacket            0      N      render_bridge::inject_wavepacket (Phase 6)
 *   entangled_pair        0      2      render_bridge::create_entangled_pair
 *   octahedron            1A     6      THEOREM_MOORE_LAYER_DECOMPOSITION §shell 1
 *   cuboctahedron         1A     12     THEOREM_MOORE_LAYER_DECOMPOSITION §shell 2
 *   stella_octangula      1A     8      THEOREM_MOORE_LAYER_DECOMPOSITION §shell 3
 *                                       + DERIV_BCC_MULTIPLICATIVE_STRUCTURE.md
 *   moore_cell            1A     26     THEOREM_MOORE_LAYER_DECOMPOSITION
 *   plane_wave            2      N³     EM wave (flux + wave_vel propagating)
 *   standing_wave         2      N³     Counter-propagating superposition
 *   uniform_e             2      N³     Constant electric field (wave_vel = -E)
 *   uniform_b             2      N³     Constant magnetic field (∇×J = B)
 *   photon_pulse          2      ~σ³    Gaussian-enveloped plane wave
 *   electric_dipole       2      N³     ±1 charges + Coulomb dressing
 *   magnetic_dipole       2      ~R     Current-loop analog
 *   vortex_line           2      N³     Azimuthal flux vortex
 *
 *   electron             3      ~σ³    m_e initial condition (Gaussian inward flux)
 *   positron             3      ~σ³    antimatter partner (Gaussian outward flux)
 *   neutrino             3      ~σ³    chirality seed (flux_L/flux_R asymmetry)
 *   quark                3      ~σ³    colored parton (charge + color + flux)
 *   antiquark            3      ~σ³    antimatter quark (reversed charge + flux)
 *   pion                 4      2×L3   quark-antiquark meson
 *   proton               4      3×L3   uud color-singlet baryon
 *   neutron              4      3×L3   udd color-singlet baryon
 *   hydrogen             5      L4+L3  proton + orbital electron
 *   helium               5      nuc+2L3  alpha nucleus + 2 electrons
 *   h2_molecule          5      2×L5   covalent H-H bond
 *   wilson_loop          6      ~8R    rectangular closed flux circuit
 *   flux_tube            6      ~L     color flux tube between quarks
 *   monopole             6      ~R³    magnetic monopole hedgehog seed
 *   instanton            6      ~R³    localized BPST-like energy lump
 *   schwarzschild        7      ~R³    latency gravitational well
 *   frw_patch            7      N³     uniform cosmological density patch
 *   gravitational_wave   7      N³     periodic latency modulation (GW)
 *   sloop                8      ~12    self-referential causal loop (observer)
 *   observer_cell        8      27     3³ Moore observer cell (alternating polarity)
 *
 * Design spec: docs/superpowers/specs/2026-04-15-ftd-constructors-design.md
 */

#include "lattice.h"    // Coord
#include "voxel.h"      // Vec3
#include "constants.h"  // K_B, GAUSSIAN_CUTOFF_SIGMA

#include <cstdint>
#include <vector>

namespace ftd {

class RenderBridge;  // forward declaration

namespace ctor {

struct StampResult {
    const char*      name;
    int              level;
    Coord            center;
    std::vector<int> sites;

    int site_count() const { return static_cast<int>(sites.size()); }
};

// Level 0 — primitive wrappers
/**
 * @brief Inject baseline continuous flux at a coordinate.
 *
 * @param rb Reference to the running render bridge.
 * @param at Coordinate to modify.
 * @param J 3D flux vector to inject.
 * @return StampResult identifying the injected site.
 */
StampResult flux(RenderBridge& rb, Coord at, Vec3 J);

/**
 * @brief Instantiate a discrete point-particle with flux profile.
 * 
 * [AXIOM] Particles are localized structures defined by their ternary state, 
 * baseline flux, and phase indicators (spin/color).
 *
 * @param rb Reference to the running render bridge.
 * @param at Coordinate to manifest at.
 * @param state Ternary ontic state (-1, 0, +1).
 * @param J Core flux vector representation.
 * @param spin Spin topology identifier.
 * @param color Quantized color phase identifier.
 * @return StampResult identifying the manifestation site.
 */
StampResult particle(RenderBridge& rb,
                     Coord  at,
                     int8_t state,
                     Vec3   J,
                     int8_t spin  = 0,
                     int8_t color = 0);

/**
 * @brief Initialize a Gaussian wavepacket disturbance.
 * 
 * @param rb Reference to the running render bridge.
 * @param at Center coordinate of the wavepacket.
 * @param state Center ternary state signature.
 * @param sigma Gaussian spread value.
 * @param amp Central amplitude.
 * @return StampResult tracking all affected region sites.
 */
StampResult wavepacket(RenderBridge& rb,
                       Coord  at,
                       int8_t state,
                       double sigma = 3.0,
                       double amp   = K_B);

/**
 * @brief Initialize a paired set of wave-particles representing an entanglement.
 * 
 * [EMERGENT] Entangled pairs are bound by shared wave_vel resonance vectors.
 * 
 * @param rb Reference to the running render bridge.
 * @param at Center coordinate separating the pair.
 * @param J Momentum injection vector.
 * @return StampResult covering the dual origin sites.
 */
StampResult entangled_pair(RenderBridge& rb, Coord at, Vec3 J);

// Level 1A — Moore polyhedral seeds (state-only; flux left zero)

/**
 * @brief Stamps an Octahedral structural seed (6 face neighbors).
 * [THEOREM] Corresponds to the inner shell of the Moore decomposition.
 */
StampResult octahedron(RenderBridge& rb, Coord center, int8_t state);

/**
 * @brief Stamps a Cuboctahedron structural seed (12 edge neighbors).
 * [THEOREM] Corresponds to the secondary shell of the Moore decomposition.
 */
StampResult cuboctahedron(RenderBridge& rb, Coord center, int8_t state);

/**
 * @brief Stamps a Stella Octangula structural seed (8 corner neighbors).
 * [THEOREM] Corresponds to the tertiary BCC shell of the Moore decomposition.
 */
StampResult stella_octangula(RenderBridge& rb, Coord center, int8_t state);

/**
 * @brief Completes a full 26-neighbor Moore Cell structure.
 */
StampResult moore_cell(RenderBridge& rb, Coord center, int8_t state);

// Level 2 — field configurations (stamp flux and/or wave_vel)

/**
 * @brief Injects an infinite plane wave spanning the lattice.
 */
StampResult plane_wave(RenderBridge& rb,
                       Vec3 direction,
                       Vec3 polarization,
                       double wavelength,
                       double amplitude);

/**
 * @brief Constructs a stationary wave pattern from counter-propagating plane waves.
 */
StampResult standing_wave(RenderBridge& rb,
                          Vec3 direction,
                          Vec3 polarization,
                          double wavelength,
                          double amplitude);

/**
 * @brief Embeds a uniform longitudinal field configuration (Electric analogue).
 */
StampResult uniform_e(RenderBridge& rb, Vec3 E);

/**
 * @brief Embeds a uniform transverse field configuration (Magnetic analogue).
 */
StampResult uniform_b(RenderBridge& rb, Vec3 B);

/**
 * @brief Constructs a Gaussian-enveloped directional photon pulse.
 */
StampResult photon_pulse(RenderBridge& rb,
                         Coord center,
                         Vec3 direction,
                         Vec3 polarization,
                         double sigma,
                         double amplitude);

/**
 * @brief Constructs a classical ±1 electric dipole and associated Coulomb envelope.
 */
StampResult electric_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 axis,
                            int separation);

/**
 * @brief Constructs a magnetic dipole (current-loop analogue).
 */
StampResult magnetic_dipole(RenderBridge& rb,
                            Coord center,
                            Vec3 moment,
                            int radius,
                            double amplitude);

/**
 * @brief Seeds an azimuthal flux vortex around a directional axis.
 */
StampResult vortex_line(RenderBridge& rb,
                        Coord center,
                        Vec3 axis,
                        double circulation);

// Level 3 — elementary particles (state + spin + color + flux envelope)
// [SELECTION] These are initial conditions structurally consistent with
// theory, not first-principles derivations. Mass does NOT encode spatial
// structure (SPEC_FTD.md §Layer 6).

/// Electron: state=-1, color=0, radial-inward Gaussian flux envelope.
/// Theory: DERIV_SPIN_STATISTICS_BRIDGE.md, K_B = 0.511 (m_e).
StampResult electron(RenderBridge& rb, Coord center, int8_t spin = -1);

/// Positron: state=+1, color=0, radial-outward Gaussian flux envelope.
/// Theory: antimatter partner of electron (CPT conjugate).
StampResult positron(RenderBridge& rb, Coord center, int8_t spin = +1);

/// Neutrino: state=0, chirality seed via flux_L/flux_R asymmetry.
/// Theory: DERIV_SPIN_STATISTICS_BRIDGE.md §chirality.
StampResult neutrino(RenderBridge& rb, Coord center, int8_t chirality = -1);

/// Quark: colored parton with charge (+1=up, -1=down), color (1/2/3).
/// Theory: THEOREM_MOORE_LAYER_DECOMPOSITION §color.
StampResult quark(RenderBridge& rb, Coord center,
                  int8_t charge, int8_t color, int8_t spin = +1);

/// Antiquark: antimatter quark (state = -charge, reversed flux).
/// Theory: CPT conjugate of quark.
StampResult antiquark(RenderBridge& rb, Coord center,
                      int8_t charge, int8_t color, int8_t spin = -1);

// Level 4 — composite particles (compose Level 3 constructors)
// [SELECTION] Quark content and geometry are conventional SM assignments.

/// Pion (pi+): quark-antiquark meson (q + qbar at ±separation/2 along x).
/// Theory: simplest color-singlet meson.
StampResult pion(RenderBridge& rb, Coord center, int separation = 3);

/// Proton: uud color-singlet baryon on equilateral triangle.
/// Theory: DERIV_NC_FROM_TOPOLOGY.md, proton mass derivation.
StampResult proton(RenderBridge& rb, Coord center, int radius = 2);

/// Neutron: udd color-singlet baryon on equilateral triangle.
/// Theory: same geometry as proton, different quark content.
StampResult neutron(RenderBridge& rb, Coord center, int radius = 2);

// Level 5 — atoms & molecules (compose Level 4 + Level 3)
// [SELECTION] Orbital geometry uses lattice-scale Bohr radii.

/// Hydrogen: proton at center + electron at orbital_radius along z.
/// Theory: simplest atom; Bohr radius ~ orbital_radius lattice units.
StampResult hydrogen(RenderBridge& rb, Coord center, int orbital_radius = 5);

/// Helium: nucleus (charge +2) + 2 electrons at ±orbital_radius along z.
/// Theory: He-4 nucleus simplified to single charged site.
StampResult helium(RenderBridge& rb, Coord center, int orbital_radius = 4);

/// H2 molecule: two hydrogen atoms separated by bond_length along x.
/// Theory: covalent bond = shared electron density.
StampResult h2_molecule(RenderBridge& rb, Coord center,
                        int bond_length = 4, int orbital_radius = 5);

// Level 6 — gauge/topological objects (flux circuits + monopoles)
// [SELECTION] These are topological seeds; dynamics must confirm stability.

/// Wilson loop: rectangular closed flux circuit in xy-plane.
/// Theory: area-law confinement at x_- — LEDGER FTD-0025
/// (the benchmark_wilson_loops.cpp battery was deleted 2026-05-03).
StampResult wilson_loop(RenderBridge& rb, Coord center,
                        int radius = 4, double flux_strength = K_B);

/// Flux tube: color flux tube between two quarks.
/// Theory: linear confinement E(r) ~ sigma*r — LEDGER FTD-0025
/// (the campaign_gluon_dynamics.cpp battery was deleted 2026-05-03).
StampResult flux_tube(RenderBridge& rb, Coord end_a, Coord end_b,
                      double strength = K_B);

/// Monopole: magnetic monopole seed (radial hedgehog flux pattern).
/// Theory: topological defect; Dirac string approach with tangential J.
StampResult monopole(RenderBridge& rb, Coord center, double charge = 1.0);

/// Instanton: localized BPST-like self-dual flux concentration.
/// Theory: gauge tunneling event; profile ~ size/(r^2 + size^2).
StampResult instanton(RenderBridge& rb, Coord center, double size = 3.0);

// Level 7 — gravity/cosmology (latency field configurations)
// [SELECTION] Latency = gravitational potential; dynamics via Poisson solver.

/// Schwarzschild: latency gravitational well around a point mass.
/// Theory: test_einstein_equations.cpp, benchmark_black_hole_thermo.cpp.
StampResult schwarzschild(RenderBridge& rb, Coord center, double r_s = 3.0);

/// FRW patch: uniform-density cosmological patch.
/// Theory: Friedmann-Robertson-Walker homogeneous expansion seed.
StampResult frw_patch(RenderBridge& rb, double density = 0.01);

/// Gravitational wave: periodic latency modulation.
/// Theory: linearized GW in latency formalism.
StampResult gravitational_wave(RenderBridge& rb, Vec3 direction,
                               double wavelength = 8.0,
                               double amplitude = 0.05);

// Level 8 — reference frame context/observer (self-referential structures)
// [CONJECTURE] These encode the observer formalism; interpretation is open.

/// sLoop: self-referential causal loop (ring of particles with circulating flux).
/// Theory: FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md §sLoop.
StampResult sloop(RenderBridge& rb, Coord center, int radius = 3);

/// Observer cell: 3^3 = 27-site Moore cell with alternating polarity.
/// Theory: FOUND_POTENTIAL_CORE_AND_GENERATIVE_INTERIOR.md §observer.
StampResult observer_cell(RenderBridge& rb, Coord center);

}  // namespace ctor
}  // namespace ftd
