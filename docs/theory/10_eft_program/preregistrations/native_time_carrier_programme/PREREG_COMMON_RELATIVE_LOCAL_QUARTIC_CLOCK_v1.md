# FTD-0843 — Common/relative local quartic clock v1

**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CERTIFICATE INVALID 26/28]`  
**Date:** 2026-08-10  
**Scope:** exact source-locked two-channel common/relative construction after
the FTD-0842 positive-edge obstruction  
**Production impact:** none

## 1. Registered question

Can the simplest two-channel system evade both FTD-0842 obstructions by
placing spatial propagation in the common mode and a local quartic clock in
the relative mode, while keeping total energy positive and every primitive
dependency within one Moore shell?

For local channel coordinates `L,R` and conjugate momenta `P_L,P_R`, define

\[
C=\frac{L+R}{\sqrt2},\qquad
D=\frac{L-R}{\sqrt2},                           \tag{1}
\]

and the same orthogonal transform for momenta. The selected architecture is:

- the source-free production kick--drift map acts on `C` with spatial
  operator `K`;
- each `D_i` evolves by the isolated FTD-0841 onsite quartic recursion; and
- the two sectors do not exchange energy.

This is the mathematical version of a “left/right” division of labor. It is
not a biological claim.

## 2. Epistemic firewall

The common/relative transform is exact. The **rank-one spatial coupling** that
makes only `C` propagate, the onsite quartic on `D`, their decoupling, and the
coefficient `lambda>0` are registered `[SELECTION]` inputs. Production's
current dual-substrate map propagates `L` and `R` separately and does not
supply this cross-gradient cancellation.

The certificate performs no numerical search, fit, target-period comparison,
or post-run tuning. Passing may establish a P4-local, positive-energy,
selection-scoped clock carrier. It cannot establish native formation,
readout, matter coupling, maintenance under perturbations, biological
function, Born frequencies, an actualization selector, or an exact integer
`G*` cadence.

## 3. Frozen source inputs

| Input | SHA-256 |
|---|---|
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |
| `engine/include/ftd/lagrangian.h` | `0225C75F34D1154CDF3783E73A86F051A3868E0E9087606E117411D75429350F` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_NATIVE_BILATERAL_QUARTIC_DYNAMICS_OBSTRUCTION_v1.md` | `2888C64166BC1E8B95807B6A8938A83971BDDF84718464B60D331B42C319C1DD` |
| `engine/include/ftd/eft/native_pair_energy_recursion.h` | `81B4941B951BC9D680A862188310706B86CDDA9DF9550204FC3F3DD567371E5A` |
| `docs/theory/10_eft_program/derivations/native_time_carrier_programme/THEOREM_COUPLED_SELF_PAIR_FIELD_ENERGY_CLOSURE_v1.md` | `6FECB7DFEA03DE14E96AD07A6780945182C92106FE64ED542318192841333C40` |

## 4. Frozen mathematics

### 4.1 Rank-one spatial energy

The general channel-symmetric quadratic edge energy for one edge difference
`dL,dR` is

\[
E_{\rm edge}
=\frac a2(|dL|^2+|dR|^2)+b\,dL\cdot dR.        \tag{2}
\]

Under (1),

\[
E_{\rm edge}
=\frac{a+b}{2}|dC|^2
+\frac{a-b}{2}|dD|^2.                           \tag{3}
\]

Positivity requires `|b|<=a`. Exact relative softness requires `b=a`, the
unique positive-semidefinite boundary point with common propagation:

\[
E_{\rm edge}=a|dC|^2,
\qquad E_{\rm edge}[D]=0.                       \tag{4}
\]

In channel coordinates the spatial metric is proportional to

\[
\begin{pmatrix}1&1\\1&1\end{pmatrix},          \tag{5}
\]

with eigenvalues `2,0`. The zero eigenvector is relative: `(1,-1)`.

### 4.2 Common propagating sector

For one primitive tick, use the frozen source-free production map on `C`:

\[
P^C_1=P^C_0-KC_0,
\qquad
C_1=C_0+P^C_1.                                  \tag{6}
\]

It preserves the FTD-0574/0293 tick invariant

\[
H_C=\frac12\langle P^C,P^C\rangle
+\frac12\langle C,KC\rangle
-\frac12\langle P^C,KC\rangle.                 \tag{7}
\]

For a `K` eigenvalue `0<a_K<4`, the metric of (7) has determinant
`a_K(1-a_K/4)>0`. The production FULL spectrum satisfies
`a_K<=16/9`, so all nonzero common modes lie in the positive region.

### 4.3 Relative onsite sector

At each site use the FTD-0841 recursion with `m=1` and primitive step `h=1`:

\[
D_{1i}-D_{0i}=\frac{P^D_{1i}+P^D_{0i}}2,        \tag{8}
\]

\[
P^D_{1i}-P^D_{0i}
=-\lambda(|D_{1i}|^2+|D_{0i}|^2)(D_{1i}+D_{0i}).\tag{9}
\]

It has one local next state and preserves

\[
H_D=\sum_i\left(\frac12|P^D_i|^2+\lambda|D_i|^4\right).\tag{10}
\]

The combined invariant is

\[
H_{\rm total}=H_C+H_D.                          \tag{11}
\]

Because the two maps are decoupled, (11) is exact. Equation (6) reads one
Moore shell and (8)--(9) read one site. Recombination to `L,R` is onsite.
The whole selected update therefore obeys P4 dependency range.

### 4.4 Local clock witness and boundary

Prepare `C=P^C=0` and a single relative site `i_0` with

\[
D_{i_0}=qe,\qquad P^D_{i_0}=pe                 \tag{12}
\]

for fixed unit `e`, with every other relative site zero. The support remains
exactly one site because the relative sector has no edge term. On (12),

\[
H_D=\frac{p^2}{2}+\lambda q^4,                  \tag{13}
\]

and the continuum period obeys

\[
TA=\sqrt\pi G^*\frac1{\sqrt{2\lambda}}.         \tag{14}
\]

The discrete swept-area orientation is strict off the origin by FTD-0840/0841.
The finite-step map is not exact quartic flow, so (14) is not an integer-tick
period theorem.

The exact decoupling that protects support also makes the clock invisible to
the common/actual channel. Any readout, synchronization, formation, or
maintenance interaction is an additional coupling whose energy leakage,
backreaction, and causal range must be audited.

### 4.5 Production control

The frozen dual production path computes separate Laplacians for `flux_L` and
`flux_R`. Therefore both `C` and `D` propagate with the same stiffness. It has
`b=0`, not the selected rank-one value `b=a`. FTD-0838 already proves the
current L/R core is block diagonal and supplies no cross-register quarter-turn
or pair closure.

## 5. Frozen exact checks

The implementation must run exactly 28 checks:

1. all seven source hashes;
2. production contains local L/R flux and wave registers;
3. production dual propagation is block diagonal/separate;
4. the common/relative transform is orthogonal;
5. the inverse transform is exact;
6. kinetic norm is preserved by the transform;
7. equation (2) transforms exactly to (3);
8. positivity requires `|b|<=a`;
9. exact relative softness forces `b=a`;
10. the rank-one channel matrix has eigenvalues `2a,0`;
11. the soft eigenvector is `(1,-1)`;
12. common propagation remains positive at `b=a`;
13. the common update is exactly the source-free production kick--drift map;
14. the common tick invariant (7) is conserved;
15. the common invariant is positive for `0<a_K<4`;
16. the production spectral ceiling `16/9` lies inside that region;
17. the relative update is exactly FTD-0841 sitewise;
18. each relative next state is globally unique at its site;
19. relative energy (10) is exact;
20. total energy (11) is exact by decoupling;
21. the combined dependency radius is one Moore shell;
22. a single-site relative support stays exact;
23. every fixed relative polarization is invariant;
24. the polarized continuum sector has the exact `G*` period factor;
25. the relative discrete orientation is strict off zero;
26. any `b<a` leaves positive relative quadratic stiffness;
27. `b>a` makes the relative edge energy negative; and
28. combined discriminator: a positive, P4-local, selection-scoped local
    quartic carrier exists, while production cross-gradient, formation,
    readout, maintenance, and finite-tick cadence remain open.

## 6. Locked implementation

```text
scripts/proofs/proof_common_relative_local_quartic_clock.py
```

Script SHA-256:
`D5CCC53504E162D9999AAAE7F0142F7FD8EA98DBE153328059A6672C79B68076`

Pre-run protocol SHA-256:
`5EE8F82A9ACBF256AF5E41E2EFAF4836CF362233CFADCD5FBA8EC3369C27FF65`

The script hash and pre-run protocol hash must be entered in
`REF_PREREGISTER_MANIFEST.md` before the first execution. Run exactly:

```text
python scripts/proofs/proof_common_relative_local_quartic_clock.py
```

## 7. Outcomes

- **Outcome A — production-native relative clock:** all checks pass and the
  frozen dual production core already has the rank-one common-only spatial
  energy and onsite relative quartic/readout.
- **Outcome B — exact selected local carrier:** all 28 checks pass. The
  rank-one common/relative architecture is positive and P4-local, and a
  single relative site is an autonomous conditional quartic carrier.
  Production cross-gradient, formation, readout, maintenance, and finite-tick
  cadence remain selected/open.
- **Outcome C — invalid:** any exact or source-hash check fails without
  establishing Outcome A. Book no theorem and repair under a fresh lock.

The expected result is Outcome B. That expectation is frozen before the run.

## 8. Recorded outcome

The first locked execution returned `26/28`. C14 compared two algebraically
equal SymPy matrices by structural equality without simplification;
`simplify(U.T*G*U-G)` is the exact zero matrix. C28 inherited C14's failed
counter. The parent certificate is invalid and books no theorem. No physical
equation, source, coefficient, or outcome is changed under this lock. A fresh
repair protocol must change only C14 and C28 to exact simplified-difference
comparisons.
