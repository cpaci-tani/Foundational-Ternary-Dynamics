# PREREG — Isolating the Transverse-Contamination Mechanism: is it the kinetic drain?

**Tag:** [PRE-REGISTRATION — ENGINE MEASUREMENT] (LOCK-STD v1; git tag `preregister-kinetic-drain-curl-isolation-v1` at the registration commit)
**Parent finding:** `PREREG_GENESIS_ENERGY_LEDGER_v1` (Outcome REFUTED-AS-STATED, 2026-07-19): a genesis-born charge locks 2.7–3.6× the synthetic minimal self-energy W_SC(17)=0.478917129, because the Gauss projector is curl-free by construction and cannot remove whatever transverse field content real manifestation dynamics leave behind. That prereg *flagged, but did not measure*, a candidate mechanism: the kinetic-drain operation (`v.wave_vel *= (1 − K_GENESIS_KINETIC_DRAIN)`, applied only at the manifesting site — a spatially localized, single-point multiplicative operation, structurally unlike the curl-free radial seed or the (gradient) coupling term). **This document tests that hypothesis directly, by removing exactly the drain and nothing else.**
**Honest note on priors:** unlike the parent document, this instrument's *answer* is genuinely unknown before running — no pilot has been executed against the frozen design below. (Some scaffolding — `e_half`, the seed construction, the fresh-bridge relax pattern — is reused verbatim from the working, already-validated parent instrument; the two NEW measurements this document adds — `curl_total` and the drain-isolation branches — have not been run.)

## 1 · Two independent tests of the same hypothesis

### Test A — the decisive intervention (does removing drain restore W_SC?)

A new arm, **G-nodrain**, follows the *exact* trajectory of the parent's G-early arm — same seed, same toggle set, same deterministic RNG stream — through tick 1 (before manifestation). At tick 2 (the tick where G-early's manifestation fires, per the parent's disclosed data), `genesis` is toggled OFF for that one tick only (wave, coupling, gauss, damping still run normally — every other same-tick operation is preserved), so the tick advances without triggering the engine's own manifest-and-drain code path. Immediately after, `rb.set_state(x,y,z,+1)` is called directly — **verified by source read** (`injection.cpp` vs. `render_bridge.cpp:set_state_unlocked`) to touch *only* the ternary state field, never flux or wave_vel — flipping the site to manifested with **zero** drain and **zero** other flux-side-effect. The field is then frozen and relaxed to the Gauss fixed point exactly as the parent's arms were (fresh-bridge relaxation, residual-1e-8 gate, cap 5000).

Every other aspect of the tick-2 dynamics (wave propagation, coupling, Gauss projection, damping) is *identical* to what G-early experienced — this isolates the drain operation specifically, not "manifestation in general."

**Frozen prediction bands** (excess ≡ e_half − W_SC(17); G-early's excess = 1.709171333089 − 0.478917129 = 1.230254204):
| Band | Condition | Reading |
|---|---|---|
| CONFIRM | e_half(G-nodrain) ≤ W_SC(17) + 0.5×excess = **1.094044237** | Drain accounts for at least half the excess — a major mechanism |
| REFUTE | e_half(G-nodrain) ≥ W_SC(17) + 0.8×excess = **1.463120492** | Drain accounts for at most a fifth — not the dominant mechanism |
| Indeterminate/partial | between the two | A real, partial effect — characterize the fraction explicitly, do not force a binary reading |

### Test B — the isolated unit test (does draining inject curl, all else exactly equal?)

From a **single, bit-identical snapshot** F_pre (state+flux+wave_vel captured at the same tick-2-genesis-off point Test A uses, before *either* branch's state flip), two fresh bridges are cloned:
- **Copy-drain:** `set_state(+1)` at the target site, then `wave_vel *= (1 − K_GENESIS_KINETIC_DRAIN)` at that same site only (replicating exactly what the engine's own manifest-at code does), then one raw leapfrog integration (`toggles.disable_all()` + one `tick()` — on a fresh bridge `delta_j_` is zero-initialized and never populated since `wave_propagation` is never enabled on it, so this applies *only* `wave_vel += 0; flux += wave_vel`, the engine's own unconditional base leapfrog, with no other operation active — this mechanism was identified, not guessed at, while debugging the parent campaign).
- **Copy-no-drain:** identical, but the wave_vel scaling step is skipped.

`curl_total ≡ Σ_lattice |∇×J|²` (central-difference discrete curl, computed once as a new diagnostic — not previously measured in this program) is compared between the two copies. Because the two copies are bit-identical up to the single scaling operation, **any difference is causally attributable to the drain step alone** — no other confound is possible by construction.

**Frozen prediction bands:**
| Band | Condition | Reading |
|---|---|---|
| CONFIRM | curl_total(drain) / curl_total(no-drain) ≥ 2.0 | Drain clearly injects transverse content in isolation |
| REFUTE | ratio within [0.83, 1.2] | Drain does not meaningfully add curl in this isolated test |
| Indeterminate/partial | between | Characterize the actual ratio |

## 2 · Joint adjudication

Test A and Test B are independent lines of evidence for the same causal claim. **Both CONFIRM** → the mechanism is identified with high confidence; update the parent document's [OPEN] mechanism line to [MEASURED]. **Both REFUTE** → the drain hypothesis is set aside; the transverse content originates elsewhere (candidates for a future campaign: ordinary lattice-anisotropy curl drift under wave+coupling dynamics even without genesis, or some other genesis-adjacent operation not yet considered — NOT claimed here, only named as the residual space). **Disagreement between A and B** → an honest, reportable finding in its own right (e.g., drain contributes measurably in the full dynamical context (A) via some indirect/compounding route but not in the single-step isolated test (B), or vice versa) — characterized as such, not forced into a false consensus.

## 3 · Validity gates

- **V1:** G-nodrain's trajectory through tick 1 must be bit-identical to G-early's (determinism check — both derive from the identical seed and RNG stream).
- **V2:** the tick-2-genesis-off step must leave the target site unmanifested (state=0) immediately before the manual `set_state` call — confirms genesis truly did not fire during the suppressed tick.
- **V3:** both Test-B copies' single leapfrog step must leave every *other* voxel's flux bit-identical (only the target site's wave_vel differs going in, so only sites within the leapfrog's local coupling — none, since the base leapfrog is per-voxel, `flux[i] += wave_vel[i]` — should differ; a broader divergence would indicate the clone or toggle-isolation is not as clean as designed).

---

## RUN 1 (2026-07-19) — **VOID on V2**, caught cleanly by its own gate

`GATE,V2,manifested_pre_manual_flip,1` — the target site was **already manifested** immediately before the intended no-drain intervention. Per this document's own §3 validity gates, this voids the run; no outcome is read from it.

**Root cause (off-by-one against the parent's own tick convention):** the parent campaign's loop is check-then-tick, breaking when the *check* finds `manifested_count ≥ 1`; its disclosed `fire_tick=2` therefore means manifestation completes **during the second `tick()` call**, not the third (trace: t=0 check false → tick() call 1; t=1 check false → tick() call 2; t=2 check **true** → break, no further call). This document's Run-1 prefix executed *two* normal `tick()` calls before applying the genesis-OFF intervention as a third call — but real genesis (with drain) had already fired during the second of those two calls, so the intervention arrived one tick too late, on an already-manifested, already-drained state. The manual `set_state` call that followed was consequently a no-op (state was already +1), and Test A's measured `e_half`=0.708 does not test the registered hypothesis — it is closer to a (differently-triggered) extra-relaxation variant of the parent's own G-late arm than to a no-drain isolation.

**This is exactly what the pre-declared V2 gate exists to catch**, and it did — on the very first execution, from the data, before any number was misread as a result.

## v1.1 (procedural amendment, instrument-only; cut before re-running)

Fix: run exactly **one** normal `tick()` call in the prefix (matching the parent's confirmed-safe post-call-1 state), then apply the genesis-OFF intervention as call 2 — the call where real genesis would otherwise fire. No change to the question, the outcome bands, Test B, or any other clause; §1's frozen prediction bands stand unchanged and unconsumed (Run 1 produced no reading against them).

---

*Registered 2026-07-19, before either instrument's first execution. Author: session 8294fddb, following LOCK-STD v1. Companion/parent: `preregister-genesis-energy-ledger-v1`.*
