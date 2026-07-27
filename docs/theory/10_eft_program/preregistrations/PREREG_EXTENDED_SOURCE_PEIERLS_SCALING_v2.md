# PRE-REGISTRATION — Extended-source Peierls scaling, v2

**Identifier:** `FTD-0555`  
**Status:** `[PRE-REGISTRATION — LOCKED BEFORE RUN; v2 OF RECORD]`  
**Date locked:** 2026-07-26  
**Supersedes before execution:**
`PREREG_EXTENDED_SOURCE_PEIERLS_SCALING_v1.md`.

No source code, campaign binary, or measurement was executed between the v1
and v2 locks. Version 2 corrects one analytically impossible finite-volume
acceptance gate discovered while deriving the observer. All theorem content,
source families, volumes, depths, fit windows, asymptotic constants, and gates
1--6 and 8--9 of v1 are incorporated unchanged by reference.

## Pre-run correction to v1 gate 7

The v1 requirement that raw `L=129` and `L=257` values agree to `1e-8` is
mathematically incompatible with the registered monopole-background source.
Its Coulomb summand behaves as `1/|k|^2` near the omitted periodic zero mode.
The missing low-momentum cell therefore gives

```text
delta U/U = O(sqrt(m)/L),
```

not an exponentially small trapezoidal error. For fixed dipoles the zero-mode
singularity is cancelled, but the directional limit still gives an algebraic
finite-volume correction. The raw-equality gate would reject the theorem for
the finite-volume effect the protocol is meant to expose.

Gate 7 is replaced, before any run, by the following conjunctive test on all
shared depths `m={8,16,32,64}`:

1. both volumes give the same strict ordering of the four source classes by
   `Pi_i` and the same strict decrease of each `Pi_i` with `m`;
2. `|Pi_i(129)/Pi_i(257)-1| < 0.10` in every shared arm;
3. at `m=64`, the `L=257` rescaled `U_0`, `Delta U`, and `Pi_i` constants are
   no farther from their locked asymptotic constants than the corresponding
   `L=129` values, allowing an absolute comparison slack of `1e-12`;
4. `m+1<L` on both quotients, so the registered locally generated envelope
   does not overlap its periodic image.

This is a convergence-direction gate, not a claim that either finite quotient
is the ontology. All other v1 gates and the v1 verdict map remain binding.

## Additional record rule

The run of record must store both v1 and v2 SHA-256 hashes and state explicitly
that v1 was not executed. A failure may not be repaired by changing volumes,
depths, fit windows, profile orientations, or the corrected 10% bound.

**LOCKED CONTENT ENDS HERE.**
