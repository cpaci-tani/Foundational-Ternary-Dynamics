# AUDIT — Quadratic-coat fixed-step matter work

**Date:** 2026-07-26  
**Identifier:** `FTD-0545`  
**Status:** `[THEOREM — UNIFORM-WITNESS NONIDENTITY] + [CLOSED NEGATIVE —
UNIVERSAL FIXED-STEP MATTER-WORK IDENTITY]`  
**Verdict:** `COAT_FIXED_STEP_MATTER_WORK_NOT_IDENTITY`  
**Pre-registration:**
[`PREREG_QUADRATIC_COAT_MATTER_WORK_v1.md`](../10_eft_program/preregistrations/PREREG_QUADRATIC_COAT_MATTER_WORK_v1.md)  
**Theorem:**
[`THEOREM_QUADRATIC_COAT_MATTER_WORK_NONIDENTITY.md`](../10_eft_program/derivations/THEOREM_QUADRATIC_COAT_MATTER_WORK_NONIDENTITY.md)  
**Run of record:** `engine/results/ftd_0545/windows_msvc_cpu.json`

## Result

The analytic endpoint variation of the FTD-0542 coat action is internally
sound, but its fixed-step Legendre map does not obey the exact matter-work
identity required by FTD-0544. In the locked uniform harmonic witness,

```text
pi0=p-beta q E/2,
pi1=p+beta q E/2,
D=H(pi1)-H(pi0)-beta<E,K>.
```

All 72 registered masses/momenta, field amplitudes, polarities, axes, and
proper-cubic diagonal arms matched the exact analytic formula. The maximum
nonzero defect was

```text
4.1017724139665729e-05,
```

which exceeds the locked `1e-8` nonidentity gate by more than three orders of
magnitude. Controls closed as follows:

```text
direct versus deposited action       3.4694469519536142e-18
analytic endpoint/work formula        1.3322676295501878e-15
gauge/pure-gauge endpoint/work defect 1.1934897514720433e-15
test failures                         0
```

This is not numerical drift. It is the exact central-difference defect of the
massive nonlinear dispersion, beginning at cubic order in the half impulse.

## Scope

The universal identity is closed negative. The selected smooth coat still has
exact current, spacetime continuity, gauge coupling, and exact field-side
Poynting exchange; none of those statements is retracted.

The witness uses an external uniform harmonic field. It is not a periodic
self-field for one net charge, so FTD-0545 alone does not close a neutral,
self-consistent coupled transaction. FTD-0546 subsequently executes that gate
and closes the frozen minimal common action negative there as well. No
production state, default, force, phase, toggle, scenario, energy projection,
or tolerance changed.

## Reproducibility

- test: `test_quadratic_coat_matter_work`, `72` registered arms, failures `0`;
- preregistration SHA256:
  `F26250FBB79F4DFA4470CFFAA14A1290F677510DDFAEFED88FC47170AB1CCCC8`;
- test SHA256:
  `09C2CB89F21854B9C36554FAE952E23755FE602D7D1FA1CCFAC1E6A65D6F7617`;
- header SHA256:
  `348F0DBBF0BDF9E2BDA4169A96B52A6E214E998A5C3AC6683B16A39E47A97F69`;
- source SHA256:
  `9CC5E6B2078D80FB5C65FF92EBA452B4E7AEEF6F098AC0CF9572E214C178CDF3`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer.
