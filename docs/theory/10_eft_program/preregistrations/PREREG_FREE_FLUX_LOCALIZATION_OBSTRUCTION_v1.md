# FTD-0557 — Free-Flux Localization Obstruction v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; POSITIVE FREE-FLUX OBSTRUCTION]  
**Date locked:** 2026-07-26  
**Scope:** analytic theorem plus observer-only CPU replay of the frozen isolated
`FULL`-stencil `(flux,wave_vel)` map  
**Production changes:** none

## 1. Question

FTD-0556 proved that the frozen integer-lattice wave map transports an
extended Bloch packet with a continuously moving centroid.  This protocol asks
whether the same *free* map can also supply a nonzero finite-energy localized
state that remains stationary or returns after finitely many ticks as an exact
integer translate.

The protocol does not search nonlinear parameter space.  It separates free
field transport from the still-open problem of a bound manifested carrier.

## 2. Frozen operator

Disable all toggles, then enable only `wave_propagation`, force the CPU backend,
use the periodic boundary, production `FULL` stencil, unit tick, single
substrate, and the default kick-drift update.  For one Fourier mode and one
Cartesian polarization,

\[
 U(\mathbf k)=
 \begin{pmatrix}1-a(\mathbf k)&1\\-a(\mathbf k)&1\end{pmatrix},
 \qquad a=C_{\rm WAVE}^2M(\mathbf k),
\]

with `M` and the positive-band phase `theta` exactly as registered by
FTD-0556.  No coupling, damping, reaction, Gauss, force, movement, imposed
clock, Floquet, or dual-substrate term may be enabled.

## 3. Analytic statements locked before execution

### 3.1 No square-summable point spectrum

For a putative eigenvalue `lambda`, a nonzero state in
`l2(Z^3;C^2)` would require its Fourier transform to be supported on

\[
 \det(U(\mathbf k)-\lambda I)=0.
\]

This determinant is a nonzero real-analytic function because
`tr U=2-a(k)` is nonconstant.  Its zero set has measure zero.  An `L2` Fourier
function supported there is zero almost everywhere.  Therefore the free map
has no nonzero square-summable eigenstate and no localized stationary mode.
Plane waves are generalized states, not exceptions.

### 3.2 No exact rigid translating square-summable packet

Suppose for integers `T>0` and `d in Z^3` that

\[
 \Psi_T(n)=e^{i\phi}\Psi_0(n-d).
\]

Then the Fourier transform is supported where

\[
 F_{T,d,\phi}(\mathbf k)=
 \det\!\left(U(\mathbf k)^T
 -e^{i\phi-i\mathbf k\cdot d}I\right)=0.
\]

`F` is real analytic and is not identically zero.  At `k=0` an identity would
force `e^{i phi}=1`; along any nonzero momentum direction orthogonal to `d`,
the translation phase remains one while the native band phase is nonzero.
Thus the zero set has measure zero and the only square-summable rigidly
translating state is zero.

### 3.3 Exact branch second-moment law

For one Bloch branch

\[
 \widehat\psi_t(\mathbf k)=
 e^{-it\theta(\mathbf k)}\widehat\psi_0(\mathbf k),
 \qquad \mathbf v_g=\nabla_{\mathbf k}\theta,
\]

the Fourier representation of position gives, componentwise,

\[
 \operatorname{Var}X_i(t)=\operatorname{Var}X_i(0)
 +2t\operatorname{Cov}_{\rm sym}(X_i,v_{g,i})
 +t^2\operatorname{Var}(v_{g,i}).
\]

For a real nonnegative unchirped spectral envelope the covariance term is
zero.  A nonzero localized packet has positive momentum-measure support, while
the native group velocity is nonconstant; hence its velocity variance is
positive and its width cannot remain fixed for all time.

These statements concern the isolated linear band.  They do not exclude a
nonlinear, defect-bound, topological, flat-band, or internally bound carrier.

## 4. Locked observer campaign

### 4.1 Analytic diagnostics

The observer must independently establish:

- nonconstant trace of `U`;
- absence of a flat native band;
- the measure-zero point-spectrum obstruction;
- the finite-time rigid-translation obstruction;
- the exact second-moment operator identity;
- positive group-velocity variance for every registered packet.

The registered directions are `<100>`, `<110>`, and `<111>`.  Use `L=65`,
positive mode numbers `1..24`, Gaussian mode envelope centered at `8` with
width `3`, and normalize the spectral amplitude to one.  The group-velocity
variance must exceed `1e-8` in every arm.

### 4.2 Frozen production replay

Encode the analytic branch in two real Cartesian flux components, translate
the packet phase coordinate to lattice coordinate `32`, and replay `16` ticks in an explicitly
forced CPU `RenderBridge`.  Require:

- exact configuration isolation and CPU backend checks;
- engine-versus-Bloch `(J,W)` residual at or below `1e-10`;
- direct-versus-spectral density-moment residual at or below `1e-10`;
- spectral norm residual at or below `1e-12`;
- no primitive manifested sites at any tick;
- at least one registered arm/tick with direct position variance exceeding
  its initial value by `1e-4`.

The ordinary position moment is used only while the packet remains more than
four initial RMS widths from the periodic seam.  If that geometric condition
fails, the protocol is invalid rather than re-centered after the run.

### 4.3 Frozen phase audit

Source inspection must record that:

- the reaction-free `phase_read/phase_write` field map is linear for fixed
  ternary state;
- with forces disabled, `phase_movement` advances manifestation from stored
  velocity and `remainder`, independently of the free field;
- with movement disabled, a manifested site is static and only sources the
  field through the one-way coupling term;
- the production force branches are separately selected/imposed and are not
  generated by the isolated free-band action tested here.

This audit licenses only the statement that the *frozen isolated free sector*
contains no binding mechanism.

### 4.4 Cardinalities

The run must report exactly 3 packet arms, 48 arm-tick replays, 3 CPU backend
checks, and 0 manifested sites.  A cardinality mismatch invalidates the run.

## 5. Verdicts

- `FREE_FLUX_TRANSPORT_HAS_NO_LOCALIZED_RIGID_CARRIER`: every analytic and
  replay gate passes.  The free `(J,W)` band transports and disperses extended
  field packets but supplies no nonzero localized stationary or rigidly
  translating `l2` carrier.
- `FREE_FLUX_LOCALIZATION_OBSTRUCTION_FAILED`: an analytic premise or locked
  numerical realization fails.  No localization conclusion advances.

Neither verdict closes the frozen nonlinear `(s,J,W)` carrier question.
Failure does not authorize a new force, mass clock, stencil, tolerance,
constant formula, or post-hoc envelope.
