# V3 two-record full STF tensor-carrier boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — FULL COMPOSITE $E_g\oplus T_{2g}$ READOUT]** +
**[THEOREM — RANK-TEN C4 TENSOR DOUBLET]** +
**[SCOPED NO-GO — SELECTED COLLISION DOES NOT PROTECT THE COMPOSITE]** +
**[OPEN — TENSOR SLOW MODE, POLE, CONSTRAINTS, UNIVERSAL COUPLING, CONE, AND
LENSING]**  
**Carrier price:** no new primitive type at composite-readout level  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Exact certificate:**
[`proof_v3_two_record_full_stf_tensor_carrier_boundary.py`](../../../../../scripts/proofs/proof_v3_two_record_full_stf_tensor_carrier_boundary.py)

---

## 1. The one-record obstruction is not the final carrier boundary

Let two simultaneous existing field records have layer readouts

\[
 (E_1,B_1),\qquad(E_2,B_2).
\]

Define their symmetric trace-free cross stress

\[
 \Sigma_{12}=
 E_1E_2^{\mathsf T}+E_2E_1^{\mathsf T}
 +B_1B_2^{\mathsf T}+B_2B_1^{\mathsf T}
 -{2\over3}(E_1\cdot E_2+B_1\cdot B_2)I.              \tag{1}
\]

The previous one-record stress contains only diagonal $E_g$. Exact census of
all 18,336 distinct two-record states instead gives

\[
 \operatorname{rank}\operatorname{span}\{\Sigma_{12}\}=5
 =2+3                                                   \tag{2}
\]

on every C3 layer, with diagonal rank two and off-diagonal rank three. Thus
the finite image is the complete cubic spatial STF representation

\[
 \boxed{E_g\oplus T_{2g}.}                             \tag{3}
\]

There are 43 distinct tensor values per layer. Their complete finite image is
covariant under all 48 signed-cubic transformations.

---

## 2. Clock and quadrature structure

Restricting to pairs whose records have the same C4 phase, use that common
phase for the real/imaginary tensor rails. The exact span has rank ten:

\[
 (E_g\oplus T_{2g})\otimes\mathbb R^2_{C4}.            \tag{4}
\]

Advancing both record flags and their C4 phases while decrementing the site
C3 layer carries equation (1) covariantly. The complete composite tensor is
therefore available inside the existing v3 clocked field bank without a new
primitive tensor record.

This closes only the **representation availability** part of the gravity
carrier question. A composite observable is not automatically a slow field.

---

## 3. Selected-collision obstruction

The frozen Phi-v2 collision is an exact fixed-point-free involution on all
18,336 distinct record pairs and preserves record number plus total electric
and magnetic field. Comparing equation (1) before and after that selected
collision gives, on each of the three C3 layers,

\[
 \boxed{192\ \text{input pairs change their composite STF tensor}.} \tag{5}
\]

Hence number plus $(E,B)$ conservation does not protect the full composite
tensor. The result refines the prior boundary:

```text
one-record full STF carrier: absent
two-record full STF readout: present
clock-covariant C4 tensor doublet: present
selected collision invariant: absent
massless constrained tensor pole: absent
```

---

## 4. Gravity status

The carrier branch no longer requires a new *primitive* type merely to write
all five spatial STF components. It still requires a target-blind common law
that makes the composite a protected infrared sector. Such a law must then:

1. produce a massless tensor pole rather than a two-particle continuum;
2. remove scalar and vector contamination with local constraints;
3. couple to the same conserved source stress as charged matter;
4. share the matter/radiation cone;
5. give equal response of material and light channels, including lensing; and
6. only then support a nonlinear self-coupling/bootstrap argument.

The theorem is therefore positive at composite-carrier level and negative for
the selected collision. It is not evidence that a graviton has emerged.
