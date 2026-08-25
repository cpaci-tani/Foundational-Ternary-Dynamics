# V3 neutral-rotor harmonic Green seam v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXISTING-FIELD-CARRIER NEUTRAL C12 ROTOR]** +
**[THEOREM — DETERMINISTIC DIRICHLET-GREEN RESIDUAL BOUND]** +
**[THEOREM — EXACT EMPIRICAL UNIT GAUSS FLOW]** +
**[CONDITIONAL — CHARGED STATIC POLE AT BLOCKED-HISTORY LEVEL]** +
**[SELECTION PRICE — ROTOR MACRO, TOKEN ROUTER, SOURCE RENEWAL, AND SINK]** +
**[OPEN — PHI INTEGRATION, RECIPROCAL ACTION, COUPLING, FORMATION, MATTER,
BORN APPARATUS, AND GRAVITY]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Exact certificate:**
[`proof_v3_neutral_rotor_harmonic_green_seam.py`](../../../../../scripts/proofs/proof_v3_neutral_rotor_harmonic_green_seam.py)

---

## 1. Why this route is different from an invariant-cycle measure

The isolated charged circulation frames are finite permutations with disjoint
cycle measures. Invariance therefore does not select a physical mixture and
their finite-support time averages cannot contain a Coulomb pole.

This theorem tests a different deterministic mechanism: repeated finite source
transactions through a local round-robin router. The relevant object is not an
arbitrary invariant ensemble. It is the operational Cesaro frequency of one
deterministic history. The finite discrepancy vanishes with the number of
source transactions independently of the initial rotor phases.

This construction is the finite-state cubic specialization of the general
rotor-router idea. Prior mathematical work proves analogous deterministic
approximations to Markov hitting and occupation frequencies; see
[Holroyd and Propp, *Rotor Walks and Markov Chains*](https://www.microsoft.com/en-us/research/wp-content/uploads/2016/12/rr.pdf)
and [Pham, *Orbits of rotor-router operation and stationary distribution of
random walks on directed graphs*](https://arxiv.org/abs/1403.5875). The FTD
certificate below is self-contained and uses neither paper as a proof oracle.

---

## 2. A neutral rotor already fits in the field bank

Let one existing Hodge record be

\[
 q=((d,n,h),p),
\]

where $(d,n,h)$ is the oriented flag and $p\in\mathbb Z_4$ its phase. Occupy
the two distinct exclusion slots with the same $q$ and opposite field
polarities. On every C3 layer their additive electric-magnetic readouts cancel:

\[
 (E,B)_{(q,+)}+(E,B)_{(q,-)}=0.                         \tag{1}
\]

The pair therefore stores a rotor state with two existing bits, constant
record number, and zero bare field readout. The positive-polarity member is a
state-only controller; no pointer identity is added.

Advance both members by the already selected internal Hodge/C4 tick. Define
the served SC direction by

\[
 r(q)=\sigma(p)d,
 \qquad
 \sigma(p)=
 \begin{cases}
 +1,&p\in\{0,1\},\\
 -1,&p\in\{2,3\}.
 \end{cases}                                           \tag{2}
\]

Every native period of twelve visits serves each of the six SC directions
exactly twice. For every initial Hodge flag and every prefix length $m$,

\[
 \boxed{
 \left|M_m(x,d)-{m\over6}\right|\le {4\over3}.}        \tag{3}
\]

The global maximum $4/3$ is attained. Both the rotor tick and equation (2)
commute with all 48 signed-cubic transformations. No coordinate coloring,
random choice, real register, or target value enters.

Equation (1) establishes carrier availability only. Phi-v2 would ordinarily
collide and stream these records; holding and advancing them on token visits
is a new candidate macro.

---

## 3. Deterministic finite-domain theorem

Let $V$ be any finite connected cubic vertex set. Every missing SC neighbor is
an edge to one absorbing exterior sink. Place one rotor of section 2 at every
$x\in V$. Sequentially inject $N$ tokens at a source $s$; on each visit the
local rotor advances once and routes the token along its served SC edge. A
token expires when it reaches the sink.

Every token is absorbed after finitely many moves. Otherwise, finiteness would
force some interior vertex to be visited infinitely often. Complete rotor
service would then send the token to every neighbor infinitely often;
connectedness propagates this property to a boundary vertex, whose complete
service necessarily uses an exterior sink edge, a contradiction.

Let

- $n_N(x)$ be the number of departures from $x$;
- $m_N(x,d)$ be the number routed from $x$ in direction $d$; and
- $L_D=6I-A_V$ be the cubic Dirichlet Laplacian.

The local rotor bound is exactly

\[
 m_N(x,d)={n_N(x)\over6}+\varepsilon_N(x,d),
 \qquad
 |\varepsilon_N(x,d)|\le {4\over3}.                    \tag{4}
\]

Every interior visit departs and every non-source visit has one interior
arrival. Therefore the normalized antisymmetric traversal flow

\[
 J_N(x,d)={m_N(x,d)-m_N(x+d,-d)\over N}                \tag{5}
\]

obeys exact empirical Gauss continuity for every finite $N$:

\[
 \boxed{\operatorname{div}J_N=\delta_s}                \tag{6}
\]

on $V$, with the compensating sink flux on the exterior boundary.

Now define the visit potential

\[
 G_N(x)={n_N(x)\over6N}.                               \tag{7}
\]

Substitution of equation (4) into visit conservation gives

\[
 L_DG_N-\delta_s
 ={1\over N}\sum_{y\sim x}\varepsilon_N(y,x).
\]

There are at most six incoming slots, hence

\[
 \boxed{
 \|L_DG_N-\delta_s\|_\infty\le {8\over N}.}           \tag{8}
\]

Similarly,

\[
 \boxed{
 \left|J_N(x,d)-[G_N(x)-G_N(x+d)]\right|
 \le {8\over3N}.}                                     \tag{9}
\]

Because $L_D$ is invertible on every fixed finite absorbing domain,
equation (8) proves

\[
 G_N\longrightarrow L_D^{-1}\delta_s,
 \qquad
 J_N\longrightarrow \nabla_D L_D^{-1}\delta_s        \tag{10}
\]

as $N\to\infty$. This limiting field is independent of the initial rotor
phases. The certificate verifies the exact identities and bounds on seven
finite boxes and solves the 27-site Dirichlet inverse over the rationals; the
general proof is equations (4)--(10), not extrapolation from those fixtures.

---

## 4. Static-pole consequence

The translation-invariant cubic operator has exact Fourier symbol

\[
 \Lambda(k)=6-2\sum_{a=1}^{3}\cos k_a,
\]

with

\[
 \Lambda(0)=0,
 \qquad
 \nabla\Lambda(0)=0,
 \qquad
 D^2\Lambda(0)=2I_3.                                  \tag{11}
\]

Thus the controlled order of limits

\[
 N\to\infty\quad\text{at fixed finite domain},
 \qquad
 V\nearrow\mathbb Z^3
\]

has the massless static symbol

\[
 \boxed{G(k)={1\over\Lambda(k)}}.                      \tag{12}
\]

This is stronger than the earlier selected reference-action pole: equations
(8)--(10) give a deterministic local history mechanism converging to the
minimum-norm Gauss representative. It is weaker than canonical charged
electromagnetism because the router is not yet part of Phi and the limiting
readout has not been written back as an instantaneous finite field state.

---

## 5. What it does and does not normalize

Equation (6) fixes source and flow in unit-token coordinates. Equation (12)
fixes the spatial Green kernel and its source residue in that history readout.
It does **not** fix the Maxwell action curvature. Under the registered
constrained functional

\[
 {H_{\rm stat}\over I_*}
 ={\chi_{\rm EM}\over2}\langle E,E\rangle,
 \qquad DE=\rho,
\]

the minimizing field is independent of the positive multiplier
$\chi_{\rm EM}$; the multiplier prices the field energy. Rotor frequencies
therefore close the spatial pole mechanism without collapsing the positive
action-scale orbit.

The uniform-bank theorem still gives only

\[
 \chi_{\rm EM}=4\lambda_{\rm common},
 \qquad
 \alpha_{\rm native}={6\lambda_{\rm common}\over\pi}, \tag{13}
\]

conditional on identifying the counting Hessian with the dynamical action.
The rotor theorem supplies no value of $\lambda_{\rm common}$.

---

## 6. Exact price and next gate

Established exactly:

1. a zero-field C12 rotor in two existing exclusion slots;
2. exact uniform service of all six SC directions;
3. exact signed-cubic covariance and prefix discrepancy $4/3$;
4. finite absorption of the registered fixtures;
5. exact empirical unit Gauss flow for every completed injection;
6. deterministic Dirichlet-Poisson residual $8/N$;
7. deterministic current-gradient discrepancy $8/(3N)$; and
8. the conditional large-domain $1/\Lambda$ history pole.

Selected or still open:

1. a state-complete rotor macro inside the charged Phi schedule;
2. composition of the now carrier-complete neutral sampler with A9 incidence
   and dressed Gauss strings;
3. autonomous formation of the neutral rotor background;
4. autonomous source renewal, sink ownership, and inverse/work accounting;
5. finite writeback beyond the certified prepared A2-memory apparatus;
6. the common action unit and physical coupling normalization;
7. a stable source, detector, and matter basin;
8. native general Born-bank preparation and apparatus backreaction; and
9. a protected tensor response of the same history action.

The next discriminator is no longer “can deterministic finite histories make
a Coulomb kernel?” They can, and the neutral sampler now has a complete local
carrier macro. The question is whether this seam can be realized by the same
state-complete Phi that carries actual A9 charge, work, stable matter, trials,
and tensor response without installing a prepared routing medium by hand.

The
[`finite A2-memory successor`](../gravity_cosmology/THEOREM_V3_ROTOR_GREEN_A2_PHYSICAL_MEMORY_AND_PHASE_PROTECTION_BOUNDARY_v1.md)
closes item 5 at one exact radius-one prepared apparatus: 108 fixed-occupancy
edge counters plus one source counter retain the current and unit divergence
in present carrier states, and the exact finite inverse bounds sensitivity to
arbitrary initial rotor phase by `O(1/N)` (15/15). It also proves the next
boundary: phase-only writes have zero delta under the common relative
occupancy action, so physical memory still is not reciprocal force or an
absolute coupling normalization.
