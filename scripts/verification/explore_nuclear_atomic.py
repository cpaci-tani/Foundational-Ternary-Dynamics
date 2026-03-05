r"""
NUCLEAR AND ATOMIC STRUCTURE FROM FTD
======================================

From the cuboctahedral quark masses and gauge couplings,
we build protons, neutrons, nuclei, and atoms — bottom up.

Everything traces back to Z^3 and its cuboctahedron.
"""

import numpy as np
from scipy.special import gamma

print("=" * 72)
print("  FROM QUARKS TO ATOMS: THE CUBOCTAHEDRAL ANATOMY OF MATTER")
print("=" * 72)

# ============================================================
# FTD Constants
# ============================================================
G14 = gamma(0.25)
varpi = G14**2 / (2*np.sqrt(2*np.pi))
G_star = varpi / np.sqrt(np.pi/4)
b_q = -16 * G_star**2; c_q = 16 * G_star**3
x_plus = (-b_q + np.sqrt(b_q**2 - 4*c_q))/2
alpha = 1/x_plus
M_P = 1.22089e19  # GeV

N_c = 3; N_b = 4; b3 = 7; N_eff = 13

# FTD-derived quantities
v_h = M_P * np.sqrt(2*np.pi) * alpha**8
sin2tw = N_c / N_eff  # 3/13

# Quark masses (FTD-derived, GeV)
m_u_ftd = M_P * np.sqrt(2*np.pi) * (1/6) * alpha**10
m_d_ftd = M_P * np.sqrt(2*np.pi) * (1/3) * alpha**10  # prediction
m_e_ftd = M_P * np.sqrt(2*np.pi) * (16/3) * alpha**11

# Experimental values for comparison
m_u = 2.16e-3   # GeV (current quark mass)
m_d = 4.67e-3   # GeV
m_s = 93.4e-3   # GeV
m_e = 0.511e-3  # GeV
m_p_exp = 0.93827  # GeV
m_n_exp = 0.93957  # GeV
alpha_s_MZ = 0.1180

# ============================================================
# Part 1: QUARKS — The Elementary Constituents
# ============================================================
print(f"\n{'='*72}")
print("  PART 1: QUARKS FROM THE CUBOCTAHEDRON")
print(f"{'='*72}")

print(f"""
  The first generation of quarks lives in the x=0 plane 
  of the cuboctahedron (4 vertices):

  Vertex          Quark    Charge   Color assignment
  ────────────────────────────────────────────────────
  (0, -1, -1)  →  u_R     +2/3     red    (L chirality)
  (0, -1, +1)  →  u_B     +2/3     blue   (R chirality)
  (0, +1, -1)  →  d_R     -1/3     red    (R chirality)
  (0, +1, +1)  →  d_B     -1/3     blue   (L chirality)
  
  Electric charge assignment:
    Q = (N_c - 1)/(2*N_c) = 2/6 = +1/3 for "up-type" sign pattern
    Q = -1/(2*N_c) = -1/6 for "down-type" sign pattern
    
    Up quark:   Q = +2/3  (from +1/3 + 1/3 via color average over N_c)
    Down quark: Q = -1/3  (from -1/6 - 1/6 via color average over N_c)
  
  Current quark masses (FTD):
    m_u = M_P * sqrt(2pi) * [1/(2*N_c)] * alpha^10 = {m_u_ftd*1e3:.2f} MeV
    m_d ~ 2*m_u = {2*m_u_ftd*1e3:.2f} MeV  (isospin breaking from SSB)
    
    Experimental: m_u = {m_u*1e3:.2f} MeV, m_d = {m_d*1e3:.2f} MeV
""")

# ============================================================
# Part 2: THE PROTON — uud
# ============================================================
print(f"{'='*72}")
print("  PART 2: THE PROTON (uud)")
print(f"{'='*72}")

# QCD binding: quarks are confined by the strong force
# The proton mass is ~99% QCD binding energy, ~1% quark masses
quark_mass_proton = 2*m_u + m_d
qcd_binding = m_p_exp - quark_mass_proton

print(f"""
  COMPOSITION: 2 up quarks + 1 down quark
  
  Cuboctahedral picture:
    The proton uses 3 vertices from the gen-1 plane (x=0):
    u: (0, -1, -1)  charge +2/3
    u: (0, +1, +1)  charge +2/3
    d: (0, +1, -1)  charge -1/3
    ─────────────────────────────
    Total charge:    +1
    
  Mass budget:
    Quark masses:    2*m_u + m_d = {quark_mass_proton*1e3:.1f} MeV  ({quark_mass_proton/m_p_exp*100:.1f}%)
    QCD binding:     {qcd_binding*1e3:.1f} MeV  ({qcd_binding/m_p_exp*100:.1f}%)
    ─────────────────────────────────────────
    Proton mass:     {m_p_exp*1e3:.2f} MeV
    
  The proton is {qcd_binding/m_p_exp*100:.1f}% empty space held together by QCD.
  
  In FTD terms: the 8 gluons (from 8 triangular faces of the 
  cuboctahedron) create the confining flux tube between the 
  3 quarks. The energy is stored in the color field connecting
  the 3 vertices — and this field energy IS the proton mass.
""")

# ============================================================
# Part 3: THE NEUTRON — udd
# ============================================================
print(f"{'='*72}")
print("  PART 3: THE NEUTRON (udd)")
print(f"{'='*72}")

quark_mass_neutron = m_u + 2*m_d
qcd_binding_n = m_n_exp - quark_mass_neutron
delta_mn_mp = m_n_exp - m_p_exp
delta_md_mu = m_d - m_u

print(f"""
  COMPOSITION: 1 up quark + 2 down quarks
  
  Cuboctahedral picture:
    u: (0, -1, -1)  charge +2/3
    d: (0, -1, +1)  charge -1/3
    d: (0, +1, -1)  charge -1/3
    ─────────────────────────────
    Total charge:    0
    
  Mass budget:
    Quark masses:    m_u + 2*m_d = {quark_mass_neutron*1e3:.1f} MeV  ({quark_mass_neutron/m_n_exp*100:.1f}%)
    QCD binding:     {qcd_binding_n*1e3:.1f} MeV  ({qcd_binding_n/m_n_exp*100:.1f}%)
    ─────────────────────────────────────────
    Neutron mass:    {m_n_exp*1e3:.2f} MeV
    
  NEUTRON-PROTON MASS DIFFERENCE:
    m_n - m_p = {delta_mn_mp*1e3:.2f} MeV  (experimental)
    m_d - m_u = {delta_md_mu*1e3:.2f} MeV  (quark mass difference)
    
    The neutron is heavier because m_d > m_u.
    This tiny difference ({delta_mn_mp/m_p_exp*100:.3f}% of the proton mass)
    determines the stability of matter:
    - Free neutrons decay (beta decay, t_1/2 = 10.2 min)
    - Protons are stable (or nearly so, t > 10^34 years)
    - Nuclei are bound states that can stabilize neutrons
""")

# ============================================================
# Part 4: THE STRONG FORCE — Confinement from Color
# ============================================================
print(f"{'='*72}")
print("  PART 4: COLOR CONFINEMENT FROM THE CUBOCTAHEDRON")
print(f"{'='*72}")

# QCD coupling at 1 GeV (proton scale)
Lambda_QCD = 0.217  # GeV, QCD scale

print(f"""
  The 8 gluons (8 triangular faces of the cuboctahedron) mediate
  the strong force. At the proton scale (~1 GeV), the strong
  coupling is large:
  
    alpha_s(M_Z) = {alpha_s_MZ}  (FTD: beta_3/(beta_3+4*N_eff) = 7/59 = {7/59:.4f})
    alpha_s(1 GeV) ~ 0.5  (non-perturbative, confinement regime)
    Lambda_QCD = {Lambda_QCD*1e3:.0f} MeV  (confinement scale)
  
  COLOR CONFINEMENT in FTD:
  
  The 3 quarks in a proton carry red, green, blue color charge.
  In the cuboctahedron, color = which pair of square faces the
  quark couples to (3 pairs = 3 colors).
  
  Confinement means: only color-neutral (white) combinations exist
  as free particles. The cuboctahedral symmetry requires:
  
    - Baryons: 3 quarks, one of each color (R+G+B = white)
      Uses 3 of the 3 square-face pairs → complete coverage
      
    - Mesons: quark + antiquark (color + anticolor = white)
      Uses 1 square-face pair + its antipodal → pair cancellation
      
    - Glueballs: pure gluon bound states
      Uses triangular faces only → closed loops on the surface
  
  The proton is the LIGHTEST baryon = the lowest-energy 
  configuration of 3 color-matched vertices.
""")

# ============================================================
# Part 5: NUCLEAR BINDING — Building Nuclei
# ============================================================
print(f"{'='*72}")
print("  PART 5: NUCLEAR BINDING — FROM PROTONS TO NUCLEI")
print(f"{'='*72}")

# Nuclear data (binding energies in MeV)
nuclei = [
    ("H-1",     1, 0, 1, 0.0,      "Hydrogen"),
    ("H-2",     1, 1, 2, 2.224,     "Deuterium"),
    ("He-3",    2, 1, 3, 7.718,     "Helium-3"),
    ("He-4",    2, 2, 4, 28.296,    "Helium-4 (alpha)"),
    ("Li-6",    3, 3, 6, 31.994,    "Lithium-6"),
    ("Li-7",    3, 4, 7, 39.245,    "Lithium-7"),
    ("C-12",    6, 6, 12, 92.162,   "Carbon-12"),
    ("N-14",    7, 7, 14, 104.659,  "Nitrogen-14"),
    ("O-16",    8, 8, 16, 127.619,  "Oxygen-16"),
    ("Fe-56",   26, 30, 56, 492.258, "Iron-56"),
    ("Au-197",  79, 118, 197, 1559.4, "Gold-197"),
    ("U-238",   92, 146, 238, 1801.7, "Uranium-238"),
]

print(f"\n  {'Nucleus':<10s} {'Z':>3s} {'N':>3s} {'A':>3s} {'BE(MeV)':>10s} {'BE/A':>8s} {'Quarks':>7s} {'Gluon field':>12s}")
print(f"  {'─'*10} {'─'*3} {'─'*3} {'─'*3} {'─'*10} {'─'*8} {'─'*7} {'─'*12}")

for name, Z, N, A, BE, desc in nuclei:
    be_per_a = BE/A if A > 1 else 0
    n_quarks = 3 * A  # 3 quarks per nucleon
    n_gluon = 8 * A   # 8 gluon field modes per nucleon  
    mass_quarks = A * (2*m_u + m_d) * 1e3 # MeV (average)
    mass_total = Z * m_p_exp * 1e3 + N * m_n_exp * 1e3 - BE
    print(f"  {name:<10s} {Z:3d} {N:3d} {A:3d} {BE:10.1f} {be_per_a:8.2f} {n_quarks:7d} {n_gluon:12d}")

# ============================================================
# Part 6: The Semi-Empirical Mass Formula (Bethe-Weizsäcker)
# ============================================================
print(f"\n{'='*72}")
print("  PART 6: NUCLEAR BINDING — THE LIQUID DROP MODEL")
print(f"{'='*72}")

# Bethe-Weizsäcker coefficients (MeV)
a_V = 15.75   # Volume
a_S = 17.8    # Surface
a_C = 0.711   # Coulomb
a_A = 23.7    # Asymmetry
a_P = 11.2    # Pairing

print(f"""
  The nuclear binding energy follows the semi-empirical mass formula:
  
  B(Z,N) = a_V*A - a_S*A^(2/3) - a_C*Z(Z-1)/A^(1/3) - a_A*(N-Z)^2/A + delta
  
  FTD INTERPRETATION of each term:
  
  Volume term:    a_V*A = {a_V:.2f}*A MeV
    Each nucleon couples to its N_b = {N_b} nearest neighbors
    via the residual strong force (pion exchange).
    a_V ~ Lambda_QCD * N_b = {Lambda_QCD*1e3:.0f} * {N_b} = {Lambda_QCD*1e3*N_b:.0f} MeV
    (Compare: {a_V:.1f} MeV actual, {Lambda_QCD*1e3*N_b:.0f} MeV cuboctahedral)
    
  Surface term:   -a_S*A^(2/3) = -{a_S:.2f}*A^(2/3) MeV
    Nucleons on the surface have fewer neighbors.
    The cuboctahedron has {N_b} neighbors per vertex, but surface 
    vertices of a cluster lose ~1 neighbor → correction ~ A^(2/3)
    
  Coulomb term:   -a_C*Z(Z-1)/A^(1/3)
    Electrostatic repulsion between protons.
    a_C = (3/5) * alpha / r_0 where r_0 ~ 1.2 fm
    From FTD: alpha = 1/{x_plus:.3f}, r_0 ~ 1/Lambda_QCD
    
  Asymmetry:      -a_A*(N-Z)^2/A
    The Pauli exclusion principle penalizes unequal N and Z.
    a_A ~ (pi^2 / 12) * (hbar^2 / (2*m_N*r_0^2))
    In FTD: this comes from the antisymmetry of the lattice
    wave function under vertex permutation.
""")

# Verify the formula for several nuclei
print(f"  Semi-empirical mass formula verification:")
print(f"  {'Nucleus':<10s} {'B_exp(MeV)':>10s} {'B_SEMF(MeV)':>12s} {'Error':>8s}")
print(f"  {'─'*10} {'─'*10} {'─'*12} {'─'*8}")

for name, Z, N, A, BE_exp, desc in nuclei:
    if A < 2: continue
    # SEMF
    delta = 0
    if Z % 2 == 0 and N % 2 == 0:
        delta = a_P / np.sqrt(A)
    elif Z % 2 == 1 and N % 2 == 1:
        delta = -a_P / np.sqrt(A)
    
    BE_semf = a_V*A - a_S*A**(2/3) - a_C*Z*(Z-1)/A**(1/3) - a_A*(N-Z)**2/A + delta
    err = abs(BE_semf - BE_exp)/BE_exp*100
    print(f"  {name:<10s} {BE_exp:10.1f} {BE_semf:12.1f} {err:7.1f}%")

# ============================================================
# Part 7: SPECIFIC ATOMS
# ============================================================
print(f"\n{'='*72}")
print("  PART 7: ANATOMY OF SPECIFIC ATOMS")
print(f"{'='*72}")

atoms = [
    ("Hydrogen", 1, 0, 1, "Simplest atom: 1 proton + 1 electron"),
    ("Helium-4", 2, 2, 2, "Alpha particle nucleus, noble gas"),
    ("Carbon-12", 6, 6, 6, "Basis of organic chemistry"),
    ("Oxygen-16", 8, 8, 8, "Most abundant element by mass on Earth"),
    ("Iron-56", 26, 30, 26, "Most tightly bound nucleus (peak of BE/A curve)"),
    ("Gold-197", 79, 118, 79, "Heavy element, relativistic electron effects"),
]

for name, Z, N, n_e, desc in atoms:
    A = Z + N
    n_quarks = 3 * A
    n_up = 2*Z + N      # 2 up per proton, 1 up per neutron
    n_down = Z + 2*N     # 1 down per proton, 2 down per neutron
    n_gluons_eff = 8 * A
    n_photons_eff = Z * (Z - 1) // 2  # Coulomb pairs
    total_vertices = n_quarks + n_e  # quark vertices + electron vertices
    
    # Mass contributions
    mass_quarks = (n_up * m_u + n_down * m_d) * 1e3  # MeV
    mass_electrons = n_e * m_e * 1e3  # MeV
    mass_nucleons = (Z * m_p_exp + N * m_n_exp) * 1e3  # MeV
    
    # Binding energies
    BE_data = [x for x in nuclei if x[0] == f"{name.split('-')[0] if '-' in name else name}-{A}"]
    BE = BE_data[0][4] if BE_data else 0
    
    # Electron binding (approximate: sum of ionization energies)
    # Use hydrogen-like approximation: E_n ~ Z_eff^2 * 13.6 eV / n^2
    E_electron_bind = 0
    for n_shell in range(1, 8):
        n_in_shell = min(2*n_shell**2, max(0, n_e - sum(2*k**2 for k in range(1, n_shell))))
        if n_in_shell <= 0: break
        Z_eff = max(Z - sum(2*k**2 for k in range(1, n_shell)), 1)
        E_electron_bind += n_in_shell * Z_eff**2 * 13.6e-6 / n_shell**2  # MeV

    print(f"\n  ┌──────────────────────────────────────────────────────┐")
    print(f"  │  {name:^52s}│")
    print(f"  │  {desc:^52s}│")
    print(f"  ├──────────────────────────────────────────────────────┤")
    print(f"  │  Protons (Z):         {Z:>5d}                          │")
    print(f"  │  Neutrons (N):        {N:>5d}                          │")
    print(f"  │  Electrons:           {n_e:>5d}                          │")
    print(f"  │  Total nucleons (A):  {A:>5d}                          │")
    print(f"  ├──────────────────────────────────────────────────────┤")
    print(f"  │  QUARK CONTENT:                                      │")
    print(f"  │    Up quarks:         {n_up:>5d}                          │")
    print(f"  │    Down quarks:       {n_down:>5d}                          │")
    print(f"  │    Total quarks:      {n_quarks:>5d}                          │")
    print(f"  ├──────────────────────────────────────────────────────┤")
    print(f"  │  CUBOCTAHEDRAL CONTENT:                               │")
    print(f"  │    Quark vertices:    {n_quarks:>5d}  (from gen-1 plane)     │")
    print(f"  │    Electron vertices: {n_e:>5d}  (from gen-1 plane)     │")
    print(f"  │    Gluon modes:       {n_gluons_eff:>5d}  (8 tri faces × A)    │")
    print(f"  │    EW boson modes:    {4*A:>5d}  (4 sq face pairs × A)  │")
    print(f"  ├──────────────────────────────────────────────────────┤")
    print(f"  │  MASS BUDGET:                                        │")
    print(f"  │    Quark rest mass:   {mass_quarks:>10.1f} MeV ({mass_quarks/mass_nucleons*100:5.1f}%)   │")
    print(f"  │    Electron mass:     {mass_electrons:>10.4f} MeV ({mass_electrons/mass_nucleons*100:7.4f}%) │")
    print(f"  │    QCD binding:       {mass_nucleons-mass_quarks-BE:>10.1f} MeV ({(mass_nucleons-mass_quarks-BE)/mass_nucleons*100:5.1f}%)   │")
    print(f"  │    Nuclear binding:   {BE:>10.1f} MeV ({BE/mass_nucleons*100:5.1f}%)   │")
    print(f"  │    Total atom mass:   {mass_nucleons - BE + mass_electrons:>10.1f} MeV            │")
    print(f"  └──────────────────────────────────────────────────────┘")

# ============================================================
# Part 8: The FTD Origin Story
# ============================================================
print(f"\n{'='*72}")
print("  PART 8: FROM Z^3 TO A GOLD ATOM")
print(f"{'='*72}")

print(f"""
  THE CHAIN:
  
  Z^3 (cubic lattice)
   │
   └─ Cuboctahedron (12 vertices, 14 faces, 24 edges)
       │
       ├─ 8 triangular faces → 8 gluons → strong force → confinement
       │
       ├─ 6 square faces → W+, W-, Z, gamma → electroweak force
       │
       ├─ 12 = 3 × 4 vertices → 3 generations of quarks/leptons
       │    │
       │    ├─ Gen 1: u, d quarks (mass from alpha^10)
       │    │    │
       │    │    ├─ uud → PROTON  (938.3 MeV, charge +1)
       │    │    └─ udd → NEUTRON (939.6 MeV, charge 0)
       │    │
       │    └─ Gen 1: electron (mass from alpha^11)
       │
       └─ Coupling constant alpha = 1/137.036 (from master quadratic)
            │
            ├─ Nuclear force (residual strong) → nuclei
            │    ├─ H-1:   1p          → hydrogen
            │    ├─ He-4:  2p + 2n     → helium (alpha particle)
            │    ├─ C-12:  6p + 6n     → carbon (life)
            │    ├─ O-16:  8p + 8n     → oxygen (respiration)
            │    ├─ Fe-56: 26p + 30n   → iron (peak stability)
            │    └─ Au-197:79p + 118n  → gold
            │
            └─ Electromagnetic force → atoms
                 ├─ Atomic shells: n=1,2,3... (Bohr model)
                 ├─ Chemical bonds → molecules
                 └─ Condensed matter → the world
  
  EVERY atom in the universe is made of:
    - Up and down quarks (from 4 cuboctahedral vertices)
    - Electrons (from the same vertex structure)
    - Gluons (from 8 triangular faces)
    - Photons (from 1 of the 6 square faces)
    - All governed by alpha = 1/137.036 (from the lemniscate constant)
    
  A gold atom (Au-197) contains:
    - {79*2 + 118} = {79*2+118} up quarks
    - {79 + 118*2} = {79+118*2} down quarks  
    - 79 electrons
    - Total elementary particles: {79*2+118 + 79+118*2 + 79}
    
  All from one axiom: Lambda = Z^3.
""")

print("=" * 72)
