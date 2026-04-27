// ==========================================================================
//  engine/src/scenarios/s0_field.cpp
//
//  Group: s0-field-* (8 scenarios)
//  JS source: engine/web/js/bridge/scenarios/s0-field-scenarios.js
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

bool setup_s0_field_scenario(RenderBridge& rb, const std::string& name) {
    if (name.rfind("s0-field-", 0) != 0) return false;
    const int    N    = rb.lattice().size();
    const double midF = (N - 1) * 0.5;
    const int    mc   = RND(midF);

    if (name == "s0-field-plane-wave") {
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double phase = k * x;
            double jz = amp * std::sin(phase);
            double wz = amp * std::cos(phase) * C_SPEED;
            if (std::fabs(jz) > 1e-12 || std::fabs(wz) > 1e-12) {
                IF(rb, x, y, z, 0, 0, jz);
                IW(rb, x, y, z, wz, 0, 0);
            }
        }
    }
    else if (name == "s0-field-standing-wave") {
        const double wl  = N / 4.0;
        const double amp = K_B * 2.0;
        const double k   = 2.0 * PI / wl;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jz = amp * std::sin(k * x);
            if (std::fabs(jz) > 1e-12) IF(rb, x, y, z, 0, 0, jz);
        }
    }
    else if (name == "s0-field-uniform-e") {
        const double eMag = 0.1;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            IW(rb, x, y, z, -eMag, 0, 0);
        }
    }
    else if (name == "s0-field-uniform-b") {
        const double bMag = 0.05;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double jx = -bMag * ry / 2, jy = bMag * rx / 2;
            if (std::fabs(jx) > 1e-12 || std::fabs(jy) > 1e-12) IF(rb, x, y, z, jx, jy, 0);
        }
    }
    else if (name == "s0-field-photon-pulse") {
        const int sigma = std::max(3, N / 8);
        const double amp = K_B * 2.0;
        const double lambdaEff = 4.0 * sigma;
        const double k = 2.0 * PI / lambdaEff;
        const double cutR = 3.0 * sigma;
        const double cutR2 = cutR * cutR;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double dx = x - mc, dy = y - mc, dz = z - mc;
            double r2 = dx * dx + dy * dy + dz * dz;
            if (r2 > cutR2) continue;
            double g = std::exp(-r2 / (2.0 * sigma * sigma));
            if (g < 1e-6) continue;
            double phase = k * dx;
            double jz = amp * g * std::sin(phase);
            double wz = amp * g * std::cos(phase) * C_SPEED;
            IF(rb, x, y, z, 0, 0, jz);
            IW(rb, x, y, z, wz, 0, 0);
        }
    }
    else if (name == "s0-field-electric-dipole") {
        const int sep  = std::max(2, N / 8);
        const int half = sep / 2;
        const int px = mc + half, nx = mc - half;
        IP(rb, px, mc, mc, +1);
        IP(rb, nx, mc, mc, -1);
        const double alpha_amp = ALPHA / (4.0 * PI);
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double jx = 0, jy = 0, jz = 0;
            double dx1 = x - px, dy1 = y - mc, dz1 = z - mc;
            double r2_1 = dx1*dx1 + dy1*dy1 + dz1*dz1 + 1.0;
            double f1 = alpha_amp / r2_1;
            jx += f1 * dx1; jy += f1 * dy1; jz += f1 * dz1;
            double dx2 = x - nx, dy2 = y - mc, dz2 = z - mc;
            double r2_2 = dx2*dx2 + dy2*dy2 + dz2*dz2 + 1.0;
            double f2 = -alpha_amp / r2_2;
            jx += f2 * dx2; jy += f2 * dy2; jz += f2 * dz2;
            double mag = std::sqrt(jx*jx + jy*jy + jz*jz);
            if (mag > 1e-6) IF(rb, x, y, z, jx, jy, jz);
        }
    }
    else if (name == "s0-field-magnetic-dipole") {
        const int loopR = std::max(3, N / 8);
        const double amp = K_B;
        const int nAngles = std::max(36, loopR * 8);
        for (int i = 0; i < nAngles; i++) {
            double theta = 2.0 * PI * i / nAngles;
            int lx = RND(mc + loopR * std::cos(theta));
            int ly = RND(mc + loopR * std::sin(theta));
            double tx = -std::sin(theta) * amp;
            double ty =  std::cos(theta) * amp;
            for (int z = 0; z < N; z++) IF(rb, lx, ly, z, tx, ty, 0);
        }
    }
    else if (name == "s0-field-vortex-line") {
        const double gamma = K_B * 4.0;
        const double half = (N - 1) / 2.0;
        for (int z = 0; z < N; z++) for (int y = 0; y < N; y++) for (int x = 0; x < N; x++) {
            double rx = x - half, ry = y - half;
            double r = std::sqrt(rx * rx + ry * ry);
            if (r < 1.0) r = 1.0;
            double mag = gamma / (2.0 * PI * r);
            if (mag < 1e-6) continue;
            IF(rb, x, y, z, -mag * ry / r, mag * rx / r, 0);
        }
    }
    return true;
}

}  // namespace ftd
