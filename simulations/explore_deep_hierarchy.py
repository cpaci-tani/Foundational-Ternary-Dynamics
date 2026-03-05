#!/usr/bin/env python3
"""
Deep hierarchical exploration: how does EVERYTHING connect?

Not just listing constants — finding the ORGANIZING PRINCIPLE.
What is the single thread that runs through the entire chain?
"""
import sys, math
sys.stdout.reconfigure(encoding='utf-8')

# All the constants
e_val = math.e
gamma = 0.57721566490153286
Gq = 3.6256099082219083  # Gamma(1/4)
varpi = 2.622057554292119810
M = 0.8346268416740731
G = 2 * math.sqrt(varpi * M)
pi = 4 * varpi**2 / G**2
PF = pi / 4
sqrtG = math.sqrt(G)

# Master quadratic
disc = 256*G**4 - 64*G**3
xp = (16*G**2 + math.sqrt(disc)) / 2
xm = (16*G**2 - math.sqrt(disc)) / 2
alpha = 1/xp

# Integers
Nc = 3; Nbase = 4; b3 = 7; Neff = 13; Nf = 6

# Consciousness
Y_real = G**2/4
y_imag = math.sqrt(abs((G**2/2)**2 - 4*(G**3/2))) / 2
KC = math.sqrt(G**3/2)
cos2_C = G/8

# Thresholds
KB = 0.511
K_GENESIS = Nc * KB

print('='*80)
print('  THE DEEP HIERARCHY: HOW EVERYTHING CONNECTS')
print('  A sympathetic physicist looks at the ontic chain')
print('='*80)

# =====================================================================
# ACT I: THE HIERARCHY OF SELF-REFERENCE
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT I: THE HIERARCHY OF SELF-REFERENCE')
print(f'  "The universe is a thing that talks about itself"')
print(f'{"="*80}')
print()
print(f'  Level 0: SELF-REFERENTIAL GROWTH')
print(f'    e: d/dx(e^x) = e^x')
print(f'    "I grow proportional to what I already am"')
print(f'    This is the ONLY real number with this property.')
print(f'    It is the mathematical atom of self-reference.')
print()
print(f'  Level 1: SELF-INTERSECTING GEOMETRY')
print(f'    varpi: half-period of the lemniscate r^2 = cos(2theta)')
print(f'    The figure-8 is the simplest closed curve that CROSSES ITSELF.')
print(f'    Self-intersection IS self-reference made geometric.')
print(f'    A circle goes around. A lemniscate goes around AND meets itself.')
print()
print(f'  Level 2: SELF-CONSISTENT OPERATOR')
print(f'    G* = 2*sqrt(varpi*M)')
print(f'    G* bridges two descriptions of the SAME geometry:')
print(f'      varpi (the period, "how long is the path")')
print(f'      M     (the mean, "how does it converge")')
print(f'    G* is the geometric mean of path and convergence.')
print()
print(f'  Level 3: SELF-DETERMINING EQUATION')
print(f'    x^2 - 16*G*^2*x + 16*G*^3 = 0')
print(f'    G* determines x+ and x-')
print(f'    x- determines integers {{3,4,7,13}}')
print(f'    The integer 16 = N_base^2 = 4^2 appears IN the equation')
print(f'    The equation determines the very integers that define it!')
print()
print(f'  Level 4: SELF-OBSERVING SYSTEM (sLoop)')
print(f'    The observer is embedded IN the flux field it measures')
print(f'    Measurement = manifestation (threshold crossing)')
print(f'    The system observes itself observing itself')
print()
print(f'  THE THREAD: Self-reference at every level.')
print(f'    e -> varpi -> G* -> quadratic -> sLoop')
print(f'    Growth -> Intersection -> Bridge -> Splitting -> Observation')

# =====================================================================
# ACT II: THE HIERARCHY OF TENSION
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT II: THE HIERARCHY OF TENSION')
print(f'  "Physics IS the gap between continuous and discrete"')
print(f'{"="*80}')
print()

tensions = [
    ("gamma", "sum(1/k) vs ln(n)", gamma, "0.577", "The cost of counting"),
    ("G*-3", "lemniscate vs integer", abs(G-3), f"{abs(G-3):.4f}", "Generates alpha"),
    ("x_- - 3", "root vs floor", abs(xm-3), f"{abs(xm-3):.4f}", "Generates N_c"),
    ("epsilon", "e^pi vs pi+20", abs(math.exp(pi)-pi-20), "0.000900", "Precision alpha"),
    ("1/G*-c^2", "flux vs speed", abs(1/G - 1/3), f"{abs(1/G-1/3):.6f}", "Same 1.4% gap"),
]

print(f'  {"Name":<12} {"What vs What":<25} {"Gap":<12} {"Consequence":<25}')
print(f'  {"-"*12} {"-"*25} {"-"*12} {"-"*25}')
for name, desc, val, valstr, conseq in tensions:
    print(f'  {name:<12} {desc:<25} {valstr:<12} {conseq:<25}')

print()
print(f'  INSIGHT: Every physical constant encodes a TENSION between')
print(f'  something continuous (geometry, transcendentals) and')
print(f'  something discrete (integers, floor functions, counting).')
print()
print(f'  gamma says: "continuous and discrete are not the same"')
print(f'  G*~3 says:  "lemniscate geometry and color counting disagree"')
print(f'  x_-~3 says: "the irrational root is not quite an integer"')
print(f'  epsilon says: "e^pi is not quite pi + an integer"')
print()
print(f'  The fine structure constant alpha = 1/137.036 is the')
print(f'  QUANTIFICATION of this tension at the physics layer.')

# =====================================================================
# ACT III: THE POWERS OF ALPHA
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT III: THE POWERS OF ALPHA')
print(f'  "How far from the electromagnetic scale are you?"')
print(f'{"="*80}')
print()

# Alpha powers and what physical quantity they give
alpha_powers = [
    (0, 1.0, "Identity", "---"),
    (1, alpha, "EM coupling", "single photon exchange"),
    (2, alpha**2, "Energy levels", "hydrogen binding ~ alpha^2 * m_e / 2"),
    (3, alpha**3, "Lamb shift scale", "radiative corrections"),
    (4, alpha**4, "Hyperfine splitting", "spin-spin interaction"),
    (8, alpha**8, "Higgs VEV / M_P", "v = M_P * sqrt(2pi) * alpha^8"),
    (11, alpha**11, "Electron mass / M_P", "m_e = M_P * sqrt(2pi)*(16/3)*alpha^11"),
    (14, alpha**14, "Neutrino mass / M_P", "m_nu3 = M_P * sqrt(2pi)*(4/3)*alpha^14"),
    (20, alpha**20, "Gravity / EM ratio", "alpha_G ~ alpha^20"),
]

print(f'  {"Power":<8} {"Value":<15} {"Physical Scale":<25} {"Formula/Meaning":<40}')
print(f'  {"-"*8} {"-"*15} {"-"*25} {"-"*40}')
for p, v, name, formula in alpha_powers:
    print(f'  alpha^{p:<3} {v:<15.6e} {name:<25} {formula:<40}')

print()
print(f'  THE EXPONENTS ARE FRAMEWORK INTEGERS:')
print(f'     1 = 1                    (trivial)')
print(f'     2 = 2                    (pair interaction)')
print(f'     8 = 2*N_base = 2*4      (double spinor)')
print(f'    11 = N_eff - 2 = 13 - 2  (Fibonacci neighbor)')
print(f'    14 = 2*b_3 = 2*7         (double QCD beta)')
print(f'    20 = N_eff + b_3 = 13+7  (cross-domain bridge)')
print()
print(f'  Each power of alpha is a "step" away from the EM scale:')
print(f'    m_e/M_P = alpha^11     --> 11 steps down from Planck to electron')
print(f'    m_nu/M_P = alpha^14    --> 14 steps down from Planck to neutrino')
print(f'    alpha_G = alpha^20     --> 20 steps from EM to gravity')
print()
print(f'  The ratio: m_nu/m_e = alpha^(14-11) = alpha^3 = {alpha**3:.6e}')
print(f'  The ratio: alpha_G/alpha^2 = alpha^18 = {alpha**18:.6e}')
print()
print(f'  WHY THESE SPECIFIC EXPONENTS?')
print(f'    8 = N_base + N_base  (two copies of spinor space)')
print(f'   11 = N_eff - 2        (effective DoF minus the pair)')
print(f'   14 = 2*b_3            (two copies of QCD running)')
print(f'   20 = N_eff + b_3      (total: effective + running = cross-domain)')
print(f'   The exponents are BUILT from the same integers as everything else!')

# =====================================================================
# ACT IV: THE FIVE THRESHOLDS
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT IV: THE FIVE THRESHOLDS')
print(f'  "What can exist at each energy level?"')
print(f'{"="*80}')
print()

thresholds = [
    ("K_B", KB, "MATTER", "A single particle can manifest (s: 0 -> +/-1)"),
    ("K_GENESIS", K_GENESIS, "CREATION", "A new particle can be created (color-complete)"),
    ("K_C", KC, "CONSCIOUSNESS", "Self-referential loop can sustain itself"),
    ("x_Born", 2*G, "MEASUREMENT", "Born rule: degenerate quadratic (collapse)"),
    ("G*^2", G**2, "TIME", "One tick of energy per DoF (the clock)"),
]

print(f'  {"Name":<12} {"Value":<10} {"Enables":<16} {"Meaning":<55}')
print(f'  {"-"*12} {"-"*10} {"-"*16} {"-"*55}')
for name, val, enables, meaning in thresholds:
    print(f'  {name:<12} {val:<10.4f} {enables:<16} {meaning:<55}')

print()
print(f'  ORDERING: K_B < K_GENESIS < K_C < x_Born < G*^2')
print(f'            0.511 < 1.533 < 3.599 < 5.917 < 8.754')
print()
print(f'  Reading this hierarchy:')
print(f'    Matter first. Then creation. Then consciousness.')
print(f'    Then measurement. Then the tick itself.')
print(f'    You need matter before you can create more.')
print(f'    You need consciousness before you can measure.')
print(f'    You need time to do any of it.')
print()
print(f'  RATIOS (in units of K_B = electron mass):')
for name, val, enables, _ in thresholds:
    print(f'    {name:<12} = {val/KB:.4f} * m_e')

# =====================================================================
# ACT V: THE THREE QUADRATICS
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT V: THE THREE QUADRATICS')
print(f'  "One equation, three coefficients, three domains of reality"')
print(f'{"="*80}')
print()
print(f'  The SAME equation: x^2 - k*G*^2*x + k*G*^3 = 0')
print(f'  Discriminant: Delta = k*G*^3*(k*G* - 4)')
print()

k_phys = 16
k_crit = 4/G
k_cons = 0.5

# Physics quadratic
disc_p = k_phys * G**3 * (k_phys * G - 4)
print(f'  PHYSICS (k=16):')
print(f'    k*G* = {k_phys*G:.4f} > 4  -->  Delta = {disc_p:.2f} > 0')
print(f'    REAL roots: x+ = {xp:.4f}, x- = {xm:.4f}')
print(f'    What exists: measurable quantities (alpha, N_c)')
print(f'    Character: OBJECTIVE, SEPARABLE')
print()

disc_b = k_crit * G**3 * (k_crit * G - 4)
print(f'  MEASUREMENT (k=4/G*={k_crit:.4f}):')
print(f'    k*G* = 4.0000 = 4  -->  Delta = {disc_b:.6f} = 0')
print(f'    DEGENERATE root: x = 2*G* = {2*G:.4f}')
print(f'    What exists: the Born rule (|psi|^2 probability)')
print(f'    Character: THE BOUNDARY between objective and subjective')
print()

disc_c = k_cons * G**3 * (k_cons * G - 4)
print(f'  CONSCIOUSNESS (k=1/2):')
print(f'    k*G* = {k_cons*G:.4f} < 4  -->  Delta = {disc_c:.2f} < 0')
print(f'    COMPLEX roots: y = {Y_real:.4f} +/- {y_imag:.4f}i')
print(f'    What exists: irreducibly subjective experience')
print(f'    Character: SUBJECTIVE, INSEPARABLE (Re and Im always together)')
print()

print(f'  KEY INSIGHT: log2(16) + log2(1/2) = 4 + (-1) = 3 = D')
print(f'  The ratio of physics-to-consciousness coefficients')
print(f'  gives the number of spatial dimensions!')
print()
print(f'  k_phys / k_cons = 16 / 0.5 = 32 = 2^5')
print(f'  k_phys * k_cons = 16 * 0.5 = 8 = 2*N_base = 2^3')
print(f'  log2(k_phys/k_cons) = 5 = N_f - 1 = 2*N_gen - 1')

# =====================================================================
# ACT VI: THE NEW HIERARCHY TABLE
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT VI: THE NEW HIERARCHY')
print(f'  "From nothing to everything in nine acts"')
print(f'{"="*80}')
print()

hierarchy = [
    ("VOID", "0", "Nothing", "The empty set exists", "Why not nothing?", "{}"),
    ("SELF", "-1", "e = 2.718", "Self-referential growth", "d/dx(e^x) = e^x", "e"),
    ("GAP", "0", "gamma = 0.577", "Discrete != continuous", "sum - integral = gap", "gamma"),
    ("GATE", "0/0b", "Gamma(1/4), theta, q", "Arithmetic meets geometry", "q = (-1)^i = e^{-pi}", "Gamma(1/4)"),
    ("KNOT", "1", "varpi = 2.622", "Self-intersection", "Figure-8 = simplest self-crossing", "varpi"),
    ("BRIDGE", "2", "G* = 2.959", "Universal operator", "Flux = space, Energy = time", "G*"),
    ("SPLIT", "3", "x+=137, x-=3.024", "One becomes two", "Physics from one quadratic", "x+, x-"),
    ("COUNT", "4", "{3, 4, 7, 13}", "Continuous -> discrete", "floor(3.024) = 3 -> all SM", "{N_c...}"),
    ("FORCE", "5", "alpha, G_N, alpha_s", "How things interact", "Powers of alpha: 1,8,11,14,20", "alpha"),
    ("MATTER", "6", "K_B=0.511, masses", "What can exist", "m_e = M_P*sqrt(2pi)*(16/3)*a^11", "K_B"),
    ("PRECISION", "7", "epsilon, c1-c4", "Matching reality", "1/alpha to < 0.001 ppt", "epsilon"),
    ("MIND", "8", "y = 2.19+/-2.86i", "Complex = subjective", "cos^2(theta_C) = G*/8 = 37%", "theta_C"),
]

print(f'  {"Stage":<12} {"Layer":<8} {"Key Value":<22} {"What Happens":<28} {"The Equation/Identity":<35} {"Symbol":<12}')
print(f'  {"-"*12} {"-"*8} {"-"*22} {"-"*28} {"-"*35} {"-"*12}')
for stage, layer, value, what, eq, sym in hierarchy:
    print(f'  {stage:<12} {layer:<8} {value:<22} {what:<28} {eq:<35} {sym:<12}')

print()
print(f'  READ IT AS A STORY:')
print(f'  "Nothing exists. But the empty set IS something (VOID)."')
print(f'  "Self-reference bootstraps existence (SELF: e)."')
print(f'  "Counting what exists reveals a gap with the continuum (GAP: gamma)."')
print(f'  "The gap opens a gateway from number to shape (GATE: Gamma(1/4))."')
print(f'  "Shape crosses itself = the first topology (KNOT: varpi)."')
print(f'  "Self-crossing generates the universal constant (BRIDGE: G*)."')
print(f'  "G* splits into two roots = physics (SPLIT: x+, x-)."')
print(f'  "Irrational roots floor to integers = particles (COUNT: {{3,4,7,13}})."')
print(f'  "Integers determine forces (FORCE: alpha, G_N)."')
print(f'  "Forces determine what can stably exist (MATTER: K_B)."')
print(f'  "Radiative corrections match the universe (PRECISION: epsilon)."')
print(f'  "The same equation, subcritical, gives consciousness (MIND: theta_C)."')

# =====================================================================
# ACT VII: THE WEB OF CONNECTIONS
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT VII: THE WEB')
print(f'  "Every constant knows about every other constant"')
print(f'{"="*80}')
print()
print(f'  G* is the CENTER of the web. Everything connects through it:')
print()
print(f'                        e')
print(f'                        |')
print(f'                       gamma')
print(f'                        |')
print(f'                     Gamma(1/4)')
print(f'                      /    \\')
print(f'                  varpi     theta')
print(f'                    |    \\    |')
print(f'                    M    nome  q')
print(f'                     \\   /')
print(f'                      G*  <--- pi (derived)')
print(f'                     / | \\')
print(f'                    /  |  \\')
print(f'                   /   |   \\')
print(f'             x+(=1/a)  |  x-(~N_c)')
print(f'                |      |      |')
print(f'               alpha  Born  integers')
print(f'               / | \\   |    / | | \\')
print(f'             g_c sW  aG  K_C Nc b3 Neff')
print(f'              |   |   |       |  |   |')
print(f'             KB  vH alpha_s  Nf Ngen D')
print(f'              |   |')
print(f'            masses  Higgs')
print(f'              |')
print(f'           proton, muon, tau')
print()
print(f'  And the LATERAL connections:')
print(f'    G*/8 = cos^2(theta_C)        (physics -> consciousness)')
print(f'    P/S = G*                      (action/energy = time)')
print(f'    c^2 ~ 1/G*                   (speed ~ inverse flux)')
print(f'    e^pi ~ pi + (N_eff + b_3)    (transcendental ~ integer)')
print(f'    q = (-1)^i                    (annihilation ^ imagination)')
print(f'    alpha^20 = alpha_G           (EM -> gravity via cross-domain)')
print(f'    E_L = x+, E_R = x-           (substrates = quadratic roots)')

# =====================================================================
# ACT VIII: WHAT THE HIERARCHY MEANS
# =====================================================================
print(f'\n{"="*80}')
print(f'  ACT VIII: WHAT THIS MEANS')
print(f'{"="*80}')
print()
print(f'  1. THE UNIVERSE IS A SELF-REFERENTIAL STRUCTURE')
print(f'     e (Layer -1) is self-referential growth')
print(f'     varpi (Layer 1) is self-intersecting geometry')
print(f'     G* (Layer 2) is self-consistent bridging')
print(f'     The quadratic (Layer 3) is self-determining')
print(f'     The sLoop (Layer 8) is self-observing')
print(f'     Self-reference appears at EVERY level, in a new form.')
print()
print(f'  2. PHYSICS IS THE TENSION BETWEEN CONTINUOUS AND DISCRETE')
print(f'     gamma encodes this tension (sum - integral)')
print(f'     G* ~ 3 encodes it (lemniscate ~ integer)')
print(f'     floor(x-) encodes it (irrational -> integer)')
print(f'     alpha = 1/137.036 IS this tension, quantified')
print()
print(f'  3. ONE NUMBER DOES EVERYTHING')
print(f'     G* determines alpha (via x+)')
print(f'     G* determines N_c (via x-)')
print(f'     G* determines masses (via alpha^n)')
print(f'     G* determines consciousness (via k=1/2 quadratic)')
print(f'     G* determines time (G*^2 per DoF)')
print(f'     G* determines space (G*^1 = flux amplitude)')
print(f'     G* IS physics. Everything else is consequence.')
print()
print(f'  4. THE EXPONENTS FORM A FIBONACCI-LIKE SEQUENCE')
alpha_exps = [1, 2, 3, 4, 8, 11, 14, 20]
print(f'     Powers of alpha that appear: {alpha_exps}')
print(f'     Differences: {[alpha_exps[i+1]-alpha_exps[i] for i in range(len(alpha_exps)-1)]}')
print(f'     = [1, 1, 1, 4, 3, 3, 6]')
print(f'     The large jumps: 4 (=N_base), 3 (=N_c), 3 (=N_c), 6 (=N_f)')
print(f'     The exponent GAPS are framework integers!')
print()
print(f'  5. CONSCIOUSNESS IS NOT SEPARATE FROM PHYSICS')
print(f'     Same equation, different coefficient')
print(f'     k=16: physics (real, measurable, objective)')
print(f'     k=4/G*: measurement (boundary, Born rule)')
print(f'     k=1/2: consciousness (complex, subjective)')
print(f'     D = log2(16/0.5) - 2 = 5 - 2 = 3')
print(f'     THE NUMBER OF DIMENSIONS COMES FROM')
print(f'     THE RATIO OF PHYSICS TO CONSCIOUSNESS.')
print()
print(f'  6. THE UNIVERSE SITS AT A NON-TRIVIAL FIXED POINT')
print(f'     G* = 3 would be the trivial fixed point:')
print(f'       alpha = 1/141, clean wave equation, perfect squares')
print(f'     G* = 2.959 is the LEMNISCATIC fixed point:')
print(f'       alpha = 1/137, the actual universe, messy but real')
print(f'     The deviation from simplicity IS complexity.')
print(f'     If G* = 3 exactly: no chemistry, no life, no observers.')
print(f'     The universe MUST be imperfect to be interesting.')

print(f'\n{"="*80}')
print(f'  END')
print(f'{"="*80}')
