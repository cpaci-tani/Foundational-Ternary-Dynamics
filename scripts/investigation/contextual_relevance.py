#!/usr/bin/env python3
"""
Contextual Relevance: alpha as gravity at different scales.

The thesis: gravity is not a separate force. It is accumulated
alpha-coupling (contextual relevance) at macroscopic scales.
The only variable between a gas cloud and a black hole is packing density.
"""
import math

# === CONSTANTS ===
alpha = 1/137.035999177
G_N = 6.67430e-11        # m^3 kg^-1 s^-2
c = 2.998e8              # m/s
hbar = 1.0546e-34        # J s
m_p = 1.6726e-27         # proton mass kg
m_e = 9.1094e-31         # electron mass kg
k_B = 1.3806e-23         # Boltzmann J/K
M_sun = 1.989e30         # kg
l_planck = 1.616e-35     # m

# === FTD hierarchy ===
N_eff = 13
b_3 = 7
prefactor = 2*math.pi * (16/3)**2 * (N_eff + 3/b_3)**2
alpha_G_FTD = prefactor * alpha**20
alpha_G_std = G_N * m_p**2 / (hbar * c)

print("=" * 78)
print("CONTEXTUAL RELEVANCE: alpha AS GRAVITY AT DIFFERENT SCALES")
print("=" * 78)

print("\n--- Fundamental Couplings ---")
print(f"  alpha (EM)            = {alpha:.6e}  = 1/{1/alpha:.6f}")
print(f"  alpha_G (grav, std)   = {alpha_G_std:.6e}")
print(f"  alpha_G (FTD)         = {alpha_G_FTD:.6e}")
print(f"  FTD/std ratio         = {alpha_G_FTD/alpha_G_std:.6f}")
print(f"  alpha^20              = {alpha**20:.6e}")
print(f"  FTD prefactor         = {prefactor:.4f}")
print(f"  alpha_G ~ alpha^{math.log(alpha_G_std)/math.log(alpha):.2f}")

# =================================================================
# THREE OBJECTS: GAS CLOUD, NEUTRON STAR, BLACK HOLE
# =================================================================
print(f"\n{'=' * 78}")
print("THREE OBJECTS: GAS CLOUD vs NEUTRON STAR vs BLACK HOLE")
print("=" * 78)

objects = [
    ("Molecular Gas Cloud", 1000*M_sun, 3.086e16*5, 20),      # 1000 Msun, 5 pc, 20K
    ("Neutron Star (1.4 Msun)", 1.4*M_sun, 10e3, 1e6),        # 1.4 Msun, 10 km
    ("Black Hole (10 Msun)", 10*M_sun, None, None),            # 10 Msun, R = R_s
]

for name, M, R, T in objects:
    R_s = 2 * G_N * M / c**2
    if R is None:
        R = R_s
        T = hbar * c**3 / (8 * math.pi * k_B * G_N * M)  # Hawking temp

    N_baryons = M / m_p
    V = (4.0/3.0) * math.pi * R**3
    rho = M / V
    Phi = G_N * M / (R * c**2)
    E_grav = 0.6 * G_N * M**2 / R
    E_rest = M * c**2
    binding = E_grav / E_rest
    alpha_G_eff = G_N * M**2 / (hbar * c)
    n_powers = math.log(alpha_G_eff) / math.log(alpha)

    print(f"\n  --- {name} ---")
    print(f"  Mass:              {M:.3e} kg  ({M/M_sun:.1f} M_sun)")
    print(f"  Radius:            {R:.3e} m   ({R/1e3:.1f} km)")
    print(f"  Schwarzschild R:   {R_s:.3e} m   ({R_s/1e3:.1f} km)")
    print(f"  N_baryons:         {N_baryons:.3e}")
    print(f"  Avg density:       {rho:.3e} kg/m^3")
    print(f"  Surface potential:  Phi = {Phi:.6e}")
    print(f"  Grav binding/rest: {binding:.6e}")
    print(f"  Self-grav coupling: alpha_G_eff = {alpha_G_eff:.3e}")
    print(f"  Equivalent to:     alpha^{n_powers:.2f}")
    print(f"  Temperature:       {T:.2e} K")

# =================================================================
# SAME MASS, DIFFERENT PACKING
# =================================================================
print(f"\n{'=' * 78}")
print("SAME MASS (1.4 M_sun), DIFFERENT PACKING")
print("=" * 78)

M_fixed = 1.4 * M_sun
N_baryons = M_fixed / m_p
R_s_fixed = 2 * G_N * M_fixed / c**2

configs = [
    ("Gas Cloud (1 parsec)",     3.086e16),
    ("Red Giant (100 R_sun)",    6.96e10),
    ("Sun-like (1 R_sun)",       6.96e8),
    ("White Dwarf (8000 km)",    8e6),
    ("Neutron Star (10 km)",     10e3),
    ("Black Hole (R_s = 4.1 km)", R_s_fixed),
]

print(f"\n  Fixed: M = 1.4 M_sun = {M_fixed:.3e} kg")
print(f"  Fixed: N_baryons = {N_baryons:.3e}")
print(f"  Fixed: R_s = {R_s_fixed:.3e} m = {R_s_fixed/1e3:.2f} km")
print(f"  Fixed: Total alpha-coupling = N * alpha = {N_baryons * alpha:.3e}")
print()

header = f"  {'Configuration':<28} {'Radius':>12} {'Density':>12} {'Phi':>12} {'Binding/mc2':>12}"
print(header)
print("  " + "-" * len(header.strip()))

for label, R in configs:
    V = (4.0/3.0) * math.pi * R**3
    rho = M_fixed / V
    Phi = G_N * M_fixed / (R * c**2)
    binding = 0.6 * G_N * M_fixed**2 / (R * M_fixed * c**2)

    # Contextual relevance interpretation
    if Phi >= 0.49:
        interp = "MAXIMUM (event horizon)"
    elif Phi >= 0.1:
        interp = "EXTREME (spacetime strongly curved)"
    elif Phi >= 1e-4:
        interp = "SIGNIFICANT (measurable GR effects)"
    elif Phi >= 1e-8:
        interp = "WEAK (Newtonian regime)"
    else:
        interp = "NEGLIGIBLE (info passes through)"

    print(f"  {label:<28} {R:>12.3e} {rho:>12.3e} {Phi:>12.6e} {binding:>12.6e}")
    print(f"  {'':28} => {interp}")

# =================================================================
# THE KEY INSIGHT
# =================================================================
print(f"\n{'=' * 78}")
print("THE CONTEXTUAL RELEVANCE SPECTRUM")
print("=" * 78)

print("""
  Object              Phi = GM/Rc^2      What happens to information
  ------              -------------      ---------------------------""")

table = [
    ("Vacuum",              0,              "Information propagates freely"),
    ("Gas cloud (1 pc)",    G_N*M_fixed/(3.086e16*c**2), "Nearly free propagation"),
    ("Earth surface",       G_N*5.97e24/(6.37e6*c**2), "Light bends 1.75 arcsec at Sun"),
    ("Sun surface",         G_N*M_sun/(6.96e8*c**2), "Light bends, clocks slow"),
    ("White dwarf",         G_N*0.6*M_sun/(8e6*c**2), "Significant redshift"),
    ("Neutron star",        G_N*1.4*M_sun/(10e3*c**2), "Strong lensing, major redshift"),
    ("Black hole (R_s)",    0.5,            "Information CANNOT escape"),
]

for name, phi, desc in table:
    print(f"  {name:<22} {phi:>14.6e}     {desc}")

print(f"""
{'=' * 78}
INTERPRETATION: GRAVITY AS ACCUMULATED ALPHA-COUPLING
{'=' * 78}

At the single-particle level:
  - One electron + one photon: coupling = alpha = 1/137
  - This is the UNIT of contextual relevance
  - It measures: how strongly does information (photon) affect matter (electron)?

At the bulk level:
  - N baryons packed into volume V
  - Each pair has EM coupling alpha, grav coupling alpha_G
  - The PACKING DENSITY determines the gravitational potential Phi = GM/Rc^2
  - Phi measures: how much do the accumulated alpha-couplings curve geometry?

The FTD hierarchy formula:
  alpha_G = 2*pi*(16/3)^2*(13+3/7)^2 * alpha^20

  This says: gravity IS alpha, compounded 20 times through the
  structural hierarchy. It is not a separate force with its own coupling.
  It is the SAME coupling (information <-> matter), accumulated.

The proof is in the packing:
  - Same 1.4 solar masses, same N_baryons, same total alpha-coupling
  - Gas cloud (1 pc):      Phi = {G_N*M_fixed/(3.086e16*c**2):.2e}  -- gravity negligible
  - Neutron star (10 km):  Phi = {G_N*M_fixed/(10e3*c**2):.2e}  -- gravity dominates
  - Black hole (R_s):      Phi = 0.5         -- gravity is everything

  The ONLY difference is how tightly the alpha-couplings are packed.
  Gravity is not pulling. Gravity is RELEVANCE -- contextual relevance
  of packed information-matter couplings.

  A black hole is not a thing with infinite gravity.
  A black hole is a region where contextual relevance reaches unity --
  where alpha-coupling density is so high that information cannot
  find a path out. Phi = 0.5 is the maximum of contextual relevance.
""")

# =================================================================
# QUANTITATIVE: ALPHA POWERS AT EACH SCALE
# =================================================================
print("=" * 78)
print("QUANTITATIVE: EFFECTIVE COUPLING AT EACH SCALE")
print("=" * 78)

print(f"\n  Scale                   N_particles    Eff coupling    = alpha^n")
print(f"  -----                   -----------    ------------    ---------")

scales = [
    ("Proton-proton",         1,           alpha_G_std),
    ("Nucleus (Fe-56)",       56,          alpha_G_std * 56**2),
    ("Atom (full)",           1,           alpha),  # this is EM, not grav
    ("Virus",                 1e7,         alpha_G_std * (1e7)**2),
    ("Cell",                  1e14,        alpha_G_std * (1e14)**2),
    ("Human (70 kg)",         70/m_p,      G_N * 70**2 / (hbar*c)),
    ("Earth",                 5.97e24/m_p, G_N * (5.97e24)**2 / (hbar*c)),
    ("Sun",                   M_sun/m_p,   G_N * M_sun**2 / (hbar*c)),
    ("Neutron star",          1.4*M_sun/m_p, G_N*(1.4*M_sun)**2/(hbar*c)),
    ("BH (10 Msun)",         10*M_sun/m_p, G_N*(10*M_sun)**2/(hbar*c)),
]

for name, N, coupling in scales:
    if coupling > 0:
        n = math.log(coupling) / math.log(alpha)
        if coupling > 1:
            print(f"  {name:<24} {N:>12.3e}    {coupling:>12.3e}    alpha^{n:.1f} (>1: DOMINANT)")
        else:
            print(f"  {name:<24} {N:>12.3e}    {coupling:>12.3e}    alpha^{n:.1f}")

print(f"""

  Note: alpha^n < 1 means gravity is weaker than EM at that scale.
  alpha^n > 1 means gravity DOMINATES at that scale.

  The crossover happens around n ~ 0, i.e., when alpha_G_eff ~ 1.
  This occurs at M ~ M_Planck = {2.176e-8*1e6:.1f} micrograms.
  Above Planck mass, gravity is THE dominant coupling.

  For stellar objects (M >> M_Planck), gravity wins by factors of 10^30+.
  But it is STILL alpha -- just alpha compounded through N^2 pairs
  of baryons, each contributing their unit of contextual relevance.
""")
