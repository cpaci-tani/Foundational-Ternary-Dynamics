# Audit — FTD-0725 lower-energy covariance conditioning v1

**Status:** `[AUDIT PASS — NUMERICAL CONDITIONING CONFIRMED / FULL RERUN
REQUIRED]`  
**Date:** 2026-07-29

## Findings

1. **The parent defect is reproduced.** The baseline condition returns the
   exact FTD-0724 scalar maximum `1.0680766715509549e-8`.

2. **Tighter roots close both covariance gates.** Scalar and complete-state
   maxima become `8.9040e-10` and `3.5992e-10`, below `1e-9` and improved by
   factors `11.996` and `9.587`.

3. **Every ontological layer converges.** Tight electric, magnetic, and matter
   component maxima are individually below `1e-9`; convergence is not confined
   to a scalar energy diagnostic.

4. **The raw energy classes are stable.** Both numerical conditions retain
   `104/130` unbound histories raw-negative and all 26 bound controls. Every
   origin/translated pair agrees.

5. **The defect is localized to matter phase space near the boundary.** The
   worst baseline complete-state difference occurs at `p=0.0095`, direction
   `1_-1_1`, tick 39, with matter as the dominant component.

6. **FTD-0724 remains unresolved.** A fresh numerical result cannot waive a
   failed locked gate. FTD-0725 licenses a new full tighter-root rerun only.

7. **No physical or production dynamics changed.** Tighter termination and a
   larger iteration budget act on the same exact residual. Production defaults
   remain untouched.

## Correct statement

The FTD-0724 covariance miss is a finite root-solver conditioning effect in the
registered matrix: a ten-times tighter solve produces translation-equivalent
complete histories below `1e-9` without changing raw energy-sign classes. The
existence and stability of a formed matter object remain open.

## Verification

- preregistration SHA-256:
  `712F491F72E9F30239060406FAA85EBB0F3635DFD3A8BD2143CBF68249A7DCB9`;
- runner SHA-256:
  `4424F879FACDF56917F5E2FE4C11E41A71169E1337DAC457D72448E91CF4B54D`;
- result JSON SHA-256:
  `829A76E2187F389318D71C8D3035957FD29E106D61D1DFBF0220006463E9E89E`;
- result CSV SHA-256:
  `C82EFD332A1D6CD02FD339E9F63D6B6BB58AD31BE5549AA22F53A962D27E32BD`;
- independent certificate:
  `95D9BE29F5E8300D0C6FDA2034C1B486E7A5C0FFC8C20740E3AAE6414404AF9B`,
  `88/88 PASS`;
- focused CTest: `1/1 PASS`;
- production defaults, tick, toggles, and scenarios: unchanged.
