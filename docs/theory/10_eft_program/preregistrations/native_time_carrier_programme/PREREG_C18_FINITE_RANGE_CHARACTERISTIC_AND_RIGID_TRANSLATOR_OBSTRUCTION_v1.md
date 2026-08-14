# FTD-0943 — Preregistration: C18 finite-range characteristic and rigid-translator obstruction v1

**Identifier:** `FTD-0943`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** exact Laurent-polynomial audit of the isolated, undamped production
`C18` relative canonical field; scalar finite-range stiffness factorization;
exact discrete characteristic discriminant; arbitrary-tick finite-support
rigid translation and recurrence; extension to the three vector components;
no numerical search, fitting, engine change, nonlinear-production no-go,
new type adoption, `G*`, gamma, Born, Bell, measurement context, or outcome
read

## 1. Question

FTD-0858 gave an exact local bond input/output energy chart but proved that
the axial reduction of the production wave map is not the exact one-site
shift. FTD-0919 then proved that the free `C18` stiffness has no nonzero
finite-support eigenfield or compact finite-dimensional invariant modal
carrier. FTD-0942 established that the existing L/R fields are an invertible
aggregate canonical carrier but not the registered occupancy-token carrier.

The remaining linear-field question is broader than each prior result:

> Can the full three-dimensional isolated `C18` relative canonical pair be
> diagonalized into exact **finite-range local characteristics**, or can any
> nonzero finite-support complete state translate rigidly or recur after an
> arbitrary positive number of production ticks?

This audit concerns the phase-complete state `(D,P_D)`, not only stiffness
eigenfields or one-tick axial pulses.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/ontic/gauge_couplings.h` | `BC862D8120E0F3D83B7FAD0201F8D4DF46B5BAD5E7D52CD571AF68BECA3EB0F3` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `THEOREM_NATIVE_EVENT_ACTIVATION_CHARACTERISTIC_BOUNDARY_v1.md` | `06ED4EFEF16CF815A44E26F04213FC67F5388E917E9ED9D7B41F9FD8BA736B53` |
| `proof_native_event_activation_characteristic_boundary_v2.py` | `E2A6D22946E0E3BD9A5CE208EB7C440567AA72B97C28F507C099F06E93740204` |
| `THEOREM_NATIVE_C4_MODAL_CIRCULATION_AND_COMPACT_SUPPORT_OBSTRUCTION_v1.md` | `CA05D786A73775B398F90EE33E207E2A4D3522D49ECA86B9BF5774E2D6B1A285` |
| `proof_native_c4_modal_circulation_compact_support_obstruction.py` | `C1C312E1B5FA9F9EB90DFD1A2B71B38736BC7F8AEE93DFDBA56B88A5133031EA` |
| `THEOREM_EXISTING_LR_AGGREGATE_CARRIER_AND_OCCUPANCY_HISTORY_REALIZATION_BOUNDARY_v1.md` | `D287ED5B5E6FCD15352E191D272A9B1A83D2952A009C1A9BEA5E0CAA985A0697` |
| `proof_existing_lr_occupancy_history_carrier_classifier.py` | `54AFAA09E6588A04B702A0F7368874ECA25AC21810E8532E8F04FB550E8C4808` |

The certificate must fail closed on source drift.

## 3. Registered Laurent symbol

Work in the integral domain

\[
 R=\mathbb C[z_x^{\pm1},z_y^{\pm1},z_z^{\pm1}].
\]

For one scalar component of the relative field, the production face/edge
stencil and `C_WAVE^2=1/3` give the positive stiffness

\[
 K(z)=\frac13\left[4-rac13\sum_{e\in F_6}z^e
                       -\frac16\sum_{e\in E_{12}}z^e\right].       \tag{1}
\]

On the unit torus, writing `c_j=cos(k_j)`,

\[
 K(k)=\frac43-\frac29(c_x+c_y+c_z+c_xc_y+c_yc_z+c_zc_x),          \tag{2}
\]

and at the vacuum mode

\[
 K(k)=\frac13(k_x^2+k_y^2+k_z^2)+O(|k|^4).                        \tag{3}
\]

The quadratic term therefore has rank three.

## 4. Finite-range characteristic gates

### 4.1 Scalar square-root gate

A scalar finite-range energy characteristic would require a Laurent
polynomial `b(z)` with `b(z)^2=K(z)`. Because `K(1,1,1)=0`, also `b(1,1,1)=0`.
If the first nonzero Taylor term of `b` is linear, the quadratic term of
`b^2` is the square of one linear form and has rank at most one. If `b`
vanishes to order at least two, `b^2` begins at order at least four. Either
case contradicts (3). The registered result is therefore that `K` is not a
square in `R`.

This gate excludes a scalar finite-range square-root factorization. It does
not exclude a separately selected multicomponent Dirac-like extension; such
an extension would add representation structure not present in the scalar
relative-field component.

### 4.2 Exact discrete characteristic gate

The kick--drift map is

\[
 U(z)=\begin{pmatrix}1-K(z)&1\\-K(z)&1\end{pmatrix},
 \qquad \det U=1,
 \qquad \operatorname{tr}U=2-K.                                  \tag{4}
\]

Its characteristic discriminant is

\[
 \Delta(z)=(2-K)^2-4=K(K-4).                                      \tag{5}
\]

Near the vacuum mode,

\[
 \Delta(k)=-\frac43(k_x^2+k_y^2+k_z^2)+O(|k|^4),                  \tag{6}
\]

again a rank-three quadratic. The same Taylor-rank argument must prove that
`Delta` is not a square in `R`. Hence the exact eigenvalues and spectral
projectors require a non-Laurent square root and are not finite-range local
characteristics of the existing two-component state.

## 5. Arbitrary-tick rigid-translation gate

A finite-support complete state is a Laurent vector `X(z) in R^2`. Register
the rigid-translation equation

\[
 U(z)^mX(z)=z^dX(z),\qquad m\ge1,\quad d\in\mathbb Z^3.             \tag{7}
\]

Because `R` is an integral domain, a nonzero solution requires

\[
 \det(U^m-z^dI)=0                                                  \tag{8}
\]

as a Laurent-polynomial identity. Since `det U=1`,

\[
 \operatorname{tr}(U^m)=2T_m(1-K/2),                              \tag{9}
\]

and (8) is equivalent to

\[
 z^d+z^{-d}=2T_m(1-K/2).                                          \tag{10}
\]

The right side is invariant under every signed permutation of the cubic
axes. For every nonzero `d`, the orbit of `d` under that group contains a
vector other than `d` or `-d`: if exactly one coordinate is nonzero, permute
it to another axis; if at least two coordinates are nonzero, flip only one
nonzero coordinate. Thus the left side of (10) is not fully cubic invariant.
Equation (10) cannot hold, so (7) has only `X=0` for every `m>=1` and every
nonzero displacement `d`.

For recurrence (`d=0`), the determinant is

\[
 2-2T_m(1-K/2)=m^2K+O(K^2),                                      \tag{11}
\]

using `T_m'(1)=m^2`. It is not the zero Laurent polynomial. Therefore no
nonzero finite-support complete state is exactly periodic under the isolated
free map for any positive tick count.

The certificate must verify the algebra exactly, including representative
finite-support matrix identities and the signed-permutation lemma, but the
theorem is the general integral-domain argument above rather than a bounded
enumeration.

## 6. Vector and scope boundary

The production relative flux has three spatial components and the isolated
stencil acts componentwise. A vector finite-support state would contain at
least one nonzero scalar component satisfying (7), so the scalar obstruction
extends directly to the full relative vector pair.

This result must **not** be reported as excluding:

- infinite-support Bloch or normal modes;
- nonlocal Fourier characteristic projectors;
- approximate, dispersive, or exponentially tailed packets;
- externally driven or maintained localized structures;
- genesis, weak, movement, boundary, or other event-mediated nonlinear and
  piecewise production actions;
- a preregistered nonlinear self-trapping term in the existing fields; or
- separately selected oriented ports or an enlarged multicomponent field.

In particular, this audit cannot prove that a new primitive type is
unavoidable. It closes the exact **isolated linear finite-range** route only.

## 7. Frozen outcomes

| Outcome | Exact condition | Verdict |
|---|---|---|
| A | `K` or `Delta` has the registered finite-range square root, or a nonzero finite-support complete state satisfies exact translation/recurrence for some registered general branch | exact local characteristic or rigid free carrier survives; obstruction refuted |
| B | neither Laurent square root exists and the determinant/invariance argument excludes every nonzero finite-support translator and periodic complete state | isolated linear `C18` has global/nonlocal modes but no exact local characteristic rail or rigid protected finite-support pulse |
| C | one no-go passes and another cannot be proved from the frozen map | mixed boundary; report only the proved branch |
| D | source drift or exact gate failure | execution invalid; no theorem |

No tolerance, fit, numerical near-miss, bounded search promoted to a general
claim, or post-hoc outcome change is permitted.

## 8. Acceptance and stop conditions

The certificate must report separately:

1. source hashes and exact production markers;
2. face/edge Laurent symbol and torus reduction;
3. rank-three vacuum quadratic for `K`;
4. scalar Laurent-square obstruction;
5. exact kick--drift determinant, trace, and discriminant;
6. Laurent-square obstruction for `Delta`;
7. Cayley--Hamilton/Chebyshev trace identity;
8. cubic-invariance obstruction for every nonzero displacement class;
9. nonzero recurrence determinant for every `m>=1`;
10. integral-domain kernel conclusion; and
11. vector-component extension and all scope firewalls.

Stop immediately on source drift. Do not modify production engine sources,
tests, CMake, toggles, physical parameters, or ontology.

## 9. Promotion boundary

Outcome B would settle that the current isolated linear relative field cannot
be the exact finite-range one-way gearbox sought after FTD-0942. The next
legitimate question would be whether already-existing **event-mediated
production actions** generate a reversible nonlinear relative-field carrier
without target-coded history or hidden journals. Only after that audit fails
would a new nonlinear action or explicit direction-port implementation become
the live design fork.
