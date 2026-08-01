# FTD-0656 — Mobile dressing structure factor v2

**Status:** `[PRE-REGISTRATION — LOCK BEFORE IMPLEMENTATION/EXECUTION]`  
**Parent:** FTD-0655 execution-invalid  
**Scope:** corrected observer-only rerun; no physical or numerical retuning

## 1. Correction

FTD-0655 is execution-invalid because its locked-arm section said `64w` ticks
per direction while its `T_phys=64`, `a=2/w` definition implies `32w`; the
runner executed `32w`.

This v2 resolves the arithmetic prospectively:

\[
N_{ticks}={T_{phys}\over a}={64\over 2/w}=32w
\]

forward and exactly `32w` in state-only reverse. Thus widths `w={2,3,4}` run
`{64,96,128}` ticks per direction. This is the sole protocol correction.

## 2. Frozen dynamics and observer

Retain verbatim FTD-0655 §§2 and 4:

- unchanged fixed-total-measure common action;
- `a=2/w`, `r_m=r_q=r_kappa=a^3`, `r_beta=a^-1`;
- periodic `L=8w+1`, `T_phys=64`;
- exact cached forward/reverse roots and inherited exact/coherence tolerances;
- independent effective-position constituent mass-density structure factor;
- component-position-aware face/edge field-energy structure factor;
- nearest-`2pi` phase unwrap, locked linear phase fit, amplitude CV, relative
  phase RMS, and centre velocity.

No equation, accepted state, force, normalization, fit, threshold, or output
observable changes from the executed FTD-0655 runner.

## 3. Locked arms

Retain the same 18 arms: at each width, `v=0.03` primaries along `<100>`,
`<110>`, `<111>`, a `-0.03<100>` mirror, and whole-state cubic images along
`<010>` and `<001>`.

## 4. Locked gates

Retain FTD-0655 §5 verbatim:

- inherited exact/coherence gates on every history;
- `mean(|F_m|)>1e-8`, `mean(|F_f|)>1e-12`;
- matter/field phase RMS `<0.10/<0.20 rad`;
- matter/field amplitude CV `<0.10/<0.20`;
- relative-phase RMS `<0.20 rad`;
- matter–centre and field–matter speed mismatch each `<0.10` after division by
  `0.03`;
- mirror and cubic residuals `<1e-8`;
- strict width-two-to-three-to-four decrease of worst field–matter velocity
  mismatch, worst relative-phase RMS, and worst field-amplitude CV.

## 5. Verdict map

Use the same four outcome classes as FTD-0655, suffixed only by the corrected
v2 record:

- `MOBILE_DRESSED_STRUCTURE_FACTOR_V2_CONSTRUCTIVE`;
- `MOBILE_CORE_FIELD_DRESSING_V2_MIXED`;
- `MOBILE_MATTER_STRUCTURE_FACTOR_V2_CLOSED_NEGATIVE`;
- `MOBILE_DRESSING_STRUCTURE_FACTOR_V2_EXECUTION_INVALID`.

The FTD-0655 raw values are not acceptance inputs and are not reused as v2
evidence. The entire 18-arm matrix must be rerun after this document is hashed.
A constructive verdict licenses retarded pole extraction, not a particle,
wake, photon, charge, Lorentz, or production claim.
