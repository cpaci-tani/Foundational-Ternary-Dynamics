# FTD-0934 — Preregistration: C4 dressing translation cocycle and directed-recoil state necessity v1

**Identifier:** `FTD-0934`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** phase-averaged FTD-0933 C4 dressing mismatch as a translation-group
cocycle; exact negative-type metric; loss of hop orientation under the
symmetric energy square; character classification of the minimum directed
translation phase; Bloch-torus/carry boundary; no numerical search, fit,
engine mutation, new ontology adoption, physical momentum normalization,
vector recoil update, production promotion, `G*`, Born, Bell, context,
outcome, or hiding read

## 1. Question

FTD-0933 proves that one abrupt integer source hop leaves a finite positive
field wake with spectral weight

\[
 2[1-\cos(k\cdot d)]=|1-e^{-ik\cdot d}|^2.             \tag{1}
\]

Equation (1) already contains a clue about the missing recoil dynamics. It is
a symmetric square. It retains displacement magnitude and anisotropy, but it
forgets the conjugate distinction between `d` and `-d`.

The present discriminator asks:

1. does the exact C4 wake define a genuine translation-group cocycle and
   metric of negative type;
2. can that positive scalar alone select a directed local hop or recoil;
3. what is the minimum mathematical datum that restores `d` versus `-d`
   without choosing a preferred spatial axis by hand; and
4. does that datum already supply physical momentum, or only a compact Bloch
   character whose dynamics and lift remain open?

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `PREREG_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md` | `5CE2119C670A7A15BD2DCA599AAE6F9F521620853BF1C08671FD3F4D7FA38EC9` |
| `THEOREM_C4_COMPANION_TRANSLATION_MISMATCH_DRESSING_METRIC_AND_RECOIL_BOUNDARY_v1.md` | `BE70433D871293C42FACD879FF4C8D5E3DCD23DAF83CAD7266806648DF17024F` |
| `proof_c4_companion_translation_mismatch_dressing_metric_recoil_boundary.py` | `5B56223709DA3957F852D889F4514D94F261F3819E3178E0E4FA43CEB74814FC` |
| `THEOREM_SYMMETRIC_CHORD_MOORE_ACTION.md` | `B80E574B8C421B28DC0AFFC35F5B898DF6FF79A1CEBA06588B22862FDCF1468D` |
| `THEOREM_CONTINUOUS_TRANSLATION_LOCALITY_TRILEMMA.md` | `527BDA49C213C1D58862A8A6254FC153416253EA3159BD7B958F8E43B69630EC` |
| `THEOREM_PASSIVE_DRESSING_DEPINNING_OBSTRUCTION.md` | `238AB6376EBC3FFE0A7324352C764D3BD5224EB89B91D05CF438067C6E6164CD` |
| `THEOREM_DRESSED_BOOST_MOMENTUM_MAP_AND_INERTIAL_IDENTIFIABILITY_BOUNDARY_v1.md` | `378E38227422336BF9956EA6668CA7C09006B3A1D226370577126944654F833C` |
| `THEOREM_BLOCH_QUASIMOMENTUM_LIFT_AND_LOCAL_MOMENTUM_MAP_TRILEMMA_v1.md` | `0C2F0C289C82D45457B5DF330F767C10AD5CA3966FB667B329391C283FD47973` |
| `THEOREM_RECIPROCAL_CARRY_RESERVOIR_AND_LOCAL_IMPULSE_LEDGER_BOUNDARY_v1.md` | `8696F6024CE6ED49120DF6A238F98C8C804AA7B8C441BCA83B5AFDCE111C6048` |
| `THEOREM_CUBIC_REACTION_VECTOR_AND_RELATIVISTIC_SOURCE_TRANSPORT_BOUNDARY_v1.md` | `56F3DF2B830A5C52320757DAF368EAA72F3E4A4B1DA388090A2E1EB7F30C2D27` |

The certificate fails closed on source drift.

## 3. Frozen energy Hilbert space and cocycle

Let `Y_r=(q_r,p_r)` be the four exact C4 companion phases. Polarize the
positive native energy `H_C4` to obtain a real Hilbert inner product on the
finite-energy field-pair space. Use the four-phase direct sum

\[
 \mathcal H_4=\bigoplus_{r=0}^3\mathcal H_{C4},
 \qquad
 \|Y\|_4^2={1\over4}\sum_{r=0}^3H_{C4}(Y_r).           \tag{2}
\]

Integer translation acts unitarily by

\[
 \pi(d)=\bigoplus_{r=0}^3T_d.                          \tag{3}
\]

Register

\[
 \boxed{b_Y(d)=\pi(d)Y-Y}                              \tag{4}
\]

and the phase-averaged wake

\[
 \boxed{
 \overline{\mathcal D}(d)=\|b_Y(d)\|_4^2
 ={1\over4}\sum_{r=0}^3\mathcal D_r(d).}              \tag{5}
\]

The certificate must prove the exact cocycle identity

\[
 \boxed{
 b_Y(d+e)=b_Y(d)+\pi(d)b_Y(e).}                        \tag{6}
\]

Translation unitarity must then give

\[
 \overline{\mathcal D}(0)=0,
 \qquad
 \overline{\mathcal D}(-d)=\overline{\mathcal D}(d), 
 \qquad
 \sqrt{\overline{\mathcal D}(d+e)}
 \le\sqrt{\overline{\mathcal D}(d)}
   +\sqrt{\overline{\mathcal D}(e)}.                 \tag{7}
\]

For nonzero compact source data with no nontrivial translation stabilizer,
`sqrt(Dbar(d-e))` must be a genuine metric on the translation orbit.

## 4. Frozen negative-type and polarization identities

For real coefficients `c_a` satisfying `sum_a c_a=0`, register

\[
 \boxed{
 \sum_{a,b}c_ac_b\overline{\mathcal D}(d_a-d_b)
 =-2\left\|\sum_a c_a\pi(d_a)Y\right\|_4^2\le0.}      \tag{8}
\]

Thus the wake is a conditionally negative-definite function on `Z^3`. Its
exact polarization is

\[
 \boxed{
 B_Y(d,e)={1\over2}
 [\overline{\mathcal D}(d)+\overline{\mathcal D}(e)
 -\overline{\mathcal D}(d-e)]
 =\langle b_Y(d),b_Y(e)\rangle_4.}                    \tag{9}
\]

Equations (4)--(9) are permitted to be called a natural dressing-space
embedding of integer source positions. They are not a source equation of
motion.

## 5. Frozen directed-hop obstruction

The edge action furnished by the field is the nonnegative scalar

\[
 S_{\rm wake}[X]=\sum_n
 \overline{\mathcal D}(X_{n+1}-X_n).                  \tag{10}
\]

It is translation invariant and time-reversal invariant, but equation (7)
implies

\[
 \overline{\mathcal D}(d)=\overline{\mathcal D}(-d).  \tag{11}
\]

Therefore it cannot distinguish a hop from its reverse. If the only allowed
step cost is equation (10), rest has zero cost while every nonzero step has
positive cost. A minimum-action local step remains at rest. If a nonzero hop
is externally forced, cubic covariance makes all directions in the same
face/edge/corner orbit degenerate.

Register the scoped no-go:

\[
 \boxed{
 \text{positive dressing energy alone cannot select a nonzero directed
 hop or vector recoil from an isotropic rest state}.}                  \tag{12}
\]

Equation (12) applies to scalar laws built only from the even dressing
distance and cubic-invariant source data. It does not exclude a law with an
existing or newly derived time-odd polar vector, incoming wave momentum,
external gradient, anisotropic body axis, or topological defect orientation.

## 6. Frozen translation-character classifier

The integer translation group has Pontryagin dual `T^3`. Every unitary
one-dimensional character is fixed by its values on the three basis steps
and has the form

\[
 \boxed{
 \chi_k(d)=e^{ik\cdot d},
 \qquad
 k\in\mathbb T^3.}                                    \tag{13}
\]

It obeys

\[
 \chi_k(d+e)=\chi_k(d)\chi_k(e),
 \qquad
 \chi_k(-d)=\overline{\chi_k(d)}.                     \tag{14}
\]

The even wake factor is exactly

\[
 |1-\chi_k(d)|^2=2[1-\cos(k\cdot d)],                 \tag{15}
\]

while its oriented companion

\[
 \operatorname{Im}\chi_k(d)=\sin(k\cdot d)            \tag{16}
\]

changes sign under `d -> -d`. Equation (15) is the symmetric square that
loses the conjugation/orientation information retained by equation (16).

The certificate may therefore classify a nontrivial compact character
`k in T^3` as the minimum representation class that can distinguish directed
translations without a fixed axis. With an independently supplied polar axis,
one compact phase along that axis suffices. This is a representation
necessity, not an adoption of a new persistent production type.

A spatially scalar internal C4 phase `i` does not by itself determine `k`.
To direct translation it must be coupled to a polar spatial axis or vector.

## 7. Frozen momentum and carry boundary

The character label `k` is Bloch quasimomentum modulo the reciprocal lattice.
FTD-0896 forbids promoting it to a globally continuous additive
`R^3` momentum without branch choice or winding history. FTD-0897 proves that,
once an opposite increment is supplied, an integer carry can preserve a
chosen lift exactly; it does not derive the increment, its energy, or its
owner.

Accordingly the present result must keep separate:

1. **even dressing energy:** equations (5), (8), and (15);
2. **directed compact phase:** equations (13), (14), and (16);
3. **unwrapped physical momentum:** still requires a scale and carry/lift;
4. **reciprocal dynamics:** still requires a common action that updates the
   source phase and field with equal/opposite impulse.

No vector recoil follows from the cocycle alone.

## 8. Registered outcomes

- **Outcome A — negative-type dressing geometry / directed-state
  necessity:** equations (4)--(16) pass. The wake defines an exact
  translation cocycle and metric of negative type, but its symmetric square
  cannot choose a directed hop. The lost information is precisely the
  conjugation sign retained by a nontrivial translation character. The
  character is compact Bloch data, not yet physical momentum. Dynamic
  common action, impulse origin, carry ownership, and scale remain open.
- **Outcome B — cocycle geometry only:** equations (4)--(9) pass, but the
  directed-state classification or cubic no-go fails. No state-minimum claim
  is licensed.
- **Outcome C — no translation geometry:** cocycle, negative type,
  polarization, or strict metric fails for the registered companion class.
- **Invalid:** source drift, post-lock formula change, numerical search,
  fitted metric, a physical-momentum or vector-recoil promotion, new ontology
  adoption, engine/CMake mutation, production promotion, context/Born read,
  or completed-infinity rhetoric.

## 9. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, paper, physical constant, phenomenological formula, Born
weight, Bell correlation, measurement context, or `G*` cadence is changed.

Even Outcome A does not derive a local source action, autonomous hop rule,
source kinetic term, physical momentum scale, vector recoil, carry ownership,
exceptional mobile carrier, source formation, attraction, recovery, collision
composition, Lorentz hiding, or framework completeness.
