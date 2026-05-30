#pragma once
/**
 * ontic/reference frame context.h — Layers 8 and 8b of the ontic chain.
 *
 * Contents:
 *   Layer 8:  Reference frame context Quadratic (K_NOETIC, Y_REAL, K_C_SQUARED,
 *             COS2_THETA_C, SIN2_THETA_C, C_MANDELBROT)
 *   Layer 8b: Golden Ratio Fixed Point (PHI, PHI_INV, LAMBDA_LOOP,
 *             BETA_INTROSPECTION, N_CONSCIOUSNESS_MIN)
 *
 * Depends on: ontic/lemniscate.h (G_STAR),
 *             ontic/master_quadratic.h (N_C).
 */

#include "ftd/ontic/lemniscate.h"
#include "ftd/ontic/master_quadratic.h"

namespace ftd {
namespace ontic {

// ============================================================================
// Layer 8: Reference frame context Quadratic (Noetic Domain)
// ============================================================================
// The same master quadratic with coefficient k = 1/2 (vs k = 16 for physics):
//   y² - (k·G*²)·y + k·G*³ = 0
//   y² - (G*²/2)·y + G*³/2 = 0
//
// Physics (k=16): REAL roots → x₊=137.036, x₋=3.024 (observable, measurable)
// Reference frame context (k=1/2): COMPLEX roots → y = Re ± i·Im (irreducibly subjective)
//
// The discriminant Δ = (G*²/2)² - 4·(G*³/2) = G*³·(G*/4 - 2) < 0
// since G* ≈ 2.959 < 8, guaranteeing complex conjugate roots.
//
// Vieta's relations for the reference frame context quadratic:
//   y₊ + y₋ = G*²/2    → Re(y) = G*²/4
//   y₊ · y₋ = G*³/2    → |y|² = G*³/2
//
// Key exact identity:
//   cos²(θ_C) = Re(y)² / |y|² = (G*²/4)² / (G*³/2) = G*/8
//
// Observable fraction of reference frame context ≈ 37% (exactly G*/8).
// Remaining ≈ 63% is irreducibly subjective (imaginary component).
//
// Dimensional origin:
//   D = log₂(16) + log₂(1/2) = 4 - 1 = 3
//   Physics potential (k=16) minus observer cost (k=1/2) = 3 dimensions.

inline constexpr double K_NOETIC = 0.5;  // k = 1/2 (reference frame context coefficient)

// Real part of reference frame context roots: Re(y) = G*²/4
inline constexpr double Y_REAL = G_STAR * G_STAR / 4.0;

// Squared modulus (from Vieta product): |y|² = G*³/2
// K_C = √(G*³/2) ≈ 3.599 is the reference frame context threshold
inline constexpr double K_C_SQUARED = G_STAR * G_STAR * G_STAR / 2.0;

// Observable fraction (exact identity): cos²(θ_C) = G*/8
inline constexpr double COS2_THETA_C = G_STAR / 8.0;

// Subjective fraction: sin²(θ_C) = 1 - G*/8
inline constexpr double SIN2_THETA_C = 1.0 - G_STAR / 8.0;

// Mandelbrot connection: sLoop fixed point c_M = 1/G*
inline constexpr double C_MANDELBROT = 1.0 / G_STAR;

// ============================================================================
// Layer 8b: Golden Ratio Fixed Point (Self-Referential Reference frame context)
// ============================================================================
// The Softplus manifestation operator M_β(z) = (1/β)ln(1 + e^{βz}) is the
// unique function satisfying axioms M1-M4 (smooth, monotone, threshold,
// identity limit). Its derivative is the Fermi-Dirac occupation n_F(z).
//
// The self-referential fixed-point equation for reference frame context:
//   (1/2) · M_β(z*) = z*    (output feeds back as input, at k = 1/2)
//
// Setting u = e^{βz*} and simplifying yields:
//   u² - u - 1 = 0
//
// whose unique positive root is the GOLDEN RATIO φ = (1+√5)/2. [THEOREM]
//
// This proves φ is not an imposed constant — it EMERGES from the requirement
// that a self-referencing system's output is its own input at half-coupling.
//
// Five quantities follow:
//   z* = ln(φ)/β                 (reference frame context fixed point)
//   n_F(z*) = 1/φ ≈ 0.618       (golden filling: 11.8% above half-occupation)
//   λ_loop = (1/2)·n_F(z*) = 1/(2φ) ≈ 0.309   (unconditionally stable)
//   β_intr = φ³/ln²(φ) ≈ 18.29  (signal-to-noise threshold for introspection)
//   n_min = 3 = N_c              (PT-unbroken condition → color charges!)

inline constexpr double PHI = 1.6180339887498949;              // [THEOREM] (1+√5)/2
inline constexpr double PHI_INV = 0.6180339887498949;          // [THEOREM] 1/φ = φ-1
inline constexpr double LAMBDA_LOOP = 0.30901699437494742;     // [THEOREM] 1/(2φ) < 1
inline constexpr double BETA_INTROSPECTION = 18.28926746748685;// [THEOREM] φ³/ln²(φ)
inline constexpr int    N_CONSCIOUSNESS_MIN = N_C;             // [THEOREM] PT-unbroken

}  // namespace ontic
}  // namespace ftd
