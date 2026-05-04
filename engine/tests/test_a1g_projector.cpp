/**
 * Unit test for the A_{1g} projector on a 27-voxel Moore block.
 *
 * Sanity battery for ftd::compute_a1g_fraction (a1g_projector.h):
 *   1. Zero field → fraction = 1 (trivial-subspace convention).
 *   2. δ_center · A → fraction = 1 (A_{1g}-pure).
 *   3. Uniform constant on all 27 voxels → fraction = 1 (A_{1g}-pure).
 *   4. Orbit-sum basis vectors (face / edge / corner) individually pure → fraction = 1.
 *   5. A pure E_g eigenvector (face-orbit, weights summing to 0 with E_g
 *      character) → fraction = 0 (orthogonal to A_{1g}).
 *   6. Random IID gaussian field → fraction ≈ 4/27 ≈ 0.148 averaged over
 *      many seeds (dim(A_{1g}) / dim(ρ_27)).
 *   7. Periodic wrap respected when block is at corner (0,0,0).
 *
 * Linked to: docs/theory/03_derivations/DERIV_FTD0110_NONLINEAR_BRIDGE.md §5.2
 *           (Bridge-I empirical instrument; FTD-0110 Option A).
 */

#include "ftd/a1g_projector.h"
#include "ftd/voxel.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

namespace {

int failures = 0;

void check_close(const char* name, double a, double b, double tol) {
    bool ok = std::abs(a - b) <= tol;
    if (ok) {
        std::cout << "  PASS  " << name << "  (got " << std::setprecision(8) << a << ")\n";
    } else {
        std::cout << "  FAIL  " << name << "  (got " << std::setprecision(8) << a
                  << ", expected " << b << ", diff=" << std::abs(a - b)
                  << ", tol=" << tol << ")\n";
        ++failures;
    }
}

std::vector<ftd::Voxel> make_lattice(int L) {
    return std::vector<ftd::Voxel>(static_cast<std::size_t>(L) * L * L);
}

void set_flux(std::vector<ftd::Voxel>& vox, int L, int x, int y, int z,
              double fx, double fy, double fz) {
    const int i = ftd::lattice_idx(x, y, z, L);
    vox[i].flux = ftd::Vec3{fx, fy, fz};
}

}  // namespace

int main() {
    using ftd::compute_a1g_fraction;

    std::cout << "================================================================\n";
    std::cout << "  TEST: A_{1g} projector sanity (FTD-0110 Bridge-I instrument)\n";
    std::cout << "================================================================\n\n";

    constexpr int L = 8;
    const int cx = L / 2, cy = L / 2, cz = L / 2;

    // 1. Zero field
    {
        auto vox = make_lattice(L);
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("1. zero field → mean = 1 (convention)", r.mean, 1.0, 0.0);
    }

    // 2. δ_center · A (A_{1g}-pure)
    {
        auto vox = make_lattice(L);
        set_flux(vox, L, cx, cy, cz, 1.0, 2.0, 3.0);
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("2. δ_center → mean = 1", r.mean, 1.0, 1e-12);
        check_close("2. δ_center → f_x = 1", r.f_x, 1.0, 1e-12);
        check_close("2. δ_center → f_y = 1", r.f_y, 1.0, 1e-12);
        check_close("2. δ_center → f_z = 1", r.f_z, 1.0, 1e-12);
    }

    // 3. Uniform constant on all 27 voxels (A_{1g}-pure)
    {
        auto vox = make_lattice(L);
        // Set all 27 block voxels to (1, 1, 1)
        for (int dz = -1; dz <= 1; ++dz)
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    set_flux(vox, L, cx + dx, cy + dy, cz + dz, 1.0, 1.0, 1.0);
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("3. uniform on block → mean = 1", r.mean, 1.0, 1e-12);
    }

    // 4a. Pure face-orbit sum (A_{1g}-pure)
    {
        auto vox = make_lattice(L);
        for (int k = 0; k < 6; ++k) {
            const auto& d = ftd::MooreOrbits::FACE_OFFSETS[k];
            set_flux(vox, L, cx + d[0], cy + d[1], cz + d[2], 1.0, 0.0, 0.0);
        }
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("4a. face-orbit sum (x-component) → mean = 1", r.mean, 1.0, 1e-12);
    }

    // 4b. Pure edge-orbit sum (A_{1g}-pure)
    {
        auto vox = make_lattice(L);
        for (int k = 0; k < 12; ++k) {
            const auto& d = ftd::MooreOrbits::EDGE_OFFSETS[k];
            set_flux(vox, L, cx + d[0], cy + d[1], cz + d[2], 0.0, 1.0, 0.0);
        }
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("4b. edge-orbit sum (y-component) → mean = 1", r.mean, 1.0, 1e-12);
    }

    // 4c. Pure corner-orbit sum (A_{1g}-pure)
    {
        auto vox = make_lattice(L);
        for (int k = 0; k < 8; ++k) {
            const auto& d = ftd::MooreOrbits::CORNER_OFFSETS[k];
            set_flux(vox, L, cx + d[0], cy + d[1], cz + d[2], 0.0, 0.0, 1.0);
        }
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        check_close("4c. corner-orbit sum (z-component) → mean = 1", r.mean, 1.0, 1e-12);
    }

    // 5. Pure E_g eigenvector on the face orbit.
    // E_g irrep on the 6 face voxels (±x̂, ±ŷ, ±ẑ) is 2-dim. One basis vector:
    //   v(±x̂) = +1, v(±ŷ) = +1, v(±ẑ) = -2  (the (3z²−r²)-like component)
    // This is orthogonal to the A_{1g} face-orbit sum (1,1,1,1,1,1) and to the
    // T_{1u} face-orbit antisymmetric pairs. Should give A_{1g} fraction = 0.
    {
        auto vox = make_lattice(L);
        // ±x̂ → +1
        set_flux(vox, L, cx + 1, cy, cz,  1.0, 0.0, 0.0);
        set_flux(vox, L, cx - 1, cy, cz,  1.0, 0.0, 0.0);
        // ±ŷ → +1
        set_flux(vox, L, cx, cy + 1, cz,  1.0, 0.0, 0.0);
        set_flux(vox, L, cx, cy - 1, cz,  1.0, 0.0, 0.0);
        // ±ẑ → -2
        set_flux(vox, L, cx, cy, cz + 1, -2.0, 0.0, 0.0);
        set_flux(vox, L, cx, cy, cz - 1, -2.0, 0.0, 0.0);
        auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
        // Sum of x-component over face orbit = 2(1+1-2) = 0 ⇒ A_{1g} = 0 in x.
        // y, z components zero → trivially A_{1g} (= 1 by convention).
        // Aggregate uses energy weighting: x has total energy 12, y/z zero.
        // So mean = (0 + 0 + 0)/12 = 0.
        check_close("5. pure E_g face vector → mean = 0", r.mean, 0.0, 1e-12);
        check_close("5. pure E_g face vector → f_x = 0", r.f_x, 0.0, 1e-12);
    }

    // 6. Random IID gaussian → ⟨fraction⟩ ≈ 4/27 over many seeds.
    {
        std::mt19937 rng(42);
        std::normal_distribution<double> nd(0.0, 1.0);
        constexpr int N_SEEDS = 2000;
        double mean_acc = 0.0;
        for (int s = 0; s < N_SEEDS; ++s) {
            auto vox = make_lattice(L);
            for (int dz = -1; dz <= 1; ++dz)
                for (int dy = -1; dy <= 1; ++dy)
                    for (int dx = -1; dx <= 1; ++dx) {
                        set_flux(vox, L, cx + dx, cy + dy, cz + dz,
                                 nd(rng), nd(rng), nd(rng));
                    }
            auto r = compute_a1g_fraction(vox, L, cx, cy, cz);
            mean_acc += r.mean;
        }
        const double observed = mean_acc / N_SEEDS;
        const double expected = 4.0 / 27.0;
        // For 2000 seeds the standard error on the mean is ~ 0.001-0.002.
        // Tolerance 0.01 is comfortable.
        check_close("6. random IID gaussian → ⟨mean⟩ ≈ 4/27", observed, expected, 0.01);
    }

    // 7. Periodic wrap: block at (0,0,0) (lattice corner) should still resolve
    //    its full 27-block neighborhood via wrap.
    {
        auto vox = make_lattice(L);
        set_flux(vox, L, 0, 0, 0, 1.0, 0.0, 0.0);
        auto r = compute_a1g_fraction(vox, L, 0, 0, 0);
        check_close("7. δ_center at (0,0,0) with wrap → mean = 1", r.mean, 1.0, 1e-12);
    }

    std::cout << "\n";
    if (failures == 0) {
        std::cout << "ALL SANITY CHECKS PASSED.\n";
        return 0;
    } else {
        std::cout << "FAILURES: " << failures << "\n";
        return 1;
    }
}
