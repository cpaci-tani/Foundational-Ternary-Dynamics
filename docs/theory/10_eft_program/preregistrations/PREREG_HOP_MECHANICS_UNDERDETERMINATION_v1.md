# PRE-REGISTRATION — Hop-mechanics underdetermination v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0444`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0443` discrete interaction-work contract  
**Engine artifact:** `engine/tests/campaign_hop_mechanics_underdetermination.cpp`  
**Campaign SHA256:** `1f3dfc3874939404eeb805c6a08c8f34b30244b99fb465badbf61ec32da347ba`  
**Helper SHA256:** `c966e6c9963774b941363e03788fadec7f71d17ceaffd24f0fd65307fbe2f045`

> **Successor correction (FTD-0450):** the selected energy convention below
> is not the production flat diagnostic; it is exactly three times production
> energy when evaluated on production momentum. This historical locked
> protocol is preserved unchanged below. Its reversible construction is
> synthetic and is superseded by the corrected production-compatible example.

## 1. Question

FTD-0443 proves exact scalar work for a state hop. FTD-0444 asks:

> Does that scalar work uniquely determine a force, a particle momentum update,
> and a local field recoil?

## 2. Independent underdetermination statements

For displacement `d` and work `W`, every

$$
F=\frac{W}{|d|^2}d+F_\perp,
\qquad F_\perp\cdot d=0
$$

has the same work. Thus force has a two-parameter transverse ambiguity.

An energy increase fixes the magnitude of an on-shell momentum but not its
direction. Momentum conservation fixes the sum of field recoil but not which
field sites receive it. Even adding a quadratic recoil norm does not select a
site, because translating the same deposit preserves both constraints.

## 3. Frozen checks

- all 26 Moore displacements;
- five force representatives per displacement with common work `1e-4`;
- six distinct axis momentum states on one flat energy shell;
- two four-cell field-recoil configurations with identical summed recoil and
  quadratic norm but different support;
- one explicitly selected longitudinal momentum update that preserves
  transverse momentum and the sign branch;
- forward work `+1e-4`, reversed displacement/work, all 26 directions.

The selected particle energy convention is the production flat diagnostic

$$
E(p)=\sqrt{M_{inertial}^2+|p|^2/C_{speed}^2}.
$$

The required total field recoil is defined only as `p_before-p_after`; no local
flux update is invented.

## 4. Locked gates

- force-family work residual `<=1e-14` and transverse separation `>=0.1`;
- energy-shell residual `<=1e-13` with distinct momentum states;
- alternative field deposits agree in total recoil and quadratic norm to
  `1e-15` while remaining distinct;
- selected-map forward/reverse energy residuals `<=1e-13`;
- selected-map round-trip and total-recoil residuals `<=1e-12`.

## 5. Locked outcomes

- `REVERSIBLE_SELECTED_MAP_BUT_LOCAL_DYNAMICS_UNDERDETERMINED`: all degeneracy
  and reversible-map gates pass.
- `SELECTED_MAP_NOT_REVERSIBLE`: degeneracy checks are valid but the selected
  longitudinal map fails its closure gates.
- `UNIQUE_MECHANICS_FORCED`: all three registered degeneracies fail while the
  protocol remains finite.
- `PROTOCOL_INVALID`: any other or nonfinite result.

## 6. Interpretation boundary

Passing the reversible map does not derive it. Preserved transverse momentum,
branch selection, and global recoil bookkeeping are explicit selections. A
unique local dynamics requires additional spacetime-link structure in the
action or a separately justified interpolation/transport law.

## 7. Banned moves

- No transverse family, shell states, recoil supports, work, energy convention,
  or gates may change after first execution.
- No selected map may be called native emergence.
- No production tick changes.
