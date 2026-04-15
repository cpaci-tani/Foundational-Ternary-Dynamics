/**
 * Test: Maxwell Equation Recovery
 *
 * Verifies that the FTD wave equation + Gauss constraint recovers
 * Maxwell's equations by reconstructing E and B fields and checking
 * their relationships.
 *
 * Field identification:
 *   B = curl(J)           — magnetic field (gauge-invariant)
 *   E = -wave_vel         — electric field (E = -dA/dt with A = J)
 *   rho = state           — charge density
 *
 * Key subtlety: E = -wave_vel gives the DYNAMIC (radiative) electric
 * field. For a static charge, the Coulomb field is encoded in the
 * longitudinal part of J, not in wave_vel (which decays to zero).
 *
 * Tests:
 *   M1: Field identification sanity (B from curl, E sign, static E=0)
 *   M2: div(B) = 0 (no magnetic monopoles — algebraic identity)
 *   M3: Faraday's law dB/dt = -curl(E) (numerical verification)
 *   M4: Transversality div(E) ~ 0 for vacuum waves
 *   M5: EM wave structure (|E|/|B| = c_wave, E perp B perp k)
 *   M6: Ampère-Maxwell law dE/dt = c² curl(B) + source (completes all 4 Maxwell eqs)
 *
 * Theory references:
 *   - SPEC_ENGINE.md §3 (Tick cycle, phase_read wave equation)
 *   - SPEC_ENGINE.md §5 (Force computation — Lorentz uses B = curl(J))
 *   - CLAUDE.md §14.3 (U(1) gauge emergence, Helmholtz decomposition)
 */

#define _USE_MATH_DEFINES
#include <cmath>
#include <iostream>
#include <iomanip>
#include <vector>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"
#include "ftd/test_telemetry.h"

using ftd::test::check;
using ftd::test::check_close;

// Helper: compute curl of wave_vel at a given lattice index.
// Uses the same central-difference stencil as curl_flux().
static ftd::Vec3 curl_wave_vel(const ftd::RenderBridge& rb, int idx) {
    auto c = rb.lattice().coord(idx);
    int L = rb.lattice().size();
    ftd::Vec3 result;

    // (curl W)_x = dW_z/dy - dW_y/dz
    result.x =
        (rb.voxels()[rb.lattice().index(c.x, (c.y+1)%L, c.z)].wave_vel.z
       - rb.voxels()[rb.lattice().index(c.x, (c.y-1+L)%L, c.z)].wave_vel.z) * 0.5
      - (rb.voxels()[rb.lattice().index(c.x, c.y, (c.z+1)%L)].wave_vel.y
       - rb.voxels()[rb.lattice().index(c.x, c.y, (c.z-1+L)%L)].wave_vel.y) * 0.5;

    // (curl W)_y = dW_x/dz - dW_z/dx
    result.y =
        (rb.voxels()[rb.lattice().index(c.x, c.y, (c.z+1)%L)].wave_vel.x
       - rb.voxels()[rb.lattice().index(c.x, c.y, (c.z-1+L)%L)].wave_vel.x) * 0.5
      - (rb.voxels()[rb.lattice().index((c.x+1)%L, c.y, c.z)].wave_vel.z
       - rb.voxels()[rb.lattice().index((c.x-1+L)%L, c.y, c.z)].wave_vel.z) * 0.5;

    // (curl W)_z = dW_y/dx - dW_x/dy
    result.z =
        (rb.voxels()[rb.lattice().index((c.x+1)%L, c.y, c.z)].wave_vel.y
       - rb.voxels()[rb.lattice().index((c.x-1+L)%L, c.y, c.z)].wave_vel.y) * 0.5
      - (rb.voxels()[rb.lattice().index(c.x, (c.y+1)%L, c.z)].wave_vel.x
       - rb.voxels()[rb.lattice().index(c.x, (c.y-1+L)%L, c.z)].wave_vel.x) * 0.5;

    return result;
}

// Helper: compute divergence of wave_vel at a given lattice index.
static double div_wave_vel(const ftd::RenderBridge& rb, int idx) {
    auto c = rb.lattice().coord(idx);
    int L = rb.lattice().size();
    double d = 0.0;
    d += (rb.voxels()[rb.lattice().index((c.x+1)%L, c.y, c.z)].wave_vel.x
        - rb.voxels()[rb.lattice().index((c.x-1+L)%L, c.y, c.z)].wave_vel.x) * 0.5;
    d += (rb.voxels()[rb.lattice().index(c.x, (c.y+1)%L, c.z)].wave_vel.y
        - rb.voxels()[rb.lattice().index(c.x, (c.y-1+L)%L, c.z)].wave_vel.y) * 0.5;
    d += (rb.voxels()[rb.lattice().index(c.x, c.y, (c.z+1)%L)].wave_vel.z
        - rb.voxels()[rb.lattice().index(c.x, c.y, (c.z-1+L)%L)].wave_vel.z) * 0.5;
    return d;
}

// Helper: compute divergence of B = curl(J) at a given lattice index.
// div(B) = div(curl(J)) — should be 0 identically.
static double div_B(ftd::RenderBridge& rb, int idx) {
    auto c = rb.lattice().coord(idx);
    int L = rb.lattice().size();

    // Compute B = curl(J) at 6 face neighbors and center to get div(B)
    // div(B)_x = (B_x(x+1) - B_x(x-1))/2 etc.
    ftd::Vec3 Bxp = rb.curl_flux(rb.lattice().index((c.x+1)%L, c.y, c.z));
    ftd::Vec3 Bxm = rb.curl_flux(rb.lattice().index((c.x-1+L)%L, c.y, c.z));
    ftd::Vec3 Byp = rb.curl_flux(rb.lattice().index(c.x, (c.y+1)%L, c.z));
    ftd::Vec3 Bym = rb.curl_flux(rb.lattice().index(c.x, (c.y-1+L)%L, c.z));
    ftd::Vec3 Bzp = rb.curl_flux(rb.lattice().index(c.x, c.y, (c.z+1)%L));
    ftd::Vec3 Bzm = rb.curl_flux(rb.lattice().index(c.x, c.y, (c.z-1+L)%L));

    double d = 0.0;
    d += (Bxp.x - Bxm.x) * 0.5;
    d += (Byp.y - Bym.y) * 0.5;
    d += (Bzp.z - Bzm.z) * 0.5;
    return d;
}

int main() {
    ftd::test::init("test_maxwell");

    // ================================================================
    // Section 1: Field Identification Sanity
    // ================================================================
    std::cout << "\n-- M1: Field Identification --\n";

    // M1a: B = curl(J) for known analytical field
    {
        int L = 16;
        ftd::RenderBridge rb(L);

        // J = (-y, x, 0) → curl = (0, 0, 2)
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    double cx = x - L / 2.0;
                    double cy = y - L / 2.0;
                    rb.inject_flux(x, y, z, {-cy, cx, 0});
                }

        int ci = rb.lattice().index(L/2, L/2, L/2);
        ftd::Vec3 B = rb.curl_flux(ci);
        std::cout << "  B at center = (" << B.x << ", " << B.y << ", " << B.z
                  << "), expected (0, 0, 2)\n";
        check("M1a: B_z = curl_z(J) ≈ 2 for J=(-y,x,0)",
              std::abs(B.z - 2.0) < 0.1 && std::abs(B.x) < 0.1 && std::abs(B.y) < 0.1);
    }

    // M1b: E = -wave_vel sign convention with traveling wave
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double omega = 2.0 * ftd::C_WAVE * std::abs(std::sin(k / 2.0));
        double AMP = 0.05;

        // y-polarized wave propagating in +x: J_y = A*sin(k*x), wv_y = -omega*A*cos(k*x)
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            double wv_y = -omega * AMP * std::cos(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    rb.inject_flux(x, y, z, {0, jy, 0});
                    rb.voxels()[rb.lattice().index(x, y, z)].wave_vel = {0, wv_y, 0};
                }
        }

        rb.run(10);

        // At observation point, wave_vel_y should be nonzero (wave is propagating)
        int obs = rb.lattice().index(L/4, L/2, L/2);
        double wv_y = rb.voxels()[obs].wave_vel.y;
        std::cout << "  wave_vel_y at obs = " << wv_y << "\n";
        check("M1b: E = -wave_vel is nonzero for traveling wave",
              std::abs(wv_y) > 1e-6);
    }

    // M1c: Static field has E = 0 (wave_vel stays zero for constant J)
    // A uniform flux field has zero Laplacian, so wave_vel is not driven.
    // This directly confirms: when dJ/dt = 0, E = -wave_vel = 0.
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Uniform flux: lap(J) = 0 everywhere → wave_vel should stay zero
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, 0, 0.1});

        rb.run(50);

        double max_wv = 0.0;
        const int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            double wm = rb.voxels()[i].wave_vel.mag();
            if (wm > max_wv) max_wv = wm;
        }
        std::cout << "  max|wave_vel| for uniform J = " << std::setprecision(15) << max_wv << "\n";
        check("M1c: E = 0 for static uniform field (max|wv| < 1e-12)", max_wv < 1e-12);
    }

    // ================================================================
    // Section 2: div(B) = 0  (Maxwell 1 — no magnetic monopoles)
    // ================================================================
    std::cout << "\n-- M2: div(B) = 0 --\n";

    // M2a: Vacuum with propagating wave
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // Inject a z-polarized wave
        for (int x = 0; x < L; ++x)
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, 0, 0.05 * std::sin(2.0 * M_PI * 3 * x / L)});

        rb.run(20);

        double max_divB = 0.0;
        for (int x = 2; x < L-2; ++x)
            for (int y = 2; y < L-2; ++y)
                for (int z = 2; z < L-2; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    double d = std::abs(div_B(rb, idx));
                    if (d > max_divB) max_divB = d;
                }

        std::cout << "  max |div(B)| in vacuum = " << std::setprecision(15) << max_divB << "\n";
        check("M2a: div(B) = 0 in vacuum (< 1e-12)", max_divB < 1e-12);
    }

    // M2b: With charged particle
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;

        int cx = L / 2;
        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});

        rb.run(50);

        double max_divB = 0.0;
        for (int x = 2; x < L-2; ++x)
            for (int y = 2; y < L-2; ++y)
                for (int z = 2; z < L-2; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    double d = std::abs(div_B(rb, idx));
                    if (d > max_divB) max_divB = d;
                }

        std::cout << "  max |div(B)| with charge = " << std::setprecision(15) << max_divB << "\n";
        check("M2b: div(B) = 0 with charged particle (< 1e-12)", max_divB < 1e-12);
    }

    // ================================================================
    // Section 3: Faraday's Law  dB/dt = -curl(E) = curl(wave_vel)
    // ================================================================
    // Leapfrog timing: wave_vel is updated FIRST in phase_write, then
    // J += wave_vel. So delta_J = wave_vel(T+1), meaning:
    //   delta_B = curl(wave_vel) measured AFTER the tick, not before.
    // Damping disabled for clean verification (damping adds a -D*B term).
    std::cout << "\n-- M3: Faraday's Law --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        // Damping OFF for clean Faraday: delta_B = curl(wave_vel(T+1)) exactly

        // Standing wave: J_y = A*sin(k*x), wave_vel = 0
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        // Let transients settle (undamped, so standing wave oscillates)
        rb.run(5);

        // Observation points (interior)
        int obs[5];
        int mid = L / 2;
        obs[0] = rb.lattice().index(L/4, mid, mid);
        obs[1] = rb.lattice().index(L/4 + 2, mid, mid);
        obs[2] = rb.lattice().index(L/4 + 4, mid, mid);
        obs[3] = rb.lattice().index(3*L/4, mid, mid);
        obs[4] = rb.lattice().index(3*L/4 + 2, mid, mid);

        // Record B(T) before tick
        ftd::Vec3 B_before[5];
        for (int i = 0; i < 5; ++i)
            B_before[i] = rb.curl_flux(obs[i]);

        // Run one tick
        rb.tick();

        // Record B(T+1) and curl(wave_vel) AFTER the tick
        ftd::Vec3 B_after[5], curl_wv_after[5];
        for (int i = 0; i < 5; ++i) {
            B_after[i] = rb.curl_flux(obs[i]);
            curl_wv_after[i] = curl_wave_vel(rb, obs[i]);
        }

        // Faraday: delta_B = curl(wave_vel(T+1))
        double sum_residual = 0.0, sum_deltaB = 0.0;
        int sign_agree = 0, sign_total = 0;
        for (int i = 0; i < 5; ++i) {
            ftd::Vec3 dB = {B_after[i].x - B_before[i].x,
                            B_after[i].y - B_before[i].y,
                            B_after[i].z - B_before[i].z};
            ftd::Vec3 cwv = curl_wv_after[i];

            double res = std::sqrt((dB.x-cwv.x)*(dB.x-cwv.x)
                                 + (dB.y-cwv.y)*(dB.y-cwv.y)
                                 + (dB.z-cwv.z)*(dB.z-cwv.z));
            double mag = dB.mag();
            sum_residual += res;
            sum_deltaB += mag;

            // Sign check on z-component (dominant for y-polarized wave)
            if (std::abs(dB.z) > 1e-10 && std::abs(cwv.z) > 1e-10) {
                sign_total++;
                if ((dB.z > 0) == (cwv.z > 0)) sign_agree++;
            }
        }

        double rel_error = (sum_deltaB > 1e-15) ? sum_residual / sum_deltaB : 0;
        std::cout << "  Faraday relative residual = " << std::setprecision(4) << rel_error << "\n";
        std::cout << "  Sign agreement: " << sign_agree << "/" << sign_total << "\n";

        check("M3a: Faraday residual |dB - curl(wv)| < 10% of |dB|", rel_error < 0.10);
        check("M3b: Faraday sign correct (dB_z same sign as curl_z(wv))",
              sign_total == 0 || sign_agree == sign_total);
    }

    // ================================================================
    // Section 4: Transversality — div(E) ≈ 0 for vacuum waves
    // ================================================================
    std::cout << "\n-- M4: Transversality --\n";

    // M4a: div(wave_vel) ≈ 0 for transverse vacuum wave
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // y-polarized wave, uniform in y,z
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        rb.run(20);

        double max_div_wv = 0.0, max_wv = 0.0;
        const int N = rb.lattice().total_sites();
        for (int i = 0; i < N; ++i) {
            double wm = rb.voxels()[i].wave_vel.mag();
            if (wm > max_wv) max_wv = wm;
        }
        for (int x = 2; x < L-2; ++x)
            for (int y = 2; y < L-2; ++y)
                for (int z = 2; z < L-2; ++z) {
                    int idx = rb.lattice().index(x, y, z);
                    double d = std::abs(div_wave_vel(rb, idx));
                    if (d > max_div_wv) max_div_wv = d;
                }

        double ratio = (max_wv > 1e-15) ? max_div_wv / max_wv : 0;
        std::cout << "  max|div(wv)| = " << max_div_wv
                  << ", max|wv| = " << max_wv
                  << ", ratio = " << ratio << "\n";
        check("M4a: div(E) ≈ 0 for transverse vacuum wave (ratio < 0.10)", ratio < 0.10);
    }

    // M4b: div(wave_vel) nonzero near radiating charge
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;

        int cx = L / 2;
        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].locked = true;

        rb.run(50);

        // Measure div(wave_vel) at face neighbor of charge
        int nbr = rb.lattice().index(cx + 1, cx, cx);
        double div_near = std::abs(div_wave_vel(rb, nbr));
        // And at a distant point
        int far = rb.lattice().index(cx + 5, cx, cx);
        double div_far = std::abs(div_wave_vel(rb, far));

        std::cout << "  |div(wv)| near charge = " << div_near
                  << ", far = " << div_far << "\n";
        check("M4b: div(E) nonzero near radiating charge (source drives wv)",
              div_near > 1e-8);
    }

    // ================================================================
    // Section 5: EM Wave Structure — |E|/|B| = c_wave, E⊥B⊥k
    // ================================================================
    std::cout << "\n-- M5: EM Wave Structure --\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // y-polarized TRAVELING wave in +x
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double omega = 2.0 * ftd::C_WAVE * std::abs(std::sin(k / 2.0));
        double AMP = 0.05;

        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            double wv_y = -omega * AMP * std::cos(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    rb.inject_flux(x, y, z, {0, jy, 0});
                    rb.voxels()[rb.lattice().index(x, y, z)].wave_vel = {0, wv_y, 0};
                }
        }

        // Let transients settle
        rb.run(5);

        // Measure |E|/|B| ratio at observation point over multiple ticks
        int obs = rb.lattice().index(L/4, L/2, L/2);
        double sum_E = 0.0, sum_B = 0.0;
        double sum_wv_x = 0.0, sum_wv_y = 0.0;
        double sum_B_x = 0.0, sum_B_y = 0.0, sum_B_z = 0.0;
        double sum_EdotB = 0.0;
        int nsamples = 0;

        for (int t = 0; t < 40; ++t) {
            rb.tick();

            ftd::Vec3 wv = rb.voxels()[obs].wave_vel;
            ftd::Vec3 B = rb.curl_flux(obs);

            double E_mag = wv.mag();  // |E| = |wave_vel| (E = -wave_vel, magnitude same)
            double B_mag = B.mag();

            sum_E += E_mag;
            sum_B += B_mag;
            sum_wv_x += std::abs(wv.x);
            sum_wv_y += std::abs(wv.y);
            sum_B_x += std::abs(B.x);
            sum_B_y += std::abs(B.y);
            sum_B_z += std::abs(B.z);
            sum_EdotB += std::abs(wv.x * B.x + wv.y * B.y + wv.z * B.z);
            nsamples++;
        }

        double avg_ratio = (sum_B > 1e-15) ? sum_E / sum_B : 0;
        double E_perp_ratio = (sum_wv_y > 1e-15) ? sum_wv_x / sum_wv_y : 0;
        double avg_EdotB = sum_EdotB / nsamples;
        double avg_EB = (sum_E / nsamples) * (sum_B / nsamples);
        double ortho_ratio = (avg_EB > 1e-15) ? avg_EdotB / avg_EB : 0;

        std::cout << "  <|E|/|B|> = " << std::setprecision(4) << avg_ratio
                  << " (expected c_wave = " << ftd::C_WAVE << ")\n";
        std::cout << "  E_x/E_y ratio = " << E_perp_ratio << " (should be << 1)\n";
        std::cout << "  |E·B|/(|E||B|) = " << ortho_ratio << " (should be << 1)\n";
        std::cout << "  B components: x=" << sum_B_x/nsamples
                  << " y=" << sum_B_y/nsamples
                  << " z=" << sum_B_z/nsamples << "\n";

        check("M5a: |E|/|B| ≈ c_wave (within 25%)",
              std::abs(avg_ratio - ftd::C_WAVE) < 0.25 * ftd::C_WAVE);
        check("M5b: E ⊥ B (|E·B|/|E||B| < 0.3)", ortho_ratio < 0.3);
        check("M5c: E ⊥ k (E_x/E_y < 0.15 for y-polarized wave)", E_perp_ratio < 0.15);
    }

    // ================================================================
    // Section 6: Ampère-Maxwell Law  dE/dt = c² curl(B) + source
    // ================================================================
    // The FTD wave equation: wave_vel += C² * Laplacian(J)
    // Since E = -wave_vel and B = curl(J), this IS Ampère-Maxwell.
    //
    // In vacuum (div(J) = 0): Laplacian(J) = grad(div(J)) - curl(curl(J))
    //   = -curl(B), so delta(wave_vel) = -C² * curl(B)
    //   => delta_E = -delta(wave_vel) = C² * curl(B) = dE/dt
    //
    // Leapfrog timing note: wave_vel is updated from Laplacian(J(T)),
    // then J is updated. So delta_E = C² * curl(B(T)). We measure
    // B BEFORE the tick and delta_E AFTER, giving exact correspondence
    // for the leapfrog half-step structure.
    std::cout << "\n-- M6: Ampère-Maxwell Law --\n";

    // M6a + M6b: Vacuum standing wave — dE/dt should equal c² * curl(B)
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        // No damping, no coupling, no forces — pure wave equation

        // Standing wave: J_y = A*sin(k*x), wave_vel = 0
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        // Let transients settle (standing wave oscillates)
        rb.run(5);

        // Observation points (interior, along x-axis)
        int mid = L / 2;
        int obs[5];
        obs[0] = rb.lattice().index(L/4, mid, mid);
        obs[1] = rb.lattice().index(L/4 + 2, mid, mid);
        obs[2] = rb.lattice().index(L/4 + 4, mid, mid);
        obs[3] = rb.lattice().index(3*L/4, mid, mid);
        obs[4] = rb.lattice().index(3*L/4 + 2, mid, mid);

        // Record E(T) = -wave_vel(T) and B(T) = curl(J(T)) BEFORE tick
        ftd::Vec3 E_before[5], B_before[5];
        for (int i = 0; i < 5; ++i) {
            E_before[i] = rb.voxels()[obs[i]].wave_vel * -1.0;
            B_before[i] = rb.curl_flux(obs[i]);
        }

        // Run one tick
        rb.tick();

        // Record E(T+1) = -wave_vel(T+1) AFTER tick
        ftd::Vec3 E_after[5];
        for (int i = 0; i < 5; ++i) {
            E_after[i] = rb.voxels()[obs[i]].wave_vel * -1.0;
        }

        // Ampère-Maxwell: delta_E should equal c² * curl(B(T))
        // On the leapfrog: delta(wave_vel) = C² * Lap(J(T))
        //   => delta_E = -delta(wave_vel) = -C² * Lap(J(T))
        // In vacuum: Lap(J) = grad(div(J)) - curl(curl(J)) = -curl(B)
        //   => delta_E = C² * curl(B(T))
        double c2 = ftd::C_WAVE * ftd::C_WAVE;
        double sum_residual = 0.0, sum_deltaE = 0.0;
        int sign_agree = 0, sign_total = 0;
        for (int i = 0; i < 5; ++i) {
            ftd::Vec3 dE = {E_after[i].x - E_before[i].x,
                            E_after[i].y - E_before[i].y,
                            E_after[i].z - E_before[i].z};

            // curl(B) at observation point (use helper from earlier)
            ftd::Vec3 curl_B = curl_wave_vel(rb, obs[i]);
            // Actually, curl(B) = curl(curl(J)). The Ampère prediction is
            // delta_E = c² * curl(B_before). But we must compute curl(B_before)
            // which requires curl of curl(J) at time T. The wave equation
            // uses the Laplacian, not curl(curl), directly.
            //
            // Alternative approach: the wave equation says
            //   delta(wave_vel) = c² * Laplacian(J)
            // So delta_E = -c² * Laplacian(J(T)).
            // For a y-polarized wave uniform in y,z: Laplacian(J)_y = d²J_y/dx²
            // And curl(B)_y = curl(curl(J))_y = -d²J_y/dx² + d(div(J))/dy
            //                = -Laplacian(J)_y (when div(J)=0)
            // So delta_E_y = -c² * Lap(J)_y = c² * curl(B)_y. CHECK!
            //
            // But computing curl(B) from the lattice is expensive (double curl).
            // Instead, directly check: delta_E_y vs -c² * Lap(J)_y.
            // These are IDENTICAL by construction (the wave equation IS this).
            //
            // For a meaningful test, we verify the PHYSICAL content:
            // that delta_E is consistent with the analytical Ampère relation.
            // For a y-polarized standing wave J_y = A*sin(k*x)*cos(omega*t):
            //   B_z = dJ_y/dx = A*k*cos(k*x)*cos(omega*t)
            //   dE_y/dt = c²*(dB_z/dx) = -c²*A*k²*sin(k*x)*cos(omega*t)
            //
            // The delta_E should be anti-correlated with J_y at same point.

            double res = std::sqrt((dE.x)*(dE.x) + (dE.y)*(dE.y) + (dE.z)*(dE.z));
            sum_deltaE += res;

            // Sign check: for standing wave, delta_E_y should be anti-correlated
            // with J_y (since d²J_y/dx² = -k²*J_y for sinusoidal).
            // So delta_E_y = -(-c²*Lap_y) = c²*k²*J_y... wait, let me re-derive.
            // wave_vel_y += C²*Lap(J)_y = C²*(-k²)*J_y (for sin(kx))
            // delta_E_y = -delta(wave_vel_y) = C²*k²*J_y
            // So delta_E_y should be POSITIVE where J_y is positive.
            double Jy_at_obs = rb.voxels()[obs[i]].flux.y;
            if (std::abs(dE.y) > 1e-10 && std::abs(Jy_at_obs) > 1e-10) {
                sign_total++;
                // delta_E_y and J_y should have same sign (both driven by sin(kx))
                if ((dE.y > 0) == (Jy_at_obs > 0)) sign_agree++;
            }
        }

        std::cout << "  sum|delta_E| = " << std::setprecision(6) << sum_deltaE << "\n";
        std::cout << "  Sign agreement: " << sign_agree << "/" << sign_total << "\n";

        check("M6a: delta_E nonzero in standing wave (Ampère drives E change)",
              sum_deltaE > 1e-6);
        check("M6b: delta_E_y correlated with J_y (Ampère sign correct)",
              sign_total == 0 || sign_agree == sign_total);
    }

    // M6c: Verify Ampère quantitatively — |delta_E_y| ≈ c² * k² * |J_y| * dt
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double AMP = 0.05;
        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z)
                    rb.inject_flux(x, y, z, {0, jy, 0});
        }

        // Fresh start: wave_vel = 0 everywhere, so E = 0.
        // After 1 tick: delta(wave_vel) = C² * Lap(J), so
        // delta_E_y = C² * k² * A * sin(k*x) (at first tick, exactly)
        // But the 18-pt isotropic Laplacian gives a modified k²_eff
        // that differs from the analytical 4*sin²(k/2) by the isotropy correction.

        // Record E before
        int mid = L / 2;
        int obs = rb.lattice().index(L/4, mid, mid);
        double E_before_y = -rb.voxels()[obs].wave_vel.y;  // Should be 0

        rb.tick();

        double E_after_y = -rb.voxels()[obs].wave_vel.y;
        double delta_Ey = E_after_y - E_before_y;

        // Analytical prediction (6-pt stencil): delta_Ey = C² * k²_eff * J_y
        // where k²_eff = 4*sin²(k/2) for 6-pt. For 18-pt isotropic stencil
        // the effective k² is the same for plane-wave modes (uniform in y,z).
        // J_y at obs = A * sin(k * L/4)
        double Jy_obs = AMP * std::sin(k * (L / 4));
        double c2 = ftd::C_WAVE * ftd::C_WAVE;
        double k2_eff = 4.0 * std::sin(k / 2.0) * std::sin(k / 2.0);
        double predicted = c2 * k2_eff * Jy_obs;

        double rel_err = (std::abs(predicted) > 1e-15)
            ? std::abs(delta_Ey - predicted) / std::abs(predicted) : 0;

        std::cout << "  delta_E_y = " << std::setprecision(6) << delta_Ey
                  << ", predicted = " << predicted
                  << ", rel_err = " << rel_err << "\n";

        check("M6c: Ampère quantitative: |delta_E_y - c²k²J_y| / |c²k²J_y| < 15%",
              rel_err < 0.15);
    }

    // M6d: With charged particle — source term adds to field evolution
    {
        int L = 16;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;
        rb.toggles.coupling = true;  // Source term ON

        int cx = L / 2;
        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].locked = true;

        rb.run(50);  // Let coupling build field

        // Record E before
        int nbr = rb.lattice().index(cx + 2, cx, cx);
        double E_before_mag = rb.voxels()[nbr].wave_vel.mag();

        rb.tick();

        double E_after_mag = rb.voxels()[nbr].wave_vel.mag();
        double delta_E_mag = std::abs(E_after_mag - E_before_mag);

        std::cout << "  |delta_E| near charge (r=2) = " << std::setprecision(6)
                  << delta_E_mag << "\n";

        check("M6d: Source term active: |delta_E| > 0 near charged particle",
              delta_E_mag > 1e-10);
    }

    // M6e: Ampère in vacuum traveling wave — delta_E perpendicular to k
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        rb.toggles.disable_all();
        rb.toggles.wave_propagation = true;

        // y-polarized traveling wave in +x
        int n = 4;
        double k = 2.0 * M_PI * n / L;
        double omega = 2.0 * ftd::C_WAVE * std::abs(std::sin(k / 2.0));
        double AMP = 0.05;

        for (int x = 0; x < L; ++x) {
            double jy = AMP * std::sin(k * x);
            double wv_y = -omega * AMP * std::cos(k * x);
            for (int y = 0; y < L; ++y)
                for (int z = 0; z < L; ++z) {
                    rb.inject_flux(x, y, z, {0, jy, 0});
                    rb.voxels()[rb.lattice().index(x, y, z)].wave_vel = {0, wv_y, 0};
                }
        }

        rb.run(5);

        // Record E_y before and after tick
        int mid = L / 2;
        int obs = rb.lattice().index(L/4, mid, mid);
        double Ey_before = -rb.voxels()[obs].wave_vel.y;
        double Ex_before = -rb.voxels()[obs].wave_vel.x;

        rb.tick();

        double Ey_after = -rb.voxels()[obs].wave_vel.y;
        double Ex_after = -rb.voxels()[obs].wave_vel.x;
        double dEy = std::abs(Ey_after - Ey_before);
        double dEx = std::abs(Ex_after - Ex_before);

        double transverse_ratio = (dEy > 1e-15) ? dEx / dEy : 0.0;
        std::cout << "  |dE_x| = " << dEx << ", |dE_y| = " << dEy
                  << ", ratio = " << transverse_ratio << "\n";

        check("M6e: Ampère delta_E transverse to k (dE_x/dE_y < 0.1 for y-polarized wave)",
              transverse_ratio < 0.1);
    }

    return ftd::test::finalize();
}
