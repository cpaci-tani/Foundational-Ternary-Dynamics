# FTD-0418 — One-tick spacetime Wilson regulator

**Date:** 2026-07-22  
**Status:** `[SELECTED BRANCH-B REGULATOR]` + `[THEOREM — selected action: gauge covariance, unique massless corner, and exact Ward hierarchy through two photons]` + `[DERIVED — leading common cone and quartic mismatch]` + `[FTD-0419 SUCCESSOR — step-scheme threshold closed negative]` + `[OPEN — on-shell threshold and ternary current]`  
**Verdict:** `LOCAL-GAUGE-MATTER-PAIR-FROZEN; WARD-COMPLETE-THROUGH-ONE-LOOP; MATCHING-INTEGRAL-OPEN`  
**Verifier:** `scripts/proofs/proof_lorentz_spacetime_wilson.py`

---

## 0. Outcome

FTD-0417 froze a local unit-plaquette photon action but left its matter clock
continuous. The smallest compatible spacetime regulator is the anisotropically
rescaled nearest-neighbour Wilson operator

$$
\boxed{
S_A=\frac12\sum_n\left[
\sum_iF_{0i}^2+c^2\sum_{i<j}F_{ij}^2
\right],
\qquad c^2=\frac17,}
$$

$$
\boxed{
(D_W\psi)_n=(m+1+3c)\psi_n
-\frac12\sum_{\mu=0}^3\left[
(r_\mu-\nu_\mu\gamma_\mu)U_\mu(n)\psi_{n+\hat\mu}
+(r_\mu+\nu_\mu\gamma_\mu)U_\mu^\dagger(n-\hat\mu)
\psi_{n-\hat\mu}
\right],}
$$

with

$$
\nu_0=r_0=1,
\qquad
\nu_i=r_i=c=\frac1{\sqrt7},
\qquad
U_\mu(n)=e^{igA_\mu(n)}.
$$

Both actions use one temporal step and unit axial links only. Their massless
infrared poles have the same leading speed `c`, all 15 non-origin spacetime
corners are Wilson-gapped, and the complete one- and two-photon vertices follow
by differentiating this one action. The exact lattice Ward identities then
hold algebraically; no vertex or seagull is added by hand.

This is a **Branch-B regulator selection**. It does not derive spinors from the
ternary state, identify the Wilson current with a current of ternary history,
or prove unitary real-time evolution of the many-to-one FTD tick. Its purpose
is narrower: it removes the mixed continuous/discrete clock ambiguity and
makes the FTD-0417 full-Brillouin-zone one-loop matching problem mathematically
defined.

---

## 1. Frozen Euclidean action

Use dimensionless lattice spacing `a=1`; restoring `a` is a separate physical
calibration. Sites are `n in Z^4`, `gamma_mu` are Euclidean gamma matrices, and
the noncompact link connection transforms as

$$
A_\mu(n)\mapsto A_\mu(n)+\chi(n)-\chi(n+\hat\mu),
$$

$$
\psi_n\mapsto e^{ig\chi(n)}\psi_n,
\qquad
\bar\psi_n\mapsto\bar\psi_ne^{-ig\chi(n)}.
$$

Consequently

$$
U_\mu(n)\mapsto
e^{ig\chi(n)}U_\mu(n)e^{-ig\chi(n+\hat\mu)},
$$

and every forward and backward Wilson hop is gauge covariant. The matter
action `S_F=sum_n bar(psi)_n(D_W psi)_n` is exactly gauge invariant.

The choice `r_i=nu_i=c` rescales the complete spatial Wilson operator rather
than only its Dirac part:

$$
D_W(p)=m+(1-\cos p_0)+c\sum_i(1-\cos q_i)
+i\gamma_0\sin p_0+ic\sum_i\gamma_i\sin q_i.
$$

It is the one-parameter anisotropic image of the standard nearest-neighbour
Wilson regulator. Positivity of every `r_mu` is what lifts the corners;
`r_i=c` is a minimal coefficient selection, not something forced by P1--P5.

### 1.1 Adoption bill

| Input | Status | Price |
|---|---|---|
| Wilson spinor and nearest-neighbour Wilson action | `[SELECTED BRANCH-B REGULATOR]` | existing imported-matter line `IMP-E3` |
| independent link carrier `A_mu` | `[SELECTED ONTOLOGY EXTENSION]` | existing carrier choice `IMP-S4` |
| physical identification `g^2=1/x_+` if used | `[IMPOSED — calibration]` + `[SMC]` | existing composition `IMP-E1 o IMP-E3`; not used in the proofs here |
| `c^2=1/7` | inherited `[SELECTION]` | common leading cone chosen in FTD-0411/0417 |
| `r_i=nu_i=c`, `r_0=nu_0=1` | `[SELECTION]` | no additional fitted physical target |

No new import-ledger type is created. FTD-0418 consumes the already priced
Branch-B Wilson matter and local-link carrier. The coupling `g` remains
symbolic throughout the exact calculation.

---

## 2. Free pole and all 15 doublers

At small momentum,

$$
D_W(p)=m+ip_0\gamma_0+ic\sum_iq_i\gamma_i
+\frac12\left(p_0^2+cS_2\right)+O(p^3),
\qquad S_2=\sum_iq_i^2.
$$

The massless Dirac part therefore has the leading cone

$$
E^2=c^2S_2+O(q^4),
\qquad c^2=\frac17,
$$

which agrees with the FTD-0417 photon only at leading order.

At a spacetime Brillouin corner, let `n_0 in {0,1}` say whether
`p_0=pi`, and let `n_s in {0,1,2,3}` count spatial components equal to
`pi`. Every sine vanishes, while the Wilson scalar is

$$
M_{n_0,n_s}=m+2n_0+2cn_s.
$$

For `m=0`, only `(n_0,n_s)=(0,0)` is zero. The other 15 corners have
strictly positive mass, with smallest shift `2/sqrt(7)`. This is an exact
corner count, not a numerical scan.

Analytically continuing `p_0=iE` gives the exact massless pole

$$
\cosh E
=1+\frac{c^2\left(P+H^2\right)}{2(1+cH)},
$$

where

$$
H=\sum_i(1-\cos q_i),
\qquad
P=\sum_i\sin^2q_i.
$$

The right side is at least one throughout the spatial Brillouin zone. This
establishes a real Euclidean correlation energy for the free regulator. It is
not a proof that the fundamental FTD tick is unitary.

### 2.1 The quartic mismatch is explicit

For `Q_4=sum_i q_i^4` and `P_22=sum_{i<j}q_i^2q_j^2`, expansion of the exact
fermion pole yields

$$
\frac{E_F^2}{c^2}
=S_2+left(-\frac2{21}-\frac1{2\sqrt7}\right)Q_4
+\left(\frac{10}{21}-\frac1{\sqrt7}\right)P_{22}
+O(q^6).
$$

FTD-0417 instead gives

$$
\frac{E_A^2}{c^2}
=S_2-\frac1{14}Q_4+\frac1{42}P_{22}+O(q^6).
$$

Thus the local pair shares the leading cone but not the quartic tensor. The
minimal axial action intentionally abandons FTD-0413's face-diagonal q4
improvement. No claim of full Lorentz invariance follows from this pair.

---

## 3. Complete one-photon vertex

Use link-midpoint Fourier phases. Let an incoming fermion have momentum `p`,
an outgoing fermion have `p'=p+k`, and define

$$
\bar p_\mu=\frac{p'_\mu+p_\mu}{2},
\qquad
\widehat k_\mu=2\sin\frac{k_\mu}{2}.
$$

The first functional derivative of `D_W[A]` at `A=0` is

$$
\boxed{
V_\mu^{(1)}(p',p)
=g\left[r_\mu\sin\bar p_\mu
+i\nu_\mu\gamma_\mu\cos\bar p_\mu\right].}
$$

The scalar sine term is the Wilson contribution. Dropping it gives the wrong
lattice current even at one photon.

The exact first Ward identity is

$$
\boxed{
\sum_\mu\widehat k_\mu V_\mu^{(1)}(p+k,p)
=g\left[D_W(p+k)-D_W(p)\right].}
$$

It follows componentwise from

$$
2\sin\frac{k}{2}\sin\left(p+\frac{k}{2}\right)
=\cos p-\cos(p+k),
$$

$$
2\sin\frac{k}{2}\cos\left(p+\frac{k}{2}\right)
=\sin(p+k)-\sin p.
$$

---

## 4. Complete two-photon seagull

Let the photons carry momenta `k` and `l`, so `p'=p+k+l` and
`bar(p)=(p'+p)/2`. The second functional derivative is

$$
\boxed{
V_{\mu\nu}^{(2)}(p',p;k,l)
=\delta_{\mu\nu}g^2
\left[r_\mu\cos\bar p_\mu
-i\nu_\mu\gamma_\mu\sin\bar p_\mu\right].}
$$

There is no mixed `mu != nu` seagull because the selected matter action has
only axial single-link hops. Face-diagonal hops would create mixed contact
terms and are outside this minimal branch.

The second Ward identity is

$$
\boxed{
\sum_\mu\widehat k_\mu V_{\mu\nu}^{(2)}(p+k+l,p;k,l)
=g\left[
V_\nu^{(1)}(p+k+l,p+k;l)
-V_\nu^{(1)}(p+l,p;l)
\right].}
$$

The contact term is therefore compulsory. A one-loop self-energy or
polarization calculation that keeps only the two one-photon vertices is not
the expansion of the frozen action and fails the second identity.

---

## 5. Local anisotropic Feynman gauge

To define the loop integrals, add the local gauge-fixing functional

$$
S_{\rm gf}=\frac1{2c^2}\sum_n
\left(\Delta_0^-A_0+c^2\sum_i\Delta_i^-A_i\right)^2.
$$

After the conventional midpoint phase alignment, this cancels the mixed
quadratic terms. With

$$
\widehat k_0=2\sin\frac{k_0}{2},
\qquad
\widehat{\mathbf k}^{,2}=\sum_i4\sin^2\frac{k_i}{2},
$$

the free photon propagator is

$$
D_{00}(k)=\frac{c^2}{\widehat k_0^2+c^2\widehat{\mathbf k}^{,2}},
\qquad
D_{ij}(k)=\frac{\delta_{ij}}
{\widehat k_0^2+c^2\widehat{\mathbf k}^{,2}},
\qquad
D_{0i}=0.
$$

The single global gauge zero mode is removed in finite volume. Gauge fixing
does not alter either Ward identity.

---

## 6. What is now calculable

The frozen pair supplies every ingredient of the one-loop full-zone
calculation:

1. exact fermion propagator `S=D_W^{-1}`;
2. exact photon propagator in §5;
3. the one-photon vertex in §3;
4. the two-photon contact vertex in §4;
5. compact integration domain `[-pi,pi]^4`.

Schematically, the fermion self-energy is the sum of exchange and seagull
terms,

$$
\Sigma(p)=g^2\int_{\rm BZ}\!V^{(1)}S(p-k)V^{(1)}D(k)
+\frac12\int_{\rm BZ}\!V^{(2)}D(k),
$$

while the photon polarization contains the fermion bubble and the matching
two-photon contact. Differentiation at zero external momentum defines
`delta Z_t`, `delta Z_s`, `delta Z_E`, and `delta Z_B`. The required threshold
is still

$$
\delta_{\rm match}
=(\delta Z_s-\delta Z_t)
-\frac12(\delta Z_B-\delta Z_E).
$$

FTD-0418 itself does **not** evaluate these integrals. Its successor FTD-0419
now evaluates one complete `xi=1` QED_L-like finite-volume step scheme with
`g²` factored out and finds `delta_match/g²=-0.32696906(5)`. Automatic
cancellation is therefore closed negative and a dimension-four counterterm is
required in that scheme. The coefficient is off-shell and scheme-specific;
gauge-independent on-shell matching remains open.

---

## 7. What remains ontologically open

The Wilson Noether current is exactly conserved on solutions of this selected
spinor action. That does not provide the missing map

$$
(s^t,J^t,s^{t+1},J^{t+1})\longmapsto j_\mu^{\rm ternary}.
$$

Nor does a Euclidean Wilson action make the current irreversible FTD update
invertible. The local gauge-matter regulator can be used to test radiative
cone matching, but it remains an imported Branch-B effective sector unless a
separate bridge derives its fields and current from the substrate history.

---

## 8. Status table

| Claim | Status |
|---|---|
| nearest-neighbour spacetime Wilson regulator | `[SELECTED BRANCH-B REGULATOR]` |
| exact matter gauge covariance | `[THEOREM — selected action]` |
| exactly one massless Brillouin corner; 15 lifted | `[THEOREM — selected action]` |
| leading matter/photon cone `c^2=1/7` | `[DERIVED given inherited selection]` |
| quartic matter/photon mismatch | `[DERIVED — selected action]` |
| exact one-photon Ward identity | `[THEOREM — selected action]` |
| exact two-photon Ward identity and seagull | `[THEOREM — selected action]` |
| local gauge-fixed propagator | `[DERIVED — selected gauge fixing]` |
| fundamental real-time unitarity | `[OPEN]` |
| conserved ternary-history current | `[OPEN]` |
| full-BZ step-scheme `delta_match` | `[NUMERICAL FACT — FTD-0419, scheme-specific]` |
| gauge-independent on-shell `delta_match` | `[OPEN — HARD]` |

The immediate defect is repaired: the gauge and matter sectors now share one
discrete spacetime regulator and one action-generated Ward hierarchy. Lorentz
recovery itself is not repaired until the full-zone coefficient is calculated
and compared with a declared phenomenological bound.
