# Conserved inventory currents and a finite-scale weak continuity bound

Date: 2026-09-05. Candidate: `phi-v2-staged-candidate-1`.
Status before implementation: **[REGISTERED CONDITIONAL PROOF CONTRACT V2]**.

## Domain and fixed questions

Use every admissible periodic complete state of the unchanged staged law.
Define the positive inventory of each polarity at a stored owner as the number
of bank channels plus primary/reserve SC/FCC records of that polarity. This
counts tokens at storage anchors, not ternary manifestation, endpoint incidence,
physical mass, electric charge or energy. In particular an FCC record's stored
owner need not be one of its two geometric endpoints.

The initial v1 question, "one-hop currents only at streaming", was rejected
during source review before implementation: a negative-direction admission
consumes a field at x and stores the relation at x-e_axis. Omitting this current
would violate local continuity despite preserving global count. That rejected
premise is retained here as provenance; no validation used it.

V2 tests exact per-owner continuity with one-hop currents at streaming AND
negative-direction admission. Positive-direction admission stores at the same
owner. Collisions and relation updates retain the owner's two polarity counts.
Each transfer lists whether it is streaming or admission.
The observer must execute the actual law and preserve all pending records;
currents cannot be inferred merely by matching equal endpoint totals.

For arbitrary rational test values on the finite torus, the exact weak identity
must be the sum of oriented edge differences weighted by transferred counts.
All arrays returned as evidence are immutable. Validation includes seam hops,
both polarities, absorption and local collision, and corrupted report rejection.
These are bounded deterministic validation fixtures, not a measurement campaign.

## Continuum comparison fixed before checks

Choose external chart spacing a>0 and a periodic twice differentiable test
function f, with a declared bound B on the absolute pure second derivatives
along axial edges. For each transfer hop Taylor's integral remainder is at most B a^2/2.
Consequently replacing exact edge differences by a d dot grad(f) at the source
has absolute error at most N_hops B a^2/2, divided by a declared positive
normalizing count if normalized measures are used. Summing over a finite
horizon uses the total actual number of hops; no temporal independence is used.

With total initial inventory M, H ticks and microtick chart duration tau,
N_hops<=M H is a conservative bound. For normalization M>0 and H tau<=T,
the accumulated error is at most B T a^2/(2 tau). At fixed bounded a/tau this
is a finite-scale O(a) weak residual. The tighter actual hop count is retained.
This is consistency of a conserved inventory current. Identifying a limiting
measure/current requires separate convergence/compactness assumptions; a
constitutive law, relaxation, physical mass or gravitational source does not
follow. Spacetime edge interpolation is external comparison data, not extra
primitive motion between physical microticks.

Any failed finite inventory identity rejects this proof contract. No fitted
transport coefficient or downstream physical identification enters the test.

## Proof and implementation

**[THEOREM — CONDITIONAL ON THE SELECTED LAW AND STORAGE MEASURE]** Let
rho_p(x,n) count inventory at stored owner x and polarity p. For every actual
stage the observer constructs integer edge counts Q_p(x,d,n). At admission it
counts each uniquely admitted negative-direction channel from x to x-e_axis;
positive-direction admissions need no edge current. At streaming it counts
every occupied channel along its pre-stream tangent. At other stages Q=0.
Then, exactly,

`rho_p(x,n+1)-rho_p(x,n) = sum_d Q_p(x-d,d,n)-sum_d Q_p(x,d,n)`.

The proof uses the actual finite maps: collision is a per-owner, per-polarity
cardinality permutation; relation rotation/swap preserves the two counts at
its stored owner; manifestation and control expiry do not carry tokens.
Admission removes exactly one channel and adds exactly one same-polarity SC
reserve at its declared owner. Streaming is injective and takes one axial hop.
There are no other sources or transfers. This proves local balance, including
periodic seams, without assigning persistent identities at ambiguous collisions.

Multiply by any finite rational owner test values and sum. Reindexing the
incoming term on the finite torus gives the exact weak identity
`sum_x f(x) delta rho_p(x) = sum_(x,d) Q_p(x,d) [f(x+d)-f(x)]`.
The spatial Taylor bound above follows by applying the one-dimensional integral
remainder along each axial edge and summing absolute errors. A periodic smooth
test function is required at seams; an arbitrary nonperiodic coordinate ramp
does not satisfy that continuum comparison contract.

`recovery_continuity.advance_observed` executes the actual complete transition
and publishes immutable inventories/transfers. A supplied `ContinuityStep`
checks algebraic balance and its finite domain; it does not authenticate a
microscopic history or uniquely recover currents from endpoint densities.
The error-bound API likewise checks rational inputs, not the caller's claimed
derivative bound or convergence of the measure/current sequence.

Initial validation passed **14 tests** including both polarities, three
negative-axis seam admissions, local collision, dense streaming, all four
stages, unchanged caller state, exact rational weak balance and corrupted
reports. Independent review then found fixed-width overflow in a supplied
report's sum of two large NumPy counts. Canonicalizing every accepted numeric
payload to Python integers repairs that mathematical input path. The final
**15 tests passed independently in 0.63 seconds**, including the exact
`2*(2^63-1)` regression and rejection of floating-point polarity labels.
The reviewer accepted the scoped local-current proof and conditional bound.
