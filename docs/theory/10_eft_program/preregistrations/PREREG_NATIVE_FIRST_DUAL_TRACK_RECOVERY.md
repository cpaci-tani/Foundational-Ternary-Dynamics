# PRE-REGISTRATION — Native-first dual-track recovery

**Date locked:** 2026-07-22  
**Identifier reservation:** `FTD-0420` through `FTD-0425`  
**Status:** `[PRE-REGISTRATION — FROZEN CORE, 70/30 NATIVE/AUXILIARY]`  
**Production-rule lock:** `scripts/proofs/native_dual_track_lock.json`  
**Verifier:** `scripts/proofs/proof_native_dual_track_lock.py`

## 1. Non-negotiable boundary

The five postulates and the production tick are frozen for this campaign.
Read-only instrumentation may observe events but must consume no RNG values,
change no state, and alter no event ordering. Observer-on and observer-off runs
must have identical selected-state and RNG hashes.

No exact all-orders Lorentz claim is required. Dimension-four preferred-frame
operators must nevertheless be absent, protected, driven small, or carried as
explicit calibrations. No numerical match to `alpha`, a mass, or another
physical target may promote a result.

## 2. Reserved milestones and dependencies

| slot | milestone | execution rule |
|---|---|---|
| `FTD-0420` | program lock and production-rule hashes | unconditional |
| `FTD-0421` | exact native additive-charge gate | unconditional |
| `FTD-0422` | native on-shell common-cone gate | only if `FTD-0421` finds a nontrivial charge |
| `FTD-0423` | native dimension-four blocking flow | only if `FTD-0422` passes |
| `FTD-0424` | auxiliary gauge-independent pole match and one-counterterm trajectory | independent auxiliary track |
| `FTD-0425` | injectivity, spectral positivity, operational signals, and SME comparison | SME comparison only after `FTD-0424` is gauge independent |

A failed dependency records a scoped non-execution, not a repair attempt.

## 3. Exact native charge gate

The local discrete feature basis is frozen as

$$
f=(|s|,s,h,sh),
$$

where `h` is the sign of the dual-substrate chirality density and is zero when
no chirality label exists. Flux magnitude and fitted nonlinear functions of
flux are excluded.

The exact transition rows include single- and dual-substrate genesis of both
signs, neutral pair production, both signs of single- and dual-substrate weak
transmutation, plus the reverse evaporation/annihilation rows. Movement has
zero global feature change. The left nullspace is calculated over the
rationals before any campaign measurement.

**Pass:** at least one nonzero additive vector annihilates every transition
row and its event-derived current closes locally below `1e-12` for every event
class and after `b=2,4` blocking.  
**Closed negative:** the exact nullspace is trivial. Snapshot reaction
bookkeeping remains valid but is not a conserved `U(1)` current.

## 4. Conditional native pole and flow gates

The on-shell campaign uses `L={32,64,128}`, modes `n={1,2,3}`, cubic direction
classes `<100>/<110>/<111>`, and three predeclared response amplitudes. It fits

$$
M_0:\Delta c=A(ka)^2+B(ka)^4+C/L^2,
$$

against

$$
M_1:\Delta c=\delta c_0+A(ka)^2+B(ka)^4+C/L^2.
$$

It passes only with identifiable poles, width/frequency below `0.05`, amplitude
independence, `Delta BIC >= 10` favoring `M0`, and a 95% engine-resolution bound
`|delta c0| < 1e-6`.

The blocking campaign freezes the complete cubic-symmetric dimension-`<=4`
operator basis before measurement and uses `b={1,2,4,8}` at `L>=64`. Every
preferred-frame marginal coefficient must vanish by an exact identity or have
a stable negative RG eigenvalue.

## 5. Auxiliary pole/counterterm gate

The auxiliary link/Wilson action remains a selected EFT, not native emergence.
The pole mismatch is evaluated at `xi={0.5,1,2}` in QED_L-like and positive-mass
IR prescriptions, retaining exchange, Wilson seagull, fermion bubble, and
two-photon contact terms. Gauge/IR extrapolations must agree within `1e-5 g^2`.

Exactly one universal relative-cone counterterm `eta` may be fixed by photon/
lightest-Dirac pole equality at `ka=2pi/128`. It may not be retuned for another
mass, charge, multiplicity, or threshold.

## 6. Unitarity and empirical gate

Every tick phase is classified for injectivity. The source-free linear sector
must have unit-modulus transfer eigenvalues, positive pole residues, and the
exact tick energy. Around stable manifested states, the spectral-density
matrix must be positive semidefinite and the extrapolated non-injective-event
rate per oscillation must be compatible with zero below `1e-6`.

No SME comparison is licensed until a gauge-independent pole coefficient
exists. Operational closure additionally requires two independent clocks/rods
and a no-faster-manifestation-signal campaign.

## 7. Stop rule

New constant formulas, geometric particle identifications, and mass matches do
not enter the active research queue until the charge, common-cone, and
unitarity gates pass. Reverse engineering remains admissible only when tagged
as phenomenological model construction.
