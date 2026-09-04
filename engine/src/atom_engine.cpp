/**
 * AtomEngine: Scale 2 simulation — class lifecycle and tick orchestration.
 *
 * Atoms as composite objects with ionic, van der Waals, and covalent forces.
 * Velocity Verlet integration (symplectic → energy-conserving).
 *
 * Force pipeline:
 *   F_ionic = -ALPHA * Q_i * Q_j * r_hat / (4*PI * (r² + soft²))
 *   F_vdW   = 24*eps * [2*(sig/r)^12 - (sig/r)^6] / r * r_hat   (LJ 12-6)
 *   F_bond  = -k * (r - r_eq) * r_hat                             (harmonic)
 *
 * Atomic identities mix shared FTD constants with empirical element tables;
 * interaction coefficients are effective [PARAMETRIC]/[IMPOSED] model inputs.
 * No gravity term is integrated at this scale (alpha_G ~ 6e-39).
 *
 * This translation unit owns:
 *   - class ctor/dtor + GpuBackend forward declaration (CUDA-side defn
 *     lives in atom_forces.cpp, the only TU that calls into it)
 *   - atom add / remove primitives
 *   - bond create / remove / index_of primitives
 *   - Velocity Verlet half-kick + drift
 *   - tick() orchestration + run()
 *   - diagnostics() energy + temperature accounting
 *
 * Force computation, bond auto-formation, and thermostat/damping/dipole
 * live in engine/src/atom/{atom_forces.cpp, atom_bonding.cpp,
 * atom_thermostat.cpp} respectively. Class members are split across these
 * TUs as allowed by C++ — no friends, no public header changes.
 */

#include "ftd/atom_engine.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace ftd {

namespace {
bool finite_vec3(const Vec3& v) {
    return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
}
}

// ============================================================================
// Default neutron count for common elements (N ≈ Z for light elements)
// ============================================================================
static int default_neutron_count(int Z) {
    // Most stable isotope approximation
    switch (Z) {
        case 1:  return 0;   // H-1 (protium)
        case 2:  return 2;   // He-4
        case 3:  return 4;   // Li-7
        case 4:  return 5;   // Be-9
        case 5:  return 6;   // B-11
        case 6:  return 6;   // C-12
        case 7:  return 7;   // N-14
        case 8:  return 8;   // O-16
        case 9:  return 10;  // F-19
        case 10: return 10;  // Ne-20
        case 11: return 12;  // Na-23
        case 12: return 12;  // Mg-24
        case 13: return 14;  // Al-27
        case 14: return 14;  // Si-28
        case 15: return 16;  // P-31
        case 16: return 16;  // S-32
        case 17: return 18;  // Cl-35
        case 18: return 22;  // Ar-40
        default: return Z;   // rough N ≈ Z
    }
}

// ============================================================================
// Constructor / Destructor
// ============================================================================
//
// Both the default constructor AND the default destructor need to see a
// complete GpuBackend type to instantiate unique_ptr<GpuBackend>'s
// internals (constructor: needs deleter type; destructor: needs to call
// delete).
//
// On CPU builds (FTD_ENABLE_CUDA undefined) there is no gpu_backend_
// member, so defaults work fine here.
//
// On CUDA builds (FTD_ENABLE_CUDA defined) the constructor AND
// destructor are defined in src/atom/atom_forces.cpp instead, where
// GpuBackend is complete.
#ifndef FTD_ENABLE_CUDA
AtomEngine::AtomEngine() = default;
AtomEngine::~AtomEngine() = default;
#endif

// ============================================================================
// Add / remove atoms
// ============================================================================

int AtomEngine::add_atom(int Z, Vec3 position, Vec3 velocity,
                         int charge, int N) {
    if (Z < 1 || Z > 118 || !finite_vec3(position) || !finite_vec3(velocity) || N < -1) {
        throw std::invalid_argument("AtomEngine atom record is outside the supported finite domain");
    }
    if (N < 0) N = default_neutron_count(Z);

    AtomicProperties props = compute_atomic_properties(Z, N);
    if (!std::isfinite(props.mass) || props.mass <= 0.0) {
        throw std::invalid_argument("AtomEngine atom mass must be finite and positive");
    }

    Atom a;
    a.id = next_id_++;
    a.Z = Z;
    a.N = N;
    a.charge = charge;
    a.mass = props.mass;
    a.radius = props.radius;
    a.position = position;
    a.velocity = velocity;
    a.vdw_epsilon = props.vdw_epsilon;
    a.vdw_sigma = props.vdw_sigma;
    a.max_bonds = props.max_bonds;
    a.valence_electrons = props.valence_e;  // From periodic table (VSEPR geometry)
    a.electronegativity = props.electronegativity;

    a.alpha_pol = props.alpha_pol;
    a.e_ion = props.e_ion;
    a.e_aff = props.e_aff;
    a.sigma_scatter = props.sigma_scatter;
    a.z_eff = props.closure_context.z_eff;
    a.q_frac = static_cast<double>(charge);

    atoms_.push_back(a);
    forces_.push_back({});
    return a.id;
}

int AtomEngine::add_locked_atom(int Z, Vec3 position, int charge, int N) {
    int id = add_atom(Z, position, {}, charge, N);
    // Find by index (just added, so it's the last one)
    atoms_.back().locked = true;
    return id;
}

// ============================================================================
// Bonding primitives (create / remove / lookup)
//
// These stay here (not in atom_bonding.cpp) because they are used across
// the whole engine — the external API exposes create_bond/remove_bond,
// and index_of is called from forces, bonding, and dipole computation.
// ============================================================================

int AtomEngine::index_of(int id) const {
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        if (atoms_[i].id == id) return i;
    }
    return -1;
}

bool AtomEngine::create_bond(int id_a, int id_b, int order,
                             double equilibrium_distance) {
    if (id_a == id_b || order < 1 || order > 3) return false;
    int ia = index_of(id_a);
    int ib = index_of(id_b);
    if (ia < 0 || ib < 0) return false;

    // Check if already bonded
    for (const auto& b : atoms_[ia].bonds) {
        if (b.partner_id == id_b) return false;
    }

    // Compute bond parameters from ontic chain
    double sigma_avg = 0.5 * (atoms_[ia].vdw_sigma + atoms_[ib].vdw_sigma);
    double r_eq = std::isfinite(equilibrium_distance) && equilibrium_distance > 0.0
        ? equilibrium_distance
        : sigma_avg;
    double k_bond = ALPHA * K_B / (r_eq * r_eq) * order;  // stiffer for multiple bonds

    Bond ba;
    ba.partner_id = id_b;
    ba.r_eq = r_eq;
    ba.k_bond = k_bond;
    ba.order = order;
    atoms_[ia].bonds.push_back(ba);

    Bond bb;
    bb.partner_id = id_a;
    bb.r_eq = r_eq;
    bb.k_bond = k_bond;
    bb.order = order;
    atoms_[ib].bonds.push_back(bb);

    return true;
}

bool AtomEngine::remove_bond(int id_a, int id_b) {
    int ia = index_of(id_a);
    int ib = index_of(id_b);
    if (ia < 0 || ib < 0) return false;

    bool found = false;

    // Remove from a's bond list
    auto& ba = atoms_[ia].bonds;
    for (auto it = ba.begin(); it != ba.end(); ++it) {
        if (it->partner_id == id_b) {
            ba.erase(it);
            found = true;
            break;
        }
    }

    // Remove from b's bond list
    auto& bb = atoms_[ib].bonds;
    for (auto it = bb.begin(); it != bb.end(); ++it) {
        if (it->partner_id == id_a) {
            bb.erase(it);
            break;
        }
    }

    return found;
}

void AtomEngine::set_dt(double d) {
    if (!std::isfinite(d) || d <= 0.0)
        throw std::invalid_argument("AtomEngine dt must be finite and positive");
    dt_ = d;
}

void AtomEngine::set_softening(double s) {
    if (!std::isfinite(s) || s < 0.0)
        throw std::invalid_argument("AtomEngine softening must be finite and nonnegative");
    soft_ = s;
}

void AtomEngine::set_target_temperature(double T) {
    if (!std::isfinite(T) || T < 0.0)
        throw std::invalid_argument("AtomEngine thermostat target must be finite and nonnegative");
    target_temperature_ = T;
}

void AtomEngine::set_thermostat_tau(double tau) {
    if (!std::isfinite(tau) || tau <= 0.0)
        throw std::invalid_argument("AtomEngine thermostat tau must be finite and positive");
    thermostat_tau_ = tau;
}

bool AtomEngine::validate_state(std::string* err) const {
    std::string message;
    if (!std::isfinite(dt_) || dt_ <= 0.0) message = "dt must be finite and positive";
    else if (!std::isfinite(soft_) || soft_ < 0.0) message = "softening must be finite and nonnegative";
    else if (!std::isfinite(target_temperature_) || target_temperature_ < 0.0)
        message = "thermostat target must be finite and nonnegative";
    else if (!std::isfinite(thermostat_tau_) || thermostat_tau_ <= 0.0)
        message = "thermostat tau must be finite and positive";
    else if (forces_.size() != atoms_.size()) message = "force buffer must match atom count";
    else {
        for (const auto& a : atoms_) {
            if (a.Z < 1 || a.Z > 118 || a.N < 0) message = "atom identity is outside the supported domain";
            else if (!std::isfinite(a.mass) || a.mass <= 0.0) message = "atom mass must be finite and positive";
            else if (!finite_vec3(a.position) || !finite_vec3(a.velocity) ||
                     !finite_vec3(a.acceleration) || !finite_vec3(a.dipole_moment))
                message = "atom vector fields must be finite";
            else if (!std::isfinite(a.q_frac) || !std::isfinite(a.vdw_epsilon) ||
                     !std::isfinite(a.vdw_sigma) || a.vdw_epsilon < 0.0 || a.vdw_sigma < 0.0)
                message = "atom scalar fields must be finite and nonnegative where required";
            if (!message.empty()) break;
            for (const auto& b : a.bonds) {
                const int partner = index_of(b.partner_id);
                if (partner < 0 || b.partner_id == a.id || !std::isfinite(b.r_eq) || b.r_eq <= 0.0 ||
                    !std::isfinite(b.k_bond) || b.k_bond < 0.0 || b.order < 1 || b.order > 3) {
                    message = "bond record is invalid";
                    break;
                }
                const auto& reverse = atoms_[partner].bonds;
                if (std::none_of(reverse.begin(), reverse.end(), [&](const Bond& back) {
                    return back.partner_id == a.id;
                })) {
                    message = "bond record is not reciprocal";
                    break;
                }
            }
            if (!message.empty()) break;
        }
    }
    if (err) *err = message;
    return message.empty();
}

// ============================================================================
// Velocity Verlet phases
// ============================================================================

void AtomEngine::half_kick() {
    double half_dt = dt_ * 0.5;
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        if (atoms_[i].locked) continue;
        double inv_m = 1.0 / atoms_[i].mass;
        atoms_[i].velocity += forces_[i] * (half_dt * inv_m);
    }
}

void AtomEngine::drift() {
    for (auto& a : atoms_) {
        if (a.locked) continue;
        a.position += a.velocity * dt_;
    }
}

// ============================================================================
// Main tick (Velocity Verlet)
// ============================================================================

void AtomEngine::tick() {
    std::string toggle_error;
    if (!toggles.validate(&toggle_error)) {
        throw std::logic_error("AtomEngine invalid toggle profile: " + toggle_error);
    }
    std::string state_error;
    if (!validate_state(&state_error)) {
        throw std::logic_error("AtomEngine invalid pre-tick state: " + state_error);
    }
    const auto atom_snapshot = atoms_;
    const auto force_snapshot = forces_;
    // 0. Compute dipole moments for dipole-dipole force
    compute_dipole_moments();
    // 1. Forces at current positions
    compute_all_forces();
    // 2. Half-kick
    half_kick();
    // 3. Drift
    drift();
    // 4. Compute dipole moments at new positions
    compute_dipole_moments();
    // 5. Forces at new positions
    compute_all_forces();
    // 6. Half-kick
    half_kick();
    // 7. Store acceleration
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        if (!atoms_[i].locked) {
            atoms_[i].acceleration = forces_[i] * (1.0 / atoms_[i].mass);
        }
    }
    // 8. Bond formation/breaking
    check_bonding();
    // 9. Thermostat (optional)
    apply_thermostat();
    // 10. Speed limit
    enforce_speed_limit();
    // 11. Damping (optional)
    apply_damping();

    if (!validate_state(&state_error)) {
        atoms_ = atom_snapshot;
        forces_ = force_snapshot;
        throw std::logic_error("AtomEngine tick rolled back: " + state_error);
    }

    ++tick_;
}

void AtomEngine::run(int num_ticks) {
    for (int i = 0; i < num_ticks; ++i) {
        tick();
    }
}

// ============================================================================
// Diagnostics
// ============================================================================

AtomDiagnostics AtomEngine::diagnostics() const {
    AtomDiagnostics d;
    d.tick = tick_;
    d.atom_count = static_cast<int>(atoms_.size());

    // Count bonds (each bond stored in both atoms, so divide by 2)
    int total_bonds = 0;
    for (const auto& a : atoms_) {
        total_bonds += static_cast<int>(a.bonds.size());
    }
    d.bond_count = total_bonds / 2;

    // Kinetic energy and momentum. Locked atoms remain part of the system
    // energy/momentum snapshot, but not the kinetic temperature population.
    double free_ke = 0.0;
    int free_atoms = 0;
    for (const auto& a : atoms_) {
        double v2 = a.velocity.mag2();
        const double ke = 0.5 * a.mass * v2;
        d.total_ke += ke;
        d.total_momentum += a.velocity * a.mass;
        if (!a.locked) {
            free_ke += ke;
            ++free_atoms;
        }
    }

    // Potential energy (pairwise)
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(atoms_.size()); ++j) {
            const auto& ai = atoms_[i];
            const auto& aj = atoms_[j];

            Vec3 r_vec = aj.position - ai.position;
            double r = std::sqrt(r_vec.mag2() + soft_ * soft_);
            bool is_bonded = false;
            for (const auto& b : ai.bonds) {
                if (b.partner_id == aj.id) { is_bonded = true; break; }
            }
            bool is_one_three = false;
            if (!is_bonded) {
                for (const auto& bi : ai.bonds) {
                    for (const auto& bj : aj.bonds) {
                        if (bi.partner_id == bj.partner_id) { is_one_three = true; break; }
                    }
                    if (is_one_three) break;
                }
            }

            // Ionic PE uses the same fractional charges and toggle as the
            // force evaluator. Integer formal charge is only an input seed.
            if (toggles.ionic && !is_bonded && !is_one_three &&
                (std::abs(ai.q_frac) > 1e-6 || std::abs(aj.q_frac) > 1e-6)) {
                d.total_pe_ionic += ALPHA * ai.q_frac * aj.q_frac
                                  / (4.0 * PI * r);
            }

            // Van der Waals PE (LJ): 4*eps * [(sig/r)^12 - (sig/r)^6]
            double eps_mix = std::sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
            double sig_mix = 0.5 * (ai.vdw_sigma + aj.vdw_sigma);
            if (toggles.van_der_waals && !is_bonded && !is_one_three &&
                eps_mix > 0.0 && sig_mix > 0.0) {
                double sr = sig_mix / r;
                double sr6 = sr * sr * sr * sr * sr * sr;
                double sr12 = sr6 * sr6;
                d.total_pe_vdw += 4.0 * eps_mix * (sr12 - sr6);
            }

            // Bond PE (harmonic): 0.5 * k * (r - r_eq)^2
            if (toggles.covalent_bonds) {
                for (const auto& bond : ai.bonds) {
                    if (bond.partner_id == aj.id) {
                        double dr = r - bond.r_eq;
                        d.total_pe_bond += 0.5 * bond.k_bond * dr * dr;
                        break;
                    }
                }
            }
        }
    }

    // Three-body harmonic VSEPR potential. This is the scalar potential
    // whose analytic gradient is applied in atom_forces.cpp, counted once
    // for every unordered terminal pair around a central atom.
    if (toggles.angle_strain) {
        for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
            const auto& center = atoms_[i];
            if (center.bonds.size() < 2) continue;
            const int nbonds = static_cast<int>(center.bonds.size());
            const int lone_pairs = std::max(0, (center.valence_electrons - nbonds) / 2);
            const int steric_number = nbonds + lone_pairs;
            double theta_eq = std::acos(-1.0 / 3.0);
            if (steric_number == 2) theta_eq = PI;
            else if (steric_number == 3) theta_eq = 2.0 * PI / 3.0;
            else if (steric_number == 4) {
                if (lone_pairs == 1) theta_eq = 107.0 * PI / 180.0;
                else if (lone_pairs >= 2) theta_eq = 104.5 * PI / 180.0;
            }
            for (int b1 = 0; b1 < nbonds; ++b1) {
                for (int b2 = b1 + 1; b2 < nbonds; ++b2) {
                    const int j1 = index_of(center.bonds[b1].partner_id);
                    const int j2 = index_of(center.bonds[b2].partner_id);
                    if (j1 < 0 || j2 < 0) continue;
                    const Vec3 r1 = atoms_[j1].position - center.position;
                    const Vec3 r2 = atoms_[j2].position - center.position;
                    const double m1 = std::sqrt(r1.mag2());
                    const double m2 = std::sqrt(r2.mag2());
                    if (m1 < 1e-30 || m2 < 1e-30) continue;
                    const double cos_theta = std::max(-1.0, std::min(1.0,
                        (r1.x*r2.x + r1.y*r2.y + r1.z*r2.z) / (m1*m2)));
                    const double delta = std::acos(cos_theta) - theta_eq;
                    d.total_pe_angle += 0.5 * K_ANGLE * delta * delta;
                }
            }
        }
    }

    d.total_energy = d.total_ke + d.total_pe_ionic + d.total_pe_vdw +
                     d.total_pe_bond + d.total_pe_angle;

    // Temperature: locked constraints do not contribute degrees of freedom.
    if (free_atoms > 0) {
        d.temperature = 2.0 * free_ke / (3.0 * free_atoms);
    }

    // H-bond, induced dipole, and torsion implementations do not currently
    // expose complete scalar potentials matching every force term.
    d.energy_complete = !(toggles.h_bonds || toggles.dipole_dipole ||
                          toggles.torsional || toggles.improper_torsional);
    d.energy_conservative = d.energy_complete &&
                            !(toggles.damping || toggles.auto_bonding ||
                              toggles.thermostat || toggles.electronegativity);

    return d;
}

}  // namespace ftd
