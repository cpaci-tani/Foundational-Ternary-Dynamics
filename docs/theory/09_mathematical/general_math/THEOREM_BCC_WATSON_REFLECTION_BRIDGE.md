# The BCC Watson Reflection Bridge

## Exact identity between the BCC Green's function and the reflection ratio

**Date:** 2026-04-22  
**Status:** [THEOREM] for the identity; [CONJECTURE] for the FTD interpretation  
**Proof script:** `scripts/proofs/proof_bcc_watson_reflection_bridge.py`

---

## 1. Statement

Let

```text
G* = Gamma(1/4) / Gamma(3/4)
```

and define the standard BCC Watson integral by

```text
W_BCC = (1/pi^3) integral_0^pi integral_0^pi integral_0^pi
        dx dy dz / (1 - cos(x) cos(y) cos(z)).
```

Then

```text
W_BCC = Gamma(1/4)^4 / (4*pi^3)
      = G*^2 / (2*pi).
```

Numerically:

```text
G*                = 2.9586751191886388923108213577...
G*^2/(2*pi)      = 1.3932039296856768591842462603...
W_BCC            = 1.3932039296856768591842462603...
```

---

## 2. Proof

Euler's reflection formula gives:

```text
Gamma(1/4) Gamma(3/4) = pi sqrt(2).
```

Therefore:

```text
Gamma(3/4) = pi sqrt(2) / Gamma(1/4)
```

and hence:

```text
G* = Gamma(1/4) / Gamma(3/4)
   = Gamma(1/4)^2 / (pi sqrt(2)).
```

Squaring:

```text
G*^2 = Gamma(1/4)^4 / (2*pi^2).
```

Dividing by `2*pi`:

```text
G*^2/(2*pi) = Gamma(1/4)^4 / (4*pi^3).
```

The classical BCC Watson integral evaluation is:

```text
W_BCC = Gamma(1/4)^4 / (4*pi^3).
```

Therefore:

```text
W_BCC = G*^2/(2*pi).
```

---

## 3. Normalization Notes

This identity depends on the BCC Watson convention:

```text
W_BCC = < 1 / (1 - cos x cos y cos z) >_[0,pi]^3.
```

It is **not** the simple-cubic Watson integral with denominator
`3 - cos x - cos y - cos z`.

For the BCC link-minimal lattice operator used in the Structure-2 tests,

```text
sigma(k) = (8/a^2) * [1 - cos(kx a/2) cos(ky a/2) cos(kz a/2)].
```

The dimensionless Watson part is the same BCC denominator. Any physical Green's
function using `sigma(k)` carries the extra prefactor from the operator
normalization, for example an `a^2/8` factor when inverting the massless
kinetic operator. The exact identity above is the normalized dimensionless
Watson integral.

---

## 4. FTD Interpretation

The theorem says that two independently motivated mathematical pipelines meet
at the same constant:

```text
Pipeline 1: BCC/Stella-Octangula pathing
            -> BCC Watson integral
            -> W_BCC

Pipeline 2: Gamma reflection at 1/4 and 3/4
            -> G* = Gamma(1/4)/Gamma(3/4)
            -> G*^2/(2*pi)
```

The equality

```text
W_BCC = G*^2/(2*pi)
```

is theorem-level mathematics. The interpretation that this identifies the
discrete BCC pathing topology with the analytic lemniscatic reflection
structure is an FTD model interpretation. It should be tagged as
`[CONJECTURE]` or `[SELECTION]` unless additional framework axioms make the
BCC lattice structure unique.

---

## 5. Epistemic Accounting

- [THEOREM] Euler reflection formula.
- [THEOREM] Classical BCC Watson evaluation.
- [THEOREM] Algebraic bridge `W_BCC = G*^2/(2*pi)`.
- [CONJECTURE] "Pipeline 1 and Pipeline 2 are the same physical object" as an
  FTD interpretation.
- [OPEN] Whether the BCC/Stella-Octangula lattice, rather than the cubic Moore
  lattice, is uniquely selected by the core FTD axioms or by a later
  consistency condition.

---

## 6. Corollary: From the Bridge to the Master Quadratic

The BCC bridge supplies the **quadratic coefficient source** once the FTD
normalization and the CM automorphism count are applied.

From the theorem:

```text
2*pi*W_BCC = G*^2.
```

Multiplying by the intrinsic CM factor

```text
|Aut(E: y^2 = x^3 - x)|^2 = 4^2 = 16
```

gives the total quadratic capacity:

```text
K = 16 * 2*pi * W_BCC
  = 16 * G*^2.
```

The next filtered-period step multiplies once more by the same bridge ratio
`G*`:

```text
G* K = 16 * G*^3.
```

Therefore the Vieta data of the master quadratic are:

```text
x_+ + x_- = K   = 16 * G*^2
x_+ * x_- = G*K = 16 * G*^3
```

and the monic quadratic is:

```text
x^2 - 16*G*^2*x + 16*G*^3 = 0.
```

This is the precise sense in which the master quadratic comes from the BCC
Watson/reflection bridge:

```text
BCC pathing          -> W_BCC
Gamma reflection     -> G*
W_BCC = G*^2/(2*pi) -> capacity K = 16G*^2
filtered step by G*  -> product 16G*^3
Vieta                -> x^2 - 16G*^2 x + 16G*^3
```

### Epistemic Tag

- [THEOREM] `W_BCC = G*^2/(2*pi)`.
- [THEOREM] `|Aut(E)|^2 = 16` for the CM curve `j = 1728`.
- [THEOREM] Given `K = 16*2*pi*W_BCC` and product `G*K`, Vieta gives the
  master quadratic.
- [SELECTION] The FTD normalization `K = |Aut(E)|^2 * 2*pi * W_BCC`.
- [SELECTION] The filtered-period/self-reference rule that the product is
  `G* K`.

So the bridge does not merely decorate the master quadratic after the fact. It
is the source of the `G*^2` coefficient. The remaining work is to promote the
normalization and filtered-step rules from `[SELECTION]` to `[THEOREM]` inside
the FTD axioms.
