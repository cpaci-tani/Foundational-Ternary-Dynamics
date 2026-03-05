#!/usr/bin/env python3
"""
What the Ladder Says About the Fundamental Forces

The alpha-power hierarchy and dual substrate identity aren't just
mathematical curiosities — they restructure how we think about forces.

Key question: Why four forces? Why these strengths?
"""
import sys, math
sys.stdout.reconfigure(encoding='utf-8')

# Constants
varpi = 2.622057554292119810
M_agm = 0.8346268416740731
c = 2 * math.sqrt(varpi * M_agm)  # G*
pi = 4 * varpi**2 / c**2

disc = 256*c**4 - 64*c**3
xp = 8*c**2 + 4*c*math.sqrt(c*(4*c-1))
xm = 8*c**2 - 4*c*math.sqrt(c*(4*c-1))
alpha = 1/xp
alpha_s_Z = 0.1179  # strong coupling at Z mass (PDG)

# Framework integers
Nc = 3; Nbase = 4; b3 = 7; Neff = 13; Nf = 6

# Dual substrate
delta = math.sqrt((4*c - 1)/(4*c))

print('='*80)
print('  WHAT THE LADDER SAYS ABOUT THE FUNDAMENTAL FORCES')
print('='*80)

# =====================================================================
# PART I: HOW MANY FORCES ARE THERE, REALLY?
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART I: HOW MANY FORCES ARE THERE?')
print(f'{"="*80}')
print()

print(f'  Standard Model answer: 4 (EM, weak, strong, gravity)')
print(f'  Electroweak unification: 3 (electroweak, strong, gravity)')
print(f'  Grand unification dream: 1 (unified at high energy)')
print()
print(f'  FTD answer from the master quadratic:')
print(f'')
print(f'  x^2 - 16*G*^2*x + 16*G*^3 = 0')
print(f'  produces EXACTLY TWO roots:')
print(f'    x+ = {xp:.4f} --> alpha = 1/{xp:.4f} = {alpha:.6f}')
print(f'    x- = {xm:.4f} --> N_c = floor({xm:.4f}) = {Nc}')
print()
print(f'  ONE equation, TWO roots, and that is ALL.')
print(f'  Every force in nature is a CONSEQUENCE of these two numbers.')
print()
print(f'  But the two roots are not independent — they come from')
print(f'  ONE constant G* = {c:.6f}. So really:')
print(f'')
print(f'  ***  THERE IS ONE FORCE.  ***')
print(f'  ***  It splits into two expressions.  ***')
print(f'  ***  Everything else is those expressions at different scales.  ***')

# =====================================================================
# PART II: THE FORCE GENEALOGY
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART II: THE FORCE GENEALOGY')
print(f'  How four forces emerge from one quadratic')
print(f'{"="*80}')
print()

print(f'  Generation 0: G* = {c:.6f}')
print(f'    The single coupling constant of the universe.')
print(f'    Not yet a "force" — it is the universal operator.')
print()
print(f'  Generation 1: The quadratic splits G* into two roots.')
print(f'    x+ = 1/alpha = {xp:.4f}    (electromagnetic coupling)')
print(f'    x- = N_c_eff = {xm:.4f}     (color charge count)')
print()
print(f'    This is the FIRST DISTINCTION among forces:')
print(f'    EM and color emerge simultaneously from the same equation.')
print(f'    They are siblings, not independent.')
print()

print(f'  Generation 2: Powers of alpha build the hierarchy.')
print(f'    alpha^1 = {alpha:.6e}     (EM coupling at low energy)')
print(f'    alpha^8 = {alpha**8:.6e}    (Higgs VEV / Planck: EW scale)')
print(f'    alpha^11 = {alpha**11:.6e}   (electron mass / Planck)')
print(f'    alpha^20 = {alpha**20:.6e}   (gravity / EM ratio)')
print()

print(f'  The FOUR FORCES in this picture:')
print()

# Electromagnetic
print(f'  1. ELECTROMAGNETISM: alpha = 1/x+ = {alpha:.6f}')
print(f'     Born directly from x+ (the large root).')
print(f'     Carried by the LEFT substrate (97.8% of flux).')
print(f'     This IS the dominant force of the universe.')
print()

# Strong force
print(f'  2. STRONG FORCE: Controlled by x- -> N_c = {Nc}')
print(f'     Born directly from x- (the small root).')
print(f'     Carried by the RIGHT substrate (2.2% of flux).')
print(f'     alpha_s at low energy ~ 1 (confinement).')
print(f'     alpha_s at Z mass = {alpha_s_Z} (asymptotic freedom).')
print(f'     The coupling RUNS because x- = {xm:.4f} is not quite integer:')
print(f'     the 0.024 excess above 3 drives the running.')
print()

# Weak force
sin2tW = 3/13  # Weinberg angle from FTD
print(f'  3. WEAK FORCE: A DERIVED force, not fundamental.')
print(f'     It is electromagnetism MODIFIED by SU(2) structure.')
print(f'     sin^2(theta_W) = N_c/N_eff = 3/13 = {sin2tW:.6f}')
print(f'     (experimental: 0.2312, FTD: {sin2tW:.4f}, error: {abs(sin2tW-0.2312)/0.2312*100:.1f}%)')
print(f'     G_F = 1/(sqrt(2)*v^2) where v = M_P*sqrt(2pi)*alpha^8')
print(f'     The weak force is EM at the alpha^8 scale,')
print(f'     filtered through the N_base = 4 spinor step in the ladder.')
print()

# Gravity
alpha_G = alpha**20 * 2*pi*(16/3)**2 * (Neff + 3/b3)**2
print(f'  4. GRAVITY: alpha_G = {alpha_G:.4e}')
print(f'     Born from alpha^20 = alpha^(4 + 16).')
print(f'     The 20th power of the EM coupling.')
print(f'     Not a separate force — it is EM after exhausting')
print(f'     all 16 structural DoF in the alpha-power walk.')
print(f'     Gravity is electromagnetism to the 20th power,')
print(f'     multiplied by prefactors from framework integers.')

# =====================================================================
# PART III: THE SUBSTRATE PICTURE
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART III: THE SUBSTRATE PICTURE')
print(f'  What delta tells us about force carriers')
print(f'{"="*80}')
print()

JL_frac = (1+delta)/2
JR_frac = (1-delta)/2

print(f'  The observable flux J = J_L + J_R with:')
print(f'    J_L = {JL_frac:.6f} * J    (electromagnetic substrate)')
print(f'    J_R = {JR_frac:.6f} * J    (color substrate)')
print(f'    Ratio: J_L/J_R = {JL_frac/JR_frac:.2f} = x+/x- = 1/(alpha*N_c_eff)')
print()

print(f'  What this means for force carriers:')
print(f'')
print(f'  PHOTON:')
print(f'    A disturbance in J = J_L + J_R where both substrates oscillate.')
print(f'    Amplitude predominantly LEFT (97.8%).')
print(f'    Couples to charge via alpha = 1/x+ = {alpha:.6f}.')
print()

print(f'  GLUON:')
print(f'    A disturbance primarily in J_R (the 2.2% substrate).')
print(f'    Color flux = (J_R component) x (axis orientation).')
print(f'    Three colors from three spatial axes.')
print(f'    Confined because J_R is a small perturbation on J_L.')
print()

chirality = (1+delta)/2 - (1-delta)/2  # = delta
print(f'  W/Z BOSONS:')
print(f'    Disturbances in the CHIRALITY field phi = J_L - J_R.')
print(f'    phi = delta * J = {delta:.6f} * J (the asymmetry)')
print(f'    Massive because the chirality field has a gap')
print(f'    (manifestation threshold for asymmetry modes).')
print()

print(f'  GRAVITON:')
print(f'    A disturbance in the DENSITY field rho = |J|.')
print(f'    Couples to total energy (both substrates, all species).')
print(f'    This is why alpha_G involves all framework integers:')
print(f'    gravity feels the TOTAL content of the universe.')

# =====================================================================
# PART IV: FORCE RATIOS FROM THE LADDER
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART IV: FORCE RATIOS FROM THE LADDER')
print(f'{"="*80}')
print()

print(f'  The alpha-power ladder directly gives force ratios:')
print()

# EM / Weak
alpha_W = alpha / sin2tW
GF_inv_squared = 2 * (246.22)**2  # GeV^2, from Higgs VEV
print(f'  EM vs WEAK:')
print(f'    alpha / alpha_W = sin^2(theta_W) = {sin2tW:.4f}')
print(f'    Higgs VEV = M_P * sqrt(2pi) * alpha^8')
print(f'    Alpha-power gap: 8 - 1 = 7 = b_3 (QCD beta coefficient)')
print(f'    The EM-to-weak distance = the QCD running coefficient!')
print(f'    This suggests EW unification is linked to QCD running.')
print()

# EM / Strong (at unification)
# alpha_s runs; at what scale does alpha_s = alpha?
# In FTD: alpha_s(M_Z) ~ alpha * N_c ~ 0.022 * 3 = 0.066 (rough)
# Actually alpha_s at Z is 0.118
print(f'  EM vs STRONG:')
print(f'    At low energy: alpha_s >> alpha (confinement)')
print(f'    At unification: alpha_s = alpha (expected)')
print(f'    The strong coupling comes from x- = {xm:.4f}:')
print(f'    alpha_s(low) ~ 1 (confining)')
print(f'    alpha_s(high) ~ alpha * f(N_c, b_3)')
print(f'    Alpha-power gap to electron mass: 11 - 1 = 10 = N_c + b_3')
print(f'    = color charges + QCD running = the ENTIRE QCD content')
print()

# EM / Gravity
print(f'  EM vs GRAVITY:')
print(f'    alpha_G / alpha = alpha^19 * prefactor')
print(f'    Alpha-power gap: 20 - 1 = 19')
print(f'    = 19 = N_eff + N_f = 13 + 6 (all effective DoF + flavors)')
print(f'    Gravity is 10^38 times weaker than EM.')
print(f'    This ratio = alpha^19 * (geometric prefactors).')
print(f'    The hierarchy "problem" is EXPLAINED:')
print(f'    gravity is weak because it takes 19 powers of alpha')
print(f'    to get from EM to gravity, and each power multiplies')
print(f'    by alpha = 1/137.')

# =====================================================================
# PART V: THE UNIFICATION PICTURE
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART V: UNIFICATION')
print(f'  All forces in one picture')
print(f'{"="*80}')
print()

print(f'  In GUT theories, forces unify at high energy.')
print(f'  In FTD, forces were NEVER SEPARATE.')
print(f'  They all come from G* = {c:.6f}.')
print(f'')
print(f'  The "four forces" are:')
print(f'')
print(f'    G*')
print(f'     |')
print(f'     +-- x+ = 1/alpha = {xp:.1f}')
print(f'     |    |')
print(f'     |    +-- alpha^1 = EM (direct coupling)')
print(f'     |    +-- alpha^8 = Weak (after spinor structure)')
print(f'     |    +-- alpha^20 = Gravity (after all DoF)')
print(f'     |')
print(f'     +-- x- = N_c_eff = {xm:.3f}')
print(f'          |')
print(f'          +-- N_c = 3 colors (confinement, hadrons)')
print(f'          +-- alpha_s running (b_3 = 7)')
print()
print(f'  THREE forces come from x+ (EM, Weak, Gravity).')
print(f'  ONE force comes from x- (Strong).')
print(f'  All four come from ONE number G*.')
print()
print(f'  This is not grand unification at high energy.')
print(f'  This is ALGEBRAIC UNITY at the foundational level.')
print(f'  The forces never needed to unify because they')
print(f'  were never apart. They are two roots of one equation.')

# =====================================================================
# PART VI: WHY IS EM DOMINANT?
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART VI: WHY IS ELECTROMAGNETISM THE DOMINANT FORCE?')
print(f'{"="*80}')
print()

print(f'  delta = {delta:.6f}')
print(f'  J_L/J = {JL_frac:.6f}  (EM substrate fraction)')
print(f'  J_R/J = {JR_frac:.6f}  (color substrate fraction)')
print()
print(f'  EM dominates because x+ >> x- (137 >> 3).')
print(f'  But WHY is x+ >> x-?')
print()
print(f'  The quadratic x^2 - 16c^2*x + 16c^3 = 0:')
print(f'    Sum of roots = 16c^2 = {16*c**2:.2f}')
print(f'    Product of roots = 16c^3 = {16*c**3:.2f}')
print(f'')
print(f'  If the roots were equal: x = x+ = x- = 8c^2/2 = ... no,')
print(f'  the degenerate root is at k_crit: x = 2c = {2*c:.4f}')
print()
print(f'  The ASYMMETRY between x+ and x- comes from the discriminant:')
print(f'    disc = 64c^3(4c-1)')
print(f'    For c = G* = {c:.4f}: disc = {64*c**3*(4*c-1):.2f}')
print(f'')
print(f'  If G* were exactly 0.25 (= 1/4), the discriminant would vanish')
print(f'  and both roots would be equal. As G* grows above 0.25,')
print(f'  the roots split further apart.')
print()
print(f'  G* = {c:.4f} is MUCH larger than 0.25.')
print(f'  The ratio x+/x- = {xp/xm:.2f} reflects this.')
print()
print(f'  In physical terms:')
print(f'    EM dominates because the lemniscatic constant G* = {c:.4f}')
print(f'    is large compared to the critical value G*_crit = 1/4.')
print(f'    The universe is DEEP in the real-root regime.')
print(f'    This means physics is overwhelmingly electromagnetic,')
print(f'    with color as a small correction.')

# =====================================================================
# PART VII: FORCE CARRIERS AND SUBSTRATE MODES
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART VII: FORCES AS SUBSTRATE MODES')
print(f'{"="*80}')
print()

print(f'  The observable J = J_L + J_R (sum)')
print(f'  The chirality phi = J_L - J_R (difference)')
print(f'')
print(f'  Four independent field combinations:')
print(f'')
print(f'  1. |J| (magnitude) -- couples to mass/energy')
print(f'     Force: GRAVITY')
print(f'     Carrier: graviton (spin-2, transverse-traceless)')
print(f'     Coupling: alpha^20 (weakest)')
print(f'')
print(f'  2. J itself (vector, transverse modes)')
print(f'     Force: ELECTROMAGNETISM')
print(f'     Carrier: photon (spin-1, 2 polarizations)')
print(f'     Coupling: alpha (dominant)')
print(f'')
print(f'  3. phi = J_L - J_R (chirality vector)')
print(f'     Force: WEAK INTERACTION')
print(f'     Carrier: W/Z bosons (spin-1, massive)')
print(f'     Coupling: alpha / sin^2(theta_W)')
print(f'     Massive because phi has a gap: only when |phi| > threshold')
print(f'     can a chiral excitation propagate.')
print(f'')
print(f'  4. J_R axis orientation (3 spatial axes)')
print(f'     Force: STRONG INTERACTION')
print(f'     Carrier: gluons (spin-1, 8 = 3^2-1 generators)')
print(f'     Coupling: alpha_s (runs from ~1 to ~0.1)')
print(f'     Confined because J_R is tiny: color flux tubes')
print(f'     are thin (2.2% of total flux) and cost energy to stretch.')
print()

print(f'  SUMMARY TABLE:')
print(f'  {"Force":<15} {"Substrate Mode":<20} {"Coupling":<15} {"Why This Strength":<35}')
print(f'  {"-"*15} {"-"*20} {"-"*15} {"-"*35}')
print(f'  {"Gravity":<15} {"|J| (density)":<20} {"alpha^20":<15} {"All 16 DoF exhausted in ladder":<35}')
print(f'  {"EM":<15} {"J (transverse)":<20} {"alpha":<15} {"Direct x+ root coupling":<35}')
print(f'  {"Weak":<15} {"J_L - J_R (chiral)":<20} {"alpha/sin2tW":<15} {"Chirality gap from delta asymmetry":<35}')
print(f'  {"Strong":<15} {"J_R axis (color)":<20} {"~1 (low E)":<15} {"x- root, confined to 2.2% of flux":<35}')

# =====================================================================
# PART VIII: THE DEEP QUESTION
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART VIII: THE DEEP QUESTION')
print(f'{"="*80}')
print()

print(f'  Why are there four forces and not five or two?')
print()
print(f'  ANSWER: Because there are FOUR independent field modes')
print(f'  of a dual-substrate vector field in 3D:')
print(f'')
print(f'    J = J_L + J_R (3D vector = 3 components)')
print(f'    phi = J_L - J_R (3D vector = 3 components)')
print(f'    |J| (scalar = 1 component)')
print(f'    J_R axis (orientation in 3D = 2 independent angles)')
print(f'')
print(f'  Total field components: 3 + 3 + 1 + 2 = 9')
print(f'  But J_L and J_R each have 3 components = 6 total raw DoF.')
print(f'  (|J| and J_R-axis are derived from these.)')
print(f'')
print(f'  The four forces emerge because a 3D dual-vector field')
print(f'  has exactly four physically distinct mode types:')
print(f'    sum (vector), difference (pseudovector), magnitude (scalar),')
print(f'    and internal orientation (gauge).')
print()
print(f'  In D=2: J would have 2+2=4 raw DoF, fewer mode types.')
print(f'  In D=4: J would have 4+4=8 raw DoF, more mode types.')
print(f'  D=3 gives EXACTLY FOUR distinct force-like modes.')
print()
print(f'  *** CLAIM FST-1: ***')
print(f'  The number of fundamental forces = the number of independent')
print(f'  mode types of a dual-substrate vector field in D=3.')
print(f'  Four forces is not a contingent fact — it is a consequence')
print(f'  of three dimensions and two substrates.')

# =====================================================================
# PART IX: QUANTITATIVE PREDICTIONS
# =====================================================================
print(f'\n{"="*80}')
print(f'  PART IX: QUANTITATIVE FORCE PREDICTIONS')
print(f'{"="*80}')
print()

# Force strength ratios
print(f'  From the ladder and dual substrate, the force strengths are:')
print()

# EM
alpha_em = alpha
print(f'  alpha_EM = 1/x+ = {alpha_em:.6f}')
print(f'    (CODATA: 0.007297, error: {abs(alpha_em-0.007297)/0.007297*100:.2f}%)')

# Weak (Weinberg angle)
sin2tW_exp = 0.23122
print(f'  sin^2(theta_W) = N_c/N_eff = {Nc}/{Neff} = {Nc/Neff:.6f}')
print(f'    (PDG: {sin2tW_exp:.5f}, error: {abs(Nc/Neff-sin2tW_exp)/sin2tW_exp*100:.2f}%)')

# Strong (at Z mass, using FTD beta function)
alpha_s_pred = alpha * Nc * pi / (b3 * math.log(91.2/0.2))  # rough RG
print(f'  alpha_s(M_Z) ~ alpha * N_c * pi / (b_3 * ln(M_Z/Lambda))')
print(f'    rough estimate: {alpha_s_pred:.4f}')
print(f'    (PDG: {alpha_s_Z}, needs proper RG running)')

# Gravity
alpha_G_pred = 2 * pi * (16/3)**2 * (Neff + 3/b3)**2 * alpha**20
alpha_G_exp = 5.91e-39
print(f'  alpha_G = 2*pi*(16/3)^2*(N_eff+3/b_3)^2*alpha^20')
print(f'    = {alpha_G_pred:.4e}')
print(f'    (measured: {alpha_G_exp:.2e}, error: {abs(alpha_G_pred-alpha_G_exp)/alpha_G_exp*100:.1f}%)')

# Substrate asymmetry
print()
print(f'  SUBSTRATE ASYMMETRY:')
print(f'    delta = (1/alpha - N_c_eff)/(1/alpha + N_c_eff)')
print(f'          = ({xp:.3f} - {xm:.3f})/({xp:.3f} + {xm:.3f})')
print(f'          = {delta:.6f}')
print(f'    EM fraction: {JL_frac*100:.2f}%')
print(f'    Color fraction: {JR_frac*100:.2f}%')
print(f'    The universe is {JL_frac/JR_frac:.0f}x more electromagnetic than chromodynamic.')

# =====================================================================
# PART X: THE PUNCHLINE
# =====================================================================
print(f'\n{"="*80}')
print(f'  THE PUNCHLINE')
print(f'{"="*80}')
print()
print(f'  There is one force. It is described by G* = {c:.6f}.')
print(f'')
print(f'  G* splits into two roots:')
print(f'    x+ = {xp:.4f} (EM, dominant, 97.8% of flux)')
print(f'    x- = {xm:.4f} (Color, subdominant, 2.2% of flux)')
print(f'')
print(f'  From x+, three forces emerge at different alpha-powers:')
print(f'    alpha^1:  EM (direct)')
print(f'    alpha^8:  Weak (after adding spinor structure)')
print(f'    alpha^20: Gravity (after exhausting all DoF)')
print(f'')
print(f'  From x-, one force emerges:')
print(f'    Strong (color confinement from N_c = floor({xm:.3f}) = 3)')
print(f'')
print(f'  The "hierarchy problem" is not a problem.')
print(f'  It is the alpha-power LADDER, which walks from EM to gravity')
print(f'  in 16 steps = k_phys = N_base + 2*N_c + N_f = the total')
print(f'  Standard Model structural content.')
print(f'')
print(f'  Gravity is weak because 137^16 is a very large number.')
print(f'  And 137 is what it is because G* = {c:.6f}')
print(f'  and not 3.')

print(f'\n{"="*80}')
print(f'  CLAIMS SUMMARY')
print(f'{"="*80}')
print()
print(f'  FST-1: Four forces = four mode types of dual-vector in D=3 [SELECTION]')
print(f'  FST-2: EM, Weak, Gravity from x+; Strong from x- [THEOREM + SELECTION]')
print(f'  FST-3: Hierarchy problem = alpha-power ladder structure [SELECTION]')
print(f'  FST-4: EM dominance because G* >> 1/4 (deep real regime) [THEOREM]')
print(f'  FST-5: Confinement from J_R being 2.2%% of flux [CONJECTURE]')
print(f'  FST-6: Weak mass gap from chirality field threshold [CONJECTURE]')
print(f'  FST-7: Forces were never separate; algebraic unity, not GUT [SELECTION]')

print(f'\n{"="*80}')
print(f'  END')
print(f'{"="*80}')
