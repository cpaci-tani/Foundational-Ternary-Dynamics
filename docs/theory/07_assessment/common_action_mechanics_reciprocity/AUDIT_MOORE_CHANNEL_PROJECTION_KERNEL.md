# AUDIT — Moore-channel projection kernel

**Date:** 2026-07-24  
**Identifier:** `FTD-0446`  
**Status:** `[THEOREM — REGISTERED 13-CHANNEL FIRST-MOMENT MAP HAS NULLITY 10]`  
**Verdict:** `VECTOR_PROJECTION_HAS_TEN_HIDDEN_CHANNEL_MODES`  
**Pre-registration:** [`PREREG_MOORE_CHANNEL_PROJECTION_KERNEL_v1.md`](../../10_eft_program/preregistrations/common_action_mechanics_reciprocity/PREREG_MOORE_CHANNEL_PROJECTION_KERNEL_v1.md)  
**Run of record:** `engine/results/ftd_0446/windows_msvc_cpu.csv`

## 1. Exact result

Pairing each directed Moore displacement with its reverse gives 13 signed
unoriented channels:

| Shell | Directed neighbors | Signed unoriented channels |
|---|---:|---:|
| face / SC | 6 | 3 |
| edge / FCC direction | 12 | 6 |
| corner / BCC direction | 8 | 4 |
| total | 26 | 13 |

For channel amplitudes `a_i` and representative displacement vectors `d_i`,
define the first moment

$$
P(a)=\sum_{i=1}^{13}a_i d_i\in\mathbb{R}^3.
$$

The first three columns are the Cartesian basis, so `rank(P)>=3`; the codomain
has dimension three, so `rank(P)=3`. Rank-nullity gives

$$
\boxed{\dim\ker P=13-3=10.}
$$

For each of the ten non-face channels, the registered integer vector

$$
k_i=e_i-d_{i,x}e_x-d_{i,y}e_y-d_{i,z}e_z
$$

projects exactly to zero. The vectors are independent because each has a
unique unit pivot in one of channels 4 through 13. The exact campaign reports
zero projection and pivot failures.

## 2. What `J` cannot distinguish

A unit of direct diagonal-channel traffic and its Cartesian face-channel
decomposition have exactly the same three-vector first moment for all ten
non-face directions. They do not have the same channel norm:

- edge: direct norm `1`, two-face norm `2`;
- corner: direct norm `1`, three-face norm `3`.

Thus equal `J` can hide channel-energy differences of `1` or `2`. A local
three-vector energy such as `|J|^2` cannot reconstruct a quadratic energy on
the 13-channel lift without another selection.

The 13-channel direction set closes up to orientation sign under all 48 signed
coordinate permutations: zero failures across 624 exact tests. Therefore a
primitive channel representation can retain cubic covariance; the loss occurs
when it is projected many-to-one into three components.

## 3. Ontological reading

This theorem establishes an information boundary, not a new ontology.

If the native movement links themselves carry current, the current
three-component `J` is naturally interpreted as their net directional first
moment. In that reading, flux streamlines display the resultant direction of
microscopic traffic, not enough information to identify which face, edge, or
corner channels were used.

The analogy is net wind versus traffic lanes: counterflow and alternative lane
patterns can share the same resultant vector while differing internally.

If instead `J` is fundamental and complete, then primitive 13-channel traffic
is not part of the ontology and edge/corner motion requires a selected rule for
how three-vector flux receives its local recoil. FTD-0445 shows that continuity
does not provide that rule.

## 4. Research consequence

The next decision is structural:

1. **Three-vector fundamental:** keep `J in R^3` and introduce a covariant,
   reversible rule mapping each Moore hop directly into `J/W` updates.
2. **Link-channel fundamental:** add 13 signed link amplitudes (or 26 directed
   amplitudes with reversal constraints), then derive `J` as their coarse first
   moment.
3. **Restricted movement:** make only face links mechanical primitives and
   reinterpret edge/corner destinations as composed subevents, changing the
   current one-tick Moore movement semantics.

None is selected by the frozen postulates. Options 2 and 3 alter the ontology
or production semantics and therefore require a separate registered cycle.

The ten kernel directions are not particles, polarizations, gauge generators,
or physical modes until dynamics, observability, energy, and stability are
defined and measured. The geometric count `13` is not identified with
`N_eff`.

## 5. Reproducibility

- campaign SHA256: `2cf327a30c2ad8ca087aee58c28b41f65be6b022092253584f3a8ee8b704b717`
- helper SHA256: `55c7da0d7ebd1ab814f32093947317e19318211acd89252e40cd115f79ca311a`
- record SHA256: `d82e64da5543b0fc7b86e583e1f06064bcc21b82af6b680bb1020622c4382a2e`
- compiler: pinned MSVC `14.44.35207`, Release
- execution: exact-integer algebraic observer, no production tick
- result: `VECTOR_PROJECTION_HAS_TEN_HIDDEN_CHANNEL_MODES`

No production dynamics were changed.
