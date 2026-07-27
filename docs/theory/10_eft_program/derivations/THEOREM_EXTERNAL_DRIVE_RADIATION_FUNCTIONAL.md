# Theorem — Exact External-Drive Radiation Functional

**Record:** FTD-0559
**Status:** [THEOREM — PRODUCTION FIELD OPERATOR] + [SELECTED EXTERNAL SOURCE]
**Scope:** isolated production `FULL`-stencil field sector; prescribed drive,
not self-consistent matter

## 1. Exact work theorem

For a nonzero Fourier mode let

\[
 a=C_{\rm WAVE}^2M(\mathbf k),\qquad 0<a<4,
\]

and define

\[
 H_a(J,W)=|W|^2+a|J|^2-a\operatorname{Re}(J^*W).
\]

This is positive definite because its Hermitian matrix has determinant
`a(1-a/4)>0`.  It is invariant under the unforced production step

\[
 W^\circ=W-aJ,\qquad J^\circ=J+W^\circ.
\]

Apply a complex modal kick `f` before the drift endpoint, so
`(J',W')=(J°+f,W°+f)`.  Since `H_a` is quadratic, its exact discrete gradient
between the unforced and forced endpoints gives

\[
 H_a(J',W')-H_a(J,W)
 =\operatorname{Re}\left[
 f^*\left(a\bar J+(2-a)\bar W\right)\right],
\]

where bars are arithmetic midpoints between `(J°,W°)` and `(J',W')`.
Therefore the cumulative external work equals the exact final field-energy
gain at every finite tick.

## 2. Exact retarded response

Using `W_t=J_t-J_{t-1}`, the forced map is

\[
 J_{t+1}-(2-a)J_t+J_{t-1}=f_t.
\]

Writing `2-a=2cos(theta)` gives the causal impulse response

\[
 G_m=\frac{\sin(m\theta)}{\sin\theta},\qquad m\ge1.
\]

For a drive `f_t=F exp(-i Omega t)` acting for `N` ticks,

\[
 J_N=F e^{-i\Omega N}
 \frac{S_N(\Omega+\theta)-S_N(\Omega-\theta)}
 {2i\sin\theta},
\]

where `S_N(x)=sum_{r=1}^N exp(ixr)`, and

\[
 W_N=J_N-J_{N-1}.
\]

This is the finite-time retarded solution; it requires no principal-value
deletion of the on-shell term.

## 3. Finite-volume resonance theorem

If `Omega` is separated modulo `2pi` from `±theta`, both geometric sums are
uniformly bounded.  Hence `J_N`, `W_N`, and `H_N` remain bounded as
`N -> infinity`.

At `Omega=theta`,

\[
 S_N(\Omega-\theta)=N
\]

while `S_N(2theta)` is bounded away from the zero/Nyquist degeneracies.  The
resonant state therefore grows linearly and direct substitution into `H_a`
gives

\[
 \boxed{\lim_{N\to\infty}\frac{H_N}{N^2}=\frac{|F|^2}{2}}.
\]

Consequently a finite lossless lattice has no generic finite constant
radiation power.  Off resonance, energy is bounded; at an exactly resonant
discrete mode, coherent energy is quadratic.  Constant power requires a
continuum/large-time limiting prescription or damping.

## 4. Distributional radiation functional

The Fejer identity

\[
 \frac{|S_N(x)|^2}{N}\Longrightarrow2\pi\delta_{2\pi}(x)
\]

turns the finite-time field energy into the large-time continuum functional

\[
 \boxed{
 P_{\rm ext}=\pi\int_{BZ}\frac{d^3k}{(2\pi)^3}|F(\mathbf k)|^2
 \sum_{\sigma=\pm1}
 \delta_{2\pi}(\Omega(\mathbf k)-\sigma\theta(\mathbf k))}.
\]

The two terms are the two symplectic field branches.  Cross terms vanish
distributionally when the branches are nondegenerate.

For a smooth translating external source, `Omega=k.v`.  Applying the coarea
formula gives the resonance-surface measure

\[
 P_{\rm ext}=\frac{\pi}{(2\pi)^3}
 \sum_{\sigma,m}\int_{\Sigma_{\sigma,m}}
 \frac{|F(\mathbf k)|^2}
 {|\mathbf v-\sigma\mathbf v_g(\mathbf k)|}\,d\Sigma,
\]

where `Sigma_{sigma,m}` satisfies `k.v-sigma theta=2pi m` and
`v_g=grad_k theta`.  The correct Jacobian is the relative group velocity, not
an unspecified lattice-mode factor.

FTD-0558 proves `theta/|k| >= 2/(3pi)`.  Therefore this smooth-drive
functional vanishes for `|v|<2/(3pi)` on the principal temporal branch.

## 5. Integer-hop corollary

For a prescribed hop `d` every `T` ticks, insert

\[
 \Omega_l=(\mathbf k\cdot d+2\pi l)/T,
 \qquad F_l=F c_l.
\]

The rate is the sum of the branch functionals weighted by `|c_l|^2`; its
coarea denominator is

\[
 |\mathbf d/T-\sigma\mathbf v_g|.
\]

The nonfundamental terms describe the acceleration/stutter content of the hop
schedule.  They are absent from the smooth-source theorem.

## 6. Boundary

`P_ext` is energy delivered by a prescribed external drive to the exact native
field operator.  It is not yet matter energy loss.  A physical radiation
theorem additionally requires a native mobile carrier, its conserved current,
the coupling normalization, and equal-and-opposite reciprocal work from the
same matter–field transaction.
