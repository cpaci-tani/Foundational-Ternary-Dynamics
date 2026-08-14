# FTD-0931 — Preregistration: native retarded static-halo radiative-formation boundary v1

**Identifier:** `FTD-0931`  
**Date locked:** 2026-08-11  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE CERTIFICATE]`  
**Scope:** fixed compact source in the frozen native C18 kick-drift field
sector; exact positive source-centered tick invariant; causal retarded step
response; uncontained local convergence to the static Green profile;
three-dimensional infrared threshold; finite-grounded recurrence and Cesaro
control; no numerical search, fit, engine mutation, damping, ontology
adoption, moving-source recoil, target-profile, `G*`, Born, Bell, context,
outcome, or hiding read

## 1. Question

FTD-0930 proves that a fresh complete port can make one local static-field
relaxation step canonical and positive, but repeated coordinate relaxation
requires an indefinite blank-port ecology. The frozen native field already
has a different complete pair: flux `J` and canonical momentum `W=wave_vel`.

Can this existing pair form a static halo without damping or fresh-port
creation by retaining the source mismatch as an outward-propagating retarded
wave? The certificate must distinguish:

1. exact global positive-energy recursion;
2. convergence of the actual instantaneous field versus only its time
   average;
3. finite grounded recurrence versus uncontained local dispersal;
4. fixed-source formation versus moving-source work and recoil; and
5. mathematical existing-type capacity versus enabled production behavior.

## 2. Frozen sources

| Source | SHA-256 |
|---|---|
| `PREREG_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md` | `D4BD884513A39EA42F1DB216D2E359A83126BB49195457663A1AE0D2B336B54A` |
| `THEOREM_EIGHT_COLOR_SOURCE_CENTERED_POSITIVE_PORT_RELAXATION_AND_MASSLESS_HALO_BOUNDARY_v1.md` | `EA70B9D7B16481B005F0FBF5DFF25893A27606A1186661677A7A944F1E301D09` |
| `proof_eight_color_source_centered_positive_port_relaxation_massless_halo_boundary.py` | `A7E338090EC10B141DC3E1336926E8B980DE348250DE0C48005498756240971E` |
| `THEOREM_NATIVE_FIELD_DISCRETE_ACTION.md` | `2CB4B2D49DED01D9B642416D3C20B89C41F5682FC52896446BEBFB3D1CA8B63C` |
| `proof_native_field_discrete_action.py` | `2E4B98A17B43BA6E765334841E9F673E548B11AD48B0589ACD998FF2C1458E12` |
| `THEOREM_NATIVE_MOVING_SOURCE_POLE_CORRECTION.md` | `D6AE447F82479E5FDC6CB2C14F67AB82F7B6E203DA97FEAA121316B750D414E4` |
| `DERIV_RETARDED_GREEN_LATTICE.md` | `30FFF6B420D9D125F698C68763228813FAF7F629457F321E685D2C0902CCD07F` |
| `engine/src/render_bridge_phases/phase_read.cpp` | `D9B521C1DE6503987E5DB3D91A8B4F2DFE52289E527352A8011C4146C71FB8A8` |
| `engine/src/render_bridge_phases/phase_write.cpp` | `2C519C4EF52614E383C4494CBE1F26A7CE33036A0924EBEFF80778021FCB57A4` |
| `engine/include/ftd/field_operators.h` | `25866EFC8474A2AEF7443C5DA67CBF79BEB352DE3E342A97D1EF0C3C16439E48` |
| `engine/include/ftd/voxel.h` | `8621F0A7ADB70F24FC63F99071C8CD63396ADB4B04461A3ABD775D13D2D1E1A3` |

The certificate fails closed on source drift.

## 3. Frozen native field map

Use the production-normalized C18 stiffness

\[
 \kappa(k)={4\over3}-{2\over9}
 (u+v+w+uv+uw+vw),                                      \tag{1}
\]

where `u=cos(k_x)`, `v=cos(k_y)`, and `w=cos(k_z)`. Its exact band is

\[
 0\le\kappa\le{16\over9}.                               \tag{2}
\]

For a fixed compact source `f`, freeze the undamped unit-tick affine map

\[
 W_{n+1}=W_n-KJ_n+f,
 \qquad
 J_{n+1}=J_n+W_{n+1},                                    \tag{3}
\]

with `J_{-1}=J_0=0` and `W_0=0`. This is the prescribed-source sector of
FTD-0574. The source remains fixed after switch-on. No claim about the
formation, motion, or reciprocal recoil of that source is permitted.

On a finite grounded positive compression, or in the uncontained
finite-energy Green class defined below, let

\[
 J_*=K^{-1}f,
 \qquad e_n=J_n-J_*.                                      \tag{4}
\]

Then equation (3) becomes the source-free native map

\[
 \binom{e_{n+1}}{W_{n+1}}
 =\begin{pmatrix}I-K&I\\-K&I\end{pmatrix}
 \binom{e_n}{W_n}.                                        \tag{5}
\]

## 4. Frozen positive radiative invariant

Register

\[
 \boxed{
 H_{\rm rad}(e,W)
 ={1\over2}\langle W,W\rangle
 +{1\over2}\langle e,Ke\rangle
 -{1\over2}\langle W,Ke\rangle.}                         \tag{6}
\]

For one mode `a=κ(k)`, the metric is

\[
 G_a=\begin{pmatrix}a&-a/2\\-a/2&1\end{pmatrix},
 \qquad
 \det G_a=a(1-a/4).                                       \tag{7}
\]

The certificate must verify exact invariance under equation (5), positivity
for every nonzero C18 mode, and the factorization

\[
 H_{\rm rad}
 ={1\over2}\|W-Ke/2\|^2
 +{1\over2}\langle e,K(I-K/4)e\rangle.                   \tag{8}
\]

The source switch stores the exact formation debit

\[
 E_{\rm form}=H_{\rm rad}(-J_*,0)
 ={1\over2}\langle f,K^{-1}f\rangle.                     \tag{9}
\]

This is a positive source-centered account. The source that pays equation
(9) is not derived by this certificate.

## 5. Frozen exact retarded response

Define the discrete frequency by

\[
 \cos\omega(k)=1-\kappa(k)/2,
 \qquad
 0\le\omega\le2\arcsin(2/3).                             \tag{10}
\]

The exact response of equation (3) must be

\[
 \boxed{
 J_n(x)=J_*(x)-
 \int_{\mathbb T^3}{d^3k\over(2\pi)^3}
 e^{ik\cdot x}{\widehat f(k)\over\kappa(k)}
 {\cos[(n+1/2)\omega(k)]\over\cos[\omega(k)/2]}.}       \tag{11}
\]

The certificate must verify the modal recurrence, initial values, production
pole

\[
 z^2-(2-\kappa)z+1,                                      \tag{12}
\]

and bound

\[
 \cos(\omega/2)=\sqrt{1-\kappa/4}\ge{\sqrt5\over3}.     \tag{13}
\]

Equation (3) is radius-one causal. Starting from a compact source and zero
field, every finite-tick field is finitely supported inside its C18 causal
cone. Equation (11) is a decomposition of that causal solution, not an
instantaneous completed halo read.

## 6. Frozen three-dimensional local-convergence theorem

For a nonzero-total compact source, `fhat(0) != 0`. Near the unique zero of
the stiffness,

\[
 \kappa(k)={|k|^2\over3}+O(|k|^4).                        \tag{14}
\]

Hence the static and transient amplitudes have infrared measure

\[
 {d^dk\over\kappa(k)}\sim r^{d-3}dr.                     \tag{15}
\]

Equation (15) is locally integrable exactly when `d>2`. Thus three is the
minimum spatial dimension in which a generic compact monopole source has a
finite local static Green profile and finite formation energy in this class.
This is a minimum-dimension threshold, not a uniqueness result against all
`d>3`.

For `d=3`, the amplitude

\[
 g_x(k)=e^{ik\cdot x}{\widehat f(k)\over
 \kappa(k)\cos[\omega(k)/2]}                              \tag{16}
\]

is in `L1(T^3)`. The phase `omega` is Lipschitz and nonconstant, and its
critical set has measure zero. By the standard coarea formula, the pushforward
of `g_x d^3k` under `omega` has an `L1` density. The
Riemann--Lebesgue lemma then requires

\[
 \boxed{
 J_n(x)\longrightarrow J_*(x),
 \qquad W_n(x)\longrightarrow0}                           \tag{17}
\]

for every fixed site `x`.

The certificate must verify the exact C18 zero, Hessian, band, phase bounds,
nonconstant analytic derivative witness, radial integrability threshold, and
modal response. Coarea and Riemann--Lebesgue are named proof dependencies,
not numerical observations. No convergence rate is registered.

Because the global positive invariant remains fixed while equation (17)
removes the mismatch from every fixed finite region, the source-switch energy
is dispersed into the outward field history. It is not erased.

## 7. Frozen finite-region control

On every finite grounded region, `K` has discrete positive modes. Each
nonzero forced mode has unit-modulus roots `exp(+-i omega)`. Its nonzero
source-centered invariant forbids actual convergence to `(J_*,0)` from the
zero initial field. The finite field is recurrent/quasiperiodic, not an
attractor.

The Cesaro average nevertheless converges modewise:

\[
 {1\over N}\sum_{n=0}^{N-1}e^{in\omega}
 ={1-e^{iN\omega}\over N(1-e^{i\omega})}\longrightarrow0. \tag{18}
\]

The certificate must verify exact finite-grounded stability,
nonconvergence-by-positive-invariant, and equation (18). No region-independent
rate is allowed because the smallest frequency approaches zero on larger
regions.

This contrast is mandatory: local static formation in equation (17) is an
uncontained dispersal effect, not finite-box damping and not an `L to
infinity` extrapolation.

## 8. Registered outcomes

- **Outcome A — native radiative static formation / finite recurrence
  boundary:** equations (6)--(18) pass. The existing `(J,W)` pair gives an
  exact positive causal fixed-source recursion whose instantaneous field
  converges locally to the static Green profile on the uncontained
  three-dimensional scaffold; finite grounded systems remain recurrent and
  converge only in Cesaro average. Moving-source common action, recoil,
  tracking, recovery, production enablement, and `G*` remain open.
- **Outcome B — stable recursion / averaged formation only:** the exact
  invariant, stability, causality, and finite Cesaro controls pass, but the
  uncontained instantaneous local-convergence proof fails.
- **Outcome C — native radiative route fails:** the invariant, causal
  response, finite-energy Green class, or even averaged formation fails.
- **Invalid:** source drift, post-lock formula/tolerance change, numerical
  search or fitted decay, damping, target/profile/context/Born read,
  engine/CMake mutation, moving-source promotion, completed-infinity or
  `L to infinity` rhetoric, or failed combined gate.

## 9. Firewalls

No engine source, CMake target, `Voxel` field, toggle, default, production
law, ontology type, paper, physical constant, or phenomenological formula is
changed. The production hashes are inspected only to verify existing field
capacity and the frozen source-free/prescribed-source recurrence. Damping,
Langevin noise, genesis, evaporation, Gauss projection, forces, movement,
and all other tick phases remain outside the theorem.

Even Outcome A does not form the time-dependent FTD-0929 companion, derive a
moving ternary source or reciprocal recoil, identify a photon, recover
gravity, identify dark matter, supply a `G*` cadence, derive Born frequencies
or Bell correlations, hide a preferred tick, or establish framework
completeness.
