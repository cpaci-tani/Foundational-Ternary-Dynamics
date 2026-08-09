# ANALYSIS — The Dispersion Substitution, Executed: Necessary and Not Sufficient

**Status:** `[MEASURED — NON-UNIVERSAL DILATION EXPONENT]` +
`[NEGATIVE — A STATIC BINDING CANNOT GIVE RELATIVISTIC DILATION]` +
`[RETRACTION — withdraws the 2026-08-08 claim that the fix was "a modelling change, not a carrier search"]` +
`[BOOKED — FTD-0814]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_composite_clock_dilation.py`
**Parents:** `ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` (**which this
corrects**), `ANALYSIS_COMPOSITE_CONE_INHERITANCE_v1.md`,
`DERIV_TWO_OWED_PROOFS_v1.md` §2.7.
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. What was owed

The minimum viable clock is Galilean because its nodes carry Newtonian
dispersion $p^2/2m$. `ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` §4
diagnosed this and asserted the remedy:

> "Replacing $p^2/2m$ by the lattice-KG dispersion is a modelling change,
> not a carrier search."

That was stated and not run. It has now been run, and **it is wrong.**

## 2. The instrument

Two constituents on the axial section of the M18 lattice, bound by a
finite square well in the relative coordinate. At fixed total momentum $K$
the relative-motion Hamiltonian
$$H_K = \mathrm{Toeplitz}\big[\mathcal{F}^{-1}(\omega(q)+\omega(K-q))\big]
       + \mathrm{diag}\,V(r)$$
is Hermitian and small, so it is diagonalized exactly. Two bound states
below the two-particle continuum give a genuine internal clock with gap
$\Omega = E_1 - E_0$.

**The test, and why universality is the whole of it.** A moving clock must
satisfy $\Omega(K) = \Omega(0)/\gamma$ with $\gamma = E_0(K)/E_0(0)$ — and
must do so *with the same exponent whatever the clock is made of*. That
universality **is** time dilation; a material-dependent rate is not
dilation at all, however cleanly it fits a power law. So the instrument
fits
$$\frac{\Omega(K)}{\Omega(0)} = \gamma^{\,p}$$
and asks whether $p = -1$ independently of constituent mass and binding
fraction.

## 3. The control confirms the substitution is necessary

With Newtonian nodes (momentum folded to $[-\pi,\pi)$ first — see §6):

$$\max_K\left|\frac{\Omega(K)}{\Omega(0)}-1\right| = 8.7\times10^{-6}
\qquad\text{while }\gamma\text{ reaches }1.399 .$$

So $p = 0$: the internal clock does not move at all however fast the
composite travels. The Galilean theorem holds numerically, and the carrier
as built could never have tested dilation. **The substitution is
necessary.**

## 4. The result: the exponent is not universal

| $M$ | well $(G,R)$ | binding fraction | $\Omega(0)$ | $p$ |
|---|---|---|---|---|
| 0.40 | (0.045, 13) | 0.0479 | 0.017961 | $-1.976$ |
| 0.40 | (0.150, 6) | 0.1550 | 0.067457 | $-1.460$ |
| 0.40 | (0.300, 4) | 0.3103 | 0.131191 | $-1.088$ |
| 0.25 | (0.030, 13) | 0.0444 | 0.018666 | $-0.937$ |
| 0.60 | (0.100, 8) | 0.0724 | 0.033247 | $-2.295$ |
| 0.80 | (0.200, 6) | 0.1113 | 0.048528 | $-2.700$ |

$p$ spans $[-2.700, -0.937]$ — a spread of $1.76$ — varying with **both**
the constituent mass and the binding fraction. Relativity admits $p=-1$,
for every clock. The lattice constituents do make the clock slow, but by
an amount that depends on what the clock is made of.

> **The substitution is necessary and not sufficient.** It moves the
> exponent off zero, and does not move it to $-1$; it does not even move it
> to a single value.

## 5. Diagnosis: the binding does not contract

The relative effective mass is exact kinematics, and the instrument
verifies it:

| $K$ | $\mu_K/\mu_0$ | $\gamma^3$ | ratio |
|---|---|---|---|
| 0.2 | 1.03525 | 1.03281 | 1.0024 |
| 0.4 | 1.14469 | 1.13328 | 1.0101 |
| 0.6 | 1.34001 | 1.30719 | 1.0251 |
| 0.8 | 1.64333 | 1.56339 | 1.0511 |

$\mu_K = \mu_0\gamma^3$, from the curvature of $\omega(q)+\omega(K-q)$ at
its equal-velocity minimum. That factor is material-independent. What is
missing is the other half: **a static lab-frame well does not
Lorentz-contract.** A covariantly-bound composite gets a binding region
narrowing as $1/\gamma$, supplying a compensating material-independent
factor; a square well nailed to the lattice supplies nothing, and the
residual is then set by how the particular well's level spacing happens to
respond to $\mu$ — which is exactly the material dependence observed.

## 6. What this retracts

`ANALYSIS_POTENTIAL_VALIDITY_CLOCK_GATE_v1.md` §2 argued that of the two
limits hidden in a distance potential — Newtonian constituents and
instantaneous binding — **only the first destroys dilation**, citing
hydrogen: instantaneous Coulomb binding, exact dilation. The inference was
that the retardation axis could be set aside.

**That inference is withdrawn.** Hydrogen escapes because its binding
fraction is $\sim10^{-5}$, so the non-covariance of its binding is a
negligible correction — *not* because non-covariant binding is harmless.
At binding fractions of $0.04$–$0.31$ it is the leading error, and it
destroys universality.

The corrected statement of the two limits:

| limit | what it controls |
|---|---|
| constituent dispersion ($v/c$) | whether the clock dilates **at all** |
| binding covariance ($\omega r/c$) | whether the dilation is **universal**, i.e. relativistic |

Both are required. The first was correctly identified; the second was
wrongly dismissed.

*A method note, recorded because it nearly inverted the control.* The
Newtonian arm initially failed its own assertion at $6.9\times10^{-2}$.
The cause was not physics: $q^2$ was evaluated on the raw grid
$[0,2\pi)$, where it is not periodic, so $\omega_{\rm NR}(K-q)$ was
discontinuous at the zone edge. Folding $q$ into $[-\pi,\pi)$ first
recovers $8.7\times10^{-6}$. A non-periodic function evaluated on a
periodic momentum grid is a silent error of exactly the kind an assert
exists to catch.

## 7. Consequence for the gate

The composite-boost item does **not** reduce to a substitution. Its
requirement is now stated more precisely than before, and is harder:

> a carrier whose constituents carry the substrate dispersion **and**
> whose binding is mediated by the substrate field, so that the binding
> region contracts with the motion.

Band clearance (C2) remains a separate obstruction. The two are now known
to be independent: this instrument's composites are bound *below* the
two-particle continuum at every $K$ — band clearance is satisfied here by
construction — and dilation still fails. Clearing the band would not have
been enough.

## 8. Reproduction

```
python scripts/experiments/temporal_interior/derive_composite_clock_dilation.py
```

About a minute; deterministic. The run asserts the Galilean control, the
boundedness of the excited state at every sampled parameter set, and that
the exponent spread exceeds $0.5$ — the last so that a future change which
accidentally restored universality would fail loudly rather than pass
unnoticed.
