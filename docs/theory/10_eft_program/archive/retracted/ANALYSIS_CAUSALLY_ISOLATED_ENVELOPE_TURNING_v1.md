# Causally isolated action-envelope turning v1

**Campaign:** FTD-0670  
**Status:** `[RETRACTED — MODAL MASS-METRIC ERROR, FTD-0675]`  
**Historical verdict:** `CAUSALLY_ISOLATED_ENVELOPE_TURNING_CONSTRUCTIVE`  
**Production impact:** none

> **Retraction (FTD-0675, 2026-07-28):** the paired displacement coordinate
> used below was `v^T delta x` for a basis normalized by `v^T M v=1`. The
> canonical coordinate is `v^T M delta x`. The legacy diagnostic overweights
> potential energy by `1/M_INERTIAL^2` and can create false trough/recovery
> sequences for a constant-energy harmonic mode. The historical run and
> hashes are preserved below as provenance, but none of its turning,
> recurrence, or reservoir-return conclusions may be cited.

## Result

The FTD-0668 excitation was rerun on `L=97` with maximum constituent momentum
amplitude reduced from `8e-6` to the preregistered held-out value `4e-6`.
The horizon remains tick 80, one tick before the conservative source-to-source
periodic contact time 81. The initial excited/control face and edge fields are
bitwise equal, actual current support remains within radius four, and all
complete-action, energy, sector, and state-only inverse gates pass.

Both polarity signs exhibit the same strict local-trough sequence over the
locked late window:

| tick | 60 | 63 | 66 | 69 | 72 | 75 | 78 |
|---:|---:|---:|---:|---:|---:|---:|---:|
| normalized doublet, `-` | `1.18129` | `0.96026` | `0.79076` | `0.68005` | `0.631280908` | `0.64330` | `0.710966515` |
| normalized doublet, `+` | `1.18129` | `0.96026` | `0.79076` | `0.68005` | `0.631280888` | `0.64330` | `0.710966485` |

Thus the last three pre-minimum troughs descend, the first two post-minimum
troughs ascend, and the tick-78 recovery increments are
`0.0796856071` and `0.0796855973`, above the locked `0.05` gate. Primary ticks
agree exactly, primary ratios differ by `2.05e-8`, and recovery increments
differ by `9.79e-9`.

At tick 80 the positive difference-field norm is `~0.550207` of initial
doublet action, only `~0.235603` remains inside radius eight, and the radial
second moment is `~493.887`. Complete inverse recoveries are
`5.61e-13` and `5.54e-13`.

## What is learned

The late turning is not an artifact of FTD-0668's failed `0.60` threshold.
It survives a fresh half-amplitude launch under a classifier fixed before the
new history existed. Because excitation energy is quadratic, this run uses one
quarter of the parent's initial doublet energy.

A non-gating post-result comparison finds the full normalized doublet histories
agree with FTD-0668 to RMS `4.62e-8` (`-`) and `3.49e-8` (`+`), with maximum
differences below `9.2e-8`; normalized positive field norms agree below
`9.7e-9`. This is descriptive evidence that both campaigns lie in the same
linear-response regime.

The narrow dynamical conclusion is therefore constructive: the selected
localized constituent core participates in an amplitude-stable coherent
exchange with binding and a distributed face/edge field reservoir. The core's
action envelope turns upward before a signal can traverse the unused periodic
arc back to the source. The behavior is not monotone dissipation and does not
require information loss.

## Ontological consequence

The candidate object's instantaneous matter coordinates are not a closed
system. Its persistent dynamical identity belongs to the complete relational
state containing the localized constituent organization, its binding
relations, and the causally coupled field memory. The result strengthens the
complete-state ontology of FTD-0669 and weakens any ontology that identifies
matter with a fixed occupied site or a matter-only Markov state.

It still does not decide whether the complete excitation is a localized hybrid
normal mode, a resonance, or a finite-time coherent superposition of continuum
field modes. That decision requires an impulse-response/spectral calculation
and spatial near/far decomposition, not another amplitude threshold.

## Run of record

- protocol: `92B98E746C02BAA980A43AF8C9E84B8CF6B5DC8161968511DBF14365D8237412`;
- runner: `982ABC83170B77660D8002B34F8037BF84BECAA730166F95D10623EED15DBCD0`;
- JSON: `631BFCD005E5B223641260F8D1A59442EAFDFCF88565B8EEDBEAC8E4F228DC10`;
- tick CSV: `8C3CBCDAC9137114B2A17202FA04FF77362465D13BE94636C19493A1A31F347A`;
- independent certificate: `67A15989584987685568DD627F1C95314BFAD516C64768FF39D59E2ACD35D2C9`.

## Scope boundary

FTD-0670 establishes a held-out, causally pre-self-contact classical envelope
turning in the selected action. It does not establish an infinite-volume bound
state, resonance width, positive pole residue, quantum stationary phase,
particle, photon, charge, lifetime, Lorentz recovery, or production ontology.
