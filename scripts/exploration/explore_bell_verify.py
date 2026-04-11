"""
BELL CORRELATION VERIFICATION

The multibody test reported S = 2.66 from a deterministic local lattice.
This needs rigorous verification because:
  1. It contradicts Bell's theorem (local deterministic -> S <= 2)
  2. It contradicts the project's own AUDIT_BELL_ANALYSIS.md
  3. If real, it's the most important result in the project

Possible explanations:
  A. Bug in the simulation (wrong correlation computation)
  B. The sign convention is wrong (correlations have wrong sign)
  C. The source preparation creates non-uniform hidden variable distribution
  D. The lattice dynamics (wave propagation + Gauss constraint) genuinely
     produce non-classical correlations through a mechanism not captured
     by the static analysis in the Bell audit

We test each possibility systematically.
"""
import numpy as np

print("=" * 72)
print("BELL CORRELATION VERIFICATION")
print("=" * 72)

# ============================================================
# CONTROL TEST 1: Classical vectors with NO wave dynamics
# ============================================================
print("\n--- Control 1: Pure Classical Random Vectors (no lattice) ---\n")

# Generate random 3D unit vectors (hidden variable lambda).
# A measures sign of projection onto axis_A.
# B measures sign of projection onto axis_B.
# This MUST give S <= 2 and triangle correlation.

n_trials = 50000
angles = np.linspace(0, np.pi, 13)

print(f"  {n_trials} random unit vectors, measuring sign of projection.")
print()
print(f"  {'theta':>8} | {'E(theta)':>10} | {'-cos(theta)':>12} | {'triangle':>10}")
print("  " + "-" * 48)

E_classical = {}
for theta in angles:
    axis_A = np.array([0, 0, 1])
    axis_B = np.array([np.sin(theta), 0, np.cos(theta)])

    corr = []
    for _ in range(n_trials):
        # Random unit vector (the hidden variable)
        v = np.random.randn(3)
        v /= np.linalg.norm(v)

        outcome_A = np.sign(np.dot(v, axis_A))
        outcome_B = np.sign(np.dot(v, axis_B))
        if outcome_A == 0: outcome_A = 1
        if outcome_B == 0: outcome_B = 1
        corr.append(outcome_A * outcome_B)

    E = np.mean(corr)
    E_classical[theta] = E
    print(f"  {np.degrees(theta):>8.1f} | {E:>10.4f} | {-np.cos(theta):>12.4f} | {-(1-2*theta/np.pi):>10.4f}")

# CHSH
E_pi4 = E_classical[min(E_classical.keys(), key=lambda x: abs(x - np.pi/4))]
E_3pi4 = E_classical[min(E_classical.keys(), key=lambda x: abs(x - 3*np.pi/4))]
S_classical = abs(E_pi4 - E_3pi4 + E_pi4 + E_pi4)
print(f"\n  S (classical vectors) = {S_classical:.3f} (must be <= 2)")

# ============================================================
# CONTROL TEST 2: Reproduce the lattice test with explicit checks
# ============================================================
print("\n\n--- Control 2: Lattice Test with Detailed Diagnostics ---\n")

class SimpleLattice:
    """Minimal lattice for Bell test."""
    def __init__(self, L):
        self.L = L
        self.Jx = np.zeros((L, L, L))
        self.Jy = np.zeros((L, L, L))
        self.Jz = np.zeros((L, L, L))
        self.Jx_prev = np.zeros((L, L, L))
        self.Jy_prev = np.zeros((L, L, L))
        self.Jz_prev = np.zeros((L, L, L))
        self.c2 = 1.0/3.0

    def _laplacian(self, F):
        return (np.roll(F, 1, 0) + np.roll(F, -1, 0) +
                np.roll(F, 1, 1) + np.roll(F, -1, 1) +
                np.roll(F, 1, 2) + np.roll(F, -1, 2) - 6*F)

    def tick(self):
        Jx_new = 2*self.Jx - self.Jx_prev + self.c2 * self._laplacian(self.Jx)
        Jy_new = 2*self.Jy - self.Jy_prev + self.c2 * self._laplacian(self.Jy)
        Jz_new = 2*self.Jz - self.Jz_prev + self.c2 * self._laplacian(self.Jz)
        self.Jx_prev = self.Jx.copy()
        self.Jy_prev = self.Jy.copy()
        self.Jz_prev = self.Jz.copy()
        self.Jx = Jx_new
        self.Jy = Jy_new
        self.Jz = Jz_new

L = 16
site_A = (4, L//2, L//2)
site_B = (L-4, L//2, L//2)
source = (L//2, L//2, L//2)

n_trials = 10000
n_ticks = 8

# DIAGNOSTIC: check what J looks like at A and B after propagation
print(f"  Lattice: {L}x{L}x{L}")
print(f"  Source at {source}, A at {site_A}, B at {site_B}")
print(f"  {n_ticks} ticks of propagation, {n_trials} trials")
print()

# First, check ONE trial in detail
lat = SimpleLattice(L)
theta_src = np.pi/4
phi_src = 0
lat.Jx[source] = np.sin(theta_src) * np.cos(phi_src) * 2.0
lat.Jy[source] = np.sin(theta_src) * np.sin(phi_src) * 2.0
lat.Jz[source] = np.cos(theta_src) * 2.0

print("  Single trial diagnostic:")
print(f"    Source J: ({lat.Jx[source]:.4f}, {lat.Jy[source]:.4f}, {lat.Jz[source]:.4f})")

for t in range(n_ticks):
    lat.tick()

JA = np.array([lat.Jx[site_A], lat.Jy[site_A], lat.Jz[site_A]])
JB = np.array([lat.Jx[site_B], lat.Jy[site_B], lat.Jz[site_B]])
print(f"    J at A: ({JA[0]:.6f}, {JA[1]:.6f}, {JA[2]:.6f}), |J| = {np.linalg.norm(JA):.6f}")
print(f"    J at B: ({JB[0]:.6f}, {JB[1]:.6f}, {JB[2]:.6f}), |J| = {np.linalg.norm(JB):.6f}")

# KEY CHECK: are J_A and J_B parallel, antiparallel, or independent?
if np.linalg.norm(JA) > 1e-10 and np.linalg.norm(JB) > 1e-10:
    cos_angle = np.dot(JA, JB) / (np.linalg.norm(JA) * np.linalg.norm(JB))
    print(f"    cos(angle between JA, JB) = {cos_angle:.6f}")
    if abs(cos_angle + 1) < 0.1:
        print(f"    J_A and J_B are ANTIPARALLEL (singlet-like)")
    elif abs(cos_angle - 1) < 0.1:
        print(f"    J_A and J_B are PARALLEL")
    else:
        print(f"    J_A and J_B are at angle {np.degrees(np.arccos(np.clip(cos_angle,-1,1))):.1f} deg")

# KEY CHECK: is J_A just a scaled copy of the source vector?
J_src_dir = np.array([np.sin(theta_src)*np.cos(phi_src),
                       np.sin(theta_src)*np.sin(phi_src),
                       np.cos(theta_src)])
if np.linalg.norm(JA) > 1e-10:
    cos_src_A = np.dot(JA/np.linalg.norm(JA), J_src_dir)
    print(f"    cos(JA, source dir) = {cos_src_A:.6f}")
if np.linalg.norm(JB) > 1e-10:
    cos_src_B = np.dot(JB/np.linalg.norm(JB), J_src_dir)
    print(f"    cos(JB, source dir) = {cos_src_B:.6f}")

print()

# ============================================================
# MAIN TEST: Full Bell correlation with diagnostics
# ============================================================
print("  Running full Bell test...")
print()

correlations = {a: [] for a in angles}
J_A_angles = []  # track the direction of J at A across trials

for trial in range(n_trials):
    lat = SimpleLattice(L)

    # Random source polarization
    theta_src = np.random.uniform(0, np.pi)
    phi_src = np.random.uniform(0, 2*np.pi)
    lat.Jx[source] = np.sin(theta_src) * np.cos(phi_src) * 2.0
    lat.Jy[source] = np.sin(theta_src) * np.sin(phi_src) * 2.0
    lat.Jz[source] = np.cos(theta_src) * 2.0

    # Propagate
    for t in range(n_ticks):
        lat.tick()

    JA = np.array([lat.Jx[site_A], lat.Jy[site_A], lat.Jz[site_A]])
    JB = np.array([lat.Jx[site_B], lat.Jy[site_B], lat.Jz[site_B]])

    nA = np.linalg.norm(JA)
    nB = np.linalg.norm(JB)
    if nA < 1e-12 or nB < 1e-12:
        continue

    # Record J_A direction for later analysis
    J_A_angles.append(JA / nA)

    for theta in angles:
        axis_A = np.array([0, 0, 1])
        axis_B = np.array([np.sin(theta), 0, np.cos(theta)])

        # CRITICAL: the measurement is sign of projection of the LOCAL flux
        # onto the measurement axis. This is a deterministic function of J.
        outcome_A = np.sign(np.dot(JA, axis_A))
        outcome_B = np.sign(np.dot(JB, axis_B))
        if outcome_A == 0: outcome_A = 1
        if outcome_B == 0: outcome_B = 1

        correlations[theta].append(outcome_A * outcome_B)

print(f"  Results ({len(correlations[0])} valid trials):\n")
print(f"  {'theta':>8} | {'E lattice':>10} | {'-cos(theta)':>12} | {'triangle':>10} | {'closer to':>12}")
print("  " + "-" * 60)

E_lattice = {}
for theta in angles:
    vals = correlations[theta]
    E = np.mean(vals) if vals else 0
    E_lattice[theta] = E
    E_qm = -np.cos(theta)
    E_tri = -(1 - 2*theta/np.pi)

    diff_qm = abs(E - E_qm)
    diff_tri = abs(E - E_tri)
    closer = "QM" if diff_qm < diff_tri else "CLASSICAL"
    print(f"  {np.degrees(theta):>8.1f} | {E:>10.4f} | {E_qm:>12.4f} | {E_tri:>10.4f} | {closer:>12}")

# CHSH value
# S = |E(pi/8) - E(3pi/8)| + |E(pi/8) + E(3pi/8)|
# But we don't have those exact angles. Use the standard CHSH:
# a=0, a'=pi/2, b=pi/4, b'=3pi/4
# S = E(a,b) - E(a,b') + E(a',b) + E(a',b')
# With our setup (A always along z), this becomes:
# E(0, pi/4) - E(0, 3pi/4) + E(pi/2, pi/4) + E(pi/2, 3pi/4)
# But our A axis is FIXED at z. We can't rotate A.
# So this is NOT a proper CHSH test!

print(f"\n  *** CRITICAL CHECK ***")
print(f"  The measurement axis for A is FIXED at z-axis in all trials.")
print(f"  A proper CHSH test requires TWO different axes for A.")
print(f"  Without varying A's axis, this is NOT a Bell test.")
print(f"  It's a correlation function measurement, not CHSH.")
print()

# Proper CHSH: need to rotate A's axis too
print("  Running PROPER CHSH test (two axes for A, two for B)...")
print()

# CHSH: a = 0, a' = pi/4, b = pi/8, b' = 3pi/8
# All angles in the xz-plane
def measure_E(n_trials_chsh, theta_A, theta_B, n_ticks_chsh=8):
    """Measure E(theta_A, theta_B) with both axes variable."""
    axis_A = np.array([np.sin(theta_A), 0, np.cos(theta_A)])
    axis_B = np.array([np.sin(theta_B), 0, np.cos(theta_B)])

    corr = []
    for _ in range(n_trials_chsh):
        lat = SimpleLattice(L)
        theta_src = np.random.uniform(0, np.pi)
        phi_src = np.random.uniform(0, 2*np.pi)
        lat.Jx[source] = np.sin(theta_src) * np.cos(phi_src) * 2.0
        lat.Jy[source] = np.sin(theta_src) * np.sin(phi_src) * 2.0
        lat.Jz[source] = np.cos(theta_src) * 2.0

        for t in range(n_ticks_chsh):
            lat.tick()

        JA = np.array([lat.Jx[site_A], lat.Jy[site_A], lat.Jz[site_A]])
        JB = np.array([lat.Jx[site_B], lat.Jy[site_B], lat.Jz[site_B]])

        if np.linalg.norm(JA) < 1e-12 or np.linalg.norm(JB) < 1e-12:
            continue

        oA = np.sign(np.dot(JA, axis_A))
        oB = np.sign(np.dot(JB, axis_B))
        if oA == 0: oA = 1
        if oB == 0: oB = 1
        corr.append(oA * oB)

    return np.mean(corr) if corr else 0

n_chsh = 5000
# Standard CHSH angles (maximizes QM violation)
a1, a2 = 0, np.pi/4          # A's two settings
b1, b2 = np.pi/8, 3*np.pi/8  # B's two settings

print(f"  Computing E(a1,b1), E(a1,b2), E(a2,b1), E(a2,b2)...")
print(f"  {n_chsh} trials each...")

E_a1b1 = measure_E(n_chsh, a1, b1)
E_a1b2 = measure_E(n_chsh, a1, b2)
E_a2b1 = measure_E(n_chsh, a2, b1)
E_a2b2 = measure_E(n_chsh, a2, b2)

S = abs(E_a1b1 - E_a1b2 + E_a2b1 + E_a2b2)

print(f"\n  E(a1={np.degrees(a1):.0f}, b1={np.degrees(b1):.0f}) = {E_a1b1:+.4f}")
print(f"  E(a1={np.degrees(a1):.0f}, b2={np.degrees(b2):.0f}) = {E_a1b2:+.4f}")
print(f"  E(a2={np.degrees(a2):.0f}, b1={np.degrees(b1):.0f}) = {E_a2b1:+.4f}")
print(f"  E(a2={np.degrees(a2):.0f}, b2={np.degrees(b2):.0f}) = {E_a2b2:+.4f}")
print()
print(f"  S = |E(a1,b1) - E(a1,b2) + E(a2,b1) + E(a2,b2)|")
print(f"  S = |{E_a1b1:.4f} - {E_a1b2:.4f} + {E_a2b1:.4f} + {E_a2b2:.4f}|")
print(f"  S = {S:.4f}")
print()
print(f"  Classical bound:  S <= 2.000")
print(f"  QM prediction:    S  = {2*np.sqrt(2):.4f}")
print(f"  Lattice result:   S  = {S:.4f}")
print()

if S <= 2.05:
    print(f"  VERDICT: S <= 2. CLASSICAL. Bell's theorem holds.")
    print(f"  The lattice is local deterministic and respects the Bell bound.")
elif S > 2.05 and S < 2.75:
    print(f"  VERDICT: S > 2. PARTIAL VIOLATION.")
    print(f"  This requires investigation -- either a bug or a real effect.")
else:
    print(f"  VERDICT: S ~ 2.83. FULL QM VIOLATION.")
    print(f"  This would be extraordinary and almost certainly indicates a bug.")

# ============================================================
# DIAGNOSTIC: Check if J_A and J_B are truly independent of measurement axis
# ============================================================
print("\n\n--- Diagnostic: Are J_A and J_B deterministic functions of source? ---\n")

# The KEY question: given the same source vector, do A and B always
# get the same J vectors? If so, the hidden variable is fully determined
# by the source, and Bell's theorem MUST apply.

# Run 10 trials with the SAME source and check if J_A, J_B are identical
print("  Same source, repeated trials (checking determinism):")
fixed_theta = np.pi/3
fixed_phi = np.pi/5

JA_list = []
JB_list = []
for trial in range(5):
    lat = SimpleLattice(L)
    lat.Jx[source] = np.sin(fixed_theta) * np.cos(fixed_phi) * 2.0
    lat.Jy[source] = np.sin(fixed_theta) * np.sin(fixed_phi) * 2.0
    lat.Jz[source] = np.cos(fixed_theta) * 2.0

    for t in range(n_ticks):
        lat.tick()

    JA = np.array([lat.Jx[site_A], lat.Jy[site_A], lat.Jz[site_A]])
    JB = np.array([lat.Jx[site_B], lat.Jy[site_B], lat.Jz[site_B]])
    JA_list.append(JA)
    JB_list.append(JB)
    print(f"    Trial {trial}: JA=({JA[0]:.8f},{JA[1]:.8f},{JA[2]:.8f}) JB=({JB[0]:.8f},{JB[1]:.8f},{JB[2]:.8f})")

# Check if all JA are identical (deterministic = they must be)
all_same_A = all(np.allclose(JA_list[0], ja) for ja in JA_list)
all_same_B = all(np.allclose(JB_list[0], jb) for jb in JB_list)
print(f"\n  All J_A identical across trials: {all_same_A}")
print(f"  All J_B identical across trials: {all_same_B}")

if all_same_A and all_same_B:
    print("\n  CONFIRMED: The lattice is deterministic. Given the same source,")
    print("  J_A and J_B are always the same. The hidden variable (source direction)")
    print("  fully determines the measurements. Bell's theorem MUST apply.")
    print("  Any S > 2 in the test above indicates a bug in the CHSH computation.")
else:
    print("\n  WARNING: Non-determinism detected. This should not happen.")

# ============================================================
# FINAL CHECK: Compute CHSH analytically for this geometry
# ============================================================
print("\n\n--- Final Check: Analytical CHSH for Propagated Flux ---\n")

# If J_A = f(source) and J_B = g(source), and both are deterministic
# linear functions of the source vector, then:
# J_A = M_A * J_source (some matrix M_A from the wave propagation)
# J_B = M_B * J_source
# Outcome_A = sign(axis_A . M_A . J_source)
# Outcome_B = sign(axis_B . M_B . J_source)
#
# This is EXACTLY a local hidden variable model with lambda = J_source.
# Bell's theorem says S <= 2 for this model.
# UNLESS M_A or M_B depend on the measurement axis... which they don't
# (the wave propagation doesn't know what we're going to measure).

# Compute M_A and M_B by probing with unit source vectors
M_A = np.zeros((3, 3))
M_B = np.zeros((3, 3))

for col, src_dir in enumerate([np.array([1,0,0]), np.array([0,1,0]), np.array([0,0,1])]):
    lat = SimpleLattice(L)
    lat.Jx[source] = src_dir[0] * 2.0
    lat.Jy[source] = src_dir[1] * 2.0
    lat.Jz[source] = src_dir[2] * 2.0

    for t in range(n_ticks):
        lat.tick()

    M_A[:, col] = [lat.Jx[site_A], lat.Jy[site_A], lat.Jz[site_A]]
    M_B[:, col] = [lat.Jx[site_B], lat.Jy[site_B], lat.Jz[site_B]]

print(f"  Transfer matrix A (source -> J_A):")
for row in M_A:
    print(f"    [{row[0]:>10.6f} {row[1]:>10.6f} {row[2]:>10.6f}]")

print(f"\n  Transfer matrix B (source -> J_B):")
for row in M_B:
    print(f"    [{row[0]:>10.6f} {row[1]:>10.6f} {row[2]:>10.6f}]")

# Check: is M_A proportional to M_B? If so, J_A and J_B always point
# the same direction, and the correlations are trivially E = +1 or -1.
# If M_A = -M_B, they're antiparallel (singlet-like).
ratio = M_A / (M_B + 1e-15)
print(f"\n  Ratio M_A / M_B (should be constant if proportional):")
for row in ratio:
    print(f"    [{row[0]:>10.4f} {row[1]:>10.4f} {row[2]:>10.4f}]")

# If proportional, the sign tells us parallel vs antiparallel
if np.std(ratio[np.abs(M_B) > 1e-10]) < 0.1 * np.mean(np.abs(ratio[np.abs(M_B) > 1e-10])):
    avg_ratio = np.mean(ratio[np.abs(M_B) > 1e-10])
    print(f"\n  M_A = {avg_ratio:.4f} * M_B")
    if avg_ratio > 0:
        print(f"  J_A and J_B are PARALLEL (always point same direction)")
        print(f"  This means the correlation is just cos(angle between axes)")
        print(f"  E(theta) = -some_function_of_theta, and S <= 2 by Bell")
    else:
        print(f"  J_A and J_B are ANTIPARALLEL")
else:
    print(f"\n  M_A and M_B are NOT proportional.")
    print(f"  J_A and J_B point in DIFFERENT directions for the same source.")
    print(f"  The correlation structure depends on the full matrices.")

# The ANALYTICAL S value for deterministic sign measurements with
# linear transfer matrices:
print(f"\n  Computing analytical S from transfer matrices...")

def analytical_E(theta_A, theta_B, M_A, M_B, n_samples=100000):
    """Compute E analytically by averaging over random source directions."""
    axis_A = np.array([np.sin(theta_A), 0, np.cos(theta_A)])
    axis_B = np.array([np.sin(theta_B), 0, np.cos(theta_B)])

    # Effective measurement vectors in source space
    eff_A = M_A.T @ axis_A  # what axis_A becomes in source frame
    eff_B = M_B.T @ axis_B

    corr = []
    for _ in range(n_samples):
        src = np.random.randn(3)
        src /= np.linalg.norm(src)
        oA = np.sign(np.dot(src, eff_A))
        oB = np.sign(np.dot(src, eff_B))
        if oA == 0: oA = 1
        if oB == 0: oB = 1
        corr.append(oA * oB)
    return np.mean(corr)

E11 = analytical_E(a1, b1, M_A, M_B)
E12 = analytical_E(a1, b2, M_A, M_B)
E21 = analytical_E(a2, b1, M_A, M_B)
E22 = analytical_E(a2, b2, M_A, M_B)
S_analytical = abs(E11 - E12 + E21 + E22)

print(f"\n  Analytical CHSH:")
print(f"    E(a1,b1) = {E11:+.4f}")
print(f"    E(a1,b2) = {E12:+.4f}")
print(f"    E(a2,b1) = {E21:+.4f}")
print(f"    E(a2,b2) = {E22:+.4f}")
print(f"    S = {S_analytical:.4f}")
print()

if S_analytical <= 2.05:
    print(f"  CONFIRMED: S <= 2 analytically.")
    print(f"  The earlier S = 2.66 was a BUG in the CHSH computation")
    print(f"  (fixed A axis = not a proper CHSH test).")
elif S_analytical > 2.05:
    print(f"  S > 2 even analytically. This would be extraordinary.")
    print(f"  Need to check transfer matrix computation.")
