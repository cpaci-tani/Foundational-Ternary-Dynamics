# AUDIT — Cubic hop work response

**Date:** 2026-07-24  
**Identifier:** `FTD-0447`  
**Status:** `[THEOREM — ISOLATED CUBIC-COVARIANT POLAR WORK RESPONSE]`  
**Verdict:** `CUBIC_STABILIZER_FIXES_LONGITUDINAL_WORK_RESPONSE`  
**Pre-registration:** [`PREREG_CUBIC_HOP_WORK_RESPONSE_v1.md`](../10_eft_program/preregistrations/PREREG_CUBIC_HOP_WORK_RESPONSE_v1.md)  
**Run of record:** `engine/results/ftd_0447/windows_msvc_cpu.csv`

## 1. Theorem

Let `d` be any nonzero Moore displacement and let `H_d` be its stabilizer in
the 48-element full cubic group of signed coordinate permutations. Suppose an
isolated-hop response `F(d,W)`:

- is an ordinary polar three-vector;
- depends directionally only on `d`;
- is covariant under the full cubic group;
- satisfies the scalar work condition `F dot d = W`.

Covariance requires `F` to be fixed by every element of `H_d`. The locked
exact-integer campaign constructs all `(g-I)F=0` constraint rows. For every one
of the 26 Moore directions, the constraint matrix has rank `2`. Its fixed
subspace therefore has dimension `1`; `d` lies in that subspace, so

$$
\operatorname{Fix}(H_d)=\operatorname{span}(d).
$$

Consequently `F=lambda d`, and the work condition uniquely gives

$$
\boxed{F(d,W)=\frac{W}{|d|^2}d.}
$$

The exact stabilizer sizes are `8`, `4`, and `6` on face, edge, and corner
orbits. With registered integer work `W=6`, there are zero work failures and
zero covariance failures across all `26*48=1248` transformed cases.

## 2. What changed relative to FTD-0444

FTD-0444 remains correct: scalar work by itself admits arbitrary transverse
components. FTD-0447 supplies an additional native criterion. For an isolated
hop with no background direction, full cubic stabilizer invariance forbids all
those transverse components.

Thus the longitudinal force representative is now theorem-grade within the
explicit FTD-0447 assumption package. It is not theorem-grade when a
background field, spin/axial datum, neighboring configuration, memory, or
other tensor participates, because those inputs reduce the stabilizer and can
support transverse covariants.

## 3. What remains open

The theorem fixes a force/work vector, not a complete event map. It does not
determine:

- how finite work changes an on-shell momentum when transverse momentum is
  already present;
- which energy branch is selected at a turning point;
- where compensating field momentum is stored;
- whether recoil uses three-vector `J/W`, 13 Moore-link channels, or another
  local state;
- how the complete particle-field state reverses.

In particular, FTD-0445's face-route ambiguity and FTD-0446's ten-dimensional
projection kernel are untouched.

## 4. Ontological consequence

The discrete geometry supplies more mechanics than scalar bookkeeping alone:
for an otherwise featureless local hop, the hop direction is the only allowed
polar response axis. This makes longitudinal event mechanics the natural
native baseline without borrowing continuum rotational symmetry.

The unresolved problem has narrowed from “which force direction?” to “what
local field state receives the equal-and-opposite exchange, and how is the
finite momentum update made reversible?”

## 5. Reproducibility

- campaign SHA256: `d0f0331ebe6231ba4bbb68a4dc3b430441cd1996be7174e9b6e4850f46906b28`
- helper SHA256: `25b149f168872f18373e8a77fd771367151fbe7df6940ca9a72d74aa97cad5a1`
- record SHA256: `662d7e08280b9bf93a30917a8eb26b80b06987d85755011cc041ef20069f9c8b`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: exact-integer algebraic observer, no production tick
- result: `CUBIC_STABILIZER_FIXES_LONGITUDINAL_WORK_RESPONSE`

No production dynamics were changed.
