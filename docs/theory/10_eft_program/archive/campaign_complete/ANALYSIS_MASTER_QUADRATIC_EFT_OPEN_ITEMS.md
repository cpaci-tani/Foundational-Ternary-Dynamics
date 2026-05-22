# Analysis: Master Quadratic Paper vs Native EFT Open Items

**Date:** 2026-04-24
**Source examined:** `docs/papers/PAPER_MASTER_QUADRATIC_FORMAL.pdf`
**Status:** [ANALYSIS] mapping document; not a new derivation

This note correlates the formal master-quadratic paper with the current native
EFT checklist. It is intentionally strict: the paper proves a clean arithmetic
chain, but it does not by itself close the native EFT action/measure,
nonlinear flow, Gauss-representation, or `g_c` open items.

---

## 1. What The Paper Actually Establishes

The formal paper separates four layers:

1. **[THEOREM] BCC Watson/reflection identity**

```text
W_BCC = Gamma(1/4)^4 / (4 pi^3) = G_*^2 / (2 pi)
```

where the BCC Watson denominator is:

```text
1 - cos(k_x) cos(k_y) cos(k_z)
```

2. **[DEFINITION] FTD capacity normalization**

```text
K = |Aut_C(E,0)|^2 (2 pi) W_BCC
  = 16 (2 pi) W_BCC
  = 16 G_*^2
```

with the CM curve:

```text
E: y^2 = x^3 - x
|Aut_C(E,0)| = 4
```

3. **[DEFINITION] filtered product rule**

```text
e_1 = K
e_2 = G_* K
```

4. **[THEOREM from definitions] master quadratic**

```text
x^2 - 16 G_*^2 x + 16 G_*^3 = 0
```

The physical identifications remain:

```text
x_+ ~= alpha(0)^-1     [SELECTION / OPEN MATCHING]
x_- ~= N_c             [SELECTION / OPEN MATCHING]
```

The paper is therefore not an alpha derivation. It is a rigorous arithmetic
construction plus a sharply stated physical matching problem.

---

## 2. Main EFT Correlation

The paper provides a candidate **arithmetic kernel** for a native sector:

```text
G_BCC(k) = 1 / (1 - cos k_x cos k_y cos k_z)
```

This is not the current production linear generator, which uses the selected
G18 operator:

```text
sigma_18(k) =
  4
  - (2/3)(cos kx + cos ky + cos kz)
  - (2/3)(cos kx cos ky + cos kx cos kz + cos ky cos kz)
```

So the correlation is:

```text
G18      -> current one-tick direct longitudinal native response
BCC      -> Moore-corner / Stella Octangula / delayed k=3 shell candidate
CM factor -> arithmetic normalization of the BCC/corner sector
Vieta    -> trace/product invariant proposal for a future native mixing matrix
```

This distinction matters. The master quadratic cannot be dropped directly into
the G18 Gaussian generator without an extra selection rule. The honest route is
to test whether BCC appears as a measured nonlinear/delayed sector of the full
engine histories.

---

## 3. Correlation With Open Items

| Open item | How the paper helps | What remains open |
|---|---|---|
| Native action/measure | Supplies a precise BCC Green kernel that could appear in a history measure or transfer operator. | Derive why the engine path measure uses the BCC denominator rather than, or in addition to, G18. |
| Canonical field basis | Points to a Moore-corner/BCC sector, not just cell-centered flux. | Decide whether the canonical basis includes a delayed corner-shell field/channel. |
| Production Gauss representation | Gives an alternate corner-shell Green structure. | Current native electrodynamics selects G18 as direct Gauss; need a timing/action principle to promote BCC. |
| Nonlinear operator flow | The trace/product data `(K, G_* K)` suggest measurable invariants of a 2-mode or 2-sector mixing matrix. | Build the measured nonlinear mixing matrix and test its trace/determinant against these values. |
| Reaction/current ledger | Full-tick GPU ledgers now make BCC/corner-channel history tests possible. | Define BCC-sensitive observables from histories: corner transport, delayed layer response, and reaction-corner coupling. |
| `g_c` first principles | Shows that a dimensionless capacity normalization exists without CODATA. | Does not derive engine `g_c`; at best it suggests a capacity-matching condition to test. |
| Physical alpha matching | Gives a mathematically exact `x_+` near alpha inverse. | Still needs independent physical normalization and correction/matching terms. |
| Color/N_c matching | Gives a smaller root near 3. | Still needs dynamical confinement/rank selection from engine histories. |

---

## 4. Best New Hypotheses To Test

These are legitimate consequences to investigate. They are not current
theorems.

### Guardrail: one ontic unit has capacity, not self-mixing

The BCC/stella shell of one ontic unit should not be described as something
that "mixes itself." For a single unit, a bilinear such as:

```text
J * J
```

is first a self-capacity, norm, or Hodge-paired quadratic form. It becomes a
mixing object only after we introduce a composite basis:

```text
multiple ontic bodies
multiple history sectors
multiple blocked cells
or a relational split such as T+ / T- observed through interactions
```

The cognitive analogy is useful: a single mind can have an introspective
capacity, but actual mixing requires distinguishable contents, sub-processes,
or multiple agents. Likewise, one stella-octangula unit supplies the local
two-tetrahedron capacity structure; many units or many histories supply the
operator mixing matrix.

### Hypothesis A: BCC as delayed Moore-layer sector

Existing native electrodynamics already states:

```text
G18 is the direct one-tick response
BCC/corner shell is natural as a delayed k=3 Moore layer
```

The paper strengthens this by showing the BCC Green integral has the exact
reflection bridge:

```text
W_BCC = G_*^2 / (2 pi)
```

Test:

```text
Measure full-tick GPU histories over 3-tick windows.
Extract corner-shell / diagonal transport observables.
Block them by b=2.
Ask whether their Green/capacity estimate tends toward W_BCC.
```

### Hypothesis B: trace/determinant invariant of operator mixing

The master quadratic is equivalent to:

```text
trace = K = 16 G_*^2
det   = G_* K = 16 G_*^3
```

A native nonlinear flow campaign can produce an operator mixing matrix `M`.
The falsifiable question is:

```text
Does any pre-registered multi-body or multi-history 2-sector BCC/native
mixing block have
tr(M) ~= 16 G_*^2 and det(M) ~= 16 G_*^3
after declared normalization?
```

This must be pre-registered before measuring. Otherwise it becomes forbidden
near-miss hunting.

For a single ontic unit, the corresponding object should be called a capacity
matrix or local quadratic form, not a Wilsonian mixing matrix.

### Hypothesis C: capacity matching for `g_c`

The `g_c` open problem asks why the charge-flux coupling has its value. The
paper suggests a possible native condition:

```text
charge coupling is fixed by matching direct G18 capacity to delayed BCC/CM capacity
```

But the paper does not provide that equation. A legal next step is to derive a
symbolic matching condition from the engine's selected operators, then test it.
It is not legal to plug alpha into `g_c` and call that derivation.

---

## 5. Checklist Updates Implied By The Paper

### Native degrees of freedom

- Add a candidate delayed BCC/corner-shell channel to the field-basis decision.
- Do not replace the G18 basis unless an independent timing/action principle is derived.

### Kinematics and constraints

- Treat BCC as a Moore-corner Green kernel candidate.
- Keep G18 as the current direct Gauss representation until a derivation selects otherwise.

### Nonlinear flow

- Add BCC-sensitive observables:

```text
corner transport count
diagonal/corner current l1
3-tick delayed corner response
reaction-corner covariance
blocked BCC capacity estimator
```

### Statistical measure/action

- Add a candidate BCC transfer kernel:

```text
T_BCC(k) or G_BCC(k) = 1 / (1 - cos k_x cos k_y cos k_z)
```

- Require a derivation from engine tick/history rules before using it in the canonical action.

### Couplings/calibrations

- Add "capacity matching from G18 to BCC/CM sector" as a new `g_c` candidate mechanism.
- Tag it [CONJECTURE] until derived.

---

## 6. Immediate Implementation Path

The highest-value next engine task is not to fit the master quadratic. It is to
measure whether the engine contains the BCC/corner channel in a native,
pre-registered way.

Proposed tests/campaigns:

1. **BCC kernel estimator**

```text
Construct a pure BCC/corner random-walk Green estimator on periodic L^3.
Verify numerical convergence to W_BCC.
No physics claims; just validates normalization and finite-L behavior.
```

2. **Engine corner-channel observable**

```text
From GPU full-tick ledgers, separate face/edge/corner transport paths.
Measure corner contribution over 1-tick and 3-tick windows.
```

3. **Blocked BCC capacity flow**

```text
Block the corner-channel histories with b=2.
Measure whether a stable BCC capacity estimator exists.
```

4. **Pre-registered 2-sector mixing matrix**

```text
Sector A: direct G18/source response.
Sector B: delayed BCC/corner response.
Measure M under b=2 blocking.
Only after M is defined, compare trace/determinant to the master-quadratic data.
```

---

## 7. Current Verdict

The paper correlates strongly with the open EFT program, but it closes none of
the remaining physics gates by itself.

It gives:

```text
exact BCC Watson kernel
exact G_* reflection bridge
exact CM-normalized capacity K
exact master quadratic from declared filtered Vieta data
```

It does not give:

```text
native path measure
engine-derived BCC action
production Gauss replacement
first-principles g_c
physical alpha derivation
color-rank derivation
```

The right next move is a BCC/corner-channel measurement program inside the
native EFT flow, using the full-tick GPU ledger we just built.
