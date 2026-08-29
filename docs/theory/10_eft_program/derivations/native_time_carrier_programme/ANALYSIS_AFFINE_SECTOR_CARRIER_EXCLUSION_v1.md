# FTD-0781 — Affine-Sector Carrier Exclusion v1

**Status:** `[ENGINE FACT — SOURCE AUDIT, PROFILE-SCOPED]` +
`[THEOREM — GIVEN THE AUDITED UPDATE RULE]` +
`[OPEN — CARRIER IN THE MOVEMENT-ENABLED SECTOR]`
**Verdict:** `AFFINE_SECTOR_CARRIER_CLASS_EMPTY_IN_LOCKED_PROFILE`
**Parents:** `FTD-0567`, `FTD-0574`, `FTD-0772`, `FTD-0776`, `FTD-0778`, `FTD-0779`, `FTD-0780`
**Production impact:** none; source reading and derivation only

## 1. Result in one sentence

In the FTD-0776/0779 locked profile, the engine's continuous dynamics is **affine** and
its only nonlinearities are **dissipative threshold events**, so the conservative
anharmonicity is identically zero, the hard-versus-soft question has the answer
**neither**, and the carrier class in this profile is **empty by construction** — the
unique native sector where a carrier could live is the movement-enabled coupled
matter–field sector, which is FTD-0772's registered target, now derived from source
rather than inferred from failures.

## 2. The audited update rule

Locked-profile toggles (`PREEXECUTION_LOCK.md`): ON — wave propagation, state–flux
coupling, Gauss projection, genesis/evaporation. OFF — Langevin, imposed de Broglie
clock, latency, dual substrate, **forces, movement**, and all other bulk-managed
toggles.

The per-tick continuous update came from the retired DagEngine CPU source
(`recursive_read`/`recursive_write`, recoverable from Git history through
baseline `21566b63`) — the path the campaign ran:

```text
delta = C_WAVE^2 * laplacian(J)          [wave_propagation]
      - G_C * grad(s)                    [coupling, electric source]
      + G_C * curl(s * velocity)         [coupling; velocity static with movement OFF]
W += delta ;  J += W                     [symplectic-Euler pair]
```

With movement OFF, `s` and `velocity` change only through genesis/evaporation events, so
between events every term is **linear in `(J, W)`** and the state sources are constants:
the smooth sector is affine. Gauss projection (six-iteration SOR) is likewise linear.

The genesis event (`transmutation_phases.cpp`, `pair_production_cpu`): at a void site
with `|J| > K_GENESIS`, with probability `1 - exp(-(|J|-K_GENESIS)/K_MANIFEST)`:

```text
wave_vel *= 0.5                              [kinetic drain]
flux     *= max(0, 1 - K_GENESIS/|J|)        [radial subtraction]
state    -> +-1 pair along the flux major axis
```

This is a **drain plus state conversion** — it removes field energy and manifests
matter. It has no restoring branch. Evaporation is likewise a drain (FTD-0567/0569,
registered: the genesis transaction is many-to-one and absolutely irreversible in
projection).

## 3. The theorem

**Given the audited rule, in this profile:**

1. **Below threshold, with fixed state:** the dynamics is *exactly* linear. By the
   linear-exclusion theorem (sidebranch §32.1: a linear functional closes iff it is an
   eigenprojection, and is then harmonic), every closing observable is harmonic with
   `dOmega/dA = 0` **identically**. FTD-0780's measurement (`dOmega/dA = 0` to
   `1.6e-3` across a `4x` amplitude range) is thereby **explained as an exact profile
   property**, not an approximation: the doublet was harmonic because *everything
   smooth in this profile is harmonic*.
2. **Above threshold:** there is no conservative anharmonicity to have a sign.
   Crossings drain energy and convert field to state; `Omega(A)` is undefined; excess
   amplitude cascades until everything is sub-threshold and linear again. This is the
   mechanism of the observed FTD-0776 thermalisation: injections at `10-16x K_GENESIS`
   fired genesis repeatedly, spread manifested pairs along flux axes, drained, and left
   linear waves dispersing over the whole lattice — `active_count -> 32768`.
3. **Hybrid state-flip cycles** (recurrences in which the ternary state participates)
   decay in this profile: every genesis firing drains, evaporation does not refund
   (FTD-0567), and with Langevin off there is no drive. No perpetual hybrid recurrence
   exists in the conservative locked profile.

**Therefore the §32.3 binary — hard or soft — has the answer NEITHER, and the carrier
class of the locked profile is empty by construction.**

## 4. Where a carrier could live — exhaustively, given the source

- **Latency ON** is the engine's *only smooth nonlinearity* (energy-dependent causal
  budget). But it is the imposed-gravity machinery, documented in source as selected,
  not derived — a carrier found there would not be native. Excluded by the program's
  own rules, not by dynamics.
- **Movement ON** makes the state sources dynamical: the field remains linear *given*
  the sources, but the sources' motion depends on the field, so the coupled
  matter–field composite is genuinely nonlinear. **This is the unique admissible native
  sector**, and it is precisely FTD-0772's next-falsifier ("an autonomous coupled
  matter–field recurrence") — previously inferred from measurement failures, now
  derived from the update rule.

## 5. Consequence for FTD-0779 — registered before the dumper is built

As configured (profile matched to FTD-0776), the FTD-0779 screen **cannot return a
carrier**: every registered channel lives in an affine-plus-drains system, so the
outcome for each is guaranteed to be harmonic, not-closed, or uninformative.

The screen is **not thereby worthless** — its purpose changes, and for the better:

1. **It becomes a test of this theorem.** The affine analysis predicts
   `dOmega/dA = 0` *exactly* for every mode-projected channel in this profile. Any
   measured nonzero anharmonicity at sub-threshold amplitude **falsifies the source
   audit** — a sharp, cheap check with a locked prediction.
2. It validates the v2 instrument on native data and surveys the mode structure the
   coupled-sector search will need.
3. The carrier search itself moves to a **movement-enabled profile** — a successor
   preregistration (FTD-0779-v2 class), not an edit to the current lock.

## 6. Scope

Source audit covers the CPU path the campaign executed: the retired DagEngine
source (wave + coupling), `transmutation_phases.cpp` (genesis), the Gauss projector's linearity, and
the locked toggle list; evaporation is characterised from registered results
(FTD-0567/0569) rather than fresh line-by-line reading. The theorem is scoped to the
locked profile: it says nothing about latency-enabled, movement-enabled, Langevin, or
dual-substrate dynamics, and it does not claim the substrate has no carrier — it
locates the only sector where one could natively exist. `[OPEN]` there.
