# EXPLR - Dyadic Trigonal Relay Family

**Document type:** Exploratory mathematical note
**Status:** [THEOREM] for the exact finite trigonometric identities; [OPEN] for
projective singularity-budget generalization
**Companion seed note:** [EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md](EXPLR_DYADIC_LACUNARY_FOURIER_CURVE.md)
**Companion amplitude atlas:** [EXPLR_DYADIC_OCTAVE_BIFURCATION_ATLAS.md](EXPLR_DYADIC_OCTAVE_BIFURCATION_ATLAS.md)
**Verifier:** `scripts/proofs/proof_dyadic_trigonal_relay_family.py`

---

## 0. Purpose

The seed curve `C_3` has an exact trigonal node relay:

```text
x-axis branch collapse
  -> hidden phase shift t -> t +/- 2*pi/3
  -> off-axis branch-overlap pair.
```

This note asks whether that relay is a one-off coefficient accident or a
family-level dyadic mechanism.

Result:

```text
The trigonal phase relay is family-level.
The projective singularity split 75 + 15 + 15 is not yet family-level.
```

No FTD physics claim is promoted here.

---

## 1. Family

Consider the finite alternating-chiral dyadic family

```text
C(t) = (x(t), y(t)),

x(t) = sum_k a_k cos(2^k t),
y(t) = beta sum_k (-1)^k a_k sin(2^k t),
```

with finite support, real coefficients `a_k`, and nonzero real `beta`.

The seed `C_3` is the special case

```text
beta = 2,
a_0 = 1,
a_1 = 1/2,
a_2 = 1/2,
a_3 = 3/8.
```

---

## 2. Dyadic three-phase barycenter

Let

```text
alpha = 2*pi/3.
```

For every dyadic frequency `n = 2^k`,

```text
cos(n alpha) = -1/2,
sin(n alpha) = (-1)^k sqrt(3)/2.
```

Therefore each individual dyadic sine/cosine mode has zero three-phase
barycenter:

```text
cos(nt) + cos(n(t+alpha)) + cos(n(t-alpha)) = 0,
sin(nt) + sin(n(t+alpha)) + sin(n(t-alpha)) = 0.
```

By linearity, any finite dyadic Fourier curve with matching sine/cosine
frequencies satisfies

```text
C(t) + C(t + alpha) + C(t - alpha) = 0.
```

This identity does not require the special `C_3` coefficients.

---

## 3. Alternating-chiral relay

The alternating-chiral coupling gives more than zero barycenter. For the family
in §1:

```text
x(t+alpha) - x(t-alpha) = -(sqrt(3)/beta) y(t),
y(t+alpha) + y(t-alpha) = -y(t).
```

Thus, if

```text
y(t) = 0,
```

then

```text
x(t+alpha) = x(t-alpha),
y(t+alpha) = -y(t-alpha).
```

So every x-axis branch-collapse seed produces a mirrored off-axis pair in the
two-branch algebraic readout:

```text
y^2 = Q(u),       u = cos(t).
```

This is the family-level mechanism behind the seed's trigonal node relay.

---

## 4. Seed specialization

For `C_3`, write

```text
u = cos(t),
y(t) = sin(t) P(u),
```

where

```text
P(u) = -96u^7 + 144u^5 - 52u^3 + 2.
```

Axis branch-collapse points are the roots of `P(u)=0` in `(-1,1)`.

If `r = cos(theta)` is such a root, the relay uses

```text
u_+ = cos(theta + 2*pi/3),
u_- = cos(theta - 2*pi/3).
```

Then

```text
u_+ + u_- = -r,
u_+ u_- = r^2 - 3/4.
```

Equivalently, in the symmetric-polynomial variables

```text
s = u_+ + u_-,
p = u_+ u_-,
```

the relay locus is

```text
p = s^2 - 3/4.
```

This recovers the exact seed relation already proved in the `C_3` note:

```text
A(s, s^2 - 3/4) = P(-s)/2.
```

---

## 5. What generalizes and what does not

### 5.1 Generalizes

The following are family-level facts:

```text
[THEOREM] Dyadic three-phase barycenter:
          C(t) + C(t+2*pi/3) + C(t-2*pi/3) = 0.

[THEOREM] Alternating-chiral relay:
          y(t)=0 implies the shifted phases have equal x and opposite y.

[THEOREM] Degree template:
          if the highest active frequency is 2^m, the Laurent/projective
          parametrization has generic homogeneous degree 2^(m+1).
```

### 5.2 Does not yet generalize

The following remain seed-specific until proven otherwise:

```text
[OPEN] The exact C3 finite self-pair factor split 14 + 28 + 108.

[OPEN] The C3 infinity cusp orders (4,11).

[OPEN] The closed C3 projective defect split 75 + 15 + 15 = 105.

[OPEN] The exact number and real/complex split of finite singularities for
       general coefficient choices.
```

The degree/genus template does generalize. The singularity distribution does
not automatically generalize.

---

## 6. Thermodynamic analogy

The three-phase identity gives a precise projection-thermodynamic metaphor.
For every hidden phase `t`, the packet

```text
{ t, t + 2*pi/3, t - 2*pi/3 }
```

has visible barycenter zero:

```text
C(t) + C(t+2*pi/3) + C(t-2*pi/3) = 0.
```

This resembles a microcanonical packet:

```text
three hidden phases
  -> conserved visible barycenter
  -> projection can still create branch overlap and degeneracy.
```

The analogy should be kept modest:

| Thermodynamic term | Curve analogue |
|---|---|
| microstate | hidden phase `t` |
| macrostate | visible point `C(t)` |
| degeneracy | multiple phases mapping to one visible location |
| entropy-like quantity | log of sampled phase-fiber size |
| conserved packet | three-phase barycenter identity |
| agitation / temperature proxy | derivative-energy hierarchy |

This is not physical thermodynamics. It is a reversible finite phase system
whose lossy projection can look thermodynamic after coarse-graining.

---

## 7. Next theorem targets

1. **Exact family relay theorem.**
   Promote §§2-3 into a formal theorem with the broadest allowed coefficient
   class.

2. **Bookend asymptotics.**
   For active modes `{0,M}`, prove node-birth and scalloping laws as
   `N=2^M` grows.

3. **Family singularity scheme.**
   Compute saturated self-pair ideals for small `m` symbolically and identify
   which singularity-budget features persist.

4. **Lift-as-desingularizer criterion.**
   Given a scalar diagnostic `Z(t)`, characterize when
   `C(t_1)=C(t_2)` but `Z(t_1) != Z(t_2)`, so the 3D lift separates a planar
   crossing.

5. **Projection thermodynamics note.**
   Formalize entropy-like fiber degeneracy and derivative-energy hierarchy
   without making physical thermodynamic claims.

---

## 8. Verification

Run:

```text
python scripts/proofs/proof_dyadic_trigonal_relay_family.py
```

Expected result:

```text
OK - trigonal relay is family-level for alternating-chiral dyadic curves.
No FTD physics claim promoted.
```

*End of document.*
