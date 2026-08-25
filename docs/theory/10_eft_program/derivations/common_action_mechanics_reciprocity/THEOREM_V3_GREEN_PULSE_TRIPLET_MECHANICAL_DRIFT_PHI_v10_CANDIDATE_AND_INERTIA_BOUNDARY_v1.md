# V3 Green-pulse triplet mechanical drift Phi-v10 candidate and inertia boundary v1

**Date:** 2026-08-24  
**Status:** **[SELECTION — PREPARED FORCE-ALIGNED PHI-v10 CORRIDOR]** +
**[THEOREM, CONDITIONAL — EXACT PULSE-CONTROLLED TRIPLET DISPLACEMENT]** +
**[THEOREM — EXACT COMBINED RESPONSE/BODY INVERSE]** +
**[THEOREM, CONDITIONAL — INHERITED INITIAL-PHASE PROTECTION]** +
**[BOUNDARY — MECHANICAL DRIFT IS NOT INERTIAL ACCELERATION]** +
**[OPEN — STEERING, INERTIA, REACTION RETURN, TRAFFIC, CANONICAL PHI,
ABSOLUTE SCALE, TENSOR POLE, COMMON CONE, LENSING, AND NONLINEAR GRAVITY]**  
**Carrier/resource price:** the Phi-v9 seven-A2 response apparatus, one
prepared clean force-aligned Phi-v6 triplet, and a clear finite corridor of
length `|C|`; no new primitive type  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Response parent:**
[`THEOREM_V3_A2_GREEN_PULSE_RECIPROCAL_IMPULSE_ACTION_PHI_v9_CANDIDATE_AND_FORCE_BOUNDARY_v1.md`](THEOREM_V3_A2_GREEN_PULSE_RECIPROCAL_IMPULSE_ACTION_PHI_v9_CANDIDATE_AND_FORCE_BOUNDARY_v1.md)  
**Triplet-motion parent:**
[`THEOREM_V3_TRANSLATING_SELF_CORRECTING_TRIPLET_CLOCK_PHI_v6_CANDIDATE_AND_INERTIA_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_TRANSLATING_SELF_CORRECTING_TRIPLET_CLOCK_PHI_v6_CANDIDATE_AND_INERTIA_BOUNDARY_v1.md)  
**Exact certificate:**
[`proof_v3_green_pulse_triplet_mechanical_drift_phi_v10_candidate.py`](../../../../../scripts/proofs/proof_v3_green_pulse_triplet_mechanical_drift_phi_v10_candidate.py)

---

## 1. Composition question

Phi-v9 produces a deterministic signed impulse sequence `iota_t` from the
physical Green records `(C,N)`:

\[
 \iota_t\in\{-1,0,+1\},
 \qquad
 \sum_{t=0}^{12N-1}\iota_t=-C.                       \tag{1}
\]

It retains equal-and-opposite momentum and exact clock work, but it does not
move matter. Phi-v6 separately supplies a clean triplet transaction

\[
 (X_t,q_t)\longmapsto(X_t+n_C,q_{t+1})               \tag{2}
\]

along the chart's carried SC normal `n_C`, and the stationary Phi-v5 clock

\[
 (X_t,q_t)\longmapsto(X_t,q_{t+1}).                   \tag{3}
\]

Phi-v10 selects a probe chart aligned so that `n_C` is the nonzero impulse
direction. It applies equation (2) on a carry pulse and equation (3) on a dark
tick. No new body rewrite is invented.

---

## 2. Exact finite mechanical response

The combined step is

\[
\begin{aligned}
 R_{t+1}&=\Phi_9(C,N;R_t),\\
 q_{t+1}&=\Phi_5(q_t),\\
 X_{t+1}&=X_t+\iota_t d,
\end{aligned}                                         \tag{4}
\]

where `d` is the canonical orientation of the measured edge and the prepared
chart satisfies `n_C=iota_t d` whenever `iota_t` is nonzero. Since all
nonzero pulses in one fixed-edge response have the same sign, one aligned
chart suffices.

Summing equation (4) over one `12N`-tick response period and using equation
(1) gives

\[
 \boxed{X_{12N}-X_0=-Cd}                              \tag{5}
\]

and therefore

\[
 \boxed{{X_{12N}-X_0\over12N}=-{C\over12N}d.}        \tag{6}
\]

Every microtick moves zero or one SC cell, so the Moore causal ceiling is
respected exactly. The finite clear-corridor price is `|C|`, not an infinite
completed rail.

The certificate verifies equation (5) for `C=-37,-7,0,7,37` at `N=37` and
checks the one-hop geometry on all 1,152 oriented charts.

---

## 3. Exact combined inverse

The Phi-v9 inverse reconstructs whether the preceding modular addition
carried. It restores the prior pulse phase, both momenta, and clock work. The
clean triplet clock has a unique predecessor on its period-16 orbit. If the
reconstructed forward tick was bright, the chart origin is shifted one cell
opposite `n_C`; if dark, it is unchanged.

Thus every admitted combined tick has one exact inverse, and reversing all
`12N` ticks restores

\[
 (R_0,X_0,q_0)                                        \tag{7}
\]

exactly. The prior direction and event identity are read from retained
response/clock state, not guessed from the final displacement.

The triplet remains on its clean orbit throughout. Each bright or dark tick
retains the same role counts

\[
 (N_F,N_{A1,SC},N_{A1,FCC},N_{A2})=(6,3,0,1).         \tag{8}
\]

---

## 4. Physical Green-response drift and protection

At the canonical radius-one source edge, the A2-memory theorem gives

\[
 \left|{C\over N}-g_e\right|
 \le B_e(N)
 ={8\over3N}+{8K_e\over N}.                           \tag{9}
\]

Equations (6) and (9) imply

\[
 \boxed{
 \left|\bar v_d+{g_e\over12}\right|
 \le {B_e(N)\over12},}                               \tag{10}
\]

where `bar v_d` is the signed block displacement per global tick. For two
arbitrary initial rotor phases,

\[
 |\bar v_d^{(p)}-\bar v_d^{(p')}|
 \le {2B_e(N)\over12}.                               \tag{11}
\]

The certificate composes the complete response and triplet maps for all 192
native uniform rotor phases at `N=37`. Every triplet displacement equals
`-Cd`; all responses satisfy equations (10)--(11); the largest exact drift
error is `5/2664` on that fixture.

This closes the statement “a finite retained Green response changes the
position history of prepared matter.” The movement is not merely a number in
an analyst's output.

---

## 5. Why this is drift, not acceleration

The exact result also exposes the next no-promotion boundary. Phi-v10 moves
the body only when a response pulse is present. At the end of a nonzero
response period, Phi-v9 retains

\[
 p_P=-C\ne0,                                          \tag{12}
\]

but if the next control window is dark, the body takes no further spatial
hop. The certificate checks this directly for `C=7`:

```text
retained probe momentum:  -7
next dark-tick impulse:     0
next dark-tick body hop:    0
```

Therefore the construction has no derived Legendre map from momentum to
velocity and no inertial continuation. Equation (6) is a field-controlled
**drift law**, not `F=ma` and not a geodesic equation. The current body also
moves at the pulse cadence rather than accumulating velocity under successive
impulses.

This distinction matters. Calling Phi-v10 “gravity recovered” would conflate
three separate claims:

1. physical response memory — closed conditionally by the A2 theorem;
2. response-controlled displacement — closed conditionally here; and
3. inertial acceleration/geodesic motion — still open.

---

## 6. Remaining composition debts

Before Phi-v10 can become a physical force law, FTD must:

1. form the aligned chart, seven-owner apparatus, work reserve, and clear
   corridor natively;
2. combine all incident-edge currents into one cubic vector and steer the
   body without erasing its previous chart;
3. derive a momentum--velocity/clock relation and inertial continuation from
   the common phase action;
4. return the retained reaction momentum causally to the source matter or
   source dressing;
5. resolve multiple probes, collisions, traffic, packet loss, and overflow;
6. integrate the complete map into homogeneous canonical `Phi`;
7. derive the absolute action multiplier and physical units;
8. establish universal response for radiation and general stable matter; and
9. recover the tensor pole, common cone, clock response, lensing, Shapiro
   delay, and nonlinear completion.

Phi-v10 is the first exact prepared composition in this chain that changes a
matter position history. It remains below the acceleration and gravity gates.

The
[`Phi-v11 clocked-inertia successor`](THEOREM_V3_TRIPLET_CLOCK_LEGENDRE_INERTIA_PHI_v11_CANDIDATE_AND_GRAVITY_BOUNDARY_v1.md)
closes the continuation subcase by selecting the triplet's exact 16-tick
internal period as the inertial denominator. One finite phase owner yields
`v=p/16`, continued dark-window motion, and the exact selected block relation
`F_bar=16a_bar` (12/12). The selection still lacks canonical-Phi provenance,
native steering/resources, dimensional units, reaction return, and general
gravity closure.

---

## 7. Reproduction

From the repository root:

```bash
python scripts/proofs/proof_v3_green_pulse_triplet_mechanical_drift_phi_v10_candidate.py
```

Expected result: `10/10` exact checks pass. The certificate reports body
displacement `-C*d`, average drift `-C*d/(12N)`, zero-or-one causal hop per
tick, exact complete inverse, constant clean-triplet occupancy, inherited
all-phase response protection, and absent inertial continuation.
