# FTD-0584 — Configuration-Space Carrier Necessity Theorem

**Status:** `[THEOREM — FIXED-SOURCE REAL-FIELD FIBRE CONTRACTIBILITY]` +
`[THEOREM — FROZEN ZERO-VACUUM HOMOTOPY]` +
`[SCOPED NO-GO — STATIC TWO-DERIVATIVE CORE]` +
`[CLASSIFICATION — MINIMUM TOPOLOGICAL ENLARGEMENT]` +
`[OPEN — ACTIVE NONLINEAR LOCALIZED MODE OR NEW VARIABLE]`  
**Date:** 2026-07-26  
**Verdict:**
`CURRENT_FIXED_SOURCE_FIBRES_CONTRACTIBLE_CURRENT_VACUUM_HAS_NO_DEFECT_HOMOTOPY_STATIC_TWO_DERIVATIVE_CORE_UNSTABLE_MINIMUM_ENLARGEMENT_CLASSIFIED_NOT_DERIVED`

## 1. Scope

This theorem classifies the frozen face-flux research variables before a
nonlinear carrier is invented. Ternary manifestation remains site-valued and
all registered field variables remain ordinary real arrays. Gauss law, global
plane flux, and a fixed ternary source may be imposed. No compact phase,
normalized direction, singular puncture, branch integer, or new action term is
assumed.

The result closes a localized **topologically protected** carrier in these
fixed-source real-field fibres. It does not close all nonlinear, time-periodic,
metastable, or dynamically conserved structures.

## 2. Fixed-source fibres are affine and contractible `[THEOREM]`

Collect every continuous registered field coordinate into
`F in R^M`. On a finite periodic lattice, the frozen matched Gauss and harmonic
conditions have the form

\[
 A F=b(s),\qquad H F=h,
\]

with linear operators `A,H`. If the fibre is nonempty, choose one solution
`F_0`. Every other solution is

\[
 F=F_0+v,\qquad v\in\ker A\cap\ker H.
\]

Therefore

\[
 \mathcal F_{s,h}=F_0+(\ker A\cap\ker H)
\]

is an affine subspace. The explicit homotopy

\[
 \boxed{\mathcal H_t(F)=F_0+t(F-F_0)},\qquad 0\le t\le1,
\]

stays in the fibre because

\[
 A\mathcal H_t(F)=b(s),\qquad H\mathcal H_t(F)=h.
\]

Thus every nonempty fixed-source/fixed-harmonic fibre is convex and
contractible. Any continuous map from such a fibre to an integer-valued
discrete set is constant. A localized integer topological charge cannot
distinguish two configurations inside one fibre.

This strengthens FTD-0583: fixing Gauss charge and global flux does not rescue
local topology. FTD-0583 classified the source-free cohomology; FTD-0584
classifies the constrained solution spaces themselves.

## 3. The uncontained case is the same theorem `[THEOREM]`

No container is needed. Let `c_00(Z^3;R^m)` be the real cochains with finite
support. Scalar homotopy `F -> tF` never enlarges support, so this linear space
is contractible. Its finite-energy `l2` completion is also contractible because

\[
 \lVert tF\rVert_2=t\lVert F\rVert_2.
\]

For a fixed source with a decaying infinite tail, every nonempty solution space
is again `F_0+ker A`, and it contracts to `F_0`. This is the appropriate result
for a finite but uncontained excitation: replacing a periodic quotient by open
space removes the three torus fluxes, but does not create a local integer
sector in an ordinary real linear field space.

## 4. Ternary discreteness is not automatically conservation `[THEOREM + BOUNDARY]`

For `N` sites the snapshot space is

\[
 \mathcal C=\{-1,0,+1\}^{N}\times\mathbb R^M,
\]

a disjoint union of `3^N` contractible continuous fibres. The ternary label is
indeed discrete. But a production event such as genesis, evaporation, pair
creation, or weak transmutation is an edge between those fibres. Snapshot
disconnectedness therefore does not imply a conserved quantity of the update
map.

The exact FTD-0421 transition matrix on the registered additive feature basis

\[
 (\text{occupancy},s,\chi,s\chi)
\]

has nine rows, rank four, and nullity zero. Hence no nonzero additive invariant
exists in that basis across every frozen production event. This is not a proof
that the full transition graph has no possible invariant; it is exactly scoped
to the registered four-feature additive class.

The distinction is now sharp:

- `s=+1` versus `s=-1` is a primitive polarity label;
- a conserved electric charge requires a production-history invariant and a
  reciprocal current/action law;
- a topological charge requires a nontrivial configuration-space sector;
- none follows from the alphabet alone.

## 5. The frozen vacuum manifold is a point `[THEOREM]`

FTD-0574 derives the positive source-free quadratic tick invariant. Its
nonzero-mode minimum is `J=W=0`; zero is an allowed value of every registered
real field. With `s=0`, the frozen free vacuum manifold is therefore

\[
 \mathcal M_{\rm vac}=\{0\}.
\]

All homotopy groups of a point vanish. In three spatial dimensions the usual
defect inventory is consequently unavailable:

| invariant | geometry in 3D | required vacuum data | frozen result |
|---|---|---|---|
| `pi_0(M)` | two-dimensional wall | disconnected vacua | zero |
| `pi_1(M)` | line/vortex | noncontractible loop, such as `S1` | zero |
| `pi_2(M)` | point/hedgehog | noncontractible `S2` at infinity | zero |
| `pi_3(M)` | texture | noncontractible three-cycle | zero |

A shell observer may compute a degree from `J/|J|` when the sampled shell is
nonzero. That degree is not protected in the full frozen configuration space:
`J` may cross zero at finite cost, where the normalized direction ceases to be
defined. This explains the FTD-0392/0398 result: a synthetic hedgehog can be
constructed, but the production genesis history neither freezes nor transports
a nonzero protected sector.

## 6. Why an ordinary static nonlinear potential is insufficient `[SCOPED NO-GO]`

Suppose the same real fields are given a static three-dimensional energy with
only a nonnegative onsite term `E_0` and a two-derivative term `E_2`. Under a
size rescaling `phi_R(x)=phi(x/R)`,

\[
 \boxed{E(R)=R E_2+R^3E_0.}
\]

For every nontrivial configuration,

\[
 \frac{dE}{dR}=E_2+3R^2E_0>0.
\]

Shrinking `R` lowers the energy toward zero. Such terms alone cannot stabilize
a finite-radius static soliton in three dimensions.

A four-derivative contribution changes the scaling to

\[
 E(R)=R E_2+\frac{E_4}{R}+R^3E_0,
\]

and can balance the size because the derivative is negative at sufficiently
small `R` and positive at sufficiently large `R`. This demonstrates a minimum
energetic ingredient; it does not derive that term from FTD.

Microscopic lattice pinning can also create a metastable minimum, but
FTD-0578–0581 prove the associated compact-carrier Peierls barrier. A pinned
excitation is not yet mobile matter. A time-periodic active nonlinear mode can
evade the static scaling argument and remains open, but FTD-0582 shows that the
current production source graph does not provide the required reciprocal
backreaction.

## 7. Minimum enlargements, classified but not derived `[CLASSIFICATION]`

The surviving routes are mathematically distinct:

1. **`S1`/compact link phase.** This supplies noncontractible phase loops and
   plaquette branch integers. Integer magnetic flux is protected only inside an
   admissible/smooth subspace that excludes a branch crossing; unrestricted
   compact lattice links can change the branch integer through a plaquette
   angle of `pi`. Compactness alone supplies neither conserved electric charge
   nor a stable particle.
2. **Fixed-magnitude `S2` order parameter.** This can classify a point hedgehog
   through `pi_2(S2)=Z`, provided the boundary condition and the energetic core
   are part of the dynamics. It is not the current unconstrained `J in R^3`.
3. **Texture plus size stabilizer.** A nontrivial `pi_3` target requires an
   additional order-parameter manifold and a higher-gradient, gauge, or other
   size-stabilizing action.
4. **Same-variable active core.** A localized time-periodic nonlinear solution
   may exist without topological protection. It must be derived from the frozen
   update or a separately declared selected dynamics and must independently
   pass energy, reciprocity, mobility, inverse, and Peierls gates.

The least ontological enlargement is not yet determined. The theorem determines
what each choice must pay for.

## 8. Verification

The native observer executes 192 exact affine-fibre fixtures and 960 homotopy
samples across `L={3,4,5,8}`, both signs, three axes, two independent curls,
four harmonic backgrounds, and five homotopy parameters. All measured Gauss,
harmonic-coordinate, affine, energy-polynomial, divergence, and support
residuals are exactly zero in binary64 for the registered rational fixtures.

The independent proof passes 38/38 checks, including frozen hashes, exact
rational affine algebra, uncontained support, the FTD-0421 transition rank,
defect codimensions, Derrick scaling, and a compact-plaquette branch crossing.
The run of record is `engine/results/ftd_0584/windows_msvc_cpu.json`.

## 9. Research consequence

The frozen `(s, ordinary real fields)` ontology does not presently contain a
localized protected particle sector. The next admissible derivation is not
another rigid coat. It is one of two mutually honest programs:

- derive an active, deforming, reciprocal localized solution from existing
  variables and accept that it is energetic/dynamical rather than topological;
- preregister one explicit compact or constrained order-parameter enlargement,
  including its admissibility condition and common action, then test whether it
  produces conservation, mobility, and finite energy rather than merely a
  formal winding number.

No particle, photon, electric charge, gauge group, Lorentz recovery, or
scenario promotion follows from this theorem.
