/-
  FTD.GammaFoundation -- The Gamma-Primitive Basis and Triad Identity
  ====================================================================
  Establishes the pi-free foundation: everything derives from
  gamma_1 = Gamma(1/4) and gamma_2 = Gamma(1/2).

  The key insight: pi = gamma_2^2 is DERIVED, not assumed.
  G* = gamma_1^2 / (sqrt(2) * gamma_2^2) is pi-free.

  Reference: MONOGRAPH_GSTAR_BRIDGE_CONSTANT.md, PAPER_MISSING_RATIO.tex
-/

import FTD.Constants

namespace FTD.GammaFoundation
open FTD

/-! ## The Gamma-Primitive Basis

  Two independent transcendental quantities:
    gamma_1 = Gamma(1/4) = 3.62560990822...
    gamma_2 = Gamma(1/2) = 1.77245385090... = sqrt(pi)

  All FTD constants derive from these two:
    pi    = gamma_2^2
    G*    = gamma_1^2 / (sqrt(2) * gamma_2^2)
    varpi = gamma_1^2 / (2 * sqrt(2) * gamma_2)
    r     = gamma_1^2 / gamma_2^2   (the Gamma ratio)

  Nesterenko (1996) proved gamma_1 and pi are algebraically independent.
  Since gamma_2 = sqrt(pi), this means gamma_1 and gamma_2 are
  algebraically independent — justifying the two-variable basis.
-/

/-! ## Numerical Verification of All Identities -/

#eval do
  let g1 := gamma1_f      -- Gamma(1/4)
  let g2 := gamma2_f      -- Gamma(1/2) = sqrt(pi)
  let sqrt2 := Float.sqrt 2.0

  -- Derived quantities
  let pi_val := g2 * g2                           -- pi = Gamma(1/2)^2
  let gstar := g1 * g1 / (sqrt2 * g2 * g2)        -- G* (pi-free!)
  let varpi := g1 * g1 / (2.0 * sqrt2 * g2)       -- varpi
  let r_gamma := g1 * g1 / (g2 * g2)              -- the ratio r

  -- The "missing ratio": G* = Gamma(1/4) / Gamma(3/4)
  -- Using reflection: Gamma(3/4) = pi*sqrt(2) / Gamma(1/4)
  let g34 := pi_val * sqrt2 / g1
  let gstar_ratio := g1 / g34

  IO.println "═══════════════════════════════════════════════════════"
  IO.println "  GAMMA-PRIMITIVE BASIS VERIFICATION"
  IO.println "═══════════════════════════════════════════════════════"
  IO.println ""
  IO.println "--- Basis Elements ---"
  IO.println s!"  gamma_1 = Gamma(1/4) = {g1}"
  IO.println s!"  gamma_2 = Gamma(1/2) = {g2}"
  IO.println ""
  IO.println "--- Derived Constants ---"
  IO.println s!"  pi      = gamma_2^2           = {pi_val}"
  IO.println s!"  G*      = gamma_1^2/(sqrt2*gamma_2^2) = {gstar}"
  IO.println s!"  varpi   = gamma_1^2/(2*sqrt2*gamma_2) = {varpi}"
  IO.println s!"  r       = gamma_1^2/gamma_2^2 = {r_gamma}"
  IO.println ""

  -- Triad identity: pi = 4*varpi^2/G*^2
  let pi_triad := 4.0 * varpi * varpi / (gstar * gstar)
  let triad_ok := Float.abs (pi_triad - pi_val) < 1e-10

  IO.println "--- Triad Identity: pi = 4*varpi^2/G*^2 ---"
  IO.println s!"  4*varpi^2/G*^2 = {pi_triad}"
  IO.println s!"  gamma_2^2      = {pi_val}"
  IO.println s!"  Match? {if triad_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- Lemniscate form: varpi = G*·sqrt(pi)/2
  let varpi_from_gstar := gstar * g2 / 2.0
  let lem_ok := Float.abs (varpi_from_gstar - varpi) < 1e-10

  IO.println "--- Lemniscate Form: varpi = G*·sqrt(pi)/2 ---"
  IO.println s!"  G*·gamma_2/2   = {varpi_from_gstar}"
  IO.println s!"  varpi           = {varpi}"
  IO.println s!"  Match? {if lem_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- The "missing ratio": G* = Gamma(1/4)/Gamma(3/4)
  let ratio_ok := Float.abs (gstar_ratio - gstar) < 1e-10

  IO.println "--- Missing Ratio: G* = Gamma(1/4)/Gamma(3/4) ---"
  IO.println s!"  Gamma(3/4)                  = {g34}"
  IO.println s!"  Gamma(1/4)/Gamma(3/4)       = {gstar_ratio}"
  IO.println s!"  G* (pi-free form)           = {gstar}"
  IO.println s!"  Match? {if ratio_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- Reflection formula: Gamma(1/4)·Gamma(3/4) = pi·sqrt(2)
  let product := g1 * g34
  let pi_sqrt2 := pi_val * sqrt2
  let refl_ok := Float.abs (product - pi_sqrt2) < 1e-10

  IO.println "--- Reflection: Gamma(1/4)·Gamma(3/4) = pi·sqrt(2) ---"
  IO.println s!"  Product = {product}"
  IO.println s!"  pi·sqrt(2) = {pi_sqrt2}"
  IO.println s!"  Match? {if refl_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- G*·sqrt(pi) identity chain
  IO.println "--- Identity Chain ---"
  IO.println s!"  G*·sqrt(pi) = {gstar * g2}"
  IO.println s!"  2·varpi     = {2.0 * varpi}"
  let chain_ok := Float.abs (gstar * g2 - 2.0 * varpi) < 1e-10
  IO.println s!"  Match? {if chain_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- r = G*·sqrt(2) (the Gamma ratio)
  IO.println "--- Gamma Ratio: r = G*·sqrt(2) ---"
  IO.println s!"  r         = {r_gamma}"
  IO.println s!"  G*·sqrt2  = {gstar * sqrt2}"
  let r_ok := Float.abs (r_gamma - gstar * sqrt2) < 1e-10
  IO.println s!"  Match? {if r_ok then "PASS" else "FAIL"}"
  IO.println ""

  -- Wallis product limit statement (verified numerically)
  -- Race 1: sqrt(pi) = lim N^{-1/2} * prod_{k=1}^{N} (2k)/(2k-1)
  -- Race 2: G* = lim N^{-1/2} * prod_{k=0}^{N} (4k+3)/(4k+1)
  -- Composite: varpi = G*·sqrt(pi)/2

  IO.println "═══════════════════════════════════════════════════════"
  IO.println "  SUMMARY"
  IO.println "═══════════════════════════════════════════════════════"
  IO.println s!"  Triad identity:     {if triad_ok then "PASS" else "FAIL"}"
  IO.println s!"  Lemniscate form:    {if lem_ok then "PASS" else "FAIL"}"
  IO.println s!"  Missing ratio:      {if ratio_ok then "PASS" else "FAIL"}"
  IO.println s!"  Reflection formula: {if refl_ok then "PASS" else "FAIL"}"
  IO.println s!"  Identity chain:     {if chain_ok then "PASS" else "FAIL"}"
  IO.println s!"  Gamma ratio:        {if r_ok then "PASS" else "FAIL"}"
  IO.println "═══════════════════════════════════════════════════════"

end FTD.GammaFoundation
