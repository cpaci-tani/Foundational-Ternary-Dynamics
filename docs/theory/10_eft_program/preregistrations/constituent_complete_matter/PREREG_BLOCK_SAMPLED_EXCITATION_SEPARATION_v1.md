# PREREGISTRATION — Block-sampled excitation separation v1

**Identifier:** `FTD-0691`  
**Status:** `[LOCKED BEFORE IMPLEMENTATION AND EXECUTION]`  
**Date:** 2026-07-28

FTD-0690 passed initialization but full spatial observation at tick 1 exceeded
five minutes and projected beyond six hours. No checkpoint or result existed.

Keep the `L=113`, tick-96 dynamics, origin, amplitude, radii, reversal, and
physical classes unchanged. Evaluate the expensive component-aware profile and
regional energies at ticks `{0,4,8,...,96}` only. Dynamics still advances and
is exact at every tick.

For every tick compute the global difference-field source exchange

```text
S(t)=H(E_after,B_after)-H(E_pre-current,B_after).
```

Because the source support must remain within radius eight, the same `S(t)`
applies to every registered region. For consecutive sampled ticks `a,b`, define
the exact block boundary transport

```text
T_R[a,b]=U_R(b)-U_R(a)-sum_{t=a+1..b} S(t).
```

Use these block transports in the unchanged outward/inward accumulation.
Arrival is the first sampled tick crossing `0.001` and has declared resolution
`±4` ticks. The late window `80..96` contains five samples. Plateau thresholds
are unchanged. Exact energy/common/source-support gates remain per tick;
spatial observer gates remain per sampled tick. Output exactly 25 sampled
records per sign. No physical result makes execution invalid.

Output: `engine/results/ftd_0691/`. Hash-lock all sources and the binary before
execution.
