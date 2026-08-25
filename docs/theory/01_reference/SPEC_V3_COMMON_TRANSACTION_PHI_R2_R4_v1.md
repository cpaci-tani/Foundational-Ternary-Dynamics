# SPEC — V3 common transaction Phi, R2--R4 v1

**Date:** 2026-08-24  
**Status:** **[SUPERSEDED DRAFT RULE — PROVENANCE RETAINED]**  
**Successor:**
[`SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md`](SPEC_V3_COMMON_ACTION_PHI_R2_R5_v2.md)
retains the exact relation/expiry/recurrence transactions but enlarges the
field bank and composes the exact cotangent collision required by R5. The v1
32/32 results remain valid for its narrower rule and are not v2 wave claims.  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Constitution:**
[`SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md`](SPEC_FTD_FRAMEWORK_V3_STRICT_DISCRETE_COMMON_ACTION.md)  
**Carrier:**
[`SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v1.md`](SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v1.md)  
**Executable reference and exact certificate:**
[`proof_v3_common_transaction_phi.py`](../../../scripts/proofs/proof_v3_common_transaction_phi.py)

---

## 0. Scope and epistemic boundary

This document instantiates one exact finite map `Phi` on the selected v3
carrier. It closes the **ratification-minimum** content of R2, R3, and R4. It
does not show that the selected law is unique, physically correct, or capable
of recovering known continuum physics.

The rule is deliberately austere. Its purpose is to cross the line from an
arena of possible cellular automata to one complete, falsifiable dynamics.
R5 must now decide whether this dynamics has a controlled wave/action limit.
If it does not, the rule is rejected or amended openly; compatibility with
isolated reference transactions is not enough.

---

## 1. Complete state used by the rule

Let `R` denote the A9 quarter-turn

\[
 R(u,v)=(-v,u),\qquad R^4=1.
\]

At each site `x`, the state contains:

1. one cached ternary actuality `s_x`; and
2. six directed packet ports `f_{x,d}` for `d` in the SC shell.

A nonblank port is

\[
 f_{x,d}=(z,n,h),
 \qquad z\in\mathcal A_9\setminus\{0\},
 \quad n\perp d,
 \quad h\in\{-1,+1\}.
\]

Every unoriented C18 relation `r=[x,y]`, owned by its minimal enclosing edge
or labeled plaquette diagonal, contains an independent A9 pair

\[
 (\lambda_r,\rho_r)\in\mathcal A_9^P\times\mathcal A_9^D.
\]

`lambda` is primary/manifest ownership and `rho` is bound reserve/response
ownership. The rule is defined on every product state, including malformed
states with both slots occupied. Prepared one-token sectors use

\[
 n(\lambda_r)+n(\rho_r)=1.
\]

No continuous variable, queue, packet identity, stored birth time, random
number, or controller word is present.

---

## 2. Relation source and manifestation quotient

Give every stored C18 relation its transported incidence orientation
`x -> y`. An occupied primary token contributes

\[
 q_{x,r}=+\epsilon(\lambda_r),
 \qquad
 q_{y,r}=-\epsilon(\lambda_r).
\]

Define the integer relation source

\[
 Q_x=\sum_{r\ni x}q_{x,r}.
\]

The public site actuality is the balanced residue

\[
 \boxed{
 s_x=\operatorname{bal}_3(Q_x),
 \qquad
 \operatorname{bal}_3(0,1,2\bmod3)=(0,+1,-1).}
\]

Thus relation slots, not competing site writers, own manifestation. The site
slot is a finite cache of a local quotient. Multiple incident transactions
compose before the quotient is read, so no arbitration among symmetric
neighbors is required.

For the isolated one-link sector `Q_x` is already in `{-1,0,+1}`, and the
readout is ordinary neutral pair creation/withdrawal rather than a modular
alias.

---

## 3. One synchronous tick

Every output below is computed from the same pre-tick state `X_n`. The four
paragraphs are definitions of output coordinates, not sequential microticks.

### 3.1 Absorption/expiry candidate

For a nonblank packet at `(x,d)`, let `e(x,d)` be the unique SC edge joining
`x` to `x+d`. It is an absorption candidate exactly when:

1. its A9 phase is `k=2`;
2. both slots of `e(x,d)` are blank; and
3. it is one of the two endpoint-directed ports targeting that edge.

The absorption is admitted only when exactly one endpoint port is a
candidate. Two opposing candidates fail closed; neither is chosen.

For the unique candidate `(z,n,h)`, set

\[
 \boxed{
 f'_{x,d}=\varnothing,
 \qquad
 (\lambda'_e,\rho'_e)=(0,Rz).}
\]

This moves the complete phase/polarity/work token into bound ownership. The
normal and handed presentation are not copied anywhere.

### 3.2 Relation crossing clock

For a relation not receiving an absorption, define the scalar field gate

\[
 g_r=1-left(
 \sum_{v\in\partial r}\sum_{d\in\mathcal D_{SC}}
 n(f_{v,d})\right)\bmod2.
\]

If exactly one of `(lambda_r,rho_r)` is occupied, its A9 phase is `k=0`, and
`g_r=1`, exchange ownership and advance the phase:

\[
 \boxed{
 (\lambda'_r,\rho'_r)=(R\rho_r,R\lambda_r).}
\]

Otherwise only advance all nonblank A9 slots:

\[
 \boxed{
 (\lambda'_r,\rho'_r)=(R\lambda_r,R\rho_r).}
\]

This is the exact controller-free crossing clock `F=R A_0` on the isolated
one-token sector. Odd local packet occupation delays a crossing rather than
altering the global tick.

### 3.3 Field-port transport and material response

Every unabsorbed packet has one unique output port:

\[
 (x,d;z,n,h)\longmapsto(x+d,d;z',n,h'),
\]

where

\[
 z'=R^{,1-2s_x^2}z,
 \qquad
 h'=(-1)^{s_x^2}h.
\]

An unmanifested departure site advances the field phase; a manifested site
reverses that local phase clock and flips handedness. Occupancy and the SC
transport direction remain unchanged. This is a microscopic reciprocal
coupling: field occupancy gates the material ownership clock, while material
actuality changes the outgoing field record.

### 3.4 Actuality cache

Finally, every site output is the single reduction

\[
 \boxed{s'_x=\operatorname{bal}_3(Q'_x)}
\]

computed from the already defined relation-output formulas. This notation
does not create a sequential dependency: `Q'_x` is an explicit radius-one
function of `X_n`.

Equations in sections 3.1--3.4 are the complete `Phi`.

---

## 4. Why the schedule is conflict-free

No coloring, coordinate parity, preferred neighbor, or external scheduler is
used.

- Each C18 relation pair is written once by its own relation formula.
- Each SC absorption target admits zero or one of its two endpoint proposals;
  a symmetric collision admits neither.
- Each directed destination port `(x,d)` has the unique predecessor
  `(x-d,d)`.
- Each site actuality slot is written once by the incidence reduction.
- Cube records are singletons and require no update.

All geometric offsets are SC or FCC vectors with Chebyshev norm one. Even the
site reduction reads only incident relations and endpoint ports. Therefore

\[
 X_{n+1}=\Phi(X_n)
\]

is deterministic, homogeneous, translation covariant, signed-cubic shell
covariant, and Moore bounded. It has no tick-parity phase and no undeclared
tie breaker.

---

## 5. R3 — the first genuine expiry boundary

Fix a directed port `(x,d)` and a phase-2 token `z`. There are eight distinct
frame presentations

\[
 (z,n,h),
 \qquad n\perp d,quad h=\pm1.
\]

When absorption is uniquely admitted, all eight map to exactly the same
output:

\[
 (z,n,h;\lambda_e=0,\rho_e=0)
 \longmapsto
 (\varnothing;\lambda'_e=0,\rho'_e=Rz).
\]

Hence `Phi` is genuinely many-to-one on admissible finite states. There is no
inverse tape or undeclared bath.

The ledger is explicit:

| Datum | Disposition |
|---|---|
| A9 occupation/work unit | survives in `rho_e` |
| C4 phase | survives, advanced by one tick |
| polarity/charge label | survives exactly |
| SC carrier line | survives as the owning edge |
| signed arrival endpoint | expires |
| perpendicular normal | expires |
| handed frame | expires |

Define microscopic work count

\[
 W=\#\{\text{nonblank ports}\}
 +\sum_r\bigl(n(\lambda_r)+n(\rho_r)\bigr).
\]

The certificate proves `W'=W` for absorption, transport, and crossing. Thus
the expiry of a frame label is not assigned an abstract heat cost. Work is
the physical ownership transfer of the retained token.

---

## 6. R4 — one-law nontriviality witnesses

The same `Phi` supplies all four ratification-minimum witnesses.

### 6.1 Finite propagation

An unabsorbed packet advances exactly one SC hop per tick. It cannot outrun
the Moore hull.

### 6.2 Reciprocal manifestation and withdrawal

On an isolated relation with one A9 token split between primary and reserve,
the crossing map has exact period eight. The token is primary-owned for four
ticks and reserve-owned for four; the endpoint manifestation quotient appears
and withdraws reciprocally.

### 6.3 Localized recurrent clock/proto-body

The same isolated relation is a spatially localized finite recurrence with a
physical C4 phase and an eight-tick complete-state period. Odd local field
occupation can miss the crossing section and delay the material cycle while
the global tick continues. This is a proto-clock, not a proof of stable
extended matter or inertia.

### 6.4 Source/current ledger

For each oriented relation, define

\[
 \Delta\epsilon_r=epsilon(\lambda'_r)
 -\epsilon(\lambda_r),
 \qquad J_r=-\Delta\epsilon_r.
\]

With the ordinary oriented incidence divergence,

\[
 \boxed{Q'_x-Q_x+(\operatorname{div}J)_x=0}
\]

holds identically. The certificate verifies it on the complete eight-tick
recurrence. This is an exact discrete source/current ledger; it is not yet
identified with electric charge or Maxwell current.

---

## 7. Exact certificate result

The executable reference verifies 32/32 gates, including:

- R1 alphabet agreement;
- C18 shell covariance;
- blank fixed point and determinism;
- one-hop propagation;
- eight-to-one frame expiry;
- fail-closed opposing absorption;
- token/work conservation;
- exact period-eight localized recurrence;
- four manifested ticks per cycle;
- exact source/current continuity;
- field-controlled clock delay;
- reciprocal material effect on field phase/handedness;
- translation covariance;
- unique writes and Moore support; and
- closure of every output in its finite alphabet.

These are exact finite-state statements. No continuum target or experimental
number is used.

---

## 8. What remains open

R2--R4 closure does not ratify v3. The decisive remaining gates are:

1. **R5:** derive or close negative a controlled real flux-wave limit and a
   non-tautological blocked action/history functional with finite-region error
   bounds; and
2. **R6:** run the target firewall over the carrier, rule, preparations, and
   recovery tests.

Even if both pass, stable many-relation matter, Maxwell reduction, operational
coupling, Lorentz/common-cone recovery, Born statistics, gravity/lensing, and
the particle spectrum remain stronger research requirements. The present
proto-body and current ledger must not be promoted to those claims.
