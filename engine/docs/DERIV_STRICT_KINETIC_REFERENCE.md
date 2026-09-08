# Exact stationary kinetic reference on a prepared finite background

Date: 2026-09-05. Law: `phi-v2-staged-candidate-1`.
Preparation proposal: `strict-kinetic-counting-reference-1`.
Status before validation: **[PREREGISTRATION — CONDITIONAL FINITE THEOREM]**.

## Contract fixed before validation

For every finite periodic L >= 3, initialize s=0, uniform ell, clear pending
controls, and the same nonblank A9 code in both slots of every SC/FCC relation.
Allow EVERY configuration of the 384 L^3 binary field slots. Externally select
rational p=a/b in [0,1]. Give a field configuration with k occupied slots the
positive-or-zero integer multiplicity a^k (b-a)^(384 L^3-k). Zero-weight
configurations are omitted; 0^0=1. No probability or random draw enters Phi.

The proof obligations are: background invariance for arbitrary bank occupancy;
bijective number-preserving collision and streaming maps; exact preservation
of these finite counting weights; complete phase-dependent lifts including
saved controls; analytical event eligibility and a finite-horizon expected
event count. Validation will enumerate the finite collision/channel alphabets
and use bounded deterministic runtime fixtures, without enumerating all global
banks, fitting a target, running a GPU campaign, or changing an earlier criterion.
Failure of any obligation is retained as an obstruction.

## Finite proof

**[THEOREM — CONDITIONAL ON THE DECLARED PREPARATION]** Both relation slots are
occupied, so SC admission never occurs and neither relation gate can trigger
a crossing or hold. At phase 1 both records rotate. Spatial homogeneity is
preserved. Each site's primary-incidence contributions cancel in oppositely
oriented pairs, so phase 3 returns s=0. Uniform ell decreases modulo 3 only
at phase 1. Thus arbitrary field occupancy cannot disturb this background.

On each 192-bit site/polarity bank, collision acts by the actual frozen
permutation on the exactly-two sector and by identity on all other cardinality
sectors. It is a bijection preserving cardinality. Different site/polarity
banks are disjoint. With s=0 streaming is (x,c) -> (x+d(c),U(c)); U is a
permutation and periodic translation is invertible for each c, hence this is
a permutation of all field slots. The other two stages leave the bank alone.
Every field stage is therefore a bijection preserving total occupied count.

Write M=384 L^3. The multiplicity w(B)=a^|B|(b-a)^(M-|B|) is unchanged by each
stage. The binomial theorem gives sum_B w(B)=b^M, without global enumeration.
Pushforward counting weights are exactly equal because each output has one
input and the same weight. Normalizing these EXTERNAL multiplicities yields
the Bernoulli(p) product field measure at every physical tick. This includes
p=0 and p=1 as degenerate measures. It proves stationary field marginals;
it does not prove attraction to this family from other preparations.

## Complete records and clock

Let B be the cycle-boundary bank, C_ell its local collision permutation, and
S the streaming permutation. At phases 0,1,2,3 the field banks are respectively
B, B, C_ell B, S C_ell B. Phase 1 stores every SC/FCC endpoint-parity gate
g(B), and phases 2 and 3 retain those same gates. The admission mask is always
zero. At the next phase 0 all controls expire and the bank is S C_ell B.
Each displayed field map is invertible, so the complete phase lift is the
deterministic pushforward of the counting ensemble with all gate correlations
retained; no marginal reprojection is performed.

For distinct endpoints the parity gate has
P(g=1)=[1+(1-2p)^768]/2. It is a deterministic function of the endpoint bank,
so full bank/gate independence is false for 0<p<1, including p=1/2. A product
field marginal must not be confused with a product full-record measure.

The homogeneous A9 background has four rotations and ell has period three.
The complete-record family (apart from the absolute clock) is periodic after
48 physical ticks; its field marginal is stationary at every tick. More
precisely mu_(t+1)=Phi_*mu_t with the correct absolute tick in each support
record. No normalized stationary measure on the unbounded absolute ordinal
clock is claimed. A uniform mixture of the 48 phase/layer/background classes
is stationary only on the explicitly clock-quotiented finite description.

This is also distinct from product-TANGENT closure: a particular invariant
product reference can coexist with first-order correlations produced by
nonuniform perturbations of that reference. No perturbation closure,
hydrodynamic limit, relaxation rate, isotropic wave equation, or matter follows.

## A separately named preparation proposal

**[THEOREM — EXTERNAL COUNTING EXPECTATION]** At every collision stage, each
site/polarity bank has exactly-two eligibility

q(p) = binom(192,2) p^2 (1-p)^190.

For 0<p<1, q'(p) has the sign of 2-192p; the unique maximum is p=1/96.
This exact optimization of the declared event criterion is a new preparation
proposal, not a retrospective change to a half-occupation certificate.
For a horizon H from phase zero there are floor((H+2)/4) collision stages,
and the expected total number of logged pair collisions is exactly
2 L^3 floor((H+2)/4) q(p). This counts eligible transactions, even when a
particular pair permutation fixes its input. It does not count guaranteed
scattering deflections, independent temporal trials, or relaxation events.
Linearity of expectation suffices; no independence between times is assumed.
The probability of at least one event is at most min(1, this expectation),
with no positive lower bound asserted here.

**[OPEN]** A future kinetic campaign must separately freeze preparations,
perturbations, observables, correlation retention, norms and horizons; record
every actual microtick; distinguish equilibrium stationarity from relaxation;
and prove or bound the correlation/closure errors before claiming a continuum
equation. No such campaign is authorized by this document alone.

## Bounded validation after the fixed contract

The implementation `scripts/phi_v2_lattice/recovery_kinetic_reference.py`
inspects all 18,336 pair inputs and outputs in each of the three frozen
collision layers, all 384 channels, and both gate values for all eight
homogeneous nonblank A9 codes. Tests additionally inspect every streaming
coordinate on L=3, run all four physical stages for empty, full, dense and
mixed exactly-two/non-two banks in all three layers, and run a 48-microtick
background/layer recurrence fixture. These finite checks support the proof
above; they are not a sampled assertion about all global configurations.

The 22 focused tests passed in 1.05 seconds. No global bank enumeration,
random preparation, measured relaxation campaign, or GPU trajectory was run.
The gate-dependence fixture verifies that equal-time bank stationarity cannot
justify replacing saved gates with independent control bits. Input validation
rejects inexact probabilities, invalid sectors and noncanonical boolean bytes.

For a separately registered L=9, 64-microtick kinetic reference at p=1/96,
the exact predicted mean field population is 2,916 and the expected pair-event
count is 23,328 q(1/96), with q given above. This prices a proposed preparation;
it is not a measured event count or a guarantee of relaxation.

An independent reviewer reproduced all 22 tests in 0.99 seconds and accepted
the finite multiplicity proof, complete correlated-gate lifts, clock
qualification, rational endpoints, derivative maximum and arbitrary-phase
event-count formula. No blocking finding remained at this prepared-reference
scope. Product-tangent closure, attraction and physical-probability recovery
remain outside the accepted result.
