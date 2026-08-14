# Derivation — Critical Quartic Clock / CM Gearbox v1

**Identifier:** `FTD-0827`  
**Status:** `[CONDITIONAL THEOREM — WITHIN THE SELECTED CRITICAL QUARTIC
HAMILTONIAN]` + `[THEOREM — ORIENTATION-PRESERVING CONDUCTOR-32 CM MAP]` +
`[RESOLUTION — THE FTD-0826 ORIENTED RANK-TWO LIFT FOR THIS CLOCK]` +
`[OPEN — NATIVE CLOCK MAINTENANCE, PRIME-INDEXED PHYSICS, AND OPERATIONAL
TICK/FROBENIUS IDENTIFICATION]`  
**Date:** 2026-08-10  
**Production status:** unchanged; exact mathematics and documentation only

## 0. Result in one sentence

The dimensionless positive-energy shell of the selected critical quartic
clock is the genus-one curve `C:y^2=1-x^4`, and the exact rational map

\[
 \boxed{
 \Psi_-:C\longrightarrow E,
 \qquad
 u=\frac1{x^2},\qquad
 v=-\frac{y}{x^3},
 \qquad E:v^2=u^3-u}
\]

obeys

\[
 \boxed{\Psi_-^*\!\left(\frac{du}{2v}\right)=\frac{dx}{y}}.
\]

Thus the clock trajectory, its time differential, its integral homology, the
lemniscatic `G*` period, and the conductor-32 Hecke/Frobenius calendar belong
to one algebraic object. The opposite sign `v=+y/x^3` reverses the pulled-back
differential, so the clock's Hamiltonian orientation fixes precisely the
clockwise/counterclockwise choice that the BCC symmetric square cannot retain.

The locked verdict is:

```text
QUARTIC_CLOCK_CM_GEARBOX_CONDITIONAL_THEOREM
```

This closes the **mathematical gearbox conditional on the quartic clock**. It
does not derive the quartic law from P1--P5, maintain such a clock in the
production substrate, or turn increasing primes into successive physical
times.

## 1. Input firewall: what is and is not being derived

The sole dynamical input is the previously declared critical quartic
Hamiltonian

\[
 H(q,p)=\frac{p^2}{2m}+\lambda q^4,
 \qquad m>0,\quad\lambda>0.
 \tag{1}
\]

Its status is unchanged:

- the quartic form is forced only **given** the named reflection-symmetric
  threshold hypotheses and the limit `A -> 0` (FTD-0821);
- the current production substrate does not select or autonomously maintain
  that critical surface (FTD-0794/FTD-0824/FTD-0825); and
- finite-amplitude and controller-work debts remain open.

The result below therefore has the form

\[
 \text{selected/critical quartic clock}
 \quad\Longrightarrow\quad
 \text{canonical conductor-32 CM calendar}.
\]

It is not the converse and not a P1--P5 derivation of the clock.

## 2. The clock energy shell is a lemniscatic genus-one curve

Fix a positive orbit with turning amplitude `A`, so `E=lambda A^4`. Define

\[
 x=\frac qA,
 \qquad
 y=\frac{p}{\sqrt{2m\lambda}\,A^2}.
 \tag{2}
\]

Energy conservation becomes

\[
 \boxed{y^2=1-x^4.}
 \tag{3}
\]

The smooth projective completion of (3) has genus one. On its real clock
branch,

\[
 \dot x=A\sqrt{\frac{2\lambda}{m}}\,y,
 \qquad
 dt=\frac1A\sqrt{\frac{m}{2\lambda}}\frac{dx}{y}.
 \tag{4}
\]

Consequently the full clock cycle is the real oval of the algebraic
differential `dx/y`:

\[
 TA=\sqrt{\frac{m}{2\lambda}}
 \oint_{C(\mathbb R)}\frac{dx}{y}
 =4I_4\sqrt{\frac{m}{2\lambda}}
 =\sqrt\pi G^*\sqrt{\frac{m}{2\lambda}}.
 \tag{5}
\]

This is stronger than a shared special value. The clock supplies the curve
and the differential whose period is being evaluated.

## 3. The direct map to the fixed CM curve

On the function field of `C`, define

\[
 u=x^{-2},
 \qquad
 v=-yx^{-3}.
 \tag{6}
\]

Using only `y^2=1-x^4`,

\[
 v^2=\frac{1-x^4}{x^6}
 =\frac1{x^6}-\frac1{x^2}
 =u^3-u.
 \tag{7}
\]

Thus (6) maps to the exact fixed curve

\[
 E:v^2=u^3-u,
 \qquad \text{Cremona }32\mathrm{a}2.
\]

The apparent pole at `x=0` extends to the point at infinity on the smooth
projective curves. The nontrivial deck involution

\[
 (x,y)\longmapsto(-x,-y)
\]

fixes `(u,v)`, so the map is degree two.

Most importantly,

\[
 du=-2x^{-3}dx,
 \qquad 2v=-2yx^{-3},
\]

and therefore

\[
 \boxed{\frac{du}{2v}=\frac{dx}{y}.}
 \tag{8}
\]

The Neron differential of the arithmetic curve is exactly the clock-time
differential after dimensionless mechanical normalization. No fitted scale,
prime trace, `G*` value, or BCC square root enters the map.

## 4. Birational and isogeny factorization

The direct map factors through the rational model

\[
 E_1:Y^2=X^3+4X,
 \qquad \text{Cremona }32\mathrm{a}1,
\]

by

\[
 X=\frac{2(1+y)}{x^2},
 \qquad
 Y=-\frac{4(1+y)}{x^3},
 \tag{9}
\]

with inverse

\[
 x=-\frac{2X}{Y},
 \qquad
 y=\frac{X^2-4}{X^2+4}.
 \tag{10}
\]

Hence `C` is birational to `32a1`. The fixed degree-two isogeny

\[
 \phi:E_1\longrightarrow E
\]

is

\[
 u=\frac X4+\frac1X,
 \qquad
 v=\frac{Y(X^2-4)}{8X^2}.
 \tag{11}
\]

It satisfies

\[
 \phi^*\!\left(\frac{du}{2v}\right)=\frac{dX}{Y},
 \qquad
 \frac{dX}{Y}=\frac{dx}{y},
\]

and the composite of (9) and (11) reduces exactly to (6).

Both `32a1` and `32a2` have conductor `32`, `j=1728`, and the same global
L-function because they are rationally isogenous. Thus the clock curve does
not merely reproduce the archimedean period: it carries the same compatible
finite-prime Euler/Frobenius data proved for `32a2` at FTD-0826.

The degree-two factorization also explains the period normalization. The
clock's full real oval has

\[
 \oint_C\frac{dx}{y}=4I_4=2\varpi=\sqrt\pi G^*,
\]

while the least positive real period of `32a2` for `du/(2v)` is `varpi`.

## 5. The substrate-clock orientation fixes the missing sign

For the quartic Hamiltonian, Hamilton's equations give

\[
 \dot q=\frac pm,
 \qquad
 \dot p=-4\lambda q^3.
\]

The signed symplectic area swept by the forward flow is

\[
 \begin{aligned}
 \chi(q,p)
 &=\Omega\bigl((q,p),(\dot q,\dot p)\bigr)\\
 &=q\dot p-p\dot q\\
 &=-4\lambda q^4-\frac{p^2}{m}<0
 \end{aligned}
 \tag{12}
\]

on every positive-energy orbit. Reversing time flips the sign.

There are correspondingly two algebraic maps:

\[
 \Psi_\pm(x,y)=\left(x^{-2},\ \pm yx^{-3}\right),
\]

with

\[
 \Psi_-^*\omega_E=+\frac{dx}{y},
 \qquad
 \Psi_+^*\omega_E=-\frac{dx}{y}.
 \tag{13}
\]

The declared forward Hamiltonian flow therefore selects `Psi_-`. This is not
a coordinate-quadrant convention: it is the sign of the action-derived time
differential.

At an inert prime, the normalized rank-two Frobenius obeys `R_p^2=-I`. Its
two directions satisfy `R_p^{-1}=-R_p`, while

\[
 \operatorname{Sym}^2(R_p^{-1})
 =\operatorname{Sym}^2(R_p).
\]

Equation (13) supplies exactly this missing rank-two orientation. The BCC
result remains correct; the clock does not invert `Sym^2`. Instead, its own
genus-one energy shell supplies the oriented rank-two object before the
symmetric square is taken.

## 6. What “global arithmetic calendar” means after the map

Because the normalized clock curve is defined over `Q` and maps to `32a2`, it
inherits one compatible arithmetic system:

\[
 P_p(T)=T^2-a_pT+p
\]

at every good prime, the conductor-32 rule at `p=2`, the inert condition
`a_p=0` for `p=3 mod 4`, and the fixed primary Gaussian character at split
primes. The conductor-32 associate rule is therefore not appended after the
clock orientation is chosen; it is part of the rational isogeny class of the
clock's algebraic energy shell.

This does **not** make prime magnitude a time coordinate. The roles remain:

| symbol | role |
|---|---|
| `n` | ontic global update order |
| clock phase / real oval | local operational time |
| `p` | arithmetic place/context |
| `F_p^r` | `r` iterations of the local Frobenius operator |

A possible common-exponent reading `n -> (F_p^n)_p` remains a `[SELECTION]`
until a physical substrate construction realizes the prime-indexed channels.
Ordering primes by size is an arithmetic scan, not a clock trajectory.

## 7. Exact scope relative to FTD-0826

FTD-0826 remains unchanged on its tested claim:

- production C18 and equal-Moore C26 do not supply the CM local system;
- BCC supplies only the twist-blind `Sym^2` period and has zero production
  weight; and
- the free modal carrier is not a maintained local clock.

This result resolves a narrower open type from that row:

\[
 \text{oriented rank-two lift for the selected quartic clock}
 \quad\textbf{is now explicit}.
\]

It does not identify the production C18 modal carrier with `H^1(E)`. The
gearbox belongs to the selected quartic energy shell, not to the free-field
stencil.

FTD-0836 adds a complementary exact coordinate statement on that same shell.
The signed energy coordinate `u_sd=x|x|` turns `x^4+y^2=1` into the self-dual
circle `u_sd^2+y^2=1`, whose physical angular speed is
`-2 sqrt(|u_sd|)`. Its full traversal weight is `sqrt(pi)G*`. This `u_sd` is
not the CM coordinate `u=x^-2` in §3. The map is non-diffeomorphic at `x=0`,
so it clarifies the self-dual energy geometry and period mechanism without
supplying a regular production update or native stabilizer.

## 8. What remains open

The remaining debts are physical rather than algebraic:

1. derive or maintain the critical surface `mu=0` from substrate dynamics;
2. localize the clock and book controller work, dissipation, detuning, and
   amplitude stability;
3. show that an operational apparatus retains the orientation sign;
4. exhibit target-blind physical contexts corresponding to the arithmetic
   places; and
5. demonstrate operational consequences beyond the already-known period.

Failure of any physical item does not falsify the algebraic map. It falsifies
the promotion of the selected mathematical clock to native hardware.

## 9. Reproducibility

The exact certificate is
[`proof_quartic_clock_cm_gearbox.sage`](../../../../../scripts/proofs/proof_quartic_clock_cm_gearbox.sage).
It verifies:

```text
genus-one quartic energy shell
direct map to v^2=u^3-u
exact differential pullback and sign reversal
deck involution
birational 32a1 model and inverse
explicit degree-two 32a1 -> 32a2 isogeny
conductor 32 and j=1728
global isogeny/L-function identity
quartic Hamiltonian orientation

PASS 22/22
```

No curve, conductor, map coefficient, prime subset, trace sign, or numerical
tolerance was selected from a search or near miss.

## 10. Completion verdict

The sentence

> The substrate has clock hardware; CM arithmetic has a global calendar; the
> gearbox identifying the two has not been derived

now splits into two honest statements:

- **Mathematical gearbox:** `[CONDITIONAL THEOREM]` — once the critical
  quartic clock is supplied, its own energy curve, differential, orientation,
  integral homology, and conductor-32 CM system provide the gearbox exactly.
- **Ontic hardware:** `[OPEN]` — FTD has not yet derived and maintained that
  critical quartic clock from the production substrate.

The missing object is no longer an abstract CM orientation map. It is the
physical mechanism that holds the substrate on the quartic critical surface.
