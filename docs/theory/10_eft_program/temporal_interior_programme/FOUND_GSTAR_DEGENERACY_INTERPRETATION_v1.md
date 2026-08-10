# FOUNDATION — What G\* Is the Constant Of: the Degeneracy Where Timekeeping and Steerability Stop Trading

**Status:** `[DERIVED — FOUR EXACT IDENTITIES, MACHINE-VERIFIED]` +
`[INTERPRETATION — NOT A RECOVERY; NO VALID CARRIER HAS PRODUCED G*]` +
`[BOOKED — FTD-0817]`
**Date:** 2026-08-08 · **Artifact:** `scripts/experiments/temporal_interior/derive_gstar_degeneracy_price.py`
**Parents:** `DERIV_CARRIER_CONSTITUENTS_ONE_ENERGY_v1.md` §5 (the
stability/non-isochrony tension this explains),
`ANALYSIS_SOLITON_SHAPE_MODE_DILATION_v1.md` (the soliton shape-mode carrier, isochronous).
**Origin:** the owner's reading that G\* "has its cake and eats it too,"
and is "almost an information/context coefficient."
**Production impact:** none. No constant is changed; no tag moves. This
interprets G\*; it does not promote any claim about it.

> ⚠ **PRIOR ART — read first.** `dissemination/papers/edge_clock/`
> ("The Clock at the Edge of Stability", draft v2, 2026-08-05) already
> establishes the marginal-stability reading of G\*, and this document was
> drafted without consulting it. Already there, and **not** contributions
> of this note: the quartic as the clock of marginal stability; the period
> law $T = \sqrt\pi G^*\sqrt{m/2\lambda}/A$; the anti-pendulum framing;
> $G^* = 2\varpi/\sqrt\pi$ with $\sqrt\pi G^*$ the lemniscate's arc length;
> the threshold normal form; the dimensionless functionals
> $\mathcal{B}_4 = 48\pi/G^{*4}$, $\langle x^2\rangle = 4/G^{*2}$,
> $\langle x'^2\rangle = G^{*2}/6\pi$; $E_n \propto n^{4/3}$; the
> two-spring apparatus recovering G\* to $0.3\%$; and the statement that
> the difficulty is *staying* at the critical point.
>
> §2 of this note is **corrected** against it (see §2a), and what remains
> genuinely new is listed in §8.

---

## 1. The question this answers

The corpus has long recorded "π native, G\* priced." The programme could
say what the price *was* — a lemniscatic rather than harmonic clock — but
not what it *bought*. A bare cost with nothing purchased is an unstable
thing to leave in a ledger.

Separately, `DERIV_CARRIER_CONSTITUENTS_ONE_ENERGY_v1.md` §5 found a
structural tension: every stable configuration has a locally quadratic
minimum, hence a harmonic internal mode, hence π; the G\* law needs the
quadratic term to *vanish*, which is a degenerate direction stabilized at
fourth order.

Both are the same fact seen from two sides, and naming it says what the
price buys.

## 2. Minimality: quartic is the least degeneracy that works

For $V = \lambda q^n$ with $n$ even, the period obeys
$T \propto A^{1-n/2}$ with constant $\tfrac1n B(\tfrac1n,\tfrac12)$:

| $n$ | 2nd-order stiffness | oscillates | coefficients that must vanish | period constant |
|---|---|---|---|---|
| 2 | **nonzero** | yes | 0 | $\pi/2 = 1.570796327$ |
| **4** | **zero** | **yes** | **1** ($q^2$) | $\sqrt\pi G^*/4 = 1.311028777$ |
| 6 | zero | yes | 2 ($q^2, q^4$) | $1.214325324$ |
| 8 | zero | yes | 3 | $1.163592571$ |

Ordinarily the two properties exclude one another: a stiff mode oscillates
but resists being moved; a soft mode moves freely but does not oscillate.
**Every $n \ge 4$ escapes the dilemma — second-order free and still
oscillatory — and quartic is the minimal escape, requiring exactly one
vanishing coefficient where $n=6$ requires two.**

G\* is the period constant of that minimal degeneracy. Higher orders are
softer still but demand finer tuning; the quartic is what one vanishing
coefficient buys.

### 2a. Correction: the quartic is generic *at* the threshold

The table above invites a wrong reading, which an earlier draft of this
note made: that G\* is "priced because degeneracy is not generic."
`edge_clock` §2 shows the opposite where it matters. For a system with a
reflection symmetry, the normal form is
$V = \tfrac12\mu x^2 + \lambda x^4 + O(x^6)$, and **at $\mu = 0$ the
quartic is generic — the cubic is forbidden by symmetry, not removed by
tuning.** It is the normal form of a perfect Euler column at critical
load and of any order parameter at a continuous symmetry-breaking
transition at mean-field level. The quartic clock is not exotic; it is
*the universal clock of marginal stability*.

So the genericity has to be located precisely:

| | status |
|---|---|
| a generic point in configuration space | nondegenerate minimum $\Rightarrow$ harmonic $\Rightarrow$ $\pi$ |
| the threshold $\mu = 0$ | **codimension one** — not generic to sit on |
| the potential *given* $\mu=0$ and reflection symmetry | **quartic, generically** — no tuning of the shape |

**The price is therefore not shaping the potential. Symmetry does that for
free. The price is holding $\mu = 0$ against detuning** — which is exactly
what `edge_clock` §5 identifies as the experimental difficulty ("the
difficulty at any critical point is staying there"), and what its
two-spring geometry addresses only halfway, by killing the *odd* terms
identically while leaving $\mu$ to be maintained.

This sharpens rather than weakens §5 below: the cost of a G\* clock is a
*maintenance* cost, continuously paid, not a one-off construction cost.

## 3. G\* is a period, hence a coefficient of translation

$$\int_0^1\!\frac{du}{\sqrt{1-u^4}} = 1.311028777146
= \frac{\varpi}{2} = \frac{\sqrt\pi\,G^*}{4},
\qquad\text{so}\qquad G^* = \frac{2\varpi}{\sqrt\pi}.$$

All three agree to $10^{-13}$ and are asserted in the artifact. (Note
$G^* \ne \varpi$; they differ by $2/\sqrt\pi$, and the corpus is right to
keep them apart.)

This is a **period** in the Kontsevich–Zagier sense — an integral of an
algebraic function over an algebraic domain — and specifically a period of
the lemniscatic elliptic curve, the CM curve for $\mathbb{Z}[i]$.

That fixes the sense in which G\* is a *context* coefficient, and it is a
precise one. A period is an entry in the matrix relating de Rham to Betti
cohomology: two descriptions of the same curve. **A period is not a
property of an object; it is a property of the translation between two
descriptions of it.** This is also why periods sit exactly on the
modulus/argument frontier (`FOUND_MODULUS_ARGUMENT_FRONTIER.md`,
FTD-0336) — they are where forced algebraic structure meets imported
transcendental value, which is the same territory the FC-W/Chudnovsky
material occupies.

## 4. The price of steerability, and the exchange rate

Fix mass, displacement and period. The energy to hold the mode displaced:

$$\frac{E\,T^2}{mA^2} = 2\pi^2 = 19.739209 \ \ \text{(harmonic)},
\qquad \frac{\pi}{2}G^{*2} = 13.750372 \ \ \text{(quartic)} .$$

The stiffness cancels from both — $k$ and $\lambda$ are each fixed by $T$ —
so the ratio is pure G\*:

$$\boxed{\ \frac{E_{\rm quartic}}{E_{\rm harmonic}}
= \frac{G^{*2}}{4\pi} = 0.696602\ }$$

A G\* mode costs **30.3 % less** to hold displaced than a π mode at the
same mass, displacement and period.

**And 30 % understates it,** because the two scale differently. At
displacement $a$ the ratio is $(a/A)^2\,G^{*2}/4\pi$:

| $a/A$ | 1.0 | 0.5 | 0.2 | 0.1 | 0.01 |
|---|---|---|---|---|---|
| times cheaper | 1.4 | 5.7 | 35.9 | **143.6** | 14355 |

A degenerate direction is **free to first order**, so the quartic mode is
arbitrarily cheaper as $a \to 0$. That is the actuator condition stated
exactly: a π-clock resists being moved; a G\*-clock does not.

## 5. The interpretation, stated once

> **G\* is the period constant of the minimal degeneracy at which
> timekeeping and steerability stop trading against each other, and
> $G^{*2}/4\pi$ is the exchange rate between them.**

This answers what the price buys. Rigidity and timekeeping are ordinarily
in tension: the stiffness that makes a mode a good clock is the same
stiffness that makes it expensive to move. At the quartic degeneracy that
tension is released — the mode both keeps time and is free to first order.

**The ledger closes.** `edge_clock` names the cost: staying at $\mu = 0$
against detuning, paid continuously. §4 names the return: a factor
$G^{*2}/4\pi$ at reference displacement, and $(a/A)^2 G^{*2}/4\pi$ —
unboundedly favourable — for small excursions. A system that pays the
maintenance cost of sitting on the threshold is repaid in the cheapness of
being moved. π is native because sitting *off* the threshold is generic;
G\* is priced because sitting *on* it must be actively held.

It also explains the standing structural tension rather than merely
recording it. A carrier that is *only* required to be stable will be
harmonic, because stability alone selects a point off the threshold. A
carrier that must also be reconfigurable at low cost is pushed onto the
threshold — and that is exactly where G\* lives.

**This is where the agency reading acquires a mechanism, and stops being
an analogy.** An agent, in the thermodynamic sense the corpus already uses,
is a system that spends free energy to hold a configuration against drift.
Holding $\mu = 0$ *is* that, exactly and quantitatively: an unmaintained
system falls off the threshold and becomes a harmonic clock. So the
question "is the G\* clock the clock of an agent?" has a sharp form — not
"do agents have G\* clocks," but **a G\* clock is one that must be actively
maintained, and what the maintenance buys is cheap actuation.** That is a
statement about control cost, and it neither requires nor supplies
anything about consciousness (§6).

## 6. Scope, stated plainly

**This interprets G\*; it does not recover it.** No valid carrier has
produced G\*:

| where G\* appears | status |
|---|---|
| MVC ringdown, $2\times10^{-6}$ | measured, but the MVC is a mechanical framework with a distance potential — Galilean, and disqualified as a dynamical carrier (`ANALYSIS_COMPOSITE_CLOCK_DILATION_v1.md`) |
| quartic oscillator, 10 digits | **mathematics**: a theorem about $\int_0^1(1-u^4)^{-1/2}du$. Any quartic oscillator anywhere gives it |
| $\varphi^4$ soliton shape-mode carrier | **isochronous** — a π-clock |

**Three readings that do not follow.**

*That a G\* clock requires an agent, or that agents have G\* clocks.* §4 is
a **selection** argument: it says the quartic mode is the efficient choice
for a system that must be both timekeeper and actuator. It derives
nothing about what does the choosing.

*That this is consciousness.* The property is **marginal stability** —
persistence together with reconfigurability. That is a genuine hallmark of
adaptive systems (neural criticality, cytoskeletal tensegrity, protein
conformational landscapes), and it is also sandpiles, buckling columns and
every second-order phase transition. Far too general to constitute
consciousness; suggestive of the class, nowhere near sufficient for the
member. The corpus's decision to drop qualia commitments from its
vocabulary (`REF_REFERENCE_FRAME_VOCABULARY.md`) stands.

*That G\* is an information coefficient.* No established sense supports
this. Periods are not information-theoretic quantities. The **context**
half of that intuition is defensible and made precise in §3; the
information half is not, and is dropped rather than stretched.

## 7. What is actually new here

Against `edge_clock` (see the banner above), this note contributes four
things and no more:

1. **The steerability price.** $E T^2/(mA^2) = 2\pi^2$ versus
   $(\pi/2)G^{*2}$, ratio $G^{*2}/4\pi = 0.696602$, with the stiffness
   cancelling. `edge_clock` gives several dimensionless $G^*$ functionals
   ($\mathcal{B}_4$, $\langle x^2\rangle$, $\langle x'^2\rangle$) but not
   this one, and none of them is a cost.
2. **The small-displacement scaling** $(a/A)^2 G^{*2}/4\pi$, which makes
   the advantage unbounded rather than $30\%$.
3. **The period-as-comparison-coefficient reading** (§3). `edge_clock`
   gives $G^* = 2\varpi/\sqrt\pi$ and the lemniscatic-arc-length
   interpretation; the de Rham/Betti framing, and the placement on the
   modulus/argument frontier, are added here.
4. **The link to the carrier programme**: that a one-energy carrier
   selected only for stability is harmonic, so the temporal-interior
   clock gate and the edge-clock threshold are the same condition.

The marginal-stability reading of G\* itself is `edge_clock`'s, not this
note's, and the correction in §2a is owed to it.

## 8. Reproduction

```
python scripts/experiments/temporal_interior/derive_gstar_degeneracy_price.py
```

Seconds; closed-form throughout. The run asserts the $\varpi$ identity,
the $G^* = 2\varpi/\sqrt\pi$ relation, both $E\,T^2/(mA^2)$ values and the
$G^{*2}/4\pi$ ratio, so a future edit that breaks any of them fails loudly.
