# V3 source-orbit coupling-normalization obstruction v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT POSITIVE ACTION-PRICE ORBIT ON THE DRESSED SOURCE
CYCLE]** + **[SCOPED NO-GO — SOURCE PERIOD, CHANNEL COUNT, GAUSS, AND PREPARED
BORN COUNTS DO NOT FIX PHYSICAL COUPLING]** + **[OPEN — COMMON BLOCKED-HISTORY
CURVATURE]**  
**Production status:** unchanged  
**Ledger status:** no row minted

**Exact certificate:**
[`proof_v3_source_orbit_coupling_normalization_obstruction.py`](../../../../../scripts/proofs/proof_v3_source_orbit_coupling_normalization_obstruction.py)
passes 11/11 symbolic and finite-cycle gates without target data.

---

## 1. Everything the source orbit fixes

For every direction, polarity, and starting C3 layer, the candidate dressed
source has:

- exact period 24;
- three activations and three withdrawals per period;
- eighteen noncrossing ticks;
- primary ownership for one half of the uniform time orbit;
- eight bound field channels while primary-owned;
- exact charge continuity; and
- exact Gauss preservation.

These facts fix the finite state trajectory and its counting observables.

---

## 2. What they do not fix

Assign a positive action price `w_cross` to each ownership crossing and
`w_hold` to each noncrossing tick. The complete orbit action is

\[
 S_{24}=6w_{\rm cross}+18w_{\rm hold}.
\]

Every positive pair gives the same deterministic orbit. Reversibility can
identify forward and reverse crossing prices but does not set their common
magnitude. Even the strongest equal-price convention leaves

\[
 S_{24}=24w,
 \qquad w>0.
\]

Likewise, the fact that activation creates eight occupied field channels fixes
a multiplicity, not the action per channel. Writing `Gamma=8 eta` merely moves
the unknown scale into `eta`.

Consequently the conditional native coupling remains

\[
 \boxed{
 \alpha_{\rm native}
 ={\Gamma\over4\pi I_*c_{\rm eff}}
 ={3\Gamma\over2\pi I_*},}
\]

with `Gamma/I_*` undetermined. Distinct positive values produce identical
`Phi` histories, source period, Gauss identities, stability predicate, and
prepared Born cardinalities while producing distinct conditional couplings.

---

## 3. Consequence

Neither of the tempting new shortcuts works:

- `8` is the dressing multiplicity, not the fine-structure coupling;
- `1/2` is the primary dwell fraction, not the coupling;
- `24` is the source/C3 recurrence period, not an action quantum; and
- `|Z|^2` event cardinalities do not price field curvature.

Physical coupling normalization can close only if the common finite-history
object independently fixes the blocked curvature

\[
 \chi_{\rm EM}=\Gamma/I_*
\]

and the same coefficient is recovered from both the free-field Hessian and a
charged static residue. Until then, comparison with the master root is
forbidden.

