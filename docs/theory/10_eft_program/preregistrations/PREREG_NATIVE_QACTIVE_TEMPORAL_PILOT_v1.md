# PRE-REGISTRATION — Native q_active Temporal Pilot v1

**Date locked/run:** 2026-08-02  
**Identifier:** `FTD-0776`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; CONFIGURATION-SCOPED FALSIFYING PILOT]`  
**Immutable run lock SHA256:**
`3FECCBCC92452DC7C066C6B7A594F65D9358A9E23464D667C7BCC77AD072662E`  
**Post-run provenance erratum SHA256:**
`D7F1C0EB1F3B9FE15BAA3DBACCDC23BF618C9B7E093F5FB6A22F8B6655113BD7`

This tracked document anchors the already executed immutable lock. The exact
pre-run file and its post-run wording erratum are preserved in
[`engine/results/gstar_qactive_pilot_20260802/`](../../../../engine/results/gstar_qactive_pilot_20260802/).
The erratum discloses value-blind performance probes and one quarantined
679-row partial trajectory that predated the formal lock. The lock itself
predated all four complete run-of-record trajectories and was restored
byte-for-byte; no scientific choice was changed post-result.

## 1. Locked question

Does the preselected aggregate

\[
q_{\rm active}(t)=\sum_{s_i\ne0}J_{i,x}(t)
\]

furnish at least eight stable complete cycles and, only if recurrence passes,
agree with the registered quartic occupancy, speed, moment, correlated
`G`-functional, and nonlinear-waveform diagnostics?

`q_active` is the sole primary observable. `q_all` and `q_center` are
descriptive controls and may not replace it after inspection.

## 2. Frozen engine profile

- engine commit `93748ac2021e4db5a9b8583cc28493332c716ac0`;
- no production-source diff in `engine/include/ftd` or `engine/src`;
- WSL2 Ubuntu-22.04, GCC 11.4, CPU backend, CUDA disabled;
- periodic `L=32`, native `dt=1`, six-iteration SOR Gauss map;
- single substrate and deterministic genesis/evaporation seed `1`;
- center injection `(A*K_GENESIS,0,0)`;
- 2,000 transient ticks and 200,000 recorded ticks;
- amplitudes `A={10,12,14,16}`; and
- eight OpenMP threads per arm, with four arms run concurrently.

A post-run source audit (not a retroactive lock change) records the inherited
FULL 18-point stencil, periodic mean-charge-subtracted approximate Gauss map,
charge coupling `1`, default selected genesis threshold/ramp,
`kinetic_drain=0.5`, and effective evaporation under the enabled genesis
path. It also records that injection is a one-time replacement and CSV tick
`0` follows engine tick 2,001. These details further narrow the result's
configuration scope.

Enabled were wave propagation, the production state--flux operator at the
selected/parametric `G_C=sqrt(alpha)`, Gauss projection, and
genesis/evaporation. Langevin, imposed de Broglie/Coulomb clocking, latency,
dual substrate, forces, movement, weak/gauge extensions, and all other
bulk-managed toggles were disabled.

## 3. Frozen analyzer contract

The transferred analyzer SHA256 is
`2CAFF9E8682CA386D5865C5B5B0367DEFD83D502618ADB07A9A92533BB7F28B0`.
Its locked arguments are:

```text
q_column=q_active
time_column=tick
burn=0
phase_samples=4096
min_cycles=8
```

Raw validity requires the exact ten-column schema, exactly 200,000 rows,
ticks `0..199999`, finite numeric values, and nonempty manifested support.

## 4. Ordered gates

The first gate requires at least eight accepted cycles, period CV `<=0.02`,
and amplitude CV `<=0.05`. Only after that gate passes may the campaign test:

1. occupancy exponent `m in [3.8,4.2]`;
2. speed exponent `m in [3.75,4.25]`;
3. the registered fourth, sixth, and eighth moments;
4. the four correlated `G` functionals; and
5. the selected waveform ratio `48*pi/G*^4`.

No fitted power, new observable, nonlinear remapping, or post-result gate
change is permitted. A recurrence failure makes every downstream diagnostic
N/A; it does not license a near-miss search.

## 5. Interpretation firewall

- A positive pilot would be hypothesis-generating only. The production
  coupling already inherits the selected/imposed `G* -> alpha -> G_C` chain.
- The four `G` functionals are correlated summaries of one waveform, not four
  independent determinations.
- The waveform ratio is not a native two-clock coupling measurement.
- A negative is scoped to `q_active` and this exact `L=32`, seed-1,
  single-substrate CPU/SOR profile.
- No outcome here derives proper time, a minimum dimensionless `dt`, a body
  recurrence, gauge structure, phase, color, `SU(2)`, or `SU(3)`.

The immutable lock's broader registered negative label is retained only as
provenance. The canonical post-run verdict must name the tested observable:

```text
Q_ACTIVE_RECURRENCE_UNQUALIFIED_IN_LOCKED_L32_SEED1_CPU_SOR_PROFILE
```

Only a pass of every primary arm could have licensed a separately locked
scale/body campaign. The actual result failed the first gate, so that work
stopped.
