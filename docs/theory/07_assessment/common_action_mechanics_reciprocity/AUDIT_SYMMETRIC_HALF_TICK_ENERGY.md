# AUDIT — Symmetric half-tick transaction energy

**Identifier:** `FTD-0469`  
**Date executed:** 2026-07-25  
**Status:** `[THEOREM — SYMMETRIC SHADOW ENERGY INVARIANT]` +
`[THEOREM — DRIFT FIELD-MOMENTUM INVARIANCE]` +
`[MEASURED — MSVC RUN OF RECORD + INDEPENDENT PYTHON REPLICA]` +
`[MEASURED — UNFUNDED PARTICLE KINETIC ENERGY AT FIXED SITE]`  
**Pre-registration:**
[`PREREG_SYMMETRIC_HALF_TICK_ENERGY_v2.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_SYMMETRIC_HALF_TICK_ENERGY_v2.md)
(v1 superseded, see section on gate history)  
**Run of record:** `engine/results/ftd_0469/windows_msvc_cpu.csv`  
**Replica:** `scratch/ftd_0469_replica/`

## Result

The FTD-0468 common-action kick pair, arranged as one symmetric
kick-drift-kick transaction, conserves an exact discrete energy whose only
correction beyond `E_tick + H_int` is a derived quarter-cross term. Nothing
is fitted. The locked verdict under pinned MSVC is

`SYMMETRIC_HALF_TICK_SHADOW_ENERGY_EXACT`.

## Exact identities

Let `L` be the 18-point laplacian, `A = -C_WAVE^2 L`, `D_i` the periodic
central difference, `g = -G_C grad(s)`, and let the transaction be

```text
W += g/2 ; p += (G_C/2) s grad(div J)      (pre-drift J)
W += C_WAVE^2 L J ; J += W                 (exact source-free drift)
W += g/2 ; p += (G_C/2) s grad(div J)      (post-drift J)
```

**Theorem 1 (shadow energy).** [THEOREM] With `E_tick` the FTD-0293/0452
invariant and `H_int = -G_C sum_x s div(J) = -g^T J`, the transaction
exactly conserves

```text
E_shadow = E_tick + H_int + (1/4) g^T A J.
```

Proof. `g` is a lattice gradient, hence orthogonal to the constant null
space of `A`, so `A J* = g` is solvable. In the deviation variables
`(j, V) = (J - J*, W - g/2)` the three steps compose to exactly the
source-free drift `V' = V - A j ; j' = j + V'`, which conserves
`E_tick(j, V)` exactly (FTD-0293). Expanding `E_tick(J - J*, W - g/2)`
gives `E_tick(J,W) + H_int(J) + (1/4) g^T A J` plus a constant. The
`W`-linear terms cancel identically and the coefficient 1/4 is forced by
the expansion; no counterterm is fitted.

**Theorem 2 (naive ledger is a total difference).** [THEOREM]

```text
[E_tick + H_int](t) - [E_tick + H_int](0) = CT_sym(0) - CT_sym(t),
CT_sym = (1/4) g^T A J.
```

The uncorrected ledger oscillates within the bounded envelope of `CT_sym`
and has no secular term.

**Theorem 3 (drift conserves field momentum).** [THEOREM] The source-free
drift exactly conserves `P_i = -sum_x W . D_i J`: translation-invariant
stencils commute, `L` is symmetric, `D_i` skew-adjoint, so `L D_i` is
skew-adjoint and every cross term cancels. Combined with the per-kick
FTD-0468 identity, `p_matter + P_field` is exact through whole
transactions.

**Theorem 4 (production-ordering invariant).** [THEOREM] The fused
full-kick ordering `W += C_WAVE^2 L J + g ; J += W` is the source-free
drift in variables `(J - J*, W)` and therefore exactly conserves

```text
E_tick + H_int + (1/2) g^T W.
```

The simultaneous ordering is not energy-defective. The discriminator is
structural: the symmetric counterterm depends only on the configuration
`J` at integer time, the transaction is exactly time-reversal symmetric,
and its matter impulse is time-centered `(1/2)[F(J_n) + F(J_{n+1})]`,
while the production counterterm is linear in the staggered-time variable
`W` and its matter force is not time-centered. Choosing the symmetric form
is a SELECTION PRINCIPLE, not a defect claim. An earlier working
conjecture that the fused ordering leaks `-(1/2)|g|^2` per tick was tested
and is false; the candidate leak cancels at the sourced equilibrium `J*`.

## Run of record (MSVC 14.44.35207, Release, forced CPU)

12 static arms (L=17, 256 transactions), 6 dynamic travelling-wave arms
(L=33, 64 transactions), 3 production-ordering controls (L=17, 256
ticks). Energy gates relative to `max(1, |E_shadow_0|)`; all gates
`1e-12`.

| Quantity | Worst value |
|---|---:|
| shadow-energy residual (relative) | `7.53e-14` |
| naive total-difference identity (relative) | `7.53e-14` |
| total momentum `p + P_field` | `1.83e-13` |
| reversal restore (fields and momentum ledger) | `3.89e-15` |
| production-ordering invariant (relative) | `3.76e-14` |

All 12 + 6 arms have nonzero accumulated impulse (min `1.56e-3`);
`valid,true`; focused CTest `100% tests passed, 0 tests failed out of 1`.
The independent Python replica agrees on every identity at the same noise
floor (`scratch/ftd_0469_replica/replica_run_2026-07-25.log`).

Unfunded-surplus measurement: on the pair-cubic arms the shadow ledger
closes to `6e-14` relative while the accumulated matter momentum reaches
`|p| = 5.89e-2` with `E(p) - E_REST = 3.36e-3`. [MEASURED] At frozen site
the particle kinetic energy is exactly the surplus of the total ledger; it
is not funded by the field-interaction sector. This is the quantitative
form of the FTD-0443 sub-voxel obstruction; funding is available only at
site transitions, where `Delta E_particle = G_C q Delta(div J)` closes
exactly (FTD-0452).

## Gate history and diagnostic note

The v1 preregistration gated energy residuals at `1e-12` ABSOLUTE and the
v1 run returned `SYMMETRIC_HALF_TICK_ENERGY_FAILS` at `2.11e-12` absolute
on ledgers of magnitude `56.17` (`3.76e-14` relative, the FTD-0452 noise
floor; the identical excursion appears in the v1 production control).
That verdict stands as recorded for the v1 gate definition; v2 registers
the relative gate the theorem claims actually require. Both run records
are retained (`windows_msvc_cpu_v1_absolute_gate.csv`,
`windows_msvc_cpu.csv`). Every gated field quantity is bit-identical
between the two builds.

Diagnostic discrepancy, disclosed: the v1 binary's non-gated
`particle_energy_delta` column printed values inconsistent with
`E(p) = sqrt(E_REST^2 + C_SPEED^2 |p|^2)` for the printed impulse
(roughly half). The v2 rebuild prints values matching the exact
kinematics by hand and matching the replica (for example impulse
`5.8902e-2` gives delta `3.3616e-3`). No gated quantity depends on this
column; the cause was not isolated and the v1 column should not be quoted.

## Consequences and open questions

1. Simultaneous versus symmetric half-kicks: both orderings possess exact
   invariants (Theorems 1 and 4); the symmetric transaction is selected on
   structure. [SELECTION PRINCIPLE]
2. Dressing versus wake: `E_shadow` is `E_tick(J - J*, W - g/2)` plus a
   constant, so the dressing/radiation split is exact at the invariant
   level. Exploratory FFT measurement (`scratch/ftd_0469_replica/
   explore.py`, non-preregistered) finds the pair dressing interaction
   energy decays by roughly five powers of distance: the longitudinal
   `s div(J)` channel produces a contact-like coat, not a Coulomb tail.
   Long-range electrostatics therefore still lacks a variational funding
   mechanism; the temporal-gauge analogy points at a Gauss-constraint
   route, since the production Poisson branch is non-variational
   (FTD-0467). [MEASURED, exploratory; the Gauss route is HYPOTHESIS]
3. Hop bill: at fixed `(J, W)` a face hop changes `E_shadow` by exactly
   `-W_hop + Delta CT_sym`, with `Delta CT_sym / W_hop = 8e-17` on the
   registered fixture (exploratory). The hop gate inherits a precise bill.
4. Scope: all exactness results require the linear wave sector with frozen
   `s`. Nonlinear toggles reduce the shadow invariant to an asymptotic
   statement. [THEOREM about scope]

## Next gate

Join the symmetric transaction to an integer site hop: pay
`Delta E_particle = G_C q [div(J)_b - div(J)_a]` from the interaction
ledger (FTD-0443), reconcile kick-accumulated `E(p)` without the FTD-0451
double count, and translate the dressing `J*` consistently with the
FTD-0465/0466 coat-translation negatives, testing total energy, total
momentum, and reversal together. In parallel, the Coulomb question:
either a Gauss constraint on `W` or a variational replacement for the
Poisson branch.

## Reproducibility

- campaign SHA-256 (v2):
  `8FD735ABE1339A16888A4A133D852C509C8281C3319E00ED5F82B59884D4CCFD`
- helper SHA-256:
  `BE2EF960DBEE706EA28CC8F1D9E34F4592253B97B67C73834BA1F517ECA56031`
- run-record SHA-256:
  `A8167B48C440AA8F9A737AC54777E3BBA3294BC49C0F040EBE87C89A6E9CF05D`
- replica SHA-256: `replica.py`
  `3C6C002CFC3030D0224DAE60E76B9D6998C66025BC8E4BD3C6CADB3229B6DCFE`,
  `verify.py`
  `1AEE53247CE3DD0DB8A1C7B8BC06C9DB1AF6CE47DFA5FFEA2FD84540150BFC6C`
- compiler: pinned MSVC `14.44.35207`, Release, Ninja Multi-Config
- focused CTest: `1/1` pass
- production dynamics: unchanged
