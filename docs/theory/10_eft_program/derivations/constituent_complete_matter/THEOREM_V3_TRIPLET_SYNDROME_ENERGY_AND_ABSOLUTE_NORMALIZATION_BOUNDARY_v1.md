# V3 triplet syndrome energy and absolute normalization boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — ADDITIVE OCCUPANCY DEFECT RANK ZERO]** +
**[THEOREM — POSITIVE SYNDROME REPAIR REQUIRES WORK EXPORT]** +
**[THEOREM — ISOTROPIC SOURCE SHAPE DOES NOT FIX RESPONSE SCALE]** +
**[OPEN — RELATIONAL ACTION CURVATURE, WORK CARRIER, AND ABSOLUTE
NORMALIZATION]**  
**Matter-clock parent:**
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_triplet_syndrome_energy_normalization_boundary.py`](../../../../../scripts/proofs/proof_v3_triplet_syndrome_energy_normalization_boundary.py)

---

## 1. Exact flat direction

The self-correcting triplet has three occupied SC A9 arm tokens and three
constant-occupancy neutral field-pair herald registers. Every clean state and
every one of the 1,488 registered valid-symbol substitutions therefore has the
same role-count vector

\[
 (N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(6,3,0,0).       \tag{1}
\]

All defect vectors are zero, so their span has rank zero. Consequently every
homogeneous phase-blind additive role energy

\[
 H_{\rm add}=\Gamma_F N_F+\Gamma_{SC}N_{A1,SC}
 +\Gamma_{FCC}N_{A1,FCC}+\Gamma_{A2}N_{A2}          \tag{2}
\]

assigns exactly the same value to the codeword and every registered error.
On the selected all-equal ray this flat value is

\[
 \boxed{H_{\rm occ}=9\Gamma.}                       \tag{3}
\]

Thus the occupancy invariant conserves the repair but cannot power it, bind
the triplet, or supply a restoring energy. Kinematic error correction is not
yet physical stability.

---

## 2. Relational curvature and the work requirement

The equality relations between the three arm copies and between the three
herald copies uniquely decode every registered error. A relational syndrome
functional can therefore assign

\[
 H_{\rm syn}=\epsilon_{\rm syn}d_H(X,\mathcal C),   \tag{4}
\]

where `C` is the clean orbit. Every radius-one error has gap
`epsilon_syn>0`, while the repaired state has zero syndrome.

For a noninjective repair, exact energy balance is then

\[
 9\Gamma+\epsilon_{\rm syn}
 =9\Gamma+W_{\rm export},
 \qquad
 \boxed{W_{\rm export}=\epsilon_{\rm syn}.}         \tag{5}
\]

There are only two conservative branches:

1. `epsilon_syn=0`: the current correction is exact but energetically flat;
2. `epsilon_syn>0`: a surviving physical work/field record must receive the
   exact syndrome energy.

Letting the distinction expire does not authorize deleting physical energy.
Conversely, no Landauer heat term follows merely from the noninjective map;
the heat/work question is equation (5), not the abstract loss of a label.

The coefficient `epsilon_syn` is not fixed by the finite code or its repair
map. Adding (4) without deriving its coefficient from the common transaction
action would be another selected scale.

---

## 3. Absolute coupling remains free

The triplet fixes the cubically isotropic source **shape**

\[
 S=-{I_3\over36}.                                  \tag{6}
\]

But the physical response is `g_response S`. The finite repair graph reads
neither `Gamma`, `epsilon_syn`, nor `g_response`; multiplying any of them by a
positive scale leaves every transition and recovery identity unchanged.

The clean period-16 triplet also has zero complete field-packet debit. It
therefore cannot by itself instantiate the finite-clock normalization formula
`3w/(dT)`, whose physical branch requires `d>0`. Clock recurrence and source
isotropy alone do not normalize light-matter or matter-gravity coupling.

---

## 4. Next constructive gate

The next common-action candidate must add no empirical target and must do all
of the following together:

1. derive a positive relational syndrome curvature from existing finite
   interactions rather than declaring `epsilon_syn`;
2. route exactly that energy into an existing local work carrier during
   noninjective repair;
3. return or disperse the work record causally on the inverse/relaxation path;
4. preserve the charged Gauss/current and gravity-source ledgers; and
5. show that the same curvature controls a blocked source-response residue.

Only such a common construction could begin to relate binding, stable matter,
coupling normalization, and gravity rather than assigning separate constants
to them.

---

## 5. Reproduction

```bash
python scripts/proofs/proof_v3_triplet_syndrome_energy_normalization_boundary.py
```

Expected result: `13/13` checks pass over all 1,488 registered errors.
