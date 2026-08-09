# ANALYSIS — The Limiting Speed Depends on the Rest Mass: the Kinematic Half of the Two-Body Register, Computed

**Status:** `[DERIVED — EXACT DISPERSION, SYMBOLIC]` +
`[MEASURED — DILATION RESIDUAL, NUMERIC]` +
`[NEGATIVE — SPECIES DO NOT SHARE A CONE EXACTLY]` +
`[AMENDS — DERIV_TWO_OWED_PROOFS_v1 §2.7: one of the three two-body items was a missing calculation, not a missing instrument]` +
`[BOOKED — FTD-0812]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_massive_cone_dispersion.py`
**Parents:** `DERIV_TWO_OWED_PROOFS_v1.md` §2.7 (the split this amends),
`AUDIT_LORENTZ_COMMON_CONE_GATE.md` (FTD-0412, the *inter-sector* result
this does **not** rescue), `ANALYSIS_CAUSAL_ISOTROPY_SCALING_v1.md`.
**Production impact:** none. No constant is changed; no tag moves.

---

## 1. Why not simply boost the clock

The obvious attack on the two-body register is to take the minimum viable
clock, give it momentum, and measure its period against $\gamma$. That
test is vacuous, and seeing why sharpens the whole question.

The MVC is a *mechanical* framework: $m\ddot q = -\nabla V(|q_i-q_j|)$.
That equation is **Galilean** invariant. Boost it and the period is
unchanged, $T(v) = T(0)$ exactly, for any $v$. It would report zero time
dilation not because the substrate fails to dilate but because a
distance-potential oscillator is the wrong object to ask.

Time dilation in a field substrate is not kinematic decoration; it comes
from *the binding being mediated at the substrate's own speed*. This is
Lorentz's original electron-theory reasoning and the light-clock argument
in modern dress. So the object to interrogate is the **dispersion of a
massive mode**, not a mechanical oscillator — and that is exactly
computable without constructing any clock at all.

**Consequence for the obligation.** The two-body register was recorded as
"open and clock-gated," with the note that "the obstruction is a missing
instrument, not a missing calculation." For one of its three items that
is not correct, and this document supplies the calculation.

## 2. The operational form of the test

A packet built around wavenumber $k$ travels at its own group velocity
$v_g = |\nabla_k\Omega|$. Its internal phase advances per unit lab time at
$\Omega$; per unit *proper* time the relativistic prediction is

$$\Omega_{\rm proper}(k) \;=\; \Omega(k)\,\sqrt{1 - v_g(k)^2/c^2}
\;=\; \text{const},$$

which is an identity for $\Omega^2 = c^2k^2 + M^2$, since
$v_g = c^2k/\Omega$ gives $1-v_g^2/c^2 = M^2/\Omega^2$. **So
$\Omega_{\rm proper}$ is $k$-independent precisely when a moving clock of
that species dilates exactly, and its $k$-dependence *is* the violation.**

Adding a mass to the M18 leapfrog,
$\phi_{t+1}-2\phi_t+\phi_{t-1} = C^2L\phi - M^2\phi$, gives the exact
dispersion
$$4\sin^2(\Omega/2) \;=\; C^2\,(-L(k)) + M^2 \;\equiv\; W(k).$$

*Scope.* This is the generic quadratic massive mode on this substrate —
what any massive excitation obeys at quadratic order in the scalar sector.
It is not a claim about FTD's own manifestation/mass mechanism.

## 3. The dispersion, exactly

Expanding $\Omega^2$ in $k$ at fixed $M$ along $[100]$, $[110]$, $[111]$
gives **identical** coefficients through $k^4$:

$$\Omega^2 = \underbrace{\Big(M^2 + \tfrac{M^4}{12}\Big)}_{k^0}
+ \underbrace{\Big(\tfrac13 + \tfrac{M^2}{18} + \tfrac{M^4}{90}\Big)}_{k^2}k^2
+ \underbrace{\Big(-\tfrac1{54} - \tfrac{M^2}{1080}\Big)}_{k^4}k^4 + \dots$$

Reading the $k^2$ coefficient as the squared limiting speed:

$$\boxed{\;C_{\rm eff}^2(M) = C^2\Big(1 + \tfrac{M^2}{6} + \tfrac{M^4}{30}\Big),
\qquad \frac{C_{\rm eff}}{C} = 1 + \frac{M^2}{12}
+ \frac{19M^4}{1440} + O(M^6).\;}$$

Two facts, both load-bearing.

**The limiting speed is exactly isotropic.** The $k^2$ coefficients agree
across all three symmetry directions identically, not approximately. So
does the $k^4$ coefficient — the first anisotropy is at $k^6$, which is
the known M18 property.

**The limiting speed depends on the rest mass.** This is a *negative*
result: two species of different mass do not share a cone exactly. It is
the classic species-dependent maximum-attainable-velocity signature.

### Cross-check against the recorded free-sector figure

At $M=0$ the phase velocities expand as
$$\frac{v_{[100]}}{C} = 1 - \frac{k^2}{36} - \frac{k^4}{1440},\quad
\frac{v_{[110]}}{C} = 1 - \frac{k^2}{36},\quad
\frac{v_{[111]}}{C} = 1 - \frac{k^2}{36} - \frac{k^4}{2592},$$
so that
$$\frac{v_{[100]} - v_{[111]}}{C} = -\frac{k^4}{3240}$$
**exactly**, reproducing the recorded free-sector anisotropy
$|\Delta v/v| = (ka)^4/3240$ by an independent route. This validates the
whole symbolic chain. (Note also that $[110]$ carries *no* $k^4$ term at
all — it is the extremal direction.)

## 4. Does a moving clock dilate?

Numerically, along $[100]$, with $C_{\rm eff}(M)$ extracted from the exact
dispersion by fit rather than from the truncated series:

| $M$ | $k$ | $v_g/C$ | dev, $c=C$ | dev, $c=C_{\rm eff}$ | $k^4/36M^2$ |
|---|---|---|---|---|---|
| 0.5 | 0.01 | 0.0119 | $-2.99\times10^{-6}$ | $1.100\times10^{-9}$ | $1.111\times10^{-9}$ |
| 0.5 | 0.10 | 0.1183 | $-2.87\times10^{-4}$ | $1.100\times10^{-5}$ | $1.111\times10^{-5}$ |
| 0.2 | 0.01 | 0.0290 | $-2.80\times10^{-6}$ | $6.935\times10^{-9}$ | $6.944\times10^{-9}$ |
| 0.2 | 0.10 | 0.2784 | $-2.11\times10^{-4}$ | $6.931\times10^{-5}$ | $6.944\times10^{-5}$ |
| 0.05 | 0.01 | 0.1147 | $-2.67\times10^{-6}$ | $1.111\times10^{-7}$ | $1.111\times10^{-7}$ |

Two readings.

**With the bare $C$ the relation fails at $O(k^2)$, coefficient $-1/36$** —
but that is the ordinary lattice dispersion, not a Lorentz anomaly, and it
is removed *exactly* by using the species' own limiting speed.

**With $C_{\rm eff}(M)$ the residual is $O(k^4)$ and equals $k^4/(36M^2)$**
to three or four significant figures at every mass and every $k$ sampled.
Analytically, if $\Omega^2 = C_{\rm eff}^2k^2 + M_{\rm eff}^2 + bk^4$ then
$\Omega_{\rm proper}^2 = M_{\rm eff}^2 - 3bk^4$, and $b = -1/54$ gives
exactly $k^4/(36M^2)$.

Converting to the physically natural variable — for a slow clock
$k \simeq (M/C)\beta$ with $\beta = v_g/C$, and $C^2 = 1/3$ —

$$\boxed{\;\frac{\Delta\Omega_{\rm proper}}{\Omega_{\rm proper}}
\;\simeq\; \frac{M^2\beta^4}{4}.\;}$$

So **each mass has its own very nearly Lorentzian sector**: within a
species, time dilation holds up to a $\beta^4$ correction suppressed by
$M^2 = (m/M_{\rm Planck})^2$. The violation lives in the *mismatch between*
sectors, not inside one.

## 5. The two-body number

| quantity | value |
|---|---|
| one tick $t_{\rm phys}$ | $3.113\times10^{-44}$ s |
| electron $M_e = \omega_e t_{\rm phys}$ | $2.4165\times10^{-23}$ |
| proton $M_p$ | $4.4370\times10^{-20}$ |
| $\Delta C/C$ electron | $4.87\times10^{-47}$ |
| $\Delta C/C$ proton | $1.64\times10^{-40}$ |
| **differential, proton vs electron** | $\mathbf{1.64\times10^{-40}}$ |
| dilation error, electron clock at $\beta=0.1$ | $1.5\times10^{-50}$ |

The species-dependent limiting speed is a dimension-six-suppressed effect,
$(m/M_{\rm Planck})^2/12$, isotropic, and of order $10^{-40}$ for the
proton–electron pair. Bounding it would require sensitivity to a
differential maximum-attainable-velocity at the $10^{-40}$ level, many
orders beyond present reach. Time dilation for any physical clock holds to
$\sim10^{-50}$.

This is consistent with, and independent of, the earlier radiative-
stability finding that the dimension-four channel is forbidden by cubic
symmetry and effects "return at dimension six." Here is a concrete
dimension-six return, computed at tree level.

## 6. What this does **not** rescue

`AUDIT_LORENTZ_COMMON_CONE_GATE.md` (FTD-0412) records
`LIVE-COMMON-CONE-FAILS`: distinct *sectors* carry leading speeds
$c^2 = 1/3$ (production flux), $1/7$ (BCC-time prototype) and raw $1$
(Wilson matter). Those are **order-unity** mismatches.

The effect computed here is $10^{-40}$ and lives *within* a single sector.
It must not be read as good news about FTD-0412. The inter-sector failure
remains the serious open problem and is untouched; this document only
establishes that the *intra-sector, mass-dependent* part of the same
question is harmless.

## 7. Amendment to the two-body register

`DERIV_TWO_OWED_PROOFS_v1.md` §2.7 records the two-body register as three
bundled items — *common cone, interacting vertices, composite boosts* —
with the single verdict "open and clock-gated," and the note that "the
obstruction is a missing instrument, not a missing calculation." The
amendment is to **split the bundle**, because the three items do not share
a blocker:

| item | status after this document |
|---|---|
| **Common cone, free massive modes** | **Computed, not clock-gated.** Species-dependent limiting speed $C_{\rm eff} = C(1+M^2/12+\dots)$, isotropic; intra-species dilation exact to $O(\beta^4)$ with error $M^2\beta^4/4$. A negative result of quantified, unobservable size. |
| **Common cone, across sectors** | **Open and failing** at order unity — FTD-0412, unchanged by this. |
| **Interacting vertices, composite boosts** | **Open and genuinely clock-gated.** Whether a bound *composite* inherits the dilation of its constituents' dispersion is not settled by a free-mode calculation, and the MVC cannot test it (§1). |

The phrase "the obstruction is a missing instrument, not a missing
calculation" was true of the bundle as a whole only because the bundle
hid an item that was calculable. It stands for the third row and should
be scoped to it.

## 8. Reproduction

```
python scripts/experiments/temporal_interior/derive_massive_cone_dispersion.py
```

Under a minute; symbolic (sympy) plus a deterministic numeric check. The
run cross-validates its own group-velocity formula against the
independently derived massless expression before using it — an earlier
draft dropped the factor $2$ in $dW/d\Omega = 2\sin\Omega$, which doubles
$v_g$ and converts the genuine $O(k^4)$ residual into a spurious $O(k^2)$
one two orders of magnitude larger. That check is now part of the script.
