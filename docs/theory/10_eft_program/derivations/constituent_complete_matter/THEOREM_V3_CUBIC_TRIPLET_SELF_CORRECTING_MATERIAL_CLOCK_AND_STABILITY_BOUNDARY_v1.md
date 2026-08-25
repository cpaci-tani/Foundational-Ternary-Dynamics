# V3 cubic-triplet self-correcting material clock and stability boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON A PREPARED ORIENTED HALO AND SELECTED
REPAIR TRANSACTION — EXACT RECURRENT PROTO-MATTER CLOCK]** +
**[THEOREM — NONZERO ONE-SUBSTITUTION BASIN]** +
**[THEOREM — CUBIC-ISOTROPIC MEAN CAPACITY DEFICIT]** +
**[OPEN — NATIVE ASSEMBLY, BINDING ENERGY, GENERAL PERTURBATIONS,
TRANSLATION, COLLISIONS, MASS, AND CANONICAL-PHI INTEGRATION]**  
**Formation parent:**
[`THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md`](THEOREM_V3_EVENT_HALO_FORMATION_SEED_AND_RESOURCE_BOUNDARY_v1.md)  
**Clock parent:**
[`THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md`](../common_action_mechanics_reciprocity/THEOREM_TERNARY_SQUARE_PHASE_POLARITY_CARRIER_AND_AUTONOMOUS_CROSSING_CLOCK_v1.md)  
**Exact certificate:**
[`proof_v3_cubic_triplet_self_correcting_material_clock.py`](../../../../../scripts/proofs/proof_v3_cubic_triplet_self_correcting_material_clock.py)

---

## 1. Result

One A9 relation clock is recurrent but has no nonzero substitution-error
basin: changing its valid phase/ownership symbol places it on another valid
trajectory. The minimum exact repetition-code repair uses three copies.

Place three mutually disjoint SC relations inside the closed Moore cube of an
oriented event-halo center, one parallel to each chart axis. In chart-relative
coordinates their tails and directions are

\[
 (-1,-1,-1)\to(0,-1,-1),\qquad
 (1,-1,0)\to(1,0,0),\qquad
 (0,1,-1)\to(0,1,0).                       \tag{1}
\]

The six endpoints are distinct, the center is not an endpoint, and the three
relations are disjoint from the charged frame and every registered event
source relation. Transforming the oriented chart transforms the whole
placement. The certificate verifies all 1,152 signed-cubic chart states.

Each arm carries the same valid owned A9 clock state `q`. Three independent
neutral field-pair registers at the center each encode the seventeen-symbol
alphabet

\[
 \{\mathrm{dark}\}\sqcup\{\text{sixteen valid owned A9 states}\}. \tag{2}
\]

The three register banks use 51 distinct existing one-particle controllers.
Every physical code symbol is an opposite-polarity pair with zero `E/B` on
all three layers. The banks avoid the static center pad, charged-frame fields,
and event herald namespace and transform covariantly with the chart.

---

## 2. Homogeneous two-tick transaction

Let `Maj` be strict majority on three symbols. The selected prepared-sector
transaction has two state-defined stages.

1. **READ:** when at least two herald registers are dark, decode
   `q=Maj(q1,q2,q3)`. If no arm majority exists, fail closed. Otherwise write
   `q` into all three neutral herald registers and leave the arms unchanged.
2. **COMMIT:** when at least two herald registers carry `q`, every arm writes
   the next autonomous crossing-clock state `U(q)` and the center restores all
   three herald registers to dark.

The center reads the complete radius-one arm star on the first tick. Every arm
then reads only the retained center record on the second tick. No same-tick
broadcast, global coloring, random choice, or external target is used.

On the exact code section,

\[
 (q,q,q;D,D,D)
 \longrightarrow(q,q,q;q,q,q)
 \longrightarrow(Uq,Uq,Uq;D,D,D).          \tag{3}
\]

Because `U` has exact period eight, the complete READ/COMMIT state has exact
period sixteen global ticks.

---

## 3. Exact perturbation basin

Use Hamming distance on the six logical registers: three complete arm states
and three constant-occupancy herald symbols. An admitted substitution replaces
one arm by another valid owned A9 state or one herald register by another of
its seventeen physical symbols. It therefore preserves arm-token count and
neutral herald occupancy.

The certificate exhausts every such radius-one mutation at every phase of the
sixteen-tick complete orbit:

\[
 16\,[3(16-1)+3(17-1)]=1{,}488             \tag{4}
\]

cases. Every case rejoins the unperturbed orbit after at most two global ticks.
The repair map is genuinely noninjective: distinct one-error inputs have the
same two-tick image. This is the constitution's expiry branch used
constructively rather than a hidden reversible syndrome tape.

The result is a nonzero exact perturbation basin, but only in the registered
valid-symbol substitution class. Deletion/insertion of occupancy, malformed
endpoint/token combinations, simultaneous multi-register errors, sustained
traffic, and collisions are not covered.

Three copies are minimal within the repetition architecture. A length-two
word has distance two and a one-symbol mutation can be equidistant between
logical codewords; length three has distance three and corrects one arbitrary
substitution.

---

## 4. Neutrality and isotropic rest source

Every arm retains exactly one relation token and zero total ternary charge.
Across its eight local clock states one arm has mean manifested activity one,
so the triplet has

\[
 \left\langle\sum_x s_x^2\right\rangle=3.   \tag{5}
\]

For one relation with unit dyad `M_a=e_a e_a^T`, the parent clock theorem gives
mean capacity deficit `-M_a/36`. The three oriented chart axes are orthonormal,
therefore

\[
 \boxed{
 \sum_{a=1}^{3}-{M_a\over36}
 =-{1\over36}\sum_{a=1}^{3}e_a e_a^T
 =-{I_3\over36}.}                            \tag{6}
\]

Thus the minimum self-correcting triplet is also a cubically isotropic,
neutral, recurrent rest-source candidate. Equation (6) is independent of the
oriented chart and verified on all 1,152 chart states.

Calling `-I_3/36` inertial or gravitational mass would be premature. The
construction has no derived positive binding energy, response pole, force
law, mobility, or absolute normalization.

---

## 5. What this changes

This theorem closes three narrower stable-matter requirements on a prepared
sector:

1. a state-only and signed-cubic-covariant object code;
2. an exact recurrent complete-state family; and
3. a nonzero registered perturbation basin with a two-tick survival/recovery
   bound.

It also supplies one isotropic neutral capacity source that can be used in the
gravity-response programme without inserting a continuum stress tensor.

It does **not** close stable matter. Still required are:

- target-blind causal assembly from the finite genesis seed or boundary
  current;
- a positive conserved binding/work ledger for the repair transaction;
- robustness to occupancy loss/gain, multiple errors, environmental traffic,
  and body overlap;
- translation, recoil, scattering, and a mass/dispersion readout; and
- integration into a canonical homogeneous `Phi` rather than a selected
  prepared-sector successor.

Born statistics gains a possible robust material clock/apparatus component,
but not bank preparation, amplification, or no-signalling. Gravity gains the
source shape (6), but not its coupling, protected propagator, common cone,
lensing, or nonlinear completion.

The five-copy successor
[`THEOREM_V3_ORIENTED_QUINTET_TWO_ERROR_MATERIAL_CLOCK_AND_CAPACITY_BOUNDARY_v1.md`](THEOREM_V3_ORIENTED_QUINTET_TWO_ERROR_MATERIAL_CLOCK_AND_CAPACITY_BOUNDARY_v1.md)
extends the prepared valid-symbol basin to two simultaneous substitutions. It
also proves that the maximum one-site copied-register clock is anisotropic:
five herald banks fit the existing controller pool, whereas the six banks
needed for equal axial multiplicity do not.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_cubic_triplet_self_correcting_material_clock.py
```

Expected result: `12/12` checks pass, including 1,488 exact one-substitution
recovery rows and 176,256 herald-code covariance rows.
