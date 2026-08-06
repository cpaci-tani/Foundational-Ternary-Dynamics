# FTD-0475 — Bound / Leading-Response / Wake Discriminator v1

**Status:** [PRE-REGISTRATION — LOCKED/RUN; NODAL PROBE DISCLOSED]  
**Date locked:** 2026-07-25  
**Production tick:** frozen; observer-only classification plus selected
production-force readout

## 1. Question and non-equivalences

Do the flux structures surrounding the current travelling neutral packet behave
as a co-moving bound field, a detached trailing wake, symmetric dispersion, or
a field that produces a leading response in a separate manifested probe?

The following are not synonyms and will not be inferred from streamline shape:

1. **bound field:** positive field activity remains attached to the translating
   packet;
2. **wake:** activity is left behind the packet and detaches from it;
3. **leading response:** flux reaches and exerts a registered force on a
   separate probe while the packet core remains behind it;
4. **pilot guidance:** a dynamical manifested history is transported by that
   leading response with reciprocal backreaction.

This campaign can classify 1--3. It cannot establish 4 because its registered
probe is locked, `coupling=false`, and the selected `emergent_forces` branch is
one-way. A positive probe result must therefore be called a **one-way leading
response**, not a pilot wave.

## 2. Frozen packet arms

### 2.1 Finite localized packet

Use the existing public `LocalizedPacketSpec` discrete-curl construction with

- `L in {49,65}`;
- propagation direction `d in {-1,+1}`;
- amplitude `A in {0.5,1.0}`;
- `sigma_x=sigma_t=3`;
- carrier `k=pi/4`;
- initial centre `x0=L/2-10d`, `y0=z0=L/2`;
- source-free production wave tick only, periodic computational window;
- ticks `0,...,32`, sampled at `{0,4,8,12,16,20,24,28,32}`.

The two amplitudes test linear scaling; the two volumes test whether the result
is a local packet property rather than immediate recirculation. The periodic
window is a numerical probe, not an ontological container.

### 2.2 Exact current scenario

Dispatch the current canonical `s0-vacuum-photon` at `L in {49,65}`, retain its
isolated wave/Gauss profile, set only the computational flux boundary to
periodic, and use the same ticks, samples, and observer. This arm answers what
the actual scenario's displayed flux does. Because its `J_z=f(x)` packet is
uniform over each transverse plane, it is not a finite-energy excitation on an
uncontained substrate. It is a scenario morphology arm only.

Total morphology runs: `8 localized + 2 exact-scenario = 10`. Total morphology
rows: `10*9=90`.

## 3. Frozen morphology observer

At each site define the nonnegative activity density

`a(x)=1/2 |W|^2 - L_grad(x)`,

where `L_grad` is the written pairs-once 18-point field-gradient Lagrangian
density. The x-profile is the transverse sum of `a`. This positive quantity is
used only for morphology. The exact source-free tick energy remains

`E*=K+G+X`,

the modified invariant from `native_energy_contract.h`; it is reported
separately and is the sole energy-loss gate.

Using the circular activity centroid, define a co-moving core by directed
distance `|xi|<=R`, with the locked `R=2 sigma_x=6`. `xi>R` is leading/bow and
`xi<-R` is trailing. Record core, leading, and trailing fractions, width,
centroid displacement, normalized divergence, and relative drift of `E*`.

Compare every profile with every integer translation of its tick-zero profile.
Record the maximum normalized overlap. At that best shift, rescale the reference
to the current total activity and record

`f_explained = 1 - ||a_t-a_0_shift||_1/(2 sum a_t)`.

Positive residual outside the core is separately accumulated as leading and
trailing excess fractions. No alternative shell, fitted core radius, activity
density, or fractional shift may replace these definitions after execution.

## 4. Frozen morphology clauses

A run is **CO-MOVING BOUND** when its tick-32 values satisfy all of:

- profile overlap `>=0.90`;
- explained fraction `>=0.85`;
- core fraction at least `0.85` of its tick-zero value;
- trailing excess `<0.05`;
- relative exact-energy drift `<=1e-10`.

A run is **DETACHED TRAILING WAKE** when all of:

- trailing excess `>=0.10`;
- trailing excess is at least twice `max(0.01, leading excess)`;
- core fraction falls by at least `0.10` from tick zero.

A run is **SYMMETRIC DISPERSION** when leading and trailing excess are each at
least `0.05` and differ by no more than a factor two.

The aggregate morphology verdicts are frozen:

- `CO_MOVING_BOUND_PACKET_NO_DETACHED_WAKE`: all 8 localized runs are bound
  and none is a detached wake;
- `TRAILING_WAKE_DOMINATES`: at least 6/8 localized runs satisfy the wake
  clause;
- `SYMMETRIC_DISPERSION_NOT_WAKE`: at least 6/8 satisfy symmetric dispersion
  and none satisfies the wake clause;
- otherwise `MIXED_OR_UNRESOLVED_MORPHOLOGY`.

Scenario-arm clauses are recorded separately and cannot substitute for the
finite-packet aggregate.

## 5. Frozen manifested-probe arm

For every localized `(L,d,A)` cell, run polarity `s=-1,+1`, giving 16 probe
runs and `16*32=512` rows. Use the same packet, set `forces=true` and the
default-off selected `emergent_forces=true`, keep `coupling=false`, and place a
locked manifested probe at the lattice centre. Movement, genesis, evaporation,
Gauss projection, Poisson force, Lorentz force, and every other interaction
remain off.

At each tick record packet centroid and the production
`force_diag.f_coulomb` value at the probe. The historical field name is retained
by the API; in this arm it stores the selected force

`F=G_C s grad |J|_tier2`.

A sample is leading when the directed probe-centroid distance exceeds `R=6`.
The aggregate response is
`ONE_WAY_POLARITY_ODD_LEADING_RESPONSE` only if:

- every one of the 16 runs has maximum leading longitudinal force `>1e-10`;
- for each paired polarity history,
  `max |F_+(t)+F_-(t)|/max(1e-30,|F_+|,|F_-|) <=1e-12`;
- the eight-tick zero-field control has maximum force `<=1e-15`.

Otherwise the response verdict is `NO_QUALIFIED_LEADING_RESPONSE`.

Even a positive result is not pilot guidance: the locked probe supplies no
trajectory, `coupling=false` supplies no reciprocal source, and no common-action
energy/momentum exchange is tested.

## 6. Structural validity

- exactly 90 morphology rows and 512 probe rows;
- all ten packet runs and sixteen probe runs finite and complete;
- observation/no-observation controls have identical wave-state and RNG hashes
  after 16 ticks;
- no production source or tick phase is modified;
- focused CTest passes under pinned MSVC 14.44;
- the seven-test golden gate is attempted and any unrelated repository blocker
  is reported without modifying user-owned work.

This campaign is CPU-scoped because the locked force diagnostic, exact
long-double tick-energy observer, and state/RNG neutrality check are host-side
instruments. The volumes are small; GPU throughput is not part of the measured
claim.

## 7. Locked implementation

The target compiled successfully before lock; it was not executed and no
campaign output existed before this preregistration.

- campaign source SHA-256:
  `0CAF042075C15BE5DC5E5B8763F84916C04D9E95C7951D6936896DDCE3447996`
- morphology observer SHA-256:
  `10F485DBCEAC044C300A710EFCEFD6DDB5FA21B8DE2C0AA1E9AA6FC13278A558`
- localized packet helper SHA-256:
  `8AA0E4DBE189D2EADD277F43A5E7652D8459663F463B0B7654CA623CF02F64BA`
- exact scenario source SHA-256:
  `CDDDD9914588BE30FD539F773B006CED914C86158408BDE675FDC6865855855E`
- shared scenario helper SHA-256:
  `5B6E421ECA88B4B22A17D63E6002343E03028451DD2358F076F0FACB174152B8`

Run-of-record directory: `engine/results/ftd_0475/`.

## 8. Execution note

The locked run completed structurally. The two exact scenario arms passed the
co-moving-bound clause and no run passed the wake clause. The localized packet
aggregate was mixed because translated-profile explained fraction was only
`0.77497`, despite trailing excess remaining `0.02293--0.02311` and exact
energy drift remaining below `9.31e-13`.

The registered on-axis probe returned only roundoff-scale forces. Post-run
inspection identified the preregistered site as the exact discrete-curl nodal
axis. No revision-1 value or threshold was changed. Revision 2 discloses the
result and moves only the probe to the existing FTD-0457 transverse lobe.
