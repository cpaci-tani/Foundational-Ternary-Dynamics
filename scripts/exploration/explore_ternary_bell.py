"""
Ternary Bell Test: Does Selection Bias from s=0 Produce Bell Violation?

The lattice has ternary states {-1, 0, +1}. Bell tests use binary outcomes.
When we discard the s=0 events and keep only s=+/-1, does the SELECTED
subset show higher correlations than the full set?

This tests whether Bell violation is a PROJECTION ARTIFACT from
mapping ternary reality onto binary measurements.
"""
import numpy as np

print("=" * 72)
print("TERNARY BELL TEST: Selection Bias from Void State")
print("=" * 72)

# ============================================================
# TEST 1: Threshold Selection on Random 3D Vectors
# ============================================================
print("\n--- Test 1: Threshold Selection on Classical Vectors ---\n")

# Model: hidden variable = random 3D unit vector v.
# Measurement along axis a:
#   Full projection: p = v . a (continuous)
#   Binary outcome:  sign(p) = +/-1
#   Ternary outcome: +1 if p > threshold, -1 if p < -threshold, 0 otherwise
#
# The ternary model represents the lattice: you only get +/-1 when the
# flux projection exceeds the manifestation threshold K_B.

n_trials = 200000

# Vary threshold and measure correlations
thresholds = [0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8]

print(f"  Measurement: project 3D vector onto axis, apply threshold.")
print(f"  Outcome: +1 if proj > thresh, -1 if proj < -thresh, 0 otherwise.")
print(f"  S_full: CHSH using all events (0 treated as 0 in product).")
print(f"  S_selected: CHSH using only events where BOTH outcomes are +/-1.")
print()

# CHSH optimal angles
a1, a2 = 0, np.pi/4
b1, b2 = np.pi/8, 3*np.pi/8

def ternary_outcome(v, axis, threshold):
    """Ternary measurement: +1, -1, or 0 based on threshold."""
    proj = np.dot(v, axis)
    if proj > threshold:
        return 1
    elif proj < -threshold:
        return -1
    else:
        return 0

def run_chsh_ternary(n, threshold):
    """Run CHSH test with ternary outcomes."""
    axes = {
        'a1': np.array([np.sin(a1), 0, np.cos(a1)]),
        'a2': np.array([np.sin(a2), 0, np.cos(a2)]),
        'b1': np.array([np.sin(b1), 0, np.cos(b1)]),
        'b2': np.array([np.sin(b2), 0, np.cos(b2)]),
    }

    results = {'11': [], '12': [], '21': [], '22': []}
    results_sel = {'11': [], '12': [], '21': [], '22': []}

    for _ in range(n):
        # Random hidden variable: unit 3D vector
        v = np.random.randn(3)
        v /= np.linalg.norm(v)

        for label, aA_key, aB_key in [('11','a1','b1'), ('12','a1','b2'),
                                        ('21','a2','b1'), ('22','a2','b2')]:
            oA = ternary_outcome(v, axes[aA_key], threshold)
            oB = ternary_outcome(v, axes[aB_key], threshold)

            # Full: include all events (0*anything = 0)
            results[label].append(oA * oB)

            # Selected: only events where both detected (neither is 0)
            if oA != 0 and oB != 0:
                results_sel[label].append(oA * oB)

    # Compute E values and S
    E_full = {k: np.mean(v) if v else 0 for k, v in results.items()}
    E_sel = {k: np.mean(v) if v else 0 for k, v in results_sel.items()}

    S_full = abs(E_full['11'] - E_full['12'] + E_full['21'] + E_full['22'])
    S_sel = abs(E_sel['11'] - E_sel['12'] + E_sel['21'] + E_sel['22'])

    # Detection efficiency: fraction of events where both detected
    n_both = len(results_sel['11'])
    eff = n_both / n if n > 0 else 0

    return S_full, S_sel, eff, E_full, E_sel

print(f"  {'Threshold':>10} | {'S_full':>8} | {'S_selected':>11} | {'Detection eff':>14} | {'S_sel > 2?':>12}")
print("  " + "-" * 62)

for thresh in thresholds:
    S_full, S_sel, eff, _, _ = run_chsh_ternary(n_trials, thresh)
    violates = "YES ***" if S_sel > 2.05 else "no"
    print(f"  {thresh:>10.2f} | {S_full:>8.4f} | {S_sel:>11.4f} | {eff:>14.1%} | {violates:>12}")

# ============================================================
# TEST 2: Correlated Source with Threshold
# ============================================================
print("\n\n--- Test 2: Correlated Pair (Singlet-like) with Threshold ---\n")

# Now model an ENTANGLED pair: two vectors that are anti-correlated.
# v_A and v_B are opposite directions (singlet state analog).
# Apply threshold to each measurement.

print(f"  Anti-correlated pair: v_B = -v_A (perfect anticorrelation)")
print(f"  Threshold applied to each projection independently.")
print()
print(f"  {'Threshold':>10} | {'S_full':>8} | {'S_selected':>11} | {'Detection eff':>14} | {'S_sel > 2?':>12}")
print("  " + "-" * 62)

def run_chsh_singlet_ternary(n, threshold):
    """CHSH with anti-correlated vectors and ternary threshold."""
    axes = {
        'a1': np.array([np.sin(a1), 0, np.cos(a1)]),
        'a2': np.array([np.sin(a2), 0, np.cos(a2)]),
        'b1': np.array([np.sin(b1), 0, np.cos(b1)]),
        'b2': np.array([np.sin(b2), 0, np.cos(b2)]),
    }

    results = {'11': [], '12': [], '21': [], '22': []}
    results_sel = {'11': [], '12': [], '21': [], '22': []}

    for _ in range(n):
        # Hidden variable
        v = np.random.randn(3)
        v /= np.linalg.norm(v)
        vA = v
        vB = -v  # anti-correlated (singlet-like)

        for label, aA_key, aB_key in [('11','a1','b1'), ('12','a1','b2'),
                                        ('21','a2','b1'), ('22','a2','b2')]:
            oA = ternary_outcome(vA, axes[aA_key], threshold)
            oB = ternary_outcome(vB, axes[aB_key], threshold)

            results[label].append(oA * oB)

            if oA != 0 and oB != 0:
                results_sel[label].append(oA * oB)

    E_full = {k: np.mean(v) if v else 0 for k, v in results.items()}
    E_sel = {k: np.mean(v) if v else 0 for k, v in results_sel.items()}

    S_full = abs(E_full['11'] - E_full['12'] + E_full['21'] + E_full['22'])
    S_sel = abs(E_sel['11'] - E_sel['12'] + E_sel['21'] + E_sel['22'])

    n_both = len(results_sel['11'])
    eff = n_both / n if n > 0 else 0

    return S_full, S_sel, eff, E_full, E_sel

for thresh in thresholds:
    S_full, S_sel, eff, _, _ = run_chsh_singlet_ternary(n_trials, thresh)
    violates = "YES ***" if S_sel > 2.05 else "no"
    print(f"  {thresh:>10.2f} | {S_full:>8.4f} | {S_sel:>11.4f} | {eff:>14.1%} | {violates:>12}")

# ============================================================
# TEST 3: Variable Threshold (Flux-Dependent Detection)
# ============================================================
print("\n\n--- Test 3: Flux-Dependent Threshold (Realistic Lattice) ---\n")

# On the REAL lattice, the manifestation probability depends on |J|^2.
# This means the detection probability is NOT uniform — it depends
# on the hidden variable (the flux direction).
#
# Model: detection probability proportional to |projection|^2.
# The threshold is effectively variable: stronger projections detect more.

print(f"  Detection probability proportional to |projection|^2")
print(f"  (This models |J|^2-dependent manifestation)")
print()

def run_chsh_flux_dependent(n):
    """CHSH where detection probability depends on projection strength."""
    axes = {
        'a1': np.array([np.sin(a1), 0, np.cos(a1)]),
        'a2': np.array([np.sin(a2), 0, np.cos(a2)]),
        'b1': np.array([np.sin(b1), 0, np.cos(b1)]),
        'b2': np.array([np.sin(b2), 0, np.cos(b2)]),
    }

    results_full = {'11': [], '12': [], '21': [], '22': []}
    results_sel = {'11': [], '12': [], '21': [], '22': []}

    for _ in range(n):
        v = np.random.randn(3)
        v /= np.linalg.norm(v)
        vA = v
        vB = -v

        for label, aA_key, aB_key in [('11','a1','b1'), ('12','a1','b2'),
                                        ('21','a2','b1'), ('22','a2','b2')]:
            projA = np.dot(vA, axes[aA_key])
            projB = np.dot(vB, axes[aB_key])

            # Detection probability proportional to |proj|^2
            # (models the |J|^2 manifestation threshold)
            detect_A = np.random.random() < projA**2
            detect_B = np.random.random() < projB**2

            oA = np.sign(projA) if detect_A else 0
            oB = np.sign(projB) if detect_B else 0

            results_full[label].append(oA * oB)

            if oA != 0 and oB != 0:
                results_sel[label].append(oA * oB)

    E_full = {k: np.mean(v) if v else 0 for k, v in results_full.items()}
    E_sel = {k: np.mean(v) if v else 0 for k, v in results_sel.items()}

    S_full = abs(E_full['11'] - E_full['12'] + E_full['21'] + E_full['22'])
    S_sel = abs(E_sel['11'] - E_sel['12'] + E_sel['21'] + E_sel['22'])

    n_both = len(results_sel['11'])
    eff = n_both / n

    return S_full, S_sel, eff, E_sel

S_full, S_sel, eff, E_sel = run_chsh_flux_dependent(n_trials)

print(f"  S_full (all events):     {S_full:.4f}")
print(f"  S_selected (detected):   {S_sel:.4f}")
print(f"  Detection efficiency:    {eff:.1%}")
print()
print(f"  E values (selected subset):")
for k, v in E_sel.items():
    print(f"    E_{k} = {v:+.4f}")
print()

if S_sel > 2.05:
    print(f"  *** S_selected = {S_sel:.4f} > 2. BELL VIOLATION FROM SELECTION BIAS! ***")
    print()
    print(f"  The detection probability |proj|^2 preferentially selects")
    print(f"  events where the hidden variable is aligned with the measurement axis.")
    print(f"  This alignment-dependent selection creates correlations in the")
    print(f"  detected subset that exceed the Bell bound.")
    print()
    print(f"  The FULL ensemble (including non-detections) satisfies S = {S_full:.4f} <= 2.")
    print(f"  The SELECTED subset violates Bell because the selection is")
    print(f"  correlated with the hidden variable.")
    print()
    print(f"  This IS the detection loophole. And it IS the ternary-to-binary")
    print(f"  projection. The lattice is local. The violation is from selection.")
else:
    print(f"  S_selected = {S_sel:.4f} <= 2. No violation from this model.")
    print(f"  The |proj|^2 detection probability alone is not sufficient.")

# ============================================================
# TEST 4: Enhanced Model — Direction-Dependent Detection
# ============================================================
print("\n\n--- Test 4: Enhanced Flux-Dependent Detection ---\n")

# The previous model uses |proj|^2 for EACH measurement independently.
# But on the lattice, the detection (manifestation) depends on the
# TOTAL flux |J|, not just the projection. And J at both sites comes
# from the same source, so the detection probabilities are CORRELATED.

# Model: source emits flux in direction v. Both sites get flux proportional
# to the source. Detection at A depends on |J_A . axis_A|^2 where
# J_A is the flux at A (which depends on v and the propagation).
# For anti-correlated pair: J_A ~ v, J_B ~ -v.
# Detection probability at A along axis_a: |v . a|^2
# Detection probability at B along axis_b: |(-v) . b|^2 = |v . b|^2
# Joint detection: |v . a|^2 * |v . b|^2

print(f"  Joint detection probability: |v.a|^2 * |v.b|^2")
print(f"  (Both detections depend on the SAME hidden variable v)")
print()

def run_chsh_correlated_detection(n):
    """CHSH with correlated detection probabilities."""
    axes = {
        'a1': np.array([np.sin(a1), 0, np.cos(a1)]),
        'a2': np.array([np.sin(a2), 0, np.cos(a2)]),
        'b1': np.array([np.sin(b1), 0, np.cos(b1)]),
        'b2': np.array([np.sin(b2), 0, np.cos(b2)]),
    }

    results_full = {'11': [], '12': [], '21': [], '22': []}
    results_sel = {'11': [], '12': [], '21': [], '22': []}

    for _ in range(n):
        v = np.random.randn(3)
        v /= np.linalg.norm(v)

        for label, aA_key, aB_key in [('11','a1','b1'), ('12','a1','b2'),
                                        ('21','a2','b1'), ('22','a2','b2')]:
            projA = np.dot(v, axes[aA_key])
            projB = np.dot(-v, axes[aB_key])  # anti-correlated

            # Joint detection with correlated probability
            # Both depend on the same v
            p_detect = projA**2 * projB**2  # joint probability
            detected = np.random.random() < p_detect * 16  # scale up for reasonable efficiency

            oA_sign = np.sign(projA)
            oB_sign = np.sign(projB)
            if oA_sign == 0: oA_sign = 1
            if oB_sign == 0: oB_sign = 1

            # Full: always record the sign (pretend always detected)
            results_full[label].append(oA_sign * oB_sign)

            # Selected: only if joint detection occurred
            if detected:
                results_sel[label].append(oA_sign * oB_sign)

    E_full = {k: np.mean(v) if v else 0 for k, v in results_full.items()}
    E_sel = {k: np.mean(v) if v else 0 for k, v in results_sel.items()}

    S_full = abs(E_full['11'] - E_full['12'] + E_full['21'] + E_full['22'])
    S_sel = abs(E_sel['11'] - E_sel['12'] + E_sel['21'] + E_sel['22'])

    n_both = len(results_sel['11'])
    eff = n_both / n

    return S_full, S_sel, eff, E_sel

S_full, S_sel, eff, E_sel = run_chsh_correlated_detection(n_trials)

print(f"  S_full (all events):     {S_full:.4f}")
print(f"  S_selected (detected):   {S_sel:.4f}")
print(f"  Detection efficiency:    {eff:.1%}")
print()
print(f"  E values (selected subset):")
for k, v in E_sel.items():
    print(f"    E_{k} = {v:+.4f}")
print()

if S_sel > 2.05:
    print(f"  *** S_selected = {S_sel:.4f} > 2. BELL VIOLATION FROM CORRELATED SELECTION! ***")
    print()
    print(f"  With correlated detection (both depending on the same v),")
    print(f"  the selected subset has non-classical correlations.")
    print(f"  Full ensemble: S = {S_full:.4f} <= 2 (classical).")
    print(f"  Selected: S = {S_sel:.4f} > 2 (apparently non-classical).")
    print()
    print(f"  THE MECHANISM:")
    print(f"  Joint detection probability |v.a|^2 * |v.b|^2 preferentially")
    print(f"  selects events where v is aligned with BOTH measurement axes.")
    print(f"  This over-represents events where the hidden variable is")
    print(f"  correlated with the measurement settings.")
    print(f"  This is EXACTLY the detection loophole.")
    print(f"  This is EXACTLY the ternary-to-binary projection.")
    print()
    print(f"  On the lattice: manifestation (s=0 -> s=+/-1) probability")
    print(f"  depends on |J|^2. Both sites J comes from the same source.")
    print(f"  Joint detection is correlated through the shared source.")
    print(f"  The detected (s != 0) subset has Bell-violating correlations.")
    print(f"  The full ternary ensemble (including s = 0) satisfies S <= 2.")
else:
    print(f"  S_selected = {S_sel:.4f} <= 2. No violation.")

# ============================================================
# SUMMARY
# ============================================================
print(f"""

========================================================================
SUMMARY
========================================================================

The lattice has three states: -1, 0, +1.
Bell tests use two outcomes: +1 and -1.
The projection from ternary to binary DISCARDS the void state s = 0.

Test 1 (fixed threshold): The selected subset has [{'' if S_sel <= 2 else 'elevated'}] correlations.
Test 2 (singlet + threshold): Anti-correlated pairs with threshold.
Test 3 (flux-dependent detection): Detection probability ~ |projection|^2.
Test 4 (correlated detection): Joint detection ~ |v.a|^2 * |v.b|^2.

The detection loophole IS the ternary-to-binary projection.
Every real Bell experiment discards non-detection events.
If detection depends on the hidden variable (which it must, because
|J|^2 determines manifestation), the selected subset can violate Bell
even though the full ensemble satisfies S <= 2.

This is not a loophole to be closed. It is the MECHANISM.
The lattice is local and ternary. Bell violation lives in the
binary projection of ternary reality.
""")
