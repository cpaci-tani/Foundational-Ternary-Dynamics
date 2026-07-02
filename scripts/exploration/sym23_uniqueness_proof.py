"""
Honest enumeration for Paper A §16.5 (leading-period exponent pairs).

REPAIRED 2026-07-01/02 (FTD-0351, executing the 2026-07-01 specialist-review
finding FTD-0348 §3.1). The previous version of this script encoded a flawed
case classification BY FIAT: it declared "Case A (2a > b): roots involve TWO
distinct G*-powers -> PASS" and "Case C (2a < b): roots are scalar multiples
of G*^a -> FAIL" without computing anything. That distinction is purely
notational -- in EVERY case the roots are identically

    x_pm = 8 G*^a +/- 4 sqrt(4 G*^{2a} - G*^b)
         = 8 G*^a +/- 4 G*^{b/2} sqrt(4 G*^{2a-b} - 1)      (same number)
         = 4 G*^a (2 +/- sqrt(4 - G*^{b-2a}))               (same number)

so a criterion cannot pass one factoring and fail the other. This script now
verifies that identity numerically, applies the criteria honestly, and reports
the TRUE result:

  * criterion (i)  "roots not CONSTANT multiples of a single G*^k"
    excludes exactly the line b = 2a (where x_pm = (8 +/- 4 sqrt(3)) G*^a);
  * criterion (ii) "positive discriminant" excludes exactly b >= 2a + 2
    (ln 4 / ln G* = 1.2779... < 2, and G* = 2.9586... < 4 keeps b = 2a+1);
  * the admissible set is {a < b < 2a} UNION {b = 2a + 1};
  * the pair (1, 3) SURVIVES (Delta = 64 G*^2 (4 - G*) > 0), so the formerly
    claimed minimal-a uniqueness of (2, 3) is FALSE. The minimal-a survivor
    is (1, 3).

The (2, 3) selection is therefore NOT a minimality theorem. The honest chain
(reported at the end) is conditional: the Watson trace 16 G*^2 = 32 pi W_3
forces a = 2 [THEOREM lineage, FTD-0002/0006], and the Vieta ratio ansatz
Det = Tr * G* (i.e. b = a + 1) forces b = 3 -- but that ansatz is exactly the
[UNDERDETERMINED] W-CRIT-2 assembly (FTD-0235). Tag: [SELECTION].
"""

from mpmath import mp, mpf, gamma, sqrt, log, pi

mp.dps = 30

# G* = Gamma(1/4)/Gamma(3/4)
G_star = gamma(mpf(1)/4) / gamma(mpf(3)/4)

TOL = mpf(10) ** -25

print("=" * 95)
print("Paper A §16.5 -- honest enumeration (repaired under FTD-0351; was a false-uniqueness script)")
print(f"G* = Gamma(1/4)/Gamma(3/4) = {float(G_star):.10f}")
print("=" * 95)

# ---------------------------------------------------------------------------
# Step 1: the old Case-A/Case-C split is notationally vacuous.
# ---------------------------------------------------------------------------
print()
print("Step 1 -- the old Case-A (2a>b) vs Case-C (2a<b) split is NOTATIONALLY VACUOUS:")
print("          the three radical spellings of the roots agree to 25+ digits in both regimes.")
print()
for (a, b) in [(2, 3), (3, 4), (3, 5), (1, 3), (2, 5), (3, 7)]:
    disc_core = 4 * G_star ** (2 * a) - G_star ** b
    if disc_core <= 0:
        continue
    plain = 4 * sqrt(disc_core)
    spellA = 4 * G_star ** (mpf(b) / 2) * sqrt(4 * G_star ** (2 * a - b) - 1)
    spellC = 4 * G_star ** a * sqrt(4 - G_star ** (b - 2 * a))
    regime = "2a>b (old 'Case A')" if 2 * a > b else "2a<b (old 'Case C')"
    ok = abs(plain - spellA) < TOL and abs(plain - spellC) < TOL
    print(f"  (a,b)=({a},{b}) [{regime}]: |plain-A|={float(abs(plain-spellA)):.1e}, "
          f"|plain-C|={float(abs(plain-spellC)):.1e}  {'IDENTICAL' if ok else 'MISMATCH!'}")
    assert ok, "radical spellings disagree -- impossible"

# ---------------------------------------------------------------------------
# Step 2: honest criteria.
#   (i)  roots not CONSTANT multiples of a single G*^k.
#        Symbolic fact (G* transcendental, treat as indeterminate t):
#        x_pm = c_pm t^{k_pm} with algebraic constants c_pm forces, via Vieta
#        (monomial sum + monomial product), k_+ = k_- = a and 2a = b.
#        So criterion (i) excludes EXACTLY b = 2a.
#   (ii) Delta > 0  <=>  G*^{b-2a} < 4  <=>  b - 2a <= 1 for integers.
# ---------------------------------------------------------------------------
print()
bound = log(4) / log(G_star)
print("Step 2 -- honest criteria:")
print(f"  (i)  excludes exactly b = 2a  (there x_pm = (8 +/- 4 sqrt(3)) G*^a; Vieta monomial argument)")
print(f"  (ii) Delta > 0  <=>  b - 2a < ln4/lnG* = {float(bound):.6f}  =>  integer b <= 2a + 1")
print(f"       (G*^1 = {float(G_star):.4f} < 4 < G*^2 = {float(G_star**2):.4f}: "
      f"b = 2a+1 survives, b >= 2a+2 fails)")

print()
print(f"{'(a, b)':>8}  {'b vs 2a':>10}  {'crit (i)':>9}  {'disc > 0':>9}  {'branch':<18}  verdict")
print("-" * 80)

valid_pairs = []
for a in range(1, 11):
    for b in range(a + 1, 13 - a):
        delta = 64 * (4 * G_star ** (2 * a) - G_star ** b)
        disc_positive = delta > 0
        crit_i = (b != 2 * a)          # the ONLY exclusion criterion (i) delivers
        if b < 2 * a:
            branch = "wedge a<b<2a"
        elif b == 2 * a:
            branch = "line b=2a"
        elif b == 2 * a + 1:
            branch = "branch b=2a+1"
        else:
            branch = "b>=2a+2"
        valid = crit_i and disc_positive
        if valid:
            valid_pairs.append((a, b))
        rel = ">" if b > 2 * a else ("=" if b == 2 * a else "<")
        print(f"{f'({a}, {b})':>8}  {f'b {rel} 2a':>10}  {('PASS' if crit_i else 'FAIL'):>9}  "
              f"{('yes' if disc_positive else 'no'):>9}  {branch:<18}  {'ADMISSIBLE' if valid else '--'}")

print()
print("=" * 95)
print("Step 3 -- the TRUE result")
print("=" * 95)
wedge = [(a, b) for (a, b) in valid_pairs if b < 2 * a]
branch_2a1 = [(a, b) for (a, b) in valid_pairs if b == 2 * a + 1]
stray = [p for p in valid_pairs if p not in wedge and p not in branch_2a1]
print(f"  admissible wedge pairs (a+b<=12):   {wedge}")
print(f"  admissible b=2a+1 pairs (a+b<=12):  {branch_2a1}")
assert stray == [], f"unexpected admissible pairs outside the two branches: {stray}"
assert (1, 3) in valid_pairs, "(1,3) must survive -- it satisfies every substantive criterion"

a, b = 1, 3
delta13 = 64 * (4 * G_star ** 2 - G_star ** 3)
xp13 = 8 * G_star + sqrt(delta13) / 2
xm13 = 8 * G_star - sqrt(delta13) / 2
print(f"\n  the (1,3) survivor: Delta = 64 G*^2 (4 - G*) = {float(delta13):.6f} > 0")
print(f"                      roots = {float(xp13):.10f}, {float(xm13):.10f}")
print(f"                      (b = 3 != 2a = 2, so roots are NOT constant multiples of one G*-power)")

min_a_pair = min(valid_pairs, key=lambda p: (p[0], p[1]))
print(f"\n  minimal-a admissible pair: {min_a_pair}")
assert min_a_pair == (1, 3)
print("  ==> the formerly claimed '(2, 3) is uniquely minimal-a' is FALSE.")
print("      [THEOREM] content that survives: admissible set = {a<b<2a} UNION {b=2a+1}.")

# ---------------------------------------------------------------------------
# Step 4: the conditional (2,3) selection -- [SELECTION], not a theorem.
# ---------------------------------------------------------------------------
print()
print("=" * 95)
print("Step 4 -- how (2,3) is actually selected  [SELECTION, conditional]")
print("=" * 95)
W3 = G_star ** 2 / (2 * pi)  # Watson identity closed form (OT-2.1)
print(f"  (1) trace: 32 pi W_3 = {float(32 * pi * W3):.10f} = 16 G*^2 = {float(16 * G_star**2):.10f}")
assert abs(32 * pi * W3 - 16 * G_star ** 2) < TOL
print("      matching the Watson trace [THEOREM lineage, FTD-0002/0006] forces a = 2")
print("  (2) Det = Tr * G*  <=>  16 G*^b = 16 G*^{a+1}  <=>  b = a + 1  =>  b = 3")
print("      BUT this Vieta-ratio ansatz is the [UNDERDETERMINED] W-CRIT-2 assembly (FTD-0235):")
print("      nothing substrate-native forces the determinant's exponent.")
a, b = 2, 3
delta = 64 * (4 * G_star ** 4 - G_star ** 3)
x_plus = (16 * G_star ** 2 + sqrt(delta)) / 2
x_minus = (16 * G_star ** 2 - sqrt(delta)) / 2
print(f"  (2,3) roots: x_+ = {float(x_plus):.10f}, x_- = {float(x_minus):.10f}")
print("  (x_+ = 1/alpha identification rides separately at [STRONGLY MOTIVATED CONJECTURE], FTD-0013)")

print()
print("=" * 95)
print("CONCLUSION (repaired, FTD-0351)")
print("=" * 95)
print("The admissible exponent set under honest criteria is {a < b < 2a} UNION {b = 2a+1}.")
print("(1, 3) survives; the minimal-a admissible pair is (1, 3), NOT (2, 3).")
print("The old 'Theorem 16.5.1' minimal-a uniqueness claim is RETRACTED (FTD-0348 §3.1 / FTD-0351).")
print("(2, 3) is a CONDITIONAL SELECTION: Watson trace (a = 2, proven) + Det = Tr*G* ansatz")
print("(b = a+1, W-CRIT-2 [UNDERDETERMINED], FTD-0235). No uniqueness theorem. No QED.")
