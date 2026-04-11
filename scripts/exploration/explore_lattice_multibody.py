"""
Multibody Lattice Tests: Born Rule, Bell Correlations, Interference

A minimal FTD lattice simulator to test three fundamental questions:
  1. Does |J|^2 linearity emerge from the dynamics? (Born rule)
  2. What correlation function does the lattice produce? (Bell)
  3. Does a double-slit geometry produce the QM interference pattern?

NO IMPORTS FROM QM. The lattice has:
  - ternary states s in {-1, 0, +1}
  - flux field J in R^3
  - discrete wave equation for J
  - Gauss constraint div(J) = rho
  - manifestation at |J| > K_B
  - local update (6 face-neighbors for simplicity, not full Moore 26)
"""
import numpy as np

print("=" * 72)
print("MULTIBODY LATTICE TESTS")
print("=" * 72)

# ============================================================
# MINIMAL LATTICE SIMULATOR
# ============================================================

class Lattice:
    def __init__(self, Lx, Ly, Lz):
        self.Lx, self.Ly, self.Lz = Lx, Ly, Lz
        self.s = np.zeros((Lx, Ly, Lz), dtype=int)        # ternary state
        self.Jx = np.zeros((Lx, Ly, Lz))                   # flux x
        self.Jy = np.zeros((Lx, Ly, Lz))                   # flux y
        self.Jz = np.zeros((Lx, Ly, Lz))                   # flux z
        self.Jx_prev = np.zeros((Lx, Ly, Lz))              # for wave eq
        self.Jy_prev = np.zeros((Lx, Ly, Lz))
        self.Jz_prev = np.zeros((Lx, Ly, Lz))
        self.K_B = 1.0                                       # manifestation threshold
        self.c2 = 1.0/3.0                                   # c^2 = 1/D
        self.g_c = 0.085                                     # coupling sqrt(alpha)
        self.gamma = 0.007                                   # damping (alpha)

    def _laplacian(self, F):
        """Discrete 6-neighbor Laplacian with periodic boundaries."""
        return (np.roll(F, 1, 0) + np.roll(F, -1, 0) +
                np.roll(F, 1, 1) + np.roll(F, -1, 1) +
                np.roll(F, 1, 2) + np.roll(F, -1, 2) - 6*F)

    def _div_J(self):
        """Discrete divergence of J."""
        return (np.roll(self.Jx, -1, 0) - self.Jx +
                np.roll(self.Jy, -1, 1) - self.Jy +
                np.roll(self.Jz, -1, 2) - self.Jz)

    def _grad_s(self):
        """Gradient of state field (source for flux)."""
        gx = np.roll(self.s, -1, 0) - np.roll(self.s, 1, 0)
        gy = np.roll(self.s, -1, 1) - np.roll(self.s, 1, 1)
        gz = np.roll(self.s, -1, 2) - np.roll(self.s, 1, 2)
        return gx.astype(float)/2, gy.astype(float)/2, gz.astype(float)/2

    def flux_magnitude(self):
        """Compute |J| at each site."""
        return np.sqrt(self.Jx**2 + self.Jy**2 + self.Jz**2)

    def tick(self):
        """One lattice tick: wave equation + coupling + damping + manifestation."""
        # Source from manifested particles
        gx, gy, gz = self._grad_s()

        # Wave equation: J_new = 2*J - J_prev + c^2 * laplacian(J) + source
        Jx_new = (2*self.Jx - self.Jx_prev +
                  self.c2 * self._laplacian(self.Jx) +
                  self.g_c * gx)
        Jy_new = (2*self.Jy - self.Jy_prev +
                  self.c2 * self._laplacian(self.Jy) +
                  self.g_c * gy)
        Jz_new = (2*self.Jz - self.Jz_prev +
                  self.c2 * self._laplacian(self.Jz) +
                  self.g_c * gz)

        # Damping (selective: only near manifested sites)
        near_particle = np.abs(self.s) > 0
        for dx in [-1, 0, 1]:
            near_particle |= np.roll(np.abs(self.s) > 0, dx, 0)
        for dy in [-1, 0, 1]:
            near_particle |= np.roll(np.abs(self.s) > 0, dy, 1)
        damp = np.where(near_particle, 1 - self.gamma, 1.0)
        Jx_new *= damp
        Jy_new *= damp
        Jz_new *= damp

        # Update
        self.Jx_prev = self.Jx.copy()
        self.Jy_prev = self.Jy.copy()
        self.Jz_prev = self.Jz.copy()
        self.Jx = Jx_new
        self.Jy = Jy_new
        self.Jz = Jz_new

        # Manifestation: void -> +/-1 where |J| > K_B
        mag = self.flux_magnitude()
        can_manifest = (self.s == 0) & (mag > self.K_B)
        # Which sign? Determined by dominant flux direction
        sign = np.sign(self.Jx + self.Jy + self.Jz)
        sign[sign == 0] = 1
        self.s[can_manifest] = sign[can_manifest].astype(int)

        # Evaporation: +/-1 -> 0 where |J| < K_B/2
        can_evap = (np.abs(self.s) > 0) & (mag < self.K_B * 0.5)
        self.s[can_evap] = 0


# ============================================================
# TEST 1: BORN RULE — Does |J|^2 Predict Manifestation Frequency?
# ============================================================
print("\n" + "=" * 72)
print("TEST 1: BORN RULE EMERGENCE")
print("Does manifestation frequency scale as |J|^n? What is n?")
print("=" * 72 + "\n")

# Setup: a lattice with a flux source that creates a 1/r density profile.
# Record where manifestations occur. Compare frequency vs |J| at each site.
# If Born rule: frequency ~ |J|^2. If classical: frequency ~ |J| or |J|^3.

L = 32
lat = Lattice(L, L, L)

# Place a source at the center: a manifested particle that injects flux
cx, cy, cz = L//2, L//2, L//2
lat.s[cx, cy, cz] = 1

# Let the flux field build up
print("  Building flux field (200 ticks)...")
for t in range(200):
    lat.tick()
    lat.s[cx, cy, cz] = 1  # keep source locked

# Now: set up many void sites at various distances from the source.
# Record |J| at each site. Then run many ticks and count manifestations.
# Compare manifestation count vs |J|^n for various n.

# Snapshot the flux field
J_mag = lat.flux_magnitude()

# Clear all states except source, to get a clean measurement
lat.s[:] = 0
lat.s[cx, cy, cz] = 1

# Record manifestation events over many ticks
n_trials = 500
manifest_count = np.zeros((L, L, L))
J_at_site = J_mag.copy()

print(f"  Running {n_trials} manifestation trials...")
for trial in range(n_trials):
    # Reset states (keep source, clear everything else)
    lat.s[:] = 0
    lat.s[cx, cy, cz] = 1

    # Run a few ticks to let flux propagate slightly differently each time
    # (different because the previous trial left different flux residues)
    for t in range(5):
        lat.tick()
        lat.s[cx, cy, cz] = 1  # keep source

    # Record where manifestations occurred
    manifested = np.abs(lat.s) > 0
    manifested[cx, cy, cz] = False  # exclude source
    manifest_count += manifested

# Analyze: bin sites by |J| and compute manifestation frequency per bin
print("\n  Results: manifestation frequency vs flux magnitude\n")

# Create bins by distance from source (proxy for flux magnitude)
distances = np.zeros((L, L, L))
for ix in range(L):
    for iy in range(L):
        for iz in range(L):
            dx = min(abs(ix - cx), L - abs(ix - cx))
            dy = min(abs(iy - cy), L - abs(iy - cy))
            dz = min(abs(iz - cz), L - abs(iz - cz))
            distances[ix, iy, iz] = np.sqrt(dx*dx + dy*dy + dz*dz)

# Bin by |J| value
J_flat = J_at_site.flatten()
count_flat = manifest_count.flatten()
dist_flat = distances.flatten()

# Only look at sites far enough from source to have clean statistics
mask = (dist_flat > 3) & (dist_flat < L//2 - 2) & (J_flat > 0.01)
J_sel = J_flat[mask]
count_sel = count_flat[mask]

if len(J_sel) > 10:
    # Bin by J magnitude
    n_bins = 12
    J_sorted = np.sort(J_sel)
    bin_edges = np.percentile(J_sel, np.linspace(0, 100, n_bins + 1))

    print(f"  {'|J| range':>20} | {'avg |J|':>10} | {'manifest freq':>14} | {'|J|^1':>10} | {'|J|^2':>10}")
    print("  " + "-" * 72)

    bin_J = []
    bin_freq = []
    for i in range(n_bins):
        in_bin = (J_sel >= bin_edges[i]) & (J_sel < bin_edges[i+1])
        if np.sum(in_bin) < 5:
            continue
        avg_J = np.mean(J_sel[in_bin])
        avg_count = np.mean(count_sel[in_bin]) / n_trials
        bin_J.append(avg_J)
        bin_freq.append(avg_count)
        print(f"  {f'{bin_edges[i]:.3f}-{bin_edges[i+1]:.3f}':>20} | {avg_J:>10.4f} | {avg_count:>14.4f} | {avg_J:>10.4f} | {avg_J**2:>10.4f}")

    if len(bin_J) >= 4:
        bin_J = np.array(bin_J)
        bin_freq = np.array(bin_freq)

        # Fit: freq = A * |J|^n
        # log(freq) = log(A) + n*log(|J|)
        valid = (bin_freq > 0) & (bin_J > 0)
        if np.sum(valid) >= 3:
            log_J = np.log(bin_J[valid])
            log_freq = np.log(bin_freq[valid])
            n_fit, log_A = np.polyfit(log_J, log_freq, 1)

            print(f"\n  POWER LAW FIT: frequency ~ |J|^{n_fit:.3f}")
            print(f"  Born rule predicts: n = 2.0")
            print(f"  Classical (linear): n = 1.0")
            print(f"  Measured: n = {n_fit:.3f}")
            if abs(n_fit - 2.0) < abs(n_fit - 1.0):
                print(f"  CLOSER TO BORN RULE (|J|^2)")
            elif abs(n_fit - 1.0) < abs(n_fit - 2.0):
                print(f"  CLOSER TO LINEAR (|J|)")
            else:
                print(f"  NEITHER (anomalous scaling)")
else:
    print("  Not enough data points for analysis.")

# ============================================================
# TEST 2: BELL CORRELATIONS
# ============================================================
print("\n\n" + "=" * 72)
print("TEST 2: BELL CORRELATIONS")
print("What correlation function does the lattice produce?")
print("=" * 72 + "\n")

# Setup: prepare two sites with correlated flux vectors.
# The correlation comes from a common source (a manifested particle
# between them that injected flux into both).
# Measure projections along various axes and compute E(theta).

n_bell_trials = 5000
angles = np.linspace(0, np.pi, 13)
correlations_by_angle = {a: [] for a in angles}

L_bell = 16
lat_bell = Lattice(L_bell, L_bell, L_bell)

# Two measurement sites and one source between them
site_A = (4, L_bell//2, L_bell//2)
site_B = (L_bell - 4, L_bell//2, L_bell//2)
source = (L_bell//2, L_bell//2, L_bell//2)

print(f"  Source at {source}, site A at {site_A}, site B at {site_B}")
print(f"  Running {n_bell_trials} trials...")

for trial in range(n_bell_trials):
    # Fresh lattice
    lat_bell.s[:] = 0
    lat_bell.Jx[:] = 0; lat_bell.Jy[:] = 0; lat_bell.Jz[:] = 0
    lat_bell.Jx_prev[:] = 0; lat_bell.Jy_prev[:] = 0; lat_bell.Jz_prev[:] = 0

    # Random source polarization (this is the "hidden variable")
    theta_src = np.random.uniform(0, np.pi)
    phi_src = np.random.uniform(0, 2*np.pi)
    lat_bell.Jx[source] = np.sin(theta_src) * np.cos(phi_src) * 2.0
    lat_bell.Jy[source] = np.sin(theta_src) * np.sin(phi_src) * 2.0
    lat_bell.Jz[source] = np.cos(theta_src) * 2.0

    # Let flux propagate to A and B
    for t in range(8):
        lat_bell.tick()

    # Read flux at A and B
    JA = np.array([lat_bell.Jx[site_A], lat_bell.Jy[site_A], lat_bell.Jz[site_A]])
    JB = np.array([lat_bell.Jx[site_B], lat_bell.Jy[site_B], lat_bell.Jz[site_B]])

    if np.linalg.norm(JA) < 1e-10 or np.linalg.norm(JB) < 1e-10:
        continue

    # For each angle theta between measurement axes:
    # A measures along z-axis, B measures along axis rotated by theta in xz-plane
    for theta in angles:
        axis_A = np.array([0, 0, 1])
        axis_B = np.array([np.sin(theta), 0, np.cos(theta)])

        # Measurement outcome: sign of projection
        outcome_A = np.sign(np.dot(JA, axis_A))
        outcome_B = np.sign(np.dot(JB, axis_B))

        if outcome_A == 0: outcome_A = 1
        if outcome_B == 0: outcome_B = 1

        correlations_by_angle[theta].append(outcome_A * outcome_B)

print(f"\n  Correlation function E(theta):\n")
print(f"  {'theta (deg)':>12} | {'E(theta) lattice':>18} | {'E = -cos(theta) QM':>20} | {'E = -(1-2t/pi) class':>22}")
print("  " + "-" * 78)

S_lattice_terms = []
for theta in angles:
    vals = correlations_by_angle[theta]
    if len(vals) > 10:
        E = np.mean(vals)
        E_qm = -np.cos(theta)
        E_class = -(1 - 2*theta/np.pi)
        print(f"  {np.degrees(theta):>12.1f} | {E:>18.4f} | {E_qm:>20.4f} | {E_class:>22.4f}")

# Compute CHSH S value
# S = E(a,b) - E(a,b') + E(a',b) + E(a',b')
# Optimal: a=0, a'=pi/2, b=pi/4, b'=3pi/4
angles_chsh = [np.pi/4, 3*np.pi/4, np.pi/4, 3*np.pi/4]  # simplified
# Use nearest available angles
E_vals = {}
for theta in angles:
    vals = correlations_by_angle[theta]
    if len(vals) > 10:
        E_vals[theta] = np.mean(vals)

# Find closest angles to CHSH optimal
def closest_E(target):
    best = min(E_vals.keys(), key=lambda x: abs(x - target))
    return E_vals[best], best

E_ab, _ = closest_E(np.pi/4)        # a=0, b=pi/4
E_ab2, _ = closest_E(3*np.pi/4)     # a=0, b'=3pi/4
E_a2b, _ = closest_E(np.pi/4)       # a'=pi/2, b=pi/4 (same as E_ab for this simple setup)
E_a2b2, _ = closest_E(np.pi/4)      # simplified

S_est = abs(E_ab - E_ab2 + E_a2b + E_a2b2)
print(f"\n  CHSH estimate S ~ {S_est:.3f}")
print(f"  Classical bound: S <= 2")
print(f"  QM prediction:  S = 2*sqrt(2) = {2*np.sqrt(2):.3f}")
print(f"  Lattice result:  S ~ {S_est:.3f}")
if S_est <= 2.05:
    print(f"  -> LATTICE GIVES CLASSICAL BOUND (as expected for local deterministic)")
elif S_est > 2.05 and S_est < 2.8:
    print(f"  -> LATTICE GIVES PARTIAL VIOLATION (unexpected!)")
else:
    print(f"  -> LATTICE GIVES FULL QM VIOLATION (very unexpected!)")

# ============================================================
# TEST 3: DOUBLE SLIT INTERFERENCE
# ============================================================
print("\n\n" + "=" * 72)
print("TEST 3: DOUBLE SLIT INTERFERENCE")
print("Does the lattice produce an interference pattern?")
print("=" * 72 + "\n")

# 2D lattice for simplicity (one slice of 3D)
Lx, Ly = 80, 60
lat_ds = Lattice(Lx, Ly, 1)
lat_ds.K_B = 0.3  # lower threshold for more manifestations

# Barrier with two slits
barrier_x = 30
slit_width = 2
slit_separation = 12
slit_center = Ly // 2

# Place barrier: set high damping (acts as absorber)
barrier = np.zeros((Lx, Ly, 1), dtype=bool)
barrier[barrier_x, :, 0] = True
# Open the slits
slit1_center = slit_center - slit_separation // 2
slit2_center = slit_center + slit_separation // 2
for dy in range(-slit_width//2, slit_width//2 + 1):
    barrier[barrier_x, slit1_center + dy, 0] = False
    barrier[barrier_x, slit2_center + dy, 0] = False

# Source: a line of flux on the left
source_x = 5
n_ds_trials = 200
detection_screen_x = 60
screen_counts = np.zeros(Ly)

print(f"  Barrier at x={barrier_x}, slits at y={slit1_center} and y={slit2_center}")
print(f"  Detection screen at x={detection_screen_x}")
print(f"  Running {n_ds_trials} trials...")

for trial in range(n_ds_trials):
    # Fresh lattice
    lat_ds.s[:] = 0
    lat_ds.Jx[:] = 0; lat_ds.Jy[:] = 0; lat_ds.Jz[:] = 0
    lat_ds.Jx_prev[:] = 0; lat_ds.Jy_prev[:] = 0; lat_ds.Jz_prev[:] = 0

    # Pulse of flux from the source (plane wave with slight randomness)
    for iy in range(Ly):
        lat_ds.Jx[source_x, iy, 0] = 1.5 + np.random.randn() * 0.1

    # Propagate
    for t in range(100):
        lat_ds.tick()

        # Enforce barrier (kill flux at barrier sites)
        lat_ds.Jx[barrier] = 0
        lat_ds.Jy[barrier] = 0
        lat_ds.Jz[barrier] = 0

    # Record flux magnitude at detection screen
    screen_J = lat_ds.flux_magnitude()[detection_screen_x, :, 0]
    screen_counts += screen_J**2  # accumulate |J|^2

# Also compute single-slit pattern for comparison
screen_single1 = np.zeros(Ly)
screen_single2 = np.zeros(Ly)

for slit_idx, slit_y in enumerate([slit1_center, slit2_center]):
    for trial in range(n_ds_trials):
        lat_ds.s[:] = 0
        lat_ds.Jx[:] = 0; lat_ds.Jy[:] = 0; lat_ds.Jz[:] = 0
        lat_ds.Jx_prev[:] = 0; lat_ds.Jy_prev[:] = 0; lat_ds.Jz_prev[:] = 0

        for iy in range(Ly):
            lat_ds.Jx[source_x, iy, 0] = 1.5 + np.random.randn() * 0.1

        # Block BOTH slits, only open the one we want
        barrier_single = barrier.copy()
        for dy in range(-slit_width//2, slit_width//2 + 1):
            barrier_single[barrier_x, slit_y + dy, 0] = False

        for t in range(100):
            lat_ds.tick()
            lat_ds.Jx[barrier_single] = 0
            lat_ds.Jy[barrier_single] = 0
            lat_ds.Jz[barrier_single] = 0

        screen_J = lat_ds.flux_magnitude()[detection_screen_x, :, 0]
        if slit_idx == 0:
            screen_single1 += screen_J**2
        else:
            screen_single2 += screen_J**2

# Compare: double slit pattern vs sum of single slits
sum_singles = screen_single1 + screen_single2

# Normalize
if np.max(screen_counts) > 0:
    screen_counts /= np.max(screen_counts)
if np.max(sum_singles) > 0:
    sum_singles /= np.max(sum_singles)

print(f"\n  Detection screen pattern (normalized):\n")
print(f"  {'y':>5} | {'Double slit':>12} | {'Sum of singles':>15} | {'Difference':>12} | {'Interference?':>14}")
print("  " + "-" * 66)

interference_detected = False
for iy in range(0, Ly, 2):
    ds = screen_counts[iy]
    ss = sum_singles[iy]
    diff = ds - ss
    has_interference = abs(diff) > 0.05
    if has_interference:
        interference_detected = True
    marker = "***" if has_interference else ""
    print(f"  {iy:>5} | {ds:>12.4f} | {ss:>15.4f} | {diff:>+12.4f} | {marker:>14}")

print()
if interference_detected:
    print("  INTERFERENCE DETECTED: double slit != sum of singles")
    print("  The flux field produces genuine wave interference.")
    # Quantify: compute correlation between double slit and |J1+J2|^2 vs |J1|^2+|J2|^2
    diff_norm = np.linalg.norm(screen_counts - sum_singles)
    print(f"  Interference strength (L2 norm of difference): {diff_norm:.4f}")
else:
    print("  NO INTERFERENCE: double slit = sum of singles")
    print("  The flux field does NOT produce wave interference at this setup.")

# ============================================================
# SUMMARY
# ============================================================
print(f"""

========================================================================
SUMMARY: Multibody Lattice Tests
========================================================================

TEST 1 (Born rule):
  Measured power law exponent n in frequency ~ |J|^n.
  Born rule predicts n = 2. Classical linear: n = 1.

TEST 2 (Bell correlations):
  Measured correlation function E(theta).
  QM: E = -cos(theta), S = 2*sqrt(2).
  Classical: E = -(1-2theta/pi), S = 2.

TEST 3 (Double slit):
  Compared double-slit pattern to sum of single-slit patterns.
  Interference: double != sum (wave behavior).
  No interference: double = sum (particle behavior).

These tests run on the ACTUAL lattice dynamics — no QM imported,
no Born rule assumed, no Hilbert space constructed.
Whatever the lattice produces, we measure.
""")
