/**
 * AtomEngine: Scale 2 simulation
 *
 * Atoms as composite objects with ionic, van der Waals, and covalent forces.
 * Velocity Verlet integration (symplectic → energy-conserving).
 *
 * Force pipeline:
 *   F_ionic = -ALPHA * Q_i * Q_j * r_hat / (4*PI * (r² + soft²))
 *   F_vdW   = 24*eps * [2*(sig/r)^12 - (sig/r)^6] / r * r_hat   (LJ 12-6)
 *   F_bond  = -k * (r - r_eq) * r_hat                             (harmonic)
 *
 * All parameters from ontic chain. No gravity (alpha_G ~ 6e-39).
 */

#include "ftd/atom_engine.h"
#include <algorithm>
#include <cmath>

namespace ftd {

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
// Constructor
// ============================================================================

AtomEngine::AtomEngine() = default;

// ============================================================================
// Add / remove atoms
// ============================================================================

int AtomEngine::add_atom(int Z, Vec3 position, Vec3 velocity,
                         int charge, int N) {
    if (N < 0) N = default_neutron_count(Z);

    AtomicProperties props = compute_atomic_properties(Z, N);

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
    a.valence_electrons = props.max_bonds;  // Initially all available

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
// Bonding
// ============================================================================

int AtomEngine::index_of(int id) const {
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        if (atoms_[i].id == id) return i;
    }
    return -1;
}

bool AtomEngine::create_bond(int id_a, int id_b, int order) {
    int ia = index_of(id_a);
    int ib = index_of(id_b);
    if (ia < 0 || ib < 0) return false;

    // Check if already bonded
    for (const auto& b : atoms_[ia].bonds) {
        if (b.partner_id == id_b) return false;
    }

    // Compute bond parameters from ontic chain
    double sigma_avg = 0.5 * (atoms_[ia].vdw_sigma + atoms_[ib].vdw_sigma);
    double r_eq = sigma_avg;  // equilibrium at sigma
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

// ============================================================================
// Force computation
// ============================================================================

Vec3 AtomEngine::compute_force(int i) const {
    Vec3 f;
    const auto& ai = atoms_[i];

    for (int j = 0; j < static_cast<int>(atoms_.size()); ++j) {
        if (j == i) continue;
        const auto& aj = atoms_[j];

        Vec3 r_vec = aj.position - ai.position;
        double r2 = r_vec.mag2() + soft_ * soft_;
        double r = std::sqrt(r2);

        if (r < 1e-30) continue;

        Vec3 r_hat = r_vec * (1.0 / r);

        // 1. Ionic force (only if either atom is charged)
        if (ai.charge != 0 && aj.charge != 0) {
            // F = -ALPHA * Qi * Qj / (4*PI * r²) * r_hat
            // Same convention as ParticleEngine: negative = attractive for opposite
            double f_ionic = -ALPHA * ai.charge * aj.charge / (4.0 * PI * r2);
            f += r_hat * f_ionic;
        }

        // 2. Van der Waals (Lennard-Jones 12-6)
        // F_LJ = 24*eps * [2*(sig/r)^12 - (sig/r)^6] / r * r_hat
        // Mixing rules: eps = sqrt(eps_i * eps_j), sigma = (sig_i + sig_j)/2
        double eps_mix = std::sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
        double sig_mix = 0.5 * (ai.vdw_sigma + aj.vdw_sigma);

        if (eps_mix > 0.0 && sig_mix > 0.0) {
            double sr = sig_mix / r;
            double sr6 = sr * sr * sr * sr * sr * sr;
            double sr12 = sr6 * sr6;
            // Convention: r_hat points FROM i TOWARD j.
            // LJ: positive scalar = attractive (toward j), negative = repulsive.
            // Standard F(r) = 24*eps*[2*sr12 - sr6]/r is positive when repulsive,
            // so we negate to match our r_hat convention.
            double f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
            f += r_hat * f_vdw;
        }

        // 3. Covalent bond force (harmonic spring)
        // Convention: F_i = dV/dr * r_hat where V = 0.5*k*(r-r_eq)²
        // dV/dr = k*(r-r_eq). Stretched (r>r_eq): positive → toward partner.
        for (const auto& bond : ai.bonds) {
            if (bond.partner_id == aj.id) {
                double dr = r - bond.r_eq;
                double f_bond = bond.k_bond * dr;
                f += r_hat * f_bond;
                break;
            }
        }
    }

    return f;
}

void AtomEngine::compute_all_forces() {
    forces_.resize(atoms_.size());
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        forces_[i] = compute_force(i);
    }
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

void AtomEngine::check_bonding() {
    if (!bonding_enabled_) return;

    // Check for bond formation and breaking
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        auto& ai = atoms_[i];

        for (int j = i + 1; j < static_cast<int>(atoms_.size()); ++j) {
            auto& aj = atoms_[j];

            Vec3 r_vec = aj.position - ai.position;
            double r = std::sqrt(r_vec.mag2());

            double sig_avg = 0.5 * (ai.vdw_sigma + aj.vdw_sigma);

            // Check if already bonded
            bool bonded = false;
            int bond_idx_i = -1;
            for (int k = 0; k < static_cast<int>(ai.bonds.size()); ++k) {
                if (ai.bonds[k].partner_id == aj.id) {
                    bonded = true;
                    bond_idx_i = k;
                    break;
                }
            }

            if (bonded) {
                // Check bond breaking: stretched beyond 2x equilibrium
                if (bond_idx_i >= 0 && r > 2.0 * ai.bonds[bond_idx_i].r_eq) {
                    remove_bond(ai.id, aj.id);
                }
            } else {
                // Check bond formation: close enough AND both have available bonds
                int ai_used = static_cast<int>(ai.bonds.size());
                int aj_used = static_cast<int>(aj.bonds.size());

                if (ai_used < ai.max_bonds && aj_used < aj.max_bonds
                    && sig_avg > 0.0 && r < 1.2 * sig_avg) {
                    create_bond(ai.id, aj.id, 1);
                }
            }
        }
    }
}

void AtomEngine::enforce_speed_limit() {
    for (auto& a : atoms_) {
        if (a.locked) continue;
        double v = a.velocity.mag();
        if (v > C_SPEED) {
            a.velocity *= (C_SPEED / v);
        }
    }
}

void AtomEngine::apply_damping() {
    if (!damping_enabled_) return;
    double factor = 1.0 - DAMPING * dt_;
    if (factor < 0.0) factor = 0.0;
    for (auto& a : atoms_) {
        if (a.locked) continue;
        a.velocity *= factor;
    }
}

// ============================================================================
// Main tick (Velocity Verlet)
// ============================================================================

void AtomEngine::tick() {
    // 1. Forces at current positions
    compute_all_forces();
    // 2. Half-kick
    half_kick();
    // 3. Drift
    drift();
    // 4. Forces at new positions
    compute_all_forces();
    // 5. Half-kick
    half_kick();
    // 6. Store acceleration
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        if (!atoms_[i].locked) {
            atoms_[i].acceleration = forces_[i] * (1.0 / atoms_[i].mass);
        }
    }
    // 7. Bond formation/breaking
    check_bonding();
    // 8. Speed limit
    enforce_speed_limit();
    // 9. Damping (optional)
    apply_damping();

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

    // Kinetic energy and momentum
    for (const auto& a : atoms_) {
        double v2 = a.velocity.mag2();
        d.total_ke += 0.5 * a.mass * v2;
        d.total_momentum += a.velocity * a.mass;
    }

    // Potential energy (pairwise)
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        for (int j = i + 1; j < static_cast<int>(atoms_.size()); ++j) {
            const auto& ai = atoms_[i];
            const auto& aj = atoms_[j];

            Vec3 r_vec = aj.position - ai.position;
            double r = std::sqrt(r_vec.mag2() + soft_ * soft_);

            // Ionic PE
            if (ai.charge != 0 && aj.charge != 0) {
                d.total_pe_ionic += ALPHA * ai.charge * aj.charge
                                  / (4.0 * PI * r);
            }

            // Van der Waals PE (LJ): 4*eps * [(sig/r)^12 - (sig/r)^6]
            double eps_mix = std::sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
            double sig_mix = 0.5 * (ai.vdw_sigma + aj.vdw_sigma);
            if (eps_mix > 0.0 && sig_mix > 0.0) {
                double sr = sig_mix / r;
                double sr6 = sr * sr * sr * sr * sr * sr;
                double sr12 = sr6 * sr6;
                d.total_pe_vdw += 4.0 * eps_mix * (sr12 - sr6);
            }

            // Bond PE (harmonic): 0.5 * k * (r - r_eq)^2
            for (const auto& bond : ai.bonds) {
                if (bond.partner_id == aj.id) {
                    double dr = r - bond.r_eq;
                    d.total_pe_bond += 0.5 * bond.k_bond * dr * dr;
                    break;
                }
            }
        }
    }

    d.total_energy = d.total_ke + d.total_pe_ionic + d.total_pe_vdw + d.total_pe_bond;

    // Temperature: T = 2*KE / (3*N) in FTD natural units (k_B_Boltzmann = 1)
    int free_atoms = 0;
    for (const auto& a : atoms_) {
        if (!a.locked) free_atoms++;
    }
    if (free_atoms > 0) {
        d.temperature = 2.0 * d.total_ke / (3.0 * free_atoms);
    }

    return d;
}

}  // namespace ftd
