# V3 triplet phase winding and one-integer coupling ladder v1

**Date:** 2026-08-24  
**Status:** **[THEOREM — EXACT MATERIAL C4 WINDING/CADENCE]** +
**[THEOREM, CONDITIONAL — ONE-INTEGER MOMENTUM-NEUTRAL COUPLING LADDER]** +
**[THEOREM, CONDITIONAL — RECOIL-REDUCED LADDER]** +
**[SCOPED NO-GO — ZERO-DEBIT SEED ASSEMBLY CANNOT NORMALIZE COUPLING]** +
**[OPEN — PACKET DEBIT, RECOIL, COMMON POLE/RESIDUE, AND CANONICAL PHI]**  
**Production status:** unchanged  
**Ledger status:** no row minted  
**Finite-clock parent:**
[`THEOREM_V3_FINITE_CLOCK_PACKET_COUPLING_LADDER_v1.md`](THEOREM_V3_FINITE_CLOCK_PACKET_COUPLING_LADDER_v1.md)  
**Material-clock parent:**
[`THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_CUBIC_TRIPLET_SELF_CORRECTING_MATERIAL_CLOCK_AND_STABILITY_BOUNDARY_v1.md)  
**Formation successor:**
[`THEOREM_V3_GENESIS_SEEDED_TRIPLET_ASSEMBLY_PHI_v12_CANDIDATE_AND_FORMATION_BOUNDARY_v1.md`](../constituent_complete_matter/THEOREM_V3_GENESIS_SEEDED_TRIPLET_ASSEMBLY_PHI_v12_CANDIDATE_AND_FORMATION_BOUNDARY_v1.md)  
**Certificate:**
[`proof_v3_triplet_phase_winding_one_integer_coupling_ladder.py`](../../../../../scripts/proofs/proof_v3_triplet_phase_winding_one_integer_coupling_ladder.py)

---

## 1. Previously open integer data

The finite-clock packet theorem proves, conditional on one reciprocal
packet/clock vertex, that a material phase making `w` windings in `T` global
ticks and absorbing `d` complete packets per action quantum has

\[
 \alpha_{\rm native}={3w\over dT}                     \tag{1}
\]

when recoil is zero. That theorem deliberately left all three integers
`(w,T,d)` unselected.

The triplet material clock now fixes the first two. This is not a substitution
of a convenient clock period into equation (1). The certificate reads the
actual C4 phase from every state of the exact finite A9 orbit and counts its
unwrapped winding under the same tick map used by the triplet.

---

## 2. Exact A9 phase winding

One occupied A9 state contains exactly one nonblank ternary-square token,
whether that token is link-owned or reserve-owned. Its native phase index is
the registered C4 coordinate

\[
 \varphi\in\mathbb Z_4.                                \tag{2}
\]

For every one of the sixteen valid owned A9 states, one admitted A9 tick gives

\[
 \varphi_{n+1}-\varphi_n=1\pmod4.                      \tag{3}
\]

The complete A9 state has period eight because four ticks return the phase
but exchange link/reserve ownership; eight ticks return phase and ownership.
The unwrapped phase advance over that complete orbit is eight quarter-turns:

\[
 \Delta\widetilde\varphi=8,
 \qquad w_{A9}={8\over4}=2.                            \tag{4}
\]

Charge conjugation changes the polarity shell but preserves the C4 phase
coordinate, so equation (4) holds for both polarity cycles and for every
initial phase.

---

## 3. Global-tick cadence of the triplet

The self-correcting triplet uses a two-layer READ/COMMIT schedule:

```text
global tick 2n:     copy the majority A9 state into the retained herald
global tick 2n+1:   commit one native A9 tick and clear the herald
```

It therefore makes eight A9 quarter-turns in its exact sixteen-global-tick
complete-state period:

\[
 T_M=16,qquad w_M=2,qquad {w_M\over T_M}={1\over8}.  \tag{5}
\]

If only the phase coordinate is tracked, the primitive presentation is
`(w,T)=(1,8)`. If the complete material state is tracked, it is `(2,16)`.
These are the same physical cadence, as required by the parent theorem's tick
refinement invariance.

The exact blocked angular cadence is consequently

\[
 \boxed{\omega_M=2\pi{w_M\over T_M}={\pi\over4}}
 \quad\hbox{per global tick}.                           \tag{6}
\]

---

## 4. Reduced coupling ladder

On the momentum-neutral reciprocal-absorption branch, the parent compliance
identity is

\[
 \chi_{\rm EM}={\omega_M\over d}.                      \tag{7}
\]

Equations (5)--(7) and the registered cotangent readout give

\[
 \chi_{\rm EM}={\pi\over4d},
 \qquad
 \boxed{\alpha_{\rm native}={3\over8d}}.              \tag{8}
\]

Thus the exact triplet clock reduces the prior three-integer ladder to a
one-integer ladder. The only remaining discrete datum on this branch is:

> `d` = the number of complete field packets whose reciprocal absorption
> creates one material action quantum.

No value of `d` is chosen here. It must be counted in a physical local
transaction that also retains packet identity, energy, recoil, and the
inverse emission history.

---

## 5. Recoil branch

With the parent's dimensionless recoil ratio

\[
 r={|p|^2\over2m\Gamma},                               \tag{9}
\]

the exact reduction is

\[
 \boxed{\alpha_{\rm native}={3\over8(d-r)}}.           \tag{10}
\]

Equation (10) is not a permission to tune `r`. The common microscopic law
must either derive the recoil ratio or prove that the operational absorption
branch is momentum neutral.

---

## 6. Why Phi-v12 does not supply `d`

The genesis-seeded Phi-v12 transaction rearranges

\[
 (6,3,0,1)_{\rm seed}\longmapsto(6,3,0,1)_{\rm body}
                                                               \tag{11}
\]

with no complete field packet absorbed or emitted. Its packet debit is

\[
 d_{\rm assembly}=0.                                  \tag{12}
\]

But equations (7)--(8) require `d>0`. The A2 phase advance that marks assembly
and the A2 READY/EXCITED work distinction are not complete field packets.
Calling either one a packet debit would be an unproved cross-carrier
identification and a prohibited substitution identity.

The formation theorem makes the clock physically reachable from a nonblank
seed. It does not normalize its coupling to radiation.

---

## 7. Exact boundary

Established:

1. all sixteen A9 states have exact period eight;
2. every A9 tick advances the C4 phase by one quarter-turn;
3. every complete A9 orbit contains exactly two windings;
4. the triplet makes eight quarter-turns in sixteen global ticks;
5. `w/T=1/8` is independent of initial phase and polarity;
6. `omega_M=pi/4` per global tick;
7. the momentum-neutral ladder is `3/(8d)`;
8. the recoil ladder is `3/[8(d-r)]`; and
9. the Phi-v12 seed transaction has zero packet debit.

Not established:

1. a positive integer packet debit into one material action quantum;
2. vanishing or derived recoil;
3. a common charged static and transverse radiative pole;
4. equality of the pole residue and free-field Hessian;
5. formation and protection of the absorption apparatus under canonical Phi;
   or
6. the physical interacting curvature and fine-structure normalization.

Reproduce with:

```powershell
python scripts/proofs/proof_v3_triplet_phase_winding_one_integer_coupling_ladder.py
```

Expected result: `12/12` exact checks pass.

The honest conclusion is:

> **The material clock fixes the coupling cadence, not the light-to-matter
> action exchange. FTD now owes one physical packet-debit/recoil theorem and
> one common protected pole, rather than three arbitrary clock integers.**

### Successor result: the minimum prepared port closes negative

The later
[`minimum counterpropagating packet clock-port theorem`](THEOREM_V3_MINIMUM_COUNTERPROPAGATING_PACKET_TRIPLET_CLOCK_PORT_AND_SHARED_CURVATURE_BOUNDARY_v1.md)
proves that two opposite unit-energy `r/6` packets are the unique minimum
moving-current-neutral batch and that their complete 32-record payload gates
one triplet COMMIT with exact inverse over 36,864 rows. Thus `d=2` for that
selected port. Its conditional branch is `chi=pi/8` and
`alpha_native=3/16`, which closes the Occam-minimum port negative as the
physical fine-structure normalization. The result fixes a prepared debit; it
does not derive field Noether momentum, the common static/free curvature,
packet refill/protection, or the canonical interaction.
