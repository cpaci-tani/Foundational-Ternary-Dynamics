/-
  FTD Proof Verification -- Main Entry Point
  ============================================
  Comprehensive Lean 4 verification of the Foundational Ternary Dynamics
  mathematical framework.

  Module structure:
  - Constants:         Framework integers, Gamma-primitive constants
  - Algebra:           Vieta relations, harmonic mean, discriminant
  - NumberTheory:      Mod arithmetic, Fermat classification, FizzBuzz
  - MasterQuadratic:   The master quadratic and all its properties
  - Precision:         Epsilon parameter, precision coefficients
  - GaussianIntegers:  Z[i] properties, norm, units, conductor
  - EllipticCurve:     E: y^2=x^3-x invariants (j, disc, Aut, torsion, BSD)
  - GammaFoundation:   Pi-free Gamma basis, triad identity, Wallis products
  - LFunction:         Axiomatized L-function results (Tier 3A)
  - SelfDuality:       The conjecture and physical axiom (Tier 3B/C)

  Usage: lake build
-/

import FTD.Constants
import FTD.Algebra
import FTD.NumberTheory
import FTD.MasterQuadratic
import FTD.Precision
import FTD.GaussianIntegers
import FTD.EllipticCurve
import FTD.GammaFoundation
import FTD.FineStructure
import FTD.LFunction
import FTD.SelfDuality

-- Axioms.lean is subsumed by LFunction.lean and SelfDuality.lean.
-- Keeping the import for backward compatibility:
import FTD.Axioms

/-! ## Verification Report

  Run `lake build` to compile the entire project.
  All #eval blocks produce verification output during compilation.

  ### Theorem Count

  **Tier 1 (Pure Algebra, native_decide/omega):**
  - Constants.lean:      11 theorems
  - Algebra.lean:         7 theorems
  - NumberTheory.lean:   ~50 theorems
  - MasterQuadratic.lean: 1 #eval (5 numerical checks)
  - Precision.lean:      13 theorems + 1 #eval

  **Tier 2 (Integer-level, no Mathlib):**
  - GaussianIntegers.lean: 12 theorems
  - EllipticCurve.lean:    20 theorems
  - GammaFoundation.lean:  1 #eval (6 numerical checks)

  **Tier 3 (Axiomatized):**
  - LFunction.lean:     8 axioms (all proven in literature)
  - SelfDuality.lean:   2 axioms (1 conjecture + 1 physical)
  - Axioms.lean:        12 axioms (legacy, overlaps with above)

  **Total: ~113 verified theorems + 22 axioms + 3 #eval blocks**

  ### Sorry Count
  - Tier 1-2: 0 sorry (all proofs complete)
  - Tier 3: all axioms are explicitly labeled with citations

  ### Numerical Verifications (#eval)
  - Master quadratic roots: x+ = 137.036, x- = 3.024
  - Sum = Product: PASS
  - Harmonic mean = 2: PASS
  - Triad identity: PASS
  - Precision formula: PASS (leading < 2 ppm)
  - Gamma-primitive basis: 6/6 PASS
-/
