# Exact one-collision response of the stationary kinetic reference

Date: 2026-09-05. **[PREREGISTRATION — BOUNDED ANALYTICAL RESPONSE]**.
Law: `phi-v2-staged-candidate-1`; preparation:
`strict-kinetic-counting-reference-1`; response protocol:
`strict-kinetic-response-1`.

## Frozen domain and observables before validation

Use the homogeneous doubly occupied relation background, s=0, and uniform
ell, as in DERIV_STRICT_KINETIC_REFERENCE.md. The external reference is exactly
p=1/96 on every field bit. Consider separately each of the three collision
layers and either polarity. At a specified site perturb the product
preparation by p_i=p+epsilon h_i, for an externally declared rational h and
sufficiently small epsilon that all occupancies remain in [0,1]. Other sites
and the other polarity remain at reference. Full saved gates retain their
deterministic preparation correlations.

Preregistered probes are h=e_0, the balanced channel probe e_0-e_1, and the
uniform probe h_i=1. The selected pair observable in each layer is the frozen
table image of (0,1), chosen by label before examining the response. Validate
the general marginal operator on all 192 channel directions. Observables are
one-bit means and connected two-bit covariances; norms are maximum absolute
pair-covariance derivative and the integer support of that derivative.
The horizon is one actual collision transaction (physical tick 1 -> 2 from
a cycle boundary), optionally followed by its actual streaming transaction
(tick 2 -> 3). No iteration of the marginal operator is licensed.

Derive formulas from sparse exactly-two corrections to the full finite
counting reference. Enumerate all 18,336 pairs per layer, not all 2^192 local
configurations and not global random trajectories. No empirical fits, search
for coincidences, GPU campaign, or source-law changes are part of this work.

Optionally classify additive channel moments by exact finite linear algebra:
collision conservation means v_a+v_b=v_c+v_d for every actual table row;
global streaming conservation also requires v_U(c)=v_c. Test the declared
scalar constant, axial tangent components, and the existing six-component
field/layer readouts. A complete common-kernel calculation, if feasible,
must report its full dimension and exact basis rather than selected matches.

## Analytical identities to validate

Let A be a two-channel input subset, F(A) its actual frozen collision output,
and Delta f(A)=f(F(A))-f(A). Outside this sector the collision is identity.
At equal p, every pair has weight w=p^2(1-p)^190 and sum_A Delta f(A)=0.
Differentiating its product weight gives

d E_out[f] = d E_in[f]
  + r sum_A Delta f(A) sum_(i in A) h_i,   r=p(1-p)^189.

The term proportional to sum_i h_i cancels exactly by the pair permutation.
Define the integer 192x192 matrix

K_ji = sum_A (1[j in F(A)]-1[j in A]) 1[i in A].

Then the one-collision marginal Jacobian is J=I+rK. For a pair B={j,k}, let
L_Bi=1[i in F^(-1)(B)]-1[i in B]. The generated connected covariance is

d Cov_out(X_j,X_k) = r [L_B - p(K_j+K_k)] h.

The incoming covariance derivative is zero for this product preparation.
This identity quantifies full counting-ensemble tangent content discarded by
a repeated product-marginal closure. Equal-p reference invariance and uniform
reference-family tangency do not imply closure for nonuniform h.

Streaming only relabels slots: (x,j) becomes (x+d(j),U(j)). It transports
each covariance to the corresponding pair of slots, potentially at different
sites, without erasing it. This is an exact one-step spatial interpretation,
not a relaxation or continuum theorem.

## Exact response obtained after the preregistration

**[THEOREM — PREPARED COUNTING-ENSEMBLE DERIVATIVE]** The identities above
hold for every rational interior p and rational probe h. The external
preparation is a genuine product family for sufficiently small epsilon;
its full-state pushforward is used, including correlations. These are exact
derivatives of finite rational-polynomial expectations, with no finite
amplitude, large-time or continuum interchange of limits.

At the prescribed p=1/96 and h=e_0, all three layers give:

| Observable | Exact response |
|---|---:|
| Nonzero connected pair derivatives | 10,305 of 18,336 |
| Maximum absolute connected pair derivative | (55/32) r |
| Derivative for B=F((0,1)) | (23/24) r |
| Perturbed-channel marginal derivative | 1-191 r |

Here r=(1/96)(95/96)^189. The selected B values are (4,5), (12,13),
and (44,45), respectively. They were selected as the image of the fixed
input label (0,1), not chosen by a favorable response value. The uniform
probe has J h=h and exactly zero covariance derivative, as required by
stationarity of the entire equal-p reference family. The balanced probe
and all 192 Jacobian columns are also checked against actual table sums.

This is a quantitative first-order failure of independent-product tangent
closure at the event-eligible reference itself. The covariance is not a
second-order numerical residual. Streaming carries it to definite (possibly
distinct-site) slots; discarding it after each transaction changes the
ensemble. No claim is made here that every such component returns to every
chosen later marginal. The later appendix bounds total finite-horizon tangent
error at a stated scope without identifying those specific feedback channels.

K has zero row and column sums, but its off-diagonal entries range from -1
to 146. Thus J has negative off-diagonal response coefficients for interior p:
it is a marginal derivative, not a stochastic channel-transition matrix.

## Complete fixed additive field-moment classification

For each collision row form the integer constraint vector
delta=e_c+e_d-e_a-e_b. Add every such outer product delta delta^T for all
three layers. Add (e_U(i)-e_i)(e_U(i)-e_i)^T for every channel i. Let this
192x192 integer Gram matrix be G. Since v^T G v is a sum of squares,
ker(G) is exactly the intersection of all these conservation constraints.

Exact fraction-free rational linear algebra gives rank(G)=191 and the
primitive integer kernel basis (1,...,1). Consequently the ONLY fixed,
spatially uniform additive field-channel moments conserved by every collision
layer and streaming are multiples of field population in each polarity.
The 384-channel space has two independent copies, N_+ and N_-; any fixed
vector-valued invariant has each component in their span. This concerns the
prepared fully occupied background where admissions are absent.

The supplied six-component layer readouts do satisfy their own collision
layer's constraints; those selected stage identities do not create additional
fixed common invariants. In particular the three axial tangent moments fail
the streaming condition. Time-dependent/covariant channel weights, spatially
varying observables, nonlinear invariants, retained relation/gate observables
and emergent material quantities are not classified by this calculation.

## Verification and remaining gate

Tests independently construct the full incidence-matrix contraction for every
Jacobian entry; differentiate actual finite product weights with their full
normalization term; and use exact finite rational +/-epsilon evaluations for
the single-channel probe. In that probe the first moments are affine and
the connected covariance is quadratic, so the symmetric secant cancels its
known quadratic term exactly. This is an algebraic identity, not a floating
epsilon convergence experiment. Actual staged streaming checks both polarities
at a periodic seam. Exact Gram nullspace computation gives the complete
normalized basis, not a sampled selection of conserved values.

The 23 focused regressions passed in 9.56 seconds. The exact common-kernel
calculation itself completed in approximately 1.35 seconds in a separate
bounded check. These timings describe analytical finite-map instrumentation,
not a simulation or relaxation campaign. Invalid reference values, malformed
probes, mixed-type cached layer keys and noninteger polarities fail closed.

**[OPEN]** Detailed repeated full-ensemble tangent dynamics, the transfer and
return of these correlations, relaxation times, spectral/kinetic closure,
hydrodynamic scaling and bounds beyond the specified preparation/comparison
remain outstanding.
The restricted finite-horizon tangent norm bound below supplies a conservative
comparison without resolving these stronger questions. The newly computed
operator and leakage provide fixed observables and rational baselines for a
separately registered response campaign; they do not authorize J to be iterated
as a closed physical law. No trajectory campaign or change to Phi was made.

## Added contract before leakage-budget validation

The next bounded proof obligation uses the same p=1/96 reference and unchanged
finite maps. Identify each complete phase-lift support with its bank; gates
remain deterministic correlated records. Work in real weighted L2 of this
finite reference measure. P is orthogonal projection onto the first-degree,
mean-zero channel-score subspace. The collision/streaming pushforwards are
isometries between the correctly indexed reference fibers.

For h, verify exact unresolved squared norm
(sum h_i^2 - sum (Jh)_i^2)/(p(1-p)); reject a negative radicand rather than
silently clamping it. This includes every higher Boolean order, not only pairs.
For every finite sequence of physical stages, prove a Duhamel estimate against
the iterated projected stages using the sum of the unresolved norms produced
by those projected inputs. Approximation does not erase errors in the actual
full score, including components which later return to retained coordinates.

The executable finite-horizon specialization repeats the declared channel
probe at every spatial site in one polarity, so streaming acts by the exact U
permutation on its spatially uniform coefficient vector. Its norm counts all
L^3 sites. Validate horizons 0 through 8, all four starting phases with their
declared current uniform layer, and an exact eight-stage full-L2 finite-model
countercheck. General spatially varying full-score Duhamel estimates share the
proof, but this compact horizon implementation does not solve their projected
spatial dynamics. Use a declared rational square-root upper enclosure, never a
floating approximation, to report a conservative norm budget.

## Full-Boolean leakage identity

**[THEOREM — FINITE TANGENT NORM]** For the reference field measure mu, set
f_h(X)=sum_i h_i (X_i-p)/(p(1-p)). Independence at the reference gives
||f_h||_mu^2=sum_i h_i^2/(p(1-p)). Each actual field stage is a bijection
preserving mu. Its Perron pushforward U:f -> f composed with F^(-1) is
therefore unitary in this finite weighted L2 space. The stage-indexed complete
reference supports are deterministic graphs over the current bank; this is
an isometric identification retaining the saved-gate correlations, not a
claim of a stationary measure on the absolute clock.

The first-degree score projection of U f_h is f_(Jh). Pythagoras gives

```text
||(I-P) U f_h||^2 = [sum_i h_i^2 - sum_i (Jh)_i^2] / [p(1-p)].
```

This includes every unresolved Boolean degree. An independent sparse check
uses U f_h-f_h: its nonzero values occur only on exactly-two configurations,
where its value is [sum_(F^-1(B)) h - sum_B h]/[p(1-p)]. Summing those exact
squared values and subtracting the retained change reproduces the identity.
The computed result is strictly greater than the squared second-degree
projection obtained from all the pair covariances for h=e_0: higher orders
are present as well.

For the registered h=e_0, every layer has ||K h||^2=58,660 and h^T K h=-191.
Its exact one-site, one-polarity unresolved squared norm is

```text
lambda^2 = [382 r - 58,660 r^2] / [p(1-p)],
p=1/96, r=p(1-p)^189.
```

This is approximately 41.5542 only as a human-readable display; the code and
certificate calculations retain the exact rational value. Uniform h produces
zero leakage, as it is the conserved population score. Negative radicands are
rejected as an operator/validation error; they are never silently clamped.

## Finite-horizon bound without discarding actual returns

**[THEOREM — FULL TANGENT VERSUS PROJECTED COMPARISON]** Let actual full scores
evolve f_(j+1)=U_j f_j, with f_0 in the retained subspace. Define only the
comparison sequence by g_0=f_0 and g_(j+1)=P U_j g_j. Set
r_j=(I-P) U_j g_j. Then

```text
f_(j+1)-g_(j+1) = U_j(f_j-g_j) + r_j,
||f_n-g_n|| <= sum_(j=0)^(n-1) ||r_j||.
```

The first identity is exact; the inequality uses unitarity and the triangle
inequality. All true unresolved components, including any later return to
first degree, remain in f_j. No closure, mixing, or lack of return is assumed.
Streaming maps the entire first-degree space into itself, hence contributes
zero residual. The bank-identity stages also contribute zero. Collision
residuals are given by the exact formula above on the current projected input.

The general theorem applies to spatially varying first-degree fields with
the appropriate full spatial projected stages. The executable specialization
`homogeneous_tangent_budget` instead repeats h at all L^3 sites in one polarity.
For that declared preparation, streaming sends coefficient h_c to h'_U(c)
exactly, collisions use the actual current ell, and ell decreases only at
input phase 1. Different site's unresolved local terms are orthogonal under
the reference product measure, so each squared collision residual is L^3
times the single-site value. No spatial gradient is modeled by this helper.

The bound records every physical-stage collision time and its exact squared
residual, plus a rational upper enclosure for each square root. With enclosure
denominator D, integer ceiling and integer square root produce a value at
least the true root and less than root+1/D. Thus the reported Duhamel sum
is conservative, including enclosure errors. The returned error bound also
uses the valid alternative ||f_0||+||g_n|| when it is smaller. All norms include
all sites, and no square root is converted to floating point in the proof.

For L=3, h=e_0 repeated at every site, ell=0, start tick 0 and D=10^6, the
norm error is bounded by 3,349,571/100,000 after two or four physical ticks
and 13,248,073/250,000 after eight ticks. These certified bounds are loose:
they do not establish a small-error kinetic window. They replace an absent
full-tangent error budget with an explicit, testable one at the stated scope.

For any square-integrable centered observable O, the difference between true
and comparison expectation derivatives is at most ||O|| times this budget.
This is a derivative statement. The next appendix supplies the additional
nonlinear remainder for the specified product preparation; other preparations
and hydrodynamic scaling require additional uniform control.

The independent eight-state projection regression is explicitly an algebraic
toy, not another FTD law: a three-bit, number-preserving permutation exchanges
two pair configurations. Exact weighted full-score iteration returns discarded
information, while repeated first-degree projection does not. The bound holds
at every tested stage, providing a direct check that its implementation does
not assume away return from unresolved modes.

The full-tangent extension passed 34 tests in 10.95 seconds. An independent
reviewer reproduced them in 10.88 seconds and checked six additional exact
identities for a rational multichannel probe at p=1/96 and p=1/3 across all
three layers. Its proof, phase accounting and conservative enclosures were
accepted at the stated scope.

## Finite-amplitude contract before its validation

Retain the same spatially homogeneous, one-polarity preparation and actual
physical horizon. Externally choose rational epsilon with every
0 <= p+epsilon h_i <= 1. For its exact initial product-density ratio r_epsilon,
derive without global enumeration the full weighted-L2 remainder after its
linear term. Use orthogonality of centered independent Bernoulli coordinates,
not a small floating-point approximation. Reject invalid occupancy values.

Transport this remainder under the exact full-law reference isometry and
combine it with |epsilon| times the accepted full-tangent Duhamel bound. The
comparison 1+epsilon g_n has unit integral but may be signed; it is explicitly
an external linear approximation, not an autonomous probability distribution
or a substitute runtime. Validate zero amplitude, zero/uniform probes, endpoint
occupancies, a direct small finite Bernoulli enumeration of the algebraic
identity, and the exact rational triangle budget. No trajectory campaign or
global configuration enumeration is introduced.

## Exact finite-amplitude product remainder and propagated bound

**[THEOREM — FINITE PREPARED ENSEMBLE COMPARISON]** Let rho_epsilon denote the
initial perturbed product measure's density relative to the reference. For
each independent coordinate its density factor is
1+epsilon h_i (X_i-p)/(p(1-p)). Orthogonality of centered coordinate products
gives, for a single spatial copy,

```text
||rho_epsilon||^2 = product_i [1+epsilon^2 h_i^2/(p(1-p))].
```

The constant term has norm squared one and the first-degree term has norm
squared epsilon^2 ||f_h||^2; both are orthogonal to every higher-degree term.
For N=L^3 independent copies of the same channel perturbation in one polarity,
the exact remaining squared norm is therefore

```text
R^2 = {product_i [1+epsilon^2 h_i^2/(p(1-p))]}^N
      - 1 - epsilon^2 N sum_i h_i^2/(p(1-p)).
```

This identity holds at every valid rational amplitude, including perturbed
occupancy endpoints 0 and 1. The reference p remains strictly between 0 and 1.
All initial measures are external finite counting ensembles: rational
coordinate probabilities can be represented by positive integer multiplicities.
No primitive probability, global configuration enumeration or fitted amplitude
is required.

Write the initial density as 1+epsilon f_0+q with ||q||=R. Exact reference
isometries preserve the remainder norm. Combining this with the full-tangent
comparison yields, at the declared physical horizon n,

```text
||rho_epsilon,n - (1+epsilon g_n)||
    <= R + |epsilon| ||f_n-g_n||
    <= sqrt_upper(R^2) + |epsilon| tangent_error_upper.
```

The implementation validates every perturbed occupancy before computing the
horizon budget and includes all N sites in R. A zero amplitude or zero probe
returns zero error. A spatially uniform channel-population probe has no tangent
evolution error, but its nonlinear product density still differs from its
linear truncation; this initial remainder is correctly retained.

The comparison density 1+epsilon g_n has unit integral and may take negative
values. It is an external linear expectation functional, never a replacement
runtime or a claimed evolving product probability. The actual finite ensemble
rho_epsilon,n is nonnegative and evolves under the complete law. For any
square-integrable observable, Cauchy-Schwarz converts the displayed density
bound into its corresponding expectation error using that observable's
reference L2 norm. This supplies a finite-amplitude, finite-horizon comparison
at the specified homogeneous preparation scope; it establishes neither a
closed nonlinear kinetic equation nor a hydrodynamic scaling window.

For L=3, h=e_0 at every site, initial ell=0, start tick 0, eight physical ticks,
epsilon=1/192 and root enclosure denominator 10^6, the certified density
error is at most 625,633/1,920,000. The remainder contribution is at most
49,849/1,000,000 and the transported tangent contribution is at most
13,248,073/48,000,000. These are exact rational upper bounds, not measured
errors. At fixed nonzero amplitude the product remainder need not stay small
as the number of sites grows; no volume-uniform continuum estimate follows.

The expanded 39-test suite passed in 11.27 seconds. Its finite-amplitude
checks include direct three- and six-bit enumeration of the product identity,
all-site accounting, occupancy endpoints, invalid probabilities, zero cases
and preservation of the nonzero linearization remainder for the exact uniform
reference family. These are bounded algebraic fixtures, not FTD trajectory
sampling campaigns.

Independent review reproduced all 39 tests in 10.83 seconds and accepted the
finite-amplitude identity, complete site accounting, endpoint validation,
isometric remainder transport and signed-comparison qualification. No blocking
finding remained within this finite prepared-ensemble comparison scope.
