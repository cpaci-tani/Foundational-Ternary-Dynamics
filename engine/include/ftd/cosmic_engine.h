#pragma once
/**
 * CosmicEngine: Scale 5 simulation
 *
 * N-body + SPH cosmic simulation with Barnes-Hut octree gravity.
 * All constants from ontic.h — zero free parameters.
 *
 * Body types map to ternary ontology:
 *   negative = collapsed/accreting (BLACK_HOLE, QUASAR, DARK_ENERGY)
 *   zero     = dark/invisible      (DARK_MATTER)
 *   positive = luminous/baryonic   (GAS, STAR, NEUTRON_STAR, NEBULA, WHITE_DWARF)
 *
 * Physics pipeline (18-phase cosmic tick cycle):
 *   build_octree -> compute_gravity -> compute_sph_density -> compute_sph_forces
 *   -> apply_hubble -> apply_dark_energy -> compute_accretion -> compute_jets
 *   -> check_star_formation -> check_stellar_evolution
 *   -> compute_magnetic_fields -> compute_radiation_pressure
 *   -> detect_gw_events -> propagate_gw
 *   -> half_kick -> drift -> half_kick -> tick++
 *
 * FTD constant mapping:
 *   G_N      = 1/(b_3+N_c)^2 = 0.01          (gravity)
 *   Omega_Λ  = 2/3                             (dark energy fraction)
 *   DM frac  = 17/27 = 63%                     (Moore theorem dark states)
 *   gamma    = (D+2)/D = 5/3                   (adiabatic index)
 *   c        = 1/sqrt(3)                        (CFL speed limit)
 *   r_s      = 2*G_N*M                          (Schwarzschild radius)
 */

#include "voxel.h"         // Vec3
#include "constants.h"     // All FTD constants
#include "scale.h"         // OnticEntity, ScaleLevel
#include "scale_engine.h"  // ScaleEngine base class
#include "barnes_hut.h"    // FTD Generalized Barnes-Hut Tree
#include <vector>
#include <cstdint>
#include <cmath>
#include <array>
#include <string>

namespace ftd {

// ============================================================================
// Cosmic body types — ternary-mapped
// ============================================================================

enum class CosmicBodyType : int8_t {
    DARK_ENERGY  = -3,  // Cosmological constant field (Lambda)
    QUASAR       = -2,  // Active SMBH — brightest, relativistic jets
    BLACK_HOLE   = -1,  // Singularity — latency L -> 1, accretion
    DARK_MATTER  =  0,  // Invisible — 17/27 dark states (Moore theorem)
    GAS          =  1,  // Baryonic diffuse — SPH dynamics
    STAR         =  2,  // Main sequence — luminous
    NEUTRON_STAR =  3,  // Compact remnant — extreme density
    NEBULA       =  4,  // Star-forming region — dense gas + protostars
    WHITE_DWARF  =  5   // Degenerate remnant — slow cooling
};

inline const char* cosmic_body_type_name(CosmicBodyType t) {
    switch (t) {
        case CosmicBodyType::DARK_ENERGY:  return "DarkEnergy";
        case CosmicBodyType::QUASAR:       return "Quasar";
        case CosmicBodyType::BLACK_HOLE:   return "BlackHole";
        case CosmicBodyType::DARK_MATTER:  return "DarkMatter";
        case CosmicBodyType::GAS:          return "Gas";
        case CosmicBodyType::STAR:         return "Star";
        case CosmicBodyType::NEUTRON_STAR: return "NeutronStar";
        case CosmicBodyType::NEBULA:       return "Nebula";
        case CosmicBodyType::WHITE_DWARF:  return "WhiteDwarf";
    }
    return "Unknown";
}

// Is this body type subject to SPH hydrodynamics?
inline bool is_sph_body(CosmicBodyType t) {
    return t == CosmicBodyType::GAS || t == CosmicBodyType::NEBULA;
}

// Is this body type a compact object (BH/Quasar/NS)?
inline bool is_compact_object(CosmicBodyType t) {
    return t == CosmicBodyType::BLACK_HOLE ||
           t == CosmicBodyType::QUASAR ||
           t == CosmicBodyType::NEUTRON_STAR;
}

// Is this body type luminous (emits radiation)?
inline bool is_luminous(CosmicBodyType t) {
    return t == CosmicBodyType::STAR ||
           t == CosmicBodyType::NEUTRON_STAR ||
           t == CosmicBodyType::QUASAR ||
           t == CosmicBodyType::WHITE_DWARF ||
           t == CosmicBodyType::NEBULA;
}

// ============================================================================
// Cosmic body struct
// ============================================================================

struct CosmicBody {
    int32_t id = -1;
    CosmicBodyType type = CosmicBodyType::DARK_MATTER;
    double mass = 1.0;              // Solar masses
    double radius = 1.0;            // kpc
    Vec3 position;                  // Comoving coordinates
    Vec3 velocity;                  // Peculiar velocity
    Vec3 acceleration;
    double temperature = 0.0;       // Kelvin
    Vec3 angular_momentum;          // Spin vector
    // SPH fields (gas/nebula only)
    double smoothing_length = 1.0;  // SPH kernel radius
    double density = 0.0;           // SPH computed
    double pressure = 0.0;          // SPH computed
    double internal_energy = 0.0;   // Specific internal energy (u)
    // Compact object fields
    double accretion_rate = 0.0;    // BH/Quasar Bondi-Hoyle Mdot
    double latency = 0.0;           // L(r) from FTD metric (BH/Quasar)
    // Stellar fields
    double luminosity = 0.0;        // Bolometric luminosity
    double metallicity = 0.0;       // Z (metal fraction by mass)
    Vec3 magnetic_field;            // Cosmic B-field vector
    // Gravitational wave
    double gw_strain = 0.0;         // h (GW strain at source)

    OnticEntity as_ontic() const {
        return {static_cast<int>(type), mass, radius};
    }

    // Schwarzschild radius: r_s = 2 * G_N * M
    double schwarzschild_radius() const {
        return 2.0 * G_N * mass;
    }

    // Sound speed for SPH: c_s = sqrt(gamma * P / rho)
    // gamma = (D+2)/D = 5/3 for D=3
    double sound_speed() const {
        if (density <= 0.0) return 0.0;
        constexpr double GAMMA = (D_SPATIAL + 2.0) / D_SPATIAL; // 5/3
        return std::sqrt(GAMMA * pressure / density);
    }
};

// ============================================================================
// Toggle struct — controls which physics are active
// ============================================================================

struct CosmicToggles {
    // Core physics (default ON)
    bool gravity = true;              // N-body gravitational force
    bool sph_gas = true;              // SPH gas dynamics
    bool hubble_expansion = true;     // Friedmann scale factor a(t)

    // Extended physics (default OFF until validated)
    bool dark_energy = false;         // Cosmological constant Omega_Lambda
    bool dark_matter_halos = false;   // DM substructure & halo profiles
    bool black_hole_accretion = false; // Bondi-Hoyle accretion + jets
    bool cosmic_radiation = false;    // Radiative cooling/heating
    bool star_formation = false;      // Gas -> star conversion (Jeans)
    bool stellar_evolution = false;   // Star -> WD/NS/BH transitions
    bool galaxy_mergers = false;      // Dynamical friction
    bool magnetic_fields = false;     // Cosmic dynamo B-field
    bool radiation_pressure = false;  // Luminosity-driven gas pushing
    bool relativistic_jets = false;   // Bipolar jets from BH/Quasar
    bool gravitational_waves = false; // GW emission from mergers

    void enable_all() {
        gravity = sph_gas = hubble_expansion = true;
        dark_energy = dark_matter_halos = black_hole_accretion = true;
        cosmic_radiation = star_formation = stellar_evolution = true;
        galaxy_mergers = magnetic_fields = radiation_pressure = true;
        relativistic_jets = gravitational_waves = true;
    }
    void minimal() {
        gravity = sph_gas = hubble_expansion = true;
        dark_energy = dark_matter_halos = black_hole_accretion = false;
        cosmic_radiation = star_formation = stellar_evolution = false;
        galaxy_mergers = magnetic_fields = radiation_pressure = false;
        relativistic_jets = gravitational_waves = false;
    }
};

// ============================================================================
// Per-body force decomposition (for diagnostics + visualization)
// ============================================================================

struct CosmicForceDiag {
    Vec3 f_gravity;            // N-body gravitational
    Vec3 f_pressure;           // SPH pressure gradient
    Vec3 f_viscosity;          // SPH artificial viscosity
    Vec3 f_hubble;             // Hubble drag: -H * v
    Vec3 f_dark_energy;        // Lambda repulsion
    Vec3 f_radiation;          // Radiation pressure from luminous objects
    Vec3 f_magnetic;           // Lorentz force from cosmic B-field
    Vec3 f_dynamical_friction; // Chandrasekhar dynamical friction

    Vec3 total() const {
        return f_gravity + f_pressure + f_viscosity + f_hubble
             + f_dark_energy + f_radiation + f_magnetic + f_dynamical_friction;
    }
};

// ============================================================================
// Diagnostics
// ============================================================================

struct CosmicDiagnostics {
    int tick = 0;
    int body_count = 0;
    // Per-type counts (indexed by type+3 to handle negative enum values)
    std::array<int, 9> counts_by_type = {};
    double total_mass = 0.0;
    double total_ke = 0.0;
    double total_grav_pe = 0.0;
    double total_thermal = 0.0;
    double total_energy = 0.0;
    Vec3 total_momentum;
    Vec3 total_angular_momentum;
    double hubble_parameter = 0.0;    // H(t) = adot/a
    double scale_factor = 1.0;        // a(t)
    double dark_energy_density = 0.0; // rho_Lambda
    double matter_density = 0.0;
    double critical_density = 0.0;
    double omega_matter = 0.0;
    double omega_lambda = 0.0;
    double omega_total = 0.0;
    double virial_ratio = 0.0;        // 2K/|W|
    double max_density = 0.0;
};

// ============================================================================
// Gravitational wave event
// ============================================================================

struct GravWaveEvent {
    Vec3 origin;              // Where the merger happened
    double emission_tick = 0; // When it was emitted
    double strain = 0.0;      // h at source
    double total_mass = 0.0;  // Combined mass of merged objects
    double current_radius = 0.0; // Current propagation radius
};

// (OctreeNode has been generalized to barnes_hut.h as BarnesHutNode)

// ============================================================================
// Cosmic-scale constants derived from FTD (zero free parameters)
// ============================================================================

namespace cosmic {

// Adiabatic index: gamma = (D+2)/D = 5/3 for D=3 [THEOREM]
inline constexpr double GAMMA_ADIABATIC = (D_SPATIAL + 2.0) / D_SPATIAL;

// Dark matter fraction: 17/27 from Moore neighborhood theorem [THEOREM]
// 27 total neighbors, 10 visible (EM-coupled), 17 dark (gravity-only)
inline constexpr double DM_FRACTION = 17.0 / 27.0;

// Baryonic fraction: 10/27 [THEOREM]
inline constexpr double BARYON_FRACTION = 10.0 / 27.0;

// Barnes-Hut opening angle (accuracy parameter, not derived)
inline constexpr double THETA_BH = 0.5;

// Gravitational softening scale factor
inline constexpr double SOFTENING_SCALE = 0.01;

// SPH kernel eta parameter (number of neighbors ~ 4/3 * pi * (eta*h)^3 * n)
inline constexpr double SPH_ETA = 1.2;

// SPH viscosity parameters (Monaghan-Gingold standard)
inline constexpr double SPH_ALPHA_VISC = 1.0;
inline constexpr double SPH_BETA_VISC = 2.0;

// Star formation: Jeans density threshold (from K_GENESIS)
// Gas collapses to star when rho > rho_jeans
inline constexpr double RHO_JEANS = K_GENESIS * K_GENESIS * K_GENESIS;

// Stellar evolution mass thresholds (solar masses, FTD-derived)
// Chandrasekhar limit: M_Ch ~ 1.4 (from alpha, m_P, m_e via FTD hierarchy)
inline constexpr double M_CHANDRASEKHAR = 1.4;
// Tolman-Oppenheimer-Volkoff limit for neutron star -> black hole
inline constexpr double M_TOV = 2.2;
// Minimum mass for core-collapse supernova
inline constexpr double M_SUPERNOVA = 8.0;

// Bondi accretion efficiency
inline constexpr double BONDI_EFFICIENCY = 0.1;

// Jet velocity fraction of c (from latency field: v_jet = c * sqrt(1 - L^2))
// At maximum latency L ~ 0.99: v_jet ~ 0.14c
// At moderate latency L ~ 0.5: v_jet ~ 0.87c
inline constexpr double JET_COLLIMATION = 0.1; // Opening half-angle (radians)

} // namespace cosmic

// ============================================================================
// CosmicEngine
// ============================================================================

class CosmicEngine : public ScaleEngine {
public:
    CosmicEngine();

    CosmicToggles toggles;

    // --- Body creation ---
    int add_body(CosmicBodyType type, double mass, Vec3 position,
                 Vec3 velocity = {}, double temperature = 0.0,
                 double radius = -1.0);

    int add_dark_matter(double mass, Vec3 position, Vec3 velocity = {});
    int add_gas(double mass, Vec3 position, Vec3 velocity = {},
                double temperature = 1e4);
    int add_star(double mass, Vec3 position, Vec3 velocity = {},
                 double luminosity = -1.0);
    int add_black_hole(double mass, Vec3 position, Vec3 velocity = {});
    int add_quasar(double mass, Vec3 position, Vec3 velocity = {});
    int add_neutron_star(double mass, Vec3 position, Vec3 velocity = {});
    int add_nebula(double mass, Vec3 position, Vec3 velocity = {},
                   double temperature = 100.0);
    int add_white_dwarf(double mass, Vec3 position, Vec3 velocity = {});

    // --- Scenario builders ---
    void setup_spiral_galaxy(int n_dm = 20000, int n_gas = 15000,
                             int n_stars = 10000, double total_mass = 1e12,
                             double disk_radius = 50.0);
    void setup_galaxy_cluster(int n_galaxies = 20, double cluster_radius = 500.0);
    void setup_cosmic_web(int n_dm = 200000, double box_size = 1000.0);
    void setup_black_hole_closeup(double bh_mass = 1e9, int n_gas = 5000);
    void setup_galaxy_merger(double mass1 = 5e11, double mass2 = 3e11,
                             double separation = 200.0);
    void setup_quasar(double mass = 1e10, int n_gas = 10000);

    // --- Access ---
    std::vector<CosmicBody>& bodies() { return bodies_; }
    const std::vector<CosmicBody>& bodies() const { return bodies_; }
    int current_tick() const override { return tick_; }
    double dt() const override { return dt_; }
    void set_dt(double d) override { dt_ = d; }
    double scale_factor() const { return a_; }
    double hubble_parameter() const { return (a_ > 0.0) ? adot_ / a_ : 0.0; }
    double box_size() const { return box_size_; }
    void set_box_size(double s) { box_size_ = s; }

    const std::vector<CosmicForceDiag>& force_diag() const { return force_diag_; }
    const std::vector<GravWaveEvent>& gw_events() const { return gw_events_; }

    // --- Simulation ---
    void tick() override;
    void run(int num_ticks) override;
    CosmicDiagnostics diagnostics() const;
    void clear() override;

    // ====================================================================
    // ScaleEngine overrides — polymorphic interface for bridge dispatch
    // ====================================================================

    int scale_level() const override { return static_cast<int>(ScaleLevel::COSMIC); }
    const char* scale_name() const override { return "CosmicEngine"; }
    int entity_count() const override { return static_cast<int>(bodies_.size()); }

    bool get_toggle(const std::string& name) const override {
        if (name == "gravity")              return toggles.gravity;
        if (name == "sph_gas")              return toggles.sph_gas;
        if (name == "hubble_expansion")     return toggles.hubble_expansion;
        if (name == "dark_energy")          return toggles.dark_energy;
        if (name == "dark_matter_halos")    return toggles.dark_matter_halos;
        if (name == "black_hole_accretion") return toggles.black_hole_accretion;
        if (name == "cosmic_radiation")     return toggles.cosmic_radiation;
        if (name == "star_formation")       return toggles.star_formation;
        if (name == "stellar_evolution")    return toggles.stellar_evolution;
        if (name == "galaxy_mergers")       return toggles.galaxy_mergers;
        if (name == "magnetic_fields")      return toggles.magnetic_fields;
        if (name == "radiation_pressure")   return toggles.radiation_pressure;
        if (name == "relativistic_jets")    return toggles.relativistic_jets;
        if (name == "gravitational_waves")  return toggles.gravitational_waves;
        return false;
    }

    void set_toggle(const std::string& name, bool value) override {
        if (name == "gravity")              { toggles.gravity = value; return; }
        if (name == "sph_gas")              { toggles.sph_gas = value; return; }
        if (name == "hubble_expansion")     { toggles.hubble_expansion = value; return; }
        if (name == "dark_energy")          { toggles.dark_energy = value; return; }
        if (name == "dark_matter_halos")    { toggles.dark_matter_halos = value; return; }
        if (name == "black_hole_accretion") { toggles.black_hole_accretion = value; return; }
        if (name == "cosmic_radiation")     { toggles.cosmic_radiation = value; return; }
        if (name == "star_formation")       { toggles.star_formation = value; return; }
        if (name == "stellar_evolution")    { toggles.stellar_evolution = value; return; }
        if (name == "galaxy_mergers")       { toggles.galaxy_mergers = value; return; }
        if (name == "magnetic_fields")      { toggles.magnetic_fields = value; return; }
        if (name == "radiation_pressure")   { toggles.radiation_pressure = value; return; }
        if (name == "relativistic_jets")    { toggles.relativistic_jets = value; return; }
        if (name == "gravitational_waves")  { toggles.gravitational_waves = value; return; }
    }

    ScaleBaseDiagnostics base_diagnostics() const override {
        auto d = diagnostics();
        ScaleBaseDiagnostics b;
        b.tick = d.tick;
        b.entity_count = d.body_count;
        b.total_energy = d.total_energy;
        b.total_ke = d.total_ke;
        b.total_pe = d.total_grav_pe;
        b.total_momentum = d.total_momentum;
        return b;
    }

private:
    // === Cosmic tick cycle (18 phases) ===

    // Phase 1: Structure
    void build_octree();

    // Phase 2: Gravity
    void compute_gravity();
    Vec3 tree_force(int body_idx, int node_idx) const;

    // Phase 3-4: SPH hydrodynamics
    void compute_sph_density();
    void compute_sph_forces();
    void find_sph_neighbors();
    double sph_kernel_w(double r, double h) const;
    Vec3 sph_kernel_grad(const Vec3& rij, double h) const;

    // Phase 5-6: Cosmology
    void apply_hubble_expansion();
    void apply_dark_energy();
    void friedmann_step();

    // Phase 7-8: Compact objects
    void compute_accretion();
    void compute_relativistic_jets();

    // Phase 9-10: Stellar physics
    void check_star_formation();
    void check_stellar_evolution();

    // Phase 11-12: Extended physics
    void compute_magnetic_fields();
    void compute_radiation_pressure();

    // Phase 13-14: Gravitational waves
    void detect_gw_events();
    void propagate_gw();

    // Phase 15-17: Integration (Velocity Verlet)
    void half_kick();
    void drift();
    void enforce_speed_limit();

    // === Internal state ===
    std::vector<CosmicBody> bodies_;
    std::vector<Vec3> forces_;
    std::vector<CosmicForceDiag> force_diag_;
    std::vector<GravWaveEvent> gw_events_;

    // Barnes-Hut octree
    using CosmicTree = BarnesHutTree<CosmicBody, 
        Vec3(*)(const CosmicBody&), 
        double(*)(const CosmicBody&), 
        double(*)(const CosmicBody&)>;
    CosmicTree octree_;

    // SPH neighbor lists
    std::vector<std::vector<int>> sph_neighbors_;

    // Friedmann cosmology state
    double a_ = 1.0;            // Scale factor
    double adot_ = 0.0;         // da/dt
    double t_cosmic_ = 0.0;     // Cosmic time
    double H0_ = 0.0;           // Initial Hubble parameter (set by scenario)

    // Simulation parameters
    int tick_ = 0;
    int next_id_ = 0;
    double dt_ = 0.001;         // Cosmic time step
    double box_size_ = 1000.0;  // Simulation box (comoving)
    double softening_ = 1.0;    // Gravitational softening
};

}  // namespace ftd
