# V3 field-bank Gaussian/Born readout v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT FINITE C4/GAUSSIAN-INTEGER READOUT]** +
**[THEOREM, CONDITIONAL — PREPARED PHASE-PAIR COUNTS HAVE BORN FORM]** +
**[OPEN — NATIVE PREPARATION, PHYSICAL TRIALS, APPARATUS, AND MULTIPARTITE
COMPOSITION]**  
**Carrier price:** none; the selected 384-channel v3 site bank  
**Production status:** unchanged  
**Ledger status:** no row minted

**Renewal/apparatus successor:**
[`THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md`](THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md)

**Exact certificate:**
[`proof_v3_field_bank_gaussian_born_readout.py`](../../../../../scripts/proofs/proof_v3_field_bank_gaussian_born_readout.py)
passes 11/11 gates and 18,355 exact counting rows. It performs no stochastic
sampling, fit, target-probability comparison, or numerical near-match search.

---

## 1. Native finite amplitude coordinate

One native v3 outcome port is a directed SC tangent and a polarity. There are
therefore twelve such ports. For a fixed port and finite block, let

\[
 (N_0,N_1,N_2,N_3)
\]

count occupied channels at the four existing C4 phases. The complete bank
retains every record; the blocked complex response is the Gaussian integer

\[
 \boxed{Z=(N_0-N_2)+i(N_1-N_3).}
\]

This is a finite integer readout, not an ontic complex amplitude. Every site
supplies eight normal/hand channels for each port and phase, so every finite
Gaussian integer has a finite spatial realization.

---

## 2. Cancellation and the square

Opposite phases are paired into retained dark records. The canonical residual
counts are

\[
 \big((\Re Z)^+,(\Im Z)^+,(-\Re Z)^+,(-\Im Z)^+\big).
\]

Only one sign survives on each of the real and imaginary rails. The number of
ordered phase-compatible pairs is therefore

\[
 B=(\Re Z)^2+(\Im Z)^2=\boxed{|Z|^2}.
\]

Adding any number of opposite-phase dark pairs changes neither `Z` nor `B`.
A common C4 rotation sends `Z -> iZ` and also leaves `B` unchanged. Polarity
conjugation exchanges complete outcome-port copies without changing their
phase count.

For a prepared finite family of ports,

\[
 \boxed{
 f_o={B_o\over\sum_rB_r}
 ={|Z_o|^2\over\sum_r|Z_r|^2}.}

Thus the Born square is literally the positive cardinality of compatible
ordered residual-history pairs after C4 cancellation. No continuous
wavefunction or fundamental random draw is used.

---

## 3. What this advances

The earlier C4 Born theorems established the prepared counting identity on
abstract finite banks and selected detector permutations. This result anchors
the amplitude and outcome coordinates directly in the selected v3 carrier:

```text
finite v3 field records
  -> C4 cancellation
  -> Gaussian-integer block response Z
  -> compatible ordered-pair cardinality |Z|^2
  -> normalized prepared frequency
```

The result supports the ontology in which probability summarizes incomplete
knowledge of deterministic finite histories. It does not make the epistemic
ensemble ontic.

---

## 4. Remaining physical Born debt

The renewal/apparatus successor conditionally closes target-blind complete
pair enumeration, a reusable ternary detector, one-pair/one-event assignment,
and an exact finite heralded-event-window bound using existing v3 carrier
states. The theorem and its successor still do not establish that `Phi`:

- forms a desired residual bank from an emitted source;
- forms and protects the selected contextual pointer apparatus;
- amplifies and retains one exclusive detector record with reciprocal
  backreaction;
- identifies one source emission with one prepared-bank traversal;
- builds a stable apparatus record with reciprocal backreaction; or
- composes spacelike multipartite apparatuses with operational no-signalling.

The later
[`finite source-history Phi-v13 successor`](THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md)
closes one finite bank-formation branch: all 4,096 eight-tick A9
source/controller histories write their actual phase visits into physical
field records with exact A2-reserve conservation and a retained-history
inverse (12/12). It does not form or protect the source/controller/chart or
apparatus, renew an unbounded source stream, or close laboratory routing.

Therefore this is a native **readout and prepared-counting** theorem, not the
general physical Born rule. Its next gate is shared with coupling
normalization: derive the physical finite-history measure/cadence from the
same source-bearing `Phi` extension rather than selecting it independently.
