# PRE-REGISTRATION — Native dynamical polarity response v2

**Date locked:** 2026-07-23  
**Identifier:** `FTD-0429`  
**Status:** `[PRE-REGISTRATION — VERSIONED AFTER INVALID v1 EXECUTION]`  
**Supersedes for execution:** `PREREG_NATIVE_DYNAMIC_POLARITY_RESPONSE_v1.md`  

## 1. Reason for versioning

The v1 Windows/MSVC CPU `L=32` and WSL2 CUDA `L=32` runs completed. Two
identical WSL2 CUDA `L=64` attempts were terminated at host allowances of ten
and thirty minutes before the buffered CSV closed. The partial `L=64` file was
empty, so no `L=64` response value was observed. This is outcome D at the
instrumentation layer, not a physical result.

The bottleneck is the read-only full-volume host Fourier projection. The
production GPU tick did not fail. V2 changes only which redundant control arms
are repeated at `L=64`; it does not change the source construction, engine
sector, Fourier estimator, time fit, operator prediction, tolerances,
infrared models, or locked outcomes in v1.

## 2. Normative inheritance

Sections 1–7 of v1 remain normative except for the execution matrix amendment
below. In particular, v2 retains exactly:

- the native `wave_propagation + coupling` sector with both Gauss mechanisms
  off;
- the three directions and harmonics `n=1,2,3`;
- sixteen phase samples over two periods;
- the complex fixed-pole fit and `Z_exact(k)`;
- every `10^-8`, `10^-7`, `10^-6`, BIC, intercept, and RMS threshold;
- the four outcome interpretations and explicit non-claims.

## 3. Versioned execution matrix

- `L=32`, profile `full`: the complete v1 matrix of 16 arms. This supplies
  both polarity mirrors in all directions and duty controls `1,2,4` along
  `(1,0,0)`. Windows/MSVC CPU and WSL2 CUDA/GCC must both complete it.
- `L=64`, profile `infrared`: six primary positive-orientation arms only:
  three directions times base harmonics `b=1,2`, all at `duty=2`. WSL2 CUDA/GCC
  must complete it.

The `L=64` profile retains all nine lower-momentum susceptibility points used
by the locked constant-versus-zero infrared model. Mirror and amplitude
independence are validity controls evaluated on the complete, independently
reproduced `L=32` matrix; they are not refitted at `L=64`.

The campaign accepts `--profile full` or `--profile infrared`. Profile
selection changes only the arm list. A profile that omits or adds an arm is an
invalid record. The existing completed `L=32` files remain admissible because
their arm list and every estimator are byte-for-byte the v2 `full` profile.

## 4. Lock consequence

The v1 source lock is preserved as provenance. Campaign profile selection and
this v2 document receive new source hashes before the first v2 `L=64` run.
No v1 timeout-truncated output may enter the result manifest.
