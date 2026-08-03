# FTD-0782 — Movement-Sector Source Map v1

**Status:** `[ENGINE FACT — SOURCE AUDIT]` + `[SYNTHESIS — CAMPAIGN DESIGN]` +
`[OPEN — PAIR-BREATHING CARRIER]`
**Verdict:** `MOVEMENT_SECTOR_MAPPED_CARRIER_CANDIDATE_IDENTIFIED`
**Parents:** `FTD-0545`, `FTD-0549`, `FTD-0560`, `FTD-0574`, `FTD-0739`, `FTD-0772`, `FTD-0781`
**Production impact:** none; source reading and synthesis only

## 1. Result in one sentence

The movement-enabled sector — the unique native home for a carrier per FTD-0781 —
divides on inspection into a **native core** and a layer of **selected/imposed force
phenomenology**; within the native core the first concrete carrier candidate is the
**breathing mode of a bound opposite-polarity pair**, whose observable (the pair
separation `r(t)`) satisfies every FTD-0779 admissibility rule, whose non-resonance
condition reproduces the §32.2 breather corner, and whose decisive unknown — the well
shape, hence `sign(dOmega/dA)` — is a computable property of the selected force
profile.

## 2. The sector inventory, from source

**Kinematics** (`phase_movement.cpp`): velocity accumulates into a per-voxel sub-cell
`remainder`; an integer hop fires on `|remainder| >= 1`; **opposite-sign contact
annihilates** (both states zeroed, flux burst distributed over the 6-neighbour shell).
Relativistic momentum integration via `gamma_FTD`. The hop scheme is the one whose
energy-consistency failures are already registered (Peierls pinning FTD-0550–0555; no
exact-energy mobile transaction FTD-0545/0546/0549).

**Forces** (`phase_forces.cpp`), four families with sharply different epistemic status:

| force | form | status |
|---|---|---|
| EM, three modes | Poisson `-alpha s grad(phi)`; legacy `-alpha s grad(div J)`; emergent `G_C s grad|J|` (tier-2) | native-adjacent; even the emergent mode's `alpha = G_C^2` is a **selected** normalization |
| gravity | `G_N grad(rho)`, `G_N = 0.01` | **[IMPOSED]** toy |
| Lorentz-shaped | `alpha s (v x B)`, `B = curl J` | **[SELECTED]**; source comments themselves cite FTD-0574's proof that this is **not** the common-action partner of the field's `+G_C curl(s v)` source |
| color | pairwise SU(3)-inspired, running `alpha_s(r)`, three regimes, **harmonic confinement** `F ~ r` at long range | **[IMPOSED]** phenomenology |

## 3. The governance split

A breather found with color, toy gravity, or the Lorentz branch enabled would be a
breather **of imposed phenomenology** — an `[ENGINE FACT]` about inserted potentials,
licensing nothing native. The **minimal admissible profile** for a native-core carrier
search is: movement + wave propagation + state–flux coupling + genesis/evaporation +
exactly one EM-mode force, everything else off. Even there, the matter-side force is a
selected normalization and the mobile transaction is registered as **not
energy-consistent** (FTD-0545 closed the fixed-step matter-work identity negative;
FTD-0546 the neutral coupled-pair energy; FTD-0549 locates the obstruction in solver
stages, not endpoint formulas). Consequence, stated plainly: **any carrier result in
this sector is `[ENGINE FACT]`-grade until the reciprocal transaction problem is
solved** — the same honest status FTD-0776 carried. That does not block the search; it
bounds what a positive licenses.

## 4. Reconciliation with the mobile-matter no-gos

The FTD-0550–0577 wall binds **translating** carriers: rigid transport radiates
(FTD-0560–0562's slow-hop resonances), point carriers self-pin (Peierls), free flux
does not solitonize (FTD-0557). A **stationary breathing bound pair** is a different
object — zero net translation, an oscillating *internal* coordinate — and is not
excluded by any registered negative. Three registered results bear on it directly:

1. **Existence of binding**: the exact pair algebra gives a compact bond minimum with
   `3/4 < r^2 < 3/2` and the theorem that an isolated energy-conserving pair **cannot
   disconnect** (registered in the matter-network derivation, FTD-0739-adjacent).
2. **The absorbing wall**: annihilation-on-contact means the pair's radial well has a
   lethal inner boundary. The breathing amplitude is bounded above by the
   contact-avoidance condition — the carrier is *metastable by construction*, and its
   `t_end` (§29 vocabulary) is a physical annihilation or dissociation event.
3. **The non-resonance condition returns**: an internally oscillating pair sheds
   energy into the acoustic band unless its breathing frequency (and harmonics) avoid
   it — precisely the §32.2 breather condition, now attached to a concrete object.
   The dressing-leakage results (FTD-0561's multipole hierarchy) suggest a *neutral*
   pair's radiation starts at higher multipole order, favouring the bound
   opposite-polarity pair over any charged configuration.

## 5. The candidate observable

The pair separation `r(t)` (with its conjugate momentum), in the pair's own frame:

- **fixed support** once the pair is tracked (two sites plus remainders — Phase-3
  `BodyTracker` style, satisfying FTD-0779's fixed-index rule);
- **recurrent** by binding (the well guarantees revisits — the recurrence
  precondition holds by construction, not by luck);
- **body-frame scalar** — exactly the class §29 and the Phase-3 roadmap anticipated;
- **not a linear functional of the field** — it is a function of matter positions, so
  the §32.1 linear-exclusion theorem does not touch it.

## 6. The decisive computable, and what comes before a campaign

The carrier question for this object reduces to the **shape of the pair well** in the
minimal admissible profile: `sign(dOmega/dA)` for radial breathing. Hardening — the
frequency climbing with amplitude toward and above the band — permits a non-radiating
breather; softening — frequency descending into the acoustic band as amplitude grows
toward dissociation, the generic behaviour of chemical-type wells — would kill it, and
would close the native carrier question for the pair channel. Neither is assumed here.

Ordered next steps, none yet executed:

1. **Fix the minimal profile** and confirm from the FTD-0739/0760–0767 campaign
   configurations which toggles the registered bound pairs actually used.
2. **Compute the well**: extract the pair's effective radial potential in that profile
   (statically, from the implemented force at held separations — cheap), and read off
   the anharmonicity sign. This is the FTD-0781-style decision point: derivable before
   any dynamical campaign.
3. **Only then preregister** the pair-breathing campaign: seed a bound pair, track
   `r(t)`, screen with the v2 metrics, Gate B via cycle-count-vs-amplitude, Gate C via
   the flow-curve collapse of §32.2.

## 7. Scope

Source audit covers `phase_movement.cpp` and `phase_forces.cpp` on the CPU path, plus
the registered results cited. Nothing here shows a bound pair breathes stably, that
its well hardens, or that any of this survives the energy-consistency defects of the
hop scheme; the annihilation wall and the non-resonance requirement are standing
threats. The section identifies the first admissible candidate and the cheapest
decisive computation — it does not claim the candidate succeeds. `[OPEN]`.
