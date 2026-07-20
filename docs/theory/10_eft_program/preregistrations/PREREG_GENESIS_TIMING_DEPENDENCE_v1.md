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

*Registered 2026-07-19, before the instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-perturbation-magnitude-curl-sweep-v1`, `preregister-kinetic-drain-curl-isolation-v1`, `preregister-genesis-energy-ledger-v1`.*
