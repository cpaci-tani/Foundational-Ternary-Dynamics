# AUDIT — Central-generator field-momentum recoil

**Date:** 2026-07-24  
**Identifier:** `FTD-0438`  
**Status:** `[MEASURED — SELECTED FORCE]` + `[CLOSED NEGATIVE — CENTRAL LOCAL TOTAL MOMENTUM]`  
**Verdict:** `NO_COMPENSATING_FIELD_RECOIL`  
**Pre-registration:** [`PREREG_CENTRAL_FIELD_MOMENTUM_RECOIL_v1.md`](../10_eft_program/preregistrations/PREREG_CENTRAL_FIELD_MOMENTUM_RECOIL_v1.md)  
**Run of record:** `engine/results/ftd_0438/windows_msvc_cpu_L33.csv`

## 1. Result

The production wave tick conserves the preregistered central-generator field
momentum in all three Cartesian directions. The worst 200-tick drift was

$$
7.99\times10^{-12}\quad\text{absolute},\qquad
6.15\times10^{-13}\quad\text{relative}.
$$

The candidate therefore passed its independent source-free control.

For every isolated-pair axis and polarity orientation, the particle momentum
reached a maximum magnitude

$$
|P_{\rm particle}|_{\max}=0.00640503776994,
$$

while the largest central field momentum in any arm was only

$$
|P_{\rm field}|_{\max}=4.90488922718\times10^{-18}.
$$

Thus the field contribution was below `7.66e-16` of the particle contribution.
The worst total residual was `0.00640503776994` and the locked closure ratio was
`0.9999999999999996`, not the required `1e-6`. Reversing the pair orientation
reversed the unbalanced momentum; rotating the pair among `x,y,z` preserved its
magnitude.

## 2. What is closed

The compensating-recoil explanation for FTD-0437 is closed for the local
central translation generator

$$
P_i^{\rm field}=-\sum_x W\cdot D_iJ.
$$

The selected `emergent_forces` branch creates momentum in the manifested
particle subsystem without transferring measurable opposite momentum into this
native flux channel. The effect is not a scan-axis artifact: it is covariant
under the measured cubic rotations and odd under dipole reversal.

## 3. Mechanism exposed

The production field source and particle force are not a reciprocal pair. The
field is sourced through `-G_C grad(s)` in `phase_read.cpp`, whereas the selected
particle force is

$$
F=G_Cs\,\nabla|J|_{r=2}
$$

in `phase_forces.cpp`. The first varies the vector field by a central gradient
of signed state; the second differentiates the scalar magnitude of the field on
a different radius-two stencil. No common discrete interaction functional is
identified whose variations yield both operations. The measured absence of
field recoil is the dynamical consequence expected from that nonreciprocity.

This does not prove that every conceivable nonlocal lattice quasimonentum fails.
It does prove that the obvious local generator exactly conserved by the free
wave sector does not close once this selected force moves matter.

## 4. Epistemic consequence

The `G_C s grad|J|` branch cannot currently be called native conservative
matter-field mechanics. It is a selected phenomenological force rule with a
closed-negative particle-plus-central-field momentum gate. Behaviors depending
on that branch may be visualized as engine behavior, but they are not qualified
as physical electromagnetic dynamics.

The flux field itself remains a valid propagating dynamical field in its
source-free sector. FTD-0438 isolates the defect to the matter-force coupling;
it does not invalidate the wave tick or its conserved pseudomomentum.

## 5. Next discriminating test

The next registered campaign must compare the three already implemented force
branches—selected magnitude-gradient, legacy divergence-gradient, and Poisson
Coulomb—on the same isolated pair. That test distinguishes a defect specific to
`grad|J|` from a broader movement/update-order defect. No branch may be
rescaled or tuned to obtain cancellation.

## 6. Reproducibility

- source SHA256: `5102a88534fa07c4a4f2b8e838e1094b5aaa649fd4058cbec5d5dd0459800780`
- record SHA256: `9f6451f082f02e7f458fc1c3855966ca07ae4cc7198662901e712616f5c967f6`
- compiler: pinned MSVC `14.44.35207`, Release
- backend: forced CPU, periodic `L=33`
- result: `NO_COMPENSATING_FIELD_RECOIL`
- focused CTest: FTD-0437/0438 `2/2` passed
- golden merge-gate battery: `7/7` passed

The full build was blocked by unrelated pre-existing failures in
`test_scenario_behavior.cpp` and a locked existing hazard-campaign executable.
The FTD-0438 target itself compiled and linked successfully under the canonical
toolchain.
