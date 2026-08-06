# FTD-0787 — The Flexural Quartic Mechanism: C3 Realized v1

> ## REFUTED 2026-08-03 — see FTD-0789
>
> **The verdict `C3_REALIZED_NATIVELY` is WITHDRAWN.** The polarity mask makes
> `A_AC = 0` identically, so `U` depends only on the two bond lengths and the
> **bend angle is an exact flat direction** (verified to machine zero at every
> angle). The transverse path used below stretches both bonds to
> `sqrt(1+d^2)`; the system reaches the same offset by bending at constant
> bond length for **zero energy**. The `24 eps d^4` is the curvature of a
> rectilinear chord across a flat valley, not a confining potential. There is
> no quartic, no barrier, no separatrix, no frequency, and no hardening mode.
> C3 is **not** realized and FTD-0783's bracket corollary is **restored**
> (this document's §6 scope amendment is withdrawn).
>
> The symbolic algebra in §3 is exact and every number reproduces; §9's
> stretch-mode labels are swapped (see FTD-0789 §5). Retained in full as a
> documented negative result, per house practice.


**Status:** `[THEOREM — EXACT NULL-FLAT QUARTIC FROM ZERO-TENSION TRANSVERSE
GEOMETRY]` + `[DERIVED — FIRST NATIVE HARDENING MODE]` +
`[MEASURED CONTROL — STATIC, SELECTED COMPACT LAW]` +
`[CLOSED NEGATIVE — C2 AT THE SELECTED `epsilon`]` +
`[SCOPE CORRECTION — FTD-0783's BRACKET COROLLARY]`
**Verdict:** `C3_REALIZED_NATIVELY_C2_FAILS_BY_SCALE_SEPARATION`
**Parents:** `FTD-0739`, `FTD-0783`, `FTD-0786`, `SPEC_CARRIER_CONSTRAINTS_v1`
**Production impact:** none; exact algebra plus quadrature, no engine execution

## 1. Result in one sentence

The transverse (flexural) displacement of a collinear trimer bound by the
**registered compact law** has an **exactly null-flat quartic potential**,
`V = -2 epsilon + 24 epsilon d^4 - 32 epsilon d^6`, with the quadratic term
vanishing by *geometry* rather than tuning — so **C3, the constraint with "no
known native realization," is realized, with no new primitive** — and the
resulting mode is the program's **first hardening mode** (`dOmega/dA = +1.017`);
but it still fails **C2**, reaching only `Omega_max = 0.4007` against a band
top of `1.2310`, and the shortfall is a *scale separation* between matter
binding and field stiffness rather than a defect of shape.

## 2. The mechanism

Two registered facts, and nothing else:

1. **The bond is untensioned at its minimum.** For
   `V(q) = -16 eps (q-3/2)^2 (q-3/4)`, `q = r^2`, one has `V(1) = -eps`,
   `V'(1) = 0`, `V''(1) = 96 eps = k_bond`. Equilibrium separation `r0 = 1`
   carries **zero tension**.
2. **Transverse displacement changes length at second order.** Offsetting a
   bonded constituent by `d` perpendicular to a bond of natural length `r0`
   gives `l = sqrt(r0^2 + d^2)`, hence `delta_l = d^2/(2 r0) + O(d^4)`.

Since the energy is quadratic in `delta_l` near an untensioned minimum, it is
**quartic in `d`**:

```text
V ~ (k/2)(delta_l)^2 = (k/8 r0^2) d^4 + ... ,      V''(0) = 0 exactly.
```

Nothing is selected to make the quadratic term vanish. It vanishes because a
bond at its own minimum exerts no restoring force along a direction that does
not change its length to first order. **This is the general mechanism; the
compact law merely instantiates it.**

## 3. The exact potential

Take a collinear trimer `A—B—C` at spacing `r0 = 1`. The outer pair sits at
separation 2, i.e. `q = 4 > 3/2`, **outside the compact support** — so `A` and
`C` do not interact and the configuration is an exact equilibrium of the
registered law with exactly two bonds. Displacing `B` transversely with the
centre of mass held fixed gives bond offset `d = 3 y_B/2` and effective mass
`m_eff = 2m/3`. Then `q = 1 + d^2` **exactly**, and

```text
V(d) = 2 V(q = 1 + d^2) = -2 eps + 24 eps d^4 - 32 eps d^6      (exact, finite)
```

with no truncation. Consequences, all exact:

| feature | value |
|---|---|
| quadratic coefficient | **0** |
| quartic coefficient | `24 eps` |
| sextic coefficient | `-32 eps` |
| separatrix | `d = 1/sqrt(2)`, which is *exactly* bond dissociation `l = sqrt(3/2)` |
| barrier height | `2 eps` |
| sextic/quartic contamination | `(4/3) d^2` — **independent of `eps`** |

## 4. The first native hardening mode

Because the law is quartic at the bottom, `Omega ∝ A` there: measured
`dOmega/dA = +1.0166` at small amplitude. **No previously registered native
mechanism hardens** — FTD-0783's pair softens everywhere, FTD-0781's affine
sector has zero anharmonicity, and FTD-0786's doublet is a linear mode with
amplitude-independent frequency. This is the first.

The hardening is nonetheless **bounded**: the negative sextic and the
separatrix turn the curve over. By quadrature (turning-point singularity
removed exactly via `d = A sin(theta)`):

```text
Omega(A) rises to Omega_max = 0.400745  at  A = 0.552524  (78.1% of separatrix)
then falls to zero at the separatrix.
```

## 5. C2: the gate, and the honest failure

```text
Omega_max = 0.400745
field one-axis band top  2 arcsin(1/sqrt 3) = 1.230959   ratio 0.3256  FAIL
acoustic/wave band top                     = 2.000000    ratio 0.2004  FAIL
```

The mode is **in band at every amplitude**. C2 fails, and C6 fails with it.

**But the failure is parametric, not structural.** `Omega ∝ sqrt(eps/m)` while
the purity boundary is `eps`-independent (both potential coefficients scale
with `eps`, so their ratio does not). Raising `eps` therefore lifts the
frequency *without* shrinking the clean-quartic window — the two windows can
be made to overlap. Requiring the mode to clear the band while still inside
the 10%-contamination window (`d < 0.2739`) gives the sharp condition

```text
eps > 0.2218   (field band, 22.2x the selected value)
eps > 0.5856   (wave band,  58.6x the selected value)
```

`eps = 0.01` is registered as **selected, not derived**
(`DERIV_MINIMAL_MANY_BODY_MATTER_NETWORK_v1.md` §5: "The selected well depth
is `epsilon=0.01`"). So C2 is no longer a structural wall here; it is a
numerical condition on one selected parameter.

**The physical reading.** `eps/C_WAVE^2 = 0.03`: matter binding sits about two
orders of magnitude below field stiffness. C2 demands they be *comparable*.
The obstruction to a native `G*` carrier is therefore an **energy-scale
separation between the matter and field sectors**, not the shape of any
potential. That is a different and more tractable problem than the one the
program has been attacking.

## 6. Scope correction to FTD-0783

FTD-0783's bracket corollary — "wells pin candidates at `n ~ 2`" — is **true
for the coordinate the bond stiffness acts on** (the radial/longitudinal one)
and its mathematics is unaffected. It does **not** hold for all coordinates of
all wells: a transverse coordinate of an untensioned bond is pinned at
`n = 4`, not `n = 2`. The corollary's reach — "`G*` lives at `n = 4` ... where
no identified native mechanism produces a potential" — is **falsified by
exhibition** here. FTD-0783's three kills of the *pair breathing* channel
stand; only the universality of its corollary is narrowed.

## 7. Scorecard against SPEC_CARRIER_CONSTRAINTS_v1

| | constraint | verdict |
|---|---|---|
| C1 | nonlinear conservative sector | PASS — exact quartic+sextic |
| C2 | spectrum avoidance | **FAIL** — `0.4007` vs `1.2310`, 3.07x short |
| C3 | intermediate-exponent confinement | **PASS — exact, no tuning** |
| C4 | localization / stress closure | PASS*, virial not checked |
| C5 | drain-free amplitude window | OPEN — `\|J\|` vs `K_GENESIS` not evaluated |
| C6 | persistence | FAIL — consequential on C2 |
| C7 | not a linear functional | PASS — matter-position coordinate |
| C8 | natural-coordinate closure | PASS — exact 1-D normal-mode reduction |
| C9 | recurrence | PASS — bound oscillation by construction |
| C10 | fixed support | PASS — three declared constituents |
| C11 | native licensing | PASS — registered compact law only |
| C12 | preregistration | N/A — no campaign run |

**8 of 12 pass** — the best score any registered candidate has achieved, and
the first to pass C3.

## 8. The signature a campaign would test

Inside the purity window the occupancy is the exact `n = 4` law, so Theorem B
gives falsifiable numbers on `d(t)`:

```text
<x^2> = 4/G*^2      = 0.456946581
<|x|> = sqrt(pi)/G* = 0.599070117
G* = 2/sqrt(<x^2>)  = 2.958675119
```

The 2%-contamination window `d < 0.1225` is the clean target.

## 9. What is NOT claimed

- **No carrier.** The mode fails C2 at the registered parameters; it is not a
  clock, and nothing here promotes it.
- **`eps` is not free in isolation.** Raising it by 22x rescales the whole
  matter sector; the same derivation's §5 already flags an `m^2`-collapse
  concern for unrestricted pairwise promotion. Whether any consistent `eps`
  satisfies C2 *and* the rest of the matter program is **OPEN** and is the
  natural successor question.
- **The trimer has in-band modes.** Longitudinal antisymmetric
  `Omega = 0.9798` (in band) and symmetric `Omega = 1.6971` (above the field
  band, inside the wave band). The transverse channel is odd under transverse
  reflection while the longitudinal ones are even, so linear coupling vanishes
  by symmetry — nonlinear coupling does not. Whether that suffices is `[OPEN]`.
- **Static analysis only.** No engine execution, no field dressing, no
  ternary-capacity admissibility check for the collinear geometry, no
  stability against the full coupled dynamics.
