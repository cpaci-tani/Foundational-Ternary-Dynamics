# Theorem — Native Periodic-Hop Dressing Obstruction

**Record:** FTD-0560
**Status:** [THEOREM — SINGLE-SITE LINEAR DRESSING NO-GO] + [OPEN — GENERAL NONLINEAR CARRIER]
**Scope:** isolated production `FULL`-stencil field operator with native
`-G_C grad(s)+G_C curl(sv)` coupling; one polarity hopping one axial site
every finite `T` ticks

## 1. Native source cannot self-cancel

The central-difference Fourier symbol is

\[
 \mathbf q(\mathbf k)=(\sin k_x,\sin k_y,\sin k_z).
\]

For state transform `S(k)` and current transform `j(k)`, source inspection of
`phase_read.cpp` gives

\[
 \mathbf f=iG_C[-\mathbf q S+\mathbf q\times\mathbf j].
\]

For a rigid point source, `j=vS`.  The two terms are longitudinal and
transverse to `q`, respectively.  Hence

\[
 \boxed{
 |\mathbf f|^2=G_C^2|S|^2
 (|\mathbf q|^2+|\mathbf q\times\mathbf v|^2)}.
\]

There is no electric/magnetic destructive interference.  The source vanishes
only if the point-source Floquet coefficient vanishes or `q=0`.

## 2. Co-moving solvability condition

For a one-site hop `d` every `T` ticks, FTD-0558 gives

\[
 \Omega_l=(\mathbf k\cdot\mathbf d+2\pi l)/T,
\quad
 c_l=\frac{1-e^{i\mathbf k\cdot\mathbf d}}
 {T[1-e^{i(\mathbf k\cdot\mathbf d+2\pi l)/T}]}.
\]

An exactly Floquet-periodic co-moving field must solve

\[
 \mathbf J_l(\mathbf k)=
 \frac{iG_Cc_l[-\mathbf q+\mathbf q\times\mathbf v]}
 {D_l(\mathbf k)},
\]

\[
 D_l=C_{\rm WAVE}^2M(\mathbf k)-4\sin^2(\Omega_l/2).
\]

Thus cancellation of the numerator on every zero of `D_l` is a necessary
condition for a square-summable co-moving dressing.

## 3. Every finite hop period has a resonance

Along a coordinate axis, the production symbol reduces exactly to

\[
 M(u,0,0)=4\sin^2(u/2),
\qquad
 \theta_a(u)=2\arcsin(\sin(u/2)/\sqrt3).
\]

### 3.1 One-tick hops

Set `k_parallel=0.1`.  At zero transverse momentum,
`theta_a(0.1)<0.1` exactly because division by `sqrt(3)` decreases the
argument of `asin`.  At transverse momentum `0.2`, direct alternating-series
interval bounds give

\[
 M(0.1,0.2,0)>0.049,
\qquad
 12\sin^2(0.05)<0.030.
\]

Therefore `theta(0.1,0.2,0)>0.1`.  Continuity supplies an oblique principal
resonance between the two endpoints.  Its transverse derivative is positive,
so the root is regular.

### 3.2 Two-tick hops

`theta_a` is strictly concave on `(0,pi)`, so `theta_a(u)/u` is strictly
decreasing.  The unique principal-harmonic solution is

\[
 u_2=2\pi/3,
\]

because `sin(u_2/2)/sqrt(3)=1/2` and therefore
`theta_a(u_2)=pi/3=u_2/2`.

### 3.3 Every `T>=3`

Use the `l=1` harmonic and write axial momentum as `k=-u`.  Its resonance
equation is

\[
 \frac{2\pi-u}{T}=\theta_a(u),\qquad0<u<\pi.
\]

The function `T theta_a(u)+u` is strictly increasing, starts at zero, and at
`u=pi` exceeds `2pi` for every `T>=3`, since
`theta_a(pi)=2asin(1/sqrt(3))>pi/3`.  Hence exactly one root exists.  Its
derivative is strictly positive, so it is regular.

This proves a native field resonance for every finite integer hop period.
The slow mean speed for large `T` does not remove the schedule harmonic.

## 4. The resonant source is always nonzero

For every axial root, `0<u<pi`, so `sin u` is nonzero.  The Floquet coefficient
satisfies

\[
 |c_l|=
 \frac{|\sin(u/2)|}{T|\sin(\Omega_l/2)|}.
\]

On resonance,
`sin(theta_a/2)=sin(u/2)/sqrt(3)`, giving the exact identity

\[
 \boxed{|c_l|=\sqrt3/T}.
\]

Because the axial curl source vanishes while the longitudinal gradient does
not,

\[
 \boxed{|f_l|=G_C\frac{\sqrt3}{T}\sin u_T>0}.
\]

The oblique `T=1` source is also nonzero; its longitudinal and transverse
pieces add in norm rather than cancel.

## 5. No square-summable co-moving point dressing

Near any regular resonance point choose a normal coordinate `n`.  Then

\[
 D_l=\lambda n+O(n^2),\qquad\lambda\ne0,
\]

while the numerator is continuous and nonzero.  Consequently

\[
 |J_l|^2\ge C/n^2
\]

in a sufficiently small patch.  Integrating over the normal direction
diverges.  Since the modal energy contains the positive term
`a(k)|J_l|^2` and `a(k)>0` at the registered root, the energy diverges as
well.

Therefore the infinite-lattice linear native field admits no finite-energy,
square-summable, exactly co-moving dressing for a periodically hopping point
polarity.

## 6. Slow-hop asymptotic

For `T>=3`, the axial phase has expansion

\[
 \theta_a(u)=u/\sqrt3-u^3/(36\sqrt3)+O(u^5).
\]

Solving the resonance equation gives

\[
 u_T=\frac{2\pi\sqrt3}{T+\sqrt3}+O(T^{-3}).
\]

Therefore

\[
 \boxed{|f_l|=6\pi G_C/T^2+O(T^{-3})}.
\]

Discrete stutter radiation becomes parametrically weak for slow hops, but is
not identically absent at any finite period.

## 7. Boundary

The theorem closes only a single-site point carrier with an exactly periodic
hop schedule in the frozen linear native coupling sector.  It does not rule
out an extended source whose form factor vanishes on every resonant surface,
an internally deforming neutral composite, nonlinear spectral detuning,
defect/topological binding, or a carrier whose field and core solve a different
self-consistent periodic orbit.

Finite periodic boxes need not contain a grid point exactly on the continuum
resonance surface.  The theorem is an infinite-lattice/continuum-BZ
square-summability statement, not a claim of exact secular growth at every
finite `L`.
