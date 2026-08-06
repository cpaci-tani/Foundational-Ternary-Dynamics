# State-only boundary-energy ledger

**Status:** `[THEOREM — CONDITIONAL ON THE SELECTED FTD-0754 SUPPORT AND READOUT] + [NUMERICAL FACT — EXISTING DISCOVERY CORPUS ONLY] + [CORRECTION — CENTERED CROSS ENERGY IS NOT PURE BOUNDARY ENERGY]`  
**Date:** 2026-07-30  
**Bookkeeping:** FTD-0754 analytic addendum (“FTD-0754B”); this is not a new
ledger identifier and FTD-0755 remains reserved for held-out M3 validation.  
**Certificate:** `scripts/proofs/proof_state_only_boundary_accounting.py` —
27/27 checks.

## 1. Verdict

The FTD-0754 bound--residual interference has an exact three-term
decomposition:

\[
I_{\rm ctr}=I_{\partial K}+I_A+I_B.
\]

- \(I_{\partial K}\) is the primitive-face exchange across the selected
  finite-support dressing boundary.
- \(I_A\) is the change induced by mapping face flux to the centered electric
  readout.
- \(I_B\) is the cross term introduced by reconstructing the integer-time
  centered magnetic readout from the half-step edge field and electric curl
  adjoint.

Only the first term has the direct discrete Green-identity interpretation as
support-boundary exchange. The total centered cross energy is not itself a pure
boundary energy and is not an invariant measure of energy “owned” by matter.

The theorem is an identity for the selected observer. The measurements below
reuse only the already-seen FTD-0753/0754 face, edge, and body histories. No
held-out perturbation, volume, support radius, or M3 validation state was
inspected.

## 2. Primitive support complex

Let \(K\) be the selected finite induced cubic graph and let
\(F_K\) be its internally oriented nearest-neighbour faces. The FTD-0754
preparation supplies

\[
E_{\rm b}=G_K\phi,
\qquad
D_KE_{\rm b}=\rho,
\]

with the orientation convention

\[
(G_K\phi)_{u\to v}=\phi_u-\phi_v.
\]

The selected solution has zero field on every face crossing the support
boundary and zero field outside. It is the minimum primitive-face energy
solution under that support condition. Let

\[
E_{\rm r}=E-E_{\rm b}.
\]

The observer requires the actual and selected fields to have the same Gauss
source, so

\[
DE_{\rm r}=0.
\]

At a support site, split the full divergence into internal and crossing-face
parts:

\[
D E_{\rm r}=D_K E_{\rm r}+D_{\partial K}E_{\rm r}=0.
\]

Discrete summation by parts gives

\[
\begin{aligned}
I_{\partial K}
  &:=\langle E_{\rm b},E_{\rm r}\rangle_{F_K}\\
  &=\langle G_K\phi,E_{\rm r}\rangle_{F_K}\\
  &=\langle\phi,D_KE_{\rm r}\rangle_K\\
  &=-\langle\phi,D_{\partial K}E_{\rm r}\rangle_K.
\end{aligned}
\]

This is exact in finite arithmetic up to the registered Poisson/Gauss
tolerances. The arbitrary additive constant in \(\phi\) cancels because the
net residual flux through the closed support boundary is zero.

The identity is local to the selected support surface. It does not say that
the interaction is an intrinsic constituent energy, that this surface is
ontologically unique, or that the value is independent of support radius.

## 3. Why the centered observer adds two terms

FTD-0754 does not evaluate the primitive face norm directly. Let \(A\) be the
linear face-to-site electric centering map, \(Q\) the edge-to-site magnetic
centering map, \(C^*\) the face-to-edge curl adjoint, and

\[
h=-\frac{c\,\Delta t}{2}.
\]

The integer-time centered fields are

\[
\bar E=AE,
\qquad
\bar B=Q(B_{1/2}+hC^*E).
\]

The selected bound preparation has \(B_{{\rm b},1/2}=0\), hence

\[
\begin{aligned}
\bar E_{\rm b}&=AE_{\rm b},
&\bar E_{\rm r}&=AE_{\rm r},\\
\bar B_{\rm b}&=Q(hC^*E_{\rm b}),
&\bar B_{\rm r}&=Q(B_{1/2}+hC^*E_{\rm r}).
\end{aligned}
\]

Polarizing the registered centered quadratic energy gives

\[
I_{\rm ctr}
=\langle AE_{\rm b},AE_{\rm r}\rangle
 +\left\langle Q(hC^*E_{\rm b}),
 Q(B_{1/2}+hC^*E_{\rm r})\right\rangle.
\]

Add and subtract the primitive-face inner product:

\[
\boxed{
I_{\rm ctr}=I_{\partial K}+I_A+I_B
}
\]

with

\[
\begin{aligned}
I_A&=\langle AE_{\rm b},AE_{\rm r}\rangle
     -\langle E_{\rm b},E_{\rm r}\rangle_{F_K},\\
I_B&=\left\langle Q(hC^*E_{\rm b}),
 Q(B_{1/2}+hC^*E_{\rm r})\right\rangle.
\end{aligned}
\]

No field equation beyond the declared observer definitions is required for
this three-term identity. The boundary interpretation of the first term uses
Gauss compatibility and zero-crossing compact support from Section 2.

## 4. Algebraic and covariance controls

The observer now records all three terms and two independent residuals:

\[
\epsilon_{\partial}
=I_{\partial K}
 +\langle\phi,D_{\partial K}E_{\rm r}\rangle_K,
\]

\[
\epsilon_{\rm ctr}
=I_{\rm ctr}-I_{\partial K}-I_A-I_B.
\]

A nontrivial Gauss-free electric plaquette crossing the support boundary gives

\[
\begin{aligned}
I_{\partial K}&=-2.7528159271024004\times10^{-4},\\
-\langle\phi,D_{\partial K}E_{\rm r}\rangle_K
&=-2.7528159271024004\times10^{-4},\\
I_A&=+6.0341383763146258\times10^{-5},\\
I_B&=-1.4549852119527404\times10^{-5}.
\end{aligned}
\]

The algebraic and proper-cubic/translation/polarity covariance tests pass 2/2.
The build now links OpenMP to `ftd_eft`, where the declared deterministic
x-slab observer loop actually lives. A serial and 32-thread replay of the face
arm produce the identical CSV SHA-256
`F82FB116590AD94E136EABBA312823BA552BD33C75A0B6B08B77E04F4DCB8C95`.
This changes observer throughput only; no tick rule or result value changes.

## 5. Existing-corpus numerical result

The post-hoc observer replayed the same periodic \(L=321\), tick-312
face/edge/body histories and the same frozen observer ticks
`{0,80,96,115,160,240,297,312}`.

The independent certificate gives:

- old FTD-0754 total-interference strings: 24/24 exact;
- state-only observations: 24/24 valid;
- boundary ledgers: 24/24 valid;
- maximum primitive/boundary identity residual: `3.6234e-16`;
- maximum direct three-term reconstruction residual: `1.3010e-18`;
- maximum recorded readout reconstruction residual: `1.3620e-18`;
- maximum net support-boundary residual flux: `7.2772e-17`;
- independent certificate: 27/27.

Across the 21 noninitial discovery snapshots:

- \(I_{\partial K}\) is nonzero and negative in 21/21;
- the primitive boundary term has the largest absolute component in 19/21;
- the centering term is largest in 2/21;
- the magnetic term is largest in 0/21;
- the mean primitive-boundary share of component \(L^1\) magnitude is
  `0.7244`;
- cancellation reaches `41.8102x` on the face arm at tick 80.

The last two numbers are descriptive summaries of this discovery corpus, not
universal constants or validation thresholds.

Representative rows are:

| arm/tick | centered total | primitive boundary | centering | magnetic |
|---|---:|---:|---:|---:|
| face/80 | `-5.4378e-5` | `-1.15085e-3` | `+1.10958e-3` | `-1.31093e-5` |
| face/115 | `+3.93775e-4` | `-1.15891e-3` | `+1.53364e-3` | `+1.90483e-5` |
| face/160 | `-1.61236e-3` | `-7.81751e-4` | `-8.31264e-4` | `+6.58222e-7` |
| face/312 | `-1.80344e-3` | `-9.74231e-4` | `-8.27834e-4` | `-1.37523e-6` |
| edge/312 | `-1.11138e-3` | `-1.22092e-3` | `+1.11570e-4` | `-2.03550e-6` |
| body/312 | `-8.30444e-4` | `-1.29907e-3` | `+4.71421e-4` | `-2.79042e-6` |

At face tick 115 the primitive boundary exchange remains negative while the
centered total becomes positive. The sign reversal is caused by the centering
term. Therefore the sign or magnitude of `bound_residual_interference` cannot
serve as a matter-membership margin or an invariant subsystem-energy test.

## 6. Corrected ontological statement

The evidence now supports the following narrower language.

1. The selected Gauss dressing is state-determined relational data associated
   with the candidate core under the declared finite-support convention.
2. Primitive-face exchange across the selected support is an exact relational
   boundary term when the residual is Gauss-free.
3. Centered streamlines and their quadratic energy are an observer/readout,
   not the primitive energy-ownership ledger.
4. The support boundary is a bookkeeping surface, not necessarily a material
   membrane.
5. The candidate object is an open subsystem. Its internal energy, boundary
   interaction, and environmental energy must remain separate entries in any
   future common-action ledger.

The result does not prove that \(K\) is the unique object boundary, that
\(F_{\rm b}\) is ontologically part of matter rather than a constraint response,
or that boundary exchange stabilizes as the support changes. It does eliminate
the naive interpretation of the centered cross energy as a literal aura energy
owned by the object.

## 7. Recursive questions opened by the theorem

1. **Support-surface stability:** does the object projection remain fixed while
   \(I_{\partial K}\), \(I_A\), and \(I_B\) redistribute over a preregistered
   support-radius ladder?
2. **Action matching:** is \(I_{\partial K}\) exactly the surface term obtained
   by restricting the native common action to the candidate subsystem?
3. **Causal fibre:** do two states with the same core/dressing but different
   exterior divergence-free fields have identical pre-contact boundary ledgers?
4. **Moving boundary:** when the core translates, what discrete Reynolds-like
   transport term accompanies the moving support surface?
5. **Formation:** does entry into the matter family coincide with a persistent
   conversion from through-flow to stored internal/bound energy?
6. **Decay:** does exit produce a controlled transfer from internal energy
   through \(I_{\partial K}\) into outgoing characteristics?
7. **Composition:** for two separated cores, does the primitive ledger split
   into two surface terms plus a decaying inter-object interaction?
8. **Charge:** does a reaction-complete invariant constrain the boundary flux,
   or is polarity only a source orientation for the selected dressing?
9. **Quiet matter:** can a stable core have zero outgoing characteristic while
   retaining nonzero internal energy and zero time-averaged boundary work?
10. **Physical pole:** after M3--M6, does the common-action subsystem carry a
    positive-residue localized excitation independent of the observer surface?

## 8. Consequence for FTD-0755

FTD-0755 must not use the centered total cross term as a classifier margin.
Its pre-registration should instead keep four ledgers distinct:

\[
E_{\rm internal},\qquad
I_{\partial K},\qquad
I_A+I_B,\qquad
E_{\rm environmental}.
\]

It should freeze at least two admissible support radii or supply a proof that
one radius is selected, then test whether object identity and common-action
exchange are stable while readout terms redistribute. It should also freeze
the causal-fibre, quiet-core, incoming-packet, nested-volume, and ordinary
negative controls already required by FTD-0743.

No held-out state has been consumed here. Production defaults, tick rules,
scenarios, matter ontology, M3 status, and the FTD-0755 identifier remain
unchanged.
