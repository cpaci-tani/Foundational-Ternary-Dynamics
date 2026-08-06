# FTD-0700 — Axial lattice-Cherenkov exposure

**Status:** `[THEOREM — SOURCE-FREE FIELD KINEMATICS]` +
`[CONDITIONAL DYNAMICAL CONSEQUENCE]`  
**Production status:** unchanged

## 1. Premises

For an axis wavevector `(k,0,0)`, `0 <= k <= pi`, the selected matched
face/edge field has dispersion

\[
\Omega_{100}(k)
=2\arcsin\!\left(\frac{\sin(k/2)}{\sqrt3}\right).
\]

A rigid source translated along the same axis with speed `v` has Fourier
forcing phase `exp[-i k(x-vt)]`. Its temporal forcing frequency is therefore
`omega_src=kv`. A nonzero axial phase match satisfies

\[
\Omega_{100}(k)=kv.
\]

This section is kinematic. It does not assume that the selected connected
matter object is rigid, freely translating, or coupled with nonzero strength
to every field wavevector.

## 2. Exact phase-speed interval

Differentiation gives

\[
\Omega'_{100}(k)
=\frac{\cos(k/2)}{\sqrt{3-\sin^2(k/2)}},
\qquad
\Omega''_{100}(k)
=-\frac{\sin(k/2)}{[3-\sin^2(k/2)]^{3/2}}<0
\]

for `0<k<pi`. Hence `Omega_100` is strictly concave with
`Omega_100(0)=0`, so its phase speed `Omega_100(k)/k` is strictly decreasing.
Its endpoint values are

\[
c_{\rm IR}=\lim_{k\to0}\frac{\Omega_{100}(k)}k=\frac1{\sqrt3},
\]

\[
v_{\rm edge}
=\frac{\Omega_{100}(\pi)}\pi
=\frac{2\arcsin(1/\sqrt3)}\pi
=0.3918265520306073\ldots .
\]

Therefore every axial speed

\[
v_{\rm edge}<v<c_{\rm IR}
\]

has exactly one nonzero collinear phase-matched wavevector `k in (0,pi)`.
Thus remaining below the long-wavelength cone is not sufficient to avoid
phase matching with the dispersive ultraviolet field band.

For a purely axial convective current, the collinear match is longitudinal:
its transverse projection vanishes. Radiation exposure requires an oblique
wavevector. Define

\[
F_v(k_x,k_y)=v k_x-\Omega(k_x,k_y,0).
\]

At the collinear root `k_0`, every sufficiently small `k_y>0` gives
`F_v(k_0,k_y)<0`. Because `F_v(pi,0)=pi(v-v_edge)>0`, continuity leaves
`F_v(pi,k_y)>0` for sufficiently small `k_y`. The intermediate value theorem
therefore supplies an oblique root `k_x in (k_0,pi)`. An axial current has a
nonzero transverse projection at every such `k_y != 0`.

Thus every `v_edge < v < c_IR` is exposed to a transverse oblique field mode,
provided the source structure factor does not vanish there. The axial value
`v_edge` is the infimum of these thresholds; at finite volume, the smallest
allowed nonzero transverse wavevector raises the realized threshold.

## 3. Exact transverse witness at `v=1/2`

At

\[
v=\frac12,
\qquad
\mathbf k_*=\left(\pi,\frac\pi2,0\right),
\]

one has

\[
\Omega(\mathbf k_*)
=2\arcsin\sqrt{\frac{1+1/2}{3}}
=2\arcsin\frac1{\sqrt2}
=\frac\pi2
=v k_{*,x}.
\]

The corresponding group velocity is

\[
\mathbf v_g(\mathbf k_*)=\left(0,\frac13,0\right).
\]

For a convective current `J` parallel to the x axis, the lattice wavevector is
`khat=(2,sqrt(2),0)`. Its transverse projection has

\[
\frac{|J_T|^2}{|J|^2}=\frac13.
\]

The packet has zero axial group velocity while the source advances at `1/2`,
and it transports transversely at `1/3`. This supplies an exact kinematic route
to a trailing-and-sideways lattice-radiation morphology below `1/sqrt(3)`.
By contrast, the collinear identity `v=1/2`, `k_x=2pi/3` is a longitudinal
control and is not by itself a transverse-radiation witness.

## 4. Matter-dynamics consequence

The connected matter candidate is exposed to axial lattice-Cherenkov
radiation whenever all of the following hold:

1. its centre translates approximately steadily with
   `v_edge < v < 1/sqrt(3)`;
2. its current has nonzero transverse structure factor at an oblique
   phase-matched wavevector;
3. nonlinear dressing does not cancel the outgoing solution; and
4. the finite volume contains the phase-matched mode to the required
   linewidth.

This identifies four possible protection mechanisms for a persistent mobile
object: a speed below the relevant directional threshold, a symmetry or
structure-factor zero, nonlinear co-moving dressing that removes the outgoing
channel, or deformation/broadening that suppresses ultraviolet coupling.
Those are alternatives to adding a new primitive; none is yet demonstrated.

The body-diagonal identity `Omega(q,q,q)=q` supplies a collinear comparison,
but it does not determine the oblique threshold for a `<111>` moving source.
Directional radiation thresholds beyond the axial theorem remain open.

## 5. Claim boundary

This theorem proves a source-free dispersion intersection. It does not prove
that the existing mobile matter solution radiates, that its observed field is
a wake, that `v_edge` is a universal matter speed limit, or that a continuum
preferred frame survives in the infrared. The next campaign must translate a
complete dressed state across speeds bracketing `v_edge`, resolve its
spatiotemporal spectrum, and test whether outgoing power activates at the
predicted phase-matched wavevector without retuning the source shape.
