# PRE-REGISTRATION — Matched-face momentum transaction v1

**Date locked:** 2026-07-25  
**Identifier:** `FTD-0473`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN; ONTOLOGY-SCOPE CORRECTION]`  
**Parents:** `FTD-0438`, `FTD-0468`, `FTD-0472`  
**Engine artifact:** `engine/tests/campaign_matched_face_momentum_transaction.cpp`  
**Campaign SHA256:** `D8128170B305D8BFFD5923040C5EE12DE69D4EF81ADB648DBD7F7EC95BD37723`  
**Helper SHA256:** `BA7B0CA7895D4DC5259527CCDCB06EC9B08DF7C4CB38AC8CDEDC31EFCD3FA62B`

## 1. Question

FTD-0472 supplied exact energy work for the selected matched current but left
momentum open. This campaign asks whether the simplest local translation
pseudomomentum of the staggered field is exactly conserved source-free, how it
changes under the event current, and whether an electrostatic hop that
exchanges nonzero energy also produces field recoil in that channel.

No production force or momentum is added. The equal-and-opposite matter
impulse is reported only as the amount a future matter law would have to own.

## 2. Translation-symmetry boundary — periodic protocol only

**Post-run scope correction:** this section describes the registered periodic
computational windows, not the FTD ontology. The ontology is uncontained and
undefined-boundary; finite realized structures are finite-extent/support
configurations, not contents of a finite box. It has no ontological `T^L=1`
relation. Therefore the `Hom(Z_L,R)=0` statement below cannot be exported as a
no-go for an ontological real momentum. The campaign measurements and local
periodic invariant remain valid; the stronger boundary inference is withdrawn.

The exact periodic protocol translation group is finite: `Z_L^3`. It canonically gives
crystal momentum in the dual group, also `Z_L^3`, with addition modulo `L`.
It does not uniquely give a real additive momentum:

```text
Hom(Z_L,R)=0,
```

because `L p=0` in `R` implies `p=0`. Equivalently, every eigenvalue of the
one-site translation operator has logarithms

```text
k_n = 2 pi n/L + 2 pi m_n,  m_n in Z.
```

A real Brillouin-zone representative therefore requires a branch selection.
This campaign does not claim that its local quadratic candidate is unique.
For an uncontained local translation algebra modeled by `Z^3`, nontrivial real
homomorphisms exist, with normalization still requiring physical definition.

## 3. Pre-derived local invariant

Let `D_i` be periodic central translation on the face arrays and `C` the
matched curl. For the source-free staggered map

```text
B_1 = B_0-lambda C^T E_0,
E_* = E_0+lambda C B_1,
```

freeze the minimal local candidate

```text
P_i(E,B)=<E,D_i C B>.
```

`D_i` is skew-adjoint and commutes with `C`, `C^T`, and their translation-
invariant products. Therefore

```text
P_i(E_*,B_1)-P_i(E_0,B_0)
 = -lambda<E_0,D_i C C^T E_0>
   +lambda<C B_1,D_i C B_1>
 = 0.
```

For the event update `E_1=E_*-K`,

```text
Delta P_i^field=-<K,D_i C B_1>.
```

The formula is exact for this candidate and fixes its current-exchange ledger
without a fit. Multiplying the whole candidate by an arbitrary constant would
remain conserved, which is another reason the result is a selected
pseudomomentum rather than a unique physical normalization.

## 4. Frozen fixtures

1. **Source-free controls:** `L={16,17}`, all three propagation axes, directed
   transverse modes `n=2`, amplitude `0.02`, 256 staggered steps.
2. **Current transactions:** the complete FTD-0472 matrix: both volumes and
   charges, all 26 Moore displacements, all 1/2/6 distinct face orderings,
   and electrostatic/transverse arms. Total 624 rows in 208 route groups.
3. Electrostatic arm: minimum-energy neutral dipole and zero initial magnetic
   field.
4. Transverse arm: the same divergence constraint plus electric challenge
   amplitude `0.037` and magnetic challenge amplitude `0.019`.
5. `wave_speed=C_SPEED`, `dt=1`; no production tick modification.

## 5. Gates

- all source-free controls begin with pseudomomentum magnitude `>1e-8`;
- source-free absolute and relative drift each `<=1e-10`;
- source-free portion of every transaction and the exact current-change
  formula each close `<=1e-10`;
- route counts remain exactly `1!`, `2!`, `3!`;
- electrostatic field recoil is at most `1e-8` while every registered
  electrostatic energy-work magnitude exceeds `0.1`;
- at least one transverse multi-route group has required-impulse span
  `>1e-6`.

## 6. Outcome map

- all gates pass:
  `LOCAL_PSEUDOMOMENTUM_EXACT_ELECTROSTATIC_HOP_RECOIL_ABSENT`;
- any algebraic/control/route gate fails with valid fixtures:
  `MATCHED_FACE_MOMENTUM_TRANSACTION_CLAIM_FAILS`;
- invalid initialization or nonfinite record:
  `PROTOCOL_INVALID`.

The positive named verdict is a negative mechanics result: the selected local
pseudomomentum exists and its exchange is exact, but the electrostatic
energy-closing hop does not recoil in it. This closes neither nonlocal crystal
momentum nor every Poynting-like candidate. It establishes that FTD-0472 alone
is not a complete autonomous matter-field hop.

## 7. Run of record

Pinned MSVC `14.44.35207`, Release, CPU observer, focused target
`campaign_matched_face_momentum_transaction`, stdout captured as
`engine/results/ftd_0473/windows_msvc_cpu.csv`.

**Recorded outcome:**
`LOCAL_PSEUDOMOMENTUM_EXACT_ELECTROSTATIC_HOP_RECOIL_ABSENT`. Six directed
wave controls conserved the candidate to `5.20e-14`; all 624 current routes
closed the exact exchange formula. Electrostatic work stayed above `0.333`
while recoil stayed below `3.70e-17`. The maximum transverse route-impulse
span was `0.001023`. See `AUDIT_MATCHED_FACE_MOMENTUM_TRANSACTION.md`.
