# PRE-REGISTRATION — Finite-memory reversible lift v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0499`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parents:** `FTD-0495`, `FTD-0497`, `FTD-0498`

## Question

Can a finite local chart label, finite-precision dressing coordinate, or other
finite hidden state make the non-injective production threshold map exactly
reversible while leaving its projected raw `(site,remainder)` update unchanged?

## Locked finite-fiber theorem

Let `f:S->S` have a collision

```text
f(s_1)=...=f(s_m)=t,
m>=2.
```

Let `H` be a nonempty finite hidden-state set. A lift `F:S x H->S x H`
preserves the frozen projected update when

```text
pr_S F(s,h)=f(s)
```

for every `(s,h)`. The `m|H|` inputs in `{s_1,...,s_m} x H` must then map into
the `|H|` outputs in `{t} x H`. By pigeonhole, `F` cannot be injective.

Test preimage multiplicities `m=2` for one collapsed chart axis and `m=8` for
three simultaneous collapsed axes against hidden capacities from `1` through
`2^20`. The cardinality deficit must equal `(m-1)|H|` exactly.

## Locked history control

An unbounded stack can retain the branch information. For a binary merge use

```text
h'=2h+b,
b in {0,1},
```

with reverse `b=h' mod 2`, `h=floor(h'/2)`. Require exact push/pop recovery for
all binary words through length 63 in a `uint64` control and demonstrate that
the required storage grows by one bit per binary merge, or three bits for an
eight-way merge.

This is a mathematical existence control, not an allowed engine variable.

## Locked existing-state discriminator

Use the explicit FTD-0497 colliding chart preimages and require:

1. identical effective position and FTD-0478 polarity distribution;
2. identical exact face current for the common displacement;
3. identical face-field update and field work from a common pre-field;
4. identical momentum output under the axial matched-work transaction;
5. identical FTD-0495 dressing update when initialized with the same `D`.

If these pass, none of the current quotient-side variables stores the erased
chart branch.

## Frozen verdicts

- `FINITE_LOCAL_LIFT_EXISTS` only if an injective finite lift preserving the
  frozen projection is explicitly constructed, invalidating the cardinality
  argument.
- `UNBOUNDED_HISTORY_REQUIRED_FOR_FROZEN_PROJECTION` if the finite-fiber
  obstruction and stack control pass and all current sidecars erase the branch.
- `EXISTING_STATE_ALREADY_CARRIES_BRANCH` if any registered field, momentum,
  or dressing record distinguishes the colliding preimages.

## Scope ceiling

A real number has infinite mathematical cardinality, but an IEEE-754 engine
value is finite-state; moreover the registered FTD-0495 update is tested
directly rather than credited with hypothetical encodings. This audit does not
authorize hiding an unbounded bit stack in a real variable, changing the raw
projection, or quotienting production matter.

## Run-of-record hashes

- test SHA256:
  `57B2D6321F51D94D94040477CBB0465A2EFC7BEA1E85BC0148463A072B06ED32`;
- header SHA256:
  `D593C991597A69DEF1BE389CB69DEE3168F44B1B774FBBBE7D6B30C59D92B092`;
- implementation SHA256:
  `13E2C4E8F4777C38C9AA01260E44A0D823DC89E89E92DA58C3BC5704ED9E5265`;
- toolchain: pinned MSVC `14.44.35207`, Release, CPU observer;
- record: `engine/results/ftd_0499/windows_msvc_cpu.json`.
