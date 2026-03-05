/**
 * Test: Discrete differential operators
 *
 * Verifies laplacian_flux(), divergence_flux(), curl_flux(),
 * gradient_density() on small lattices with known configurations.
 *
 * Theory references:
 *   - SPEC_FTD_LAGRANGIAN.md   (discrete operators, 6-neighbor stencil)
 *   - SPEC_SIX_ALGORITHMS.md   (6 algorithms on 3D grid)
 */

#include <iostream>
#include <iomanip>
#include <cmath>
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

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) < tol;
    if (ok) {
        std::cout << "  PASS  " << name << "\n";
    } else {
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15)
                  << a << ", expected " << b << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Discrete Operators\n";
    std::cout << "================================================================\n\n";

    const int N = 8;

    // ---- Laplacian of uniform flux = 0 ----
    {
        ftd::RenderBridge bridge(N);
        // Set all voxels to uniform flux (1, 0, 0)
        for (int i = 0; i < bridge.lattice().total_sites(); ++i) {
            bridge.voxels()[i].flux = {1.0, 0.0, 0.0};
        }
        // Laplacian at center should be zero
        int center = bridge.lattice().index(4, 4, 4);
        auto lap = bridge.laplacian_flux(center);
        check_close("Laplacian uniform flux: x = 0", lap.x, 0.0, 1e-12);
        check_close("Laplacian uniform flux: y = 0", lap.y, 0.0, 1e-12);
        check_close("Laplacian uniform flux: z = 0", lap.z, 0.0, 1e-12);
    }

    // ---- Laplacian of single-site spike ----
    // Set one voxel to (1,0,0), all others 0
    // Laplacian at center: sum_neighbors - 6*center = 0 - 6*(1,0,0) = (-6,0,0)
    // Laplacian of spike at center: 18-pt isotropic stencil
    // Center weight = -4, face weight = 1/3, edge weight = 1/6
    // Spike (1,0,0) at center only: lap(center) = -4*(1,0,0) = (-4,0,0)
    // Face neighbor (+x): sees center via face weight 1/3: lap = (1/3,0,0)
    {
        ftd::RenderBridge bridge(N);
        int center = bridge.lattice().index(4, 4, 4);
        bridge.voxels()[center].flux = {1.0, 0.0, 0.0};

        auto lap_center = bridge.laplacian_flux(center);
        check_close("Laplacian spike center: x = -4", lap_center.x, -4.0, 1e-12);
        check_close("Laplacian spike center: y = 0", lap_center.y, 0.0, 1e-12);

        // Check a face neighbor (+x)
        int nx = bridge.lattice().index(5, 4, 4);
        auto lap_nbr = bridge.laplacian_flux(nx);
        check_close("Laplacian spike neighbor +x: x = 1/3", lap_nbr.x, 1.0/3.0, 1e-12);
        check_close("Laplacian spike neighbor +x: y = 0", lap_nbr.y, 0.0, 1e-12);
    }

    // ---- Divergence of zero flux = 0 ----
    {
        ftd::RenderBridge bridge(N);
        // All flux is zero by default
        int center = bridge.lattice().index(4, 4, 4);
        double div = bridge.divergence_flux(center);
        check_close("Divergence of zero flux = 0", div, 0.0, 1e-12);
    }

    // ---- Divergence of radial flux ----
    // Set flux pointing outward from center at face neighbors
    {
        ftd::RenderBridge bridge(N);
        int cx = 4, cy = 4, cz = 4;
        // Set flux at +x neighbor pointing in +x direction
        bridge.voxels()[bridge.lattice().index(cx+1, cy, cz)].flux = {1.0, 0.0, 0.0};
        // Set flux at -x neighbor pointing in -x direction
        bridge.voxels()[bridge.lattice().index(cx-1, cy, cz)].flux = {-1.0, 0.0, 0.0};
        // Set flux at +y neighbor pointing in +y direction
        bridge.voxels()[bridge.lattice().index(cx, cy+1, cz)].flux = {0.0, 1.0, 0.0};
        // Set flux at -y neighbor pointing in -y direction
        bridge.voxels()[bridge.lattice().index(cx, cy-1, cz)].flux = {0.0, -1.0, 0.0};
        // Set flux at +z neighbor pointing in +z direction
        bridge.voxels()[bridge.lattice().index(cx, cy, cz+1)].flux = {0.0, 0.0, 1.0};
        // Set flux at -z neighbor pointing in -z direction
        bridge.voxels()[bridge.lattice().index(cx, cy, cz-1)].flux = {0.0, 0.0, -1.0};

        int center = bridge.lattice().index(cx, cy, cz);
        double div = bridge.divergence_flux(center);
        // div = (Jx(+x) - Jx(-x))/2 + (Jy(+y) - Jy(-y))/2 + (Jz(+z) - Jz(-z))/2
        // = (1-(-1))/2 + (1-(-1))/2 + (1-(-1))/2 = 1 + 1 + 1 = 3
        check("Divergence of radial flux > 0", div > 0);
        check_close("Divergence of radial flux = 3", div, 3.0, 1e-12);
    }

    // ---- Gradient of uniform density = 0 ----
    {
        ftd::RenderBridge bridge(N);
        // Set all voxels to same flux magnitude
        for (int i = 0; i < bridge.lattice().total_sites(); ++i) {
            bridge.voxels()[i].flux = {1.0, 0.0, 0.0};
        }
        int center = bridge.lattice().index(4, 4, 4);
        auto grad = bridge.gradient_density(center);
        check_close("Gradient uniform density: x = 0", grad.x, 0.0, 1e-12);
        check_close("Gradient uniform density: y = 0", grad.y, 0.0, 1e-12);
        check_close("Gradient uniform density: z = 0", grad.z, 0.0, 1e-12);
    }

    // ---- Gradient of single-site density ----
    // High density at one site, zero elsewhere
    // Gradient at +x neighbor should point away (negative x component)
    {
        ftd::RenderBridge bridge(N);
        int center = bridge.lattice().index(4, 4, 4);
        bridge.voxels()[center].flux = {5.0, 0.0, 0.0};  // density = 5

        // Gradient at +x neighbor: (density(+x+1) - density(+x-1)) / 2
        // = (density(6,4,4) - density(4,4,4)) / 2 = (0 - 5) / 2 = -2.5
        int nx = bridge.lattice().index(5, 4, 4);
        auto grad = bridge.gradient_density(nx);
        check("Gradient near spike: x < 0 (points away)", grad.x < 0);
    }

    // ---- Curl of zero flux = 0 ----
    {
        ftd::RenderBridge bridge(N);
        int center = bridge.lattice().index(4, 4, 4);
        auto curl = bridge.curl_flux(center);
        check_close("Curl of zero flux: x = 0", curl.x, 0.0, 1e-12);
        check_close("Curl of zero flux: y = 0", curl.y, 0.0, 1e-12);
        check_close("Curl of zero flux: z = 0", curl.z, 0.0, 1e-12);
    }

    // ---- Curl of uniform flux = 0 ----
    {
        ftd::RenderBridge bridge(N);
        for (int i = 0; i < bridge.lattice().total_sites(); ++i) {
            bridge.voxels()[i].flux = {2.0, 3.0, 1.0};
        }
        int center = bridge.lattice().index(4, 4, 4);
        auto curl = bridge.curl_flux(center);
        check_close("Curl of uniform flux: x = 0", curl.x, 0.0, 1e-12);
        check_close("Curl of uniform flux: y = 0", curl.y, 0.0, 1e-12);
        check_close("Curl of uniform flux: z = 0", curl.z, 0.0, 1e-12);
    }

    // ---- Curl of rotational pattern ----
    // Set flux in a pattern with J = (0, -z, y) around center
    // curl should have a positive x-component (rotation around x-axis)
    {
        ftd::RenderBridge bridge(N);
        int cx = 4, cy = 4, cz = 4;
        // Set flux at neighbors in a rotational pattern
        // At (cx, cy+1, cz): J = (0, -(cz-cz), (cy+1-cy)) = (0, 0, 1)
        bridge.voxels()[bridge.lattice().index(cx, cy+1, cz)].flux = {0.0, 0.0, 1.0};
        // At (cx, cy-1, cz): J = (0, 0, -1)
        bridge.voxels()[bridge.lattice().index(cx, cy-1, cz)].flux = {0.0, 0.0, -1.0};
        // At (cx, cy, cz+1): J = (0, -1, 0)
        bridge.voxels()[bridge.lattice().index(cx, cy, cz+1)].flux = {0.0, -1.0, 0.0};
        // At (cx, cy, cz-1): J = (0, 1, 0)
        bridge.voxels()[bridge.lattice().index(cx, cy, cz-1)].flux = {0.0, 1.0, 0.0};

        int center = bridge.lattice().index(cx, cy, cz);
        auto curl = bridge.curl_flux(center);
        // curl_x = dJz/dy - dJy/dz
        //        = (Jz(y+1) - Jz(y-1))/2 - (Jy(z+1) - Jy(z-1))/2
        //        = (1 - (-1))/2 - ((-1) - 1)/2 = 1 - (-1) = 2
        check("Curl of rotational flux: x > 0", curl.x > 0);
        check_close("Curl of rotational flux: x = 2", curl.x, 2.0, 1e-12);
    }

    // ---- Divergence of uniform flux = 0 ----
    {
        ftd::RenderBridge bridge(N);
        for (int i = 0; i < bridge.lattice().total_sites(); ++i) {
            bridge.voxels()[i].flux = {3.0, 2.0, 1.0};
        }
        int center = bridge.lattice().index(4, 4, 4);
        double div = bridge.divergence_flux(center);
        check_close("Divergence of uniform flux = 0", div, 0.0, 1e-12);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All discrete operator tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
