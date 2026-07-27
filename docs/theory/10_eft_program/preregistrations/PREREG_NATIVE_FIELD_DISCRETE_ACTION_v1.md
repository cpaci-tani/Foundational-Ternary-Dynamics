# Pre-registration — Native field discrete action and source audit (FTD-0574)

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE IMPLEMENTATION]`  
**Date:** 2026-07-26  
**Parent results:** FTD-0293, FTD-0295, FTD-0452, FTD-0467, FTD-0556,
FTD-0573.  
**Production changes permitted:** none. Observer code, tests, proofs, and
corrections to inaccurate comments/specification prose are permitted.

## 1. Question

FTD-0573 showed that cubic covariance uniquely selects the standard constant
onsite skew form on `(J,W)` if those fields are already treated as equivalent
vectors. This campaign asks the missing prior question:

1. Does the frozen source-free production wave tick itself follow from an
   exact local discrete action?
2. Is production `wave_vel` the discrete Legendre momentum of that action?
3. Does the action select the standard canonical form and the already measured
   modified tick energy without adding a canonical primitive?
4. For prescribed moving state, what interaction functional generates the
   coded source `-G_C grad(s)+G_C curl(s v)`?
5. Does that functional equal the six-term action currently documented by the
   engine?

This is not a search over actions. The production map and central-difference
operators are frozen before the derivation.

## 2. Frozen source-free map

On one periodic finite computational quotient let

```text
K = -C_WAVE^2 L,
W_(n+1) = W_n - K J_n,
J_(n+1) = J_n + W_(n+1),
```

where `L` is the symmetric production 18-point Laplacian. Test the candidate

```text
L_d(J_n,J_(n+1))
  = 1/2 ||J_(n+1)-J_n||^2 - 1/2 <J_n,K J_n>.
```

The discrete Euler--Lagrange equation must reproduce the production map
exactly. Its left and right Legendre transforms must give

```text
p_n^- = -D_1 L_d = W_n,
p_(n+1)^+ = D_2 L_d = W_(n+1).
```

Therefore the standard onsite form

```text
Omega_0 = [[0,I],[-I,0]]
```

is to be classified as derived for the source-free field sector, rather than
merely selected by FTD-0573's symmetry hypothesis.

## 3. Frozen mode classification

For every scalar eigenvalue `a` of `K`, use

```text
U_a = [[1-a,1],[-a,1]].
```

Require:

- `U_a^T Omega U_a=Omega` exactly;
- the real symmetric solution of `U_a^T G U_a=G` is one-dimensional and
  spanned by
  `G_a=[[a,-a/2],[-a/2,1]]`;
- `G_a` is positive definite exactly for `0<a<4` and degenerates at the zero
  mode;
- with onsite kinetic normalization `G_WW=I`, the finite-range global
  invariant is exactly
  `H_tick=1/2<W,W>+1/2<J,KJ>-1/2<W,KJ>`.

The registered production modes are the 36 combinations

```text
L in {16,32,64}, n in {0,1,2,3},
direction in {<100>,<110>,<111>}.
```

No dispersion fit or physical constant comparison is permitted.

## 4. Exact continuous shadow generator

For `0<a<4`, define

```text
theta = acos(1-a/2),
mu(a) = theta/sin(theta),
G_log(a) = mu(a) G_a.
```

Require

```text
U_a = exp(Omega_0 G_log(a)).
```

The exact continuous-time generator is mode dependent. A volume-independent
finite-range translation-invariant generator would have a finite Laurent-
polynomial symbol. Along `<100>`, `a=(2/3)(1-cos k)` for the production speed,
while `mu(a)` has a square-root branch at `a=4`, corresponding to finite
nonzero complex `z=e^{ik}`. The exact generator therefore cannot be a fixed
finite-range continuous-time convolution. This no-go is limited to a linear,
translation-invariant exact logarithm; the nearest-time-slice discrete action
and the exact tick invariant remain local.

## 5. Prescribed-source action and falsifier

Let `D` be the periodic central divergence, `G` the matching gradient, and `C`
the central curl. Verify the exact adjoints

```text
D^T = -G,
C^T = C.
```

For prescribed `s_n,v_n`, test

```text
I_src(J;s,v)
  = G_C <s,DJ> + G_C <C J,s v>,

grad_J I_src
  = -G_C Gs + G_C C(sv),
```

which is the source coded by `phase_read`. The resulting affine kick-drift map
must retain the same symplectic Jacobian.

The existing documented magnetic interaction is instead

```text
I_doc = -G_C <s v,J>,
grad_J I_doc = -G_C s v.
```

Register the exact uniform counterexample `s=+1`, `v=v_0` on a periodic box:

```text
grad(s)=0, curl(sv_0)=0,
coded source=0,
grad_J I_doc=-G_C v_0 != 0.
```

The defect is established if this residual is nonzero analytically and exceeds
`1e-12` numerically for each registered nonzero velocity. No alternative
force law or production source may be introduced in response.

## 6. Registered operator arms

- periodic sizes `L in {5,7}`;
- two deterministic trigonometric scalar/vector fixtures per size;
- four uniform velocities per size: the three axes and `(1,-2,3)/sqrt(14)`;
- both electric and curl adjoint identities;
- centered finite-difference directional derivatives of both interactions;
- proper cubic component permutations and sign inversions;
- affine-source symplectic Jacobian.

Every algebraic, adjoint, action-gradient, mode-flow, invariant, and covariance
residual must be `<=1e-12`. The uniform documented-action mismatch must be
strictly `>1e-6`. Independent SymPy and C++ implementations must agree.

## 7. Outcome map and scope

Pass verdict:

```text
NATIVE_FIELD_DISCRETE_ACTION_DERIVED_MAGNETIC_SOURCE_ACTION_MISMATCH
```

A pass establishes an exact canonical discrete action only for the isolated
source-free field and an affine symplectic action for externally prescribed
sources. It does not establish a common action for dynamically evolving
matter, genesis, evaporation, damping, Gauss projection, collisions, weak
transmutation, or any optional force branch. It does not derive a photon,
unitarity, Lorentz invariance, a bath, or a stable matter pole.

If the coded magnetic source is not generated by the currently documented
onsite velocity interaction, all claims that the full coded source follows
from that six-term action must be corrected. The production arithmetic remains
unchanged.

## 8. Locked source provenance

The `lagrangian` hashes below are the pre-correction evidence. They may change
only by comment/specification corrections after the mismatch is established;
the arithmetic return values and production source must not change.

```text
phase_read.cpp                         D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8
phase_write.cpp                        2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4
lagrangian.h                           0C0149DBE0D5FC0B9970539B0028A8E0B20E86181E7EB284E3D7DF07FCB80A59
lagrangian.cpp                         090D4B8E1C17C2CF443D674AC94ABEA13490831DD88223FB9B9562F7F5859911
field_operators.h                      25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48
native_energy_contract.h               3DB8F2DC573E7F4A87E17409878915E7B5A52CE1673713998C544516E0175621
DERIV_DISCRETE_TICK_ENERGY_INVARIANT   F74BE8F07C1034988FF01A698B2611BE37A3EF876417C97685E52FC8537B32AD
THEOREM_INTEGER_TRANSLATION_BLOCH      F472E65AFD9EB1B97B2EA4A8CC5C613960006928752F5A87F50302974DC2E6FD
THEOREM_GENESIS_CUBIC_CANONICAL_FORM   A548ED6CE992D67FD241E32B0E6100B23E1331386DAC9D6CB84821CD01020396
```
