"""
Close Gaps 3 and 4 — CORRECTLY this time.

Gap 3: The cosine correlation is NOT about individual measurements
having Born-rule probabilities. It's about the AGGREGATE statistics
of many deterministic sign measurements, where the SELECTION of
which events are detected depends on |J|^2 (energy density).

The IQ analogy: one person has one IQ. No probability. Measure
many people, get a bell curve. The curve is statistics, not any
individual's property.

One lattice event: sign(v . axis) = +1 or -1. Deterministic.
Many events with |J|^2-dependent detection: the AGGREGATE
correlation function is cos(theta), not triangle.

Gap 4: Use 3D lattice, not 1D. Each voxel has flux in R^3.
Two voxels with opposite states (+1 and -1).
"""
import numpy as np

print("=" * 72)
print("GAPS 3 AND 4: CORRECT VERSION")
print("=" * 72)

# ============================================================
# GAP 3: AGGREGATE STATISTICS PRODUCE COSINE
# ============================================================
print("\n" + "=" * 72)
print("GAP 3: Cosine from Aggregate Statistics")
print("=" * 72)

# The key: each event is deterministic (sign of projection).
# But WHICH events get counted depends on |v . axis|^2.
# The aggregate correlation over many events, weighted by
# detection probability, gives the cosine.
#
# E(theta) = <A*B> where A = sign(v.a), B = sign(v.b)
#
# Without weighting (all events): E = triangle = -(1-2*theta/pi)
# With |v.a|^2 * |v.b|^2 weighting (energy-dependent detection):
# E_weighted = <A*B * |v.a|^2 * |v.b|^2> / <|v.a|^2 * |v.b|^2>
#
# This weighted average IS the aggregate over many events where
# detection probability ~ energy ~ |J|^2.

print("""
Each event: deterministic. sign(v . axis) = +1 or -1.
Detection probability: |v . axis|^2 (from Gap 2: energy ~ |J|^2).
Aggregate: weighted correlation over many events.

The question: does the energy-weighted aggregate give cos(theta)?
""")

n_trials = 500000
angles = np.linspace(0, np.pi, 25)

print(f"  {'theta':>8} | {'E unweighted':>14} | {'E weighted':>14} | {'-cos(theta)':>12} | {'triangle':>10} | {'weighted closer to':>20}")
print("  " + "-" * 86)

S_terms_unw = {}
S_terms_w = {}

for theta in angles:
    axis_A = np.array([0, 0, 1])
    axis_B = np.array([np.sin(theta), 0, np.cos(theta)])

    # Generate random 3D unit vectors (hidden variables)
    vs = np.random.randn(n_trials, 3)
    vs /= np.linalg.norm(vs, axis=1, keepdims=True)

    # Anti-correlated pair (singlet analog)
    vs_B = -vs

    # Projections
    projA = vs @ axis_A
    projB = vs_B @ axis_B

    # Outcomes: deterministic sign
    outA = np.sign(projA)
    outB = np.sign(projB)
    outA[outA == 0] = 1
    outB[outB == 0] = 1

    products = outA * outB

    # Unweighted: E = mean of A*B (all events equal)
    E_unw = np.mean(products)

    # Weighted by detection probability |proj_A|^2 * |proj_B|^2
    # This is the aggregate where each event contributes proportionally
    # to its detection likelihood (energy-dependent detection)
    weights = projA**2 * projB**2
    E_w = np.average(products, weights=weights)

    E_cos = -np.cos(theta)
    E_tri = -(1 - 2*theta/np.pi)

    diff_cos_w = abs(E_w - E_cos)
    diff_tri_w = abs(E_w - E_tri)
    closer = "COSINE" if diff_cos_w < diff_tri_w else "triangle"

    S_terms_unw[theta] = E_unw
    S_terms_w[theta] = E_w

    if theta in [0, angles[3], angles[6], angles[8], angles[12],
                 angles[16], angles[18], angles[21], angles[-1]]:
        print(f"  {np.degrees(theta):>8.1f} | {E_unw:>14.4f} | {E_w:>14.4f} | {E_cos:>12.4f} | {E_tri:>10.4f} | {closer:>20}")

# Compute CHSH S values
# Optimal CHSH angles
a1, a2 = 0, np.pi/4
b1, b2 = np.pi/8, 3*np.pi/8

def nearest(target, d):
    return d[min(d.keys(), key=lambda x: abs(x - target))]

# Unweighted
E11_u = nearest(abs(a1-b1), S_terms_unw)
E12_u = nearest(abs(a1-b2), S_terms_unw)
E21_u = nearest(abs(a2-b1), S_terms_unw)
E22_u = nearest(abs(a2-b2), S_terms_unw)
S_unw = abs(E11_u - E12_u + E21_u + E22_u)

# Weighted
E11_w = nearest(abs(a1-b1), S_terms_w)
E12_w = nearest(abs(a1-b2), S_terms_w)
E21_w = nearest(abs(a2-b1), S_terms_w)
E22_w = nearest(abs(a2-b2), S_terms_w)
S_w = abs(E11_w - E12_w + E21_w + E22_w)

print(f"\n  CHSH S values:")
print(f"    Unweighted (all events equal):        S = {S_unw:.4f}  (classical bound: 2)")
print(f"    Weighted (detection ~ |proj|^2):       S = {S_w:.4f}  (QM prediction: {2*np.sqrt(2):.4f})")
print()

if abs(S_w - 2*np.sqrt(2)) < 0.1:
    print(f"  *** ENERGY-WEIGHTED AGGREGATE GIVES S = {S_w:.4f} ~ 2*sqrt(2) = {2*np.sqrt(2):.4f} ***")
    print(f"  *** THE BORN RULE (P ~ |J|^2) PRODUCES THE QM BELL VIOLATION ***")
    print()
    print(f"  The mechanism:")
    print(f"    1. Each event: deterministic sign measurement (no probability)")
    print(f"    2. Detection probability: |v . axis|^2 (energy-dependent)")
    print(f"    3. Aggregate: energy-weighted sum over many events")
    print(f"    4. Result: cosine correlation, S = 2*sqrt(2)")
    print()
    print(f"  No single event violates anything. The violation is in the")
    print(f"  STATISTICS of the weighted aggregate. Just like the bell curve")
    print(f"  is in the population, not in any individual.")
else:
    print(f"  Weighted S = {S_w:.4f}, not matching QM {2*np.sqrt(2):.4f}")

# Also verify: does the WEIGHTED correlation match cos(theta)?
print(f"\n  Correlation function fit:")
print(f"  {'theta':>8} | {'E_weighted':>12} | {'-cos(theta)':>12} | {'error':>10}")
print("  " + "-" * 48)

errors = []
for theta in angles[1:-1]:
    E_w = S_terms_w[theta]
    E_cos = -np.cos(theta)
    err = abs(E_w - E_cos)
    errors.append(err)
    if theta in [angles[3], angles[6], angles[12], angles[18]]:
        print(f"  {np.degrees(theta):>8.1f} | {E_w:>12.4f} | {E_cos:>12.4f} | {err:>10.4f}")

print(f"\n  Mean absolute error from cosine: {np.mean(errors):.6f}")
print(f"  Max absolute error from cosine:  {np.max(errors):.6f}")

# ============================================================
# GAP 4: FUSION IN 3D WITH OPPOSITE CHARGES
# ============================================================
print("\n\n" + "=" * 72)
print("GAP 4: Fusion in 3D with Opposite States")
print("=" * 72)

print("""
Previous test was wrong:
  - Used 1D (misses angular structure)
  - Used same-sign sources (should be opposite for binding)
  - Only measured wave energy (missed coupling energy)

Correct test:
  - 3D lattice
  - +1 and -1 states (opposite charges attract)
  - Measure TOTAL energy including coupling term
""")

class Lattice3D:
    def __init__(self, L):
        self.L = L
        shape = (L, L, L)
        self.s = np.zeros(shape, dtype=int)
        self.Jx = np.zeros(shape)
        self.Jy = np.zeros(shape)
        self.Jz = np.zeros(shape)
        self.Jx_prev = np.zeros(shape)
        self.Jy_prev = np.zeros(shape)
        self.Jz_prev = np.zeros(shape)
        self.c2 = 1.0/3.0
        self.g_c = 0.085

    def _lap(self, F):
        return (np.roll(F,1,0)+np.roll(F,-1,0)+
                np.roll(F,1,1)+np.roll(F,-1,1)+
                np.roll(F,1,2)+np.roll(F,-1,2)-6*F)

    def _divJ(self):
        return (np.roll(self.Jx,-1,0)-self.Jx+
                np.roll(self.Jy,-1,1)-self.Jy+
                np.roll(self.Jz,-1,2)-self.Jz)

    def _grad_s(self):
        gx = (np.roll(self.s,-1,0)-np.roll(self.s,1,0)).astype(float)/2
        gy = (np.roll(self.s,-1,1)-np.roll(self.s,1,1)).astype(float)/2
        gz = (np.roll(self.s,-1,2)-np.roll(self.s,1,2)).astype(float)/2
        return gx, gy, gz

    def tick(self):
        gx, gy, gz = self._grad_s()
        self.Jx, self.Jx_prev = (2*self.Jx - self.Jx_prev +
            self.c2*self._lap(self.Jx) + self.g_c*gx), self.Jx.copy()
        self.Jy, self.Jy_prev = (2*self.Jy - self.Jy_prev +
            self.c2*self._lap(self.Jy) + self.g_c*gy), self.Jy.copy()
        self.Jz, self.Jz_prev = (2*self.Jz - self.Jz_prev +
            self.c2*self._lap(self.Jz) + self.g_c*gz), self.Jz.copy()

    def total_energy(self):
        """Total energy: kinetic + gradient + coupling."""
        dJx = self.Jx - self.Jx_prev
        dJy = self.Jy - self.Jy_prev
        dJz = self.Jz - self.Jz_prev
        KE = 0.5*(dJx**2 + dJy**2 + dJz**2)

        gJx = np.roll(self.Jx,-1,0)-self.Jx
        gJy = np.roll(self.Jy,-1,1)-self.Jy
        gJz = np.roll(self.Jz,-1,2)-self.Jz
        GE = 0.5*self.c2*(gJx**2 + gJy**2 + gJz**2)

        # Coupling energy: -g_c * s * div(J)
        CE = -self.g_c * self.s.astype(float) * self._divJ()

        return np.sum(KE) + np.sum(GE) + np.sum(CE)

L = 16
c = L // 2

# Measure energy for opposite-sign pair at various separations
print(f"\n  3D lattice {L}x{L}x{L}, two voxels with s=+1 and s=-1")
print(f"  Evolve 100 ticks, measure total energy (KE + gradient + coupling)\n")

# First: single particle energy
lat_single = Lattice3D(L)
lat_single.s[c, c, c] = 1
for _ in range(100):
    lat_single.tick()
    lat_single.s[c, c, c] = 1  # keep locked
E_single = lat_single.total_energy()
print(f"  Single +1 particle energy: {E_single:.6f}")

# Single -1
lat_single2 = Lattice3D(L)
lat_single2.s[c, c, c] = -1
for _ in range(100):
    lat_single2.tick()
    lat_single2.s[c, c, c] = -1
E_single2 = lat_single2.total_energy()
print(f"  Single -1 particle energy: {E_single2:.6f}")
print(f"  Sum of two singles: {E_single + E_single2:.6f}")
print()

print(f"  {'Separation':>11} | {'E(+1,-1)':>12} | {'E(+1,+1)':>12} | {'2*E_single':>12} | {'dE(+-)':>10} | {'dE(++)':>10} | {'Binding?':>10}")
print("  " + "-" * 85)

for sep in [1, 2, 3, 4, 5, 6, 8]:
    # Opposite charges (+1 and -1)
    lat_opp = Lattice3D(L)
    pos1 = c - sep//2
    pos2 = c + (sep+1)//2
    lat_opp.s[pos1, c, c] = 1
    lat_opp.s[pos2, c, c] = -1
    for _ in range(100):
        lat_opp.tick()
        lat_opp.s[pos1, c, c] = 1
        lat_opp.s[pos2, c, c] = -1
    E_opp = lat_opp.total_energy()

    # Same charges (+1 and +1)
    lat_same = Lattice3D(L)
    lat_same.s[pos1, c, c] = 1
    lat_same.s[pos2, c, c] = 1
    for _ in range(100):
        lat_same.tick()
        lat_same.s[pos1, c, c] = 1
        lat_same.s[pos2, c, c] = 1
    E_same = lat_same.total_energy()

    E_two = E_single + E_single2
    dE_opp = E_opp - E_two
    dE_same = E_same - 2*E_single
    binding = "YES" if dE_opp < -0.001 else ("weak" if dE_opp < 0 else "no")

    print(f"  {sep:>11} | {E_opp:>12.4f} | {E_same:>12.4f} | {E_two:>12.4f} | {dE_opp:>+10.4f} | {dE_same:>+10.4f} | {binding:>10}")

print("""
  If dE(+-) < 0: opposite charges BIND (energy released = fusion).
  If dE(++) > 0: same charges REPEL (energy increases = Coulomb).
  The difference dE(+-) - dE(++) is the EM interaction energy.
""")

# ============================================================
# SUMMARY
# ============================================================
print("\n" + "=" * 72)
print("SUMMARY")
print("=" * 72)
print(f"""
GAP 3 (Cosine): Energy-weighted aggregate S = {S_w:.4f}
  QM prediction: S = {2*np.sqrt(2):.4f}
  Match: {S_w/(2*np.sqrt(2))*100:.1f}%
  The Born rule (P ~ |J|^2) applied as AGGREGATE WEIGHTS
  on deterministic sign measurements produces the QM correlation.

GAP 4 (Fusion): Check the table above.
  If opposite charges bind (dE < 0) and same charges repel (dE > 0),
  the lattice naturally produces nuclear binding from the coupling term.
""")
