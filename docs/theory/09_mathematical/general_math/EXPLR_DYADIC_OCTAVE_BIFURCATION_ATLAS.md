# EXPLR - Dyadic Octave-8 Bifurcation Atlas

**Document type:** Exploratory mathematical note
**Status:** [THEOREM] for the stated one-parameter axis-seed, tangency, and
regularity thresholds; [OPEN] for a full all-node and turning-number atlas
**Primary curve note:** [EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md](EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md)
**Relay theorem:** [EXPLR_DYADIC_TRIGONAL_RELAY_FAMILY.md](EXPLR_DYADIC_TRIGONAL_RELAY_FAMILY.md)
**Master-quadratic boundary:** [EXPLR_DYADIC_CUBIC_MASTER_QUADRATIC_BOUNDARY.md](EXPLR_DYADIC_CUBIC_MASTER_QUADRATIC_BOUNDARY.md)
**Verifier:** `scripts/proofs/proof_dyadic_octave_bifurcation_atlas.py`

---

## 0. Purpose and scope

The interactive lab makes the octave-8 mode feel unusually consequential. This
note turns that observation into an exact one-parameter atlas. Keep the first
three amplitudes fixed and vary only the octave-8 amplitude:

```text
a = (a_0, a_1, a_2, a_3) = (1, 1/2, 1/2, q),
beta = 2.
```

Thus

```text
x_q(t) = cos(t) + (1/2)cos(2t) + (1/2)cos(4t) + q cos(8t),
y_q(t) = 2sin(t) - sin(2t) + sin(4t) - 2q sin(8t).
```

The seed is `q=3/8`.

The result is not an evolution law: `q` labels different static curves. The
"mode dynamics" in this note are exact geometric transitions of that family
as the control is changed.

This atlas classifies:

1. roots that seed x-axis branch collapses;
2. values where an axis branch collapse becomes tangential;
3. values where the full plane parametrization loses regularity.

It does not yet classify every off-axis self-intersection or every turning
number chamber.

No FTD physics claim is promoted here.

---

## 1. Chebyshev branch reduction

Put `u=cos(t)`. Then `x_q(t)=X_q(u)` with

```text
X_q(u) =
128q u^8 - 256q u^6 + 160q u^4 - 32q u^2 + q
+ 4u^4 - 3u^2 + u.
```

The alternating-chiral sine coordinate factors as

```text
y_q(t) = sin(t) P_q(u),

P_q(u) =
-256q u^7 + 384q u^5 + (8-160q)u^3 + (16q-6)u + 2.
```

Every simple root `u` of `P_q` in `(-1,1)` gives the phase pair

```text
t = +/- arccos(u)
```

at the same x-axis point. When also `X_q'(u) != 0`, this is a transverse
axis branch-collapse seed.

The endpoints are special:

```text
P_q(-1) = 16q,
P_q(1)  = -4(4q-1).
```

So `q=0` and `q=1/4` are endpoint events before any interior discriminant is
considered.

---

## 2. Exact axis-seed chambers

The interior multiple-root discriminant is

```text
Res_u(P_q, dP_q/du) = nonzero_constant * q^5 Q(q),
```

where

```text
Q(q) =
4194304q^8 + 1572864q^7 - 11304960q^6 - 3051520q^5
+2763648q^4 + 1024848q^3 + 72225q^2 + 1188q - 108.
```

`Q` has six simple real roots `rho_1 < ... < rho_6`. Exact rational isolating
intervals are:

| Root | Isolating interval |
|---|---|
| `rho_1` | `(-299/183, -1013/620)` |
| `rho_2` | `(-38/89, -415/972)` |
| `rho_3` | `(-110/359, -91/297)` |
| `rho_4` | `(4/147, 15/551)` |
| `rho_5` | `(239/414, 295/511)` |
| `rho_6` | `(2806/1871, 2809/1873)` |

Sturm counts of roots of `P_q` in `(-1,1)` give the complete axis-seed table
away from the listed critical parameters:

| Amplitude chamber | Interior axis seeds |
|---|---:|
| `q < rho_1` | 7 |
| `rho_1 < q < rho_2` | 5 |
| `rho_2 < q < rho_3` | 3 |
| `rho_3 < q < 0` | 1 |
| `0 < q < rho_4` | 2 |
| `rho_4 < q < 1/4` | 2 |
| `1/4 < q < rho_5` | 3 |
| `rho_5 < q < rho_6` | 5 |
| `rho_6 < q` | 7 |

The small positive algebraic threshold `rho_4` is inactive for the physical
axis-root count: its discriminant collision occurs outside `u in (-1,1)`.
The count changes visible in the positive-amplitude lab are therefore:

```text
2 seeds  -- q=1/4 -->  3 seeds
3 seeds  -- q=rho_5 -->  5 seeds
5 seeds  -- q=rho_6 -->  7 seeds.
```

The seed `q=3/8` lies in the three-seed chamber, exactly as its three x-axis
branch collapses show.

---

## 3. Tangent-axis thresholds

An axis seed is transverse precisely when the two phase branches do not share
the same vertical tangent. The candidate condition is `P_q(u)=X_q'(u)=0`.
Its resultant is

```text
Res_u(P_q, X_q') = nonzero_constant * q^5 (21q^2 - 28q + 4).
```

Besides the degenerate `q=0` case, there are two exact tangent-axis thresholds:

```text
tau_- = (14 - 4sqrt(7))/21,
tau_+ = (14 + 4sqrt(7))/21.
```

Their corresponding physical cosine values are

```text
u_- = (sqrt(7)-1)/4,
u_+ = -(sqrt(7)+1)/4.
```

Both lie strictly in `(-1,1)`. The axis-root discriminant `Q` is coprime to
`21q^2-28q+4`, so these are tangent events of simple `P_q` roots, not seed
birth/death events. They preserve the seed count while changing the local
geometry.

---

## 4. Full regularity thresholds

The previous sections concern the x-axis branch system. The entire plane curve
can also lose regularity: both components of `C_q'(t)` vanish.

For `sin(t) != 0`, this is the exact polynomial system

```text
X_q'(u) = 0,
Y_q(u) = 0,

Y_q(u) = u P_q(u) - (1-u^2)P_q'(u).
```

Its resultant is

```text
Res_u(X_q', Y_q) =
nonzero_constant * q^5 (4q-1) G(q)^3,

G(q) = 128q^3 + 16q^2 - 18q - 3.
```

The three real roots `gamma_1 < gamma_2 < gamma_3` of `G` have exact isolating
intervals:

| Root | Isolating interval |
|---|---|
| `gamma_1` | `(-43/128, -173/515)` |
| `gamma_2` | `(-199/1112, -17/95)` |
| `gamma_3` | `(154/395, 131/336)` |

At every `gamma_j`, the common cosine values are the three roots in `(-1,1)`
of

```text
u^3 - (3/4)u - 4gamma_j^2 + gamma_j/2 + 7/16 = 0.
```

Each such cosine gives two phase values, so the parametrization has six
interior speed-zero phases at every `gamma_j`. This is a real singular event
of the curve family, not just a readout crossing.

There are also the two endpoint-degenerate values:

```text
q = 0:    u=1/2 is an interior speed-zero cosine, and t=pi is endpoint-degenerate;
q = 1/4:  u=-1/2 is an interior speed-zero cosine, and t=0 is endpoint-degenerate.
```

Outside

```text
{0, 1/4, gamma_1, gamma_2, gamma_3},
```

the `C_q` parametrization is regular. In particular,

```text
G(3/8) = -3/4,
```

so the seed curve is regular, but it lives below the positive speed-zero wall
`gamma_3` in the same three-axis-seed chamber.

---

## 5. Relay consequences

Let `alpha=2*pi/3`. The family relay theorem gives, for every axis seed at
phase `t`, the exact coincidences

```text
C_q(t)          = C_q(-t),
C_q(t + alpha)  = C_q(-t + alpha),
C_q(t - alpha)  = C_q(-t - alpha).
```

Thus a transverse axis seed forces a three-member relay packet: one axis
coincidence plus a reflected off-axis pair. Away from tangencies and other
unclassified global collisions, an `n`-seed axis chamber supplies `3n` forced
relay coincidences.

For the seed:

```text
3 axis seeds -> 9 forced real relay coincidences,
```

matching the existing real-node picture.

This is not yet a proof that these are all real nodes for every `q`; the full
off-axis correspondence remains open.

---

## 6. What the mode control is actually doing

The octave-8 slider moves the curve across several distinct exact events:

```text
axis-seed births/deaths    -> root-count changes of P_q,
tangent-axis events        -> tau_- and tau_+,
speed-zero singular events -> gamma_1, gamma_2, gamma_3.
```

This explains why a small amplitude adjustment can appear to make the curve
switch species. It crosses a discriminant wall: the branch network or the
regularity class changes discontinuously, even though the coefficient itself
varies smoothly.

The result remains geometry of a finite Fourier family. It does not give an
FTD update rule, a lattice interaction, or a physical phase transition.

---

## 7. Next exact targets

1. Eliminate the full off-axis two-phase correspondence for generic `q` and
   decide when the relay packet exhausts the real node set.
2. Compute the rotation number in every regular chamber and determine its
   jump across the `gamma_j` speed-zero walls.
3. Extend the discriminant calculation to a two-control slice, for example
   `(a_2, a_3)`, while retaining exact semialgebraic chamber certificates.
4. Render the atlas as a visual overlay in the lab, but label thresholds as
   geometric family facts rather than FTD dynamics.

---

## 8. Verification

Run:

```text
python scripts/proofs/proof_dyadic_octave_bifurcation_atlas.py
```

The verifier uses exact polynomial algebra and Sturm root counts only. It does
not conduct a numerical near-miss search.

*End of document.*
