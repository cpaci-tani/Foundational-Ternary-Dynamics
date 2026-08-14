# Theorem — `i`, Gamma, and the Quartic-Square Split

**Identifier:** `FTD-0839`  
**Date:** 2026-08-10  
**Status:** `[THEOREM — CONDITIONAL SPECTRAL IDENTITY]` +
`[THEOREM — ORIENTATION-LOSS CONTROL]` +
`[THEOREM — SQUARE-FIELD QUARTICITY]` +
`[CORRECTION — i ALONE DOES NOT DERIVE G*]` +
`[OPEN — NATIVE LIFT-TO-PAIR GEARBOX]`  
**Certificate:**
[`proof_i_gamma_square_gearbox_probe.py`](../../../../../scripts/proofs/proof_i_gamma_square_gearbox_probe.py)
(`24/24` exact checks)  
**Pre-registration:**
[`PREREG_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md`](../../preregistrations/native_time_carrier_programme/PREREG_I_GAMMA_QUARTIC_SQUARE_SPLIT_v1.md)

## 1. Result

The simplest rigorous answer to "how do we get Gamma from `i`?" is a split
answer.

1. A selected real complex structure `J` with `J^2=-I` supplies an oriented
   order-four lift and eigenvalues `+i,-i`.
2. A selected twisted-circle domain converts those eigenphases into the
   quarter shifts `1/4,3/4`.
3. A selected anchored chiral half-line converts the shifts into arithmetic
   progressions.
4. Lerch's standard determinant theorem converts those progressions into
   Gamma values.
5. Their oriented ratio is `G*`.

Thus Gamma does not follow from `i` alone. It enters through the regularized
product of a selected infinite spectrum. The exact chain is

\[
\boxed{
J
\longrightarrow
\text{quarter eigenphases}
\xrightarrow{\text{twisted circle}}
\{n+\tfrac14\},\{n+\tfrac34\}
\xrightarrow{\text{chiral }n\geq0}
\det_\zeta
\xrightarrow{\text{Lerch}}
\frac{\Gamma(1/4)}{\Gamma(3/4)}=G^* .}
\]

Separately, the complex square

\[
U=\psi^2
\]

supplies a natural quartic energy because

\[
|U|^2=|\psi|^4.
\]

But it also identifies the two orientations:

\[
(+i\psi)^2=(-i\psi)^2=-\psi^2.
\]

The orientation carrier and the quartic-energy carrier are therefore not the
same object. A physical gearbox between them remains to be derived.

## 2. What `i` proves

Take

\[
J=\begin{pmatrix}0&-1\\1&0\end{pmatrix}.
\]

Then

\[
J^T J=I,\qquad \det J=1,\qquad J^2=-I,\qquad J^4=I.
\]

Over the complexification, the characteristic polynomial is

\[
\lambda^2+1=0,
\]

so the eigenvalues are `+i` and `-i`. This is the complete spectral content of
`J^2=-I` by itself. It does not specify a circle, differential operator,
Fourier tower, positive-frequency sector, spectral unit, or vacuum origin.

The direction is also additional data: `J` and `-J` both square to `-I`.
Choosing multiplication by `+i` selects an orientation; the equation
`J^2=-I` alone does not choose between the two.

## 3. Conditional quarter spectrum

Now add a circle coordinate and the boundary law

\[
\psi(\phi+2\pi)=J\psi(\phi).
\]

In the two eigensectors, a Fourier mode `exp(i lambda phi)` obeys

\[
e^{2\pi i\lambda}=+i
\quad\text{or}\quad
e^{2\pi i\lambda}=-i.
\]

Therefore

\[
\lambda\in\mathbb Z+\frac14
\quad\text{or}\quad
\lambda\in\mathbb Z+\frac34.
\]

This implication is a theorem **conditional on the twisted-circle domain**.
The domain is not a theorem of `J^2=-I`.

## 4. Conditional Gamma ratio

Choose the dimensionless, multiplicity-one, positive half-line spectra

\[
D_a=\{n+a:n=0,1,2,\ldots\}.
\]

The Hurwitz zeta function is

\[
\zeta_H(s,a)=\sum_{n=0}^{\infty}(n+a)^{-s},
\]

continued analytically to `s=0`. Its zeta determinant satisfies Lerch's
identity

\[
\det_\zeta D_a
=e^{-\zeta_H'(0,a)}
=\frac{\sqrt{2\pi}}{\Gamma(a)}.
\]

Hence

\[
\boxed{
\frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}}
=\frac{\Gamma(1/4)}{\Gamma(3/4)}
=G^*.}
\]

The identity is exact. Its physical premise is conditional.

## 5. Load-bearing controls

FTD-0839 tested five ways of weakening or changing the spectral realization.

### 5.1 Spectral origin

Starting at `n=1` instead of `n=0` gives

\[
\frac{\Gamma(5/4)}{\Gamma(7/4)}
=\frac13\frac{\Gamma(1/4)}{\Gamma(3/4)}
=\frac{G^*}{3}.
\]

The vacuum/origin anchoring is therefore load-bearing.

### 5.2 Multiplicity

With `r` identical copies, determinant multiplicativity gives

\[
\left(
\frac{\det_\zeta D_{3/4}}{\det_\zeta D_{1/4}}
\right)^r
=(G^*)^r.
\]

Multiplicity one is load-bearing.

### 5.3 Operator order

Using the squared positive operator gives

\[
\frac{\det_\zeta D_{3/4}^2}{\det_\zeta D_{1/4}^2}
=(G^*)^2.
\]

The first-order spectral choice is load-bearing.

### 5.4 Spectral scale

Because

\[
\zeta_H(0,a)=\frac12-a,
\]

zeta determinants obey

\[
\det_\zeta(cD_a)=c^{1/2-a}\det_\zeta D_a.
\]

Therefore a common scale gives

\[
\frac{\det_\zeta(cD_{3/4})}{\det_\zeta(cD_{1/4})}
=c^{-1/2}G^*.
\]

Equal finite products cancel a common factor, but their zeta-regularized
limits have unequal scaling exponents. The earlier statement that every
overall normalization cancels was too broad. A dimensionless unit or a
renormalization prescription is load-bearing.

### 5.5 Full-line orientation-blind control

For the positive full-line Laplacian

\[
\Delta_a=\{(n+a)^2:n\in\mathbb Z\},
\]

the exact determinant is

\[
\det_\zeta\Delta_a=4\sin^2(\pi a).
\]

At the conjugate quarter shifts,

\[
\det_\zeta\Delta_{1/4}
=\det_\zeta\Delta_{3/4}
=2,
\]

so

\[
\boxed{
\frac{\det_\zeta\Delta_{3/4}}
     {\det_\zeta\Delta_{1/4}}=1.}
\]

This is the decisive discriminator. An orientation-blind two-sided energy
spectrum loses the asymmetry that produces `G*`.

## 6. Why the complex square is still useful

Write

\[
\psi=x+iy,
\qquad
U=\psi^2=(x^2-y^2)+2ixy.
\]

Then

\[
|U|^2=(x^2-y^2)^2+(2xy)^2=(x^2+y^2)^2=|\psi|^4.
\]

If the radial coordinate is `q=|psi|`, the potential

\[
V=\lambda |U|^2=\lambda q^4
\]

has restoring force

\[
F_q=-\frac{dV}{dq}=-4\lambda q^3.
\]

This is a genuine algebraic mechanism for quarticity. It does not insert a
quartic exponent by numerical fitting: the fourth power follows from taking
the norm of a square.

It is nevertheless only a kinematic construction until the substrate
supplies the complex lift, the pair field, its coupling, and an energy-closed
update.

## 7. The information destroyed by squaring

The square map is a two-to-one quotient:

\[
p:\psi\mapsto\psi^2,
\qquad p(\psi)=p(-\psi).
\]

For the two oriented quarter-turns,

\[
p(+i\psi)=p(-i\psi).
\]

Equivalently, the twist shifts double modulo one:

\[
2\left(\frac14\right)=\frac12,
\qquad
2\left(\frac34\right)=\frac32\equiv\frac12\pmod1.
\]

Both land in the same half-twist sector. Its half-line determinant is

\[
\det_\zeta D_{1/2}
=\frac{\sqrt{2\pi}}{\Gamma(1/2)}
=\sqrt2,
\]

so the ratio of the two squared images is `1`.

This is exactly the information that a symmetric square of the BCC/CM carrier
cannot retain: `Sym^2(J)=Sym^2(-J)` on the one-complex-dimensional lift.

## 8. The smallest faithful architecture

The smallest exact architecture cannot store only `U=psi^2`. It must retain
the unsquared lift or an equivalent sheet/orientation label.

For a discrete trajectory, define the orientation witness

\[
\chi_n=operatorname{sgn}
\operatorname{Im}(\overline{\psi_n}\psi_{n+1})
\]

when the imaginary part is nonzero. Under the two ideal quarter-turns,

\[
\psi_{n+1}=+i\psi_n\Rightarrow\chi_n=+1,
\qquad
\psi_{n+1}=-i\psi_n\Rightarrow\chi_n=-1.
\]

But in both cases

\[
U_{n+1}=\psi_{n+1}^2=-U_n.
\]

Therefore the pair

\[
\boxed{(U_n,\chi_n)}
\]

is the simplest faithful mathematical state: `U` carries quartic energy and
`chi` carries the lost orientation. Calling this structure physically native
would require a substrate realization; FTD-0839 does not supply one.

The proposed alphabet `{0,i}` is insufficient because it is not closed:
`i*i=-1` is missing. The smallest zero-augmented orbit closed under
multiplication by `i` is

\[
\{0,+1,+i,-1,-i\}.
\]

This is a potential-phase alphabet, not a replacement for the actual ternary
record alphabet.

## 9. Exact gearbox debt

A native lift-to-pair gearbox must now satisfy all of the following without
reading the target value `G*`:

1. **Lift:** produce a local two-channel state carrying a selected `J` or its
   equivalent oriented area form.
2. **Direction:** dynamically preserve or select the sign of `chi`, rather
   than erase it in an energy-only observable.
3. **Domain:** produce the clock circle or recurrent orbit whose monodromy is
   `J`.
4. **Polarization:** justify a chiral/positive-frequency half-line from the
   update law.
5. **Anchor:** identify the physical `n=0` origin.
6. **Unit:** fix the dimensionless spectral scale or supply an explicit
   renormalization convention.
7. **Multiplicity:** determine why one copy contributes to the clock
   determinant.
8. **Pair coupling:** generate `U=psi^2` or an equivalent constituent-pair
   closure from native dynamics.
9. **Energy closure:** couple `|U|^2` to a positive bath/work ledger without
   free maintenance.
10. **Clock test:** show a context-blind phase gate and `G*` period behavior
    distinct from a mere time rescaling.

Until those gates pass, the correct ledger statement is:

> **The substrate has candidate orientation and pair-field hardware in
> separate mathematical layers. CM arithmetic supplies a coherent chiral
> calendar conditionally. The native gearbox identifying the two has not been
> derived.**

## 10. Epistemic disposition

| Claim | Status |
|---|---|
| `J^2=-I` gives `J^4=I` and eigenvalues `+i,-i` | `[THEOREM]` |
| twisted boundary gives shifts `1/4,3/4` | `[THEOREM — CONDITIONAL ON DOMAIN]` |
| anchored chiral determinant ratio equals `G*` | `[THEOREM — CONDITIONAL SPECTRAL IDENTITY]` |
| `J^2=-I` alone derives `G*` | `[REJECTED BY EXACT CONTROLS]` |
| full-line orientation-blind ratio equals `1` | `[THEOREM]` |
| `U=psi^2` gives `|U|^2=|psi|^4` | `[THEOREM]` |
| the square retains clockwise/counterclockwise orientation | `[REJECTED BY EXACT CONTROL]` |
| `(U,chi)` is the minimum faithful mathematical lift | `[THEOREM — INFORMATIONAL]` |
| the production substrate realizes `(U,chi)` and its chiral determinant | `[OPEN]` |
| `G*` is already a native actualization cadence | `[OPEN]` |

## 11. Certificate outcome

The source-locked exact certificate returned:

```text
FTD-0839 i/Gamma/quartic-square split: 24/24 PASS
I_FORCES_ORIENTED_QUARTER_EIGENPHASES_ONLY
GSTAR_REQUIRES_TWISTED_DOMAIN_CHIRAL_HALF_LINE_SCALE_AND_ORIGIN
COMPLEX_SQUARE_SUPPLIES_QUARTIC_ENERGY_AND_ERASES_ORIENTATION
LIFT_TO_PAIR_GEARBOX_STATUS=OPEN
```

This is registered Outcome B. No production code was changed.
