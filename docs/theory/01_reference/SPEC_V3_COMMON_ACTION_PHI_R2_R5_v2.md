# SPEC — V3 common action Phi, R2--R5 v2

**Date:** 2026-08-24  
**Status:** **[SELECTION — EXACT FINITE V3 REFERENCE LAW]** +
**[THEOREM — CONFLICT-FREE HOMOGENEOUS RADIUS-ONE SCHEDULE]** +
**[THEOREM — GENUINE FRAME EXPIRY WITH CONSERVED TOKEN LEDGER]** +
**[THEOREM — R4 RATIFICATION-MINIMUM WITNESSES]** +
**[THEOREM — CONTROLLED TRANSVERSE REAL-WAVE/ACTION RECOVERY]**  
**Production status:** unchanged  
**Ledger status:** adopted constitutionally through FTD-1023; no independent row minted  
**Supersedes:**
[`SPEC_V3_COMMON_TRANSACTION_PHI_R2_R4_v1.md`](SPEC_V3_COMMON_TRANSACTION_PHI_R2_R4_v1.md)  
**Carrier:**
[`SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md`](SPEC_V3_FINITE_CARRIER_INVENTORY_R1_v2.md)  
**Integrated verifier:**
[`proof_v3_common_action_phi_v2.py`](../../../scripts/proofs/proof_v3_common_action_phi_v2.py)  
**Collision certificates:**
[`THEOREM_GLOBAL_C3_COTANGENT_LAYER_COLLISION_AND_VACUUM_MAXWELL_PASS_v1.md`](../10_eft_program/derivations/charge_gauss_native_em/THEOREM_GLOBAL_C3_COTANGENT_LAYER_COLLISION_AND_VACUUM_MAXWELL_PASS_v1.md)

---

## 0. What changed and what did not

The v1 common transaction crossed R2--R4 but failed the strong R5 benchmark:
six uncoupled straight streams are ballistic advections, not one isotropic 3D
wave. V2 does not relabel that failure. It retains the relation ownership,
absorption/expiry, manifestation quotient, and source/current ledger, while
replacing the narrow port state by the exact finite cotangent channel bank and
its frozen layer-covariant collision.

The rule remains a **selection**. Its wave sector is theorem-grade *given the
selected collision table and registered preparation*. No physical coupling,
charged Maxwell sector, or action normalization is inferred from the wave
speed.

---

## 1. State

At every site `x`, the state is

\[
 (s_x,\ell_x,b_x)
 \in\mathbb T\times\mathbb Z_3\times\{0,1\}^{384}.
\]

An occupied field channel is

\[
 \gamma=(d,n,h,k,\epsilon),
\]

with directed SC tangent `d`, perpendicular axial normal `n`, handed flag
`h`, C4 phase `k`, and polarity `epsilon`. Every C18 relation owns

\[
 (\lambda_r,\rho_r)\in\mathcal A_9^P\times\mathcal A_9^D.
\]

The complete alphabets and ownership rules are frozen by the R1-v2 carrier
specification.

---

## 2. Frozen field collision

At fixed polarity, the 192 Hodge/C4 channels are indexed exactly as in the
global-C3 cotangent theorem. Let `C_q` be its frozen unordered-pair involution
at layer `q`. The complete three-layer table has hash

`D0BB71DBED7938ED286E1D6D91A16700DA31F4550E83B2FB3580CCC347B2BD25`.

At site `x`, after removing a uniquely admitted absorption candidate:

1. if a polarity layer contains exactly two occupied channels, replace that
   unordered pair by `C_{ell_x}` of the pair;
2. otherwise leave that polarity layer unchanged.

The two polarity layers are disjoint and use the same table. Charge
conjugation exchanges them. Every `C_q` is a fixed-point-free involution on
the two-record sector, preserves record number and the six layer-appropriate
`(E,B)` sums, and obeys

\[
 U C_q=C_{q-1}U.
\]

The collision table was selected by a deterministic symmetry-orbit and rank
construction before any spectrum was inspected. Its exact rank is 185 and
its additive nullity is seven: record number plus six field components.

---

## 3. Complete synchronous tick

All outputs below are functions of `X_n`; the list is a coordinate definition,
not a sequence of hidden microticks.

### 3.1 Unique absorption and expiry

A phase-2 field channel at `(x,d)` targets the SC edge joining `x` to `x+d`.
It is admitted only when both A9 relation slots are blank and exactly one
endpoint/channel presentation targets that edge. Competing proposals fail
closed.

The admitted channel is cleared and its phase/polarity token `z(k,epsilon)`
is written as

\[
 (\lambda'_e,\rho'_e)=(0,Rz).
\]

The normal, hand, and signed arrival endpoint expire. A9 phase, polarity, one
token/work unit, and the SC carrier line survive.

### 3.2 Local collision, Hodge tick, and streaming

Apply the local `C_{ell_x}` pair collision described in section 2 to every
nonabsorbed field bank. Then every occupied output channel advances by the
shared-edge internal tick

\[
 U(d,n,h,k)=igl(hn,h(d\times n),h,k+1\bmod4\bigr)
\]

and streams zero or one SC hop along its pre-update tangent. The registered
vacuum rule uses one hop. A manifested departure site adds a C4 half-turn
`k -> k+2`; this is a channel permutation and hence cannot create a write
collision. It gives the material record a reciprocal effect on the outgoing
field phase without changing the vacuum operator.

Every local layer advances

\[
 \ell'_x=\ell_x-1\pmod3.
\]

The registered wave preparation has uniform `ell`; local synchronization is
preserved exactly.

### 3.3 Relation crossing

For every nonabsorbing C18 relation, define `g_r=1` when the total endpoint
field occupation is even and `g_r=0` when it is odd. If exactly one of
`(lambda_r,rho_r)` is occupied, its phase is zero, and `g_r=1`, set

\[
 (\lambda'_r,\rho'_r)=(R\rho_r,R\lambda_r).
\]

Otherwise rotate all nonblank relation tokens in place. Field occupation
therefore changes the rate of the localized ownership clock, while manifested
relations change outgoing field phase. Both effects are cases of the same
tick.

### 3.4 Manifestation and source

For every oriented primary relation, assign `+epsilon(lambda)` to its tail
and `-epsilon(lambda)` to its head. Let `Q'_x` be their integer incidence sum
and set

\[
 s'_x=\operatorname{bal}_3(Q'_x).
\]

This is the only site writer. The oriented current

\[
 J_r=-\bigl(\epsilon(\lambda'_r)-
 \epsilon(\lambda_r)\bigr)
\]

obeys `Delta Q + div J=0` identically.

---

## 4. R2--R4 remain closed after enlargement

The v2 field bank does not reopen the earlier minimum gates:

- every relation pair is written once;
- every occupied channel has one collision output and one streaming
  destination;
- the Hodge/internal/phase/material maps are finite channel permutations;
- every site actuality is one incidence reduction;
- every dependency lies in the Moore cube;
- absorption remains genuinely many-to-one and conserves the token/work
  count;
- an isolated A9 relation retains its exact period-eight
  manifestation/withdrawal recurrence; and
- relation source/current continuity remains an identity.

No coordinate coloring, tick parity, stochastic branch, or replay controller
is used. The C3 layer is an explicit finite state advanced by the same rule.

---

## 5. Registered R5 vacuum preparation

The R5 comparison is made on a finite periodic region with:

1. synchronized site layer `ell=0` initially;
2. `s=0` and a translation-invariant relation background with both A9 slots
   occupied, so relation crossing and absorption are inert and their oriented
   primary sources cancel site by site;
3. one polarity field layer prepared by the uniform independent binary
   reference at occupation `1/2`; and
4. the conjugate polarity layer blank.

The uniform binary reference is a finite counting measure on the declared
field bank. It is not a Born rule, physical vacuum-energy claim, or adjustable
probability. Its role is to define the exact tangent/blocked field response.

The local collision triggers on the exactly-two-occupied sector. Under the
uniform reference its exact tangent coefficient is `1/2^191`; this is a
finite census result and is not a coupling.

---

## 6. Exact slow generator

The certified three-tick Floquet derivative on the seven additive slow
readouts has characteristic polynomial

\[
 \lambda
 \left(\lambda^2+{|k|^2\over27}\right)
 \left(\lambda^2+{|k|^2\over36}\right)^2.
\]

On the constrained vacuum subspace

\[
 \rho=0,
 \qquad k\cdot E=0,
 \qquad k\cdot B=0,
\]

there are exactly two transverse electric--magnetic pairs with

\[
 \omega^2={|k|^2\over36},
 \qquad c_{\rm eff}={1\over6}.
\]

Consequently each real transverse field component satisfies

\[
 \partial_t^2 J_T={1\over36}\nabla^2J_T
\]

at leading blocked order.

This is the physically admissible vacuum restriction of the v1 ACT-1
benchmark. ACT-1 also carries the Gauss constraint; its unconstrained third
component is not an independent physical vacuum polarization. The scalar
block of `Phi` is acoustic and does **not** provide a charged Gauss law. The
R5 pass therefore does not claim recovery of charged electromagnetism.

---

## 7. Non-tautological blocked action

Let `A_T` be either real transverse blocked coordinate. The quadratic
finite-resolution action is

\[
 \boxed{
 S_{\rm wave}[A_T]
 ={1\over2}\sum_{x,t}
 \left[
 |D_tA_T|^2-{1\over36}
 \sum_{j=1}^3|D_jA_T|^2
 \right].}
\]

Its Euler--Lagrange equation is the real lattice wave equation

\[
 D_t^-D_tA_T={1\over36}\Delta_LA_T.
\]

The action is not the mismatch indicator
`sum 1[X_{n+1} != Phi(X_n)]`. Its field content, two-mode count, cubic spatial
form, and speed are fixed by the exact slow generator. It makes independent
predictions that the certificate can fail. An overall positive multiplication
of `S_wave` is not fixed by vacuum equations; source normalization and a
physical coupling remain open.

---

## 8. Finite-region error contract

Let `P_T` be the normalized transverse slow projection and let `F_3(k)` be
the exact three-tick tangent map of `Phi` on a finite periodic region. For
`|k| <= kappa`, every streaming character obeys

\[
 |e^{-ik\cdot d}-1+i k\cdot d|
 \le {|k|^2\over2}.
\]

Collision and internal maps are contractions/permutations in the normalized
counting norm. Expanding the product of three ticks therefore gives the
explicit conservative bound

\[
 \boxed{
 \left\|P_TF_3(k)P_T-
 \left(I+3G_T(k)\right)\right\|_2
 \le {9\over2}\,\kappa^2e^{3\kappa},}
\]

where `G_T` is the exact speed-`1/6` transverse generator.

For `M` coarse three-tick steps, telescoping gives

\[
 \|F_3(k)^M-(I+3G_T(k))^M\|_2
 \le M R_\kappa
 \exp\!\left(M(\kappa/2+R_\kappa)\right),
\]

with

\[
 R_\kappa={9\over2}\kappa^2e^{3\kappa}.
\]

This bound is intentionally loose but finite, target independent, and tends
to zero for fixed `M` as `kappa -> 0`. The difference between `k_j` and the
lattice symbol `2 sin(k_j/2)` is bounded by `|k_j|^3/24`, so replacing the
leading continuum gradient by the displayed lattice action adds a controlled
`O(kappa^3)` symbol error.

R5 is therefore a controlled infrared recovery statement, not an assertion
that a finite lattice is exactly Lorentz invariant at all wave numbers.

---

## 9. R5 disposition

R5 closes for the selected v2 law at the following precise scope:

- real, divergence-free vacuum flux waves;
- two transverse polarization pairs;
- speed `1/6` in lattice units per global tick;
- the quadratic lattice action in section 7, up to an unfixed overall scale;
- finite periodic regions and the explicit infrared error contract in
  section 8.

It does not close:

- a local charged Gauss pole;
- source-to-field action normalization or the fine-structure coupling;
- finite-amplitude nonlinear invariance of the slow manifold;
- stable extended matter;
- Lorentz/common-cone recovery with material clocks; or
- gravity/lensing.

Those remain post-ratification physics requirements. R6 has since passed its
operational/dataflow certificate; no listed R1--R6 formal gate remains open.
