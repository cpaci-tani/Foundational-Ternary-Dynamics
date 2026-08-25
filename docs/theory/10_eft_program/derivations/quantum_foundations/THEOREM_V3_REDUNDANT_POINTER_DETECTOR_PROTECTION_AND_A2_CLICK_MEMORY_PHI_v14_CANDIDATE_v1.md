# V3 redundant pointer-detector protection and A2 click memory Phi-v14 candidate v1

**Date:** 2026-08-25  
**Status:** **[SELECTION — PREPARED PHI-v14 APPARATUS WRAPPER]** +
**[THEOREM, CONDITIONAL — EXACT ONE-COPY POINTER/DETECTOR SYMBOL BASIN]** +
**[THEOREM — FINITE PERSISTENT OUTCOME MEMORY AND CLEAN INVERSE]** +
**[THEOREM — EXISTING-CARRIER 23-SITE MOORE-BLOCK REALIZATION]** +
**[BOUNDARY — FORMATION, RECIPROCAL DETECTOR WORK, MULTI-BLOCK TRAFFIC,
MACROSCOPIC AMPLIFICATION, AND LABORATORY BELL ROUTING OPEN]**  
**Additional carrier price:** three copies of each of the two existing neutral
pointers, three primitive detector trits, twelve existing fixed-occupancy A2
click counters, one A2 READ/COMMIT/work owner, and the prepared bank; 23 of 27
sites in one Moore block  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Source-bank parent:**
[`THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md`](THEOREM_V3_FINITE_SOURCE_HISTORY_BORN_BANK_FORMATION_PHI_v13_CANDIDATE_v1.md)  
**Apparatus parent:**
[`THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md`](THEOREM_V3_CONTEXTUAL_NEUTRAL_POINTER_BORN_RENEWAL_APPARATUS_v1.md)  
**Physical-counter parent:**
[`THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md`](../gravity_cosmology/THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md)  
**Certificate:**
[`proof_v3_redundant_pointer_detector_protection_and_a2_click_memory_phi_v14_candidate.py`](../../../../../scripts/proofs/proof_v3_redundant_pointer_detector_protection_and_a2_click_memory_phi_v14_candidate.py)

---

## 1. The apparatus state is finite and reconstructible

The prepared v3 apparatus uses two contextual pointer cycles of coprime
lengths

\[
 L_1=384,
 \qquad
 L_2=385.                                             \tag{1}
\]

Their joint pointer has period

\[
 L=L_1L_2=147{,}840.                                  \tag{2}
\]

At every step, the two physical pointer residues

\[
 (a,b)=(p\bmod384,p\bmod385)                          \tag{3}
\]

determine one unique `p` by the Chinese remainder theorem. Explicitly,

\[
 p=a+384\,[384(b-a)\bmod385].                         \tag{4}
\]

The certificate exhausts all 147,840 residue pairs in the joint orbit and
recovers the exact pointer. No unbounded counter or external time index is
required.

Each 384-state pointer address is already one zero-`E/B` opposite-polarity
field pair plus one A9 polarity token. The 385th state is the existing blank
delay configuration. Phi-v14 places three copies of each pointer on distinct
sites and three copies of the primitive detector trit on three more sites.

---

## 2. Two-layer protected apparatus transaction

One existing A2 owner carries the layer `READ/COMMIT` and generic correction
work bit. On READ:

1. strict-majority decode the three left-pointer copies;
2. strict-majority decode the three right-pointer copies;
3. strict-majority decode the three detector copies;
4. fail closed if any register has no majority;
5. repair at most one total copy mismatch; and
6. retain work `w=1` if a repair occurred.

On COMMIT, the rule reconstructs the joint pointer using equation (4), applies
the exact parent apparatus permutation, and writes three copies of its output.
Thus every protected macro-step equals one parent apparatus step:

\[
 \pi\Phi_{14}^2(X)=\Phi_{\rm app}\pi(X),              \tag{5}
\]

where `pi` decodes the copied registers.

The proof exhausts all

\[
 147{,}840\times3=443{,}520                            \tag{6}
\]

clean pointer/detector states. Every transition has the exact parent inverse,
including its click-memory update.

---

## 3. Complete one-copy perturbation basin

The three repetition alphabets have sizes 384, 385, and 3. Replacing one of
three copies by any other valid symbol gives

\[
 3[384(383)+385(384)+3(2)]
 =884{,}754                                            \tag{7}
\]

registered perturbations. Every row repairs on READ, retains generic work,
and then produces the same pointer/detector output as the clean apparatus.

The repair is genuinely noninjective: different erroneous symbols collapse
to the same corrected copied state. The detailed error identity expires, but
the A2 work consequence survives. A busy work port and a three-distinct-symbol
no-majority state fail closed.

This is a snapshot basin. Two coherent copy substitutions can redirect a
majority, and a new fault arriving between READ and COMMIT is not covered.

---

## 4. Persistent physical click memory

The twelve tangent/polarity outcomes each own one existing fixed-occupancy A2
signed counter. At a genuine bright transition

```text
READY -> MANIFESTED
```

the counter associated with the current physical pointer outcome increments
by one. Detector recovery does not clear it.

For every Phi-v13 source-formed phase-count vector,

\[
 Z=(N_0-N_2)+i(N_1-N_3),                              \tag{8}
\]

and one complete pointer enumeration writes

\[
 C=|Z|^2                                              \tag{9}
\]

into the corresponding A2 counter. The certificate checks all 151 attainable
source-history count vectors. Since the source window contains eight records,

\[
 0\le C\le64,                                         \tag{10}
\]

well inside the existing exact A2 range.

The click record is not biological memory and it is not an epistemic tally
kept by the experimenter. It is a surviving finite physical state at the
apparatus. Its detailed event order need not survive for equation (9) to
remain a consequence.

---

## 5. Full-cycle forward and reverse witness

For the registered count fixture `(8,1,3,0)`,

\[
 |Z|^2=(8-3)^2+(1-0)^2=26.                            \tag{11}
\]

The operational pointer/detector orbit has

\[
 147{,}840+2(26)=147{,}892                            \tag{12}
\]

parent apparatus steps. After the protected forward cycle:

1. both pointer residues return to their initial values;
2. the detector returns READY;
3. the outcome A2 counter contains exactly 26; and
4. all other outcome counters remain zero.

Running the exact clean inverse for the same number of steps restores every
counter to zero and the complete initial apparatus state. Persistent memory
therefore does not require fundamental irreversibility. It persists because
the physical counter state persists, and it disappears only under an actual
inverse/reset transaction.

---

## 6. Existing-carrier and covariance census

The apparatus block uses:

| Role | Sites |
|---|---:|
| prepared field bank | 1 |
| three left-pointer copies | 3 |
| three right-pointer copies | 3 |
| three detector copies | 3 |
| twelve outcome A2 counters | 12 |
| READ/COMMIT/work A2 owner | 1 |
| **Total** | **23** |

Thus the complete prepared apparatus fits inside one 27-site Moore block with
four sites unassigned. The source-formation block need not coexist in that
same block; causal transfer and arbitration between them remain open.

Transforming the oriented apparatus chart transforms its 384-address order
at the same indices. All three copies transform together, while the detector
and counter values are intrinsic. The certificate checks 18,432 signed-cubic
pointer rows.

---

## 7. What has closed

Conditional on a prepared source bank and oriented apparatus chart, Phi-v14
supplies:

1. exact reconstruction of all 147,840 joint pointer states;
2. a 23-site existing-carrier apparatus realization;
3. a two-layer wrapper reproducing all 443,520 parent states;
4. exact clean inverse including click memory;
5. a complete 884,754-row one-copy valid-symbol perturbation basin;
6. generic correction-work retention and fail-closed busy/no-majority states;
7. one persistent physical A2 outcome counter per local port;
8. exact `C=|Z|^2` memory for all 151 Phi-v13 source-formed bank classes;
9. full forward/reverse event-cycle closure; and
10. signed-cubic covariance with the contextual apparatus chart.

Reproduce with:

```powershell
python scripts/proofs/proof_v3_redundant_pointer_detector_protection_and_a2_click_memory_phi_v14_candidate.py
```

Expected result: `13/13` exact checks pass.

---

## 8. What remains open

This theorem does not yet derive:

1. formation and protection of the source, controller, chart, reserve, or
   apparatus block itself;
2. faults in bank occupancy, two coherent copy substitutions, or faults
   between READ and COMMIT;
3. reciprocal detector work, heat, material recoil, or source backreaction;
4. traffic arbitration between source, apparatus, and event-halo blocks;
5. indefinite counter renewal, overflow handling, or macroscopic
   amplification;
6. causal bipartite source splitting and owned paired-trial routing;
7. recovery of laboratory Bell correlations; or
8. integration into canonical homogeneous `Phi`.

The correct conclusion is:

> FTD now has an exact finite source-history bank, a fault-protected pair
> enumerator, and a surviving physical `|Z|^2` outcome record. It does not yet
> have a formed laboratory apparatus or a theorem for reciprocal measurement
> work and multipartite routing.

The later
[`Phi-v15 transitive scheduler successor`](THEOREM_V3_TRANSITIVE_A2_SOURCE_HISTORY_ODOMETER_BORN_TIME_MEASURE_PHI_v15_CANDIDATE_v1.md)
presents this apparatus with all ordered pairs of Phi-v13 histories in one
exact two-A2 odometer and pads every trial to the same 148,096 apparatus
macros. It thereby supplies one selected physical trial-clock measure. The
source and apparatus require separate blocks, and full-cycle clicks exceed
the finite local counters, so causal routing, reciprocal work, and record
export remain open.
