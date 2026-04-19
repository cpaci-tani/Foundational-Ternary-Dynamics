/**
 * AtomEngine dynamic bond formation / breaking.
 *
 * Extracted from atom_engine.cpp (ticket AE2). The auto_bonding toggle
 * runs this once per tick after the Velocity Verlet step — it scans
 * every atom pair, forms bonds when within 1.2 * sigma_avg (scaled by
 * electronegativity difference), and breaks bonds stretched past 2 * r_eq.
 *
 * create_bond / remove_bond / index_of remain in atom_engine.cpp because
 * they are primitives used across the whole class (forces, bonding,
 * diagnostics, external API).
 */

#include "ftd/atom_engine.h"
#include <cmath>

namespace ftd {

void AtomEngine::check_bonding() {
    if (!toggles.auto_bonding) return;

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

                // Electronegativity extends bond formation radius for polar pairs
                double bond_radius = 1.2 * sig_avg;
                if (toggles.electronegativity) {
                    double chi_diff = std::abs(ai.electronegativity - aj.electronegativity);
                    bond_radius *= (1.0 + 0.2 * chi_diff);
                }

                if (ai_used < ai.max_bonds && aj_used < aj.max_bonds
                    && sig_avg > 0.0 && r < bond_radius) {
                    create_bond(ai.id, aj.id, 1);
                    ai_used++;  // Update cached count after bond formation
                    if (ai_used >= ai.max_bonds) break;  // No more bonds available
                }
            }
        }
    }
}

}  // namespace ftd
