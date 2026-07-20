# PREREG — Genesis's Operating Point: is the mass excess timing-dependent?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-genesis-timing-dependence-v1` at the registration commit)
**Parent finding:** `PREREG_PERTURBATION_MAGNITUDE_CURL_SWEEP_v1` (2026-07-19): the injected-curl response to a single-site perturbation is now known exactly, `curl_total(s) = 7.706578 + 8.259921·s + 3.266963·s²` (R²=1.0000000000). What remains open, named explicitly in that prereg's outcome: *where on this curve genesis's own mechanics actually land* — `s=1.0` (undrained) is the closest measured proxy, but every measurement so far has fixed the manifestation tick at the *earliest* opportunity. **This document asks whether that operating point is stable — a characteristic property of manifestation — or an accident of exactly when, in the underlying stochastic hazard process, the event happens to fire.**

## 1 · Why this matters

FTD's genesis hazard is stochastic (`p = 1 − exp(−(|J|−K_GENESIS)/K_MANIFEST)` per tick once above threshold). Every measurement in this line of inquiry so far (`PREREG_GENESIS_ENERGY_LEDGER_v1` onward) has used a seed whose margin above threshold gives a high per-tick firing probability (~93%), so manifestation fires almost immediately once eligible — meaning the *specific* tick sampled by every measurement so far is close to the earliest possible one. If a real particle's birth were instead delayed by several ticks (as would happen routinely for a lower-margin, more slowly-approaching manifestation event — a realistic case this campaign does not need to construct, since it can test timing-sensitivity directly by *forcing* a delay on the existing seed), does the field it inherits — and hence the locked energy it later carries — look substantially different? If rest mass is to mean anything like a fixed particle property, the answer should not depend on when, within the eligible window, birth happens to occur. **This campaign tests that directly, for the first time in this line of inquiry.**

## 2 · Design

`engine/tests/campaign_genesis_timing_dependence.cpp`, locked at the registration commit. Reuses the identical seed and Phase-A construction as the parent campaigns (deterministic RNG stream; `T=0` exactly reproduces the already-measured baseline as a reproducibility check).

For each `T ∈ {0, 1, 2, 3, 5, 8}` (extra ticks of *forced* delay beyond the original earliest-opportunity tick — a controlled counterfactual, not the seed's own natural stochastic timing, which would almost always fire near `T=0` given its margin): run 1 normal tick (call 1, `genesis` ON), then run `(1+T)` further ticks with `genesis` OFF (wave/coupling/gauss/damping all continue normally — every other operation fires exactly as it would in the real stochastic process; only the manifestation decision itself is deferred). At that point, extract the field snapshot F_pre(T).

**Per T, three measurements** (all via the validated fresh-bridge / isolated-leapfrog-step machinery from the parent campaigns):
- `curl_drained(T)`: one-leapfrog-step curl, wave_vel scaled by 0.5 at the target (matching the *real* engine's actual drain mechanics).
- `curl_undrained(T)`: the same, unscaled (`s=1`) — the direct cross-check point against the now-known response curve.
- `e_half_relaxed(T)`: state flipped, wave_vel *drained* (matching real genesis exactly), then relaxed to the Gauss fixed point (the residual-1e-8 protocol used throughout this line) — **the primary observable**: this is the actual "locked energy" a real, drained manifestation event would leave behind if it happened to fire at delay T.

## 3 · Frozen reading (a characterization, not a binary pass/fail — stated before running)

Compute the coefficient of variation of `e_half_relaxed(T)` across all 6 delays: `CV = std(e_half_relaxed) / mean(e_half_relaxed)`.

| Band | Reading |
|---|---|
| `CV < 0.05` (< 5% spread) | The locked energy is **stable** across timing — genesis has a genuinely characteristic operating point, largely independent of exactly when it fires. Supports treating the mass excess as a real, well-defined (if not yet derived) property of manifestation, not an artifact of this campaign's specific tick choice. |
| `CV ≥ 0.30` (≥ 30% spread, or non-monotonic swings of comparable size to the mean itself) | The locked energy is **history-dependent** — timing materially matters, and "the" mass excess is not a single number without specifying the birth circumstances. A significant finding in its own right about the theory's current state, not a null result. |
| Between | Report the actual spread and pattern (monotonic drift vs. oscillation vs. plateau) without forcing either reading. |

`curl_undrained(T)` is additionally checked against the parent's known `curl_total(1.0)=19.233462` at `T=0` (reproducibility gate) and tracked across `T` as a secondary, cheaper (no full relaxation) proxy for the same question.

## 4 · Validity gates

- **V1:** `T=0`'s `curl_undrained` must equal the parent's disclosed `19.233462…` to reported precision (bit-identical prefix reproduction).
- **V2:** at every `T`, exactly one site remains above `K_GENESIS` and unmanifested immediately before the flip (the forced delay must not let a second site cross threshold, nor let the target's own field decay back below threshold — either would contaminate the comparison across `T`).
- **V3:** every relaxation converges within the standard residual-1e-8 / cap-5000 protocol.

---

## OUTCOME (2026-07-19) — **VOID on V2 for T≥1**, and the violation is itself the finding

Data: `engine/build/timing_dep_v1/run.csv`.

**T=0 is valid** — V1 passes exactly (`curl_undrained_matches_parent=1`), reproducing the parent campaigns bit-identically, as every prior gate has.

**T≥1 all fail V2**: `sites_above_threshold=0` and `target_above_threshold=0` at every one of T∈{1,2,3,5,8}. The design assumed that suppressing genesis would leave the target site *sitting* above threshold, waiting; instead, the field **sloshes** — `fpre_e_half` swings 3.13 (T=0) → 0.48 (T=1) → 2.24 (T=2) → 1.17 (T=3) → 1.78 (T=5) → 0.89 (T=8), non-monotonically, consistent with energy trading between the flux and wave_vel channels under active wave dynamics. The target's own `|J|` drops *below* `K_GENESIS` within a single further tick and does not reliably return. **The T≥1 measurements are therefore not testing "manifestation delayed within its eligible window" — they are testing a counterfactual forced-manifestation of a site the real stochastic hazard would not have fired at all, since it is no longer above threshold.** Per this document's own validity gate, these points are VOID and are not read as an answer to the registered question.

**Why this is not a wasted run.** The violation is a real, informative measurement in its own right: for this seed, the *eligible window* — the span during which a threshold-crossing site remains above `K_GENESIS` before ordinary dynamics carries it back down — appears to be very short, on the order of a single tick. This is consistent with, and helps explain, every prior campaign's own observation that genesis fires at the *earliest* opportunity with high probability (~93%/tick from this seed's margin): if the window is this narrow, there may be little practical "natural" timing variation for a seed built this way to explore at all — the stochastic hazard's high firing rate and the field's short eligible window are two views of the same fact.

**Disposition: no v1.1 patch of this design.** Extending the eligible window artificially (e.g. re-injecting flux each tick to hold the site above threshold) would itself become the confound under test, defeating the purpose. The underlying question — is genesis's operating point on the curl-response curve a stable, circumstance-independent property — is better answered by a design that does not depend on sustaining threshold-eligibility at all: comparing *independently constructed* manifestation events (different seeds, each firing at its own natural earliest opportunity, matching how the real stochastic process actually behaves) rather than forcing one seed to wait past its own field dynamics. See the companion document `PREREG_MANIFESTATION_SEED_DIVERSITY_v1` for that redesign.

---

*Registered 2026-07-19, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-perturbation-magnitude-curl-sweep-v1`, `preregister-kinetic-drain-curl-isolation-v1`, `preregister-genesis-energy-ledger-v1`.*
