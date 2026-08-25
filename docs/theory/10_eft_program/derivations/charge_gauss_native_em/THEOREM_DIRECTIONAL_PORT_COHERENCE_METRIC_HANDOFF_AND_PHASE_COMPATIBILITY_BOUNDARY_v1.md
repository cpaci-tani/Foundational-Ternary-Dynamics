# Directional-port coherence metric, handoff, and phase-compatibility boundary v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT THREE-PARAMETER INVARIANT CHANNEL GRAM]** +
**[THEOREM — STANDING/OUTGOING/FREE ENERGY AND PSD HANDOFF FAMILY]** +
**[SCOPED NO-GO — SYMMETRY AND STREAMING DO NOT FIX EMISSION WORK]** +
**[BOUNDARY CLOSED CONDITIONALLY BY SUCCESSOR — C4 FIELD TYPE SELECTS THE HANDOFF METRIC]** +
**[CORRECTED BY SUCCESSOR — FIELD AND RAW BORN KERNELS ARE DISTINCT C4 SECTORS]**  
**Production status:** unchanged  
**Ledger status:** no row minted; no alpha or Born claim promoted

**Exact certificate:**
[proof_directional_port_coherence_metric_handoff_family.py](../../../../../scripts/proofs/proof_directional_port_coherence_metric_handoff_family.py)
performs **216 exact symbolic checks**. It diagonalizes the complete normalized
four-channel Gram, proves its positive-semidefinite handoff family, evaluates
four exact control metrics, and checks the spatial/charge carrier geometry on
all 24 ordered planes, both propagation branches, and both charge signs. No
target coupling, probability, master root, or experimental number enters.

---

## 1. The records do not determine which cross terms are physical

At one directional-port edge there are four records labelled by:

- internal cotangent handedness \(h=\pm1\); and
- the two retained C4 phase bands \(p\) and \(p+2\).

Normalize each channel's self-weight to one. Invariance under exchanging the
two handed channels and exchanging the two phase bands leaves three independent
cross weights:

\[
 \begin{array}{c|l}
 a & \text{same phase band, opposite handedness},\\
 b & \text{same handedness, opposite phase band},\\
 c & \text{both labels opposite}.
 \end{array}                                                \tag{1}
\]

In channel order \((h_+,p),(h_-,p),(h_+,p+2),(h_-,p+2)\), the exact Gram is

\[
 G(a,b,c)=
 \begin{pmatrix}
 1&a&b&c\\
 a&1&c&b\\
 b&c&1&a\\
 c&b&a&1
 \end{pmatrix}.                                             \tag{2}
\]

Spatial signed-cubic transformations act orthogonally on the \((E,B)\)
vectors and only permute/sign the channel labels, so they do not reduce the
three scalar weights in equation (2).

---

## 2. Exact positivity spectrum

The four Klein characters diagonalize equation (2). Its eigenvalues are

\[
 \boxed{
 \begin{aligned}
 \lambda_0&=1+a+b+c,\\
 \lambda_h&=1-a+b-c,\\
 \lambda_p&=1+a-b-c,\\
 \lambda_{hp}&=1-a-b+c.
 \end{aligned}}                                             \tag{3}
\]

A candidate energy metric must make all four nonnegative. Positivity does not
select one point; it defines a finite convex region.

---

## 3. Standing, outgoing, and separated energies

The exact port fields give the normalized energies

\[
 \boxed{
 H_{\cal S}={1+a\over2},
 \qquad
 H_{\cal O}={1+a+b+c\over2}.}                               \tag{4}
\]

After two ticks, opposite-handed rays are spatially separated while the two
phase bands of each ray remain co-located. The free multi-ray carrier has

\[
 \boxed{H_{\rm free}={1+b\over2}.}                          \tag{5}
\]

Therefore

\[
 \Delta H_{\rm emit}=H_{\cal O}-H_{\cal S}={b+c\over2},
 \qquad
 \Delta H_{\rm handoff}=H_{\cal O}-H_{\rm free}={a+c\over2}. \tag{6}
\]

Exact collisionless handoff conservation requires

\[
 \boxed{c=-a.}                                              \tag{7}
\]

On that section the PSD conditions reduce to

\[
 \boxed{b\ge-1,\qquad |2a|\le1-b,}                       \tag{8}
\]

and the emission work is

\[
 \boxed{\Delta H_{\rm emit}={b-a\over2}.}                  \tag{9}
\]

Positive field emission additionally requires \(b>a\). Equations (8)--(9)
leave a continuum, not a unique coefficient.

---

## 4. Exact controls

Three useful metrics demonstrate the underdetermination:

\[
 \begin{array}{c|c|c|c|c}
 (a,b,c)&H_{\cal S}&H_{\cal O}&H_{\rm free}&\text{interpretation}\\ \hline
 (0,0,0)&1/2&1/2&1/2&\text{fully channel resolved}\\
 (0,1,0)&1/2&1&1&\text{phase-coherent, flag resolved}\\
 (1,1,1)&1&2&1&\text{fully coarse moment square}.
 \end{array}                                                \tag{10}
\]

All three Grams are positive semidefinite and symmetry compatible. The first
two preserve port-to-free energy; the first assigns zero emission work and the
second assigns one half. The fully coarse metric is the previous
\((2,r)\to(1,r/2)\) handoff defect: its omitted cross-term energy is exactly
one.

The interior point

\[
 (a,b,c)=\left({1\over4},{1\over2},-{1\over4}\right)        \tag{11}
\]

also preserves handoff and is positive semidefinite, proving that equation
(7) is a genuine family rather than a choice between two endpoints.

---

## 5. Consequence for the native coupling measure

Let \(I_*\) be released source work, \(\Gamma\) the coefficient multiplying
the channel Gram, and \(\mu/2\) the positive unit recoil energy. The complete
immediate-step reference partition is

\[
 \boxed{
 I_*=\Gamma{(b-a)\over2}+{\mu\over2}}                       \tag{12}
\]

on the handoff-conserving section. The physical field-work fraction remains

\[
 {\Gamma\Delta H_{\rm emit}\over I_*}
 =1-{\mu\over2I_*},                                         \tag{13}
\]

but the bare sector coefficient is

\[
 {\Gamma\over I_*}
 ={2\over b-a}\left(1-{\mu\over2I_*}\right).               \tag{14}
\]

Thus deriving inertia alone is not enough. The common action must also derive
which record channels interfere. Packet count, Poynting normalization, and
symmetry cannot determine the coupling without that coherence metric.

The later exact
[C4-trivial field-sector successor](THEOREM_C4_TRIVIAL_FIELD_SECTOR_UNIQUE_DIRECTIONAL_PORT_HANDOFF_METRIC_v1.md)
adds the physical information intentionally absent here: the actual cotangent
field readout is C4 phase blind. That factorization fixes (b=1,c=a), while
equation (7) fixes (c=-a). Hence the registered positive
handoff-conserving class has the unique member
((a,b,c)=(0,1,0)). The continuum in this theorem remains a valid
classification before the field C4 type is imposed; it is no longer the final
field-metric boundary.

The later
[clocked-remainder recoil theorem](../common_action_mechanics_reciprocity/THEOREM_CLOCKED_REMAINDER_RECOIL_AND_DISCRETE_TRANSLATION_CHARGE_BOUNDARY_v1.md)
further replaces the immediate material term \(\mu/2\) by \(\mu/(2L)\),
where \(\mu=m/L\) is the persistent impulse magnitude of a speed-\(1/L\)
worldline. Thus equations (12)--(14) remain the correct classification for
the registered immediate-step model, not the current slow-material energy
ledger.

---

## 6. Corrected connection to contextual actualization

This theorem established that the field records do not select their physical
quadratic form. It did **not** establish that the same scalar phase Gram can be
used for raw Born histories. The exact
[C4 kernel-separation successor](../common_action_mechanics_reciprocity/THEOREM_C4_BORN_RADIATION_KERNEL_SEPARATION_AND_CONTEXTUAL_MIXER_BOUNDARY_v1.md)
performs that comparison and proves otherwise:

- the present phase-blind cotangent field readout lies in the trivial C4
  sector and has opposite-band cross weight \(+1\);
- the raw equal-weight Born form lies in the two-dimensional quadrature sector
  and has opposite-phase cross weight \(-1\); and
- imposing that raw Born value on the positive, handoff-conserving port family
  makes positive emission impossible.

Therefore “one native action” does not mean one untyped compatibility Gram.
It means one action must derive both orthogonal C4 sector projectors and the
physical detector-context vertex that couples them, including reciprocal work
and the event pushforward. No equality between \(b\), a Born probability,
alpha, or the master root is asserted.

---

## 7. Updated gate

The port-to-free “missing energy” is not evidence that information or energy
vanishes. It shows that the fully coarse moment squared merged a microscopic
channel that physical conservation requires the field Hamiltonian to resolve.
The field-sector successor now closes that metric ambiguity conditionally,
giving (H_{\cal O}=H_{\rm free}=1). The next gate is a finite common action
that actually realizes the selected metric, supplies translational
Noether/Legendre momentum and formed inertia, and reduces the eight-ray
carrier to two Maxwell modes. The contextual Born preparation and frequency
pushforward remain separately open through their nonlinear quadrature-pair
vertex.
