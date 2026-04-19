#pragma once
/**
 * The Ontic Derivation Chain — umbrella header.
 *
 * Everything from nothing: e → γ → Γ(1/4) → θ₃ → ϖ → M → G* → π → all physics.
 *
 * Nine layers, each derived from the one above.
 * The only inputs are D=3 (spatial dimensions) and the lemniscate constant ϖ.
 * Every physical constant in the engine traces back through this chain.
 *
 * Layer -1: Self-Referential Seed  (e)                        → ontic/lemniscate.h
 * Layer 0:  Transcendental Seeds   (γ, Γ(1/4))                → ontic/lemniscate.h
 * Layer 0b: Modular Selection      (q, θ₃)                    → ontic/lemniscate.h
 * Layer 1:  Elliptic Geometry      (ϖ, M)                     → ontic/lemniscate.h
 * Layer 2:  Universal Operator     (G*, π, PF, √G*)           → ontic/lemniscate.h
 * Layer 2b: Euler's Identity       (i emerges at k_crit=4/G*) → ontic/lemniscate.h
 * Layer 3:  Master Quadratic       (x₊ = 1/α, x₋ = N_c)       → ontic/master_quadratic.h
 * Layer 3b: Dual-Substrate         (E_SUM, E_PRODUCT, δ²)     → ontic/master_quadratic.h
 * Layer 3c: Charge-Space Duality   (E2_COLOR)                 → ontic/master_quadratic.h
 * Layer 4:  Framework Integers     (N_c, b₃, N_eff, D)        → ontic/master_quadratic.h
 * Layer 4b: Neutrino Mixing        (PMNS angles)              → ontic/master_quadratic.h
 * Layer 5:  Coupling Constants     (α, g_c, G_N, α_G)         → ontic/gauge_couplings.h
 * Layer 5b: QCD Sector             (α_s, b₀, Λ_QCD, M_Z)      → ontic/gauge_couplings.h
 * Layer 6:  Mass Scale             (K_B, K_GENESIS)           → ontic/particle_masses.h
 * Layer 6b: Electroweak Scale      (V_HIGGS, M_HIGGS, λ_H)    → ontic/particle_masses.h
 * Layer 6c: Mass Ratios            (MU_RATIO, TAU_RATIO, ...) → ontic/particle_masses.h
 * Layer 7:  Precision Formula      (ε, c₁-c₄, corrected α)    → ontic/gauge_couplings.h
 * Layer 7b: Absolute ν Masses      (seesaw: m_D, M_R, m_1/2/3)→ ontic/neutrino.h
 * Layer 8:  Consciousness          (y, θ_C, K_C)              → ontic/consciousness.h
 * Layer 8b: Golden Ratio Fixed Pt  (φ, λ_loop, β_intr)        → ontic/consciousness.h
 *
 * All constants live in namespace ftd::ontic. Downstream code should
 * continue to #include "ftd/ontic.h" — this umbrella re-exports every
 * name that previously lived in the monolithic header.
 *
 * Extraction audit: O1 (theme split) + O2 (move alpha_s_running out of
 * the header). See engine/src/ontic_running_coupling.cpp.
 */

#include "ftd/ontic/lemniscate.h"
#include "ftd/ontic/master_quadratic.h"
#include "ftd/ontic/gauge_couplings.h"
#include "ftd/ontic/particle_masses.h"
#include "ftd/ontic/neutrino.h"
#include "ftd/ontic/consciousness.h"
