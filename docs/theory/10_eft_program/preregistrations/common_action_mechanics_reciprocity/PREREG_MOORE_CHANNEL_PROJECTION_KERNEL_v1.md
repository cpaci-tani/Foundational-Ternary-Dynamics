# PRE-REGISTRATION — Moore-channel projection kernel v1

**Date locked:** 2026-07-24  
**Identifier:** `FTD-0446`  
**Status:** `[PRE-REGISTRATION — LOCKED/RUN]`  
**Parent:** `FTD-0445` Moore-hop route ambiguity  
**Engine artifact:** `engine/tests/campaign_moore_channel_projection_kernel.cpp`  
**Campaign SHA256:** `2cf327a30c2ad8ca087aee58c28b41f65be6b022092253584f3a8ee8b704b717`  
**Helper SHA256:** `55c7da0d7ebd1ab814f32093947317e19318211acd89252e40cd115f79ca311a`

## 1. Question

The 26 directed Moore displacements define 13 unoriented channels with signed
amplitudes: 3 face, 6 edge, and 4 corner channels. FTD stores a site vector
`J in R^3`. FTD-0446 asks:

> Is the natural first-moment map from 13 link channels to `J` injective, and
> can `J` distinguish a primitive diagonal link from face-channel transport?

## 2. Frozen map

Choose one representative `d_a` from each opposite Moore pair and define

$$
P(a)=\sum_{a=1}^{13}a_a d_a\in\mathbb{R}^3.
$$

The first three channels are the Cartesian basis. For each remaining diagonal
channel `d_a`, register the integer kernel vector

$$
k_a=e_a-d_{a,x}e_x-d_{a,y}e_y-d_{a,z}e_z.
$$

## 3. Exact gates

- rank `P = 3`, hence nullity `10`;
- all ten registered `k_a` project exactly to zero;
- the ten vectors are independent by their unique unit pivots in channels
  4 through 13;
- one direct diagonal-channel unit and its Cartesian face decomposition have
  identical `J` for all ten non-face channels;
- their channel quadratic energies differ by `1` for edges and `2` for
  corners;
- the 13 unoriented directions close up to orientation sign under all 48
  signed coordinate permutations, with zero failures across 624 tests.

All checks use exact integer arithmetic.

## 4. Locked outcomes

- `VECTOR_PROJECTION_HAS_TEN_HIDDEN_CHANNEL_MODES`: every exact gate passes.
- `PROTOCOL_INVALID`: any gate fails.

## 5. Interpretation boundary

The kernel dimension is a theorem about the registered projection, not evidence
that 13 link fields physically exist. If link channels are adopted, `J` is a
coarse first moment and cannot reconstruct their microscopic traffic or
quadratic energy. Adopting those channels would expand the ontology and is not
part of the frozen production cycle.

The number 13 here is the unoriented count of the Moore shell. This campaign
makes no identification with `N_eff` or any measured constant.

## 6. Banned moves

- No direction basis, projection, kernel basis, energy diagnostic, or symmetry
  group may change after first execution.
- No hidden channel may be called a particle, gauge boson, polarization, or
  physical degree of freedom without a separate dynamical result.
- No production tick changes.
