# FTD-0556 — Integer Translation and Bloch Transport

**Status:** [THEOREM — TYPE BOUNDARY] + [THEOREM — ISOLATED FREE-FLUX MAP] + [NUMERICAL FACT — CPU PRODUCTION REPLAY]  
**Date:** 2026-07-26  
**Depends on:** the frozen production `FULL` stencil and unit-tick default kick-drift map  
**Does not establish:** a stable manifested matter pole, a photon identification, a common matter/field cone, or microscopic Lorentz invariance

## 1. Result

Integer-only local lattice updates do not force observable motion to occur in
integer jumps.  They force the microscopic support and exact translation group
to be integer-valued.  An extended field packet can nevertheless have a
continuously moving centroid because different integer-lattice Fourier modes
accumulate different continuous phases.

The escape has a type cost.  A scalar, first-order, finite-range, exactly
norm-preserving update cannot carry a nontrivial dispersive band.  The frozen
free-flux sector escapes because each polarization has the two-component state

\[
 (J,W)=(\texttt{flux},\texttt{wave\_vel}).
\]

This pair is symplectic and preserves a positive mode energy.  Primitive
ternary manifestation `s` by itself has not been shown to possess the required
fiber.

## 2. Scalar finite-range theorem

Consider a homogeneous scalar first-order update on `l2(Z)` with finite range,

\[
 q'_n=\sum_{r=a}^{b}c_rq_{n-r}.
\]

Its Bloch symbol is the Laurent polynomial

\[
 p(z)=\sum_{r=a}^{b}c_rz^r,
 \qquad z=e^{ik}.
\]

Exact scalar norm preservation requires `|p(e^{ik})|=1` for every `k`.  The
Fourier coefficients of `|p|^2` are the autocorrelations of the coefficient
sequence.  At the largest nonzero lag `b-a`, the coefficient is

\[
 c_b\overline{c_a}=0.
\]

If `a` and `b` label the extreme nonzero coefficients this is impossible unless
`a=b`.  Hence

\[
 p(z)=e^{i\phi}z^m.
\]

The update is only an integer shift times a phase.  Therefore a nontrivial
finite-range dispersive band requires an internal fiber, temporal memory,
nonlocality, or loss of exact norm preservation/invertibility.  This theorem
does not forbid local waves; it identifies the minimum type they need.

## 3. Exact frozen FTD transfer map

For the normalized 18-point stencil,

\[
 \nabla^2 e^{i\mathbf k\cdot\mathbf n}
 =-M(\mathbf k)e^{i\mathbf k\cdot\mathbf n},
\]

where

\[
 M(\mathbf k)=4-\frac23(c_x+c_y+c_z)
 -\frac23(c_xc_y+c_xc_z+c_yc_z),
 \qquad c_i=\cos k_i.
\]

Write `a=C_WAVE^2 M`.  With every other toggle disabled, the production
`phase_read` and default `phase_write` perform

\[
 W_{t+1}=W_t-aJ_t,
 \qquad
 J_{t+1}=J_t+W_{t+1}.
\]

Thus

\[
 U(\mathbf k)=
 \begin{pmatrix}1-a&1\\-a&1\end{pmatrix},
 \quad
 \det U=1,
 \quad
 \operatorname{tr}U=2-a.
\]

The characteristic equation is

\[
 \lambda^2-(2-a)\lambda+1=0.
\]

For `0<a<4`, the roots are the conjugate unit-circle pair
`lambda=exp(+-i theta)` with

\[
 \cos\theta=1-\frac a2,
 \qquad
 \boxed{\theta(\mathbf k)=2\arcsin
 \left(\frac{C_{\rm WAVE}\sqrt{M(\mathbf k)}}2\right)}.
\]

This is the exact discrete-time pole.  Replacing `theta` by
`C_WAVE sqrt(M)` is only its infrared approximation.

## 4. Exact positive invariant

For a complex analytic mode define

\[
 I_{\mathbf k}(J,W)=|W|^2+a|J|^2
 -a\operatorname{Re}(J^*W).
\]

Direct substitution of the kick-drift map gives

\[
 I_{\mathbf k}(J_{t+1},W_{t+1})
 =I_{\mathbf k}(J_t,W_t).
\]

Its Hermitian matrix is

\[
 G_a=\begin{pmatrix}a&-a/2\\-a/2&1\end{pmatrix},
 \qquad
 \det G_a=a\left(1-\frac a4\right).
\]

It is positive definite precisely for `0<a<4`.  The raw `(J,W)` Euclidean
norm is not the conserved quantity; the map is symplectic and unitary only in
the mode-dependent positive metric `G_a`.  Calling the production map
“unitary” without this qualification is incorrect.

## 5. Continuous centroid from integer dynamics

Choose one eigenbranch and form the complex packet

\[
 J_n(t)=\frac1L\sum_m A_m
 e^{ik_mn-i\theta_mt}.
\]

Every spatial operation used to evolve it connects integer lattice sites.  The
periodic first-harmonic moment is nevertheless

\[
 Z(t)=\sum_n|J_n(t)|^2e^{2\pi in/L}
 =\frac1L\sum_m A_m(t)\overline{A_{m+1}(t)},
\]

so

\[
 \Delta X_c(t)=\frac{L}{2\pi}
 \arg\frac{Z(t)}{Z(0)}
\]

is a continuous real number.  No site contains a fractional primitive state;
the fraction belongs to an observer of an extended interference pattern.

In the narrow-band, large-volume limit,

\[
 \frac{dX_i}{dt}=\left\langle\partial_{k_i}\theta\right\rangle,
\]

with

\[
 \partial_{k_i}\theta
 =\frac{C_{\rm WAVE}^2\partial_{k_i}M}{2\sin\theta},
 \qquad
 \partial_{k_i}M=\frac23\sin k_i
 (1+\cos k_j+\cos k_k).
\]

The signal front is a separate statement.  Since one tick reads at most the
18 face/edge neighbors, compact support expands by no more than one Moore shell
per tick even when the centroid displacement is noninteger.

## 6. Cubic infrared structure

Let

\[
 r^2=\sum_i k_i^2,\quad
 S_6=\sum_i k_i^6,\quad
 T_{42}=\sum_{i\ne j}k_i^4k_j^2.
\]

Expansion of the exact symbol gives

\[
 \boxed{M(\mathbf k)=r^2-\frac{r^4}{12}
 +\frac{S_6+5T_{42}}{360}+O(k^8)}.
\]

The complete fourth-order term is isotropic.  Cubic anisotropy first appears
in `M` at sixth order, hence in relative propagation observables at fourth
order in momentum.  Expanding the pole itself gives

\[
 \theta=C_{\rm WAVE}r\left[
 1+\frac{C_{\rm WAVE}^2-1}{24}r^2+O(k^4)
 \right].
\]

For the selected `C_WAVE^2=1/3`, the isotropic cubic correction is
`-C_WAVE r^3/36`.  The selected speed is not derived by this theorem and is not
the stability saturation of the `FULL` stencil.

## 7. Locked CPU replay

The corrected CPU run of
`test_integer_bloch_transport` returned:

| Diagnostic | Result |
|---|---:|
| CPU backend checks | `13/13` |
| mode arms | `9` |
| production mode-tick replays | `576` |
| maximum mode identity residual | `2.483e-16` |
| maximum production-mode residual | `2.253e-17` |
| maximum group speed | `0.570741` cells/tick |
| compact support outside the causal shell | exactly `0` |
| integer-translation covariance residual | `2.776e-17` |
| packet/Bloch replay residual | `1.515e-16` |
| centroid displacement residual | `9.326e-15` |
| one-tick centroid displacement | `0.565426363216128` cells |
| distance from the nearest integer | `0.434574` cells |

The first executable invocation auto-selected CUDA and was discarded as
`PROTOCOL_IMPLEMENTATION_INVALID`; it carried no verdict.  The only correction
was an explicit `force_cpu()` plus backend assertions.  No physics formula,
sample, or threshold changed.

The locked verdict is

`INTEGER_TRANSLATION_SUPPORTS_CONTINUOUS_BLOCH_CENTROID_FLUX_ONLY`.

## 8. Ontological boundary

What is now solid:

- strict lattice locality and continuous centroid motion are compatible;
- fractional microscopic site position is unnecessary for a free extended
  flux wave;
- an internal two-component phase-space fiber is necessary for the exact local
  reversible dispersive escape used here;
- the native free-flux pair supplies that fiber.

What remains unproved:

- a localized free packet is not a stable particle; dispersion generally
  broadens it;
- `s` has not been derived as a bound excitation of the `(J,W)` band;
- no rest gap, charge, common cone, scattering state, or reciprocal
  matter-field transaction follows from this result;
- identifying this band with light is still a selection unless operational
  clock/rod and signal tests close.

The next admissible gate is therefore a stable localized spectral carrier:
either the frozen nonlinear dynamics binds a finite-energy `(s,J,W)` composite,
or the manifested-matter claim remains open.  Adding an imposed mass clock or
calling the free packet a particle would not close that gate.

