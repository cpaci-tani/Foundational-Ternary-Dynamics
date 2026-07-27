# FTD-0421 — Exact native conserved-charge gate

**Date:** 2026-07-22  
**Status:** `[THEOREM — exact finite transition algebra]` + `[ENGINE VERIFIED — observer neutrality/event coverage]` + `[CLOSED NEGATIVE — preregistered additive basis]`  
**Verdict:** `NATIVE-ADDITIVE-U1-CHARGE-NULLSPACE-TRIVIAL-FOR-FROZEN-TICK`

## Result

The preregistered local feature vector is

$$
f=(|s|,s,h,sh),
$$

with `h` the sign of dual-substrate chirality and zero when no discrete
chirality label exists. For a putative additive charge
`q=a|s|+bs+ch+dsh`, invariance requires every allowed event row to annihilate
`(a,b,c,d)`.

A rank-generating subset is

$$
\begin{pmatrix}
1& 1& 0&0\\
1&-1& 0&0\\
1& 1& 1&1\\
1&-1&-1&1
\end{pmatrix}.
$$

The first two rows force `a=b=0`; the last two then force `c=d=0`. The full
matrix additionally includes neutral pair production and both signs of single-
and dual-substrate weak transmutation. Exact rational elimination gives

$$
\operatorname{rank}M=4,
\qquad
\dim\ker M=0.
$$

Movement has zero global feature change. Evaporation and annihilation add the
negative creation rows and cannot restore a null direction.

## Independent engine gate

The read-only journal records movement, genesis, evaporation, pair production,
annihilation, and weak transmutation directly at the accepted event sites.
Observer-on and observer-off runs have identical selected-state hashes and RNG
state hashes. The engine test independently confirms that movement preserves
all four global features, pair creation/annihilation preserve signed `s` while
changing occupancy, genesis changes occupancy and signed `s`, and weak
transmutation changes signed `s`.

## Correct statement

The existing reaction-transport identity

$$
\Delta s+\operatorname{div}j_s=S_{\rm reaction}
$$

remains valid bookkeeping. It is not a source-free continuity law and cannot
serve as a native `U(1)` gauge current. The Wilson/link current remains a
selected auxiliary current, not a current derived from `(s,J)` histories.

This closure is scoped to the locked discrete feature basis and frozen tick.
Expanding the basis to fitted real functions of flux or changing an event rule
would be a new ontology/model branch, prohibited in this cycle.

**Verifier:** `engine/tests/test_native_charge_gate.cpp` and
`scripts/proofs/proof_native_dual_track_lock.py`.
