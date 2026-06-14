# PRE-REGISTRATION — Proton-stability forcedness audit (FTD-0301)

**Status:** `[PRE-REGISTRATION]` — design lock; run of record follows the hash-lock.
**Date:** 2026-06-13
**LEDGER id (reserved):** FTD-0301
**Git tag (to be applied at lock):** `preregister-proton-stability-v1`
**Executes:** the "micro" pier candidate after the dark-matter halo (FTD-0300). Is the
proton's infinite lifetime (`τ_p = ∞`) a *forced* prediction of FTD's discrete ontology
— the sharpest claimed divergence from grand-unified theories, which predict decay — or
is the proton at best emergent-metastable while FTD's own native dynamics actively decay
it? (The forcing-vs-fitting question of FTD-0269/0290, applied to baryon stability.)

---

## §1 · Purpose and narrow target

`proof_complete_sm.py:460–471` tags **`τ_p = ∞` as a `[THEOREM]`**, resting on four
asserted props: (a) charge conservation is exact; (b) the proton is a *locked triad* of
three same-sign particles; (c) locked triads are exempt from evaporation; (d) weak
transmutation "flips polarity but **preserves triad structure**", so there is "no
mechanism for baryon-number violation." This audit asks whether any of that is *forced*.

**Single narrow question:** under FTD's native dynamics, does the engine's proton (a uud
triad) (a) remain protected by the triad lock, (b) resist decay, and (c) conserve its
only exact charge `Σs` — so that `τ_p = ∞` is forced — or does it fail at least one of
these, making the stability unforced (emergent metastability at best)?

**Not in scope:** the *numerical value* of any proton lifetime (FTD fixes no second-scale
here); the proton *mass*; promotion or demotion of any LEDGER claim. This adjudicates
only whether baryon stability is FORCED.

## §2 · Frozen definitions

- **Proton seed** `[DEFINITION]` = `ctor::proton(rb, {C,C,C}, radius)` — three quark cores
  at an equilateral triangle, `uud = (+1,+1,−1)` (constructors_bulk_matter.cpp:45–68).
  **Same-sign control** = `uuu = (+1,+1,+1)` at the same vertices (a *lockable* object).
- **Core states** = the state at the three seeded vertex indices `ci[0..2]`, read each
  tick. **`tau_persist`** `[DEFINITION]` = first tick the core-state multiset changes from
  the seed; **`fail_mode`** ∈ {`intact`, `evaporation` (a core → 0), `transmutation` (a
  core flips sign, multiset sum changes), `evap+transmute`}.
- **`Σs`** `[DEFINITION]` = `rb.charge_sum()` = total lattice state sum = FTD's only exact
  U(1) vector charge. (Baryon number has **no** operator in FTD — it is an emergent
  cluster label, not a conserved current. §4.)
- **Lock** `[DEFINITION]` = `locked_final` = number of the 3 cores with `voxel.locked` set
  by `triad_binding`. `triad_binding_cpu` skips any pair with `state != state`
  (transmutation_phases.cpp:148,153) — it locks **only same-sign** triples.
- **Heated arms** `[DEFINITION]` — drive stress past `WEAK_THRESHOLD = K_GENESIS ≈ 1.533`
  so `weak_transmutation` can fire:
  - `heat=inject` (`genesis=off`, `dual=on`): a single localized flux pulse `±10·K_B` at
    the d-quark neighbours over one tick after a 200-tick warmup — the proven
    `campaign_weak_transmutation` recipe; `inject_flux` fills `flux_L` so it reaches the
    `compute_stress_left` that `weak` reads under `dual_substrate`.
  - `heat=langevin` (`genesis=off`, `dual=off`): the OU thermostat at temperature T,
    `gamma=0.02`. NOTE `[FROZEN — disclosed]`: under `dual=on` the thermostat writes only
    the merged `wave_vel`, which is overwritten each tick by `wave_vel_L+wave_vel_R`
    (phase_write.cpp:217,243) and never reaches `flux_L` — so the physical-heating arm
    runs `dual=off`, where `weak` reads `compute_stress` (the single substrate) that the
    thermostat does drive. This is an engine plumbing fact, not a physics null.
- **Frozen thresholds:** `WEAK_FIRE_MIN = 0.50` (a heated `weak=on` arm "fires" iff ≥ 50%
  of seeds transmute); `CONTROL_MAX = 0.00` (the `weak=off` control must show exactly zero
  transmutation — there is no other state-flip mechanism with `genesis=off`).

## §3 · Frozen artifacts

| Artifact | SHA256 |
|---|---|
| `engine/tests/campaign_proton_stability.cpp` | `56fe09548e98787e66b988161b4e57e66f42aa235c8ac4ea7930c8009058bd48` |
| `scripts/exploration/analyze_proton_stability.py` | `eb076a16c4033cd869ac4bdfd08e49862d3a3a01a4b13e89e41229529786a8e0` |

Observation-only measurement (new TU; no edits to any `phase_*.cpp`, kernel, or constant
default). Golden-neutral: `test_render_bridge_golden` = `0x56fa28acb5b9fe88`, to be
verified green with the new TU present at lock. The analyzer encodes the §5 verdict and is
frozen before the run of record. Run of record is **CPU** (L=32; Langevin is CPU-only at
runtime; every cell is bit-reproducible given its seed — `seed_rng` keys both the OU noise
and the weak-flip RNG, render_bridge.cpp:256).

## §4 · Prior information (disclosed for integrity — NOT a blind test)

Two legs are analytical/source-level (no engine needed); a directional scout (un-registered,
before this lock) established the engine direction. Disclosed:

- **Analytical — no forced baryon symmetry.** FTD's postulates contain one exact vector
  charge (the U(1) `Σs`) and **no** baryon-number or B−L current. Baryon number is an
  *emergent* 3-quark-color-singlet cluster label. `Σs` conservation does **not** forbid
  `p → e⁺ + π⁰` (charge-balanced products). The `[THEOREM]`'s prop (a) is true but does
  not protect the proton; props (b)–(d) silently promote "baryon number" to a law FTD
  never derives — and contradict FTD's *own* baryogenesis sector, which invokes
  weak-transmutation B-violation (`DERIV_LATTICE_CHIRAL_ANOMALY.md §6`,
  `CHECKLIST_PHYSICS.md:534`).
- **Source-level code fact.** `triad_binding` locks only same-sign triples
  (transmutation_phases.cpp:148,153); the proton is mixed-sign `uud`. **The triad lock
  cannot fire on a real proton** — so prop (c)'s protection (locked-triad evaporation
  exemption) is structurally inapplicable to its target object.
- **Scout (CPU, L=32, seeds≤8):**
  - *Cold* (`heat=none`, `genesis=on`): **metastable, not lock-protected** (`locked=0`
    on every proton seed). radius-1 8/8 intact over 2000 ticks; radius-2 ~1/3 of seeds
    lose the d-quark to **evaporation** (`Σs 1→2`, not conserved), identical with weak
    on/off (the cold proton's stress is below `WEAK_THRESHOLD`, so weak is inert).
  - *Heated `inject`* (`genesis=off`): `weak=on` **7/8 seeds transmute** `uud → uuu`,
    `Σs 1→3` (`ΔQ=+2` = one clean d-quark flip); `weak=off` **0/8** (the flip is entirely
    the weak channel). A too-strong `genesis=on` pulse instead detonates the lattice
    (FTD-0274 thermal ignition) — excluded; the clean arm uses `genesis=off`.
  - *Heated `langevin`* (`genesis=off`, `dual=off`): the physical thermostat **also fires
    weak** and transmutes the proton at `T≥0.3` (`τ` as low as 4 ticks; `Σs` swings
    {−1,1,3} as quarks flip) — corroborating that the result is not an artifact of the
    hand-placed pulse.
- **Prior-favoured outcome: UNFORCED-METASTABLE `[BOUNDARY]`.** `τ_p = ∞` is expected to
  be unforced: the lock cannot protect the proton, no postulate forbids decay, and FTD's
  weak channel transmutes the proton while violating `Σs`. The §5 thresholds are fixed
  independent of the scout magnitudes.

## §5 · Frozen verdict logic (analyzer-encoded)

The verdict reads **discrete outcomes only** (decay yes/no, lock fires yes/no, `Σs`
conserved yes/no). No fitting, no near-miss search.

- **D1 — cold stability + lock** (proton, `heat=none`, `genesis=on`): does the proton ever
  lock (`max locked_final == 3`)? cold decay fraction at radius 1 and 2.
- **D2 — weak fires & breaks the proton** (proton, `heat∈{inject,langevin}`,
  `genesis=off`): `weak=on` transmutation fraction (fires iff `≥ WEAK_FIRE_MIN`);
  `weak=off` transmutation fraction (control; clean iff `≤ CONTROL_MAX`).
- **D3 — charge (`Σs`) conservation across decay:** any proton decay row with
  `q_final != q_init` ⇒ the only exact FTD charge is violated.
- **D4 — same-sign control:** uuu `max locked_final` (contrast — the lock protects an
  artificial same-sign object, not the proton).
- **Composite verdict:**
  - **STABLE-FORCED** iff the proton never decays in any arm AND the triad lock protects
    the proton (`max locked == 3`) AND `Σs` is always conserved.
  - **UNFORCED-METASTABLE `[BOUNDARY]`** iff the proton decays in ≥ 1 arm (cold
    evaporation OR heated `weak=on` transmutation with the `weak=off` control clean) OR
    the lock never fires on the proton OR `Σs` is not conserved in a decay.
  - **INDETERMINATE** otherwise (e.g. `weak=on` fires but `weak=off` also fires ⇒ channel
    not isolated).

## §6 · Run of record (frozen invocation)

CPU build (`engine/build/Release`, deterministic + seeded; Langevin CPU-only).

```
# Arm S — cold stability, both radii, both species, weak on/off (genesis on)
campaign_proton_stability --species=proton,samesign --weak=on,off --heat=none --radius=1 --genesis=on --seeds=16 --ticks=2000 --cpu --tag=ror_cold_r1
campaign_proton_stability --species=proton,samesign --weak=on,off --heat=none --radius=2 --genesis=on --seeds=16 --ticks=2000 --cpu --tag=ror_cold_r2
# Arm H-inject — controlled weak fire (genesis off, dual on, single pulse)
campaign_proton_stability --species=proton --weak=on,off --heat=inject --genesis=off --radius=1 --warmup=200 --heat-dwell=1 --heat-amp=10 --seeds=16 --ticks=260 --cpu --tag=ror_inject
# Arm H-langevin — physical thermostat weak fire (genesis off, dual off)
campaign_proton_stability --species=proton --weak=on,off --heat=langevin --genesis=off --dual=off --heat-T=0.3 --radius=1 --seeds=16 --ticks=500 --cpu --tag=ror_lan_T0p3
campaign_proton_stability --species=proton --weak=on,off --heat=langevin --genesis=off --dual=off --heat-T=0.8 --radius=1 --seeds=16 --ticks=500 --cpu --tag=ror_lan_T0p8
python scripts/exploration/analyze_proton_stability.py --dir engine/results/proton_stability --prefix ror
```

## §7 · Pre-declared outcomes

- **OUTCOME A:** the prior-favoured **UNFORCED-METASTABLE `[BOUNDARY]`** — the proton's
  stability is not forced; the cited lock mechanism is structurally inapplicable, FTD's
  weak channel transmutes the proton, and `Σs` is not conserved across decay. A publishable
  boundary (cf. FTD-0269/0290): the proton pier does not open as a forced FTD prediction,
  and the `proof_complete_sm.py` `τ_p = ∞` `[THEOREM]` is corrected to `[SELECTION]` /
  emergent-metastable.
- **OUTCOME B:** **STABLE-FORCED** — the proton resists every arm, the lock protects it,
  and `Σs` is conserved; `τ_p = ∞` survives as a forced result. (Contradicted in advance
  by the §4 source facts; would require the scout to be non-reproducible.)
- **OUTCOME C (indeterminate):** the weak channel is not isolated (control also fires) or
  the arms disagree irreconcilably → fix the instrument and re-register (v2).

## §8 · Pre-declared exclusions (banned moves)

1. `WEAK_FIRE_MIN=0.50`, `CONTROL_MAX=0.00`, the heated-arm recipe (amp `10·K_B`, the
   `genesis=off` isolation, `dual=off` for the thermostat), and the species/radius grid are
   frozen — no post-hoc adjustment to move a verdict.
2. The `genesis=off` isolation is mandatory for the weak-fire arms: a `genesis=on` pulse
   that detonates the lattice (global manifestation, `Σs → L³`) is a thermal-ignition
   artifact (FTD-0274), **not** proton decay, and is excluded from the transmutation count.
3. The same-sign `uuu` arm is a control, not the proton; a verdict on baryon stability is
   not evaded by citing the lockability of the artificial same-sign object.
4. No search for engine parameters that *would* stabilize the proton; the run of record
   uses the default constants.
5. An UNFORCED-METASTABLE-BOUNDARY does **not** demote any algebraic-spine result, the
   N_c = 3 derivations, the SM mass identifications, or weak-transmutation's status as an
   `[IMPOSED]` electroweak analog — it adjudicates only whether `τ_p = ∞` is FORCED.
6. Zero promotions: FTD-0013 `[SMC]`, MC-T4.3, FTD-0110/0261/0269/0290, and the SM sector
   claims are unchanged regardless of outcome. The only epistemic change a BOUNDARY licenses
   is correcting the `proof_complete_sm.py` `τ_p = ∞` tag from `[THEOREM]` to the honest
   `[SELECTION]` / emergent-metastable, recorded in the LEDGER row.

## §9 · Hash-lock declaration

This document, the campaign instrument, and the analyzer are committed together and tagged
`preregister-proton-stability-v1` BEFORE the §6 run of record executes. The §3 SHA256
hashes bind the artifact versions. Any post-lock edit to §§2, 5, 6, 8 or to the artifacts
invalidates the lock and requires a v2.
