# PRE-REGISTRATION — Subcell polarity and straight face current v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0478`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0477` and the user-locked Face-Flux Mobile-Matter Plan  
**Artifacts:** `test_subcell_polarity_shape`, `test_face_current_segment`,
`test_face_flux_normalization`

The implementation was locked by the user-supplied plan before construction.
The hashes below freeze the run-of-record implementation:

| artifact | SHA-256 |
|---|---|
| `subcell_polarity_shape.h` | `CA557171B064A493662981A22C392F1C5C55CDD41D124B22E82239BA7B366199` |
| `face_current_segment.h` | `BA86AA25BD52B80A7D11DF72012F20109DD89830C5DD80F44A6729548E30ECB9` |
| `face_flux_normalization.h` | `AD625FA1D407A0816498C250312BA048287FDFE65B3330695F9B35F867558A3F` |
| `test_subcell_polarity_shape.cpp` | `73460F9F2C0DDAC80A7743A789C47417F76550EB654E6D45B5AEED19EC6057AA` |
| `test_face_current_segment.cpp` | `2DECEB7F8833A76BDF797364661DF0DA5EF56F81A17E77C6B230DF954721DCB8` |
| `test_face_flux_normalization.cpp` | `54EA6A740036B51330D1558BE5CCF7A8AECBCD2014C2AD315DAB7923E4025E43` |

## Question and frozen construction

Can the existing `(site,remainder,polarity)` state define a compact fractional
coupling representation and a unique straight-segment oriented-face current
without changing primitive ternary state?

The frozen shape is the tensor product of the signed one-dimensional hats
`{1-|r|,|r|}` on the anchor and the neighbor selected by `sign(r)`. The path
is the nearest periodic image of the straight effective-position segment. It
is split only at crossed integer coordinate planes. On each piece the product
of the two transverse hats is integrated analytically.

The selected face/native normalization is fixed, not fitted:

```text
z = G_C / C_WAVE^2,
J_face = z E,
K_J = z K,
H_longitudinal scale = C_WAVE^2.
```

It must reproduce both native infrared susceptibility and the work coefficient
of the written `G_C s div(J)` interaction with one `z`.

## Gates

- polarity is exactly `+/-1`; remainders lie in `[-1,+1]`;
- support is at most eight sites per endpoint and local crossed faces only;
- partition, first moment, continuity, locality, translation, cubic rotation,
  and inversion residuals are each `<=1e-12`;
- both polarities are exact signed partners;
- stationary paths carry zero current and threshold/periodic crossings remain
  continuous;
- normalization susceptibility and action-work residuals are `<=1e-15`.

## Outcome map

- all gates pass: `SUBCELL_FACE_CURRENT_EXACT_SELECTED_REPRESENTATION`;
- any algebraic or covariance gate fails: `SUBCELL_FACE_CURRENT_CLAIM_FAILS`;
- invalid or non-finite construction: `PROTOCOL_INVALID`.

Success licenses an observer representation only. It does not derive a force,
particle trajectory, electromagnetic ontology, production toggle, or scenario.

## Run of record

MSVC `14.44.35207`, Release CPU. All three focused targets pass. Worst current
continuity residual is `6.66134e-16`; worst first-moment residual is
`1.77636e-15`; translation/locality/inversion residuals are zero; the
normalization susceptibility and work residuals are exactly zero. Record:
`engine/results/ftd_0478/windows_msvc_cpu.json`.
