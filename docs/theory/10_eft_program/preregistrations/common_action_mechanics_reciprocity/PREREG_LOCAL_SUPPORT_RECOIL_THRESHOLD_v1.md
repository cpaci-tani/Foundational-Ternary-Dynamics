# PRE-REGISTRATION — Local-support recoil threshold v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0456`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; IMPLEMENTATION CORRECTION RECORDED]`  
**Parents:** `FTD-0454`, `FTD-0455`  
**Engine artifact:** `engine/tests/campaign_local_support_recoil_threshold.cpp`

**Locked SHA-256:**

- campaign: `B01E6016F72C6D1929FCC55FA73ECFDA9B58CC35F695B909C0AC027E0D7A0D9A`
- solver: `5103AA5CD427CF57C983606AFDF4CE1A2B49108263324A84D46FBBA11D0CA94B`

**Implementation correction after invalid first execution:** the original solver
computed `K(control_j)` and `K(control_w)` while those arrays were only partly
populated. All 24 analytic arms bracketed, but the direct complete-energy
residual was `5.28e-5` to `9.93e-5` (with one `L=33` global arm at `1.06e-5`),
so the registered `PROTOCOL_INVALID` condition fired. The protocol, campaign,
arms, tolerances, and classification are unchanged. The corrected solver first
copies the complete control fields and only then evaluates the stencil.

- corrected solver SHA-256:
  `60F192FD417EBC94AF7D156EEB2C951AD3A75BF5A2397F7717AD3AE36533D376`

## 1. Question

Does the FTD-0455 travelling-wave-assisted zero-energy recoil exist when the
paired impulse is restricted to a fixed causal neighborhood of the hop, or
does it require access to field degrees of freedom at distances that grow with
the periodic box?

## 2. Frozen arms

- face hop `d=(+1,0,0)`, `q=+1`, speed `0.15`, work `1e-4`;
- exact transverse mode from FTD-0455, `n=1`, phase `0`;
- propagation signs `-1,+1`;
- volumes `L in {11,17,33}`;
- impulse supports:
  - `R=1`: union of source/target Chebyshev balls of radius one;
  - `R=2`;
  - `R=3`;
  - `GLOBAL`: every site, as a reproduction control.

The support mask is fixed before solving. The optimizer re-solves the exact
quadratic inside each supported subspace; it does not truncate or renormalize
the global solution.

## 3. Frozen threshold protocol

For all 24 `(L,sign,support)` arms:

- amplitude bracket `[0,1]`;
- require minimum `>1e-8` at zero amplitude and `<-1e-8` at amplitude one;
- exactly 80 bisection iterations with the upper endpoint non-positive;
- threshold-side analytic minimum magnitude `<=1e-10`;
- directly constructed zero-energy impulse must close complete event energy and
  central momentum to `1e-10`;
- impulse values outside the registered support must be exactly zero;
- control work residual `<=1e-12`;
- all Gram systems nonsingular and all values finite.

For each local support report site count, threshold amplitude, threshold pure-
wave energy, ratio to the paired global threshold, participation sites, and
direct residuals.

## 4. Locked classification

- `R1_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED`: every `R=1` arm
  crosses and closes;
- otherwise `R2_...` if every `R=2` arm crosses and closes;
- otherwise `R3_...` if every `R=3` arm crosses and closes;
- `NO_FIXED_R3_TRAVELLING_WAVE_RECOIL_THRESHOLD`: no local radius succeeds but
  every global control does;
- `MIXED_LOCAL_SUPPORT_RECOIL_THRESHOLD`: partial local or global arms;
- `PROTOCOL_INVALID`: any registered algebraic, support, or residual gate fails.

The smallest successful radius is the result; larger-radius data remain
diagnostic.

## 5. Interpretation boundary

A fixed-radius crossing proves local kinematic existence under the chosen
travelling background. It does not derive which zero-energy impulse is selected
by the five postulates, insert it into the production tick, establish edge or
corner transport, or prove no-superluminal signalling over successive events.

No production dynamics are changed and no numerical near-match search is run.

## 6. Recorded outcome

All 24 arms passed after the recorded implementation correction. Every `R=1`
arm bracketed and closed with exact outside-support zero; worst direct energy
and momentum residuals were `4.00e-15` and `9.77e-16`.

**Verdict:** `R1_LOCAL_TRAVELLING_WAVE_RECOIL_THRESHOLD_CONSTRUCTED`.

The local threshold amplitude and total wave energy increase strongly with
volume, so the verdict establishes fixed-radius kinematic existence but not a
finite-energy isolated infrared excitation.
