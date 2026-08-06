# FTD-0705 — Moving dressed-matter transverse-field growth v1

**Status:** `[PRE-REGISTRATION — LOCKED BEFORE EXECUTION]`  
**Production status:** unchanged  
**Execution prerequisite:** FTD-0704 constructive  
**Field observer:** FTD-0696 carrier-aware matched face/edge spectrum  
**Current observer:** FTD-0702 carrier-aware deposited face-current spectrum

## 1. Question

Does the coherent selected dressed-matter candidate drive a growing transverse
field at finite-volume modes phase matched above the exact FTD-0700 axial
threshold, while the same modes below threshold and collinear-current controls
do not show comparable growth?

This is a finite-horizon resonant-field discriminator. It does not by itself
establish detached radiation, an asymptotic photon, or an infrared matter pole.

## 2. Frozen histories

- periodic `L=65`, FTD-0638 orientation-0 geometry centred at `(32,32,32)`;
- minimum-energy matched-field redressing, fibre limit `8`, tolerance `1e-13`;
- positive axial production velocities `v={0.35,0.45,0.50}` assigned to all
  16 constituents;
- 24 complete common-action forward ticks followed by 24 reverse ticks;
- exact local-residual and sparse-current representations from FTD-0692;
- no legacy force, reaction, damping, collision, post-hoc recoil, or production
  tick.

The `v=0.35` arm is below
`v_edge=2 asin(1/sqrt(3))/pi=0.3918265520306073`; the other two are above.

## 3. Frozen periodic modes

All wavevectors are `k=2pi(n_x,n_y,n_z)/65`:

| label | mode | intended role | `|Omega-v k_x|` |
|---|---:|---|---:|
| `R45` | `(31,9,0)` | transverse phase match at `v=0.45` | `0.002107115970665596` |
| `R50` | `(24,5,0)` | transverse phase match at `v=0.50` | `0.0023086346200831276` |
| `C45` | `(26,0,0)` | collinear current control at `v=0.45` | `0.03158323690864484` |
| `C50` | `(22,0,0)` | collinear current control at `v=0.50` | `0.0055042725497549405` |

At `v=0.35`, the `R45` and `R50` detunings are respectively
`0.3017667229284613` and `0.3456831670083248`. No mode may be added after
execution.

## 4. Frozen observables

At every forward tick and every mode:

1. subtract the initial complete field;
2. compute the carrier-aware matched spectrum and lattice-transverse
   projection;
3. project electric field and deposited current onto
   `e_perp=(khat_y,-khat_x,0)/|khat_xy|`;
4. multiply by `exp(+i k_x Delta X_cm)` to enter the co-moving source phase;
5. record complex field/current coefficients, transverse powers, projection
   residuals, source speed, shape, strain, energy, and common residual.

On ticks `8,16,24`, also record the magnetic-only component-aware radial profile
about the nearest current centre, including the norm fraction outside radius
`6`. This morphology is supporting evidence only.

For ticks `9..24`, fit the complex co-moving electric coefficient to
`z(t)=a+b t`. Record `|b|`, complex `R^2`, mean deposited transverse-current
density, and `response=|b|/mean|j_perp|` when the denominator is nonzero.

## 5. Locked algebra and execution gates

- all 3 arms complete 24 forward and 24 reverse ticks;
- every common residual and energy drift is `<=1e-10`;
- complete inverse distance is `<=1e-9`;
- maximum chart multiplicity `<=8`, finite same-anchor separation `>=0.9`,
  RMS shape change `<=0.05`, edge strain `<=0.05`, transverse centre motion
  `<=1e-8`;
- mean speed differs from its target by at most `0.05`, axial increments stay
  positive, and their coefficient of variation is `<=0.15`;
- every field/current spectrum is valid and every projection residual is
  `<=1e-12`;
- `R45@0.45` and `R50@0.50` mean transverse-current density is nonzero and
  exceeds `1e-12`;
- the corresponding collinear control transverse-current power fractions are
  `<=1e-20`.

## 6. Locked discriminator

Define:

- `Q45=response(R45@0.45)` and `Q50=response(R50@0.50)`;
- below-threshold controls `B45=response(R45@0.35)` and
  `B50=response(R50@0.35)`;
- collinear field controls by dividing the fitted `C45@0.45` and `C50@0.50`
  field-slope magnitudes by the current denominator of their paired resonant
  mode, yielding `K45,K50`.

The resonant-growth discriminator passes only if, for both pairs:

- complex fit `R^2>=0.80`;
- final co-moving field amplitude is at least twice its tick-9 amplitude;
- `Q >= 5 max(B,K)`.

## 7. Verdicts

- `MOVING_DRESSED_MATTER_RESONANT_TRANSVERSE_GROWTH`: all execution and both
  resonant-growth gates pass;
- `MOVING_DRESSED_MATTER_DYNAMIC_TRANSVERSE_NO_THRESHOLD_SEPARATION`: execution
  and nonzero transverse response pass, but either registered contrast fails;
- `MOVING_DRESSED_MATTER_NO_TRANSVERSE_RESPONSE`: execution passes but either
  selected phase-matched current or field response is absent;
- `MOVING_DRESSED_MATTER_FIELD_EXECUTION_INVALID`: initialization, algebra,
  observer, coherence, source-quality, or inversion fails.

The first verdict is evidence for a lattice-Cherenkov-like driven field channel,
not yet proof of detached radiation. A later causal-separation test must remove
the launch transient and distinguish co-moving dressing from escaping field.
