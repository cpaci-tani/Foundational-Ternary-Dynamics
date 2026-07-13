# AUDIT — Effective Toggle Set of the Vertex / Program-F Campaigns

**Tag:** [AUDIT / MEASUREMENT-CONDITIONALITY]
**Scope:** FTD-0383 Arc 1 (Consumption Program). Queued by `ANALYSIS_VERTEX_DK_CLOSURE_v1.md` §1.4 (the "disclosed anomaly — predecessor toggle-set validity"). This audit resolves that anomaly by reading the engine source.
**Method:** READ-ONLY static code reading. No engine build, no smoke run. Every classification cites `file:line`. Where a classification depends on the *runtime* particle configuration (a dynamical fact not settled by reading), it is marked **NEEDS-SMOKE-RUN** rather than guessed.
**Promotes nothing.** No epistemic tag moves. FTD-0088 stays [MEASURED]; the vertex negatives (FTD-0379/0380) stay [CLOSED NEGATIVE]. This document only makes the *effective* (post-validation, per-platform, per-prerequisite) toggle set explicit so future locks can declare it.

---

## 0 · One-paragraph verdict

The three phases flagged by `ANALYSIS_VERTEX_DK_CLOSURE_v1.md` §1.4 (`weak_transmutation`, `exchange_force`, `triad_binding`) did **not** act *inertly* on the GPU backend the campaigns actually ran on — but they did not act as the "full non-local dynamics" label implies either. `weak_transmutation` ran **degraded** (single-substrate stress fallback; the L↔R chirality swap silently dropped). `exchange_force` and `triad_binding` ran **fully**, because their declared `requires_` dependencies (`poisson_coulomb`, `dual_substrate`) are **nominal** — the GPU kernels never read the required buffers. The load-bearing structural fact is different and stronger: **all** of CONFIG-N's non-core force phases (`color_forces`, `strong_force`, `exchange_force`) write only to `velocity`, `triad_binding` writes only to `locked`, and CONFIG-N has the state→flux **`coupling` term switched OFF**. The FTD-0088 grade skeleton is a **flux-only** observable, so those phases touch it at most *indirectly* (via `movement` → state relocation → the Gauss `∇·J=s` source) and at *higher order* over the 2-tick protocol. FTD-0088's 12/12 is therefore a **kinematic** measurement whose value does not rest on the flagged phases; its [MEASURED] status **survives**, and the anomaly is common-mode (identical GPU code path in FTD-0088 and M1), so **internal consistency is preserved, not asymmetric**. What must change is the *label*: "full non-local toggle set" overstates the effective dynamics acting on the measured observable.

---

## 1 · The toggle machinery, as read

### 1.1 Validation is warning-only, and its dependency graph is intent-level, not data-level

`TermToggles::validate()` (`engine/include/ftd/term_toggles.h:264-323`) runs two passes: a table-driven `requires_`/`conflicts` sweep over `TOGGLE_SPECS[]` (`term_toggles.h:185-226`), plus hand-rolled cross-cutting rules (`term_toggles.h:304-320`). It returns a message string; it does **not** throw unless `strict_validation` is set (`term_toggles.h:104`). At runtime the message is printed once and memoized (`engine/src/render_bridge.cpp:562-569`) — **runs proceed regardless**.

Three `requires_` edges are relevant here:
- `weak_transmutation` requires `dual_substrate` (`term_toggles.h:202`) + hand-rolled duplicate (`term_toggles.h:316-317` prints "operates on J_L/J_R" for `triad_binding`).
- `exchange_force` requires `poisson_coulomb` (`term_toggles.h:206`).
- `triad_binding` requires `color_forces` (`term_toggles.h:204`) **and** (Pass 2) `dual_substrate` (`term_toggles.h:316-317`).

**Finding V-1 (dependency-graph accuracy).** These `requires_` edges are *declared intent*, not verified data-dependencies. Reading the phase kernels (§3) shows:
- `triad_binding` never reads `J_L`/`J_R` on either backend — the message "operates on J_L/J_R" is **inaccurate** (`transmutation_phases.cpp:137-176`; `kernels_forces.cu:816-826` reads only `state`, `locked`).
- `exchange_force` never reads the Coulomb potential — the `requires poisson_coulomb` edge is **spurious** w.r.t. the kernel's inputs (`kernels_forces.cu:757-808`: inputs are `state`, `spin`, `velocity` only).
- `weak_transmutation` *does* have a real `dual_substrate` sensitivity, but only for the L↔R swap sub-behavior; the polarity flip runs on a single-substrate fallback (`kernels_aux.cu:131-133`).

So the §1.4 warnings correctly flag that CONFIG-N is *not a validated configuration*, but they do **not** by themselves establish that any phase was inert.

### 1.2 The campaigns ran on the GPU backend — this is decisive

With CUDA compiled in, the **default backend is GPU** (`engine/src/backend.cpp:224-233`; the banner `[RenderBridge] GPU backend active` is emitted at `backend.cpp:227`). `RenderBridge::tick()` dispatches the whole tick to `GpuBackend::tick()` and returns before the CPU phase ladder (`render_bridge.cpp:577-587`).

Independent confirmation that the Program-F / vertex runs used GPU:
- `DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md:273` "[MEASURED] on GPU"; `:396` "the engine's GPU implementation"; `:591` lists the CONFIG-N set as the FTD-0088 config; `test_clifford_multigrade.cpp` is tagged "GPU test" (`:528`, `:773`).
- `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:123` "runs executed on the Windows-native CUDA backend (`[RenderBridge] GPU backend active`)".

**This flips the F2 finding.** The known F2 result — `strong_force` / `exchange_force` have no CPU implementation and are no-ops on CPU — is encoded as GPU-only warnings (`term_toggles.h:203,206`) that print via `cpu_runtime_warnings()` **only on the CPU path** (`render_bridge.cpp:592-598`, *after* the GPU dispatch has already returned at `:586`). On the GPU backend those phases **do** have kernels (`kernels_forces.cu:959-983`) and the GPU-only warnings are never printed — consistent with §1.4 reporting only the three `validate()` messages and none of the F2 CPU warnings. **INERT-no-op-platform therefore does not apply to the campaigns as run; it would apply only to a hypothetical CPU-only rebuild.**

---

## 2 · The CONFIG families

Three configs bound the campaigns. All three are `disable_all()` + explicit re-enables, so every toggle not listed is **GATED-OFF** — including, notably, `coupling`, `damping`, `selective_damping`, `gravity`, `lorentz_force`, and `poisson_coulomb`.

| Config | Where | Enabled toggles (everything else GATED-OFF) | `validate()` |
|---|---|---|---|
| **CONFIG-N** (FTD-0088 "full non-local") | `test_clifford_multigrade.cpp:159-173` (`enable_full_nonlocal`); `test_dk_evolution.cpp:293-307` (`enable_config_N`) | wave_propagation, gauss_projection, genesis, movement, forces, emergent_forces, pair_production, weak_transmutation, exchange_force, strong_force, triad_binding, color_forces | **3 warnings**: weak_transmutation⊃dual_substrate; exchange_force⊃poisson_coulomb; triad_binding⊃dual_substrate |
| **CONFIG-M** (M1 linear control) | `test_dk_evolution.cpp:309-313` (`enable_config_M`) | wave_propagation, gauss_projection | clean |
| **CONFIG-F″** (FTD-0087 / M2) | `test_bivector_closure_v2.cpp:99-110` | wave_propagation, gauss_projection, genesis, movement, forces, emergent_forces, pair_production, weak_transmutation, **dual_substrate**, triad_binding, color_forces (no exchange/strong) | **clean** — dual_substrate on satisfies weak_transmutation & triad prerequisites |

Protocol common to all: L=8, A=10, WH-mode 2-injection on the 2³ corner block (`test_clifford_multigrade.cpp:146-157,183-206,217-218`). FTD-0088's grade readout is a **2-tick** snapshot (`run_2inj`: inject→run(1)→inject→run(1), `:188-192`). M1 records 30 further ticks (`test_dk_evolution.cpp:460-478`).

**Finding V-2 (`coupling` is OFF in the "full non-local" set).** Neither CONFIG-N nor CONFIG-F″ re-enables `coupling`, so the core state→flux source term `g_c·∇s` is disabled (gate: `render_bridge.cpp:626-627`; GPU kernel receives `toggles.coupling=false`, `gpu_engine.cu:325,331`). The only surviving state→flux channels are (i) the Gauss projection source `∇·J=s` (`gpu_engine.cu:245-251`) and (ii) genesis's K_GENESIS flux drain (`phase_write.cpp:341-357`). This is a second respect in which "full non-local dynamics" overstates the configuration.

---

## 3 · Per-(config, toggle) classification

Taxonomy used (superset of the requested four, refined by what reading actually shows):
- **ACTIVE** — kernel dispatches and mutates state relevant to the run.
- **ACTIVE-DEGRADED** — dispatches, but a sub-behavior is silently dropped for lack of a prerequisite.
- **ACTIVE (nominal-dep)** — dispatches fully; the failed `requires_` edge is not a real data-dependency.
- **PARTICLE-GATED** — dispatches, but only mutates state if manifested particles exist / cluster / meet a threshold (structure determinable by reading; **magnitude = NEEDS-SMOKE-RUN**).
- **INERT-missing-prerequisite** — genuine no-op because a required input is absent/zero.
- **INERT-no-op-platform** — no implementation on the active backend.
- **GATED-OFF** — toggle false.

"Flux effect" = effect on the measured **flux-only** grade skeleton (S=|J|², V=J, P=plaquette bilinear, T=JₓJᵧJ_z; `test_dk_evolution.cpp:223-245`).

### 3.1 CONFIG-N on the GPU backend (the FTD-0088 / M1-primary config)

| Toggle | GPU dispatch site | Classification | Flux effect | Evidence |
|---|---|---|---|---|
| `wave_propagation` | `gpu_engine.cu:236,330-336` | **ACTIVE** | DIRECT (leapfrog on J) | single-substrate read path (dual_substrate off) |
| `gauss_projection` | `gpu_engine.cu:245-251` | **ACTIVE** | DIRECT (enforces ∇·J=s) | dual-sync skipped (dual off) |
| `genesis` | `gpu_engine.cu:237,364-379` | **ACTIVE**, PARTICLE-GATED | DIRECT-ish (flux drain) + feeds Gauss source | manifests when `flux.mag2>K_GENESIS²`; at A=10, 100≫0.257 → fires (`phase_write.cpp:341-357`) |
| `movement` | `gpu_engine.cu:279-281` | **ACTIVE**, PARTICLE-GATED | INDIRECT via state relocation | acts on manifested particles only |
| `forces` (+`emergent_forces`) | `gpu_engine.cu:261-263,438-446` | **ACTIVE**, PARTICLE-GATED | INDIRECT (writes `velocity`) | emergent mode (poisson_coulomb off); gravity/lorentz GATED-OFF; Coulomb-from-∇J on velocity |
| `emergent_forces` | consumed at `gpu_engine.cu:438-446` | **ACTIVE** (mode selector) | — | `!poisson_coulomb` guard honored |
| `pair_production` | `gpu_engine.cu:240-242,461-465` | **ACTIVE**, PARTICLE-GATED (void) | DIRECT (writes state at void sites → Gauss source) | fires at void sites with `|J|>2·K_GENESIS` (`kernels_aux.cu:196-266`) |
| `weak_transmutation` | `gpu_engine.cu:284-286,455-459` | **ACTIVE-DEGRADED**, PARTICLE-GATED | INDIRECT via Gauss (polarity flip → source sign) | single-substrate stress fallback used; **L↔R swap dropped** (`kernels_aux.cu:123,131-133,175-185`) |
| `exchange_force` | `gpu_engine.cu:266-271,485-487` | **ACTIVE (nominal-dep)**, PARTICLE-GATED | INDIRECT via movement (writes `velocity`) | kernel reads state/spin/velocity, **never phi_coulomb**; needs same-spin pair, `spin≠0` (`kernels_forces.cu:757-808`) |
| `strong_force` | `gpu_engine.cu:266-271,482-484` + stencil `:382-384` | **ACTIVE**, PARTICLE-GATED | INDIRECT via movement (writes `velocity`) | yukawa kernel writes velocity only (`kernels_forces.cu:959-970`); CPU would be INERT-no-op-platform |
| `triad_binding` | `gpu_engine.cu:274-276,490-493` | **ACTIVE (nominal-dep)**, PARTICLE-GATED | INDIRECT via movement (writes `locked`) | kernel reads state/locked only, **never J_L/J_R**; needs ≥3 clustered same-state (`kernels_forces.cu:816-826`) |
| `color_forces` | `gpu_engine.cu:266-271,479-481` + stencil `:382-384` | **ACTIVE**, PARTICLE-GATED | INDIRECT via movement (writes `velocity`,`fd_strong`) | needs `color≠0` (`kernels_forces.cu:944-957`) |
| `coupling`, `damping`, `selective_damping`, `gravity`, `lorentz_force`, `poisson_coulomb`, `dual_substrate`, … | not enabled | **GATED-OFF** | — | `disable_all()` at `test_clifford_multigrade.cpp:160` |

The pairwise-force finding is load-bearing and line-verified: **`exchange_force` (`kernels_forces.cu:805-807`), `yukawa`/`strong` (`:959-970`), and `color` (`:944-957`) mutate only `velocity` (+ force-diag); `triad_binding` mutates only `locked` (`:816-826`). None write `flux`.** Over 2 ticks, `velocity` reaches the flux observable only through `movement`, whose displacement is capped at `C_SPEED = 1/√3` per tick, so its contribution to the 2-tick FTD-0088 flux snapshot is second-order at most.

### 3.2 CONFIG-M (M1 linear control)

`wave_propagation`, `gauss_projection` = **ACTIVE**; everything else **GATED-OFF** (`test_dk_evolution.cpp:309-313`). No genesis → no particles → the entire force/transmutation/triad layer is vacuously absent. This is the clean linear anchor; its grade-1 field fits KG at ρ=0.088 (`ANALYSIS_VERTEX_DK_CLOSURE_v1.md:41`).

### 3.3 CONFIG-F″ (FTD-0087 / M2) on GPU

Identical to CONFIG-N **except** `dual_substrate` ON and `exchange_force`/`strong_force` absent (`test_bivector_closure_v2.cpp:99-110`). Consequences:
- `weak_transmutation` → **ACTIVE (full)**: dual_substrate on, so the L↔R swap executes (`kernels_aux.cu:175-185`); the wave path is the dual read/write (`gpu_engine.cu:322-337,350-362`).
- `triad_binding` → **ACTIVE (nominal-dep, full)** (still never reads J_L/J_R; dual on merely silences the warning).
- `exchange_force`, `strong_force` → **GATED-OFF**.
- `coupling` still **GATED-OFF** (Finding V-2 applies to F″ too).

So M2 ran on a **warning-free** config, consistent with `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:72`. M2's execution is unaffected by the toggle-validity anomaly.

### 3.4 Stale-comment defect (documentation, not behavior)

`render_bridge.cpp:660-664` claims pair_production is "No-op on CPU until the pair-production CPU port lands," yet `pair_production_cpu()` is a complete implementation (`transmutation_phases.cpp:78-135`) and *is* called on the CPU path — the port landed (per the F11.A-5 note in `term_toggles.h:205`). The comment is **stale**. This does not affect the GPU campaigns but would mislead a CPU-path reader.

---

## 4 · Consequence for FTD-0088's conditionality

FTD-0088's claim is the 12/12 Cl(3,0) grade skeleton at CONFIG-N (`test_clifford_multigrade.cpp`), which `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:20,112` records as [MEASURED] "subject to the §1.4 effective-toggle audit." This audit is that audit. Verdict on the four sub-questions:

1. **Were the flagged phases inert?** No — not on GPU. `weak_transmutation` ran degraded; `exchange_force` and `triad_binding` ran fully (their failed prerequisites are nominal). So the §1.4 worst case ("acted inertly") is **partly disconfirmed**; the accurate statement is "acted, but on `velocity`/`locked`/degraded-polarity, not on the measured flux."

2. **Does the 12/12 skeleton depend on them?** No. The skeleton is a **flux-only** observable (`test_dk_evolution.cpp:223-245`); the flagged phases write `velocity`/`locked`, reaching flux only via `movement`→Gauss at higher order over 2 ticks (§3.1). The dominant contributors to the skeleton are `wave_propagation` + `gauss_projection` + `genesis` (via the Gauss `∇·J=s` source), with `coupling` **off** (Finding V-2). The 12/12 is thus **kinematic** — which is exactly the reading M1 independently reached ("kinematic-only at the tested scope," `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:17,102`). The audit **reinforces** that reading rather than threatening it.

3. **Symmetric or asymmetric?** **Symmetric / common-mode.** FTD-0088 and M1's CONFIG-N are the same `disable_all()`+re-enable set compiled into the same GPU code path (`test_clifford_multigrade.cpp:159-173` ≡ `test_dk_evolution.cpp:293-307`). Any degradation/nominal-dependency is identical on both sides, and M1's negative held identically in CONFIG-N (ρ_all 1.71) and the flagged-phase-free CONFIG-M (ρ_all 1.72) — i.e. the verdict is **invariant** to whether the flagged phases acted. Internal consistency is preserved.

4. **Net on the [MEASURED] tag.** **Survives, unchanged.** No promotion, no demotion. The one required correction is documentary: replace the label "full non-local toggle set / full non-local dynamics" with the **effective** description — *"undamped bare-wave leapfrog + Gauss `∇·J=s` + genesis, with `coupling` OFF and the non-core force phases acting on `velocity`/`locked` only."*

Corollary (already noted at `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:74`): FTD-0088 §3.4.2's "4-injection dynamical noise from forces, triad, exchange" names a per-tick mixing that (a) writes velocity/locked not flux, and (b) for `weak_transmutation` ran degraded. M2 refuted that noise hypothesis on its own (dual-substrate, warning-free) config; this audit adds that the named mixing was partly velocity/locked-only even where it did act — strengthening, not weakening, the M2 [CLOSED NEGATIVE].

---

## 5 · The matched-EFFECTIVE-protocol declaration template

Any future lock (IMP-S4 battery, V1/V2, M1-v2, or any re-run of FTD-0085–0089) must declare the **effective** toggle set, not the nominal one. Required fields:

```
MATCHED-EFFECTIVE-PROTOCOL DECLARATION  (attach to every prereg/lock)
────────────────────────────────────────────────────────────────────
1. Build provenance
   - backend           : GPU (engine/build_wsl, RTX 5090)  |  CPU (engine/build)
                         [state which — it changes the effective set, §1.2]
   - FTD_ENABLE_CUDA   : ON/OFF   commit: <sha>   golden hash: <hash>
2. Nominal toggle set  : verbatim enable_* list (file:line)
3. validate() result   : PASTE the message string, or "clean"
                         [a non-empty string = an UNVALIDATED config; each line
                          must be resolved in fields 4-5 below]
4. Effective toggle set (post-validation, per-platform):
   For each enabled toggle, one row —
     toggle | dispatches? (kernel file:line) | prerequisites present? |
            classification {ACTIVE / ACTIVE-DEGRADED / ACTIVE(nominal-dep) /
            PARTICLE-GATED / INERT-missing-prereq / INERT-no-op-platform} |
            fields mutated {flux/state/velocity/locked/latency} |
            effect on the MEASURED observable {DIRECT / INDIRECT / NONE}
5. Prerequisites verified (not assumed):
   - for every requires_ edge that FAILED validation, state whether the kernel
     actually reads the required buffer (nominal vs real dependency) — cite
     the kernel signature file:line, do NOT infer from the toggle table
   - confirm each requires_ edge that PASSED is genuinely consumed
6. Particle-manifestation check (if any PARTICLE-GATED phase is claimed active):
   - does genesis fire at this A? (flux.mag2 vs K_GENESIS², phase_write.cpp:328,341)
   - are ≥2 particles within pairwise range / ≥3 within TRIAD_RADIUS?
     → if not read-determinable: mark NEEDS-SMOKE-RUN and list the run
7. Observable-coupling check:
   - is `coupling` ON? (the g_c·∇s state→flux channel) — if OFF, say so
   - which channels connect the toggled phases to the measured observable?
```

A lock that fills only fields 1–3 (the nominal set + a green/red validate flag) is **insufficient** — it is exactly what let CONFIG-N be described as "full non-local" while `coupling` was off and three phases ran degraded/nominal-dep.

---

## 6 · NEEDS-SMOKE-RUN follow-ups (optional; not required to accept §4)

These are magnitude questions that reading cannot settle. §4's verdict does **not** depend on them (it rests on the flux-only structure of the observable, which is read-determined); they would only *quantify* the residual effect.

- **SR-1** — Instrument the CONFIG-N 2-tick run and count: manifested particles, `weak_transmutation` flip events, `pair_production` events, particles entering pairwise range, triads locked. Confirms which PARTICLE-GATED phases fire *at all* at (L=8, A=10). Expected: genesis + pair_production fire; pairwise clustering marginal (8 injected sites in the corner).
- **SR-2** — Differential: re-run FTD-0088's grade readout with the flagged phases forced OFF (drop weak/exchange/strong/triad/color, keep wave+gauss+genesis+movement+forces). If the 12/12 skeleton is unchanged to measurement tolerance, §4.2 (skeleton is kinematic) is confirmed empirically. This is the decisive check.
- **SR-3** — A *prerequisite-valid* full-non-local cell (add `dual_substrate`+`poisson_coulomb`+`coupling` so all edges are real and the state→flux coupling is on) for both M1 and M2 — the accessible-but-unrun cell named at `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:74`. Tests whether "full non-local dynamics done properly" changes either verdict.
- **SR-4** — Verify (or fix) the stale CPU pair_production comment (`render_bridge.cpp:660-664`) against `transmutation_phases.cpp:78-135`. Pure documentation.

---

## Appendix — file:line index of every claim

| Claim | Source |
|---|---|
| Validation warning-only; memoized | `term_toggles.h:264-323`; `render_bridge.cpp:562-569` |
| requires_ edges (weak/exchange/triad) | `term_toggles.h:202,204,206,316-317` |
| GPU default backend + banner | `backend.cpp:224-233,227` |
| GPU tick dispatch / early return | `render_bridge.cpp:577-587` |
| F2 CPU-only warnings print after GPU return | `render_bridge.cpp:592-598` |
| Program-F ran on GPU | `DERIV_BIVECTOR_ALGEBRA_PROGRAM_F.md:273,396,591`; `ANALYSIS_VERTEX_DK_CLOSURE_v1.md:123` |
| CONFIG-N set | `test_clifford_multigrade.cpp:159-173`; `test_dk_evolution.cpp:293-307` |
| CONFIG-M set | `test_dk_evolution.cpp:309-313` |
| CONFIG-F″ set (dual on) | `test_bivector_closure_v2.cpp:99-110` |
| 2-tick grade protocol | `test_clifford_multigrade.cpp:183-206` |
| Grade fields are flux-only | `test_dk_evolution.cpp:223-245` |
| `coupling` OFF gate | `render_bridge.cpp:626-627`; `gpu_engine.cu:325,331` |
| GPU tick phase order | `gpu_engine.cu:220-301` |
| weak_transmutation single-substrate fallback + swap | `kernels_aux.cu:123,131-133,175-185`; CPU `transmutation_phases.cpp:26-42` |
| pair_production kernel (void, 2·K_GENESIS) | `kernels_aux.cu:196-266`; CPU `transmutation_phases.cpp:78-135` |
| exchange kernel writes velocity, no phi_coulomb | `kernels_forces.cu:757-808` (mutate `:805-807`) |
| yukawa/strong writes velocity | `kernels_forces.cu:959-970` |
| color writes velocity/fd_strong | `kernels_forces.cu:944-957` |
| triad reads state/locked, writes locked | `kernels_forces.cu:816-826`; CPU `transmutation_phases.cpp:137-176` |
| pairwise-force guards `num_particles<=0` | `kernels_forces.cu:945,960,973,986` |
| genesis manifest gate | `phase_write.cpp:328,341-357` |
| stale CPU pair_production comment | `render_bridge.cpp:660-664` vs `transmutation_phases.cpp:78-135` |
| CPU color_forces only (no strong/exchange) | `phase_forces.cpp:54,144` |
