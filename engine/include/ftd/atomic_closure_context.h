#pragma once
/**
 * Atomic closure-context diagnostics.
 *
 * Purpose:
 *   Represent atomic scale as a shell-context vector instead of a raw scalar
 *   function of Z. This is a physics-facing diagnostic/readout object; it does
 *   not retune Atom.radius, vdw_sigma, or any force law.
 *
 * Epistemic accounting:
 *   - Electron filling order and shell bookkeeping are standard chemistry
 *     structure used as a reference model.
 *   - Slater shielding constants are [IMPOSED] empirical screening rules.
 *   - r_cloud = R_BOHR*n^2/Z_eff is a parametric hydrogenic reference
 *     scale estimate, not an FTD derivation of atomic radii.
 */

#include "constants.h"

#include <algorithm>
#include <cmath>
#include <initializer_list>
#include <utility>
#include <vector>

namespace ftd {

struct AtomicSubshell {
    int n = 0;      // principal shell
    int l = 0;      // 0=s, 1=p, 2=d, 3=f
    int count = 0;  // electrons in this subshell
};

enum class AtomicClosureRegime : int {
    Invalid = 0,
    ShellOpening,
    ShellActive,
    ShellClosed,
    TransitionBlock,
    Lanthanide,
    Actinide,
};

struct AtomicClosureConfig {
    double lattice_spacing = 1.0;  // a, for kappa = r_cloud/a
    double box_extent = 0.0;       // L, optional; 0 means zeta unavailable
    double tau_reference = 1.0;    // bath/reference time for theta
};

struct AtomicClosureContext {
    int Z = 0;
    int electron_count = 0;
    int n_shell = 0;
    int target_l = 0;
    int valence_electrons = 0;  // electrons in the outer n shell
    int active_electrons = 0;   // outer shell plus active d/f correction shell
    int active_capacity = 0;

    double source_loading = 0.0;      // Z as nuclear activation/source count
    double shielding = 0.0;           // sigma in Slater's Z_eff = Z - sigma
    double z_eff = 0.0;               // valence effective nuclear charge
    double shell_fill_fraction = 0.0; // active_electrons / active_capacity

    double r_cloud = 0.0;        // self-confined electron-cloud scale estimate
    double delta_valence = 0.0;  // active shell thickness proxy
    double xi_orbital = 0.0;     // orbital coherence-length proxy
    double tau_electronic = 0.0; // dimensionless electronic timescale proxy

    double kappa = 0.0;      // r_cloud / a
    double zeta = 0.0;       // r_cloud / L, if L supplied
    double beta = 0.0;       // delta_valence / r_cloud
    double xi_ratio = 0.0;   // xi_orbital / r_cloud
    double theta = 0.0;      // tau_electronic / tau_reference

    bool heavy_corrections_likely = false; // relativistic/poor-shielding flag
    AtomicClosureRegime regime = AtomicClosureRegime::Invalid;
};

inline int atomic_subshell_capacity(int l) {
    switch (l) {
        case 0: return 2;
        case 1: return 6;
        case 2: return 10;
        case 3: return 14;
        default: return 0;
    }
}

inline bool atomic_is_shell_closed(int Z) {
    switch (Z) {
        case 2: case 10: case 18: case 36: case 54: case 86: case 118:
            return true;
        default:
            return false;
    }
}

inline bool atomic_is_shell_opening(int Z) {
    switch (Z) {
        case 1: case 3: case 11: case 19: case 37: case 55: case 87:
            return true;
        default:
            return false;
    }
}

inline bool atomic_is_transition_block(int Z) {
    return (Z >= 21 && Z <= 30) ||
           (Z >= 39 && Z <= 48) ||
           (Z >= 72 && Z <= 80) ||
           (Z >= 104 && Z <= 112);
}

inline bool atomic_is_lanthanide(int Z) {
    return Z >= 57 && Z <= 70;
}

inline bool atomic_is_actinide(int Z) {
    return Z >= 89 && Z <= 102;
}

inline void atomic_sort_config(std::vector<AtomicSubshell>& config) {
    std::sort(config.begin(), config.end(),
              [](const AtomicSubshell& a, const AtomicSubshell& b) {
                  if (a.n != b.n) return a.n < b.n;
                  return a.l < b.l;
              });
}

inline void atomic_replace_subshells(
    std::vector<AtomicSubshell>& config,
    std::initializer_list<std::pair<int, int>> remove_keys,
    std::initializer_list<AtomicSubshell> additions)
{
    config.erase(
        std::remove_if(config.begin(), config.end(),
            [&](const AtomicSubshell& sub) {
                for (const auto& key : remove_keys) {
                    if (sub.n == key.first && sub.l == key.second) return true;
                }
                return false;
            }),
        config.end());

    for (const auto& sub : additions) {
        if (sub.count > 0) config.push_back(sub);
    }
}

inline std::vector<AtomicSubshell> atomic_electron_configuration(int Z) {
    static constexpr AtomicSubshell aufbau[] = {
        {1,0,2}, {2,0,2}, {2,1,6}, {3,0,2}, {3,1,6}, {4,0,2}, {3,2,10},
        {4,1,6}, {5,0,2}, {4,2,10}, {5,1,6}, {6,0,2}, {4,3,14}, {5,2,10},
        {6,1,6}, {7,0,2}, {5,3,14}, {6,2,10}, {7,1,6},
    };

    std::vector<AtomicSubshell> config;
    if (Z <= 0) return config;

    int remaining = std::min(Z, 118);
    for (const auto& shell : aufbau) {
        if (remaining <= 0) break;
        const int count = std::min(remaining, shell.count);
        config.push_back({shell.n, shell.l, count});
        remaining -= count;
    }

    // Standard confirmed Aufbau exceptions mirrored from the browser orbital
    // helper. They are structural bookkeeping, not FTD-derived dynamics.
    switch (Z) {
        case 24:  atomic_replace_subshells(config, {{4,0},{3,2}}, {{4,0,1},{3,2,5}}); break;
        case 29:  atomic_replace_subshells(config, {{4,0},{3,2}}, {{4,0,1},{3,2,10}}); break;
        case 41:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,1},{4,2,4}}); break;
        case 42:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,1},{4,2,5}}); break;
        case 44:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,1},{4,2,7}}); break;
        case 45:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,1},{4,2,8}}); break;
        case 46:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,0},{4,2,10}}); break;
        case 47:  atomic_replace_subshells(config, {{5,0},{4,2}}, {{5,0,1},{4,2,10}}); break;
        case 57:  atomic_replace_subshells(config, {{4,3}},       {{5,2,1}}); break;
        case 58:  atomic_replace_subshells(config, {{4,3}},       {{4,3,1},{5,2,1}}); break;
        case 64:  atomic_replace_subshells(config, {{4,3}},       {{4,3,7},{5,2,1}}); break;
        case 78:  atomic_replace_subshells(config, {{6,0},{5,2}}, {{6,0,1},{5,2,9}}); break;
        case 79:  atomic_replace_subshells(config, {{6,0},{5,2}}, {{6,0,1},{5,2,10}}); break;
        case 89:  atomic_replace_subshells(config, {{5,3}},       {{6,2,1}}); break;
        case 90:  atomic_replace_subshells(config, {{5,3}},       {{6,2,2}}); break;
        case 91:  atomic_replace_subshells(config, {{5,3}},       {{5,3,2},{6,2,1}}); break;
        case 92:  atomic_replace_subshells(config, {{5,3}},       {{5,3,3},{6,2,1}}); break;
        case 93:  atomic_replace_subshells(config, {{5,3}},       {{5,3,4},{6,2,1}}); break;
        case 96:  atomic_replace_subshells(config, {{5,3}},       {{5,3,7},{6,2,1}}); break;
        case 103: atomic_replace_subshells(config, {{5,3},{6,2}}, {{5,3,14},{7,1,1}}); break;
        default: break;
    }

    atomic_sort_config(config);
    return config;
}

inline double atomic_slater_shielding(const std::vector<AtomicSubshell>& config,
                                      int target_n,
                                      int target_l) {
    const bool target_df = target_l >= 2;
    double sigma = 0.0;

    for (const auto& sub : config) {
        if (sub.count <= 0) continue;

        if (sub.n == target_n && sub.l == target_l) {
            const double same = (target_n == 1 && target_l == 0) ? 0.30 : 0.35;
            sigma += std::max(0, sub.count - 1) * same;
        } else if (target_df) {
            if (sub.n < target_n || (sub.n == target_n && sub.l < target_l)) {
                sigma += sub.count * 1.00;
            }
        } else {
            if (sub.n == target_n && sub.l <= 1 && target_l <= 1) {
                sigma += sub.count * 0.35;
            } else if (sub.n == target_n - 1) {
                sigma += sub.count * 0.85;
            } else if (sub.n < target_n - 1) {
                sigma += sub.count * 1.00;
            }
        }
    }

    return sigma;
}

inline AtomicClosureContext compute_atomic_closure_context(
    int Z,
    const AtomicClosureConfig& cfg = AtomicClosureConfig())
{
    AtomicClosureContext c;
    c.Z = Z;
    c.electron_count = (Z > 0) ? std::min(Z, 118) : 0;
    c.source_loading = static_cast<double>(std::max(0, Z));

    const auto config = atomic_electron_configuration(Z);
    if (config.empty()) return c;

    for (const auto& sub : config) {
        if (sub.count <= 0) continue;
        c.n_shell = std::max(c.n_shell, sub.n);
    }

    for (const auto& sub : config) {
        if (sub.count <= 0) continue;
        if (sub.n == c.n_shell) {
            c.valence_electrons += sub.count;
            c.active_electrons += sub.count;
            c.active_capacity += atomic_subshell_capacity(sub.l);
            c.target_l = std::max(c.target_l, sub.l);
        }
    }

    // d/f electrons one or two shells below the boundary are chemically active
    // corrections to the shell context. They do not redefine n_shell, but they
    // do enter the active boundary vector and classification.
    for (const auto& sub : config) {
        if (sub.count <= 0) continue;
        if (sub.l >= 2 && sub.n < c.n_shell && sub.n >= c.n_shell - 2) {
            c.active_electrons += sub.count;
            c.active_capacity += atomic_subshell_capacity(sub.l);
        }
    }

    if (c.active_capacity > 0) {
        c.shell_fill_fraction =
            static_cast<double>(c.active_electrons) / static_cast<double>(c.active_capacity);
    }

    const double sigma = atomic_slater_shielding(config, c.n_shell, c.target_l);
    c.shielding = sigma;
    c.z_eff = std::max(static_cast<double>(Z) - sigma, 1.0);

    const double n = static_cast<double>(std::max(1, c.n_shell));
    c.r_cloud = R_BOHR * n * n / c.z_eff;
    c.delta_valence = c.r_cloud / n;
    const double active = static_cast<double>(std::max(1, c.active_electrons));
    c.xi_orbital = std::max(c.delta_valence, c.r_cloud / std::sqrt(active));
    c.tau_electronic = 2.0 * PI * n * n * n / (c.z_eff * c.z_eff);

    const double a = (cfg.lattice_spacing > 0.0) ? cfg.lattice_spacing : 1.0;
    c.kappa = c.r_cloud / a;
    c.zeta = (cfg.box_extent > 0.0) ? c.r_cloud / cfg.box_extent : 0.0;
    c.beta = (c.r_cloud > 0.0) ? c.delta_valence / c.r_cloud : 0.0;
    c.xi_ratio = (c.r_cloud > 0.0) ? c.xi_orbital / c.r_cloud : 0.0;
    c.theta = (cfg.tau_reference > 0.0) ? c.tau_electronic / cfg.tau_reference : 0.0;

    c.heavy_corrections_likely = Z >= 55;
    if (atomic_is_lanthanide(Z)) {
        c.regime = AtomicClosureRegime::Lanthanide;
    } else if (atomic_is_actinide(Z)) {
        c.regime = AtomicClosureRegime::Actinide;
    } else if (atomic_is_transition_block(Z)) {
        c.regime = AtomicClosureRegime::TransitionBlock;
    } else if (atomic_is_shell_closed(Z)) {
        c.regime = AtomicClosureRegime::ShellClosed;
    } else if (atomic_is_shell_opening(Z)) {
        c.regime = AtomicClosureRegime::ShellOpening;
    } else {
        c.regime = AtomicClosureRegime::ShellActive;
    }

    return c;
}

}  // namespace ftd
