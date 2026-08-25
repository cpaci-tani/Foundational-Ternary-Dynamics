# V3 charged-frame atomic syndrome-repair transaction v1

**Date:** 2026-08-24  
**Status:** **[THEOREM, CONDITIONAL ON THE SELECTED TRANSACTION — COMPLETE
ONE-DEFECT REPAIR/SYNDROME INJECTION]** +
**[THEOREM — EXACT FINITE INVERSE AND MOORE-LOCAL PLACEMENT]** +
**[THEOREM — CYCLIC-AXIS/C4-C3 FRAME-GAUGE DEFECT INDEX]** +
**[THEOREM, FORMAL — GENERALIZED RECORD/TOKEN COUNT CLOSURE]** +
**[SELECTION — TWO-SLOT A2 WORK PORT, CROSS-CARRIER TOKEN ROLE, AND
EIGHTEEN-RECORD CLOCK DEBIT]** +
**[REFLECTION CLOSURE — CONDITIONALLY CLOSED BY ORIENTED-HEADER SUCCESSOR]** +
**[OPEN — PHYSICAL ACTION, ORIENTED-CHART FORMATION/ADMISSION, ARBITRATION,
PHI INTEGRATION, AND REPEATED SURVIVAL]**  
**Scope:** all 37,632 registered one-coordinate defects of the 24 exact
charged circulation frames  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Decoder parent:**
[`THEOREM_V3_CHARGED_FRAME_UNIQUE_ONE_DEFECT_DECODER_v1.md`](THEOREM_V3_CHARGED_FRAME_UNIQUE_ONE_DEFECT_DECODER_v1.md)  
**Physical syndrome carrier:**
[`THEOREM_V3_NEUTRAL_SYNDROME_BUNDLE_CONVEYOR_v1.md`](THEOREM_V3_NEUTRAL_SYNDROME_BUNDLE_CONVEYOR_v1.md)  
**A2 work-port successor:**
[`THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md`](THEOREM_V3_CHARGED_FRAME_PAYLOAD_COMPLETE_A2_WORK_PORT_v1.md)  
**Exact certificate:**
[`proof_v3_charged_frame_atomic_syndrome_repair_transaction.py`](../../../../../scripts/proofs/proof_v3_charged_frame_atomic_syndrome_repair_transaction.py)  
**Reflection successor:**
[`THEOREM_V3_ORIENTED_REPAIR_CHART_FULL_OH_COVARIANCE_AND_PRICE_v1.md`](THEOREM_V3_ORIENTED_REPAIR_CHART_FULL_OH_COVARIANCE_AND_PRICE_v1.md)

---

## 1. Closed question

The decoder and syndrome-carrier theorems previously left their connecting
vertex open. The present construction closes the finite kinematic question:

> Can one registered malformed charged frame be replaced by its exact next
> frame while an existing-carrier neutral bundle leaves with the unique defect
> syndrome, all inside one Moore-local transaction with an exact inverse?

For the selected transaction defined below, the answer is yes. This is not a
claim that the canonical common action already selects the transaction or
assigns its physical energy.

Let `C` be the 24 charged frames and let `D_x` be the 1,568-element defect
shell of `x in C`. The decoder supplies an intrinsic descriptor map

\[
 d_x:D_x\longrightarrow\{1,\ldots,1568\},              \tag{1}
\]

which is bijective for every parent. Code `0` denotes the ready environment.
Every code `0,...,1568` is represented by one eighteen-record neutral syndrome
bundle from the existing 384-channel site field bank.

---

## 2. Intrinsic repair geometry

Write the oriented plane basis of a charged frame as `(a,b,c)`, with

\[
 c=a\times b.                                           \tag{2}
\]

The boundary role whose C4 payload phase is zero selects an intrinsic anchor
`z`: the tail of that role. Place the ready syndrome bundle at

\[
 p_{\rm in}=z+c,
 \qquad
 p_{\rm out}=z+2c.                                     \tag{3}
\]

Its state-only header is chosen so that its rotor successor is `c`. Thus the
normal direction, input port, and outgoing port are read from the presented
frame and bundle; no coordinate origin, external target, or fitted direction
enters.

The four plaquette vertices and both ports lie in the single Moore cube
centered on `p_in`:

\[
 \max_j|y_j-(p_{\rm in})_j|\le1.                        \tag{4}
\]

Consequently the complete read/write support is radius one even though the
outgoing port is two normal steps from the intrinsic anchor.

---

## 3. Selected atomic map and exact inverse

For a defect `y in D_x`, ready syndrome `S_0`, and one formal work token, the
selected transaction is

\[
 \boxed{
 (y,p_{\rm in},S_0,1)
 \longmapsto
 (\Phi x,p_{\rm out},S_{d_x(y)},w').}                  \tag{5}
\]

Here `Phi x` is the already certified exact next charged frame and
`S_d` is the physical eighteen-record syndrome codeword. The bundle is stalled
internally while moving from `p_in` to `p_out`, using the conveyor theorem's
selected whole-bundle clock debit.

Equation (1) makes equation (5) injective. From the output one recovers
`x=Phi^{-1}(Phi x)`, reads the syndrome index, and uses the unique decoder row
to reconstruct `y`. The certificate exhausts all 37,632 inputs and proves that
no two have the same complete output. This is retained inverse information,
not erasure disguised as repair.

The syndrome remains electromagnetically neutral on every C3 layer:

\[
 E(S_d)=B(S_d)=0.                                      \tag{6}
\]

It therefore does not alter the charged object's additive `E/B` source merely
by recording which repair occurred.

---

## 4. Exact generalized token ledger

Define the object-record count

\[
 N_{\rm obj}=N_{\rm relation}+N_{\rm field}             \tag{7}
\]

and

\[
 \Delta=N_{\rm obj}(x)-N_{\rm obj}(y)\in\{-1,0,+1\}.
                                                               \tag{8}
\]

With one work token at the input, set

\[
 \boxed{w'=1-\Delta\in\{0,1,2\}.}                     \tag{9}
\]

Because every syndrome bundle contains eighteen records, equations (7)--(9)
give the exact formal identity

\[
 N_{\rm obj}(y)+18+1
 =N_{\rm obj}(\Phi x)+18+w'.                           \tag{10}
\]

The exhaustive census is:

| defect class | rows | `Delta` | input work | output work |
|---|---:|---:|---:|---:|
| missing relation or field record | 480 | `+1` | 1 | 0 |
| payload/layer substitution | 672 | `0` | 1 | 1 |
| extra field record | 36,480 | `-1` | 1 | 2 |

The selected finite inventory already contains plaquette A2 A9 slots, so two
chosen exclusion slots have enough occupancy capacity to represent `w'=0,1,2`
without adding an unbounded register or a new primitive carrier type.

This statement is deliberately only a **generalized count closure**. The A2
work-port successor now realizes `w'=0,1,2` in two existing nine-state A9
slots, retains complete work phase/polarity plus the full defect syndrome, and
makes the registered repair sections an exact finite bijection. Under a
selected equal-occupancy metric it also conserves a finite positive energy.
The common action still has not derived equality of the carrier energies,
their absolute multiplier, or the bundle clock-debit work.

---

## 5. Frame-gauge covariance actually proved

Defect coordinates are expressed in the intrinsic `(a,b,c)` basis, phases are
measured relative to the parent C4 offset, and polarities relative to the
parent charge. One further normalization is necessary: the charged scheduler
identifies plane family `f` with local C3 layer `f`. Returning it to the
family-zero clock section requires

\[
 R^{4f},                                                \tag{11}
\]

because four internal Z12 ticks leave the C4 phase fixed and change the C3
layer by one.

With equation (11), the complete ordered descriptor set is identical under
the three proper cyclic permutations of the coordinate axes and under charge
conjugation. The isolated syndrome conveyor separately has full signed-cubic
covariance. The reflection successor identifies why this certificate stopped
there: `a x b` is axial and cannot itself be used as a polar repair-port
displacement. Using the existing ready header's polar rotor successor and the
orientation-corrected axial descriptor closes the selected repair section
under all 48 signed-cubic maps. Admission/formation of that enlarged oriented
chart in homogeneous `Phi` remains open.

---

## 6. Matter verdict

The previous abstract phrase “couple the decoder to the physical syndrome” is
now replaced by an explicit finite relation:

```text
registered one-defect frame
  + ready neutral bundle
  + one formal work token
    -> exact next charged frame
       + emitted physical syndrome
       + exactly balanced generalized token count.
```

This closes atomic kinematic repair, local syndrome emission, and finite
inverse retention for the registered shell. It does **not** establish stable
physical matter. The remaining gates are:

1. derivation of the selected equal-occupancy invariant and its absolute
   multiplier from the physical common action;
2. native admission and formation of the full signed-cubic oriented repair
   chart closed conditionally by the successor;
3. reciprocal clock-debit work and native formation of the ready bundle and
   funded work port;
4. arbitration with ordinary fields and simultaneous repair events;
5. integration into the homogeneous candidate `Phi`; and
6. repeated perturbation/scattering survival with mass and dispersion.

---

## 7. Reproduction

```bash
python scripts/proofs/proof_v3_charged_frame_atomic_syndrome_repair_transaction.py
```

Expected result: `12/12` exact checks pass, with 37,632 injective repair rows
and work census `{(+1,0):480, (0,1):672, (-1,2):36480}`.
