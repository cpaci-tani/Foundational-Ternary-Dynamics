# FTD-0699 — Internal-excitation symmetry-ray spectrum v2

**Status:** `[MEASURED — CONSTRUCTIVE FINITE-VOLUME CLASSICAL RESONANT TRANSFER]`  
**Production status:** unchanged

## Locked result

The corrected fresh-amplitude `L=113` campaign passes every execution and
constructive gate. After normalizing integrated transverse difference-field
power by the measured deposited-current power on each harmonic, all six
sign/ray arms peak within the registered one-bin neighborhood of the first
internal tick phase `phi_int=1.0911648733663635`.

| ray | closest harmonic | response peak | `Omega_peak` | detuning | allowed | contrast range |
|---|---:|---:|---:|---:|---:|---:|
| `<100>` | 40 | 40 | 1.088231471536037 | 0.00293340 | 0.0170365 | 94.3831–94.3832 |
| `<110>` | 25 | 24 | 1.059378567701002 | 0.0317863 | 0.0411189 | 87.4113–87.4115 |
| `<111>` | 20 | 20 | 1.112068195960989 | 0.0209033 | 0.0556034 | 182.559–182.567 |

All 56 axis and face-diagonal harmonics and 54 body-diagonal harmonics pass the
current-support gate. The eligible-mode sign residuals are
`8.29375276895125e-5` for field power and `4.396238680466864e-7` for current
power. Maximum projection residual is `2.02e-28`.

The parent causal result repeats at the new `5e-8` amplitude: arrivals are
ticks `20,40,60,80,96` at radii `8,16,24,32,40`, shell speed is `0.425532`,
and complete-state recovery is below `3.82e-13`.

## Independent certificate

The independent Python certificate verifies file hashes, all 32,592 row
indices, the analytic dispersion value at every row, complex-coefficient power
reconstruction, response aggregation, eligibility, one-bin tests, contrasts,
sign residuals, and parent inversion. Maximum analytic frequency residual is
`1.78e-15`; maximum reconstructed-power residual is `5.88e-39`.

## Interpretation

The arbitrary-background interpretation of FTD-0694 is now disfavored on the
three tested rays. The localized internal coordinate deposits a broad current
form factor, but the field/current response is sharply enhanced at the native
field modes whose exact tick phases match the internal phase. This is the
classical signature expected when a localized matter oscillator is embedded
in and coupled to a propagating field band.

Post-hoc time-window checks, not part of the locked verdict, find that the
axis/body peaks are already near the resonant bins in ticks 1--32 and the
contrasts grow strongly in ticks 33--96. The face-diagonal peak moves from
harmonic 23 early to 24 later. This is consistent with resonant buildup but is
not separately promoted.

## Ontological consequence

The selected matter candidate now has an operational internal-to-field
selection rule. Its constituent/binding geometry supplies a current form
factor; the native field dispersion supplies the receiving modes; their phase
intersection controls where reversible excitation redistribution is enhanced.
Thus effective decay can be a property of a localized *subsystem* even though
the complete matter-plus-field state is deterministic and reversible.

Stable matter still requires a complete dressed normal mode with zero net
outward flux. This campaign measures one excited coordinate, not the existence
or residue of such a mode.

## Boundary

Only three symmetry rays were observed. The statistic is Fourier morphology,
not an exact per-mode decomposition of the modified leapfrog energy. No
off-ray radiation pattern, lifetime, particle pole, photon number, quantum
transition, universal cone, or Lorentz recovery follows.
