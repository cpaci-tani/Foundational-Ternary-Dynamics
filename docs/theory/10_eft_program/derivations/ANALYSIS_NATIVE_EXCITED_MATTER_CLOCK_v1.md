# FTD-0659 — Native excited matter clock v1

**Status:** `[SELECTED DYNAMICS — MIXED]`  
**Verdict:** `NATIVE_EXCITED_MATTER_CLOCK_MIXED`  
**Production impact:** none

## Question and covariant observable

The first internal FTD-0640 frequency is a two-dimensional degenerate cubic
eigenspace, not one preferred eigenvector. FTD-0659 therefore tests the
basis-independent doubled phase

\[
I={|p|^2+\omega^2|q|^2\over2\omega},\qquad
Z=(\omega^2|q|^2-|p|^2)-2i\omega q\cdot p,\qquad
\Theta=\arg Z
\]

for the complete modal doublet `q=(q_6,q_7)`, `p=(p_6,p_7)`. For a linearly
polarized oscillator, `Z=2 omega I exp(2 i theta)`. This removes the arbitrary
choice of basis inside the degenerate eigenspace.

## Execution

All `74` locked arms complete: `72` nonzero histories spanning two cyclic
orientations, three polarizations, three amplitudes, and four quadratures,
plus two exact zero-amplitude controls. Every arm runs `256` forward and `256`
state-only inverse ticks.

| gate/diagnostic | result | threshold |
|---|---:|---:|
| doublet relative splitting | `3.98e-15` | `<=1e-9` |
| preceding stiffness gap | `221.67476` | `>100` |
| common-action residual | `1.9994e-11` | `<=1e-10` |
| total-energy drift | `2.6645e-15` | `<=1e-12` |
| inverse recovery | `4.2489e-11` | `<=1e-10` |
| minimum linear-polarization support | `0.999999999998887` | `>=0.90` |
| doubled-phase frequency error | `0.0010426` | `<=0.02` |
| phase-step RMS | `0.0094887` rad | `<=0.05` rad |
| amplitude phase residual | `2.74e-9` | `<=0.005` |
| amplitude-squared action residual | `1.94e-10` | `<=0.02` |
| quadrature history RMS | `0.0138322` rad | `<=0.05` rad |
| polarization frequency residual | `1.97e-9` | `<=0.005` |
| cyclic frequency residual | `1.87e-9` | `<=0.005` |
| zero-control action and support | exactly `0` | `<=1e-20` |
| maximum matter-doublet action drift | **`0.898691`** | **`<=0.02`** |

Every gate except action conservation passes. The verdict is therefore mixed,
not constructive.

## What the failure means

The doublet retains an exceptionally clean phase while its matter-only action
changes by almost ninety percent. This is not numerical energy loss: total
energy remains constant to `2.7e-15`, the complete state reverses to
`4.3e-11`, and the result is amplitude-, quadrature-, polarization-, and
orientation-stable. Leakage into other matter-mode groups remains below
`0.066` in the registered norm.

The exact location of the complementary action was not a locked observer, so
FTD-0659 does not label it radiation. The data do establish that the bare
constituent doublet is not a closed action--angle subsystem. Its phase is a
coherent projected signal, not yet an autonomous matter clock.

## Ontological consequence

The current strongest matter picture is no longer “a bound constituent core
plus a passive dressing.” The internal coordinate and matched face/edge field
belong to one dynamical object. An excitation prepared only in the constituent
coordinate can coherently lose local modal amplitude while the complete
matter--field state remains deterministic, energy-conserving, and reversible.

That supplies a concrete classical route to effective decay: a localized
coordinate may disperse into additional field/binding degrees of freedom even
when fundamental evolution is invertible. It does not yet prove detached
radiation, irreversible decay, a lifetime, or a particle resonance.

The next discriminator is a direct action-transfer ledger and coupled
matter--field mode construction. A stable matter excitation requires a
localized positive-action eigenmode of the complete tangent dynamics or a
protected nonlinear/topological continuation. If only continuum-coupled
resonances remain under volume scaling, the internal excitation is metastable
rather than an intrinsic clock.
