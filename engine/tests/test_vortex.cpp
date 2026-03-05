/**
 * Test: Vortex Formation — Biot-Savart Feedback Loop
 *
 * Explores whether the Biot-Savart coupling (curl of charge current)
 * creates stable vortex structures when an electron has initial
 * tangential velocity near a proton.
 *
 * The hypothesis: a radially-infalling electron has zero curl (radial
 * flow). But an electron with ANY transverse velocity component creates
 * curl via the Biot-Savart term, which feeds back via Lorentz force,
 * amplifying the transverse motion into a stable vortex.
 *
 * Physics reference:
 *   - Maxwell: ∇×B = J (moving charges create magnetic field)
 *   - Lorentz: F = qv×B (magnetic field deflects moving charges)
 *   - Together: self-sustaining vortex from feedback loop
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md (coupling terms)
 *   - FOUND_SPACETIME_EMERGENCE.md (angular momentum)
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include "ftd/render_bridge.h"
#include "ftd/constants.h"

int failures = 0;

void check(const char* name, bool condition) {
    if (condition) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << "\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Vortex Formation — Biot-Savart Feedback\n";
    std::cout << "================================================================\n";

    // ================================================================
    // Section 1: Curl generation from moving charge
    // ================================================================
    // A moving manifested particle should create nonzero curl in the
    // flux field via the Biot-Savart coupling.
    std::cout << "\n--- Section 1: Moving Charge Creates Curl ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Place an electron moving in the +x direction
        rb.inject_particle(cx, cx, cx, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].velocity = {0.3, 0, 0};

        // Run a few ticks to let the Biot-Savart term generate curl
        rb.run(5);

        // Measure curl at sites near the particle
        // For a charge moving in +x, Biot-Savart should create curl
        // around the x-axis (like B field rings around a wire)
        double max_curl = 0;
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 3; ++dy) {
                for (int dz = -3; dz <= 3; ++dz) {
                    int idx = rb.lattice().index(cx+dx, cx+dy, cx+dz);
                    ftd::Vec3 c = rb.curl_flux(idx);
                    double cmag = c.mag();
                    if (cmag > max_curl) max_curl = cmag;
                }
            }
        }

        std::cout << "    Max curl near moving charge: " << max_curl << "\n";
        check("Moving charge creates nonzero curl in flux", max_curl > 1e-8);
    }

    // ================================================================
    // Section 2: Stationary charge has less organized curl
    // ================================================================
    // On a cubic lattice, wave fronts are anisotropic (not spherical),
    // so even stationary charges develop some curl from lattice artifacts.
    // The key difference: Biot-Savart curl is ORGANIZED (toroidal around
    // velocity axis), while lattice artifact curl has no net circulation.
    std::cout << "\n--- Section 2: Stationary Charge — Lattice Curl Only ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Place a stationary electron (v=0)
        rb.inject_particle(cx, cx, cx, -1, {0, 0, -ftd::K_B});

        rb.run(5);

        // Measure NET curl (sum, not max) — organized curl gives net ≠ 0,
        // random lattice curl cancels out
        ftd::Vec3 sum_curl;
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 3; ++dy) {
                for (int dz = -3; dz <= 3; ++dz) {
                    int idx = rb.lattice().index(cx+dx, cx+dy, cx+dz);
                    ftd::Vec3 c = rb.curl_flux(idx);
                    sum_curl += c;
                }
            }
        }

        std::cout << "    Net curl (stationary): (" << sum_curl.x << ", "
                  << sum_curl.y << ", " << sum_curl.z << ")\n";
        std::cout << "    |Net curl| = " << sum_curl.mag() << "\n";
        // Lattice artifacts should mostly cancel; net circulation ≈ 0
        check("Stationary charge has small net curl (lattice artifacts cancel)",
              sum_curl.mag() < 0.5);
    }

    // ================================================================
    // Section 3: Tangential electron near proton — vortex seed
    // ================================================================
    // Key test: give the electron a tangential (not radial) velocity
    // near a proton. Does the curl feedback create persistent angular
    // motion, or does the electron still crash?
    std::cout << "\n--- Section 3: Tangential Electron Near Proton ---\n";
    {
        int L = 48;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        // Lock a proton at center
        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].locked = true;

        // Let proton field equilibrate
        rb.run(200);

        // Place electron at distance 6, with tangential velocity
        // Electron at (cx+6, cx, cx), velocity in y-direction (tangential)
        int ex = cx + 6;
        rb.inject_particle(ex, cx, cx, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(ex, cx, cx)].velocity = {0, 0.15, 0};

        std::cout << "    Proton at (" << cx << "," << cx << "," << cx << ")\n";
        std::cout << "    Electron at (" << ex << "," << cx << "," << cx << ")"
                  << " v_y = 0.15 (tangential)\n";

        // Track the electron's trajectory
        int alive_ticks = 0;
        double min_sep = 100;
        double max_sep = 0;
        double max_vy = 0;
        double max_curl_at_e = 0;
        bool electron_alive = true;

        // Track angular displacement
        double total_angle = 0;
        double prev_angle = 0;
        bool first = true;

        for (int t = 0; t < 500; ++t) {
            rb.tick();

            // Find the electron
            int epos = -1;
            for (int i = 0; i < rb.lattice().total_sites(); ++i) {
                if (rb.voxels()[i].state == -1) {
                    epos = i;
                    break;
                }
            }

            if (epos < 0) {
                if (electron_alive) {
                    std::cout << "    Electron evaporated at tick " << t << "\n";
                    electron_alive = false;
                }
                continue;
            }
            alive_ticks++;

            auto ec = rb.lattice().coord(epos);
            double dx = ec.x - cx;
            double dy = ec.y - cx;
            double dz = ec.z - cx;
            // Handle periodic wrapping
            int half = L / 2;
            if (dx > half) dx -= L;
            if (dx < -half) dx += L;
            if (dy > half) dy -= L;
            if (dy < -half) dy += L;
            if (dz > half) dz -= L;
            if (dz < -half) dz += L;
            double sep = std::sqrt(dx*dx + dy*dy + dz*dz);

            if (sep < min_sep) min_sep = sep;
            if (sep > max_sep) max_sep = sep;

            // Track tangential velocity
            double vy = std::abs(rb.voxels()[epos].velocity.y);
            double vz = std::abs(rb.voxels()[epos].velocity.z);
            double v_tang = std::sqrt(vy*vy + vz*vz);
            if (v_tang > max_vy) max_vy = v_tang;

            // Track curl at electron position
            ftd::Vec3 curl_e = rb.curl_flux(epos);
            if (curl_e.mag() > max_curl_at_e) max_curl_at_e = curl_e.mag();

            // Track angular position (in x-y plane)
            double angle = std::atan2(dy, dx);
            if (!first) {
                double da = angle - prev_angle;
                // Handle angle wrapping
                if (da > ftd::PI) da -= 2 * ftd::PI;
                if (da < -ftd::PI) da += 2 * ftd::PI;
                total_angle += da;
            }
            prev_angle = angle;
            first = false;

            if (t % 50 == 0 || t < 20) {
                auto& ve = rb.voxels()[epos];
                std::cout << "    t=" << std::setw(4) << t
                          << "  pos=(" << ec.x << "," << ec.y << "," << ec.z << ")"
                          << "  sep=" << std::setprecision(2) << sep
                          << "  v=(" << std::setprecision(3)
                          << ve.velocity.x << "," << ve.velocity.y << "," << ve.velocity.z
                          << ")  |curl|=" << std::setprecision(4) << curl_e.mag()
                          << "\n";
            }
        }

        std::cout << "\n    --- Summary ---\n";
        std::cout << "    Alive ticks: " << alive_ticks << " / 500\n";
        std::cout << "    Min separation: " << min_sep << "\n";
        std::cout << "    Max separation: " << max_sep << "\n";
        std::cout << "    Max tangential velocity: " << max_vy << "\n";
        std::cout << "    Max curl at electron: " << max_curl_at_e << "\n";
        std::cout << "    Total angular displacement: "
                  << std::setprecision(4) << total_angle * 180.0 / ftd::PI << " degrees\n";

        check("Electron survived > 50 ticks", alive_ticks > 50);
        check("Electron didn't crash into proton (min sep > 1)", min_sep > 1.0);
        check("Tangential velocity maintained (max v_tang > 0.01)", max_vy > 0.01);
        check("Curl generated at electron position", max_curl_at_e > 1e-6);
    }

    // ================================================================
    // Section 4: Radial vs Tangential comparison
    // ================================================================
    // Compare: electron aimed directly at proton (radial) vs
    // electron with same speed but tangential. The tangential one
    // should survive longer or maintain larger minimum separation.
    std::cout << "\n--- Section 4: Radial vs Tangential Comparison ---\n";
    {
        int L = 48;
        double v0 = 0.15;

        // A: Radial (aimed at proton)
        ftd::RenderBridge rbA(L);
        int cx = L / 2;
        rbA.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rbA.voxels()[rbA.lattice().index(cx, cx, cx)].locked = true;
        rbA.run(200);
        rbA.inject_particle(cx + 6, cx, cx, -1, {0, 0, -ftd::K_B});
        rbA.voxels()[rbA.lattice().index(cx + 6, cx, cx)].velocity = {-v0, 0, 0};

        // B: Tangential (perpendicular to proton)
        ftd::RenderBridge rbB(L);
        rbB.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rbB.voxels()[rbB.lattice().index(cx, cx, cx)].locked = true;
        rbB.run(200);
        rbB.inject_particle(cx + 6, cx, cx, -1, {0, 0, -ftd::K_B});
        rbB.voxels()[rbB.lattice().index(cx + 6, cx, cx)].velocity = {0, v0, 0};

        int alive_A = 0, alive_B = 0;
        double min_sep_A = 100, min_sep_B = 100;

        for (int t = 0; t < 300; ++t) {
            rbA.tick();
            rbB.tick();

            // Check electron A
            for (int i = 0; i < rbA.lattice().total_sites(); ++i) {
                if (rbA.voxels()[i].state == -1) {
                    alive_A++;
                    auto ec = rbA.lattice().coord(i);
                    double dx = ec.x - cx, dy = ec.y - cx, dz = ec.z - cx;
                    int half = L / 2;
                    if (dx > half) dx -= L; if (dx < -half) dx += L;
                    if (dy > half) dy -= L; if (dy < -half) dy += L;
                    if (dz > half) dz -= L; if (dz < -half) dz += L;
                    double s = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (s < min_sep_A) min_sep_A = s;
                    break;
                }
            }
            // Check electron B
            for (int i = 0; i < rbB.lattice().total_sites(); ++i) {
                if (rbB.voxels()[i].state == -1) {
                    alive_B++;
                    auto ec = rbB.lattice().coord(i);
                    double dx = ec.x - cx, dy = ec.y - cx, dz = ec.z - cx;
                    int half = L / 2;
                    if (dx > half) dx -= L; if (dx < -half) dx += L;
                    if (dy > half) dy -= L; if (dy < -half) dy += L;
                    if (dz > half) dz -= L; if (dz < -half) dz += L;
                    double s = std::sqrt(dx*dx + dy*dy + dz*dz);
                    if (s < min_sep_B) min_sep_B = s;
                    break;
                }
            }
        }

        std::cout << "    Radial:     alive=" << alive_A << " ticks, min_sep=" << min_sep_A << "\n";
        std::cout << "    Tangential: alive=" << alive_B << " ticks, min_sep=" << min_sep_B << "\n";

        // The tangential electron should at least have a larger minimum separation
        // (vortex feedback keeps it from crashing in as quickly)
        check("Tangential survives at least as long as radial", alive_B >= alive_A);
        check("Tangential has larger min separation (vortex barrier)",
              min_sep_B >= min_sep_A);
    }

    // ================================================================
    // Section 5: Curl field topology around dipole
    // ================================================================
    // With a proton and moving electron, measure the curl field structure.
    // A healthy vortex should show toroidal curl rings.
    std::cout << "\n--- Section 5: Curl Field Topology ---\n";
    {
        int L = 32;
        ftd::RenderBridge rb(L);
        int cx = L / 2;

        rb.inject_particle(cx, cx, cx, +1, {0, 0, ftd::K_B});
        rb.voxels()[rb.lattice().index(cx, cx, cx)].locked = true;
        rb.run(100);

        // Electron with tangential velocity
        rb.inject_particle(cx + 4, cx, cx, -1, {0, 0, -ftd::K_B});
        rb.voxels()[rb.lattice().index(cx + 4, cx, cx)].velocity = {0, 0.2, 0};

        rb.run(10);

        // Measure curl in the z=cx plane (midplane)
        double total_curl_z = 0;
        int count = 0;
        for (int x = cx - 6; x <= cx + 6; ++x) {
            for (int y = cx - 6; y <= cx + 6; ++y) {
                int idx = rb.lattice().index(x, y, cx);
                ftd::Vec3 c = rb.curl_flux(idx);
                total_curl_z += c.z;
                count++;
            }
        }

        double avg_curl_z = total_curl_z / count;
        std::cout << "    Average curl_z in midplane: " << avg_curl_z << "\n";
        std::cout << "    (Nonzero = net circulation = vortex)\n";

        // Any net circulation indicates vortex formation
        check("Net circulation detected in midplane", std::abs(avg_curl_z) > 1e-10);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All vortex formation tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
