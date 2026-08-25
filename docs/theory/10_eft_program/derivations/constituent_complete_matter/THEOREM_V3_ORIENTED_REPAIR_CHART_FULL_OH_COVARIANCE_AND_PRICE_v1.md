# V3 oriented repair chart full-O_h covariance and price v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON THE SELECTED ORIENTED-HEADER CHART —
FULL SIGNED-CUBIC REPAIR COVARIANCE]** +
**[THEOREM — EXISTING READY HEADER SUPPLIES THE POLAR REPAIR NORMAL]** +
**[SELECTION — ENLARGED ORIENTED FRAME ADMISSION, NOT CANONICAL PHI]** +
**[OPEN — FORMATION, WRITER ARBITRATION, ACTION, PHI, AND SURVIVAL]**  
**Scope:** the complete 37,632-row registered repair shell, its 1,569-state
syndrome codebook, and payload-complete A2 work port  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Atomic parent:**
[`THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md`](THEOREM_V3_CHARGED_FRAME_ATOMIC_SYNDROME_REPAIR_TRANSACTION_v1.md)  
**Work-port parent:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Exact certificate:**
[`proof_v3_oriented_repair_chart_full_oh_covariance_and_price.py`](../../../../../scripts/proofs/proof_v3_oriented_repair_chart_full_oh_covariance_and_price.py)

---

## 1. The reflection defect was a polar/axial mismatch

The original repair chart used

\[
 n_{\rm old}=a\times b                                \tag{1}
\]

both as the plaquette normal and as the spatial direction of the syndrome
port. For polar edge directions `a,b`, equation (1) is axial. Under an
improper signed-cubic map `M`,

\[
 (Ma)\times(Mb)=\det(M)M(a\times b),                  \tag{2}
\]

whereas a physical displacement must transform as `Mn_old`. Thus the old
proper-rotation proof could not simply be extended to reflections. The gap
was representational and exact; it was not evidence of dynamical instability.

The ready syndrome bundle already contains a unique header. Its rotor
successor

\[
 r=\operatorname{succ}(h)                             \tag{3}
\]

is a **polar** SC direction and transforms as `r -> Mr` under all 48
signed-cubic maps. Select `r`, rather than `a x b`, as the repair-port normal.
For the original positive chart they agree; an improper image selects the
opposite axial side automatically through the transformed header.

No new bit or primitive carrier is added. The header was already required by
the reversible syndrome bundle.

---

## 2. Oriented existing-carrier chart

Define the finite chart

\[
 C=(x_0,a,b,r,q,k,\epsilon),                           \tag{4}
\]

where `x_0` is the plaquette origin; `a,b,r` are an orthonormal **polar**
basis; `q in Z3` is the existing site layer; `k in Z4` is the frame offset;
and `epsilon in {+1,-1}` is the existing polarity. The charged frame uses the
same four A1 relations and sixteen field records as before. The ready header
selects `r`.

The full orbit of the original 24 frames contains:

| object | exact count |
|---|---:|
| charged-frame presentations | 576 |
| combined frame-plus-header charts | 1,152 |

The factor two is physical bookkeeping, not a new type: one charged-frame
presentation admits the two existing header-defined repair sides. The
combined state `(charged presentation, ready header)` distinguishes all
1,152 charts state-only.

The certificate verifies 55,296 chart/group rows:

\[
 M\,P(C)=P(MC),\qquad
 M\,P(C^+)=P((MC)^+),                                 \tag{5}
\]

where `P` is the finite carrier presentation and `+` is the period-four
successor. Thus the enlarged selected frame family and its successor close
under all of `O_h`.

---

## 3. Reflection-safe intrinsic descriptors

Let

\[
 \omega_C=\det[a\ b\ r]\in\{-1,+1\}.                 \tag{6}
\]

Tangent vectors are polar, while channel normals are axial and Hodge
handedness is pseudoscalar. The intrinsic channel descriptor therefore uses

\[
 \bigl([t]_C,\;\omega_C[n]_C,\;omega_C h,\;
       k_{\rm channel}-k,\;\epsilon_{\rm channel}\epsilon\bigr). \tag{7}
\]

Every component of equation (7) is invariant under the simultaneous
signed-cubic action on the record and chart. This is the missing determinant
factor in the earlier proper-rotation-only descriptor.

Two adjacent axis swaps plus one axis sign reflection generate the complete
48-element signed-cubic group. Exhaustive evaluation of those generators on
all 37,632 registered defects gives

\[
 37{,}632\times3=112{,}896                             \tag{8}
\]

exact descriptor rows. Every transformed defect retains the same intrinsic
index.

---

## 4. Complete repair covariance

The deterministic codebook chooses its header and fifteen orbit
representatives by the reflection-safe intrinsic ordering. The certificate
checks all

\[
 24\times3\times1{,}569=112{,}968                     \tag{9}
\]

generator/codeword rows. Ready word and every defect syndrome transform to
the word with the same intrinsic index.

For every row in equation (8), it also verifies:

1. input and output repair ports transform as the polar displacements
   `x_anchor+r` and `x_anchor+2r`;
2. the complete two-slot A2 phase/polarity work payload is unchanged in the
   intrinsic chart;
3. the exact repaired next frame transforms to the next oriented frame; and
4. the group generators close to all 48 signed-cubic maps.

Therefore the selected complete repair section has full `O_h` covariance.
The earlier reflection gap is conditionally closed without a new carrier
type.

---

## 5. Exact price and remaining matter gate

The theorem does **not** show that canonical `Phi-v2`, or the charged Phi-v3
candidate, admits and forms all 1,152 combined charts. It selects a larger
state-only frame recognizer and uses a ready syndrome header whose native
formation is still absent.

Remaining stable-matter debts are:

1. native formation of the oriented frame plus ready polar header;
2. integration with charged-Phi writer priority;
3. parallel repair, overlapping-event, and ordinary-field arbitration;
4. common-action provenance and absolute normalization;
5. reciprocal work for the eighteen-record syndrome clock debit; and
6. repeated environmental survival, scattering, mass, and dispersion.

This is consequently a symmetry closure of the finite repair transaction,
not yet a stable-particle theorem.

The
[`matter-anchored event-seam successor`](../common_action_mechanics_reciprocity/THEOREM_V3_MATTER_ANCHORED_BORN_GAUSS_GRAVITY_EVENT_SEAM_v1.md)
uses the same exact frame/header chart as a physical contextual apparatus and
proves that exact-apparatus, forward-repair, and retained-syndrome reverse-
repair admissions are state-disjoint across all 37,632 registered defects.
That closes one local schedule collision; native chart formation and repeated
environmental survival remain open.

---

## 6. Reproduction

```bash
python scripts/proofs/proof_v3_oriented_repair_chart_full_oh_covariance_and_price.py
```

Expected result: `12/12` checks pass, with 1,152 combined charts, 55,296
chart/group rows, 112,968 codeword rows, and 112,896 defect-generator rows.
