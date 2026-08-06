# FTD-0696 — Matched symmetry-ray spectrum observer v1

**Status:** `[PRE-REGISTRATION — OBSERVER QUALIFICATION]`  
**Production status:** unchanged  
**Campaign class:** deterministic algebraic/numerical measurement qualification

## 1. Purpose

FTD-0695 derives the constant-frequency group-velocity surface of the native
matched face/edge field. FTD-0694 records radial spreading but no wavevector
content. This protocol qualifies an observer that can measure native field
content on the cubic symmetry rays without treating a morphology norm as a
Fourier energy or a front speed as a group velocity.

No connected-matter history is measured in this campaign.

## 2. Frozen observer

For integer wavevector `n=(nx,ny,nz)` on an `L^3` periodic lattice, set

\[
k_a=2\pi n_a/L,
\qquad
\widehat k_a=2\sin(k_a/2).
\]

The complex Fourier coefficient of each component uses its actual carrier:

- `E_x`: `(x+1/2,y,z)`; `E_y`: `(x,y+1/2,z)`;
  `E_z`: `(x,y,z+1/2)`;
- `B_x`: `(x,y+1/2,z+1/2)`; `B_y`: `(x+1/2,y,z+1/2)`;
  `B_z`: `(x+1/2,y+1/2,z)`.

Coefficients are normalized by `L^-3`. The transverse electric coefficient is

\[
E_T=E-\widehat k\,{\widehat k\cdot E\over|\widehat k|^2},
\]

and likewise for `B_T`. The reported nonnegative morphology power is

\[
P_T=|E_T|^2+c^2|B_T|^2.
\]

`P_T` is explicitly not the exact modified leapfrog energy. The observer also
reports longitudinal power, total coefficient power, and projection residuals.

The first implementation supports arbitrary nonzero integer wavevectors; the
later matter campaign will register the `<100>`, `<110>`, and `<111>` rays.

## 3. Locked qualification arms

Use `L=31` and exact periodic fields with modes `n=3,5,7`.

1. zero field;
2. nonfinite-field rejection;
3. longitudinal face field constructed from the matched forward gradient;
4. transverse face field constructed as `matched_curl(edge potential)` for
   each of `<100>`, `<110>`, and `<111>`;
5. both independent transverse polarizations where nondegenerate;
6. positive and negative amplitudes;
7. amplitudes `A=1e-4` and `2A`;
8. all integer translations by one site along each axis;
9. cyclic cubic copies of every symmetry family;
10. a two-mode superposition with distinct registered wavevectors.

The tests use direct complex sums independent of any FFT library.

## 4. Acceptance gates

All valid arms must satisfy:

- finite and valid result;
- zero field produces exactly zero coefficients and power;
- nonfinite input is rejected;
- a pure longitudinal mode has transverse/total power below `1e-24`;
- a curl-generated mode has longitudinal/total power below `1e-24`;
- unoccupied registered modes have power below `1e-24` of the occupied mode;
- doubling amplitude multiplies power by four within relative `1e-12`;
- changing amplitude sign negates every complex coefficient within `1e-12`
  relative while leaving power unchanged within `1e-12`;
- integer translation changes coefficient phase by
  `exp(+i k dot delta)` under the frozen transform convention and preserves
  power within relative `1e-12`;
- proper cubic copies preserve power within relative `1e-12`;
- the two-mode superposition coefficient equals the corresponding single-mode
  coefficient at each occupied wavevector within relative `1e-12`;
- transverse plus longitudinal reconstructed coefficient equals the original
  coefficient within absolute `1e-14`.

Any failed algebraic gate closes this observer version. Tolerances are not
changed after execution.

## 5. Allowed conclusion

Passing qualifies an observer for symmetry-ray Fourier morphology. It does not
show that matter emits a resonant field, that the unobserved Brillouin zone is
empty, or that the measured power is a particle/photon count or the exact
field-energy partition.

## 6. Failure consequence

Failure blocks the proposed spectral matter campaign until the carrier phase,
projection, or covariance defect is identified in a newly registered observer
version. No matter-dynamics conclusion may be drawn from an observer failure.
