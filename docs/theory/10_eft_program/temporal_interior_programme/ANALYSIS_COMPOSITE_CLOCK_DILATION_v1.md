# ANALYSIS — Composite Spectral Gap Against the Finite-Gap SR Comparator

**Status:** `[OPEN]` — exploratory/inconclusive finite-gap diagnostic. The
former $p=-1$ exponent verdict and static-binding no-go are withdrawn.
**Date:** 2026-08-08; corrected 2026-08-09
**Artifact:** `scripts/experiments/temporal_interior/derive_composite_clock_dilation.py`
**Parents:** `ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` (which the first
version attempted to correct), `ANALYSIS_COMPOSITE_CONE_INHERITANCE_v1.md`,
`DERIV_TWO_OWED_PROOFS_v1.md` §2.7.
**Production impact:** none. No constant or production rule is changed.

---

## 1. What was owed

The minimum viable mechanical clock used Newtonian constituent dispersion
$p^2/2m$ and was therefore Galilean. A proposed diagnostic replaced that
dispersion with the axial lattice-KG dispersion and asked whether the gap
between two bound levels slowed relativistically.

The first version of this analysis used

$$
\frac{\Omega(K)}{\Omega(0)}=\gamma^p,
\qquad \gamma=\frac{E_0(K)}{E_0(0)},
$$

and treated $p=-1$ as the exact special-relativistic target. That target is
not exact for a finite spectral gap. Because all sampled $\gamma$ values are
close to one, taking a ratio of logarithms also magnified small discrepancies
into a large apparent spread in $p$. The earlier negative verdict therefore
does not follow from the reported exponent range.

## 2. Instrument and scope

Two constituents on the axial section of the M18 lattice are bound by a
finite square well in the relative coordinate. At fixed total momentum $K$,

$$
H_K=\operatorname{Toeplitz}
\!\left[\mathcal F^{-1}\!\left(\omega(q)+\omega(K-q)\right)\right]
+\operatorname{diag}V(r)
$$

is diagonalized numerically. Two eigenvalues below the free two-particle
continuum define the spectral gap
$\Omega(K)=E_1(K)-E_0(K)$.

The scope is one dimensional, one lattice size ($L=512$), one square-well
interaction class, six fixed parameter cases, and three comparison momenta
$K\in\{0.10,0.15,0.20\}$. No finite-volume, momentum-window, continuum, or
interacting common-cone convergence study is supplied. The calculation is
therefore an exploratory diagnostic, not a structural result about static
potentials or Lorentz recovery.

## 3. The correct finite-gap comparator

For a Lorentz-covariant two-level system with rest energies $M_0$ and $M_1$,
the exact energies at common momentum $P$ are

$$
E_j(P)=\sqrt{M_j^2+c^2P^2}.
$$

The fixed-momentum gap ratio is consequently

$$
R_{\rm SR}(P)
=\frac{E_1(P)-E_0(P)}{M_1-M_0}
=\frac{M_0+M_1}{E_0(P)+E_1(P)}.
\tag{1}
$$

Writing $\gamma_0=E_0(P)/M_0$ gives the form used by the artifact,

$$
R_{\rm SR}
=\frac{M_0+M_1}
{\gamma_0M_0+\sqrt{M_1^2+M_0^2(\gamma_0^2-1)}}.
\tag{2}
$$

Only in the infinitesimal-gap limit
$(M_1-M_0)/M_0\to0$ does (2) reduce to $1/\gamma_0$. Thus $p=-1$ is a
small-gap approximation, not the exact target for the finite gaps in this
instrument.

For the table below, the measured ground-state ratio
$E_0(K)/E_0(0)$ is used as a conditional proxy for $\gamma_0$. This asks
whether the excited level shares the dispersion inferred from the ground
level. It does **not** independently establish that the ground level itself
has the correct common-cone relativistic dispersion.

## 4. Controls

With Newtonian constituents, after folding momentum into $[-\pi,\pi)$, the
gap stays fixed to

$$
\max_K\left|\frac{\Omega(K)}{\Omega(0)}-1\right|
=8.69\times10^{-6}
$$

while the ground-state ratio reaches $\gamma_0=1.3992$. At that endpoint the
maximum relative discrepancy from (2) is $39.1\%$. Within this declared
instrument, Newtonian constituent dispersion therefore fails the finite-gap
SR comparator. This is a valid control result; it is not a proof that a
particular lattice substitution is the unique remedy.

The method note from the first run remains relevant. The Newtonian arm had
initially evaluated $q^2$ on the raw periodic grid $[0,2\pi)$, making
$\omega_{\rm NR}(K-q)$ discontinuous at the zone boundary. Folding to
$[-\pi,\pi)$ removes that numerical artifact.

## 5. Corrected fixed-case results

Define the reported residual

$$
r(K)=\frac{\Omega(K)/\Omega(0)}{R_{\rm SR}(K)}-1.
$$

The deterministic rerun gives:

| $M$ | well $(G,R)$ | binding fraction | $\Omega(0)/E_0(0)$ | $r(0.10)$ | $r(0.15)$ | $r(0.20)$ | $\max|r|$ |
|---:|---:|---:|---:|---:|---:|---:|---:|
| 0.40 | (0.045, 13) | 0.0479 | 0.0234 | $-0.269\%$ | $-0.605\%$ | $-1.076\%$ | $1.076\%$ |
| 0.40 | (0.150, 6) | 0.1550 | 0.0991 | $-0.158\%$ | $-0.357\%$ | $-0.638\%$ | $0.638\%$ |
| 0.40 | (0.300, 4) | 0.3103 | 0.2362 | $-0.092\%$ | $-0.208\%$ | $-0.373\%$ | $0.373\%$ |
| 0.25 | (0.030, 13) | 0.0444 | 0.0390 | $+0.034\%$ | $+0.044\%$ | $-0.0001\%$ | $0.044\%$ |
| 0.60 | (0.100, 8) | 0.0724 | 0.0294 | $-0.165\%$ | $-0.372\%$ | $-0.660\%$ | $0.660\%$ |
| 0.80 | (0.200, 6) | 0.1113 | 0.0332 | $-0.130\%$ | $-0.292\%$ | $-0.519\%$ | $0.519\%$ |

The maximum fixed-case residuals span $0.044\%$ to $1.076\%$. These values
do not establish exact Lorentzian dilation, but they also do not support the
earlier headline that a material-dependent exponent spanning
$[-2.700,-0.937]$ is a structural failure. One case is consistent with the
conditional comparator at the present numerical scope, while the others
show sub-percent to one-percent residuals whose origin has not been isolated.

## 6. The $\mu_K/\mu_0$ relation is approximate

For the free pair, let $\mu_K^{-1}$ be the curvature of
$\omega(q)+\omega(K-q)$ at its equal-velocity minimum, and define the
corresponding free-pair ratio

$$
\gamma_f=\frac{\omega(K/2)}{\omega(0)}.
$$

The continuum relativistic small-momentum relation is
$\mu_K/\mu_0\simeq\gamma_f^3$. It is not an exact identity of the lattice
dispersion:

| $K$ | $\mu_K/\mu_0$ | $\gamma_f^3$ | relative residual |
|---:|---:|---:|---:|
| 0.2 | 1.03525 | 1.03183 | $+0.332\%$ |
| 0.4 | 1.14469 | 1.12907 | $+1.383\%$ |
| 0.6 | 1.34001 | 1.29673 | $+3.338\%$ |
| 0.8 | 1.64333 | 1.54242 | $+6.542\%$ |

The earlier table compared the free-pair curvature to an interacting
ground-state $\gamma$ and then called the result exact despite residuals up
to several percent. The corrected comparison keeps both quantities in the
same free-pair sector and labels the relation at its actual asymptotic
strength.

## 7. Revised verdict and retractions

The supported conclusions are now only:

1. In this instrument, Newtonian constituent dispersion leaves the internal
   gap essentially fixed and fails the finite-gap SR comparator.
2. With the chosen lattice constituent dispersion, the six fixed square-well
   cases lie within $0.044\%$--$1.076\%$ of the conditional comparator over
   the three sampled momenta.
3. Those residuals have not been separated into finite-volume,
   finite-momentum, lattice-dispersion, interaction, or genuine common-cone
   contributions.

The following former conclusions are withdrawn:

- $p=-1$ as the exact finite-gap target;
- the spread of fitted $p$ as a structural non-universality result;
- $\mu_K/\mu_0=\gamma^3$ as an exact lattice identity;
- the claim that the static well has been shown to be the cause of the
  residual;
- the claimed static-binding no-go and the associated assertion that a
  Lorentz-contracting binding region is the uniquely diagnosed repair.

A covariant interacting action remains the appropriate eventual test, but
that is a programme requirement, not a conclusion of this six-case screen.
Band clearance is likewise a separate condition: the two bound levels lie
below the free continuum in these cases, but this does not settle their
boost law.

## 8. Reproduction

```text
python scripts/experiments/temporal_interior/derive_composite_clock_dilation.py
```

The run is deterministic. It asserts the Newtonian control, verifies that
the excited level remains below the continuum in every fixed case, and
checks the infinitesimal-gap limit of (2). It does not encode the scientific
outcome as a pass/fail gate.
