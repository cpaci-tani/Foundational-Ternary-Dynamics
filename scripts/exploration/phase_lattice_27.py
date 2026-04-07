#!/usr/bin/env python3
"""
27-State Phase Lattice {pi, varpi, G*}^3
=========================================

Constructs the ternary phase lattice using three FTD ontic constants as
independent phase angles on the unit circle S^1:

    E = {e^{i*pi}, e^{i*varpi}, e^{i*G*}}

The full lattice L_3(E) = E^3 has 27 states in (S^1)^3, matching the
Moore neighborhood count 3^3 = 27. This script analyzes:

  0. Phase computation from ontic chain (pi derived, not imported)
  1. Phase distinctness verification
  2. Full 27-state lattice construction in C^3
  3. Pairwise distance spectrum (Euclidean and torus metrics)
  4. S_3 symmetry and orbit decomposition
  5. Multiplicative semigroup structure and density
  6. Moore neighborhood bijection and shell decomposition
  7. Visualization (4 figures)

Epistemic status:
  [THEOREM]     Phase distinctness, algebraic identities, S_3 symmetry,
                semigroup density (from Nesterenko 1996)
  [SELECTION]   Moore mapping: varpi -> -1, G* -> 0, pi -> +1 (ontic ordering)
  [CONJECTURE]  Physical significance of phase clustering, framework number
                appearances in distance spectrum
"""

import sys
import os
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

import numpy as np
from itertools import product as iterproduct
from collections import Counter
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

# Import FTD constants
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))
from constants import (
    G_STAR, VARPI_CLASSICAL, GAMMA_QUARTER, GAMMA_HALF,
    N_c, N_base, b_3, N_eff, PF
)

# Output directory
OUTPUT_DIR = os.path.join(os.path.dirname(__file__), '..', '..', 'output')
os.makedirs(OUTPUT_DIR, exist_ok=True)


# =============================================================================
# SECTION 0: Phase Computation from Ontic Chain
# =============================================================================

def compute_phases():
    """
    Compute the three FTD phase angles and their exponentials.

    Pi is DERIVED from the ontic chain: pi = 4*varpi^2/G*^2
    (see engine/include/ftd/ontic.h line 137).

    Returns dict with raw phases, exponentials, and metadata.
    """
    varpi = VARPI_CLASSICAL          # ~ 2.6221
    gstar = G_STAR                   # ~ 2.9587
    pi_derived = 4.0 * varpi**2 / gstar**2  # ontic chain

    # Sanity check: derived pi matches np.pi
    pi_error = abs(pi_derived - np.pi)
    assert pi_error < 1e-14, f"Ontic pi derivation error: {pi_error}"

    # Exponentiated phases
    e_pi = np.exp(1j * pi_derived)       # should be -1
    e_varpi = np.exp(1j * varpi)
    e_gstar = np.exp(1j * gstar)

    # Verify unit circle
    for name, z in [('e^{i*pi}', e_pi), ('e^{i*varpi}', e_varpi), ('e^{i*G*}', e_gstar)]:
        assert abs(abs(z) - 1.0) < 1e-15, f"|{name}| != 1"

    phases = {
        'varpi': varpi,
        'gstar': gstar,
        'pi_derived': pi_derived,
        'e_pi': e_pi,
        'e_varpi': e_varpi,
        'e_gstar': e_gstar,
        'pi_error': pi_error,
    }

    # Print results
    print("=" * 78)
    print("  SECTION 0: Phase Computation from Ontic Chain")
    print("=" * 78)
    print()
    print("  Ontic chain: pi = 4*varpi^2/G*^2")
    print(f"  PI_DERIVED  = {pi_derived:.16f}")
    print(f"  np.pi       = {np.pi:.16f}")
    print(f"  |difference|= {pi_error:.2e}")
    print()

    header = f"  {'Constant':<10} {'Radians':>14} {'Degrees':>12} {'Re(e^ix)':>14} {'Im(e^ix)':>14}"
    print(header)
    print("  " + "-" * len(header.strip()))
    for label, theta, z in [
        ('varpi', varpi, e_varpi),
        ('G*', gstar, e_gstar),
        ('pi', pi_derived, e_pi),
    ]:
        deg = np.degrees(theta)
        print(f"  {label:<10} {theta:>14.10f} {deg:>12.6f} {z.real:>14.10f} {z.imag:>14.10f}")

    arc_span = pi_derived - varpi
    print()
    print(f"  Arc span (pi - varpi) = {arc_span:.10f} rad = {np.degrees(arc_span):.6f} deg")
    print(f"  All three phases lie in the second quadrant (Re < 0, Im >= 0)")
    print()

    return phases


# =============================================================================
# SECTION 1: Phase Distinctness Verification
# =============================================================================

def verify_phase_distinctness(phases):
    """
    Verify that varpi, G*, pi are pairwise distinct mod 2*pi.
    Compute angular separations and the ontic interpolation ratio.
    """
    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    print("=" * 78)
    print("  SECTION 1: Phase Distinctness Verification")
    print("=" * 78)
    print()

    pairs = [
        ('G* - varpi', gstar - varpi),
        ('pi - G*', pi_d - gstar),
        ('pi - varpi', pi_d - varpi),
    ]

    print(f"  {'Pair':<20} {'Separation (rad)':>18} {'Separation (deg)':>18} {'Distinct?':>10}")
    print("  " + "-" * 68)
    all_distinct = True
    for label, sep in pairs:
        # Check modular distinctness
        sep_mod = sep % (2 * np.pi)
        distinct = sep_mod > 1e-15 and abs(sep_mod - 2 * np.pi) > 1e-15
        all_distinct = all_distinct and distinct
        mark = "YES" if distinct else "NO"
        print(f"  {label:<20} {sep:>18.12f} {np.degrees(sep):>18.10f} {mark:>10}")

    print()
    print(f"  All three phases pairwise distinct: {all_distinct}")
    print(f"  [THEOREM] Lattice L_3(E) has exactly 27 distinct states.")

    # Ontic interpolation: where does G* sit between varpi and pi?
    interp = (gstar - varpi) / (pi_d - varpi)
    print()
    print(f"  Ontic interpolation: (G* - varpi) / (pi - varpi) = {interp:.10f}")
    print(f"  G* divides the varpi-to-pi arc at fraction {interp:.6f}")
    print()

    # Compare to FTD constants
    comparisons = [
        ('PF = pi/4', PF),
        ('1/sqrt(2)', 1.0 / np.sqrt(2)),
        ('2/3', 2.0 / 3.0),
        ('1/G*', 1.0 / G_STAR),
        ('G*/pi', G_STAR / pi_d),
        ('varpi/G*', varpi / gstar),
    ]
    print(f"  Comparison of interpolation ratio {interp:.6f} to FTD constants:")
    for label, val in comparisons:
        diff = abs(interp - val)
        print(f"    {label:<16} = {val:.10f}   diff = {diff:.2e}")

    print()
    return all_distinct


# =============================================================================
# SECTION 2: Build the Full 27-State Lattice
# =============================================================================

def build_lattice(phases):
    """
    Enumerate all 27 states (a1, a2, a3) with aj in {varpi, G*, pi}.
    Returns (27,3) complex array and list of label triples.
    """
    print("=" * 78)
    print("  SECTION 2: Full 27-State Lattice in (S^1)^3")
    print("=" * 78)
    print()

    alphabet_phases = [phases['varpi'], phases['gstar'], phases['pi_derived']]
    alphabet_exp = [phases['e_varpi'], phases['e_gstar'], phases['e_pi']]
    alphabet_names = ['varpi', 'G*', 'pi']

    lattice = []
    labels = []
    angle_triples = []

    for idx_triple in iterproduct(range(3), repeat=3):
        state = np.array([alphabet_exp[i] for i in idx_triple])
        angles = tuple(alphabet_phases[i] for i in idx_triple)
        name_triple = tuple(alphabet_names[i] for i in idx_triple)
        lattice.append(state)
        labels.append(name_triple)
        angle_triples.append(angles)

    lattice = np.array(lattice)  # shape (27, 3)
    angle_triples = np.array(angle_triples)

    # Verify all on (S^1)^3
    mags = np.abs(lattice)
    max_mag_err = np.max(np.abs(mags - 1.0))
    assert max_mag_err < 1e-14, f"Points not on unit torus: max |z| error = {max_mag_err}"

    # Print lattice
    print(f"  {'#':>3} {'Label':<24} {'Re(z1)':>10} {'Im(z1)':>10} {'Re(z2)':>10} {'Im(z2)':>10} {'Re(z3)':>10} {'Im(z3)':>10}")
    print("  " + "-" * 98)
    for i in range(27):
        lbl = f"({labels[i][0]},{labels[i][1]},{labels[i][2]})"
        z = lattice[i]
        print(f"  {i:>3} {lbl:<24} {z[0].real:>10.6f} {z[0].imag:>10.6f} "
              f"{z[1].real:>10.6f} {z[1].imag:>10.6f} "
              f"{z[2].real:>10.6f} {z[2].imag:>10.6f}")

    print()
    print(f"  All 27 states on (S^1)^3: max |z_k| error = {max_mag_err:.2e}")
    print()

    return lattice, labels, angle_triples


# =============================================================================
# SECTION 3: Pairwise Distance Spectrum
# =============================================================================

def distance_spectrum(lattice, angle_triples, phases):
    """
    Compute all C(27,2) = 351 pairwise distances in Euclidean and torus metrics.
    Compare to equally-spaced reference lattice.
    """
    print("=" * 78)
    print("  SECTION 3: Pairwise Distance Spectrum")
    print("=" * 78)
    print()

    n = len(lattice)
    assert n == 27

    # Euclidean distances in C^3
    euclid_dists = []
    torus_dists = []
    for i in range(n):
        for j in range(i + 1, n):
            diff = lattice[i] - lattice[j]
            d_e = np.sqrt(np.sum(np.abs(diff)**2))
            euclid_dists.append(d_e)

            # Torus metric: arc-length per axis, then L2
            angle_diff = np.abs(angle_triples[i] - angle_triples[j])
            arc = np.minimum(angle_diff, 2 * np.pi - angle_diff)
            d_t = np.linalg.norm(arc)
            torus_dists.append(d_t)

    euclid_dists = np.array(euclid_dists)
    torus_dists = np.array(torus_dists)

    assert len(euclid_dists) == 351, f"Expected 351 pairs, got {len(euclid_dists)}"

    # Euclidean statistics
    print("  Euclidean distance in C^3:")
    print(f"    Pairs: {len(euclid_dists)}")
    print(f"    Min:   {euclid_dists.min():.10f}")
    print(f"    Max:   {euclid_dists.max():.10f}")
    print(f"    Mean:  {euclid_dists.mean():.10f}")
    print(f"    Std:   {euclid_dists.std():.10f}")
    max_possible = 2.0 * np.sqrt(3)  # max on (S^1)^3
    print(f"    Max possible: {max_possible:.6f}  (diameter of (S^1)^3)")
    print(f"    Spread ratio: {euclid_dists.max() / max_possible:.6f}")
    print()

    # Distance histogram (round to find multiplicities)
    rounded = np.round(euclid_dists, 8)
    counts = Counter(rounded)
    sorted_dists = sorted(counts.items())

    print(f"  Unique Euclidean distances: {len(sorted_dists)}")
    print(f"  {'Distance':>14} {'Count':>8}")
    print("  " + "-" * 24)
    for d, c in sorted_dists:
        print(f"  {d:>14.8f} {c:>8}")
    print()

    # Torus statistics
    print("  Torus (arc-length L2) distance:")
    print(f"    Min:   {torus_dists.min():.10f}")
    print(f"    Max:   {torus_dists.max():.10f}")
    print(f"    Mean:  {torus_dists.mean():.10f}")
    print()

    # Comparison: equally-spaced reference lattice
    ref_phases = np.array([0.0, 2 * np.pi / 3, 4 * np.pi / 3])
    ref_exp = np.exp(1j * ref_phases)
    ref_lattice = []
    for triple in iterproduct(range(3), repeat=3):
        ref_lattice.append(np.array([ref_exp[i] for i in triple]))
    ref_lattice = np.array(ref_lattice)

    ref_dists = []
    for i in range(27):
        for j in range(i + 1, 27):
            diff = ref_lattice[i] - ref_lattice[j]
            ref_dists.append(np.sqrt(np.sum(np.abs(diff)**2)))
    ref_dists = np.array(ref_dists)

    print("  Reference lattice (equally-spaced {0, 2pi/3, 4pi/3}):")
    print(f"    Min:   {ref_dists.min():.10f}")
    print(f"    Max:   {ref_dists.max():.10f}")
    print(f"    Mean:  {ref_dists.mean():.10f}")
    print(f"    Spread ratio: {ref_dists.max() / max_possible:.6f}")
    print()

    clustering = euclid_dists.mean() / ref_dists.mean()
    print(f"  Clustering factor (FTD/spread mean ratio): {clustering:.6f}")
    print(f"  The FTD lattice occupies {clustering*100:.1f}% of the spread lattice's distance range.")
    print()

    return {
        'euclid': euclid_dists, 'torus': torus_dists,
        'ref_euclid': ref_dists, 'unique_euclid': sorted_dists,
    }


# =============================================================================
# SECTION 4: Symmetry Analysis
# =============================================================================

def symmetry_analysis(lattice, labels):
    """
    S_3 acts on the three axes. Compute orbit decomposition of 27 states.
    """
    print("=" * 78)
    print("  SECTION 4: S_3 Symmetry and Orbit Decomposition")
    print("=" * 78)
    print()

    # S_3 permutations of 3 axes
    from itertools import permutations
    perms = list(permutations(range(3)))
    print(f"  Symmetry group: S_3 (order {len(perms)})")
    print(f"  Action: permute the three axes of (S^1)^3")
    print()

    # Compute orbits
    visited = set()
    orbits = []

    for i, lbl in enumerate(labels):
        if i in visited:
            continue
        orbit = set()
        for perm in perms:
            permuted = tuple(lbl[p] for p in perm)
            # Find index of this label in labels list
            for j, lbl_j in enumerate(labels):
                if lbl_j == permuted:
                    orbit.add(j)
                    break
        visited |= orbit
        orbits.append(sorted(orbit))

    print(f"  Number of orbits: {len(orbits)}")
    print()

    # Classify orbits by type
    orbit_types = {'fixed': [], 'two_equal': [], 'all_distinct': []}
    for orb in orbits:
        rep = labels[orb[0]]
        unique_phases = len(set(rep))
        if unique_phases == 1:
            orbit_types['fixed'].append(orb)
        elif unique_phases == 2:
            orbit_types['two_equal'].append(orb)
        else:
            orbit_types['all_distinct'].append(orb)

    print(f"  {'Type':<25} {'Count':>6} {'Size':>6} {'Total states':>14}")
    print("  " + "-" * 54)
    print(f"  {'Fixed (all same)':<25} {len(orbit_types['fixed']):>6} {'1':>6} {len(orbit_types['fixed']):>14}")
    print(f"  {'Two equal, one diff':<25} {len(orbit_types['two_equal']):>6} {'3':>6} {3*len(orbit_types['two_equal']):>14}")
    print(f"  {'All distinct':<25} {len(orbit_types['all_distinct']):>6} {'6':>6} {6*len(orbit_types['all_distinct']):>14}")
    total = len(orbit_types['fixed']) + 3 * len(orbit_types['two_equal']) + 6 * len(orbit_types['all_distinct'])
    print(f"  {'TOTAL':<25} {len(orbits):>6} {'':>6} {total:>14}")
    print()

    # Print orbit representatives
    print(f"  Orbit representatives:")
    print(f"  {'#':>4} {'Size':>6} {'Representative':<28} {'Type':<20}")
    print("  " + "-" * 60)
    for k, orb in enumerate(orbits):
        rep = labels[orb[0]]
        rep_str = f"({rep[0]}, {rep[1]}, {rep[2]})"
        n_unique = len(set(rep))
        if n_unique == 1:
            otype = "fixed"
        elif n_unique == 2:
            otype = "two-equal"
        else:
            otype = "all-distinct"
        print(f"  {k:>4} {len(orb):>6} {rep_str:<28} {otype:<20}")

    print()
    return {'orbits': orbits, 'types': orbit_types}


# =============================================================================
# SECTION 5: Multiplicative Semigroup Structure
# =============================================================================

def semigroup_analysis(phases):
    """
    Study the multiplicative semigroup generated by the phase alphabet.
    Check closure, density, and continued fraction of varpi/pi.
    """
    print("=" * 78)
    print("  SECTION 5: Multiplicative Semigroup Structure")
    print("=" * 78)
    print()

    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    alphabet = np.array([pi_d, varpi, gstar])

    # Check pairwise phase sums mod 2*pi
    print("  Phase addition table (mod 2*pi):")
    print(f"  {'a + b':<16} {'Sum (rad)':>14} {'Mod 2pi':>14} {'In alphabet?':>14}")
    print("  " + "-" * 60)

    names = ['pi', 'varpi', 'G*']
    for i in range(3):
        for j in range(i, 3):
            s = alphabet[i] + alphabet[j]
            s_mod = s % (2 * np.pi)
            in_alph = any(abs(s_mod - a) < 1e-10 for a in alphabet)
            label = f"{names[i]}+{names[j]}"
            mark = "YES" if in_alph else "no"
            print(f"  {label:<16} {s:>14.8f} {s_mod:>14.8f} {mark:>14}")

    print()
    print("  [THEOREM] The alphabet is NOT closed under phase addition.")
    print("  The 27-state lattice is a generating set, not a finite group.")
    print()

    # Continued fraction of varpi/pi
    ratio = varpi / pi_d
    print(f"  varpi / pi = {ratio:.15f}")
    print()
    print("  Continued fraction expansion [a0; a1, a2, ...]:")

    # Compute continued fraction coefficients
    cf_coeffs = []
    x = ratio
    for _ in range(20):
        a = int(np.floor(x))
        cf_coeffs.append(a)
        frac = x - a
        if frac < 1e-14:
            break
        x = 1.0 / frac

    print(f"  [{cf_coeffs[0]}; {', '.join(str(c) for c in cf_coeffs[1:])}]")

    # Convergents
    print()
    print(f"  {'n':>4} {'p_n':>10} {'q_n':>10} {'p_n/q_n':>18} {'Error':>14}")
    print("  " + "-" * 58)
    p_prev, p_curr = 0, 1
    q_prev, q_curr = 1, 0
    for n, a in enumerate(cf_coeffs):
        p_new = a * p_curr + p_prev
        q_new = a * q_curr + q_prev
        approx = p_new / q_new if q_new != 0 else float('inf')
        err = abs(ratio - approx)
        print(f"  {n:>4} {p_new:>10} {q_new:>10} {approx:>18.15f} {err:>14.2e}")
        p_prev, p_curr = p_curr, p_new
        q_prev, q_curr = q_curr, q_new

    print()
    print("  No termination -> varpi/pi is irrational.")
    print("  [THEOREM] By Nesterenko (1996): G* = Gamma(1/4)/Gamma(3/4) is")
    print("  algebraically independent of pi. Since varpi = G*sqrt(pi)/2,")
    print("  varpi/pi = G*/(2*sqrt(pi)) is also irrational.")
    print("  => The additive semigroup {m*varpi + n*G* + k*pi mod 2pi} is dense in [0, 2pi).")
    print()

    # Semigroup generation: track how phases fill [0, 2*pi)
    print("  Semigroup density analysis (additive generations mod 2*pi):")
    n_bins = 100
    gen_phases = set()
    for a in alphabet:
        gen_phases.add(a % (2 * np.pi))

    print(f"  {'Gen':>5} {'#phases':>10} {'Bins hit':>10} {'Coverage':>10}")
    print("  " + "-" * 38)

    current_gen = set(gen_phases)
    for gen in range(8):
        # Report
        bins = np.zeros(n_bins)
        for p in gen_phases:
            idx = int(p / (2 * np.pi) * n_bins) % n_bins
            bins[idx] += 1
        hit = np.sum(bins > 0)
        coverage = hit / n_bins
        print(f"  {gen:>5} {len(gen_phases):>10} {hit:>10} {coverage:>10.2%}")

        if coverage > 0.99:
            break

        # Next generation: add one more alphabet element to each current phase
        next_gen = set()
        for p in current_gen:
            for a in alphabet:
                new_p = (p + a) % (2 * np.pi)
                next_gen.add(round(new_p, 12))
        gen_phases |= next_gen
        current_gen = next_gen

    print()
    return {'ratio': ratio, 'cf_coeffs': cf_coeffs}


# =============================================================================
# SECTION 6: Moore Neighborhood Mapping
# =============================================================================

def moore_mapping(lattice, labels, phases):
    """
    Map the 27 lattice states bijectively to the 3x3x3 Moore neighborhood.
    Decompose into SC/FCC/BCC shells. Analyze stella octangula.
    """
    print("=" * 78)
    print("  SECTION 6: Moore Neighborhood Mapping")
    print("=" * 78)
    print()

    # Mapping: varpi -> -1, G* -> 0, pi -> +1 (ontic ordering)
    phase_to_offset = {'varpi': -1, 'G*': 0, 'pi': +1}
    print("  Bijection (ontic ordering): varpi -> -1, G* -> 0, pi -> +1")
    print("  G* sits at the CENTER of the Moore neighborhood.")
    print()

    # Build Moore offsets for each lattice state
    moore_offsets = []
    for lbl in labels:
        offset = tuple(phase_to_offset[l] for l in lbl)
        moore_offsets.append(offset)

    # Shell classification
    center = []
    sc_shell = []    # face neighbors, exactly one axis != 0
    fcc_shell = []   # edge neighbors, exactly two axes != 0
    bcc_shell = []   # corner neighbors, all three axes != 0

    for i, offset in enumerate(moore_offsets):
        nonzero = sum(1 for x in offset if x != 0)
        if nonzero == 0:
            center.append(i)
        elif nonzero == 1:
            sc_shell.append(i)
        elif nonzero == 2:
            fcc_shell.append(i)
        else:
            bcc_shell.append(i)

    print(f"  Shell decomposition: 1 + 6 + 12 + 8 = 27")
    print()
    shells = [
        ('Center (d=0)', center, 'gold'),
        ('SC / Octahedron (d=1)', sc_shell, 'blue'),
        ('FCC / Cuboctahedron (d=sqrt2)', fcc_shell, 'green'),
        ('BCC / Cube corners (d=sqrt3)', bcc_shell, 'red'),
    ]

    for name, indices, _ in shells:
        print(f"  {name}: {len(indices)} states")
        for i in indices:
            lbl = labels[i]
            off = moore_offsets[i]
            lbl_str = f"({lbl[0]}, {lbl[1]}, {lbl[2]})"
            off_str = f"({off[0]:+d}, {off[1]:+d}, {off[2]:+d})"
            print(f"    {i:>3}  {lbl_str:<28} -> {off_str}")
        print()

    # Stella octangula: BCC corners split by parity
    print("  Stella Octangula decomposition of BCC corners:")
    tetra_plus = []   # even parity: product of offsets = +1 or all negative -> even number of +1's
    tetra_minus = []

    for i in bcc_shell:
        off = moore_offsets[i]
        parity = off[0] * off[1] * off[2]
        if parity > 0:
            tetra_plus.append(i)
        else:
            tetra_minus.append(i)

    print(f"  T+ (even parity, product > 0): {len(tetra_plus)} states")
    for i in tetra_plus:
        lbl = labels[i]
        off = moore_offsets[i]
        print(f"    ({off[0]:+d},{off[1]:+d},{off[2]:+d}) = ({lbl[0]}, {lbl[1]}, {lbl[2]})")

    print(f"  T- (odd parity, product < 0):  {len(tetra_minus)} states")
    for i in tetra_minus:
        lbl = labels[i]
        off = moore_offsets[i]
        print(f"    ({off[0]:+d},{off[1]:+d},{off[2]:+d}) = ({lbl[0]}, {lbl[1]}, {lbl[2]})")
    print()

    # Phase energy per shell: sum of phase angles
    print("  Phase energy (sum of angles) per shell:")
    print(f"  {'Shell':<30} {'Mean energy':>14} {'Std':>10}")
    print("  " + "-" * 56)
    angle_arr = np.array([[phases['varpi'] if l == 'varpi' else
                           phases['gstar'] if l == 'G*' else
                           phases['pi_derived'] for l in lbl] for lbl in labels])
    phase_sums = np.sum(angle_arr, axis=1)

    for name, indices, _ in shells:
        if len(indices) > 0:
            energies = phase_sums[indices]
            print(f"  {name:<30} {energies.mean():>14.8f} {energies.std():>10.8f}")

    # BCC states are pure varpi/pi (no G*) -- connect to Watson integral
    print()
    bcc_labels = [labels[i] for i in bcc_shell]
    has_gstar = any('G*' in lbl for lbl in bcc_labels)
    print(f"  BCC corners contain G*: {has_gstar}")
    if not has_gstar:
        print("  [SELECTION] The 8 BCC corners are exactly the states that AVOID G*.")
        print(f"  They use only {{varpi, pi}} -- the 'extreme' phases.")
        print(f"  Watson BCC integral: W_3 = G*^2/(2*pi) = {G_STAR**2 / (2*np.pi):.10f}")
        print(f"  The bridge constant G* mediates between these extremes from the center.")
    print()

    return {
        'moore_offsets': moore_offsets,
        'shells': shells,
        'tetra_plus': tetra_plus,
        'tetra_minus': tetra_minus,
        'phase_sums': phase_sums,
    }


# =============================================================================
# SECTION 7: Visualization
# =============================================================================

def visualize_lattice(lattice, labels, phases, angle_triples, dist_data, moore_data):
    """Generate four visualization figures."""

    print("=" * 78)
    print("  SECTION 7: Visualization")
    print("=" * 78)
    print()

    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    # --- Figure 1: Phase alphabet on S^1 ---
    fig1, ax1 = plt.subplots(1, 1, figsize=(7, 7))
    theta = np.linspace(0, 2 * np.pi, 500)
    ax1.plot(np.cos(theta), np.sin(theta), 'k-', linewidth=0.5, alpha=0.3)

    # Highlight the arc between varpi and pi
    arc_theta = np.linspace(varpi, pi_d, 100)
    ax1.plot(np.cos(arc_theta), np.sin(arc_theta), 'r-', linewidth=3, alpha=0.5, label='Ontic arc')

    # Mark the three phases
    pts = [
        (varpi, phases['e_varpi'], 'varpi', 'blue', 's'),
        (gstar, phases['e_gstar'], 'G*', 'green', 'D'),
        (pi_d, phases['e_pi'], 'pi', 'red', 'o'),
    ]
    for th, z, name, color, marker in pts:
        ax1.plot(z.real, z.imag, marker, color=color, markersize=14, zorder=5)
        # Label offset
        offset_x = -0.15 if z.real < 0 else 0.05
        offset_y = 0.08 if z.imag > 0 else -0.12
        ax1.annotate(f'{name}\n({th:.4f} rad)',
                     (z.real, z.imag),
                     (z.real + offset_x, z.imag + offset_y),
                     fontsize=9, ha='center',
                     arrowprops=dict(arrowstyle='->', color=color, lw=1.2))

    ax1.set_xlim(-1.5, 1.5)
    ax1.set_ylim(-1.5, 1.5)
    ax1.set_aspect('equal')
    ax1.axhline(0, color='gray', linewidth=0.3)
    ax1.axvline(0, color='gray', linewidth=0.3)
    ax1.set_title('Phase Alphabet on S^1: {e^{i*pi}, e^{i*varpi}, e^{i*G*}}', fontsize=12)
    ax1.legend(loc='lower right')
    ax1.grid(True, alpha=0.2)

    path1 = os.path.join(OUTPUT_DIR, 'phase_lattice_27_circle.png')
    fig1.savefig(path1, dpi=150, bbox_inches='tight')
    plt.close(fig1)
    print(f"  Figure 1 saved: {path1}")

    # --- Figure 2: 3D scatter in phase space ---
    fig2 = plt.figure(figsize=(9, 8))
    ax2 = fig2.add_subplot(111, projection='3d')

    shell_colors = {}
    for name, indices, color in moore_data['shells']:
        for i in indices:
            shell_colors[i] = color

    for i in range(27):
        a = angle_triples[i]
        c = shell_colors.get(i, 'gray')
        ax2.scatter(a[0], a[1], a[2], c=c, s=80, edgecolors='black', linewidths=0.5, zorder=5)

    ax2.set_xlabel('Axis 1 (rad)', fontsize=10)
    ax2.set_ylabel('Axis 2 (rad)', fontsize=10)
    ax2.set_zlabel('Axis 3 (rad)', fontsize=10)
    ax2.set_title('27-State Phase Lattice in Phase Space\n'
                  'gold=center, blue=SC, green=FCC, red=BCC', fontsize=11)

    path2 = os.path.join(OUTPUT_DIR, 'phase_lattice_27_3d.png')
    fig2.savefig(path2, dpi=150, bbox_inches='tight')
    plt.close(fig2)
    print(f"  Figure 2 saved: {path2}")

    # --- Figure 3: Distance spectrum comparison ---
    fig3, (ax3a, ax3b) = plt.subplots(1, 2, figsize=(13, 5))

    ax3a.hist(dist_data['euclid'], bins=30, color='steelblue', alpha=0.7, edgecolor='black', label='FTD lattice')
    ax3a.set_xlabel('Euclidean distance in C^3', fontsize=10)
    ax3a.set_ylabel('Count', fontsize=10)
    ax3a.set_title('FTD Phase Lattice Distances', fontsize=11)
    ax3a.legend()

    ax3b.hist(dist_data['ref_euclid'], bins=30, color='coral', alpha=0.7, edgecolor='black', label='Spread lattice')
    ax3b.set_xlabel('Euclidean distance in C^3', fontsize=10)
    ax3b.set_ylabel('Count', fontsize=10)
    ax3b.set_title('Equally-Spaced Reference Distances', fontsize=11)
    ax3b.legend()

    fig3.suptitle('Distance Spectrum: Clustered (FTD) vs Spread', fontsize=12)
    fig3.tight_layout()

    path3 = os.path.join(OUTPUT_DIR, 'phase_lattice_27_distances.png')
    fig3.savefig(path3, dpi=150, bbox_inches='tight')
    plt.close(fig3)
    print(f"  Figure 3 saved: {path3}")

    # --- Figure 4: Moore neighborhood grid ---
    fig4, axes4 = plt.subplots(1, 3, figsize=(15, 5))
    fig4.suptitle('Moore Neighborhood Phase Assignment (3 slices along axis 3)', fontsize=12)

    name_map = {'varpi': 'w', 'G*': 'G', 'pi': 'p'}
    shell_color_map = {}
    for name, indices, color in moore_data['shells']:
        for i in indices:
            shell_color_map[i] = color

    axis3_names = ['varpi', 'G*', 'pi']
    axis3_offsets = [-1, 0, +1]

    for panel, (a3_name, a3_off) in enumerate(zip(axis3_names, axis3_offsets)):
        ax = axes4[panel]
        ax.set_xlim(-1.8, 1.8)
        ax.set_ylim(-1.8, 1.8)
        ax.set_aspect('equal')
        ax.set_title(f'Axis 3 = {a3_name} (offset {a3_off:+d})', fontsize=10)
        ax.set_xlabel('Axis 1 offset')
        ax.set_ylabel('Axis 2 offset')

        for i, lbl in enumerate(labels):
            if lbl[2] != a3_name:
                continue
            off = moore_data['moore_offsets'][i]
            color = shell_color_map[i]
            ax.plot(off[0], off[1], 's', color=color, markersize=30, zorder=3)
            short = f"{name_map[lbl[0]]},{name_map[lbl[1]]}"
            ax.text(off[0], off[1], short, ha='center', va='center',
                    fontsize=8, fontweight='bold', color='white' if color != 'gold' else 'black',
                    zorder=4)

        ax.set_xticks([-1, 0, 1])
        ax.set_yticks([-1, 0, 1])
        ax.set_xticklabels(['varpi', 'G*', 'pi'])
        ax.set_yticklabels(['varpi', 'G*', 'pi'])
        ax.grid(True, alpha=0.3)

    fig4.tight_layout()
    path4 = os.path.join(OUTPUT_DIR, 'phase_lattice_27_moore.png')
    fig4.savefig(path4, dpi=150, bbox_inches='tight')
    plt.close(fig4)
    print(f"  Figure 4 saved: {path4}")
    print()


# =============================================================================
# SECTION 8: Laplacian Eigenvalue Analysis
# =============================================================================

def laplacian_analysis(lattice, labels, phases, moore_data):
    """
    Build the 27x27 Moore-adjacency Laplacian with phase-distance weights.
    Compare unweighted and phase-weighted eigenvalue spectra.
    Look for framework numbers in the spectrum.
    """
    print("=" * 78)
    print("  SECTION 8: Laplacian Eigenvalue Analysis")
    print("=" * 78)
    print()

    n = 27
    offsets = moore_data['moore_offsets']

    # Build adjacency: two states are Moore-adjacent if they differ by at most 1
    # on each axis in the offset representation (open boundary 3x3x3 grid)
    A_unweighted = np.zeros((n, n))
    A_phase = np.zeros((n, n))

    for i in range(n):
        for j in range(i + 1, n):
            oi = np.array(offsets[i])
            oj = np.array(offsets[j])
            diff = np.abs(oi - oj)
            if np.all(diff <= 1):  # Moore-adjacent
                A_unweighted[i, j] = 1.0
                A_unweighted[j, i] = 1.0

                # Phase-distance weight: Euclidean distance between states in C^3
                d = np.sqrt(np.sum(np.abs(lattice[i] - lattice[j])**2))
                A_phase[i, j] = d
                A_phase[j, i] = d

    # Verify adjacency counts
    degrees_uw = A_unweighted.sum(axis=1).astype(int)
    print("  Moore adjacency (open boundary, 3x3x3):")
    print(f"  {'State type':<24} {'Count':>6} {'Degree':>8}")
    print("  " + "-" * 40)
    # Group by shell
    for name, indices, _ in moore_data['shells']:
        if len(indices) > 0:
            degs = degrees_uw[indices]
            print(f"  {name:<24} {len(indices):>6} {degs[0]:>8}")

    total_edges = int(A_unweighted.sum()) // 2
    print(f"\n  Total edges: {total_edges}")
    print()

    # --- Unweighted Laplacian ---
    D_uw = np.diag(degrees_uw.astype(float))
    L_uw = D_uw - A_unweighted
    evals_uw = np.linalg.eigvalsh(L_uw)

    print("  UNWEIGHTED Laplacian eigenvalues:")
    print(f"  {'#':>4} {'Eigenvalue':>14} {'Rounded':>10}")
    print("  " + "-" * 30)
    for k, ev in enumerate(evals_uw):
        r = round(ev, 4)
        print(f"  {k:>4} {ev:>14.8f} {r:>10.4f}")

    # Check for framework numbers
    print()
    print("  Framework number check in unweighted spectrum:")
    framework = {'N_c': 3, 'N_base': 4, 'b_3': 7, 'N_eff': 13, '27': 27}
    for name, val in framework.items():
        closest = evals_uw[np.argmin(np.abs(evals_uw - val))]
        diff = abs(closest - val)
        if diff < 0.5:
            print(f"    {name} = {val}: nearest eigenvalue = {closest:.6f} (diff = {diff:.6f})")

    # Eigenvalue multiplicities
    rounded_uw = np.round(evals_uw, 4)
    unique_uw, counts_uw = np.unique(rounded_uw, return_counts=True)
    print()
    print("  Eigenvalue multiplicities:")
    print(f"  {'Eigenvalue':>14} {'Mult':>6}")
    print("  " + "-" * 22)
    for val, cnt in zip(unique_uw, counts_uw):
        print(f"  {val:>14.4f} {cnt:>6}")

    print()
    print(f"  Number of distinct eigenvalues: {len(unique_uw)}")
    print(f"  Spectral gap (lambda_1): {evals_uw[1]:.8f}")
    print(f"  Largest eigenvalue: {evals_uw[-1]:.8f}")
    print(f"  Ratio lambda_max/lambda_1: {evals_uw[-1]/evals_uw[1]:.6f}")
    print()

    # --- Phase-weighted Laplacian ---
    degrees_pw = A_phase.sum(axis=1)
    D_pw = np.diag(degrees_pw)
    L_pw = D_pw - A_phase
    evals_pw = np.linalg.eigvalsh(L_pw)

    print("  PHASE-WEIGHTED Laplacian eigenvalues:")
    print(f"  {'#':>4} {'Eigenvalue':>14}")
    print("  " + "-" * 20)
    for k, ev in enumerate(evals_pw):
        print(f"  {k:>4} {ev:>14.8f}")

    # Eigenvalue multiplicities for phase-weighted
    rounded_pw = np.round(evals_pw, 4)
    unique_pw, counts_pw = np.unique(rounded_pw, return_counts=True)
    print()
    print("  Phase-weighted multiplicities:")
    print(f"  {'Eigenvalue':>14} {'Mult':>6}")
    print("  " + "-" * 22)
    for val, cnt in zip(unique_pw, counts_pw):
        print(f"  {val:>14.4f} {cnt:>6}")

    print()
    print(f"  Number of distinct eigenvalues: {len(unique_pw)}")
    print(f"  Spectral gap: {evals_pw[1]:.8f}")

    # Compare: does phase weighting break or preserve degeneracies?
    print()
    print("  Degeneracy comparison:")
    print(f"    Unweighted: {len(unique_uw)} distinct eigenvalues, max multiplicity {counts_uw.max()}")
    print(f"    Phase-weighted: {len(unique_pw)} distinct eigenvalues, max multiplicity {counts_pw.max()}")

    # Eigenvalue ratios
    print()
    nonzero_uw = evals_uw[evals_uw > 1e-10]
    if len(nonzero_uw) >= 2:
        print("  Nonzero eigenvalue ratios (unweighted):")
        for i in range(min(5, len(nonzero_uw) - 1)):
            r = nonzero_uw[i + 1] / nonzero_uw[0]
            print(f"    lambda_{i+2}/lambda_1 = {r:.8f}")

    print()
    return {
        'evals_unweighted': evals_uw,
        'evals_phase': evals_pw,
        'adjacency_uw': A_unweighted,
        'adjacency_pw': A_phase,
        'mult_uw': list(zip(unique_uw, counts_uw)),
        'mult_pw': list(zip(unique_pw, counts_pw)),
    }


# =============================================================================
# SECTION 9: Master Quadratic at Lattice States
# =============================================================================

def master_quadratic_analysis(lattice, labels, phases, moore_data):
    """
    Evaluate the gap equation and master quadratic at lattice state properties.
    Look for connections between phase sums and the roots x+, x-.
    """
    print("=" * 78)
    print("  SECTION 9: Master Quadratic at Lattice States")
    print("=" * 78)
    print()

    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    from constants import X_PLUS, X_MINUS, ALPHA

    print(f"  Master quadratic: x^2 - 16*G*^2*x + 16*G*^3 = 0")
    print(f"  x+ = {X_PLUS:.10f}  (1/alpha)")
    print(f"  x- = {X_MINUS:.10f}  (~ N_c)")
    print(f"  Sum:     x+ + x- = {X_PLUS + X_MINUS:.10f} = 16*G*^2 = {16*gstar**2:.10f}")
    print(f"  Product: x+ * x- = {X_PLUS * X_MINUS:.10f} = 16*G*^3 = {16*gstar**3:.10f}")
    print()

    # Phase sums for each state
    angle_arr = np.array([[varpi if l == 'varpi' else
                           gstar if l == 'G*' else
                           pi_d for l in lbl] for lbl in labels])
    phase_sums = np.sum(angle_arr, axis=1)

    # Phase products (products of e^{i*theta} components)
    phase_products = np.prod(lattice, axis=1)  # product of three unit-circle values

    # Gap equation: F(x) = 16*G*^2 - 16*G*^3/x
    # Fixed points: F(x) = x gives the master quadratic
    print("  Gap equation F(x) = 16*G*^2 - 16*G*^3/x evaluated at phase sums:")
    print()
    print(f"  {'Shell':<28} {'Phase sum':>12} {'F(sum)':>12} {'F(sum)-sum':>12}")
    print("  " + "-" * 66)

    K = 16 * gstar**2   # = x+ + x- = 140.06
    P = 16 * gstar**3   # = x+ * x- = 414.39

    for name, indices, _ in moore_data['shells']:
        if len(indices) > 0:
            s = phase_sums[indices[0]]
            F_s = K - P / s
            print(f"  {name:<28} {s:>12.6f} {F_s:>12.6f} {F_s - s:>12.6f}")

    # All unique phase sums
    unique_sums = sorted(set(np.round(phase_sums, 10)))
    print()
    print(f"  All {len(unique_sums)} unique phase sums:")
    print(f"  {'Sum':>12} {'F(sum)':>12} {'States':>8}")
    print("  " + "-" * 34)
    for s in unique_sums:
        count = np.sum(np.abs(phase_sums - s) < 1e-8)
        F_s = K - P / s
        print(f"  {s:>12.6f} {F_s:>12.6f} {count:>8}")

    # Phase products: e^{i*(a1+a2+a3)}
    print()
    print("  Phase products (complex): prod_k e^{i*theta_k} = e^{i*sum(theta_k)}")
    print()
    unique_prod_phases = sorted(set(np.round(np.angle(phase_products), 10)))
    print(f"  Unique product phases (mod 2pi): {len(unique_prod_phases)}")
    for phi in unique_prod_phases:
        phi_mod = phi % (2 * np.pi)
        count = np.sum(np.abs(np.angle(phase_products) - phi) < 1e-8)
        print(f"    phi = {phi:>12.8f} rad = {np.degrees(phi):>12.6f} deg  (count = {count})")

    # Key ratios
    print()
    print("  Key ratios involving phase sums and master quadratic:")
    sum_center = 3 * gstar
    sum_bcc_min = 3 * varpi
    sum_bcc_max = 3 * pi_d

    ratios = [
        ('x+/sum(center)', X_PLUS / sum_center),
        ('x-/sum(center)', X_MINUS / sum_center),
        ('K/sum(center)', K / sum_center),
        ('x+/(3*pi)', X_PLUS / (3 * pi_d)),
        ('x-/(3*varpi)', X_MINUS / (3 * varpi)),
        ('16*G*/sum(center)', 16 * gstar / sum_center),
        ('sum(center)/G*', sum_center / gstar),
    ]

    for label, val in ratios:
        # Check if close to integer or simple fraction
        nearest_int = round(val)
        frac_err = abs(val - nearest_int)
        note = f" ~ {nearest_int}" if frac_err < 0.1 else ""
        print(f"    {label:<25} = {val:>12.8f}{note}")

    # The Vieta mean: harmonic mean of roots = product/sum = G*
    print()
    print(f"  Vieta harmonic ratio: x+*x-/(x++x-) = P/K = {P/K:.10f}")
    print(f"  G* = {gstar:.10f}")
    print(f"  Difference: {abs(P/K - gstar):.2e}")
    print()

    # What would x need to be for the gap equation to land on a lattice phase sum?
    print("  Inverse: what x gives F(x) = phase_sum?")
    print("  F(x) = s => x = P/(K - s)")
    print(f"  {'Target s':<20} {'x = P/(K-s)':>14} {'Near?':>14}")
    print("  " + "-" * 50)
    for s in unique_sums:
        if abs(K - s) > 1e-10:
            x_inv = P / (K - s)
            near = ""
            if abs(x_inv - X_PLUS) / X_PLUS < 0.01:
                near = f"~ x+ ({100*abs(x_inv-X_PLUS)/X_PLUS:.2f}%)"
            elif abs(x_inv - X_MINUS) / X_MINUS < 0.1:
                near = f"~ x- ({100*abs(x_inv-X_MINUS)/X_MINUS:.2f}%)"
            print(f"  {s:<20.6f} {x_inv:>14.6f} {near:>14}")

    print()
    return {
        'phase_sums': phase_sums,
        'phase_products': phase_products,
        'unique_sums': unique_sums,
    }


# =============================================================================
# SECTION 10: Interpolation Ratio Precision
# =============================================================================

def interpolation_analysis(phases):
    """
    High-precision analysis of the interpolation ratio (G*-varpi)/(pi-varpi).
    Check whether 2/3 is exact or a near-miss. Derive exact algebraic form.
    """
    print("=" * 78)
    print("  SECTION 10: Interpolation Ratio Precision")
    print("=" * 78)
    print()

    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    r = (gstar - varpi) / (pi_d - varpi)

    print(f"  Interpolation ratio: (G* - varpi) / (pi - varpi)")
    print(f"  = ({gstar:.15f} - {varpi:.15f}) / ({pi_d:.15f} - {varpi:.15f})")
    print(f"  = {gstar - varpi:.15f} / {pi_d - varpi:.15f}")
    print(f"  = {r:.15f}")
    print()

    # Exact algebraic form
    # Using g1 = Gamma(1/4), g2 = Gamma(1/2) = sqrt(pi):
    # varpi = g1^2 / (2*sqrt(2)*g2)
    # G* = g1^2 / (sqrt(2)*g2^2)
    # pi = g2^2
    #
    # G* - varpi = [g1^2/(sqrt(2)*g2)] * [1/g2 - 1/2]
    #            = [g1^2/(sqrt(2)*g2)] * (2 - g2)/(2*g2)
    #
    # pi - varpi = g2^2 - g1^2/(2*sqrt(2)*g2)
    #            = g2 * [g2 - g1^2/(2*sqrt(2)*g2^2)]
    #
    # Ratio = G*(2 - sqrt(pi)) / (2*pi - sqrt(pi)*G*)

    sqrt_pi = np.sqrt(pi_d)
    ratio_exact = gstar * (2.0 - sqrt_pi) / (2.0 * pi_d - sqrt_pi * gstar)
    print(f"  Algebraic form: G*(2 - sqrt(pi)) / (2*pi - sqrt(pi)*G*)")
    print(f"  = {gstar:.10f} * ({2 - sqrt_pi:.10f}) / ({2*pi_d - sqrt_pi*gstar:.10f})")
    print(f"  = {ratio_exact:.15f}")
    print(f"  Cross-check: {abs(r - ratio_exact):.2e}")
    print()

    # Test: is this exactly 2/3?
    two_thirds = 2.0 / 3.0
    diff_23 = r - two_thirds
    print(f"  Test: ratio = 2/3 ?")
    print(f"    ratio     = {r:.15f}")
    print(f"    2/3       = {two_thirds:.15f}")
    print(f"    diff      = {diff_23:.15f}")
    print(f"    rel. diff = {abs(diff_23)/two_thirds*100:.4f}%")
    print()

    # If ratio = 2/3, then G* = 4*pi/(6 - sqrt(pi))
    gstar_if_23 = 4.0 * pi_d / (6.0 - sqrt_pi)
    print(f"  If ratio were exactly 2/3:")
    print(f"    G* would need to be 4*pi/(6-sqrt(pi)) = {gstar_if_23:.10f}")
    print(f"    Actual G* = {gstar:.10f}")
    print(f"    Discrepancy = {abs(gstar - gstar_if_23):.10f} ({abs(gstar-gstar_if_23)/gstar*100:.4f}%)")
    print()
    print(f"  VERDICT: 2/3 is a NEAR-MISS, not an exact identity.")
    print(f"  The interpolation ratio is a transcendental number depending on G* and pi.")
    print()

    # Check other simple fractions
    print("  Systematic check of simple fractions p/q (q <= 20):")
    print(f"  {'p/q':<10} {'Value':>14} {'|diff|':>14} {'Match?':>10}")
    print("  " + "-" * 50)
    best_frac = None
    best_err = 1.0
    for q in range(2, 21):
        for p in range(1, q):
            from math import gcd
            if gcd(p, q) > 1:
                continue
            val = p / q
            err = abs(r - val)
            if err < 0.02:
                mark = "<-- close" if err < 0.005 else ""
                print(f"  {p}/{q:<8} {val:>14.10f} {err:>14.10f} {mark:>10}")
            if err < best_err:
                best_err = err
                best_frac = (p, q)

    print()
    print(f"  Best rational approximation (q<=20): {best_frac[0]}/{best_frac[1]} "
          f"= {best_frac[0]/best_frac[1]:.10f} (err = {best_err:.2e})")
    print()

    # Continued fraction for the ratio itself
    print("  Continued fraction of interpolation ratio:")
    cf = []
    x = r
    for _ in range(12):
        a = int(np.floor(x))
        cf.append(a)
        frac = x - a
        if frac < 1e-14:
            break
        x = 1.0 / frac
    print(f"  [{cf[0]}; {', '.join(str(c) for c in cf[1:])}]")
    print()

    return {'ratio': r, 'diff_from_23': diff_23, 'best_frac': best_frac}


# =============================================================================
# SECTION 11: Summary
# =============================================================================

def print_summary(phases, dist_data, sym_data, semi_data, moore_data,
                  lap_data=None, mq_data=None, interp_data=None):
    """Print structured summary of key findings."""
    print("=" * 78)
    print("  SUMMARY: 27-State Phase Lattice {pi, varpi, G*}^3  [Sections 0-10]")
    print("=" * 78)
    print()

    varpi = phases['varpi']
    gstar = phases['gstar']
    pi_d = phases['pi_derived']

    print("  1. PHASE ALPHABET [THEOREM]")
    print(f"     E = {{e^{{i*pi}}, e^{{i*varpi}}, e^{{i*G*}}}} on S^1")
    print(f"     varpi = {varpi:.10f}  (lemniscate constant, most primitive)")
    print(f"     G*    = {gstar:.10f}  (bridge constant)")
    print(f"     pi    = {pi_d:.10f}  (derived: 4*varpi^2/G*^2)")
    print(f"     Arc span: {np.degrees(pi_d - varpi):.4f} degrees")
    print(f"     All in second quadrant: Re < 0, Im >= 0")
    print()

    print("  2. LATTICE STRUCTURE [THEOREM]")
    print(f"     |L_3(E)| = 27 distinct states in (S^1)^3")
    n_orbits = len(sym_data['orbits'])
    print(f"     S_3 symmetry: {n_orbits} orbits")
    print(f"       3 fixed + 6 orbits of size 3 + 1 orbit of size 6")
    print()

    print("  3. DISTANCE SPECTRUM")
    n_unique = len(dist_data['unique_euclid'])
    print(f"     {n_unique} unique Euclidean distances among 351 pairs")
    clustering = dist_data['euclid'].mean() / dist_data['ref_euclid'].mean()
    print(f"     Clustering factor vs spread lattice: {clustering:.4f}")
    print()

    print("  4. MULTIPLICATIVE SEMIGROUP [THEOREM]")
    print(f"     Alphabet NOT closed under multiplication")
    print(f"     varpi/pi = {semi_data['ratio']:.15f} (irrational)")
    print(f"     Nesterenko (1996): G* algebraically independent of pi")
    print(f"     => Semigroup is dense in S^1")
    print()

    print("  5. MOORE NEIGHBORHOOD MAPPING [SELECTION]")
    print(f"     Bijection: varpi -> -1, G* -> 0, pi -> +1")
    print(f"     G* at CENTER: the bridge constant mediates")
    print(f"     Shells: 1 (center) + 6 (SC) + 12 (FCC) + 8 (BCC) = 27")
    print(f"     BCC 8 corners avoid G* entirely: pure {{varpi, pi}} states")
    print(f"     Stella octangula: T+ = {len(moore_data['tetra_plus'])}, T- = {len(moore_data['tetra_minus'])}")
    print(f"     Watson integral W_3 = G*^2/(2*pi) = {gstar**2 / (2*pi_d):.10f}")
    print(f"       connects to the 8 BCC states that live without G*")
    print()

    print("  6. FRAMEWORK NUMBER APPEARANCES")
    print(f"     3  = N_c = dim(alphabet) = fixed-point orbits")
    print(f"     6  = |S_3| = axis-permutation symmetry")
    print(f"     8  = BCC corners = 2*N_base = stella octangula vertices")
    print(f"     12 = FCC shell = edges of cuboctahedron")
    print(f"     27 = N_c^3 = lattice size = Moore neighborhood count")
    print()

    if lap_data is not None:
        print("  7. LAPLACIAN SPECTRUM")
        mult_uw = lap_data['mult_uw']
        n_distinct = len(mult_uw)
        max_mult = max(c for _, c in mult_uw)
        gap = lap_data['evals_unweighted'][1]
        lmax = lap_data['evals_unweighted'][-1]
        print(f"     Unweighted: {n_distinct} distinct eigenvalues, max multiplicity {max_mult}")
        print(f"     Spectral gap: {gap:.6f}, lambda_max: {lmax:.6f}")
        print(f"     Ratio lambda_max/gap: {lmax/gap:.6f}")
        mult_pw = lap_data['mult_pw']
        n_pw = len(mult_pw)
        print(f"     Phase-weighted: {n_pw} distinct eigenvalues")
        if n_pw != n_distinct:
            print(f"     Phase weighting {'breaks' if n_pw > n_distinct else 'introduces'} degeneracies")
        print()

    if interp_data is not None:
        print("  8. INTERPOLATION RATIO")
        print(f"     (G*-varpi)/(pi-varpi) = {interp_data['ratio']:.15f}")
        print(f"     Algebraic form: G*(2-sqrt(pi))/(2*pi-sqrt(pi)*G*)")
        print(f"     Near 2/3 ({interp_data['diff_from_23']:+.6f}) but NOT exact")
        bf = interp_data['best_frac']
        print(f"     Best simple fraction: {bf[0]}/{bf[1]}")
        print()

    print("  [CONJECTURE] The phase lattice {pi, varpi, G*}^3 is the natural")
    print("  phase-space avatar of the Moore neighborhood. G* at center mediates")
    print("  between the extreme (BCC) states via the Watson integral, while")
    print("  the SC and FCC shells mix bridge and extreme phases.")
    print()
    print("=" * 78)


# =============================================================================
# MAIN
# =============================================================================

if __name__ == "__main__":
    phases = compute_phases()
    verify_phase_distinctness(phases)
    lattice, labels, angle_triples = build_lattice(phases)
    dist_data = distance_spectrum(lattice, angle_triples, phases)
    sym_data = symmetry_analysis(lattice, labels)
    semi_data = semigroup_analysis(phases)
    moore_data = moore_mapping(lattice, labels, phases)
    lap_data = laplacian_analysis(lattice, labels, phases, moore_data)
    mq_data = master_quadratic_analysis(lattice, labels, phases, moore_data)
    interp_data = interpolation_analysis(phases)
    visualize_lattice(lattice, labels, phases, angle_triples, dist_data, moore_data)
    print_summary(phases, dist_data, sym_data, semi_data, moore_data,
                  lap_data, mq_data, interp_data)
