# FTD-0841 — Local flux self-pair tensor recursion v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; EXACT CERTIFICATE 26/26]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked local-vector self-pair, symmetry, recursion,
polarization, and production-boundary certificate  
**Production impact:** none

## 1. Registered question

Can FTD-0840's scalar signed-pair mechanism be localized without selecting a
spatial axis, using only the flux and conjugate wave register already stored
at one voxel?

For a local vector coordinate `J in R^3`, define its rank-one self-pair tensor

\[
U=J\otimes J.
\]

Does the induced Frobenius energy

\[
V(J)=\lambda\|U\|_F^2
\]

give a natural local quartic, a globally deterministic exact vector
discrete-gradient recursion, and the scalar `G*` clock on a linearly polarized
invariant sector? Which parts follow from the production type, and which
remain selected?

## 2. Epistemic firewall

This is an exact algebraic/source discriminator. It performs no numerical
search, coefficient fitting, target-period insertion, near-miss comparison,
or post-execution choice. The self-pair tensor, its induced Euclidean norm,
the positive coefficient `lambda`, and the discrete-gradient update are
registered mathematical inputs.

The certificate must distinguish:

- **native local type:** `Voxel::flux` and `Voxel::wave_vel` exist, and the
  registered Lagrangian calls `wave_vel` the conjugate momentum of flux;
- **exact construction:** `||J tensor J||_F^2=|J|^4`;
- **selected coupling:** production does not contain
  `lambda |J|^4` or its cubic force;
- **symmetry boundary:** full `O(3)` invariance selects the radial quartic,
  but finite cubic symmetry alone permits two independent even quartics;
- **clock-sector boundary:** a linearly polarized zero-angular-momentum sector
  reduces exactly to FTD-0840, while generic vector motion does not have the
  pure scalar clock period; and
- **cadence boundary:** the continuum `G*` factor does not become a finite
  global-tick cadence merely because the local recursion exists.

Passing cannot establish a localized persistent clock body, production
interaction, autonomous formation law, Born rule, or actualization selector.

## 3. Frozen source inputs

The certificate must fail closed unless these SHA-256 values match:

| Input | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/lagrangian.h` | `0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/src/energy_ledger_compute.cpp` | `2E5138BA43F74624C47842E9C3B0372ADFA9288BFE175BFE75ED901F237DD61B` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |
| `engine/include/ftd/eft/native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |

## 4. Frozen mathematics

### 4.1 Local self-pair

Let `J,W in R^3`, `m>0`, and `lambda>0`. Define

\[
U_{ab}=J_aJ_b.
\]

Then

\[
\|U\|_F^2
=\sum_{a,b}J_a^2J_b^2
=\left(\sum_aJ_a^2\right)^2
=|J|^4.                                         \tag{1}
\]

The selected local Hamiltonian is

\[
H(J,W)=\frac{|W|^2}{2m}+\lambda|J|^4
=\frac{|W|^2}{2m}+\lambda\|U\|_F^2.            \tag{2}
\]

Its equations are

\[
\dot J=\frac Wm,
\qquad
\dot W=-4\lambda|J|^2J.                        \tag{3}
\]

The angular momentum `L=J cross W` is conserved.

### 4.2 Polarized scalar sector

For any fixed unit vector `e`, the subspace

\[
J=qe,qquad W=pe                              \tag{4}
\]

is invariant under (3). On it,

\[
H=\frac{p^2}{2m}+\lambda q^4,                  \tag{5}
\]

so FTD-0840 and its continuum law

\[
TA=\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}       \tag{6}
\]

follow exactly. The condition `J cross W=0` is an invariant sector, not an
attractor supplied by this conservative dynamics.

### 4.3 Cubic-isotropy control

The general homogeneous quartic invariant under coordinate sign flips and
permutations is

\[
V_4=aI_1+bI_2,
\]

where

\[
I_1=J_x^4+J_y^4+J_z^4,
\qquad
I_2=J_x^2J_y^2+J_y^2J_z^2+J_z^2J_x^2.          \tag{7}
\]

A 45-degree rotation sends `(1,0,0)` to
`(1/sqrt(2),1/sqrt(2),0)`. Equality of (7) at those points requires

\[
a=\frac a2+\frac b4
\quad\Longleftrightarrow\quad b=2a.             \tag{8}
\]

Only then

\[
V_4=a(I_1+2I_2)=a|J|^4.                        \tag{9}
\]

Thus full rotational invariance or the induced self-pair/Frobenius norm
selects the radial ratio. Cubic symmetry by itself does not.

### 4.4 Exact vector discrete gradient

For one signed step `h != 0`, define

\[
\frac{J_1-J_0}{h}=\frac{W_1+W_0}{2m},          \tag{10}
\]

\[
\frac{W_1-W_0}{h}
=-\lambda(|J_1|^2+|J_0|^2)(J_1+J_0).           \tag{11}
\]

The force in (11) is the exact vector discrete gradient because

\[
(J_1-J_0)\cdot
\lambda(|J_1|^2+|J_0|^2)(J_1+J_0)
=\lambda(|J_1|^4-|J_0|^4).                     \tag{12}
\]

It tends to `4 lambda |J|^2 J` on the diagonal.

Equations (10)--(12) conserve (2) exactly and are self-adjoint under endpoint
exchange with `h -> -h`.

### 4.5 Global uniqueness

Eliminating `W_1` gives the vector equation

\[
F(X)=\frac{2m}{h}(X-J_0)-2W_0
+h\lambda(|X|^2+|J_0|^2)(X+J_0)=0.             \tag{13}
\]

Multiply by `sign(h)`. For

\[
g(X)=(|X|^2+|J_0|^2)(X+J_0)
\]

and any direction `v`, write `A=v dot X`, `B=v dot J_0`, and
`V2=|v|^2`. Then

\[
v\cdot Dg_Xv
=(|X|^2+|J_0|^2)V2+2A(A+B)                    \tag{14}
\]

has the exact decomposition

\[
2A^2+(A+B)^2
+(|X|^2V2-A^2)+(|J_0|^2V2-B^2).                \tag{15}
\]

Every term in (15) is nonnegative by Cauchy--Schwarz. The linear term in
(13) adds `(2m/|h|)|v|^2`, so the oriented map is strongly monotone. Its
leading radial term is coercive. The standard finite-dimensional
strong-monotonicity/coercivity theorem therefore gives exactly one solution
for every input state.

### 4.6 Conserved angular momentum and orientation

Let `Jbar=(J_1+J_0)/2` and `Wbar=(W_1+W_0)/2`. Then

\[
J_1\times W_1-J_0\times W_0
=Jbar\times(W_1-W_0)+(J_1-J_0)\times Wbar=0.   \tag{16}
\]

The swept-area scalar is

\[
\begin{aligned}
\chi_h
&=Jbar\cdot(W_1-W_0)-Wbar\cdot(J_1-J_0)\\
&=-h\left[
\frac\lambda2(|J_1|^2+|J_0|^2)|J_1+J_0|^2
+\frac{|W_1+W_0|^2}{4m}
\right].                                       \tag{17}
\end{aligned}
\]

For `h>0`, (17) is strictly negative on every nonzero step. Equality forces
antipodal endpoints, and then (10)--(11) force the origin.

## 5. Frozen exact checks

The implementation must run exactly 26 checks:

1. all seven frozen source hashes;
2. voxel-local `flux`/`wave_vel` fields and the conjugate-momentum declaration;
3. absence of a registered production pair-energy channel;
4. symmetry and rank-one form of `U=J tensor J`;
5. Frobenius identity `||U||_F^2=|J|^4`;
6. Hamiltonian/pair-energy identity;
7. radial cubic gradient `4 lambda |J|^2 J`;
8. vector Hamilton equations;
9. continuous angular-momentum conservation;
10. invariance of every fixed linearly polarized sector;
11. exact reduction to the scalar FTD-0840 Hamiltonian;
12. exact continuum period-amplitude `G*` law on that sector;
13. invariance of `I1,I2` under cubic signed-permutation generators;
14. 45-degree rotation forces `b=2a` for full isotropy;
15. the self-pair Frobenius choice has exactly that radial ratio;
16. vector discrete-gradient secant identity;
17. diagonal force limit;
18. exact discrete energy conservation;
19. endpoint/step and physical momentum reversal;
20. exact discrete angular-momentum conservation;
21. exact swept-area factorization;
22. strict orientation off the origin;
23. strong-monotonicity decomposition;
24. coercivity plus strong monotonicity gives one global next state;
25. compact positive-energy shell bounds; and
26. combined discriminator: local mathematical self-pair recursion passes,
    while production coupling, radial-isotropy selection, polarized-sector
    formation, maintained support, and finite-tick cadence remain open.

No check may be removed, reinterpreted, or tolerance-relaxed after execution.

## 6. Locked implementation

```text
scripts/proofs/proof_local_flux_self_pair_tensor.py
```

Script SHA-256:
`090A40C764FB72D5077A4519EFB46411F1003A35B6E173A65EFAFB5AD9974454`

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before the first execution. Run exactly:

```text
python scripts/proofs/proof_local_flux_self_pair_tensor.py
```

## 7. Outcomes

- **Outcome A — production-native local gearbox:** all exact checks pass and
  the frozen production source already contains the self-pair coupling,
  radial-isotropy selection, polarized formation law, and finite-tick
  cadence.
- **Outcome B — exact local mathematical gearbox, physical coupling open:**
  all 26 checks pass. The existing local field type admits an axis-free
  self-pair tensor and a unique, reversible, energy/rotation conserving
  recursion. The polarized sector has the exact continuum `G*` law.
  Production coupling, radial selection from cubic symmetry, autonomous
  polarization/localization, maintenance, and finite-tick cadence remain
  selected/open.
- **Outcome C — invalid:** any exact or source-hash check fails without
  establishing Outcome A. Book no theorem and repair under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run.

## 8. Recorded outcome

The first locked execution returned `26/26 PASS`. Registered Outcome B is
selected:

```text
VOXEL_FLUX_AND_WAVE_VELOCITY_SUPPLY_LOCAL_CANONICAL_TYPE
SELF_PAIR_TENSOR_FROBENIUS_ENERGY_GIVES_AXIS_FREE_QUARTIC
VECTOR_RECURSION_UNIQUE_REVERSIBLE_ENERGY_AND_ANGULAR_MOMENTUM_CLOSED
POLARIZED_CONTINUUM_SECTOR_HAS_GSTAR_PERIOD
PRODUCTION_COUPLING_ISOTROPY_POLARIZATION_SUPPORT_AND_TICK_CADENCE_OPEN
```

The local phase-space type and vector recursion are exact. The source audit
finds no production pair-energy channel, and cubic symmetry does not force
the radial quartic ratio. See
[`THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md`](../../derivations/native_time_carrier_programme/THEOREM_LOCAL_FLUX_SELF_PAIR_TENSOR_RECURSION_v1.md).
