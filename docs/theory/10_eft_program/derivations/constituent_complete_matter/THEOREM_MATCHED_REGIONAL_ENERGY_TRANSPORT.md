# THEOREM — Exact matched regional energy transport

**Identifier:** `FTD-0671`  
**Status:** `[THEOREM — EXACT AUXILIARY REGIONAL ENERGY LEDGER]`  
**Scope:** the selected matched face/edge leapfrog field complex; observer only

## Statement

Let `C` be the periodic edge-to-face curl and `C^T` its exact transpose. The
connected matched-field step is

```text
B1 = B0 - lambda C^T E0,
E* = E0 + lambda C B1,
E1 = E* - K.                                      (1)
```

Its source-free modified energy is

```text
H(E,B) = (||E||^2+||B||^2)/2
         - (lambda/2)<B,C^T E>.                   (2)
```

For any diagonal face and edge projectors `P_E,P_B`, define the symmetrically
allocated regional energy

```text
U_R(E,B) = <P_E E,E>/2 + <P_B B,B>/2
 - (lambda/4)[<P_B B,C^T E>+<B,C^T P_E E>].       (3)
```

If `Pbar=I-P`, then

```text
U_R(E,B)+U_Rbar(E,B)=H(E,B).                       (4)
```

The exact one-step regional ledger is

```text
U_R(E1,B1)-U_R(E0,B0) = T_R + S_R,                (5)
T_R = U_R(E*,B1)-U_R(E0,B0),
S_R = U_R(E1,B1)-U_R(E*,B1).
```

Here `T_R` is signed source-free field transport into the selected region and
`S_R` is the field-energy exchange caused by the deposited current. Moreover,

```text
T_R + T_Rbar = 0.                                  (6)
```

Thus `T_R` is an exact transfer between the region and its complement, not an
inference from field amplitude or radial spread.

## Proof

Equation (4) follows by substituting `P_E+(I-P_E)=I` and
`P_B+(I-P_B)=I` into (3). The two half-weight cross terms then sum to the
single cross term in (2).

For the magnetic substep in (1), polarization gives

```text
(||B1||^2-||B0||^2)/2
= -(lambda/2)<C^T E0,B1+B0>.                      (7)
```

For the electric source-free substep,

```text
(||E*||^2-||E0||^2)/2
= (lambda/2)<E*+E0,C B1>.                         (8)
```

Adding (7), (8), and the change of the cross term in (2), then using
`<E,C B>=<C^T E,B>`, cancels every term exactly. Therefore

```text
H(E*,B1)=H(E0,B0).                                 (9)
```

Apply (4) before and after (9) to obtain (6). Equation (5) is the exact
telescoping split through the intermediate state `(E*,B1)`. Finally,
substituting `E1=E*-K` makes the current contribution explicit:

```text
S_R = -<P_E E*,P_E K> + ||P_E K||^2/2
      + (lambda/4)[<P_B B1,C^T K>
                    + <B1,C^T P_E K>].            (10)
```

No fitted transport velocity, continuum Poynting vector, or post-hoc source
term enters the identity.

## Covariance and locality

For component-aware periodic Chebyshev projectors, integer translations and
proper cubic rotations merely permute the face and edge coordinates. Equations
(1)–(10) are invariant under those permutations. The current exchange in
(10) is supported on the current coat and the one-curl neighborhood touching
its regional boundary.

## Boundary of the theorem

The split is exact for the chosen symmetric allocation of the leapfrog cross
term. A lattice energy density is not unique: adding a discrete divergence can
move energy between adjacent cells while leaving (2) unchanged. Therefore:

- `T_R>0` means exact net field transfer into this region under (3);
- it does not by itself identify photons, radiation, a wake, or a bound coat;
- classification requires nested radii and time-directed transport histories;
- this theorem does not establish a matter pole, separability, or production
  dynamics.

The theorem applies to the selected matched field sidecar. It does not promote
that sidecar to the five-postulate ontology or alter the production tick.
