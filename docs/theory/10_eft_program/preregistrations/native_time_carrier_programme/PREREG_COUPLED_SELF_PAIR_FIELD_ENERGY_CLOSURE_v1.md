# FTD-0842 — Coupled self-pair field-energy closure v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 26/26]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked test of combined spatial-gradient and onsite
self-pair energy, implicit-solve causality, and localized critical-quartic
clock compatibility  
**Production impact:** none

## 1. Registered question

FTD-0841 closes the onsite vector recursion conditional on the selected
radial self-pair energy. Production flux is not onsite: the 18-point gradient
energy couples neighboring voxels. Can the two positive energies be combined
in one exact, reversible update, and does that update produce a strictly
Moore-local, spatially bounded critical-quartic `G*` clock?

The registered finite-quotient Hamiltonian is

\[
H(Q,P)=\frac1{2m}\sum_i|P_i|^2
       +\frac12\langle Q,KQ\rangle
       +\lambda\sum_i|Q_i|^4,                  \tag{1}
\]

where `K=-C_WAVE^2 L_18` is the positive-semidefinite production spatial
operator, `m>0`, and `lambda>0` is the same selected onsite coupling as
FTD-0841.

## 2. Epistemic firewall

This is an exact algebraic/source discriminator. It performs no numerical
search, coefficient fit, period comparison, tolerance tuning, or target-coded
insertion. Equation (1), the symmetric discrete gradient below, and the
finite connected periodic quotient are frozen inputs.

Passing may establish an exact **conditional reference map**. It may not be
called the production tick, because the production free field uses the
source-free kick--drift map and its own quadratic tick invariant. It may not
be called a strictly local ontic update if its exact next state depends on
arbitrarily distant simultaneous unknowns. It may not be called a localized
critical-quartic clock if every nonconstant spatial profile carries positive
quadratic gradient stiffness.

No result here establishes Born frequencies, an actualization selector,
matter formation, a maintained body, or an exact finite-tick `G*` cadence.

## 3. Frozen source inputs

The certificate must fail closed unless these SHA-256 values match:

| Input | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/lagrangian.h` | `0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/eft/native_energy_contract.h` | `3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md` | `62A95FF322C99773D03002444376B9244A93CC19D01CF4400230277288CADAEB` |
| `docs/theory/10_eft_program/derivations/common_action_mechanics_reciprocity/THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |

## 4. Frozen mathematics

### 4.1 Combined discrete gradient

For a signed step `h != 0`, set

\[
\frac{Q_1-Q_0}{h}=\frac{P_1+P_0}{2m},          \tag{2}
\]

\[
\frac{P_1-P_0}{h}
=-\frac12K(Q_1+Q_0)-\lambda G(Q_0,Q_1),        \tag{3}
\]

with sitewise

\[
G_i(Q_0,Q_1)
=(|Q_{1i}|^2+|Q_{0i}|^2)(Q_{1i}+Q_{0i}).       \tag{4}
\]

The quadratic and quartic secant identities are

\[
(Q_1-Q_0)\mathbin{\cdot}\frac12K(Q_1+Q_0)
=\frac12\langle Q_1,KQ_1\rangle
-\frac12\langle Q_0,KQ_0\rangle,              \tag{5}
\]

\[
\sum_i(Q_{1i}-Q_{0i})\cdot G_i
=\sum_i(|Q_{1i}|^4-|Q_{0i}|^4).                \tag{6}
\]

Equations (2)--(6) conserve (1) exactly and are self-adjoint under endpoint
exchange with `h -> -h`.

### 4.2 Existence and uniqueness

Eliminating `P_1` gives

\[
F(X)=2m(X-Q_0)-2hP_0
 +\frac{h^2}{2}K(X+Q_0)+h^2\lambda G(Q_0,X)=0. \tag{7}
\]

The first term is strongly monotone, `K` is positive semidefinite, and the
sitewise self-pair secant has the nonnegative derivative decomposition proved
in FTD-0841. Hence `F` is strongly monotone and coercive, so it has exactly
one global solution.

### 4.3 Global internal angular momentum

Because `K` acts identically on the three flux components, (2)--(4) preserve

\[
L_{\rm int}=\sum_i Q_i\mathbin{\times}P_i.      \tag{8}
\]

The onsite torque vanishes sitewise. The edge torque cancels pairwise by the
symmetry of `K`. This is a global internal-vector invariant, not by itself a
body-relative clock orientation.

### 4.4 Critical-localization obstruction

For positive edge weights on a connected graph,

\[
\frac12\langle Q,KQ\rangle
=\frac{C_{\rm WAVE}^2}{2}
 \sum_{\{i,j\}}w_{ij}|Q_i-Q_j|^2.              \tag{9}
\]

It vanishes exactly when `Q` is spatially constant. On an uncontained
connected substrate, a constant field with finite support is zero. Therefore
every nonzero bounded profile has positive quadratic edge energy.

For a normalized profile `phi` and fixed polarization `e`, the ray
`Q_i=q phi_i e`, `P_i=p phi_i e` restricts (1) to

\[
H_{\phi}(q,p)=\frac{p^2}{2m}
+\frac{\kappa_\phi}{2}q^2
+\lambda\left(\sum_i\phi_i^4\right)q^4,
\quad
\kappa_\phi=\langle\phi,K\phi\rangle.          \tag{10}
\]

Exact critical quarticity requires `kappa_phi=0`, hence a spatially constant
profile. The positive gradient plus positive onsite self-pair cannot by
itself supply a nonzero bounded exact critical-quartic clock body.

### 4.5 Exact solve versus Moore causality

At `lambda=0`, (7) requires inversion of

\[
A=2mI+\frac{h^2}{2}K.                          \tag{11}
\]

For the regular connected production quotient, write
`K=C_WAVE^2(dI-A_w)` with nonnegative weighted adjacency `A_w` and row sum
`d`. Then

\[
A^{-1}=\frac1\alpha\sum_{r\ge0}
\left(\frac\beta\alpha A_w\right)^r,
\quad
\alpha=2m+\frac{h^2C_{\rm WAVE}^2d}{2},
\quad
\beta=\frac{h^2C_{\rm WAVE}^2}{2},             \tag{12}
\]

and `beta d/alpha < 1`. Every pair of sites is joined by a path, so an entry
of some power `A_w^r` is positive. Thus every entry of `A^{-1}` is positive:
the exact simultaneous next state has global algebraic dependence already in
the linear control.

The residual equation remains finite-range, but solving it exactly in one
ontic tick is not a one-Moore-shell dependency. A strict-local alternative
must use a different transaction architecture, additional local storage, or
an explicitly multi-tick local solver. This protocol does not preselect which.

### 4.6 Production control

At `lambda=0`, (2)--(3) are implicit midpoint. Production instead uses the
FTD-0574 kick--drift map and conserves its normalized tick invariant with a
`-<P,KQ>/2` cross term. Therefore the exact map above is a selected reference
replacement, not an additive production phase.

## 5. Frozen exact checks

The implementation must run exactly 26 checks:

1. all seven frozen source hashes;
2. production supplies local `(J,W)` and the positive 18-point edge energy;
3. production contains no radial onsite quartic force;
4. the combined Hamiltonian has kinetic, edge, and onsite terms;
5. the quadratic secant identity (5);
6. the quartic secant identity (6);
7. exact combined energy conservation;
8. endpoint/step reversibility;
9. correct continuous Hamiltonian limit;
10. strong monotonicity of the eliminated map;
11. coercivity and global uniqueness;
12. global internal angular-momentum conservation;
13. pairwise cancellation of edge torque;
14. the edge energy is a positive weighted sum of squares;
15. zero edge energy implies a constant field on a connected graph;
16. nonzero finite support cannot lie in the zero-stiffness kernel;
17. the profile restriction has the exact quadratic-plus-quartic form (10);
18. exact critical quarticity requires `kappa_phi=0`;
19. the only finite-quotient zero mode is spatially constant;
20. the linear implicit matrix is (11);
21. the Neumann ratio in (12) is strictly below one;
22. connected paths make the inverse algebraically dense;
23. the residual stencil is finite-range while its exact solution is global;
24. the `lambda=0` control differs from the production kick--drift map;
25. `K=0` reduces exactly to the FTD-0841 onsite recursion; and
26. combined discriminator: exact conditional energy closure passes, while
    strict one-tick locality and bounded exact critical quarticity fail for
    this positive edge-plus-onsite architecture.

No check may be removed, reinterpreted, or tolerance-relaxed after execution.

## 6. Locked implementation

```text
scripts/proofs/proof_coupled_self_pair_field_energy_closure.py
```

Script SHA-256:
`A963EDBA1B9F698EB66C3E6AD1A3A296DE0E03DA18E824BD3D6C156542C7EB8A`

Pre-run protocol SHA-256:
`16B96F59DB44F77B30A71417D740355C35D70BEA48E76B9108AE0B08062E91E4`

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before the first execution. Run exactly:

```text
python scripts/proofs/proof_coupled_self_pair_field_energy_closure.py
```

## 7. Outcomes

- **Outcome A — production-native local critical clock:** all exact checks
  pass and the frozen production update already supplies the onsite coupling,
  one-shell exact energy closure, and a bounded zero-stiffness profile.
- **Outcome B — exact global closure, local critical clock obstructed:** all
  26 checks pass. The selected simultaneous discrete gradient is unique,
  reversible, and exactly energy closed, but it is globally coupled and every
  nonconstant/bounded profile has positive quadratic edge stiffness.
- **Outcome C — invalid:** any exact or source-hash check fails without
  establishing Outcome A. Book no theorem and repair under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run.

## 8. Recorded outcome

The first locked execution returned `26/26 PASS`. Registered Outcome B is
selected:

```text
COMBINED_DISCRETE_GRADIENT_UNIQUE_REVERSIBLE_AND_ENERGY_CLOSED
EXACT_SIMULTANEOUS_SOLVE_HAS_GLOBAL_ALGEBRAIC_DEPENDENCE
POSITIVE_EDGE_ENERGY_EXCLUDES_NONZERO_BOUNDED_ZERO_STIFFNESS_MODE
LOCAL_CRITICAL_GSTAR_CLOCK_REQUIRES_ADDITIONAL_DYNAMICAL_STRUCTURE
```

The exact conditional global map exists, but it is not a one-Moore-shell
ontic update and the registered positive edge-plus-onsite energy has no
nonzero bounded zero-stiffness profile. See
[`THEOREM_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md).
