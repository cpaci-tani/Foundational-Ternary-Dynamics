# THEOREM — The axial surd obstruction and the Pythagorean currency of unit-strut closure v1

**Date:** 2026-08-14
**Status:** `[THEOREM — T1/T2/T3, elementary number theory + case trees, verifier 8/8]`
+ `[OBSERVATION — appendix identity, classical]` + `[OPEN — multi-ring axial class]`
**Verifier:** `scripts/proofs/proof_axial_surd_pythagorean.py` (8/8)
**Position:** successor to FTD-1004 (the first exact-certificate run of the
clock-minimum spec §8 step 2). FTD-1004 closed six families and named its open
escapes; this note proves the *pattern* behind those kills, closes one named
escape in full generality, strengthens a second, and derives the arithmetic
that any surviving candidate must use. No dynamical claim is made; every
result here is exact mathematics about the registered realization class
(struts = single unit bonds; straight integer-span tension chains; polarity;
support clearances).

## 1. The observed pattern

Every family killed by certificate in FTD-1004 died on one Diophantine shape:
`d² − p² = 1` (ladder), `s² − 4k² = −1` (lens), `√(k²−1) ± √(m²−1) = 1`
(two-term axial closures). Each asks a difference of integer squares, or a
signed sum of quadratic surds, to pay for exactly one unit — the strut. The
present note proves this is not a coincidence of small cases.

## 2. T1 — the surd obstruction (all stages)

**Lemma (classical; Besicovitch 1940).** Square roots of distinct squarefree
positive integers, together with 1, are linearly independent over ℚ.
(Verifier P01 confirms field-degree instances `[ℚ(√m₁,…,√m_r):ℚ] = 2^r`.)

**Theorem T1.** Let r₁,…,r_N be positive rationals, none a square of a
rational, and ε_i ∈ {±1}. Then Σ ε_i √(r_i) ≠ 1.

*Proof.* Write each √(r_i) = a_i √(m_i) with a_i ∈ ℚ₊ and m_i > 1 squarefree.
Group terms by kernel m: the sum is Σ_m c_m √m with c_m ∈ ℚ. By the lemma,
{1} ∪ {√m} is ℚ-independent, so Σ c_m √m = 1 forces every c_m = 0 and 0 = 1,
a contradiction. ∎

**Consequence for the class.** In any axially-symmetric configuration, the
axial rise of a cable stage of span k to a ring at radius ρ is √(k² − ρ²).
Closure through a unit strut demands a signed sum of stage rises equal to 1.
By T1, **no all-surd closure pays a unit strut, for any number of stages** —
the "≥ 3-term axial closures" escape named in FTD-1004 is closed in full
generality, not merely at two terms. (Verifier P02 sweeps instances: no
signed sum of √(k²−1), k ≤ 30, up to four terms, equals 1.)

## 3. T2 — the Pythagorean currency

For closure arithmetic to escape T1, at least one stage must have a
*rational* rise: k² − ρ² must be the square of a rational. The ring radius is
not free — integer ring chains of span t pin it to ρ = t / (2 sin(π/n)).

**Theorem T2 (crystallographic restriction, machine-verified P03).** ρ² is
rational iff n ∈ {3, 4, 6} (for 3 ≤ n ≤ 12; the excluded cases carry √5, √2,
√3 in ρ²/t²). The rational values are ρ² = t²/3, t²/2, t² respectively, so
every rational-rise stage satisfies a generalized Pythagorean equation

> **k² − t²/c = h²**, c ∈ {3, 2, 1} for n ∈ {3, 4, 6}, h ∈ ℤ₊ the rise.

The currency is nonempty (verifier P04): the primitive stages with k, t ≤ 12
are **(k,t,h) = (2,3,1) at n = 3, (3,4,1) at n = 4, (5,4,3) and (5,3,4) at
n = 6**, with their multiples. The n = 6 family is the classical Pythagorean
triple equation itself.

**Reading.** The unit strut can only be paid in Pythagorean coin, and the
mint only operates at the crystallographic ring orders. The arithmetic face
of the C3 wall, exposed by FTD-1004's kill table, is therefore exactly:
*small configurations cannot raise Pythagorean funding, and surd funding is
counterfeit at every denomination* (T1).

## 4. T3 — single-ring closure, unconditional

FTD-1004's F4 verdict covered single rings with *unit* ring bonds. The
restriction can be removed.

**Theorem T3.** No axially-symmetric unit-strut tensegrity with exactly one
ring exists, for any ring span t and radius ρ.

*Proof (case tree over the ring's stress sign).*
- **Ring in tension.** Ring joints are pulled radially inward by their
  cables; equilibrium needs an outward force, which only a compressed member
  supplies, and compressed members are single unit bonds (the FTD-0804/0805
  buckling criterion). In a single-ring configuration the only non-ring
  joints are axial, so the pusher is a unit spoke from an axis point:
  ρ² + h² = 1, hence ρ ≤ 1 and t = 2ρ sin(π/n) ≤ 2. t = 2 forces
  ρ = 1/sin(π/n) ≥ 1, hence ρ = 1 and n = 2, degenerate (verifier P05). So
  t = 1: a unit ring with unit spokes — the hub–rim–rim cycle carries three
  parity flips and no polarity 2-coloring exists (odd n kills the ring cycle
  outright; verifier P06). Dead.
- **Ring in compression.** Compressed members are single bonds, so t = 1:
  the alternating unit ring meets the single-polarity cable constraint of
  FTD-1004's F4 case (a) (z-equilibrium requires cables to both strut ends;
  `k+m` odd forces one common ring polarity, contradicting alternation).
  The ring-unbonded variant is F4 case (b). Dead. ∎

T3 strictly extends FTD-1004: the surviving axial territory begins at **two
rings**.

## 5. The surviving territory, named

Combining T1–T3 with FTD-1004's certificates: any axially-symmetric native
C3 candidate must have ≥ 2 rings, ring orders n ∈ {3, 4, 6}, and at least
one Pythagorean stage from the currency table funding its unit struts. The
smallest currency — (2,3,1) at n = 3 and (3,4,1) at n = 4 — gives concrete
drum/bicupola-type targets for a v2 decision campaign. This is a search
specification in the sense of the clock-minimum spec §7, not an existence
claim; a v2 campaign must be preregistered before any stress/blocking
verdict is computed on these candidates.

## 6. Appendix — the squircle identity `[OBSERVATION]`

Three exact classical facts, absent from the corpus, recorded for
orientation (no tag moves; verifier P07/P08):

1. The area of the unit squircle x⁴ + y⁴ = 1 equals Γ(1/4)²/(2√π) =
   √(π/2)·G\* = 3.70814935…, which is **exactly the quartic clock invariant
   T·A** (FTD-0817/0823 context; verified symbolically, numeric difference
   0 at 170+ digits).
2. The substitution w = y² maps the squircle identically onto w² = 1 − x⁴,
   the quartic clock's time curve (the lemniscatic CM curve of spine
   Theorem 3 via FTD-0827): the squircle is a double cover of the clock's
   complexified time domain.
3. The flat-bottomed quartic well and the squircle are therefore two real
   slices of one object, and the three faces of the C3 wall align as:
   *rigidity* (FTD-0789: second-order blocking), *arithmetic* (FTD-1004 +
   T1–T3: Pythagorean currency), *polarization* (FTD-0841: the lattice's
   native quartic is separable/squircle-type, not radial; only linearly
   polarized sectors inherit the scalar period).

## 7. Scope

T1–T3 are elementary and unconditional within the registered realization
class; they import the buckling criterion (FTD-0804/0805) and the F4 case
tree (FTD-1004) where stated. Nothing here touches dynamics, the band
question (C2), amplitude decades, or any period identity; no target constant
was used in §§2–5. Non-axial (asymmetric) configurations are outside every
statement above and remain governed only by FTD-1004's named escapes.
