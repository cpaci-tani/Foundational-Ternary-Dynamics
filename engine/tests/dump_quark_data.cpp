/**
 * Dump quark/color visualization data from the FTD engine.
 *
 * Voxels have color ∈ {0=colorless, 1=R, 2=G, 3=B} assigned at genesis
 * by dominant flux axis (Moore Layer Theorem: 3 colors = 3 spatial dims).
 *
 * Tests:
 *   1. Single quark per color (pure x/y/z flux injection)
 *   2. Quark-antiquark "meson" (state=+1 + state=-1, same color)
 *   3. Three-quark "baryon" (R+G+B at triangle vertices)
 *   4. Mixed-direction injection (45° flux → which color wins?)
 *   5. Color-segregation under +color_forces toggle
 *   6. Quark separation (try to pull R apart, look for confinement string)
 */
#include <iostream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <functional>
#include "ftd/render_bridge.h"
#include "ftd/voxel.h"
#include "ftd/constants.h"

struct Coord3D {
    int x, y, z;
    int8_t state;
    int8_t color;
    double flux_x, flux_y, flux_z;
};

static std::vector<Coord3D> get_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const auto& lat = rb.lattice();
    const int64_t total = lat.total_sites();
    std::vector<Coord3D> coords;
    for (int64_t i = 0; i < total; ++i) {
        if (vox[i].state != 0) {
            auto c = lat.coord(static_cast<int>(i));
            coords.push_back({c.x, c.y, c.z, vox[i].state, vox[i].color,
                              vox[i].flux.x, vox[i].flux.y, vox[i].flux.z});
        }
    }
    return coords;
}

static int count_manifested(const ftd::RenderBridge& rb) {
    const auto& vox = rb.voxels();
    const int64_t total = rb.lattice().total_sites();
    int n = 0;
    for (int64_t i = 0; i < total; ++i) if (vox[i].state != 0) ++n;
    return n;
}

static void dump_coords_json(const std::vector<Coord3D>& coords, const char* indent) {
    std::cout << indent << "[\n";
    for (size_t i = 0; i < coords.size(); ++i) {
        std::cout << indent << "  {\"x\":" << coords[i].x
                  << ",\"y\":" << coords[i].y
                  << ",\"z\":" << coords[i].z
                  << ",\"s\":" << static_cast<int>(coords[i].state)
                  << ",\"c\":" << static_cast<int>(coords[i].color)
                  << ",\"fx\":" << std::fixed << std::setprecision(3) << coords[i].flux_x
                  << ",\"fy\":" << coords[i].flux_y
                  << ",\"fz\":" << coords[i].flux_z << "}";
        if (i + 1 < coords.size()) std::cout << ",";
        std::cout << "\n";
    }
    std::cout << indent << "]";
}

static void dump_run(const char* label, const char* description,
                     std::function<void(ftd::RenderBridge&)> setup,
                     int L, std::vector<int> sample_ticks, bool color_forces = true) {
    ftd::RenderBridge rb(L);
    if (color_forces) rb.toggles.color_forces = true;
    rb.toggles.langevin_seed = 1;

    setup(rb);

    std::cout << "    \"" << label << "\": {\n";
    std::cout << "      \"L\": " << L << ", \"description\": \"" << description << "\",\n";
    std::cout << "      \"snapshots\": [\n";

    bool first = true;
    for (int target : sample_ticks) {
        while (rb.current_tick() < target) rb.tick();
        auto coords = get_manifested(rb);
        if (!first) std::cout << ",\n";
        std::cout << "        { \"tick\": " << target
                  << ", \"n_manifested\": " << coords.size() << ",\n"
                  << "          \"coords\":";
        dump_coords_json(coords, "          ");
        std::cout << "\n        }";
        first = false;
    }
    std::cout << "\n      ]\n";
    std::cout << "    }";
}

int main() {
    std::cerr << "[quark-viz] Generating quark/color data ..." << std::endl;

    std::cout << "{\n";
    std::cout << "  \"meta\": {\n";
    std::cout << "    \"K_GENESIS\": " << ftd::K_GENESIS << ",\n";
    std::cout << "    \"color_legend\": {\"0\":\"colorless\",\"1\":\"red\",\"2\":\"green\",\"3\":\"blue\"},\n";
    std::cout << "    \"state_legend\": {\"-1\":\"antimatter\",\"0\":\"void\",\"+1\":\"matter\"}\n";
    std::cout << "  },\n";

    const int L = 32;
    const int c = L / 2;
    const double A = 5.0 * ftd::K_GENESIS;
    std::vector<int> ticks_short = {0, 30, 100, 300};

    std::cout << "  \"experiments\": {\n";

    // Experiment 1: Pure x-flux at center → RED quark
    dump_run("red_quark", "Single quark from pure +x flux injection",
             [&](ftd::RenderBridge& rb) {
                 rb.inject_flux(c, c, c, {A, 0.0, 0.0});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 2: Pure y-flux → GREEN quark
    dump_run("green_quark", "Single quark from pure +y flux injection",
             [&](ftd::RenderBridge& rb) {
                 rb.inject_flux(c, c, c, {0.0, A, 0.0});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 3: Pure z-flux → BLUE quark
    dump_run("blue_quark", "Single quark from pure +z flux injection",
             [&](ftd::RenderBridge& rb) {
                 rb.inject_flux(c, c, c, {0.0, 0.0, A});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 4: Diagonal flux (45° in xy) → which color wins?
    dump_run("diagonal_xy",
             "Diagonal +x+y flux: dominant axis competition (R vs G)",
             [&](ftd::RenderBridge& rb) {
                 double a = A / std::sqrt(2.0);
                 rb.inject_flux(c, c, c, {a, a, 0.0});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 5: Symmetric 3-axis (R=G=B amplitude) → all three?
    dump_run("rgb_symmetric",
             "Symmetric flux in x=y=z directions (color tie)",
             [&](ftd::RenderBridge& rb) {
                 double a = A / std::sqrt(3.0);
                 rb.inject_flux(c, c, c, {a, a, a});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 6: Quark + Antiquark "meson"
    // Same flux direction, opposite states. Use state=+1 R quark and state=-1 R antiquark.
    dump_run("meson_RRbar",
             "Quark-antiquark pair (R q at +x, R q-bar at -x, separated by 6 voxels)",
             [&](ftd::RenderBridge& rb) {
                 // R-quark at center+3
                 rb.inject_flux(c+3, c, c, {A, 0.0, 0.0});
                 // R-antiquark at center-3 (negative state via inject_particle)
                 rb.inject_particle(c-3, c, c, -1, {A, 0.0, 0.0});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 7: Three-quark "baryon" R+G+B at triangle
    dump_run("baryon_RGB",
             "Three-quark configuration: R, G, B at triangle vertices",
             [&](ftd::RenderBridge& rb) {
                 // R at +x
                 rb.inject_flux(c+3, c, c, {A, 0.0, 0.0});
                 // G at +y
                 rb.inject_flux(c, c+3, c, {0.0, A, 0.0});
                 // B at +z
                 rb.inject_flux(c, c, c+3, {0.0, 0.0, A});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 8: Same as 7 but with confinement toggle
    dump_run("baryon_RGB_with_strong",
             "Three-quark + strong_force toggle (Yukawa nuclear force)",
             [&](ftd::RenderBridge& rb) {
                 rb.toggles.strong_force = true;
                 rb.inject_flux(c+3, c, c, {A, 0.0, 0.0});
                 rb.inject_flux(c, c+3, c, {0.0, A, 0.0});
                 rb.inject_flux(c, c, c+3, {0.0, 0.0, A});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 9: Two same-color quarks (R+R) — should they repel? attract?
    dump_run("two_R_quarks",
             "Two same-color quarks (R+R, separated by 6 voxels in x)",
             [&](ftd::RenderBridge& rb) {
                 rb.inject_flux(c+3, c, c, {A, 0.0, 0.0});
                 rb.inject_flux(c-3, c, c, {A, 0.0, 0.0});
             }, L, ticks_short);
    std::cout << ",\n";

    // Experiment 10: NO color_forces, just defaults — does color still get assigned?
    dump_run("baryon_RGB_no_color_forces",
             "Three-quark RGB but with color_forces toggle OFF (defaults only)",
             [&](ftd::RenderBridge& rb) {
                 rb.inject_flux(c+3, c, c, {A, 0.0, 0.0});
                 rb.inject_flux(c, c+3, c, {0.0, A, 0.0});
                 rb.inject_flux(c, c, c+3, {0.0, 0.0, A});
             }, L, ticks_short, /*color_forces=*/false);

    std::cout << "\n  }\n";
    std::cout << "}\n";
    std::cerr << "[quark-viz] DONE" << std::endl;
    return 0;
}
