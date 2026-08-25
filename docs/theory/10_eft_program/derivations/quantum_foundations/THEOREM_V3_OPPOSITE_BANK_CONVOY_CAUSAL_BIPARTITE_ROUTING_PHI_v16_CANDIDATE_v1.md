# V3 opposite-bank convoy causal bipartite routing Phi-v16 candidate v1

**Date:** 2026-08-25  
**Status:** **[SELECTION — PHI-v16 PREPARED OPPOSITE-BANK CONVOY]** +
**[THEOREM — EXACT RADIUS-ONE BANK/OWNERSHIP TRANSPORT AND TERMINAL
HANDOFF]** + **[THEOREM — RECORD-COMPLETE INVERSE, ROLE CONSERVATION, AND
SIGNED-CUBIC COVARIANCE]** + **[THEOREM, CONDITIONAL — COMMON PHYSICAL
ORIGINATION RECORD AND REMOTE-BANK-INDEPENDENT CAUSAL ROUTING]** +
**[OPEN — FORMATION, CANONICAL PHI, TRAFFIC, BANK-FAULT PROTECTION,
SETTINGS, BACKREACTION, AMPLIFICATION, AND LABORATORY BELL RECOVERY]**  
**Carrier price:** one existing fixed-occupancy A2 TRANSIT owner per moving
eight-record bank and one existing A2 ENDPOINT owner per prepared apparatus
terminus; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Scheduler parent:**
[`THEOREM_V3_TRANSITIVE_A2_SOURCE_HISTORY_ODOMETER_BORN_TIME_MEASURE_PHI_v15_CANDIDATE_v1.md`](THEOREM_V3_TRANSITIVE_A2_SOURCE_HISTORY_ODOMETER_BORN_TIME_MEASURE_PHI_v15_CANDIDATE_v1.md)  
**Prepared bipartite parent:**
[`THEOREM_V3_BIPARTITE_PREPARED_BORN_NO_SIGNALLING_AND_LOCAL_CHSH_BOUNDARY_v1.md`](THEOREM_V3_BIPARTITE_PREPARED_BORN_NO_SIGNALLING_AND_LOCAL_CHSH_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_opposite_bank_convoy_causal_bipartite_routing_phi_v16_candidate.py`](../../../../../scripts/proofs/proof_v3_opposite_bank_convoy_causal_bipartite_routing_phi_v16_candidate.py)

---

## 1. One physical pair record before separation

Phi-v15 retains one ordered two-A2 scheduler address

\[
 \lambda=(a_L,a_R)\in A_2\times A_2.                  \tag{1}
\]

The address is not created by comparing later detector records. It exists at
the source before either bank is formed and remains there until both deliveries
complete. Its two digits decode the two physical Phi-v13 source histories.

Phi-v16 selects two tangent/polarity ports

\[
 o_L=(d,\epsilon),qquad o_R=(-d,\epsilon),            \tag{2}
\]

and forms one eight-record bank on each. Their counts are

\[
 N^L=(N_0^L,N_1^L,N_2^L,N_3^L),qquad
 N^R=(N_0^R,N_1^R,N_2^R,N_3^R).                       \tag{3}
\]

Thus the paired trial has a genuine finite common origination record:

\[
 \boxed{
 \lambda\longmapsto({\cal B}_L,{\cal B}_R)
 \longmapsto(\text{left route},\text{right route}).}  \tag{4}
\]

Equation (4) is conditional on the selected scheduler/formation branch. It is
the ontic condition that would make later joint analysis meaningful. Two
unrelated banks combined only in an observer's notebook would not satisfy it.

---

## 2. Existing-carrier route tokens

Use three distinct phase states of one existing fixed-occupancy A2 owner:

```text
TRANSIT
ENDPOINT
DELIVERED
```

The moving owner and the eight field records form a convoy. A bank is locally
recognizable because all eight occupied records have the same outcome port
$(d,\epsilon)$. Its phase distribution is arbitrary within the 151 physically
formed Phi-v13 count classes.

For a clear SC neighbor in direction $d$, select the local swap

\[
 \boxed{
 ({\cal B},T)_x+(\varnothing,\varnothing)_{x+d}
 \longmapsto
 (\varnothing,\varnothing)_x+({\cal B},T)_{x+d}.}      \tag{5}
\]

Every field record and the occupied A2 TRANSIT owner move one lattice unit.
No phase, polarity, channel identity, or count changes. The inverse swaps the
same records back.

If the destination contains an ENDPOINT owner, the terminal handoff is

\[
 \boxed{
 ({\cal B},T)_x+(\varnothing,E)_{x+d}
 \longmapsto
 (\varnothing,E)_x+({\cal B},D)_{x+d}.}                \tag{6}
\]

Here $T,E,D$ denote TRANSIT, ENDPOINT, and DELIVERED. Equation (6) retains two
occupied A2 owners and all eight field records. Its explicit inverse restores
the pre-delivery sites exactly.

Mixed-port banks, occupied destinations, missing endpoint owners, and
malformed token states fail closed.

---

## 3. Exact census and covariance

The certificate exhausts all 4,096 scheduler histories on both ports. For all
8,192 banks it checks both equation (5) and equation (6), together with their
exact inverses. This gives

\[
 16{,}384                                             \tag{7}
\]

local move/delivery rows and the same number of inverse rows.

All 151 physically formed count classes are then routed in both directions at
distances

\[
 H\in\{1,2,7,37\}.                                    \tag{8}
\]

Every endpoint lies exactly $H$ SC hops from its route origin after $H$ route
ticks. The local induction is immediate: if the bank is at $x+nd$, equation
(5) places it at $x+(n+1)d$ and changes no internal record. Hence the result
holds for every declared finite $H$, conditional on a clear prepared corridor
and endpoint owner.

The complete local transaction commutes with all 48 signed-cubic chart
transformations. The certificate checks 7,248 covariance rows. The selected
axis is part of the transformed physical source/apparatus chart; it is not a
new globally privileged direction.

---

## 4. Born data survive transport exactly

Because equations (5)--(6) retain every channel identity,

\[
 N_k^{\rm delivered}=N_k^{\rm formed}                 \tag{9}
\]

for every phase. Therefore

\[
 Z^{\rm delivered}
 =(N_0-N_2)+i(N_1-N_3)=Z^{\rm formed},                \tag{10}
\]

and

\[
 \boxed{
 M^{\rm delivered}=|Z^{\rm delivered}|^2
 =|Z^{\rm formed}|^2.}                                \tag{11}
\]

No probability is transmitted. The convoy transports the finite records from
which the contextual apparatus deterministically enumerates $M$ compatible
events.

The route is local and wing-factorized. For a left bank ${\cal B}_L$, the
next left state depends only on ${\cal B}_L$, its local TRANSIT owner, and the
left destination. Replacing the remote bank cannot change the left route:

\[
 \boxed{
 R_L({\cal B}_L;{\cal B}_R)
 =R_L({\cal B}_L;{\cal B}'_R).}                        \tag{12}
\]

The certificate checks equation (12) on the full $151^2=22{,}801$ paired
count-class census. This is routing independence, not yet a measurement-
setting no-signalling theorem.

---

## 5. What common origination does—and does not—mean

The construction distinguishes three cases:

```text
same scheduler address, opposite routed descendants:
    physically paired in Phi-v16 candidate scope

separate source addresses with no shared retained record:
    not one ontic pair merely because an observer groups them

four setting contexts formed by one explicit common source protocol:
    may be aggregated, but their statistics remain constrained by the
    actual local/contextual response law
```

Phi-v16 supplies the first case. It does not license the claim that every
historical or laboratory pairing has this ontology. Conversely, once equation
(1) is physically retained, dismissing the paired data as a purely epistemic
association would be incorrect.

The prepared local CHSH parent still applies. If settings are independent of
the retained common source and both wings have complete local deterministic
responses, then

\[
 |S|\le2.                                               \tag{13}
\]

The route cannot produce a Bell violation by itself. Any alternative FTD
account must put setting dependence, context-dependent source sectors,
incomplete retention, or another nonfactorizable physical mechanism into the
finite state and test it empirically.

---

## 6. Exact remaining laboratory boundary

Closed at prepared finite scope:

1. two opposite outcome-port banks formed from one retained pair address;
2. one-hop Moore-local movement of all bank records and route ownership;
3. arbitrary declared finite route length by exact local induction;
4. terminal delivery and exact inverse;
5. exact role-count conservation;
6. full signed-cubic covariance;
7. retention of $N_k$, $Z$, and $|Z|^2$; and
8. remote-bank-independent route evolution.

Still open:

1. genesis formation and protection of scheduler, source, route tokens,
   endpoint owners, and apparatuses;
2. writer integration into canonical state-complete $\Phi$;
3. collision/traffic arbitration and occupancy-fault protection;
4. physical setting carriers, setting timing, and spacelike laboratory
   geometry;
5. reciprocal detector work, material response, and macroscopic
   amplification;
6. finite-memory export/reset over repeated trials; and
7. recovery of laboratory Bell correlations or an empirically adequate
   alternative prediction.

The correct conclusion is:

> **FTD now has an exact candidate for common origination and causal delivery
> of two physical Born banks. It does not yet have a complete Bell experiment
> or the observed correlation law.**

---

## 7. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_opposite_bank_convoy_causal_bipartite_routing_phi_v16_candidate.py
```

Expected result: `13/13` exact checks pass, including 8,192 formed banks,
16,384 local hop/delivery rows and inverses, 1,208 finite-route rows, 22,801
remote-independence rows, and 7,248 signed-cubic covariance rows.
