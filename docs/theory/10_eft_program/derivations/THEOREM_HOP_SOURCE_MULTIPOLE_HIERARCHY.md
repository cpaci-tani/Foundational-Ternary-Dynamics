# Theorem — Periodic-Hop Source Multipole Hierarchy

**Record:** FTD-0561
**Status:** [THEOREM — FINITE-SOURCE SLOW-HOP HIERARCHY] + [CLOSED NEGATIVE — CHARGED EXTENSION CURE]
**Scope:** rigid finite manifested profiles on the FTD-0560 axial `l=1`
resonance; frozen linear native coupling

## 1. Exact finite-source forcing

For a source translated along an axis, group its site polarities by axial
coordinate and write

\[
 S(u)=\sum_n s_n e^{iun},
\qquad
 M_r=\sum_n s_n n^r.
\]

At the unique FTD-0560 slow-hop resonance,

\[
 \frac{2\pi-u_T}{T}=\theta_a(u_T),
\qquad
 |c_1|=\sqrt3/T.
\]

The axial velocity-curl source is zero, while the gradient symbol has
magnitude `sin(u_T)`.  Therefore

\[
 \boxed{
 A_T=G_C\frac{\sqrt3}{T}\sin u_T\,|S(u_T)|}.
\]

This formula is exact for every finite rigid source and every `T>=3`.

## 2. Multipole theorem

Let `m` be the first index for which `M_m` is nonzero.  Since the source has
finite support,

\[
 S(u)=\sum_{r=0}^{\infty}\frac{(iu)^r}{r!}M_r
 =\frac{(iu)^m}{m!}M_m+O(u^{m+1}).
\]

FTD-0560 gives

\[
 u_T=\frac{2\pi\sqrt3}{T}+O(T^{-2}),
\qquad
 \sin u_T=u_T+O(u_T^3).
\]

Substitution yields

\[
 \boxed{
 A_T=
 G_C\frac{\sqrt3(2\pi\sqrt3)^{m+1}}{m!}
 \frac{|M_m|}{T^{m+2}}
 +O(T^{-(m+3)})}.
\]

The leakage order is therefore set by the first nonzero axial multipole.

## 3. Consequences

### 3.1 Charged sources

For net polarity `Q=M_0 != 0`,

\[
 \boxed{A_T=6\pi G_C|Q|/T^2+O(T^{-3})}.
\]

The leading term is independent of radius, shape, or internal arrangement.
Making a rigid charged carrier wider cannot remove the FTD-0560 channel.

A quantitative nonasymptotic version follows from

\[
 |S(u)-Q|\le |u|\sum_n|s_n n|.
\]

Because `u_T -> 0`, every fixed profile with `Q != 0` has nonzero form factor
at the slow-hop resonance for all sufficiently large finite `T`.

### 3.2 Neutral sources

If `Q=0` but `M_1 != 0`,

\[
 A_T=12\sqrt3\pi^2G_C|M_1|/T^3+O(T^{-4}).
\]

If both `Q=M_1=0` but `M_2 != 0`,

\[
 A_T=36\pi^3G_C|M_2|/T^4+O(T^{-5}).
\]

Neutrality and multipole balance suppress schedule radiation progressively;
they do not generically eliminate it.

## 4. Exact axial cancellation criterion

Let `a_n` be the total polarity in the plane at axial coordinate `n`.  Then

\[
 S(u)=\sum_n a_n e^{iun}
\]

is a finite Laurent polynomial.  If it vanishes on any open interval of `u`,
analytic continuation makes it identically zero, and uniqueness of Laurent
coefficients gives

\[
 \boxed{S(u)\equiv0\quad\Longleftrightarrow\quad a_n=0
 \text{ for every axial plane}.}
\]

Plane-by-plane neutrality is necessary and sufficient to cancel the entire
axial witness family.  It is not sufficient to cancel oblique resonances.
The registered transverse dipole has `S(u,0,0)=0` for all axial `u`, yet at
the FTD-0560 one-tick oblique root its magnitude is `0.141480289...`.

## 5. Boundary

The theorem closes rigid *charged extension* as a radiationless linear cure.
It identifies microscopic neutrality and moment balance as genuine suppression
mechanisms, not as complete carrier constructions.  A radiationless linear
profile must vanish on every two-dimensional resonant surface, not merely on
one axial family.  Internally deforming, nonlinear, defect-bound, and
topological carriers remain open.

**Subsequent closure:** FTD-0562 proves that no fixed nonzero finite rigid form
factor—neutral or charged—can vanish on the complete slow-hop surface for all
sufficiently large periods.  The surviving scope is therefore deforming,
nonlinear, period-growing, defect/topological, or self-consistent dynamics.
