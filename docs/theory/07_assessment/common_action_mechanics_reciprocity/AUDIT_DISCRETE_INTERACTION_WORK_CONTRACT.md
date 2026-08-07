# AUDIT — Discrete interaction-work contract

**Date:** 2026-07-24  
**Identifier:** `FTD-0443`  
**Status:** `[THEOREM — FINITE-SITE HOP WORK]` + `[MEASURED — PRODUCTION CROSS-CONTRACT]` + `[CLOSED NEGATIVE — LEGACY VARIATIONAL CLAIM]`  
**Verdict:** `EXACT_HOP_WORK_PRODUCTION_MISMATCH`  
**Pre-registration:** [`PREREG_DISCRETE_INTERACTION_WORK_CONTRACT_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_DISCRETE_INTERACTION_WORK_CONTRACT_v1.md)  
**Run of record:** `engine/results/ftd_0443/windows_msvc_cpu_L17.csv`

## 1. Exact result

For the declared finite interaction

$$
L_{int}=G_C\sum_xs_x\phi_x,
\qquad \phi_x=(\nabla\cdot J)_x,
$$

moving charge `q` from site `a` to site `b` at fixed field changes the action by

$$
\boxed{\Delta L_{int}=G_Cq(\phi_b-\phi_a)}.
$$

This follows by direct cancellation of every unchanged site. It is exact on a
finite lattice and requires no continuum limit.

The registered implementation recomputed the complete interaction sum for both
signs and all 26 Moore hops. All 52 cases pass; the worst residual is
`1.73472e-18`. Reverse-hop residual is zero, four-link closed-loop work is
`2.16840e-19`, and the symmetric central-difference bridge agrees exactly.

## 2. Production mismatch

The symmetric positional candidate implied by the declared action is

$$
F_{action}=+G_Cq\,\nabla_c(\nabla\cdot J).
$$

On the locked three-component field fixture, production returns

$$
F_{production}=-\alpha q\,\nabla_c(\nabla\cdot J).
$$

The measured vectors have cosine `-1.0000000000000002`. Their magnitude ratio is

$$
\frac{|F_{production}|}{|F_{action}|}
=0.08542454310285438
=\frac{\alpha}{G_C}
=G_C
$$

to `1.39e-17`. Production matches its copied negative formula exactly and does
not match the action-derived candidate (`1.08542` relative vector error).

This closes the legacy variational claim negative. Its sign is reversed and it
contains one extra power of `G_C`. Since the sourced field response already
carries one source vertex, the declared probe variation supplies the other
`G_C`; replacing that probe vertex by `alpha=G_C^2` makes the effective response
third order in `G_C`.

## 3. Sub-voxel obstruction

Changing only the particle remainder from zero to `(0.31,-0.27,0.19)` changes
the implemented coupling action by exactly zero. The action reads the ternary
site state but not the continuous remainder. Therefore it supplies:

- exact work for a discrete site transition;
- no continuous force law between site transitions.

The production engine nevertheless accelerates the remainder continuously each
tick. That acceleration is not the variation of the frozen site-valued
interaction.

There are only two honest continuations:

1. **Event-native route:** mechanics is expressed as exact link/hop work and
   impulses; no continuous force is claimed between hops.
2. **Interpolated-source route:** introduce a normalized source-shape
   `w_x(R)` and derive
   `F=G_C q sum_x grad_R(w_x(R)) phi_x`. The interpolation is new selected
   structure unless forced by a deeper ontology.

## 4. Correct scope of the action claim

The current action correctly supplies the field source and exact static/hop
interaction differences. It does not generate the production continuous
particle-force phase. The statement “the tick cycle is the Euler–Lagrange
equations of this action” must be restricted to the validated field sector and
must exclude particle movement until one of the two routes above is completed.

FTD-0439's net-pair balance for the legacy branch remains a fact about the coded
rule, not evidence for its sign, normalization, physical strength, or action
provenance.

## 5. Reproducibility

- campaign SHA256: `9417064ff0ebe2de2a98e1125350bae22ab04621f1d7c3fe095f1e52a620e827`
- helper SHA256: `4ce99516be120486af2bd28cff98dfd6f5e24edb4f27b16d56c1c99b6dd143a1`
- record SHA256: `aa7877f6b8bb157d8668336b6258367b3baf05d1c4aac01323322b2fce2128f8`
- compiler: pinned MSVC `14.44.35207`, Release
- backend: forced CPU, periodic `L=17`
- result: `EXACT_HOP_WORK_PRODUCTION_MISMATCH`

No production dynamics were changed.
