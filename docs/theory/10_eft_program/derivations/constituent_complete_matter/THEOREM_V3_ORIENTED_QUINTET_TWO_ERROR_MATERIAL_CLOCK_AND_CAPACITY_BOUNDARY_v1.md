# V3 oriented quintet two-error material clock and capacity boundary v1

**Date:** 2026-08-25  
**Status:** **[SELECTION — PREPARED FIVE-COPY ORIENTED MATTER CLOCK]** +
**[THEOREM, CONDITIONAL — EXACT RADIUS-TWO VALID-SYMBOL BASIN AND WORK
COUNT]** + **[THEOREM — ONE-SITE CONTROLLER CAPACITY BOUNDARY]** +
**[BOUNDARY — ISOTROPY, FORMATION, BINDING, AND GENERAL STABILITY OPEN]**  
**Additional carrier price:** none; five copied 33-symbol neutral registers use
165 states of the existing 170-state chart-local clear controller pool, and
one existing fixed-occupancy A2 owner retains work level `0`, `1`, or `2`  
**Production status:** unchanged  
**Ledger status:** no row minted  
**One-error parent:**
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md)  
**Relative-work parent:**
[`THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md`](../common_action_mechanics_reciprocity/THEOREM_V3_TRIPLET_RELATIONAL_REPAIR_WORK_PORT_PHI_v5_CANDIDATE_v1.md)  
**Certificate:**
[`proof_v3_oriented_quintet_two_error_material_clock_and_capacity_boundary.py`](../../../../../scripts/proofs/proof_v3_oriented_quintet_two_error_material_clock_and_capacity_boundary.py)

---

## 1. Question and result

The prepared Phi-v5 triplet corrects one valid-symbol substitution. The next
Occam-minimal question is whether the existing finite carrier can protect the
same period-16 clock against **two simultaneous substitutions** without
introducing a new type, hidden history, random draw, or continuous variable.

Within the copied-register repetition architecture the answer is exact:

\[
 n_{\min}=2t+1=5\qquad(t=2).                         \tag{1}
\]

Five copied logical owners and five copied herald registers supply minimum
mixed-symbol code distance five. Strict-majority READ/COMMIT correction then
recovers every admitted one- or two-position substitution in at most two
global ticks. The work owner records the number of corrected substitutions,
so recovery is not cost-free deletion.

This closes a finite radius-two **symbol-error** basin. It does not establish
formation, energetic binding, environmental persistence, or a physical
particle.

---

## 2. Existing-carrier capacity

Each herald register uses the already certified 33-symbol alphabet

\[
 \mathcal H=\{{\tt DARK}\}\cup
 \{({\tt logical},{\tt syndrome})\},
 \qquad |\mathcal H|=33.                             \tag{2}
\]

The exact chart-local pool contains 170 distinct zero-`E/B` neutral
field-pair states. Therefore

\[
 5|\mathcal H|=165\le170,
 \qquad
 6|\mathcal H|=198>170.                              \tag{3}
\]

Five copied registers fit at one site with no new ontic carrier. Six do not.
This is a finite pigeonhole boundary, not a numerical fit.

One existing A2 owner has fixed occupancy throughout. Three of its four C4
levels encode retained work count

\[
 w\in\{0,1,2\}.                                     \tag{4}
\]

The complete role count is consequently constant on the clean orbit and on
every admitted error state:

\[
 (N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(10,5,0,1).       \tag{5}
\]

---

## 3. Oriented five-copy geometry

Relative to the existing oriented chart
`(first,second,repair_normal)`, the five disjoint SC relations use directions

\[
 (e,f,n,e,f).                                        \tag{6}
\]

The two added owners have relative tails `(-1,-1,0)` and `(-1,-1,1)`.
Together with the triplet's original three owners they are endpoint-disjoint,
remain inside one Moore block, and avoid all retained chart and registered
source relations in every one of the 1,152 signed-cubic chart images.

Transforming the chart, the five relation owners, the five neutral register
banks, and the work owner together commutes with each signed-cubic generator.
The construction is therefore covariant, although a particular material
instance retains an orientation record.

The price of using five SC directions is the exact dyadic source shape

\[
 Q_5=2ee^{\mathsf T}+2ff^{\mathsf T}+nn^{\mathsf T}
    =\operatorname{diag}(2,2,1),                     \tag{7}
\]

in its own chart. It is not the isotropic tensor `5I/3`. Equal axial
multiplicity would require six copies, which equation (3) excludes at one
site in this register realization.

---

## 4. State-complete READ/COMMIT transaction

On a DARK-majority layer, the rule:

1. decodes the strict majority of the five logical owners;
2. counts every logical or herald mismatch in the admitted snapshot;
3. fails closed if the count exceeds two or if a nonzero work record is
   already being spent; and
4. writes five copies of `(decoded logical,syndrome)` while retaining the
   exact mismatch count in A2.

On a pending-majority layer, the rule restores all five logical copies to the
next clock state, clears all five heralds to DARK, and preserves the retained
work level. The clean complete orbit has exact period sixteen:

\[
 \Phi_5^{16}(X)=X,
 \qquad
 \Phi_5^k(X)\ne X\quad(0<k<16).                      \tag{8}
\]

For every admitted mutant `Y` at clean phase `j`, there is an
`r in {1,2}` such that its body projection rejoins the clean orbit:

\[
 \pi_{\rm body}\Phi_5^r(Y)
 =\pi_{\rm body}\Phi_5^r(X_j).                      \tag{9}
\]

The output work is exactly one for a one-position substitution and exactly
two for a two-position substitution.

The basin is a registered **snapshot** basin. Errors admitted before READ, or
on a complete registered phase, are covered. A new error injected between
READ and COMMIT is not generally distinguishable with the retained memory and
is explicitly outside the theorem.

---

## 5. Complete finite census

At each of sixteen clean phases, an arm position has fifteen nonidentity
replacements and a herald position has thirty-two. Hence the one-position
census is

\[
 16[5(15)+5(32)]=3{,}760.                            \tag{10}
\]

The two-position census separates arm--arm, arm--herald, and herald--herald
replacements:

\[
 16\left[\binom52 15^2+25(15)(32)+\binom52 32^2\right]
 =391{,}840.                                         \tag{11}
\]

Therefore the complete audited radius-two basin contains

\[
 3{,}760+391{,}840=395{,}600                         \tag{12}
\]

error rows. Every row recovers within two ticks with the correct work count.
All clean and one-position occupancy presentations additionally preserve
equation (5), for 3,776 audited role rows.

Reproduce with:

```powershell
python scripts/proofs/proof_v3_oriented_quintet_two_error_material_clock_and_capacity_boundary.py
```

Expected result: `14/14` exact checks pass.

---

## 6. What has closed

The theorem establishes, conditional on the prepared oriented hardware:

1. a period-16 five-copy material clock made only from existing carriers;
2. exact minimum repetition length five for two-substitution correction;
3. exact minimum mixed-symbol code distance five;
4. a complete 395,600-row radius-two valid-symbol perturbation basin;
5. recovery in at most two global ticks;
6. explicit retained work levels `0`, `1`, and `2` in one fixed-occupancy A2;
7. fail-closed behavior for more than two decoded snapshot errors and a busy
   work port;
8. spatial locality and signed-cubic covariance on all 1,152 charts; and
9. the exact one-site capacity inequality `165 <= 170 < 198`.

This is materially stronger than demonstrating a periodic trajectory. It is
a finite protected invariant family with a nonzero, exhaustively enumerated
perturbation neighborhood.

---

## 7. What remains open

The result must not be promoted to general stable matter. It does not yet
derive:

1. genesis-seed formation of the quintet or its oriented chart;
2. positive relational binding curvature or the absolute action scale;
3. export/reset of accumulated repair work;
4. loss or gain of an occupied carrier, malformed carrier states, more than
   two errors, or faults arriving between READ and COMMIT;
5. an isotropic one-site rest source; equation (3) and equation (7) show the
   exact obstruction for this copied-register realization;
6. a multisite six-copy architecture, its traffic, or its causal arbitration;
7. translation, collisions, scattering, packet absorption, or emission;
8. mass, dispersion, inertial response, or a common protected pole; or
9. integration into the canonical state-complete `Phi`.

The correct conclusion is:

> The existing finite v3 carrier supports a prepared, local, covariant
> five-copy material clock that corrects every one- and two-symbol snapshot
> substitution with retained integer work. One-site isotropy and physical
> binding remain separate, unsolved requirements.

