# ANALYSIS — Protonucleus continued growth: result of the locked v1 campaign

**Status:** `[ENGINE FACT — MEASURED]` + `[CONFIRMED PREDICTION — NUCLEATION
THRESHOLD]` + `[CLOSED NEGATIVE — NO DERIVED BODY SIZE]`
**Verdict:** `SATURATION_BY_POLARITY_DILUTION_AT_A_SEED_DEPENDENT_SIZE`
**Protocol:** `../preregistrations/PREREG_PROTONUCLEUS_GROWTH_v1.md`, locked
2026-08-03 23:31 before execution. Executed 2026-08-04.
**Runner:** `scripts/experiments/protonucleus_growth.py`
**Raw:** `scripts/experiments/recorded_results/ftd_0799/protonucleus_growth_results.json`
(arms A–D),
`scripts/experiments/recorded_results/ftd_0799/protonucleus_controls_t2.json`
(T2 lattice-size + held-out-seed controls)
**Production impact:** none. Standalone simulation; no engine constant, toggle,
scenario, or golden state changed.

---

## 1 · What was asked

Does a manifested body above the quasi-static critical radius `R_c = 12.63`
grow without bound, saturate at a preferred size, or collapse? The quasi-static
calculation has a uniform manifested ball radiating from its surface with
`|J| ~ 0.1212 R - 0.0141`, crossing `K_GENESIS = 1.5163860592` at `R_c = 12.63`.
Above `R_c` genesis can fire at the surface, adding a layer, raising `R`, which
raises `|J|` — a self-reinforcing accretion instability. Genesis is also a
dissipative drain (FTD-0567), so each layer costs. Whether growth pays for
itself was undecided.

## 2 · Execution and validity

Four of the five declared arms were run (A, B, C, D). **Arm E (`R0 = 20`,
`L = 201`, 8.1M sites) was NOT run** — deferred for runtime. This is recorded
as an incomplete arm, not a silent omission; arm E is deep super-critical and
was declared as confirmation, not as a discriminator.

| arm | `R0` | `L` | `N0` | `N_final` | growth | `R_eff` start → end | last `dN` | flat for | final `max\|J\|` |
|---|---|---|---|---|---|---|---|---|---|
| A | 8  | 129 | 2,109  | 2,109  | 0.00%  | 7.955 → 7.955   | t=0   | 599 ticks | 0.6666 |
| B | 12 | 129 | 7,153  | 7,153  | 0.00%  | 11.953 → 11.953 | t=0   | 599 ticks | 0.9884 |
| C | 13 | 129 | 9,171  | 9,539  | +4.01% | 12.985 → 13.156 | t=75  | 524 ticks | 1.1145 |
| D | 16 | 161 | 17,077 | 30,167 | +76.65%| 15.975 → 19.311 | t=200 | 399 ticks | 1.2828 |

**All three kill conditions clear:**

1. Arm A (sub-critical) **did not grow** — the run is valid and does not
   contradict FTD-0586's registered no-go.
2. Arm C (just super-critical) **did grow** — `R_c` is not falsified.
3. **Charge is exactly conserved in every arm**, `q(t) = q(0) = N0` at every
   logged tick.

> **Correction to the prereg, §4.** It requires net charge "must remain 0 —
> pairs", but §3 seeds *a uniform ball of `s = +1`*, so charge starts at `N0`,
> not 0. The invariant that actually holds — and that the data satisfies
> exactly — is **conservation at the seed value**: genesis adds one `-1` and
> one `+1` per event, leaving `sum(s)` fixed while the *count* rises. The
> lock is respected; the defect is recorded here rather than edited into the
> locked file.

**Controls (T2, run before the main campaign).** The `L = 161` lattice control
reproduces arm D **exactly to the digit** (`N = 30,167`, `R_eff = 19.311`,
`max|J| = 1.2828`), confirming determinism. The held-out RNG seed 7 gives
`N = 30,157` — a **0.03%** difference. The arrest point is therefore robust to
both lattice size and RNG stream.

## 3 · Outcome, against the prereg's own decision rule

§5 defines `SATURATION` as *"`N` converges to `N*` with `|dN/dt| < 0.1%/tick`
sustained over 100 ticks."*

**Every arm meets it, and far more strongly than required: `dN/dt` is
*exactly zero* for the final 399–599 ticks.** The registered outcome is
`SATURATION`. `RUNAWAY` and `COLLAPSE` are both excluded; no arm reached the
40%-of-volume boundary stop, and no arm lost sites.

**Positive result — the nucleation threshold is confirmed and bracketed.**
`R0 = 12` (below `R_c = 12.63`) produces **zero** growth; `R0 = 13` (above)
produces growth. The quasi-static critical radius survives its own falsifier
as a *nucleation* criterion.

## 4 · The negative that matters: there is no derived body size

§5 glosses `SATURATION` as *"the interesting outcome: **a derived body
size**."* **That reading is not supported.** The saturated size tracks the
seed:

| seed `R0` | 8 | 12 | 13 | 16 |
|---|---|---|---|---|
| saturated `N*` | 2,109 | 7,153 | 9,539 | 30,167 |
| saturated `R_eff` | 7.955 | 11.953 | 13.156 | 19.311 |

A derived body size requires super-critical seeds to converge on a **common**
`N*`. They do not — arm C arrests at 9,539 and arm D at 30,167, a factor of
3.2 apart. What the campaign measures is **arrest**, which is a property of the
initial configuration, not a preferred scale of the theory. **No derived
particle size, mass, or radius is licensed by this result.**

## 5 · Mechanism: growth is self-quenching by polarity dilution

Every arm arrests with `max|J|` **below** `K_GENESIS = 1.5164`
(0.667 / 0.988 / 1.115 / 1.283). Genesis simply stops having candidates. The
reason it stops is visible in the charge ledger:

- Arm D seeds 17,077 sites, all `s = +1` → net polarity fraction **1.000**.
- It ends with 30,167 sites and charge still 17,077 → net polarity fraction
  **0.566**.

Genesis deposits `-1` upstream and `+1` downstream, so **each event dilutes the
body's net polarity while conserving its charge**. The source that drives `|J|`
is net polarity, not site count. Growth therefore erodes its own driver: the
ball converts from uniformly polarized to mixed, the field falls, and firing
ceases. This is a negative feedback intrinsic to pair genesis, and it is what
defeats the accretion instability the quasi-static picture predicted.

**Consequence for the quasi-static law.** At arm D's arrest radius the law
predicts `|J| = 0.1212 x 19.311 - 0.0141 = 2.326`, well above `K_GENESIS`, so
it predicts continued firing. Measured `max|J|` is **1.283**, low by a factor
of **1.81**. The law is sound for the *initial* uniform ball — which is why it
gets `R_c` right — and wrong for the *post-genesis* mixed body, because it
assumes a uniform-polarity source that genesis has destroyed. **The quasi-static
picture is a valid nucleation criterion and an invalid growth law.**

## 6 · Scope — what this cannot show

Movement, curl coupling, evaporation, and Gauss projection are all **off**
(prereg §7). This is the frozen coupled wave + genesis profile only. Every
statement above is scoped to that profile.

**Evaporation is the most likely omitted stabiliser** and is deferred to v2.
Its absence cuts both ways here: it cannot have caused the arrest (it was
never enabled), but a profile with evaporation could arrest somewhere else
entirely. The polarity-dilution mechanism of §5 is structural rather than
parametric, so it should survive, but that is a prediction and not a result.

**Arm E is unrun.** Nothing here is claimed for `R0 = 20`.

## 7 · What follows

1. **The accretion instability is closed negative** in this profile. There is
   no runaway protonucleus.
2. **`R_c = 12.63` is confirmed as a nucleation threshold** — a genuine
   quasi-static prediction that survived a preregistered falsifier.
3. **No derived body size exists**, so this supplies no mass scale, no radius,
   and no particle candidate. It does not bear on the C3 carrier question.
4. The live follow-ups are arm E, a v2 profile with evaporation enabled, and a
   direct test of the §5 dilution mechanism (seed a *mixed*-polarity ball at
   super-critical radius and predict zero growth from the outset).
