# Audit — does the engine's transaction structure realise the `C₄` cycle? A pre-registered census

**Claim id:** FTD-1030 (drafted, unbooked)
**Verdict:** `[MEASURED — PRE-REGISTERED]` → **`[CLOSED NEGATIVE]`: the legacy engine law does
not realise the four-phase oriented cycle** `+1 → 0↓ → −1 → 0↑ → +1` at the site level.
Gates T1 and T3 fail by three orders of magnitude and by a majority respectively.
**Tags moved:** none. Sharpens FTD-1027 §F5 and gives FTD-1028's retirement path a
quantitative target.
**Lock:** `PREREG_C4_TRANSACTION_CENSUS_v1.md` sha256[:16] `4A4CD523A3CB0CE5` (recorded
before the run). **Instrument:** `engine/tests/test_c4_transaction_census.cpp`
(CTest `c4_transaction_census`, CPU backend, default toggles, `L = 16`, 4000 ticks, the
FTD-1027 deterministic seed, nothing tuned).
**Date:** 2026-09-04

---

## 1. The claim, as engine-checkable statements

The theory reads a manifestation transaction as a `C₄` cycle through an oriented null.
Its content at the level of site-readout transitions is: **(A)** a polarity reversal is
`+s → 0 → −s`, never `+s → −s` in one step — the premise that generates `C₄` and hence
`G*`; **(B)** the null is a passage, not a bounce — a site entering `0` from `+s` exits to
`−s`; **(C)** the cycle has a characteristic period (4, or 8 with the half-step).

## 2. Result

**D1 — transition census** (site-level rows over 4000 ticks):

| transition | count |
|---|---|
| `+→0` | 258,539 |
| `0→+` | 259,133 |
| `−→0` | 38 |
| `0→−` | 15 |
| `+→−` in place (null skipped) | 42 |
| `−→+` in place (null skipped) | 19 |

**T1 — passage or bounce** (gate: reversal fraction ≥ 0.5): of 255,052 reconstructed
nulls, **46** exit to the opposite sign (a `C₄` passage) and **255,006** exit to the same
sign (a bounce). Reversal fraction **1.8 × 10⁻⁴**. **FAIL.**

**T3 — do reversals pass through the null** (gate: skip fraction < 0.1): 61 in-place flips
against 46 via-null reversals. Skip fraction **0.570**. **FAIL.**

**T2 — characteristic period** (gate: χ² above the p<0.01 critical value on any series).
T2a, half-cycles: `n = 25`, underpowered, no verdict. T2b, positive null dwells,
`n = 253,002`: residues mod 4 `[61720, 64847, 63726, 62709]`, `χ² = 85.5` (crit 11.3);
mod 8 `[29969, 33784, 32724, 32334, 31751, 31063, 31002, 30375]`, `χ² = 360` (crit 18.5).
**Passes as declared** — and see §3.2 for why that pass should not be read as support.

**D2 — dwell histogram** (ticks): `0: 2050 · 1: 4500 · 2: 4253 · 3: 4276 · 4: 4376 ·
5–8: 18442 · 9–16: 35530 · 17+: 181625`; mean 53.0, CV 1.15. Broad and heavy-tailed; no
peak at 4 or 8.

**Verdict per the lock:** T1 fails and T3 fails ⇒ **C₄-NOT-REALISED.**

## 3. Reading

### 3.1 What the nulls actually are

`258,539` of the `258,577` site deaths are `+→0`, and `259,133` of the `259,148` births are
`0→+`. The run's lattice is overwhelmingly single-polarity, and its site-level nulls are
**transport gaps**: a `+1` record vacates a site (`+→0`) and a `+1` record later arrives
(`0→+`). That is `Movement` seen from the site, and it is a bounce by construction. The
`C₄` picture's null is an oriented *reversal* passage; the engine's null is an *interval
between occupants of the same sign*. They are different objects that share a readout
symbol.

### 3.2 The T2 pass is real, small, and the wrong shape

The gate was declared as "any non-uniformity at p<0.01," and with `n = 253,002` it fires
on a ~2.5 % deviation. That is a **design gap in the lock** — a one-sided hypothesis
(excess at residue 0, the multiples of the period) should have been declared — and is
recorded here as such rather than quietly narrowed after the fact. Read one-sidedly, the
data cut *against* the theory: residue `0 mod 8` is the **least** populated bin (29,969
against a mean of 31,625), and residue `1` the most. A `C₄`/`√i` clock predicts an excess
at 0. The small structure that exists is consistent with integer-hop transport kinematics
(`phase_movement` jumps on `|remainder| ≥ 1`), not with a four- or eight-stage clock.
T2 therefore contributes no support to the theory in this run.

### 3.3 Reversal without a carrier

Among the 107 genuine polarity reversals, the majority (61) are `WeakTransmutation`'s
in-place sign flip, which never visits the null — FTD-1027 §F5, now quantified. The
minority (46) do pass through `0`, but as *site* events: the `+s` record's `particle_id`
is destroyed at `+→0` and a fresh id is minted at `0→−s`. **No record in this engine
survives a passage through the null.** The `C₄` cycle, as a cycle *of one thing* through
zero, has no carrier in the legacy law; at best it is a statistical coincidence of two
unrelated records at one site.

### 3.4 What this closes and what it does not

Closed: "the legacy engine realises `C₄`." This was the pre-declared expectation (lock
§D0.4) given FTD-1028's finding that the engine probes a legacy `Φ`, and it is now a
measured fact rather than an inference from the schedule listing.

Not touched: the algebraic content (`G*` from the quarter-sector determinant,
Theorem-grade regardless of any engine), the `C₄` reading of v3's constitution (whose
selected expiry acts on oriented `(normal, hand)` presentations — an oriented passage by
design), and the question of whether a v3-native `Φ` realises the cycle.

### 3.5 What it gives FTD-1028 — criterion **corrected by §5**

The retirement path there was "implement `hodge_expiry`, rerun, compare." The first
draft of this audit proposed the acceptance criterion `T1 ≥ 0.5, T3 < 0.1` — i.e. that a
candidate `Φ` must realise *polarity reversal through an oriented null*. **§5 shows the
constitution's own law does not do that and is not meant to.** The corrected criterion,
read off `Φ v2`'s certified relation, is: an exact period-8 site square wave with 4/8
duty; the carrier's `C₄` orbit conserved across every null; polarity conserved by every
map; and no in-place sign flip. The legacy engine fails all four; the census instrument
measures all four.

## 5. The same questions asked of the constitution's own law — `[MEASURED, EXACT]`

"If the legacy engine fails, test the engine with the proper physics." The proper physics
exists as a **certified relation, not an engine**: the register's `executable_reference`
for P5 is `scripts/proofs/proof_v3_common_action_phi_v2.py` (31/31 checks, 39 s), which
defines the cell law exactly and has no lattice driver. The census questions were
therefore asked of that law at the level where it exists — the isolated primary/reserve
relation — exhaustively over all eight payloads
(`scripts/proofs/proof_phi_v2_isolated_relation_census.py`, 8/8):

- The record is a Gaussian integer `z ∈ A9 = {−1,0,1}²`. `readout` gives occupancy
  (`BLANK` = 0), **polarity** (+1 on the four axis cells, −1 on the four diagonals), and a
  **phase** (four per polarity). `rotate(z) = iz` — the `C₄` — **preserves polarity and
  advances phase** (C11).
- Every payload has **exact period 8** and **duty 4/8**: the primary readout is the square
  wave `++++0000` (or `----0000`) at the starting phase's offset.
- **Every null is a bounce** (`+s → 0 → +s`); zero reversal passages. The null is the token
  *residing in the reserve*, not an annihilation.
- The **carrier persists**: the token's `C₄` orbit is constant across every null.
- **Polarity is conserved by every map**: by `rotate` (C11), by `relation_tick`, and by the
  55,008-row collision table, whose 192 states are `(Hodge flag, phase)` with no polarity
  variable at all (polarity is a separate copy label, C8). Polarity reversal **does not
  occur in `Φ v2`**.
- The external gate controls **transport** (the crossing at phase 0), not the clock: an
  alternating gate leaves the period-8 wave intact; a closed gate holds the readout at 0
  while the token keeps rotating (C17).

**Reading.** The `C₄` of the constitution is a *phase* clock living in the part of the record
the ternary readout quotients away (P3), and it appears at the readout as a period-8
square wave — the eighth root, the square wave, and the persistent carrier of the
2026-09-04 discussion, now certified rather than conjectured. The derivation's §1 premise
— *polarity* reversal through an *oriented* null — has **no referent** in the constitution's
law: `Φ v2` never reverses polarity, and its nulls are transport residences. The census
instrument's gates T1/T3 encoded the derivation's `C₄`, not the constitution's; §3.5 is
corrected accordingly.

**Two constitution violations in the legacy engine, now separable.** FTD-1027 §F5 found the
engine's reversal skips the null. §5 shows something stronger: in `Φ v2` there is nothing
to skip — polarity is a conserved label. The engine's 61 `WeakTransmutation` flips violate
the constitution twice, by flipping a conserved quantity and by doing so in one step.

**What remains open, precisely.** Whether the period-8 square wave and carrier persistence
survive *composition* — many relations interacting through the collision layers on a
lattice with a conflict-free schedule. The isolated result is exact; the composed result
is the FTD-1028 lattice driver that does not yet exist, and the corrected criterion above
is what it must reproduce.

## 6. Reproduction

```
engine\build_native.bat build --target test_c4_transaction_census
cd engine\build && ctest -C Release -R c4_transaction -V
python scripts/proofs/proof_v3_common_action_phi_v2.py            # 31/31, ~40 s
python scripts/proofs/proof_phi_v2_isolated_relation_census.py    # 8/8, <1 s
```

The CTest exit status reports instrument validity, not the theory verdict; the lock
declares the negative an expected, informative outcome.
