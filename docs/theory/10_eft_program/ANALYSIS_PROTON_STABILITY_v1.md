# Proton-stability forcedness audit (FTD-0291) — run-of-record verdict

**Status:** `[MEASURED — UNFORCED-METASTABLE (frozen); boundary]`
**Date:** 2026-06-13
**Pre-registration:** `PREREG_PROTON_STABILITY_v1.md`, tag `preregister-proton-stability-v1`,
lock commit `bb99a20d`. Artifacts SHA256-bound (§3 of the pre-reg). Golden-neutral
(`render_bridge_golden = 0x56fa28acb5b9fe88`, green with the new TU).
**Verdict (frozen analyzer):** **UNFORCED-METASTABLE [BOUNDARY]** — `τ_p = ∞` is **not**
a forced prediction of FTD's discrete ontology.

---

## 1 · The question

`proof_complete_sm.py:460–471` tags the proton's infinite lifetime **`τ_p = ∞` as a
`[THEOREM]`** and calls it "the sharpest falsifiable prediction FTD makes" (the Standard
Model's accidental baryon stability; the divergence from grand-unified theories, which
predict decay at ~10³⁴ yr). It rests on four asserted props: (a) charge conservation is
exact; (b) the proton is a *locked triad* of three same-sign particles; (c) locked triads
are exempt from evaporation; (d) weak transmutation "flips polarity but **preserves triad
structure**", so there is "NO mechanism for baryon-number violation." This audit asked, by
the forcing-vs-fitting standard of FTD-0269/0290, whether any of that is *forced*.

## 2 · Two legs that need no engine

- **No forced baryon symmetry.** FTD's postulates contain exactly one exact vector charge —
  the U(1) `Σs` (the Gauss constraint) — and **no** baryon-number or B−L current. Baryon
  number is an *emergent* 3-quark-color-singlet cluster label, not a conserved quantity of
  the dynamics. `Σs` conservation does not forbid `p → e⁺ + π⁰` (the products are
  charge-balanced). Prop (a) is true but **does not protect the proton**; props (b)–(d)
  silently promote "baryon number" to a law the framework never derives — and contradict
  FTD's **own** baryogenesis sector, which invokes weak-transmutation B-violation
  (`DERIV_LATTICE_CHIRAL_ANOMALY.md §6`, `CHECKLIST_PHYSICS.md:534`).
- **The cited lock mechanism is structurally inapplicable (source fact).** `triad_binding`
  locks only **same-sign** triples — `transmutation_phases.cpp:148,153` skip any pair with
  `state != state`. The proton is `uud = (+1,+1,−1)`, **mixed sign**. So prop (b)'s "locked
  triad" and prop (c)'s evaporation exemption **can never apply to a real proton**. This is
  a code-level fact, not a measurement.

## 3 · Run of record (CPU, L=32; pre-registered §6 matrix)

224 deterministic+seeded cells (160 proton, 64 same-sign control), 16 seeds/arm.

### 3a · Cold stability (genesis on, no heat) — the proton is metastable, never lock-protected

| Arm | decay fraction | mode | lock |
|---|---|---|---|
| proton radius 1 | **0.125** (4/32) | evaporation | **0** |
| proton radius 2 | **0.4375** (14/32) | evaporation | **0** |
| same-sign `uuu` (control) | — | — | **0** |

The proton persists by incidental field-energy binding (radius-dependent: tighter = more
stable), **not** by the lock — `locked_final = 0` on every one of the 160 proton rows.
Spontaneous evaporation (a core → 0) removes the d-quark in 12.5% (r=1) to 43.75% (r=2) of
seeds over 2000 ticks. **Honesty note (discriminator D4):** the same-sign `uuu` control
*also* shows `locked = 0` in this seeded-triangle geometry (the lock additionally needs a
clustering condition the static seed does not meet), so the empirical lock-null is
**non-discriminating** — it does not, by itself, isolate the mixed-sign property as the
cause. The load-bearing basis for "the lock cannot protect the proton" is the §2 source
fact (the same-sign requirement), which the empirical null is consistent with but does not
prove.

### 3b · Heated — FTD's weak channel transmutes the proton and violates `Σs`

`genesis=off` isolates the weak channel (no evaporation, no thermal-ignition storm). Two
independent heating routes drive stress past `WEAK_THRESHOLD = K_GENESIS ≈ 1.533`:

| Route | weak=on transmute | weak=off (control) | effect |
|---|---|---|---|
| `inject` (single flux pulse, dual on) | 14/16 | **0/16** | `uud → uuu`, `Σs 1→3` |
| `langevin` T=0.3, T=0.8 (OU thermostat, dual off) | 32/32 | **0/32** | `uud → (flipped)`, `Σs ∈ {−1,1,3}` |
| **pooled weak=on** | **46/48 = 0.958** | **0/48** | transmutation |

When `weak_transmutation` fires on a proton quark it flips its polarity (`inject`: the
d-quark `−1 → +1`, giving `uud → uuu`, charge `Σs 1 → 3`, `ΔQ = +2` per clean flip). The
multiset changes and the **only exact FTD charge is not conserved**. The `weak=off`
controls (0/48) confirm the flip is entirely the weak channel, not the heating. This
directly falsifies prop (d): weak transmutation does *not* "preserve triad structure" — it
transmutes the proton into a different (and charge-different) object.

> Note on the Langevin route: under `dual_substrate=on` the OU thermostat writes only the
> merged `wave_vel`, which is overwritten each tick by `wave_vel_L+wave_vel_R`
> (phase_write.cpp:217,243) and never reaches the `flux_L` that `weak` reads — so the
> physical-heating arm runs `dual=off` (single-substrate stress). This is engine plumbing,
> disclosed in the pre-reg §2; it is why the thermostat arm uses `dual=off` and the
> controlled pulse uses `dual=on`.

### 3c · Charge (`Σs`) conservation across decay

Of **64** proton decay rows (cold + heated), **47 violate `Σs`** (`q_final ≠ q_init`) —
both decay channels are non-conserving: evaporation (`−1 → 0` raises `Σs` by 1, e.g.
`cold_r1` seed 8: `q 1→2`) and weak transmutation (`−1 → +1` raises `Σs` by 2, `q 1→3`).

## 4 · Frozen verdict

| Discriminator | reading |
|---|---|
| D1 lock protects proton? | **NO** (`max locked = 0`; basis = source fact, D4 confound noted) |
| D1 cold decay? | **YES** — 12.5% (r1), 43.75% (r2) spontaneous evaporation |
| D2 weak fires & breaks proton? | **YES** — 46/48 transmute; control 0/48 (clean) |
| D3 `Σs` conserved across decay? | **NO** — 47/64 decays violate it |

**UNFORCED-METASTABLE [BOUNDARY].** `τ_p = ∞` is not forced by the discrete ontology. The
lock the `[THEOREM]` invokes is structurally inapplicable to the mixed-sign proton; no
postulate forbids decay; the proton decays spontaneously by evaporation; FTD's own weak
channel transmutes it; and its only exact charge `Σs` is not conserved across decay. The
`proof_complete_sm.py` `τ_p = ∞` claim is corrected from `[THEOREM]` to **`[SELECTION]` /
emergent-metastable** (recorded in LEDGER FTD-0291).

## 5 · What this does and does not touch

- It adjudicates only whether `τ_p = ∞` is **forced**. It does **not** demote the
  algebraic-spine theorems, the `N_c = 3` derivations, the SM mass identifications, or
  weak-transmutation's status as an `[IMPOSED]` electroweak analog.
- Like the dark-matter halo (FTD-0290 = INDETERMINATE), the "micro" pier candidate
  (proton stability) **does not open as a forced FTD prediction** — a Number-One-Goal
  clause-2 boundary: the discrete ontology does **not** determine baryon stability, and
  its native dynamics in fact license proton decay.
- Zero promotions: FTD-0013 `[SMC]`, MC-T4.3 `[FOUNDATIONAL OBSTRUCTION]`,
  FTD-0110/0261/0269/0290 — all unchanged. The only epistemic change is the honest
  downgrade of the `τ_p = ∞` tag.

## 6 · Provenance

Pre-reg + instruments hash-locked at `bb99a20d` (tag `preregister-proton-stability-v1`)
**before** this run. Run of record: `engine/results/proton_stability/proton_stability_ror_*.csv`
(arms `ror_cold_r1`, `ror_cold_r2`, `ror_inject`, `ror_lan_T0p3`, `ror_lan_T0p8`).
Analyzer: `scripts/exploration/analyze_proton_stability.py` (frozen D1–D4). Campaign:
`engine/tests/campaign_proton_stability.cpp` (observation-only; golden-neutral). Deps:
FTD-0290 (the dark-matter sibling boundary), FTD-0269 (the forcing-vs-fitting precedent),
FTD-0272 (first-order genesis = the evaporation/condensation background), FTD-0273
(weak-transmutation B-violation context).
