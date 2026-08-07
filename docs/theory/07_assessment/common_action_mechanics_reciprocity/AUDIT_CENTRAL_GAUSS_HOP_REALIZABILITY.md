# AUDIT — Central-Gauss hop realizability

**Identifier:** `FTD-0471`  
**Date run:** 2026-07-25  
**Status:** `[THEOREM — EVEN-L IMAGE OBSTRUCTION]` +
`[THEOREM — ODD-L SUPPORT LOWER BOUND]` +
`[MEASURED — MATCHED FACE LOCAL CLOSURE]` +
`[CLOSED NEGATIVE — CELL-CENTERED J AS SOLE LOCAL SOURCE-TRANSPORT FIELD]` +
`[OPEN — MATCHED-FACE ENERGY/MOMENTUM TRANSACTION]`  
**Pre-registration:** [`PREREG_CENTRAL_GAUSS_HOP_REALIZABILITY_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_CENTRAL_GAUSS_HOP_REALIZABILITY_v1.md)  
**Run of record:** `engine/results/ftd_0471/windows_msvc_cpu.csv`

## Verdict

`CENTRAL_GAUSS_HOP_EVEN_IMPOSSIBLE_ODD_NONLOCAL_FACE_LOCAL`

The production cell-centered field with central divergence cannot be the sole
local carrier of a one-site Gauss-source move. On even periodic lattices the
required source delta is outside the divergence image. On odd lattices an
exact solution exists only through a path whose support grows with the box.
The matched oriented-face complex transports the same source delta exactly on
one face for every tested parity and volume.

## Step-two graph

For cell-centered `J`,

```text
(D_i J_i)(x) = [J_i(x+e_i)-J_i(x-e_i)]/2.
```

A value `J_i(m)` contributes opposite source at `m+e_i` and `m-e_i`.
Therefore it is an edge on a graph whose vertices differ by two lattice
sites, not by one.

### Even periodic size

For even `L`, the checkerboard character

```text
chi_i(x)=(-1)^(x_i)
```

obeys `D_i chi_i=0`. Periodic summation by parts then gives

```text
sum_x chi_i(x) div_central(J)(x)=0
```

for every cell-centered field. An adjacent hop `a -> a+e_i` has source delta
`-q delta_a+q delta_(a+e_i)` and checkerboard pairing magnitude `2`. Hence it
is not in the central-divergence image. This is an exact left-nullspace
certificate, not a failed iterative solve.

All 24 registered `L=16,32` direction/polarity rows returned
`realizable=false`; deterministic nonzero-field null-pairing residuals were at
most `3.64e-16`.

### Odd periodic size

For odd `L`, step two generates the axial cycle. The shortest path from `a`
to `a+e_i` requires `n=(L-1)/2` edges because

```text
-2n = 1 (mod L).
```

Components in the other two axes cannot shorten this axial congruence. The
registered construction assigns the corresponding signed value `2q` along
that path and saturates the lower bound:

| `L` | exact support |
|---:|---:|
| 17 | 8 |
| 33 | 16 |
| 65 | 32 |

All 36 odd-L rows had exactly zero Gauss residual. The solution is exact but
box-scale; it is not a local event map.

## Matched face comparison

On the backward-difference face complex, a hop history satisfies

```text
Delta rho + div_face(current)=0.
```

Applying `Delta E=-current` gives `div_face(Delta E)=Delta rho`. All 60
registered rows across `L={16,17,32,33,65}` closed continuity and Gauss
exactly, with one nonzero current face and one changed electric face.

This is the representation-level reason the FTD-0427 matched complex succeeds:
its field lives on the same oriented faces as the transport current. The result
is independent of the coupling magnitude; multiplying source and field update
by `G_C` only rescales the identity.

## Consequences

1. **Odd-volume success is misleading if used alone.** The frequent `L=33`
   campaign choice avoids the even checkerboard obstruction, but its exact
   central-field transport still spans 16 sites and grows with `L`.
2. **Gauss projection is not event transport.** A global projection may repair
   a post-hop diagnostic target, but it cannot turn the central operator into
   a one-face local continuity update. On even `L`, the exact adjacent source
   delta is outside the central image before any solver tolerance is discussed.
3. **Cell-centered `J` remains useful for smooth propagation.** The theorem
   closes only the claim that it is also the sole exact local carrier of
   integer source motion.
4. **A staggered link/face layer is structurally required** if exact local
   Gauss transport is retained as a demand. This may be a separate event
   transport layer coupled to the smooth field; identifying the two without a
   map is not justified.

## Next gate

The one-face update proves local continuity but not mechanics. The next test
must combine matched face transport with FTD-0470's exact link work and ask
whether the field energy and momentum changes pay the particle update. In
particular, a purely electric face update has no demonstrated magnetic/Poynting
recoil partner. The required record is one face hop with:

```text
Delta E_particle + Delta H_interaction + Delta E_face = 0,
Delta p_particle + Delta P_face = 0,
```

plus exact inverse and one-face causal support. Failure would show that the
staggered representation fixes continuity but not the complete transaction.

## Reproducibility

- campaign SHA-256:
  `D6B917E3110AA9C5295BEC30261D465D364CD6258375E443345D38D5A40D88CC`
- helper SHA-256:
  `971C601EE7355670F519D0499FDEB37D1D980FE76F7D2C0F920E3E2C7A071DE2`
- toolchain: MSVC `14.44.35207`, Release, CPU observer
- production dynamics: unchanged
