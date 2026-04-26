/**
 * @file test_smallest_particle_emergence.cpp
 * @brief Phase-4h: Material emergence from the lattice. What IS the smallest
 *        particle that manifests from genesis?
 *
 * Observational study, not a pass/fail test. Triggers controlled genesis
 * events and catalogs the quantum numbers of each emergent particle:
 *
 *     state   ∈ {+1, -1}         (charge sign)
 *     spin    ∈ {+1, -1, 0}       (from sign of dominant curl component)
 *     color   ∈ {1, 2, 3, 0}      (from dominant flux axis; 0 = colorless)
 *     flavor  ∈ {0, 1, 2, 3}      (from weak substrate; 0 if no weak field)
 *
 * Quantum numbers then compared to:
 *   - Electron:  state = ±1, spin = ±1, color = 0, flavor = 1
 *   - Up quark:  state = ±(2/3), spin = ±1, color ∈ {1,2,3}, flavor = 0 (? or not tracked)
 *   - Down quark: state = ±(1/3), spin = ±1, color ∈ {1,2,3}, flavor = 0
 *
 * Protocol:
 *   Five distinct flux injections:
 *   (i)   Pure x-axis flux         → expect color = 1 (R)
 *   (ii)  Pure y-axis flux         → expect color = 2 (G)
 *   (iii) Pure z-axis flux         → expect color = 3 (B)
 *   (iv)  Mixed curl flux          → measure spin non-trivially
 *   (v)   Balanced-axis flux       → does color ever come out 0?
 *
 * Expected finding: EVERY genesis event produces a colored (color ≠ 0)
 * single-voxel particle. That particle matches QUARK quantum numbers, not
 * electron quantum numbers. If confirmed, it clarifies the FTD emergence
 * story: the lattice produces QUARKS natively; electrons require a
 * different mechanism (selection, binding, or bound-state residue).
 */

#include "ftd/constants.h"
#include "ftd/render_bridge.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

struct EmergentParticle {
    int x = 0, y = 0, z = 0;
    int state = 0;
    int spin = 0;
    int color = 0;
    int flavor = 0;
    double flux_mag = 0.0;
    double wave_vel_mag = 0.0;
};

const char* color_name(int c) {
    switch (c) {
        case 0: return "colorless";
        case 1: return "red";
        case 2: return "green";
        case 3: return "blue";
        default: return "?";
    }
}

const char* spin_name(int s) {
    if (s == +1) return "↑";
    if (s == -1) return "↓";
    return "0";
}

// Inject a given flux pattern at a single site, run one tick with genesis,
// and identify the resulting emergent particle (if any).
EmergentParticle run_genesis(const ftd::Vec3& flux_at_center,
                             bool add_curl_off_axis,
                             unsigned int seed) {
    const int L = 16;
    const int c = L / 2;

    ftd::RenderBridge rb(L);
    rb.toggles.disable_all();
    rb.toggles.wave_propagation = true;
    rb.toggles.gauss_projection = true;
    rb.toggles.genesis          = true;
    rb.toggles.dual_substrate   = false;
    rb.seed_rng(seed);

    // Inject flux at the center site and its neighbors to create a
    // localized high-flux region. Neighbor contributions give divergence.
    rb.inject_flux(c, c, c, flux_at_center);

    // Optional off-axis flux for curl generation (needed for non-zero spin)
    if (add_curl_off_axis) {
        // Small perpendicular flux at (c+1, c, c) creates ∂_x J_y or similar
        ftd::Vec3 perp{0, flux_at_center.mag() * 0.3, flux_at_center.mag() * 0.2};
        rb.inject_flux(c + 1, c, c, perp);
    }

    rb.run(1);

    EmergentParticle p;
    const auto& vox = rb.voxels();
    // Scan for the manifested site (should be near center for a sharp pulse)
    const int R = 2;
    for (int dx = -R; dx <= R; ++dx)
    for (int dy = -R; dy <= R; ++dy)
    for (int dz = -R; dz <= R; ++dz) {
        const int xx = c + dx, yy = c + dy, zz = c + dz;
        const int i = xx * L * L + yy * L + zz;
        if (vox[i].state != 0) {
            p.x = xx; p.y = yy; p.z = zz;
            p.state  = static_cast<int>(vox[i].state);
            p.spin   = static_cast<int>(vox[i].spin);
            p.color  = static_cast<int>(vox[i].color);
            p.flavor = static_cast<int>(vox[i].flavor);
            p.flux_mag = vox[i].flux.mag();
            p.wave_vel_mag = vox[i].wave_vel.mag();
            return p;
        }
    }
    return p;  // no manifestation
}

void report_particle(const std::string& scenario, const EmergentParticle& p) {
    std::printf("  %-35s", scenario.c_str());
    if (p.state == 0) {
        std::printf(" → NO MANIFESTATION (genesis did not fire)\n");
        return;
    }
    std::printf(" → at (%2d,%2d,%2d): state=%+d spin=%-3s color=%s flavor=%d",
                p.x, p.y, p.z,
                p.state, spin_name(p.spin),
                color_name(p.color), p.flavor);
    std::printf("  |J|=%.3f\n", p.flux_mag);
}

}  // namespace

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("================================================================\n");
    std::printf("  Phase-4h: Material Emergence — The Smallest Particle\n");
    std::printf("================================================================\n");
    std::printf("  Quantum-number classification of single-voxel manifestations.\n\n");

    const double A = 5.0 * ftd::K_GENESIS;  // well above threshold

    std::vector<std::pair<std::string, EmergentParticle>> results;

    // (i) Pure x-axis flux pulse
    results.push_back({"(i)   flux = (A, 0, 0), no curl",
                       run_genesis({A, 0, 0}, false, 0xAA01)});

    // (ii) Pure y-axis flux pulse
    results.push_back({"(ii)  flux = (0, A, 0), no curl",
                       run_genesis({0, A, 0}, false, 0xBB02)});

    // (iii) Pure z-axis flux pulse
    results.push_back({"(iii) flux = (0, 0, A), no curl",
                       run_genesis({0, 0, A}, false, 0xCC03)});

    // (iv) Mixed flux with curl (off-axis neighbor perturbation)
    results.push_back({"(iv)  flux = (A, 0, 0) + curl",
                       run_genesis({A, 0, 0}, true, 0xDD04)});

    // (v) Balanced all-three-axis flux
    results.push_back({"(v)   flux = (A, A, A)/√3 balanced",
                       run_genesis({A / std::sqrt(3.0), A / std::sqrt(3.0),
                                     A / std::sqrt(3.0)}, false, 0xEE05)});

    // (vi) Strongly biased with small orthogonal component
    results.push_back({"(vi)  flux = (A, A/10, 0)",
                       run_genesis({A, A / 10, 0}, false, 0xFF06)});

    std::printf("--- Emergent particles from six distinct flux configurations ---\n");
    for (const auto& kv : results) {
        report_particle(kv.first, kv.second);
    }

    // Count by color
    int colored = 0, colorless = 0, no_manifest = 0;
    int has_spin = 0, has_flavor = 0;
    for (const auto& kv : results) {
        const auto& p = kv.second;
        if (p.state == 0) { ++no_manifest; continue; }
        if (p.color == 0) ++colorless;
        else ++colored;
        if (p.spin != 0) ++has_spin;
        if (p.flavor != 0) ++has_flavor;
    }

    std::printf("\n--- Statistics over %zu genesis events ---\n", results.size());
    std::printf("  manifested:          %zu\n", results.size() - no_manifest);
    std::printf("  colored (R,G,B):     %d\n", colored);
    std::printf("  colorless:           %d\n", colorless);
    std::printf("  with non-zero spin:  %d\n", has_spin);
    std::printf("  with non-zero flavor:%d  (requires weak substrate)\n", has_flavor);

    std::printf("\n--- Comparison to Standard-Model quantum numbers ---\n");
    std::printf("  Electron: state=±1, spin=±1, color=0 (colorless), flavor=1\n");
    std::printf("  Quark (u/d):state=±1, spin=±1, color∈{1,2,3}, flavor=0\n");

    std::printf("\n--- Verdict ---\n");
    if (colored > 0 && colorless == 0) {
        std::printf("  Every genesis event produces a COLORED single-voxel particle.\n");
        std::printf("  By quantum-number match, the smallest particle that emerges\n");
        std::printf("  natively from the lattice is a **QUARK** (single color,\n");
        std::printf("  spin-1/2-like, integer charge), NOT an electron.\n\n");
        std::printf("  Implication: electrons in FTD are not primitive single-voxel\n");
        std::printf("  emergences. They must arise via a different mechanism —\n");
        std::printf("  candidates:\n");
        std::printf("    (a) Color-singlet bound state of 3 quarks (exchange-force\n");
        std::printf("        confinement + triad_binding), but that gives baryons.\n");
        std::printf("    (b) Annihilation residue (pair production → photon → ???).\n");
        std::printf("    (c) Flavor-tagged leptonic branch (flavor=1 in weak sector).\n");
        std::printf("    (d) Composite object from multiple void interactions.\n");
    } else if (colorless > 0) {
        std::printf("  Some genesis events produce colorless particles.\n");
        std::printf("  Count: %d colored, %d colorless.\n", colored, colorless);
    }
    std::printf("================================================================\n");
    return 0;
}
