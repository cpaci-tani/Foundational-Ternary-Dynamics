# Pre-registration: cubic reaction vector and relativistic source transport v1

**ID:** FTD-0889  
**Status:** `[PRE-REGISTERED / LOCKED BEFORE EXECUTION]`  
**Date:** 2026-08-11  
**Production status:** no coupling authorized

## 1. Question

FTD-0888 produced a positive scalar source-reaction pair and an exact
history/reaction energy split, but explicitly did not produce spatial ternary
source recoil. This protocol asks the next narrow question:

> Can the scalar reaction output determine a cubic spatial direction, and if
> not, what is the minimum cubic-covariant canonical carrier? Given such a
> carrier or an independently available oriented field impulse, is there an
> exact symplectic energy chart from reaction energy to the already selected
> production relativistic momentum and an exact reversible transport/current
> continuation?

The protocol is a fixed algebraic certificate. It is not a numerical search,
parameter fit, production campaign, or mass derivation.

## 2. Frozen sources

| source | SHA256 before execution |
|---|---|
| `THEOREM_AUTONOMOUS_PHASE_PARITY_AND_SOURCE_REACTION_BOUNDARY_v1.md` | `0FEEF83C38BE9A4929644A229EAEA1B22424A54161BE8E2F3F8B882194DFDF39` |
| `engine/include/ftd/eft/autonomous_phase_parity_source_reaction.h` | `D052A463C3F62F3326BBBB1CECB56E9CF4EB5A92EDE411A4DD42F28602AA6FB9` |
| `engine/include/ftd/ontic/particle_masses.h` | `EFE9D68C9ECF6520510519B972D5CDD5925FD86026270AB0E4CAA5BFD6F1B0B1` |
| `engine/include/ftd/eft/production_hop_kinematics.h` | `4FCE830B79CD4590108B7FEA28063B489B33CF3CA69925E5405043B78D1C2EBD` |
| `engine/include/ftd/eft/matched_face_momentum_transaction.h` | `BA7B0CA7895D4DC5259527CCDCB06EC9B08DF7C4CB38AC8CDEDC31EFCD3FA62B` |
| `engine/include/ftd/eft/face_current_segment.h` | `BA86AA25BD52B80A7D11DF72012F20109DD89830C5DD80F44A6729548E30ECB9` |
| `engine/include/ftd/eft/canonical_subcell_section.h` | `8DBA6784C6B0D61B5A78430EB6A5949F215AFCD1C635B67BAF05F2B94595B42F` |
| `FOUND_MATTER_EVENT_CURRENT_ONTOLOGY_v1.md` | `4C5DF2533F63628B68E612A8197010C2B0D85FC6BF6E7C9F6D55C71FF31DFF67` |

Any mismatch invalidates execution before a mathematical verdict.

## 3. Frozen representation question

Let `O_h` act on a spatial vector by signed permutation matrices. The
FTD-0888 local residual and reaction output are scalars under this action.
For any equivariant map `F : R -> R^3`, equivariance requires

```text
F(z) = R F(z)  for every R in O_h.
```

The common fixed subspace of the vector representation is `{0}`. Therefore
`F=0`, including nonlinear maps. A scalar reaction magnitude cannot choose a
spatial recoil direction.

One vector copy `T1u` is three-dimensional and carries no nondegenerate
invariant alternating form. In the registered onsite-direct-sum class, the
minimum orientation-free spatial canonical carrier is

```text
(R, Pi) in R^3 + R^3,
omega = sum_i dR_i wedge dPi_i,
```

namely three canonical pairs transforming as `T1u + T1u`. One scalar pair is
sufficient only on a one-dimensional submanifold whose unit direction is
supplied independently by a field/current context and held fixed during the
gate.

## 4. Frozen relativistic cotangent chart

Let the ready vector reaction carrier have energy

```text
E_react = |Pi|^2 / 2
```

and let the selected physical source dispersion be

```text
K(p) = sqrt(E0^2 + c^2 |p|^2) - E0,  E0>0, c>0.
```

For `rho=|Pi|`, define

```text
a(rho) = sqrt(E0 + rho^2/4) / c,
p = g(Pi) = a(rho) Pi,
A(Pi) = Dg(Pi),
x = A(Pi)^(-T) R.
```

The Jacobian eigenvalues are frozen as

```text
lambda_t = sqrt(E0 + rho^2/4)/c                    (multiplicity 2),
lambda_r = (E0 + rho^2/2)/(c sqrt(E0 + rho^2/4))  (multiplicity 1).
```

They are positive. The one-form identity

```text
x dot dp = R dot dPi
```

makes the map symplectic, and the exact energy identity is

```text
K(g(Pi)) = |Pi|^2/2.
```

At small momentum,

```text
p = sqrt(E0)/c Pi + O(|Pi|^3),
K = |p|^2/(2m) + O(|p|^4),
m = E0/c^2.
```

This recovers the existing production relation
`E_REST=M_INERTIAL*C_SPEED^2` conditionally. It does not determine `E0`, `c`,
or `M_INERTIAL`; canonical rescaling preserves the symplectic form while
changing the numerical mass normalization.

## 5. Frozen transport and conservation continuation

The free physical source Hamiltonian is

```text
E(p) = sqrt(E0^2 + c^2 |p|^2),
v(p) = c^2 p/E(p),
q' = q + dt v(p),
p' = p.
```

This is an exact Hamiltonian drift, preserves energy, has inverse `dt -> -dt`,
and obeys `|v|<c` for finite momentum. Re-expression of `q` as
`(integer site, subcell remainder)` is a quotient chart; the known centered
half-cell section obstruction remains and is not erased. The existing
`FaceCurrentSegment` supplies exact endpoint continuity for an admissible
one-tick segment.

The existing matched-field momentum observer defines the required matter
impulse

```text
Delta p_matter = -Delta P_field.
```

This vector can supply orientation without an arbitrary axis. For an initially
stationary source, let `K_req=K(Delta p_matter)` and let the FTD-0888 residual
energy be `E_res=u^2/2`. Exact compatibility requires

```text
0 <= K_req <= E_res,
sin^2(eta) = K_req/E_res = 2 K_req/u^2.
```

For `u != 0` this fixes the unique `eta in [0,pi/2]`. Equal history/reaction
splitting occurs only when `K_req=E_res/2`; `eta=pi/4` is not a universal
recoil law. If `K_req>E_res`, the event is energetically inadmissible at this
gate and must not be forced by clipping or target-coded weights.

## 6. Locked certificate gates

1. all eight source hashes match;
2. the scalar fixed-subspace argument closes for three independent cubic
   sign flips/half-turns;
3. the no-go applies to nonlinear scalar-only equivariant maps;
4. a three-dimensional alternating form is singular;
5. one `T1u` copy has no invariant nondegenerate symplectic form;
6. `T1u + T1u` with `sum dR_i wedge dPi_i` is invariant and nondegenerate;
7. three canonical pairs are minimum in the registered orientation-free
   onsite class;
8. a fixed independently supplied direction reduces the carrier to one pair
   only conditionally;
9. the frozen `a(rho)`, Jacobian, and both eigenvalues are recovered;
10. the Jacobian is positive and invertible at and away from `rho=0`;
11. the one-form and symplectic identities hold;
12. the relativistic energy identity holds exactly;
13. the inverse radial momentum chart closes;
14. the low-energy mass is `E0/c^2`;
15. the canonical-rescaling mass degeneracy is explicit;
16. exact free drift, inverse, energy preservation, and speed ceiling close;
17. the physical-position quotient and half-cell section boundary are stated;
18. exact face-current continuity is inherited, not re-derived from a ternary
   snapshot;
19. matched field momentum supplies the action-reaction orientation candidate;
20. zero field impulse produces no arbitrary direction;
21. the energy-compatibility inequality and unique angle close;
22. equal split is conditional, not universal;
23. insufficient residual energy fails closed;
24. production, stable matter, mass-scale derivation, `G*`, Born/Bell,
   Lorentz recovery, biology, and completeness firewalls all hold.

The independent verifier may subdivide these into at most 72 named checks but
may not change an equation, input hash, outcome rule, or scope ceiling.

## 7. Outcome map

- **Outcome A — exact conditional source-transport gearbox:** all gates pass.
  Book the scalar-to-vector obstruction, minimum cubic canonical triplet, exact
  relativistic cotangent chart, conservation-fixed split angle, and reversible
  transport continuation. Keep mass scale, native vector formation, full
  common-action coupling, stable matter, and production open.
- **Outcome B — representation boundary only:** the cubic no-go/minimum pass
  but the energy chart, transport, or compatibility algebra fails. Book only
  the passed representation theorem.
- **Execution invalid:** any source hash, frozen equation, or certificate
  contract mismatch occurs. Book no new mathematical verdict.

## 8. Banned moves

- no numerical near-miss or coincidence search;
- no fitted mass, speed, split angle, or direction;
- no use of a scalar magnitude as a spatial direction;
- no claim that `M_INERTIAL`, the production dispersion, or a stable particle
  has been derived;
- no production wiring;
- no reinterpretation of a fixed-context symplectic slice as a global
  autonomous common-action theorem;
- no change to Born/Bell, `G*`, Lorentz, or completeness status.
