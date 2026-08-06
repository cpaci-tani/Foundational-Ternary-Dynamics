# Pre-registration — Native Hodge energy and central-continuity gate (FTD-0576)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parents:** FTD-0478, FTD-0502, FTD-0543, FTD-0574, FTD-0575.  
**Production changes permitted:** none. Observer code, tests, proofs, theorem,
audit, and documentation corrections are permitted.

## 1. Question

FTD-0575 derived the reciprocal Hodge force of the exact FTD-0574 source
action but left exact finite-step common energy and mobile matter open. This
campaign asks:

1. What is the exact work performed by one affinely driven production
   kick-drift field tick?
2. Which time-staggered field coordinate is selected by that identity?
3. What continuity equation is sufficient for exact Hodge field, interaction,
   and matter-energy exchange?
4. Can a legitimate cardinal one-site polarity hop satisfy that continuity
   equation with a finite-range local current?
5. Can the already derived local face current be converted to the native
   site-centered current by a finite-range translation-covariant commuting
   projection?

No search over currents, stencils, forces, or tolerances is permitted. The
native central operators and the FTD-0478 cardinal endpoint representation are
frozen.

## 2. Frozen driven field map and work identity

On a periodic finite quotient let `K` be the symmetric production field
operator and let an arbitrary prescribed source kick be `S_n`:

```text
W_1 = W_0-K J_0+S_n,
J_1 = J_0+W_1.
```

Use the FTD-0574 invariant

```text
H(J,W)=1/2<W,W>+1/2<J,KJ>-1/2<W,KJ>.
```

The registered identity is

```text
H_1-H_0=<S_n,(W_0+W_1)/2>.                       (1)
```

Define

```text
R_n=J_n-W_n/2.                                   (2)
```

Then

```text
R_1-R_0=(W_0+W_1)/2,                             (3)
H_1-H_0=<S_n,R_1-R_0>.                           (4)
```

Within `R=J-cW`, equation (3) must select `c=1/2` uniquely. For constant
`S`, the exact affine invariant is

```text
H_S=H-<S,R>.                                     (5)
```

## 3. Conditional exact Hodge energy identity

Let density endpoints `rho_0,rho_1` and an integrated site-vector current
`Q` satisfy the same native central divergence `D` used in FTD-0574:

```text
rho_1-rho_0+D Q=0.                               (6)
```

Set

```text
rho_bar=(rho_0+rho_1)/2,
R_bar=(R_0+R_1)/2,
delta_R=R_1-R_0,
S=-G_C G rho_bar+G_C C Q.                        (7)
```

Using `D^T=-G` and `C^T=C`, require the exact identities

```text
delta H_field
 =G_C<rho_bar,D delta_R>+G_C<Q,C delta_R>,       (8)

delta U_int,
U_int=-G_C<rho,D R>,
 =-G_C<rho_bar,D delta_R>-G_C<Q,G D R_bar>,      (9)

delta H_matter
 =G_C<Q,G D R_bar-C delta_R>.                    (10)
```

Equations (8)--(10) must sum to zero. Equation (10) is a conditional matched
work requirement, not a claim that the frozen production movement phase
implements it. A magnetic term proportional to the path velocity crossed with
`C^2 R_bar` may be added only as a zero-work identity; it cannot repair failed
continuity.

## 4. Cardinal-hop locality discriminator

Restrict an axial hop to one dimension. Let `z` denote the translation symbol.
For the cardinal endpoint shape, the start symbol is `P(z)=1` and the endpoint
difference is `(z-1)P(z)`. The central-difference divergence symbol is

```text
d_c(z)=(z-z^-1)/2=(z-1)(z+1)/(2z).               (11)
```

Exact continuity forces

```text
Q(z)=-2z P(z)/(z+1).                             (12)
```

The candidate is locally admissible only if `Q(z)` is a finite Laurent
polynomial. For the cardinal shape it has a pole at `z=-1` and is not
finite-range.

Registered consequences:

- even periodic `L in {16,32,64}`: the `k=pi` source component is nonzero
  while `d_c(pi)=0`; no current solution exists;
- odd periodic `L in {17,33,65}`: an exact solution exists, but every solution
  of minimum norm has support spanning the periodic box and does not approach
  a fixed support radius as `L` grows;
- both polarities, all three axes, integer translations, and all 24 proper
  cubic rotations preserve the classification.

The odd-volume solve is structural evidence for nonlocality, not a continuum
fit. Residuals must be below `1e-12`.

## 5. Face-to-site commuting projection discriminator

For oriented face current, the axial divergence symbol contains the hop factor

```text
d_f(z)=1-z^-1.
```

A translation-invariant projection `A` from face current to native site
current would have to satisfy

```text
d_c(z) A(z)=d_f(z),
A(z)=2/(z+1).                                    (13)
```

No finite-range projection has this symbol. This is the registered
face/native bridge obstruction. A box-dependent global projection, an
additional staggered current primitive, or a change of native operators is an
enlarged/selected model and must not be reported as closure of the frozen
action.

## 6. Registered arms

- 36 scalar mode work arms:
  `L in {16,32,64}`, `n in {0,1,2,3}`, and the three principal directions;
- four periodic full-field driven-work arms: `L in {5,7}` and two deterministic
  fixtures;
- four conditional total-energy arms on odd volumes using independently
  generated `rho_0,rho_1,Q,R_0,R_1` fixtures;
- 18 axial cardinal-hop solvability arms:
  six volumes, three axes, with both polarities checked inside each arm;
- 24 proper-cubic covariance controls;
- exact symbolic checks of equations (1)--(13), including uniqueness of
  `c=1/2` and the pole at `z=-1`.

All algebraic, work, continuity, covariance, and conditional-energy residuals
must be `<=1e-12`. Even-volume solver residuals must remain strictly nonzero
at the checkerboard mode. Odd-volume support must grow with `L` and reach at
least `(L-1)/2` in the registered minimum-norm solution.

## 7. Outcome map

Positive registered verdict:

```text
NATIVE_HODGE_ENERGY_IDENTITY_CENTRAL_LOCAL_MOBILE_CURRENT_OBSTRUCTED
```

It establishes an exact finite-step field/source work coordinate and a
conditional total-energy identity, while closing a finite-range local
cardinal-hop current and finite-range face-to-native projection for the frozen
central operators.

The verdict does not close all nonlinear or enlarged mobile matter. It leaves
three explicit routes:

1. select face/link fields and their matched divergence as a distinct
   dynamics;
2. introduce a staggered current/connection primitive;
3. derive a non-cardinal or nonlinear carrier whose endpoint symbol cancels
   the checkerboard factor without losing the ternary manifestation contract.

No production branch, toggle, scenario, particle, electromagnetic, Lorentz,
or unitarity claim is licensed.

## 8. Frozen production provenance

The implementation must verify the following SHA-256 hashes and must not edit
these files:

```text
phase_read.cpp                  D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                 2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
field_operators.h               25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h        3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
```
