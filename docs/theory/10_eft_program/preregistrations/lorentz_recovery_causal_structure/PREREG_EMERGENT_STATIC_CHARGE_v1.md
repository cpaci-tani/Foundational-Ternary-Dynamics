# PRE-REGISTRATION — Polarity-sourced static-charge discriminator v1

**Date locked:** 2026-07-22  
**Identifier reservation:** `FTD-0426`  
**Status:** `[PRE-REGISTRATION — FROZEN PRODUCTION TICK]`  
**Campaign:** `engine/tests/campaign_emergent_static_charge.cpp`  
**Observer:** `engine/include/ftd/eft/emergent_charge_surface.h`  
**Lock:** `scripts/proofs/emergent_static_charge_lock.json`  
**Verifier:** `scripts/proofs/proof_emergent_static_charge_lock.py`

## 1. Claim under test

The primitive ternary signs are treated only as **polarity**. The campaign asks
whether spatially separating initially neutral polarity pairs produces an
operational, static electric-charge analogue in the flux field. It does not
assume that `s=+1/-1` is already physical electric charge.

Two logically distinct gates are frozen:

1. **Gauss-readout gate:** after an actual production movement event separates
   polarity, a closed surface around each body carries equal and opposite flux,
   with sign reversal under a global polarity mirror.
2. **Autonomous-dressing gate:** the same surface flux remains stable under the
   live wave/coupling/Gauss evolution, and is not merely rewritten on each tick
   by the explicitly imposed `div(J)=s` projector.

Passing gate 1 alone establishes only a polarity-sourced Gauss-law realization
inside the selected engine rule. It is not a derivation of `U(1)`, charge
conservation, Maxwell dynamics, or the empirical electric coupling. The exact
production-event additive-charge gate already closed negative in `FTD-0421`;
this campaign cannot override that algebraic result.

## 2. Frozen ontology and production rules

The five postulates, tick order, stencils, constants, and toggle implementations
are frozen. The new surface observable is read-only and does not alter
`RenderBridge`, event ordering, state, or RNG state. No physical constant is a
target. `coulomb_charge_coupling=1` is the existing engine default, not a fitted
parameter.

The campaign uses the single-substrate path to avoid adding chirality as a
hidden source. The periodic lattice has even `L>=32`. The primary run is WSL2
CUDA at `L=64`; an independent Windows CPU run at `L=32` is required.

## 3. Closed-surface estimator

For the cube `R=[a,b]^3`, the measured central-stencil surface flux is

$$
Q_{\partial R}=\frac12\sum_{y,z}
[J_x(b+1,y,z)+J_x(b,y,z)-J_x(a,y,z)-J_x(a-1,y,z)]
$$

plus the two cyclic terms in `y` and `z`. This is the exact boundary form of
the engine's central-difference divergence. The observer also computes

$$
Q_{\mathrm{div}}=\sum_{i\in R}\nabla_c\cdot J_i,
\qquad
Q_s=\sum_{i\in R}s_i,
$$

and the finite-periodic Gauss target

$$
Q_G=\sum_{i\in R}(s_i-\bar s).
$$

The algebraic telescope must satisfy
`|Q_boundary-Q_div|<=1e-12*(1+|Q_div|)`. This identity checks the observer,
not the physics.

Cube radii are frozen to `r={3,4,5,6}`. The two centers are
`A=(L/4,L/2,L/2)` and `B=(3L/4,L/2,L/2)`, so the surfaces do not overlap or
cross the periodic seam.

## 4. Preparation and evolution protocol

Two mirror arms are run with orientation `q=+1` and `q=-1`.

- Body A begins with a locked core `q` and one mobile polarity `-q`.
- Body B begins with a locked core `q` and one locked polarity `-q`.
- Thus each body and the entire lattice begin neutral.
- Gauss projection alone runs for 128 ticks with 30 SOR sweeps per tick.
- The mobile `-q` site is transported from A to B by the production movement
  phase at speed `0.99*C_SPEED`; the initial sub-voxel remainder is primed so
  the first hop occurs on the next tick. No teleporting state edit is admitted.
- The arriving site is locked. Gauss projection alone then runs for 256 ticks.
- The separated configuration runs for 128 further ticks under the live
  low-energy profile: `wave_propagation`, `coupling`, `damping`, and
  `gauss_projection` on; `selective_damping` on; manifestation, reactions,
  forces, movement, clocks, gauge links, and phenomenological potentials off.

Measurements are recorded at `neutral`, `projected`, and `live` stages. The
preparation is an imposed analogue of rubbing/contact electrification; this
campaign does not derive the material-specific transfer mechanism.

## 5. Frozen acceptance metrics

For each stage, body, orientation, and radius, the CSV record stores
`Q_boundary`, `Q_div`, `Q_s`, `Q_G`, telescope residual, Gauss residual, and
backend metadata.

The Gauss-readout gate passes only if all conditions hold on CPU and CUDA:

- neutral baseline: `max_r |Q_boundary| <= 0.10` for both bodies;
- separated sign: `sign(mean_r Q_A)=q` and `sign(mean_r Q_B)=-q`;
- nontrivial amplitude: `min(|mean_r Q_A|,|mean_r Q_B|) >= 0.50`;
- equal/opposite: `|mean_r Q_A+mean_r Q_B| <= 0.10`;
- radius plateau: `(max_r Q-min_r Q)/|mean_r Q| <= 0.15` per body;
- polarity mirror: the `q=-1` means equal the negatives of the `q=+1` means
  within absolute error `0.10`;
- Gauss fidelity: `max_r |Q_boundary-Q_G| <= 0.15`.

The autonomous-dressing gate applies the same sign, amplitude,
equal/opposite, plateau, and mirror conditions to the `live` stage, and also
requires `|Q_live-Q_projected|<=0.10` for each body mean.

CPU/CUDA means must agree within absolute error `0.10`. This is an engine
reproduction bound, not an experimental precision claim.

## 6. Locked outcomes

| outcome | exact interpretation |
|---|---|
| A: both gates pass | `[EMERGENT — restricted low-energy engine sector]`: transported polarity supports an operational static flux charge under the frozen live profile; microscopic conservation, `U(1)`, Maxwell recovery, and coupling normalization remain open/negative as stated elsewhere |
| B: readout passes, autonomous dressing fails | `[SELECTED CONSTRAINT REALIZATION]`: the Gauss projector converts primitive polarity into a charge-like flux readout, but the engine has not produced an autonomous electromagnetic dressing |
| C: readout fails | `[CLOSED NEGATIVE]`: even the selected Gauss rule does not produce radius-stable equal/opposite static charge under the frozen protocol |
| D: backend or transport contract fails | `[INVALID CAMPAIGN]`: no ontological inference; repair only the observer/fixture and rerun under a new versioned lock |

## 7. Explicit non-claims

No force-law test is used for promotion because the engine's Poisson-Coulomb
force takes signed state as an explicit source and would be circular here. A
passing surface test does not show that flux is photons, that photons are
lattice sites, that physical electric charge is exactly `s`, or that the weak
transmutation/genesis event set conserves this effective charge.
