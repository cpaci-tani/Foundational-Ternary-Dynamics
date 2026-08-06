# Theorem — Native Moving-Source Pole Correction

**Record:** FTD-0558
**Status:** [THEOREM — PRODUCTION FIELD OPERATOR]
**Scope:** frozen reaction-free `FULL`-stencil field map; selected additive
drive and periodic integer-hop histories

## Theorem 1 — exact driven pole

Let

\[
 U(\mathbf k)=\begin{pmatrix}1-a&1\\-a&1\end{pmatrix},
 \qquad a=C_{\rm WAVE}^2M(\mathbf k),
\]

and apply an additive kick `f_t` before the production drift, so the forcing
vector is `(1,1)^T`.  If `f_t=fz^t`, then

\[
 \det(zI-U)=z^2-(2-a)z+1
\]

and Cramer's rule yields `J=fz/det(zI-U)`.  On `z=e^{-i Omega}`,

\[
 \frac{\det(zI-U)}z=a-4\sin^2(\Omega/2).
\]

Therefore

\[
 J=\frac f{a-4\sin^2(\Omega/2)},
 \qquad \Omega=\pm2\arcsin(\sqrt a/2)\pmod{2\pi}.
\]

At zero frequency the static resolvent is `J=f/a`.  For the undamped
unit-modulus map this is the Abel/static-resolvent meaning of the integrated
retarded response; an ordinary convergent time sum is not asserted.

## Theorem 2 — positive smooth-source speed floor

The production symbol is the positive neighbor sum

\[
 M=\sum_r w_r(1-\cos(\mathbf k\cdot r)),\qquad w_r>0.
\]

Dropping all edge terms leaves

\[
 M\ge\frac23\sum_i(1-\cos k_i).
\]

For `|k_i|<=pi`, `1-cos k_i >= 2k_i^2/pi^2`, hence

\[
 M\ge\frac4{3\pi^2}|\mathbf k|^2.
\]

The free phase `theta=2 asin(C_WAVE sqrt(M)/2)` and `asin x>=x` give

\[
 \theta/|\mathbf k|\ge 2C_{\rm WAVE}/(\pi\sqrt3)=2/(3\pi).
\]

If a smooth rigid drive has `Omega=k.v`, then
`|Omega|<=|k||v|`; no resonance exists when `|v|<2/(3pi)`.

For comparison, the auxiliary seven-point symbol obeys
`|khat|/|k|>=2/pi`, independently refuting its former any-speed claim.

## Theorem 3 — periodic integer-hop spectrum

Let `x_{nT+r}=nd`, `0<=r<T`.  Its source factor satisfies

\[
 s_{t+T}(k)=e^{-ik\cdot d}s_t(k).
\]

Writing `s_t=e^{-i(k.d)t/T}p_r` gives
`p_r=e^{i(k.d)r/T}`.  The exact `T`-point Fourier expansion of `p_r` has

\[
 c_l=\frac{1-e^{ik\cdot d}}
 {T[1-e^{i(k\cdot d+2\pi l)/T}]},
 \quad
 \Omega_l=(k\cdot d+2\pi l)/T.
\]

Thus integer hopping is generically multi-harmonic.  This theorem is
kinematic and does not assert that any pole intersection carries a particular
radiated power.

## Corollary — the old threshold was an alias

On an `L`-site periodic axis, `n` and `n-L` are the same crystal mode.  The
old `L=16,n=15` computation used the periodic symbol of `n=-1` but divided by
the unwrapped `15*2pi/16`.  It therefore depressed the inferred phase speed by
exactly 15.  The locked observer confirms symbol and phase equality to
`2.22e-16` and `1.67e-16`.

## Boundary

These theorems do not identify the additive drive with native charge, do not
derive a force or recoil, and do not supply an energy-normalized radiation
rate.
