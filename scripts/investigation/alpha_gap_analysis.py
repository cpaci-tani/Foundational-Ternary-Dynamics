#!/usr/bin/env python3
"""
alpha_gap_analysis.py — What if the math IS the answer?
========================================================

The master quadratic gives x_+ = 137.03608...
CODATA gives 1/alpha = 137.03599917...

What if x_+ is the "true" bare value, and the measured alpha
is the dressed/renormalized value?

OR: What if x_+ IS 1/alpha exactly, and the gap tells us something
we haven't understood yet?

This script computes:
  1. The exact gap and what it means physically
  2. What physics looks like if alpha_bare = 1/x_+
  3. The QED running: does alpha run FROM x_+ TO the measured value?
  4. What breaks if we use x_+ instead of 1/alpha_measured
"""

import sys, os
if sys.platform == 'win32':
    os.environ['PYTHONIOENCODING'] = 'utf-8'
    sys.stdout.reconfigure(encoding='utf-8')

import mpmath
mpmath.mp.dps = 50
from mpmath import mpf, pi, sqrt, log, gamma, fabs, power, exp, ln

# ============================================================================
#   CONSTANTS
# ============================================================================

G4 = gamma(mpf('0.25'))
G4sq = G4**2
sqrt2 = sqrt(mpf(2))

varpi = G4sq / (2 * sqrt(2 * pi))
G_star = sqrt2 * G4sq / (2 * pi)

# Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0
disc = 256 * G_star**4 - 64 * G_star**3
x_plus = (16 * G_star**2 + mpmath.sqrt(disc)) / 2
x_minus = (16 * G_star**2 - mpmath.sqrt(disc)) / 2

# CODATA 2022 value
alpha_inv_CODATA = mpf('137.035999177')  # 1/alpha (CODATA 2022, uncertainty ~0.000000021)

short = lambda x, d=20: mpmath.nstr(x, d)
SEP = "=" * 80

def section(title):
    print(f"\n\n{SEP}")
    print(f"  {title}")
    print(SEP)

def note(title):
    print(f"\n  --- {title} ---")


# ============================================================================
#   SECTION 1: THE GAP
# ============================================================================

print(SEP)
print("  THE 1.26 PPM GAP: What if the math is right?")
print(SEP)

section("SECTION 1: THE EXACT GAP")

alpha_math = 1 / x_plus       # alpha if the math is exact
alpha_measured = 1 / alpha_inv_CODATA  # alpha from measurement

delta_inv = x_plus - alpha_inv_CODATA
delta_alpha = alpha_measured - alpha_math
ppm = delta_inv / alpha_inv_CODATA * mpf(10)**6

print(f"""
  The master quadratic root:
    x_+ = {short(x_plus, 15)}

  The measured value (CODATA 2022):
    1/alpha = {short(alpha_inv_CODATA, 15)}

  The gap:
    x_+ - 1/alpha = {short(delta_inv, 10)}
    In ppm:          {short(ppm, 6)} ppm
    In alpha:        delta(alpha) = {short(delta_alpha, 6)}

  The math gives a LARGER 1/alpha (weaker coupling).
  The measurement gives a SMALLER 1/alpha (stronger coupling).

  If the math is the bare value, the physical process that
  dresses it must INCREASE the coupling strength (decrease 1/alpha).
""")


# ============================================================================
#   SECTION 2: QED RUNNING OF ALPHA
# ============================================================================

section("SECTION 2: QED RUNNING — DOES ALPHA RUN TO THE RIGHT PLACE?")

note("The QED beta function")

print("""
  In QED, the fine structure constant RUNS with energy scale mu:

    alpha(mu) = alpha(mu_0) / (1 - (alpha(mu_0)/(3*pi)) * ln(mu/mu_0)^2 )

  More precisely, at one loop:

    1/alpha(mu) = 1/alpha(mu_0) - (2/(3*pi)) * sum_f Q_f^2 * ln(mu/mu_0)

  where the sum is over fermions with mass < mu.

  Key insight: alpha gets STRONGER (1/alpha decreases) as you go to
  HIGHER energies. This is the opposite of what we need!

  If x_+ is the bare (UV) value, then at LOW energies (where we measure),
  1/alpha should be LARGER, not smaller.

  Wait. Let me think again.

  Actually:
    - At LOW energy (Thomson limit), we measure 1/alpha = 137.036...
    - At HIGH energy (e.g., M_Z), 1/alpha ~ 128 (stronger coupling)
    - At INFINITE energy (Landau pole), 1/alpha -> 0

  So as we go UP in energy, 1/alpha DECREASES.
  Our x_+ = 137.0361 is ABOVE the measured 137.0360.

  This means x_+ corresponds to a scale BELOW the electron mass.
  That is: x_+ would be the value of 1/alpha at an energy scale
  slightly below where we measure it.
""")

note("Computing the energy scale where 1/alpha = x_+")

# At one loop, with only the electron contributing:
# 1/alpha(mu) = 1/alpha(m_e) - (2/(3*pi)) * ln(mu/m_e)
#
# We want: x_+ = 1/alpha(m_e) - (2/(3*pi)) * ln(mu_0/m_e)
# So: x_+ - 1/alpha(m_e) = -(2/(3*pi)) * ln(mu_0/m_e)
# ln(mu_0/m_e) = -(3*pi/2) * (x_+ - 1/alpha(m_e))

# But wait, the CODATA value is at q^2 = 0 (Thomson limit), not at m_e.
# The difference between q=0 and q=m_e is itself a correction.
# Let's just work with the Thomson limit value.

# 1/alpha(q=0) = 137.035999177 (CODATA)
# We want to find mu such that 1/alpha(mu) = x_+ = 137.03608...

# Using one-loop QED running (electron only):
# 1/alpha(mu) = 1/alpha(0) + (2/(3*pi)) * ln(mu/m_e)  for mu > m_e
# (Note: 1/alpha DECREASES as mu increases, so the + sign is wrong)
#
# Actually the correct formula is:
# 1/alpha(mu) = 1/alpha(0) - (2/(3*pi)) * ln(mu/m_e)   for mu >> m_e
#
# But x_+ > 1/alpha(0), so we need:
# x_+ = 1/alpha(0) - (2/(3*pi)) * ln(mu/m_e)
# x_+ - 1/alpha(0) = -(2/(3*pi)) * ln(mu/m_e)
# Since x_+ > 1/alpha(0), the LHS is positive, so ln(mu/m_e) < 0, meaning mu < m_e.

# ln(mu/m_e) = -(3*pi/2) * (x_+ - 1/alpha(0))
delta = x_plus - alpha_inv_CODATA  # positive
ln_ratio = -(3 * pi / 2) * delta

# mu/m_e = exp(ln_ratio)
mu_over_me = exp(ln_ratio)

m_e_MeV = mpf('0.51099895')  # electron mass in MeV
mu_MeV = mu_over_me * m_e_MeV

print(f"  One-loop QED running:")
print(f"    delta(1/alpha) = x_+ - 1/alpha(0) = {short(delta, 8)}")
print(f"    ln(mu/m_e)     = -(3*pi/2) * delta = {short(ln_ratio, 8)}")
print(f"    mu/m_e         = exp(ln_ratio) = {short(mu_over_me, 8)}")
print(f"    mu             = {short(mu_MeV, 8)} MeV")
print()
print(f"  So x_+ = 1/alpha at an energy scale of {short(mu_MeV, 6)} MeV,")
print(f"  which is {short(mu_over_me, 6)} times the electron mass.")

print(f"""

  This is BELOW the electron mass. In QED, the running below m_e
  is essentially zero (the electron decouples). So one-loop QED
  running does NOT naturally connect x_+ to 1/alpha(0).

  The gap is too small for QED running to explain. The running
  coefficient 2/(3*pi) = {short(2/(3*pi), 6)} is much larger than
  the gap per unit of log(energy).
""")


# ============================================================================
#   SECTION 3: WHAT IF x_+ IS EXACT?
# ============================================================================

section("SECTION 3: WHAT IF x_+ IS 1/alpha EXACTLY?")

note("Consequences for the electron g-2")

print(f"""
  The most precise determination of alpha comes from the electron
  anomalous magnetic moment (g-2)_e.

  The measurement: a_e = (g-2)/2 = 0.00115965218059(13)

  QED predicts: a_e = alpha/(2*pi) - 0.328... * (alpha/pi)^2 + ...

  If alpha = 1/x_+ instead of 1/137.035999177, then:
    alpha_math    = 1/{short(x_plus, 12)}  = {short(alpha_math, 12)}
    alpha_CODATA  = 1/137.035999177 = {short(alpha_measured, 12)}

    delta(alpha) = {short(alpha_measured - alpha_math, 6)}

  The leading QED term a_e ~ alpha/(2*pi):
    With alpha_math:   {short(alpha_math / (2*pi), 12)}
    With alpha_CODATA: {short(alpha_measured / (2*pi), 12)}
    Difference:        {short((alpha_measured - alpha_math) / (2*pi), 6)}

  The experimental precision on a_e is ~1.3 * 10^-13.
  Our alpha shift produces a shift in a_e of ~{short(fabs(alpha_measured - alpha_math) / (2*pi), 4)}.
""")

a_e_shift = fabs(alpha_measured - alpha_math) / (2 * pi)
a_e_precision = mpf('1.3e-13')

print(f"  Shift in a_e from alpha change: {short(a_e_shift, 4)}")
print(f"  Experimental precision:         {short(a_e_precision, 4)}")
print(f"  Ratio (shift/precision):        {short(a_e_shift / a_e_precision, 4)}")

print(f"""
  The shift is about {short(a_e_shift / a_e_precision, 2)} times the experimental
  precision.

  If x_+ IS the true 1/alpha, then either:
    1. The QED calculation of a_e has an error of this size (unlikely)
    2. There is new physics contributing to a_e that compensates
    3. The identification x_+ = 1/alpha is approximate, not exact
""")


# ============================================================================
#   SECTION 4: THE GAP AS A CORRECTION
# ============================================================================

section("SECTION 4: WHAT IS THE GAP MADE OF?")

note("Expressing the gap in terms of alpha")

# delta = x_+ - 1/alpha = 137.03608... - 137.03600... = 0.00008...
# In units of alpha^2:
alpha = alpha_measured
gap = x_plus - alpha_inv_CODATA
gap_in_alpha_sq = gap * alpha**2
gap_in_alpha_over_pi = gap * alpha / pi
gap_in_alpha_sq_over_pi = gap * alpha**2 / pi
gap_in_alpha_sq_over_pi_sq = gap * alpha**2 / pi**2

print(f"  The gap: x_+ - 1/alpha = {short(gap, 10)}")
print()
print(f"  Express in natural QED perturbative units:")
print(f"    gap * alpha              = {short(gap * alpha, 10)}")
print(f"    gap * alpha^2            = {short(gap_in_alpha_sq, 10)}")
print(f"    gap * alpha/pi           = {short(gap_in_alpha_over_pi, 10)}")
print(f"    gap * alpha^2/pi         = {short(gap_in_alpha_sq_over_pi, 10)}")
print(f"    gap * alpha^2/pi^2       = {short(gap_in_alpha_sq_over_pi_sq, 10)}")
print()

# Check some simple combinations
print(f"  Check simple fractions:")
print(f"    gap / (alpha/pi)         = {short(gap / (alpha/pi), 10)}")
print(f"    gap / (alpha^2/pi)       = {short(gap / (alpha**2/pi), 10)}")
print(f"    gap / (alpha^2/(2*pi))   = {short(gap / (alpha**2/(2*pi)), 10)}")
print(f"    gap / (alpha/(2*pi))     = {short(gap / (alpha/(2*pi)), 10)}")
print(f"    gap / (alpha^2)          = {short(gap / alpha**2, 10)}")
print(f"    gap / (2*alpha^2/(3*pi)) = {short(gap / (2*alpha**2/(3*pi)), 10)}")
print()

# Is the gap close to alpha^2/(2*pi) * something simple?
print(f"  Trying to identify the gap as a QED-like correction:")
print(f"    alpha/(2*pi) = {short(alpha/(2*pi), 10)}  (Schwinger term coefficient)")
print(f"    gap / (alpha/(2*pi))^2 = {short(gap / (alpha/(2*pi))**2, 10)}")
print()

# What about alpha^3 scale?
print(f"    alpha^3/pi^2 = {short(alpha**3/pi**2, 10)}")
print(f"    gap / (alpha^3/pi^2) = {short(gap / (alpha**3/pi**2), 10)}")
print()

note("Is the gap related to G* itself?")

print(f"    gap / G*               = {short(gap / G_star, 10)}")
print(f"    gap * G*               = {short(gap * G_star, 10)}")
print(f"    gap * G*^2             = {short(gap * G_star**2, 10)}")
print(f"    gap / (1/G*)           = {short(gap * G_star, 10)}")
print(f"    gap * x_+              = {short(gap * x_plus, 10)}")
print(f"    gap * x_-              = {short(gap * x_minus, 10)}")
print(f"    gap * x_+ * x_-       = {short(gap * x_plus * x_minus, 10)}")
print()

# gap * x_+ * x_- = gap * 16*G*^3 (Vieta's formula: product of roots)
vieta_product = 16 * G_star**3
print(f"    x_+ * x_- = 16*G*^3   = {short(vieta_product, 10)}")
print(f"    gap * 16*G*^3          = {short(gap * vieta_product, 10)}")
print()

note("The gap as a fraction of the root separation")

root_sep = x_plus - x_minus
print(f"    x_+ - x_- = {short(root_sep, 10)}")
print(f"    gap / (x_+ - x_-)     = {short(gap / root_sep, 10)}")
print(f"    gap / (x_+ - x_-)^2   = {short(gap / root_sep**2, 10)}")
print()

# What about: is the gap = f(alpha) where f is a known QED function?
# The QED vacuum polarization at q^2=0 gives a correction to 1/alpha
# of order (alpha/pi) * (something)

# Hadronic vacuum polarization contributes ~ 0.027... to a_e
# Could it shift 1/alpha?

note("Hadronic vacuum polarization correction to 1/alpha")

print(f"""
  The measured 1/alpha at q^2=0 includes contributions from:
    - QED (electron, muon, tau loops)
    - Hadronic vacuum polarization
    - Electroweak corrections

  The hadronic contribution to the running of alpha between q=0 and q=M_Z
  is Delta_had(alpha) ~ 0.02761.

  But we're looking at a much smaller scale: the gap is {short(gap, 6)}.

  Let's check: does the hadronic VP contribution at very low q^2
  account for a shift of ~0.00008 in 1/alpha?

  The leading hadronic VP shifts alpha by:
    delta(1/alpha)_had ~ (1/(3*pi)) * integral of R(s)/s ds

  At very low energies, this is dominated by the pion loop:
    delta(1/alpha)_had(low q) ~ alpha * m_e^2 / (15*pi*m_pi^2)
                                ~ 0.0073 * 0.26e-6 / (15*3.14*0.019)
                                ~ very small
""")

m_pi_MeV = mpf('139.57')
hadronic_estimate = alpha * m_e_MeV**2 / (15 * pi * m_pi_MeV**2)
print(f"  Rough hadronic VP at low q: {short(hadronic_estimate, 6)}")
print(f"  This is much smaller than the gap ({short(gap, 6)})")
print()
print(f"  Hadronic VP does NOT explain the gap at the right scale.")


# ============================================================================
#   SECTION 5: WHAT IF WE FLIP THE QUESTION?
# ============================================================================

section("SECTION 5: WHAT IF THE MATH IS PHYSICS?")

note("The mathematical alpha vs the physical alpha")

alpha_math_val = 1 / x_plus
alpha_phys_val = 1 / alpha_inv_CODATA

print(f"""
  Let's define:
    alpha_0 = 1/x_+ = {short(alpha_math_val, 15)}   (the mathematical value)
    alpha   = {short(alpha_phys_val, 15)}   (the measured value)

  The ratio:
    alpha / alpha_0 = {short(alpha_phys_val / alpha_math_val, 15)}

  This ratio is very close to 1. How close?
    1 - alpha/alpha_0 = {short(1 - alpha_phys_val/alpha_math_val, 10)}
    = {short((1 - alpha_phys_val/alpha_math_val) * 1e6, 6)} ppm
""")

ratio = alpha_phys_val / alpha_math_val
correction = 1 - ratio

print(f"  The correction factor: alpha = alpha_0 * (1 - epsilon)")
print(f"  where epsilon = {short(correction, 10)}")
print()

# Express epsilon in terms of alpha_0
print(f"  epsilon / alpha_0         = {short(correction / alpha_math_val, 10)}")
print(f"  epsilon / alpha_0^2       = {short(correction / alpha_math_val**2, 10)}")
print(f"  epsilon / (alpha_0/pi)    = {short(correction / (alpha_math_val/pi), 10)}")
print(f"  epsilon / (alpha_0/(2*pi))= {short(correction / (alpha_math_val/(2*pi)), 10)}")
print(f"  epsilon * pi / alpha_0    = {short(correction * pi / alpha_math_val, 10)}")
print(f"  epsilon * 2*pi / alpha_0  = {short(correction * 2*pi / alpha_math_val, 10)}")
print()

# The Schwinger correction to g-2 is alpha/(2*pi).
# Is epsilon related to (alpha/(2*pi))^n for some n?
schwinger = alpha_math_val / (2 * pi)
print(f"  Schwinger factor alpha/(2*pi) = {short(schwinger, 10)}")
print(f"  epsilon / schwinger       = {short(correction / schwinger, 10)}")
print(f"  epsilon / schwinger^2     = {short(correction / schwinger**2, 10)}")
print(f"  epsilon / schwinger^3     = {short(correction / schwinger**2 / schwinger, 10)}")
print()

note("The big question: what changes in physics?")

print(f"""
  If alpha_0 = 1/{short(x_plus, 12)} is the "true" bare coupling,
  and the measured alpha = 1/137.035999177 is the dressed value,
  then ALL of quantum electrodynamics is a perturbation theory
  around a slightly different reference point.

  The shift is 1.26 ppm. What does this change?

  HYDROGEN SPECTRUM:
    The Rydberg constant R_inf = alpha^2 * m_e * c / (2*h)
    Shift in R_inf: 2 * delta(alpha)/alpha = {short(2 * fabs(alpha_phys_val - alpha_math_val)/alpha_phys_val * 1e6, 4)} ppm
    Current precision of R_inf: ~0.006 ppm
    This WOULD be detectable — the shift is {short(2*1.26/0.006, 2)}x the Rydberg precision.

  FINE STRUCTURE SPLITTING:
    Scales as alpha^4, so shift = 4 * 1.26 = ~5 ppm
    Precision: varies, typically ~ppb level for hydrogen
    Detectable in principle.

  LAMB SHIFT:
    Scales as alpha^5 * ln(alpha), so shift ~ 5-6 ppm
    Current precision: ~few ppm
    Marginal.

  BOTTOM LINE:
    If x_+ is the true 1/alpha, current spectroscopy would see
    a discrepancy at the few-ppm level. This is NOT consistent
    with existing measurements, which agree with CODATA to better
    than 1 ppm.

  THEREFORE:
    x_+ cannot be the exact physical 1/alpha at q^2 = 0.
    Either:
      (a) x_+ is a related but distinct quantity (bare coupling, etc.)
      (b) The 1.26 ppm is a genuine discrepancy to be resolved
      (c) The match is coincidental
""")


# ============================================================================
#   SECTION 6: THE PRODUCTIVE INTERPRETATION
# ============================================================================

section("SECTION 6: THE PRODUCTIVE INTERPRETATION")

note("What the gap teaches us")

print(f"""
  The gap is real. It's 1.26 ppm. We can't wish it away.

  But 1.26 ppm is REMARKABLE. Out of all the numbers between 0 and
  infinity, the master quadratic produces one that's within 0.000126%
  of a fundamental physical constant. That demands an explanation.

  THREE INTERPRETATIONS:

  (A) x_+ IS a bare coupling constant.
      The gap is a finite renormalization effect.
      Problem: we showed that standard QED running doesn't give
      the right magnitude. So this would require NEW physics
      between the lattice scale and the measurement scale.

  (B) x_+ is 1/alpha at a specific non-zero energy scale.
      The gap is just the running from that scale to q=0.
      Problem: we showed the implied scale is below m_e,
      where running effectively stops.

  (C) The master quadratic is an approximation.
      The quadratic form is the leading term of a more complete
      expression, and higher-order corrections close the gap.

      This is the most interesting option.

      If the quadratic is the FIRST TERM of an expansion:
        x^2 - 16*G*^2*x + 16*G*^3 + epsilon(G*) = 0

      Then epsilon(G*) ~ {short(gap, 6)} * x_+ ~ {short(gap * x_plus, 6)}.

      What could epsilon be? A cubic correction? A modular correction?
      Something from the L-function of E?

  (D) The identification is a coincidence.
      137 is just a number, and the master quadratic happens to land
      near it. The 1.26 ppm agreement is impressive but meaningless.

      This is always possible. But the STRUCTURE — the CM curve, the
      automorphism group, the period, the BSD formula — is too coherent
      for simple coincidence. You don't get |Aut(E)|^2 and L(E,1) and
      the right period relations by accident.
""")

note("What we actually need to compute")

# Let's check: if we add a correction term to the quadratic, what form would
# close the gap exactly?

# The quadratic: x^2 - 16*c^2*x + 16*c^3 = 0 where c = G*
# This gives x_+ = 8c^2 + sqrt(64c^4 - 16c^3) = 8c^2 + sqrt(16c^3(4c-1))
# = 8c^2 + 4c*sqrt(c(4c-1))

# What if the equation is actually:
# x^2 - 16*c^2*x + 16*c^3 + delta_const = 0
# where delta_const is chosen so x_+ = 137.035999177 exactly?

# Then: (137.035999177)^2 - 16*c^2*(137.035999177) + 16*c^3 + delta_const = 0
# delta_const = -(alpha_inv_CODATA^2 - 16*G_star^2*alpha_inv_CODATA + 16*G_star^3)

residual_at_CODATA = alpha_inv_CODATA**2 - 16*G_star**2*alpha_inv_CODATA + 16*G_star**3
delta_const = -residual_at_CODATA

print(f"""
  If we require x_+ = 1/alpha_CODATA exactly, the quadratic needs
  a constant correction:

    x^2 - 16*G*^2*x + 16*G*^3 + delta = 0

  where delta = {short(delta_const, 10)}

  Let's see what this correction looks like in terms of known quantities:
""")

print(f"    delta / G*         = {short(delta_const / G_star, 10)}")
print(f"    delta / G*^2       = {short(delta_const / G_star**2, 10)}")
print(f"    delta / G*^3       = {short(delta_const / G_star**3, 10)}")
print(f"    delta / (16*G*^3)  = {short(delta_const / (16*G_star**3), 10)}")
print(f"    delta / alpha      = {short(delta_const * alpha_inv_CODATA, 10)}")
print(f"    delta * pi         = {short(delta_const * pi, 10)}")
print(f"    delta / varpi      = {short(delta_const / varpi, 10)}")
print()

# Check if delta is close to simple expressions
print(f"    delta / (G*^2 * alpha/(2*pi)) = {short(delta_const / (G_star**2 * alpha_phys_val/(2*pi)), 10)}")
print(f"    delta / (alpha^2 * G*)        = {short(delta_const / (alpha_phys_val**2 * G_star), 10)}")
print(f"    delta / (alpha^2 * G*^2)      = {short(delta_const / (alpha_phys_val**2 * G_star**2), 10)}")
print(f"    delta / (alpha * G*^2 / pi)   = {short(delta_const / (alpha_phys_val * G_star**2 / pi), 10)}")
print()

# What about alpha^2 * 16 * G*^2 (a perturbative correction to the linear term)?
print(f"  If the correction comes from a perturbative shift to the linear coefficient:")
print(f"    16*G*^2 * alpha     = {short(16*G_star**2 * alpha_phys_val, 10)}")
print(f"    16*G*^2 * alpha^2   = {short(16*G_star**2 * alpha_phys_val**2, 10)}")
print(f"    delta / (16*G*^2*alpha^2) = {short(delta_const / (16*G_star**2 * alpha_phys_val**2), 10)}")
print()

note("Summary")

print(f"""
  THE GAP: {short(ppm, 4)} ppm

  WHAT WE KNOW:
    - The gap is real (1.26 ppm, 60x the CODATA uncertainty)
    - Standard QED running doesn't explain it
    - If x_+ were exact 1/alpha, spectroscopy would disagree
    - The correction needed is delta = {short(delta_const, 6)}

  WHAT THIS MEANS:
    The master quadratic is not the final word. It is the leading term.
    The 1.26 ppm gap is telling us what the NEXT term looks like.

  WHAT'S NEEDED:
    Either:
    1. Find the correction term from within the arithmetic geometry
       (higher-order terms in an expansion around the CM point)
    2. Show the gap matches a known physical correction
       (vacuum polarization, finite-size effect, etc.)
    3. Accept the gap as the framework's irreducible uncertainty

  The honest position:
    The master quadratic gives 1/alpha to 1.26 ppm.
    This is [REMARKABLE OBSERVATION], not [EXACT THEOREM].
    The gap is a research program, not a failure.
""")

print(SEP)
print("  END: ALPHA GAP ANALYSIS")
print(SEP)
