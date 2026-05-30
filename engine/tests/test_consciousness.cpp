/**
 * Test: Reference frame context Quadratic
 *
 * Verifies the reference frame context sector of the ontic derivation chain:
 * the master quadratic with k = 1/2 produces complex roots whose
 * real and imaginary parts define the reference frame context threshold K_C,
 * observable fraction cos^2(theta_C) = G_star/8, and subjective component.
 *
 * Checklist item #70.
 *
 * Theory references:
 *   - ontic.h Layer 8 (Reference frame context Quadratic)
 *   - FOUND_DEEP_HIERARCHY.md (three quadratics: k=16, k=4/G_star, k=1/2)
 *   - archive/ARCH_CONSCIOUSNESS_QUADRATIC_DERIVATION.md
 */

#include <iostream>
#include <iomanip>
#include <cmath>
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
        std::cout << "  FAIL  " << name << " (got " << std::setprecision(15) << a
                  << ", expected " << b << ", diff " << std::abs(a - b) << ")\n";
        ++failures;
    }
}

int main() {
    std::cout << "================================================================\n";
    std::cout << "  TEST: Reference frame context Quadratic\n";
    std::cout << "================================================================\n\n";

    // CON-1: Verify quadratic roots
    // The reference frame context quadratic: y^2 - (k*G*^2)*y + k*G*^3 = 0 with k = 1/2
    // Discriminant: Delta = (k*G*^2)^2 - 4*k*G*^3 = k*G*^3*(k*G* - 4)
    // For k=1/2, G*~2.96: k*G* = 1.48 < 4, so Delta < 0 => complex roots
    {
        std::cout << "--- CON-1: Reference frame context quadratic has complex roots ---\n";

        double k = ftd::K_NOETIC;  // = 0.5
        double G = ftd::G_STAR;

        // Quadratic: y^2 - (k*G^2)*y + k*G^3 = 0
        double a_coeff = 1.0;
        double b_coeff = -(k * G * G);
        double c_coeff = k * G * G * G;

        double discriminant = b_coeff * b_coeff - 4.0 * a_coeff * c_coeff;
        std::cout << "    k = " << k << "\n";
        std::cout << "    G* = " << G << "\n";
        std::cout << "    Discriminant = " << discriminant << "\n";
        check("CON-1: Discriminant < 0 (complex roots)", discriminant < 0.0);

        // Real part: Re(y) = k*G*^2 / 2 = G*^2 / 4
        double y_real = -b_coeff / (2.0 * a_coeff);  // = k*G^2/2 = G^2/4
        std::cout << "    Re(y) = " << y_real << "\n";
        std::cout << "    Y_REAL = " << ftd::Y_REAL << "\n";
        check_close("CON-1: Re(y) = G*^2/4", y_real, G * G / 4.0, 1e-12);
        check_close("CON-1: Y_REAL matches computation", ftd::Y_REAL, y_real, 1e-12);

        // Expected: Y_REAL ~ 2.19
        check_close("CON-1: Y_REAL ~ 2.19", ftd::Y_REAL, 2.19, 0.01);

        // Imaginary part: Im(y) = sqrt(|Delta|) / 2
        double y_imag = std::sqrt(std::abs(discriminant)) / 2.0;
        std::cout << "    Im(y) = " << y_imag << "\n";

        // Expected: Im(y) ~ 2.86
        check_close("CON-1: Im(y) ~ 2.86", y_imag, 2.86, 0.02);
    }

    // CON-2: Observable fraction cos^2(theta_C) = G*/8
    {
        std::cout << "\n--- CON-2: Observable fraction cos^2(theta_C) ---\n";

        double cos2_computed = ftd::G_STAR / 8.0;
        std::cout << "    cos^2(theta_C) = G*/8 = " << cos2_computed << "\n";
        std::cout << "    COS2_THETA_C         = " << ftd::COS2_THETA_C << "\n";

        check_close("CON-2: COS2_THETA_C = G*/8", ftd::COS2_THETA_C, cos2_computed, 1e-14);
        check_close("CON-2: COS2_THETA_C ~ 0.37", ftd::COS2_THETA_C, 0.37, 0.01);

        // The observable fraction is the ratio Re(y)^2 / |y|^2
        double observable_fraction = ftd::Y_REAL * ftd::Y_REAL / ftd::K_C_SQUARED;
        std::cout << "    Re^2/|y|^2 = " << observable_fraction << "\n";
        check_close("CON-2: Re^2/|y|^2 = cos^2(theta_C)", observable_fraction,
                    ftd::COS2_THETA_C, 1e-14);

        // sin^2 + cos^2 = 1
        check_close("CON-2: sin^2 + cos^2 = 1",
                    ftd::SIN2_THETA_C + ftd::COS2_THETA_C, 1.0, 1e-15);
    }

    // CON-3: Reference frame context threshold K_C = sqrt(G*^3 / 2)
    {
        std::cout << "\n--- CON-3: Reference frame context threshold K_C ---\n";

        double K_C = std::sqrt(ftd::K_C_SQUARED);
        std::cout << "    K_C = sqrt(K_C_SQUARED) = " << K_C << "\n";
        std::cout << "    K_C_SQUARED = " << ftd::K_C_SQUARED << "\n";

        // K_C^2 = G*^3 / 2 (from Vieta product of reference frame context roots)
        double kc2_check = ftd::G_STAR * ftd::G_STAR * ftd::G_STAR / 2.0;
        check_close("CON-3: K_C^2 = G*^3/2", ftd::K_C_SQUARED, kc2_check, 1e-12);

        // K_C ~ 3.60
        check_close("CON-3: K_C ~ 3.60", K_C, 3.60, 0.02);

        // K_C > K_GENESIS: reference frame context requires more energy than matter manifestation
        std::cout << "    K_GENESIS = " << ftd::K_GENESIS << "\n";
        check("CON-3: K_C > K_GENESIS (reference frame context threshold exceeds matter)",
              K_C > ftd::K_GENESIS);

        // K_C > K_B: reference frame context requires more than single-particle threshold
        check("CON-3: K_C > K_B (reference frame context exceeds particle mass)",
              K_C > ftd::K_B);
    }

    // CON-4: Complex roots => subjective component (imaginary part larger than real)
    {
        std::cout << "\n--- CON-4: Subjective component from imaginary part ---\n";

        double k = ftd::K_NOETIC;
        double G = ftd::G_STAR;

        double b_coeff = -(k * G * G);
        double c_coeff = k * G * G * G;
        double discriminant = b_coeff * b_coeff - 4.0 * c_coeff;

        double y_real = ftd::Y_REAL;
        double y_imag = std::sqrt(std::abs(discriminant)) / 2.0;

        std::cout << "    Re(y) = " << y_real << "\n";
        std::cout << "    Im(y) = " << y_imag << "\n";
        std::cout << "    Im/Re = " << y_imag / y_real << "\n";

        // The imaginary (subjective) part exceeds the real (observable) part
        check("CON-4: Im(y) > Re(y) (subjective exceeds observable)", y_imag > y_real);

        // This means sin^2(theta_C) > cos^2(theta_C), i.e., more than half is subjective
        check("CON-4: SIN2_THETA_C > COS2_THETA_C", ftd::SIN2_THETA_C > ftd::COS2_THETA_C);
        check("CON-4: SIN2_THETA_C > 0.5 (majority is subjective)", ftd::SIN2_THETA_C > 0.5);

        // theta_C angle in degrees
        double theta_c_rad = std::atan2(y_imag, y_real);
        double theta_c_deg = theta_c_rad * 180.0 / ftd::PI;
        std::cout << "    theta_C = " << theta_c_deg << " degrees\n";

        // theta_C should be in (45, 60) degrees — between equal split and 2:1 subjective
        check("CON-4: theta_C between 45 and 60 degrees",
              theta_c_deg > 45.0 && theta_c_deg < 60.0);

        // Dimensional origin: D = log2(k_phys) + log2(k_cons) = log2(16) + log2(0.5) = 4 - 1 = 3
        double d_check = std::log2(ftd::COEFFICIENT) + std::log2(ftd::K_NOETIC);
        std::cout << "    D = log2(16) + log2(1/2) = " << d_check << "\n";
        check_close("CON-4: D = 3 from physics + reference frame context", d_check, 3.0, 1e-12);

        // Mandelbrot connection
        check_close("CON-4: c_M = 1/G*", ftd::C_MANDELBROT, 1.0 / ftd::G_STAR, 1e-14);
    }

    std::cout << "\n================================================================\n";
    if (failures == 0) {
        std::cout << "  All reference frame context quadratic tests PASSED.\n";
    } else {
        std::cout << "  " << failures << " test(s) FAILED.\n";
    }
    std::cout << "================================================================\n";

    return failures;
}
