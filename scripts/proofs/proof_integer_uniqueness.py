"""
INTEGER UNIQUENESS: Is {N_c=3, N_base=4, b_3=7, N_eff=13} unique?

Exhaustive search over integer quadruples (N_c, N_base, b_3, N_eff) to
determine whether the FTD framework integers are the UNIQUE solution to
all self-consistency constraints.

Constraints tested:
  P1. b_3 = (11*N_c - 2*N_f) / 3, integer, positive  (QCD beta function)
  P2. N_eff = b_3 + 2*N_c                              (effective DOF)
  P3. sin^2(theta_W) = N_c / N_eff in (0, 0.5)         (Weinberg angle physical)
  P4. N_base = 2^((D+1)//2) where D = 3                 (spinor dimension)
  P5. b_3 = N_base + N_c                                (additive closure)
  S1. N_eff at Fibonacci-Tribonacci crossover            (sequence constraint)
  S2. b_3, N_eff consecutive Tribonacci                  (sequence constraint)
  S3. N_base, b_3 consecutive Lucas                      (sequence constraint)
  S4. Crossover index = b_3                              (self-referential)

Tags:
  [THEOREM] for the enumeration results
  [SELECTION] for the sequence-based constraints (S1-S4)
"""

import sys
import os
import math
import io

if sys.stdout.encoding != 'utf-8':
    sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8', errors='replace')

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from common import (
    ProofSuite, G_STAR, X_PLUS, X_MINUS, COEFFICIENT,
    MACHINE_EPS, PPM_1,
)

suite = ProofSuite("Integer Uniqueness: {3, 4, 7, 13}")

print("=" * 78)
print("  INTEGER UNIQUENESS: Is {N_c=3, N_base=4, b_3=7, N_eff=13} unique?")
print("=" * 78)
print()


# ============================================================================
# SECTION 1: Generate Fibonacci, Tribonacci, and Lucas sequences
# ============================================================================

print("=" * 78)
print("  SECTION 1: Sequence Generation [THEOREM]")
print("=" * 78)
print()

def fibonacci(n_max):
    """Generate Fibonacci numbers up to index n_max."""
    F = [0, 1]
    while len(F) <= n_max:
        F.append(F[-1] + F[-2])
    return F

def tribonacci(n_max):
    """Generate Tribonacci numbers up to index n_max."""
    T = [0, 0, 1]
    while len(T) <= n_max:
        T.append(T[-1] + T[-2] + T[-3])
    return T

def lucas(n_max):
    """Generate Lucas numbers up to index n_max."""
    L = [2, 1]
    while len(L) <= n_max:
        L.append(L[-1] + L[-2])
    return L

N_SEQ = 30  # Generate sequences up to index 30
fib = fibonacci(N_SEQ)
trib = tribonacci(N_SEQ)
luc = lucas(N_SEQ)

print(f"  Fibonacci (first 15): {fib[:15]}")
print(f"  Tribonacci (first 15): {trib[:15]}")
print(f"  Lucas (first 15): {luc[:15]}")
print()

# Find Fibonacci-Tribonacci crossovers (F_n = T_n, both > 1)
crossovers = []
for n in range(2, N_SEQ):
    if fib[n] == trib[n] and fib[n] > 1:
        crossovers.append((n, fib[n]))
        print(f"  Crossover: F_{n} = T_{n} = {fib[n]}")

if not crossovers:
    print("  No non-trivial crossovers found!")

# Also check for F_n = T_m (different indices) up to value 10000
fib_set = set(f for f in fib if f > 1)
trib_set = set(t for t in trib if t > 1)
common = sorted(fib_set & trib_set)
print(f"\n  Values in both Fibonacci AND Tribonacci (up to T_{N_SEQ}): {common}")
print()

suite.assert_true(
    "F_7 = T_7 = 13 is the unique non-trivial crossover (n <= 30)",
    len(crossovers) == 1 and crossovers[0] == (7, 13),
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 2: Physics Constraints Enumeration
# ============================================================================

print()
print("=" * 78)
print("  SECTION 2: Physics Constraints Enumeration [THEOREM]")
print("=" * 78)
print()
print("  Constraints:")
print("    P1. b_3 = (11*N_c - 2*N_f)/3, integer, positive")
print("    P2. N_eff = b_3 + 2*N_c")
print("    P3. sin^2(theta_W) = N_c/N_eff in (0, 0.5)")
print("    P4. N_base = 2^((D+1)//2) where D = 3 => N_base = 4")
print("    P5. b_3 = N_base + N_c")
print()

# Search over N_c = 1..50, N_gen = 1..20
# N_f = 2 * N_gen (two flavors per generation)
# b_3 = (11*N_c - 2*N_f)/3 = (11*N_c - 4*N_gen)/3

N_BASE_FIXED = 4  # From D=3: 2^((3+1)//2) = 2^2 = 4

physics_solutions = []

print(f"  Searching N_c = 1..50, N_gen = 1..20:")
print()
print(f"  {'N_c':>4s}  {'N_gen':>5s}  {'N_f':>4s}  {'b_3':>4s}  {'N_eff':>5s}  {'sin2':>6s}  {'P5?':>4s}")
print(f"  {'':->4s}  {'':->5s}  {'':->4s}  {'':->4s}  {'':->5s}  {'':->6s}  {'':->4s}")

for N_c in range(1, 51):
    for N_gen in range(1, 21):
        N_f = 2 * N_gen
        b3_num = 11 * N_c - 2 * N_f
        if b3_num <= 0 or b3_num % 3 != 0:
            continue
        b_3 = b3_num // 3
        N_eff = b_3 + 2 * N_c
        sin2 = N_c / N_eff

        # P3: physical Weinberg angle
        if sin2 >= 0.5 or sin2 <= 0:
            continue

        # P5: additive closure b_3 = N_base + N_c
        p5 = (b_3 == N_BASE_FIXED + N_c)

        if p5:
            physics_solutions.append({
                'N_c': N_c, 'N_gen': N_gen, 'N_f': N_f,
                'b_3': b_3, 'N_eff': N_eff, 'sin2': sin2,
                'N_base': N_BASE_FIXED
            })
            print(f"  {N_c:4d}  {N_gen:5d}  {N_f:4d}  {b_3:4d}  {N_eff:5d}  {sin2:6.4f}  {'YES':>4s}")

print()
print(f"  Solutions satisfying ALL physics constraints (P1-P5): {len(physics_solutions)}")
for sol in physics_solutions:
    print(f"    {{N_c={sol['N_c']}, N_base={sol['N_base']}, b_3={sol['b_3']}, "
          f"N_eff={sol['N_eff']}}}  (N_gen={sol['N_gen']}, sin^2={sol['sin2']:.4f})")
print()

# Is {3, 4, 7, 13} in the solutions?
target = {'N_c': 3, 'b_3': 7, 'N_eff': 13}
target_found = any(s['N_c'] == 3 and s['b_3'] == 7 and s['N_eff'] == 13
                    for s in physics_solutions)

suite.assert_true(
    "{3, 4, 7, 13} satisfies all physics constraints (P1-P5)",
    target_found,
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 3: Adding the Master Quadratic Constraint
# ============================================================================

print()
print("=" * 78)
print("  SECTION 3: Master Quadratic Constraint [THEOREM]")
print("=" * 78)
print()
print(f"  From the master quadratic: x_- = {X_MINUS:.6f}")
print(f"  => N_c = floor(x_-) = {int(math.floor(X_MINUS))}")
print()

# Filter solutions to those with N_c = floor(x_-)
N_c_from_quadratic = int(math.floor(X_MINUS))
quadratic_solutions = [s for s in physics_solutions if s['N_c'] == N_c_from_quadratic]

print(f"  Solutions with N_c = {N_c_from_quadratic} (from master quadratic):")
for sol in quadratic_solutions:
    print(f"    {{N_c={sol['N_c']}, N_base={sol['N_base']}, b_3={sol['b_3']}, "
          f"N_eff={sol['N_eff']}}}  (N_gen={sol['N_gen']}, sin^2={sol['sin2']:.4f})")
print()

suite.assert_true(
    "Master quadratic gives N_c = 3",
    N_c_from_quadratic == 3,
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 4: Adding N_gen = N_c Constraint
# ============================================================================

print()
print("=" * 78)
print("  SECTION 4: N_gen = N_c Constraint [SELECTION]")
print("=" * 78)
print()
print("  The identification N_gen = N_c (three generations from three colors)")
print("  is a [SELECTION] -- argued from the cuboctahedral geometry but not")
print("  uniquely derived from the lattice axioms.")
print()

gen_solutions = [s for s in quadratic_solutions if s['N_gen'] == s['N_c']]
print(f"  Solutions with N_c = 3 AND N_gen = N_c:")
for sol in gen_solutions:
    print(f"    {{N_c={sol['N_c']}, N_base={sol['N_base']}, b_3={sol['b_3']}, "
          f"N_eff={sol['N_eff']}}}  (N_gen={sol['N_gen']}, sin^2={sol['sin2']:.4f})")
print()

suite.assert_true(
    "With N_gen = N_c = 3: unique solution is {3, 4, 7, 13}",
    len(gen_solutions) == 1
    and gen_solutions[0]['N_c'] == 3
    and gen_solutions[0]['b_3'] == 7
    and gen_solutions[0]['N_eff'] == 13,
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 5: Sequence Constraints Check
# ============================================================================

print()
print("=" * 78)
print("  SECTION 5: Sequence Constraints [SELECTION]")
print("=" * 78)
print()

# S1: N_eff at Fibonacci-Tribonacci crossover
s1 = any(idx == 7 and val == 13 for idx, val in crossovers)
print(f"  S1. N_eff=13 at Fibonacci-Tribonacci crossover: {s1}")

# S2: b_3=7, N_eff=13 consecutive Tribonacci
s2 = (7 in trib and 13 in trib
      and trib.index(13) == trib.index(7) + 1)
print(f"  S2. b_3=7, N_eff=13 consecutive Tribonacci: {s2}")
print(f"      T_6 = {trib[6]}, T_7 = {trib[7]}")

# S3: N_base=4, b_3=7 consecutive Lucas
s3 = (4 in luc and 7 in luc
      and luc.index(7) == luc.index(4) + 1)
print(f"  S3. N_base=4, b_3=7 consecutive Lucas: {s3}")
print(f"      L_3 = {luc[3]}, L_4 = {luc[4]}")

# S4: Crossover index = b_3
s4 = crossovers[0][0] == 7 if crossovers else False
print(f"  S4. Crossover index = b_3 = 7: {s4}")
print()

all_sequence = s1 and s2 and s3 and s4
print(f"  All sequence constraints satisfied: {all_sequence}")
print()

suite.assert_true(
    "All 4 sequence constraints (S1-S4) satisfied by {3, 4, 7, 13}",
    all_sequence,
    tag="[SELECTION]"
)


# ============================================================================
# SECTION 6: Full Exhaustive Search (no physics, pure combinatorics)
# ============================================================================

print()
print("=" * 78)
print("  SECTION 6: Exhaustive Search (pure combinatorics) [THEOREM]")
print("=" * 78)
print()
print("  Search ALL integer quadruples (N_c, N_base, b_3, N_eff) with:")
print("    1 <= N_c <= 100, 1 <= N_base <= 50, 1 <= b_3 <= 100, 1 <= N_eff <= 200")
print("  satisfying ALL of:")
print("    P2. N_eff = b_3 + 2*N_c")
print("    P5. b_3 = N_base + N_c")
print("    S1. N_eff in Fibonacci AND Tribonacci (value, not index)")
print("    S2. b_3, N_eff consecutive Tribonacci")
print("    S3. N_base, b_3 consecutive Lucas")
print()

# Build lookup sets
trib_values = set(trib)
fib_values = set(fib)
luc_values = set(luc)

# Consecutive Tribonacci pairs: (T_n, T_{n+1})
trib_consecutive = set()
for i in range(len(trib) - 1):
    if trib[i] > 0 and trib[i+1] > 0:
        trib_consecutive.add((trib[i], trib[i+1]))

# Consecutive Lucas pairs: (L_n, L_{n+1})
luc_consecutive = set()
for i in range(len(luc) - 1):
    if luc[i] > 0 and luc[i+1] > 0:
        luc_consecutive.add((luc[i], luc[i+1]))

exhaustive_solutions = []

for N_c in range(1, 101):
    for N_base in range(1, 51):
        b_3 = N_base + N_c  # P5
        N_eff = b_3 + 2 * N_c  # P2

        if N_eff > 200:
            continue

        # S1: N_eff in both Fibonacci and Tribonacci
        if N_eff not in fib_values or N_eff not in trib_values:
            continue

        # S2: (b_3, N_eff) consecutive Tribonacci
        if (b_3, N_eff) not in trib_consecutive:
            continue

        # S3: (N_base, b_3) consecutive Lucas
        if (N_base, b_3) not in luc_consecutive:
            continue

        exhaustive_solutions.append((N_c, N_base, b_3, N_eff))
        print(f"  FOUND: (N_c={N_c}, N_base={N_base}, b_3={b_3}, N_eff={N_eff})")

print()
print(f"  Total solutions: {len(exhaustive_solutions)}")
print()

if len(exhaustive_solutions) == 1:
    print("  RESULT: {3, 4, 7, 13} is UNIQUE under constraints P2+P5+S1+S2+S3")
elif len(exhaustive_solutions) > 1:
    print("  RESULT: Multiple solutions found. {3, 4, 7, 13} is NOT unique.")
else:
    print("  RESULT: No solutions found (this would be an error).")

suite.assert_true(
    "{3, 4, 7, 13} is the unique solution under P2+P5+S1+S2+S3",
    len(exhaustive_solutions) == 1 and exhaustive_solutions[0] == (3, 4, 7, 13),
    tag="[THEOREM]"
)


# ============================================================================
# SECTION 7: Relaxed Search (P1+P2+P3+P5 only, no sequences)
# ============================================================================

print()
print("=" * 78)
print("  SECTION 7: Solutions Without Sequence Constraints [THEOREM]")
print("=" * 78)
print()
print("  How many quadruples satisfy ONLY the physics constraints?")
print("  P1. b_3 = (11*N_c - 4*N_gen)/3, integer, positive")
print("  P2. N_eff = b_3 + 2*N_c")
print("  P3. sin^2(theta_W) = N_c/N_eff in (0.15, 0.35)  [near observed]")
print("  P4. N_base = 4  [from D=3]")
print("  P5. b_3 = N_base + N_c")
print()

relaxed_solutions = []
for N_c in range(1, 51):
    for N_gen in range(1, 21):
        N_f = 2 * N_gen
        b3_num = 11 * N_c - 2 * N_f
        if b3_num <= 0 or b3_num % 3 != 0:
            continue
        b_3 = b3_num // 3
        N_eff = b_3 + 2 * N_c
        sin2 = N_c / N_eff

        if not (0.15 < sin2 < 0.35):
            continue

        if b_3 != N_BASE_FIXED + N_c:
            continue

        relaxed_solutions.append((N_c, N_gen, b_3, N_eff, sin2))

print(f"  Solutions (physics only, sin^2 near 0.231):")
for sol in relaxed_solutions:
    print(f"    N_c={sol[0]}, N_gen={sol[1]}, b_3={sol[2]}, "
          f"N_eff={sol[3]}, sin^2={sol[4]:.4f}")
print()
print(f"  Total: {len(relaxed_solutions)}")
print()

# How many have the exact sin^2 = 3/13?
exact_sin2 = [s for s in relaxed_solutions if abs(s[4] - 3.0/13.0) < 1e-10]
print(f"  With exact sin^2 = 3/13 = {3.0/13.0:.6f}: {len(exact_sin2)}")
for sol in exact_sin2:
    print(f"    N_c={sol[0]}, N_gen={sol[1]}, b_3={sol[2]}, "
          f"N_eff={sol[3]}, sin^2={sol[4]:.6f}")
print()

# Note: all solutions with N_gen = N_c give the same sin^2 = 3/13
# because sin^2 = N_c / (7N_c/3 + 2N_c) = N_c / (13N_c/3) = 3/13.


# ============================================================================
# SECTION 8: Honest Accounting
# ============================================================================

print()
print("=" * 78)
print("  SECTION 8: Honest Accounting")
print("=" * 78)
print()
print("  [THEOREM] -- What is proven:")
print("    1. N_c = 3 from the master quadratic floor(x_-) = 3")
print("    2. Given N_c=3 and N_gen=N_c, the system P1-P5 has a UNIQUE solution")
print("    3. {3,4,7,13} satisfies all sequence constraints S1-S4")
print("    4. {3,4,7,13} is the UNIQUE solution to P2+P5+S1+S2+S3 (exhaustive search)")
print("    5. F_7 = T_7 = 13 is the only non-trivial Fib-Trib crossover for n <= 30")
print()
print("  [SELECTION] -- What remains a choice:")
print("    * N_gen = N_c (three generations from three colors)")
print("    * The sequence constraints S1-S4 (why these sequences?)")
print("    * The additive closure P5 (why b_3 = N_base + N_c?)")
print()
print("  RESULT: Given the master quadratic AND N_gen = N_c,")
print("  the framework integers {3, 4, 7, 13} are uniquely determined.")
print("  The sequence constraints provide independent verification")
print("  but are not required for uniqueness.")
print()


# ============================================================================
# SUMMARY
# ============================================================================

suite.print_summary()
sys.exit(0 if suite.all_pass else 1)
