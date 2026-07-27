# Theorem — Genesis admits a one-event dilation, not a finite reversible production cycle (FTD-0569)

**Status:** `[THEOREM — CONDITIONAL GENESIS INVERSE]` +
`[THEOREM — BERNOULLI PHASE DILATION]` +
`[THEOREM — FROZEN CYCLE NOT DETAILED-BALANCED]` +
`[CLOSED NEGATIVE — FINITE LOCAL REVERSIBLE PRODUCTION LIFT]`

**Verdict:** `ONE_EVENT_DILATION_OPEN_SYSTEM_ONLY`

**Dependencies:** FTD-0425, FTD-0499, FTD-0567.

## 1. Accepted genesis is not the source of ordinary information loss

Let a canonical void site have

\[
J=(k_g+x)n,\qquad |n|=1,\qquad x>0,
\]

and let \(0\le d<1\) be the kinetic drain. An accepted single-substrate
genesis event gives

\[
J'=xn,
\qquad
W'=(1-d)W,
\qquad
s'=\sigma\in\{-1,+1\}.
\]

Because \(x=|J'|>0\), its inverse on this event domain is

\[
\boxed{
J=\left(1+\frac{k_g}{|J'|}\right)J',
\qquad
W=\frac{W'}{1-d},
\qquad
s=0.}
\]

The manifested state distinguishes the accepted branch from the rejected
identity branch. Production's post-event spin and color do not obstruct this
inverse: they are outputs overwritten on a canonical void input, and the
inverse restores the canonical void labels.

The registered 540-arm observer closes this inverse at
\(1.11\times10^{-16}\). This corrects any reading in which the ordinary
accepted radial subtraction itself was assumed many-to-one.

There are two exact scope boundaries:

1. at \(d=1\), every incoming \(W\) maps to \(W'=0\), so the wave state must be
   retained elsewhere;
2. noncanonical labels already present on a void site are overwritten and
   likewise require a record.

The dual path is still simpler at the field level because it does not drain
\(J\) or \(W\); it nevertheless overwrites the void labels.

## 2. The Bernoulli acceptance law has an exact deterministic dilation

Let \(u\) be a uniform environmental phase in \([0,1)\), and let

\[
p(x)=1-e^{-x/k_m}\in(0,1).
\]

Define

\[
(b,u')=
\begin{cases}
(1,u/p),&u<p,\\
(0,(u-p)/(1-p)),&u\ge p.
\end{cases}
\]

Then

\[
u=
\begin{cases}
pu',&b=1,\\
p+(1-p)u',&b=0.
\end{cases}
\]

This is a bijection between one uniform phase and a Bernoulli branch carrying
its normalized future phase. With the codomain branch weights \(p\) and
\(1-p\), it is probability preserving. The 16 registered arms invert exactly
in binary64 arithmetic.

Therefore the exponential probability is not, by itself, incompatible with a
deterministic underlying environment. It can be read as a threshold on an
unobserved phase.

Production does not implement this phase dynamics. Its `voxel_uniform` value
is a pure function of seed, site, tick, and stream. It is a reproducible
external schedule. Calling the output a physical reservoir microstate would
add an interpretation and a state-update law not present in the engine.

## 3. Erasing branch outcomes recreates the history obstruction

If \(b\) is discarded, every interior \(u'\) has two preimages:

\[
u_1=pu',
\qquad
u_0=p+(1-p)u'.
\]

After \(N\) erased trials, each future phase has \(2^N\) branch-labelled
preimages. Exact reversal therefore requires \(N\) branch bits in the worst
case. The 20-step control has

\[
2^{20}=1,048,576
\]

preimages and recovers its initial phase exactly only when all 20 branches are
retained.

This is the binary specialization of FTD-0499. One exact real can hide an
infinite symbolic past in its digits, but the engine has no such protected
two-sided phase, and a finite-precision local payload cannot retain an
indefinite event history.

## 4. Production evaporation is not reverse genesis

Let \(G\) denote accepted genesis and \(E\) the production evaporation
assignment applied to the resulting manifested state. Since evaporation
leaves the continuous fields unchanged,

\[
E\circ G(0,J,W)
=
\left(0,J-k_gn,(1-d)W\right).
\]

Hence

\[
\left|J-(E\circ G)_J\right|=k_g,
\qquad
\left|W-(E\circ G)_W\right|=d|W|.
\]

The registered composition residuals close these identities below
\(1.12\times10^{-16}\). Evaporation deletes the manifested labels but does not
restore either drained field.

For the exact pair of raw states \(a\xrightarrow{G}b\), the production event
kernel therefore has

\[
P(a,b)>0,
\qquad
P(b,a)=0.
\]

No strictly positive stationary weights can satisfy same-kernel detailed
balance

\[
\pi(a)P(a,b)=\pi(b)P(b,a)
\]

on that pair. This does not rule out reversible microscopic dynamics driven by
a nonequilibrium environment; it proves that its reverse process is not the
projected production evaporation rule.

## 5. Exact extended energy requires a continuous, branch-dependent payload

The single-path field withdrawal is

\[
D(x,W,d)
=k_gx+\frac{k_g^2}{2}
+\left(d-\frac{d^2}{2}\right)|W|^2.
\]

For fixed \(k_g>0\),

\[
\frac{\partial D}{\partial x}=k_g.
\]

Thus an exact continuum model cannot balance every overshoot with finitely
many reservoir energy levels or with one fixed ternary-state energy quantum.
The reservoir exchange must vary continuously with the incoming field and the
selected drain. The dual path assigns the same kind of ternary state while
withdrawing zero from \(J/W\), so the exchange law must also be branch aware.

For the actual binary64 engine, the set of withdrawals is finite but enormous;
one finite-horizon record can encode it. That finite-state fact does not evade
the indefinite-history theorem: repeated unmatched events either exhaust the
record, export it, or change the raw transition.

## 6. Classification

The minimal exact interpretation is now:

- **accepted genesis:** conditionally reversible for canonical inputs and
  \(d<1\);
- **acceptance probability:** admits a one-event deterministic phase dilation;
- **production RNG:** a stateless prescribed schedule, not a dynamical bath;
- **evaporation:** not reverse genesis and not same-kernel detailed-balanced;
- **closed-cycle energy/information:** requires a continuous energy carrier and
  an indefinitely extensible/exported history.

Therefore a finite local reservoir cannot turn the frozen production cycle
into an autonomous reversible common-action system. The reservoir route
survives only as an explicitly open-system construction: environmental degrees
of freedom must arrive, absorb energy and records, and depart or be reset.

This is compatible with a flame-like ontology of matter as a maintained
nonequilibrium pattern. It is not compatible with calling the current isolated
voxel variables a closed, unitary particle system.

## 7. Non-implications

- Fundamental irreversibility remains an allowed FTD postulate; no reservoir is
  required unless global reversibility or unitarity is claimed.
- An uncontained environment may carry records outward; FTD-0569 does not
  require a finite container.
- The result does not derive the reservoir, its spectrum, its temperature, or
  its coupling action.
- No charge unit, mobile bound state, reaction current, production toggle, or
  scenario is produced.
