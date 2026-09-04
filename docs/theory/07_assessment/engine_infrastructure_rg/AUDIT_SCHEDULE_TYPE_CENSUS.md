# Audit — the schedule as a typed law: a kind-census of `RenderBridge::tick()`

**Claim id:** FTD-1027 (drafted, unbooked)
**Verdict:** `[MEASURED — SOURCE-LINT]`; one listing-based claim `[REFUTED]`
**Tags moved:** none. Underwrites FTD-0209 (C2-1) at schedule level; supplies a prediction
for the native two-body Lorentz test (FTD-1009 successor).
**Verifier:** `scripts/proofs/proof_schedule_type_audit.py` (21/21 source assertions)
**Date:** 2026-09-04

---

## 1. Purpose

P5 requires one state-complete law with an explicit conflict-free schedule. The engine's
schedule — the ordered phases of `RenderBridge::tick()` — is therefore the law in
operational form. `SPEC_ENGINE.md` §4 types each phase by *data flow* (what it reads,
what it writes). No document types the phases by **kind**: whether a step is hyperbolic
(second-order in time, propagating), elliptic (a constraint solved instantaneously),
combinatorial (threshold events, collisions, non-injective transitions), or algebraic
(local, invertible bookkeeping). Kind determines what structures a phase is *permitted*
to emit and what it forbids, independently of any parameter. This audit supplies that
census from the C++ source, with every classification tied to a line the verifier checks.

## 2. Method and scope

Classification is by the update rule actually executed, not by the schedule listing or
the phase's name. Each row below cites the source lines the verifier asserts on. Phases
are separated into the **default law** (toggles at their `term_toggles.h` defaults) and
the **toggle-gated extensions** (off by default). Where the listing suggested one thing
and the source another, the source governs and the discrepancy is recorded (§7).

## 3. Census of the default law

| step | phase | kind | update rule (source) | may emit | cannot emit |
|---|---|---|---|---|---|
| 1c / 2 | `phase_read` → `phase_write` | **hyperbolic** | `δJ = c²∇²J − G_C∇s + G_C∇×(s v)`; commit `wave_vel += δJ·dt; J += wave_vel·dt` (`phase_read.cpp:326`, `phase_write.cpp:217–225`). Mass term `−ω₀²J` exists only under the `de_broglie_clock` branch (`phase_read.cpp:177–178, 338–342`) — **massless by default** | propagating transverse spin-1 `J` modes | any other propagating field |
| 2 | `phase_write` (same pass) | **dissipative** | damping factor + selective mask, `damping = true` by default | amplitude decay | energy conservation |
| 2 | `phase_write` (same pass) | **combinatorial, non-injective** | genesis: `state == 0 && |J|² > K_GENESIS²`, then seeded draw `voxel_uniform(gseed, i, tick)`, then flux drain (`phase_write.cpp:354–358, 381–385`) | particle records from flux | reversible manifestation |
| 3 | `gauss_project` | **elliptic, inexact** | SOR relaxation `φ += ω(gs − φ)` to `1e-18` on the Poisson equation (`poisson_solvers.cpp:94,115`), then one gradient-subtraction `J −= ∇φ`. Source: **"NOT AN IDEMPOTENT PROJECTION"** (`:263`); ~40% of target per application; residual **saturates near `1e-2`** (`:278`) | approximate `∇·J = s` | exact constraint; any propagating mode |
| 4 | `phase_forces` | **algebraic / local**, with an **elliptic** sub-solve | reads potentials, sums EM/gravity/Lorentz/colour into one `f_total`, integrates momentum under the `γ_FTD` bandwidth. Under `poisson_coulomb = true` (default) it first runs `solve_coulomb_poisson` (`phase_forces.cpp:48–49, 112`) | particle velocities; an instantaneous `1/r` potential | new fields; retardation |
| 5 | `phase_movement` | **combinatorial, non-injective** | integer jumps on `|remainder| ≥ 1`; same-sign → elastic bounce; opposite-sign → **annihilation, "both particles return to void"** (`phase_movement.cpp:217, 363`) | discrete transport, collisions | reversible collisions |
| 6 | `weak_transmutation` | **combinatorial, non-injective** | `stress > WEAK_THRESHOLD`, seeded draw, then **sign flip** `set_state(i, −state)` (`transmutation_phases.cpp:38, 45`) | ternary state flips | reversibility (the pre-flip branch is not recoverable from the post-state) |

Two structural observations about the default law follow immediately from the table and
are verified as source assertions:

- **There is exactly one hyperbolic field step.** No phase other than `phase_write`
  integrates `wave_vel` from `δJ`; the only other integrator in the tree is the
  velocity-Verlet KDK half-kick in `render_bridge.cpp:1017–1031`, which is a variant of
  the *same* step behind `verlet_wave_integrator = false` (source: "Default OFF ⇒ dead
  branch").
- **Two elliptic solves run every default tick** — Coulomb (inside `phase_forces`) and
  Gauss — and both are instantaneous in the global-tick frame.

The particle sector (`phase_forces` = kick, `phase_movement` = drift) is a second-order
kinematic integrator for *records*, not a field; it transports localized objects and does
not propagate modes.

## 4. Toggle-gated extensions (off by default)

| step | phase | kind | default | note |
|---|---|---|---|---|
| 1b | `solve_coulomb_poisson` (pre-read) | elliptic | `db_clock_coulomb = false` | second Coulomb path for the de Broglie clock |
| 2a | Verlet second half-kick | hyperbolic (variant) | `verlet_wave_integrator = false` | same field, KDK ordering |
| 2b | `pair_production` | combinatorial | `false` | `|J| > K_GENESIS` threshold, seeded draw, drain — same class as genesis (`transmutation_phases.cpp:97`) |
| 3a / 5b | strong stress-energy | accounting | `false` | begin/complete bracket; not a field integrator |
| 3b | `solve_latency_poisson` | **elliptic** | **`latency_field = false`** | **gravity is off by default** (`render_bridge.cpp:1060–1061`) |
| 4b / 5a | matched-Gauss snapshot/extract | accounting | `false` | |
| 7 | `triad_binding` | combinatorial | `false` | sets a monotone `locked` flag on same-state triples within `TRIAD_RADIUS` (`:223`); no dynamics |
| 7b | `relax_su2/su3_links` | elliptic-type relaxation | `false` | `U_new = Proj[U_old + dt·β·staple†]` (`:358–359`) — first-order gradient descent with projection to the group; no momentum, no propagation |
| 8 | `accumulate_proper_time` | algebraic | gated on **`latency_field ∨ de_broglie_clock`** (`render_bridge.cpp:1167`) — **both false** | `τ += √max(1 − u²/C² − L², 0)`; the source labels this "the selected clock/bandwidth axiom, not a substrate derivation" (`:75`) |

## 5. Findings

**F1 — Only spin-1 propagates, by construction of the schedule.** One hyperbolic field
step, massless by default, and every other field-touching phase is elliptic, first-order
relaxational, or accounting. This underwrites FTD-0209's clause (C2-1) — "2 transverse
spin-1 `J` modes per wavevector + 1 quasi-static scalar ℒ; no rank-2 propagating mode" —
at the level of the schedule itself: the ℒ clause holds not because the free theory is
linear but because `solve_latency_poisson` is a Poisson relaxation with no `Δ_t²` term at
any toggle setting. FTD-0209 tags (C2-1) `[THEOREM]` at free-theory level and `[SMC]` at
canonical-toggle level; for the ℒ clause specifically the schedule supplies the
canonical-toggle argument. The `J`-bilinear clause (C2-2) is untouched — interactions can
bind continua into poles, and this audit says nothing about that.

**F2 — The default law has no gravity and no proper time.** `latency_field` is off, so
the latency solve never runs; proper time is gated on `latency_field ∨ de_broglie_clock`,
both off. The canonical schedule advances on the global ordinal tick alone. Every
gravitational and proper-time result in the corpus is therefore a result about a
toggle-extended law, and should be read as such.

**F3 — Gauss's law is approximate in the default law, and the step is injective.**
`gauss_project` is a gradient-subtraction step, not an orthogonal projector; the
surviving per-mode residual carries the factor `1 − σ_wide(k)/σ_18(k)`, which equals 1
(no correction) where every `k`-component is 0 or π and is never exactly 0 at a grid
`k ≠ 0`. So the map attenuates the infrared longitudinal sector and kills no mode: it is
**injective** and hence **not an expiry event** in P5's sense. The residual floors near
`1e-2` after roughly six applications. The default law is approximately, not exactly,
Gauss-constrained — consistent with the `gauss_law_fidelity` target recorded RED at HEAD.

**F4 — Every long-range interaction other than `J`-radiation is instantaneous in the tick
frame.** Coulomb and Gauss (default), latency and link relaxation (gated) are all
elliptic solves completed within a tick. A composite bound through them has no
retardation in its binding. Retardation is what makes bound states boost-covariant at
`O(u²/C²)`; its absence is a **schedule-level prediction** that engine composites will fail
the two-body Lorentz test at that order. FTD-1009 measured `p̂ = 0.851` against a required
`+1` on a `φ⁴` surrogate whose binding is *local* — it could not see this. The native
engine's binding is elliptic, and the type census says the native result should be no
better.

**F5 — The default law's expiry events are threshold and collision events, not the v3
Hodge-packet map.** Three non-injective steps run by default: genesis (threshold on
`|J|`, seeded draw, drain), annihilation (opposite-sign collision), and the weak sign flip
(threshold on stress, seeded draw). The v3 constitution's *selected first expiry map* —
eight `(normal, hand)` presentations of a phase-2 Hodge packet absorbed into an SC reserve
— does not appear in the engine. The engine's `Φ` and the v3 reference `Φ` differ at the
expiry clause. This is a specific, named gap between constitution and implementation.

**F6 — The default law is dissipative.** `damping = true` with a selective mask; energy
is not conserved by design in the canonical run. Conservation results are
toggle-conditioned.

**F7 — The combinatorial phases are deterministic in `X_n`.** Every "random" draw is
`voxel_uniform(gseed, i, tick)` — stateless and index-keyed, a function of the seed, the
site, and the tick. The seed is state. This satisfies P5's requirement that every
pseudorandom state used by the dynamics be part of `X_n`; the threshold events are
deterministic many-to-one transitions, not stochastic ones.

## 6. What the census underwrites and predicts

| finding | underwrites | predicts |
|---|---|---|
| F1 | FTD-0209 (C2-1), ℒ clause at canonical-toggle level | — |
| F2 | reading of every gravity / proper-time row as toggle-conditioned | — |
| F3 | `gauss_law_fidelity` RED; no undeclared expiry in Gauss | — |
| F4 | — | native two-body Lorentz fails at `O(u²/C²)` (successor to FTD-1009) |
| F5 | — | constitution/implementation gap at the P5 expiry clause requires resolution before the engine can be cited as v3's `Φ` |
| F6 | toggle-conditioning of conservation results | — |

## 7. A listing-based claim refuted by source

Working from the schedule listing alone, `gauss_project` reads as a projection — hence
idempotent, hence many-to-one, hence a candidate undeclared expiry event. The source
refutes every link: the step is explicitly non-idempotent, realises ~40% of its target
per application, and attenuates rather than annihilates. Recorded here so the argument is
not re-proposed from the listing.

## 8. Reproduction

```
python scripts/proofs/proof_schedule_type_audit.py
```

Twenty-one regex assertions against `engine/src` and `engine/include/ftd/term_toggles.h`.
Nothing in this document is transcribed from a listing; every line citation is asserted
by the verifier.
