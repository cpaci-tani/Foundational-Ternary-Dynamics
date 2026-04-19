// ==========================================================================
//  engine/src/scenarios/light.cpp
//
//  Group: light-* (4 scenarios)
//  JS source: engine/web/js/bridge/scenarios/light-scenarios.js
//
//  Split out of engine/src/scenarios.cpp (ticket S1).
// ==========================================================================

#include "ftd/scenarios.h"
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/voxel.h"

#include "_helpers.h"

#include <cmath>

namespace ftd {

bool setup_light_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("light-", 0) != 0) return false;
    const int    N     = rb.lattice().size();
    const int    mid   = N / 2;
    const double C_WAVE = 1.0 / std::sqrt(3.0);
    const double amp   = 0.15;

    if (name == "light-rainbow") {
        struct W { int n; int pol; };
        const W waves[3] = { {1,1}, {3,2}, {6,0} };
        for (int w = 0; w < 3; w++) {
            double k = 2.0 * SCN_PI * waves[w].n / N;
            int pol = waves[w].pol;
            for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
                double J_val  = amp * std::sin(k * x);
                double wv_val = -2.0 * C_WAVE * std::sin(k / 2.0) * amp * std::cos(k * x);
                double fv[3] = {0,0,0}, wvv[3] = {0,0,0};
                fv[pol] = J_val;
                wvv[pol] = wv_val;
                IF(rb, x, y, z, fv[0], fv[1], fv[2]);
                IW(rb, x, y, z, wvv[0], wvv[1], wvv[2]);
            }
        }
    }
    else if (name == "light-dipole") {
        const int sigma = 3;
        const double dAmp = 0.5;
        for (int x = 0; x < N; x++) for (int y = 0; y < N; y++) for (int z = 0; z < N; z++) {
            double dx = x - mid, dy = y - mid, dz = z - mid;
            double g = dAmp * std::exp(-(dx*dx + dy*dy + dz*dz) / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            IF(rb, x, y, z, 0, 0, g);
            IW(rb, x, y, z, 0, 0, g);
        }
    }
    else if (name == "light-two-slit") {
        const int sigma = 2;
        const double sAmp = 0.3;
        const int slit_sep = N / 6;
        const int slit_x   = N / 4;
        const int slit_ys[2] = { mid - slit_sep, mid + slit_sep };
        for (int i = 0; i < 2; i++) {
            int sy = slit_ys[i];
            for (int z = 0; z < N; z++) for (int dy = -4; dy <= 4; dy++) for (int dx = -4; dx <= 4; dx++) {
                double g = sAmp * std::exp(-(dx*dx + dy*dy) / (2.0 * sigma * sigma));
                if (g < 1e-6) continue;
                int px = slit_x + dx, py = sy + dy;
                if (px < 0 || px >= N || py < 0 || py >= N) continue;
                IF(rb, px, py, z, 0, 0, g);
                IW(rb, px, py, z, g, 0, 0);
            }
        }
    }
    else if (name == "light-photon-race") {
        const int sigma = 3;
        const int x_start = N / 4;
        const double pAmps[2] = { 0.05, 0.5 };
        const int y_off[2] = { mid - N / 6, mid + N / 6 };
        for (int p = 0; p < 2; p++) {
            for (int x = 0; x < N; x++) {
                double dx = x - x_start;
                double g = pAmps[p] * std::exp(-dx * dx / (2.0 * sigma * sigma));
                if (g < 1e-8) continue;
                for (int y = y_off[p] - 2; y <= y_off[p] + 2; y++)
                for (int z = mid - 2; z <= mid + 2; z++) {
                    if (y < 0 || y >= N || z < 0 || z >= N) continue;
                    IF(rb, x, y, z, 0, 0, g);
                    IW(rb, x, y, z, 0, 0, g);
                }
            }
        }
    }
    return true;
}

}  // namespace ftd
