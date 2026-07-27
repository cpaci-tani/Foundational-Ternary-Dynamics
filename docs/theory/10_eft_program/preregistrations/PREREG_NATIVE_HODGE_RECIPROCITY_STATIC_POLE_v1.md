# PRE-REGISTRATION — Native Hodge reciprocity and static-pole gate

**Identifier:** `FTD-0575`  
**Date locked:** 2026-07-26  
**Scope:** observer-only consequence of the frozen FTD-0574 source interaction  
**Production changes authorized:** none

## 1. Question

Does the exact prescribed-source functional derived in FTD-0574 supply a
reciprocal matter force without an independent connection, and can that force
carry a Coulomb-like static massless pole?

The frozen source interaction is

```text
I_src(J;rho,j)=G_C <rho,DJ> + G_C <CJ,j>,
```

where `D`, `G`, and `C` are the periodic central divergence, gradient, and
curl, and the source-free stiffness is

```text
K=-C_WAVE^2 L_FULL.
```

No Poisson solve, Gauss projection, legacy force, fitted normalization,
subcell-force amplification, or new field variable is admitted.

## 2. Predeclared derivation

Define the Hodge-derived potentials

```text
Phi_J=-G_C D J,
A_J=+G_C C J.
```

Then a point-path interaction has the minimal-coupling form

```text
L_int=q(v dot A_J-Phi_J).
```

For a differentiable interpolation the registered path variation is

```text
F_J=q(E_J+v cross B_J),
E_J=G_C G D J-G_C partial_t C J,
B_J=G_C C^2 J.
```

The magnetic scalar-work identity is `v dot (v cross B_J)=0`. The homogeneous
identities to be tested are `D B_J=0` and
`partial_t B_J+C E_J=0`, using the same commuting central operators.

For static sources, the field equation is

```text
K J=-G_C G rho+G_C C j.
```

For a nonzero Fourier mode let

```text
s_i=sin(k_i),
sigma2=sum_i s_i^2,
M=4-(2/3)sum_i cos(k_i)
    -(2/3)sum_(i<j) cos(k_i)cos(k_j),
R=sigma2/(C_WAVE^2 M).
```

The registered static exchange claims are

```text
Phi_J=-G_C^2 R rho,
A_J=+G_C^2 R P_T j,
V_eff_charge=-(G_C^2/2)<rho,R rho>.
```

Thus a positive charge mode has a negative static potential and the
same-polarity cross term is attractive. `R` must remain bounded at the origin;
there must be no `1/M` Coulomb pole.

## 3. Exact algebraic gates

Set `u_i=1-cos(k_i)`, `U=sum u_i`, `P=sum_(i<j)u_i u_j`, and
`Q=sum u_i^2`. The independent proof must establish

```text
M=2U-(2/3)P,
sigma2=2U-Q,
M-sigma2=Q-(2/3)P >= 0,
0 <= R <= 1/C_WAVE^2 = 3,
lim_(k->0) R=3.
```

The inequality must be proved from the cube domain `0<=u_i<=2`, not inferred
from a momentum scan. The zero mode is excluded because both source vertices
vanish there. At nonzero Brillouin corners `sigma2=0`, the response must vanish
rather than diverge.

## 4. Registered observer arms

The native observer and independent proof will execute:

- `27` infrared symbol arms: `L=16,32,64`, `n=1,2,3`, and
  `<100>/<110>/<111>`;
- `24` exact proper-cubic rotations on one generic nonzero mode;
- `12` static charge-response arms: `L=16,32`, both polarities, and the three
  principal directions;
- `12` static transverse-current arms with the same volume/direction grid and
  two transverse orientations;
- `4` Brillouin-corner controls;
- `4` periodic operator-identity fixtures on `L=5,7`;
- `8` smooth point-path variation fixtures covering both charges, affine time
  dependence, nonzero electric induction, and nonzero magnetic curvature.

## 5. Acceptance gates

The constructive Hodge-force result passes only if:

1. the interaction equals `q(v dot A_J-Phi_J)` below `1e-12`;
2. direct path variation matches `q(E_J+v cross B_J)` below `1e-10`;
3. magnetic scalar work is below `1e-12`;
4. `D C^2 J=0` and the discrete Faraday identity close below `1e-12`;
5. every static field solve and projected-potential formula closes below
   `1e-12`;
6. every nonzero mode satisfies `0<=R<=3+1e-12`;
7. the infrared arms approach `R=3` with error decreasing under volume
   refinement;
8. charge and current exchange kernels use the same `R`;
9. the charge effective energy is negative semidefinite and the registered
   same-polarity cross terms are strictly negative;
10. proper-cubic covariance closes below `1e-12`;
11. all zero/corner/invalid controls return their predeclared classification;
12. production hashes and defaults remain unchanged.

## 6. Locked interpretation

- If all gates pass, record a positive theorem that the FTD-0574 source action
  has a reciprocal Hodge-derived Lorentz-form matter variation.
- The same result is a scoped no-go for identifying that reciprocal force with
  static Coulomb electromagnetism if `R` is bounded and the same-sign energy is
  negative.
- A surviving dynamic radiation pole is not a Coulomb rescue. Its numerator is
  `O(sigma2)`, so the soft residue vanishes quadratically even though finite
  nonzero on-shell modes may radiate.
- The theorem does not close exact finite-step total energy, a mobile
  manifested solution, a stable dressing, a photon identity, or infrared
  Lorentz recovery.
- Failure does not authorize changing the source sign, adding a Poisson branch,
  retuning `G_C`, or introducing an independent connection after the run.

## 7. Required artifacts

- `engine/include/ftd/eft/native_hodge_reciprocity.h`
- `engine/src/eft/native_hodge_reciprocity.cpp`
- `engine/tests/test_native_hodge_reciprocity.cpp`
- `scripts/proofs/proof_native_hodge_reciprocity_static_pole.py`
- `engine/results/ftd_0575/windows_msvc_cpu.json`
- theorem and audit documents registered in every canonical navigation layer

