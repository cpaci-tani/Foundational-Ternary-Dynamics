/**
 * AtomEngine force computation.
 *
 * Extracted from atom_engine.cpp (ticket AE1) — this is the heaviest single
 * responsibility in the atom engine: pairwise ionic/vdW/H-bond forces,
 * Barnes-Hut tree traversal, and the O(N) + O(N log N) force accumulation
 * pipeline including bond, angle-strain, dipole-dipole, torsional, and
 * improper-torsional terms.
 *
 * Methods here are DEFINITIONS of members declared in
 * engine/include/ftd/atom_engine.h — the class is split across multiple
 * translation units. No friends, no public-API changes.
 *
 * Force pipeline:
 *   F_ionic = -ALPHA * Q_i * Q_j * r_hat / (4*PI * (r² + soft²))
 *   F_vdW   = 24*eps * [2*(sig/r)^12 - (sig/r)^6] / r * r_hat   (LJ 12-6)
 *   F_bond  = -k * (r - r_eq) * r_hat                             (harmonic)
 */

#include "ftd/atom_engine.h"
#include <algorithm>
#include <cmath>

#ifdef FTD_ENABLE_CUDA
#include "ftd/gpu_atom_engine.h"
#endif

namespace ftd {

#ifdef FTD_ENABLE_CUDA
// Opaque wrapper so atom_engine.h doesn't need to include <cuda_runtime.h>.
// Holds the gpu::AtomEngineGpu and is lazily instantiated on first use.
// Defined here (not in atom_engine.cpp) because this is the only TU that
// actually calls gpu_backend_->engine.compute_pair_forces().
struct AtomEngine::GpuBackend {
    gpu::AtomEngineGpu engine;
};

// The AtomEngine destructor must be defined in a TU where GpuBackend is
// complete, so std::unique_ptr<GpuBackend>'s default_delete can see its
// destructor. atom_engine.cpp's `= default` destructor failed because
// GpuBackend is incomplete there. Redefining here overrides that.
AtomEngine::~AtomEngine() = default;
#endif

Vec3 AtomEngine::compute_pairwise_force(int i, int j) const {
    Vec3 f;
    const auto& ai = atoms_[i];
    const auto& aj = atoms_[j];

    AtomForceDiag* diag = nullptr;
    if (i < static_cast<int>(force_diag_.size())) {
        diag = &force_diag_[i];
    }

    Vec3 r_vec = aj.position - ai.position;
    double r2 = r_vec.mag2() + soft_ * soft_;
    double r = std::sqrt(r2);

    if (r < 1e-30) return f;

        Vec3 r_hat = r_vec * (1.0 / r);

        // 1. Ionic force (only if either atom is charged)
        if (toggles.ionic && ai.charge != 0 && aj.charge != 0) {
            double f_ionic = -ALPHA * ai.charge * aj.charge / (4.0 * PI * r2);
            Vec3 fi = r_hat * f_ionic;
            f += fi;
            if (diag) diag->f_ionic += fi;
        }

        // 2. Van der Waals (Lennard-Jones 12-6)
        if (toggles.van_der_waals) {
            double eps_mix = std::sqrt(ai.vdw_epsilon * aj.vdw_epsilon);
            double sig_mix = 0.5 * (ai.vdw_sigma + aj.vdw_sigma);

            if (eps_mix > 0.0 && sig_mix > 0.0) {
                double sr = sig_mix / r;
                double sr6 = sr * sr * sr * sr * sr * sr;
                double sr12 = sr6 * sr6;
                double f_vdw = -24.0 * eps_mix * (2.0 * sr12 - sr6) / r;
                Vec3 fv = r_hat * f_vdw;
                f += fv;
                if (diag) diag->f_vdw += fv;
            }
        }

        // 3. Hydrogen bond (LJ 10-12 + angular dependence)
        if (toggles.h_bonds) {
            // H-bond: D-H...A where D is electronegative donor, A is acceptor
            // Check if i is H bonded to electronegative D, and j is electronegative A
            // OR j is H bonded to D, and i is A
            auto is_electronegative = [](int Z) { return Z == 7 || Z == 8 || Z == 9; };

            bool i_is_hdonor = false;
            int donor_idx = -1;
            if (ai.Z == 1) {
                // i is H — check if bonded to electronegative atom
                for (const auto& bond : ai.bonds) {
                    int didx = index_of(bond.partner_id);
                    if (didx >= 0 && is_electronegative(atoms_[didx].Z)) {
                        i_is_hdonor = true;
                        donor_idx = didx;
                        break;
                    }
                }
            }

            bool j_is_hdonor = false;
            int donor_idx_j = -1;
            if (aj.Z == 1) {
                for (const auto& bond : aj.bonds) {
                    int didx = index_of(bond.partner_id);
                    if (didx >= 0 && is_electronegative(atoms_[didx].Z)) {
                        j_is_hdonor = true;
                        donor_idx_j = didx;
                        break;
                    }
                }
            }

            // Case 1: i is H-donor, j is electronegative acceptor (j != donor of i)
            if (i_is_hdonor && is_electronegative(aj.Z) && j != donor_idx) {
                double sig_hb = 0.5 * (ai.radius + aj.radius) * N_BASE;
                if (sig_hb > 0.0 && r > 1e-10) {
                    double sr = sig_hb / r;
                    double sr10 = std::pow(sr, 10.0);
                    double sr12 = sr10 * sr * sr;
                    // Radial: F = eps * [60*sig^12/r^13 - 60*sig^10/r^11]
                    double f_rad = H_BOND_EPSILON * 60.0 * (sr12 - sr10) / r;

                    // Angular: cos^2(theta_DHA) where D is donor, H is i, A is j
                    double cos_theta = 1.0;
                    if (donor_idx >= 0) {
                        Vec3 dh = ai.position - atoms_[donor_idx].position;
                        Vec3 ha = aj.position - ai.position;
                        double dh_mag = std::sqrt(dh.mag2());
                        double ha_mag = std::sqrt(ha.mag2());
                        if (dh_mag > 1e-30 && ha_mag > 1e-30) {
                            cos_theta = (dh.x*ha.x + dh.y*ha.y + dh.z*ha.z)
                                      / (dh_mag * ha_mag);
                        }
                    }
                    double ang_factor = cos_theta * cos_theta;

                    Vec3 fhb = r_hat * (f_rad * ang_factor);
                    f += fhb;
                    if (diag) diag->f_hbond += fhb;
                }
            }

            // Case 2: j is H-donor, i is electronegative acceptor (i != donor of j)
            if (j_is_hdonor && is_electronegative(ai.Z) && i != donor_idx_j) {
                double sig_hb = 0.5 * (ai.radius + aj.radius) * N_BASE;
                if (sig_hb > 0.0 && r > 1e-10) {
                    double sr = sig_hb / r;
                    double sr10 = std::pow(sr, 10.0);
                    double sr12 = sr10 * sr * sr;
                    double f_rad = H_BOND_EPSILON * 60.0 * (sr12 - sr10) / r;

                    double cos_theta = 1.0;
                    if (donor_idx_j >= 0) {
                        Vec3 dh = aj.position - atoms_[donor_idx_j].position;
                        Vec3 ha = ai.position - aj.position;
                        double dh_mag = std::sqrt(dh.mag2());
                        double ha_mag = std::sqrt(ha.mag2());
                        if (dh_mag > 1e-30 && ha_mag > 1e-30) {
                            cos_theta = (dh.x*ha.x + dh.y*ha.y + dh.z*ha.z)
                                      / (dh_mag * ha_mag);
                        }
                    }
                    double ang_factor = cos_theta * cos_theta;

                    Vec3 fhb = r_hat * (f_rad * ang_factor);
                    f += fhb;
                    if (diag) diag->f_hbond += fhb;
                }
            }
        }

        return f;
}

Vec3 AtomEngine::tree_force(int i, int node_idx) const {
    const BarnesHutNode& node = octree_.nodes[node_idx];
    const auto& ai = atoms_[i];

    // Skip empty nodes
    if (node.total_mass <= 0.0 && node.total_charge == 0.0) return {};

    Vec3 r_vec = node.center_of_mass - ai.position;
    double r2 = r_vec.mag2() + soft_ * soft_;
    double r = std::sqrt(r2);

    if (node.is_leaf) {
        if (node.body_indices.empty()) return {};
        Vec3 lf;
        for (int b_idx : node.body_indices) {
            if (b_idx == i) continue;
            lf += compute_pairwise_force(i, b_idx);
        }
        return lf;
    }


    // Barnes-Hut opening angle test (THETA_BH = 0.5)
    if (node.width() / r < 0.5) {
        // Far away: monopole approximation ONLY for long-range 1/r^2 Ionic forces
        Vec3 r_hat = r_vec * (1.0 / r);
        Vec3 f;
        AtomForceDiag* diag = nullptr;
        if (i < static_cast<int>(force_diag_.size())) diag = &force_diag_[i];

        if (toggles.ionic && ai.charge != 0 && node.total_charge != 0.0) {
            double f_ionic = -ALPHA * ai.charge * node.total_charge / (4.0 * PI * r2);
            Vec3 fi = r_hat * f_ionic;
            f += fi;
            if (diag) diag->f_ionic += fi;
        }
        return f;
    }

    // Recurse into children
    Vec3 force = {};
    for (int c = 0; c < 8; ++c) {
        if (node.children[c] >= 0) {
            Vec3 cf = tree_force(i, node.children[c]);
            force.x += cf.x;
            force.y += cf.y;
            force.z += cf.z;
        }
    }
    return force;
}

Vec3 AtomEngine::compute_force(int i) const {
    Vec3 f;
    for (int j = 0; j < static_cast<int>(atoms_.size()); ++j) {
        if (i == j) continue;
        f += compute_pairwise_force(i, j);
    }
    // Also include bond forces for backward compatibility in tests
    const auto& ai = atoms_[i];
    if (toggles.covalent_bonds) {
        for (const auto& bond : ai.bonds) {
            int j = index_of(bond.partner_id);
            if (j < 0) continue;
            Vec3 r_vec = atoms_[j].position - ai.position;
            double r2 = r_vec.mag2() + soft_ * soft_;
            double r = std::sqrt(r2);
            if (r > 1e-10) {
                Vec3 r_hat = r_vec * (1.0 / r);
                double dr = r - bond.r_eq;
                double f_bond = bond.k_bond * dr;
                f += r_hat * f_bond;
            }
        }
    }
    return f;
}

void AtomEngine::compute_all_forces() {
    forces_.resize(atoms_.size());
    force_diag_.resize(atoms_.size());

    // O(N) Initialization
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        force_diag_[i] = {};
        forces_[i] = {};
    }

    // =========================================================================
    // Wave 5.3 Phase 1: GPU pair-force fast path
    // =========================================================================
    // When use_gpu_ is on AND CUDA is compiled in, upload atoms to the device
    // and run an O(N²) CUDA kernel for ionic + vdW pair forces. Multi-body
    // terms (bonds, angle strain, dipole-dipole, torsion, thermostat) still
    // run on CPU below — they require atom-local topology (bond lists) that
    // isn't on the device yet.
    //
    // Hydrogen bonds also stay on CPU: they need bond-topology lookups for
    // donor identification + angular dependence. We fall back to the CPU
    // Barnes-Hut path whenever toggles.h_bonds is on OR atom count is too
    // small (< 8) for the GPU upload/download overhead to pay off.
    bool gpu_pair_handled = false;
#ifdef FTD_ENABLE_CUDA
    if (use_gpu_ && !toggles.h_bonds && atoms_.size() >= 8) {
        if (!gpu_backend_) {
            gpu_backend_ = std::make_unique<GpuBackend>();
        }
        gpu_backend_->engine.compute_pair_forces(
            atoms_, toggles, soft_, forces_, force_diag_);
        gpu_pair_handled = true;
    }
#endif

    // Build O(N log N) spatial partition tree — still needed on CPU for the
    // H-bond fallback path and for deterministic diagnostics in GPU-off mode.
    octree_.build(atoms_,
        [](const Atom& a) { return a.position; },
        [](const Atom& a) { return a.mass; },
        [](const Atom& a) { return static_cast<double>(a.charge); }
    );

    // Tree force accumulation (Ionic Coulomb, LJ 12-6, Hbonds) — only if the
    // GPU fast path did not already handle the pair loop.
    if (!gpu_pair_handled) {
        for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
            if (octree_.root >= 0) {
                forces_[i] += tree_force(i, octree_.root);
            }
        }
    }

    // O(N) Topological Bond evaluations (Harmoic, Angle Strain)
    for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
        const auto& ai = atoms_[i];
        AtomForceDiag* diag = &force_diag_[i];

        if (toggles.covalent_bonds) {
            for (const auto& bond : ai.bonds) {
                int j = index_of(bond.partner_id);
                if (j < 0) continue;

                // Note: bonds are reciprocal, but we process them symmetrically
                // by looping over all atoms and pushing forces locally.
                Vec3 r_vec = atoms_[j].position - ai.position;
                double r2 = r_vec.mag2() + soft_ * soft_;
                double r = std::sqrt(r2);
                if (r > 1e-10) {
                    Vec3 r_hat = r_vec * (1.0 / r);
                    double dr = r - bond.r_eq;
                    double f_bond = bond.k_bond * dr;
                    Vec3 fb = r_hat * f_bond;
                    forces_[i] += fb;
                    diag->f_bond += fb;
                }
            }
        }
    }

    // C5 fix: Distribute angle-strain forces to terminal atoms (Newton's 3rd law).
    // compute_force(i) only applies the reaction force to central atom i.
    // This second pass adds the correct forces to j1 and j2.
    if (toggles.angle_strain) {
        for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
            const auto& ai = atoms_[i];
            for (int b1 = 0; b1 < static_cast<int>(ai.bonds.size()); ++b1) {
                for (int b2 = b1 + 1; b2 < static_cast<int>(ai.bonds.size()); ++b2) {
                    int j1 = index_of(ai.bonds[b1].partner_id);
                    int j2 = index_of(ai.bonds[b2].partner_id);
                    if (j1 < 0 || j2 < 0) continue;

                    Vec3 r1 = atoms_[j1].position - ai.position;
                    Vec3 r2v = atoms_[j2].position - ai.position;
                    double m1 = std::sqrt(r1.mag2());
                    double m2 = std::sqrt(r2v.mag2());
                    if (m1 < 1e-30 || m2 < 1e-30) continue;

                    double cos_theta = (r1.x*r2v.x + r1.y*r2v.y + r1.z*r2v.z) / (m1 * m2);
                    if (cos_theta > 1.0) cos_theta = 1.0;
                    if (cos_theta < -1.0) cos_theta = -1.0;
                    double theta = std::acos(cos_theta);

                    int nbonds = static_cast<int>(ai.bonds.size());
                    int lone_pairs = ai.valence_electrons - nbonds;
                    if (lone_pairs < 0) lone_pairs = 0;
                    int steric_number = nbonds + lone_pairs;

                    double theta_eq;
                    switch (steric_number) {
                        case 2: theta_eq = PI; break;
                        case 3: theta_eq = 2.0 * PI / 3.0; break;
                        case 4:
                            if (lone_pairs == 0) theta_eq = std::acos(-1.0/3.0);
                            else if (lone_pairs == 1) theta_eq = 107.0 * PI / 180.0;
                            else theta_eq = 104.5 * PI / 180.0;
                            break;
                        default: theta_eq = std::acos(-1.0/3.0); break;
                    }

                    double delta_theta = theta - theta_eq;
                    double sin_theta = std::sin(theta);
                    if (std::abs(sin_theta) < 1e-15) continue;

                    double dV = K_ANGLE * delta_theta;

                    Vec3 r1_hat = r1 * (1.0 / m1);
                    Vec3 r2_hat = r2v * (1.0 / m2);
                    Vec3 perp1 = r2_hat - r1_hat * cos_theta;
                    double perp1_mag = std::sqrt(perp1.mag2());
                    if (perp1_mag < 1e-30) continue;
                    perp1 = perp1 * (1.0 / perp1_mag);

                    Vec3 f_j1 = perp1 * (dV / (m1 * sin_theta));

                    Vec3 perp2 = r1_hat - r2_hat * cos_theta;
                    double perp2_mag = std::sqrt(perp2.mag2());
                    if (perp2_mag < 1e-30) continue;
                    perp2 = perp2 * (1.0 / perp2_mag);

                    Vec3 f_j2 = perp2 * (dV / (m2 * sin_theta));

                    // Newton's 3rd law: center atom gets the equal-and-opposite
                    // reaction force. Without this, angle_strain would conserve
                    // neither momentum nor energy, AND force_diag_[i].f_angle
                    // would never be populated (causing AS6 and similar diag
                    // checks to fail). Wave 4a.1 audit (2026-04-14).
                    Vec3 f_center = {-(f_j1.x + f_j2.x),
                                     -(f_j1.y + f_j2.y),
                                     -(f_j1.z + f_j2.z)};

                    forces_[j1] += f_j1;
                    forces_[j2] += f_j2;
                    forces_[i]  += f_center;
                    force_diag_[i].f_angle += f_center;
                    if (j1 < static_cast<int>(force_diag_.size())) force_diag_[j1].f_angle += f_j1;
                    if (j2 < static_cast<int>(force_diag_.size())) force_diag_[j2].f_angle += f_j2;
                }
            }
        }
    }

    // 6b. Dipole-Dipole interaction between polar atoms.
    //
    // The dipole_dipole toggle existed and force_diag had a f_dipole field,
    // but the actual force computation was MISSING from atom_engine prior to
    // this commit. compute_dipole_moments() populates mu_i from bond structure
    // + electronegativity, but no code turned those moments into a force.
    //
    // Standard two-dipole interaction (point dipoles in vacuum):
    //   U(r) = (1/(4πε₀ r³)) * [μ_i·μ_j - 3(μ_i·r̂)(μ_j·r̂)]
    //   F_ij = -∇U = (3/(4πε₀ r⁴)) * [
    //            (μ_i·μ_j) r̂
    //          + (μ_i·r̂) μ_j
    //          + (μ_j·r̂) μ_i
    //          - 5 (μ_i·r̂)(μ_j·r̂) r̂
    //       ]
    //
    // On the FTD lattice we use ALPHA for the 1/(4πε₀) prefactor (same
    // convention as the ionic Coulomb block) and apply Newton's 3rd law
    // by pushing -F_ij to j. Wave 4a.1 audit (2026-04-14).
    if (toggles.dipole_dipole) {
        for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
            const auto& ai = atoms_[i];
            const Vec3& mu_i = ai.dipole_moment;
            if (mu_i.mag2() < 1e-30) continue;
            for (int j = i + 1; j < static_cast<int>(atoms_.size()); ++j) {
                const auto& aj = atoms_[j];
                const Vec3& mu_j = aj.dipole_moment;
                if (mu_j.mag2() < 1e-30) continue;

                // Skip bonded atom pairs: intra-molecular dynamics is handled
                // by the harmonic bond + angle_strain terms. Counting
                // dipole-dipole between bonded atoms would double-book the
                // physics AND swamp the inter-molecular signal the toggle
                // is designed to measure.
                bool bonded = false;
                for (const auto& b : ai.bonds) {
                    if (b.partner_id == aj.id) { bonded = true; break; }
                }
                if (bonded) continue;

                Vec3 r_vec = aj.position - ai.position;
                double r2 = r_vec.mag2() + soft_ * soft_;
                double r = std::sqrt(r2);
                if (r < 1e-10) continue;
                double inv_r = 1.0 / r;
                double inv_r4 = inv_r * inv_r * inv_r * inv_r;
                Vec3 r_hat = r_vec * inv_r;

                double mu_i_dot_r = mu_i.x * r_hat.x + mu_i.y * r_hat.y + mu_i.z * r_hat.z;
                double mu_j_dot_r = mu_j.x * r_hat.x + mu_j.y * r_hat.y + mu_j.z * r_hat.z;
                double mu_i_dot_mu_j =
                    mu_i.x * mu_j.x + mu_i.y * mu_j.y + mu_i.z * mu_j.z;

                double coeff = 3.0 * ALPHA / (4.0 * PI) * inv_r4;
                Vec3 f_ij;
                f_ij.x = coeff * (mu_i_dot_mu_j * r_hat.x
                                  + mu_i_dot_r * mu_j.x
                                  + mu_j_dot_r * mu_i.x
                                  - 5.0 * mu_i_dot_r * mu_j_dot_r * r_hat.x);
                f_ij.y = coeff * (mu_i_dot_mu_j * r_hat.y
                                  + mu_i_dot_r * mu_j.y
                                  + mu_j_dot_r * mu_i.y
                                  - 5.0 * mu_i_dot_r * mu_j_dot_r * r_hat.y);
                f_ij.z = coeff * (mu_i_dot_mu_j * r_hat.z
                                  + mu_i_dot_r * mu_j.z
                                  + mu_j_dot_r * mu_i.z
                                  - 5.0 * mu_i_dot_r * mu_j_dot_r * r_hat.z);

                // Newton's 3rd law: force on i from j is +f_ij; on j from i is -f_ij
                forces_[i] += f_ij;
                forces_[j].x -= f_ij.x;
                forces_[j].y -= f_ij.y;
                forces_[j].z -= f_ij.z;
                force_diag_[i].f_dipole += f_ij;
                Vec3 neg_f_ij = {-f_ij.x, -f_ij.y, -f_ij.z};
                force_diag_[j].f_dipole += neg_f_ij;
            }
        }
    }

    // 7. Torsional (Dihedral) Strain — 4-body chains (1-2-3-4)
    if (toggles.torsional) {
        for (int i2 = 0; i2 < static_cast<int>(atoms_.size()); ++i2) {
            const auto& a2 = atoms_[i2];
            for (const auto& b2 : a2.bonds) {
                int i3 = index_of(b2.partner_id);
                if (i2 >= i3) continue; // Process each central bond exactly once

                const auto& a3 = atoms_[i3];
                // For all other neighbors of i2 (atom 1)
                for (const auto& b1 : a2.bonds) {
                    int i1 = index_of(b1.partner_id);
                    if (i1 == i3) continue;

                    // For all other neighbors of i3 (atom 4)
                    for (const auto& b3 : a3.bonds) {
                        int i4 = index_of(b3.partner_id);
                        if (i4 == i2 || i4 == i1) continue;

                        // Chain found: i1 - i2 - i3 - i4
                        Vec3 r1 = atoms_[i1].position;
                        Vec3 r2 = a2.position;
                        Vec3 r3 = a3.position;
                        Vec3 r4 = atoms_[i4].position;

                        Vec3 b1v = r2 - r1;
                        Vec3 b2v = r3 - r2;
                        Vec3 b3v = r4 - r3;

                        Vec3 m = Vec3::cross(b1v, b2v);
                        Vec3 n = Vec3::cross(b2v, b3v);
                        double m2 = m.mag2();
                        double n2 = n.mag2();
                        double b2_mag_sq = b2v.mag2();
                        double b2_mag = std::sqrt(b2_mag_sq);

                        if (m2 < 1e-30 || n2 < 1e-30 || b2_mag < 1e-30) continue;

                        double costheta = m.dot(n) / std::sqrt(m2 * n2);
                        if (costheta > 1.0) costheta = 1.0;
                        if (costheta < -1.0) costheta = -1.0;

                        // Sign of dihedral
                        double sign = b1v.dot(n);
                        double phi = std::acos(costheta);
                        if (sign < 0) phi = -phi;

                        // Generic sp3 organic chemistry: 3-fold periodicity
                        constexpr int n_fold = 3;
                        constexpr double gamma = 0.0;
                        double dV_dphi = -0.5 * V_TORSION * n_fold * std::sin(n_fold * phi - gamma);

                        // Gradient chain rule for dihedral derivatives
                        Vec3 f1 = m * (dV_dphi * b2_mag / m2);
                        Vec3 f4 = n * (-dV_dphi * b2_mag / n2);

                        double dot12 = b1v.dot(b2v) / b2_mag_sq;
                        double dot23 = b2v.dot(b3v) / b2_mag_sq;

                        Vec3 f2 = f1 * (dot12 - 1.0) - f4 * dot23;
                        Vec3 f3 = f4 * (dot23 - 1.0) - f1 * dot12;

                        forces_[i1] += f1;
                        forces_[i2] += f2;
                        forces_[i3] += f3;
                        forces_[i4] += f4;
                    }
                }
            }
        }
    }

    // 8. Improper Torsions (Planarity) — 4-body (center + 3 neighbors)
    if (toggles.improper_torsional) {
        for (int i = 0; i < static_cast<int>(atoms_.size()); ++i) {
            const auto& ai = atoms_[i];
            int nbonds = static_cast<int>(ai.bonds.size());
            int lone_pairs = ai.valence_electrons - nbonds;
            if (lone_pairs < 0) lone_pairs = 0;
            int steric_number = nbonds + lone_pairs;

            // Apply to trigonal planar (sp2) centers
            if (steric_number == 3 && lone_pairs == 0 && nbonds == 3) {
                int j1 = index_of(ai.bonds[0].partner_id);
                int j2 = index_of(ai.bonds[1].partner_id);
                int j3 = index_of(ai.bonds[2].partner_id);
                if (j1 < 0 || j2 < 0 || j3 < 0) continue;

                // Polyhedral volume formulation (strictly conserves energy and momentum)
                Vec3 v1 = atoms_[j1].position - ai.position;
                Vec3 v2 = atoms_[j2].position - ai.position;
                Vec3 v3 = atoms_[j3].position - ai.position;

                Vec3 cross23 = Vec3::cross(v2, v3);
                double vol = v1.dot(cross23);

                // F_j1 = -dV/dr_j1 = -K * Vol * grad(Vol)
                Vec3 cross31 = Vec3::cross(v3, v1);
                Vec3 cross12 = Vec3::cross(v1, v2);

                Vec3 f1 = cross23 * (-K_IMPROPER * vol);
                Vec3 f2 = cross31 * (-K_IMPROPER * vol);
                Vec3 f3 = cross12 * (-K_IMPROPER * vol);
                Vec3 f0 = f1 + f2 + f3;
                f0 = f0 * (-1.0); // Newton's 3rd law

                forces_[j1] += f1;
                forces_[j2] += f2;
                forces_[j3] += f3;
                forces_[i]  += f0;
            }
        }
    }
}

}  // namespace ftd
