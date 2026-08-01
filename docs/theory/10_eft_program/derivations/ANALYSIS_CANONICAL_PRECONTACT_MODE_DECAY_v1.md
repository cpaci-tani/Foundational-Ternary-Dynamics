# FTD-0676 — Canonical pre-contact mode-decay analysis v1

**Status:** `[SELECTED DYNAMICS — CONSTRUCTIVE PRE-CONTACT EXPONENTIAL TRANSFER]`  
**Verdict:** `CANONICAL_PRECONTACT_EXPONENTIAL_TRANSFER_CONSTRUCTIVE`  
**Production status:** unchanged  
**Locked protocol SHA256:**
`1DCD7CEB1FCF429FDF63CE7251D713C76D9E5B9F80DBD75B4D061E715564A6B6`

## 1. Result

At a fresh maximum constituent momentum of `5e-7`, both polarities reproduce
the canonical energy-decay rate generated exploratorily from FTD-0674:

| quantity | negative | positive | locked gate |
|---|---:|---:|---:|
| `Gamma_E` per tick | `0.00653712338791` | `0.00653712653094` | positive; within 5% of parent |
| parent relative difference | `4.89e-9` | `4.76e-7` | `<=0.05` |
| `BIC(M0)-BIC(M1)` | `412.6959` | `412.6960` | `>=10` |
| linear-log `R^2` | `0.999331992` | `0.999331993` | `>=0.995` |
| tick-8--64 target decline | `0.312296985` | `0.312297095` | `>=0.20` |
| max observer residual | `4.88e-12` | `5.81e-12` | `<=1e-8` |
| reverse recovery | `6.18e-13` | `5.54e-13` | `<=1e-8` |

The two fitted rates differ relatively by `4.81e-7`; the full normalized target
histories have polarity RMS `1.33e-7`. The initial canonical target energy is
`2.79863047247e-12`, one quarter of FTD-0674's value at twice the momentum, as
required in the linear-amplitude limit.

The direct executable run completed all 80 forward and 80 reverse ticks in
987 seconds. The independent certificate recomputes both regressions from all
162 CSV rows and passes.

## 2. Reservoir reading

For the negative arm, the exact normalized decomposition gives:

| tick | target modes | dynamic field | other modes | nonlinear matter | field interference |
|---:|---:|---:|---:|---:|---:|
| 0 | `1` | `0` | `~1.47e-30` | `~-7.11e-13` | `0` |
| 8 | `0.96367434` | `0.03632065` | `2.97e-5` | `5.59e-4` | `-5.82e-4` |
| 64 | `0.66272175` | `0.33730037` | `5.49e-6` | `-6.69e-4` | `6.47e-4` |
| 80 | `0.60156521` | `0.39842219` | `1.73e-5` | `0.00354479` | `-0.00353920` |

At tick 64 the target has lost `0.33727825` of its initial energy while the
dynamic-field self energy has gained `0.33730037`; the remaining exact terms
are small compensating corrections. This is the cleanest current evidence that
the chosen bare matter doublet is coupled to and exports action into propagating
field degrees of freedom.

## 3. What follows

The rate is amplitude stable over the locked factor-of-two test. In the reduced
matter coordinate, an energy law `E_target ~ exp(-Gamma_E t)` corresponds to
an amplitude rate `Gamma_A=Gamma_E/2 ~ 0.00326856` per tick. This is a classical
finite-time response coefficient of the selected auxiliary dynamics.

The result supports the following narrower dynamical picture:

1. the constituent doublet is not an autonomous normal mode;
2. its field coupling is already visible in the linear-amplitude limit;
3. reduced-coordinate damping coexists with exact complete-state inversion;
4. exact dynamics require the complete constituent--field state, but an
   operational matter object may still be the localized ground-state basin
   plus bound dressing, with detached outgoing field assigned to its
   environment.

## 4. What does not follow

This run does not establish irreversible damping, an infinite-volume decay
width, a quantum pole, a localized hybrid resonance, a photon, or a particle.
The lattice is periodic; the protocol merely keeps all observations before the
conservative self-contact bound at tick 81. A finite reversible system can move
energy out of one coordinate and later return it.

The next decisive gate is therefore not another matter-only envelope. It is a
localized-basin relaxation test under increasing volume or a qualified
outgoing boundary: the constituent geometry and bound dressing must approach
the same state-identifiable rest family while detached field energy leaves.
Frequency-resolved complete-system localization and positive residue then
decide whether the relaxation is governed by a hybrid pole or only a transient.

## 5. Run of record

- runner SHA256:
  `F91C1ACB442A0F541A81D130832EB8D57E7528EF90EB5ED571F88577BA834A06`
- JSON SHA256:
  `6592E523EDDC37648A39FE39CFF02FF4371555CAEF6DE830D822114D98858206`
- CSV SHA256:
  `D1BB98C6C178201D9B8A289FD5E3026439D57239BEDDE235FE9010A44B888AA4`
- independent proof SHA256:
  `F0293CC85C8E87E11A6EC1572355EAA5AB09DE6A596C93BB3B7EC6C2C78DA350`
